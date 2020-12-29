/*
 * Copyright 2020 Free Software Foundation, Inc.
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
/* The following comment block is used for
/* gr_modtool to insert function prototypes
/* Please do not delete
/**************************************/
// BINDING_FUNCTION_PROTOTYPES(
    void bind_agc_fast_ff(py::module& m);
    void bind_agc_fast(py::module& m);
    void bind_CC2F2ByteVector(py::module& m);
    void bind_costas2(py::module& m);
    void bind_costas4(py::module& m);
    void bind_MTFIRFilterCCC(py::module& m);
    void bind_MTFIRFilterCCF(py::module& m);
    void bind_MTFIRFilterFF(py::module& m);
    void bind_nlog10volk(py::module& m);
    void bind_quad_demod_volk(py::module& m);
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

PYBIND11_MODULE(lfast_python, m)
{
    // Initialize the numpy C API
    // (otherwise we will see segmentation faults)
    init_numpy();

    // Allow access to base block methods
    py::module::import("gnuradio.gr");

    /**************************************/
    /* The following comment block is used for
    /* gr_modtool to insert binding function calls
    /* Please do not delete
    /**************************************/
    // BINDING_FUNCTION_CALLS(
    bind_agc_fast_ff(m);
    bind_agc_fast(m);
    bind_CC2F2ByteVector(m);
    bind_costas2(m);
    bind_costas4(m);
    bind_MTFIRFilterCCC(m);
    bind_MTFIRFilterCCF(m);
    bind_MTFIRFilterFF(m);
    bind_nlog10volk(m);
    bind_quad_demod_volk(m);
    // ) END BINDING_FUNCTION_CALLS
}