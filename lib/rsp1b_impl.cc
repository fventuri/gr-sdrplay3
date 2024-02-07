/* -*- c++ -*- */
/*
 * Copyright 2024 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rsp1b_impl.h"

namespace gr {
namespace sdrplay3 {

rsp1b::sptr rsp1b::make(const std::string& selector,
                        const struct stream_args_t& stream_args)
{
    //return gnuradio::get_initial_sptr(new rsp1b_impl(selector));
    return rsp1b::sptr(new rsp1b_impl(selector, stream_args));
}

rsp1b_impl::rsp1b_impl(const std::string& selector,
                       const struct stream_args_t& stream_args,
                       const std::string& name,
                       const unsigned char hwVer)
      : rsp(name, args_to_io_sig(stream_args)),
        rsp1a_impl(selector, stream_args, name, hwVer)
{
    // NOP
}

rsp1b_impl::~rsp1b_impl() {}


// Gain methods
const std::vector<int> rsp1b_impl::rf_gr_values(const double freq)
{
    if (freq <= 50e6) {
        static const std::vector<int> rf_gr = { 0, 6, 12, 18, 37, 42, 61 };
        return rf_gr;
    } else if (freq <= 60e6) {
        static const std::vector<int> rf_gr = { 0, 6, 12, 18, 20, 26, 32, 38, 57, 62 };
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

const std::vector<int> rsp1b_impl::rf_gr_values() const
{
    return rf_gr_values(rx_channel_params->tunerParams.rfFreq.rfHz);
}

} /* namespace sdrplay3 */
} /* namespace gr */
