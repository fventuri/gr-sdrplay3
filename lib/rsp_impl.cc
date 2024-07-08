/* -*- c++ -*- */
/*
 * Copyright 2020-2024 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "rsp_impl.h"
#include "sdrplay_api.h"

namespace gr {
namespace sdrplay3 {

static constexpr double SDRPLAY_SAMPLE_RATE_MIN = 62.5e3;
static constexpr double SDRPLAY_SAMPLE_RATE_MAX = 10.66e6;
static constexpr double SDRPLAY_FREQ_MIN = 1e3;
static constexpr double SDRPLAY_FREQ_MAX = 2000e6;

const std::map<std::string, struct rsp_impl::_output_type> rsp_impl::output_types = {
    { "fc32", { OutputType::fc32, sizeof(gr_complex) } },
    { "sc16", { OutputType::sc16, sizeof(short[2]) } }
};

/**********************************************************************
 * Structors
 *********************************************************************/
rsp::rsp(const std::string& name, gr::io_signature::sptr output_signature)
    : gr::sync_block(name, gr::io_signature::make(0, 0, 0), output_signature)
{
    // nop
}

rsp_impl::rsp_impl(const unsigned char hwVer,
                   const std::string& selector,
                   const struct stream_args_t& stream_args,
                   std::function<bool()> specific_select) :
    output_type(output_types.at(stream_args.output_type).output_type)
{
    sdrplay_api::get_instance();

    sdrplay_api_ErrT err;
    err = sdrplay_api_LockDeviceApi();
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_LockDeviceApi() Error: {}", sdrplay_api_GetErrorString(err));
    }

    bool device_valid = rsp_select(hwVer, selector);
    if (device_valid && specific_select) {
        device_valid = specific_select();
    }
    if (device_valid) {
        err = sdrplay_api_SelectDevice(&device);
        if (err != sdrplay_api_Success) {
            d_logger->error("sdrplay_api_SelectDevice() Error: {}", sdrplay_api_GetErrorString(err));
        }
    }

    err = sdrplay_api_UnlockDeviceApi();
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_UnlockDeviceApi() Error: {}", sdrplay_api_GetErrorString(err));
    }

    if (!device_valid) {
        throw std::runtime_error("sdrplay device not found or invalid mode/antenna selection");
    }

    err = sdrplay_api_GetDeviceParams(device.dev, &device_params);
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_GetDeviceParams() Error: {}", sdrplay_api_GetErrorString(err));
    }
    if (device_params) {
        rx_channel_params = device.tuner != sdrplay_api_Tuner_B ?
                                            device_params->rxChannelA :
                                            device_params->rxChannelB;
    }

    sample_rate = 0;
    nchannels = 1;
    run_status = RunStatus::idle;

    ring_buffers[0].xi = nullptr;
    ring_buffers[0].xq = nullptr;
    ring_buffers[0].head = 0;
    ring_buffers[0].tail = 0;
    ring_buffers[1].xi = nullptr;
    ring_buffers[1].xq = nullptr;
    ring_buffers[1].head = 0;
    ring_buffers[1].tail = 0;

    sample_sequence_gaps_check = false;
    show_gain_changes = false;
}

rsp_impl::~rsp_impl()
{
    if (run_status >= RunStatus::init)
        stop();

    // unset the ring buffers
    ring_buffers[0].xi = nullptr;
    ring_buffers[0].xq = nullptr;
    ring_buffers[1].xi = nullptr;
    ring_buffers[1].xq = nullptr;

    d_logger->info("total samples: [{},{}]", ring_buffers[0].head, ring_buffers[1].head);

    sdrplay_api_ErrT err;
    err = sdrplay_api_ReleaseDevice(&device);
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_ReleaseDevice() Error: {}", sdrplay_api_GetErrorString(err));
    }
}

io_signature::sptr rsp_impl::args_to_io_sig(const struct stream_args_t& args) const
{
    const int nchan = std::max<int>(static_cast<int>(args.channels_size), 1);
    const int size = static_cast<int>(output_types.at(args.output_type).size);
    return io_signature::make(nchan, nchan, size);
}


// Sample rate methods
double rsp_impl::set_sample_rate(const double rate, const bool synchronous)
{
    auto sample_rate_range = get_sample_rate_range();
    if (rate < sample_rate_range[0] || rate > sample_rate_range[1]) {
        d_logger->warn("invalid sample rate: {:g}Hz", rate);
        return get_sample_rate();
    }
    if (rate == sample_rate)
        return get_sample_rate();

    int decimation;
    double fsHz;
    sdrplay_api_If_kHzT if_type;
#ifdef USE_LOWIF
    if (rate == 62.5e3 || rate == 125e3 || rate == 250e3 || rate == 500e3 ||
        rate == 1000e3 || rate == 2000e3) {
        decimation = int(2000e3 / rate);
        fsHz = 6000e3;
        sample_rate = 2000e3 / decimation;
        if_type = sdrplay_api_IF_1_620;
    } else {
#endif /* USE_LOWIF */
        for (decimation = 1; decimation <= 32; decimation *= 2) {
            fsHz = rate * decimation;
            if (fsHz >= 2000e3) {
                break;
            }
        }
        if (decimation > 32) {
            d_logger->warn("invalid sample rate: {:g}Hz", rate);
            return get_sample_rate();
        }
        sample_rate = rate;
        if_type = sdrplay_api_IF_Zero;
#ifdef USE_LOWIF
    }
#endif /* USE_LOWIF */
    update_sample_rate_and_decimation(fsHz, decimation, if_type, synchronous);
    return get_sample_rate();
}

