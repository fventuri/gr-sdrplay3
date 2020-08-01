/* -*- c++ -*- */
/*
 * Copyright 2020 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSP1A_H
#define INCLUDED_SDRPLAY3_RSP1A_H

#include <sdrplay3/rsp.h>

namespace gr {
namespace sdrplay3 {

/*! SDRplay RSP1A -- SDR Receiver
 * \ingroup sdrplay3
 *
 * The RSP1A source block receives samples and writes to a stream.
 * The source block also provides API calls for receiver settings.
 * See also gr::sdrplay3::rsp for more public API calls.
 *
 */

class SDRPLAY3_API rsp1a : virtual public rsp
{
public:
    // gr::sdrplay3::rsp1a::sptr
    typedef std::shared_ptr<rsp1a> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of sdrplay3::rsp1a.
     *
     * \param selector the selector (serial number or GetDevices() index) to identify the hardware
     * \param stream_args stream args (output type, number of channels, etc)
     */
    static sptr make(const std::string& selector,
                     const struct stream_args_t& stream_args);

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
     * Enable Bias-T
     *
     * \param enable enable (or disable) Bias-T
     */
    virtual void set_biasT(bool enable) = 0;
};

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSP1A_H */
