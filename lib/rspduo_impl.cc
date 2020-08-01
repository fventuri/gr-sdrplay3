/* -*- c++ -*- */
/*
 * Copyright 2020 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rspduo_impl.h"

namespace gr {
namespace sdrplay3 {

rspduo::sptr rspduo::make(const std::string& selector,
                          const std::string& rspduo_mode,
                          const std::string& antenna,
                          const struct stream_args_t& stream_args)
{
    //return gnuradio::get_initial_sptr(new rspduo_impl(selector));
    return rspduo::sptr(new rspduo_impl(selector, rspduo_mode, antenna,
                                        stream_args));
}

struct _rspduo_mode {
    sdrplay_api_RspDuoModeT rspduo_mode;
    double rspduo_sample_freq;
    bool dual_mode_independent_rx;
};

static const std::map<std::string, struct _rspduo_mode> rspduo_modes = {
    { "Single Tuner", { sdrplay_api_RspDuoMode_Single_Tuner, 0, false } },
    { "Dual Tuner (diversity reception)", { sdrplay_api_RspDuoMode_Dual_Tuner, 6e6, false } },
    { "Dual Tuner (independent RX)", { sdrplay_api_RspDuoMode_Dual_Tuner, 6e6, true } },
    { "Master", { sdrplay_api_RspDuoMode_Master, 6e6, false } },
    { "Master (SR=8Mhz)", { sdrplay_api_RspDuoMode_Master, 8e6, false } },
    { "Slave", { sdrplay_api_RspDuoMode_Slave, 0, false } }
};

rspduo_impl::rspduo_impl(const std::string& selector,
                         const std::string& rspduo_mode,
                         const std::string& antenna,
                         const struct stream_args_t& stream_args)
      : rsp("rspduo", args_to_io_sig(stream_args)),
        rsp_impl(SDRPLAY_RSPduo_ID, selector, stream_args,
                 [&]() { return rspduo_select(rspduo_mode, antenna); })
{
    nchannels = device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner ? 2 : 1;
    dual_mode_independent_rx = rspduo_modes.at(rspduo_mode).dual_mode_independent_rx;
    rspduo_mode_change_type = sdrplay_api_SlaveAttached;
    if (device.rspDuoMode == sdrplay_api_RspDuoMode_Master) {
        rspduo_mode_change_type = sdrplay_api_SlaveDllDisappeared;
    }
}

rspduo_impl::~rspduo_impl() {}


// Sample rate methods
double rspduo_impl::set_sample_rate(const double rate)
{
    if (device.rspDuoMode == sdrplay_api_RspDuoMode_Single_Tuner) {
        return rsp_impl::set_sample_rate(rate);
    }
    std::vector<double> valid_rates = get_sample_rates();
    if (!std::binary_search(valid_rates.begin(), valid_rates.end(), rate)) {
        GR_LOG_WARN(d_logger, boost::format("invalid sample rate: %lgHz") % rate);
        return get_sample_rate();
    }
    if (rate == sample_rate)
        return get_sample_rate();

    int decimation = int(2000e3 / rate);
    double fsHz = device.rspDuoSampleFreq;
    sample_rate = 2000e3 / decimation;
    sdrplay_api_If_kHzT if_type = device.rspDuoSampleFreq != 8000e3 ?
                                  sdrplay_api_IF_1_620 : sdrplay_api_IF_2_048;
    update_sample_rate_and_decimation(fsHz, decimation, if_type);
    return get_sample_rate();
}

const std::vector<double> rspduo_impl::get_sample_rates() const
{
    if (device.rspDuoMode == sdrplay_api_RspDuoMode_Single_Tuner)
        return rsp_impl::get_sample_rates();
    static const std::vector<double> rates = { 62.5e3, 125e3, 250e3, 500e3,
            1000e3, 2000e3 };
    return rates;
}


// Center frequency methods
double rspduo_impl::set_center_freq(const double freq, const int tuner)
{
    get_independent_rx_channel_params(tuner)->tunerParams.rfFreq.rfHz = freq;
    update_if_streaming(sdrplay_api_Update_Tuner_Frf,
                        get_independent_rx_tuner(tuner));
    return get_center_freq(tuner);
}

double rspduo_impl::get_center_freq(const int tuner) const
{
    return get_independent_rx_channel_params(tuner)->tunerParams.rfFreq.rfHz;
}


// Antenna methods
struct _antenna {
    sdrplay_api_TunerSelectT tuner;
    bool high_z;
};

static const std::map<std::string, struct _antenna> antennas = {
    { "Tuner 1 50 ohm", { sdrplay_api_Tuner_A,    false } },
    { "Tuner 2 50 ohm", { sdrplay_api_Tuner_B,    false } },
    { "High Z",         { sdrplay_api_Tuner_A,    true } },
    { "Both Tuners",    { sdrplay_api_Tuner_Both, false } }
};

const std::string rspduo_impl::set_antenna(const std::string& antenna)
{
    if (device.rspDuoMode == sdrplay_api_RspDuoMode_Single_Tuner ||
        device.rspDuoMode == sdrplay_api_RspDuoMode_Master) {
        if (!(antennas.at(antenna).tuner == sdrplay_api_Tuner_A ||
              antennas.at(antenna).tuner == sdrplay_api_Tuner_B)) {
            GR_LOG_WARN(d_logger, boost::format("invalid antenna: %s") % antenna);
            return get_antenna();
        }
    } else if (device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner ||
               device.rspDuoMode == sdrplay_api_RspDuoMode_Slave) {
        if (!(antennas.at(antenna).tuner == device.tuner)) {
            GR_LOG_WARN(d_logger, boost::format("invalid antenna: %s") % antenna);
            return get_antenna();
        }
    }

    sdrplay_api_TunerSelectT tuner = antennas.at(antenna).tuner;
    sdrplay_api_RspDuo_AmPortSelectT& device_tuner1AmPortSel =
                        rx_channel_params->rspDuoTunerParams.tuner1AmPortSel;
    sdrplay_api_RspDuo_AmPortSelectT tuner1AmPortSel =
                antennas.at(antenna).high_z ? sdrplay_api_RspDuo_AMPORT_1 :
                                           sdrplay_api_RspDuo_AMPORT_2;

    if (tuner == device.tuner && tuner1AmPortSel == device_tuner1AmPortSel) {
        return get_antenna();
    }

    if (run_status == RunStatus::idle) {
        device.tuner = tuner;
        device_tuner1AmPortSel = tuner1AmPortSel;
        return get_antenna();
    }

    // we are streaming here: do we need to switch tuner?
    sdrplay_api_ErrT err;
    if (tuner != device.tuner) {
        if (device.rspDuoMode == sdrplay_api_RspDuoMode_Single_Tuner) {
            err = sdrplay_api_SwapRspDuoActiveTuner(device.dev,
                                                    &device.tuner,
                                                    tuner1AmPortSel);
            if (err != sdrplay_api_Success) {
                GR_LOG_ERROR(d_logger, boost::format("sdrplay_api_SwapRspDuoActiveTuner() Error: %s") % sdrplay_api_GetErrorString(err));
            }
            rx_channel_params = device.tuner != sdrplay_api_Tuner_B ?
                                                device_params->rxChannelA :
                                                device_params->rxChannelB;
            device_tuner1AmPortSel = rx_channel_params->rspDuoTunerParams.tuner1AmPortSel;
        } else if (device.rspDuoMode == sdrplay_api_RspDuoMode_Master) {
            // can't change tuner if a slave is attached
            if (rspduo_mode_change_type != sdrplay_api_SlaveDllDisappeared) {
                GR_LOG_WARN(d_logger, "cannot change tuner in master mode while a slave is attached");
                tuner1AmPortSel = rx_channel_params->rspDuoTunerParams.tuner1AmPortSel;
            } else {
                // stop, change tuner, and restart
                stop();
                device.tuner = tuner;
                start();
                device_tuner1AmPortSel = rx_channel_params->rspDuoTunerParams.tuner1AmPortSel;
            }
        }
    }

    // we are streaming here: do we need to switch the AM port?
    if (tuner1AmPortSel != device_tuner1AmPortSel) {
        device_tuner1AmPortSel = tuner1AmPortSel;
        update_if_streaming(sdrplay_api_Update_RspDuo_AmPortSelect);
    }

    return get_antenna();
}

const std::string rspduo_impl::get_antenna() const
{
    bool high_z = rx_channel_params->rspDuoTunerParams.tuner1AmPortSel ==
                  sdrplay_api_RspDuo_AMPORT_1;
    for (const auto& antenna : antennas) {
        if (antenna.second.tuner == device.tuner &&
            antenna.second.high_z == high_z) {
            return antenna.first;
        }
    }
    return "Unknown";
}

const std::vector<std::string> rspduo_impl::get_antennas() const
{
    std::vector<std::string> antenna_names = {};
    if (device.rspDuoMode == sdrplay_api_RspDuoMode_Single_Tuner ||
        device.rspDuoMode == sdrplay_api_RspDuoMode_Master) {
        for (const auto& antenna : antennas) {
            if (antenna.second.tuner == sdrplay_api_Tuner_A ||
                antenna.second.tuner == sdrplay_api_Tuner_B) {
                antenna_names.push_back(antenna.first);
            }
        }
    } else if (device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner ||
               device.rspDuoMode == sdrplay_api_RspDuoMode_Slave) {
        for (const auto& antenna : antennas) {
            if (antenna.second.tuner == device.tuner) {
                antenna_names.push_back(antenna.first);
            }
        }
    }
    return antenna_names;
}


// Gain methods
double rspduo_impl::set_gain(const double gain, const std::string& name, const int tuner)
{
    if (name == "IF") {
        return set_if_gain(gain, tuner);
    } else if (name == "RF") {
        return set_rf_gain(gain, rf_gr_values(), tuner);
    }
    GR_LOG_ERROR(d_logger, boost::format("invalid gain name: %s") % name);
    return 0;
}

double rspduo_impl::get_gain(const std::string& name, const int tuner) const
{
    if (name == "IF") {
        return get_if_gain(tuner);
    } else if (name == "RF") {
        return get_rf_gain(rf_gr_values(tuner), tuner);
    }
    GR_LOG_ERROR(d_logger, boost::format("invalid gain name: %s") % name);
    return 0;
}

const double (&rspduo_impl::get_gain_range(const std::string& name, const int tuner) const)[2]
{
    if (name == "IF") {
        return get_if_gain_range();
    } else if (name == "RF") {
        return get_rf_gain_range(rf_gr_values(tuner));
    }
    GR_LOG_ERROR(d_logger, boost::format("invalid gain name: %s") % name);
    static const double null_gain_range[] = { 0, 0 };
    return null_gain_range;
}

const std::vector<int> rspduo_impl::rf_gr_values(const double freq, const bool highz)
{
    if (freq <= 60e6 && !highz) {
        static const std::vector<int> rf_gr = { 0, 6, 12, 18, 37, 42, 61 };
        return rf_gr;
    } else if (freq <= 60e6 && highz) {
        static const std::vector<int> rf_gr = { 0, 6, 12, 18, 37 };
        return rf_gr;
    } else if (freq <= 420e6) {
        static const std::vector<int> rf_gr = { 0, 6, 12, 18, 20, 26, 32, 38, 57, 62 };
        return rf_gr;
    } else if (freq <= 1000e6) {
        static const std::vector<int> rf_gr = { 0, 7, 13, 19, 20, 27, 33, 39, 45, 64 };
        return rf_gr;
    } else if (freq <= 2000e6) {
        static const std::vector<int> rf_gr = { 0, 6, 12, 20, 26, 32, 38, 43, 62 };
        return rf_gr;
    } else {
        return std::vector<int>();
    }
}

const std::vector<int> rspduo_impl::rf_gr_values() const
{
    bool highz = device.tuner == sdrplay_api_Tuner_A &&
                 rx_channel_params->rspDuoTunerParams.tuner1AmPortSel == sdrplay_api_RspDuo_AMPORT_1;
    return rf_gr_values(rx_channel_params->tunerParams.rfFreq.rfHz, highz);
}

const std::vector<int> rspduo_impl::rf_gr_values(const int tuner) const
{
    sdrplay_api_RxChannelParamsT *indrx_chparams =
                                  get_independent_rx_channel_params(tuner);
    bool highz = device.tuner == sdrplay_api_Tuner_A &&
                 indrx_chparams->rspDuoTunerParams.tuner1AmPortSel == sdrplay_api_RspDuo_AMPORT_1;
    return rf_gr_values(indrx_chparams->tunerParams.rfFreq.rfHz, highz);
}

double rspduo_impl::set_if_gain(const double gain, const int tuner)
{
    unsigned int gRdB = static_cast<unsigned int>(-gain);
    sdrplay_api_RxChannelParamsT *indrx_chparams =
                                  get_independent_rx_channel_params(tuner);
    if (gRdB == indrx_chparams->tunerParams.gain.gRdB)
        return get_if_gain(tuner);
    indrx_chparams->tunerParams.gain.gRdB = gRdB;
    update_if_streaming(sdrplay_api_Update_Tuner_Gr,
                        get_independent_rx_tuner(tuner));
    return get_if_gain(tuner);
}

double rspduo_impl::set_rf_gain(const double gain, const std::vector<int> rf_gRs,
                                const int tuner)
{
    unsigned char LNAstate = get_closest_LNAstate(gain, rf_gRs);
    sdrplay_api_RxChannelParamsT *indrx_chparams =
                                  get_independent_rx_channel_params(tuner);
    if (LNAstate == indrx_chparams->tunerParams.gain.LNAstate)
        return get_rf_gain(rf_gRs, tuner);
    indrx_chparams->tunerParams.gain.LNAstate = LNAstate;
    update_if_streaming(sdrplay_api_Update_Tuner_Gr,
                        get_independent_rx_tuner(tuner));
    return get_rf_gain(rf_gRs, tuner);
}

double rspduo_impl::get_if_gain(const int tuner) const
{
    sdrplay_api_RxChannelParamsT *indrx_chparams =
                                  get_independent_rx_channel_params(tuner);
    return -static_cast<double>(indrx_chparams->tunerParams.gain.gRdB);
}

double rspduo_impl::get_rf_gain(const std::vector<int> rf_gRs, const int tuner) const
{
    sdrplay_api_RxChannelParamsT *indrx_chparams =
                                  get_independent_rx_channel_params(tuner);
    unsigned char LNAstate = indrx_chparams->tunerParams.gain.LNAstate;
    return static_cast<double>(-rf_gRs.at(static_cast<unsigned int>(LNAstate)));
}

bool rspduo_impl::set_gain_mode(bool automatic, const int tuner)
{
    sdrplay_api_RxChannelParamsT *indrx_chparams =
                                  get_independent_rx_channel_params(tuner);
    sdrplay_api_AgcT *agc = &indrx_chparams->ctrlParams.agc;
    agc->setPoint_dBfs = 0;
    agc->attack_ms = 0;
    agc->decay_ms = 0;
    agc->decay_delay_ms = 0;
    agc->decay_threshold_dB = 0;
    agc->syncUpdate = 0;
    if (automatic && agc->enable != sdrplay_api_AGC_CTRL_EN) {
        // enable AGC
        agc->enable = sdrplay_api_AGC_CTRL_EN;
        agc->setPoint_dBfs = -30; /*TODO: magic number */
    } else if (!automatic && agc->enable != sdrplay_api_AGC_DISABLE) {
        // disable AGC
        agc->enable = sdrplay_api_AGC_DISABLE;
        agc->setPoint_dBfs = -30; /*TODO: magic number */
    } else {
        return get_gain_mode(tuner);
    }

    update_if_streaming(sdrplay_api_Update_Ctrl_Agc,
                        get_independent_rx_tuner(tuner));
    return get_gain_mode(tuner);
}