double rsp_impl::get_sample_rate() const
{
    return sample_rate;
}

const double (&rsp_impl::get_sample_rate_range() const)[2]
{
    static const double sample_rate_range[] = { SDRPLAY_SAMPLE_RATE_MIN, SDRPLAY_SAMPLE_RATE_MAX };
    return sample_rate_range;
}

void rsp_impl::update_sample_rate_and_decimation(double fsHz, int decimation,
                                                 sdrplay_api_If_kHzT if_type,
                                                 const bool synchronous)
{
    sdrplay_api_ReasonForUpdateT reason = sdrplay_api_Update_None;
    if (device_params->devParams && fsHz != device_params->devParams->fsFreq.fsHz) {
        device_params->devParams->fsFreq.fsHz = fsHz;
        reason = (sdrplay_api_ReasonForUpdateT)(reason | sdrplay_api_Update_Dev_Fs);
    }
    sdrplay_api_DecimationT *decimation_params = &rx_channel_params->ctrlParams.decimation;
    unsigned char decimation_factor = static_cast<unsigned char>(decimation);
    if (decimation_factor != decimation_params->decimationFactor) {
        decimation_params->decimationFactor = decimation_factor;
        decimation_params->enable = decimation_factor > 1;
        reason = (sdrplay_api_ReasonForUpdateT)(reason | sdrplay_api_Update_Ctrl_Decimation);
    }
    sdrplay_api_TunerParamsT *tuner_params = &rx_channel_params->tunerParams;
    if (if_type != tuner_params->ifType) {
        tuner_params->ifType = if_type;
        reason = (sdrplay_api_ReasonForUpdateT)(reason | sdrplay_api_Update_Tuner_IfType);
    }

    // set the bandwidth to the largest value compatible with the sample rate
    sdrplay_api_Bw_MHzT bw_type = get_auto_bandwidth();
    if (bw_type != tuner_params->bwType) {
        tuner_params->bwType = bw_type;
        reason = (sdrplay_api_ReasonForUpdateT)(reason | sdrplay_api_Update_Tuner_BwType);
    }

    update_if_streaming(reason, synchronous);
}


// Center frequency methods
double rsp_impl::set_center_freq(const double freq, const bool synchronous)
{
    if (freq == rx_channel_params->tunerParams.rfFreq.rfHz)
        return get_center_freq();
    rx_channel_params->tunerParams.rfFreq.rfHz = freq;
    update_if_streaming(sdrplay_api_Update_Tuner_Frf, synchronous);
    return get_center_freq();
}

double rsp_impl::get_center_freq() const
{
    return rx_channel_params->tunerParams.rfFreq.rfHz;
}

const double (&rsp_impl::get_freq_range() const)[2]
{
    static const double freq_range[] = { SDRPLAY_FREQ_MIN, SDRPLAY_FREQ_MAX };
    return freq_range;
}


// Bandwidth methods
double rsp_impl::set_bandwidth(const double bandwidth)
{
    if (bandwidth > sample_rate)
        d_logger->warn("bandwidth: {:g} is greater than sample rate: {:g}", bandwidth, sample_rate);

    sdrplay_api_Bw_MHzT bw_type;
    // bandwidth == 0 means 'AUTO', i.e. the largest bandwidth compatible with the sample rate
    if      (bandwidth ==     0) { bw_type = get_auto_bandwidth(); }
    else if (bandwidth <  300e3) { bw_type = sdrplay_api_BW_0_200; }
    else if (bandwidth <  600e3) { bw_type = sdrplay_api_BW_0_300; }
    else if (bandwidth < 1536e3) { bw_type = sdrplay_api_BW_0_600; }
    else if (bandwidth < 5000e3) { bw_type = sdrplay_api_BW_1_536; }
    else if (bandwidth < 6000e3) { bw_type = sdrplay_api_BW_5_000; }
    else if (bandwidth < 7000e3) { bw_type = sdrplay_api_BW_6_000; }
    else if (bandwidth < 8000e3) { bw_type = sdrplay_api_BW_7_000; }
    else                         { bw_type = sdrplay_api_BW_8_000; }

    if (bw_type == rx_channel_params->tunerParams.bwType)
        return get_bandwidth();
    rx_channel_params->tunerParams.bwType = bw_type;
    update_if_streaming(sdrplay_api_Update_Tuner_BwType);
    return get_bandwidth();
}

double rsp_impl::get_bandwidth() const
{
    sdrplay_api_Bw_MHzT bw_type = rx_channel_params->tunerParams.bwType;
    if      (bw_type == sdrplay_api_BW_0_200) { return  200e3; }
    else if (bw_type == sdrplay_api_BW_0_300) { return  300e3; }
    else if (bw_type == sdrplay_api_BW_0_600) { return  600e3; }
    else if (bw_type == sdrplay_api_BW_1_536) { return 1536e3; }
    else if (bw_type == sdrplay_api_BW_5_000) { return 5000e3; }
    else if (bw_type == sdrplay_api_BW_6_000) { return 6000e3; }
    else if (bw_type == sdrplay_api_BW_7_000) { return 7000e3; }
    else if (bw_type == sdrplay_api_BW_8_000) { return 8000e3; }
    else                                      { return      0; }
}

const std::vector<double> rsp_impl::get_bandwidths() const
{
    static const std::vector<double> bandwidths = { 200e3, 300e3, 600e3,
            1536e3, 5000e3, 6000e3, 7000e3, 8000e3 };
    return bandwidths;
}

