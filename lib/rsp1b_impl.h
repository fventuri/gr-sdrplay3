/* -*- c++ -*- */
/*
 * Copyright 2024 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSP1B_IMPL_H
#define INCLUDED_SDRPLAY3_RSP1B_IMPL_H

#include "rsp1a_impl.h"
#include <gnuradio/sdrplay3/rsp1b.h>

namespace gr {
namespace sdrplay3 {

// The class structure here follows the dual hierarchy approach shown here:
// https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rh-kind
// However the lattice DAG of this class structure triggers Visual C++ warning
// C4250 (https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4250?view=msvc-170)
// Because of that we disable Visual C++ warning C4250 just for the
// derived class declarations
#pragma warning( push )
#pragma warning( disable : 4250 )
class rsp1b_impl : virtual public rsp1b, public rsp1a_impl
{
public:
    rsp1b_impl(const std::string& selector,
               const struct stream_args_t& stream_args,
               const std::string& name = "rsp1b",
               const unsigned char hwVer = SDRPLAY_RSP1B_ID);
    ~rsp1b_impl();

private:

    static const std::vector<int> rf_gr_values(const double freq);
    const std::vector<int> rf_gr_values() const override;
};
#pragma warning( pop )

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSP1B_IMPL_H */
