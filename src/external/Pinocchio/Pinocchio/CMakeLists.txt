CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(PINOCCHIO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to pinocchio headers")

# INCLUDE_DIRECTORIES(
# )

ADD_LIBRARY( pinocchio
        Pinocchio.cpp
        attachment.cpp
        discretization.cpp
        embedding.cpp
        graphutils.cpp
        indexer.cpp
        intersector.cpp
        lsqSolver.cpp
        matrix.cpp
        pin_mesh.cpp
        pinocchioApi.cpp
        quatinterface.cpp
        refinement.cpp
        skeleton.cpp
        Pinocchio.h
        attachment.h
        debugging.h
        deriv.h
        dtree.h
        dual_quat_cu.h
        graphutils.h
        hashutils.h
        indexer.h
        intersector.h
        lsqSolver.h
        mat3.h
        mathutils.h
        matrix.h
        pin_mesh.h
        multilinear.h
        pinocchioApi.h
        point3.h
        pointprojector.h
        quaddisttree.h
        quat_cu.h
        quatinterface.h
        rect.h
        skeleton.h
        transfo.h
        transform.h
        utils.h
        vec3.h
        vector.h
        vecutils.h
)

# ADD_DEPENDENCIES( pinocchio
# )
