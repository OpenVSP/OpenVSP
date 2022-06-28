//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "QuadEdge.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                               QUAD_EDGE constructor                          #
#                                                                              #
##############################################################################*/

QUAD_EDGE::QUAD_EDGE(void)
{

    Cell_[0] = Cell_[1] = 0.;

}

/*##############################################################################
#                                                                              #
#                              QUAD_EDGE destructor                            #
#                                                                              #
##############################################################################*/

QUAD_EDGE::~QUAD_EDGE(void)
{

    Cell_[0] = Cell_[1] = 0.;
   
}

/*##############################################################################
#                                                                              #
#                                QUAD_EDGE Copy                                #
#                                                                              #
##############################################################################*/

QUAD_EDGE::QUAD_EDGE(const QUAD_EDGE &QuadEdge)
{
   
    PRINTF("Copy not implemented for QUAD_EDGE class! \n");
    exit(1);
     
}

#include "END_NAME_SPACE.H"


