CMAKE_MINIMUM_REQUIRED( VERSION 3.24 )

# include the CTest framework
include( CTest )

IF( SWIG_FOUND AND PYTHONLIBS_FOUND )

    MESSAGE( STATUS "Configuring Python API" )

    if( NOT VSP_NO_PYDOC )
        execute_process(
            COMMAND ${PYTHON_EXECUTABLE} -c "import sphinx"
            RESULT_VARIABLE EXIT_SPHINX
        )

        if ( ${EXIT_SPHINX} EQUAL 0 )
            SET ( SPHINX_FOUND true CACHE BOOL "SPHINX" )
            mark_as_advanced( SPHINX_FOUND )
            MESSAGE( STATUS "Sphinx found, Python API doc will be generated." )
        ELSE( ${EXIT_SPHINX} EQUAL 0 )
            MESSAGE( STATUS "Sphinx not found, Python API doc will not be generated." )
        endif()
    endif()

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
        ${NANOFLANN_INCLUDE_DIR}
        ${PYTHON_INCLUDE_DIR}
        ${PYTHON_INCLUDE_PATH}
        ${STEPCODE_INCLUDE_DIR}
        ${TRIANGLE_INCLUDE_DIR}
        ${UTIL_INCLUDE_DIR}
        ${UTIL_API_INCLUDE_DIR}
        ${XMLVSP_INCLUDE_DIR}
        ${VSP_SOURCE_DIR}
     )

    SET( SWIG_MODULE_pyvsp_EXTRA_DEPS
        ${GEOM_API_INCLUDE_DIR}/APIDefines.h
        ${GEOM_API_INCLUDE_DIR}/APIErrorMgr.h
        ${GEOM_API_INCLUDE_DIR}/VSP_Geom_API.h
        ${GEOM_CORE_INCLUDE_DIR}/SWIGDefines.h
        ${UTIL_API_INCLUDE_DIR}/Vec3d.h
    )

    set(I_FILES
        ../geom_api/vsp.i
    )

    SET_SOURCE_FILES_PROPERTIES( ${I_FILES} PROPERTIES CPLUSPLUS ON )

    # Although not documented this way, CMAKE_SWIG_FLAGS is apparently a list and must be built up this way.
    SET( CMAKE_SWIG_FLAGS "-threads" )

    if( DOXYGEN_FOUND AND NOT VSP_NO_PYDOC )
        list( APPEND CMAKE_SWIG_FLAGS "-doxygen" )
    endif()

    SWIG_ADD_LIBRARY( pyvsp
        LANGUAGE python
        SOURCES ${I_FILES}
    )

    SET_TARGET_PROPERTIES( pyvsp PROPERTIES OUTPUT_NAME "vsp" )

    IF( APPLE )
        SET( PY_LIB "" )
        SET_TARGET_PROPERTIES( pyvsp PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
    ELSE()
        SET( PY_LIB ${PYTHON_LIBRARIES} )
    ENDIF()

    if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        TARGET_COMPILE_OPTIONS( pyvsp PUBLIC -Wno-deprecated-declarations)
    endif()

    SWIG_LINK_LIBRARIES( pyvsp
        ${VSP_LIBRARIES_API_FIRST}
        ${PY_LIB}
    )


    IF( NOT VSP_NO_GRAPHICS )

        SET( SWIG_MODULE_pyvsp_g_EXTRA_DEPS
            ${GEOM_API_INCLUDE_DIR}/APIDefines.h
            ${GEOM_API_INCLUDE_DIR}/APIErrorMgr.h
            ${GEOM_API_INCLUDE_DIR}/VSP_Geom_API.h
            ${GEOM_CORE_INCLUDE_DIR}/SWIGDefines.h
            ${UTIL_API_INCLUDE_DIR}/Vec3d.h
        )

        set(I_FILES_g
            ../geom_api/vsp_g.i
            )

        SET_SOURCE_FILES_PROPERTIES( ${I_FILES_g} PROPERTIES CPLUSPLUS ON )

        SWIG_ADD_LIBRARY( pyvsp_g
            LANGUAGE python
            SOURCES ${I_FILES_g}
        )

        SET_TARGET_PROPERTIES( pyvsp_g PROPERTIES OUTPUT_NAME "vsp_g" )

        IF( APPLE )
            SET( PY_LIB "" )
            SET_TARGET_PROPERTIES( pyvsp_g PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
        ELSE()
            SET( PY_LIB ${PYTHON_LIBRARIES} )
        ENDIF()

        if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
            TARGET_COMPILE_OPTIONS( pyvsp_g PUBLIC -Wno-deprecated-declarations )
        endif()

        SWIG_LINK_LIBRARIES( pyvsp_g
            ${VSP_G_LIBRARIES_API_FIRST}
            ${PY_LIB}
        )

        add_custom_command( TARGET pyvsp_g POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:pyvsp_g>
            ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/$<TARGET_FILE_NAME:pyvsp_g> )

        #copy vsp_g to folder and add "placeholder" facade functions
        add_custom_command( TARGET pyvsp_g POST_BUILD
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/auto_facade.py
            ${CMAKE_CURRENT_BINARY_DIR}/vsp_g.py False

            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/vsp_g.py
            ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/vsp_g.py
        )

        INSTALL( TARGETS pyvsp_g LIBRARY DESTINATION python/openvsp/openvsp )
        INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/vsp_g.py DESTINATION python/openvsp/openvsp )

        add_dependencies( pyvsp_g copy_package ) # Make pyvsp depend on copy_package.  Not sure this is required.

    ENDIF()

    CONFIGURE_FILE( packages/openvsp/setup.py.in python/packages/openvsp/setup.py )
    CONFIGURE_FILE( packages/openvsp/conda-recipe/meta.yaml.in python/packages/openvsp/conda-recipe/meta.yaml )
    CONFIGURE_FILE( packages/environment.yml.in python/packages/environment.yml )

    INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/packages/ DESTINATION python
        PATTERN setup.py.in EXCLUDE
        PATTERN meta.yaml.in EXCLUDE
        PATTERN environment.yml.in EXCLUDE
        PATTERN conf.py EXCLUDE
        PATTERN index.rst EXCLUDE
        PATTERN openvsp.rst EXCLUDE )

    INSTALL( TARGETS pyvsp LIBRARY DESTINATION python/openvsp/openvsp )
    INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/vsp.py DESTINATION python/openvsp/openvsp )
    INSTALL( FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.py DESTINATION python/openvsp/openvsp/tests )
    INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/python/packages/openvsp/setup.py DESTINATION python/openvsp)
    INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/python/packages/openvsp/conda-recipe/meta.yaml DESTINATION python/openvsp/conda-recipe )
    INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/python/packages/environment.yml DESTINATION python )
    INSTALL( FILES ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/facade.py DESTINATION python/openvsp/openvsp )
    INSTALL( FILES ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/facade_server.py DESTINATION python/openvsp/openvsp )

    if ( SPHINX_FOUND )
        INSTALL( DIRECTORY ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/doc DESTINATION python/openvsp )
    endif( SPHINX_FOUND )

    INSTALL( FILES ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/tests/test_vsp_api.py DESTINATION python/openvsp/openvsp/tests )

    # Copy python package files to ${CMAKE_BINARY_DIR}/python_pseudo such that a virtual environment can be
    # built from there.  This brings both generated files (from BINARY_DIR) and source files (from SOURCE_DIR) together
    # and places them in the expected relative places.

    # Add copy_package target to copy full directory.  Unfortunately, will copy every build.  However, there is no
    # obvious way to have CMake track the contents of the directory for changes.
    add_custom_target( copy_package
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/packages/
        ${CMAKE_BINARY_DIR}/python_pseudo/ )
    add_dependencies( pyvsp copy_package ) # Make pyvsp depend on copy_package.  Not sure this is required.

    # Copy _vsp library post build.
    add_custom_command( TARGET pyvsp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove
        ${CMAKE_BINARY_DIR}/python_pseudo/environment.yml.in )

    add_custom_command( TARGET pyvsp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:pyvsp>
        ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/$<TARGET_FILE_NAME:pyvsp> )

    # Copy vsp.py wrapper post SWIG generation and run post copy commands.
    add_custom_command( TARGET pyvsp POST_BUILD
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/auto_facade.py
        ${CMAKE_CURRENT_BINARY_DIR}/vsp.py True

        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/vsp.py
        ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/vsp.py

        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/facade_server.py
        ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/facade_server.py

        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/facade.py
        ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/facade.py
    )

    add_custom_command( TARGET pyvsp POST_BUILD
        COMMAND ${PYTHON_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/gen_unit_test.py
        ${CMAKE_BINARY_DIR}/python_pseudo/openvsp
        ${VSP_SOURCE_DIR}/geom_api/VSP_Geom_API.h
    )

    if ( SPHINX_FOUND )
        add_custom_target( pydoc ALL
                COMMAND sphinx-build
                ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp
                ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/doc
                DEPENDS pyvsp
        )

        add_custom_command( TARGET pydoc POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E tar "cf"
                "${CMAKE_BINARY_DIR}/pydoc.zip"
                --format=zip
                -- .
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/python_pseudo/openvsp/doc"
        )

    ENDIF()

    # Configure files to pseudo install -- redundant to package configure above, but tracks dependencies best.
    # Unfortunately, it does not know if you delete the files from ${CMAKE_BINARY_DIR}.
    CONFIGURE_FILE( packages/openvsp/setup.py.in ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/setup.py )
    CONFIGURE_FILE( packages/openvsp/conda-recipe/meta.yaml.in ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/conda-recipe/meta.yaml )

    add_custom_command( TARGET pyvsp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/python/packages/environment.yml
        ${CMAKE_BINARY_DIR}/python_pseudo/environment.yml )

    # Create virtual environment with openvsp package installed.
    CreateVirtualEnvironment( venv
        REQUIREMENTS_TXT ${CMAKE_BINARY_DIR}/python_pseudo/requirements-dev.txt
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/python_pseudo
        OUT_PYTHON_EXE PYTHON_VENV_EXE
        OUT_BINARY_DIR PYTHON_VENV_BIN_DIR
        OUT_VENV_DIR PYTHON_VENV_DIR )

    # MESSAGE( STATUS "PYTHON_VENV_EXE = ${PYTHON_VENV_EXE}")
    # MESSAGE( STATUS "PYTHON_VENV_BIN_DIR = ${PYTHON_VENV_BIN_DIR}")

    SET( PYTHON_VENV_EXE ${PYTHON_VENV_EXE} PARENT_SCOPE )
    SET( PYTHON_VENV_BIN_DIR ${PYTHON_VENV_BIN_DIR} PARENT_SCOPE )
    SET( PYTHON_VENV_DIR ${PYTHON_VENV_DIR} PARENT_SCOPE )

    ADD_CUSTOM_TARGET( run_pyvsp ${PYTHON_VENV_EXE} -m pyvsp.pyvsp )


ENDIF(SWIG_FOUND AND PYTHONLIBS_FOUND)
