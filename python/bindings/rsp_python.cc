/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <sdrplay3/rsp.h>
// pydoc.h is automatically generated in the build directory
#include <rsp_pydoc.h>

void bind_rsp(py::module& m)
{
    using rsp = gr::sdrplay3::rsp;

    py::class_<rsp,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<rsp>>(m, "rsp", D(rsp))

        .def("set_sample_rate",
             &rsp::set_sample_rate,
             py::arg("rate"),
             D(rsp, set_sample_rate))

        .def("get_sample_rate",
             &rsp::get_sample_rate,
             D(rsp, get_sample_rate))

        .def("get_sample_rates",
             &rsp::get_sample_rates,
             D(rsp, get_sample_rates))

        .def("set_center_freq",
             &rsp::set_center_freq,
             py::arg("freq"),
             D(rsp, set_center_freq))

        .def("get_center_freq",
             &rsp::get_center_freq,
             D(rsp, get_center_freq))

        .def("get_freq_range",
             &rsp::get_freq_range,
             D(rsp, get_freq_range))

        .def("set_bandwidth",
             &rsp::set_bandwidth,
             py::arg("bandwidth"),
             D(rsp, set_bandwidth))

        .def("get_bandwidth",
             &rsp::get_bandwidth,
             D(rsp, get_bandwidth))

        .def("get_bandwidths",
             &rsp::get_bandwidths,
             D(rsp, get_bandwidths))

        .def("get_gain_names",
             &rsp::get_gain_names,
             D(rsp, get_gain_names))

        .def("set_gain",
             &rsp::set_gain,
             py::arg("gain"),
             py::arg("name"),
             D(rsp, set_gain))

        .def("get_gain",
             &rsp::get_gain,
             py::arg("name"),
             D(rsp, get_gain))

        .def("get_gain_range",
             &rsp::get_gain_range,
             py::arg("name"),
             D(rsp, get_gain_range))

        .def("set_gain_mode",
             &rsp::set_gain_mode,
             py::arg("automatic"),
             D(rsp, set_gain_mode))

        .def("get_gain_mode",
             &rsp::get_gain_mode,
             D(rsp, get_gain_mode))

        .def("set_freq_corr",
             &rsp::set_freq_corr,
             py::arg("freq"),
             D(rsp, set_freq_corr))

        .def("get_freq_corr",
             &rsp::get_freq_corr,
             D(rsp, get_freq_corr))

        .def("set_dc_offset_mode",
             &rsp::set_dc_offset_mode,
             py::arg("enable"),
             D(rsp, set_dc_offset_mode))

        .def("set_iq_balance_mode",
             &rsp::set_iq_balance_mode,
             py::arg("enable"),
             D(rsp, set_iq_balance_mode))

        .def("set_agc_setpoint",
             &rsp::set_agc_setpoint,
             py::arg("set_point"),
             D(rsp, set_agc_setpoint))

        .def("set_debug_mode",
             &rsp::set_debug_mode,
             py::arg("enable"),
             D(rsp, set_debug_mode))

        .def("set_sample_sequence_gaps_check",
             &rsp::set_sample_sequence_gaps_check,
             py::arg("enable"),
             D(rsp, set_sample_sequence_gaps_check))

        .def("set_show_gain_changes",
             &rsp::set_show_gain_changes,
             py::arg("enable"),
             D(rsp, set_show_gain_changes))

    ;
}
