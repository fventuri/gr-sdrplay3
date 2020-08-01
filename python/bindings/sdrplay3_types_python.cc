/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/complex.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <sdrplay3/sdrplay3_types.h>

void bind_sdrplay3_types(py::module& m)
{
    using stream_args_t = gr::sdrplay3::stream_args_t;

    py::class_<stream_args_t>(m, "stream_args")
        .def(py::init<const std::string&, const size_t>(),
             py::arg("output_type") = "fc32",
             py::arg("channels_size") = 1)
        // Properties
        .def_readwrite("output_type", &stream_args_t::output_type)
        .def_readwrite("channels_size", &stream_args_t::channels_size);
}
