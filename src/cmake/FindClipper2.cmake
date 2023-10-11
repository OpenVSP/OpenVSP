#
# Try to find Clipper2 library and include path.
# Once done this will define
#
# CLIPPER2_FOUND
# CLIPPER2_INCLUDE_DIR
# CLIPPER2_LIBRARIES
# 

FIND_PATH( CLIPPER2_INCLUDE_DIR
    NAMES clipper2/clipper.h
    HINTS
    ${CLIPPER2_INSTALL_DIR}
    ${CLIPPER2_INSTALL_DIR}/include
    /usr/include
    /usr/local/include
    /sw/include
    /opt/local/include
    ${CMAKE_INSTALL_PREFIX}/include
    DOC "The directory where clipper2/clipper.h resides")

FIND_LIBRARY( CLIPPER2_LIBRARIES
    NAMES Clipper2
    HINTS
    ${CLIPPER2_INSTALL_DIR}
    ${CLIPPER2_INSTALL_DIR}/lib
    /usr/lib64
    /usr/lib
    /usr/local/lib64
    /usr/local/lib
    /sw/lib
    /opt/local/lib
    ${LIB_INSTALL_DIR}
    DOC "The Clipper2 library")

IF (CLIPPER2_INCLUDE_DIR)
    SET( CLIPPER2_FOUND 1 CACHE STRING "Set to 1 if Clipper2 is found, 0 otherwise")
ELSE (CLIPPER2_INCLUDE_DIR)
    SET( CLIPPER2_FOUND 0 CACHE STRING "Set to 1 if Clipper2 is found, 0 otherwise")
ENDIF (CLIPPER2_INCLUDE_DIR)

MARK_AS_ADVANCED( CLIPPER2_FOUND )