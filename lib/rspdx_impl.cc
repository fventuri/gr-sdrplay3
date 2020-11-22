/* -*- c++ -*- */
/*
 * Copyright 2020 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rspdx_impl.h"

namespace gr {
namespace sdrplay3 {

rspdx::sptr rspdx::make(const std::string& selector,
                        const struct stream_args_t& stream_args)
{
    //return gnuradio::get_initial_sptr(new rspdx_impl(selector));
    return rspdx::sptr(new rspdx_impl(selector, stream_args));
}

rspdx_impl::rspdx_impl(const std::string& selector,
                       const struct stream_args_t& stream_args)
      : rsp("rspdx", args_to_io_sig(stream_args)),
        rsp_impl(SDRPLAY_RSPdx_ID, selector, stream_args)
{
    nchannels = 1;
}

rspdx_impl::~rspdx_impl() {}


// Antenna methods
struct _antenna {
    sdrplay_api_RspDx_AntennaSelectT select;
};

static const std::map<std::string, struct _antenna> antennas = {
    { "Antenna A", { sdrplay_api_RspDx_ANTENNA_A } },
    { "Antenna B", { sdrplay_api_RspDx_ANTENNA_B } },
    { "Antenna C", { sdrplay_api_RspDx_ANTENNA_C } }
};

const std::string rspdx_impl::set_antenna(const std::string& antenna)
{
    if (antennas.count(antenna) == 0) {
        GR_LOG_WARN(d_logger, boost::format("invalid antenna: %s") % antenna);
        return get_antenna();
    }

    sdrplay_api_RspDx_AntennaSelectT antennaSel = antennas.at(antenna).select;
    sdrplay_api_RspDx_AntennaSelectT& device_antennaSel = device_params->devParams->rspDxParams.antennaSel;
    if (antennaSel != device_antennaSel) {
        device_antennaSel = antennaSel;
        update_ext1_if_streaming(sdrplay_api_Update_RspDx_AntennaControl);
    }

    return get_antenna();
}

const std::string rspdx_impl::get_antenna() const
{
    sdrplay_api_RspDx_AntennaSelectT antennaSel = device_params->devParams->rspDxParams.antennaSel;
    for (const auto& antenna : antennas) {
        if (antenna.second.select == antennaSel) {
            return antenna.first;
        }
    }
    return "Unknown";
}

const std::vector<std::string> rspdx_impl::get_antennas() const
{
    std::vector<std::string> antenna_names = {};
    for (const auto& antenna : antennas) {
        antenna_names.push_back(antenna.first);
    }
    return antenna_names;
}


// Gain methods
const std::vector<int> rspdx_impl::rf_gr_values(const double freq, const bool hdr_mode)
{
    if (freq <= 2e6 && hdr_mode) {
        static const std::vector<int> rf_gr = { 0, 3, 6, 9, 12, 15, 18, 21, 24, 25, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 60 };
        return rf_gr;
    } else if (freq <= 12e6) {
        static const std::vector<int> rf_gr = { 0, 3, 6, 9, 12, 15, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 60 };
        return rf_gr;
    } else if (freq <= 60e6) {
        static const std::vector<int> rf_gr = { 0, 3, 6, 9, 12, 15, 18, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 60 };
        return rf_gr;
    } else if (freq <= 250e6) {
        static const std::vector<int> rf_gr = { 0, 3, 6, 9, 12, 15, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 72, 75, 78, 81, 84 };
        return rf_gr;
    } else if (freq <= 420e6) {
        static const std::vector<int> rf_gr = { 0, 3, 6, 9, 12, 15, 18, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 72, 75, 78, 81, 84 };
        return rf_gr;
    } else if (freq <= 1000e6) {
        static const std::vector<int> rf_gr = { 0, 7, 10, 13, 16, 19, 22, 25, 31, 34, 37, 40, 43, 46, 49, 52, 55, 58, 61, 64, 67 };
        return rf_gr;
    } else if (freq <= 2000e6) {
        static const std::vector<int> rf_gr = { 0, 5, 8, 11, 14, 17, 20, 32, 35, 38, 41, 44, 47, 50, 53, 56, 59, 62, 65 };
        return rf_gr;
    } else {
        return std::vector<int>();
    }
}

const std::vector<int> rspdx_impl::rf_gr_values() const
{
    bool hdr_mode = device_params->devParams->rspDxParams.hdrEnable;
    return rf_gr_values(rx_channel_params->tunerParams.rfFreq.rfHz, hdr_mode);
}


// Miscellaneous stuff
bool rspdx_impl::set_hdr_mode(bool enable)
{
    unsigned char hdr_mode_enable = enable ? 1 : 0;
    if (hdr_mode_enable == device_params->devParams->rspDxParams.hdrEnable)
        return get_hdr_mode();
    device_params->devParams->rspDxParams.hdrEnable = hdr_mode_enable;
    update_ext1_if_streaming(sdrplay_api_Update_RspDx_HdrEnable);
    return get_hdr_mode();
}

bool rspdx_impl::get_hdr_mode() const
{
    return rx_channel_params->ctrlParams.agc.enable != sdrplay_api_AGC_DISABLE;
}

void rspdx_impl::set_rf_notch_filter(bool enable)
{
    unsigned char rf_notch_enable = enable ? 1 : 0;
    if (rf_notch_enable == device_params->devParams->rspDxParams.rfNotchEnable)
        return;
    device_params->devParams->rspDxParams.rfNotchEnable = rf_notch_enable;
    update_ext1_if_streaming(sdrplay_api_Update_RspDx_RfNotchControl);
}

void rspdx_impl::set_dab_notch_filter(bool enable)
{
    unsigned char dab_notch_enable = enable ? 1 : 0;
    if (dab_notch_enable == device_params->devParams->rspDxParams.rfDabNotchEnable)
        return;
    device_params->devParams->rspDxParams.rfDabNotchEnable = dab_notch_enable;
    update_ext1_if_streaming(sdrplay_api_Update_RspDx_RfDabNotchControl);
}

void rspdx_impl::set_biasT(bool enable)
{
    unsigned char biasT_enable = enable ? 1 : 0;
    if (biasT_enable == device_params->devParams->rspDxParams.biasTEnable)
        return;
    device_params->devParams->rspDxParams.biasTEnable = biasT_enable;
    update_ext1_if_streaming(sdrplay_api_Update_RspDx_BiasTControl);
}


// internal methods
static const std::string reason_ext1_as_text(sdrplay_api_ReasonForUpdateExtension1T reason_for_update);
void rspdx_impl::update_ext1_if_streaming(sdrplay_api_ReasonForUpdateExtension1T reason_for_update)
{
    if (run_status == RunStatus::idle || reason_for_update == sdrplay_api_Update_Ext1_None)
        return;
    sdrplay_api_ErrT err;
    err = sdrplay_api_Update(device.dev, device.tuner, sdrplay_api_Update_None,
                             reason_for_update);
    if (err != sdrplay_api_Success) {
        GR_LOG_ERROR(d_logger, boost::format("sdrplay_api_Update(%s) Error: %s") % reason_ext1_as_text(reason_for_update) % sdrplay_api_GetErrorString(err));
    }
}

static const std::string reason_ext1_as_text(sdrplay_api_ReasonForUpdateExtension1T reason_for_update)
{
    static const std::unordered_map<sdrplay_api_ReasonForUpdateExtension1T, const std::string, std::hash<int>> reasons = {
        { sdrplay_api_Update_RspDx_HdrEnable, "RspDx_HdrEnable" },
        { sdrplay_api_Update_RspDx_BiasTControl, "RspDx_BiasTControl" },
        { sdrplay_api_Update_RspDx_AntennaControl, "RspDx_AntennaControl" },
        { sdrplay_api_Update_RspDx_RfNotchControl, "RspDx_RfNotchControl" },
        { sdrplay_api_Update_RspDx_RfDabNotchControl, "RspDx_RfDabNotchControl" },
        { sdrplay_api_Update_RspDx_HdrBw, "RspDx_HdrBw" }
    };
    try {
        return reasons.at(reason_for_update);
    } catch (std::out_of_range& e) {
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

void rspdx_impl::print_device_config() const
{
    rsp_impl::print_device_config();

    sdrplay_api_DeviceParamsT *params;
    sdrplay_api_ErrT err = sdrplay_api_GetDeviceParams(device.dev, &params);
    if (err != sdrplay_api_Success) {
        GR_LOG_ERROR(d_logger, boost::format("sdrplay_api_GetDeviceParams() Error: %s") % sdrplay_api_GetErrorString(err));
        return;
    }
    std::cerr << "# RSPdx specific config:" << std::endl;
    sdrplay_api_RspDxParamsT *rspDxParams = &device_params->devParams->rspDxParams;
    std::cerr << "    rspDxParams.antennaSel=" << rspDxParams->antennaSel << std::endl;
    std::cerr << "    rspDxParams.hdrEnable=" << rspDxParams->hdrEnable << std::endl;
    std::cerr << std::endl;
    return;
}

} /* namespace sdrplay3 */
} /* namespace gr */
