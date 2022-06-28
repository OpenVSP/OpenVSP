//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "InteractionLoop.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#               LOOP_INTERACTION_ENTRY Constructor                             #
#                                                                              #
##############################################################################*/

LOOP_INTERACTION_ENTRY::LOOP_INTERACTION_ENTRY(void)
{

    // Just zero things outs

    Level_ = 0;
    
    Loop_ = 0;

    NumberOfVortexEdges_ = 0;

    SurfaceVortexEdgeInteractionList_ = NULL;

}

/*##############################################################################
#                                                                              #
#                       LOOP_INTERACTION_ENTRY Destructor                      #
#                                                                              #
##############################################################################*/

LOOP_INTERACTION_ENTRY::~LOOP_INTERACTION_ENTRY(void)
{

    // Just zero the matrix

    if ( SurfaceVortexEdgeInteractionList_ != NULL ) {
       
       delete [] SurfaceVortexEdgeInteractionList_;
       
    }
    
    SurfaceVortexEdgeInteractionList_ = NULL;
    
    NumberOfVortexEdges_ = 0;
    
}

/*##############################################################################
#                                                                              #
#                      LOOP_INTERACTION_ENTRY SizeList                         #
#                                                                              #
##############################################################################*/

void LOOP_INTERACTION_ENTRY::SizeList(int NumberOfVortexEdges)
{

    if ( SurfaceVortexEdgeInteractionList_ != NULL ) {
       
       delete [] SurfaceVortexEdgeInteractionList_;
       
    }
    
    NumberOfVortexEdges_ = NumberOfVortexEdges;
    
    SurfaceVortexEdgeInteractionList_ = new VSP_EDGE*[NumberOfVortexEdges_ + 1];

}

/*##############################################################################
#                                                                              #
#                    LOOP_INTERACTION_ENTRY DeleteList                         #
#                                                                              #
##############################################################################*/

void LOOP_INTERACTION_ENTRY::DeleteList(void)
{

    if ( SurfaceVortexEdgeInteractionList_ != NULL ) {
       
       delete [] SurfaceVortexEdgeInteractionList_;
       
    }
    
    SurfaceVortexEdgeInteractionList_ = NULL;
    
    NumberOfVortexEdges_ = 0;

}

/*##############################################################################
#                                                                              #
#                      LOOP_INTERACTION_ENTRY Copy                             #
#                                                                              #
##############################################################################*/

LOOP_INTERACTION_ENTRY::LOOP_INTERACTION_ENTRY(const LOOP_INTERACTION_ENTRY &LoopInteractionEntry)
{

    *this = LoopInteractionEntry;

}

/*##############################################################################
#                                                                              #
#                LOOP_INTERACTION_ENTRY Operator =                             #
#                                                                              #
##############################################################################*/

LOOP_INTERACTION_ENTRY &LOOP_INTERACTION_ENTRY::operator=(const LOOP_INTERACTION_ENTRY &LoopInteractionEntry)
{

    int i;
    
    Level_ = LoopInteractionEntry.Level_;
    
    Loop_ = LoopInteractionEntry.Loop_;
    
    NumberOfVortexEdges_ = LoopInteractionEntry.NumberOfVortexEdges_;
    
    SizeList(NumberOfVortexEdges_);
   
    // Copy contents of list
    
    for ( i = 1 ; i <= NumberOfVortexEdges_ ; i++ ) {

       SurfaceVortexEdgeInteractionList_[i] = LoopInteractionEntry.SurfaceVortexEdgeInteractionList_[i];

    }

    return *this;

}

/*##############################################################################
#                                                                              #
#                      LOOP_INTERACTION_ENTRY UseList                          #
#                                                                              #
##############################################################################*/

void LOOP_INTERACTION_ENTRY::UseList(int NumberOfVortexEdges, VSP_EDGE **TempList)
{

    if ( SurfaceVortexEdgeInteractionList_ != NULL ) {
       
       delete [] SurfaceVortexEdgeInteractionList_;
       
    }
    
    NumberOfVortexEdges_ = NumberOfVortexEdges;

    SurfaceVortexEdgeInteractionList_= TempList;
    
}

#include "END_NAME_SPACE.H"

