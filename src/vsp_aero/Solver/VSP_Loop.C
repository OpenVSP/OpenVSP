//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Loop.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                              VSP_LOOP constructor                            #
#                                                                              #
##############################################################################*/

VSP_LOOP::VSP_LOOP(void)
{

    NumberOfNodes_ = 0;
    
    NumberOfEdges_ = 0;
    
    NumberOfFineGridLoops_ = 0;
    
    NodeList_ = NULL;
    
    UVNodeList_ = NULL;
  
    EdgeList_ = NULL;
    
    EdgeDirection_ = NULL;
    
    EdgeIsUpwind_ = NULL;
    
    EdgeUpwindWeight_ = NULL;
    
    FineGridLoopList_ = NULL;
    
    XYZc_[0] = XYZc_[1] = XYZc_[2] = 0.;

    Normal_[0] = Normal_[1] = Normal_[2] = 0.;

    UVc_[0] = UVc_[1] = 0.;

    Velocity_[0] = Velocity_[1] = Velocity_[2] = 0.;
    
    dVelocity_dGamma_[0] = dVelocity_dGamma_[1] = dVelocity_dGamma_[2] = 0.;

    dVelocity_dMesh_[0] = dVelocity_dMesh_[1] = dVelocity_dMesh_[2] = 0.;
    
    dVelocity_dMach_[0] = dVelocity_dMach_[1] = dVelocity_dMach_[2] = 0.;
    
    dVelocity_dStall_[0] = dVelocity_dStall_[1] = dVelocity_dStall_[2] = 0.;
    
    dVelocity_dRatio_[0] = dVelocity_dRatio_[1] = dVelocity_dRatio_[2] = 0.;
    
    Force_[0] = Force_[1] = Force_[2] = 0.;
    
    Area_ = 0.;
    
    Length_ = 0.;

    CentroidOffSet_ = 0.;
    
    RefLength_ = 0.;
    
    Circumference_[0] = Circumference_[1] = 0.;
    
    Gamma_ = 0.;
        
    DeltaGamma_ = 0.;
    
    StallFactor_ = 1.;

    DeltaStallFactor_ = 0.;
    
    VortexStretchingRatio_ = 1.;
    
    DeltaVortexStretchingRatio_ = 0.;      
         
    Psi_ = 0.;
    
    dCp_ = 0.;
    
    dCp_Unsteady_ = 0.;
    
    CoreWidth_ = 0.;
    
    KTFact_ = 1.;
    
    SurfaceType_ = 0;
    
    VortexSheetID_ = 0;

    SurfaceID_ = 0;

    ComponentID_ = 0;
        
    SpanStation_ = 0;
    
    LocalFreeStreamVelocity_[0] = 0.;
    LocalFreeStreamVelocity_[1] = 0.;
    LocalFreeStreamVelocity_[2] = 0.;
    LocalFreeStreamVelocity_[3] = 0.;
    LocalFreeStreamVelocity_[4] = 0.;

    CoarseGridLoop_        = 0;
    
    NumberOfFineGridLoops_ = 0;
    
    FineGridLoopList_      = NULL;
    
    Level_                 = 0;
   
    VortexLoop_            = 0;
    
    MGVortexLoop_          = 0;
    
    MinValidTimeStep_      = 0;
    
    UpwindWakeLoop_        = 0;
    
     pForces_pGamma_[0] = 0.;
     pForces_pGamma_[1] = 0.;
     pForces_pGamma_[2] = 0.;
     pForces_pGamma_[3] = 0.;
     pForces_pGamma_[4] = 0.;
     pForces_pGamma_[5] = 0.;
     pForces_pGamma_[6] = 0.;
     pForces_pGamma_[7] = 0.;
     pForces_pGamma_[8] = 0.;
          
    pMoments_pGamma_[0] = 0.;
    pMoments_pGamma_[1] = 0.;
    pMoments_pGamma_[2] = 0.;
    pMoments_pGamma_[3] = 0.;
    pMoments_pGamma_[4] = 0.;
    pMoments_pGamma_[5] = 0.;

     pForces_pRatio_[0] = 0.;
     pForces_pRatio_[1] = 0.;
     pForces_pRatio_[2] = 0.;
     pForces_pRatio_[3] = 0.;
     pForces_pRatio_[4] = 0.;
     pForces_pRatio_[5] = 0.;
     pForces_pRatio_[6] = 0.;
     pForces_pRatio_[7] = 0.;
     pForces_pRatio_[8] = 0.;
                        
    pMoments_pRatio_[0] = 0.;
    pMoments_pRatio_[1] = 0.;
    pMoments_pRatio_[2] = 0.;
    pMoments_pRatio_[3] = 0.;
    pMoments_pRatio_[4] = 0.;
    pMoments_pRatio_[5] = 0.;         
          
}

