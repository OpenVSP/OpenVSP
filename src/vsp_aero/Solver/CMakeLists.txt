CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

if ( EP_BUILD )
    PROJECT( VSPAERO_SOLVER )
    SET( CMAKE_CXX_STANDARD 17 )

    string( REPLACE "__VSP_DELIMETER__" ";" VSP_LIBRARIES_API_FIRST "${vsp_lib_escaped}" )

    string( REPLACE "__VSP_DELIMETER__" ";" CodeEli_INCLUDE_DIRS  "${CodeEli_INCLUDE_DIRS_escaped}" )
endif()

if( APPLE )
  set( CMAKE_FIND_LIBRARY_SUFFIXES_ORIG ${CMAKE_FIND_LIBRARY_SUFFIXES} )
  set( CMAKE_FIND_LIBRARY_SUFFIXES ".a;.tbd;.dylib;.so" )
  MESSAGE( STATUS "Forcing static linking on GCC and APPLE for VSPAERO Solver")
endif()

FIND_PACKAGE( OpenMP )

if( APPLE )
  set( CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES_ORIG}" )
endif()

MESSAGE( STATUS "OPENMP_FOUND = ${OPENMP_FOUND}" )
MESSAGE( STATUS "OpenMP_CXX_INCLUDE_DIR = ${OpenMP_CXX_INCLUDE_DIR}" )
MESSAGE( STATUS "OpenMP_CXX_LIBRARIES = ${OpenMP_CXX_LIBRARIES}" )

if(OPENMP_FOUND AND NOT CXX_OMP_COMPILER)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND APPLE)
    # Historically was here to force OpenMP static.  Might not be needed anymore,
    # not sure if these are included with default MacOS.
    # set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
  endif()

  set(BUILD_VSPAERO true)
else()

  if( CXX_OMP_COMPILER AND NOT EP_BUILD )

    INCLUDE( ExternalProject )

    set( vsp_lib_string )
    foreach( lib ${VSP_LIBRARIES_API_FIRST} )

        string( COMPARE EQUAL "${lib}" "OpenABF::OpenABF" isOpenABF )

        if ( isOpenABF )
            # Do nothing
        elseif ( TARGET ${lib} )
            get_target_property( libpath ${lib} BINARY_DIR )
            list( APPEND vsp_lib_string ${libpath}/lib${lib}.a )
        else()
            list( APPEND vsp_lib_string ${lib} )
        endif()
    endforeach()

    string( REPLACE ";" "__VSP_DELIMETER__" vsp_lib_escaped "${vsp_lib_string}" )

    string( REPLACE ";" "__VSP_DELIMETER__" CodeEli_INCLUDE_DIRS_escaped "${CodeEli_INCLUDE_DIRS}" )


    ExternalProject_Add( SOLVER
        DOWNLOAD_COMMAND ""
        CMAKE_ARGS -DCMAKE_C_COMPILER=${C_OMP_COMPILER}
            -DCMAKE_CXX_COMPILER=${CXX_OMP_COMPILER}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -Dvsp_lib_escaped=${vsp_lib_escaped}
            -DCodeEli_INCLUDE_DIRS_escaped=${CodeEli_INCLUDE_DIRS_escaped}
            -DGEOM_API_INCLUDE_DIR=${GEOM_API_INCLUDE_DIR}
            -DUTIL_INCLUDE_DIR=${UTIL_INCLUDE_DIR}
            -DEIGEN3_INCLUDE_DIR=${EIGEN3_INCLUDE_DIR}
            -DEP_BUILD=TRUE
        INSTALL_COMMAND ""
        DEPENDS cfd_mesh xmlvsp sixseries util tritri wavedragEL stb_image geom_core geom_api
    )
    ExternalProject_Get_Property( SOLVER BINARY_DIR )
    ExternalProject_Get_Property( SOLVER SOURCE_DIR )

    file( GLOB_RECURSE SOLVER_SOURCES RELATIVE
        "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*"
    )

    ExternalProject_Add_Step( SOLVER copy
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCE_DIR}
        DEPENDS ${SOLVER_SOURCES}
        DEPENDEES download
        DEPENDERS configure
    )

    INSTALL( PROGRAMS ${BINARY_DIR}/vspaero DESTINATION . )
    INSTALL( PROGRAMS ${BINARY_DIR}/vspaero_opt DESTINATION . )

  else()
    set(BUILD_VSPAERO true)
  endif()
endif()


