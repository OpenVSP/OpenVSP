CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

IF( NOT VSP_NO_GRAPHICS )

  # ADD_FLTK_FILE is a function that roughly replaces FLTK_WRAP_UI.
  # ADD_FLTK_FILE only works one file at a time -- and it is passed the base file name (without extension).
  # Also, ADD_FLTK_FILE passes causes fltk to generate source files with the .C and .H extension instead of
  # .cxx and .h.
  function( ADD_FLTK_FILE prjname basename)
    add_custom_command(
      OUTPUT ${basename}.C ${basename}.H
      MAIN_DEPENDENCY ${basename}.fl
      COMMAND ${FLTK_FLUID_EXECUTABLE} -o .C -h .H -c ${CMAKE_CURRENT_SOURCE_DIR}/${basename}.fl
    )
    set( ${prjname}_FLTK_UI_SRCS
      ${${prjname}_FLTK_UI_SRCS} ${basename}.C
      PARENT_SCOPE
    )
  endfunction()

  ADD_FLTK_FILE( viewerscreens ContourLevel )
  ADD_FLTK_FILE( viewerscreens CuttingPlane )
  ADD_FLTK_FILE( viewerscreens viewerUI )
  # These three lines are roughly equivalent to:
  #  FLTK_WRAP_UI( viewerscreens
  #  ContourLevel.fl
  #  CuttingPlane.fl
  #  viewerUI.fl
  #  )

  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${STB_INCLUDE_DIR}
    ${FLTK_INCLUDE_DIR}
  )

  ADD_LIBRARY(viewerscreens ${viewerscreens_FLTK_UI_SRCS} )


  ADD_EXECUTABLE(vspviewer
    EngineFace.C
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
    EngineFace.H
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

  TARGET_COMPILE_DEFINITIONS( vspviewer PRIVATE -DDO_GRAPHICS -DOCTREE_DO_GRAPHICS )

  TARGET_LINK_LIBRARIES(vspviewer
    viewerscreens
    stb_image
    ${FLTK_LIBRARIES}
    ${OPENGL_LIBRARIES}
  )

  INSTALL( TARGETS vspviewer RUNTIME DESTINATION . )

ENDIF()