// get the largest bandwidth compatible with the sample rate
sdrplay_api_Bw_MHzT rsp_impl::get_auto_bandwidth() const
{
    double largest_compatible_bw = 0;
    for (const double bandwidth : get_bandwidths()) {
        if (largest_compatible_bw == 0)
            largest_compatible_bw = bandwidth;
        if (bandwidth > sample_rate)
            break;
        largest_compatible_bw = bandwidth;
    }
    if      (largest_compatible_bw <  300e3) { return sdrplay_api_BW_0_200; }
    else if (largest_compatible_bw <  600e3) { return sdrplay_api_BW_0_300; }
    else if (largest_compatible_bw < 1536e3) { return sdrplay_api_BW_0_600; }
    else if (largest_compatible_bw < 5000e3) { return sdrplay_api_BW_1_536; }
    else if (largest_compatible_bw < 6000e3) { return sdrplay_api_BW_5_000; }
    else if (largest_compatible_bw < 7000e3) { return sdrplay_api_BW_6_000; }
    else if (largest_compatible_bw < 8000e3) { return sdrplay_api_BW_7_000; }
    else                                     { return sdrplay_api_BW_8_000; }
}


// Gain methods
const std::vector<std::string> rsp_impl::get_gain_names() const
{
    static const std::vector<std::string> gain_names = { "IF", "RF", "LNAstate" };
    return gain_names;
}

double rsp_impl::set_gain(const double gain, const std::string& name,
                          const bool synchronous)
{
    if (name == "IF") {
        return set_if_gain(gain, synchronous);
    } else if (name == "RF") {
        return set_rf_gain(gain, rf_gr_values(), synchronous);
    } else if (name == "LNAstate") {
        return set_lna_state(gain, rf_gr_values(), synchronous);
    }
    d_logger->error("invalid gain name: {}", name);
    return 0;
}

double rsp_impl::get_gain(const std::string& name) const
{
    if (name == "IF") {
        return get_if_gain();
    } else if (name == "RF") {
        return get_rf_gain(rf_gr_values());
    } else if (name == "LNAstate") {
        return get_lna_state();
    }
    d_logger->error("invalid gain name: {}", name);
    return 0;
}

const double (&rsp_impl::get_gain_range(const std::string& name) const)[2]
{
    if (name == "IF") {
        return get_if_gain_range();
    } else if (name == "RF") {
        return get_rf_gain_range(rf_gr_values());
    } else if (name == "LNAstate") {
        auto LNAstate_range = get_lna_state_range(rf_gr_values());
        static const double LNAstate_range_double[] = {
            static_cast<double>(LNAstate_range[0]),
            static_cast<double>(LNAstate_range[1])
        };
        return LNAstate_range_double;
    }
    d_logger->error("invalid gain name: {}", name);
    static const double null_gain_range[] = { 0, 0 };
    return null_gain_range;
}

double rsp_impl::set_if_gain(const double gain, const bool synchronous)
{
    unsigned int gRdB = static_cast<unsigned int>(-gain);
    if (gRdB == rx_channel_params->tunerParams.gain.gRdB)
        return get_if_gain();
    rx_channel_params->tunerParams.gain.gRdB = gRdB;
    update_if_streaming(sdrplay_api_Update_Tuner_Gr, synchronous);
    return get_if_gain();
}

double rsp_impl::set_rf_gain(const double gain, const std::vector<int> rf_gRs,
                             const bool synchronous)
{
    unsigned char LNAstate = get_closest_LNAstate(gain, rf_gRs);
    if (LNAstate == rx_channel_params->tunerParams.gain.LNAstate)
        return get_rf_gain(rf_gRs);
    rx_channel_params->tunerParams.gain.LNAstate = LNAstate;
    update_if_streaming(sdrplay_api_Update_Tuner_Gr, synchronous);
    return get_rf_gain(rf_gRs);
}

unsigned char rsp_impl::get_closest_LNAstate(const double gain,
                                             const std::vector<int> rf_gRs)
{
    int gRdB = static_cast<int>(-gain);
    // since the sequence of rf_gR values is not necessarily monotonic, we
    // can't use bisection here, but we have to go value by value
    unsigned char LNAstate = 0;
    int min_diff = abs(gRdB);
    for (int i = 0; i < (int)(rf_gRs.size()); ++i) {
        int diff = abs(gRdB - rf_gRs.at(i));
        if (diff < min_diff) {
            LNAstate = static_cast<unsigned char>(i);
            min_diff = diff;
        }
    }
    return LNAstate;
}

int rsp_impl::set_lna_state(const int LNAstate, const std::vector<int> rf_gRs,
                            const bool synchronous)
{
    if (LNAstate < 0 || LNAstate >= rf_gRs.size()) {
        d_logger->error("invalid LNA state: {}", LNAstate);
    } else {
        rx_channel_params->tunerParams.gain.LNAstate = LNAstate;
        update_if_streaming(sdrplay_api_Update_Tuner_Gr, synchronous);
    }
    return rx_channel_params->tunerParams.gain.LNAstate;
}

double rsp_impl::get_if_gain() const
{
    return -static_cast<double>(rx_channel_params->tunerParams.gain.gRdB);
}

double rsp_impl::get_rf_gain(const std::vector<int> rf_gRs) const
{
    unsigned char LNAstate = rx_channel_params->tunerParams.gain.LNAstate;
    return static_cast<double>(-rf_gRs.at(static_cast<unsigned int>(LNAstate)));
}

int rsp_impl::get_lna_state() const
{
    return rx_channel_params->tunerParams.gain.LNAstate;
}

const double (&rsp_impl::get_if_gain_range() const)[2]
{
    static const double if_gain_range[] = { -(MAX_BB_GR), -(sdrplay_api_NORMAL_MIN_GR) };
    return if_gain_range;
}

