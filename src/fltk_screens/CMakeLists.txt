CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE_DIRECTORIES( ${FLTK_INCLUDE_DIR} ${GUI_AND_DRAW_INCLUDE_DIR} )

SET(SCREENS_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Path to FLTK generated *.h files")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

FLTK_WRAP_UI( screens
aboutScreen.fl
awaveFlScreen.fl
curveEditFlScreen.fl
feaStructFlScreen.fl
labelFlScreen.fl
massPropFlScreen.fl
parmFlScreen.fl
pickSetFlScreen.fl
pSliceFlScreen.fl
selectFileFlScreen.fl
setEditorFlScreen.fl
sliceScreen.fl
textureMgrFlScreen.fl
typeEditorFlScreen.fl
)

ADD_LIBRARY(screens ${screens_FLTK_UI_SRCS} )
