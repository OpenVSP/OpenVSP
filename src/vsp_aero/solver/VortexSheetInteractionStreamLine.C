//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VortexSheetInteractionStreamLine.H"

/*##############################################################################
#                                                                              #
#            VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY Constructor             #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY::VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY(void)
{

    // Just zero things out
    
    Level_ = 0;
    
    Edge_ = 0;

    NumberOfEdges_ = 0;
    
    VortexStreamLineList_ = NULL;

}

/*##############################################################################
#                                                                              #
#             VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY Destructor             #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY::~VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY(void)
{

    if ( VortexStreamLineList_ != NULL ) {
       
       delete [] VortexStreamLineList_;
       
    }
    
    VortexStreamLineList_ = NULL;
    
    Level_ = 0;
    
    Edge_ = 0;

    NumberOfEdges_ = 0;
    
}

/*##############################################################################
#                                                                              #
#          VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY SizeList                  #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY::SizeList(int NumberOfEdges)
{

    if ( VortexStreamLineList_ != NULL ) {
       
       delete [] VortexStreamLineList_;
       
    }
    
    VortexStreamLineList_ = NULL;
    
    NumberOfEdges_ = NumberOfEdges;
    
    VortexStreamLineList_ = new VORTEX_STREAMLINE_ENTRY[NumberOfEdges_ + 1];

}

/*##############################################################################
#                                                                              #
#             VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY DeleteList             #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY::DeleteList(void)
{

    if ( VortexStreamLineList_ != NULL ) {
       
       delete [] VortexStreamLineList_;
       
    }
    
    VortexStreamLineList_ = NULL;
    
    NumberOfEdges_ = 0;    

}

/*##############################################################################
#                                                                              #
#                  VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY Copy              #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY::VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY(const VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY &StreamlineInteractionEntry)
{

    *this = StreamlineInteractionEntry;

}

/*##############################################################################
#                                                                              #
#              VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY Operator =            #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY &VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY::operator=(const VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY &StreamlineInteractionEntry)
{

    int i;
    
    Level_ =  StreamlineInteractionEntry.Level_;
    
    Edge_ = StreamlineInteractionEntry.Edge_;

    NumberOfEdges_ = StreamlineInteractionEntry.NumberOfEdges_;
    
    SizeList(NumberOfEdges_);
   
    // Copy contents of list
    
    for ( i = 1 ; i <= NumberOfEdges_ ; i++ ) {

       VortexStreamLineList_[i].Edge     = StreamlineInteractionEntry.VortexStreamLineList_[i].Edge;
       VortexStreamLineList_[i].Level    = StreamlineInteractionEntry.VortexStreamLineList_[i].Level;
       VortexStreamLineList_[i].EdgeID   = StreamlineInteractionEntry.VortexStreamLineList_[i].EdgeID;
       VortexStreamLineList_[i].Distance = StreamlineInteractionEntry.VortexStreamLineList_[i].Distance;

    }

    return *this;

}

/*##############################################################################
#                                                                              #
#             VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY UseList                #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET_STREAMLINE_INTERACTION_ENTRY::UseList(int NumberOfEdges, VORTEX_STREAMLINE_ENTRY *TempList)
{

    if ( VortexStreamLineList_ != NULL ) {
       
       delete [] VortexStreamLineList_;
       
    }
    
    VortexStreamLineList_ = NULL;
    
    NumberOfEdges_ = NumberOfEdges;

    VortexStreamLineList_= TempList;
    
}


