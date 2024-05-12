/* -*- c++ -*- */
/*
 * Copyright 2024 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rspdxr2_impl.h"

namespace gr {
namespace sdrplay3 {

rspdxr2::sptr rspdxr2::make(const std::string& selector,
                            const struct stream_args_t& stream_args)
{
    //return gnuradio::get_initial_sptr(new rspdxr2_impl(selector));
    return rspdxr2::sptr(new rspdxr2_impl(selector, stream_args));
}

rspdxr2_impl::rspdxr2_impl(const std::string& selector,
                           const struct stream_args_t& stream_args,
                           const std::string& name,
                           const unsigned char hwVer)
      : rsp(name, args_to_io_sig(stream_args)),
        rspdx_impl(selector, stream_args, name, hwVer)
{
    // NOP
}

rspdxr2_impl::~rspdxr2_impl() {}

} /* namespace sdrplay3 */
} /* namespace gr */
