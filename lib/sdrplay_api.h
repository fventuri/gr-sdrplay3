/* -*- c++ -*- */
/*
 * Copyright 2020 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_SDRPLAY_API_H
#define INCLUDED_SDRPLAY3_SDRPLAY_API_H

#include <sdrplay_api.h>

namespace gr {
namespace sdrplay3 {

class sdrplay_api
{
public:
    static sdrplay_api& get_instance()
    {
        static sdrplay_api instance;
        return instance;
    }

private:
    sdrplay_api();

public:
    ~sdrplay_api();
    sdrplay_api(sdrplay_api const&)    = delete;
    void operator=(sdrplay_api const&) = delete;

private:
    static gr::logger_ptr d_logger;       //! Default logger
    static gr::logger_ptr d_debug_logger; //! Verbose logger
};

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_SDRPLAY_API_H */
