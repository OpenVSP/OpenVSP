//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "SaveState.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                              SAVE_STATE constructor                          #
#                                                                              #
##############################################################################*/

SAVE_STATE::SAVE_STATE(void)
{
    
    NumberOfNodes_      = 0;
    NumberOfGroups_     = 0;
    NumberOfLoops_      = 0;
    NumberOfKuttaEdges_ = 0;
    
    Time_ = 0.;

    xyz_[0] = NULL;
    xyz_[1] = NULL;
    xyz_[2] = NULL;     
       
    Group_ = NULL;
    
    Gamma_ = NULL;

    StallFactor_ = NULL;

    VortexStretchingRatio_ = NULL;
    
    VortexCircumference_ = NULL;
    
}

/*##############################################################################
#                                                                              #
#                                 SAVE_STATE Copy                              #
#                                                                              #
##############################################################################*/

SAVE_STATE::SAVE_STATE(const SAVE_STATE &SaveState)
{

    // Just use operator = code
    
    *this = SaveState;

}

/*##############################################################################
#                                                                              #
#                         SAVE_STATE SizeLists                                 #
#                                                                              #
##############################################################################*/

void SAVE_STATE::SizeLists(int NumberOfNodes, int NumberOfLoops, int NumberOfGroups, int NumberOfKuttaEdges)
{

    NumberOfNodes_      = NumberOfNodes;
    NumberOfGroups_     = NumberOfGroups; 
    NumberOfLoops_      = NumberOfLoops; 
    NumberOfKuttaEdges_ = NumberOfKuttaEdges; 

    xyz_[0] = new double[NumberOfNodes_ + 1];
    xyz_[1] = new double[NumberOfNodes_ + 1];
    xyz_[2] = new double[NumberOfNodes_ + 1];
    
    Group_ = new COMPONENT_GROUP[NumberOfGroups_ + 1];
    
    Gamma_ = new double[NumberOfLoops_ + 1];

    StallFactor_ = new double[NumberOfKuttaEdges_ + 1];

    VortexStretchingRatio_ = new double[NumberOfLoops_ + 1];
    
    VortexCircumference_ = new double[NumberOfLoops_ + 1];
    
}

/*##############################################################################
#                                                                              #
#                            SAVE_STATE operator=                              #
#                                                                              #
##############################################################################*/

SAVE_STATE& SAVE_STATE::operator=(const SAVE_STATE &SaveState)
{

    int i;

    if (  xyz_[0] != NULL ) delete []  xyz_[0];
    if (  xyz_[1] != NULL ) delete []  xyz_[1];
    if (  xyz_[2] != NULL ) delete []  xyz_[2];     
       
    if ( Group_ != NULL ) delete [] Group_;
    
    if ( Gamma_ != NULL ) delete [] Gamma_;
 
    if ( StallFactor_ != NULL ) delete [] StallFactor_;

    if ( VortexStretchingRatio_ != NULL ) delete [] VortexStretchingRatio_;
    
    if ( VortexCircumference_ != NULL ) delete [] VortexCircumference_;
    
    NumberOfNodes_      = SaveState.NumberOfNodes_;
    NumberOfGroups_     = SaveState.NumberOfGroups_; 
    NumberOfLoops_      = SaveState.NumberOfLoops_; 
    NumberOfKuttaEdges_ = SaveState.NumberOfKuttaEdges_; 

    xyz_[0] = new double[NumberOfNodes_ + 1];
    xyz_[1] = new double[NumberOfNodes_ + 1];
    xyz_[2] = new double[NumberOfNodes_ + 1];
    
    Group_ = new COMPONENT_GROUP[NumberOfGroups_ + 1];
    
    Gamma_ = new double[NumberOfLoops_ + 1];

    StallFactor_ = new double[NumberOfKuttaEdges_ + 1];

    VortexStretchingRatio_ = new double[NumberOfLoops_ + 1];
    
    VortexCircumference_ = new double[NumberOfLoops_ + 1];
    
    // Copy over mesh nodal data
    
    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {

        xyz_[0][i] = SaveState.xyz_[0][i];
        xyz_[1][i] = SaveState.xyz_[1][i];
        xyz_[2][i] = SaveState.xyz_[2][i];
    
    }

    // Copy over group data
    
    for ( i = 1 ; i <= NumberOfGroups_ ; i++ ) {
       
       Group_[i] = SaveState.Group_[i];
       
    }
    
    // Copy over gamma and vortex stretching data
    
    for ( i = 1 ; i <= NumberOfLoops_ ; i++ ) {
       
       Gamma_[i] = SaveState.Gamma_[i];

       VortexStretchingRatio_[i] = SaveState.VortexStretchingRatio_[i];
       
       VortexCircumference_[i] = SaveState.VortexCircumference_[i];
       
    }

    // Copy over stall factor data
    
    for ( i = 1 ; i <= NumberOfKuttaEdges_ ; i++ ) {
       
       StallFactor_[i] = SaveState.StallFactor_[i];

    }
                
    return *this;

}

/*##############################################################################
#                                                                              #
#                               SAVE_STATE destructor                          #
#                                                                              #
##############################################################################*/

SAVE_STATE::~SAVE_STATE(void)
{

    if (  xyz_[0] != NULL ) delete []  xyz_[0];
    if (  xyz_[1] != NULL ) delete []  xyz_[1];
    if (  xyz_[2] != NULL ) delete []  xyz_[2];     
    
    if ( Group_ != NULL ) delete [] Group_;
    
    if ( Gamma_ != NULL ) delete [] Gamma_;

    if ( StallFactor_ != NULL ) delete [] StallFactor_;

    if ( VortexStretchingRatio_ != NULL ) delete [] VortexStretchingRatio_;
    
    if ( VortexCircumference_ != NULL ) delete [] VortexCircumference_;
    
    NumberOfNodes_      = 0;
    NumberOfGroups_     = 0;
    NumberOfLoops_      = 0;
    NumberOfKuttaEdges_ = 0;

    xyz_[0] = NULL;
    xyz_[1] = NULL;
    xyz_[2] = NULL;    
        
    Group_ = NULL;
    
    Gamma_ = NULL;

    StallFactor_ = NULL;

    VortexStretchingRatio_ = NULL;
    
    VortexCircumference_ = NULL;
        
}

#include "END_NAME_SPACE.H"


