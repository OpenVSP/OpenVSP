CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(GEOM_API_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to geom_api headers")

INCLUDE_DIRECTORIES(
    ${NANOFLANN_INCLUDE_DIR}
    ${UTIL_INCLUDE_DIR}
    ${GEOM_CORE_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${XMLVSP_INCLUDE_DIR}
    ${FLTK_INCLUDE_DIR}
    ${SCREENS_INCLUDE_DIR}
    ${GUI_AND_DRAW_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${CodeEli_INCLUDE_DIRS}
    ${STEPCODE_INCLUDE_DIR}
    ${LIBIGES_INCLUDE_DIR}
    ${CFD_MESH_INCLUDE_DIR}
    ${GEOM_API_INCLUDE_DIR}
    ${CARTESIAN_INCLUDE_DIR}
)

ADD_LIBRARY( geom_api
APIErrorMgr.cpp
GuiInterface.cpp
VSP_Geom_API.cpp
APIDefines.h
APIErrorMgr.h
GuiInterface.h
VSP_Geom_API.h
)

ADD_DEPENDENCIES( geom_api
util
)

IF( NOT VSP_NO_GRAPHICS )
    ADD_LIBRARY( geom_api_g
    APIErrorMgr.cpp
    GuiInterface.cpp
    VSP_Geom_API.cpp
    APIDefines.h
    APIErrorMgr.h
    GuiInterface.h
    VSP_Geom_API.h
    )

    ADD_DEPENDENCIES( geom_api_g
    util
    )

    set_target_properties( geom_api_g PROPERTIES COMPILE_FLAGS "-DVSP_USE_FLTK=1")

    ADD_LIBRARY( gui_interface_g
    GuiInterface.cpp
    GuiInterface.h
    )

    ADD_DEPENDENCIES( gui_interface_g
    util
    )

    set_target_properties( gui_interface_g PROPERTIES COMPILE_FLAGS "-DVSP_USE_FLTK=1")
ENDIF()
