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
#include <gnuradio/sdrplay3/rspdx.h>
// pydoc.h is automatically generated in the build directory
#include <rspdx_pydoc.h>

template <typename... Args>
using overload_cast_ = py::detail::overload_cast_impl<Args...>;

void bind_rspdx(py::module& m)
{
    using rspdx = gr::sdrplay3::rspdx;

    py::class_<rspdx,
               gr::sdrplay3::rsp,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<rspdx>> RSPdx(m, "rspdx", D(rspdx));

    RSPdx.def(py::init(&rspdx::make),
              py::arg("selector"),
              py::arg("stream_args") = "",
              D(rspdx, make))

        .def("set_antenna",
             &rspdx::set_antenna,
             py::arg("antenna"),
             D(rspdx, set_antenna))

        .def("get_antenna",
             &rspdx::get_antenna,
             D(rspdx, get_antenna))

        .def("get_antennas",
             &rspdx::get_antennas,
             D(rspdx, get_antennas))

        .def("set_hdr_mode",
             &rspdx::set_hdr_mode,
             py::arg("enable"),
             D(rspdx, set_hdr_mode))

        .def("get_hdr_mode",
             &rspdx::get_hdr_mode,
             D(rspdx, get_hdr_mode))

        .def("set_rf_notch_filter",
             &rspdx::set_rf_notch_filter,
             py::arg("enable"),
             D(rspdx, set_rf_notch_filter))

        .def("set_dab_notch_filter",
             &rspdx::set_dab_notch_filter,
             py::arg("enable"),
             D(rspdx, set_dab_notch_filter))

        .def("set_biasT",
             &rspdx::set_biasT,
             py::arg("enable"),
             D(rspdx, set_biasT))

    ;
}
