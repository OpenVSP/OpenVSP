CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT (VSP)

# Use this to see the command that CMake is using to build code
#SET(CMAKE_VERBOSE_MAKEFILE   ON  CACHE BOOL "" FORCE)

# add the OpenVSP specific cmake files to module path
set(CMAKE_MODULE_PATH ${VSP_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

INCLUDE( VSP_Version )

INCLUDE( SetupC++11 )

IF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "amd64")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
ENDIF()

IF( VSP_LIBRARY_PATH )
	INCLUDE( ${VSP_LIBRARY_PATH}/VSP_Libraries_Config.cmake )
ENDIF( VSP_LIBRARY_PATH )

IF(WIN32)
    ADD_DEFINITIONS(/D _CRT_SECURE_NO_WARNINGS /D "NOMINMAX" /MP )
ENDIF(WIN32)

IF(WIN32)
  SET( WINSOCK_LIBRARIES
    wsock32
    ws2_32
  )
ELSE()
  SET( WINSOCK_LIBRARIES )
ENDIF()

INCLUDE( Today )
TODAY( VSPDATE )

FIND_PACKAGE( Doxygen )
IF( DOXYGEN_FOUND )
    CONFIGURE_FILE(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY
    )
    ADD_CUSTOM_TARGET( doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
ENDIF( DOXYGEN_FOUND )


CONFIGURE_FILE(
      ${VSP_SOURCE_DIR}/vsp/main.h.in
      ${VSP_BINARY_DIR}/main.h
)
INCLUDE_DIRECTORIES( ${VSP_BINARY_DIR} )

ADD_SUBDIRECTORY( external )
ADD_SUBDIRECTORY( external/angelscript/sdk )

# INCLUDE( IdentifyLibs )  # Debugging to confirm which libraries are set.

IF( NOT VSP_NO_GRAPHICS )
	SET ( GUI_AND_DRAW_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/gui_and_draw )
	ADD_SUBDIRECTORY( vsp_graphic )
	ADD_SUBDIRECTORY( fltk_screens )
ENDIF()

SET(GEOM_API_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/geom_api)
ADD_SUBDIRECTORY( util )
ADD_SUBDIRECTORY( xmlvsp )
ADD_SUBDIRECTORY( geom_core )
ADD_SUBDIRECTORY( cfd_mesh )

IF( NOT VSP_NO_GRAPHICS )
	ADD_SUBDIRECTORY( gui_and_draw )
ENDIF()

ADD_SUBDIRECTORY( geom_api )
ADD_SUBDIRECTORY( vsp )
ADD_SUBDIRECTORY( python_api )

ADD_SUBDIRECTORY( vsp_aero )


# TODO consider using CPackDMG for an even better macOS install experience
# https://cmake.org/cmake/help/v3.5/module/CPackDMG.html
# https://gitlab.kitware.com/cmake/cmake/blob/v3.6.2/Packaging/CMakeDMGSetup.scpt

IF( APPLE )
  add_custom_target(BuildMacApp ALL)
  set( APP_NAME "VSP")
  set( APP_VERSION "${VSPVER_MAJOR}.${VSPVER_MINOR}.${VSPVER_PATCH}")
  set( ICON_NAME "vsp.icns" )
  set( ICON_PATH "${PROJECT_SOURCE_DIR}/${ICON_NAME}" )
  set( APP_PATH "${CMAKE_BINARY_DIR}/${APP_NAME}.app/Contents")
  set( APP_EXC_PATH "${APP_PATH}/MacOS/")

  add_custom_command(TARGET BuildMacApp
                    PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory
                    "${APP_PATH}/MacOS/")
  add_custom_command(TARGET BuildMacApp
                    PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory
                    "${APP_PATH}/Resources/")

  add_custom_command(TARGET BuildMacApp
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vsp>
                     "${APP_PATH}/MacOS/")
  add_custom_command(TARGET BuildMacApp
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vspaero>
                     "${APP_PATH}/MacOS/")
  add_custom_command(TARGET BuildMacApp
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vspscript>
                     "${APP_PATH}/MacOS/")
  add_custom_command(TARGET BuildMacApp
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vspslicer>
                     "${APP_PATH}/MacOS/")
  add_custom_command(TARGET BuildMacApp
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vspviewer>
                     "${APP_PATH}/MacOS/")


  SET( MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
  SET( MACOSX_BUNDLE_GUI_IDENTIFIER "org.openvsp.vsp")
  SET( MACOSX_BUNDLE_BUNDLE_NAME ${APP_NAME})
  SET( MACOSX_BUNDLE_EXECUTABLE_NAME "vsp")
  SET( MACOSX_BUNDLE_PRODUCT_NAME ${APP_NAME})
  SET( MACOSX_BUNDLE_INFO_STRING "${APP_NAME} ${APP_VERSION}")
  SET( MACOSX_BUNDLE_LONG_VERSION_STRING ${APP_VERSION})
  SET( MACOSX_BUNDLE_SHORT_VERSION_STRING ${APP_VERSION})
  SET( MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VERSION})
  # Using a custom plist template file is required because the default
  # cmake template doesn't have the NSPrincipalClass key which is needed
  # for making the app retina-compatible. This bug should be fixed upstream.
  SET( MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/MacOSXBundleInfo-custom.plist.in")
  configure_file(${MACOSX_BUNDLE_INFO_PLIST}
                 "${APP_PATH}/Info.plist")

  add_custom_command(TARGET BuildMacApp
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                   "${PROJECT_SOURCE_DIR}/${MACOSX_BUNDLE_ICON_FILE}"
                   "${APP_PATH}/Resources/")
ENDIF( APPLE )


SET( CPACK_GENERATOR ZIP )
SET( CPACK_PACKAGE_NAME OpenVSP )
SET( CPACK_PACKAGE_VERSION_MAJOR ${VSPVER_MAJOR} )
SET( CPACK_PACKAGE_VERSION_MINOR ${VSPVER_MINOR} )
SET( CPACK_PACKAGE_VERSION_PATCH ${VSPVER_PATCH} )

IF( APPLE )
  SET( CPACK_SYSTEM_NAME MacOS )
  # Need to set the right permissions on all executables within the MacOS
  # directory, otherwise the app won't run
  INSTALL(DIRECTORY "${CMAKE_BINARY_DIR}/${APP_NAME}.app/"
          DESTINATION "${APP_NAME}.app"
          PATTERN "MacOS/*"
          PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
          )
ENDIF( APPLE )

INCLUDE( CPack )


INSTALL( DIRECTORY ../examples/airfoil/ DESTINATION airfoil )
INSTALL( DIRECTORY ../examples/CustomScripts/ DESTINATION CustomScripts )
INSTALL( DIRECTORY ../examples/matlab/ DESTINATION matlab )
INSTALL( DIRECTORY ../examples/scripts/ DESTINATION scripts )
INSTALL( DIRECTORY ../examples/textures/ DESTINATION textures )
INSTALL( FILES ../LICENSE DESTINATION . )
INSTALL( FILES ../README.md DESTINATION . )