bool rspduo_impl::get_gain_mode(const int tuner) const {
    sdrplay_api_RxChannelParamsT *indrx_chparams =
                                  get_independent_rx_channel_params(tuner);
    return indrx_chparams->ctrlParams.agc.enable != sdrplay_api_AGC_DISABLE;
}


// Miscellaneous stuff
void rspduo_impl::set_rf_notch_filter(bool enable)
{
    unsigned char rf_notch_enable = enable ? 1 : 0;
    if (rf_notch_enable == rx_channel_params->rspDuoTunerParams.rfNotchEnable)
        return;
    rx_channel_params->rspDuoTunerParams.rfNotchEnable = rf_notch_enable;
    update_if_streaming(sdrplay_api_Update_RspDuo_RfNotchControl);
}

void rspduo_impl::set_dab_notch_filter(bool enable)
{
    unsigned char dab_notch_enable = enable ? 1 : 0;
    if (dab_notch_enable == rx_channel_params->rspDuoTunerParams.rfDabNotchEnable)
        return;
    rx_channel_params->rspDuoTunerParams.rfDabNotchEnable = dab_notch_enable;
    update_if_streaming(sdrplay_api_Update_RspDuo_RfDabNotchControl);
}

void rspduo_impl::set_am_notch_filter(bool enable)
{
    unsigned char am_notch_enable = enable ? 1 : 0;
    if (am_notch_enable == rx_channel_params->rspDuoTunerParams.tuner1AmNotchEnable)
        return;
    rx_channel_params->rspDuoTunerParams.tuner1AmNotchEnable = am_notch_enable;
    update_if_streaming(sdrplay_api_Update_RspDuo_Tuner1AmNotchControl);
}

