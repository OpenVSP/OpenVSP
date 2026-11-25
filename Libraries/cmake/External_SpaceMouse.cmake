# libspnavdev, which reads six-axis devices such as 3Dconnexion's SpaceMouse straight from USB
# with no vendor driver, and hidapi, which it reads them through.  libspnavdev is
# LGPL-3.0-or-later; hidapi is offered under GPL-3.0, a BSD licence or its own, see LICENSE.txt
# in its archive.  Both are built as static libraries.  libspnavdev has no install rules, so
# OpenVSP links it from its build tree.
# On Linux, hidapi reads devices through hidraw, which needs libudev.

ExternalProject_Add( HIDAPI
	URL ${CMAKE_CURRENT_SOURCE_DIR}/hidapi-0.15.0.tar.gz
	URL_HASH SHA256=c167d847278ffe15b676dd35c1a0b34a383a70cddf98407b6faa48e9157f0f76
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
	CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
		-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
		-DCMAKE_INSTALL_LIBDIR=lib
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON
		-DBUILD_SHARED_LIBS=OFF
		-DHIDAPI_BUILD_HIDTEST=OFF
		-DHIDAPI_WITH_LIBUSB=OFF
)
ExternalProject_Get_Property( HIDAPI SOURCE_DIR INSTALL_DIR )
SET( HIDAPI_SOURCE_DIR ${SOURCE_DIR} )
SET( HIDAPI_INSTALL_DIR ${INSTALL_DIR} )

ExternalProject_Add( SPNAVDEV
	URL ${CMAKE_CURRENT_SOURCE_DIR}/libspnavdev-883bfca53c28.zip
	URL_HASH SHA256=f5519c1dda6f848b2bbb72a1aa6b8185646098c52ad1f007eee659ea14af7c54
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
	DEPENDS HIDAPI
	CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
		-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON
		-DBUILD_SHARED_LIBS=OFF
	# The DLL runtime, CMake's default and so hidapi's and OpenVSP's; libspnavdev would otherwise
	# choose the static one.
	CMAKE_CACHE_ARGS -DCMAKE_PREFIX_PATH:STRING=${HIDAPI_INSTALL_DIR}
		-DCMAKE_MSVC_RUNTIME_LIBRARY:STRING=MultiThreaded$<$<CONFIG:Debug>:Debug>DLL
	INSTALL_COMMAND ""
)
ExternalProject_Get_Property( SPNAVDEV SOURCE_DIR BINARY_DIR )
SET( SPNAVDEV_SOURCE_DIR ${SOURCE_DIR} )
SET( SPNAVDEV_BINARY_DIR ${BINARY_DIR} )
