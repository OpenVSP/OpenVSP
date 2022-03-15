#[=============================================================================[
------------------------
CreateVirtualEnvironment
------------------------
Function that creates a virtual Python environment with specific requirements.

Author: zoodinger
License: WTFPL

Required Arguments:
    TARGET (string):
        Target name of the virtual environment that can be used by other
        targets as a dependency.

Optional Arguments:
    REQUIREMENTS_TXT (string):
        Path to requirements.txt list to install with pip
    REQUIREMENTS (list of strings):
        Any additional requirements to install with pip that aren't part of
        requirements.txt, e.g. local packages
    SOURCES (list of string):
        Any sources that local packages depend on.
    PREFIX (string):
        Path where the environment will be placed at. Can be relative (under
        ${CMAKE_BINARY_DIR}) or absolute.
    ENV_NAME (string):
        The name of the virtual environment. Unless otherwise specified, this
        is the same as TARGET.
    WORKING_DIRECTORY (string):
        The name of the directory to run the pip command in.  Should be the
        location of the requirements.txt file.

Optional Output Arguments:
    OUT_PYTHON_EXE (output variable):
        Stores the path to the python executable of the virtual environment.
    OUT_BINARY_DIR (output variable):
        Stores the directory of any scripts / executables that are created as
        part of the environment, such as python[.exe], pip[.exe], activate, etc.
   OUT_VENV_DIR
        Stores the root path of the virtual environment.

#]=============================================================================]

function(CreateVirtualEnvironment TARGET)
    set(KEYWORD_ARGS REQUIREMENTS_TXT PREFIX ENV_NAME WORKING_DIRECTORY
            OUT_PYTHON_EXE OUT_BINARY_DIR OUT_VENV_DIR)

    set(MULTI_ARGS SOURCES REQUIREMENTS)

    cmake_parse_arguments(ARG "" "${KEYWORD_ARGS}" "${MULTI_ARGS}" ${ARGN})

    if (NOT ARG_ENV_NAME)
        set(ARG_ENV_NAME ${TARGET})
    endif ()

    if ( NOT ARG_WORKING_DIRECTORY )
        set( WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
    endif()

    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    if (ARG_PREFIX)
        if (IS_ABSOLUTE ${ARG_PREFIX})
            set(VENV ${ARG_PREFIX}/${ARG_ENV_NAME})
        else ()
            set(VENV ${CMAKE_BINARY_DIR}/${ARG_PREFIX}/${ARG_ENV_NAME})
        endif ()
    else ()
        set(VENV ${CMAKE_BINARY_DIR}/${ARG_ENV_NAME})
    endif ()

    if (WIN32)
        set(BIN_DIR ${VENV}/Scripts)
        set(PYTHON ${BIN_DIR}/python.exe)
    else ()
        set(BIN_DIR ${VENV}/bin)
        set(PYTHON ${BIN_DIR}/python)
    endif ()

    set(INSTALL_CMD ${BIN_DIR}/pip install --disable-pip-version-check)

    if (ARG_REQUIREMENTS_TXT)
        set(REQUIREMENTS -r ${ARG_REQUIREMENTS_TXT})
    endif()

    set(REQUIREMENTS ${REQUIREMENTS} "${ARG_REQUIREMENTS}")

    if (REQUIREMENTS)
        set(INSTALL_CMD ${BIN_DIR}/pip install --disable-pip-version-check)
        set(INSTALL_CMD ${INSTALL_CMD} ${REQUIREMENTS})
    else()
        set(INSTALL_CMD "")
    endif()

    set(CFG_FILE ${VENV}/pyvenv.cfg)
    add_custom_command(
            OUTPUT ${CFG_FILE}
            COMMAND ${Python3_EXECUTABLE} -m venv ${VENV}
    )
    set(OUTPUT_FILE ${VENV}/environment.txt)
    add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY}
            COMMAND ${INSTALL_CMD}
            COMMAND ${BIN_DIR}/pip freeze > ${OUTPUT_FILE}
            DEPENDS ${CFG_FILE} ${ARG_SOURCES} ${ARG_REQUIREMENTS_TXT}
    )

    add_custom_target(${TARGET} DEPENDS ${OUTPUT_FILE})

    if (ARG_OUT_VENV_DIR)
        set(${ARG_OUT_VENV_DIR} ${VENV} PARENT_SCOPE)
    endif ()

    if (ARG_OUT_PYTHON_EXE)
        set(${ARG_OUT_PYTHON_EXE} ${PYTHON} PARENT_SCOPE)
    endif ()

    if (ARG_OUT_BINARY_DIR)
        set(${ARG_OUT_BINARY_DIR} ${BIN_DIR} PARENT_SCOPE)
    endif ()

endfunction()
