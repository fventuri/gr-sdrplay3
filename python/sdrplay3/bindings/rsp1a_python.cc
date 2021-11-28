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
#include <gnuradio/sdrplay3/rsp1a.h>
// pydoc.h is automatically generated in the build directory
#include <rsp1a_pydoc.h>

template <typename... Args>
using overload_cast_ = py::detail::overload_cast_impl<Args...>;

void bind_rsp1a(py::module& m)
{
    using rsp1a = gr::sdrplay3::rsp1a;

    py::class_<rsp1a,
               gr::sdrplay3::rsp,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<rsp1a>> RSP1A(m, "rsp1a", D(rsp1a));

    RSP1A.def(py::init(&rsp1a::make),
              py::arg("selector"),
              py::arg("stream_args") = "",
              D(rsp1a, make))

        .def("set_rf_notch_filter",
             &rsp1a::set_rf_notch_filter,
             py::arg("enable"),
             D(rsp1a, set_rf_notch_filter))

        .def("set_dab_notch_filter",
             &rsp1a::set_dab_notch_filter,
             py::arg("enable"),
             D(rsp1a, set_dab_notch_filter))

        .def("set_biasT",
             &rsp1a::set_biasT,
             py::arg("enable"),
             D(rsp1a, set_biasT))

    ;
}
