CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET( VSPSCRIPT ${VSP_BINARY_DIR}/vsp/vspscript -script )
SET( SCRIPT_DIR ${VSP_SOURCE_DIR}/../examples/scripts/ )

# The Test*.vspscript examples ship in the distribution, so they stay where they
# are and are referenced from here.  testanimate.vspscript is deliberately left
# out because it needs the GUI.
#
# TestAll reads a fuselage cross section file and an airfoil file by relative
# path.  Copy them next to where the tests run so the scripts do not depend on
# the directory the test happens to be launched from.
CONFIGURE_FILE( ${VSP_SOURCE_DIR}/../TestXSec.fxs
                ${CMAKE_CURRENT_BINARY_DIR}/TestXSec.fxs COPYONLY )
CONFIGURE_FILE( ${VSP_SOURCE_DIR}/../examples/airfoil/N0012_VSP.af
                ${CMAKE_CURRENT_BINARY_DIR}/airfoil/N0012_VSP.af COPYONLY )

ADD_TEST( NAME AreaProj      COMMAND ${VSPSCRIPT} ${SCRIPT_DIR}TestAreaProj.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
ADD_TEST( NAME AxisProj      COMMAND ${VSPSCRIPT} ${SCRIPT_DIR}TestAxisProj.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
ADD_TEST( NAME SurfQuery     COMMAND ${VSPSCRIPT} ${SCRIPT_DIR}TestSurfQuery.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
ADD_TEST( NAME SaveLoad      COMMAND ${VSPSCRIPT} ${SCRIPT_DIR}TestSaveLoad.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
ADD_TEST( NAME Measure       COMMAND ${VSPSCRIPT} ${SCRIPT_DIR}TestMeasure.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
ADD_TEST( NAME Facet         COMMAND ${VSPSCRIPT} ${SCRIPT_DIR}TestFacet.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
ADD_TEST( NAME DXF           COMMAND ${VSPSCRIPT} ${SCRIPT_DIR}TestDXF.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
ADD_TEST( NAME SVG           COMMAND ${VSPSCRIPT} ${SCRIPT_DIR}TestSVG.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
ADD_TEST( NAME Analysis      COMMAND ${VSPSCRIPT} ${SCRIPT_DIR}TestAnalysis.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
ADD_TEST( NAME AnalysisVSPAERO COMMAND ${VSPSCRIPT} ${SCRIPT_DIR}TestAnalysisVSPAERO.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
ADD_TEST( NAME MassProperties COMMAND ${VSPSCRIPT} ${SCRIPT_DIR}TestMassProperties.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
ADD_TEST( NAME All           COMMAND ${VSPSCRIPT} ${SCRIPT_DIR}TestAll.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

# gen_unit_test.py turns every \code{.cpp} block in VSP_Geom_API.h into a
# routine in this script.  The Python half of the same generator is already run
# by PySampleCodeTest; this is the AngelScript half, which was being generated
# and then never run.
ADD_TEST( NAME ScriptSampleCodeTest
          COMMAND ${VSPSCRIPT} ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/tests/test_vsp_api.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

ADD_TEST( NAME TestChordAdjust
          COMMAND ${VSPSCRIPT} ${CMAKE_CURRENT_SOURCE_DIR}/TestChordAdjust/TestChordAdjust.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/TestChordAdjust/ )

ADD_TEST( RoutingGeom ${VSPSCRIPT} ${CMAKE_CURRENT_SOURCE_DIR}/TestRoutingGeom.vspscript )

ADD_TEST( Vec3d ${VSPSCRIPT} ${CMAKE_CURRENT_SOURCE_DIR}/TestVec3d.vspscript )

ADD_TEST( ErrorObj ${VSPSCRIPT} ${CMAKE_CURRENT_SOURCE_DIR}/TestErrorObj.vspscript )

ADD_TEST( Matrix4d ${VSPSCRIPT} ${CMAKE_CURRENT_SOURCE_DIR}/TestMatrix4d.vspscript )
