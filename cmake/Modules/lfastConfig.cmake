INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_LFAST lfast)

FIND_PATH(
    LFAST_INCLUDE_DIRS
    NAMES lfast/api.h
    HINTS $ENV{LFAST_DIR}/include
        ${PC_LFAST_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    LFAST_LIBRARIES
    NAMES gnuradio-lfast
    HINTS $ENV{LFAST_DIR}/lib
        ${PC_LFAST_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/lfastTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LFAST DEFAULT_MSG LFAST_LIBRARIES LFAST_INCLUDE_DIRS)
MARK_AS_ADVANCED(LFAST_LIBRARIES LFAST_INCLUDE_DIRS)
