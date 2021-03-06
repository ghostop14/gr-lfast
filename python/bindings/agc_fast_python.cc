/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(agc_fast.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(f55acbf11fe5c50f0d411d9b32439965)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <lfast/agc_fast.h>
// pydoc.h is automatically generated in the build directory
#include <agc_fast_pydoc.h>

void bind_agc_fast(py::module& m)
{

    using agc_fast    = ::gr::lfast::agc_fast;


    py::class_<agc_fast, gr::sync_block, gr::block, gr::basic_block,
        std::shared_ptr<agc_fast>>(m, "agc_fast", D(agc_fast))

        .def(py::init(&agc_fast::make),
           py::arg("rate") = 1.0E-4,
           py::arg("reference") = 1.,
           py::arg("gain") = 1.,
           D(agc_fast,make)
        )
        




        
        .def("rate",&agc_fast::rate,       
            D(agc_fast,rate)
        )


        
        .def("reference",&agc_fast::reference,       
            D(agc_fast,reference)
        )


        
        .def("gain",&agc_fast::gain,       
            D(agc_fast,gain)
        )


        
        .def("max_gain",&agc_fast::max_gain,       
            D(agc_fast,max_gain)
        )


        
        .def("set_rate",&agc_fast::set_rate,       
            py::arg("rate"),
            D(agc_fast,set_rate)
        )


        
        .def("set_reference",&agc_fast::set_reference,       
            py::arg("reference"),
            D(agc_fast,set_reference)
        )


        
        .def("set_gain",&agc_fast::set_gain,       
            py::arg("gain"),
            D(agc_fast,set_gain)
        )


        
        .def("set_max_gain",&agc_fast::set_max_gain,       
            py::arg("max_gain"),
            D(agc_fast,set_max_gain)
        )

        ;



        py::module m_kernel = m.def_submodule("kernel");







}








