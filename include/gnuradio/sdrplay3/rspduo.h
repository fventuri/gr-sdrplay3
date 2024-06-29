/* -*- c++ -*- */
/*
 * Copyright 2020-2024 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSPDUO_H
#define INCLUDED_SDRPLAY3_RSPDUO_H

#include <gnuradio/sdrplay3/rsp.h>

namespace gr {
namespace sdrplay3 {

/*! SDRplay RSPduo -- SDR Receiver
 * \ingroup sdrplay3
 *
 * The RSPduo source block receives samples and writes to a stream.
 * The source block also provides API calls for receiver settings.
 * See also gr::sdrplay3::rsp for more public API calls.
 *
 */

class SDRPLAY3_API rspduo : virtual public rsp
{
public:
    // gr::sdrplay3::rspduo::sptr
    typedef std::shared_ptr<rspduo> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of sdrplay3::rspduo.
     *
     * \param selector the selector (serial number or GetDevices() index) to identify the hardware
     * \param rspduo_mode RSPduo mode (single tuner, dual tuner, master, slave)
     * \param antenna antenna selector (Tuner 1 50ohm, High-Z, Tuner 2 50ohm)
     * \param stream_args stream args (output type, number of channels, etc)
     */
    static sptr make(const std::string& selector,
                     const std::string& rspduo_mode,
                     const std::string& antenna,
                     const struct stream_args_t& stream_args);

    /*!
     * Set the sample rate for this RSPduo.
     *
     * \param rate a new rate in Sps
     * \param synchronous return after the change has taken effect
     */
    virtual double set_sample_rate(const double rate,
                                   const bool synchronous = false) = 0;

    /*!
     * Get the sample rate range.
     * (only valid in Single Tuner mode)
     *
     * \return the sample rate range in Sps
     */
    virtual const double (&get_sample_rate_range() const)[2] = 0;

    /*!
     * Get the possible sample rates for this RSPduo.
     * (only valid in Dual Tuner, Master, or Slave mode)
     *
     * \return a vector of valid sample rates in Sps
     */
    virtual const std::vector<double> get_valid_sample_rates() const = 0;

    /*!
     * Tune to the desired center frequency.
     *
     * \param freq the requested center frequency
     * \param synchronous return after the change has taken effect
     */
    virtual double set_center_freq(const double freq,
                                   const bool synchronous = false) = 0;

    /*!
     * Tune to the desired center frequency (independent RX case).
     *
     * \param freq the requested center frequency
     * \param tuner tuner number (0 or 1)
     * \param synchronous return after the change has taken effect
     */
    virtual double set_center_freq(const double freq, const int tuner,
                                   const bool synchronous = false) = 0;

    /*!
     * Tune to the desired center frequencies (independent RX case).
     *
     * \param freq_A the requested center frequency for tuner A
     * \param freq_B the requested center frequency for tuner B
     * \param synchronous return after the change has taken effect
     */
    virtual void set_center_freq(const double freq_A, const double freq_B,
                                 const bool synchronous = false) = 0;

    /*!
     * Get the center frequency.
     *
     * \return the frequency in Hz
     */
    virtual double get_center_freq() const = 0;

    /*!
     * Get the center frequency (independent RX case).
     *
     * \param tuner tuner number (0 or 1)
     * \return the frequency in Hz
     */
    virtual double get_center_freq(int tuner) const = 0;

    /*!
     * Set the antenna for this RSPduo.
     *
     * \param antenna a new antenna
     */
    virtual const std::string set_antenna(const std::string& antenna) = 0;

    /*!
     * Get the antenna for this RSPduo.
     *
     * \return the current antenna type for this RSPduo
     */
    virtual const std::string get_antenna() const = 0;

    /*!
     * Get a vector of antennas for this RSPduo.
     *
     * \return a vector of antennas for this RSPduo
     */
    virtual const std::vector<std::string> get_antennas() const = 0;

