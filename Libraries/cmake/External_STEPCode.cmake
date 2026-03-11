
# Workaround for SC_BUILD_SHARED_LIBS flag.
# Would prefer to set to OFF.  However, it won't build on Mac with
# flag set to OFF -- and it won't build on MSVC with it set to ON.
IF( WIN32 )
    SET( SC_SHARED OFF )
ELSE()
    SET( SC_SHARED ON )
ENDIF()


SET(SC_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
SET(SC_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

IF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "amd64")
	SET(SC_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	SET(SC_CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
ENDIF()

ExternalProject_Add( STEPCODE
    URL ${CMAKE_CURRENT_SOURCE_DIR}/stepcode-v0.8.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    # Remove TESTABLE flag from sdai_ap203 static schema target - TESTABLE
    # suppresses the install rule (SC_Targets.cmake), preventing
    # libsdai_ap203-static.a from being installed. This should be fixed
    # upstream, remove this patch when bumping to the fix release.
    PATCH_COMMAND sed -i "s/LINK_LIBRARIES stepdai-static stepcore-static stepeditor-static steputils-static TESTABLE/LINK_LIBRARIES stepdai-static stepcore-static stepeditor-static steputils-static/" <SOURCE_DIR>/cmake/SC_CXX_schema_macros.cmake
    INSTALL_DIR ${CMAKE_BINARY_DIR}
    CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_CXX_FLAGS=${SC_CMAKE_CXX_FLAGS}
        -DCMAKE_C_FLAGS=${SC_CMAKE_C_FLAGS}
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DSC_BUILD_TYPE=Release
        -DSC_BUILD_SCHEMAS=ap203/ap203.exp
        -DBUILD_STATIC_LIBS=ON
        -DSC_BUILD_STATIC_LIBS=ON
        -DBUILD_SHARED_LIBS=${SC_SHARED}
        -DSC_BUILD_SHARED_LIBS=${SC_SHARED}
        -DSC_PYTHON_GENERATOR=OFF
        -DSC_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
)
ExternalProject_Get_Property( STEPCODE SOURCE_DIR )
ExternalProject_Get_Property( STEPCODE BINARY_DIR )
ExternalProject_Get_Property( STEPCODE INSTALL_DIR )


SET( STEPCODE_INSTALL_DIR ${INSTALL_DIR} )
SET( STEPCODE_BINARY_DIR ${BINARY_DIR} )
