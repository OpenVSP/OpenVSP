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
    
    /** Size the vortex sheet interlaction list **/
    
    void SizeList(int NumberOfVortexSheets);
    
    /** Delete the list data **/
    
    void DeleteList(void);
    
    /** Pass in a pointer to an existing list and just use it... **/
    
    void UseList( int NumberOfVortexSheets, VORTEX_SHEET_ENTRY *VortexSheetList);
    
    /** Multipole level of this interaction list **/
    
    int &Level(void) { return Level_; };
    
    /** Loop we are evaluating for with this interaction list ... note that internally this is 
     * also used to store the edge we may be evaluating for... choose one! **/
    
    int &Loop(void) { return Loop_; };
    
    /** Edge we are evaluating for with this interaction list ... note that internally this is 
     * also used to store the loop we may be evaluating for... choose one! **/
        
    int &Edge(void) { return Loop_; }; // Doubles as Edge storage... depends on context
    
    /** Trailing vortex we are evaluating for **/
    
    int &TrailingVortex(void) { return TrailingVortex_; };
    
    /** Number of vortex sheets in this list **/

    int NumberOfVortexSheets(void) { return NumberOfVortexSheets_; };
    
    /** Pointer to the vortex sheet list **/
    
    VORTEX_SHEET_ENTRY *VortexSheetList_;
    
    /** Access to the i'th vortex sheet in the list **/

    VORTEX_SHEET_ENTRY &VortexSheetList(int i) { return VortexSheetList_[i]; };
    
};

#include "END_NAME_SPACE.H"

#endif

