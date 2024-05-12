/* -*- c++ -*- */
/*
 * Copyright 2024 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSPDXR2_H
#define INCLUDED_SDRPLAY3_RSPDXR2_H

#include <gnuradio/sdrplay3/rspdx.h>

namespace gr {
namespace sdrplay3 {

/*! SDRplay RSPdx-R2 -- SDR Receiver
 * \ingroup sdrplay3
 *
 * The RSPdx-R2 source block receives samples and writes to a stream.
 * The source block also provides API calls for receiver settings.
 * See also gr::sdrplay3::rsp for more public API calls.
 *
 */

class SDRPLAY3_API rspdxr2 : virtual public rspdx
{
public:
    // gr::sdrplay3::rspdxr2::sptr
    typedef std::shared_ptr<rspdxr2> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of sdrplay3::rspdxr2.
     *
     * \param selector the selector (serial number or GetDevices() index) to identify the hardware
     * \param stream_args stream args (output type, number of channels, etc)
     */
    static sptr make(const std::string& selector,
                     const struct stream_args_t& stream_args);
};

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSPDXR2_H */
