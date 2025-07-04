//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef FAST_MATRIX_H
#define FAST_MATRIX_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Interaction.H"
#include "InteractionLoop.H"
#include "VSP_Geom.H"
#include "VSPAERO_OMP.H"
#include "MergeSort.H"
#include "utils.H"

#include "START_NAME_SPACE.H"

#define ALL_LOOPS          1
#define RELATIVE_MOTION    2
#define NO_RELATIVE_MOTION 3

#define FIXED_LOOPS  0
#define MOVING_LOOPS 1

#define FORWARD_LIST 0
#define ADJOINT_LIST 1

// Small class for loop interaction

class FAST_MATRIX {

private:

    /************************** FORWARD ***********************/

    // Forward loop list
    
    long double ForwardSpeedRatio_;
    
    int MaxNumberOfForwardInteractionLoops_[2];    
    int    NumberOfForwardInteractionLoops_[2];

    LOOP_INTERACTION_ENTRY *ForwardInteractionLoopList_[2];
    
    // Forward edge list

    int MaxNumberOfForwardInteractionEdges_[2];    
    int    NumberOfForwardInteractionEdges_[2];

    LOOP_INTERACTION_ENTRY *ForwardInteractionEdgeList_[2];
        
    // Clean the forward edge list

    void CleanForwardEdgeList(int NumberOfThreads, int LoopType, int MaxInteractionEdges);
           
    /************************** ADJOINT ***********************/
    
    // Adjoint

    long double AdjointSpeedRatio_;
    
    int MaxNumberOfAdjointInteractionLoops_[2];
    int    NumberOfAdjointInteractionLoops_[2];

    LOOP_INTERACTION_ENTRY *AdjointInteractionLoopList_[2];

    // Adjoint edge list

    int MaxNumberOfAdjointInteractionEdges_[2];    
    int    NumberOfAdjointInteractionEdges_[2];

    LOOP_INTERACTION_ENTRY *AdjointInteractionEdgeList_[2];
        
    // Clean the adjoint edge list
    
    void CleanAdjointEdgeList(int LoopType);
          
public:

    FAST_MATRIX(void);
   ~FAST_MATRIX(void);
    FAST_MATRIX(const FAST_MATRIX &FastMatrix);

    FAST_MATRIX& operator=(const FAST_MATRIX &FastMatrix);

    /************************** FORWARD ***********************/

    /** Size the forward list **/
        
    void SizeForwardLoopList(int LoopType, int NumberOfLoops);
        
    /** Delete the forward list **/
    
    void DeleteForwardLoopList(int LoopType);
    
    /** Pass in a pointer to an existing foward interaction list **/
    
    void UseForwardLoopList(int LoopType, int NumberOfLoops, LOOP_INTERACTION_ENTRY *List);

    /** Merge the forward list **/
    
    void MergeForwardList(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom, double Mach, double FarAway);

    /** Access to the Forward data number of loops **/
    
    int &NumberOfForwardInteractionLoops(int LoopType) { return NumberOfForwardInteractionLoops_[LoopType]; };
    
    /** Access to the Forward data loop list **/
    
    LOOP_INTERACTION_ENTRY *ForwardInteractionLoopList(int LoopType) { return ForwardInteractionLoopList_[LoopType]; };
    
    /** Speed up for forward list **/
    
    long double ForwardSpeedRatio(void) { return ForwardSpeedRatio_; };

    /** Size the forward edge list **/

    void SizeForwardEdgeList(int LoopType, int NumberOfEdges);

    /** Delete the forward edge list **/

    void DeleteForwardEdgeList(int LoopType);

    /** Create the forward edge list based on the forward loop list **/
    
    void CreateForwardEdgeList(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom);

    /** Access to the Forward data number of edges **/
    
    int &NumberOfForwardInteractionEdges(int LoopType) { return NumberOfForwardInteractionEdges_[LoopType]; };
    
    /** Access to the Forward data edge list **/
    
    LOOP_INTERACTION_ENTRY *ForwardInteractionEdgeList(int LoopType) { return ForwardInteractionEdgeList_[LoopType]; };
                    
/************************** ADJOINT ***********************/
        
    /** Size the adjoint list **/
    
    void SizeAdjointLoopList(int LoopType, int NumberOfLoops);

    /** Delete adjoint list **/
    
    void DeleteAdjointLoopList(int LoopType);

    /** Pass in a pointer to an existing adjoint interaction list **/
    
    void UseAdjointLoopList(int LoopType, int NumberOfLoops, LOOP_INTERACTION_ENTRY *List);

    /** Merge the adjoint list **/
    
    void MergeAdjointList(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom, double Mach, double FarAway);

    /** Speed up for adjoint list **/
    
    long double &AdjointSpeedRatio(void) { return AdjointSpeedRatio_; };
             
    /** Access to the Adjoint data number of loops **/
    
    int &NumberOfAdjointInteractionLoops(int LoopType) { return NumberOfAdjointInteractionLoops_[LoopType]; };
    
    /** Access to the Adjoint data loop list **/
    
    LOOP_INTERACTION_ENTRY *AdjointInteractionLoopList(int LoopType) { return AdjointInteractionLoopList_[LoopType]; };

    /** Size the adjoint edge list **/

    void SizeAdjointEdgeList(int LoopType, int NumberOfEdges);

    /** Delete the adjoint edge list **/

    void DeleteAdjointEdgeList(int LoopType);
         
    /** Create the adjoint edge list based on the forward loop list **/
    
    void CreateAdjointEdgeList(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom);

    /** Access to the adjoint data number of edges **/
    
    int &NumberOfAdjointInteractionEdges(int LoopType) { return NumberOfForwardInteractionEdges_[LoopType]; };
    
    /** Access to the adjoint data edge list **/
    
    LOOP_INTERACTION_ENTRY *AdjointInteractionEdgeList(int LoopType) { return AdjointInteractionEdgeList_[LoopType]; };        
};

#include "END_NAME_SPACE.H"

#endif