if(BUILD_VSPAERO)

  IF(WIN32)
  ELSE()
    ADD_DEFINITIONS( -DMYTIME )
  ENDIF()

  SET( VSPAERO_CORE_FILES
  AdjointGradient.C
  BoundaryConditionData.C
  ComponentGroup.C
  ControlSurface.C
  ControlSurfaceGroup.C
  EngineFace.C
  FastMatrix.C
  FEM_Node.C
  GlobalWakeLoop.C
  Gradient.C
  InteractionLoop.C
  MatPrecon.C
  matrix.C
  MergeSort.C
  OptimizationFunction.C
  QuadCell.C
  QuadEdge.C
  QuadNode.C
  QuadTree.C
  quat.C
  RotorDisk.C
  SaveState.C
  Search.C
  SearchLeaf.C
  time.C
  utils.C
  Vortex_Sheet.C
  Vortex_Trail.C
  VortexSheetInteractionLoop.C
  VortexSheetVortex_To_VortexInteractionSet.C
  VSP_Agglom.C
  VSP_Edge.C
  VSP_Geom.C
  VSP_Grid.C
  VSP_Loop.C
  VSP_Node.C
  VSP_Solver.C
  WakeEdgeData.C
  # WOPWOP.C
  AdjointGradient.H
  BoundaryConditionData.H
  ComponentGroup.H
  ControlSurface.H
  ControlSurfaceGroup.H
  EngineFace.H
  FastMatrix.H
  FEM_Node.H
  Gradient.H
  InteractionLoop.H
  MatPrecon.H
  matrix.H
  MergeSort.H
  OptimizationFunction.H
  QuadCell.H
  QuadEdge.H
  QuadNode.H
  QuadTree.H
  quat.H
  RotorDisk.H
  SaveState.H
  Search.H
  SearchLeaf.H
  time.H
  utils.H
  Vortex_Sheet.H
  Vortex_Trail.H
  VortexSheetInteractionLoop.H
  VortexSheetVortex_To_VortexInteractionSet.H
  VSP_Agglom.H
  VSP_Edge.H
  VSP_Geom.H
  VSP_Grid.H
  VSP_Loop.H
  VSP_Node.H
  VSP_Solver.H
  WakeEdgeData.H
  # WOPWOP.H
  )

  ADD_LIBRARY( vspaero_lib
    ${VSPAERO_CORE_FILES}
  )

  ADD_EXECUTABLE( vspaero
    vspaero.C
  )

  IF( NOT VSP_NO_VSPAERO_OPT )
    ADD_EXECUTABLE( vspaero_opt
      vspaero_opt.C
    )
  ENDIF()

  TARGET_LINK_LIBRARIES( vspaero PUBLIC vspaero_lib )

  IF( NOT VSP_NO_VSPAERO_OPT )
    TARGET_LINK_LIBRARIES( vspaero_opt PUBLIC vspaero_lib )

    TARGET_LINK_LIBRARIES( vspaero_opt PUBLIC ${VSP_LIBRARIES_API_FIRST} )

    TARGET_INCLUDE_DIRECTORIES( vspaero_opt PUBLIC
      ${GEOM_API_INCLUDE_DIR}
      ${UTIL_API_INCLUDE_DIR}
    )
  ENDIF()


  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    TARGET_COMPILE_OPTIONS( vspaero_lib PUBLIC -Wno-non-pod-varargs -Wno-format-security -Wno-format )
    TARGET_COMPILE_OPTIONS( vspaero PUBLIC -Wno-non-pod-varargs -Wno-format-security -Wno-format )
    IF( NOT VSP_NO_VSPAERO_OPT )
      TARGET_COMPILE_OPTIONS( vspaero_opt PUBLIC -Wno-non-pod-varargs -Wno-format-security -Wno-format )
    ENDIF()
  endif()


  if( OpenMP_CXX_FOUND AND NOT CXX_OMP_COMPILER )
    TARGET_LINK_LIBRARIES( vspaero_lib PRIVATE OpenMP::OpenMP_CXX )
    TARGET_COMPILE_DEFINITIONS( vspaero_lib PRIVATE -DVSPAERO_OPENMP )

    TARGET_LINK_LIBRARIES( vspaero PRIVATE OpenMP::OpenMP_CXX )
    TARGET_COMPILE_DEFINITIONS( vspaero PRIVATE -DVSPAERO_OPENMP )

    IF( NOT VSP_NO_VSPAERO_OPT )
      TARGET_LINK_LIBRARIES( vspaero_opt PRIVATE OpenMP::OpenMP_CXX )
      TARGET_COMPILE_DEFINITIONS( vspaero_opt PRIVATE -DVSPAERO_OPENMP )
    ENDIF()
  endif()


  if ( NOT EP_BUILD )

    if(MSVC)
      if(OPENMP_FOUND)
        SET( CMAKE_INSTALL_OPENMP_LIBRARIES "true" )
      endif()
      SET( CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "." )
      INCLUDE( InstallRequiredSystemLibraries )
    endif()

    IF( SWIG_FOUND AND PYTHONLIBS_FOUND )
        INSTALL( TARGETS vspaero RUNTIME DESTINATION python/openvsp/openvsp )

        add_custom_command(TARGET vspaero POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:vspaero>
                ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/$<TARGET_FILE_NAME:vspaero> )

        IF( NOT VSP_NO_VSPAERO_OPT )
            INSTALL( TARGETS vspaero_opt RUNTIME DESTINATION python/openvsp/openvsp )

            add_custom_command(TARGET vspaero_opt POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    $<TARGET_FILE:vspaero_opt>
                    ${CMAKE_BINARY_DIR}/python_pseudo/openvsp/openvsp/$<TARGET_FILE_NAME:vspaero_opt> )
        ENDIF()

    ENDIF( SWIG_FOUND AND PYTHONLIBS_FOUND )

    get_property( is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG )

    if ( is_multi_config )
            SET( conf_suffix "$<CONFIG>/" )
    endif()

    INSTALL( TARGETS vspaero RUNTIME DESTINATION . )

    add_custom_command(TARGET vspaero POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:vspaero>
            ${CMAKE_BINARY_DIR}/src/vsp/${conf_suffix}$<TARGET_FILE_NAME:vspaero> )

    IF( NOT VSP_NO_VSPAERO_OPT )
      INSTALL( TARGETS vspaero_opt RUNTIME DESTINATION . )

      add_custom_command(TARGET vspaero_opt POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy
              $<TARGET_FILE:vspaero_opt>
              ${CMAKE_BINARY_DIR}/src/vsp/${conf_suffix}$<TARGET_FILE_NAME:vspaero_opt> )
    ENDIF()

  endif()

endif()
