CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

if ( EP_BUILD )
    PROJECT( VSPAERO_SOLVER )
    SET( CMAKE_CXX_STANDARD 17 )

    string( REPLACE "__VSP_DELIMETER__" ";" VSP_LIB "${VSP_LIB_ESCAPED}" )
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

    string( REPLACE ";" "__VSP_DELIMETER__" vsp_lib_escaped "${VSP_LIB}" )

    ExternalProject_Add( SOLVER
        DOWNLOAD_COMMAND ""
        CMAKE_ARGS -DCMAKE_C_COMPILER=${C_OMP_COMPILER}
            -DCMAKE_CXX_COMPILER=${CXX_OMP_COMPILER}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DVSP_LIB_ESCAPED=${vsp_lib_escaped}
            -DVSP_API_DIR=${VSP_API_DIR}
            -DEP_BUILD=TRUE
        INSTALL_COMMAND ""
        DEPENDS copy_api
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

  TARGET_LINK_LIBRARIES( vspaero PUBLIC vspaero_lib )

  IF( NOT VSP_NO_VSPAERO_OPT )
    ADD_EXECUTABLE( vspaero_opt
      vspaero_opt.C
    )

    TARGET_LINK_LIBRARIES( vspaero_opt PUBLIC vspaero_lib )

    TARGET_LINK_LIBRARIES( vspaero_opt PUBLIC ${VSP_LIB} )

    TARGET_INCLUDE_DIRECTORIES( vspaero_opt PUBLIC
      ${VSP_API_DIR}/include
    )

    IF( NOT EP_BUILD )
        ADD_DEPENDENCIES( vspaero_opt copy_api )
    ENDIF()
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
