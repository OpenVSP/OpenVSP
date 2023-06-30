#
# Try to find DELABELLA library and include path.
# Once done this will define
#
# DELABELLA_FOUND
# DELABELLA_INCLUDE_PATH
# DELABELLA_LIBRARY
# 

FIND_PATH( DELABELLA_INCLUDE_DIR
    NAMES delabella.h
    PATHS
    ${DELABELLA_INSTALL_DIR}/include
    /usr/include
    /usr/local/include
    /sw/include
    /opt/local/include
    ${CMAKE_INSTALL_PREFIX}/include
    DOC "The directory where dellabella.h resides"
    REQUIRED)
FIND_LIBRARY( DELABELLA_LIBRARIES
    NAMES delabella
    PATHS
    ${DELABELLA_INSTALL_DIR}
    ${DELABELLA_INSTALL_DIR}/lib
    /usr/lib64
    /usr/lib
    /usr/local/lib64
    /usr/local/lib
    /sw/lib
    /opt/local/lib
    ${LIB_INSTALL_DIR}
    DOC "The dellabella library"
    REQUIRED)

IF (DELABELLA_INCLUDE_DIR)
    SET( DELABELLA_FOUND 1 CACHE STRING "Set to 1 if dellabella is found, 0 otherwise")
ELSE (DELABELLA_INCLUDE_DIR)
    SET( DELABELLA_FOUND 0 CACHE STRING "Set to 1 if dellabella is found, 0 otherwise")
ENDIF (DELABELLA_INCLUDE_DIR)
