# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT lfast_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT lfast_sources)

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

list(APPEND lfast_python_files
    agc_fast_ff_python.cc
    agc_fast_python.cc
    CC2F2ByteVector_python.cc
    costas2_python.cc
    costas4_python.cc
    MTFIRFilterCCC_python.cc
    MTFIRFilterCCF_python.cc
    MTFIRFilterFF_python.cc
    nlog10volk_python.cc
    quad_demod_volk_python.cc python_bindings.cc)

GR_PYBIND_MAKE_OOT(lfast 
   ../..
   gr::lfast
   "${lfast_python_files}")

install(TARGETS lfast_python DESTINATION ${GR_PYTHON_DIR}/lfast COMPONENT pythonapi)
