CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(CFD_MESH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to cfd_mesh headers")

get_target_property( OpenABF_INCLUDE_DIR OpenABF::OpenABF INTERFACE_INCLUDE_DIRECTORIES )

if ( MSVC )
    add_compile_options( /Zc:__cplusplus )
endif()

INCLUDE_DIRECTORIES( 
    ${NANOFLANN_INCLUDE_DIR}
    ${UTIL_INCLUDE_DIR}
    ${UTIL_API_INCLUDE_DIR}
    ${CPPTEST_INCLUDE_DIR} 
    ${LIBXML2_INCLUDE_DIR}
    ${GEOM_CORE_INCLUDE_DIR}
    ${XMLVSP_INCLUDE_DIR}
    ${TRIANGLE_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${CodeEli_INCLUDE_DIRS}
    ${TRITRI_INCLUDE_DIR}
    ${GEOM_API_INCLUDE_DIR}
    ${STEPCODE_INCLUDE_DIR}
    ${LIBIGES_INCLUDE_DIR}
    ${OpenABF_INCLUDE_DIR}
   )

ADD_LIBRARY(cfd_mesh
BezierCurve.cpp
CfdMeshMgr.cpp
Face.cpp
FeaElement.cpp
FeaMesh.cpp
FeaMeshMgr.cpp
ICurve.cpp
IntersectPatch.cpp
ISegChain.cpp
Mesh.cpp
MeshAnalysis.cpp
NURBS.cpp
SCurve.cpp
SimpleBC.cpp
SimpleMeshSettings.cpp
SimpleSubSurface.cpp
Surf.cpp
SurfaceIntersectionMgr.cpp
SurfCore.cpp
SurfPatch.cpp
TwoDNN.cpp
BezierCurve.h
CfdMeshMgr.h
Face.h
FeaElement.h
FeaMesh.h
FeaMeshMgr.h
ICurve.h
IntersectPatch.h
ISegChain.h
MapSource.h
Mesh.h
MeshAnalysis.h
NURBS.h
SCurve.h
SimpleBC.h
SimpleMeshSettings.h
SimpleSubSurface.h
Surf.h
SurfaceIntersectionMgr.h
SurfCore.h
SurfPatch.h
TwoDNN.h
)

ADD_DEPENDENCIES( cfd_mesh
util
)
