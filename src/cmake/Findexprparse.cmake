#
# Try to find GLEW library and include path.
# Once done this will define
#
# EXPRPARSE_FOUND
# EXPRPARSE_INCLUDE_PATH
# EXPRPARSE_LIBRARY
# 

FIND_PATH( EXPRPARSE_INCLUDE_PATH
    NAMES exprparse/exprparse.h
    HINTS
    ${EXPRPARSE_INSTALL_DIR}
    ${EXPRPARSE_INSTALL_DIR}/include
    /usr/include
    /usr/local/include
    /sw/include
    /opt/local/include
    ${CMAKE_INSTALL_PREFIX}/include
    DOC "The directory where exprparse/exprparse.h resides")
FIND_LIBRARY( EXPRPARSE_LIBRARY
    NAMES exprparse
    HINTS
    ${EXPRPARSE_INSTALL_DIR}
    ${EXPRPARSE_INSTALL_DIR}/lib
    /usr/lib64
    /usr/lib
    /usr/local/lib64
    /usr/local/lib
    /sw/lib
    /opt/local/lib
    ${LIB_INSTALL_DIR}
    DOC "The exprparse library")

IF (EXPRPARSE_INCLUDE_PATH)
    SET( EXPRPARSE_FOUND 1 CACHE STRING "Set to 1 if exprparse is found, 0 otherwise")
ELSE (EXPRPARSE_INCLUDE_PATH)
    SET( EXPRPARSE_FOUND 0 CACHE STRING "Set to 1 if exprparse is found, 0 otherwise")
ENDIF (EXPRPARSE_INCLUDE_PATH)

MARK_AS_ADVANCED( EXPRPARSE_FOUND )