const double (&rsp_impl::get_rf_gain_range(const std::vector<int> rf_gRs) const)[2]
{
    auto rf_gR_bounds = std::minmax_element(rf_gRs.begin(), rf_gRs.end());
    static const double rf_gain_range[] = {
                     static_cast<double>(-(*rf_gR_bounds.second)),
                     static_cast<double>(-(*rf_gR_bounds.first))
    };
    return rf_gain_range;
}

const int (&rsp_impl::get_lna_state_range(const std::vector<int> rf_gRs) const)[2]
{
    static const int LNAstate_range[] = { 0,
                                          static_cast<int>(rf_gRs.size()) - 1 };
    return LNAstate_range;
}

bool rsp_impl::set_gain_mode(bool automatic)
{
    sdrplay_api_AgcT *agc = &rx_channel_params->ctrlParams.agc;
    if (automatic && agc->enable == sdrplay_api_AGC_DISABLE) {
        // enable AGC
        agc->enable = sdrplay_api_AGC_50HZ;
    } else if (!automatic && agc->enable != sdrplay_api_AGC_DISABLE) {
        // disable AGC
        agc->enable = sdrplay_api_AGC_DISABLE;
    } else {
        return get_gain_mode();
    }

    update_if_streaming(sdrplay_api_Update_Ctrl_Agc);
    return get_gain_mode();
}

bool rsp_impl::get_gain_mode() const
{
    return rx_channel_params->ctrlParams.agc.enable != sdrplay_api_AGC_DISABLE;
}


// Miscellaneous stuff
double rsp_impl::set_freq_corr(const double freq)
{
    if (!device_params->devParams || freq == device_params->devParams->ppm)
        return get_freq_corr();
    device_params->devParams->ppm = freq;
    update_if_streaming(sdrplay_api_Update_Dev_Ppm);
    return get_freq_corr();
}

double rsp_impl::get_freq_corr() const
{
    return device_params->devParams ? device_params->devParams->ppm : 0.0;
}

void rsp_impl::set_dc_offset_mode(bool enable)
{
    unsigned char dc_enable = enable ? 1 : 0;
    if (dc_enable == rx_channel_params->ctrlParams.dcOffset.DCenable)
        return;
    rx_channel_params->ctrlParams.dcOffset.DCenable = dc_enable;
    // fv - not sure if these are needed
    sdrplay_api_TunerParamsT &tuner_params = rx_channel_params->tunerParams;
    tuner_params.dcOffsetTuner.dcCal = 4;
    tuner_params.dcOffsetTuner.speedUp = 0;
    tuner_params.dcOffsetTuner.trackTime = 63;
    update_if_streaming(sdrplay_api_Update_Ctrl_DCoffsetIQimbalance);
}

void rsp_impl::set_iq_balance_mode(bool enable)
{
    unsigned char iq_enable = enable ? 1 : 0;
    if (iq_enable == rx_channel_params->ctrlParams.dcOffset.IQenable)
        return;
    rx_channel_params->ctrlParams.dcOffset.DCenable = 1;
    rx_channel_params->ctrlParams.dcOffset.IQenable = iq_enable;
    update_if_streaming(sdrplay_api_Update_Ctrl_DCoffsetIQimbalance);
}

void rsp_impl::set_agc_setpoint(double set_point)
{
    int set_point_dBfs = static_cast<int>(set_point);
    if (set_point_dBfs == rx_channel_params->ctrlParams.agc.setPoint_dBfs)
        return;
    rx_channel_params->ctrlParams.agc.setPoint_dBfs = set_point_dBfs;
    update_if_streaming(sdrplay_api_Update_Ctrl_Agc);
}


// Streaming methods
static void sample_copy_fc32(size_t start, size_t end, int noutput_items,
                             short *xi, short *xq, void *out);
static void sample_copy_sc16(size_t start, size_t end, int noutput_items,
                             short *xi, short *xq, void *out);

bool rsp_impl::start()
{
    //print_device_config();

    if (!start_api_init())
        return false;
    run_status = RunStatus::init;
    return true;
}

bool rsp_impl::stop()
{
    sdrplay_api_ErrT err;
    if (run_status >= RunStatus::init) {
        err = sdrplay_api_Uninit(device.dev);
        if (err != sdrplay_api_Success) {
            d_logger->error("sdrplay_api_Uninit() Error: {}", sdrplay_api_GetErrorString(err));
            return false;
        }
    }
    run_status = RunStatus::idle;

    // notify the callback threads so they can terminate
    ring_buffers[0].tail = ring_buffers[0].head;
    ring_buffers[1].tail = ring_buffers[1].head;
    ring_buffers[0].overflow.notify_one();
    ring_buffers[1].overflow.notify_one();

    return true;
}

