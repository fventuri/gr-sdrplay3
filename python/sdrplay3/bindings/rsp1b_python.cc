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
#include <gnuradio/sdrplay3/rsp1b.h>
// pydoc.h is automatically generated in the build directory
#include <rsp1b_pydoc.h>

template <typename... Args>
using overload_cast_ = py::detail::overload_cast_impl<Args...>;

void bind_rsp1b(py::module& m)
{
    using rsp1b = gr::sdrplay3::rsp1b;

    py::class_<rsp1b,
               gr::sdrplay3::rsp1a,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<rsp1b>> RSP1B(m, "rsp1b", D(rsp1b));

    RSP1B.def(py::init(&rsp1b::make),
              py::arg("selector"),
              py::arg("stream_args") = "",
              D(rsp1b, make))

    ;
}
