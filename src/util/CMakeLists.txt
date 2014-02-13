CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(UTIL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to util headers")

INCLUDE_DIRECTORIES(
    ${NANOFLANN_INCLUDE_DIR}
    ${XMLVSP_INCLUDE_DIR}
    ${UTIL_INCLUDE_DIR}
    ${CPPTEST_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${CodeEli_INCLUDE_DIRS}
    )

ADD_LIBRARY(util
BndBox.cpp
DrawObj.cpp
Matrix.cpp
MessageMgr.cpp
PntNodeMerge.cpp
Quat.cpp
StlHelper.cpp
StringUtil.cpp
SuperEllipse.cpp
UtilTestSuite.cpp
Vec2d.cpp
Vec3d.cpp
VspCurve.cpp
VspSurf.cpp
BndBox.h
Defines.h
DrawObj.h
Matrix.h
MessageMgr.h
PntNodeMerge.h
Quat.h
StlHelper.h
StreamUtil.h
StringUtil.h
SuperEllipse.h
Util.h
UtilTestSuite.h
UsingCpp11.h
Vec2d.h
Vec3d.h
VspCurve.h
VspSurf.h
)
