CMAKE_MINIMUM_REQUIRED(VERSION 1.4)

FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(JPEG REQUIRED)
FIND_PACKAGE(FLTK REQUIRED)

IF(WIN32)
  SET(OPENNURBS_LIBRARIES
    openNURBS_staticlib
    zlib
    rpcrt4)
  SET(OPENNURBS_LINK_DIR
    ${VSP_SOURCE_DIR}/opennurbs/release
    ${VSP_SOURCE_DIR}/opennurbs/zlib/Release)
ELSE(WIN32)
  SET(OPENNURBS_LIBRARIES
    openNURBS)
  SET(OPENNURBS_LINK_DIR
    ${VSP_SOURCE_DIR}/opennurbs)
ENDIF(WIN32)

INCLUDE_DIRECTORIES( ${VSP_SOURCE_DIR}
    ${UTIL_CODE_INCLUDE_DIR}
    ${TRIANGLE_INCLUDE_DIR}
    ${NANOFLANN_INCLUDE_DIR}
    ${LOAD_JPG_INCLUDE_DIR}
    ${SCREENS_INCLUDE_DIR}
    ${XMLVSP_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIR}
    ${FLTK_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${VSP_SOURCE_DIR}/opennurbs )

LINK_DIRECTORIES(
    ${OPENNURBS_LINK_DIR}
)

ADD_EXECUTABLE(vsp
af.cpp
aircraft.cpp
blankScreen.cpp
cabinLayoutGeom.cpp
cabinLayoutScreen.cpp
CfdMeshMgr.cpp
cfdMeshScreen.cpp
curveEditScreen.cpp
drawBase.cpp
ductGeom.cpp
ductScreen.cpp
editCurve.cpp
engineGeom.cpp
engineScreen.cpp
ext_geom.cpp
extScreen.cpp
FeaMeshMgr.cpp
FeaPart.cpp
feaStructScreen.cpp
fontMgr.cpp
fuse_geom.cpp
fuse_xsec.cpp
FuselageGeom.cpp
FuselageScreen.cpp
FuselageXSec.cpp
fuseScreen.cpp
geom.cpp
geomScreen.cpp
glfont2.cpp
GridDensity.cpp
groupScreen.cpp
gui_device.cpp
havoc_geom.cpp
havocScreen.cpp
hrmScreen.cpp
hwb.cpp
hwbScreen.cpp
ICurve.cpp
ISegChain.cpp
labelGeom.cpp
labelScreen.cpp
main.cpp
mapSource.cpp
materialMgr.cpp
Mesh.cpp
meshGeom.cpp
meshScreen.cpp
ms_wing.cpp
msWingScreen.cpp
parm.cpp
parmLinkMgr.cpp
parmLinkScreen.cpp
parmScreen.cpp
part.cpp
propGeom.cpp
propScreen.cpp
screenMgr.cpp
scriptMgr.cpp
SCurve.cpp
selectFileScreen.cpp
structureMgr.cpp
structureScreen.cpp
Surf.cpp
SurfPatch.cpp
textureMgr.cpp
textureMgrScreen.cpp
tMesh.cpp
trackball.c
track_ball.cpp
Tri.cpp
vorGeom.cpp
vorSlice.cpp
vorviewScreen.cpp
vspGlWindow.cpp
VspPreferences.cpp
vspScreen.cpp
wingGeom.cpp
wingScreen.cpp
xsec_surf.cpp
)

TARGET_LINK_LIBRARIES(vsp
    util_code
    triangle
    load_jpg
    screens
    xmlvsp
    sixseries
    ${JPEG_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${FLTK_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${OPENNURBS_LIBRARIES}
)

INSTALL( TARGETS vsp RUNTIME DESTINATION . )
