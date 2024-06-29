/* -*- c++ -*- */
/*
 * Copyright 2020-2024 Franco Venturi.
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
                         const struct stream_args_t& stream_args,
                         const std::string& name,
                         const unsigned char hwVer)
      : rsp(name, args_to_io_sig(stream_args)),
        rsp_impl(hwVer, selector, stream_args,
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
double rspduo_impl::set_sample_rate(const double rate, const bool synchronous)
{
    if (device.rspDuoMode == sdrplay_api_RspDuoMode_Single_Tuner) {
        return rsp_impl::set_sample_rate(rate);
    }
    std::vector<double> valid_rates = get_valid_sample_rates();
    if (!std::binary_search(valid_rates.begin(), valid_rates.end(), rate)) {
        d_logger->warn("invalid sample rate: {:g}Hz", rate);
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

const double (&rspduo_impl::get_sample_rate_range() const)[2]
{
    if (device.rspDuoMode == sdrplay_api_RspDuoMode_Single_Tuner) {
        return rsp_impl::get_sample_rate_range();
    }
    d_logger->warn("In Dual Tuner, Master, or Slave mode please use get_valid_sample_rates() instead");
    static const double null_sample_rate_range[] = { 0, 0 };
    return null_sample_rate_range;
}

const std::vector<double> rspduo_impl::get_valid_sample_rates() const
{
    if (device.rspDuoMode == sdrplay_api_RspDuoMode_Single_Tuner) {
        d_logger->warn("In Single Tuner mode please use get_sample_rate_range() instead");
        static const std::vector<double> empty_rates;
        return empty_rates;
    }
    static const std::vector<double> rates = { 62.5e3, 125e3, 250e3, 500e3,
            1000e3, 2000e3 };
    return rates;
}


// Center frequency methods
double rspduo_impl::set_center_freq(const double freq, const int tuner,
                                    const bool synchronous)
{
    get_independent_rx_channel_params(tuner)->tunerParams.rfFreq.rfHz = freq;
    update_if_streaming(sdrplay_api_Update_Tuner_Frf,
                        get_independent_rx_tuner(tuner));
    return get_center_freq(tuner);
}

void rspduo_impl::set_center_freq(const double freq_A, const double freq_B,
                                  const bool synchronous)
{
    if (!(device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner)) {
        d_logger->warn("invalid call to set_center_freq(freq_A, freq_B) - device is not in dual tuner mode");
        return;
    }
    sdrplay_api_TunerSelectT tuner = sdrplay_api_Tuner_Neither;
    if (device_params->rxChannelA->tunerParams.rfFreq.rfHz != freq_A) {
        tuner = sdrplay_api_Tuner_A;
        device_params->rxChannelA->tunerParams.rfFreq.rfHz = freq_A;
        if (device_params->rxChannelB->tunerParams.rfFreq.rfHz != freq_B) {
            tuner = sdrplay_api_Tuner_Both;
            device_params->rxChannelB->tunerParams.rfFreq.rfHz = freq_B;
        }
    } else if (device_params->rxChannelB->tunerParams.rfFreq.rfHz != freq_B) {
        tuner = sdrplay_api_Tuner_B;
        device_params->rxChannelB->tunerParams.rfFreq.rfHz = freq_B;
    }
    if (tuner != sdrplay_api_Tuner_Neither)
        update_if_streaming(sdrplay_api_Update_Tuner_Frf, tuner);
    return;
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
            d_logger->warn("invalid antenna: {}", antenna);
            return get_antenna();
        }
    } else if (device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner ||
               device.rspDuoMode == sdrplay_api_RspDuoMode_Slave) {
        if (!(antennas.at(antenna).tuner == device.tuner)) {
            d_logger->warn("invalid antenna: {}", antenna);
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
                d_logger->error("sdrplay_api_SwapRspDuoActiveTuner() Error: {}", sdrplay_api_GetErrorString(err));
            }
            rx_channel_params = device.tuner != sdrplay_api_Tuner_B ?
                                                device_params->rxChannelA :
                                                device_params->rxChannelB;
            device_tuner1AmPortSel = rx_channel_params->rspDuoTunerParams.tuner1AmPortSel;
        } else if (device.rspDuoMode == sdrplay_api_RspDuoMode_Master) {
            // can't change tuner if a slave is attached
            if (rspduo_mode_change_type != sdrplay_api_SlaveDllDisappeared) {
                d_logger->warn("cannot change tuner in master mode while a slave is attached");
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
double rspduo_impl::set_gain(const double gain, const std::string& name,
                             const int tuner, const bool synchronous)
{
    if (name == "IF") {
        return set_if_gain(gain, tuner, synchronous);
    } else if (name == "RF") {
        return set_rf_gain(gain, rf_gr_values(), tuner, synchronous);
    } else if (name == "LNAstate") {
        return set_lna_state(gain, rf_gr_values(), tuner, synchronous);
    }
    d_logger->error("invalid gain name: {}", name);
    return 0;
}

void rspduo_impl::set_gain(const double gain_A, const double gain_B,
                           const std::string& name, const bool synchronous)
{
    if (!(device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner)) {
        d_logger->warn("invalid call to set_gain(gain_A, gain_B) - device is not in dual tuner mode");
        return;
    }
    if (name == "IF") {
        set_if_gain(gain_A, gain_B, synchronous);
        return;
    } else if (name == "RF") {
        set_rf_gain(gain_A, gain_B, rf_gr_values(), synchronous);
        return;
    } else if (name == "LNAstate") {
        set_lna_state(gain_A, gain_B, rf_gr_values(), synchronous);
        return;
    }
    d_logger->error("invalid gain name: {}", name);
    return;
}

double rspduo_impl::get_gain(const std::string& name, const int tuner) const
{
    if (name == "IF") {
        return get_if_gain(tuner);
    } else if (name == "RF") {
        return get_rf_gain(rf_gr_values(tuner), tuner);
    } else if (name == "LNAstate") {
        return get_lna_state(tuner);
    }
    d_logger->error("invalid gain name: {}", name);
    return 0;
}

const double (&rspduo_impl::get_gain_range(const std::string& name, const int tuner) const)[2]
{
    if (name == "IF") {
        return get_if_gain_range();
    } else if (name == "RF") {
        return get_rf_gain_range(rf_gr_values(tuner));
    } else if (name == "LNAstate") {
        auto LNAstate_range = get_lna_state_range(rf_gr_values(tuner));
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

double rspduo_impl::set_if_gain(const double gain, const int tuner,
                                const bool synchronous)
{
    unsigned int gRdB = static_cast<unsigned int>(-gain);
    sdrplay_api_RxChannelParamsT *indrx_chparams =
                                  get_independent_rx_channel_params(tuner);
    if (gRdB == indrx_chparams->tunerParams.gain.gRdB)
        return get_if_gain(tuner);
    indrx_chparams->tunerParams.gain.gRdB = gRdB;
    update_if_streaming(sdrplay_api_Update_Tuner_Gr,
                        get_independent_rx_tuner(tuner), synchronous);
    return get_if_gain(tuner);
}

void rspduo_impl::set_if_gain(const double gain_A, const double gain_B,
                              const bool synchronous)
{
    unsigned int gRdB_A = static_cast<unsigned int>(-gain_A);
    unsigned int gRdB_B = static_cast<unsigned int>(-gain_B);
    sdrplay_api_TunerSelectT tuner = sdrplay_api_Tuner_Neither;
    if (device_params->rxChannelA->tunerParams.gain.gRdB != gRdB_A) {
        tuner = sdrplay_api_Tuner_A;
        device_params->rxChannelA->tunerParams.gain.gRdB = gRdB_A;
        if (device_params->rxChannelB->tunerParams.gain.gRdB != gRdB_B) {
            tuner = sdrplay_api_Tuner_Both;
            device_params->rxChannelB->tunerParams.gain.gRdB = gRdB_B;
        }
    } else if (device_params->rxChannelB->tunerParams.gain.gRdB != gRdB_B) {
        tuner = sdrplay_api_Tuner_B;
        device_params->rxChannelB->tunerParams.gain.gRdB = gRdB_B;
    }
    if (tuner != sdrplay_api_Tuner_Neither)
        update_if_streaming(sdrplay_api_Update_Tuner_Gr, tuner, synchronous);
    return;
}

double rspduo_impl::set_rf_gain(const double gain, const std::vector<int> rf_gRs,
                                const int tuner, const bool synchronous)
{
    unsigned char LNAstate = get_closest_LNAstate(gain, rf_gRs);
    sdrplay_api_RxChannelParamsT *indrx_chparams =
                                  get_independent_rx_channel_params(tuner);
    if (LNAstate == indrx_chparams->tunerParams.gain.LNAstate)
        return get_rf_gain(rf_gRs, tuner);
    indrx_chparams->tunerParams.gain.LNAstate = LNAstate;
    update_if_streaming(sdrplay_api_Update_Tuner_Gr,
                        get_independent_rx_tuner(tuner), synchronous);
    return get_rf_gain(rf_gRs, tuner);
}

void rspduo_impl::set_rf_gain(const double gain_A, const double gain_B,
                              const std::vector<int> rf_gRs,
                              const bool synchronous)
{
    unsigned char LNAstate_A = get_closest_LNAstate(gain_A, rf_gRs);
    unsigned char LNAstate_B = get_closest_LNAstate(gain_B, rf_gRs);
    sdrplay_api_TunerSelectT tuner = sdrplay_api_Tuner_Neither;
    if (device_params->rxChannelA->tunerParams.gain.LNAstate != LNAstate_A) {
        tuner = sdrplay_api_Tuner_A;
        device_params->rxChannelA->tunerParams.gain.LNAstate = LNAstate_A;
        if (device_params->rxChannelB->tunerParams.gain.LNAstate != LNAstate_B) {
            tuner = sdrplay_api_Tuner_Both;
            device_params->rxChannelB->tunerParams.gain.LNAstate = LNAstate_B;
        }
    } else if (device_params->rxChannelB->tunerParams.gain.LNAstate != LNAstate_B) {
        tuner = sdrplay_api_Tuner_B;
        device_params->rxChannelB->tunerParams.gain.LNAstate = LNAstate_B;
    }
    if (tuner != sdrplay_api_Tuner_Neither)
        update_if_streaming(sdrplay_api_Update_Tuner_Gr, tuner, synchronous);
    return;
}

int rspduo_impl::set_lna_state(const int LNAstate, const std::vector<int> rf_gRs,
                               const int tuner,
                               const bool synchronous)
{
    sdrplay_api_RxChannelParamsT *indrx_chparams =
                                  get_independent_rx_channel_params(tuner);
    if (LNAstate < 0 || LNAstate >= rf_gRs.size()) {
        d_logger->error("invalid LNA state: {}", LNAstate);
    } else {
        if (LNAstate == indrx_chparams->tunerParams.gain.LNAstate)
            return indrx_chparams->tunerParams.gain.LNAstate;
        indrx_chparams->tunerParams.gain.LNAstate = LNAstate;
        update_if_streaming(sdrplay_api_Update_Tuner_Gr,
                            get_independent_rx_tuner(tuner), synchronous);
    }
    return indrx_chparams->tunerParams.gain.LNAstate;
}

void rspduo_impl::set_lna_state(const int LNAstate_A, const int LNAstate_B,
                                const std::vector<int> rf_gRs,
                                const bool synchronous)
{
    if (LNAstate_A < 0 || LNAstate_A >= rf_gRs.size()) {
        d_logger->error("invalid LNA state: {}", LNAstate_A);
        return;
    }
    if (LNAstate_B < 0 || LNAstate_B >= rf_gRs.size()) {
        d_logger->error("invalid LNA state: {}", LNAstate_B);
        return;
    }
    sdrplay_api_TunerSelectT tuner = sdrplay_api_Tuner_Neither;
    if (device_params->rxChannelA->tunerParams.gain.LNAstate != LNAstate_A) {
        tuner = sdrplay_api_Tuner_A;
        device_params->rxChannelA->tunerParams.gain.LNAstate = LNAstate_A;
        if (device_params->rxChannelB->tunerParams.gain.LNAstate != LNAstate_B) {
            tuner = sdrplay_api_Tuner_Both;
            device_params->rxChannelB->tunerParams.gain.LNAstate = LNAstate_B;
        }
    } else if (device_params->rxChannelB->tunerParams.gain.LNAstate != LNAstate_B) {
        tuner = sdrplay_api_Tuner_B;
        device_params->rxChannelB->tunerParams.gain.LNAstate = LNAstate_B;
    }
    if (tuner != sdrplay_api_Tuner_Neither)
        update_if_streaming(sdrplay_api_Update_Tuner_Gr, tuner, synchronous);
    return;
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

int rspduo_impl::get_lna_state(const int tuner) const
{
    sdrplay_api_RxChannelParamsT *indrx_chparams =
                                  get_independent_rx_channel_params(tuner);
    return indrx_chparams->tunerParams.gain.LNAstate;
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

void rspduo_impl::set_gain_mode(bool automatic_A, bool automatic_B)
{
    if (!(device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner)) {
        d_logger->warn("invalid call to set_gain_mode(automatic_A, automatic_B) - device is not in dual tuner mode");
        return;
    }
    sdrplay_api_AgcControlT enable_A = automatic_A ? sdrplay_api_AGC_CTRL_EN : sdrplay_api_AGC_DISABLE;
    sdrplay_api_AgcControlT enable_B = automatic_B ? sdrplay_api_AGC_CTRL_EN : sdrplay_api_AGC_DISABLE;
    sdrplay_api_AgcT *agc_A = &device_params->rxChannelA->ctrlParams.agc;
    sdrplay_api_AgcT *agc_B = &device_params->rxChannelB->ctrlParams.agc;
    sdrplay_api_TunerSelectT tuner = sdrplay_api_Tuner_Neither;
    if (agc_A->enable != enable_A) {
        tuner = sdrplay_api_Tuner_A;
        agc_A->enable = enable_A;
        agc_A->setPoint_dBfs = -30; /*TODO: magic number */
        if (agc_B->enable != enable_B) {
            tuner = sdrplay_api_Tuner_Both;
            agc_B->enable = enable_B;
            agc_B->setPoint_dBfs = -30; /*TODO: magic number */
        }
    } else if (agc_B->enable != enable_B) {
        tuner = sdrplay_api_Tuner_B;
        agc_B->enable = enable_B;
        agc_B->setPoint_dBfs = -30; /*TODO: magic number */
    }
    if (tuner != sdrplay_api_Tuner_Neither)
        update_if_streaming(sdrplay_api_Update_Ctrl_Agc, tuner);

    return;
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


