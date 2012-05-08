CMAKE_MINIMUM_REQUIRED(VERSION 1.4)

SET(UTIL_CODE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to util_code headers")

ADD_LIBRARY(util_code
bbox.cpp
bezier_curve.cpp
bezier_patch.cpp
bezier_surf.cpp
bicubic_surf.cpp
dyn_array_dbl.cpp
herm_curve.cpp
int_curve.cpp
intersect_triangle.cpp
matrix.cpp
stringc.cpp
timer.cpp
tritri.cpp
util.cpp
vec2d.cpp
vec3d.cpp
vector_util.cpp
)
