# - Try to find the Triangle library
#

find_path( TRIANGLE_INCLUDE_DIR triangle.h
    HINTS
        ${TRIANGLE_INSTALL_DIR}/include
        ${TRIANGLE_INSTALL_DIR}
        ${CMAKE_PREFIX_PATH}/include
        ${CMAKE_PREFIX_PATH}
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
        ${CMAKE_INSTALL_PREFIX}/include
        DOC "The directory where triangle.h resides")

find_path( TRIANGLE_API_INCLUDE_DIR triangle_api.h
    HINTS
        ${TRIANGLE_INSTALL_DIR}/include
        ${TRIANGLE_INSTALL_DIR}
        ${CMAKE_PREFIX_PATH}/include
        ${CMAKE_PREFIX_PATH}
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
        ${CMAKE_INSTALL_PREFIX}/include
        DOC "The directory where triangle_api.h resides")

find_library( TRIANGLE_LIBRARY triangle
    HINTS
        ${TRIANGLE_INSTALL_DIR}/lib
        ${TRIANGLE_INSTALL_DIR}
        ${CMAKE_PREFIX_PATH}/lib
        ${CMAKE_PREFIX_PATH}
        /usr/lib64
        /usr/lib
        /usr/local/lib64
        /usr/local/lib
        /sw/lib
        /opt/local/lib
        ${LIB_INSTALL_DIR}
        DOC "The Triangle Library")

find_library( TRIANGLE_API_LIBRARY triangle-api
    HINTS
        ${TRIANGLE_INSTALL_DIR}/lib
        ${TRIANGLE_INSTALL_DIR}
        ${CMAKE_PREFIX_PATH}/lib
        ${CMAKE_PREFIX_PATH}
        /usr/lib64
        /usr/lib
        /usr/local/lib64
        /usr/local/lib
        /sw/lib
        /opt/local/lib
        ${LIB_INSTALL_DIR}
        DOC "The Triangle API Library")

set(TRIANGLE_INCLUDE_DIR ${TRIANGLE_INCLUDE_DIR} ${TRIANGLE_API_INCLUDE_DIR})
set(TRIANGLE_LIBRARIES ${TRIANGLE_API_LIBRARY} ${TRIANGLE_LIBRARY})

MESSAGE( STATUS "TRIANGLE_INCLUDE_DIR: " ${TRIANGLE_INCLUDE_DIR} )
MESSAGE( STATUS "TRIANGLE_LIBRARIES: " ${TRIANGLE_LIBRARIES} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TRIANGLE DEFAULT_MSG TRIANGLE_INCLUDE_DIR TRIANGLE_LIBRARIES )

