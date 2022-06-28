//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "QuadCell.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                               QUAD_CELL constructor                          #
#                                                                              #
##############################################################################*/

QUAD_CELL::QUAD_CELL(void)
{

    HasPoint_ = 0;
    
    Node_[0] = Node_[1] = Node_[2] = Node_[3] = 0;  
    
    Child_[0] = Child_[1] = Child_[2] = Child_[3] = 0;  

    Edge_[0] = Edge_[1] = Edge_[2] = Edge_[3] = 0;  

    Edge_[4] = Edge_[5] = Edge_[6] = Edge_[7] = 0;  

    Parent_ = 0;

    Level_ = 0;
    
    InsertedPoint_[0] = InsertedPoint_[1] = InsertedPoint_[2] = 0.;

}

/*##############################################################################
#                                                                              #
#                              QUAD_TREE destructor                            #
#                                                                              #
##############################################################################*/

QUAD_CELL::~QUAD_CELL(void)
{

    HasPoint_ = 0;
    
    Node_[0] = Node_[1] = Node_[2] = Node_[3] = 0;  
    
    Child_[0] = Child_[1] = Child_[2] = Child_[3] = 0;  

    Edge_[0] = Edge_[1] = Edge_[2] = Edge_[3] = 0;  

    Edge_[4] = Edge_[5] = Edge_[6] = Edge_[7] = 0;  

    Parent_ = 0;

    Level_ = 0;
    
    InsertedPoint_[0] = InsertedPoint_[1] = InsertedPoint_[2] = 0.;

}

/*##############################################################################
#                                                                              #
#                                QUAD_CELL Copy                                #
#                                                                              #
##############################################################################*/

QUAD_CELL::QUAD_CELL(const QUAD_CELL &QuadCell)
{
   
    PRINTF("Copy not implemented for QUAD_CELL class! \n");
    exit(1);
     
}

#include "END_NAME_SPACE.H"



