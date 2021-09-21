CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

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
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -DVSPAERO_OPENMP")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -DVSPAERO_OPENMP")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND APPLE)
    # Historically was here to force OpenMP static.  Might not be needed anymore,
    # not sure if these are included with default MacOS.
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
  endif()

  set(BUILD_VSPAERO true)
else()

  if( CXX_OMP_COMPILER AND NOT EP_BUILD )

    INCLUDE( ExternalProject )

    ExternalProject_Add( SOLVER
        DOWNLOAD_COMMAND ""
        CMAKE_ARGS -DCMAKE_C_COMPILER=${C_OMP_COMPILER}
            -DCMAKE_CXX_COMPILER=${CXX_OMP_COMPILER}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DEP_BUILD=TRUE
        INSTALL_COMMAND ""
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
        DEPENDERS patch
    )

    INSTALL( PROGRAMS ${BINARY_DIR}/vspaero DESTINATION . )

  else()
    set(BUILD_VSPAERO true)
  endif()
endif()


if(BUILD_VSPAERO)

  IF(WIN32)
  ELSE()
    ADD_DEFINITIONS( -DMYTIME )
  ENDIF()

  ADD_EXECUTABLE(vspaero
  binaryio.C
  ComponentGroup.C
  ControlSurface.C
  ControlSurfaceGroup.C
  FEM_Node.C
  Gradient.C
  InteractionLoop.C
  RotorDisk.C
  SpanLoadData.C
  SpanLoadRotorData.C
  VSP_Agglom.C
  VSP_Edge.C
  VSP_Geom.C
  VSP_Grid.C
  VSP_Loop.C
  VSP_Node.C
  VSP_Solver.C
  VSP_Surface.C
  VortexSheetInteractionLoop.C
  VortexSheetVortex_To_VortexInteractionSet.C
  Vortex_Bound.C
  Vortex_Sheet.C
  Vortex_Trail.C
  matrix.C
  MatPrecon.C
  MergeSort.C
  quat.C
  Search.C
  SearchLeaf.C
  time.C
  utils.C
  VSPAERO_TYPES.C
  vspaero.C
  WOPWOP.C
  binaryio.H
  CharSizes.H
  ComponentGroup.H
  ControlSurface.H
  ControlSurfaceGroup.H
  FEM_Node.H
  Gradient.H
  Interaction.H
  InteractionLoop.H
  RotorDisk.H
  SpanLoadData.H
  SpanLoadRotorData.H
  VSPAERO_OMP.H
  VSP_Agglom.H
  VSP_Edge.H
  VSP_Geom.H
  VSP_Grid.H
  VSP_Loop.H
  VSP_Node.H
  VSP_Solver.H
  VSP_Surface.H
  VortexSheetInteractionLoop.H
  VortexSheetVortex_To_VortexInteractionSet.H
  Vortex_Bound.H
  Vortex_Sheet.H
  Vortex_Trail.H
  matrix.H
  MatPrecon.H
  MergeSort.H
  quat.H
  Search.H
  SearchLeaf.H
  time.H
  utils.H
  VSPAERO_TYPES.H
  WOPWOP.H
  )

  if(OPENMP_FOUND)
    TARGET_INCLUDE_DIRECTORIES( vspaero PRIVATE
        ${OpenMP_CXX_INCLUDE_DIR}
    )

    TARGET_LINK_LIBRARIES(vspaero
        ${OpenMP_CXX_LIBRARIES}
    )
  endif()

  if ( NOT EP_BUILD )

    if(MSVC)
      if(OPENMP_FOUND)
        SET( CMAKE_INSTALL_OPENMP_LIBRARIES "true" )
      endif()
      SET( CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "." )
      INCLUDE( InstallRequiredSystemLibraries )
    endif()

    INSTALL( TARGETS vspaero RUNTIME DESTINATION . )
  endif()

endif()
