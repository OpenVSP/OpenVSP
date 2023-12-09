CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

# include the CTest framework
include(CTest)

IF(SWIG_FOUND AND MATLAB_FOUND)

    MESSAGE(STATUS "MATLAB Found, MATLAB MEX will be compiled")
    MESSAGE(STATUS "    MATLAB_INCLUDE_DIRS: ${Matlab_INCLUDE_DIRS}")
    MESSAGE(STATUS "    MATLAB_LIBRARIES:    ${Matlab_LIBRARIES}")
    MESSAGE(STATUS "    MATLAB_MEX_LIBRARY:  ${Matlab_MEX_LIBRARY}")
    MESSAGE(STATUS "    MATLAB_MX_LIBRARY:   ${Matlab_MX_LIBRARY}")
            
    INCLUDE_DIRECTORIES(
        ${CLIPPER2_INCLUDE_DIR}
        ${CodeEli_INCLUDE_DIRS}
        ${CPPTEST_INCLUDE_DIR}
        ${CPPTEST_INCLUDE_DIR}
        ${DELABELLA_INCLUDE_DIR}
        ${EIGEN3_INCLUDE_DIR}
        ${FLTK_INCLUDE_DIR}
        ${GEOM_API_INCLUDE_DIR}
        ${GEOM_CORE_INCLUDE_DIR}
        ${GUI_AND_DRAW_INCLUDE_DIR}
        ${LIBIGES_INCLUDE_DIR}
        ${LIBXML2_INCLUDE_DIR}
        ${Matlab_INCLUDE_DIRS}
        ${NANOFLANN_INCLUDE_DIR}
        ${STEPCODE_INCLUDE_DIR}
        ${TRIANGLE_INCLUDE_DIR}
        ${UTIL_INCLUDE_DIR}
        ${XMLVSP_INCLUDE_DIR}
        ${VSP_SOURCE_DIR}
     )

    # TODO get both Matrix.h files included in vspMATLAB_wrap.cxx

    SET( SWIG_MODULE_vsp_EXTRA_DEPS
        ${UTIL_INCLUDE_DIR}/Defines.h
        ${GEOM_API_INCLUDE_DIR}/APIDefines.h
        ${GEOM_API_INCLUDE_DIR}/APIErrorMgr.h
        ${GEOM_API_INCLUDE_DIR}/VSP_Geom_API.h
        ${GEOM_CORE_INCLUDE_DIR}/SWIGDefines.h
        ${UTIL_INCLUDE_DIR}/Vec3d.h
        ${UTIL_INCLUDE_DIR}/Matrix4d.h
        ${Matlab_ROOT_DIR}/extern/include/Matrix.h
        ${Matlab_ROOT_DIR}/extern/include/mex.h
    )
    
    SET(CMAKE_SWIG_FLAGS "-verbose")
    
    set(I_FILES 
        ../geom_api/vsp.i
        )
    
    SET_SOURCE_FILES_PROPERTIES(${I_FILES} PROPERTIES CPLUSPLUS ON)

    SWIG_ADD_LIBRARY( vspMEX
        LANGUAGE matlab
        SOURCES ${I_FILES}
    )

    list(APPEND VSPMEX_LIBS 
        ${Matlab_LIBRARIES}
        ${VSP_LIBRARIES_API_FIRST}
    )
    
    MESSAGE(STATUS "${VSPMEX_LIBS}")
    
    SWIG_LINK_LIBRARIES( vspMEX ${VSPMEX_LIBS})

    add_definitions(/DMATLAB_MEX_FILE) #define matlab macros
    add_definitions(/DMX_COMPAT_32)
    
    # Needed for entry point: mxFunction()
    SET_TARGET_PROPERTIES(vspMEX PROPERTIES LINK_FLAGS "/export:mexFunction" )

    if(WIN32) # 32-bit or 64-bit mex
        if (CMAKE_CL_64)
            SET_TARGET_PROPERTIES(vspMEX PROPERTIES PREFIX "" SUFFIX .mexw64)
        else()
            SET_TARGET_PROPERTIES(vspMEX PROPERTIES SUFFIX .mexw32)
        endif()
    elseif(APPLE)
        SET_TARGET_PROPERTIES(vspMEX PROPERTIES PREFIX "" SUFFIX .mexmaci64) # Not tested
    elseif(UNIX)
        SET_TARGET_PROPERTIES(vspMEX PROPERTIES PREFIX "" SUFFIX .mexa64) # Not tested
    endif()

    install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/+vsp DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
    install (TARGETS vspMEX DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/SwigGet.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/SwigMem.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/SwigRef.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/APITestMain.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/APITestSuite_test.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/APITestSuiteVSPAERO_test.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/assert_delta.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)

ENDIF(SWIG_FOUND AND MATLAB_FOUND)

