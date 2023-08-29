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

    // Forward
    
    long long int ForwardTotalHits_;
    long double ForwardSpeedRatio_;
    
    int MaxNumberOfForwardInteractionLoops_[2];    
    int    NumberOfForwardInteractionLoops_[2];

    LOOP_INTERACTION_ENTRY *ForwardInteractionList_[2];
    
    // Adjoint

    long long int AdjointTotalHits_;
    long double AdjointSpeedRatio_;
    
    int MaxNumberOfAdjointInteractionLoops_[2];
    int    NumberOfAdjointInteractionLoops_[2];

    LOOP_INTERACTION_ENTRY *AdjointInteractionList_[2];
      
public:

    FAST_MATRIX(void);
   ~FAST_MATRIX(void);
    FAST_MATRIX(const FAST_MATRIX &FastMatrix);

    FAST_MATRIX& operator=(const FAST_MATRIX &FastMatrix);

    /** Size the forward list **/
        
    void SizeForwardList(int LoopType, int NumberOfLoops);
    
    /** Size the adjoint list **/
    
    void SizeAdjointList(int LoopType, int NumberOfLoops);
    
    /** Delete the forward list **/
    
    void DeleteForwardList(int LoopType);
    
    /** Delete adjoint list **/
    
    void DeleteAdjointList(int LoopType);
    
    /** Pass in a pointer to an existing foward interaction list **/
    
    void UseForwardList(int LoopType, int NumberOfLoops, LOOP_INTERACTION_ENTRY *List);
    
    /** Pass in a pointer to an existing adjoint interaction list **/
    
    void UseAdjointList(int LoopType, int NumberOfLoops, LOOP_INTERACTION_ENTRY *List);
    
    /** Access to the Forward data number of loops **/
    
    int &NumberOfForwardInteractionLoops(int LoopType) { return NumberOfForwardInteractionLoops_[LoopType]; };
    
    /** Access to the Forward data list **/
    
    LOOP_INTERACTION_ENTRY *ForwardInteractionList(int LoopType) { return ForwardInteractionList_[LoopType]; };

    /** Access to the Adjoint data number of loops **/
    
    int &NumberOfAdjointInteractionLoops(int LoopType) { return NumberOfAdjointInteractionLoops_[LoopType]; };
    
    /** Access to the Adjoint data list **/
    
    LOOP_INTERACTION_ENTRY *AdjointInteractionList(int LoopType) { return AdjointInteractionList_[LoopType]; };
    
    /** Merge the forward list **/
    
    void MergeForwardList(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom, VSPAERO_DOUBLE Mach, VSPAERO_DOUBLE FarAway);
    
    /** Merge the adjoint list **/
    
    void MergeAdjointList(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom, VSPAERO_DOUBLE Mach, VSPAERO_DOUBLE FarAway);
        
    /** Total number of interactions for forward list **/
            
    long long int &ForwardTotalHits(void) { return ForwardTotalHits_; };
    
    /** Speed up for forward list **/
    
    long double &ForwardSpeedRatio(void) { return ForwardSpeedRatio_; };

    /** Total number of interactions for adjoint list **/
            
    long long int &AdjointTotalHits(void) { return AdjointTotalHits_; };
    
    /** Speed up for adjoint list **/
    
    long double &AdjointSpeedRatio(void) { return AdjointSpeedRatio_; };
                     
};

#include "END_NAME_SPACE.H"

#endif

