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

    void SizeList(int NumberOfSets_);
    
    int NumberOfSets(void) { return NumberOfSets_; };

    int &VortexW(int i) { return Vortex_w[i]; };

    int &TrailingVortexT(int i) { return TrailingVortex_t[i]; };
    
    int &NumberOfVortexSheetInteractionEdges(int i) { return NumberOfVortexSheetInteractionEdges_[i]; };
    
    VORTEX_SHEET_LOOP_INTERACTION_ENTRY*& VortexSheetInteractionTrailingVortexList(int i) { return VortexSheetInteractionTrailingVortexList_[i]; };
    
};

#include "END_NAME_SPACE.H"

#endif
