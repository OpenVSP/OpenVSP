# - Find the STEPCode libraries
#
# By default, uses the static libraries built by
# Libraries/cmake/External_STEPCode.cmake in ${STEPCODE_INSTALL_DIR}.
#
# With VSP_USE_SYSTEM_STEPCODE, searches for an installed (typically shared,
# distribution packaged) STEPCode built with the AP203 schema.

set( STEPCODE_LIB_NAMES sdai_ap203 stepeditor stepcore stepdai steputils )
set( STEPCODE_REQUIRED_VARS STEPCODE_ROOT_INCLUDE_DIR )

IF( NOT VSP_USE_SYSTEM_STEPCODE )

    # Static libraries are named <lib>-static.  Order matters for single-pass
    # linkers: dependents before their dependencies.
    set( STEPCODE_LIBRARIES )
    foreach( SC_LIB ${STEPCODE_LIB_NAMES} )
        list( APPEND STEPCODE_LIBRARIES
            ${STEPCODE_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${SC_LIB}-static${CMAKE_STATIC_LIBRARY_SUFFIX} )
    endforeach()

    set( STEPCODE_ROOT_INCLUDE_DIR ${STEPCODE_INSTALL_DIR}/include )

    # Code including STEPCode headers must define SC_STATIC when linking the
    # static libraries, otherwise the headers declare everything
    # __declspec(dllimport) on Windows.
    add_compile_definitions( SC_STATIC )

ELSE()

    find_path( STEPCODE_ROOT_INCLUDE_DIR stepcode/cleditor/STEPfile.h
        HINTS ${STEPCODE_INSTALL_DIR}/include )

    set( STEPCODE_LIBRARIES )
    foreach( SC_LIB ${STEPCODE_LIB_NAMES} )
        find_library( STEPCODE_${SC_LIB}_LIBRARY NAMES ${SC_LIB}
            HINTS ${STEPCODE_INSTALL_DIR}/lib ${STEPCODE_INSTALL_DIR}/lib64 )
        list( APPEND STEPCODE_LIBRARIES ${STEPCODE_${SC_LIB}_LIBRARY} )
        list( APPEND STEPCODE_REQUIRED_VARS STEPCODE_${SC_LIB}_LIBRARY )
    endforeach()

ENDIF()

IF( WIN32 )
    list( APPEND STEPCODE_LIBRARIES shlwapi )
ENDIF()

set( STEPCODE_INCLUDE_DIR
${STEPCODE_ROOT_INCLUDE_DIR}/stepcode
${STEPCODE_ROOT_INCLUDE_DIR}/stepcode/clstepcore
${STEPCODE_ROOT_INCLUDE_DIR}/stepcode/cldai
${STEPCODE_ROOT_INCLUDE_DIR}/stepcode/clutils
${STEPCODE_ROOT_INCLUDE_DIR}/stepcode/cleditor
${STEPCODE_ROOT_INCLUDE_DIR}/schemas/sdai_ap203 )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( STEPCODE REQUIRED_VARS ${STEPCODE_REQUIRED_VARS} )

# MESSAGE( STATUS "STEPCODE_INCLUDE_DIR: " ${STEPCODE_INCLUDE_DIR} )
# MESSAGE( STATUS "STEPCODE_LIBRARIES: " ${STEPCODE_LIBRARIES} )