/*##############################################################################
#                                                                              #
#                                 VSP_LOOP Copy                                #
#                                                                              #
##############################################################################*/

VSP_LOOP::VSP_LOOP(const VSP_LOOP &VSPTri)
{

    printf("Copy not implemented for VSP_NODE! \n");

    exit(1);

}

/*##############################################################################
#                                                                              #
#                                 VSP_NODE Copy                                #
#                                                                              #
##############################################################################*/

VSP_LOOP& VSP_LOOP::operator=(const VSP_LOOP &VSPTri)
{

    int i;
    
    // Copy over node list
 
    NumberOfNodes_ = VSPTri.NumberOfNodes_;
 
    SizeNodeList(NumberOfNodes_);
    
    for ( i = 0 ; i < NumberOfNodes_ ; i++ ) {
     
       NodeList_[i] = VSPTri.NodeList_[i];

    }

    for ( i = 0 ; i < 2*NumberOfNodes_ ; i++ ) {

       UVNodeList_[i] = VSPTri.UVNodeList_[i];
       
    }        
    
    // Copy over edge list
 
    NumberOfEdges_ = VSPTri.NumberOfEdges_;
 
    SizeEdgeList(NumberOfEdges_);
    
    for ( i = 0 ; i < NumberOfEdges_ ; i++ ) {
     
       EdgeList_[i] = VSPTri.EdgeList_[i];
       
       EdgeDirection_[i] = VSPTri.EdgeDirection_[i];
       
       EdgeIsUpwind_[i] = VSPTri.EdgeIsUpwind_[i];

       EdgeUpwindWeight_[i] = VSPTri.EdgeUpwindWeight_[i];
       
    }

    SurfaceID_                  = VSPTri.SurfaceID_;
    ComponentID_                = VSPTri.ComponentID_;
    OpenVSP_ComponentID_        = VSPTri.OpenVSP_ComponentID_;
    
    IsTrailingEdgeTri_          = VSPTri.IsTrailingEdgeTri_;
    VortexLoop_                 = VSPTri.VortexLoop_;
    MGVortexLoop_               = VSPTri.MGVortexLoop_;
    MinValidTimeStep_           = VSPTri.MinValidTimeStep_;
    UpwindWakeLoop_             = VSPTri.UpwindWakeLoop_;
                                
    SpanStation_                = VSPTri.SpanStation_;
                                
    Area_                       = VSPTri.Area_;
    Length_                     = VSPTri.Length_;
    RefLength_                  = VSPTri.RefLength_;
    CentroidOffSet_             = VSPTri.CentroidOffSet_;
                                
    Normal_[0]                  = VSPTri.Normal_[0];
    Normal_[1]                  = VSPTri.Normal_[1];
    Normal_[2]                  = VSPTri.Normal_[2]; 
                                
    XYZc_[0]                    = VSPTri.XYZc_[0];
    XYZc_[1]                    = VSPTri.XYZc_[1];
    XYZc_[2]                    = VSPTri.XYZc_[2];
                                
    UVc_[0]                     = VSPTri.UVc_[0];
    UVc_[1]                     = VSPTri.UVc_[1];

    Gamma_                      = VSPTri.Gamma_;
    
    DeltaGamma_                 = VSPTri.DeltaGamma_;

    StallFactor_                = VSPTri.StallFactor_;
    DeltaStallFactor_           = VSPTri.DeltaStallFactor_;
    
    VortexStretchingRatio_      = VSPTri.VortexStretchingRatio_;
    DeltaVortexStretchingRatio_ = VSPTri.DeltaVortexStretchingRatio_;         
    
    Psi_                        = VSPTri.Psi_;
    
    dCp_                        = VSPTri.dCp_;
                                
    Velocity_[0]                = VSPTri.Velocity_[0];
    Velocity_[1]                = VSPTri.Velocity_[1];
    Velocity_[2]                = VSPTri.Velocity_[2];

    LocalFreeStreamVelocity_[0] = VSPTri.LocalFreeStreamVelocity_[0];
    LocalFreeStreamVelocity_[1] = VSPTri.LocalFreeStreamVelocity_[1];
    LocalFreeStreamVelocity_[2] = VSPTri.LocalFreeStreamVelocity_[2];
    LocalFreeStreamVelocity_[3] = VSPTri.LocalFreeStreamVelocity_[3];
    LocalFreeStreamVelocity_[4] = VSPTri.LocalFreeStreamVelocity_[4];
    
    Force_[0]                   = VSPTri.Force_[0];
    Force_[1]                   = VSPTri.Force_[1];
    Force_[2]                   = VSPTri.Force_[2];
                                  
    KTFact_                     = VSPTri.KTFact_;
                                
    SurfaceType_                = VSPTri.SurfaceType_;
                                
    VortexSheetID_              = VSPTri.VortexSheetID_;
                                
    CoarseGridLoop_             = VSPTri.CoarseGridLoop_;
                                
    NumberOfFineGridLoops_      = VSPTri.NumberOfFineGridLoops_;
                                
    Level_                      = VSPTri.Level_;
    
    SizeFineGridLoopList(NumberOfFineGridLoops_);
    
    for ( i = 0 ; i < NumberOfFineGridLoops_ ; i++ ) {
     
       FineGridLoopList_[i] = VSPTri.FineGridLoopList_[i];
       
    }
    
    BoundBox_ = VSPTri.BoundBox_;

     pForces_pGamma_[0] =  VSPTri.pForces_pGamma_[0];
     pForces_pGamma_[1] =  VSPTri.pForces_pGamma_[1];
     pForces_pGamma_[2] =  VSPTri.pForces_pGamma_[2];
                     
    pMoments_pGamma_[0] = VSPTri.pMoments_pGamma_[0];
    pMoments_pGamma_[1] = VSPTri.pMoments_pGamma_[1];
    pMoments_pGamma_[2] = VSPTri.pMoments_pGamma_[2];

     pForces_pRatio_[0] =  VSPTri.pForces_pRatio_[0];
     pForces_pRatio_[1] =  VSPTri.pForces_pRatio_[1];
     pForces_pRatio_[2] =  VSPTri.pForces_pRatio_[2];
             
    pMoments_pRatio_[0] = VSPTri.pMoments_pRatio_[0];
    pMoments_pRatio_[1] = VSPTri.pMoments_pRatio_[1];
    pMoments_pRatio_[2] = VSPTri.pMoments_pRatio_[2];
        
    return *this;          

}

