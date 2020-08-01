# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND sdrplay3_python_files
    sdrplay3_types_python.cc
    rsp_python.cc
    rsp1_python.cc
    rsp1a_python.cc
    rsp2_python.cc
    rspduo_python.cc
    rspdx_python.cc
    python_bindings.cc)

GR_PYBIND_MAKE_OOT(sdrplay3 
   ../..
   gr::sdrplay3
   "${sdrplay3_python_files}")

install(TARGETS sdrplay3_python DESTINATION ${GR_PYTHON_DIR}/sdrplay3 COMPONENT pythonapi)
