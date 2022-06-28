//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef LOOP_INTERACTION_ENTRY_H
#define LOOP_INTERACTION_ENTRY_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.H"
#include "VSP_Geom.H"

#include "START_NAME_SPACE.H"

// Small class for loop interaction

class LOOP_INTERACTION_ENTRY {

private:

    int Level_;
    int Loop_;

    int NumberOfVortexEdges_;
    
public:


    LOOP_INTERACTION_ENTRY(void);
   ~LOOP_INTERACTION_ENTRY(void);
    LOOP_INTERACTION_ENTRY(const LOOP_INTERACTION_ENTRY &LoopInteractionEntry);

    LOOP_INTERACTION_ENTRY& operator=(const LOOP_INTERACTION_ENTRY &LoopInteractionEntry);
    
    void SizeList(int NumberOfVortexEdges);
    
    void DeleteList(void);
    
    void UseList(int NumberOfVortexEdges, VSP_EDGE **TempList);

    int &Level(void) { return Level_; };
    
    int &Loop(void) { return Loop_; };

    int NumberOfVortexEdges(void) { return NumberOfVortexEdges_; };
    
    VSP_EDGE **SurfaceVortexEdgeInteractionList_;

    VSP_EDGE *SurfaceVortexEdgeInteractionList(int i) { return SurfaceVortexEdgeInteractionList_[i]; };
    
    VSP_EDGE **SurfaceVortexEdgeInteractionList(void) { return SurfaceVortexEdgeInteractionList_; };
    
};

#include "END_NAME_SPACE.H"

#endif

