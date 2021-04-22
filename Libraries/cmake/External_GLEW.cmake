# An external project for GLEW, The OpenGL Extension Wrangler Library
# http://glew.sourceforge.net/

set( glew_version "2.1.0" )

set( glew_file glew-${glew_version}.tgz )

# Only thing that gets configured is the version number.
configure_file(
	"${CMAKE_SOURCE_DIR}/cmake/glew.CMakeLists.txt.in"
	"${CMAKE_CURRENT_BINARY_DIR}/glew.CMakeLists.txt"
	@ONLY
)

ExternalProject_Add( GLEW
	URL ${CMAKE_SOURCE_DIR}/${glew_file}
	PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_CURRENT_BINARY_DIR}/glew.CMakeLists.txt"
		"<SOURCE_DIR>/CMakeLists.txt"
    CMAKE_ARGS
		-DBUILD_SHARED_LIBS:BOOL=OFF
		-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
		-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
		-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)
ExternalProject_Get_Property( GLEW INSTALL_DIR )
SET( GLEW_INSTALL_DIR ${INSTALL_DIR} )