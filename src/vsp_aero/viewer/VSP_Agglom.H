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
#include "VSP_Node.H"
#include "VSP_Tris.H"
#include "VSP_Grid.H"

#define CORNER_BC        1
#define TE_EDGE_BC       2
#define LE_EDGE_BC       3
#define BOUNDARY_EDGE_BC 4
#define INTERIOR_EDGE_BC 5

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
    int *VortexLoopWasAgglomerated_;
    int *CoarseEdgeList_;    
    int *CoarseNodeList_;
    int NextBestEdgeOnFront_;

    // Agglomeration Routines
   
    void InitializeFront_(void);
    
    int NextAgglomerationEdge_(void);
    
    void UpdateFront_(void);
    
    void MergeVortexLoops_(void);
    
    void CleanUpLoneLoops_(void);
    
    void CreateCoarseMesh_(void);
           
    VSP_GRID* Agglomerate_(VSP_GRID &Grid);

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

};

#endif