    /*!
     * Set the gain for this RSPduo.
     *
     * \param gain gain value in dB
     * \param name gain name
     * \param synchronous return after the change has taken effect
     */
    virtual double set_gain(const double gain, const std::string& name,
                            const bool synchronous = false) = 0;

    /*!
     * Set the gain for this RSPduo (independent RX case).
     *
     * \param name gain name
     * \param gain gain value in dB
     * \param tuner tuner number (0 or 1)
     * \param synchronous return after the change has taken effect
     */
    virtual double set_gain(const double gain, const std::string& name,
                            const int tuner,
                            const bool synchronous = false) = 0;

    /*!
     * Set both gains for this RSPduo.
     *
     * \param name gain name
     * \param gain_A gain value in dB for tuner A
     * \param gain_B gain value in dB for tuner B
     * \param synchronous return after the change has taken effect
     */
    virtual void set_gain(const double gain_A, const double gain_B,
                          const std::string& name,
                          const bool synchronous = false) = 0;

    /*!
     * Get the gain for this RSPduo.
     *
     * \param name gain name
     * \return the gain value in dB
     */
    virtual double get_gain(const std::string& name) const = 0;

    /*!
     * Get the gain for this RSPduo (independent RX case).
     *
     * \param name gain name
     * \param tuner tuner number (0 or 1)
     * \return the gain value in dB
     */
    virtual double get_gain(const std::string& name, const int tuner) const = 0;

    /*!
     * Get the gain range for this RSPduo.
     *
     * \param name gain name
     * \return the gain range in dB
     */
    virtual const double (&get_gain_range(const std::string& name) const)[2] = 0;

    /*!
     * Get the gain range for this RSPduo (independent RX case).
     *
     * \param name gain name
     * \param tuner tuner number (0 or 1)
     * \return the gain range in dB
     */
    virtual const double (&get_gain_range(const std::string& name, const int tuner) const)[2] = 0;

    /*!
     * Set the IF gain mode (automatic=true/false) for this RSPduo.
     *
     * \param automatic enable or disable AGC
     */
    virtual bool set_gain_mode(bool automatic) = 0;

    /*!
     * Set the IF gain mode (automatic=true/false) for this RSPduo (independent RX case).
     *
     * \param automatic enable or disable AGC
     * \param tuner tuner number (0 or 1)
     */
    virtual bool set_gain_mode(bool automatic, const int tuner) = 0;

    /*!
     * Set both IF gain modes (automatic=true/false) for this RSPduo.
     *
     * \param automatic_A enable or disable AGC for tuner A
     * \param automatic_B enable or disable AGC for tuner B
     */
    virtual void set_gain_mode(bool automatic_A, bool automatic_B) = 0;

    /*!
     * Get the IF gain mode (AGC=true/false) for this RSPduo.
     *
     * \return the gain mode (automatic=true/false)
     */
    virtual bool get_gain_mode() const = 0;

    /*!
     * Get the IF gain mode (AGC=true/false) for this RSPduo (independent RX case).
     *
     * \param tuner tuner number (0 or 1)
     * \return the gain mode (automatic=true/false)
     */
    virtual bool get_gain_mode(const int tuner) const = 0;

    /*!
     * Enable RF notch filter
     *
     * \param enable enable (or disable) the RF notch filter
     */
    virtual void set_rf_notch_filter(bool enable) = 0;

    /*!
     * Enable DAB notch filter
     *
     * \param enable enable (or disable) the DAB notch filter
     */
    virtual void set_dab_notch_filter(bool enable) = 0;

    /*!
     * Enable AM RF notch filter
     *
     * \param enable enable (or disable) the AM RF notch filter
     */
    virtual void set_am_notch_filter(bool enable) = 0;

    /*!
     * Enable Bias-T
     *
     * \param enable enable (or disable) Bias-T
     */
    virtual void set_biasT(bool enable) = 0;
};

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSPDUO_H */
