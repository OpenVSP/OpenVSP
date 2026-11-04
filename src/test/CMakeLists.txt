CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

ADD_SUBDIRECTORY( scripttest )
ADD_SUBDIRECTORY( py )
ADD_SUBDIRECTORY( dba_test )

# The C++ API test suites built into apitest.  They write their files without naming a
# path, so they run from their own directory in the build tree.
SET( CPPTEST_WORK_DIR ${CMAKE_CURRENT_BINARY_DIR}/CppApiTests )
FILE( MAKE_DIRECTORY ${CPPTEST_WORK_DIR} )

ADD_TEST( NAME CppApiTests COMMAND $<TARGET_FILE:apitest> --suites
          WORKING_DIRECTORY ${CPPTEST_WORK_DIR} )

# The unstable C++ tests (the VSPAERO solves of one model, which crash vspaero now and then,
# and the CpSlicer test that reads their results) are left out of the suite; configure with
# -DVSP_UNSTABLE_TESTS=ON to register them, and run them with ctest -L unstable.
IF( VSP_UNSTABLE_TESTS )
    ADD_TEST( NAME CppApiUnstableTests COMMAND $<TARGET_FILE:apitest> --unstable-suites
              WORKING_DIRECTORY ${CPPTEST_WORK_DIR} )
    SET_TESTS_PROPERTIES( CppApiUnstableTests PROPERTIES LABELS unstable )
ENDIF()