// Streaming methods
bool rspduo_impl::start()
{
    if (!(device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner &&
          dual_mode_independent_rx)) {
        return rsp_impl::start();
    }

    // since sdrplay_api_Init() resets channelB settings to channelA values,
    // we need to save all the settings for channelB before sdrplay_api_Init()
    // and reapply all those that the user can change (like center frequency)
    // only needed in dual tuner mode
    sdrplay_api_RxChannelParamsT rxChannelB = *device_params->rxChannelB;

    if (!rsp_impl::start_api_init())
        return false;

    // restore channelB settings that the user can change in independent RX
    // mode (which is an option available for dual tuner mode)
    if (device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner) {
        sdrplay_api_ReasonForUpdateT reason = sdrplay_api_Update_None;

        // 1. tuner params
        const sdrplay_api_TunerParamsT& tuner_before = rxChannelB.tunerParams;
        sdrplay_api_TunerParamsT& tuner = device_params->rxChannelB->tunerParams;
        // 1A. gains
        if (tuner.gain.gRdB != tuner_before.gain.gRdB) {
            tuner.gain.gRdB = tuner_before.gain.gRdB;
            reason = (sdrplay_api_ReasonForUpdateT)(reason | sdrplay_api_Update_Tuner_Gr);
        }
        if (tuner.gain.LNAstate != tuner_before.gain.LNAstate) {
            tuner.gain.LNAstate = tuner_before.gain.LNAstate;
            reason = (sdrplay_api_ReasonForUpdateT)(reason | sdrplay_api_Update_Tuner_Gr);
        }

        // 1B. frequency
        if (tuner.rfFreq.rfHz != tuner_before.rfFreq.rfHz) {
            tuner.rfFreq.rfHz = tuner_before.rfFreq.rfHz;
            reason = (sdrplay_api_ReasonForUpdateT)(reason | sdrplay_api_Update_Tuner_Frf);
        }

        // 2. control params
        const sdrplay_api_ControlParamsT& ctrl_before = rxChannelB.ctrlParams;
        sdrplay_api_ControlParamsT& ctrl = device_params->rxChannelB->ctrlParams;

        // 2A. AGC
        if (ctrl.agc.enable != ctrl_before.agc.enable) {
            ctrl.agc.enable = ctrl_before.agc.enable;
            reason = (sdrplay_api_ReasonForUpdateT)(reason | sdrplay_api_Update_Ctrl_Agc);
        }

        // 3. RSPduo tuner params
        const sdrplay_api_RspDuoTunerParamsT& rspDuoTuner_before = rxChannelB.rspDuoTunerParams;
        sdrplay_api_RspDuoTunerParamsT& rspDuoTuner = device_params->rxChannelB->rspDuoTunerParams;

        // 3A. AM port select
        if (rspDuoTuner.tuner1AmPortSel != rspDuoTuner_before.tuner1AmPortSel) {
            rspDuoTuner.tuner1AmPortSel = rspDuoTuner_before.tuner1AmPortSel;
            reason = (sdrplay_api_ReasonForUpdateT)(reason | sdrplay_api_Update_RspDuo_AmPortSelect);
        }

        if (reason != sdrplay_api_Update_None) {
            sdrplay_api_ErrT err;
            err = sdrplay_api_Update(device.dev, sdrplay_api_Tuner_B, reason, sdrplay_api_Update_Ext1_None);
            if (err != sdrplay_api_Success) {
                d_logger->error("sdrplay_api_Update({:0x%08x}) Error: {}", static_cast<unsigned int>(reason), sdrplay_api_GetErrorString(err));
                return false;
            }
        }
    }

    //print_device_config();
    run_status = RunStatus::init;
    return true;
}


