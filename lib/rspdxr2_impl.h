/* -*- c++ -*- */
/*
 * Copyright 2024 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSPDXR2_IMPL_H
#define INCLUDED_SDRPLAY3_RSPDXR2_IMPL_H

#include "rspdx_impl.h"
#include <gnuradio/sdrplay3/rspdxr2.h>

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
class rspdxr2_impl : virtual public rspdxr2, public rspdx_impl
{
public:
    rspdxr2_impl(const std::string& selector,
                 const struct stream_args_t& stream_args,
                 const std::string& name = "rspdxr2",
                 const unsigned char hwVer = SDRPLAY_RSPdxR2_ID);
    ~rspdxr2_impl();
};
#pragma warning( pop )

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSPDXR2_IMPL_H */
