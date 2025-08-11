//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "MeshGradient.H"

/*##############################################################################
#                                                                              #
#                         MESH_GRADIENT constructor                            #
#                                                                              #
##############################################################################*/

MESH_GRADIENT::MESH_GRADIENT(void)
{
   
    px_ = 0.;
    py_ = 0.;
    pz_ = 0.;

}

/*##############################################################################
#                                                                              #
#                                 MESH_GRADIENT Copy                           #
#                                                                              #
##############################################################################*/

MESH_GRADIENT::MESH_GRADIENT(const MESH_GRADIENT &Gradient)
{

    printf("Copy not implemented for MESH_GRADIENT! \n");

    exit(1);

}

/*##############################################################################
#                                                                              #
#                               MESH_GRADIENT destructor                       #
#                                                                              #
##############################################################################*/

MESH_GRADIENT::~MESH_GRADIENT(void)
{

   // Nothing to do...
}

/*##############################################################################
#                                                                              #
#                              MESH_GRADIENT Zero                              #
#                                                                              #
##############################################################################*/

void MESH_GRADIENT::Zero(void)
{

    px_ = 0.;
    py_ = 0.;
    pz_ = 0.;  
 
}
