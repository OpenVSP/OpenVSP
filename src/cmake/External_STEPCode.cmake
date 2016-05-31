
# Workaround for SC_BUILD_SHARED_LIBS flag.
# Would prefer to set to OFF.  However, it won't build on Mac with
# flag set to OFF -- and it won't build on MSVC with it set to ON.
IF( WIN32 )
    SET( SC_SHARED OFF )
ELSE()
    SET( SC_SHARED ON )
ENDIF()

ExternalProject_Add( STEPCODE
	URL ${CMAKE_CURRENT_SOURCE_DIR}/stepcode-28350d91294b.zip
	CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
		-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
		-DSC_BUILD_TYPE=Debug
		-DSC_BUILD_SCHEMAS=ap203/ap203.exp
		-DSC_BUILD_STATIC_LIBS=ON
		-DSC_BUILD_SHARED_LIBS=${SC_SHARED}
		-DSC_PYTHON_GENERATOR=OFF
		-DSC_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)
ExternalProject_Get_Property( STEPCODE SOURCE_DIR )
ExternalProject_Get_Property( STEPCODE BINARY_DIR )
ExternalProject_Get_Property( STEPCODE INSTALL_DIR )

IF( NOT WIN32 )
	SET( STEPCODE_INSTALL_DIR ${SOURCE_DIR}/../sc-install )
ELSE()
	SET( STEPCODE_INSTALL_DIR ${INSTALL_DIR} )
ENDIF()

SET( STEPCODE_BINARY_DIR ${BINARY_DIR} )

# SC CMake does not honor -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
# Consequently, force Debug so it installs in ../sc-install directory
# instead of /usr/local/lib.
#
# SC's own programs fail to build with -DSC_BUILD_SHARED_LIBS=OFF