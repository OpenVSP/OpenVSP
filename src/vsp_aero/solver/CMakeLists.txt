CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

FIND_PACKAGE( OpenMP )

if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -DVSPAERO_OPENMP")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -DVSPAERO_OPENMP")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
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

  ADD_EXECUTABLE(vspaero
  RotorDisk.C
  VSP_Agglom.C
  VSP_DegenGeom.C
  VSP_Edge.C
  VSP_Grid.C
  VSP_Loop.C
  VSP_Node.C
  VSP_Solver.C
  VSP_Surface.C
  Vortex.C
  Vortex_Trail.C
  matrix.C
  quat.C
  utils.C
  vspaero.C
  CharSizes.H
  RotorDisk.H
  VSPAERO_OMP.H
  VSP_Agglom.H
  VSP_DegenGeom.H
  VSP_Edge.H
  VSP_Grid.H
  VSP_Loop.H
  VSP_Node.H
  VSP_Solver.H
  VSP_Surface.H
  Vortex.H
  Vortex_Trail.H
  matrix.H
  quat.H
  utils.H
  )

  TARGET_LINK_LIBRARIES(vspaero
  )

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
