/* -*- c++ -*- */
/*
 * Copyright 2020-2024 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSPDUO_IMPL_H
#define INCLUDED_SDRPLAY3_RSPDUO_IMPL_H

#include "rsp_impl.h"
#include <gnuradio/sdrplay3/rspduo.h>

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
class rspduo_impl : virtual public rspduo, public rsp_impl
{
public:
    rspduo_impl(const std::string& selector,
                const std::string& rspduo_mode,
                const std::string& antenna,
                const struct stream_args_t& stream_args,
                const std::string& name = "rspduo",
                const unsigned char hwVer = SDRPLAY_RSPduo_ID);
    ~rspduo_impl();

    // Sample rate methods
    double set_sample_rate(const double rate) override;
    const pair_of_doubles &get_sample_rate_range() const override;
    const std::vector<double> get_valid_sample_rates() const override;

    // Center frequency methods
    // we need to redefine the overloaded methods because of C++ name hiding
    double set_center_freq(const double freq) override { return rsp_impl::set_center_freq(freq); }
    double set_center_freq(const double freq, const int tuner) override;
    void set_center_freq(const double freq_A, const double freq_B) override;
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
    void set_gain(const double gain_A, const double gain_B, const std::string& name) override;
    double get_gain(const std::string& name) const override {
        return rsp_impl::get_gain(name);
    }
    double get_gain(const std::string& name, const int tuner) const override;
    const pair_of_doubles &get_gain_range(const std::string& name) const override {
        return rsp_impl::get_gain_range(name);
    }
    const pair_of_doubles &get_gain_range(const std::string& name, const int tuner) const override;
    bool set_gain_mode(bool automatic) override { return rsp_impl::set_gain_mode(automatic); }
    bool set_gain_mode(bool automatic, const int tuner) override;
    void set_gain_mode(bool automatic_A, bool automatic_B) override;
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
    const std::vector<int> rf_gr_values() const override;
    const std::vector<int> rf_gr_values(const int tuner) const;

    // we need to redefine the overloaded methods because of C++ name hiding
    double set_if_gain(const double gain) {
        return rsp_impl::set_if_gain(gain);
    }
    double set_if_gain(const double gain, const int tuner);
    void set_if_gain(const double gain_A, const double gain_B);
    double set_rf_gain(const double gain, const std::vector<int> rf_gRs) {
        return rsp_impl::set_rf_gain(gain, rf_gRs);
    }
    double set_rf_gain(const double gain, const std::vector<int> rf_gRs,
                       const int tuner);
    void set_rf_gain(const double gain_A, const double gain_B,
                     const std::vector<int> rf_gRs);
    int set_lna_state(const int LNAstate, const std::vector<int> rf_gRs) {
        return rsp_impl::set_lna_state(LNAstate, rf_gRs);
    }
    int set_lna_state(const int LNAstate, const std::vector<int> rf_gRs,
                      const int tuner);
    void set_lna_state(const int LNAstate_A, const int LNAstate_B,
                       const std::vector<int> rf_gRs);
    double get_if_gain() const { return rsp_impl::get_if_gain(); }
    double get_if_gain(const int tuner) const;
    double get_rf_gain(const std::vector<int> rf_gRs) const {
        return rsp_impl::get_rf_gain(rf_gRs);
    }
    double get_rf_gain(const std::vector<int> rf_gRs, const int tuner) const;
    int get_lna_state() const {
        return rsp_impl::get_lna_state();
    }
    int get_lna_state(const int tuner) const;

    void print_device_config() const override;


    bool dual_mode_independent_rx;
    sdrplay_api_RspDuoModeCbEventIdT rspduo_mode_change_type;
};
#pragma warning( pop )

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSPDUO_IMPL_H */