int rsp_impl::work(int noutput_items,
                   gr_vector_const_void_star& input_items,
                   gr_vector_void_star& output_items)
{
    if (run_status < RunStatus::init)
        return 0;
    run_status = RunStatus::streaming;

    int nstreams = static_cast<int>(output_items.size());
    // start from the highest stream and go down to stream 0 since the streams
    // are produced in ascending order and we want to make sure we have at
    // least the same number of samples to return
    for (int stream_index = nstreams - 1; stream_index >= 0; --stream_index) {
        auto& ring_buffer = ring_buffers[stream_index];
        void *out = output_items[stream_index];

        std::unique_lock<std::mutex> lock(ring_buffer.mtx);

        ring_buffer.empty.wait(lock, [&ring_buffer]() {
                return ring_buffer.tail < ring_buffer.head;
        });

        int nsamples = ring_buffer.head - ring_buffer.tail;
        noutput_items = std::min(nsamples, noutput_items);
        uint64_t new_tail = ring_buffer.tail + noutput_items;
        size_t start = static_cast<size_t>(ring_buffer.tail & RingBufferMask);
        size_t end = static_cast<size_t>(new_tail & RingBufferMask);
        if (output_type == OutputType::fc32) {
            sample_copy_fc32(start, end, noutput_items, ring_buffer.xi,
                             ring_buffer.xq, output_items[stream_index]);
        } else if (output_type == OutputType::sc16) {
            sample_copy_sc16(start, end, noutput_items, ring_buffer.xi,
                             ring_buffer.xq, output_items[stream_index]);
        }
        ring_buffer.tail = new_tail;

        ring_buffer.overflow.notify_one();
    }

    return noutput_items;
}

bool rsp_impl::start_api_init()
{
    // set the ring buffers
    ring_buffers[0].xi = new short[RingBufferSize];
    ring_buffers[0].xq = new short[RingBufferSize];
    ring_buffers[0].head = 0;
    ring_buffers[0].tail = 0;
    ring_buffers[1].xi = nchannels > 1 ? new short[RingBufferSize] : nullptr;
    ring_buffers[1].xq = nchannels > 1 ? new short[RingBufferSize] : nullptr;
    ring_buffers[1].head = 0;
    ring_buffers[1].tail = 0;

    sdrplay_api_CallbackFnsT callbackFns = {
        stream_A_callback,
        stream_B_callback,
        event_callback,
    };
    sdrplay_api_ErrT err;
    err = sdrplay_api_Init(device.dev, &callbackFns, this);
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_Init() Error: {}", sdrplay_api_GetErrorString(err));
        return false;
    }
    return true;
}

static void sample_copy_fc32(size_t start, size_t end, int noutput_items,
                             short *xi, short *xq, void *out)
{
    if (noutput_items == 0)
        return;
    short *from = xi + start;
    gr_complex *to = static_cast<gr_complex *>(out);
    if (end > start || end == 0) {
        // no wrap around case
        size_t size = noutput_items;
        for (int i = 0; i < size; ++i)
            to[i].real(static_cast<float>(from[i]) / 32768.0f);
        from = xq + start;
        for (int i = 0; i < size; ++i)
            to[i].imag(static_cast<float>(from[i]) / 32768.0f);
    } else {
        // wrap around case
        size_t size = noutput_items - end;
        for (int i = 0; i < size; ++i)
            to[i].real(static_cast<float>(from[i]) / 32768.0f);
        from = xq + start;
        for (int i = 0; i < size; ++i)
            to[i].imag(static_cast<float>(from[i]) / 32768.0f);
        from = xi;
        to += size;
        size = end;
        for (int i = 0; i < size; ++i)
            to[i].real(static_cast<float>(from[i]) / 32768.0f);
        from = xq;
        for (int i = 0; i < size; ++i)
            to[i].imag(static_cast<float>(from[i]) / 32768.0f);
    }
    return;
}

static void sample_copy_sc16(size_t start, size_t end, int noutput_items,
                             short *xi, short *xq, void *out)
{
    if (noutput_items == 0)
        return;
    short *from = xi + start;
    short (*to)[2] = static_cast<short (*)[2]>(out);
    if (end > start || end == 0) {
        // no wrap around case
        size_t size = noutput_items;
        for (int i = 0; i < size; ++i)
            to[i][0] = from[i];
        from = xq + start;
        for (int i = 0; i < size; ++i)
            to[i][1] = from[i];
    } else {
        // wrap around case
        size_t size = noutput_items - end;
        for (int i = 0; i < size; ++i)
            to[i][0] = from[i];
        from = xq + start;
        for (int i = 0; i < size; ++i)
            to[i][1] = from[i];
        from = xi;
        to += size;
        size = end;
        for (int i = 0; i < size; ++i)
            to[i][0] = from[i];
        from = xq;
        for (int i = 0; i < size; ++i)
            to[i][1] = from[i];
    }
    return;
}


// callback functions
static void sample_gaps_check(unsigned int num_samples,
                              unsigned int first_sample_num,
                              unsigned int& next_sample_num,
                              gr::logger_ptr logger,
                              int stream_index);

void rsp_impl::stream_A_callback(short *xi, short *xq,
                                 sdrplay_api_StreamCbParamsT *params,
                                 unsigned int numSamples, unsigned int reset,
                                 void *cbContext)
{
    rsp_impl *rsp = static_cast<rsp_impl *>(cbContext);
    if (rsp->sample_sequence_gaps_check) {
        static unsigned int next_sample_num = 0;
        sample_gaps_check(numSamples, params->firstSampleNum, next_sample_num,
                          rsp->d_logger, 0);
    }
    rsp->sample_rate_changed |= params->fsChanged;
    rsp->frequency_changed |= params->rfChanged;
    rsp->gain_reduction_changed |= params->grChanged;
    if (params->fsChanged || params->rfChanged || params->grChanged) {
        std::lock_guard<std::mutex> value_changed_lock(rsp->value_changed_mutex);
        rsp->value_changed_cv.notify_all();
    }
    rsp->stream_callback(xi, xq, params, numSamples, reset, 0);
}

