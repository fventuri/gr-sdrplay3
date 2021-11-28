/* -*- c++ -*- */
/*
 * Copyright 2020 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSPDX_IMPL_H
#define INCLUDED_SDRPLAY3_RSPDX_IMPL_H

#include "rsp_impl.h"
#include <gnuradio/sdrplay3/rspdx.h>

namespace gr {
namespace sdrplay3 {

class rspdx_impl : public rspdx, public rsp_impl
{
public:
    rspdx_impl(const std::string& selector,
               const struct stream_args_t& stream_args);
    ~rspdx_impl();

    // Antenna methods
    const std::string set_antenna(const std::string& antenna) override;
    const std::string get_antenna() const override;
    const std::vector<std::string> get_antennas() const override;

    // Miscellaneous stuff
    bool set_hdr_mode(bool enable) override;
    bool get_hdr_mode() const override;
    void set_rf_notch_filter(bool enable) override;
    void set_dab_notch_filter(bool enable) override;
    void set_biasT(bool enable) override;

private:

    void update_ext1_if_streaming(sdrplay_api_ReasonForUpdateExtension1T reason_for_update);
    static const std::vector<int> rf_gr_values(const double freq, const bool hdr_mode);
    const std::vector<int> rf_gr_values() const override;

    void print_device_config() const override;
};

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSPDX_IMPL_H */
