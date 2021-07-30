CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(TRITRI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to tritri headers")

INCLUDE_DIRECTORIES(${TRITRI_INCLUDE_DIR}
    )

ADD_LIBRARY(tritri
intersect_AABB.cpp
intersect_triangle.cpp
tri_tri_intersect.cpp
tri_tri_intersect.h
)