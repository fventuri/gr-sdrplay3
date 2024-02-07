/* -*- c++ -*- */
/*
 * Copyright 2020-2024 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rsp1_impl.h"

namespace gr {
namespace sdrplay3 {

rsp1::sptr rsp1::make(const std::string& selector,
                      const struct stream_args_t& stream_args)
{
    //return gnuradio::get_initial_sptr(new rsp1_impl(selector));
    return rsp1::sptr(new rsp1_impl(selector, stream_args));
}

rsp1_impl::rsp1_impl(const std::string& selector,
                     const struct stream_args_t& stream_args,
                     const std::string& name,
                     const unsigned char hwVer)
      : rsp(name, args_to_io_sig(stream_args)),
        rsp_impl(hwVer, selector, stream_args)
{
    nchannels = 1;
}

rsp1_impl::~rsp1_impl() {}


// Gain methods
const std::vector<int> rsp1_impl::rf_gr_values(const double freq)
{
    if (freq <= 420e6) {
        static const std::vector<int> rf_gr = { 0, 24, 19, 43 };
        return rf_gr;
    } else if (freq <= 1000e6) {
        static const std::vector<int> rf_gr = { 0, 7, 19, 26 };
        return rf_gr;
    } else if (freq <= 2000e6) {
        static const std::vector<int> rf_gr = { 0, 5, 19, 24 };
        return rf_gr;
    } else {
        return std::vector<int>();
    }
}

const std::vector<int> rsp1_impl::rf_gr_values() const
{
    return rf_gr_values(rx_channel_params->tunerParams.rfFreq.rfHz);
}

} /* namespace sdrplay3 */
} /* namespace gr */