void rspduo_impl::set_biasT(bool enable)
{
    unsigned char biasT_enable = enable ? 1 : 0;
    if (biasT_enable == rx_channel_params->rspDuoTunerParams.biasTEnable)
        return;
    rx_channel_params->rspDuoTunerParams.biasTEnable = biasT_enable;
    update_if_streaming(sdrplay_api_Update_RspDuo_BiasTControl);
}


// callback functions
void rspduo_impl::event_callback(sdrplay_api_EventT eventId,
                                 sdrplay_api_TunerSelectT tuner,
                                 sdrplay_api_EventParamsT *params)
{
    if (eventId == sdrplay_api_RspDuoModeChange) {
        // save last RSPduo mode change
        rspduo_mode_change_type = params->rspDuoModeParams.modeChangeType;
        GR_LOG_INFO(d_logger, boost::format("RSPduo mode change - modeChangeType=%u") % rspduo_mode_change_type);
    } else {
        rsp_impl::event_callback(eventId, tuner, params);
    }
}


// internal methods
bool rspduo_impl::rspduo_select(const std::string& rspduo_mode,
                                const std::string& antenna)
{
    // RSPduo mode
    bool valid_mode = device.rspDuoMode & rspduo_modes.at(rspduo_mode).rspduo_mode;
    if (valid_mode) {
        device.rspDuoMode = rspduo_modes.at(rspduo_mode).rspduo_mode;
    } else {
        GR_LOG_ERROR(d_logger, boost::format("invalid RSPduo mode selection: %s") % rspduo_mode);
        return valid_mode;
    }

    // RSPduo sample rate
    if (rspduo_modes.at(rspduo_mode).rspduo_sample_freq > 0) {
        device.rspDuoSampleFreq = rspduo_modes.at(rspduo_mode).rspduo_sample_freq;
    }

    // RSPduo antenna
    bool valid_tuner = device.tuner & antennas.at(antenna).tuner;
    if (valid_tuner) {
        device.tuner = antennas.at(antenna).tuner;
    } else {
        GR_LOG_ERROR(d_logger, boost::format("invalid RSPduo antenna selection: %s") % antenna);
        return valid_tuner;
    }

    return true;
}

