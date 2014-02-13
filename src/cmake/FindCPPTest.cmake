# - Try to find the CPPTest library
#
#
if(CPPTEST_LIBRARIES)
  set(CPPTEST_FIND_QUIETLY TRUE)
else()
  find_path(CPPTEST_INCLUDE_DIRS cpptest.h
            PATHS
              /usr/include
              /usr/local/include
              /opt/local/include
              ${CMAKE_INSTALL_PREFIX}/include
              $ENV{CPPTESTDIR}
              ${INCLUDE_INSTALL_DIR})
  find_library(CPPTEST_LIBRARIES cpptest
               PATHS
                 $ENV{CPPTESTDIR}
                 /usr/local/lib
                 /opt/local/lib
                 ${LIB_INSTALL_DIR})
  get_filename_component(CPPTEST_LIBRARY_DIRS ${CPPTEST_LIBRARIES} PATH)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(CPPTest DEFAULT_MSG CPPTEST_INCLUDE_DIRS CPPTEST_LIBRARIES CPPTEST_LIBRARY_DIRS )
endif()
