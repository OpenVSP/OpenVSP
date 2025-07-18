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
#include "VSP_Geom.H"
#include "Search.H"

#include "START_NAME_SPACE.H"

#define CORNER_BC        1
#define TE_EDGE_BC       2
#define LE_EDGE_BC       3
#define BOUNDARY_EDGE_BC 4
#define INTERIOR_EDGE_BC 5

#define   VLM_MODEL 1
#define PANEL_MODEL 2

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
    
    int *NumberOfLoopsForNode_;
    int **LoopListForNode_;

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
    
    // Fast search data structure
    
    SEARCH *Search_;

    // Agglomeration Routines
    
    int NumberOfLoopsMerged_;
    
    int *NodeOnSurfaceBorder_;
   
    void InitializeFront_(void);
    
    int FindMatchingSymmetryEdge_(int Edge);
    
    int NextAgglomerationEdge_(void);

    void UpdateFront_(void);
    
    void MergeVortexLoops_(void);
    
    void MergeVortexLoopsARTest_(void);
    
    void MergeSmallLoops_(void);
    
    void MergeSmallLoopsOld_(void);
    
    void MergeSurroundedLoops_(void);
    
    void MergeSmallLoopsBroken_(void);
        
    void CheckLoopQuality_(void);
    
    void CreateCoarseMesh_(void);
    
    VSP_GRID* MergeCoLinearEdges_(void);
           
    VSP_GRID* Agglomerate_(VSP_GRID &Grid);

    // Simplify and clean up mesh routines
 
    VSP_GRID* SimplifyMesh_(VSP_GRID &Grid);

    VSP_GRID* SimplifyMesh_(VSP_GRID &Grid, VSP_GRID &GridC);

    VSP_GRID* MergeHighAspectRatioQuads_(VSP_GRID &Grid);
    
    void CheckMesh_(VSP_GRID &ThisGrid);
    
    void CreateMixedMesh_(void);
    
    void CreateMixedMesh_(VSP_GRID &GridC);
        
    VSP_GRID* DeleteDuplicateNodes_(VSP_GRID &Grid);
    
    void CleanUpMesh_(void);
    
    void CleanUpFans_(void);
    
    void CleanUpHighAspectRatioTris_(void);

    void CleanUpHighAspectRatioQuads_(void);

    void CleanUpSmallAreaLoops_(void);
    
    void CheckForDegenerateNodes_(void);

    void FindNeighborLoopOnLocalEdge_(VSP_GRID &ThisGrid, int Tri, int LocalEdge, 
                                      int &NeighborTri, int &Node1, int &Node2);

    void FindNeighborLoopOnEdge_(VSP_GRID &ThisGrid, int Tri, int Edge, 
                                 int &NeighborTri, int &Node1, int &Node2);

    double CalculateQuadQuality_(VSP_GRID &ThisGrid, int Node1,
                                 int Node2, int Node3, int Node4);    
                                 
    int BadTriangle_(VSP_GRID &ThisGrid, int Loop, int &MinEdge);
    
    int BadQuad_(VSP_GRID &ThisGrid, int Loop, int LoopList[2]);
    
    double CalculateAspectRatioOld_(VSP_GRID &ThisGrid, int Loop);
    
    int LoopsAreCoplanar_(VSP_GRID &ThisGrid, int Loop1, int Loop2, double MaxAngle);
    
    int EdgesAreColinear_(VSP_GRID &ThisGrid, int Edge1, int Edge2, double MaxAngle);

    int MergedLoopsAreConvex_(VSP_GRID &ThisGrid, int Loop1, int Loop2, double MaxAngle);
    
    double CalculateLoopAngle_(VSP_GRID &ThisGrid, int Loop, int Node);
    
    void CheckLoopTopology_(VSP_GRID &ThisGrid);
    
    void CleanUpSingularFans_(void);
       
    // Initialization
    
    void Initialize_(void);

    // Fine grid
    
    VSP_GRID &FineGrid(void) { return *FineGrid_; };
    
    // Coarse grid
    
    VSP_GRID &CoarseGrid(void) { return *CoarseGrid_; };

    // Merge high aspect ratio quads

    VSP_GRID* MergeHighAspectRatioQuads(VSP_GRID &Grid) { return MergeHighAspectRatioQuads_(Grid); };
        
public:

    // Constructor, Destructor, Copy
    
    VSP_AGGLOM(void);
   ~VSP_AGGLOM(void);
    VSP_AGGLOM(const VSP_AGGLOM &agglom);

    /** Agglomerate the input grid to create a coarse grid... and return that coarse grid **/
    
    VSP_GRID* Agglomerate(VSP_GRID &Grid) { return Agglomerate_(Grid); };    
    
    /** Simplify the input mesh... this merges tris into quads and for panel solves it attempts
     * merge splinter tris at intersections into general polygons **/
    
    VSP_GRID* SimplifyMesh(VSP_GRID &Grid) { return SimplifyMesh_(Grid); };

    VSP_GRID* SimplifyMesh(VSP_GRID &Grid, VSP_GRID &GridC) { return SimplifyMesh_(Grid, GridC); };

};

#include "END_NAME_SPACE.H"

#endif