void rsp_impl::stream_B_callback(short *xi, short *xq,
                                 sdrplay_api_StreamCbParamsT *params,
                                 unsigned int numSamples, unsigned int reset,
                                 void *cbContext)
{
    rsp_impl *rsp = static_cast<rsp_impl *>(cbContext);
    if (rsp->sample_sequence_gaps_check) {
        static unsigned int next_sample_num = 0;
        sample_gaps_check(numSamples, params->firstSampleNum, next_sample_num,
                          rsp->d_logger, 1);
    }
    rsp->sample_rate_changed |= params->fsChanged;
    rsp->frequency_changed |= params->rfChanged;
    rsp->gain_reduction_changed |= params->grChanged;
    if (params->fsChanged || params->rfChanged || params->grChanged) {
        std::lock_guard<std::mutex> value_changed_lock(rsp->value_changed_mutex);
        rsp->value_changed_cv.notify_all();
    }
    rsp->stream_callback(xi, xq, params, numSamples, reset, 1);
}

void rsp_impl::event_callback(sdrplay_api_EventT eventId,
                              sdrplay_api_TunerSelectT tuner,
                              sdrplay_api_EventParamsT *params,
                              void *cbContext)
{
    rsp_impl *rsp = static_cast<rsp_impl *>(cbContext);
    rsp->event_callback(eventId, tuner, params);
}

void rsp_impl::stream_callback(short *xi, short *xq,
                               sdrplay_api_StreamCbParamsT *params,
                               unsigned int numSamples, unsigned int reset,
                               int stream_index)
{
    int ring_buffer_overflow = RingBufferSize - numSamples;
    auto& ring_buffer = ring_buffers[stream_index];

    std::unique_lock<std::mutex> lock(ring_buffer.mtx);

    ring_buffer.overflow.wait(lock, [&ring_buffer, ring_buffer_overflow]() {
            return ring_buffer.tail + ring_buffer_overflow >= ring_buffer.head;
    });

    if (run_status != RunStatus::streaming) {
        return;
    }

    uint64_t new_head = ring_buffer.head + numSamples;
    size_t start = static_cast<size_t>(ring_buffer.head & RingBufferMask);
    size_t end = static_cast<size_t>(new_head & RingBufferMask);
    if (end > start || end == 0) {
        // no wrap around - just one memcpy for I and Q
        size_t memcpy_size = numSamples * sizeof(short);
        std::memcpy(ring_buffer.xi + start, xi, memcpy_size);
        std::memcpy(ring_buffer.xq + start, xq, memcpy_size);
    } else {
        // wrap around - two memcpy's for I and two for Q
        size_t first = numSamples - end;
        size_t memcpy_size_first = first * sizeof(short);
        size_t memcpy_size_rest = end * sizeof(short);
        std::memcpy(ring_buffer.xi + start, xi, memcpy_size_first);
        std::memcpy(ring_buffer.xi, xi + first, memcpy_size_rest);
        std::memcpy(ring_buffer.xq + start, xq, memcpy_size_first);
        std::memcpy(ring_buffer.xq, xq + first, memcpy_size_rest);
    }
    ring_buffer.head = new_head;

    ring_buffer.empty.notify_one();

    return;
}

void rsp_impl::event_callback(sdrplay_api_EventT eventId,
                              sdrplay_api_TunerSelectT tuner,
                              sdrplay_api_EventParamsT *params)
{
    switch (eventId) {
    case sdrplay_api_GainChange:
        if (show_gain_changes) {
            sdrplay_api_GainCbParamT *gainParams = &params->gainParams;
            d_logger->info("gain change - gRdB={} lnaGRdB={} currGain={:.2f}", gainParams->gRdB, gainParams->lnaGRdB, gainParams->currGain);
        }
        break;
    case sdrplay_api_PowerOverloadChange:
        // send ack back for overload events
        if (run_status == RunStatus::streaming) {
            switch (params->powerOverloadParams.powerOverloadChangeType) {
            case sdrplay_api_Overload_Detected:
                d_logger->warn("overload detected - please reduce gain");
                break;
            case sdrplay_api_Overload_Corrected:
                d_logger->warn("overload corrected");
                break;
            }
            sdrplay_api_Update(device.dev, device.tuner,
                               sdrplay_api_Update_Ctrl_OverloadMsgAck,
                               sdrplay_api_Update_Ext1_None);
        }
        break;
    case sdrplay_api_DeviceRemoved:
        d_logger->error("device removed");
        break;
    case sdrplay_api_RspDuoModeChange:
        break;
    case sdrplay_api_DeviceFailure:
        d_logger->error("device failure");
        break;
    }
}


// Debug methods
void rsp_impl::set_debug_mode(bool enable)
{
    sdrplay_api_ErrT err;
    err = sdrplay_api_DebugEnable(device.dev, enable ? sdrplay_api_DbgLvl_Verbose : sdrplay_api_DbgLvl_Disable);
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_DebugEnable() Error: {}", sdrplay_api_GetErrorString(err));
    }
}

void rsp_impl::set_sample_sequence_gaps_check(bool enable)
{
    sample_sequence_gaps_check = enable;
}

void rsp_impl::set_show_gain_changes(bool enable)
{
    show_gain_changes = enable;
}


