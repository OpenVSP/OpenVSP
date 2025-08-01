CMAKE_MINIMUM_REQUIRED( VERSION 3.24 )

PROJECT( VSP_LIBRARIES )

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/../src/cmake ${CMAKE_MODULE_PATH} )

INCLUDE( VSP_Version )

SET( CMAKE_CXX_STANDARD 17 )

INCLUDE( ExternalProject )

IF( NOT VSP_USE_SYSTEM_ANGELSCRIPT )
	INCLUDE( External_AngelScript )
ENDIF()

IF( NOT VSP_USE_SYSTEM_CLIPPER2 )
	INCLUDE( External_Clipper2 )
ENDIF()

IF( NOT VSP_USE_SYSTEM_CMINPACK )
	INCLUDE( External_CMinpack )
ENDIF()

IF( NOT VSP_USE_SYSTEM_CODEELI )
	INCLUDE( External_CodeEli )
ENDIF()

IF( NOT VSP_USE_SYSTEM_CPPTEST )
	INCLUDE( External_CppTest )
ENDIF()

IF( NOT VSP_USE_SYSTEM_DELABELLA )
	INCLUDE( External_delabella )
ENDIF()

IF( NOT VSP_USE_SYSTEM_EIGEN )
	INCLUDE( External_Eigen )
ENDIF()

IF( NOT VSP_USE_SYSTEM_LIBIGES )
	INCLUDE( External_libIGES )
ENDIF()

IF( NOT VSP_USE_SYSTEM_LIBXML2 )
	INCLUDE( External_LibXml2 )
ENDIF()

IF( NOT VSP_USE_SYSTEM_OPENABF )
	INCLUDE( External_OpenABF )
ENDIF()

IF( NOT VSP_USE_SYSTEM_PINOCCHIO )
	INCLUDE( External_pinocchio )
ENDIF()

IF( NOT VSP_USE_SYSTEM_STEPCODE )
    INCLUDE( External_STEPCode )
ENDIF()

IF( NOT VSP_USE_SYSTEM_TRIANGLE )
	INCLUDE( External_Triangle )
ENDIF()

IF( NOT VSP_NO_GRAPHICS )
	IF( NOT NOREGEXP )
		IF ( NOT VSP_USE_SYSTEM_EXPRPARSE )
			INCLUDE( External_exprparse )
		ENDIF()
	ENDIF()

	IF( NOT VSP_USE_SYSTEM_FLTK )
		INCLUDE( External_FLTK )
	ENDIF()

	IF( NOT VSP_USE_SYSTEM_GLEW )
		INCLUDE( External_GLEW )
	ENDIF()

	IF( NOT VSP_USE_SYSTEM_GLM )
		INCLUDE( External_GLM )
	ENDIF()
ENDIF()

# Set targeted variables in file to be read by VSP's CMake
configure_file(
   ${CMAKE_SOURCE_DIR}/cmake/VSP_Libraries_Config.cmake.in
   ${CMAKE_BINARY_DIR}/VSP_Libraries_Config.cmake
   @ONLY
)

