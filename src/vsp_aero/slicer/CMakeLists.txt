CMAKE_MINIMUM_REQUIRED(VERSION 3.1)



ADD_EXECUTABLE(vspslicer
ADBSlicer.C
RotorDisk.C
Slicer.C
binaryio.C
quat.C
utils.C
ADBSlicer.H
ControlSurface.H
PropElement.H
RotorDisk.H
binaryio.H
quat.H
surfIDs.H
utils.H
)

TARGET_LINK_LIBRARIES(vspslicer
)

INSTALL( TARGETS vspslicer RUNTIME DESTINATION . )
