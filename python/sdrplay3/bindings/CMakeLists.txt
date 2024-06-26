# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT sdrplay3_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT sdrplay3_sources)

########################################################################
# Check for pygccxml
########################################################################
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
    rsp1b_python.cc
    rsp2_python.cc
    rspduo_python.cc
    rspdx_python.cc
    rspdxr2_python.cc
    python_bindings.cc)

GR_PYBIND_MAKE_OOT(sdrplay3
   ../../..
   gr::sdrplay3
   "${sdrplay3_python_files}")

## copy in bindings .so file for use in QA test module
#add_custom_target(
#  copy_bindings_for_tests ALL
#  COMMAND
#    ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/*.so"
#    ${CMAKE_BINARY_DIR}/test_modules/gnuradio/sdrplay3/
#  DEPENDS sdrplay3_python)

install(TARGETS sdrplay3_python DESTINATION ${GR_PYTHON_DIR}/gnuradio/sdrplay3 COMPONENT pythonapi)
