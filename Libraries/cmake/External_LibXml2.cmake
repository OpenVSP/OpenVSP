# An external project for libxml2
set(libxml2_source  "${CMAKE_BINARY_DIR}/LIBXML2-prefix/src/LIBXML2")
set(libxml2_install "${CMAKE_BINARY_DIR}/LIBXML2-prefix")

# If Windows we use configure.js and nmake, otherwise ./configure and make
if(WIN32)
  get_filename_component(_self_dir ${CMAKE_CURRENT_LIST_FILE} PATH)

  file(TO_NATIVE_PATH ${libxml2_install} libxml2_install_win)

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/libxml2_config.cmake "
execute_process(
  COMMAND cscript configure.js prefix=${libxml2_install_win} iconv=no
  WORKING_DIRECTORY \"${libxml2_source}/win32\"
  )")

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/libxml2_build.cmake "
execute_process(
  COMMAND nmake /f Makefile.msvc libxmla
  WORKING_DIRECTORY \"${libxml2_source}/win32\"
  )")

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/libxml2_install.cmake "
execute_process(
  COMMAND nmake /f Makefile.msvc install
  WORKING_DIRECTORY \"${libxml2_source}/win32\"
  )")

  set(libxml2_patch_command "")
  set(libxml2_config_command CONFIGURE_COMMAND ${CMAKE_COMMAND} -P
    ${CMAKE_CURRENT_BINARY_DIR}/libxml2_config.cmake)
  set(libxml2_build_command BUILD_COMMAND ${CMAKE_COMMAND} -P
    ${CMAKE_CURRENT_BINARY_DIR}/libxml2_build.cmake)
  set(libxml2_install_command INSTALL_COMMAND ${CMAKE_COMMAND} -P
    ${CMAKE_CURRENT_BINARY_DIR}/libxml2_install.cmake
	COMMAND ${CMAKE_COMMAND} -E remove ${libxml2_install}/lib/libxml2.dll
	COMMAND ${CMAKE_COMMAND} -E remove ${libxml2_install}/lib/libxml2.lib
	COMMAND ${CMAKE_COMMAND} -E remove ${libxml2_install}/lib/libxml2_a_dll.lib
	COMMAND ${CMAKE_COMMAND} -E rename ${libxml2_install}/lib/libxml2_a.lib ${libxml2_install}/lib/libxml2.lib )
else()
  set(libxml2_patch_command "")
  set(libxml2_config_command CONFIGURE_COMMAND ./configure
		--prefix=${libxml2_install}
		--without-python
		--without-iconv
		--without-zlib
		--without-lzma
		--disable-shared
  )
  set(libxml2_build_command "")
  set(libxml2_install_command "")
endif()

ExternalProject_Add(LIBXML2
  URL ${CMAKE_SOURCE_DIR}/libxml2-2.9.10.tar.gz
  SOURCE_DIR ${libxml2_source}
  INSTALL_DIR ${libxml2_install}
  BUILD_IN_SOURCE 1
  ${libxml2_patch_command}
  ${libxml2_config_command}
  ${libxml2_build_command}
  ${libxml2_install_command}
)
ExternalProject_Get_Property( LIBXML2 INSTALL_DIR )
SET( LIBXML2_INSTALL_DIR ${INSTALL_DIR} )
