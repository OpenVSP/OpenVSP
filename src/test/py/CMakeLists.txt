CMAKE_MINIMUM_REQUIRED( VERSION 3.24 )

ADD_SUBDIRECTORY( other )

# The suite runs from its own directory in the build tree, so the files a test writes without
# naming a path land there and not beside the tests.  pytest's cache and Python's bytecode go
# there too.
SET( PYTEST_WORK_DIR ${CMAKE_CURRENT_BINARY_DIR}/PyTest )
FILE( MAKE_DIRECTORY ${PYTEST_WORK_DIR} )

ADD_TEST( NAME PyTest COMMAND ${PYTHON_VENV_EXE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}
          --rootdir=${CMAKE_CURRENT_SOURCE_DIR} -o cache_dir=${PYTEST_WORK_DIR}/.pytest_cache
          WORKING_DIRECTORY ${PYTEST_WORK_DIR} )
SET_TESTS_PROPERTIES( PyTest PROPERTIES ENVIRONMENT "PYTHONPYCACHEPREFIX=${PYTEST_WORK_DIR}/__pycache__" )

# OpenVSP reads custom components from ./CustomScripts, so the suite's own go beside where it runs.
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/CustomScripts/IDPart.vsppart
                ${PYTEST_WORK_DIR}/CustomScripts/IDPart.vsppart COPYONLY )
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/CustomScripts/GuiArgsPart.vsppart
                ${PYTEST_WORK_DIR}/CustomScripts/GuiArgsPart.vsppart COPYONLY )

# The generated sample code test reads a fuselage cross section file and an
# airfoil file by relative path, the same way the AngelScript half does.  Copy
# them next to where the test runs and pin the working directory, so the test
# does not depend on the directory it happens to be launched from.
CONFIGURE_FILE( ${VSP_SOURCE_DIR}/../TestXSec.fxs
                ${CMAKE_CURRENT_BINARY_DIR}/TestXSec.fxs COPYONLY )
CONFIGURE_FILE( ${VSP_SOURCE_DIR}/../examples/airfoil/N0012_VSP.af
                ${CMAKE_CURRENT_BINARY_DIR}/airfoil/N0012_VSP.af COPYONLY )
CONFIGURE_FILE( ${VSP_SOURCE_DIR}/../examples/CCE/SD-24L.cce
                ${CMAKE_CURRENT_BINARY_DIR}/CCE/SD-24L.cce COPYONLY )
CONFIGURE_FILE( ${VSP_SOURCE_DIR}/../examples/textures/nasa-logo.tga
                ${CMAKE_CURRENT_BINARY_DIR}/textures/nasa-logo.tga COPYONLY )
CONFIGURE_FILE( ${VSP_SOURCE_DIR}/../examples/textures/window.tga
                ${CMAKE_CURRENT_BINARY_DIR}/textures/window.tga COPYONLY )

ADD_TEST( NAME PySampleCodeTest COMMAND ${PYTHON_VENV_EXE} -m unittest openvsp.tests.test_vsp_api
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

ADD_TEST( NAME PyDegenGeomTest COMMAND ${PYTHON_VENV_EXE} -m unittest openvsp.tests.test_degen_geom  )
