CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

IF(WIN32)
ELSE()
  ADD_DEFINITIONS( -DMYTIME )
ENDIF()

ADD_EXECUTABLE(vsploads
ADBSlicer.C
EngineFace.C
RotorDisk.C
adb2loads.C
binaryio.C
interp.C
quat.C
search.C
utils.C
ADBSlicer.H
ControlSurface.H
EngineFace.H
PropElement.H
RotorDisk.H
binaryio.H
interp.H
quat.H
surfIDs.H
utils.H
)

TARGET_LINK_LIBRARIES(vsploads
)

INSTALL( TARGETS vsploads RUNTIME DESTINATION . )
