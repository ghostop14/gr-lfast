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
/* BINDTOOL_HEADER_FILE(MTFIRFilterCCF.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(983a0f4b4b48b4bb69ea58b2df23d301)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <lfast/MTFIRFilterCCF.h>
// pydoc.h is automatically generated in the build directory
#include <MTFIRFilterCCF_pydoc.h>

void bind_MTFIRFilterCCF(py::module& m)
{

    using MTFIRFilterCCF    = ::gr::lfast::MTFIRFilterCCF;


    py::class_<MTFIRFilterCCF, gr::sync_decimator,
        std::shared_ptr<MTFIRFilterCCF>>(m, "MTFIRFilterCCF", D(MTFIRFilterCCF))

        .def(py::init(&MTFIRFilterCCF::make),
           py::arg("decimation"),
           py::arg("taps"),
           py::arg("nthreads"),
           D(MTFIRFilterCCF,make)
        )
        




        
        .def("set_taps",&MTFIRFilterCCF::set_taps,       
            py::arg("taps"),
            D(MTFIRFilterCCF,set_taps)
        )


        
        .def("taps",&MTFIRFilterCCF::taps,       
            D(MTFIRFilterCCF,taps)
        )

        ;




}







