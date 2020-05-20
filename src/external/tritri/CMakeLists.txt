CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(TRITRI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to tritri headers")

INCLUDE_DIRECTORIES(${TRITRI_INCLUDE_DIR}
    )

ADD_LIBRARY(tritri
intersect_AABB.cpp
intersect_triangle.cpp
Tritri.cpp
Tritri.h
tri_tri_intersect.cpp
tri_tri_intersect.h
)