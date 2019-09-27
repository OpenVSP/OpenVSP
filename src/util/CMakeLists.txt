CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(UTIL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to util headers")

# set to allow /bigobj when building under Microsoft Visual C
IF (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
ENDIF()

INCLUDE_DIRECTORIES(
    ${UTIL_API_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${NANOFLANN_INCLUDE_DIR}
    ${XMLVSP_INCLUDE_DIR}
    ${UTIL_INCLUDE_DIR}
    ${CPPTEST_INCLUDE_DIRS}
    ${CodeEli_INCLUDE_DIRS}
    ${STEPCODE_INCLUDE_DIR}
    ${GEOM_API_INCLUDE_DIR}
    ${LIBIGES_INCLUDE_DIR}
    ${PCGCPP_INCLUDE_DIR}
    ${GTE_INCLUDE_DIR}
    )

ADD_LIBRARY(util
BndBox.cpp
CADutil.cpp
Cluster.cpp
DrawObj.cpp
DXFUtil.cpp
FileUtil.cpp
PntNodeMerge.cpp
ProcessUtil.cpp
Quat.cpp
SimpleFeatureTess.cpp
SimpleTess.cpp
StackTrace.cpp
StlHelper.cpp
StringUtil.cpp
SuperEllipse.cpp
UnitConversion.cpp
UtilTestSuite.cpp
VKTAirfoil.cpp
Vsp1DCurve.cpp
VspCurve.cpp
VspSurf.cpp
VspUtil.cpp
BitMask.h
BndBox.h
CADutil.h
Cluster.h
Combination.h
DrawObj.h
DXFUtil.h
FileUtil.h
GuiDeviceEnums.h
PntNodeMerge.h
ProcessUtil.h
Quat.h
SimpleFeatureTess.h
SimpleTess.h
StackTrace.h
StlHelper.h
StreamUtil.h
StringUtil.h
SuperEllipse.h
tinydir.h
UnitConversion.h
UtilTestSuite.h
VKTAirfoil.h
Vsp1DCurve.h
VspCurve.h
VspError.h
VspSurf.h
VspUtil.h
WriteMatlab.h
XferSurf.h
)

target_link_libraries( util Eigen3::Eigen )

IF( WIN32 )
    # DbgHelp is a standard Windows SDK component; provides symbol + line info.
    target_link_libraries( util dbghelp )
ELSEIF( UNIX AND NOT APPLE )
    # dl provides dladdr (used for per-frame module resolution on Linux).
    target_link_libraries( util dl )
ELSE()
    # macOS: dladdr is in libSystem (always linked); atos is invoked at runtime.
ENDIF()
