/* -*- c++ -*- */
/*
 * Copyright 2020 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSP2_H
#define INCLUDED_SDRPLAY3_RSP2_H

#include <sdrplay3/rsp.h>

namespace gr {
namespace sdrplay3 {

/*! SDRplay RSP2 -- SDR Receiver
 * \ingroup sdrplay3
 *
 * The RSP2 source block receives samples and writes to a stream.
 * The source block also provides API calls for receiver settings.
 * See also gr::sdrplay3::rsp for more public API calls.
 *
 */

class SDRPLAY3_API rsp2 : virtual public rsp
{
public:
    // gr::sdrplay3::rsp2::sptr
    typedef std::shared_ptr<rsp2> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of sdrplay3::rsp2.
     *
     * \param selector the selector (serial number or GetDevices() index) to identify the hardware
     * \param stream_args stream args (output type, number of channels, etc)
     */
    static sptr make(const std::string& selector,
                     const struct stream_args_t& stream_args);

    /*!
     * Set the antenna for this RSP2.
     *
     * \param antenna a new antenna
     */
    virtual const std::string set_antenna(const std::string& antenna) = 0;

    /*!
     * Get the antenna for this RSP2.
     *
     * \return the current antenna type for this RSP2
     */
    virtual const std::string get_antenna() const = 0;

    /*!
     * Get a vector of antennas for this RSP2.
     *
     * \return a vector of antennas for this RSP2
     */
    virtual const std::vector<std::string> get_antennas() const = 0;

    /*!
     * Enable RF notch filter
     *
     * \param enable enable (or disable) the RF notch filter
     */
    virtual void set_rf_notch_filter(bool enable) = 0;

    /*!
     * Enable Bias-T
     *
     * \param enable enable (or disable) Bias-T
     */
    virtual void set_biasT(bool enable) = 0;
};

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSP2_H */
