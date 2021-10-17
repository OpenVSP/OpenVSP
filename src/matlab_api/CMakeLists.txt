CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

# include the CTest framework
include(CTest)

IF(SWIG_FOUND AND MATLAB_FOUND)

    MESSAGE(STATUS "MATLAB Found, MATLAB MEX will be compiled")
    MESSAGE(STATUS "    MATLAB_INCLUDE_DIRS: ${Matlab_INCLUDE_DIRS}")
    MESSAGE(STATUS "    MATLAB_LIBRARIES:    ${Matlab_LIBRARIES}")
    MESSAGE(STATUS "    MATLAB_MEX_LIBRARY:  ${Matlab_MEX_LIBRARY}")
    MESSAGE(STATUS "    MATLAB_MX_LIBRARY:   ${Matlab_MX_LIBRARY}")
            
    # CMake 3.9 and below used 'LIBXML2_LIBRARIES' as the name of
    # the cache entry storing the find_library result.  Use the
    # value if it was set by the project or user.
    if(DEFINED LIBXML2_LIBRARY AND NOT DEFINED LIBXML2_LIBRARIES)
      set(LIBXML2_LIBRARIES ${LIBXML2_LIBRARY})
    endif()
    
    INCLUDE_DIRECTORIES( ${VSP_SOURCE_DIR}
        ${Matlab_INCLUDE_DIRS}
        ${UTIL_INCLUDE_DIR}
        ${GEOM_CORE_INCLUDE_DIR}
        ${GEOM_API_INCLUDE_DIR}
        ${GUI_AND_DRAW_INCLUDE_DIR}
        ${TRIANGLE_INCLUDE_DIR}
        ${NANOFLANN_INCLUDE_DIR}
        ${CLIPPER_INCLUDE_DIR}
        ${CPPTEST_INCLUDE_DIR}
        ${XMLVSP_INCLUDE_DIR}
        ${CPPTEST_INCLUDE_DIR}
        ${FLTK_INCLUDE_DIR}
        ${LIBXML2_INCLUDE_DIR}
        ${EIGEN3_INCLUDE_DIR}
        ${CodeEli_INCLUDE_DIRS}
        ${STEPCODE_INCLUDE_DIR}
        ${LIBIGES_INCLUDE_DIR}
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

    IF (${CMAKE_VERSION} VERSION_LESS "3.8.0")
        SWIG_ADD_MODULE( vspMEX matlab ${I_FILES} )
    ELSE()
        SWIG_ADD_LIBRARY( vspMEX
        LANGUAGE matlab
        SOURCES ${I_FILES} )
    ENDIF()
        
    list(APPEND VSPMEX_LIBS 
        ${Matlab_LIBRARIES}
        geom_api
        geom_core
        cfd_mesh
        xmlvsp
        sixseries
        util
        tritri
        clipper
        Angelscript
        wavedragEL
        pinocchio
        ${CPPTEST_LIBRARIES}
        ${LIBXML2_LIBRARIES}
        ${WINSOCK_LIBRARIES}
        ${CMINPACK_LIBRARIES}
        ${STEPCODE_LIBRARIES}
        ${LIBIGES_LIBRARIES}
        ${TRIANGLE_LIBRARIES}
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

