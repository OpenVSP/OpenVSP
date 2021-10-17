#
# Try to find GLEW library and include path.
# Once done this will define
#
# GLEW_FOUND
# GLEW_INCLUDE_PATH
# GLEW_LIBRARY
# 

FIND_PATH( GLEW_INCLUDE_PATH
    NAMES GL/glew.h
    HINTS
    ${GLEW_INSTALL_DIR}
    ${GLEW_INSTALL_DIR}/include
    /usr/include
    /usr/local/include
    /sw/include
    /opt/local/include
    ${CMAKE_INSTALL_PREFIX}/include
    DOC "The directory where GL/glew.h resides")
FIND_LIBRARY( GLEW_LIBRARY
    NAMES glews glew glew32 glew32s GLEW
    HINTS
    ${GLEW_INSTALL_DIR}
    ${GLEW_INSTALL_DIR}/lib
    /usr/lib64
    /usr/lib
    /usr/local/lib64
    /usr/local/lib
    /sw/lib
    /opt/local/lib
    ${LIB_INSTALL_DIR}
    DOC "The GLEW library")

IF (GLEW_INCLUDE_PATH)
    SET( GLEW_FOUND 1 CACHE STRING "Set to 1 if GLEW is found, 0 otherwise")
ELSE (GLEW_INCLUDE_PATH)
    SET( GLEW_FOUND 0 CACHE STRING "Set to 1 if GLEW is found, 0 otherwise")
ENDIF (GLEW_INCLUDE_PATH)

MARK_AS_ADVANCED( GLEW_FOUND )
