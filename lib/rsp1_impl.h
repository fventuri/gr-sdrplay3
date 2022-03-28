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

// The class structure here follows the dual hierarchy approach shown here:
// https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rh-kind
// However the lattice DAG of this class structure triggers Visual C++ warning
// C4250 (https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4250?view=msvc-170)
// Because of that we disable Visual C++ warning C4250 just for the
// derived class declarations
#pragma warning( push )
#pragma warning( disable : 4250 )
class rsp1_impl : public virtual rsp1, public rsp_impl
{
public:
    rsp1_impl(const std::string& selector,
              const struct stream_args_t& stream_args);
    ~rsp1_impl();

private:

    static const std::vector<int> rf_gr_values(const double freq);
    const std::vector<int> rf_gr_values() const;
};
#pragma warning( pop )

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSP1_IMPL_H */
