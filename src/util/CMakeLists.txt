CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(UTIL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to util headers")

# set to allow /bigobj when building under Microsoft Visual C
IF (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
ENDIF()

INCLUDE_DIRECTORIES(
    ${LIBXML2_INCLUDE_DIR}
    ${NANOFLANN_INCLUDE_DIR}
    ${XMLVSP_INCLUDE_DIR}
    ${UTIL_INCLUDE_DIR}
    ${CPPTEST_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${CodeEli_INCLUDE_DIRS}
    ${STEPCODE_INCLUDE_DIR}
    ${GEOM_API_INCLUDE_DIR}
    ${LIBIGES_INCLUDE_DIR}
    ${PCGCPP_INCLUDE_DIR}
    )

ADD_LIBRARY(util
BndBox.cpp
CADutil.cpp
Cluster.cpp
DrawObj.cpp
DXFUtil.cpp
FileUtil.cpp
Matrix4d.cpp
MessageMgr.cpp
PntNodeMerge.cpp
ProcessUtil.cpp
Quat.cpp
SimpleFeatureTess.cpp
SimpleTess.cpp
StlHelper.cpp
StringUtil.cpp
SuperEllipse.cpp
UnitConversion.cpp
UtilTestSuite.cpp
Vec2d.cpp
Vec3d.cpp
VKTAirfoil.cpp
Vsp1DCurve.cpp
VspCurve.cpp
VspSurf.cpp
VspUtil.cpp
BndBox.h
CADutil.h
Cluster.h
Combination.h
Defines.h
DrawObj.h
DXFUtil.h
FileUtil.h
GuiDeviceEnums.h
Matrix4d.h
MessageMgr.h
PntNodeMerge.h
ProcessUtil.h
Quat.h
SimpleFeatureTess.h
SimpleTess.h
StlHelper.h
StreamUtil.h
StringUtil.h
SuperEllipse.h
UnitConversion.h
UtilTestSuite.h
UsingCpp11.h
Vec2d.h
Vec3d.h
VKTAirfoil.h
Vsp1DCurve.h
VspCurve.h
VspSurf.h
VspUtil.h
WriteMatlab.h
XferSurf.h
)
