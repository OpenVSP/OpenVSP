CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

IF( NOT VSP_NO_GRAPHICS )

  FLTK_WRAP_UI( viewerscreens
  ContourLevel.fl
  CuttingPlane.fl
  viewerUI.fl
  )

  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${STB_INCLUDE_DIR}
    ${FLTK_INCLUDE_DIR}
  )

  ADD_LIBRARY(viewerscreens ${viewerscreens_FLTK_UI_SRCS} )


  ADD_EXECUTABLE(vspviewer
    RotorDisk.C
    TagList.C
    TagListGroup.C
    TagRegion.C
    VSP_Agglom.C
    VSP_Body.C
    VSP_DegenGeom.C
    VSP_Edge.C
    VSP_Grid.C
    VSP_Node.C
    VSP_Tris.C
    VSP_Wing.C
    binaryio.C
    FEM_Node.C
    Optimization_Node.C
    glf.C
    glviewer.C
    quat.C
    trackball.C
    utils.C
    vspaero_viewer.C
    CharSizes.H
    ControlSurface.H
    GL_VIEWER.forward.declaration.H
    PropElement.H
    RotorDisk.H
    TagList.H
    TagListGroup.H
    TagRegion.H
    VSP_Agglom.H
    VSP_Body.H
    VSP_DegenGeom.H
    VSP_Edge.H
    VSP_Grid.H
    VSP_Node.H
    VSP_Tris.H
    VSP_Wing.H
    binaryio.H
    FEM_Node.H
    Optimization_Node.H
    glf.H
    glviewer.H
    quat.H
    surfIDs.H
    trackball.H
    utils.H
  )

  set_target_properties( vspviewer PROPERTIES COMPILE_FLAGS "-DDO_GRAPHICS=1")

  TARGET_LINK_LIBRARIES(vspviewer
    viewerscreens
    stb_image
    ${FLTK_LIBRARIES}
    ${OPENGL_LIBRARIES}
  )

  INSTALL( TARGETS vspviewer RUNTIME DESTINATION . )

ENDIF()
