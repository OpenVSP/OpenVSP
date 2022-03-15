CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

# include the CTest framework
include(CTest)

# CMake 3.9 and below used 'LIBXML2_LIBRARIES' as the name of
# the cache entry storing the find_library result.  Use the
# value if it was set by the project or user.
if(DEFINED LIBXML2_LIBRARY AND NOT DEFINED LIBXML2_LIBRARIES)
  set(LIBXML2_LIBRARIES ${LIBXML2_LIBRARY})
endif()

IF(SWIG_FOUND AND PYTHONLIBS_FOUND)

    MESSAGE( STATUS "Configuring Python API" )

    INCLUDE_DIRECTORIES( ${VSP_SOURCE_DIR}
        ${PYTHON_INCLUDE_PATH}
        ${PYTHON_INCLUDE_DIR}
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

    SET(CMAKE_SWIG_FLAGS "")

    SET( SWIG_MODULE_vsp_EXTRA_DEPS
        ${UTIL_INCLUDE_DIR}/Defines.h
        ${GEOM_API_INCLUDE_DIR}/APIDefines.h
        ${GEOM_API_INCLUDE_DIR}/APIErrorMgr.h
        ${GEOM_API_INCLUDE_DIR}/VSP_Geom_API.h
        ${GEOM_CORE_INCLUDE_DIR}/SWIGDefines.h
        ${UTIL_INCLUDE_DIR}/Vec3d.h
    )

    set(I_FILES 
        ../geom_api/vsp.i
        )

    SET_SOURCE_FILES_PROPERTIES(${I_FILES} PROPERTIES CPLUSPLUS ON)

    IF (${CMAKE_VERSION} VERSION_LESS "3.8.0")
        SWIG_ADD_MODULE( vsp python ${I_FILES} )
    ELSE()
        SWIG_ADD_LIBRARY( vsp
        LANGUAGE python
        SOURCES ${I_FILES} )
    ENDIF()

    IF( APPLE )
        SET( PY_LIB "" )
        SET_TARGET_PROPERTIES(_vsp PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
    ELSE()
        SET( PY_LIB ${PYTHON_LIBRARIES} )
    ENDIF()

    SWIG_LINK_LIBRARIES( vsp
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
        ${PY_LIB}
        ${CMINPACK_LIBRARIES}
        ${STEPCODE_LIBRARIES}
        ${LIBIGES_LIBRARIES}
        ${TRIANGLE_LIBRARIES}
    )

    CONFIGURE_FILE(packages/openvsp/setup.py.in python/packages/openvsp/setup.py)
    CONFIGURE_FILE(packages/openvsp/conda-recipe/meta.yaml.in python/packages/openvsp/conda-recipe/meta.yaml)

    INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/packages/ DESTINATION python
            PATTERN setup.py.in EXCLUDE
            PATTERN meta.yaml.in EXCLUDE)

    INSTALL( TARGETS _vsp LIBRARY DESTINATION python/openvsp/openvsp )
    INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/vsp.py DESTINATION python/openvsp/openvsp )
    INSTALL( FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.py DESTINATION python/openvsp/openvsp/tests )
    INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/python/packages/openvsp/setup.py DESTINATION python/openvsp)
    INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/python/packages/openvsp/conda-recipe/meta.yaml DESTINATION python/openvsp/conda-recipe)

    # Copy python package files to ${CMAKE_BINARY_DIR}/python_pseudo such that a virtual environment can be
    # built from there.  This brings both generated files (from BINARY_DIR) and source files (from SOURCE_DIR) together
    # and places them in the expected relative places.
    add_custom_command( TARGET _vsp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/packages/
            ${CMAKE_BINARY_DIR}/python_pseudo )

    add_custom_command(TARGET _vsp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:_vsp>
            ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp )

    add_custom_command(TARGET _vsp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/vsp.py
            ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp )

    add_custom_command(TARGET _vsp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/python/packages/openvsp/setup.py
            ${CMAKE_BINARY_DIR}/python_pseudo/openvsp )

    add_custom_command(TARGET _vsp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/python/packages/openvsp/conda-recipe/meta.yaml
            ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/conda-recipe )

    # Create virtual environment with openvsp package installed.
    CreateVirtualEnvironment( venv
            REQUIREMENTS_TXT ${CMAKE_BINARY_DIR}/python_pseudo/requirements.txt
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/python_pseudo
            OUT_PYTHON_EXE PYTHON_VENV_EXE
            OUT_BINARY_DIR PYTHON_VENV_BIN_DIR
            OUT_VENV_DIR PYTHON_VENV_DIR )

    # MESSAGE( STATUS "PYTHON_VENV_EXE = ${PYTHON_VENV_EXE}")
    # MESSAGE( STATUS "PYTHON_VENV_BIN_DIR = ${PYTHON_VENV_BIN_DIR}")

    # Add python test file to CTest to run in virtual environment.
    ADD_TEST( PyTest ${PYTHON_VENV_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py )

ENDIF(SWIG_FOUND AND PYTHONLIBS_FOUND)
