/* -*- c++ -*- */
/*
 * Copyright 2020 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSP1A_IMPL_H
#define INCLUDED_SDRPLAY3_RSP1A_IMPL_H

#include "rsp_impl.h"
#include <sdrplay3/rsp1a.h>

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
class rsp1a_impl : public virtual rsp1a, public rsp_impl
{
public:
    rsp1a_impl(const std::string& selector,
               const struct stream_args_t& stream_args);
    ~rsp1a_impl();

    // Miscellaneous stuff
    void set_rf_notch_filter(bool enable) override;
    void set_dab_notch_filter(bool enable) override;
    void set_biasT(bool enable) override;

private:

    static const std::vector<int> rf_gr_values(const double freq);
    const std::vector<int> rf_gr_values() const override;
};
#pragma warning( pop )

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSP1A_IMPL_H */
