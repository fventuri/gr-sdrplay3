/* -*- c++ -*- */
/*
 * Copyright 2020 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <csignal>
#include <stdexcept>
#include <gnuradio/logger.h>
#include "sdrplay_api.h"

namespace gr {
namespace sdrplay3 {

gr::logger_ptr sdrplay_api::d_logger = nullptr;
gr::logger_ptr sdrplay_api::d_debug_logger = nullptr;

// signal handler to make sure std::exit is called (instead of std::terminate)
void signal_handler(int signum)
{
    std::exit(signum);
}

// Singleton class for SDRplay API (only one per process)
sdrplay_api::sdrplay_api()
{
    gr::configure_default_loggers(d_logger, d_debug_logger, "sdrplay_api");

    sdrplay_api_ErrT err;
    // Open API
    err = sdrplay_api_Open();
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_Open() Error: {}", sdrplay_api_GetErrorString(err));
        d_logger->error("Please check the sdrplay_api service to make sure it is up. If it is up, please restart it.");
        throw std::runtime_error("sdrplay_api_Open() failed");
    }
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Check API versions match
    float ver;
    // --TIMEOUT--
    err = sdrplay_api_ApiVersion(&ver);
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_ApiVersion() Error: {}", sdrplay_api_GetErrorString(err));
        sdrplay_api_Close();
        throw std::runtime_error("ApiVersion() failed");
    }
    if (ver != SDRPLAY_API_VERSION) {
        d_logger->warn("sdrplay_api version: '{:.3f}' does not equal build version: '{:.3f}'", ver, SDRPLAY_API_VERSION);
    }
}

sdrplay_api::~sdrplay_api()
{
    sdrplay_api_ErrT err;
    // Close API
    err = sdrplay_api_Close();
    if (err != sdrplay_api_Success) {
        d_logger->error("sdrplay_api_Close() Error: {}", sdrplay_api_GetErrorString(err));
    }
}

} /* namespace sdrplay3 */
} /* namespace gr */
