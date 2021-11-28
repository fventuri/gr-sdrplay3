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

#include <gnuradio/sdrplay3/sdrplay3_types.h>
#include <gnuradio/sdrplay3/rsp2.h>
// pydoc.h is automatically generated in the build directory
#include <rsp2_pydoc.h>

template <typename... Args>
using overload_cast_ = py::detail::overload_cast_impl<Args...>;

void bind_rsp2(py::module& m)
{
    using rsp2 = gr::sdrplay3::rsp2;

    py::class_<rsp2,
               gr::sdrplay3::rsp,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<rsp2>> RSP2(m, "rsp2", D(rsp2));

    RSP2.def(py::init(&rsp2::make),
              py::arg("selector"),
              py::arg("stream_args") = "",
              D(rsp2, make))

        .def("set_antenna",
             &rsp2::set_antenna,
             py::arg("antenna"),
             D(rsp2, set_antenna))

        .def("get_antenna",
             &rsp2::get_antenna,
             D(rsp2, get_antenna))

        .def("get_antennas",
             &rsp2::get_antennas,
             D(rsp2, get_antennas))

        .def("set_rf_notch_filter",
             &rsp2::set_rf_notch_filter,
             py::arg("enable"),
             D(rsp2, set_rf_notch_filter))

        .def("set_biasT",
             &rsp2::set_biasT,
             py::arg("enable"),
             D(rsp2, set_biasT))

    ;
}
