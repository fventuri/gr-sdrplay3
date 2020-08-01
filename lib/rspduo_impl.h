/* -*- c++ -*- */
/*
 * Copyright 2020 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSPDUO_IMPL_H
#define INCLUDED_SDRPLAY3_RSPDUO_IMPL_H

#include "rsp_impl.h"
#include <sdrplay3/rspduo.h>

namespace gr {
namespace sdrplay3 {

class rspduo_impl : public rspduo, public rsp_impl
{
public:
    rspduo_impl(const std::string& selector,
                const std::string& rspduo_mode,
                const std::string& antenna,
                const struct stream_args_t& stream_args);
    ~rspduo_impl();

    // Sample rate methods
    double set_sample_rate(const double rate) override;
    const std::vector<double> get_sample_rates() const override;

    // Center frequency methods
    // we need to redefine the overloaded methods because of C++ name hiding
    double set_center_freq(const double freq) override { return rsp_impl::set_center_freq(freq); }
    double set_center_freq(const double freq, const int tuner) override;
    double get_center_freq() const override { return rsp_impl::get_center_freq(); }
    double get_center_freq(const int tuner) const override;

    // Antenna methods
    const std::string set_antenna(const std::string& antenna) override;
    const std::string get_antenna() const override;
    const std::vector<std::string> get_antennas() const override;

    // Gain methods
    double set_gain(const double gain, const std::string& name) override {
        return rsp_impl::set_gain(gain, name);
    }
    double set_gain(const double gain, const std::string& name, const int tuner) override;
    double get_gain(const std::string& name) const override {
        return rsp_impl::get_gain(name);
    }
    double get_gain(const std::string& name, const int tuner) const override;
    const double (&get_gain_range(const std::string& name) const override)[2] {
        return rsp_impl::get_gain_range(name);
    }
    const double (&get_gain_range(const std::string& name, const int tuner) const override)[2];
    bool set_gain_mode(bool automatic) override { return rsp_impl::set_gain_mode(automatic); }
    bool set_gain_mode(bool automatic, const int tuner) override;
    bool get_gain_mode() const override { return rsp_impl::get_gain_mode(); };
    bool get_gain_mode(const int tuner) const override;

    // Miscellaneous stuff
    void set_rf_notch_filter(bool enable) override;
    void set_dab_notch_filter(bool enable) override;
    void set_am_notch_filter(bool enable) override;
    void set_biasT(bool enable) override;

    // callback functions
    void event_callback(sdrplay_api_EventT eventId,
                        sdrplay_api_TunerSelectT tuner,
                        sdrplay_api_EventParamsT *params) override;

private:

    bool rspduo_select(const std::string& rspduo_mode,
                       const std::string& antenna);
    sdrplay_api_RxChannelParamsT *get_independent_rx_channel_params(int tuner) const;
    sdrplay_api_TunerSelectT get_independent_rx_tuner(int tuner) const;

    static const std::vector<int> rf_gr_values(const double freq, const bool highz);
    const std::vector<int> rf_gr_values() const;
    const std::vector<int> rf_gr_values(const int tuner) const;

    // we need to redefine the overloaded methods because of C++ name hiding
    double set_if_gain(const double gain) {
        return rsp_impl::set_if_gain(gain);
    }
    double set_if_gain(const double gain, const int tuner);
    double set_rf_gain(const double gain, const std::vector<int> rf_gRs) {
        return rsp_impl::set_rf_gain(gain, rf_gRs);
    }
    double set_rf_gain(const double gain, const std::vector<int> rf_gRs,
                       const int tuner);
    double get_if_gain() const { return rsp_impl::get_if_gain(); }
    double get_if_gain(const int tuner) const;
    double get_rf_gain(const std::vector<int> rf_gRs) const {
        return rsp_impl::get_rf_gain(rf_gRs);
    }
    double get_rf_gain(const std::vector<int> rf_gRs, const int tuner) const;

    void print_device_config() const override;


    bool dual_mode_independent_rx;
    sdrplay_api_RspDuoModeCbEventIdT rspduo_mode_change_type;
};

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSPDUO_IMPL_H */