/*##############################################################################
#                                                                              #
#                               VSP_LOOP destructor                            #
#                                                                              #
##############################################################################*/

VSP_LOOP::~VSP_LOOP(void)
{

    if ( NumberOfEdges_ != 0 ) {
       
       if ( EdgeList_         != NULL ) delete [] EdgeList_;
       if ( EdgeIsUpwind_     != NULL ) delete [] EdgeIsUpwind_;
       if ( EdgeDirection_    != NULL ) delete [] EdgeDirection_;
       if ( EdgeUpwindWeight_ != NULL ) delete [] EdgeUpwindWeight_;
       
       NumberOfEdges_ = 0;

       EdgeList_ = NULL;
       EdgeIsUpwind_ = NULL;
       EdgeDirection_ = NULL;
       EdgeUpwindWeight_ = NULL;
           
    }
    
    if ( NumberOfNodes_ != 0 ) {
       
       if ( NodeList_      != NULL ) delete [] NodeList_;
       if ( UVNodeList_    != NULL ) delete [] UVNodeList_;
       
       NumberOfNodes_ = 0;

       NodeList_ = NULL;
       UVNodeList_ = NULL;       
       
    }

}

/*##############################################################################
#                                                                              #
#                          VSP_GRID SizeNodeList                               #
#                                                                              #
##############################################################################*/

void VSP_LOOP::SizeNodeList(int NumberOfNodes)
{
    int i;
    
    // Delete any old list
    
    if ( NumberOfNodes_ != 0 && NodeList_ != NULL ) delete [] NodeList_;

    if ( NumberOfNodes_ != 0 && UVNodeList_ != NULL ) delete [] UVNodeList_;

    // Allocate space for list
    
    NumberOfNodes_ = NumberOfNodes;

    NodeList_ = new int[NumberOfNodes_ + 1];
    
    UVNodeList_ = new double[2*NumberOfNodes_ + 1];

    for ( i = 0 ; i <= NumberOfNodes_ ; i++ ) {
       
       NodeList_[i] = 0;

    }

    for ( i = 0 ; i <= 2*NumberOfNodes_ ; i++ ) {
      
       UVNodeList_[i] = 0.;
       
    }
    
}

