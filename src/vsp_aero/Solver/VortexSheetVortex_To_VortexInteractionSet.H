//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET_H
#define VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.H"
#include "VortexSheetInteractionLoop.H"

#include "START_NAME_SPACE.H"

// Small class for vortex sheet to loop interaction

class VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET {

private:

    
    int NumberOfSets_;
    int *Vortex_w;
    int *TrailingVortex_t;
        
    int *NumberOfVortexSheetInteractionEdges_;
    
    VORTEX_SHEET_LOOP_INTERACTION_ENTRY **VortexSheetInteractionTrailingVortexList_;
    
public:

    VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET(void);
   ~VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET(void);
    VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET(const VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET &VortexSheetVortexToVortexSet);

    /** Size the list **/
    
    void SizeList(int NumberOfSets_);
    
    /** Number of sets in the list **/
    
    int NumberOfSets(void) { return NumberOfSets_; };

    /** Vortex sheet we are calculating the influence from **/
    
    int &VortexW(int i) { return Vortex_w[i]; };

    /** Trailing vortex we are calculating the influence of Vortex sheet W on **/
    
    int &TrailingVortexT(int i) { return TrailingVortex_t[i]; };
    
    /** Number of edges we are evaluating for **/
    
    int &NumberOfVortexSheetInteractionEdges(int i) { return NumberOfVortexSheetInteractionEdges_[i]; };
    
    /** List of trailing vortex edges... this includes the edge, and the level it is on ... as well as the vortex sheet list 
     * we are calculating the influence from **/
    
    VORTEX_SHEET_LOOP_INTERACTION_ENTRY*& VortexSheetInteractionTrailingVortexList(int i) { return VortexSheetInteractionTrailingVortexList_[i]; };
    
};

#include "END_NAME_SPACE.H"

#endif
