CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

PROJECT( VSP_LIBRARIES )

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/../src/cmake ${CMAKE_MODULE_PATH} )

INCLUDE( VSP_Version )

INCLUDE( SetupC++11 )

INCLUDE( ExternalProject )

IF( NOT VSP_USE_SYSTEM_CPPTEST )
	INCLUDE( External_CppTest )
ENDIF()

IF( NOT VSP_USE_SYSTEM_LIBXML2 )
	INCLUDE( External_LibXml2 )
ENDIF()

IF( NOT VSP_USE_SYSTEM_EIGEN )
	INCLUDE( External_Eigen )
ENDIF()

IF( NOT VSP_USE_SYSTEM_CODEELI )
	INCLUDE( External_CodeEli )
ENDIF()

IF( NOT VSP_USE_SYSTEM_CMINPACK )
    INCLUDE( External_CMinpack )
ENDIF()

IF( NOT VSP_USE_SYSTEM_LIBIGES )
    INCLUDE( External_libIGES )
ENDIF()

IF( NOT VSP_NO_GRAPHICS )
	IF( NOT VSP_USE_SYSTEM_FLTK )
		INCLUDE( External_FLTK )
	ENDIF()

	IF( NOT VSP_USE_SYSTEM_GLM )
		INCLUDE( External_GLM )
	ENDIF()

	IF( NOT VSP_USE_SYSTEM_GLEW )
		INCLUDE( External_GLEW )
	ENDIF()
ENDIF()

INCLUDE( External_STEPCode )

# Set targeted variables in file to be read by VSP's CMake
configure_file(
   ${CMAKE_SOURCE_DIR}/cmake/VSP_Libraries_Config.cmake.in
   ${CMAKE_BINARY_DIR}/VSP_Libraries_Config.cmake
   @ONLY
)

