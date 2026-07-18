CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(SIXSERIES_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to sixseries header.")

ADD_LIBRARY(sixseries
NACA_6_series.c
r_sign.c
s_cmp.c
s_copy.c
s_stop.c
f2c.h
)

# These sources are machine translated from Fortran by f2c.  The generated form
# trips benign warnings on every compiler -- unused locals/parameters from the
# translation, and numeric-conversion warnings (MSVC C4244/C4267) from f2c's real
# and integer types.  Suppress them for this target rather than editing generated
# numeric code; correctness of the translation is unchanged.
IF(MSVC)
    TARGET_COMPILE_OPTIONS(sixseries PRIVATE /wd4101 /wd4244 /wd4267)
ELSE()
    TARGET_COMPILE_OPTIONS(sixseries PRIVATE
        -Wno-unused-but-set-variable
        -Wno-unused-parameter
        -Wno-unused-variable)
ENDIF()
