CMAKE_MINIMUM_REQUIRED( VERSION 3.24 )

ADD_SUBDIRECTORY( other )

ADD_TEST( NAME PyTest COMMAND ${PYTHON_VENV_EXE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

# The generated sample code test reads a fuselage cross section file and an
# airfoil file by relative path, the same way the AngelScript half does.  Copy
# them next to where the test runs and pin the working directory, so the test
# does not depend on the directory it happens to be launched from.
CONFIGURE_FILE( ${VSP_SOURCE_DIR}/../TestXSec.fxs
                ${CMAKE_CURRENT_BINARY_DIR}/TestXSec.fxs COPYONLY )
CONFIGURE_FILE( ${VSP_SOURCE_DIR}/../examples/airfoil/N0012_VSP.af
                ${CMAKE_CURRENT_BINARY_DIR}/airfoil/N0012_VSP.af COPYONLY )

ADD_TEST( NAME PySampleCodeTest COMMAND ${PYTHON_VENV_EXE} -m unittest openvsp.tests.test_vsp_api
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

ADD_TEST( NAME PyDegenGeomTest COMMAND ${PYTHON_VENV_EXE} -m unittest openvsp.tests.test_degen_geom  )
