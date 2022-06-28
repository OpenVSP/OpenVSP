//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "QuadNode.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                               QUAD_NODE constructor                          #
#                                                                              #
##############################################################################*/

QUAD_NODE::QUAD_NODE(void)
{

    xyz_[0] = xyz_[1] = xyz_[2] = 0.;

    velocity_[0] = velocity_[1] = velocity_[2] = 0.;
    
    Cp_ = 0.;
    
    InsideBody_ = 0;

}

/*##############################################################################
#                                                                              #
#                              QUAD_NODE destructor                            #
#                                                                              #
##############################################################################*/

QUAD_NODE::~QUAD_NODE(void)
{

    xyz_[0] = xyz_[1] = xyz_[2] = 0.;

    velocity_[0] = velocity_[1] = velocity_[2] = 0.;
    
    Cp_ = 0.;
    
    InsideBody_ = 0;
    
}

/*##############################################################################
#                                                                              #
#                                QUAD_NODE Copy                                #
#                                                                              #
##############################################################################*/

QUAD_NODE::QUAD_NODE(const QUAD_NODE &QuadNode)
{
   
    PRINTF("Copy not implemented for QUAD_CELL class! \n");
    exit(1);
     
}

#include "END_NAME_SPACE.H"


