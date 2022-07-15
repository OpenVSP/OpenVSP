# - Try to find the AngelScript library
#

find_path( ANGELSCRIPT_INCLUDE_DIR angelscript.h
    HINTS
        ${ANGELSCRIPT_INSTALL_DIR}/include
        ${ANGELSCRIPT_INSTALL_DIR}
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
        ${CMAKE_INSTALL_PREFIX}/include
        DOC "The directory where angelscript.h resides")

find_library( ANGELSCRIPT_LIBRARIES angelscript
    HINTS
        ${ANGELSCRIPT_INSTALL_DIR}/lib
        ${ANGELSCRIPT_INSTALL_DIR}
        /usr/lib64
        /usr/lib
        /usr/local/lib64
        /usr/local/lib
        /sw/lib
        /opt/local/lib
        ${LIB_INSTALL_DIR}
        DOC "The AngelScript Library")


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ANGELSCRIPT DEFAULT_MSG ANGELSCRIPT_INCLUDE_DIR ANGELSCRIPT_LIBRARIES )

