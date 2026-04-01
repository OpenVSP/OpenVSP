CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

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

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  TARGET_COMPILE_OPTIONS( vsploads PUBLIC -Wno-deprecated-declarations)
endif()

INSTALL( TARGETS vsploads RUNTIME DESTINATION . )


IF( SWIG_FOUND AND PYTHONLIBS_FOUND )
  INSTALL( TARGETS vsploads RUNTIME DESTINATION python/openvsp/openvsp )

  add_custom_command(TARGET vsploads POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE:vsploads>
          ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/$<TARGET_FILE_NAME:vsploads>)
ENDIF( SWIG_FOUND AND PYTHONLIBS_FOUND )


get_property( is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG )

if ( is_multi_config )
        SET( conf_suffix "$<CONFIG>/" )
endif()

add_custom_command(TARGET vsploads POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:vsploads>
        ${CMAKE_BINARY_DIR}/src/vsp/${conf_suffix}$<TARGET_FILE_NAME:vsploads> )
