CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE_DIRECTORIES( ${FLTK_INCLUDE_DIR} )

SET(SCREENS_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Path to FLTK generated *.h files")

FLTK_WRAP_UI( screens
advLinkFlScreen.fl
testFlScreen.fl
aboutScreen.fl
aeroRefScreen.fl
awaveFlScreen.fl
backgroundScreen.fl
cabinLayoutFLScreen.fl
cfdMeshFlScreen.fl
compGeomFlScreen.fl
curveEditFlScreen.fl
ductFlScreen.fl
engineFlScreen.fl
exportFileScreen.fl
extStoreFlScreen.fl
feaStructFlScreen.fl
geomBrowserScreen.fl
groupFlScreen.fl
havocFlScreen.fl
hrmFlScreen.fl
hwbFlScreen.fl
importFileScreen.fl
labelFlScreen.fl
lightingScreen.fl
mainScreen.fl
massPropFlScreen.fl
msWingFlScreen.fl
parmFlScreen.fl
parmLinkFlScreen.fl
pickSetFlScreen.fl
propFlScreen.fl
pSliceFlScreen.fl
scriptOutScreen.fl
scriptScreen.fl
selectFileFlScreen.fl
setEditorFlScreen.fl
sliceScreen.fl
structureBrowserScreen.fl
textureMgrFlScreen.fl
typeEditorFlScreen.fl
userParmFlScreen.fl
viewScreen.fl
wingFlScreen.fl
xsecGeomFlScreen.fl
)

ADD_LIBRARY(screens ${screens_FLTK_UI_SRCS} )
