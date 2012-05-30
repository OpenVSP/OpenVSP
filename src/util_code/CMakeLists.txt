CMAKE_MINIMUM_REQUIRED(VERSION 1.4)

SET(UTIL_CODE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to util_code headers")

ADD_LIBRARY(util_code
bbox.cpp
bezier_curve.cpp
bezier_patch.cpp
bezier_surf.cpp
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
array.h
array_2d.h
bbox.h
bezier_curve.h
bezier_patch.h
bezier_surf.h
defines.h
dl_list.h
dyn_array.h
dyn_array_dbl.h
herm_curve.h
int_curve.h
matrix.h
misc_func.h
sarray.h
sdyn_array.h
state.h
stringc.h
timer.h
tritri.h
util.h
vec2d.h
vec3d.h
vector_util.h
)
