/* -*- c++ -*- */
/*
 * Copyright 2020 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSP_H
#define INCLUDED_SDRPLAY3_RSP_H

#include <sdrplay3/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace sdrplay3 {

/*! Base class for SDRplay RSP devices
 * \ingroup sdrplay3
 *
 */
class SDRPLAY3_API rsp : public gr::sync_block
{
protected:
    rsp(){}; // For virtual sub-classing
    rsp(const std::string& name, gr::io_signature::sptr output_signature);
public:
    /*!
     * Set the sample rate for this RSP.
     *
     * \param rate a new rate in Sps
     */
    virtual double set_sample_rate(const double rate) = 0;

    /*!
     * Get the sample rate for this RSP.
     * This is the actual sample rate and may differ from the rate set.
     *
     * \return the actual rate in Sps
     */
    virtual double get_sample_rate() const = 0;

    /*!
     * Get the possible sample rates for this RSP.
     *
     * \return a vector of rates in Sps
     */
    virtual const std::vector<double> get_sample_rates() const = 0;

    /*!
     * Tune to the desired center frequency.
     *
     * \param freq the requested center frequency
     */
    virtual double set_center_freq(const double freq) = 0;

    /*!
     * Get the center frequency.
     *
     * \return the frequency in Hz
     */
    virtual double get_center_freq() const = 0;

    /*!
     * Get the tunable frequency range.
     *
     * \return the frequency range in Hz
     */
    virtual const double (&get_freq_range() const)[2] = 0;

    /*!
     * Set the bandwidth for this RSP.
     *
     * \param bandwidth a new bandwidth in Hz
     */
    virtual double set_bandwidth(const double bandwidth) = 0;

    /*!
     * Get the bandwidth for this RSP.
     * This is the actual bandwidth and may differ from the bandwidth set.
     *
     * \return the actual bandwidth in Hz
     */
    virtual double get_bandwidth() const = 0;

    /*!
     * Get the possible bandwidths for this RSP.
     *
     * \return a vector of bandwidths in Hz
     */
    virtual const std::vector<double> get_bandwidths() const = 0;

    /*!
     * Get gain names for this RSP.
     *
     * \return a vector of gain names
     */
    virtual const std::vector<std::string> get_gain_names() const = 0;

    /*!
     * Set the gain for this RSP.
     *
     * \param gain gain value in dB
     * \param name gain name
     */
    virtual double set_gain(const double gain, const std::string& name) = 0;

    /*!
     * Get the gain for this RSP.
     *
     * \param name gain name
     * \return the gain value in dB
     */
    virtual double get_gain(const std::string& name) const = 0;

    /*!
     * Get the gain range for this RSP.
     *
     * \param name gain name
     * \return the gain range in dB
     */
    virtual const double (&get_gain_range(const std::string& name) const)[2] = 0;

    /*!
     * Set the IF gain mode (automatic=true/false) for this RSP.
     *
     * \param automatic enable or disable AGC
     */
    virtual bool set_gain_mode(bool automatic) = 0;

    /*!
     * Get the IF gain mode (AGC=true/false) for this RSP.
     *
     * \return the gain mode (automatic=true/false)
     */
    virtual bool get_gain_mode() const = 0;

    /*!
     * Set the frequency correction (ppm).
     *
     * \param freq the requested frequency correction (ppm)
     */
    virtual double set_freq_corr(const double freq) = 0;

    /*!
     * Get the frequency correction (ppm).
     *
     * \return the frequency correction in ppm
     */
    virtual double get_freq_corr() const = 0;

    /*!
     * Enable/disable DC offset correction
     *
     * \param enable enable (or disable) DC offset correction
     */
    virtual void set_dc_offset_mode(bool enable) = 0;

    /*!
     * Enable/disable I/Q imbalance correction
     *
     * \param enable enable (or disable) I/Q imbalance correction
     */
    virtual void set_iq_balance_mode(bool enable) = 0;

    /*!
     * AGC set point (dBfs)
     *
     * \param set_point AGC set point in dBfs
     */
    virtual void set_agc_setpoint(double set_point) = 0;

    /*!
     * Set debug mode for SDRplay API
     *
     * \param enable enable (or disable) debug mode for SDRplay API
     */
    virtual void set_debug_mode(bool enable) = 0;

    /*!
     * Check sample sequence numbers for gaps
     *
     * \param enable enable (or disable) check for gaps in sample sequence numbers
     */
    virtual void set_sample_sequence_gaps_check(bool enable) = 0;

    /*!
     * Show gain changes (can be very noisy when AGC is enabled)
     *
     * \param enable if enabled logs a message every time the IF gain changes (can be very noisy when AGC is enabled)
     */
    virtual void set_show_gain_changes(bool enable) = 0;
};

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSP_H */
