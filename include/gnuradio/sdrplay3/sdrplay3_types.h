/* -*- c++ -*- */
/*
 * Copyright 2020,2021 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_GR_SDRPLAY3_TYPES_H
#define INCLUDED_GR_SDRPLAY3_TYPES_H

#include <string>
#include <unordered_map>
#include <gnuradio/gr_complex.h>

namespace gr {
namespace sdrplay3 {

struct stream_args_t
{
    stream_args_t(const std::string& output_type = "fc32",
                  const size_t channels_size = 1) :
        output_type(output_type),
        channels_size(channels_size) {
    }
    std::string output_type;
    size_t channels_size;
};

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_GR_SDRPLAY3_TYPES_H */
