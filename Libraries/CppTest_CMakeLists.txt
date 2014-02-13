CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT( CPPTEST )

SET( CPPTEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE PATH "Path to cpptest headers" )

INCLUDE_DIRECTORIES( 
     ${CPPTEST_INCLUDE_DIR}
     )

SET( CPPTEST_INCLUDE_HEADERS
src/cpptest-assert.h
src/cpptest-collectoroutput.h
src/cpptest-compileroutput.h
src/cpptest-htmloutput.h
src/cpptest-output.h
src/cpptest-source.h
src/cpptest-suite.h
src/cpptest-textoutput.h
src/cpptest-time.h
src/cpptest.h
)

ADD_LIBRARY(cpptest
src/collectoroutput.cpp
src/compileroutput.cpp
src/htmloutput.cpp
src/missing.cpp
src/source.cpp
src/suite.cpp
src/textoutput.cpp
src/time.cpp
src/utils.cpp
src/missing.h
src/utils.h
${CPPTEST_INCLUDE_HEADERS}
)

MESSAGE( STATUS "CMAKE_INSTALL_PREFIX: " ${CMAKE_INSTALL_PREFIX} )


INSTALL( TARGETS cpptest
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib )
INSTALL( FILES ${CPPTEST_INCLUDE_HEADERS}
	DESTINATION include )