/*##############################################################################
#                                                                              #
#                          VSP_GRID SizeEdgeList                               #
#                                                                              #
##############################################################################*/

void VSP_LOOP::SizeEdgeList(int NumberOfEdges)
{

    // Delete any old list
    
    if ( NumberOfEdges_ != 0 ) {
       
       if ( EdgeList_ != NULL ) delete [] EdgeList_;
       if ( EdgeList_ != NULL ) delete [] EdgeIsUpwind_;
       if ( EdgeList_ != NULL ) delete [] EdgeDirection_;
       if ( EdgeList_ != NULL ) delete [] EdgeUpwindWeight_;
    
    }
    
    // Allocate space for list
    
    NumberOfEdges_ = NumberOfEdges;

    EdgeList_ = new int[NumberOfEdges_ + 1];    
    EdgeIsUpwind_ = new int[NumberOfEdges_ + 1];
    EdgeDirection_ = new int[NumberOfEdges_ + 1];
    
    EdgeUpwindWeight_ = new double[NumberOfEdges_ + 1];
    
    zero_int_array(EdgeList_, NumberOfEdges_);
    zero_int_array(EdgeIsUpwind_, NumberOfEdges_);
    zero_int_array(EdgeDirection_, NumberOfEdges_);
    
    zero_double_array(EdgeUpwindWeight_, NumberOfEdges_);

}

/*##############################################################################
#                                                                              #
#                          VSP_GRID SizeFineGridLoopList                       #
#                                                                              #
##############################################################################*/

void VSP_LOOP::SizeFineGridLoopList(int NumberOfLoops)
{
   
    if ( NumberOfFineGridLoops_ != 0 ) delete [] FineGridLoopList_;
    
    NumberOfFineGridLoops_ = NumberOfLoops;

    FineGridLoopList_ = new int[NumberOfFineGridLoops_ + 1];

}

/*##############################################################################
#                                                                              #
#                      VSP_LOOP UpdateGeometryLocation                         #
#                                                                              #
##############################################################################*/

void VSP_LOOP::UpdateGeometryLocation(double *TVec, double *OVec, QUAT &Quat, QUAT &InvQuat)
{

    QUAT Vec;

    // Update centroid

    Vec(0) = XYZc_[0] - OVec[0];
    Vec(1) = XYZc_[1] - OVec[1];
    Vec(2) = XYZc_[2] - OVec[2];

    Vec = Quat * Vec * InvQuat;

    XYZc_[0] = Vec(0) + OVec[0] + TVec[0];
    XYZc_[1] = Vec(1) + OVec[1] + TVec[1];
    XYZc_[2] = Vec(2) + OVec[2] + TVec[2];    
    
    // Update normal
    
    Vec(0) = Normal_[0];
    Vec(1) = Normal_[1];
    Vec(2) = Normal_[2];

    Vec = Quat * Vec * InvQuat;
    
    Normal_[0] = Vec(0);
    Normal_[1] = Vec(1);
    Normal_[2] = Vec(2);
    
    // Update bounding box min
    
    Vec(0) = BoundBox_.x_min - OVec[0];
    Vec(1) = BoundBox_.y_min - OVec[1];
    Vec(2) = BoundBox_.z_min - OVec[2];

    Vec = Quat * Vec * InvQuat;

    BoundBox_.x_min = Vec(0) + OVec[0] + TVec[0];
    BoundBox_.y_min = Vec(1) + OVec[1] + TVec[1];
    BoundBox_.z_min = Vec(2) + OVec[2] + TVec[2];      
    
    // Update bounding box max
    
    Vec(0) = BoundBox_.x_max - OVec[0];
    Vec(1) = BoundBox_.y_max - OVec[1];
    Vec(2) = BoundBox_.z_max - OVec[2];

    Vec = Quat * Vec * InvQuat;

    BoundBox_.x_max = Vec(0) + OVec[0] + TVec[0];
    BoundBox_.y_max = Vec(1) + OVec[1] + TVec[1];
    BoundBox_.z_max = Vec(2) + OVec[2] + TVec[2];            

}


#include "END_NAME_SPACE.H"
