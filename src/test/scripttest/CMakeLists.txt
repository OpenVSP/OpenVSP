CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET( VSPSCRIPT ${VSP_BINARY_DIR}/vsp/vspscript -script )
SET( SCRIPT_DIR ${VSP_SOURCE_DIR}/../examples/scripts/ )

ADD_TEST( AreaProj ${VSPSCRIPT} ${SCRIPT_DIR}TestAreaProj.vspscript )