// internal methods
bool rsp_impl::rsp_select(const unsigned char hwVer, const std::string& selector)
{
    unsigned int ndevices = SDRPLAY_MAX_DEVICES;
    sdrplay_api_DeviceT devices[SDRPLAY_MAX_DEVICES];
    sdrplay_api_ErrT err;
    err = sdrplay_api_GetDevices(devices, &ndevices, ndevices);
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_GetDevices() Error: {}", sdrplay_api_GetErrorString(err));
    }
    // device index or serial number?
    bool device_found = false;
    if (selector.size() <= 2) {
        unsigned int device_index = 0;
        try {
            device_index = std::stoi(selector);
        } catch (std::invalid_argument&) {
            // if the sector is empty or it is just '' or "", choose device 0
            // otherwise make it return a device-not-found error
            if (!(selector == "" || selector == "''" || selector == "\"\"")) {
                device_index = ndevices;
            }
        }
        if (device_index < ndevices && 
                devices[device_index].valid &&
                hwVer == devices[device_index].hwVer) {
            device = devices[device_index];
            device_found = true;
        }
    } else {
        // look for the serial number
        for (unsigned int device_index = 0; device_index < ndevices; ++device_index) {
            if (devices[device_index].valid &&
                    hwVer == devices[device_index].hwVer &&
                    selector == devices[device_index].SerNo) {
                device = devices[device_index];
                device_found = true;
                break;
            }
        }
    }
    if (!device_found) {
        d_logger->error("SDRplay device not found: {}", selector);
    }

    return device_found;
}

void rsp_impl::update_if_streaming(sdrplay_api_ReasonForUpdateT reason_for_update,
                                   const bool synchronous)
{
    update_if_streaming(reason_for_update, device.tuner, synchronous);
}

static const std::string reason_as_text(sdrplay_api_ReasonForUpdateT reason_for_update);
void rsp_impl::update_if_streaming(sdrplay_api_ReasonForUpdateT reason_for_update,
                                   sdrplay_api_TunerSelectT tuner,
                                   const bool synchronous)
{
    if (run_status == RunStatus::idle || reason_for_update == sdrplay_api_Update_None)
        return;
    if (synchronous) {
        if (reason_for_update & sdrplay_api_Update_Dev_Fs)
            sample_rate_changed = 0;
        if (reason_for_update & sdrplay_api_Update_Tuner_Frf)
            frequency_changed = 0;
        if (reason_for_update & sdrplay_api_Update_Tuner_Gr)
            gain_reduction_changed = 0;
    }
    sdrplay_api_ErrT err;
    err = sdrplay_api_Update(device.dev, tuner, reason_for_update,
                             sdrplay_api_Update_Ext1_None);
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_Update({}) Error: {}", reason_as_text(reason_for_update), sdrplay_api_GetErrorString(err));
    }
    if (synchronous) {
        if (reason_for_update & sdrplay_api_Update_Dev_Fs) {
            std::unique_lock<std::mutex> lock(value_changed_mutex);
            if (!value_changed_cv.wait_for(lock, update_timeout, [this]{return sample_rate_changed;})) {
               d_logger->warn("sample rate update timeout.");
            }
        }
        if (reason_for_update & sdrplay_api_Update_Tuner_Frf) {
            std::unique_lock<std::mutex> lock(value_changed_mutex);
            if (!value_changed_cv.wait_for(lock, update_timeout, [this]{return frequency_changed;})) {
               d_logger->warn("RF center frequency update timeout.");
            }
        }
        if (reason_for_update & sdrplay_api_Update_Tuner_Gr) {
            std::unique_lock<std::mutex> lock(value_changed_mutex);
            if (!value_changed_cv.wait_for(lock, update_timeout, [this]{return gain_reduction_changed;})) {
               d_logger->warn("gain reduction update timeout.");
            }
        }
    }
}

static const std::string reason_as_text(sdrplay_api_ReasonForUpdateT reason_for_update)
{
    static const std::unordered_map<sdrplay_api_ReasonForUpdateT, const std::string, std::hash<int>> reasons = {
        { sdrplay_api_Update_Dev_Fs, "Dev_Fs" },
        { sdrplay_api_Update_Dev_Ppm, "Dev_Ppm" },
        { sdrplay_api_Update_Dev_SyncUpdate, "Dev_SyncUpdate" },
        { sdrplay_api_Update_Dev_ResetFlags, "Dev_ResetFlags" },
        { sdrplay_api_Update_Rsp1a_BiasTControl, "Rsp1a_BiasTControl" },
        { sdrplay_api_Update_Rsp1a_RfNotchControl, "Rsp1a_RfNotchControl" },
        { sdrplay_api_Update_Rsp1a_RfDabNotchControl, "Rsp1a_RfDabNotchControl" },
        { sdrplay_api_Update_Rsp2_BiasTControl, "Rsp2_BiasTControl" },
        { sdrplay_api_Update_Rsp2_AmPortSelect, "Rsp2_AmPortSelect" },
        { sdrplay_api_Update_Rsp2_AntennaControl, "Rsp2_AntennaControl" },
        { sdrplay_api_Update_Rsp2_RfNotchControl, "Rsp2_RfNotchControl" },
        { sdrplay_api_Update_Rsp2_ExtRefControl, "Rsp2_ExtRefControl" },
        { sdrplay_api_Update_RspDuo_ExtRefControl, "RspDuo_ExtRefControl" },
        { sdrplay_api_Update_Master_Spare_1, "Master_Spare_1" },
        { sdrplay_api_Update_Master_Spare_2, "Master_Spare_2" },
        { sdrplay_api_Update_Tuner_Gr, "Tuner_Gr" },
        { sdrplay_api_Update_Tuner_GrLimits, "Tuner_GrLimits" },
        { sdrplay_api_Update_Tuner_Frf, "Tuner_Frf" },
        { sdrplay_api_Update_Tuner_BwType, "Tuner_BwType" },
        { sdrplay_api_Update_Tuner_IfType, "Tuner_IfType" },
        { sdrplay_api_Update_Tuner_DcOffset, "Tuner_DcOffset" },
        { sdrplay_api_Update_Tuner_LoMode, "Tuner_LoMode" },
        { sdrplay_api_Update_Ctrl_DCoffsetIQimbalance, "Ctrl_DCoffsetIQimbalance" },
        { sdrplay_api_Update_Ctrl_Decimation, "Ctrl_Decimation" },
        { sdrplay_api_Update_Ctrl_Agc, "Ctrl_Agc" },
        { sdrplay_api_Update_Ctrl_AdsbMode, "Ctrl_AdsbMode" },
        { sdrplay_api_Update_Ctrl_OverloadMsgAck, "Ctrl_OverloadMsgAck" },
        { sdrplay_api_Update_RspDuo_BiasTControl, "RspDuo_BiasTControl" },
        { sdrplay_api_Update_RspDuo_AmPortSelect, "RspDuo_AmPortSelect" },
        { sdrplay_api_Update_RspDuo_Tuner1AmNotchControl, "RspDuo_Tuner1AmNotchControl" },
        { sdrplay_api_Update_RspDuo_RfNotchControl, "RspDuo_RfNotchControl" },
        { sdrplay_api_Update_RspDuo_RfDabNotchControl, "RspDuo_RfDabNotchControl" }
    };
    try {
        return reasons.at(reason_for_update);
    } catch (std::out_of_range&) {
        std::string reason;
        // multiple reasons - append them to reason string
        for (auto r : reasons) {
            if (reason_for_update & r.first) {
                if (!reason.empty()) reason.append("+");
                reason.append(r.second);
            }
        }
        return reason;
    }
}

