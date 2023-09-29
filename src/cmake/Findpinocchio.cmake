#
# Try to find PINOCCHIO library and include path.
# Once done this will define
#
# PINOCCHIO_FOUND
# PINOCCHIO_INCLUDE_PATH
# PINOCCHIO_LIBRARY
# 

FIND_PATH( PINOCCHIO_INCLUDE_DIR
    NAMES pinocchioApi.h
    HINTS
    ${PINOCCHIO_INSTALL_DIR}
    ${PINOCCHIO_INSTALL_DIR}/include
    /usr/include
    /usr/local/include
    /sw/include
    /opt/local/include
    ${CMAKE_INSTALL_PREFIX}/include
    DOC "The directory where pinocchio/pinocchioApi.h resides")

FIND_LIBRARY( PINOCCHIO_LIBRARIES
    NAMES pinocchio
    HINTS
    ${PINOCCHIO_INSTALL_DIR}
    ${PINOCCHIO_INSTALL_DIR}/lib
    /usr/lib64
    /usr/lib
    /usr/local/lib64
    /usr/local/lib
    /sw/lib
    /opt/local/lib
    ${LIB_INSTALL_DIR}
    DOC "The pinocchio library")

IF (PINOCCHIO_INCLUDE_PATH)
    SET( PINOCCHIO_FOUND 1 CACHE STRING "Set to 1 if pinocchio is found, 0 otherwise")
ELSE (PINOCCHIO_INCLUDE_PATH)
    SET( PINOCCHIO_FOUND 0 CACHE STRING "Set to 1 if pinocchio is found, 0 otherwise")
ENDIF (PINOCCHIO_INCLUDE_PATH)

MARK_AS_ADVANCED( PINOCCHIO_FOUND )