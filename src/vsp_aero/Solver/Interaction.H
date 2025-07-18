//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef INTERACTION_CLASSES_H
#define INTERACTION_CLASSES_H

#include "utils.H"
#include "InteractionLoop.H"
#include "START_NAME_SPACE.H"

// Class to store loop to edge, loop to loop interaction lists

class INTERACTION_LOOP_LIST {

public:
   
    int NumberOfInteractionLoops[2];   
     
    LOOP_INTERACTION_ENTRY *InteractionLoopList[2];
 
};

// Small class to return both the edge and loop lists

class INTERACTION_LISTS {

public:
   
    int NumberOfInteractionEdges;
    int NumberOfInteractionLoops;
    
    VSP_EDGE **EdgeInteractionList;    
    VSP_LOOP **LoopInteractionList;
   
};

// Loop entry class

class LOOP_ENTRY {

public:

    int NextEdge;
    int NumberOfVortexEdges;
    VSP_EDGE** Edge;
    
    int NextLoop;
    int NumberOfVortexLoops;
    VSP_LOOP** Loop;
        
};

// Small class for stack list

class STACK_ENTRY {

public:

    int Level;
    int Loop;

};

// Small class for list of vortex sheets and their level

class VORTEX_SHEET_ENTRY {

public:

   int Sheet;
   int Level;
   int SheetID;
   double Distance;

};

// Small class for list of vortex sheets and their level

class VORTEX_SHEET_LIST {

public:

   int NextSheet;
   int NumberOfSheets;
   VORTEX_SHEET_ENTRY *Sheet;
   
};

// Small class for list of vortex streamlines and their level

class VORTEX_STREAMLINE_ENTRY {

public:

   int Edge;
   int Level;
   int EdgeID;
   double Distance;
   
};

// Small class for list of vortex streamlines and their level

class VORTEX_STREAMLINE_LIST {

public:

   int NextEdge;
   int NumberOfEdges;
   VORTEX_STREAMLINE_ENTRY *Edge;
   
};

#include "END_NAME_SPACE.H"

#endif
