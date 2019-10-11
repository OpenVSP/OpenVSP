# - Try to find the STEPCode library
#
#   TODO: Use find_path and find_library

IF(NOT WIN32)
    set( STEPCODE_LIBRARIES
    ${STEPCODE_INSTALL_DIR}/lib/libsdai_ap203.a
    ${STEPCODE_INSTALL_DIR}/lib/libstepeditor.a
    ${STEPCODE_INSTALL_DIR}/lib/libstepcore.a
    ${STEPCODE_INSTALL_DIR}/lib/libstepdai.a
    ${STEPCODE_INSTALL_DIR}/lib/libsteputils.a
    ${STEPCODE_INSTALL_DIR}/lib/libbase.a )
ELSE()
    set( STEPCODE_LIBRARIES
    ${STEPCODE_INSTALL_DIR}/lib/libexpress.lib
    ${STEPCODE_INSTALL_DIR}/lib/libexppp.lib
    ${STEPCODE_INSTALL_DIR}/lib/libsdai_ap203.lib
    ${STEPCODE_INSTALL_DIR}/lib/libstepeditor.lib
    ${STEPCODE_INSTALL_DIR}/lib/libstepcore.lib
    ${STEPCODE_INSTALL_DIR}/lib/libstepdai.lib
    ${STEPCODE_INSTALL_DIR}/lib/libsteputils.lib
    ${STEPCODE_INSTALL_DIR}/lib/libbase.lib
    shlwapi.lib )
ENDIF()


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
