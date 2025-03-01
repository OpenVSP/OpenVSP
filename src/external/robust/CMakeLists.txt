CMAKE_MINIMUM_REQUIRED( VERSION 3.24 )

SET( ROBUST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to robust predicates library" )

INCLUDE_DIRECTORIES( ${ROBUST_INCLUDE_DIR} )

ADD_LIBRARY( robust
        predicates.c
        predicates.h
)
