CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# include the CTest framework
include(CTest)

FIND_PACKAGE(SWIG REQUIRED)
IF( SWIG_FOUND )
	INCLUDE(${SWIG_USE_FILE})
ELSE(SWIG_FOUND)
	MESSAGE("SWIG not found")
ENDIF( SWIG_FOUND )

FIND_PACKAGE(Matlab REQUIRED MX_LIBRARY)
IF(SWIG_FOUND AND MATLAB_FOUND)
	message(STATUS "SWIG & MATLAB Found, MATLAB MEX will be compiled.")
	MESSAGE(STATUS "    swig.exe: ${SWIG_EXECUTABLE}")
	MESSAGE(STATUS "    Matlab_INCLUDE_DIRS: ${Matlab_INCLUDE_DIRS}")
	MESSAGE(STATUS "    Matlab_LIBRARIES:    ${Matlab_LIBRARIES}")
	MESSAGE(STATUS "    Matlab_MEX_LIBRARY:  ${Matlab_MEX_LIBRARY}")
	MESSAGE(STATUS "    Matlab_MX_LIBRARY:   ${Matlab_MX_LIBRARY}")
ELSE(SWIG_FOUND AND MATLAB_FOUND)
	MESSAGE("SWIG & MATLAB not found... MATLAB API will NOT be built.")
ENDIF(SWIG_FOUND AND MATLAB_FOUND)


IF(SWIG_FOUND AND MATLAB_FOUND)

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
		${SCREENS_INCLUDE_DIR}
		${XMLVSP_INCLUDE_DIR}
		${CPPTEST_INCLUDE_DIR}
		${JPEG_INCLUDE_DIR}
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
		${UTIL_INCLUDE_DIR}/Matrix.h
		${Matlab_ROOT_DIR}/extern/include/Matrix.h
		${Matlab_ROOT_DIR}/extern/include/mex.h
	)

	SET(CMAKE_SWIG_FLAGS "-verbose")
	set(I_FILES 
        ${CMAKE_SOURCE_DIR}/src/matlab_api/vsp.i
        #${CMAKE_SOURCE_DIR}/src/matlab_api/vsp_common.i
        )
	SET_PROPERTY(SOURCE ${I_FILES} PROPERTY CPLUSPLUS ON)

	SWIG_ADD_LIBRARY( vspMEX
		LANGUAGE matlab
		SOURCES ${I_FILES} )

	list(APPEND VSPMEX_LIBS 
		${Matlab_LIBRARIES}
		geom_api
		geom_core
		cfd_mesh
		triangle
		xmlvsp
		sixseries
		util
		tritri
		clipper
		Angelscript
		wavedragEL
		${CPPTEST_LIBRARIES}
		${LIBXML2_LIBRARIES}
		${WINSOCK_LIBRARIES}
		${CMINPACK_LIBRARIES}
		${STEPCODE_LIBRARIES}
		${LIBIGES_LIBRARIES}
	)
	MESSAGE(STATUS "${VSPMEX_LIBS}")
	
	SWIG_LINK_LIBRARIES( vspMEX ${VSPMEX_LIBS})

	add_definitions(/DMATLAB_MEX_FILE) #define matlab macros
	add_definitions(/DMX_COMPAT_32)

	# Needed for entry point: mxFunction()
	SET_TARGET_PROPERTIES(vspMEX
	PROPERTIES
	LINK_FLAGS "/export:mexFunction"
	)

	if(WIN32) # 32-bit or 64-bit mex
		if (CMAKE_CL_64)
			SET_TARGET_PROPERTIES(vspMEX PROPERTIES PREFIX "" SUFFIX .mexw64)
		else()
			SET_TARGET_PROPERTIES(vspMEX PROPERTIES SUFFIX .mexw32)
		endif()
	else()
		# other platforms
	endif()

	#add_custom_command(TARGET vspMEX
	#				 POST_BUILD
	#				 COMMAND 7z a "+vsp.7z" "+vsp"
	#				 WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

	install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/+vsp DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
	install (TARGETS vspMEX DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/SwigGet.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/SwigMem.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/SwigRef.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
	install (FILES ${CMAKE_SOURCE_DIR}/src/matlab_api/APITestMain.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
	install (FILES ${CMAKE_SOURCE_DIR}/src/matlab_api/APITestSuite_test.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
	install (FILES ${CMAKE_SOURCE_DIR}/src/matlab_api/APITestSuiteVSPAERO_test.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)
	install (FILES ${CMAKE_SOURCE_DIR}/src/matlab_api/assert_delta.m DESTINATION ${CMAKE_INSTALL_PREFIX}/matlab_api)


ENDIF(SWIG_FOUND AND MATLAB_FOUND)
