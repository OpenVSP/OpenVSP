//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VORTEX_SHEET_LOOP_INTERACTION_ENTRY_H
#define VORTEX_SHEET_LOOP_INTERACTION_ENTRY_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.H"
#include "VSP_Geom.H"
#include "Interaction.H"

#include "START_NAME_SPACE.H"

// Small class for vortex sheet to loop interaction

class VORTEX_SHEET_LOOP_INTERACTION_ENTRY {

private:

    int Level_;
    int Loop_;
    int TrailingVortex_;

    int NumberOfVortexSheets_;
        
public:


    VORTEX_SHEET_LOOP_INTERACTION_ENTRY(void);
   ~VORTEX_SHEET_LOOP_INTERACTION_ENTRY(void);
    VORTEX_SHEET_LOOP_INTERACTION_ENTRY(const VORTEX_SHEET_LOOP_INTERACTION_ENTRY &LoopInteractionEntry);

    VORTEX_SHEET_LOOP_INTERACTION_ENTRY& operator=(const VORTEX_SHEET_LOOP_INTERACTION_ENTRY &LoopInteractionEntry);
    
    void SizeList(int NumberOfVortexSheets);
    
    void DeleteList(void);
    
    void UseList( int NumberOfVortexSheets, VORTEX_SHEET_ENTRY *VortexSheetList);
    
    int &Level(void) { return Level_; };
    
    int &Loop(void) { return Loop_; };
    
    int &Edge(void) { return Loop_; }; // Doubles as Edge storage... depends on context
    
    int &TrailingVortex(void) { return TrailingVortex_; };

    int NumberOfVortexSheets(void) { return NumberOfVortexSheets_; };
    
    VORTEX_SHEET_ENTRY *VortexSheetList_;

    VORTEX_SHEET_ENTRY &VortexSheetList(int i) { return VortexSheetList_[i]; };
    
};

#include "END_NAME_SPACE.H"

#endif

