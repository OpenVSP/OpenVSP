CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(UTIL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to util headers")

INCLUDE_DIRECTORIES(
    ${NANOFLANN_INCLUDE_DIR}
    ${XMLVSP_INCLUDE_DIR}
    ${UTIL_INCLUDE_DIR}
    ${CPPTEST_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${CodeEli_INCLUDE_DIRS}
    ${STEPCODE_INCLUDE_DIR}
    ${GEOM_API_INCLUDE_DIR}
    ${LIBIGES_INCLUDE_DIR}
    )

ADD_LIBRARY(util
BndBox.cpp
Cluster.cpp
DrawObj.cpp
DXFUtil.cpp
FileUtil.cpp
Matrix.cpp
MessageMgr.cpp
PntNodeMerge.cpp
ProcessUtil.cpp
Quat.cpp
STEPutil.cpp
StlHelper.cpp
StringUtil.cpp
SuperEllipse.cpp
Util.cpp
UtilTestSuite.cpp
Vec2d.cpp
Vec3d.cpp
Vsp1DCurve.cpp
VspCurve.cpp
VspSurf.cpp
BndBox.h
Cluster.h
Combination.h
Defines.h
DrawObj.h
DXFUtil.h
FileUtil.h
GuiDeviceEnums.h
Matrix.h
MessageMgr.h
PntNodeMerge.h
ProcessUtil.h
Quat.h
StlHelper.h
STEPutil.h
StreamUtil.h
StringUtil.h
SuperEllipse.h
Util.h
UtilTestSuite.h
UsingCpp11.h
Vec2d.h
Vec3d.h
Vsp1DCurve.h
VspCurve.h
VspSurf.h
WriteMatlab.h
XferSurf.h
)

ADD_DEPENDENCIES( util
STEPCODE
)
