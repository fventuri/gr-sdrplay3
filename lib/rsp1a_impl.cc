/* -*- c++ -*- */
/*
 * Copyright 2020 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rsp1a_impl.h"

namespace gr {
namespace sdrplay3 {

rsp1a::sptr rsp1a::make(const std::string& selector,
                        const struct stream_args_t& stream_args)
{
    //return gnuradio::get_initial_sptr(new rsp1a_impl(selector));
    return rsp1a::sptr(new rsp1a_impl(selector, stream_args));
}

rsp1a_impl::rsp1a_impl(const std::string& selector,
                       const struct stream_args_t& stream_args)
      : rsp("rsp1a", args_to_io_sig(stream_args)),
        rsp_impl(SDRPLAY_RSP1A_ID, selector, stream_args)
{
    nchannels = 1;
}

rsp1a_impl::~rsp1a_impl() {}


// Gain methods
const std::vector<int> rsp1a_impl::rf_gr_values(const double freq)
{
    if (freq <= 60e6) {
        static const std::vector<int> rf_gr = { 0, 6, 12, 18, 37, 42, 61 };
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

const std::vector<int> rsp1a_impl::rf_gr_values() const
{
    return rf_gr_values(rx_channel_params->tunerParams.rfFreq.rfHz);
}


// Miscellaneous stuff
void rsp1a_impl::set_rf_notch_filter(bool enable)
{
    unsigned char rf_notch_enable = enable ? 1 : 0;
    if (rf_notch_enable == device_params->devParams->rsp1aParams.rfNotchEnable)
        return;
    device_params->devParams->rsp1aParams.rfNotchEnable = rf_notch_enable;
    update_if_streaming(sdrplay_api_Update_Rsp1a_RfNotchControl);
}

void rsp1a_impl::set_dab_notch_filter(bool enable)
{
    unsigned char dab_notch_enable = enable ? 1 : 0;
    if (dab_notch_enable == device_params->devParams->rsp1aParams.rfDabNotchEnable)
        return;
    device_params->devParams->rsp1aParams.rfDabNotchEnable = dab_notch_enable;
    update_if_streaming(sdrplay_api_Update_Rsp1a_RfDabNotchControl);
}

void rsp1a_impl::set_biasT(bool enable)
{
    unsigned char biasT_enable = enable ? 1 : 0;
    if (biasT_enable == rx_channel_params->rsp1aTunerParams.biasTEnable)
        return;
    rx_channel_params->rsp1aTunerParams.biasTEnable = biasT_enable;
    update_if_streaming(sdrplay_api_Update_Rsp1a_BiasTControl);
}

} /* namespace sdrplay3 */
} /* namespace gr */
