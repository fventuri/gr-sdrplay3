/* -*- c++ -*- */
/*
 * Copyright 2020 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSP1_IMPL_H
#define INCLUDED_SDRPLAY3_RSP1_IMPL_H

#include "rsp_impl.h"
#include <sdrplay3/rsp1.h>

namespace gr {
namespace sdrplay3 {

class rsp1_impl : public rsp1, public rsp_impl
{
public:
    rsp1_impl(const std::string& selector,
              const struct stream_args_t& stream_args);
    ~rsp1_impl();

private:

    static const std::vector<int> rf_gr_values(const double freq);
    const std::vector<int> rf_gr_values() const;
};

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSP1_IMPL_H */
