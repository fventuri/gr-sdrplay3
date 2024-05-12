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
#include <gnuradio/sdrplay3/rspdxr2.h>
// pydoc.h is automatically generated in the build directory
#include <rspdxr2_pydoc.h>

template <typename... Args>
using overload_cast_ = py::detail::overload_cast_impl<Args...>;

void bind_rspdxr2(py::module& m)
{
    using rspdxr2 = gr::sdrplay3::rspdxr2;

    py::class_<rspdxr2,
               gr::sdrplay3::rspdx,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<rspdxr2>> RSPdxR2(m, "rspdxr2", D(rspdxr2));

    RSPdxR2.def(py::init(&rspdxr2::make),
                py::arg("selector"),
                py::arg("stream_args") = "",
                D(rspdxr2, make))

    ;
}
