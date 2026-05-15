# - Try to find the STEPCode library
#
#   TODO: Use find_path and find_library

IF(NOT WIN32)
    set( STEPCODE_LIBRARIES
    ${STEPCODE_INSTALL_DIR}/lib/libsdai_ap203-static.a
    ${STEPCODE_INSTALL_DIR}/lib/libstepeditor-static.a
    ${STEPCODE_INSTALL_DIR}/lib/libstepcore-static.a
    ${STEPCODE_INSTALL_DIR}/lib/libstepdai-static.a
    ${STEPCODE_INSTALL_DIR}/lib/libsteputils-static.a )
ELSE()
    # Necessary for static libs in STEPCode
    add_compile_definitions(SC_STATIC)
    set( STEPCODE_LIBRARIES
    ${STEPCODE_INSTALL_DIR}/lib/express-static.lib
    ${STEPCODE_INSTALL_DIR}/lib/libexppp-static.lib
    ${STEPCODE_INSTALL_DIR}/lib/sdai_ap203-static.lib
    ${STEPCODE_INSTALL_DIR}/lib/stepeditor-static.lib
    ${STEPCODE_INSTALL_DIR}/lib/stepcore-static.lib
    ${STEPCODE_INSTALL_DIR}/lib/stepdai-static.lib
    ${STEPCODE_INSTALL_DIR}/lib/steputils-static.lib
    # ${STEPCODE_INSTALL_DIR}/lib/libbase.lib removed in 0.8.2
    shlwapi.lib )
ENDIF()

#${STEPCODE_INSTALL_DIR}/lib/libbase-static.a

set( STEPCODE_INCLUDE_DIR
${STEPCODE_INSTALL_DIR}/include/stepcode
${STEPCODE_INSTALL_DIR}/include/stepcode/base
${STEPCODE_INSTALL_DIR}/include/stepcode/clstepcore
${STEPCODE_INSTALL_DIR}/include/stepcode/cldai
${STEPCODE_INSTALL_DIR}/include/stepcode/clutils
${STEPCODE_INSTALL_DIR}/include/stepcode/cleditor
${STEPCODE_INSTALL_DIR}/include/schemas/sdai_ap203 )

# MESSAGE( STATUS "STEPCODE_INCLUDE_DIR: " ${STEPCODE_INCLUDE_DIR} )
# MESSAGE( STATUS "STEPCODE_LIBRARIES: " ${STEPCODE_LIBRARIES} )
