INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_SDRPLAY3 sdrplay3)

FIND_PATH(
    SDRPLAY3_INCLUDE_DIRS
    NAMES sdrplay3/api.h
    HINTS $ENV{SDRPLAY3_DIR}/include
        ${PC_SDRPLAY3_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    SDRPLAY3_LIBRARIES
    NAMES gnuradio-sdrplay3
    HINTS $ENV{SDRPLAY3_DIR}/lib
        ${PC_SDRPLAY3_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/sdrplay3Target.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDRPLAY3 DEFAULT_MSG SDRPLAY3_LIBRARIES SDRPLAY3_INCLUDE_DIRS)
MARK_AS_ADVANCED(SDRPLAY3_LIBRARIES SDRPLAY3_INCLUDE_DIRS)
