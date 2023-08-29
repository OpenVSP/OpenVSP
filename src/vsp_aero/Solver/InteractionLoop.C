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
    
    NumberOfVortexLoops_ = 0;
    
    SurfaceVortexLoopInteractionList_ = NULL;

}

/*##############################################################################
#                                                                              #
#                       LOOP_INTERACTION_ENTRY Destructor                      #
#                                                                              #
##############################################################################*/

LOOP_INTERACTION_ENTRY::~LOOP_INTERACTION_ENTRY(void)
{

    // Delete the edge list

    if ( SurfaceVortexEdgeInteractionList_ != NULL ) {
       
       delete [] SurfaceVortexEdgeInteractionList_;
       
    }
    
    SurfaceVortexEdgeInteractionList_ = NULL;
    
    NumberOfVortexEdges_ = 0;

    // Delete the loop list
    
    if ( SurfaceVortexLoopInteractionList_ != NULL ) {
       
       delete [] SurfaceVortexLoopInteractionList_;
       
    }
    
    SurfaceVortexLoopInteractionList_ = NULL;
    
    NumberOfVortexLoops_ = 0;
        
}

/*##############################################################################
#                                                                              #
#                      LOOP_INTERACTION_ENTRY SizeEdgeList                     #
#                                                                              #
##############################################################################*/

void LOOP_INTERACTION_ENTRY::SizeEdgeList(int NumberOfVortexEdges)
{

    int i;
    
    if ( SurfaceVortexEdgeInteractionList_ != NULL ) {
       
       delete [] SurfaceVortexEdgeInteractionList_;
       
    }
    
    NumberOfVortexEdges_ = NumberOfVortexEdges;
    
    SurfaceVortexEdgeInteractionList_ = new VSP_EDGE*[NumberOfVortexEdges_ + 1];
    
    for ( i = 1 ; i <= NumberOfVortexEdges_ ; i++ ) {
       
       SurfaceVortexEdgeInteractionList_[i] = NULL;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                    LOOP_INTERACTION_ENTRY DeleteEdgeList                     #
#                                                                              #
##############################################################################*/

void LOOP_INTERACTION_ENTRY::DeleteEdgeList(void)
{

    if ( SurfaceVortexEdgeInteractionList_ != NULL ) {
       
       delete [] SurfaceVortexEdgeInteractionList_;
       
    }
    
    SurfaceVortexEdgeInteractionList_ = NULL;
    
    NumberOfVortexEdges_ = 0;

}

/*##############################################################################
#                                                                              #
#                      LOOP_INTERACTION_ENTRY SizeLoopList                     #
#                                                                              #
##############################################################################*/

void LOOP_INTERACTION_ENTRY::SizeLoopList(int NumberOfVortexLoops)
{

    int i;
    
    if ( NumberOfVortexLoops_ > 0 && SurfaceVortexLoopInteractionList_ != NULL ) {
       
       delete [] SurfaceVortexLoopInteractionList_;
       
       SurfaceVortexLoopInteractionList_ = NULL;
       
    }
    
    NumberOfVortexLoops_ = NumberOfVortexLoops;
    
    SurfaceVortexLoopInteractionList_ = new VSP_LOOP*[NumberOfVortexLoops_ + 1];
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       SurfaceVortexLoopInteractionList_[i] = NULL;
       
    }

}

/*##############################################################################
#                                                                              #
#                    LOOP_INTERACTION_ENTRY DeleteLoopList                     #
#                                                                              #
##############################################################################*/

void LOOP_INTERACTION_ENTRY::DeleteLoopList(void)
{

    if ( SurfaceVortexLoopInteractionList_ != NULL ) {
       
       delete [] SurfaceVortexLoopInteractionList_;
       
    }
    
    SurfaceVortexLoopInteractionList_ = NULL;
    
    NumberOfVortexLoops_ = 0;

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
    
    SizeEdgeList(NumberOfVortexEdges_);
    
    NumberOfVortexLoops_ = LoopInteractionEntry.NumberOfVortexLoops_;
    
    SizeLoopList(NumberOfVortexLoops_);
   
    // Copy contents of edge list
    
    for ( i = 1 ; i <= NumberOfVortexEdges_ ; i++ ) {

       SurfaceVortexEdgeInteractionList_[i] = LoopInteractionEntry.SurfaceVortexEdgeInteractionList_[i];

    }

    // Copy contents of loop list
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       SurfaceVortexLoopInteractionList_[i] = LoopInteractionEntry.SurfaceVortexLoopInteractionList_[i];

    }
    
    return *this;

}

/*##############################################################################
#                                                                              #
#                      LOOP_INTERACTION_ENTRY UseEdgeList                      #
#                                                                              #
##############################################################################*/

void LOOP_INTERACTION_ENTRY::UseEdgeList(int NumberOfVortexEdges, VSP_EDGE **List)
{

    if ( SurfaceVortexEdgeInteractionList_ != NULL ) {
       
       delete [] SurfaceVortexEdgeInteractionList_;
       
    }
    
    NumberOfVortexEdges_ = NumberOfVortexEdges;

    SurfaceVortexEdgeInteractionList_ = List;
    
}

/*##############################################################################
#                                                                              #
#                      LOOP_INTERACTION_ENTRY UseLoopList                      #
#                                                                              #
##############################################################################*/

void LOOP_INTERACTION_ENTRY::UseLoopList(int NumberOfVortexLoops, VSP_LOOP **List)
{

    if ( SurfaceVortexLoopInteractionList_ != NULL ) {
       
       delete [] SurfaceVortexLoopInteractionList_;
       
    }
        
    NumberOfVortexLoops_ = NumberOfVortexLoops;

    SurfaceVortexLoopInteractionList_ = List;
    
}

#include "END_NAME_SPACE.H"

