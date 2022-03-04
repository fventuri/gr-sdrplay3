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

#include <sdrplay3/sdrplay3_types.h>
#include <sdrplay3/rspduo.h>
// pydoc.h is automatically generated in the build directory
#include <rspduo_pydoc.h>

template <typename... Args>
using overload_cast_ = py::detail::overload_cast_impl<Args...>;

void bind_rspduo(py::module& m)
{
    using rspduo = gr::sdrplay3::rspduo;

    py::class_<rspduo,
               gr::sdrplay3::rsp,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<rspduo>> RSPduo(m, "rspduo", D(rspduo));

    RSPduo.def(py::init(&rspduo::make),
               py::arg("selector"),
               py::arg("rspduo_mode") = "Single Tuner",
               py::arg("antenna") = "Tuner 1 50 ohm",
               py::arg("stream_args") = "",
               D(rspduo, make))

        .def("set_sample_rate",
             &rspduo::set_sample_rate,
             py::arg("rate"),
             D(rspduo, set_sample_rate))

        .def("get_sample_rate_range",
             &rspduo::get_sample_rate_range,
             D(rspduo, get_sample_rate_range))

        .def("get_valid_sample_rates",
             &rspduo::get_valid_sample_rates,
             D(rspduo, get_valid_sample_rates))

        .def("set_center_freq",
             overload_cast_<double>()(&rspduo::set_center_freq),
             py::arg("freq"),
             D(rspduo, set_center_freq))

        .def("set_center_freq",
             overload_cast_<double, int>()(&rspduo::set_center_freq),
             py::arg("freq"),
             py::arg("tuner"),
             D(rspduo, set_center_freq))

        .def("set_center_freq",
             overload_cast_<double, double>()(&rspduo::set_center_freq),
             py::arg("freq_A"),
             py::arg("freq_B"),
             D(rspduo, set_center_freq))

        .def("get_center_freq",
             overload_cast_<>()(&rspduo::get_center_freq, py::const_),
             D(rspduo, get_center_freq))

        .def("get_center_freq",
             overload_cast_<int>()(&rspduo::get_center_freq, py::const_),
             py::arg("tuner"),
             D(rspduo, get_center_freq))

        .def("set_antenna",
             &rspduo::set_antenna,
             py::arg("antenna"),
             D(rspduo, set_antenna))

        .def("get_antenna",
             &rspduo::get_antenna,
             D(rspduo, get_antenna))

        .def("get_antennas",
             &rspduo::get_antennas,
             D(rspduo, get_antennas))

        .def("set_gain",
             overload_cast_<double, const std::string&>()(&rspduo::set_gain),
             py::arg("gain"),
             py::arg("name"),
             D(rspduo, set_gain))

        .def("set_gain",
             overload_cast_<double, const std::string&, const int>()(&rspduo::set_gain),
             py::arg("gain"),
             py::arg("name"),
             py::arg("tuner"),
             D(rspduo, set_gain))

        .def("set_gain",
             overload_cast_<double, double, const std::string&>()(&rspduo::set_gain),
             py::arg("gain_A"),
             py::arg("gain_B"),
             py::arg("name"),
             D(rspduo, set_gain))

        .def("get_gain",
             overload_cast_<const std::string&>()(&rspduo::get_gain, py::const_),
             py::arg("name"),
             D(rspduo, get_gain))

        .def("get_gain",
             overload_cast_<const std::string&, const int>()(&rspduo::get_gain, py::const_),
             py::arg("name"),
             py::arg("tuner"),
             D(rspduo, get_gain))

        .def("get_gain_range",
             [](const rspduo& self, const std::string& name) {
                 const double *range = self.get_gain_range(name);
                 return py::make_tuple(range[0], range[1]);
             },
             py::arg("name"),
             D(rspduo, get_gain_range))

        .def("get_gain_range",
             [](const rspduo& self, const std::string& name, const int tuner) {
                 const double *range = self.get_gain_range(name, tuner);
                 return py::make_tuple(range[0], range[1]);
             },
             py::arg("name"),
             py::arg("tuner"),
             D(rspduo, get_gain_range))

        .def("set_gain_mode",
             overload_cast_<bool>()(&rspduo::set_gain_mode),
             py::arg("automatic"),
             D(rspduo, set_gain_mode))

        .def("set_gain_mode",
             overload_cast_<bool, const int>()(&rspduo::set_gain_mode),
             py::arg("automatic"),
             py::arg("tuner"),
             D(rspduo, set_gain_mode))

        .def("set_gain_modes",
             overload_cast_<bool, bool>()(&rspduo::set_gain_mode),
             py::arg("automatic_A"),
             py::arg("automatic_B"),
             D(rspduo, set_gain_mode))

        .def("get_gain_mode",
             overload_cast_<>()(&rspduo::get_gain_mode, py::const_),
             D(rspduo, get_gain_mode))

        .def("get_gain_mode",
             overload_cast_<const int>()(&rspduo::get_gain_mode, py::const_),
             py::arg("tuner"),
             D(rspduo, get_gain_mode))

        .def("set_rf_notch_filter",
             &rspduo::set_rf_notch_filter,
             py::arg("enable"),
             D(rspduo, set_rf_notch_filter))

        .def("set_dab_notch_filter",
             &rspduo::set_dab_notch_filter,
             py::arg("enable"),
             D(rspduo, set_dab_notch_filter))

        .def("set_am_notch_filter",
             &rspduo::set_am_notch_filter,
             py::arg("enable"),
             D(rspduo, set_am_notch_filter))

        .def("set_biasT",
             &rspduo::set_biasT,
             py::arg("enable"),
             D(rspduo, set_biasT))

    ;
}
