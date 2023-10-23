CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET( VSPSCRIPT ${VSP_BINARY_DIR}/vsp/vspscript -script )
SET( SCRIPT_DIR ${VSP_SOURCE_DIR}/../examples/scripts/ )

ADD_TEST( AreaProj ${VSPSCRIPT} ${SCRIPT_DIR}TestAreaProj.vspscript )

ADD_TEST( NAME TestChordAdjust
          COMMAND ${VSPSCRIPT} ${CMAKE_CURRENT_SOURCE_DIR}/TestChordAdjust/TestChordAdjust.vspscript
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/TestChordAdjust/ )

