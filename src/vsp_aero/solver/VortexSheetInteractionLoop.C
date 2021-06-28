//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VortexSheetInteractionLoop.H"

/*##############################################################################
#                                                                              #
#            VORTEX_SHEET_LOOP_INTERACTION_ENTRY Constructor                   #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_LOOP_INTERACTION_ENTRY::VORTEX_SHEET_LOOP_INTERACTION_ENTRY(void)
{

    // Just zero things out

    Level_ = 0;
    
    Loop_ = 0;
    
    TrailingVortex_ = 0;

    NumberOfVortexSheets_ = 0;
    
    VortexSheetList_ = NULL;

}

/*##############################################################################
#                                                                              #
#             VORTEX_SHEET_LOOP_INTERACTION_ENTRY Destructor                   #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_LOOP_INTERACTION_ENTRY::~VORTEX_SHEET_LOOP_INTERACTION_ENTRY(void)
{

    if ( VortexSheetList_ != NULL ) {
       
       delete [] VortexSheetList_;
       
    }
    
    VortexSheetList_ = NULL;
    
    Level_ = 0;
    
    Loop_ = 0;
    
    TrailingVortex_ = 0;

    NumberOfVortexSheets_ = 0;
    
}

/*##############################################################################
#                                                                              #
#          VORTEX_SHEET_LOOP_INTERACTION_ENTRY SizeList                        #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET_LOOP_INTERACTION_ENTRY::SizeList(int NumberOfVortexSheets)
{

    if ( VortexSheetList_ != NULL ) {
       
       delete [] VortexSheetList_;
       
    }

    NumberOfVortexSheets_ = NumberOfVortexSheets;
    
    VortexSheetList_ = new VORTEX_SHEET_ENTRY[NumberOfVortexSheets_ + 1];

}

/*##############################################################################
#                                                                              #
#             VORTEX_SHEET_LOOP_INTERACTION_ENTRY DeleteList                   #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET_LOOP_INTERACTION_ENTRY::DeleteList(void)
{

    if ( VortexSheetList_ != NULL ) {
       
       delete [] VortexSheetList_;
       
    }
    
    VortexSheetList_ = NULL;
    
    Level_ = 0;
    
    Loop_ = 0;
    
    TrailingVortex_ = 0;

    NumberOfVortexSheets_ = 0;
 
}

/*##############################################################################
#                                                                              #
#                  VORTEX_SHEET_LOOP_INTERACTION_ENTRY Copy                    #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_LOOP_INTERACTION_ENTRY::VORTEX_SHEET_LOOP_INTERACTION_ENTRY(const VORTEX_SHEET_LOOP_INTERACTION_ENTRY &VortexLoopInteractionEntry)
{

    *this = VortexLoopInteractionEntry;

}

/*##############################################################################
#                                                                              #
#              VORTEX_SHEET_LOOP_INTERACTION_ENTRY Operator =                  #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_LOOP_INTERACTION_ENTRY &VORTEX_SHEET_LOOP_INTERACTION_ENTRY::operator=(const VORTEX_SHEET_LOOP_INTERACTION_ENTRY &VortexLoopInteractionEntry)
{

    int i;
    
    Level_ =  VortexLoopInteractionEntry.Level_;
    
    Loop_ = VortexLoopInteractionEntry.Loop_;
    
    TrailingVortex_ = VortexLoopInteractionEntry.TrailingVortex_;

    NumberOfVortexSheets_ = VortexLoopInteractionEntry.NumberOfVortexSheets_;
    
    SizeList(NumberOfVortexSheets_);
   
    // Copy contents of list
    
    for ( i = 1 ; i <= NumberOfVortexSheets_ ; i++ ) {

       VortexSheetList_[i].Sheet    = VortexLoopInteractionEntry.VortexSheetList_[i].Sheet;
       VortexSheetList_[i].Level    = VortexLoopInteractionEntry.VortexSheetList_[i].Level;
       VortexSheetList_[i].SheetID  = VortexLoopInteractionEntry.VortexSheetList_[i].SheetID;
       VortexSheetList_[i].Distance = VortexLoopInteractionEntry.VortexSheetList_[i].Distance;

    }

    return *this;

}

/*##############################################################################
#                                                                              #
#             VORTEX_SHEET_LOOP_INTERACTION_ENTRY UseList                      #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET_LOOP_INTERACTION_ENTRY::UseList(int NumberOfVortexSheets, VORTEX_SHEET_ENTRY *TempList)
{

    if ( VortexSheetList_ != NULL ) {
       
       delete [] VortexSheetList_;
       
    }

    NumberOfVortexSheets_ = NumberOfVortexSheets;

    VortexSheetList_= TempList;
    
}


