# - Try to find the Code-Eli library
#
#

# if no minimum version numbers set then set them to zero
if(NOT CodeEli_FIND_VERSION)
  if(NOT CodeEli_FIND_VERSION_MAJOR)
    set(CodeEli_FIND_VERSION_MAJOR 0)
  endif()
  if(NOT CodeEli_FIND_VERSION_MINOR)
    set(CodeEli_FIND_VERSION_MINOR 0)
  endif()
  if(NOT CodeEli_FIND_VERSION_PATCH)
    set(CodeEli_FIND_VERSION_PATCH 0)
  endif()

  set(CodeEli_FIND_VERSION "${CodeEli_FIND_VERSION_MAJOR}.${CodeEli_FIND_VERSION_MINOR}.${CodeEli_FIND_VERSION_PATCH}")
endif()

# checks the version numbers against minimum version number
macro(_codeeli_check_version)
  file(READ "${CodeEli_FIRST_INCLUDE_DIR}/eli/code_eli.hpp" _CodeEli_HEADER_CONTENT)
  string(REGEX MATCH "define[ \t]+ELI_VERSION_MAJOR[ \t]+([0-9]+)" _CodeEli_VERSION_MAJOR_MATCH "${_CodeEli_HEADER_CONTENT}")
  set(CodeEli_VERSION_MAJOR "${CMAKE_MATCH_1}")
  string(REGEX MATCH "define[ \t]+ELI_VERSION_MINOR[ \t]+([0-9]+)" _CodeEli_VERSION_MINOR_MATCH "${_CodeEli_HEADER_CONTENT}")
  set(CodeEli_VERSION_MINOR "${CMAKE_MATCH_1}")
  string(REGEX MATCH "define[ \t]+ELI_VERSION_PATCH[ \t]+([0-9]+)" _CodeEli_VERSION_PATCH_MATCH "${_CodeEli_HEADER_CONTENT}")
  set(CodeEli_VERSION_PATCH "${CMAKE_MATCH_1}")
  set(CodeEli_VERSION "${CodeEli_VERSION_MAJOR}.${CodeEli_VERSION_MINOR}.${CodeEli_VERSION_PATCH}")

  set(CodeEli_FOUND FALSE)
  string(REGEX REPLACE "(\r?\n)+$" "" CodeEli_VERSION_STR "${CodeEli_VERSION}")
  
  # if found exact version then are good
  if (CodeEli_VERSION VERSION_EQUAL CodeEli_FIND_VERSION)
    set(CodeEli_FOUND TRUE)
    if (NOT CodeEli_FIND_QUIETLY)
      message(STATUS "Found Code-Eli version ${CodeEli_VERSION_STR}, and requested version ${CodeEli_FIND_VERSION}")
    endif()
  else()
    # if looking for exact then problem
    if (CodeEli_FIND_VERSION_EXACT)
      # if required send error message
      if (CodeEli_FIND_REQUIRED)
        message(FATAL_ERROR "Found Code-Eli version ${CodeEli_VERSION_STR}, but version ${CodeEli_FIND_VERSION} is required")
      # else send status if not quiet mode
      else()
        if (NOT CodeEli_FIND_QUIETLY)
          message(STATUS "Found Code-Eli version ${CodeEli_VERSION_STR}, but version ${CodeEli_FIND_VERSION} is required")
        endif()
      endif()
    else()
      # if version is greater than desired version (and don't need exact version)
      if (CodeEli_VERSION VERSION_GREATER CodeEli_FIND_VERSION)
        set(CodeEli_FOUND TRUE)
        if (NOT CodeEli_FIND_QUIETLY)
          message(STATUS "Found Code-Eli version ${CodeEli_VERSION_STR}, and requested at least version ${CodeEli_FIND_VERSION}")
        endif()
      # otherwise found version is tool old
      else()
        if (CodeEli_FIND_REQUIRED)
          message(FATAL_ERROR "Found Code-Eli version ${CodeEli_VERSION_STR}, but version ${CodeEli_FIND_VERSION} is required")
        else()
          if (NOT CodeEli_FIND_QUITELY)
            message(STATUS "Found Code-Eli version ${CodeEli_VERSION_STR}, but version ${CodeEli_FIND_VERSION} is required")
          endif()
        endif()
      endif()
    endif()
  endif()
endmacro()

# if already have Code-Eli include directories set then just check version
if(CodeEli_INCLUDE_DIRS)
  _codeeli_check_version()

  # if version number is not OK then set the find to fail
  if (NOT CodeEli_VERSION_OK)
    set(CodeEli_INCLUDE_DIRS "")
  endif()
# else need to find Code-Eli
else()
  # try and find where the code-eli.hpp file is
  find_path(CodeEli_FIRST_INCLUDE_DIR code_eli.hpp
            PATHS
              /usr/include
              /usr/local/include
              /opt/local/include
              ${CMAKE_INSTALL_PREFIX}/include
              $ENV{CodeEliDIR}
              ${INCLUDE_INSTALL_DIR}
            PATH_SUFFIXES
              include/eli)
  if (CodeEli_FIRST_INCLUDE_DIR STREQUAL "CodeEli_FIRST_INCLUDE_DIR-NOTFOUND")
    # if couldn't find it then done
    set(CodeEli_INCLUDE_DIRS "CodeEli_INCLUDE_DIRS-NOTFOUND")
    message(WARNING "Couldn't find code_eli.hpp")
  else()
    # strip off the eli path suffix
    get_filename_component(CodeEli_FIRST_INCLUDE_DIR ${CodeEli_FIRST_INCLUDE_DIR} PATH)
    
    # try and find the constants.hpp file
    find_path(CodeEli_SECOND_INCLUDE_DIR constants.hpp
              PATHS
                /usr/include
                /usr/local/include
                /opt/local/include
                ${CMAKE_INSTALL_PREFIX}/include
                $ENV{CodeEliDIR}
                ${INCLUDE_INSTALL_DIR}
              PATH_SUFFIXES
                include/eli)
    if (CodeEli_SECOND_INCLUDE_DIR STREQUAL "CodeEli_SECOND_INCLUDE_DIR-NOTFOUND")
      # if couldn't find it then done
      set(CodeEli_INCLUDE_DIRS "CodeEli_INCLUDE_DIRS-NOTFOUND")
      message(WARNING "Couldn't find constants.hpp")
    else()
      # strip off the eli path suffix
      get_filename_component(CodeEli_SECOND_INCLUDE_DIR ${CodeEli_SECOND_INCLUDE_DIR} PATH)
    
      # only add both directories if they are unique
      if (${CodeEli_FIRST_INCLUDE_DIR} STREQUAL ${CodeEli_SECOND_INCLUDE_DIR})
        set(CodeEli_INCLUDE_DIRS ${CodeEli_FIRST_INCLUDE_DIR})
      else()
        set(CodeEli_INCLUDE_DIRS ${CodeEli_FIRST_INCLUDE_DIR} ${CodeEli_SECOND_INCLUDE_DIR})
      endif()
    endif()
  endif()

  # check version number
  if (CodeEli_INCLUDE_DIRS STREQUAL "CodeEli_INCLUDE_DIRS-NOTFOUND")
    set(CodeEli_FOUND FALSE)
  else()
    _codeeli_check_version()
  endif()

  if (NOT CodeEli_FOUND)
    set(CodeEli_INCLUDE_DIRS "")
  endif()
endif()

mark_as_advanced(CodeEli_INCLUDE_DIRS)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CodeEli DEFAULT_MSG CodeEli_INCLUDE_DIRS)

