//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_AGGLOM_H
#define VSP_AGGLOM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "utils.H"
#include "time.H"
#include "VSP_Node.H"
#include "VSP_Loop.H"
#include "VSP_Grid.H"
#include "VSP_Surface.H"

#define CORNER_BC        1
#define TE_EDGE_BC       2
#define LE_EDGE_BC       3
#define BOUNDARY_EDGE_BC 4
#define INTERIOR_EDGE_BC 5

// Small stack list class

class EDGE_STACK_LIST {
   
public:

    int Edge;
    int Side[2];
    
};

// Definition of the VSP_AGGLOM class

class VSP_AGGLOM {

private:

    // References to the find and coarse grids
    
    VSP_GRID *FineGrid_;
    VSP_GRID *CoarseGrid_;
 
    // Agglomeration data
    
    int NumberOfEdgesOnTE_;
    int NumberOfEdgesOnLE_;
    int NumberOfEdgesOnBoundary_;

    int *EdgeIsOnFront_;
    int *NodeIsOnFront_;
    int *VortexLoopWasAgglomerated_;
    int *CoarseEdgeList_;    
    int *CoarseNodeList_;
    int *EdgeDegree_;
    int *FrontEdgeQueue_;

    int NextEdgeInQueue_;
    int NumberOfEdgesInQueue_;
    int NextBestEdgeOnFront_;
    
    double GoodQuadAngle_;
    double WorstQuadAngle_;
    
    // Scratch arrays
    
    int *LoopHits_;
    int *DidThisLoop_;
    int *LoopListStack_;
    int StackSize_;

    // Agglomeration Routines
    
    int NumberOfLoopsMerged_;
    
    int *NodeOnSurfaceBorder_;
   
    void InitializeFront_(void);
    
    int NextAgglomerationEdge_(void);

    void UpdateFront_(void);
    
    void MergeVortexLoops_(void);
    void MergeVortexLoopsOld_(void);

    void CheckLoopQuality_(void);
    
    void CreateCoarseMesh_(void);
    
    VSP_GRID* MergeCoLinearEdges_(void);
           
    VSP_GRID* Agglomerate_(VSP_GRID &Grid);

    // Simplify and clean up mesh routines
 
    VSP_GRID* SimplifyMesh_(VSP_GRID &Grid);
    
    void CheckMesh_(VSP_GRID &ThisGrid);
    
    void CreateMixedMesh_(void);
    
    VSP_GRID* DeleteDuplicateNodes_(VSP_GRID &Grid);
    
    void CleanUpMesh_(void);
    
    void CleanUpFans_(void);
    
    void CleanUpHighAspectRatioTris_(void);

    void CleanUpSmallAreaLoops_(void);
    
    void CheckForDegenerateNodes_(void);

    void FindNeighborLoopOnLocalEdge_(VSP_GRID &ThisGrid, int Tri, int LocalEdge, 
                                      int &NeighborTri, int &Node1, int &Node2);

    void FindNeighborLoopOnEdge_(VSP_GRID &ThisGrid, int Tri, int Edge, 
                                 int &NeighborTri, int &Node1, int &Node2);

    double CalculateQuadQuality_(VSP_GRID &ThisGrid, int Node1,
                                 int Node2, int Node3, int Node4);    
                                 
    int BadTriangle_(VSP_GRID &ThisGrid, int Loop, int &MinEdge);
    
    double CalculateAspectRatioOld_(VSP_GRID &ThisGrid, int Loop);
    
    int LoopsAreCoplanar_(VSP_GRID &ThisGrid, int Loop1, int Loop2, double MaxAngle);
    
    int EdgesAreColinear_(VSP_GRID &ThisGrid, int Edge1, int Edge2, double MaxAngle);

    int MergedLoopsAreConvex_(VSP_GRID &ThisGrid, int Loop1, int Loop2, double MaxAngle);
    
    double CalculateLoopAngle_(VSP_GRID &ThisGrid, int Loop, int Node);
                                  
    // Initialization
    
    void Initialize_(void);
    
public:

    // Constructor, Destructor, Copy
    
    VSP_AGGLOM(void);
   ~VSP_AGGLOM(void);
    VSP_AGGLOM(const VSP_AGGLOM &agglom);

    // Reference to the coarse and fine grids
    
    VSP_GRID &FineGrid(void) { return *FineGrid_; };
    VSP_GRID &CoarseGrid(void) { return *CoarseGrid_; };
    
    // Agglomerate the grid
    
    VSP_GRID* Agglomerate(VSP_GRID &Grid) { return Agglomerate_(Grid); };    
    
    // Simplify mesh
    
    VSP_GRID* SimplifyMesh(VSP_GRID &Grid) { return SimplifyMesh_(Grid); };

};

#endif