void rsp_impl::print_device_config() const
{
    sdrplay_api_DeviceParamsT *params;
    sdrplay_api_ErrT err = sdrplay_api_GetDeviceParams(device.dev, &params);
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_GetDeviceParams() Error: {}", sdrplay_api_GetErrorString(err));
        return;
    }
    std::cerr << std::endl;
    std::cerr << "# Device config:" << std::endl;
    for (auto rx_channel : { params->rxChannelA, params->rxChannelB }) {
        std::cerr << "RX channel=" << (rx_channel == params->rxChannelA ? "A" :
                   (rx_channel == params->rxChannelB ? "B" : "?")) << std::endl;
        if (!rx_channel)
            continue;
        sdrplay_api_TunerParamsT *tunerParams = &rx_channel->tunerParams;
        std::cerr << "    rfHz=" << tunerParams->rfFreq.rfHz << std::endl;
        std::cerr << "    bwType=" << tunerParams->bwType << std::endl;
        std::cerr << "    ifType=" << tunerParams->ifType << std::endl;
        sdrplay_api_DecimationT *decimation = &rx_channel->ctrlParams.decimation;
        std::cerr << "    decimationFactor=" << static_cast<unsigned int>(decimation->decimationFactor) << std::endl;
        std::cerr << "    decimation.enable=" << static_cast<unsigned int>(decimation->enable) << std::endl;
        std::cerr << "    gain.gRdB=" << tunerParams->gain.gRdB << std::endl;
        std::cerr << "    gain.LNAstate=" << static_cast<unsigned int>(tunerParams->gain.LNAstate) << std::endl;
        sdrplay_api_AgcT *agc = &rx_channel->ctrlParams.agc;
        std::cerr << "    agc.enable=" << agc->enable << std::endl;
        std::cerr << "    agc.setPoint_dBfs=" << agc->setPoint_dBfs << std::endl;
        std::cerr << "    agc.attack_ms=" << agc->attack_ms << std::endl;
        std::cerr << "    agc.decay_ms=" << agc->decay_ms << std::endl;
        std::cerr << "    agc.decay_delay_ms=" << agc->decay_delay_ms << std::endl;
        std::cerr << "    agc.decay_threshold_dB=" << agc->decay_threshold_dB << std::endl;
        std::cerr << "    agc.syncUpdate=" << agc->syncUpdate << std::endl;
        std::cerr << "    dcOffset.DCenable=" << static_cast<unsigned int>(rx_channel->ctrlParams.dcOffset.DCenable) << std::endl;
        std::cerr << "    dcOffsetTuner.dcCal=" << static_cast<unsigned int>(tunerParams->dcOffsetTuner.dcCal) << std::endl;
        std::cerr << "    dcOffsetTuner.speedUp=" << static_cast<unsigned int>(tunerParams->dcOffsetTuner.speedUp) << std::endl;
        std::cerr << "    dcOffsetTuner.trackTime=" << tunerParams->dcOffsetTuner.trackTime << std::endl;
    }
    std::cerr << std::endl;
    if (params->devParams) {
        std::cerr << "fsHz=" << params->devParams->fsFreq.fsHz << std::endl;
        std::cerr << "ppm=" << params->devParams->ppm << std::endl;
    }
    std::cerr << std::endl;
    return;
}

void sample_gaps_check(unsigned int num_samples, unsigned int first_sample_num,
                       unsigned int& next_sample_num,
                       gr::logger_ptr logger, int stream_index)
{
    if (next_sample_num != 0 && next_sample_num != first_sample_num) {
        unsigned int sample_num_gap;
        if (next_sample_num < first_sample_num) {
            sample_num_gap = first_sample_num - next_sample_num;
        } else {
            sample_num_gap = UINT_MAX - (first_sample_num - next_sample_num) + 1;
        }
        logger->warn("sample num gap in stream {}: {} [{}:{}] -> {}+{}",
                     stream_index, sample_num_gap, next_sample_num,
                     first_sample_num, sample_num_gap / num_samples,
                     sample_num_gap % num_samples);
    }
    next_sample_num = first_sample_num + num_samples;
}

} /* namespace sdrplay3 */
} /* namespace gr */
