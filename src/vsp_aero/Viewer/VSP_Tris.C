//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Tris.H"

/*##############################################################################
#                                                                              #
#                              VSP_TRIS constructor                            #
#                                                                              #
##############################################################################*/

VSP_TRIS::VSP_TRIS(void)
{


}

/*##############################################################################
#                                                                              #
#                                 VSP_TRIS Copy                                #
#                                                                              #
##############################################################################*/

VSP_TRIS::VSP_TRIS(const VSP_TRIS &VSPTri)
{

    printf("Copy not implemented for VSP_NODE! \n");fflush(NULL);

    exit(1);

}

/*##############################################################################
#                                                                              #
#                                 VSP_NODE Copy                                #
#                                                                              #
##############################################################################*/

VSP_TRIS& VSP_TRIS::operator=(const VSP_TRIS &VSPTri)
{

    Node1_                       = VSPTri.Node1_;
    Node2_                       = VSPTri.Node2_;
    Node3_                       = VSPTri.Node3_;

    Edge1_                       = VSPTri.Edge1_;
    Edge2_                       = VSPTri.Edge2_;
    Edge3_                       = VSPTri.Edge3_;
    
    Edge1IsUpWind_               = VSPTri.Edge1IsUpWind_;
    Edge2IsUpWind_               = VSPTri.Edge2IsUpWind_;
    Edge3IsUpWind_               = VSPTri.Edge3IsUpWind_;

    SurfaceID_                   = VSPTri.SurfaceID_;
    IsTrailingEdgeTri_           = VSPTri.IsTrailingEdgeTri_;
    IsLeadingEdgeTri_            = VSPTri.IsLeadingEdgeTri_;
    VortexLoop_                  = VSPTri.VortexLoop_;
    
    SpanStation_                 = VSPTri.SpanStation_;

    Area_                        = VSPTri.Area_;
   
    Nx_                          = VSPTri.Nx_;
    Ny_                          = VSPTri.Ny_;
    Nz_                          = VSPTri.Nz_; 
  
    NxCamber_                    = VSPTri.NxCamber_;
    NyCamber_                    = VSPTri.NyCamber_;
    NzCamber_                    = VSPTri.NzCamber_;
   
    Xc_                          = VSPTri.Xc_;
    Yc_                          = VSPTri.Yc_;
    Zc_                          = VSPTri.Xc_;
   
    Edge1UpwindWeight_           = VSPTri.Edge1UpwindWeight_;
    Edge2UpwindWeight_           = VSPTri.Edge2UpwindWeight_;
    Edge3UpwindWeight_           = VSPTri.Edge3UpwindWeight_;
    
    Gamma_                       = VSPTri.Gamma_;
    U_                           = VSPTri.U_;
    V_                           = VSPTri.V_;
    W_                           = VSPTri.W_;
    dCp_                         = VSPTri.dCp_;

    return *this;
}

/*##############################################################################
#                                                                              #
#                               VSP_TRIS destructor                            #
#                                                                              #
##############################################################################*/

VSP_TRIS::~VSP_TRIS(void)
{
 


}
