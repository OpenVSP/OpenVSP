//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VortexSheetVortex_To_VortexInteractionSet.H"

/*##############################################################################
#                                                                              #
#       VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET Constructor              #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET::VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET(void)
{

    // Just zero things out

    NumberOfSets_ = 0;
    
    Vortex_w = NULL;
    
    TrailingVortex_t = NULL;
        
    NumberOfVortexSheetInteractionEdges_ = NULL;
        
    VortexSheetInteractionTrailingVortexList_ = NULL;
    
}

/*##############################################################################
#                                                                              #
#         VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET Destructor             #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET::~VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET(void)
{

    int i;
    
    if ( Vortex_w != NULL ) delete [] Vortex_w;
    
    if ( TrailingVortex_t != NULL ) delete [] TrailingVortex_t;
    
    if ( NumberOfVortexSheetInteractionEdges_ != NULL ) delete [] NumberOfVortexSheetInteractionEdges_;

    if ( VortexSheetInteractionTrailingVortexList_ != NULL ) {
       
       for ( i = 1 ; i <= NumberOfSets_ ; i++ ) {
          
          delete [] VortexSheetInteractionTrailingVortexList_[i];
          
       }
       
       delete [] VortexSheetInteractionTrailingVortexList_;
       
    }
    
    Vortex_w = NULL;
    
    TrailingVortex_t = NULL;
    
    NumberOfVortexSheetInteractionEdges_ = NULL;
    
    VortexSheetInteractionTrailingVortexList_ = NULL;
   
}

/*##############################################################################
#                                                                              #
#          VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET SizeList              #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET::SizeList(int NumberOfSets)
{

    NumberOfSets_ = NumberOfSets;
        
    Vortex_w = new int[NumberOfSets_ + 1];
    
    TrailingVortex_t = new int[NumberOfSets_ + 1];

    NumberOfVortexSheetInteractionEdges_ = new int[NumberOfSets_ + 1];

    VortexSheetInteractionTrailingVortexList_ = new VORTEX_SHEET_LOOP_INTERACTION_ENTRY*[NumberOfSets_ + 1];

}

/*##############################################################################
#                                                                              #
#             VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET Copy               #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET::VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET(const VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET &VortexSheetVortexToVortexSet)
{

    // Not implemented
    
    PRINTF("VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET Copy not implemented! \n");
    
    fflush(NULL);
    
    exit(1);
    
}


