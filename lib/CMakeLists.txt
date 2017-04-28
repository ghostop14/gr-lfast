# Copyright 2011,2012,2016 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

set (CMAKE_CXX_STANDARD 11)

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

list(APPEND lfast_sources
    costas2_impl.cc
    costas4_impl.cc
    agc_fast_impl.cc
    agc_fast_ff_impl.cc
    CC2F2ByteVector_impl.cc
)

set(lfast_sources "${lfast_sources}" PARENT_SCOPE)
if(NOT lfast_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT lfast_sources)

add_library(gnuradio-lfast SHARED ${lfast_sources})
target_link_libraries(gnuradio-lfast ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES} boost_thread gnuradio-blocks volk
)
set_target_properties(gnuradio-lfast PROPERTIES DEFINE_SYMBOL "gnuradio_lfast_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-lfast PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
include(GrMiscUtils)
GR_LIBRARY_FOO(gnuradio-lfast RUNTIME_COMPONENT "lfast_runtime" DEVEL_COMPONENT "lfast_devel")

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})

list(APPEND test_lfast_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_lfast.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_lfast.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_CC2F2ByteVector.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_agc_fast.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_costas2.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/costas2_impl.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/costas4_impl.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/agc_fast_impl.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/CC2F2ByteVector_impl.cc
)

add_executable(test-lfast ${test_lfast_sources})

target_link_libraries(
  test-lfast
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  boost_thread
  ${CPPUNIT_LIBRARIES}
  gnuradio-blocks
  volk
  gnuradio-lfast
)

install(TARGETS test-lfast DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" RUNTIME)


GR_ADD_TEST(test_lfast test-lfast)

########################################################################
# Print summary
########################################################################
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Building for version: ${VERSION} / ${LIBVER}")

