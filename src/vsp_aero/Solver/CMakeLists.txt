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
            -DAdept2_INSTALL_DIR=${Adept2_INSTALL_DIR}
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
	IF ( NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
        INSTALL( PROGRAMS ${BINARY_DIR}/vspaero_complex DESTINATION . )
    ENDIF()

    # If this level finds Adept2, then we can assume the ExternalProject_Add level also found it and
    # succeeded in building vspaero_adjoint.
    SET( ORIG_CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} )
    SET( CMAKE_PREFIX_PATH ${Adept2_INSTALL_DIR} ${ORIG_CMAKE_PREFIX_PATH} )
    FIND_PACKAGE(Adept2 CONFIG )
    SET( CMAKE_PREFIX_PATH ${ORIG_CMAKE_PREFIX_PATH} )

    IF( Adept2_FOUND AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        INSTALL( PROGRAMS ${BINARY_DIR}/vspaero_adjoint DESTINATION . )
        INSTALL( PROGRAMS ${BINARY_DIR}/vspaero_opt DESTINATION . )
    ENDIF()

  else()
    set(BUILD_VSPAERO true)
  endif()
endif()


if(BUILD_VSPAERO)

  IF(WIN32)
  ELSE()
    ADD_DEFINITIONS( -DMYTIME )
  ENDIF()

  SET(LIB_TARGETS solver)
  SET(SOLVER_TARGETS vspaero)

  IF( NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
      LIST(APPEND LIB_TARGETS complex)
      LIST(APPEND SOLVER_TARGETS vspaero_complex)
  ENDIF()

  SET( ORIG_CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} )
  SET( CMAKE_PREFIX_PATH ${Adept2_INSTALL_DIR} ${ORIG_CMAKE_PREFIX_PATH} )
  FIND_PACKAGE(Adept2 CONFIG )
  SET( CMAKE_PREFIX_PATH ${ORIG_CMAKE_PREFIX_PATH} )

  MESSAGE(STATUS "Adept2_FOUND " ${Adept2_FOUND})

  IF(Adept2_FOUND AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      LIST(APPEND LIB_TARGETS adjoint)
      LIST(APPEND SOLVER_TARGETS vspaero_adjoint)
  ENDIF()

  SET( VSPAERO_CORE_FILES
  BoundaryConditionData.C
  ComponentGroup.C
  ControlSurface.C
  ControlSurfaceGroup.C
  EngineFace.C
  FEM_Node.C
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
  Search.C
  SearchLeaf.C
  SpanLoadData.C
  SpanLoadRotorData.C
  time.C
  utils.C
  Vortex_Bound.C
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
  VSP_Surface.C
  VSPAERO_TYPES.C
  WOPWOP.C
  BoundaryConditionData.H
  ComponentGroup.H
  ControlSurface.H
  ControlSurfaceGroup.H
  EngineFace.H
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
  Search.H
  SearchLeaf.H
  SpanLoadData.H
  SpanLoadRotorData.H
  time.H
  utils.H
  Vortex_Bound.H
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
  VSP_Surface.H
  VSPAERO_TYPES.H
  WOPWOP.H
  )

  LIST( LENGTH SOLVER_TARGETS ntarget )
  SET( itarget 0 )
  WHILE( itarget LESS ${ntarget} )
    LIST( GET SOLVER_TARGETS ${itarget} sol )
    LIST( GET LIB_TARGETS ${itarget} lib )

    ADD_LIBRARY( ${lib}
    ${VSPAERO_CORE_FILES}
    )

    ADD_EXECUTABLE( ${sol}
    vspaero.C
    VSPAERO.H
    VSPAERO_OMP.H
    )

    TARGET_LINK_LIBRARIES( ${sol} PUBLIC ${lib} )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
      TARGET_COMPILE_OPTIONS( ${sol} PUBLIC -Wno-non-pod-varargs -Wno-format-security -Wno-format )
      TARGET_COMPILE_OPTIONS( ${lib} PUBLIC -Wno-non-pod-varargs -Wno-format-security -Wno-format )
    endif()

    IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        TARGET_COMPILE_OPTIONS( ${sol} PUBLIC -funroll-loops -funroll-all-loops -Wno-unused -Wno-format-security -Wno-format-overflow -Wno-unused-result -Wno-format )
        TARGET_COMPILE_OPTIONS( ${lib} PUBLIC -funroll-loops -funroll-all-loops -Wno-unused -Wno-format-security -Wno-format-overflow -Wno-unused-result -Wno-format )
    ENDIF()

    MATH(EXPR itarget "${itarget}+1")
  ENDWHILE()

  IF( NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
    TARGET_COMPILE_DEFINITIONS( complex PRIVATE -DCOMPLEXDIFF )
    TARGET_COMPILE_DEFINITIONS( vspaero_complex PRIVATE -DCOMPLEXDIFF )
  ENDIF()

  if( OpenMP_CXX_FOUND AND NOT CXX_OMP_COMPILER )
    TARGET_LINK_LIBRARIES( solver PRIVATE OpenMP::OpenMP_CXX )
    TARGET_LINK_LIBRARIES( vspaero PRIVATE OpenMP::OpenMP_CXX )

    TARGET_COMPILE_DEFINITIONS( solver PRIVATE -DVSPAERO_OPENMP )
    TARGET_COMPILE_DEFINITIONS( vspaero PRIVATE -DVSPAERO_OPENMP )

    IF( NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
      TARGET_LINK_LIBRARIES( complex PRIVATE OpenMP::OpenMP_CXX )
      TARGET_LINK_LIBRARIES( vspaero_complex PRIVATE OpenMP::OpenMP_CXX )

      TARGET_COMPILE_DEFINITIONS( complex PRIVATE -DVSPAERO_OPENMP )
      TARGET_COMPILE_DEFINITIONS( vspaero_complex PRIVATE -DVSPAERO_OPENMP )
    ENDIF()
  endif()

  if(Adept2_FOUND AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    TARGET_COMPILE_DEFINITIONS( vspaero_adjoint PRIVATE -DAUTODIFF -DADEPT_RECORDING_PAUSABLE )
    TARGET_COMPILE_DEFINITIONS( adjoint PRIVATE -DAUTODIFF -DADEPT_RECORDING_PAUSABLE )

    TARGET_LINK_LIBRARIES( vspaero_adjoint PRIVATE adept::adept )
    TARGET_LINK_LIBRARIES( adjoint PRIVATE adept::adept )

    TARGET_INCLUDE_DIRECTORIES( vspaero_adjoint PRIVATE adept::adept )
    TARGET_INCLUDE_DIRECTORIES( adjoint PRIVATE adept::adept )


    ADD_EXECUTABLE( vspaero_opt
    VSP_Optimizer.C
    vspaero_opt.C
    VSP_Optimizer.H
    )

    TARGET_LINK_LIBRARIES( vspaero_opt PRIVATE adept::adept )
    TARGET_LINK_LIBRARIES( vspaero_opt PUBLIC solver adjoint )

    TARGET_INCLUDE_DIRECTORIES( vspaero_opt PRIVATE adept::adept )

    TARGET_COMPILE_OPTIONS( vspaero_opt PUBLIC -Wno-non-pod-varargs -Wno-format-security -Wno-format )

    if( OpenMP_CXX_FOUND AND NOT CXX_OMP_COMPILER )
      TARGET_LINK_LIBRARIES( vspaero_opt PRIVATE OpenMP::OpenMP_CXX )
      TARGET_COMPILE_DEFINITIONS( vspaero_opt PRIVATE -DVSPAERO_OPENMP )
    endif()

  ENDIF()

  if ( NOT EP_BUILD )

    if(MSVC)
      if(OPENMP_FOUND)
        SET( CMAKE_INSTALL_OPENMP_LIBRARIES "true" )
      endif()
      SET( CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "." )
      INCLUDE( InstallRequiredSystemLibraries )
    endif()

    INSTALL( TARGETS ${SOLVER_TARGETS} RUNTIME DESTINATION . )
    # INSTALL( TARGETS solver LIBRARY DESTINATION . )

    if(Adept2_FOUND AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      INSTALL( TARGETS vspaero_opt RUNTIME DESTINATION . )
      # INSTALL( TARGETS adjoint LIBRARY DESTINATION . )
    ENDIF()
  endif()

endif()