sdrplay_api_RxChannelParamsT *rspduo_impl::get_independent_rx_channel_params(int tuner) const
{
    if (!(device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner &&
          dual_mode_independent_rx)) {
        GR_LOG_WARN(d_logger, "invalid call to get_independent_rx_channel_params() - device is not in independent RX mode");
        return nullptr;
    }
    return tuner != 1 ?  device_params->rxChannelA : device_params->rxChannelB;
}

sdrplay_api_TunerSelectT rspduo_impl::get_independent_rx_tuner(const int tuner) const
{
    if (!(device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner &&
          dual_mode_independent_rx)) {
        GR_LOG_WARN(d_logger, "invalid call to get_independent_rx_tuner() - device is not in independent RX mode");
        return sdrplay_api_Tuner_Neither;
    }
    return tuner != 1 ?  sdrplay_api_Tuner_A : sdrplay_api_Tuner_B;
}

void rspduo_impl::print_device_config() const
{
    rsp_impl::print_device_config();

    sdrplay_api_DeviceParamsT *params;
    sdrplay_api_ErrT err = sdrplay_api_GetDeviceParams(device.dev, &params);
    if (err != sdrplay_api_Success) {
        GR_LOG_ERROR(d_logger, boost::format("sdrplay_api_GetDeviceParams() Error: %s") % sdrplay_api_GetErrorString(err));
        return;
    }
    std::cerr << "# RSPduo specific config:" << std::endl;
    for (auto rx_channel : { params->rxChannelA, params->rxChannelB }) {
        std::cerr << "RX channel=" << (rx_channel == params->rxChannelA ? "A" :
                   (rx_channel == params->rxChannelB ? "B" : "?")) << std::endl;
        if (!rx_channel)
            continue;
        sdrplay_api_RspDuoTunerParamsT *rspDuoTunerParams = &rx_channel->rspDuoTunerParams;
        std::cerr << "    rspDuoTunerParams.tuner1AmPortSel=" << rspDuoTunerParams->tuner1AmPortSel << std::endl;
    }
    std::cerr << std::endl;
    return;
}

} /* namespace sdrplay3 */
} /* namespace gr */
