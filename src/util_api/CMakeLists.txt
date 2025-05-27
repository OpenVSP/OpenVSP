CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(UTIL_API_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to util_api headers")

INCLUDE_DIRECTORIES(
        ${UTIL_API_INCLUDE_DIR}
        ${EIGEN3_INCLUDE_DIR}
        ${ROBUST_INCLUDE_DIR}
)

ADD_LIBRARY(util_api
Matrix4d.cpp
MessageMgr.cpp
Vec2d.cpp
Vec3d.cpp
Matrix4d.h
MessageMgr.h
Vec2d.h
Vec3d.h
)
