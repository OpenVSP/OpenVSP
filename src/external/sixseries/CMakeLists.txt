CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(SIXSERIES_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to sixseries header.")

ADD_LIBRARY(sixseries
NACA_6_series.c
r_sign.c
s_cmp.c
s_copy.c
s_stop.c
f2c.h
)
