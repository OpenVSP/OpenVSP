CMAKE_MINIMUM_REQUIRED(VERSION 1.4)

FIND_PACKAGE(FLTK REQUIRED)

INCLUDE_DIRECTORIES( ${FLTK_INCLUDE_DIR} )

SET(SCREENS_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Path to FLTK generated *.h files")

FLTK_WRAP_UI( screens
aboutScreen.fl
aeroRefScreen.fl
awaveScreen.fl
backgroundScreen.fl
blankFlScreen.fl
cabinLayoutFLScreen.fl
cfdMeshFlScreen.fl
compGeomScreen.fl
curveEditFlScreen.fl
ductFlScreen.fl
engineFlScreen.fl
exportFileScreen.fl
extStoreFlScreen.fl
feaStructFlScreen.fl
fuseFlScreen.fl
fuselageFlScreen.fl
geomBrowserScreen.fl
groupFlScreen.fl
havocFlScreen.fl
hrmFlScreen.fl
hwbFlScreen.fl
importFileScreen.fl
labelFlScreen.fl
lightingScreen.fl
mainScreen.fl
massPropScreen.fl
meshFlScreen.fl
msWingFlScreen.fl
parmFlScreen.fl
parmLinkFlScreen.fl
parmPickerFlScreen.fl
podScreen.fl
propFlScreen.fl
scriptOutScreen.fl
scriptScreen.fl
selectFileFlScreen.fl
sliceScreen.fl
structureBrowserScreen.fl
textureMgrFlScreen.fl
viewScreen.fl
vorlaxOutScreen.fl
vorviewFlScreen.fl
wingFlScreen.fl
xsecGeomFlScreen.fl
)

ADD_LIBRARY(screens ${screens_FLTK_UI_SRCS} )
