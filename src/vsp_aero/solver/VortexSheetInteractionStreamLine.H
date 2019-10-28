//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY_H
#define VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.H"
#include "VSP_Geom.H"
#include "Interaction.H"


// Small class for vortex sheet to loop interaction

class VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY {

private:

    int Level_;
    int Trail_;
    int Edge_;

    int NumberOfEdges_;
        
public:

    VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY(void);
   ~VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY(void);
    VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY(const VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY &StreamlineInteractionEntry);

    VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY& operator=(const VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY &StreamlineInteractionEntry);
    
    void SizeList(int NumberOfEdges);
    
    void DeleteList(void);
    
    void UseList( int NumberOfEdges, VORTEX_STREAMLINE_ENTRY *VortexStreamlineList);
    
    int &Level(void) { return Level_; };
    
    int &Edge(void) { return Edge_; };

    int NumberOfEdges(void) { return NumberOfEdges_; };
    
    VORTEX_STREAMLINE_ENTRY *VortexStreamLineList_;

    VORTEX_STREAMLINE_ENTRY &VortexStreamLineList(int i) { return VortexStreamLineList_[i]; };
    
};

#endif

