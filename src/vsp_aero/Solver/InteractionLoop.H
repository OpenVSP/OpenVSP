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
    
    VSP_EDGE **SurfaceVortexEdgeInteractionList_;
    
public:


    LOOP_INTERACTION_ENTRY(void);
   ~LOOP_INTERACTION_ENTRY(void);
    LOOP_INTERACTION_ENTRY(const LOOP_INTERACTION_ENTRY &LoopInteractionEntry);

    LOOP_INTERACTION_ENTRY& operator=(const LOOP_INTERACTION_ENTRY &LoopInteractionEntry);

    /** Size the list **/
        
    void SizeList(int NumberOfVortexEdges);
    
    /** Delete the list **/
    
    void DeleteList(void);
    
    /** Pass in pointer to an existing list... and well, use it ;-) **/
    
    void UseList(int NumberOfVortexEdges, VSP_EDGE **TempList);
    
    /** Mesh level this list corresponds to, ie in the full multipole we may be calculating 
     * induced velocities for a group of loops (agglomerated) at a coarse level as we 
     * have decided they are far enough away as a group to be considered a single evaulation
     * point... **/

    int &Level(void) { return Level_; };
    
    /** Loop this list evaluates for... again, this may be a coarse grid loop **/
    
    int &Loop(void) { return Loop_; };
    
    /** Number of vortex edges in this interaction list * */

    int NumberOfVortexEdges(void) { return NumberOfVortexEdges_; };
    
    /** Access the ith entry of the vortex interaction list **/

    VSP_EDGE *SurfaceVortexEdgeInteractionList(int i) { return SurfaceVortexEdgeInteractionList_[i]; };

    /** Pointer access to the full interaction list **/
        
    VSP_EDGE **SurfaceVortexEdgeInteractionList(void) { return SurfaceVortexEdgeInteractionList_; };
    
};

#include "END_NAME_SPACE.H"

#endif

