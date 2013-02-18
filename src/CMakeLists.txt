CMAKE_MINIMUM_REQUIRED(VERSION 1.4)

PROJECT (VSP)

SET( VSPVER_MAJOR 2 )
SET( VSPVER_MINOR 2 )
SET( VSPVER_PATCH 2 )

SET(CMAKE_PREFIX_PATH
      "C:/Program Files/FLTK"
      ${VSP_SOURCE_DIR}/libxml2-2.7.8.win32
      ${VSP_SOURCE_DIR}/jpeg-8c
      ${VSP_SOURCE_DIR}/jpeg-8c/Release
)

MACRO (TODAY RESULT)
    IF (WIN32)
        EXECUTE_PROCESS(COMMAND "cmd" " /C date /T" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\2/\\1/\\3" ${RESULT} ${${RESULT}})
    ELSEIF(UNIX)
        EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\2/\\1/\\3" ${RESULT} ${${RESULT}})
    ELSE (WIN32)
        MESSAGE(SEND_ERROR "date not implemented")
        SET(${RESULT} 000000)
    ENDIF (WIN32)
ENDMACRO (TODAY)

TODAY( VSPDATE )

CONFIGURE_FILE(
      ${VSP_SOURCE_DIR}/vsp/main.h.in
      ${VSP_BINARY_DIR}/main.h
)
INCLUDE_DIRECTORIES( ${VSP_BINARY_DIR} )

ADD_SUBDIRECTORY( util_code )
ADD_SUBDIRECTORY( nanoflann )
ADD_SUBDIRECTORY( triangle )
ADD_SUBDIRECTORY( load_jpg )
ADD_SUBDIRECTORY( xmlvsp )
ADD_SUBDIRECTORY( screens )
ADD_SUBDIRECTORY( sixseries )
ADD_SUBDIRECTORY( vsp )

SET( CPACK_GENERATOR ZIP )
SET( CPACK_PACKAGE_NAME OpenVSP )
SET( CPACK_PACKAGE_VERSION_MAJOR ${VSPVER_MAJOR} )
SET( CPACK_PACKAGE_VERSION_MINOR ${VSPVER_MINOR} )
SET( CPACK_PACKAGE_VERSION_PATCH ${VSPVER_PATCH} )

IF( APPLE )
     SET( CPACK_SYSTEM_NAME MacOS )
ENDIF( APPLE )

INCLUDE( CPack )

INSTALL( DIRECTORY ../examples/airfoil/ DESTINATION airfoil )
INSTALL( DIRECTORY ../examples/cabin/ DESTINATION cabin )
INSTALL( DIRECTORY ../examples/fonts/ DESTINATION fonts )
INSTALL( DIRECTORY ../examples/fuselage/ DESTINATION fuselage )
INSTALL( DIRECTORY ../examples/models/ DESTINATION models )
INSTALL( DIRECTORY ../examples/textures/ DESTINATION textures )
INSTALL( FILES ../examples/setup/VspPreferences.xml DESTINATION . )
INSTALL( FILES ../examples/models/BWB_Def.vsp DESTINATION Defaults )
INSTALL( FILES ../LICENSE DESTINATION . )
INSTALL( FILES ../README DESTINATION . )
