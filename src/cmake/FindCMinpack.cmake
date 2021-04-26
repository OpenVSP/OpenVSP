# - Find CMINPACK
# Find the native CMINPACK includes and library
# This module defines
#  CMINPACK_INCLUDE_DIR, where to find jpeglib.h, etc.
#  CMINPACK_LIBRARIES, the libraries needed to use CMINPACK.
#  CMINPACK_FOUND, If false, do not try to use CMINPACK.
# also defined, but not for general use are
#  CMINPACK_LIBRARY, where to find the CMINPACK library.

FIND_PATH(CMINPACK_INCLUDE_DIR cminpack.h PATHS
   ${CMINPACK_INSTALL_DIR}/include
   ${CMAKE_INSTALL_PREFIX}/include
   /usr/local/include
   /usr/include
   PATH_SUFFIXES cminpack cminpack-1
)

SET(CMINPACK_NAMES ${CMINPACK_NAMES} cminpack minpack cminpack_s cminpack_d cminpack_s_d)

FIND_LIBRARY(CMINPACK_LIBRARY
   NAMES ${CMINPACK_NAMES} PATHS
   ${CMINPACK_INSTALL_DIR}
   /usr
   /usr/local
   PATH_SUFFIXES lib lib64
)

MESSAGE( STATUS "CMINPACK_LIBRARY " ${CMINPACK_LIBRARY} )

SET(CMINPACK_LIBRARIES ${CMINPACK_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CMINPACK DEFAULT_MSG CMINPACK_INCLUDE_DIR CMINPACK_LIBRARIES )
