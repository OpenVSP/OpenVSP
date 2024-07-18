CMAKE_MINIMUM_REQUIRED( VERSION 3.24 )

SET( CARTESIAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to cartesian library" )

ADD_LIBRARY( cartesian
    Cartesian.cpp
    Cartesian.H
)

INCLUDE_DIRECTORIES( ${FLTK_INCLUDE_DIR} )

ADD_EXECUTABLE( cartesian_example
    example.cpp
)

TARGET_LINK_LIBRARIES( cartesian_example
    cartesian
    fltk::gl
)
