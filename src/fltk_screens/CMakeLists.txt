CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE_DIRECTORIES( ${FLTK_INCLUDE_DIR} ${GUI_AND_DRAW_INCLUDE_DIR} )

SET(SCREENS_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Path to FLTK generated *.h files")

FLTK_WRAP_UI( screens
advLinkFlScreen.fl
testFlScreen.fl
aboutScreen.fl
awaveFlScreen.fl
backgroundScreen.fl
cfdMeshFlScreen.fl
compGeomFlScreen.fl
curveEditFlScreen.fl
degenGeomFlScreen.fl
exportFileScreen.fl
feaStructFlScreen.fl
geomBrowserScreen.fl
importFileScreen.fl
labelFlScreen.fl
lightingScreen.fl
mainScreen.fl
massPropFlScreen.fl
parmFlScreen.fl
parmLinkFlScreen.fl
pickSetFlScreen.fl
pSliceFlScreen.fl
selectFileFlScreen.fl
setEditorFlScreen.fl
sliceScreen.fl
structureBrowserScreen.fl
textureMgrFlScreen.fl
typeEditorFlScreen.fl
userParmFlScreen.fl
viewScreen.fl
)

ADD_LIBRARY(screens ${screens_FLTK_UI_SRCS} )
