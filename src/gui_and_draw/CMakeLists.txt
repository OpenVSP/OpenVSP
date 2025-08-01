CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(GUI_AND_DRAW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to gui_and_draw headers")

INCLUDE_DIRECTORIES( 
    ${NANOFLANN_INCLUDE_DIR}
    ${CLIPPER2_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${XMLVSP_INCLUDE_DIR}
    ${UTIL_INCLUDE_DIR}
    ${UTIL_API_INCLUDE_DIR}
    ${GEOM_CORE_INCLUDE_DIR}
    ${CFD_MESH_INCLUDE_DIR}
    ${FLTK_INCLUDE_DIR}
    ${VSP_GRAPHIC_INCLUDE_DIR}
    ${GLEW_INCLUDE_PATH}
    ${GLM_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${CodeEli_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/geom_api
    ${GEOM_API_INCLUDE_DIR}
    ${STEPCODE_INCLUDE_DIR}
    ${LIBIGES_INCLUDE_DIR}
    ${GUI_AND_DRAW_INCLUDE_DIR}
    ${CARTESIAN_INCLUDE_DIR}
    ${PINOCCHIO_INCLUDE_DIR}
    ${EXPRPARSE_INCLUDE_PATH}
)

ADD_LIBRARY(gui_and_draw
AdvLinkScreen.cpp
AdvLinkVarRenameScreen.cpp
AeroStructScreen.cpp
AirfoilExportScreen.cpp
AttributeEditor.cpp
AttributeScreen.cpp
AuxiliaryGeomScreen.cpp
BlankScreen.cpp
BEMOptionsScreen.cpp
BORScreen.cpp
CfdMeshScreen.cpp
ClippingScreen.cpp
CompGeomScreen.cpp
ConformalScreen.cpp
CurveEditScreen.cpp
CustomScreen.cpp
DegenGeomScreen.cpp
DesignVarScreen.cpp
DXFOptionsScreen.cpp
EllipsoidScreen.cpp
ExportScreen.cpp
FeaPartEditScreen.cpp
FitModelScreen.cpp
FuselageScreen.cpp
GearScreen.cpp
GeometryAnalysisScreen.cpp
GroupLayout.cpp
GuiDevice.cpp
HingeScreen.cpp
HumanGeomScreen.cpp
IGESOptionsScreen.cpp
IGESStructureOptionsScreen.cpp
ImportScreen.cpp
MainGLWindow.cpp
MainGLWindow.cpp
MainVSPScreen.cpp
ManageBackgroundScreen.cpp
ManageBackground3DScreen.cpp
ManageCORScreen.cpp
ManageGeomScreen.cpp
ManageLightingScreen.cpp
ManageMeasureScreen.cpp
ManageTextureScreen.cpp
ManageViewScreen.cpp
MassPropScreen.cpp
MaterialEditScreen.cpp
MenuItem.cpp
MeshScreen.cpp
ModeEditorScreen.cpp
MultTransScreen.cpp
NerfManageGeomScreen.cpp
NGonMeshScreen.cpp
ParasiteDragScreen.cpp
ParmDebugScreen.cpp
ParmLinkScreen.cpp
ParmScreen.cpp
PickSetScreen.cpp
PodScreen.cpp
PreferencesScreen.cpp
ProjectionScreen.cpp
PropScreen.cpp
PSliceScreen.cpp
PtCloudScreen.cpp
ResultsViewer.cpp
RoutingScreen.cpp
ScreenBase.cpp
ScreenMgr.cpp
ScreenshotScreen.cpp
SelectFileScreen.cpp
SetEditorScreen.cpp
SnapToScreen.cpp
SpreadSheetWidget.cpp
StackScreen.cpp
StructScreen.cpp
StructAssemblyScreen.cpp
STEPOptionsScreen.cpp
STEPStructureOptionsScreen.cpp
STLOptionsScreen.cpp
SubGLWindow.cpp
SurfaceIntersectionScreen.cpp
SVGOptionsScreen.cpp
TreeColumnWidget.cpp
TreeIconWidget.cpp
CustomGeomExportScreen.cpp
UserParmScreen.cpp
VarPresetEditorScreen.cpp
VehNotesScreen.cpp
VSPAEROPlotScreen.cpp
VSPAEROScreen.cpp
VSPWindow.cpp
WaveDragScreen.cpp
WingScreen.cpp
WireScreen.cpp
AdvLinkScreen.h
AdvLinkVarRenameScreen.h
AeroStructScreen.h
AirfoilExportScreen.h
AttributeEditor.h
AttributeScreen.h
AuxiliaryGeomScreen.h
BlankScreen.h
BEMOptionsScreen.h
BORScreen.h
CfdMeshScreen.h
ClippingScreen.h
CompGeomScreen.h
ConformalScreen.h
CurveEditScreen.h
CustomScreen.h
DegenGeomScreen.h
DesignVarScreen.h
DXFOptionsScreen.h
ExportScreen.h
EllipsoidScreen.h
FeaPartEditScreen.h
FitModelScreen.h
FuselageScreen.h
GearScreen.h
GeometryAnalysisScreen.h
GroupLayout.h
GuiDevice.h
HingeScreen.h
HumanGeomScreen.h
IGESOptionsScreen.h
IGESStructureOptionsScreen.h
ImportScreen.h
MainGLWindow.h
MainGLWindow.h
MainVSPScreen.h
ManageBackgroundScreen.h
ManageBackground3DScreen.h
ManageCORScreen.h
ManageGeomScreen.h
ManageLightingScreen.h
ManageMeasureScreen.h
ManageTextureScreen.h
ManageViewScreen.h
MassPropScreen.h
MaterialEditScreen.h
MenuItem.h
MeshScreen.h
ModeEditorScreen.h
MultTransScreen.h
NerfManageGeomScreen.h
NGonMeshScreen.h
ParasiteDragScreen.h
ParmDebugScreen.h
ParmLinkScreen.h
ParmScreen.h
PickSetScreen.h
PodScreen.h
PreferencesScreen.h
ProjectionScreen.h
PropScreen.h
PSliceScreen.h
PtCloudScreen.h
ResultsViewer.h
RoutingScreen.h
ScreenBase.h
ScreenMgr.h
ScreenshotScreen.h
SelectFileScreen.h
SetEditorScreen.h
SnapToScreen.h
SpreadSheetWidget.h
StackScreen.h
StructScreen.h
StructAssemblyScreen.h
STEPOptionsScreen.h
STEPStructureOptionsScreen.h
STLOptionsScreen.h
SubGLWindow.h
SurfaceIntersectionScreen.h
SVGOptionsScreen.h
TreeColumnWidget.h
TreeIconWidget.h
CustomGeomExportScreen.h
UserParmScreen.h
VarPresetEditorScreen.h
VehNotesScreen.h
VSPAEROPlotScreen.h
VSPAEROScreen.h
VSPWindow.h
WaveDragScreen.h
WingScreen.h
WireScreen.h
)

IF( ${NOREGEXP} )
  TARGET_COMPILE_DEFINITIONS(gui_and_draw PRIVATE
    NOREGEXP=${NOREGEXP}
  )
ENDIF()

ADD_DEPENDENCIES(gui_and_draw
geom_core
vsp_graphic
util
)
