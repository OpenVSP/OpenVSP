CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

IF( NOT VSP_NO_GRAPHICS )

#   FLTK_WRAP_UI( demoscreens
#   demo.fl
#   )

  INCLUDE_DIRECTORIES(
    ${PINOCCHIO_INCLUDE_DIR}
    ${FLTK_INCLUDE_DIR}
  )

#  ADD_LIBRARY(demoscreens ${demoscreens_FLTK_UI_SRCS} )


  ADD_EXECUTABLE(demoui
          DemoUI.cpp
          MyWindow.cpp
          defmesh.cpp
          filter.cpp
          motion.cpp
          processor.cpp
          DisplayMesh.h
          MyWindow.h
          defmesh.h
          filter.h
          motion.h
          processor.h
          shared.h
  )

  TARGET_LINK_LIBRARIES(demoui
    pinocchio
    ${FLTK_LIBRARIES}
    ${OPENGL_LIBRARIES}
  )

#  INSTALL( TARGETS vspviewer RUNTIME DESTINATION . )

ENDIF()
