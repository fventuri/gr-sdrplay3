/*
 * Copyright 2020,2021 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

namespace py = pybind11;

// Headers for binding functions
/**************************************/
// The following comment block is used for
// gr_modtool to insert function prototypes
// Please do not delete
/**************************************/
// BINDING_FUNCTION_PROTOTYPES(
    void bind_sdrplay3_types(py::module& m);
    void bind_rsp(py::module& m);
    void bind_rsp1(py::module& m);
    void bind_rsp1a(py::module& m);
    void bind_rsp1b(py::module& m);
    void bind_rsp2(py::module& m);
    void bind_rspduo(py::module& m);
    void bind_rspdx(py::module& m);
    void bind_rspdxr2(py::module& m);
// ) END BINDING_FUNCTION_PROTOTYPES


// We need this hack because import_array() returns NULL
// for newer Python versions.
// This function is also necessary because it ensures access to the C API
// and removes a warning.
void* init_numpy()
{
    import_array();
    return NULL;
}

PYBIND11_MODULE(sdrplay3_python, m)
{
    // Initialize the numpy C API
    // (otherwise we will see segmentation faults)
    init_numpy();

    // Allow access to base block methods
    py::module::import("gnuradio.gr");

    /**************************************/
    // The following comment block is used for
    // gr_modtool to insert binding function calls
    // Please do not delete
    /**************************************/
    // BINDING_FUNCTION_CALLS(
    bind_sdrplay3_types(m);
    bind_rsp(m);
    bind_rsp1(m);
    bind_rsp1a(m);
    bind_rsp1b(m);
    bind_rsp2(m);
    bind_rspduo(m);
    bind_rspdx(m);
    bind_rspdxr2(m);
    // ) END BINDING_FUNCTION_CALLS
}
