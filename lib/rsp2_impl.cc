/* -*- c++ -*- */
/*
 * Copyright 2020-2024 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rsp2_impl.h"

namespace gr {
namespace sdrplay3 {

rsp2::sptr rsp2::make(const std::string& selector,
                      const struct stream_args_t& stream_args)
{
    //return gnuradio::get_initial_sptr(new rsp2_impl(selector));
    return rsp2::sptr(new rsp2_impl(selector, stream_args));
}

rsp2_impl::rsp2_impl(const std::string& selector,
                     const struct stream_args_t& stream_args,
                     const std::string& name,
                     const unsigned char hwVer)
      : rsp(name, args_to_io_sig(stream_args)),
        rsp_impl(hwVer, selector, stream_args)
{
    nchannels = 1;
}

rsp2_impl::~rsp2_impl() {}


// Antenna methods
struct _antenna {
    sdrplay_api_Rsp2_AntennaSelectT select;
    bool high_z;
};

static const std::map<std::string, struct _antenna> antennas = {
    { "Antenna A", { sdrplay_api_Rsp2_ANTENNA_A, false } },
    { "Antenna B", { sdrplay_api_Rsp2_ANTENNA_B, false } },
    { "Hi-Z",      { sdrplay_api_Rsp2_ANTENNA_A, true } }
};

const std::string rsp2_impl::set_antenna(const std::string& antenna)
{
    if (antennas.count(antenna) == 0) {
        d_logger->warn("invalid antenna: {}", antenna);
        return get_antenna();
    }

    sdrplay_api_ReasonForUpdateT reason = sdrplay_api_Update_None;

    sdrplay_api_Rsp2_AntennaSelectT antennaSel = antennas.at(antenna).select;
    sdrplay_api_Rsp2_AntennaSelectT& device_antennaSel = rx_channel_params->rsp2TunerParams.antennaSel;
    if (antennaSel != device_antennaSel) {
        device_antennaSel = antennaSel;
        reason = (sdrplay_api_ReasonForUpdateT)(reason | sdrplay_api_Update_Rsp2_AntennaControl);
    }

    sdrplay_api_Rsp2_AmPortSelectT amPortSel = antennas.at(antenna).high_z ?
                        sdrplay_api_Rsp2_AMPORT_1 : sdrplay_api_Rsp2_AMPORT_2;
    sdrplay_api_Rsp2_AmPortSelectT& device_amPortSel = rx_channel_params->rsp2TunerParams.amPortSel;
    if (amPortSel != device_amPortSel) {
        device_amPortSel = amPortSel;
        reason = (sdrplay_api_ReasonForUpdateT)(reason | sdrplay_api_Update_Rsp2_AmPortSelect);
    }

    update_if_streaming(reason);

    return get_antenna();
}

const std::string rsp2_impl::get_antenna() const
{
    sdrplay_api_Rsp2_AntennaSelectT antennaSel = rx_channel_params->rsp2TunerParams.antennaSel;
    bool high_z = rx_channel_params->rsp2TunerParams.amPortSel == sdrplay_api_Rsp2_AMPORT_1;
    for (const auto& antenna : antennas) {
        if (antenna.second.select == antennaSel &&
            antenna.second.high_z == high_z) {
            return antenna.first;
        }
    }
    return "Unknown";
}

const std::vector<std::string> rsp2_impl::get_antennas() const
{
    std::vector<std::string> antenna_names = {};
    for (const auto& antenna : antennas) {
        antenna_names.push_back(antenna.first);
    }
    return antenna_names;
}


// Gain methods
const std::vector<int> rsp2_impl::rf_gr_values(const double freq, const bool highz)
{
    if (freq <= 60e6 && highz) {
        static const std::vector<int> rf_gr = { 0, 6, 12, 18, 37 };
        return rf_gr;
    } else if (freq <= 420e6) {
        static const std::vector<int> rf_gr = { 0, 10, 15, 21, 24, 34, 39, 45, 64 };
        return rf_gr;
    } else if (freq <= 1000e6) {
        static const std::vector<int> rf_gr = { 0, 7, 10, 17, 22, 41 };
        return rf_gr;
    } else if (freq <= 2000e6) {
        static const std::vector<int> rf_gr = { 0, 5, 21, 15, 15, 34 };
        return rf_gr;
    } else {
        return std::vector<int>();
    }
}

const std::vector<int> rsp2_impl::rf_gr_values() const
{
    bool highz = rx_channel_params->rsp2TunerParams.antennaSel == sdrplay_api_Rsp2_ANTENNA_A &&
                 rx_channel_params->rsp2TunerParams.amPortSel == sdrplay_api_Rsp2_AMPORT_1;
    return rf_gr_values(rx_channel_params->tunerParams.rfFreq.rfHz, highz);
}


// Miscellaneous stuff
void rsp2_impl::set_rf_notch_filter(bool enable)
{
    unsigned char rf_notch_enable = enable ? 1 : 0;
    if (rf_notch_enable == rx_channel_params->rsp2TunerParams.rfNotchEnable)
        return;
    rx_channel_params->rsp2TunerParams.rfNotchEnable = rf_notch_enable;
    update_if_streaming(sdrplay_api_Update_Rsp2_RfNotchControl);
}

void rsp2_impl::set_biasT(bool enable)
{
    unsigned char biasT_enable = enable ? 1 : 0;
    if (biasT_enable == rx_channel_params->rsp2TunerParams.biasTEnable)
        return;
    rx_channel_params->rsp2TunerParams.biasTEnable = biasT_enable;
    update_if_streaming(sdrplay_api_Update_Rsp2_BiasTControl);
}


// internal methods
void rsp2_impl::print_device_config() const
{
    rsp_impl::print_device_config();

    sdrplay_api_DeviceParamsT *params;
    sdrplay_api_ErrT err = sdrplay_api_GetDeviceParams(device.dev, &params);
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_GetDeviceParams() Error: {}", sdrplay_api_GetErrorString(err));
        return;
    }
    std::cerr << "# RSP2 specific config:" << std::endl;
    sdrplay_api_Rsp2TunerParamsT *rsp2TunerParams = &rx_channel_params->rsp2TunerParams;
    std::cerr << "    rsp2TunerParams.antennaSel=" << rsp2TunerParams->antennaSel << std::endl;
    std::cerr << "    rsp2TunerParams.amPortSel=" << rsp2TunerParams->amPortSel << std::endl;
    std::cerr << std::endl;
    return;
}

} /* namespace sdrplay3 */
} /* namespace gr */