// callback functions
void rspduo_impl::event_callback(sdrplay_api_EventT eventId,
                                 sdrplay_api_TunerSelectT tuner,
                                 sdrplay_api_EventParamsT *params)
{
    if (eventId == sdrplay_api_RspDuoModeChange) {
        // save last RSPduo mode change
        rspduo_mode_change_type = params->rspDuoModeParams.modeChangeType;
        // not sure why fmt{} now wants me to cast this enum to int - fv
        d_logger->info("RSPduo mode change - modeChangeType={}", static_cast<int>(rspduo_mode_change_type));
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
        d_logger->error("invalid RSPduo mode selection: {}", rspduo_mode);
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
        d_logger->error("invalid RSPduo antenna selection: {}", antenna);
        return valid_tuner;
    }

    return true;
}

sdrplay_api_RxChannelParamsT *rspduo_impl::get_independent_rx_channel_params(int tuner) const
{
    if (!(device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner &&
          dual_mode_independent_rx)) {
        d_logger->warn("invalid call to get_independent_rx_channel_params() - device is not in independent RX mode");
        return nullptr;
    }
    return tuner != 1 ?  device_params->rxChannelA : device_params->rxChannelB;
}

sdrplay_api_TunerSelectT rspduo_impl::get_independent_rx_tuner(const int tuner) const
{
    if (!(device.rspDuoMode == sdrplay_api_RspDuoMode_Dual_Tuner &&
          dual_mode_independent_rx)) {
        d_logger->warn("invalid call to get_independent_rx_tuner() - device is not in independent RX mode");
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
        d_logger->error("sdrplay_api_GetDeviceParams() Error: {}", sdrplay_api_GetErrorString(err));
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
