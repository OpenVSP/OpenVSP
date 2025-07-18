CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

# include the CTest framework
include(CTest)

INCLUDE_DIRECTORIES(
    ${CLIPPER2_INCLUDE_DIR}
    ${CodeEli_INCLUDE_DIRS}
    ${CPPTEST_INCLUDE_DIR}
    ${CPPTEST_INCLUDE_DIRS}
    ${DELABELLA_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${FLTK_INCLUDE_DIR}
    ${GEOM_API_INCLUDE_DIR}
    ${GEOM_CORE_INCLUDE_DIR}
    ${GUI_AND_DRAW_INCLUDE_DIR}
    ${LIBIGES_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${LOAD_JPG_INCLUDE_DIR}
    ${NANOFLANN_INCLUDE_DIR}
    ${STEPCODE_INCLUDE_DIR}
    ${TRIANGLE_INCLUDE_DIR}
    ${UTIL_INCLUDE_DIR}
    ${UTIL_API_INCLUDE_DIR}
    ${XMLVSP_INCLUDE_DIR}
    ${VSP_SOURCE_DIR}
 )

IF( NOT VSP_NO_GRAPHICS )

    ADD_EXECUTABLE(vsp
    common.cpp
    main.cpp
    common.h
    main.h.in
    )

    set_target_properties( vsp PROPERTIES COMPILE_FLAGS "-DVSP_USE_FLTK=1")

    TARGET_LINK_LIBRARIES(vsp
        ${VSP_G_LIBRARIES_CORE_FIRST}
    )

    INSTALL( TARGETS vsp RUNTIME DESTINATION . )

    ADD_EXECUTABLE(apitest_g
    apitest_main.cpp
    APITestSuite.cpp
    APITestSuiteMassProp.cpp
    APITestSuiteVSPAERO.cpp
    APITestSuiteParasiteDrag.cpp
    APITestSuiteCFDMesh.cpp
    ../vsp/main.h.in
    )

    set_target_properties( apitest_g PROPERTIES COMPILE_FLAGS "-DVSP_USE_FLTK=1")

    TARGET_LINK_LIBRARIES(apitest_g
        ${VSP_G_LIBRARIES_API_FIRST}
    )

#    INSTALL( TARGETS apitest_g RUNTIME DESTINATION . )

ENDIF()

ADD_EXECUTABLE(apitest
apitest_main.cpp
APITestSuite.cpp
APITestSuiteMassProp.cpp
APITestSuiteVSPAERO.cpp
APITestSuiteParasiteDrag.cpp
APITestSuiteCFDMesh.cpp
../vsp/main.h.in
)

TARGET_LINK_LIBRARIES(apitest
    ${VSP_LIBRARIES_API_FIRST}
)

IF( VSP_INSTALL_API_TEST )
    INSTALL( TARGETS apitest RUNTIME DESTINATION . )
ENDIF()

ADD_EXECUTABLE(vspscript
common.cpp
scriptonly_main.cpp
common.h
../vsp/main.h.in
)

TARGET_LINK_LIBRARIES(vspscript
    ${VSP_LIBRARIES_CORE_FIRST}
)

INSTALL( TARGETS vspscript RUNTIME DESTINATION . )
