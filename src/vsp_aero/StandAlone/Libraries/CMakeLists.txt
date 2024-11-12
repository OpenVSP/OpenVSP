CMAKE_MINIMUM_REQUIRED( VERSION 3.24 )

PROJECT( VSPAERO_LIBRARIES )

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/../cmake ${CMAKE_MODULE_PATH} )

INCLUDE( SetupC++11 )

INCLUDE( ExternalProject )

IF( NOT VSP_NO_GRAPHICS )
	IF( NOT VSP_USE_SYSTEM_FLTK )
		INCLUDE( External_FLTK )
	ENDIF()
ENDIF()

# Set targeted variables in file to be read by VSP's CMake
configure_file(
   ${CMAKE_SOURCE_DIR}/cmake/VSPAERO_Libraries_Config.cmake.in
   ${CMAKE_BINARY_DIR}/VSPAERO_Libraries_Config.cmake
   @ONLY
)

