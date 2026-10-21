# - Try to find the gl2ps library
#

find_path( GL2PS_INCLUDE_DIR gl2ps.h
    HINTS
        ${GL2PS_INSTALL_DIR}/include
        ${GL2PS_INSTALL_DIR}
        ${CMAKE_PREFIX_PATH}/include
        ${CMAKE_PREFIX_PATH}
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
        ${CMAKE_INSTALL_PREFIX}/include
        DOC "The directory where gl2ps.h resides")

find_library( GL2PS_LIBRARIES gl2ps
    HINTS
        ${GL2PS_INSTALL_DIR}/lib
        ${GL2PS_INSTALL_DIR}
        ${CMAKE_PREFIX_PATH}/lib
        ${CMAKE_PREFIX_PATH}
        /usr/lib64
        /usr/lib
        /usr/local/lib64
        /usr/local/lib
        /sw/lib
        /opt/local/lib
        ${LIB_INSTALL_DIR}
        DOC "The gl2ps Library")

MESSAGE( STATUS "GL2PS_INCLUDE_DIR: " ${GL2PS_INCLUDE_DIR} )
MESSAGE( STATUS "GL2PS_LIBRARIES: " ${GL2PS_LIBRARIES} )

# OpenVSP uses additions in the gl2ps of Libraries/gl2ps-*.zip, which released gl2ps lacks.
if ( GL2PS_INCLUDE_DIR AND EXISTS ${GL2PS_INCLUDE_DIR}/gl2ps.h )
    file( READ ${GL2PS_INCLUDE_DIR}/gl2ps.h GL2PS_HEADER_TEXT )
    string( FIND "${GL2PS_HEADER_TEXT}" "gl2psSetPageScale" GL2PS_PAGE_SCALE_AT )
    if ( GL2PS_PAGE_SCALE_AT EQUAL -1 )
        MESSAGE( STATUS "gl2ps at ${GL2PS_INCLUDE_DIR} lacks gl2psSetPageScale; build the one in Libraries instead" )
        SET( GL2PS_INCLUDE_DIR GL2PS_INCLUDE_DIR-NOTFOUND )
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GL2PS DEFAULT_MSG GL2PS_INCLUDE_DIR GL2PS_LIBRARIES )
