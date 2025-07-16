//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////
////////////

#include "VSP_Edge.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                      Allocate space for statics                              #
#                                                                              #
##############################################################################*/

    double VSP_EDGE::Tolerance_1_ = 1.e-7;
    double VSP_EDGE::Tolerance_2_ = 1.e-7 * 1.e-7;
    double VSP_EDGE::Tolerance_4_ = 1.e-7 * 1.e-7 * 1.e-7 * 1.e-7;

/*##############################################################################
#                                                                              #
#                              VSP_EDGE constructor                            #
#                                                                              #
##############################################################################*/

VSP_EDGE::VSP_EDGE(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                 VSP_EDGE init                                #
#                                                                              #
##############################################################################*/

void VSP_EDGE::init(void)
{
 
    Verbose_ = 1;
   
    Node1_ = 0;
    Node2_ = 0;

    Loop1_ = 0;
    Loop2_ = 0;
    
    LoopL_ = 0;
    LoopR_ = 0;

    VortexEdge_ = 0;
    
    SurfaceID_ = 0;
    ComponentID_ = 0;
    Group_ = 0;

    IsTrailingEdge_        = 0;
    IsBoundaryEdge_        = 0;
    IsIntersectionEdge_    = 0;
    IsWakeTrailingEdge_    = 0;
    IsConcaveTrailingEdge_ = 0;
    IsFrozenTrailingEdge_  = 0;
    IsSymmetryPlaneEdge_   = 0;    
    
    Node1_IsOnSymmetryPlane_ = 0;
    Node2_IsOnSymmetryPlane_ = 0;
    
    CoarseGridEdge_   = 0;
    FineGridEdge_     = 0;
    Level_            = 0;
    ThisEdge_         = 0;    
    TimeAccurate_     = 0;
    Time_             = 0;
    MinValidTimeStep_ = 0;    
 
    Length_ = 0.;
    
             Forces_[0] =              Forces_[1] =              Forces_[2] = 0.;
     Trefftz_Forces_[0] =      Trefftz_Forces_[1] =      Trefftz_Forces_[2] = 0.;
    Unsteady_Forces_[0] =     Unsteady_Forces_[1] =     Unsteady_Forces_[2] = 0.;
  
    Verbose_ = 0;
 
    X1_ = 0.;
    Y1_ = 0.;
    Z1_ = 0.;
    
    X2_ = 0.;
    Y2_ = 0.;
    Z2_ = 0.;

    XYZc_[0] = XYZc_[1] = XYZc_[2] = 0.;
    
    Vec_[0] = Vec_[1] = Vec_[2] = 0.;

    u_ = v_ = w_ = 0.;

    VortexLoopLIsDownWind_ = 0;
    VortexLoopRIsDownWind_ = 0;
    
    VortexLoopLDownWindWeight_ = 0.;
    VortexLoopRDownWindWeight_ = 0.;

    Gamma_ = 0.;   
    
    DeltaGamma_ = 0.;
    
    DeltaStallGamma_ = 0.;    
    
    DeltaVortexStretchingRatioGamma_ = 0.;
    
    CoreWidth_ = 0.;

    SuperSonicCoreWidth_ = 0.;

    Mach_ = -1.;
    
    Kappa_ = 1.;
       
    KTFact_ = 1.;  
    
    TrailingEdgeStallFactor_ = 0.;
    
    Normal_[0] = Normal_[1] = Normal_[2] = 0.;

    Velocity_[0] = Velocity_[1] = Velocity_[2] = 0.;

    DownWashVelocity_[0] = DownWashVelocity_[1] = DownWashVelocity_[2] = 0.;
    
    dVelocity_dGamma_[0] = dVelocity_dGamma_[1] = dVelocity_dGamma_[2] = 0.;

    dVelocity_dMesh_[0] = dVelocity_dMesh_[1] = dVelocity_dMesh_[2] = 0.;

    dVelocity_dMach_[0] = dVelocity_dMach_[1] = dVelocity_dMach_[2] = 0.;
    
    dVelocity_dStall_[0] = dVelocity_dStall_[1] = dVelocity_dStall_[2] = 0.;
    
    LocalFreeStreamVelocity_[0] = 0.;
    LocalFreeStreamVelocity_[1] = 0.;
    LocalFreeStreamVelocity_[2] = 0.;
    LocalFreeStreamVelocity_[3] = 0.;
    LocalFreeStreamVelocity_[4] = 0.;
    
    dxyz1_[0] = 0.;
    dxyz1_[1] = 0.;
    dxyz1_[2] = 0.;
    
    dxyz2_[0] = 0.;
    dxyz2_[1] = 0.;
    dxyz2_[2] = 0.;      
    
    IsWakeEdge_ = 0;  
    
    WakeEdgeData_ = NULL;

    NumberOfAdjointEdges_ = 0;
    
    AdjointEdgeList_ = NULL;

    AdjointEdgeFactor_ = NULL;

    VortexSheet_ = 0;
  
    KuttaNode_[0] = 0;

    KuttaNode_[1] = 0;
  
}

/*##############################################################################
#                                                                              #
#                                 VSP_EDGE Copy                                #
#                                                                              #
##############################################################################*/

VSP_EDGE::VSP_EDGE(const VSP_EDGE &VSPEdge)
{

     init();
     
     // Just use operator= code
    
    *this = VSPEdge;

}

/*##############################################################################
#                                                                              #
#                                 VSP_EDGE Operator =                          #
#                                                                              #
##############################################################################*/

VSP_EDGE& VSP_EDGE::operator=(const VSP_EDGE &VSPEdge)
{
 
    int i;
    
    // Nodal data
    
    Node1_          = VSPEdge.Node1_;         
    Node2_          = VSPEdge.Node2_;         

    // Loop data
    
    Loop1_          = VSPEdge.Loop1_;          
    Loop2_          = VSPEdge.Loop2_;          
    
    // Left/Right tri data
    
    LoopL_           = VSPEdge.LoopL_;
    LoopR_           = VSPEdge.LoopR_;
    
    // Upwind data
    
    VortexLoopLIsDownWind_ = VSPEdge.VortexLoopLIsDownWind_;
    VortexLoopRIsDownWind_ = VSPEdge.VortexLoopRIsDownWind_;
    
    VortexLoopLDownWindWeight_ = VSPEdge.VortexLoopLDownWindWeight_;
    VortexLoopRDownWindWeight_ = VSPEdge.VortexLoopRDownWindWeight_;
    
    // Vortex edge data
    
    VortexEdge_     = VSPEdge.VortexEdge_;    
    
    // Edge type
    
    IsTrailingEdge_        = VSPEdge.IsTrailingEdge_;
    IsBoundaryEdge_        = VSPEdge.IsBoundaryEdge_;
    IsIntersectionEdge_    = VSPEdge.IsIntersectionEdge_;
    IsWakeTrailingEdge_    = VSPEdge.IsWakeTrailingEdge_;
    IsConcaveTrailingEdge_ = VSPEdge.IsConcaveTrailingEdge_;
    IsFrozenTrailingEdge_  = VSPEdge.IsFrozenTrailingEdge_;
    IsSymmetryPlaneEdge_   = VSPEdge.IsSymmetryPlaneEdge_;

    Node1_IsOnSymmetryPlane_ = VSPEdge.Node1_IsOnSymmetryPlane_;
    Node2_IsOnSymmetryPlane_ = VSPEdge.Node2_IsOnSymmetryPlane_;
    
    ComponentID_            = VSPEdge.ComponentID_;
    SurfaceID_              = VSPEdge.SurfaceID_;    
    OpenVSP_ComponentID_    = VSPEdge.OpenVSP_ComponentID_;
    
    ThisEdge_         = VSPEdge.ThisEdge_;
    
    MinValidTimeStep_ = VSPEdge.MinValidTimeStep_; 
    
    // Multi-Grid stuff
    
    CoarseGridEdge_     = VSPEdge.CoarseGridEdge_;     
    FineGridEdge_       = VSPEdge.FineGridEdge_;     
    Level_              = VSPEdge.Level_;     

    // XYZ of end points
    
    X1_ = VSPEdge.X1_;
    Y1_ = VSPEdge.Y1_;
    Z1_ = VSPEdge.Z1_;

    X2_ = VSPEdge.X2_;
    Y2_ = VSPEdge.Y2_;
    Z2_ = VSPEdge.Z2_;
    
    // Center of filament

    XYZc_[0] = VSPEdge.XYZc_[0];
    XYZc_[1] = VSPEdge.XYZc_[1];
    XYZc_[2] = VSPEdge.XYZc_[2];
     
    // Unit vector in direction of edge
    
    Vec_[0] = VSPEdge.Vec_[0];
    Vec_[1] = VSPEdge.Vec_[1];
    Vec_[2] = VSPEdge.Vec_[2];
    
    u_ = VSPEdge.u_;
    v_ = VSPEdge.v_;
    w_ = VSPEdge.w_;

    Length_ = VSPEdge.Length_;

    Mach_ = VSPEdge.Mach_;
    
    Kappa_ = VSPEdge.Kappa_;
    
    // Tolerances
    
    Tolerance_1_ = VSPEdge.Tolerance_1_;
    Tolerance_2_ = VSPEdge.Tolerance_2_;
    Tolerance_4_ = VSPEdge.Tolerance_4_;

    // Circulation strength
    
    Gamma_ = VSPEdge.Gamma_;
    
    DeltaGamma_ = VSPEdge.DeltaGamma_;

    CoreWidth_ = VSPEdge.CoreWidth_;

    // KT correction
    
    KTFact_ = VSPEdge.KTFact_;
    
    // Trailing edge stall factor
    
    TrailingEdgeStallFactor_ = VSPEdge.TrailingEdgeStallFactor_;

    // Normal vector
    
    Normal_[0] = VSPEdge.Normal_[0];
    Normal_[1] = VSPEdge.Normal_[1];
    Normal_[2] = VSPEdge.Normal_[2];
    
    // Forces
    
    for ( i = 0 ; i <= 2 ; i++ ) {
       
                Forces_[i] = VSPEdge.Forces_[i];
        Trefftz_Forces_[i] = VSPEdge.Trefftz_Forces_[i];
       Unsteady_Forces_[i] = VSPEdge.Unsteady_Forces_[i];
       
    }

    Velocity_[0] = VSPEdge.Velocity_[0];
    Velocity_[1] = VSPEdge.Velocity_[1];
    Velocity_[2] = VSPEdge.Velocity_[2];

    DownWashVelocity_[0] = VSPEdge.DownWashVelocity_[0];
    DownWashVelocity_[1] = VSPEdge.DownWashVelocity_[1];
    DownWashVelocity_[2] = VSPEdge.DownWashVelocity_[2];

    LocalFreeStreamVelocity_[0] = VSPEdge.LocalFreeStreamVelocity_[0];
    LocalFreeStreamVelocity_[1] = VSPEdge.LocalFreeStreamVelocity_[1];
    LocalFreeStreamVelocity_[2] = VSPEdge.LocalFreeStreamVelocity_[2];
    LocalFreeStreamVelocity_[3] = VSPEdge.LocalFreeStreamVelocity_[3];
    LocalFreeStreamVelocity_[4] = VSPEdge.LocalFreeStreamVelocity_[4];
    
    Q_ = VSPEdge.Q_;
    
    IsWakeEdge_ = VSPEdge.IsWakeEdge_;
    
    VortexSheet_ = VSPEdge.VortexSheet_;
  
    KuttaNode_[0] = VSPEdge.KuttaNode_[0];

    KuttaNode_[1] = VSPEdge.KuttaNode_[1];

    if ( VSPEdge.IsWakeEdge_ ) {
   
       CreateWakeEdgeSpace();

       // 1st of two (possible) trailing edge loops this wake look extends from
       
       WakeEdgeData_->GlobalTrailingEdgeLoopL() = VSPEdge.WakeEdgeData_->GlobalTrailingEdgeLoopL();
       
       // 2nd of two (possible) trailing edge loops this wake look extends from
       
       WakeEdgeData_->GlobalTrailingEdgeLoopR() = VSPEdge.WakeEdgeData_->GlobalTrailingEdgeLoopR();
       
       // Trailing edge edge this wake look extends from
        
       WakeEdgeData_->GlobalTrailingEdgeEdge() = VSPEdge.WakeEdgeData_->GlobalTrailingEdgeEdge();
       
       // Wake residual equation number for the 'x' equation
       
       WakeEdgeData_->WakeResidualEquationNumberForX() = VSPEdge.WakeEdgeData_->WakeResidualEquationNumberForX();
       
       // Direction of this edge wrt wake
       
       WakeEdgeData_->Direction() = VSPEdge.WakeEdgeData_->Direction();
       
       // Length of this wake edge... this is fixed
           
       WakeEdgeData_->dS() = VSPEdge.WakeEdgeData_->dS();

    }
        
    return *this;
    
}

/*##############################################################################
#                                                                              #
#                               VSP_EDGE destructor                            #
#                                                                              #
##############################################################################*/

VSP_EDGE::~VSP_EDGE(void)
{

   if ( WakeEdgeData_ != NULL ) delete WakeEdgeData_;
   
   IsWakeEdge_ = 0;

}

/*##############################################################################
#                                                                              #
#                              VSP_EDGE Setup                                  #
#                                                                              #
##############################################################################*/

void VSP_EDGE::Setup_(VSP_NODE &Node1, VSP_NODE &Node2)
{

    // Get beginning and end points of the bound vortex

    X1_ = Node1.x();
    Y1_ = Node1.y();
    Z1_ = Node1.z();

    X2_ = Node2.x();
    Y2_ = Node2.y();
    Z2_ = Node2.z();

    // Unit vector in direction of edge
    
    Vec_[0] = X2_ - X1_;
    Vec_[1] = Y2_ - Y1_;
    Vec_[2] = Z2_ - Z1_;
    
    Length_ = sqrt(vector_dot(Vec_,Vec_));
    
    u_ = Vec_[0];
    v_ = Vec_[1];
    w_ = Vec_[2];
    
    Vec_[0] /= Length_;
    Vec_[1] /= Length_;
    Vec_[2] /= Length_;

    // Zero out forces
    
    Forces_[0] = Forces_[1] = Forces_[2] = 0.;

}

/*##############################################################################
#                                                                              #
#                              VSP_EDGE Setup                                  #
#                                                                              #
##############################################################################*/

void VSP_EDGE::StoreWakeLocation(void)
{

    if ( IsWakeEdge_ && WakeEdgeData_->dS() > 0. ) {

       if ( WakeEdgeData_->Direction() > 0 ) {
          
          X2m_ = X1_;
          Y2m_ = Y1_;
          Z2m_ = Z1_;

       }
              
       else {
          
          X1m_ = X2_;
          Y1m_ = Y2_;
          Z1m_ = Z2_;

       } 
       
    }         

}

/*##############################################################################
#                                                                              #
#                              VSP_EDGE Setup                                  #
#                                                                              #
##############################################################################*/

void VSP_EDGE::SetupGradients(double &dLength_dX1, 
                              double &dLength_dY1,
                              double &dLength_dZ1,
                              
                              double &dLength_dX2,
                              double &dLength_dY2,
                              double &dLength_dZ2,
                              
                              double &dVecX_dX1,
                              double &dVecX_dY1,
                              double &dVecX_dZ1,
                                                      
                              double &dVecY_dX1,
                              double &dVecY_dY1,
                              double &dVecY_dZ1,
                                                     
                              double &dVecZ_dX1,
                              double &dVecZ_dY1,
                              double &dVecZ_dZ1,
                                                     
                              double &dVecX_dX2,
                              double &dVecX_dY2,
                              double &dVecX_dZ2,
                                                     
                              double &dVecY_dX2,
                              double &dVecY_dY2,
                              double &dVecY_dZ2,
                                                     
                              double &dVecZ_dX2,
                              double &dVecZ_dY2,
                              double &dVecZ_dZ2)                             
{

    // Unit vector in direction of edge
    
    Vec_[0] = X2_ - X1_;
    Vec_[1] = Y2_ - Y1_;
    Vec_[2] = Z2_ - Z1_;
    
    // Gradients of Vec_
    
    dVecX_dX1 = -1.;
    dVecX_dY1 =  0.;
    dVecX_dZ1 =  0.;

    dVecY_dX1 =  0.;
    dVecY_dY1 = -1.;
    dVecY_dZ1 =  0.;
    
    dVecZ_dX1 =  0.;
    dVecZ_dY1 =  0.;
    dVecZ_dZ1 = -1.;
    
    dVecX_dX2 =  1.;
    dVecX_dY2 =  0.;
    dVecX_dZ2 =  0.;
          
    dVecY_dX2 =  0.;
    dVecY_dY2 =  1.;
    dVecY_dZ2 =  0.;
         
    dVecZ_dX2 =  0.;
    dVecZ_dY2 =  0.;
    dVecZ_dZ2 =  1.;
      
    Length_ = sqrt(vector_dot(Vec_,Vec_));
    
    dLength_dX1 = (Vec_[0]*dVecX_dX1 + Vec_[1]*dVecY_dX1 + Vec_[2]*dVecZ_dX1)/Length_; 
    dLength_dY1 = (Vec_[0]*dVecX_dY1 + Vec_[1]*dVecY_dY1 + Vec_[2]*dVecZ_dY1)/Length_;
    dLength_dZ1 = (Vec_[0]*dVecX_dZ1 + Vec_[1]*dVecY_dZ1 + Vec_[2]*dVecZ_dZ1)/Length_;
                                                                            
    dLength_dX2 = (Vec_[0]*dVecX_dX2 + Vec_[1]*dVecY_dX2 + Vec_[2]*dVecZ_dX2)/Length_;
    dLength_dY2 = (Vec_[0]*dVecX_dY2 + Vec_[1]*dVecY_dY2 + Vec_[2]*dVecZ_dY2)/Length_;
    dLength_dZ2 = (Vec_[0]*dVecX_dZ2 + Vec_[1]*dVecY_dZ2 + Vec_[2]*dVecZ_dZ2)/Length_;
        
    u_ = Vec_[0];
    v_ = Vec_[1];
    w_ = Vec_[2];
    
    Vec_[0] /= Length_;
    Vec_[1] /= Length_;
    Vec_[2] /= Length_;

    // Gradients of normalized Vec_

    dVecX_dX1 = dVecX_dX1/Length_ - u_ * dLength_dX1 / (Length_*Length_);
    dVecX_dY1 = dVecX_dY1/Length_ - u_ * dLength_dY1 / (Length_*Length_); 
    dVecX_dZ1 = dVecX_dZ1/Length_ - u_ * dLength_dZ1 / (Length_*Length_); 
                               
    dVecY_dX1 = dVecY_dX1/Length_ - v_ * dLength_dX1 / (Length_*Length_); 
    dVecY_dY1 = dVecY_dY1/Length_ - v_ * dLength_dY1 / (Length_*Length_); 
    dVecY_dZ1 = dVecY_dZ1/Length_ - v_ * dLength_dZ1 / (Length_*Length_); 
                                 
    dVecZ_dX1 = dVecZ_dX1/Length_ - w_ * dLength_dX1 / (Length_*Length_);  
    dVecZ_dY1 = dVecZ_dY1/Length_ - w_ * dLength_dY1 / (Length_*Length_);  
    dVecZ_dZ1 = dVecZ_dZ1/Length_ - w_ * dLength_dZ1 / (Length_*Length_);  
    
    dVecX_dX2 = dVecX_dX2/Length_ - u_ * dLength_dX2 / (Length_*Length_);
    dVecX_dY2 = dVecX_dY2/Length_ - u_ * dLength_dY2 / (Length_*Length_); 
    dVecX_dZ2 = dVecX_dZ2/Length_ - u_ * dLength_dZ2 / (Length_*Length_); 
                                                    
    dVecY_dX2 = dVecY_dX2/Length_ - v_ * dLength_dX2 / (Length_*Length_); 
    dVecY_dY2 = dVecY_dY2/Length_ - v_ * dLength_dY2 / (Length_*Length_); 
    dVecY_dZ2 = dVecY_dZ2/Length_ - v_ * dLength_dZ2 / (Length_*Length_); 
                                                  
    dVecZ_dX2 = dVecZ_dX2/Length_ - w_ * dLength_dX2 / (Length_*Length_);  
    dVecZ_dY2 = dVecZ_dY2/Length_ - w_ * dLength_dY2 / (Length_*Length_);  
    dVecZ_dZ2 = dVecZ_dZ2/Length_ - w_ * dLength_dZ2 / (Length_*Length_);  

}

/*##############################################################################
#                                                                              #
#                       VSP_EDGE SetTolerance                                  #
#                                                                              #
##############################################################################*/

void VSP_EDGE::SetTolerance(double Tolerance) 
{

    Tolerance_1_ = double(Tolerance);
    Tolerance_2_ = Tolerance_1_ * Tolerance_1_;
    Tolerance_4_ = Tolerance_2_ * Tolerance_2_;

}

/*##############################################################################
#                                                                              #
#                            VSP_EDGE SetMachNumber                            #
#                                                                              #
##############################################################################*/

void VSP_EDGE::SetMachNumber(double Mach) 
{

    Mach_ = Mach;

    if ( Mach_ < 1. ) {
   
       Kappa_ = 2.;
   
    }
   
     else {
   
       Kappa_ = 1.;
   
    }
    
}

/*##############################################################################
#                                                                              #
#                          VSP_EDGE InducedVelocity                            #
#                                                                              #
##############################################################################*/

void VSP_EDGE::InducedVelocity(double xyz_p[3], double q[3]) 
{

    BoundVortex(Gamma_, xyz_p, q);
    
}

/*##############################################################################
#                                                                              #
#                   VSP_EDGE dInducedVelocity_dGamma                           #
#                                                                              #
##############################################################################*/

void VSP_EDGE::dInducedVelocity_dGamma(double xyz_p[3], double q[3]) 
{

    BoundVortex(DeltaGamma_, xyz_p, q);

    // Test derivatives
    
//if (1&& ABS(Gamma_) > 0. ) {
//
//       printf("Resetting xyz_p... \n");
//       
//       printf("Length_: %f \n",Length_);
//              
////       xyz_p[0] = 0.5*(X1_ + X2_);
////       xyz_p[1] = 0.5*(Y1_ + Y2_);
////       xyz_p[2] = 0.5*(Z1_ + Z2_);
//
//       BoundVortex(Gamma_, xyz_p, q);
//       
//       printf("q: %f %f %f \n",q[0],q[1],q[2]);       
//          
//       double dq[3];
//       double x;
//       double dx;
//       double q2[3];
//       double dqdx[3];
//       
//       dx = 1.e-12;
//       
//       x = Z1_;
//       
//       Z1_ += dx;
//
//       // Unit vector in direction of edge
//       
//       Vec_[0] = X2_ - X1_;
//       Vec_[1] = Y2_ - Y1_;
//       Vec_[2] = Z2_ - Z1_;
//       
//       Length_ = sqrt(vector_dot(Vec_,Vec_));
//       
//       u_ = Vec_[0];
//       v_ = Vec_[1];
//       w_ = Vec_[2];
//       
//       Vec_[0] /= Length_;
//       Vec_[1] /= Length_;
//       Vec_[2] /= Length_;
//    
//    
//       BoundVortex(Gamma_, xyz_p, q2);
//       
//       dqdx[0] = (q2[0] - q[0])/dx;
//       dqdx[1] = (q2[1] - q[1])/dx;
//       dqdx[2] = (q2[2] - q[2])/dx;
//       
//       printf("FD: dqdx[0]: %f \n",dqdx[0]);
//       printf("FD: dqdx[1]: %f \n",dqdx[1]);
//       printf("FD: dqdx[2]: %f \n",dqdx[2]);
//
//       Z1_  = x;
//
//       // Unit vector in direction of edge
//       
//       Vec_[0] = X2_ - X1_;
//       Vec_[1] = Y2_ - Y1_;
//       Vec_[2] = Z2_ - Z1_;
//       
//       Length_ = sqrt(vector_dot(Vec_,Vec_));
//       
//       u_ = Vec_[0];
//       v_ = Vec_[1];
//       w_ = Vec_[2];
//       
//       Vec_[0] /= Length_;
//       Vec_[1] /= Length_;
//       Vec_[2] /= Length_;
//
//       dxyz1_[0] = 0.; dxyz2_[0] = 0.;
//       dxyz1_[1] = 0.; dxyz2_[1] = 0.;
//       dxyz1_[2] = 1.; dxyz2_[2] = 0.;
//
//       dInducedVelocity_dMesh(xyz_p, dq);
//
//       printf("Exact: dqdx[0]: %f \n",dq[0]);
//       printf("Exact: dqdx[1]: %f \n",dq[1]);
//       printf("Exact: dqdx[2]: %f \n",dq[2]);           
//    }
    
        
}

/*##############################################################################
#                                                                              #
#                        VSP_EDGE pVelocity_pGamma                             #
#                                                                              #
##############################################################################*/

void VSP_EDGE::pVelocity_pGamma(double xyz_p[3], double q[3]) 
{

    BoundVortex(1., xyz_p, q);
   
}

/*##############################################################################
#                                                                              #
#                   VSP_EDGE dInducedVelocity_dStallGamma                      #
#                                                                              #
##############################################################################*/

void VSP_EDGE::dInducedVelocity_dStallGamma(double xyz_p[3], double q[3]) 
{

    BoundVortex(DeltaStallGamma_, xyz_p, q);
     
}

/*##############################################################################
#                                                                              #
#                   VSP_EDGE dInducedVelocity_dVortexStretchingRatio           #
#                                                                              #
##############################################################################*/

void VSP_EDGE::dInducedVelocity_dVortexStretchingRatio(double xyz_p[3], double q[3]) 
{

    BoundVortex(DeltaVortexStretchingRatioGamma_, xyz_p, q);
     
}


/*##############################################################################
#                                                                              #
#                          VSP_EDGE BoundVortex                                #
#                                                                              #
##############################################################################*/

void VSP_EDGE::BoundVortex(double Gamma, double xyz_p[3], double q[3])
{

    if ( !TimeAccurate_ || !IsWakeEdge_ || Time_ >= MinValidTimeStep_ ) {

       double U2, U4;
       double V2, V4;
       double W2, W4;
       double Beta2, C_Gamma;
       double a, b, c, d, dx, dy, dz;
       double s1, s2, F, F1, F2, Test;
       double u,v,w;

       Beta2 = 1. - SQR(KTFact_*Mach_);
   
       // Constants
   
       dx = X1_ - xyz_p[0];
       dy = Y1_ - xyz_p[1];
       dz = Z1_ - xyz_p[2];
   
       // Integral constants
       
      // a = dx*dx + Beta2*( dy*dy + dz*dz );    
      // b = 2.*( u_*dx + Beta2*( v_*dy + w_*dz ) );
      // c = u_*u_ + Beta2 * ( v_*v_ + w_*w_ );
      // d = 4.*a*c - b*b;
   
       dx /= Length_;
       dy /= Length_;
       dz /= Length_;
               
       u = u_/Length_;
       v = v_/Length_;
       w = w_/Length_;
       
       a = dx*dx + Beta2*( dy*dy + dz*dz );    
       b = 2.*( u*dx + Beta2*( v*dy + w*dz ) );
       c = u*u + Beta2 * ( v*v + w*w );
       d = 4.*a*c - b*b;    
       
       a *= Length_*Length_;
       b *= Length_*Length_;
       c *= Length_*Length_;
       d *= Length_*Length_*Length_*Length_;
      
       dx *= Length_;
       dy *= Length_;
       dz *= Length_;
       
       // Leading coefficient for velocity integrals
       
       C_Gamma = Gamma * Beta2 / (2.*PI*Kappa_);
       
       // Determine integration limits
   
       s1 = 0.;
       s2 = 1.;
   
       if ( Mach_ < 1. || xyz_p[0] >= X1_ || xyz_p[0] >= X2_ ) {
        
          // F function evaluated at node 1
   
          //SuperSonicCoreWidth_ = 0.;
          //
          //if ( Xp >= X1_ && Mach_ > 1. && SQR(X1_-Xp) + Beta2*( SQR(Y1_-Yp) + SQR(Z1_-Zp) ) >= 0. ) {
          //
          //   Test = SQR(X1_-Xp)/(Beta2*( SQR(Y1_-Yp) + SQR(Z1_-Zp)));
          //
          //   Test = sqrt(ABS(Test));
          //
          ////   SuperSonicCoreWidth_ = ABS(X1_-Xp)*Length_*sqrt(ABS(d))*exp(-Test/0.3);
          //
          //}
          
          F1 = 0.;
   
          if ( Mach_ < 1. || ( xyz_p[0] > X1_ && SQR(X1_-xyz_p[0]) + Beta2*( SQR(Y1_-xyz_p[1]) + SQR(Z1_-xyz_p[2]) )/0.7 > 0. ) ) {
   
              F1 = Fint(a,b,c,d,s1);
          
          }
   
          // F function evaluated at node 2
          
          //SuperSonicCoreWidth_ = 0.;
          //
          //if ( Xp >= X2_ && Mach_ > 1. && SQR(X2_-Xp) + Beta2*( SQR(Y2_-Yp) + SQR(Z2_-Zp) ) >= 0. ) {
          //
          //   Test = SQR(X2_-Xp)/(Beta2*( SQR(Y2_-Yp) + SQR(Z2_-Zp)));
          //   
          //   Test = sqrt(ABS(Test));
          //
          // //  SuperSonicCoreWidth_ = ABS(X2_-Xp)*Length_*sqrt(ABS(d))*exp(-Test/0.3);
          //
          //}
       
          F2 = 0.;
          
          if ( Mach_ < 1. || ( xyz_p[0] > X2_ && SQR(X2_-xyz_p[0]) + Beta2*( SQR(Y2_-xyz_p[1]) + SQR(Z2_-xyz_p[2]) )/0.7 > 0. ) ) {
         
              F2 = Fint(a,b,c,d,s2);
     
          }
          
          // Evalulate integrals
          
          F = F2 - F1;
   
          // U Velocity
   
          U2 =  v_ *     dz * F;
          U4 =     -w_ * dy * F;     
          
          q[0] = -C_Gamma*(U2 + U4);
          
          // V Velocity
     
          V2 =  u_ *     dz * F;
          V4 =     -w_ * dx * F;
          
          q[1] =  C_Gamma*(V2 + V4);
          
          // W Velocity
          
          W2 =  u_ *     dy * F;
          W4 =     -v_ * dx * F;
          
          q[2] = -C_Gamma*(W2 + W4);
   
       }
       
       else {
        
          q[0] = q[1] = q[2] = 0.;
          
       }
       
       // Edge comes off concave edge, so we will force induced velocity to zero
       
       if ( IsConcaveTrailingEdge_ ) q[0] = q[1] = q[2] = 0.;

       if ( IsSymmetryPlaneEdge_ ) q[1] = 0.;
       
    //  double ratio = 450./sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2]);
    //  
    //  ratio = MIN(1.,ratio);
    //  
    //  q[0] *= ratio;
    //  q[1] *= ratio;
    //  q[2] *= ratio;
       
    }
    
    // Time accurate case... but this edge is on part of wake not yet evaluated
    
    else {
       
       q[0] = q[1] = q[2] = 0.;
       
    }       
    
}

/*##############################################################################
#                                                                              #
#                     VSP_EDGE dInducedVelocity_dMesh                          #
#                                                                              #
##############################################################################*/

void VSP_EDGE::dInducedVelocity_dMesh(double xyz_p[3], double dxyz_i[3], double q[3])
{

    double dq0_dx1;
    double dq0_dy1;
    double dq0_dz1;
    double dq0_dx2;
    double dq0_dy2;
    double dq0_dz2;
    
    double dq1_dx1;
    double dq1_dy1;
    double dq1_dz1;
    double dq1_dx2;
    double dq1_dy2;
    double dq1_dz2;

    double dq2_dx1;
    double dq2_dy1;
    double dq2_dz1;
    double dq2_dx2;
    double dq2_dy2;
    double dq2_dz2;

    double dq0_dxp;
    double dq0_dyp;
    double dq0_dzp;
                         
    double dq1_dxp;
    double dq1_dyp;
    double dq1_dzp;
                          
    double dq2_dxp;
    double dq2_dyp;
    double dq2_dzp;     

    double dq0_dMach;
    double dq1_dMach;
    double dq2_dMach;

    GradientBoundVortex(Gamma_, xyz_p,
                         dq0_dx1,
                         dq0_dy1,
                         dq0_dz1,
                         dq0_dx2,
                         dq0_dy2,
                         dq0_dz2,
                         
                         dq1_dx1,
                         dq1_dy1,
                         dq1_dz1,
                         dq1_dx2,
                         dq1_dy2,
                         dq1_dz2,
                         
                         dq2_dx1,
                         dq2_dy1,
                         dq2_dz1,
                         dq2_dx2,
                         dq2_dy2,
                         dq2_dz2,
                               
                         dq0_dxp,
                         dq0_dyp,
                         dq0_dzp,
                               
                         dq1_dxp,
                         dq1_dyp,
                         dq1_dzp,
                               
                         dq2_dxp,
                         dq2_dyp,
                         dq2_dzp,
                         
                         dq0_dMach,
                         dq1_dMach,
                         dq2_dMach);                                                       
                           

    // Calculate perturbation velocities per the input xyz perturbations at node 1 and 2
            
    q[0] = dq0_dx1 * dxyz1_[0] + dq0_dx2 * dxyz2_[0]
         + dq0_dy1 * dxyz1_[1] + dq0_dy2 * dxyz2_[1]
         + dq0_dz1 * dxyz1_[2] + dq0_dz2 * dxyz2_[2]
         + dq0_dxp * dxyz_i[0]
         + dq0_dyp * dxyz_i[1]
         + dq0_dzp * dxyz_i[2];
                                                                                                 
    q[1] = dq1_dx1 * dxyz1_[0] + dq1_dx2 * dxyz2_[0]
         + dq1_dy1 * dxyz1_[1] + dq1_dy2 * dxyz2_[1]
         + dq1_dz1 * dxyz1_[2] + dq1_dz2 * dxyz2_[2]
         + dq1_dxp * dxyz_i[0]
         + dq1_dyp * dxyz_i[1]
         + dq1_dzp * dxyz_i[2];
                                                                           
    q[2] = dq2_dx1 * dxyz1_[0] + dq2_dx2 * dxyz2_[0]
         + dq2_dy1 * dxyz1_[1] + dq2_dy2 * dxyz2_[1]
         + dq2_dz1 * dxyz1_[2] + dq2_dz2 * dxyz2_[2]
         + dq2_dxp * dxyz_i[0]
         + dq2_dyp * dxyz_i[1]
         + dq2_dzp * dxyz_i[2];

}

/*##############################################################################
#                                                                              #
#                     VSP_EDGE dInducedVelocity_dMach                          #
#                                                                              #
##############################################################################*/

void VSP_EDGE::dInducedVelocity_dMach(double xyz_p[3], double q[3])
{

    double dq0_dx1;
    double dq0_dy1;
    double dq0_dz1;
    double dq0_dx2;
    double dq0_dy2;
    double dq0_dz2;
    
    double dq1_dx1;
    double dq1_dy1;
    double dq1_dz1;
    double dq1_dx2;
    double dq1_dy2;
    double dq1_dz2;

    double dq2_dx1;
    double dq2_dy1;
    double dq2_dz1;
    double dq2_dx2;
    double dq2_dy2;
    double dq2_dz2;

    double dq0_dxp;
    double dq0_dyp;
    double dq0_dzp;
                         
    double dq1_dxp;
    double dq1_dyp;
    double dq1_dzp;
                          
    double dq2_dxp;
    double dq2_dyp;
    double dq2_dzp;     

    double dq0_dMach;
    double dq1_dMach;
    double dq2_dMach;
            
    GradientBoundVortex(Gamma_, xyz_p,
                        dq0_dx1,
                        dq0_dy1,
                        dq0_dz1,
                        dq0_dx2,
                        dq0_dy2,
                        dq0_dz2,
                        
                        dq1_dx1,
                        dq1_dy1,
                        dq1_dz1,
                        dq1_dx2,
                        dq1_dy2,
                        dq1_dz2,
                        
                        dq2_dx1,
                        dq2_dy1,
                        dq2_dz1,
                        dq2_dx2,
                        dq2_dy2,
                        dq2_dz2,
                               
                        dq0_dxp,
                        dq0_dyp,
                        dq0_dzp,
                               
                        dq1_dxp,
                        dq1_dyp,
                        dq1_dzp,
                               
                        dq2_dxp,
                        dq2_dyp,
                        dq2_dzp,
                        
                        dq0_dMach,
                        dq1_dMach,
                        dq2_dMach);                                                       
                        
    // Return gradient velocity wrt Mach
          
    q[0] = dq0_dMach;
    q[1] = dq1_dMach;
    q[2] = dq2_dMach;
    
    // djk // Test
    // djk 
    // djk double dMach, qo[3], qp[3], dq_dMach[3];
    // djk 
    // djk BoundVortex(Gamma_, xyz_p, qo);
    // djk 
    // djk dMach = 0.001;
    // djk 
    // djk Mach_ += dMach;
    // djk 
    // djk BoundVortex(Gamma_, xyz_p, qp);
    // djk 
    // djk Mach_ -= dMach;
    // djk     
    // djk dq_dMach[0] = ( qp[0] - qo[0] )/dMach;
    // djk dq_dMach[1] = ( qp[1] - qo[1] )/dMach;
    // djk dq_dMach[2] = ( qp[2] - qo[2] )/dMach;
    // djk 
    // djk printf("Exact: %f %f %f  ... FD: %f %f %f \n",
    // djk dq0_dMach,
    // djk dq1_dMach,
    // djk dq2_dMach,
    // djk dq_dMach[0],
    // djk dq_dMach[1],
    // djk dq_dMach[2]);
                                                       
}

/*##############################################################################
#                                                                              #
#                          VSP_EDGE GradientBoundVortex                        #
#                                                                              #
##############################################################################*/

void VSP_EDGE::GradientBoundVortex(double &Gamma, double xyz_p[3],
                                   double &dq0_dx1,
                                   double &dq0_dy1,
                                   double &dq0_dz1,
                                   double &dq0_dx2,
                                   double &dq0_dy2,
                                   double &dq0_dz2,
                                                  
                                   double &dq1_dx1,
                                   double &dq1_dy1,
                                   double &dq1_dz1,
                                   double &dq1_dx2,
                                   double &dq1_dy2,
                                   double &dq1_dz2,
                                                  
                                   double &dq2_dx1,
                                   double &dq2_dy1,
                                   double &dq2_dz1,
                                   double &dq2_dx2,
                                   double &dq2_dy2,
                                   double &dq2_dz2,
                                   
                                   double &dq0_dxp,
                                   double &dq0_dyp,
                                   double &dq0_dzp,
                                                      
                                   double &dq1_dxp,
                                   double &dq1_dyp,
                                   double &dq1_dzp,
                                                        
                                   double &dq2_dxp,
                                   double &dq2_dyp,
                                   double &dq2_dzp,
                                   
                                   double &dq0_dMach,
                                   double &dq1_dMach,
                                   double &dq2_dMach)   
                                   

{

    if ( !TimeAccurate_ || !IsWakeEdge_ || Time_ >= MinValidTimeStep_ ) {

       double U2, U4;
       double V2, V4;
       double W2, W4;
       double Beta2, C_Gamma;
       double a, b, c, d, dx, dy, dz;
       double s1, s2, F, F1, F2, Test;
       double q[3];
       double u,v,w;
    
       double du_dx1;
       double dv_dy1;
       double dw_dz1;
                            
       double du_dx2;
       double dv_dy2;
       double dw_dz2;     
   
       double ddx_dx1;
       double ddy_dy1;
       double ddz_dz1;
       
       double da_ddx;
       double da_ddy;
       double da_ddz;
                           
       double db_ddx;
       double db_ddy;
       double db_ddz;
       double db_du;
       double db_dv;
       double db_dw;
                           
       double dc_du;
       double dc_dv;
       double dc_dw;
                           
       double dd_da;
       double dd_db;
       double dd_dc;
   
       double dF1_da;
       double dF1_db;
       double dF1_dc;
       double dF1_dd;
       double dF1_ds;
   
       double dF2_da;
       double dF2_db;
       double dF2_dc;
       double dF2_dd;
       double dF2_ds;                        
   
       double dF_da;
       double dF_db;
       double dF_dc;
       double dF_dd;
       double dF_ds;
   
       double dU2_dv;
       double dU2_ddz;
       double dU2_dF;
                            
       double dU4_dw;
       double dU4_ddy;
       double dU4_dF;
   
       double dq0_dU2;
       double dq0_dU4;
       
       double dV2_du;
       double dV2_ddz;
       double dV2_dF;
                            
       double dV4_dw;
       double dV4_ddx;
       double dV4_dF;
   
       double dq1_dV2;
       double dq1_dV4;
   
       double dW2_du;
       double dW2_ddy;
       double dW2_dF;
                            
       double dW4_dv;
       double dW4_ddx;
       double dW4_dF;
    
       double dq2_dW2;
       double dq2_dW4;
   
       double ddx_dxp;
       double ddy_dyp;
       double ddz_dzp;
       
       double dBeta2_dMach;
       double da_dBeta2;
       double db_dBeta2;
       double dc_dBeta2;
       double dC_Gamma_dBeta2;
       double dq0_dC_Gamma;
       double dq1_dC_Gamma;
       double dq2_dC_Gamma;
                                          
       Beta2 = 1. - SQR(KTFact_*Mach_);
       
       dBeta2_dMach = -2.*KTFact_*KTFact_*Mach_;
  
       // Constants
   
       du_dx1 = -1.;
       dv_dy1 = -1.;
       dw_dz1 = -1.;
       
       du_dx2 = 1.;
       dv_dy2 = 1.;
       dw_dz2 = 1.;        
   
       dx = X1_ - xyz_p[0];
       dy = Y1_ - xyz_p[1];
       dz = Z1_ - xyz_p[2];
       
       ddx_dx1 = 1.;
       ddy_dy1 = 1.;
       ddz_dz1 = 1.;
       
       ddx_dxp = -1.;
       ddy_dyp = -1.;
       ddz_dzp = -1.;
   
       // Integral constants
       
   //    a = dx*dx + Beta2*( dy*dy + dz*dz );    
   //    b = 2.*( u_*dx + Beta2*( v_*dy + w_*dz ) );
   //    c = u_*u_ + Beta2 * ( v_*v_ + w_*w_ );
   //    d = 4.*a*c - b*b;
   
       // Scale the constants before computing...
       
       dx /= Length_;
       dy /= Length_;
       dz /= Length_;
               
       u = u_/Length_;
       v = v_/Length_;
       w = w_/Length_;
       
       a = dx*dx + Beta2*( dy*dy + dz*dz );    
       b = 2.*( u*dx + Beta2*( v*dy + w*dz ) );
       c = u*u + Beta2 * ( v*v + w*w );
       d = 4.*a*c - b*b;    
   
       // Unscale the constants
           
       a *= Length_*Length_;
       b *= Length_*Length_;
       c *= Length_*Length_;
       d *= Length_*Length_*Length_*Length_;
   
       dx *= Length_;
       dy *= Length_;
       dz *= Length_;
       
       // Derivatives...
           
       da_ddx = 2.*dx;
       da_ddy = 2.*Beta2*dy;
       da_ddz = 2.*Beta2*dz;
       
       db_ddx = 2.*u_;
       db_ddy = 2.*Beta2*v_;
       db_ddz = 2.*Beta2*w_;
       db_du  = 2.*dx;
       db_dv  = 2.*Beta2*dy;
       db_dw  = 2.*Beta2*dz;
       
       dc_du = 2.*u_;
       dc_dv = 2.*Beta2*v_;
       dc_dw = 2.*Beta2*w_;
       
       dd_da = 4.*c;
       dd_db = -2.*b;
       dd_dc = 4.*a;
   
       da_dBeta2 = ( dy*dy + dz*dz );
       db_dBeta2 = 2.*( v_*dy + w_*dz );
       dc_dBeta2 = ( v_*v_ + w_*w_ );
      
       // Leading coefficient for velocity integrals
       
       C_Gamma = Gamma * Beta2 / (2.*PI*Kappa_);
       
       dC_Gamma_dBeta2 = Gamma / (2.*PI*Kappa_);
       
       // Determine integration limits
   
       s1 = 0.;
       s2 = 1.;
   
       if ( Mach_ < 1. || xyz_p[0] >= X1_ || xyz_p[0] >= X2_ ) {
        
          // F function evaluated at node 1
   
          F1 = 0.;
   
          dF1_da = dF1_db = dF1_dc = dF1_dd = dF1_ds = 0.;          
   
          if ( Mach_ < 1. || ( xyz_p[0] > X1_ && SQR(X1_-xyz_p[0]) + Beta2*( SQR(Y1_-xyz_p[1]) + SQR(Z1_-xyz_p[2]) )/0.7 > 0. ) ) {
   
              F1 = GradientFint(a,b,c,d,s1,dF1_da,dF1_db,dF1_dc,dF1_dd,dF1_ds);
   
           // // test derivatives
           // 
           // double xx;
           // double delta;
           // double F3;
           // double Deriv;
           // double Error;
           // 
           // delta = 1.e-7;
           //            
           // xx = d + delta;
           // 
           // F3 = GradientFint(a,b,c,xx,s1,dF2_da,dF2_db,dF2_dc,dF2_dd,dF2_ds);
           // 
           // Deriv = (F3 - F1)/delta;
           // 
           // Error = log10(ABS(dF1_dd - Deriv));
           // 
           // printf("Exact: %f ... FD: %f ... Error: %f \n",dF1_dd, Deriv, Error);
                   
          }
   
          // F function evaluated at node 2
       
          F2 = 0.;
          
          dF2_da = dF2_db = dF2_dc = dF2_dd = dF2_ds = 0.;
          
          if ( Mach_ < 1. || ( xyz_p[0] > X2_ && SQR(X2_-xyz_p[0]) + Beta2*( SQR(Y2_-xyz_p[1]) + SQR(Z2_-xyz_p[2]) )/0.7 > 0. ) ) {
   
              F2 = GradientFint(a,b,c,d,s2,dF2_da,dF2_db,dF2_dc,dF2_dd,dF2_ds);
   
            // // test derivatives
            // 
            // double xx;
            // double delta;
            // double F3;
            // double Deriv;
            // double Error;
            // 
            // delta = 1.e-7;
            //            
            // xx = s2 + delta;
            // 
            // F3 = GradientFint(a,b,c,d,xx,dF2_da,dF2_db,dF2_dc,dF2_dd,dF2_ds);
            // 
            // Deriv = (F3 - F2)/delta;
            // 
            // Error = log10(ABS(dF2_ds - Deriv));
            // 
            // printf("Exact: %f ... FD: %f ... Error: %f \n",dF2_ds, Deriv, Error);
             
          }
          
          // Evalulate integrals
          
          F = F2 - F1;
          
          dF_da = dF2_da - dF1_da;
          dF_db = dF2_db - dF1_db;
          dF_dc = dF2_dc - dF1_dc;
          dF_dd = dF2_dd - dF1_dd;
          dF_ds = dF2_ds - dF1_ds;
   
          // U Velocity
   
          U2 =  v_ *     dz * F;
          U4 =     -w_ * dy * F;     
          
          dU2_dv  = dz * F;
          dU2_ddz = v_ * F;
          dU2_dF  = v_ * dz;
   
          dU4_dw  = -dy * F;
          dU4_ddy = -w_ * F;
          dU4_dF  = -w_ * dy;
                 
          q[0] = -C_Gamma*(U2 + U4);
          
          dq0_dU2 = -C_Gamma;
          dq0_dU4 = -C_Gamma;
          
          dq0_dC_Gamma = -(U2 + U4);
    
          // q[0] wrt x1
                
          dq0_dx1 =
                    dq0_dU2 * dU2_dv  * 0.
                  + dq0_dU2 * dU2_ddz * 0.               
   
                  + dq0_dU2 * dU2_dF  * dF_da * da_ddx * ddx_dx1
               //   + dq0_dU2 * dU2_dF  * dF_da * da_ddy * 0.
               //   + dq0_dU2 * dU2_dF  * dF_da * da_ddz * 0.
                                      
                  + dq0_dU2 * dU2_dF  * dF_db * db_ddx * ddx_dx1
               //   + dq0_dU2 * dU2_dF  * dF_db * db_ddy * 0.
               //   + dq0_dU2 * dU2_dF  * dF_db * db_ddz * 0.
               
                  + dq0_dU2 * dU2_dF  * dF_db * db_du  * du_dx1
               //   + dq0_dU2 * dU2_dF  * dF_db * db_dv  * 0.
               //   + dq0_dU2 * dU2_dF  * dF_db * db_dw  * 0.
                                                       
                  + dq0_dU2 * dU2_dF  * dF_dc * dc_du  * du_dx1
               //   + dq0_dU2 * dU2_dF  * dF_dc * dc_dv  * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dc * dc_dw  * 0.
                                                       
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddx * ddx_dx1
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddy * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddx * ddx_dx1
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddy * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddz * 0.
               
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_du  * du_dx1
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_dv  * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                                
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_du  * du_dx1
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_dv  * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
               //   + dq0_dU2 * dU2_dF  * dF_ds * 0.
                                      
               //   + dq0_dU4 * dU4_dw  * 0.
               //   + dq0_dU4 * dU4_ddy * 0.               
   
                  + dq0_dU4 * dU4_dF  * dF_da * da_ddx * ddx_dx1
               //   + dq0_dU4 * dU4_dF  * dF_da * da_ddy * 0.
               //   + dq0_dU4 * dU4_dF  * dF_da * da_ddz * 0.
                                      
                  + dq0_dU4 * dU4_dF  * dF_db * db_ddx * ddx_dx1
               //   + dq0_dU4 * dU4_dF  * dF_db * db_ddy * 0.
               //   + dq0_dU4 * dU4_dF  * dF_db * db_ddz * 0.
               
                  + dq0_dU4 * dU4_dF  * dF_db * db_du  * du_dx1
               //   + dq0_dU4 * dU4_dF  * dF_db * db_dv  * 0.
               //   + dq0_dU4 * dU4_dF  * dF_db * db_dw  * 0.
                                                       
                  + dq0_dU4 * dU4_dF  * dF_dc * dc_du  * du_dx1
               //   + dq0_dU4 * dU4_dF  * dF_dc * dc_dv  * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dc * dc_dw  * 0.
                                                       
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddx * ddx_dx1
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddy * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddx * ddx_dx1
                //  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddy * 0.
                //  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddz * 0.
               
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_du  * du_dx1
                //  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_dv  * 0.
                //  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                                
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_du  * du_dx1
                // + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_dv  * 0.
                // + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
                  + dq0_dU4 * dU4_dF  * dF_ds * 0.;
   
          // q[0] wrt x2
                
          dq0_dx2 =
   
               //     dq0_dU2 * dU2_dv  * 0.
               //   + dq0_dU2 * dU2_ddz * 0.               
   
               //   + dq0_dU2 * dU2_dF  * dF_da * da_ddx * 0.
               //   + dq0_dU2 * dU2_dF  * dF_da * da_ddy * 0.
               //   + dq0_dU2 * dU2_dF  * dF_da * da_ddz * 0.
                                      
               //   + dq0_dU2 * dU2_dF  * dF_db * db_ddx * 0.
               //   + dq0_dU2 * dU2_dF  * dF_db * db_ddy * 0.
               //   + dq0_dU2 * dU2_dF  * dF_db * db_ddz * 0.
               
                  + dq0_dU2 * dU2_dF  * dF_db * db_du  * du_dx2
               //   + dq0_dU2 * dU2_dF  * dF_db * db_dv  * 0.
               //   + dq0_dU2 * dU2_dF  * dF_db * db_dw  * 0.
                                                       
                  + dq0_dU2 * dU2_dF  * dF_dc * dc_du  * du_dx2
               //   + dq0_dU2 * dU2_dF  * dF_dc * dc_dv  * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dc * dc_dw  * 0.
                                                       
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddx * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddy * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddx * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddy * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddz * 0.
               
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_du  * du_dx2
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_dv  * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                                
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_du  * du_dx2
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_dv  * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
               //   + dq0_dU2 * dU2_dF  * dF_ds * 0.
                                      
               //   + dq0_dU2 * dU4_dw  * 0.
               //   + dq0_dU2 * dU4_ddy * 0.               
   
               //   + dq0_dU4 * dU4_dF  * dF_da * da_ddx * 0.
               //   + dq0_dU4 * dU4_dF  * dF_da * da_ddy * 0.
               //   + dq0_dU4 * dU4_dF  * dF_da * da_ddz * 0.
                                      
               //   + dq0_dU4 * dU4_dF  * dF_db * db_ddx * 0.
               //   + dq0_dU4 * dU4_dF  * dF_db * db_ddy * 0.
               //   + dq0_dU4 * dU4_dF  * dF_db * db_ddz * 0.
               
                  + dq0_dU4 * dU4_dF  * dF_db * db_du  * du_dx2
               //   + dq0_dU4 * dU4_dF  * dF_db * db_dv  * 0.
               //   + dq0_dU4 * dU4_dF  * dF_db * db_dw  * 0.
                                                       
                  + dq0_dU4 * dU4_dF  * dF_dc * dc_du  * du_dx2
               //   + dq0_dU4 * dU4_dF  * dF_dc * dc_dv  * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dc * dc_dw  * 0.
                                                       
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddx * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddy * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddx * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddy * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddz * 0.
               
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_du  * du_dx2
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_dv  * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                                
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_du  * du_dx2
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_dv  * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
                  + dq0_dU4 * dU4_dF  * dF_ds * 0.;
   
          // q[0] wrt y1
                
          dq0_dy1 =
   
                    dq0_dU2 * dU2_dv  * dv_dy1
               //   + dq0_dU2 * dU2_ddz * 0.               
   
               //   + dq0_dU2 * dU2_dF  * dF_da * da_ddx * 0.
                  + dq0_dU2 * dU2_dF  * dF_da * da_ddy * ddy_dy1
               //   + dq0_dU2 * dU2_dF  * dF_da * da_ddz * 0.
                                      
               //   + dq0_dU2 * dU2_dF  * dF_db * db_ddx * 0
                  + dq0_dU2 * dU2_dF  * dF_db * db_ddy * ddy_dy1
               //   + dq0_dU2 * dU2_dF  * dF_db * db_ddz * 0.
               //   + dq0_dU2 * dU2_dF  * dF_db * db_du  * 0.
                  + dq0_dU2 * dU2_dF  * dF_db * db_dv  * dv_dy1
               //   + dq0_dU2 * dU2_dF  * dF_db * db_dw  * 0.
                                                       
               //   + dq0_dU2 * dU2_dF  * dF_dc * dc_du  * 0
                  + dq0_dU2 * dU2_dF  * dF_dc * dc_dv  * dv_dy1
               //   + dq0_dU2 * dU2_dF  * dF_dc * dc_dw  * 0.
                                                       
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddx * 0.
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddy * ddy_dy1
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddx * 0.
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddy * ddy_dy1
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddz * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_du  * 0.
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_dv  * dv_dy1
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                       
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_du  * 0.
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_dv  * dv_dy1
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
               //   + dq0_dU2 * dU2_dF  * dF_ds * 0.
                                      
               //   + dq0_dU4 * dU4_dw  * 0.
                  + dq0_dU4 * dU4_ddy * ddy_dy1            
   
               //   + dq0_dU4 * dU4_dF  * dF_da * da_ddx * 0.
                  + dq0_dU4 * dU4_dF  * dF_da * da_ddy * ddy_dy1
               //   + dq0_dU4 * dU4_dF  * dF_da * da_ddz * 0.
                                      
               //   + dq0_dU4 * dU4_dF  * dF_db * db_ddx * 0.
                  + dq0_dU4 * dU4_dF  * dF_db * db_ddy * ddy_dy1
               //   + dq0_dU4 * dU4_dF  * dF_db * db_ddz * 0.
               //   + dq0_dU4 * dU4_dF  * dF_db * db_du  * 0.
                  + dq0_dU4 * dU4_dF  * dF_db * db_dv  * dv_dy1
               //   + dq0_dU4 * dU4_dF  * dF_db * db_dw  * 0.
                                      
               //   + dq0_dU4 * dU4_dF  * dF_dc * dc_du  * 0.
                  + dq0_dU4 * dU4_dF  * dF_dc * dc_dv  * dv_dy1
               //   + dq0_dU4 * dU4_dF  * dF_dc * dc_dw  * 0.
                                                       
               //  + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddx * 0.
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddy * ddy_dy1
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddx * 0.
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddy * ddy_dy1
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddz * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_du  * 0.
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_dv  * dv_dy1
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                       
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_du  * 0.
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_dv  * dv_dy1
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
                  + dq0_dU4 * dU4_dF  * dF_ds * 0.;
   
          // q[0] wrt y2
   
          dq0_dy2 =
   
                    dq0_dU2 * dU2_dv  * dv_dy2
               //   + dq0_dU2 * dU2_ddz * 0.               
   
               //   + dq0_dU2 * dU2_dF  * dF_da * da_ddx * 0.
               //   + dq0_dU2 * dU2_dF  * dF_da * da_ddy * 0.
               //   + dq0_dU2 * dU2_dF  * dF_da * da_ddz * 0.
                                      
               //  + dq0_dU2 * dU2_dF  * dF_db * db_ddx * 0
               //  + dq0_dU2 * dU2_dF  * dF_db * db_ddy * 0.
               //  + dq0_dU2 * dU2_dF  * dF_db * db_ddz * 0.
               //  + dq0_dU2 * dU2_dF  * dF_db * db_du  * 0.
                  + dq0_dU2 * dU2_dF  * dF_db * db_dv  * dv_dy2
               //   + dq0_dU2 * dU2_dF  * dF_db * db_dw  * 0.
                                                       
               //   + dq0_dU2 * dU2_dF  * dF_dc * dc_du  * 0
                  + dq0_dU2 * dU2_dF  * dF_dc * dc_dv  * dv_dy2
                  + dq0_dU2 * dU2_dF  * dF_dc * dc_dw  * 0.
                                                       
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddx * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddy * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddx * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddy * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddz * 0.
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_du  * 0.
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_dv  * dv_dy2
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                       
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_du  * 0.
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_dv  * dv_dy2
               //   + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
               //   + dq0_dU2 * dU2_dF  * dF_ds * 0.
                                      
               //   + dq0_dU4 * dU4_dw  * 0.
               //   + dq0_dU4 * dU4_ddy * 0.           
   
               //   + dq0_dU4 * dU4_dF * dF_da * da_ddx * 0.
               //   + dq0_dU4 * dU4_dF * dF_da * da_ddy * 0.
               //   + dq0_dU4 * dU4_dF * dF_da * da_ddz * 0.
                      
               //   + dq0_dU4 * dU4_dF * dF_db * db_ddx * 0.
               //   + dq0_dU4 * dU4_dF * dF_db * db_ddy * 0.
               //   + dq0_dU4 * dU4_dF * dF_db * db_ddz * 0.
               //   + dq0_dU4 * dU4_dF * dF_db * db_du  * 0.
                  + dq0_dU4 * dU4_dF * dF_db * db_dv  * dv_dy2
               //   + dq0_dU4 * dU4_dF * dF_db * db_dw  * 0.
                  
               //   + dq0_dU4 * dU4_dF * dF_dc * dc_du  * 0.
                  + dq0_dU4 * dU4_dF * dF_dc * dc_dv  * dv_dy2
               //   + dq0_dU4 * dU4_dF * dF_dc * dc_dw  * 0.
                                                      
               //   + dq0_dU4 * dU4_dF * dF_dd * dd_da  * da_ddx * 0.
               //   + dq0_dU4 * dU4_dF * dF_dd * dd_da  * da_ddy * 0.
               //   + dq0_dU4 * dU4_dF * dF_dd * dd_da  * da_ddz * 0.
                                                      
               //   + dq0_dU4 * dU4_dF * dF_dd * dd_db  * db_ddx * 0.
               //   + dq0_dU4 * dU4_dF * dF_dd * dd_db  * db_ddy * 0.
               //   + dq0_dU4 * dU4_dF * dF_dd * dd_db  * db_ddz * 0.
               //   + dq0_dU4 * dU4_dF * dF_dd * dd_db  * db_du  * 0.
                  + dq0_dU4 * dU4_dF * dF_dd * dd_db  * db_dv  * dv_dy2
               //   + dq0_dU4 * dU4_dF * dF_dd * dd_db  * db_dw  * 0.
                                                      
               //   + dq0_dU4 * dU4_dF * dF_dd * dd_dc  * dc_du  * 0.
                  + dq0_dU4 * dU4_dF * dF_dd * dd_dc  * dc_dv  * dv_dy2
               //   + dq0_dU4 * dU4_dF * dF_dd * dd_dc  * dc_dw  * 0.
                   
                  + dq0_dU4 * dU4_dF * dF_ds * 0.;
   
          // q[0] wrt z1
   
          dq0_dz1 =
   
               //     dq0_dU2 * dU2_dv * 0.
                  + dq0_dU2 * dU2_ddz * ddz_dz1             
   
               //   + dq0_dU2 * dU2_dF * dF_da * da_ddx * 0.
               //   + dq0_dU2 * dU2_dF * dF_da * da_ddy * 0.
                  + dq0_dU2 * dU2_dF * dF_da * da_ddz * ddz_dz1
                  
               //   + dq0_dU2 * dU2_dF * dF_db * db_ddx * 0
               //   + dq0_dU2 * dU2_dF * dF_db * db_ddy * 0.
                  + dq0_dU2 * dU2_dF * dF_db * db_ddz * ddz_dz1
               //   + dq0_dU2 * dU2_dF * dF_db * db_du  * 0.
               //   + dq0_dU2 * dU2_dF * dF_db * db_dv  * 0.
                  + dq0_dU2 * dU2_dF * dF_db * db_dw  * dw_dz1
                                                      
               //   + dq0_dU2 * dU2_dF * dF_dc * dc_du  * 0
               //   + dq0_dU2 * dU2_dF * dF_dc * dc_dv  * 0.
                  + dq0_dU2 * dU2_dF * dF_dc * dc_dw  * dw_dz1
                                                      
               //   + dq0_dU2 * dU2_dF * dF_dd * dd_da  * da_ddx * 0.
               //   + dq0_dU2 * dU2_dF * dF_dd * dd_da  * da_ddy * 0.
                  + dq0_dU2 * dU2_dF * dF_dd * dd_da  * da_ddz * ddz_dz1
                                                      
               //   + dq0_dU2 * dU2_dF * dF_dd * dd_db  * db_ddx * 0.
               //   + dq0_dU2 * dU2_dF * dF_dd * dd_db  * db_ddy * 0.
                  + dq0_dU2 * dU2_dF * dF_dd * dd_db  * db_ddz * ddz_dz1
               //   + dq0_dU2 * dU2_dF * dF_dd * dd_db  * db_du  * 0.
               //   + dq0_dU2 * dU2_dF * dF_dd * dd_db  * db_dv  * 0.
                  + dq0_dU2 * dU2_dF * dF_dd * dd_db  * db_dw  * dw_dz1
                                                      
               //   + dq0_dU2 * dU2_dF * dF_dd * dd_dc  * dc_du  * 0.
               //   + dq0_dU2 * dU2_dF * dF_dd * dd_dc  * dc_dv  * 0.
                  + dq0_dU2 * dU2_dF * dF_dd * dd_dc  * dc_dw  * dw_dz1
                                
               //   + dq0_dU2 * dU2_dF * dF_ds * 0.
   
                  + dq0_dU4 * dU4_dw  * dw_dz1
               //   + dq0_dU4 * dU4_ddy * 0.               
   
               //   + dq0_dU4 * dU4_dF  * dF_da * da_ddx * 0.
               //   + dq0_dU4 * dU4_dF  * dF_da * da_ddy * 0.
                  + dq0_dU4 * dU4_dF  * dF_da * da_ddz * ddz_dz1
                                      
               //   + dq0_dU4 * dU4_dF  * dF_db * db_ddx * 0.
               //   + dq0_dU4 * dU4_dF  * dF_db * db_ddy * 0.
                  + dq0_dU4 * dU4_dF  * dF_db * db_ddz * ddz_dz1
               //   + dq0_dU4 * dU4_dF  * dF_db * db_du  * 0.
               //   + dq0_dU4 * dU4_dF  * dF_db * db_dv  * 0.
                  + dq0_dU4 * dU4_dF  * dF_db * db_dw  * dw_dz1
                                      
               //   + dq0_dU4 * dU4_dF  * dF_dc * dc_du  * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dc * dc_dv  * 0.
                  + dq0_dU4 * dU4_dF  * dF_dc * dc_dw  * dw_dz1
                                                       
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddx * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddy * 0.
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddz * ddz_dz1
                                                       
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddx * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddy * 0.
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddz * ddz_dz1
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_du  * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_dv  * 0.
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_dw  * dw_dz1
                                                       
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_du  * 0.
               //   + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_dv  * 0.
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_dw  * dw_dz1
                                      
                  + dq0_dU4 * dU4_dF  * dF_ds * 0.;
   
         // q[0] wrt z2
   
         dq0_dz2 =
   
              //      dq0_dU2 * dU2_dv  * 0.
              //    + dq0_dU2 * dU2_ddz * 0.               
   
              //    + dq0_dU2 * dU2_dF  * dF_da * da_ddx * 0.
              //    + dq0_dU2 * dU2_dF  * dF_da * da_ddy * 0.
              //    + dq0_dU2 * dU2_dF  * dF_da * da_ddz * 0.
                                      
              //    + dq0_dU2 * dU2_dF  * dF_db * db_ddx * 0
              //    + dq0_dU2 * dU2_dF  * dF_db * db_ddy * 0.
              //    + dq0_dU2 * dU2_dF  * dF_db * db_ddz * 0.
              //    + dq0_dU2 * dU2_dF  * dF_db * db_du  * 0.
              //    + dq0_dU2 * dU2_dF  * dF_db * db_dv  * 0.
                  + dq0_dU2 * dU2_dF  * dF_db * db_dw  * dw_dz2
                                                       
              //    + dq0_dU2 * dU2_dF  * dF_dc * dc_du  * 0
              //    + dq0_dU2 * dU2_dF  * dF_dc * dc_dv  * 0.
                  + dq0_dU2 * dU2_dF  * dF_dc * dc_dw  * dw_dz2
                                                       
              //    + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddx * 0.
              //    + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddy * 0.
              //    + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
              //    + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddx * 0.
              //    + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddy * 0.
              //    + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddz * 0.
              //    + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_du  * 0.
              //    + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_dv  * 0.
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_dw  * dw_dz2
                                                       
              //    + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_du  * 0.
              //    + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_dv  * 0.
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_dc  * dc_dw  * dw_dz2
                                      
              //    + dq0_dU2 * dU2_dF  * dF_ds * 0.
                                      
                  + dq0_dU4 * dU4_dw  * dw_dz2
              //    + dq0_dU4 * dU4_ddy * 0.               
   
              //    + dq0_dU4 * dU4_dF  * dF_da * da_ddx * 0.
              //    + dq0_dU4 * dU4_dF  * dF_da * da_ddy * 0.
              //    + dq0_dU4 * dU4_dF  * dF_da * da_ddz * 0.
                                     
              //    + dq0_dU4 * dU4_dF  * dF_db * db_ddx * 0.
              //    + dq0_dU4 * dU4_dF  * dF_db * db_ddy * 0.
              //    + dq0_dU4 * dU4_dF  * dF_db * db_ddz * 0.
              //    + dq0_dU4 * dU4_dF  * dF_db * db_du  * 0.
              //    + dq0_dU4 * dU4_dF  * dF_db * db_dv  * 0.
                  + dq0_dU4 * dU4_dF  * dF_db * db_dw  * dw_dz2
                                      
              //    + dq0_dU4 * dU4_dF  * dF_dc * dc_du  * 0.
              //    + dq0_dU4 * dU4_dF  * dF_dc * dc_dv  * 0.
                  + dq0_dU4 * dU4_dF  * dF_dc * dc_dw  * dw_dz2
                                                       
              //    + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddx * 0.
              //    + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddy * 0.
              //    + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
              //    + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddx * 0.
              //    + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddy * 0.
              //    + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddz * 0.
              //    + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_du  * 0.
              //    + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_dv  * 0.
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_dw  * dw_dz2
                                                       
              //    + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_du  * 0.
              //    + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_dv  * 0.
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_dc  * dc_dw  * dw_dz2
                                      
                  + dq0_dU4 * dU4_dF  * dF_ds * 0.;
                    
          // V Velocity
     
          V2 =  u_ *     dz * F;
          V4 =     -w_ * dx * F;
          
          dV2_du  = dz * F;
          dV2_ddz = u_ * F;
          dV2_dF  = u_ * dz;
          
          dV4_dw  = -dx * F;
          dV4_ddx = -w_ * F;
          dV4_dF  = -w_ * dx;
   
          q[1] =  C_Gamma*(V2 + V4);
          
          dq1_dV2 = C_Gamma;
          dq1_dV4 = C_Gamma;
   
          dq1_dC_Gamma = (V2 + V4);
   
          // q[1] wrt x1
                
          dq1_dx1 =
   
                    dq1_dV2 * dV2_du  * du_dx1
             //     + dq1_dV2 * dV2_ddz * 0.               
                               
                  + dq1_dV2 * dV2_dF  * dF_da * da_ddx * ddx_dx1
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddz * 0.
                                      
                  + dq1_dV2 * dV2_dF  * dF_db * db_ddx * ddx_dx1
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddz * 0.
                  + dq1_dV2 * dV2_dF  * dF_db * db_du  * du_dx1
             //     + dq1_dV2 * dV2_dF  * dF_db * db_dv  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_dw  * 0.
                                                       
                  + dq1_dV2 * dV2_dF  * dF_dc * dc_du  * du_dx1
             //     + dq1_dV2 * dV2_dF  * dF_dc * dc_dv  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dc * dc_dw  * 0.
                                                       
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddx * ddx_dx1
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddx * ddx_dx1
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddz * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_du  * du_dx1
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_dv  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                                
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_du  * du_dx1
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_dv  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
             //     + dq1_dV2 * dV2_dF  * dF_ds * 0.
                                      
             //     + dq1_dV4 * dV4_dw  * 0.
                  + dq1_dV4 * dV4_ddx * ddx_dx1               
                               
                  + dq1_dV4 * dV4_dF  * dF_da * da_ddx * ddx_dx1
             //     + dq1_dV4 * dV4_dF  * dF_da * da_ddy * 0.
             //     + dq1_dV4 * dV4_dF  * dF_da * da_ddz * 0.
                                      
                  + dq1_dV4 * dV4_dF  * dF_db * db_ddx * ddx_dx1
             //     + dq1_dV4 * dV4_dF  * dF_db * db_ddy * 0.
             //     + dq1_dV4 * dV4_dF  * dF_db * db_ddz * 0.
                  + dq1_dV4 * dV4_dF  * dF_db * db_du  * du_dx1
             //     + dq1_dV4 * dV4_dF  * dF_db * db_dv  * 0.
             //     + dq1_dV4 * dV4_dF  * dF_db * db_dw  * 0.
                                                       
                  + dq1_dV4 * dV4_dF  * dF_dc * dc_du  * du_dx1
             //     + dq1_dV4 * dV4_dF  * dF_dc * dc_dv  * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dc * dc_dw  * 0.
                                                       
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddx * ddx_dx1
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddy * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddx * ddx_dx1
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddy * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddz * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_du  * du_dx1
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_dv  * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                                
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_du  * du_dx1
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_dv  * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
                  + dq1_dV4 * dV4_dF  * dF_ds * 0.;
   
          // q[1] wrt x2
   
          dq1_dx2 =
   
                    dq1_dV2 * dV2_du  * du_dx2
             //     + dq1_dV2 * dV2_ddz * 0.               
                               
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddz * 0.
                                      
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddz * 0.
                  + dq1_dV2 * dV2_dF  * dF_db * db_du  * du_dx2
             //     + dq1_dV2 * dV2_dF  * dF_db * db_dv  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_dw  * 0.
                                                       
                  + dq1_dV2 * dV2_dF  * dF_dc * dc_du  * du_dx2
             //     + dq1_dV2 * dV2_dF  * dF_dc * dc_dv  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dc * dc_dw  * 0.
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddz * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_du  * du_dx2
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_dv  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                                
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_du  * du_dx2
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_dv  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
             //     + dq1_dV2 * dV2_dF  * dF_ds * 0.
                                      
             //     + dq1_dV4 * dV4_dw  * 0.
             //     + dq1_dV4 * dV4_ddx * 0.               
                               
             //     + dq1_dV4 * dV4_dF * dF_da * da_ddx * 0.
             //     + dq1_dV4 * dV4_dF * dF_da * da_ddy * 0.
             //     + dq1_dV4 * dV4_dF * dF_da * da_ddz * 0.
                               
             //     + dq1_dV4 * dV4_dF * dF_db * db_ddx * 0.
             //     + dq1_dV4 * dV4_dF * dF_db * db_ddy * 0.
             //     + dq1_dV4 * dV4_dF * dF_db * db_ddz * 0.
                  + dq1_dV4 * dV4_dF * dF_db * db_du  * du_dx2
             //     + dq1_dV4 * dV4_dF * dF_db * db_dv  * 0.
             //     + dq1_dV4 * dV4_dF * dF_db * db_dw  * 0.
                                                      
                  + dq1_dV4 * dV4_dF * dF_dc * dc_du  * du_dx2
             //     + dq1_dV4 * dV4_dF * dF_dc * dc_dv  * 0.
             //     + dq1_dV4 * dV4_dF * dF_dc * dc_dw  * 0.
                                                      
             //     + dq1_dV4 * dV4_dF * dF_dd * dd_da  * da_ddx * 0.
             //     + dq1_dV4 * dV4_dF * dF_dd * dd_da  * da_ddy * 0.
             //     + dq1_dV4 * dV4_dF * dF_dd * dd_da  * da_ddz * 0.
                                                      
             //     + dq1_dV4 * dV4_dF * dF_dd * dd_db  * db_ddx * 0.
             //     + dq1_dV4 * dV4_dF * dF_dd * dd_db  * db_ddy * 0.
             //     + dq1_dV4 * dV4_dF * dF_dd * dd_db  * db_ddz * 0.
                  + dq1_dV4 * dV4_dF * dF_dd * dd_db  * db_du  * du_dx2
             //     + dq1_dV4 * dV4_dF * dF_dd * dd_db  * db_dv  * 0.
             //     + dq1_dV4 * dV4_dF * dF_dd * dd_db  * db_dw  * 0.
                                                               
                  + dq1_dV4 * dV4_dF * dF_dd * dd_dc  * dc_du  * du_dx2
             //     + dq1_dV4 * dV4_dF * dF_dd * dd_dc  * dc_dv  * 0.
             //     + dq1_dV4 * dV4_dF * dF_dd * dd_dc  * dc_dw  * 0.
                               
                  + dq1_dV4 * dV4_dF * dF_ds * 0.;
   
          // q[1] wrt y1
   
          dq1_dy1 =
   
             //       dq1_dV2 * dV2_du  * 0.
             //     + dq1_dV2 * dV2_ddz * 0.               
                               
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddx * 0.
                  + dq1_dV2 * dV2_dF  * dF_da * da_ddy * ddy_dy1
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddz * 0.
                                      
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddx * 0.
                  + dq1_dV2 * dV2_dF  * dF_db * db_ddy * ddy_dy1
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddz * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_du  * 0.
                  + dq1_dV2 * dV2_dF  * dF_db * db_dv  * dv_dy1
             //     + dq1_dV2 * dV2_dF  * dF_db * db_dw  * 0.
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dc * dc_du  * 0.
                  + dq1_dV2 * dV2_dF  * dF_dc * dc_dv  * dv_dy1
             //     + dq1_dV2 * dV2_dF  * dF_dc * dc_dw  * 0.
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddx * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddy * ddy_dy1
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddx * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddy * ddy_dy1
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddz * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_du  * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_dv  * dv_dy1
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                                
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_du  * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_dv  * dv_dy1
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
             //     + dq1_dV2 * dV2_dF  * dF_ds * 0.
                                      
             //     + dq1_dV4 * dV4_dw  * 0.
             //     + dq1_dV4 * dV4_ddx * 0.               
                               
             //     + dq1_dV4 * dV4_dF  * dF_da * da_ddx * 0.
                  + dq1_dV4 * dV4_dF  * dF_da * da_ddy * ddy_dy1
             //     + dq1_dV4 * dV4_dF  * dF_da * da_ddz * 0.
                                      
             //     + dq1_dV4 * dV4_dF  * dF_db * db_ddx * 0.
                  + dq1_dV4 * dV4_dF  * dF_db * db_ddy * ddy_dy1
             //     + dq1_dV4 * dV4_dF  * dF_db * db_ddz * 0.
             //     + dq1_dV4 * dV4_dF  * dF_db * db_du  * 0.
                  + dq1_dV4 * dV4_dF  * dF_db * db_dv  * dv_dy1
             //     + dq1_dV4 * dV4_dF  * dF_db * db_dw  * 0.
                                                       
             //     + dq1_dV4 * dV4_dF  * dF_dc * dc_du  * 0.
                  + dq1_dV4 * dV4_dF  * dF_dc * dc_dv  * dv_dy1
             //     + dq1_dV4 * dV4_dF  * dF_dc * dc_dw  * 0.
                                                       
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddx * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddy * ddy_dy1
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddx * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddy * ddy_dy1
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddz * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_du  * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_dv  * dv_dy1
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                                
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_du  * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_dv  * dv_dy1
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
                  + dq1_dV4 * dV4_dF  * dF_ds * 0.;
   
          // q[1] wrt y2
   
          dq1_dy2 =
   
             //       dq1_dV2 * dV2_du  * 0.
             //     + dq1_dV2 * dV2_ddz * 0.               
                               
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddz * 0.
                                      
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddz * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_du  * 0.
                  + dq1_dV2 * dV2_dF  * dF_db * db_dv  * dv_dy2
             //     + dq1_dV2 * dV2_dF  * dF_db * db_dw  * 0.
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dc * dc_du  * 0.
                  + dq1_dV2 * dV2_dF  * dF_dc * dc_dv  * dv_dy2
             //     + dq1_dV2 * dV2_dF  * dF_dc * dc_dw  * 0.
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddz * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_du  * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_dv  * dv_dy2
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                                
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_du  * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_dv  * dv_dy2
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
             //     + dq1_dV2 * dV2_dF  * dF_ds * 0.
                                      
             //     + dq1_dV4 * dV4_dw  * 0.
             //     + dq1_dV4 * dV4_ddx * 0.               
                               
             //     + dq1_dV4 * dV4_dF  * dF_da * da_ddx * 0.
             //     + dq1_dV4 * dV4_dF  * dF_da * da_ddy * 0.
             //     + dq1_dV4 * dV4_dF  * dF_da * da_ddz * 0.
                                      
             //     + dq1_dV4 * dV4_dF  * dF_db * db_ddx * 0.
             //     + dq1_dV4 * dV4_dF  * dF_db * db_ddy * 0.
             //     + dq1_dV4 * dV4_dF  * dF_db * db_ddz * 0.
             //     + dq1_dV4 * dV4_dF  * dF_db * db_du  * 0.
                  + dq1_dV4 * dV4_dF  * dF_db * db_dv  * dv_dy2
             //     + dq1_dV4 * dV4_dF  * dF_db * db_dw  * 0.
                                                       
             //     + dq1_dV4 * dV4_dF  * dF_dc * dc_du  * 0.
                  + dq1_dV4 * dV4_dF  * dF_dc * dc_dv  * dv_dy2
             //     + dq1_dV4 * dV4_dF  * dF_dc * dc_dw  * 0.
                                                       
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddx * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddy * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                       
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddx * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddy * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddz * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_du  * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_dv  * dv_dy2
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                                
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_du  * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_dv  * dv_dy2
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
                  + dq1_dV4 * dV4_dF  * dF_ds * 0.;
   
          // q[1] wrt z1
   
          dq1_dz1 =
   
             //       dq1_dV2 * dV2_du  * 0.
                  + dq1_dV2 * dV2_ddz * ddz_dz1            
                               
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddy * 0.
                  + dq1_dV2 * dV2_dF  * dF_da * da_ddz * ddz_dz1
                                      
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddy * 0.
                  + dq1_dV2 * dV2_dF  * dF_db * db_ddz * ddz_dz1
             //     + dq1_dV2 * dV2_dF  * dF_db * db_du  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_dv  * 0.
                  + dq1_dV2 * dV2_dF  * dF_db * db_dw  * dw_dz1
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dc * dc_du  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dc * dc_dv  * 0.
                  + dq1_dV2 * dV2_dF  * dF_dc * dc_dw  * dw_dz1
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddy * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddz * ddz_dz1
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddy * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddz * ddz_dz1
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_du  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_dv  * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_dw  * dw_dz1
                                                                
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_du  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_dv  * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_dw  * dw_dz1
                                      
             //     + dq1_dV2 * dV2_dF  * dF_ds * 0.
                                      
                  + dq1_dV4 * dV4_dw  * dw_dz1
             //     + dq1_dV4 * dV4_ddx * 0.              
                               
             //     + dq1_dV4 * dV4_dF  * dF_da * da_ddx * 0.
             //     + dq1_dV4 * dV4_dF  * dF_da * da_ddy * 0.
                  + dq1_dV4 * dV4_dF  * dF_da * da_ddz * ddz_dz1
                                      
             //     + dq1_dV4 * dV4_dF  * dF_db * db_ddx * 0.
             //     + dq1_dV4 * dV4_dF  * dF_db * db_ddy * 0.
                  + dq1_dV4 * dV4_dF  * dF_db * db_ddz * ddz_dz1
             //     + dq1_dV4 * dV4_dF  * dF_db * db_du  * 0.
             //     + dq1_dV4 * dV4_dF  * dF_db * db_dv  * 0.
                  + dq1_dV4 * dV4_dF  * dF_db * db_dw  * dw_dz1
                                                       
             //     + dq1_dV4 * dV4_dF  * dF_dc * dc_du  * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dc * dc_dv  * 0.
                  + dq1_dV4 * dV4_dF  * dF_dc * dc_dw  * dw_dz1
                                                       
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddx * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddy * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddz * ddz_dz1
                                                       
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddx * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddy * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddz * ddz_dz1
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_du  * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_dv  * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_dw  * dw_dz1
                                                                
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_du  * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_dv  * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_dw  * dw_dz1
                                      
                  + dq1_dV4 * dV4_dF  * dF_ds * 0.;
   
          // q[1] wrt z2
   
          dq1_dz2 =
   
             //       dq1_dV2 * dV2_du  * 0.
             //     + dq1_dV2 * dV2_ddz * 0.            
                               
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_da * da_ddz * 0.  
                                      
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_ddz * 0.  
             //     + dq1_dV2 * dV2_dF  * dF_db * db_du  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_db * db_dv  * 0.
                  + dq1_dV2 * dV2_dF  * dF_db * db_dw  * dw_dz2
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dc * dc_du  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dc * dc_dv  * 0.
                  + dq1_dV2 * dV2_dF  * dF_dc * dc_dw  * dw_dz2
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddz * 0.  
                                                       
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddx * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddy * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddz * 0.  
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_du  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_dv  * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_dw  * dw_dz2
                                                                
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_du  * 0.
             //     + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_dv  * 0.
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_dc  * dc_dw  * dw_dz2
                                      
             //     + dq1_dV2 * dV2_dF  * dF_ds * 0.
                                      
                  + dq1_dV4 * dV4_dw  * dw_dz2
             //     + dq1_dV4 * dV4_ddx * 0.              
                               
             //     + dq1_dV4 * dV4_dF  * dF_da * da_ddx * 0.
             //     + dq1_dV4 * dV4_dF  * dF_da * da_ddy * 0.
             //     + dq1_dV4 * dV4_dF  * dF_da * da_ddz * 0.  
                                      
             //     + dq1_dV4 * dV4_dF  * dF_db * db_ddx * 0.
             //     + dq1_dV4 * dV4_dF  * dF_db * db_ddy * 0.
             //     + dq1_dV4 * dV4_dF  * dF_db * db_ddz * 0.  
             //     + dq1_dV4 * dV4_dF  * dF_db * db_du  * 0.
             //     + dq1_dV4 * dV4_dF  * dF_db * db_dv  * 0.
                  + dq1_dV4 * dV4_dF  * dF_db * db_dw  * dw_dz2
                                                       
             //     + dq1_dV4 * dV4_dF  * dF_dc * dc_du  * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dc * dc_dv  * 0.
                  + dq1_dV4 * dV4_dF  * dF_dc * dc_dw  * dw_dz2
                                                       
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddx * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddy * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddz * 0.  
                                                       
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddx * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddy * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddz * 0.  
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_du  * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_dv  * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_dw  * dw_dz2
                                                                
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_du  * 0.
             //     + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_dv  * 0.
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_dc  * dc_dw  * dw_dz2
                                      
                  + dq1_dV4 * dV4_dF  * dF_ds * 0.;
   
   
                                                                            
          // W Velocity
          
          W2 =  u_ *     dy * F;
          W4 =     -v_ * dx * F;
          
          dW2_du  = dy * F;
          dW2_ddy = u_ * F;
          dW2_dF  = u_ * dy;
          
          dW4_dv  = -dx * F;
          dW4_ddx = -v_ * F;
          dW4_dF  = -v_ * dx;
          
          q[2] = -C_Gamma*(W2 + W4);
          
          dq2_dW2 = -C_Gamma;
          dq2_dW4 = -C_Gamma;
   
          dq2_dC_Gamma = -(W2 + W4);
   
          // q[2] wrt x1
   
          dq2_dx1 =
   
                    dq2_dW2 * dW2_du  * du_dx1
              //    + dq2_dW2 * dW2_ddy * 0.               
                               
                  + dq2_dW2 * dW2_dF  * dF_da * da_ddx * ddx_dx1
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddy * 0.
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddz * 0.
                                      
                  + dq2_dW2 * dW2_dF  * dF_db * db_ddx * ddx_dx1
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddy * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddz * 0.
                  + dq2_dW2 * dW2_dF  * dF_db * db_du  * du_dx1
              //    + dq2_dW2 * dW2_dF  * dF_db * db_dv  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_dw  * 0.
                                                    
                  + dq2_dW2 * dW2_dF  * dF_dc * dc_du  * du_dx1
              //    + dq2_dW2 * dW2_dF  * dF_dc * dc_dv  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dc * dc_dw  * 0.
                                                    
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddx * ddx_dx1
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddy * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                   
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddx * ddx_dx1
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddy * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddz * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_du  * du_dx1
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_dv  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                           
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_du  * du_dx1
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_dv  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
              //    + dq2_dW2 * dW2_dF  * dF_ds * 0.
                                      
              //    + dq2_dW4 * dW4_dv  * 0.
                  + dq2_dW4 * dW4_ddx * ddx_dx1               
                               
                  + dq2_dW4 * dW4_dF * dF_da * da_ddx * ddx_dx1
              //    + dq2_dW4 * dW4_dF * dF_da * da_ddy * 0.
              //    + dq2_dW4 * dW4_dF * dF_da * da_ddz * 0.
                               
                  + dq2_dW4 * dW4_dF * dF_db * db_ddx * ddx_dx1
              //    + dq2_dW4 * dW4_dF * dF_db * db_ddy * 0.
              //    + dq2_dW4 * dW4_dF * dF_db * db_ddz * 0.
                  + dq2_dW4 * dW4_dF * dF_db * db_du  * du_dx1
              //    + dq2_dW4 * dW4_dF * dF_db * db_dv  * 0.
              //    + dq2_dW4 * dW4_dF * dF_db * db_dw  * 0.
                                                 
                  + dq2_dW4 * dW4_dF * dF_dc * dc_du  * du_dx1
              //    + dq2_dW4 * dW4_dF * dF_dc * dc_dv  * 0.
              //    + dq2_dW4 * dW4_dF * dF_dc * dc_dw  * 0.
                                                 
                  + dq2_dW4 * dW4_dF * dF_dd * dd_da  * da_ddx * ddx_dx1
              //    + dq2_dW4 * dW4_dF * dF_dd * dd_da  * da_ddy * 0.
              //    + dq2_dW4 * dW4_dF * dF_dd * dd_da  * da_ddz * 0.
                                                
                  + dq2_dW4 * dW4_dF * dF_dd * dd_db  * db_ddx * ddx_dx1
              //    + dq2_dW4 * dW4_dF * dF_dd * dd_db  * db_ddy * 0.
              //    + dq2_dW4 * dW4_dF * dF_dd * dd_db  * db_ddz * 0.
                  + dq2_dW4 * dW4_dF * dF_dd * dd_db  * db_du  * du_dx1
              //    + dq2_dW4 * dW4_dF * dF_dd * dd_db  * db_dv  * 0.
              //    + dq2_dW4 * dW4_dF * dF_dd * dd_db  * db_dw  * 0.
                                                          
                  + dq2_dW4 * dW4_dF * dF_dd * dd_dc  * dc_du  * du_dx1
              //    + dq2_dW4 * dW4_dF * dF_dd * dd_dc  * dc_dv  * 0.
              //    + dq2_dW4 * dW4_dF * dF_dd * dd_dc  * dc_dw  * 0.
                               
                  + dq2_dW4 * dW4_dF * dF_ds * 0.;
   
          // q[2] wrt x2
   
          dq2_dx2 =
   
                    dq2_dW2 * dW2_du  * du_dx2
              //    + dq2_dW2 * dW2_ddy * 0.               
                               
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddy * 0.
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddz * 0.
                                      
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddy * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddz * 0.
                  + dq2_dW2 * dW2_dF  * dF_db * db_du  * du_dx2
              //    + dq2_dW2 * dW2_dF  * dF_db * db_dv  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_dw  * 0.
                                                    
                  + dq2_dW2 * dW2_dF  * dF_dc * dc_du  * du_dx2
              //    + dq2_dW2 * dW2_dF  * dF_dc * dc_dv  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dc * dc_dw  * 0.
                                                    
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddy * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                   
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddy * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddz * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_du  * du_dx2
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_dv  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                           
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_du  * du_dx2
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_dv  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
              //    + dq2_dW2 * dW2_dF  * dF_ds * 0.
                                      
              //    + dq2_dW4 * dW4_dv  * 0.
              //    + dq2_dW4 * dW4_ddx * 0.               
                               
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddy * 0.
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddz * 0.
                                      
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddy * 0.
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddz * 0.
                  + dq2_dW4 * dW4_dF  * dF_db * db_du  * du_dx2
              //    + dq2_dW4 * dW4_dF  * dF_db * db_dv  * 0.
              //    + dq2_dW4 * dW4_dF  * dF_db * db_dw  * 0.
                                                  
                  + dq2_dW4 * dW4_dF  * dF_dc * dc_du  * du_dx2
              //    + dq2_dW4 * dW4_dF  * dF_dc * dc_dv  * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dc * dc_dw  * 0.
                                                  
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddy * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                 
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddy * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddz * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_du  * du_dx2
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_dv  * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                           
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_du  * du_dx2
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_dv  * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
                  + dq2_dW4 * dW4_dF  * dF_ds * 0.;
   
          // q[2] wrt y1
   
          dq2_dy1 =
   
              //      dq2_dW2 * dW2_du  * 0.
                  + dq2_dW2 * dW2_ddy * ddy_dy1               
                               
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddx * 0.
                  + dq2_dW2 * dW2_dF  * dF_da * da_ddy * ddy_dy1
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddz * 0.
                                      
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddx * 0.
                  + dq2_dW2 * dW2_dF  * dF_db * db_ddy * ddy_dy1
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddz * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_du  * 0.
                  + dq2_dW2 * dW2_dF  * dF_db * db_dv  * dv_dy1
              //    + dq2_dW2 * dW2_dF  * dF_db * db_dw  * 0.
                                                    
              //    + dq2_dW2 * dW2_dF  * dF_dc * dc_du  * 0.
                  + dq2_dW2 * dW2_dF  * dF_dc * dc_dv  * dv_dy1
              //    + dq2_dW2 * dW2_dF  * dF_dc * dc_dw  * 0.
                                                    
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddx * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddy * ddy_dy1
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                   
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddx * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddy * ddy_dy1
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddz * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_du  * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_dv  * dv_dy1
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                           
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_du  * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_dv  * dv_dy1
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
              //    + dq2_dW2 * dW2_dF  * dF_ds * 0.
                               
                  + dq2_dW4 * dW4_dv  * dv_dy1
              //    + dq2_dW4 * dW4_ddx * 0.             
                               
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddx * 0.
                  + dq2_dW4 * dW4_dF  * dF_da * da_ddy * ddy_dy1
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddz * 0.
                                      
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddx * 0.
                  + dq2_dW4 * dW4_dF  * dF_db * db_ddy * ddy_dy1
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddz * 0.
              //    + dq2_dW4 * dW4_dF  * dF_db * db_du  * 0.
                  + dq2_dW4 * dW4_dF  * dF_db * db_dv  * dv_dy1
              //    + dq2_dW4 * dW4_dF  * dF_db * db_dw  * 0.
                                                  
              //    + dq2_dW4 * dW4_dF  * dF_dc * dc_du  * 0.
                  + dq2_dW4 * dW4_dF  * dF_dc * dc_dv  * dv_dy1
              //    + dq2_dW4 * dW4_dF  * dF_dc * dc_dw  * 0.
                                                  
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddx * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddy * ddy_dy1
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                 
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddx * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddy * ddy_dy1
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddz * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_du  * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_dv  * dv_dy1
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                           
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_du  * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_dv  * dv_dy1
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
                  + dq2_dW4 * dW4_dF  * dF_ds * 0.;
   
          // q[2] wrt y1
   
          dq2_dy2 =
   
              //      dq2_dW2 * dW2_du  * 0.
              //    + dq2_dW2 * dW2_ddy * 0.               
                               
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddy * 0.  
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddz * 0.
                                      
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddy * 0.  
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddz * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_du  * 0.
                  + dq2_dW2 * dW2_dF  * dF_db * db_dv  * dv_dy2
              //    + dq2_dW2 * dW2_dF  * dF_db * db_dw  * 0.
                                                    
              //    + dq2_dW2 * dW2_dF  * dF_dc * dc_du  * 0.
                  + dq2_dW2 * dW2_dF  * dF_dc * dc_dv  * dv_dy2
              //    + dq2_dW2 * dW2_dF  * dF_dc * dc_dw  * 0.
                                                    
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddy * 0.  
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                   
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddy * 0.  
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddz * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_du  * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_dv  * dv_dy2
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                           
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_du  * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_dv  * dv_dy2
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
              //    + dq2_dW2 * dW2_dF  * dF_ds * 0.
                               
                  + dq2_dW4 * dW4_dv  * dv_dy2
              //    + dq2_dW4 * dW4_ddx * 0.        
                  
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddy * 0.  
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddz * 0.
                                      
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddy * 0.  
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddz * 0.
              //    + dq2_dW4 * dW4_dF  * dF_db * db_du  * 0.
                  + dq2_dW4 * dW4_dF  * dF_db * db_dv  * dv_dy2
              //    + dq2_dW4 * dW4_dF  * dF_db * db_dw  * 0.
                                                  
              //    + dq2_dW4 * dW4_dF  * dF_dc * dc_du  * 0.
                  + dq2_dW4 * dW4_dF  * dF_dc * dc_dv  * dv_dy2
              //    + dq2_dW4 * dW4_dF  * dF_dc * dc_dw  * 0.
                                                  
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddy * 0.  
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                 
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddy * 0.  
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddz * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_du  * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_dv  * dv_dy2
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_dw  * 0.
                                                           
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_du  * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_dv  * dv_dy2
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_dw  * 0.
                                      
                  + dq2_dW4 * dW4_dF  * dF_ds * 0.;
   
          // q[2] wrt z1
   
          dq2_dz1 =
   
              //      dq2_dW2 * dW2_du  * 0.
              //    + dq2_dW2 * dW2_ddy * 0.               
                               
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddy * 0.
                  + dq2_dW2 * dW2_dF  * dF_da * da_ddz * ddz_dz1
                                      
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddy * 0.
                  + dq2_dW2 * dW2_dF  * dF_db * db_ddz * ddz_dz1
              //    + dq2_dW2 * dW2_dF  * dF_db * db_du  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_dv  * 0.
                  + dq2_dW2 * dW2_dF  * dF_db * db_dw  * dw_dz1
                                                    
              //    + dq2_dW2 * dW2_dF  * dF_dc * dc_du  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dc * dc_dv  * 0.
                  + dq2_dW2 * dW2_dF  * dF_dc * dc_dw  * dw_dz1
                                                    
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddy * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddz * ddz_dz1
                                                   
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddy * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddz * ddz_dz1
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_du  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_dv  * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_dw  * dw_dz1
                                                           
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_du  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_dv  * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_dw  * dw_dz1
                                      
              //    + dq2_dW2 * dW2_dF  * dF_ds * 0.
                               
              //    + dq2_dW4 * dW4_dv  * 0.
              //    + dq2_dW4 * dW4_ddx * 0.             
                               
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddy * 0.
                  + dq2_dW4 * dW4_dF  * dF_da * da_ddz * ddz_dz1
                                      
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddy * 0.
                  + dq2_dW4 * dW4_dF  * dF_db * db_ddz * ddz_dz1
              //    + dq2_dW4 * dW4_dF  * dF_db * db_du  * 0.
              //    + dq2_dW4 * dW4_dF  * dF_db * db_dv  * 0.
                  + dq2_dW4 * dW4_dF  * dF_db * db_dw  * dw_dz1
                                                  
              //    + dq2_dW4 * dW4_dF  * dF_dc * dc_du  * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dc * dc_dv  * 0.
                  + dq2_dW4 * dW4_dF  * dF_dc * dc_dw  * dw_dz1
                                                  
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddy * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddz * ddz_dz1
                                                 
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddy * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddz * ddz_dz1
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_du  * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_dv  * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_dw  * dw_dz1
                                                           
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_du  * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_dv  * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_dw  * dw_dz1
                                      
                  + dq2_dW4 * dW4_dF  * dF_ds * 0.;
   
          // q[2] wrt z2
   
          dq2_dz2 =
   
              //      dq2_dW2 * dW2_du  * 0.
              //    + dq2_dW2 * dW2_ddy * 0.               
                               
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddy * 0.
              //    + dq2_dW2 * dW2_dF  * dF_da * da_ddz * 0.
                                      
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddy * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_ddz * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_du  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_db * db_dv  * 0.
                  + dq2_dW2 * dW2_dF  * dF_db * db_dw  * dw_dz2
                                               
              //    + dq2_dW2 * dW2_dF  * dF_dc * dc_du  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dc * dc_dv  * 0.
                  + dq2_dW2 * dW2_dF  * dF_dc * dc_dw  * dw_dz2
                                                    
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddy * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                   
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddx * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddy * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddz * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_du  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_dv  * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_dw  * dw_dz2
                                                           
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_du  * 0.
              //    + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_dv  * 0.
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_dc  * dc_dw  * dw_dz2
                                      
              //    + dq2_dW2 * dW2_dF  * dF_ds * 0.
                               
              //    + dq2_dW4 * dW4_dv  * 0.
              //    + dq2_dW4 * dW4_ddx * 0.             
                               
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddy * 0.
              //    + dq2_dW4 * dW4_dF  * dF_da * da_ddz * 0.
                                      
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddy * 0.
              //    + dq2_dW4 * dW4_dF  * dF_db * db_ddz * 0.
              //    + dq2_dW4 * dW4_dF  * dF_db * db_du  * 0.
              //    + dq2_dW4 * dW4_dF  * dF_db * db_dv  * 0.
                  + dq2_dW4 * dW4_dF  * dF_db * db_dw  * dw_dz2
                                                  
              //    + dq2_dW4 * dW4_dF  * dF_dc * dc_du  * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dc * dc_dv  * 0.
                  + dq2_dW4 * dW4_dF  * dF_dc * dc_dw  * dw_dz2
                                                  
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddy * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddz * 0.
                                                 
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddx * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddy * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddz * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_du  * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_dv  * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_dw  * dw_dz2
                                                           
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_du  * 0.
              //    + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_dv  * 0.
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_dc  * dc_dw  * dw_dz2
                                      
                  + dq2_dW4 * dW4_dF  * dF_ds * 0.;
   
          // q[0] wrt xp
                
          dq0_dxp =
   
                  + dq0_dU2 * dU2_dF  * dF_da * da_ddx *          ddx_dxp
                                                                  
                  + dq0_dU2 * dU2_dF  * dF_db * db_ddx *          ddx_dxp
                                        
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddx * ddx_dxp
                                        
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddx * ddx_dxp
           
                  + dq0_dU4 * dU4_dF  * dF_da * da_ddx *          ddx_dxp
                                                                  
                  + dq0_dU4 * dU4_dF  * dF_db * db_ddx *          ddx_dxp
                                                        
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddx * ddx_dxp
                                        
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddx * ddx_dxp;
   
   
          // q[0] wrt yp
                
          dq0_dyp =
   
                  + dq0_dU2 * dU2_dF  * dF_da * da_ddy *          ddy_dyp
                                                                  
                  + dq0_dU2 * dU2_dF  * dF_db * db_ddy *          ddy_dyp
                                                       
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_da  * da_ddy * ddy_dyp
                                                       
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_db  * db_ddy * ddy_dyp
   
                  + dq0_dU4 * dU4_ddy *                           ddy_dyp            
   
                  + dq0_dU4 * dU4_dF  * dF_da * da_ddy *          ddy_dyp
                                                                  
                  + dq0_dU4 * dU4_dF  * dF_db * db_ddy *          ddy_dyp
                  
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_da  * da_ddy * ddy_dyp
   
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_db  * db_ddy * ddy_dyp;
   
          // q[0] wrt zp
                  
          dq0_dzp =
   
                  + dq0_dU2 * dU2_ddz *                          ddz_dzp             
                                                                 
                  + dq0_dU2 * dU2_dF * dF_da * da_ddz *          ddz_dzp
                                                                 
                  + dq0_dU2 * dU2_dF * dF_db * db_ddz *          ddz_dzp
   
                  + dq0_dU2 * dU2_dF * dF_dd * dd_da  * da_ddz * ddz_dzp
                                                      
                  + dq0_dU2 * dU2_dF * dF_dd * dd_db  * db_ddz * ddz_dzp
    
                  + dq0_dU4 * dU4_dF * dF_da * da_ddz *          ddz_dzp
                                                                 
                  + dq0_dU4 * dU4_dF * dF_db * db_ddz *          ddz_dzp
    
                  + dq0_dU4 * dU4_dF * dF_dd * dd_da  * da_ddz * ddz_dzp
   
                  + dq0_dU4 * dU4_dF * dF_dd * dd_db  * db_ddz * ddz_dzp;
   
          // q[1] wrt xp
                
          dq1_dxp =
   
                  + dq1_dV2 * dV2_dF  * dF_da * da_ddx *          ddx_dxp
                                                                  
                  + dq1_dV2 * dV2_dF  * dF_db * db_ddx *          ddx_dxp
                                      
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddx * ddx_dxp
                                                       
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddx * ddx_dxp
                                      
                  + dq1_dV4 * dV4_ddx *                           ddx_dxp               
                                      
                  + dq1_dV4 * dV4_dF  * dF_da * da_ddx *          ddx_dxp
                                                                  
                  + dq1_dV4 * dV4_dF  * dF_db * db_ddx *          ddx_dxp
                                                        
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddx * ddx_dxp
                                                       
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddx * ddx_dxp;
    
          // q[1] wrt yp
   
          dq1_dyp =
   
                  + dq1_dV2 * dV2_dF * dF_da * da_ddy *          ddy_dyp
                                                                 
                  + dq1_dV2 * dV2_dF * dF_db * db_ddy *          ddy_dyp
    
                  + dq1_dV2 * dV2_dF * dF_dd * dd_da  * da_ddy * ddy_dyp
   
                  + dq1_dV2 * dV2_dF * dF_dd * dd_db  * db_ddy * ddy_dyp
    
                  + dq1_dV4 * dV4_dF * dF_da * da_ddy *          ddy_dyp
                                                                 
                  + dq1_dV4 * dV4_dF * dF_db * db_ddy *          ddy_dyp
    
                  + dq1_dV4 * dV4_dF * dF_dd * dd_da  * da_ddy * ddy_dyp
    
                  + dq1_dV4 * dV4_dF * dF_dd * dd_db  * db_ddy * ddy_dyp;
   
   
          // q[1] wrt zp
   
          dq1_dzp =
   
                  + dq1_dV2 * dV2_ddz *                           ddz_dzp            
                                                                  
                  + dq1_dV2 * dV2_dF  * dF_da * da_ddz *          ddz_dzp
                                                                  
                  + dq1_dV2 * dV2_dF  * dF_db * db_ddz *          ddz_dzp
   
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_da  * da_ddz * ddz_dzp
                                                       
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_db  * db_ddz * ddz_dzp
   
                  + dq1_dV4 * dV4_dF  * dF_da * da_ddz *          ddz_dzp
                                                                  
                  + dq1_dV4 * dV4_dF  * dF_db * db_ddz *          ddz_dzp
    
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_da  * da_ddz * ddz_dzp                                                    
   
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_db  * db_ddz * ddz_dzp;
   
          // q[2] wrt xp
   
          dq2_dxp =
   
                  + dq2_dW2 * dW2_dF  * dF_da * da_ddx *          ddx_dxp
                                                                  
                  + dq2_dW2 * dW2_dF  * dF_db * db_ddx *          ddx_dxp
   
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddx * ddx_dxp
   
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddx * ddx_dxp
   
                  + dq2_dW4 * dW4_ddx *                           ddx_dxp               
                               
                  + dq2_dW4 * dW4_dF  * dF_da * da_ddx *          ddx_dxp
                                                                  
                  + dq2_dW4 * dW4_dF  * dF_db * db_ddx *          ddx_dxp
                                      
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddx * ddx_dxp
                                                 
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddx * ddx_dxp;
                  
          // q[2] wrt yp
   
          dq2_dyp =
   
                  + dq2_dW2 * dW2_ddy * ddy_dyp              
                               
                  + dq2_dW2 * dW2_dF  * dF_da * da_ddy *          ddy_dyp
                                                                  
                  + dq2_dW2 * dW2_dF  * dF_db * db_ddy *          ddy_dyp
   
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddy * ddy_dyp
    
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddy * ddy_dyp
    
                  + dq2_dW4 * dW4_dF  * dF_da * da_ddy *          ddy_dyp
                                                                  
                  + dq2_dW4 * dW4_dF  * dF_db * db_ddy *          ddy_dyp
   
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddy * ddy_dyp
    
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddy * ddy_dyp;
   
          // q[2] wrt zp
   
          dq2_dzp =
   
                  + dq2_dW2 * dW2_dF  * dF_da * da_ddz *          ddz_dzp
                                                                  
                  + dq2_dW2 * dW2_dF  * dF_db * db_ddz *          ddz_dzp
   
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_da  * da_ddz * ddz_dzp
                                                   
                  + dq2_dW2 * dW2_dF  * dF_dd * dd_db  * db_ddz * ddz_dzp
   
                  + dq2_dW4 * dW4_dF  * dF_da * da_ddz *          ddz_dzp
                                                                  
                  + dq2_dW4 * dW4_dF  * dF_db * db_ddz *          ddz_dzp
    
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_da  * da_ddz * ddz_dzp
                                                 
                  + dq2_dW4 * dW4_dF  * dF_dd * dd_db  * db_ddz * ddz_dzp;
   
          // Mach
   
          dq0_dMach = 
                    
                    dq0_dC_Gamma * dC_Gamma_dBeta2 * dBeta2_dMach
   
                  + dq0_dU2 * dU2_dF  * dF_da *         da_dBeta2 * dBeta2_dMach
                                                        
                  + dq0_dU2 * dU2_dF  * dF_db *         db_dBeta2 * dBeta2_dMach
                                                        
                  + dq0_dU2 * dU2_dF  * dF_db *         db_dBeta2 * dBeta2_dMach
                                                               
                  + dq0_dU2 * dU2_dF  * dF_dc *         dc_dBeta2 * dBeta2_dMach
                                                       
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_da * da_dBeta2 * dBeta2_dMach
                      
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_db * db_dBeta2 * dBeta2_dMach
               
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_db * db_dBeta2 * dBeta2_dMach
                                                     
                  + dq0_dU2 * dU2_dF  * dF_dd * dd_dc * dc_dBeta2 * dBeta2_dMach
      
                  + dq0_dU4 * dU4_dF  * dF_da *         da_dBeta2 * dBeta2_dMach
                                                      
                  + dq0_dU4 * dU4_dF  * dF_db *         db_dBeta2 * dBeta2_dMach
                                                   
                  + dq0_dU4 * dU4_dF  * dF_dc *         dc_dBeta2 * dBeta2_dMach
            
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_da * da_dBeta2 * dBeta2_dMach
       
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_db * db_dBeta2 * dBeta2_dMach
        
                  + dq0_dU4 * dU4_dF  * dF_dd * dd_dc * dc_dBeta2 * dBeta2_dMach;
   
         dq1_dMach =
         
                    dq1_dC_Gamma * dC_Gamma_dBeta2 * dBeta2_dMach
   
                  + dq1_dV2 * dV2_dF  * dF_da *         da_dBeta2 * dBeta2_dMach 
   
                  + dq1_dV2 * dV2_dF  * dF_db *         db_dBeta2 * dBeta2_dMach
                                                     
                  + dq1_dV2 * dV2_dF  * dF_dc *         dc_dBeta2 * dBeta2_dMach
                                                     
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_da * da_dBeta2 * dBeta2_dMach
                                                        
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_db * db_dBeta2 * dBeta2_dMach
                                                        
                  + dq1_dV2 * dV2_dF  * dF_dd * dd_dc * dc_dBeta2 * dBeta2_dMach
                
                  + dq1_dV4 * dV4_dF  * dF_da *         da_dBeta2 * dBeta2_dMach
                                    
                  + dq1_dV4 * dV4_dF  * dF_db *         db_dBeta2 * dBeta2_dMach
                                                     
                  + dq1_dV4 * dV4_dF  * dF_dc *         dc_dBeta2 * dBeta2_dMach
                                                     
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_da * da_dBeta2 * dBeta2_dMach
                                                     
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_db * db_dBeta2 * dBeta2_dMach
                                                        
                  + dq1_dV4 * dV4_dF  * dF_dd * dd_dc * dc_dBeta2 * dBeta2_dMach;
                                      
          dq2_dMach =
          
                    dq2_dC_Gamma * dC_Gamma_dBeta2 * dBeta2_dMach
   
                  + dq2_dW2 * dW2_dF * dF_da *         da_dBeta2 * dBeta2_dMach 
                                    
                  + dq2_dW2 * dW2_dF * dF_db *         db_dBeta2 * dBeta2_dMach 
                                                 
                  + dq2_dW2 * dW2_dF * dF_dc *         dc_dBeta2 * dBeta2_dMach 
                                                 
                  + dq2_dW2 * dW2_dF * dF_dd * dd_da * da_dBeta2 * dBeta2_dMach 
                                                       
                  + dq2_dW2 * dW2_dF * dF_dd * dd_db * db_dBeta2 * dBeta2_dMach 
                                                       
                  + dq2_dW2 * dW2_dF * dF_dd * dd_dc * dc_dBeta2 * dBeta2_dMach 
         
                  + dq2_dW4 * dW4_dF * dF_da *         da_dBeta2 * dBeta2_dMach  
                             
                  + dq2_dW4 * dW4_dF * dF_db *         db_dBeta2 * dBeta2_dMach  
                                               
                  + dq2_dW4 * dW4_dF * dF_dc *         dc_dBeta2 * dBeta2_dMach  
                                               
                  + dq2_dW4 * dW4_dF * dF_dd * dd_da * da_dBeta2 * dBeta2_dMach   
                                                       
                  + dq2_dW4 * dW4_dF * dF_dd * dd_db * db_dBeta2 * dBeta2_dMach   
                                                       
                  + dq2_dW4 * dW4_dF * dF_dd * dd_dc * dc_dBeta2 * dBeta2_dMach;   
                               
       }
       
       else {
        
          q[0] = q[1] = q[2] = 0.;
   
          dq0_dx1 = 0.;
          dq0_dy1 = 0.;
          dq0_dz1 = 0.;
          
          dq0_dx2 = 0.;
          dq0_dy2 = 0.;
          dq0_dz2 = 0.;
                
          dq1_dx1 = 0.;
          dq1_dy1 = 0.;
          dq1_dz1 = 0.;
          
          dq1_dx2 = 0.;
          dq1_dy2 = 0.;
          dq1_dz2 = 0.;
                  
          dq2_dx1 = 0.;
          dq2_dy1 = 0.;
          dq2_dz1 = 0.;
          
          dq2_dx2 = 0.;
          dq2_dy2 = 0.;
          dq2_dz2 = 0.;
   
          dq0_dxp = 0.;
          dq0_dyp = 0.;
          dq0_dzp = 0.;
   
          dq1_dxp = 0.;
          dq1_dyp = 0.;
          dq1_dzp = 0.;
   
          dq2_dxp = 0.;
          dq2_dyp = 0.;
          dq2_dzp = 0.;
          
          dq0_dMach = 0.;
          dq1_dMach = 0.;
          dq2_dMach = 0.;
                                                  
       }
   
       // Zero things out if we are on or near a convex trailing edge wake
       
       if ( IsConcaveTrailingEdge_ ) {
   
          q[0] = q[1] = q[2] = 0.;
   
          dq0_dx1 = 0.;
          dq0_dy1 = 0.;
          dq0_dz1 = 0.;
          
          dq0_dx2 = 0.;
          dq0_dy2 = 0.;
          dq0_dz2 = 0.;
                
          dq1_dx1 = 0.;
          dq1_dy1 = 0.;
          dq1_dz1 = 0.;
          
          dq1_dx2 = 0.;
          dq1_dy2 = 0.;
          dq1_dz2 = 0.;
                  
          dq2_dx1 = 0.;
          dq2_dy1 = 0.;
          dq2_dz1 = 0.;
          
          dq2_dx2 = 0.;
          dq2_dy2 = 0.;
          dq2_dz2 = 0.;
         
          dq0_dxp = 0.;
          dq0_dyp = 0.;
          dq0_dzp = 0.;
         
          dq1_dxp = 0.;
          dq1_dyp = 0.;
          dq1_dzp = 0.;
         
          dq2_dxp = 0.;
          dq2_dyp = 0.;
          dq2_dzp = 0.;
          
          dq0_dMach = 0.;
          dq1_dMach = 0.;
          dq2_dMach = 0.;
                                             
       }
       
       // Symmetry plane dge
   
       if ( IsSymmetryPlaneEdge_ ) {
          
          q[1] = 0.;
          
          dq0_dy1 = 0.;
          dq0_dy2 = 0.;
                
          dq1_dy1 = 0.;
          dq1_dy2 = 0.;
                
          dq2_dy1 = 0.;
          dq2_dy2 = 0.;
          
       }
   
       if ( Node1_IsOnSymmetryPlane_ ) { dq0_dy1 = dq1_dy1 = dq2_dy1 = 0.; };
       if ( Node2_IsOnSymmetryPlane_ ) { dq0_dy2 = dq1_dy2 = dq2_dy2 = 0.; };
       
    }
    
    else {
   
       dq0_dx1 = 0.;
       dq0_dy1 = 0.;
       dq0_dz1 = 0.;
       
       dq0_dx2 = 0.;
       dq0_dy2 = 0.;
       dq0_dz2 = 0.;
             
       dq1_dx1 = 0.;
       dq1_dy1 = 0.;
       dq1_dz1 = 0.;
       
       dq1_dx2 = 0.;
       dq1_dy2 = 0.;
       dq1_dz2 = 0.;
               
       dq2_dx1 = 0.;
       dq2_dy1 = 0.;
       dq2_dz1 = 0.;
       
       dq2_dx2 = 0.;
       dq2_dy2 = 0.;
       dq2_dz2 = 0.;

       dq0_dxp = 0.;
       dq0_dyp = 0.;
       dq0_dzp = 0.;

       dq1_dxp = 0.;
       dq1_dyp = 0.;
       dq1_dzp = 0.;

       dq2_dxp = 0.;
       dq2_dyp = 0.;
       dq2_dzp = 0.;
       
       dq0_dMach = 0.;
       dq1_dMach = 0.;
       dq2_dMach = 0.;
       
    }       
    
}

/*##############################################################################
#                                                                              #
#                          VSP_EDGE Fint                                       #
#                                                                              #
##############################################################################*/

double VSP_EDGE::Fint(double &a, double &b, double &c, double &d, double &s)
{
 
    double R, Denom;

    R = a + b*s + c*s*s;

    if ( ABS(d) <= Tolerance_2_ || R < Tolerance_1_ ) return 0.;
    
    if ( ABS(d) <= CoreWidth_*CoreWidth_ || R < CoreWidth_ ) return 0.;

    Denom = d * sqrt(R);

    return 2.*(2.*c*s + b)*Denom/(Denom*Denom);

}

/*##############################################################################
#                                                                              #
#                     VSP_EDGE GradientFint                                    #
#                                                                              #
##############################################################################*/

double VSP_EDGE::GradientFint(double &a, double &b, double &c, double &d, double &s,
                                      double &dF_da,
                                      double &dF_db,
                                      double &dF_dc,
                                      double &dF_dd,
                                      double &dF_ds)
{
 
    double R, Denom, F;
    double dR_da;
    double dR_db;
    double dR_dc;
    double dR_dd;
    double dR_ds;
    double dDenom_dd;
    double dDenom_dR;
    double dF_Denom;
  
    R = a + b*s + c*s*s;
    
    dR_da = 1.;
    dR_db = s;
    dR_dc = s*s;    
    dR_ds = b + 2.*c*s;

    dF_da = 0.;
    dF_db = 0.;
    dF_dc = 0.;
    dF_dd = 0.;
    dF_ds = 0.;

    if ( ABS(d) <= Tolerance_2_ || R < Tolerance_1_ ) return 0.;
    
    if ( ABS(d) <= CoreWidth_*CoreWidth_ || R < CoreWidth_ ) return 0.;

    Denom = d * sqrt(R);
    
    F = 2.*(2.*c*s + b)*Denom/(Denom*Denom );
    
    dDenom_dd = sqrt(R);
    dDenom_dR = 0.5*d/sqrt(R);
    
    dF_Denom = 2.*(2.*c*s + b)/(Denom*Denom + 0.*CoreWidth_*CoreWidth_)
             - 2.*(2.*c*s + b)*Denom/pow(Denom*Denom,2.)*2.*Denom;
             
    dF_da = dF_Denom * dDenom_dR * dR_da;
    dF_db = dF_Denom * dDenom_dR * dR_db + 2.*  Denom/(Denom*Denom);
    dF_dc = dF_Denom * dDenom_dR * dR_dc + 4.*s*Denom/(Denom*Denom);
    dF_dd = dF_Denom * dDenom_dd;
    dF_ds = dF_Denom * dDenom_dR * dR_ds + 4.*c*Denom/(Denom*Denom);

    return F;
    
}

/*##############################################################################
#                                                                              #
#                          VSP_EDGE Gint                                       #
#                                                                              #
##############################################################################*/

double VSP_EDGE::Gint(double &a, double &b, double &c, double &d, double &s)
 
{
   
    double R, Denom;
    double dR_da;
    double dR_db;
    double dR_dc;
    double dR_dd;
    double dR_ds;
    double dDenom_dd;
    double dDenom_dR;
    double dF_Denom;
        
    R = a + b*s + c*s*s;

    if ( ABS(d) < Tolerance_2_ || R < Tolerance_1_ ) return 0.;
    
    if ( ABS(d) <= CoreWidth_*CoreWidth_ || R < CoreWidth_ ) return 0.;
    
    Denom = d * sqrt(R);
    
    return -2.*(2.*a+b*s)*Denom/(Denom*Denom);

} 

/*##############################################################################
#                                                                              #
#                          VSP_EDGE GradientGint                               #
#                                                                              #
##############################################################################*/

double VSP_EDGE::GradientGint(double &a, double &b, double &c, double &d, double &s,
                                      double &dF_da,
                                      double &dF_db,
                                      double &dF_dc,
                                      double &dF_dd,
                                      double &dF_ds)      
{
   
    double R, Denom;
    
    R = a + b*s + c*s*s;

    if ( ABS(d) < Tolerance_2_ || R < Tolerance_1_ ) return 0.;
    
    if ( ABS(d) <= CoreWidth_*CoreWidth_ || R < CoreWidth_ ) return 0.;
    
    Denom = d * sqrt(R);
    
    return -2.*(2.*a+b*s)*Denom/(Denom*Denom);

} 

/*##############################################################################
#                                                                              #
#                     VSP_EDGE FindLineConicIntersection                       #
#                                                                              #
##############################################################################*/

void VSP_EDGE::FindLineConicIntersection(double &Xp, double &Yp, double &Zp,
                                         double &X1, double &Y1, double &Z1,
                                         double &Xd, double &Yd, double &Zd,
                                         double &t1, double &t2)
{
 
    int Solved;
    double Xi1, Xi2, Xi, Yi, Zi, Xo, Yo, Zo, Normal[3], Vec[3], Dot;
    double A, B, C, Aq, Bq, Cq;
    double Tol, Delta, v1, v2;
     
    Tol = 1.e-8;
    
    A = 1;
    B = C = 1. - SQR(Mach_);
    
    Xo = X1 - Xp;
    Yo = Y1 - Yp;
    Zo = Z1 - Zp;
    
    Aq = A*Xd*Xd + B*Yd*Yd + C*Zd*Zd;
    Bq = 2.*A*Xo*Xd + 2.*B*Yo*Yd + 2.*C*Zo*Zd;
    Cq = A*Xo*Xo + B*Yo*Yo + C*Zo*Zo;
    
    // Solve for the root(s)
    
    Solved = 0;
    
    t1 = t2 = 100.;
    
    // Two possible solutions
    
    if ( ABS(Aq) >= Tol ) {
   
       Delta = Bq*Bq - 4.*Aq*Cq;
       
       // 2 real solutions
       
       if ( Delta >= 0. ) {

          v1 = -(Bq + sqrt(Delta))/(2.*Aq);
          v2 = -(Bq - sqrt(Delta))/(2.*Aq);
          
          t1 = MIN(v1,v2);
          t2 = MAX(v1,v2);
          
          Solved = 1;
    
       }
       
    }
    
    // 1 solution
    
    else {
       
      t1 = t2 = -Cq / Bq;
      
      Solved = 1;
     
    }  
    
    if ( Solved ) {
     
       // Check if t1 is on upwind facing cone
      
       Xi1 = X1 - Xp + t1*Xd;

       Xi2 = X1 - Xp + t2*Xd;
       
       if ( Xi1 > 0. && Xi2 > 0. ) {
        
          t1 = t2 = 100.;
          
       }
       
       else {
        
          if ( Xi1 > 0. && Xi2 <= 0. ) t1 = t2;
          if ( Xi2 > 0. && Xi1 <= 0. ) t2 = t1;
           
          // If there is only one intersection, we need to determine which end of 
          // the segment is inside the cone and which is out

          if ( t1 > 0. && t2 < 1. && ABS(t1-t2) <= Tolerance_1_ ) {
          
             Xi = X1 - Xp + t1*Xd;
             Yi = Y1 - Yp + t1*Yd;
             Zi = Z1 - Zp + t1*Zd;
             
             Normal[0] = -2.*A*Xi;
             Normal[1] = -2.*B*Yi;
             Normal[2] = -2.*C*Zi;

             Vec[0] = (X1 - Xp) - Xi;
             Vec[1] = (Y1 - Yp) - Yi;
             Vec[2] = (Z1 - Zp) - Zi;
             
             Dot = vector_dot(Normal, Vec);
             
             // Node 1 is outside
             
             if ( Dot > 0. ) {

                t2 = 1.;
          
             }
             
             else {
              
                t1 = 0.;
      
             }
             
          }

          if ( t1 <= 0. ) t1 = 0.;
          if ( t2 >= 1. ) t2 = 1.;

       }
       
    }
    
} 

/*##############################################################################
#                                                                              #
#                   VSP_EDGE GeneralizedPrincipalPartOfDownWash                #
#                                                                              #
##############################################################################*/

double VSP_EDGE::GeneralizedPrincipalPartOfDownWash(void)
{
 
    if ( Mach_ < 1. ) return 0.;

    double Beta_2, T, Ws, Mag, Theta, Arg;

    Beta_2 = 1. - SQR(Mach_);

    Mag = MAX(MIN(Vec_[0],1.),-1.); // this is just vector_dot(Vec,Xaxis);

    Theta = 0.5*PI - acos(Mag);

    T = tan(Theta);

    Arg = -Beta_2 - T*T;

    Ws = 0.;

    if ( Mach_ > 1. && Arg > 0. ) Ws = 0.50*sqrt(Arg)*cos(Theta);

    return Ws;
    
}

/*##############################################################################
#                                                                              #
#                   VSP_EDGE GeneralizedPrincipalPartOfDownWash                #
#                                                                              #
##############################################################################*/

int VSP_EDGE::IsSupersonic(void)
{
 
    double Beta_2, T, Mag, Theta, Arg;
    
    Beta_2 = 1. - SQR(Mach_);
    
    Mag = MAX(MIN(Vec_[0],1.),-1.);
    
    if ( ABS(Mag) > 0.99 ) return 1;

    Theta = 0.5*PI - acos(Mag);
    
    T = tan(Theta);

    Arg = -Beta_2 - T*T;

    if ( Mach_ > 1. && Arg > 0. ) return 1;

    return 0;
    
}

/*##############################################################################
#                                                                              #
#                         VSP_EDGE CalculateForces                             #
#                                                                              #
##############################################################################*/

void VSP_EDGE::CalculateForces(void)
{

    // Full forces
    
    vector_cross(Velocity_, Vec_, Forces_);

    Forces_[0] *= Length_*Gamma_;
    Forces_[1] *= Length_*Gamma_;
    Forces_[2] *= Length_*Gamma_;

}

/*##############################################################################
#                                                                              #
#                   VSP_EDGE CalculateTrefftzForces                            #
#                                                                              #
##############################################################################*/

void VSP_EDGE::CalculateTrefftzForces(void)
{

    // Wake forces

    vector_cross(DownWashVelocity_, Vec_, Trefftz_Forces_);
   
    Trefftz_Forces_[0] *= -Length_*Gamma_;
    Trefftz_Forces_[1] *= -Length_*Gamma_;
    Trefftz_Forces_[2] *= -Length_*Gamma_;

}

/*##############################################################################
#                                                                              #
#                         VSP_EDGE CalculateForces                             #
#                                                                              #
##############################################################################*/

void VSP_EDGE::CalculateForces_dGamma(double dForce[3])
{

    // Full forces
    
    vector_cross(Velocity_, Vec_, dForce);

    dForce[0] *= Length_;
    dForce[1] *= Length_;
    dForce[2] *= Length_;
    
}

/*##############################################################################
#                                                                              #
#              VSP_EDGE CalculateTrefftzForces_dGamma                          #
#                                                                              #
##############################################################################*/

void VSP_EDGE::CalculateTrefftzForces_dGamma(double dForce[3])
{

    // Wake forces
    
    vector_cross(DownWashVelocity_, Vec_, dForce);

    dForce[0] *= -Length_;
    dForce[1] *= -Length_;
    dForce[2] *= -Length_;
    
}

/*##############################################################################
#                                                                              #
#                      VSP_EDGE CalculateForces_dVelocity                      #
#                                                                              #
##############################################################################*/

void VSP_EDGE::CalculateForces_dVelocity(double dVelocity[3], double dForce[3])
{

    // Full forces
    
    vector_cross(dVelocity, Vec_, dForce);

    dForce[0] *= Length_*Gamma_;
    dForce[1] *= Length_*Gamma_;
    dForce[2] *= Length_*Gamma_;
    
}

/*##############################################################################
#                                                                              #
#                      VSP_EDGE CalculateForces_dMesh                          #
#                                                                              #
##############################################################################*/

void VSP_EDGE::CalculateForces_dMesh(double dForce[18])
{

    double dLength_dX1; 
    double dLength_dY1;
    double dLength_dZ1;
                              
    double dLength_dX2;
    double dLength_dY2;
    double dLength_dZ2;
    
    double dVecX_dX1;
    double dVecX_dY1;
    double dVecX_dZ1;
                           
    double dVecY_dX1;
    double dVecY_dY1;
    double dVecY_dZ1;
                            
    double dVecZ_dX1;
    double dVecZ_dY1;
    double dVecZ_dZ1;
                            
    double dVecX_dX2;
    double dVecX_dY2;
    double dVecX_dZ2;
                           
    double dVecY_dX2;
    double dVecY_dY2;
    double dVecY_dZ2;
                            
    double dVecZ_dX2;
    double dVecZ_dY2;
    double dVecZ_dZ2;   

    SetupGradients(dLength_dX1, 
                   dLength_dY1,
                   dLength_dZ1,
                   
                   dLength_dX2,
                   dLength_dY2,
                   dLength_dZ2,
                   
                   dVecX_dX1,
                   dVecX_dY1,
                   dVecX_dZ1,
                           
                   dVecY_dX1,
                   dVecY_dY1,
                   dVecY_dZ1,
                          
                   dVecZ_dX1,
                   dVecZ_dY1,
                   dVecZ_dZ1,
                          
                   dVecX_dX2,
                   dVecX_dY2,
                   dVecX_dZ2,
                          
                   dVecY_dX2,
                   dVecY_dY2,
                   dVecY_dZ2,
                          
                   dVecZ_dX2,
                   dVecZ_dY2,
                   dVecZ_dZ2);

  // i    j    k
  // velx vely velz
  // vecx vecy vecz
  //
  // dForce[0] =  i x ( vecz * vely - vecy * velz )
  // dForce[1] = -j x ( vecz * velx - vecx * velz )
  // dForce[2] =  k x ( vecy * velx - vecx * vely )
  // 
  // dForce[0] =  i x ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )
  // dForce[1] = -j x ( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )
  // dForce[2] =  k x ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )      
  //
  //
  //  vector_cross(Velocity_, Vec_, Forces_);
  //
  //  dForce[0] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )*Length_*Gamma_;
  //  dForce[1] = -( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )*Length_*Gamma_;
  //  dForce[2] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )*Length_*Gamma_;     
       
       
    // Node 1, wrt x
                                                                                   
    dForce[ 0] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )*dLength_dX1*Gamma_ + ( dVecZ_dX1 * Velocity_[1] - dVecY_dX1 * Velocity_[2] )*Length_*Gamma_;
    dForce[ 1] = -( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )*dLength_dX1*Gamma_ - ( dVecZ_dX1 * Velocity_[0] - dVecX_dX1 * Velocity_[2] )*Length_*Gamma_;
    dForce[ 2] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )*dLength_dX1*Gamma_ + ( dVecY_dX1 * Velocity_[0] - dVecX_dX1 * Velocity_[1] )*Length_*Gamma_;          

    // Node 1, wrt y

    dForce[ 3] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )*dLength_dY1*Gamma_ + ( dVecZ_dY1 * Velocity_[1] - dVecY_dY1 * Velocity_[2] )*Length_*Gamma_;
    dForce[ 4] = -( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )*dLength_dY1*Gamma_ - ( dVecZ_dY1 * Velocity_[0] - dVecX_dY1 * Velocity_[2] )*Length_*Gamma_;
    dForce[ 5] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )*dLength_dY1*Gamma_ + ( dVecY_dY1 * Velocity_[0] - dVecX_dY1 * Velocity_[1] )*Length_*Gamma_;          

    // Node 1, wrt z

    dForce[ 6] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )*dLength_dZ1*Gamma_ + ( dVecZ_dZ1 * Velocity_[1] - dVecY_dZ1 * Velocity_[2] )*Length_*Gamma_;
    dForce[ 7] = -( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )*dLength_dZ1*Gamma_ - ( dVecZ_dZ1 * Velocity_[0] - dVecX_dZ1 * Velocity_[2] )*Length_*Gamma_;
    dForce[ 8] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )*dLength_dZ1*Gamma_ + ( dVecY_dZ1 * Velocity_[0] - dVecX_dZ1 * Velocity_[1] )*Length_*Gamma_;         
       
    // Node 2, wrt x
                                                                                   
    dForce[ 9] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )*dLength_dX2*Gamma_ + ( dVecZ_dX2 * Velocity_[1] - dVecY_dX2 * Velocity_[2] )*Length_*Gamma_;
    dForce[10] = -( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )*dLength_dX2*Gamma_ - ( dVecZ_dX2 * Velocity_[0] - dVecX_dX2 * Velocity_[2] )*Length_*Gamma_;
    dForce[11] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )*dLength_dX2*Gamma_ + ( dVecY_dX2 * Velocity_[0] - dVecX_dX2 * Velocity_[1] )*Length_*Gamma_;          
                                                                                                   
    // Node 2, wrt y                                                                               
                                                                                                   
    dForce[12] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )*dLength_dY2*Gamma_ + ( dVecZ_dY2 * Velocity_[1] - dVecY_dY2 * Velocity_[2] )*Length_*Gamma_;
    dForce[13] = -( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )*dLength_dY2*Gamma_ - ( dVecZ_dY2 * Velocity_[0] - dVecX_dY2 * Velocity_[2] )*Length_*Gamma_;
    dForce[14] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )*dLength_dY2*Gamma_ + ( dVecY_dY2 * Velocity_[0] - dVecX_dY2 * Velocity_[1] )*Length_*Gamma_;          
                                                                                                   
    // Node 2, wrt z                                                                               
                                                                                                   
    dForce[15] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )*dLength_dZ2*Gamma_ + ( dVecZ_dZ2 * Velocity_[1] - dVecY_dZ2 * Velocity_[2] )*Length_*Gamma_;
    dForce[16] = -( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )*dLength_dZ2*Gamma_ - ( dVecZ_dZ2 * Velocity_[0] - dVecX_dZ2 * Velocity_[2] )*Length_*Gamma_;
    dForce[17] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )*dLength_dZ2*Gamma_ + ( dVecY_dZ2 * Velocity_[0] - dVecX_dZ2 * Velocity_[1] )*Length_*Gamma_;        

}

/*##############################################################################
#                                                                              #
#                      VSP_EDGE CalculateTrefftzForces_dMesh                   #
#                                                                              #
##############################################################################*/

void VSP_EDGE::CalculateTrefftzForces_dMesh(double dForce[18])
{

    double dLength_dX1; 
    double dLength_dY1;
    double dLength_dZ1;
                              
    double dLength_dX2;
    double dLength_dY2;
    double dLength_dZ2;
    
    double dVecX_dX1;
    double dVecX_dY1;
    double dVecX_dZ1;
                           
    double dVecY_dX1;
    double dVecY_dY1;
    double dVecY_dZ1;
                            
    double dVecZ_dX1;
    double dVecZ_dY1;
    double dVecZ_dZ1;
                            
    double dVecX_dX2;
    double dVecX_dY2;
    double dVecX_dZ2;
                           
    double dVecY_dX2;
    double dVecY_dY2;
    double dVecY_dZ2;
                            
    double dVecZ_dX2;
    double dVecZ_dY2;
    double dVecZ_dZ2;   

    SetupGradients(dLength_dX1, 
                   dLength_dY1,
                   dLength_dZ1,
                   
                   dLength_dX2,
                   dLength_dY2,
                   dLength_dZ2,
                   
                   dVecX_dX1,
                   dVecX_dY1,
                   dVecX_dZ1,
                           
                   dVecY_dX1,
                   dVecY_dY1,
                   dVecY_dZ1,
                          
                   dVecZ_dX1,
                   dVecZ_dY1,
                   dVecZ_dZ1,
                          
                   dVecX_dX2,
                   dVecX_dY2,
                   dVecX_dZ2,
                          
                   dVecY_dX2,
                   dVecY_dY2,
                   dVecY_dZ2,
                          
                   dVecZ_dX2,
                   dVecZ_dY2,
                   dVecZ_dZ2);

  // i    j    k
  // velx vely velz
  // vecx vecy vecz
  //
  // dForce[0] =  i x ( vecz * vely - vecy * velz )
  // dForce[1] = -j x ( vecz * velx - vecx * velz )
  // dForce[2] =  k x ( vecy * velx - vecx * vely )
  // 
  // dForce[0] =  i x ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )
  // dForce[1] = -j x ( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )
  // dForce[2] =  k x ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )      
  //
  //
  //  vector_cross(Velocity_, Vec_, Forces_);
  //
  //  dForce[0] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )*Length_*Gamma_;
  //  dForce[1] = -( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )*Length_*Gamma_;
  //  dForce[2] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )*Length_*Gamma_;     
       
       
    // Node 1, wrt x
                                                                                   
    dForce[ 0] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * DownWashVelocity_[2] )*dLength_dX1*Gamma_ + ( dVecZ_dX1 * Velocity_[1] - dVecY_dX1 * DownWashVelocity_[2] )*Length_*Gamma_;
    dForce[ 1] = -( Vec_[2] * Velocity_[0] - Vec_[0] * DownWashVelocity_[2] )*dLength_dX1*Gamma_ - ( dVecZ_dX1 * Velocity_[0] - dVecX_dX1 * DownWashVelocity_[2] )*Length_*Gamma_;
    dForce[ 2] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * DownWashVelocity_[1] )*dLength_dX1*Gamma_ + ( dVecY_dX1 * Velocity_[0] - dVecX_dX1 * DownWashVelocity_[1] )*Length_*Gamma_;          
                                                                                                                                   
    // Node 1, wrt y                                                                                                               
                                                                                                                                   
    dForce[ 3] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * DownWashVelocity_[2] )*dLength_dY1*Gamma_ + ( dVecZ_dY1 * Velocity_[1] - dVecY_dY1 * DownWashVelocity_[2] )*Length_*Gamma_;
    dForce[ 4] = -( Vec_[2] * Velocity_[0] - Vec_[0] * DownWashVelocity_[2] )*dLength_dY1*Gamma_ - ( dVecZ_dY1 * Velocity_[0] - dVecX_dY1 * DownWashVelocity_[2] )*Length_*Gamma_;
    dForce[ 5] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * DownWashVelocity_[1] )*dLength_dY1*Gamma_ + ( dVecY_dY1 * Velocity_[0] - dVecX_dY1 * DownWashVelocity_[1] )*Length_*Gamma_;          
                                                                                                                                   
    // Node 1, wrt z                                                                                                               
                                                                                                                                   
    dForce[ 6] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * DownWashVelocity_[2] )*dLength_dZ1*Gamma_ + ( dVecZ_dZ1 * Velocity_[1] - dVecY_dZ1 * DownWashVelocity_[2] )*Length_*Gamma_;
    dForce[ 7] = -( Vec_[2] * Velocity_[0] - Vec_[0] * DownWashVelocity_[2] )*dLength_dZ1*Gamma_ - ( dVecZ_dZ1 * Velocity_[0] - dVecX_dZ1 * DownWashVelocity_[2] )*Length_*Gamma_;
    dForce[ 8] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * DownWashVelocity_[1] )*dLength_dZ1*Gamma_ + ( dVecY_dZ1 * Velocity_[0] - dVecX_dZ1 * DownWashVelocity_[1] )*Length_*Gamma_;         
                                                                                                                                   
    // Node 2, wrt x                                                                                                               
                                                                                                                                   
    dForce[ 9] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * DownWashVelocity_[2] )*dLength_dX2*Gamma_ + ( dVecZ_dX2 * Velocity_[1] - dVecY_dX2 * DownWashVelocity_[2] )*Length_*Gamma_;
    dForce[10] = -( Vec_[2] * Velocity_[0] - Vec_[0] * DownWashVelocity_[2] )*dLength_dX2*Gamma_ - ( dVecZ_dX2 * Velocity_[0] - dVecX_dX2 * DownWashVelocity_[2] )*Length_*Gamma_;
    dForce[11] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * DownWashVelocity_[1] )*dLength_dX2*Gamma_ + ( dVecY_dX2 * Velocity_[0] - dVecX_dX2 * DownWashVelocity_[1] )*Length_*Gamma_;          
                                                                                                                                   
    // Node 2, wrt y                                                                                                               
                                                                                                                                   
    dForce[12] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * DownWashVelocity_[2] )*dLength_dY2*Gamma_ + ( dVecZ_dY2 * Velocity_[1] - dVecY_dY2 * DownWashVelocity_[2] )*Length_*Gamma_;
    dForce[13] = -( Vec_[2] * Velocity_[0] - Vec_[0] * DownWashVelocity_[2] )*dLength_dY2*Gamma_ - ( dVecZ_dY2 * Velocity_[0] - dVecX_dY2 * DownWashVelocity_[2] )*Length_*Gamma_;
    dForce[14] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * DownWashVelocity_[1] )*dLength_dY2*Gamma_ + ( dVecY_dY2 * Velocity_[0] - dVecX_dY2 * DownWashVelocity_[1] )*Length_*Gamma_;          
                                                                                                                                  
    // Node 2, wrt z                                                                                                              
                                                                                                                                  
    dForce[15] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * DownWashVelocity_[2] )*dLength_dZ2*Gamma_ + ( dVecZ_dZ2 * Velocity_[1] - dVecY_dZ2 * DownWashVelocity_[2] )*Length_*Gamma_;
    dForce[16] = -( Vec_[2] * Velocity_[0] - Vec_[0] * DownWashVelocity_[2] )*dLength_dZ2*Gamma_ - ( dVecZ_dZ2 * Velocity_[0] - dVecX_dZ2 * DownWashVelocity_[2] )*Length_*Gamma_;
    dForce[17] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * DownWashVelocity_[1] )*dLength_dZ2*Gamma_ + ( dVecY_dZ2 * Velocity_[0] - dVecX_dZ2 * DownWashVelocity_[1] )*Length_*Gamma_;        

    // Minus sign for Trefftz forces
    
    dForce[ 0] *= -1.;
    dForce[ 1] *= -1.;
    dForce[ 2] *= -1.;         
                                                                                                                                   
    dForce[ 3] *= -1.;
    dForce[ 4] *= -1.;
    dForce[ 5] *= -1.;        

    dForce[ 6] *= -1.;
    dForce[ 7] *= -1.;
    dForce[ 8] *= -1.;       

    dForce[ 9] *= -1.;
    dForce[10] *= -1.;
    dForce[11] *= -1.;        

    dForce[12] *= -1.;
    dForce[13] *= -1.;
    dForce[14] *= -1.;        

    dForce[15] *= -1.;
    dForce[16] *= -1.;
    dForce[17] *= -1.;      

}

/*##############################################################################
#                                                                              #
#                   VSP_EDGE pForces_pVinf                                     #
#                                                                              #
##############################################################################*/

void VSP_EDGE::pForces_pVinf(double pForces_pVinf[3][3])
{
   
  // i    j    k
  // velx vely velz
  // vecx vecy vecz
  //
  // dForce[0] =  i x ( vecz * vely - vecy * velz )
  // dForce[1] = -j x ( vecz * velx - vecx * velz )
  // dForce[2] =  k x ( vecy * velx - vecx * vely )
  // 
  // dForce[0] =  i x ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )
  // dForce[1] = -j x ( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )
  // dForce[2] =  k x ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )      
  //
  //
  //  vector_cross(Velocity_, Vec_, Forces_);
  //
  //  dForce[0] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )*Length_*Gamma_;
  //  dForce[1] = -( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )*Length_*Gamma_;
  //  dForce[2] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )*Length_*Gamma_;     
  

    // X force
    
    pForces_pVinf[0][0] = 0.;
    pForces_pVinf[0][1] =  (  Vec_[2] )*Length_*Gamma_;
    pForces_pVinf[0][2] =  ( -Vec_[1] )*Length_*Gamma_;
    
    // Y force
    
    pForces_pVinf[1][0] = -(  Vec_[2] )*Length_*Gamma_;
    pForces_pVinf[1][1] = 0.;
    pForces_pVinf[1][2] = -( -Vec_[0] )*Length_*Gamma_;
    
    // Z force
       
    pForces_pVinf[2][0] =  (  Vec_[1] )*Length_*Gamma_;     
    pForces_pVinf[2][1] =  ( -Vec_[0] )*Length_*Gamma_;     
    pForces_pVinf[2][2] =  0.;    

}

/*##############################################################################
#                                                                              #
#                   VSP_EDGE pTrefftzForces_pVinf                              #
#                                                                              #
##############################################################################*/

void VSP_EDGE::pTrefftzForces_pVinf(double pForces_pVinf[3][3])
{
   
  // i    j    k
  // velx vely velz
  // vecx vecy vecz
  //
  // dForce[0] =  i x ( vecz * vely - vecy * velz )
  // dForce[1] = -j x ( vecz * velx - vecx * velz )
  // dForce[2] =  k x ( vecy * velx - vecx * vely )
  // 
  // dForce[0] =  i x ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )
  // dForce[1] = -j x ( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )
  // dForce[2] =  k x ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )      
  //
  //
  //  vector_cross(Velocity_, Vec_, Forces_);
  //
  //  dForce[0] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )*Length_*Gamma_;
  //  dForce[1] = -( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )*Length_*Gamma_;
  //  dForce[2] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )*Length_*Gamma_;     
  

    // X force
    
    pForces_pVinf[0][0] = 0.;
    pForces_pVinf[0][1] =  (  Vec_[2] )*Length_*Gamma_;
    pForces_pVinf[0][2] =  ( -Vec_[1] )*Length_*Gamma_;
    
    // Y force
    
    pForces_pVinf[1][0] = -(  Vec_[2] )*Length_*Gamma_;
    pForces_pVinf[1][1] = 0.;
    pForces_pVinf[1][2] = -( -Vec_[0] )*Length_*Gamma_;
    
    // Z force
       
    pForces_pVinf[2][0] =  (  Vec_[1] )*Length_*Gamma_;     
    pForces_pVinf[2][1] =  ( -Vec_[0] )*Length_*Gamma_;     
    pForces_pVinf[2][2] =  0.;    

    pForces_pVinf[0][0] *= 1.;
    pForces_pVinf[0][1] *= 1.;
    pForces_pVinf[0][2] *= 1.;
    pForces_pVinf[1][0] *= 1.;
    pForces_pVinf[1][1] *= 1.;
    pForces_pVinf[1][2] *= 1.;
    pForces_pVinf[2][0] *= 1.; 
    pForces_pVinf[2][1] *= 1.; 
    pForces_pVinf[2][2] *= 1.;
    
}

/*##############################################################################
#                                                                              #
#                   VSP_EDGE pMoments_pVinf                                    #
#                                                                              #
##############################################################################*/

void VSP_EDGE::pMoments_pVinf(double MomArm[3], double pMoments_pVinf[3][3])
{
   
  double pForces_pVinf[3][3];
   
  // i    j    k
  // velx vely velz
  // vecx vecy vecz
  //
  // dForce[0] =  i x ( vecz * vely - vecy * velz )
  // dForce[1] = -j x ( vecz * velx - vecx * velz )
  // dForce[2] =  k x ( vecy * velx - vecx * vely )
  // 
  // dForce[0] =  i x ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )
  // dForce[1] = -j x ( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )
  // dForce[2] =  k x ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )      
  //
  //
  //  vector_cross(Velocity_, Vec_, Forces_);
  //
  //  dForce[0] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )*Length_*Gamma_;
  //  dForce[1] = -( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )*Length_*Gamma_;
  //  dForce[2] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )*Length_*Gamma_;     
  

    // X force
    
    pForces_pVinf[0][0] = 0.;
    pForces_pVinf[0][1] =  (  Vec_[2] )*Length_*Gamma_;
    pForces_pVinf[0][2] =  ( -Vec_[1] )*Length_*Gamma_;
    
    // Y force
    
    pForces_pVinf[1][0] = -(  Vec_[2] )*Length_*Gamma_;
    pForces_pVinf[1][1] = 0.;
    pForces_pVinf[1][2] = -( -Vec_[0] )*Length_*Gamma_;
    
    // Z force
       
    pForces_pVinf[2][0] =  (  Vec_[1] )*Length_*Gamma_;     
    pForces_pVinf[2][1] =  ( -Vec_[0] )*Length_*Gamma_;     
    pForces_pVinf[2][2] =  0.;    
    
    
    // need r X F = vector_cross(MomArm, pForces_pVinf, pMoments_pVinf);
    
    // X moment

    pMoments_pVinf[0][0] =  ( pForces_pVinf[2][0] * MomArm[1] - pForces_pVinf[1][0] * MomArm[2] );
    pMoments_pVinf[0][1] =  ( pForces_pVinf[2][1] * MomArm[1] - pForces_pVinf[1][1] * MomArm[2] );
    pMoments_pVinf[0][2] =  ( pForces_pVinf[2][2] * MomArm[1] - pForces_pVinf[1][2] * MomArm[2] );
        
    // Y moment

    pMoments_pVinf[1][0] = -( pForces_pVinf[2][0] * MomArm[0] - pForces_pVinf[0][0] * MomArm[2] );
    pMoments_pVinf[1][1] = -( pForces_pVinf[2][1] * MomArm[0] - pForces_pVinf[0][1] * MomArm[2] );
    pMoments_pVinf[1][2] = -( pForces_pVinf[2][2] * MomArm[0] - pForces_pVinf[0][2] * MomArm[2] );
        
    // Z moment

    pMoments_pVinf[2][0] =  ( pForces_pVinf[1][0] * MomArm[0] - pForces_pVinf[0][0] * MomArm[1] );
    pMoments_pVinf[2][1] =  ( pForces_pVinf[1][1] * MomArm[0] - pForces_pVinf[0][1] * MomArm[1] );
    pMoments_pVinf[2][2] =  ( pForces_pVinf[1][2] * MomArm[0] - pForces_pVinf[0][2] * MomArm[1] );
}

/*##############################################################################
#                                                                              #
#                   VSP_EDGE pForces_pMach                                     #
#                                                                              #
##############################################################################*/

void VSP_EDGE::pForces_pMach(double pForces_pMach[3])
{

    // Mach gradient induced forces
    
    vector_cross(dVelocity_dMach_, Vec_, pForces_pMach);

    pForces_pMach[0] *= Length_*Gamma_;
    pForces_pMach[1] *= Length_*Gamma_;
    pForces_pMach[2] *= Length_*Gamma_;
   
}

/*##############################################################################
#                                                                              #
#                   VSP_EDGE pTrefftzForces_pMach                              #
#                                                                              #
##############################################################################*/

void VSP_EDGE::pTrefftzForces_pMach(double pForces_pMach[3])
{

    // Mach gradient induced forces
    
    vector_cross(dVelocity_dMach_, Vec_, pForces_pMach);

    pForces_pMach[0] *= -Length_*Gamma_;
    pForces_pMach[1] *= -Length_*Gamma_;
    pForces_pMach[2] *= -Length_*Gamma_;
   
}

/*##############################################################################
#                                                                              #
#                   VSP_EDGE pMoments_pMach                                    #
#                                                                              #
##############################################################################*/

void VSP_EDGE::pMoments_pMach(double MomArm[3], double pMoments_pMach[3])
{
   
  double pForces_pMach[3];
   
  // i    j    k
  // velx vely velz
  // vecx vecy vecz
  //
  // dForce[0] =  i x ( vecz * vely - vecy * velz )
  // dForce[1] = -j x ( vecz * velx - vecx * velz )
  // dForce[2] =  k x ( vecy * velx - vecx * vely )
  // 
  // dForce[0] =  i x ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )
  // dForce[1] = -j x ( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )
  // dForce[2] =  k x ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )      
  //
  //
  //  vector_cross(Velocity_, Vec_, Forces_);
  //
  //  dForce[0] =  ( Vec_[2] * Velocity_[1] - Vec_[1] * Velocity_[2] )*Length_*Gamma_;
  //  dForce[1] = -( Vec_[2] * Velocity_[0] - Vec_[0] * Velocity_[2] )*Length_*Gamma_;
  //  dForce[2] =  ( Vec_[1] * Velocity_[0] - Vec_[0] * Velocity_[1] )*Length_*Gamma_;     
  

    vector_cross(dVelocity_dMach_, Vec_, pForces_pMach);

    pForces_pMach[0] *= Length_*Gamma_;
    pForces_pMach[1] *= Length_*Gamma_;
    pForces_pMach[2] *= Length_*Gamma_;
    
    // X moment
    
    pMoments_pMach[0] =  ( MomArm[2] * pForces_pMach[1] - MomArm[1] * pForces_pMach[2] );
   
    // Y moment
   
    pMoments_pMach[1] = -( MomArm[2] * pForces_pMach[0] - MomArm[0] * pForces_pMach[2] );
    
    // Z moment
    
    pMoments_pMach[2] =  ( MomArm[1] * pForces_pMach[0] - MomArm[0] * pForces_pMach[1] );     
    
//    printf("pMoments_pMach: %f %f %f \n",pMoments_pMach[0],pMoments_pMach[1],pMoments_pMach[2]);

}
  
/*##############################################################################
#                                                                              #
#                      VSP_EDGE UpdateGeometryLocation                         #
#                                                                              #
##############################################################################*/

void VSP_EDGE::UpdateGeometryLocation(double *TVec, double *OVec, QUAT &Quat, QUAT &InvQuat)
{

    QUAT Vec;

    // Update node 1
    
    Vec(0) = X1_ - OVec[0];
    Vec(1) = Y1_ - OVec[1];
    Vec(2) = Z1_ - OVec[2];

    Vec = Quat * Vec * InvQuat;

    X1_ = Vec(0) + OVec[0] + TVec[0];
    Y1_ = Vec(1) + OVec[1] + TVec[1];
    Z1_ = Vec(2) + OVec[2] + TVec[2];    
    
    // Update node 2
    
    Vec(0) = X2_ - OVec[0];
    Vec(1) = Y2_ - OVec[1];
    Vec(2) = Z2_ - OVec[2];

    Vec = Quat * Vec * InvQuat;

    X2_ = Vec(0) + OVec[0] + TVec[0];
    Y2_ = Vec(1) + OVec[1] + TVec[1];
    Z2_ = Vec(2) + OVec[2] + TVec[2];        

    // Update centroid vector

    XYZc_[0] = 0.5*( X1_ + X2_ );
    XYZc_[1] = 0.5*( Y1_ + Y2_ );
    XYZc_[2] = 0.5*( Z1_ + Z2_ );
 
    // Unit vector in direction of edge
    
    Vec(0) = Vec_[0];
    Vec(1) = Vec_[1];
    Vec(2) = Vec_[2];

    Vec = Quat * Vec * InvQuat;
    
    Vec_[0] = Vec(0);
    Vec_[1] = Vec(1);
    Vec_[2] = Vec(2);

    u_ = Vec_[0] * Length_;
    v_ = Vec_[1] * Length_;
    w_ = Vec_[2] * Length_;
    
}
     
/*##############################################################################
#                                                                              #
#                      VSP_EDGE UpdateGeometryLocation                         #
#                                                                              #
##############################################################################*/

void VSP_EDGE::UpdateGeometryLocation(VSP_NODE &Node1, VSP_NODE &Node2)
{

    // Get beginning and end points of the bound vortex

    X1_ = Node1.x();
    Y1_ = Node1.y();
    Z1_ = Node1.z();

    X2_ = Node2.x();
    Y2_ = Node2.y();
    Z2_ = Node2.z();
    
    // Get center of filament

    XYZc_[0] = 0.5*( X1_ + X2_ );
    XYZc_[1] = 0.5*( Y1_ + Y2_ );
    XYZc_[2] = 0.5*( Z1_ + Z2_ );

    // Unit vector in direction of edge
    
    Vec_[0] = X2_ - X1_;
    Vec_[1] = Y2_ - Y1_;
    Vec_[2] = Z2_ - Z1_;
    
    Length_ = sqrt(vector_dot(Vec_,Vec_));
    
    u_ = Vec_[0];
    v_ = Vec_[1];
    w_ = Vec_[2];
    
    Vec_[0] /= Length_;
    Vec_[1] /= Length_;
    Vec_[2] /= Length_;

}

/*##############################################################################
#                                                                              #
#                        VSP_EDGE CreateWakeEdgeSpace                          #
#                                                                              #
##############################################################################*/

void VSP_EDGE::CreateWakeEdgeSpace(void)
{
   
   if ( WakeEdgeData_ != NULL ) delete WakeEdgeData_;
   
   WakeEdgeData_ = new WAKE_EDGE;
   
   IsWakeEdge_ = 1;
   
}

/*##############################################################################
#                                                                              #
#                      VSP_EDGE CalculateWakeResidual                          #
#                                                                              #
##############################################################################*/

void VSP_EDGE::CalculateWakeResidual(double *Residual)
{
    
    if ( IsWakeEdge_ && WakeEdgeData_->dS() > 0. ) {
    
       double Vec[3], Res[3], dx, dy, dz, Dot;
   
       if ( WakeEdgeData_->Direction() > 0 ) {
          
          dx = X2_ - X1_;
          dy = Y2_ - Y1_;
          dz = Z2_ - Z1_;
          
       }
              
       else {
          
          dx = X1_ - X2_;
          dy = Y1_ - Y2_;
          dz = Z1_ - Z2_;
          
       }          

       Vec[0] = Velocity_[0];
       Vec[1] = Velocity_[1];
       Vec[2] = Velocity_[2];

       if ( WakeEdgeData_->IsRotorWake() > 0 ) {

          // Subtract out free stream...
          
          Vec[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
          Vec[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
          Vec[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
          
          // Determine component in thrust direction
          
          Dot = vector_dot(Vec,WakeEdgeData_->RotorThrustVector());
          
          // Limiter... we only take corrections in minus the thrust direction
          
          Dot = MIN(Dot,0.);
          
          // Add back in free stream component
          
          Vec[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
          Vec[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
          Vec[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);    
                                   
       }  

       Q_ = sqrt(vector_dot(Vec,Vec));

       if ( Q_ > 0. ) {
       
          Vec[0] /= Q_;
          Vec[1] /= Q_;
          Vec[2] /= Q_;
        
          Residual[WakeEdgeData_->WakeResidualEquationNumberForX()] = -(dx - Vec[0]*WakeEdgeData_->dS());
          Residual[WakeEdgeData_->WakeResidualEquationNumberForY()] = -(dy - Vec[1]*WakeEdgeData_->dS()); 
          Residual[WakeEdgeData_->WakeResidualEquationNumberForZ()] = -(dz - Vec[2]*WakeEdgeData_->dS());   
          
          if ( WakeEdgeData_->IsRotorWake() > 0 ) {

             Res[0] = Residual[WakeEdgeData_->WakeResidualEquationNumberForX()];
             Res[1] = Residual[WakeEdgeData_->WakeResidualEquationNumberForY()];
             Res[2] = Residual[WakeEdgeData_->WakeResidualEquationNumberForZ()];
                       
             Dot = vector_dot(Res, WakeEdgeData_->RotorThrustVector());
             
             Residual[WakeEdgeData_->WakeResidualEquationNumberForX()] = Dot * WakeEdgeData_->RotorThrustVector(0);
             Residual[WakeEdgeData_->WakeResidualEquationNumberForY()] = Dot * WakeEdgeData_->RotorThrustVector(1);
             Residual[WakeEdgeData_->WakeResidualEquationNumberForZ()] = Dot * WakeEdgeData_->RotorThrustVector(2);
          
          }

       }
       
       else {
          
          Residual[WakeEdgeData_->WakeResidualEquationNumberForX()] = 0.;
          Residual[WakeEdgeData_->WakeResidualEquationNumberForY()] = 0.;
          Residual[WakeEdgeData_->WakeResidualEquationNumberForZ()] = 0.;

       }   
         
    }
 
}

/*##############################################################################
#                                                                              #
#                       VSP_EDGE WakeMatrixMultiply                            #
#                                                                              #
##############################################################################*/

void VSP_EDGE::WakeMatrixMultiply(double *VecIn, double *VecOut)
{
    
    if ( IsWakeEdge_ && WakeEdgeData_->dS() > 0. ) {

       if ( WakeEdgeData_->IsRotorWake() == 0 ) {
    
          double C1, C2, C3, C4;
       
          C1 = WakeEdgeData_->dS()/Q_;
          C2 = WakeEdgeData_->dS()*(U()*dU_dGamma() + V()*dV_dGamma() + W()*dW_dGamma() )/(Q_*Q_*Q_);
          C3 = WakeEdgeData_->dS()*(U()*dU_dMesh()  + V()*dV_dMesh()  + W()*dW_dMesh()  )/(Q_*Q_*Q_);
          C4 = WakeEdgeData_->dS()*(U()*dU_dStall() + V()*dV_dStall() + W()*dW_dStall() )/(Q_*Q_*Q_);
          
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()] - VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()-3] - (dU_dGamma() + dU_dMesh() + dU_dStall())*C1 + U()*(C2 + C3 + C4);  
          
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()] - VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()-3] - (dV_dGamma() + dV_dMesh() + dV_dStall())*C1 + V()*(C2 + C3 + C4);  
                                                                  
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()] - VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()-3] - (dW_dGamma() + dW_dMesh() + dW_dStall())*C1 + W()*(C2 + C3 + C4);  

       }
       
       else {
          
          double Dir[3], Mag, Dot, Vec[3], Res[3];

          double C1, C2, C3, C4;
       
          double VecR[3], VecP[3];
          double pVecR_Velocity[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pVelocity[3][3];
       
          double pUp_pGamma;                
          double pVp_pGamma;                
          double pWp_pGamma;
                         
          double pUp_pMesh;               
          double pVp_pMesh;               
          double pWp_pMesh;
                         
          double pUp_pStall;                
          double pVp_pStall;                
          double pWp_pStall;
       
          // Subtract out free stream...
          
          VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
          VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
          VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
          
          pVecR_Velocity[0] = 1.;
          pVecR_Velocity[1] = 1.;
          pVecR_Velocity[2] = 1.;
                    
          // Determine component in thrust direction
          
          Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
          
          // Limiter... we only take velocity components in minus the thrust direction
          
          if ( Dot < 0. ) {
             
             pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
             pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
             pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
       
          }
          
          else {
             
             Dot = 0.;
       
             pDot_pVecR[0] = 0.;
             pDot_pVecR[1] = 0.;
             pDot_pVecR[2] = 0.;
             
          }
       
          // Add back in free stream component
          
          VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
          VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
          VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
          
          pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
          pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
          pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                 
          // Chain rule
          
          pVecP_pVelocity[0][0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[0][1] = pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[0][2] = pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pVecP_pVelocity[1][0] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[1][1] = pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[1][2] = pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pVecP_pVelocity[2][0] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[2][1] = pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[2][2] = pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pUp_pGamma = pVecP_pVelocity[0][0] * dU_dGamma() + pVecP_pVelocity[0][1] * dV_dGamma() + pVecP_pVelocity[0][2] * dW_dGamma();                   
          pVp_pGamma = pVecP_pVelocity[1][0] * dU_dGamma() + pVecP_pVelocity[1][1] * dV_dGamma() + pVecP_pVelocity[1][2] * dW_dGamma();                   
          pWp_pGamma = pVecP_pVelocity[2][0] * dU_dGamma() + pVecP_pVelocity[2][1] * dV_dGamma() + pVecP_pVelocity[2][2] * dW_dGamma();   
          
          pUp_pMesh  = pVecP_pVelocity[0][0] * dU_dMesh()  + pVecP_pVelocity[0][1] * dV_dMesh()  + pVecP_pVelocity[0][2] * dW_dMesh();                   
          pVp_pMesh  = pVecP_pVelocity[1][0] * dU_dMesh()  + pVecP_pVelocity[1][1] * dV_dMesh()  + pVecP_pVelocity[1][2] * dW_dMesh();                   
          pWp_pMesh  = pVecP_pVelocity[2][0] * dU_dMesh()  + pVecP_pVelocity[2][1] * dV_dMesh()  + pVecP_pVelocity[2][2] * dW_dMesh();   
                          
          pUp_pStall = pVecP_pVelocity[0][0] * dU_dStall() + pVecP_pVelocity[0][1] * dV_dStall() + pVecP_pVelocity[0][2] * dW_dStall();                   
          pVp_pStall = pVecP_pVelocity[1][0] * dU_dStall() + pVecP_pVelocity[1][1] * dV_dStall() + pVecP_pVelocity[1][2] * dW_dStall();                   
          pWp_pStall = pVecP_pVelocity[2][0] * dU_dStall() + pVecP_pVelocity[2][1] * dV_dStall() + pVecP_pVelocity[2][2] * dW_dStall();   
       
          C1 = WakeEdgeData_->dS()/Q_;
          C2 = WakeEdgeData_->dS()*(VecP[0]*pUp_pGamma + VecP[1]*pVp_pGamma + VecP[2]*pWp_pGamma )/(Q_*Q_*Q_);
          C3 = WakeEdgeData_->dS()*(VecP[0]*pUp_pMesh  + VecP[1]*pVp_pMesh  + VecP[2]*pWp_pMesh  )/(Q_*Q_*Q_);
          C4 = WakeEdgeData_->dS()*(VecP[0]*pUp_pStall + VecP[1]*pVp_pStall + VecP[2]*pWp_pStall )/(Q_*Q_*Q_);
          
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()] - VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()-3] - (pUp_pGamma + pUp_pMesh + pUp_pStall)*C1 + VecP[0]*(C2 + C3 + C4);  
          
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()] - VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()-3] - (pVp_pGamma + pVp_pMesh + pVp_pStall)*C1 + VecP[1]*(C2 + C3 + C4);  
                                                                  
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()] - VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()-3] - (pWp_pGamma + pWp_pMesh + pWp_pStall)*C1 + VecP[2]*(C2 + C3 + C4);  

          // Now only solve a projection of the three wake equations into a single 1D solution in the rotor thrust direction
 
          Vec[0] = VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()];
          Vec[1] = VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()];
          Vec[2] = VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()];
          
          Dot = vector_dot(Vec,WakeEdgeData_->RotorThrustVector());
 
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()] = Dot * WakeEdgeData_->RotorThrustVector(0);
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()] = Dot * WakeEdgeData_->RotorThrustVector(1);
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()] = Dot * WakeEdgeData_->RotorThrustVector(2);

          if ( ABS(WakeEdgeData_->RotorThrustVector(0)) <= 100.*Tolerance_1_ ) VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()];
          if ( ABS(WakeEdgeData_->RotorThrustVector(1)) <= 100.*Tolerance_1_ ) VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()];
          if ( ABS(WakeEdgeData_->RotorThrustVector(2)) <= 100.*Tolerance_1_ ) VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];

       }
       
    }
    
    else {

       VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()];  
       
       VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()];  
                                                               
       VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];  
       
    }

}

/*##############################################################################
#                                                                              #
#                    VSP_EDGE DoWakeForwardMatrixPrecondition                  #
#                                                                              #
##############################################################################*/

void VSP_EDGE::DoWakeForwardMatrixPrecondition(double *VecIn)
{

    if ( IsWakeEdge_ && WakeEdgeData_->dS() > 0. ) {

       VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()] + VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()-3];  
       
       VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()] + VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()-3];  
                                                               
       VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()] + VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()-3];  

    }
    
}

/*##############################################################################
#                                                                              #
#                   VSP_EDGE CalculateUnsteadyWakeResidual                     #
#                                                                              #
##############################################################################*/

void VSP_EDGE::CalculateUnsteadyWakeResidualOld(double *Residual, double &DeltaTime)
{
    
    if ( IsWakeEdge_ && Time_ >= MinValidTimeStep_ ) {
    
       double dx, dy, dz;
   
       if ( WakeEdgeData_->Direction() > 0 ) {
          
          dx = X2_ - X2m_;
          dy = Y2_ - Y2m_;
          dz = Z2_ - Z2m_;
 
       }
              
       else {
          
          dx = X1_ - X1m_;
          dy = Y1_ - Y1m_;
          dz = Z1_ - Z1m_;
     
       }          
       
       Residual[WakeEdgeData_->WakeResidualEquationNumberForX()] = -(dx - Velocity_[0]*DeltaTime);
       Residual[WakeEdgeData_->WakeResidualEquationNumberForY()] = -(dy - Velocity_[1]*DeltaTime); 
       Residual[WakeEdgeData_->WakeResidualEquationNumberForZ()] = -(dz - Velocity_[2]*DeltaTime);   

    }
    
    else {
       
       Residual[WakeEdgeData_->WakeResidualEquationNumberForX()] = 0.;
       Residual[WakeEdgeData_->WakeResidualEquationNumberForY()] = 0.; 
       Residual[WakeEdgeData_->WakeResidualEquationNumberForZ()] = 0.;   

    }       
 
}

/*##############################################################################
#                                                                              #
#                       VSP_EDGE UnsteadyWakeMatrixMultiply                    #
#                                                                              #
##############################################################################*/

void VSP_EDGE::UnsteadyWakeMatrixMultiply(double *VecIn, double *VecOut, double &DeltaTime)
{
    
    if ( IsWakeEdge_ && Time_ >= MinValidTimeStep_ ) {
       
       if ( WakeEdgeData_->IsRotorWake() == 0 || WakeEdgeData_->IsRotorWake() == 2 ) {
       
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()] - (dU_dGamma() + dU_dMesh() + dU_dStall() + dU_dRatio())*DeltaTime;  
                                                                                                                                        
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()] - (dV_dGamma() + dV_dMesh() + dV_dStall() + dV_dRatio())*DeltaTime;  
                                                                                                                                         
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()] - (dW_dGamma() + dW_dMesh() + dW_dStall() + dW_dRatio())*DeltaTime;  
          
       }
       
       else {
          
          double VecR[3], VecP[3], Dot;
          double pVecR_Velocity[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pVelocity[3][3], pVecP_pStuff[3];

          // Subtract out free stream...
          
          VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
          VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
          VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
          
          pVecR_Velocity[0] = 1.;
          pVecR_Velocity[1] = 1.;
          pVecR_Velocity[2] = 1.;
                    
          // Determine component in thrust direction
          
          Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
          
          // Limiter... we only take corrections in minus the thrust direction
          
          if ( Dot < 0. ) {
             
             pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
             pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
             pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
      
          }
          
          else {
             
             Dot = 0.;

             pDot_pVecR[0] = 0.;
             pDot_pVecR[1] = 0.;
             pDot_pVecR[2] = 0.;
             
          }

          // Add back in free stream component
          
          VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
          VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
          VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
          
          pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
          pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
          pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                 
          // Chain rule
          
          pVecP_pVelocity[0][0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[0][1] = pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[0][2] = pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pVecP_pVelocity[1][0] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[1][1] = pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[1][2] = pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pVecP_pVelocity[2][0] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[2][1] = pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[2][2] = pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          // Residual[WakeEdgeData_->WakeResidualEquationNumberForX()] = -(dx - VecP[0]*DeltaTime);
          // Residual[WakeEdgeData_->WakeResidualEquationNumberForY()] = -(dy - VecP[1]*DeltaTime); 
          // Residual[WakeEdgeData_->WakeResidualEquationNumberForZ()] = -(dz - VecP[2]*DeltaTime);      
          
          pVecP_pStuff[0] = pVecP_pVelocity[0][0] * ( dU_dGamma() + dU_dMesh() + dU_dStall() + dU_dRatio() )
                          + pVecP_pVelocity[0][1] * ( dV_dGamma() + dV_dMesh() + dV_dStall() + dV_dRatio() )
                          + pVecP_pVelocity[0][2] * ( dW_dGamma() + dW_dMesh() + dW_dStall() + dW_dRatio() );
                             
          pVecP_pStuff[1] = pVecP_pVelocity[1][0] * ( dU_dGamma() + dU_dMesh() + dU_dStall() + dU_dRatio() )
                          + pVecP_pVelocity[1][1] * ( dV_dGamma() + dV_dMesh() + dV_dStall() + dV_dRatio() )
                          + pVecP_pVelocity[1][2] * ( dW_dGamma() + dW_dMesh() + dW_dStall() + dW_dRatio() );
                              
          pVecP_pStuff[2] = pVecP_pVelocity[2][0] * ( dU_dGamma() + dU_dMesh() + dU_dStall() + dU_dRatio() )
                          + pVecP_pVelocity[2][1] * ( dV_dGamma() + dV_dMesh() + dV_dStall() + dV_dRatio() )
                          + pVecP_pVelocity[2][2] * ( dW_dGamma() + dW_dMesh() + dW_dStall() + dW_dRatio() );
 
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()] - (pVecP_pStuff[0])*DeltaTime;  
                                                                                                                                        
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()] - (pVecP_pStuff[1])*DeltaTime;  
                                                                                                                                         
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()] - (pVecP_pStuff[2])*DeltaTime;  
                                                                
       }

    }
    
    else {

       VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()];  
       
       VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()];  
                                                               
       VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()] = VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];  
       
    }

}

/*##############################################################################
#                                                                              #
#              VSP_EDGE CalculateUnsteadyWakeResidual                          #
#                                                                              #
##############################################################################*/

void VSP_EDGE::CalculateUnsteadyWakeResidual(double *Residual, double &DeltaTime)
{
    
    if ( IsWakeEdge_ && Time_ >= MinValidTimeStep_ ) {
    
       double dx, dy, dz;
       double Vec[3], Dot;
   
       if ( WakeEdgeData_->Direction() > 0 ) {
          
          dx = X2_ - X2m_;
          dy = Y2_ - Y2m_;
          dz = Z2_ - Z2m_;
 
       }
              
       else {
          
          dx = X1_ - X1m_;
          dy = Y1_ - Y1m_;
          dz = Z1_ - Z1m_;
     
       }          
       
       if ( WakeEdgeData_->IsRotorWake() == 0 || WakeEdgeData_->IsRotorWake() == 2 ) {

          Vec[0] = Velocity_[0];
          Vec[1] = Velocity_[1];
          Vec[2] = Velocity_[2];
       
       }
       
       else {
                 
          // Subtract out free stream...
          
          Vec[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
          Vec[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
          Vec[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
          
          // Determine component in thrust direction
          
          Dot = vector_dot(Vec,WakeEdgeData_->RotorThrustVector());
          
          // Limiter... we only take corrections in minus the thrust direction
          
          Dot = MIN(Dot,0.);
          
          // Add back in free stream component
          
          Vec[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
          Vec[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
          Vec[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);    
          
       }  
       
       Residual[WakeEdgeData_->WakeResidualEquationNumberForX()] = -(dx - Vec[0]*DeltaTime);
       Residual[WakeEdgeData_->WakeResidualEquationNumberForY()] = -(dy - Vec[1]*DeltaTime); 
       Residual[WakeEdgeData_->WakeResidualEquationNumberForZ()] = -(dz - Vec[2]*DeltaTime);   

    }
    
    else {
       
       Residual[WakeEdgeData_->WakeResidualEquationNumberForX()] = 0.;
       Residual[WakeEdgeData_->WakeResidualEquationNumberForY()] = 0.; 
       Residual[WakeEdgeData_->WakeResidualEquationNumberForZ()] = 0.;   

    }       
 
}

/*##############################################################################
#                                                                              #
#               VSP_EDGE WakeTranposeMatrixMultiply                            #
#                                                                              #
##############################################################################*/

double VSP_EDGE::WakeTranposeMatrixMultiply(double q[3][3])
{
    
    double Coef;
    
    if ( IsWakeEdge_ && WakeEdgeData_->dS() > 0. && !IsFrozenTrailingEdge_ && !IsConcaveTrailingEdge_ ) {
    
       if ( WakeEdgeData_->IsRotorWake() == 0 ) {
       
          double C1;
          
          C1 = WakeEdgeData_->dS()/(Q_*Q_*Q_);
          
          Coef = -WakeEdgeData_->dS()*q[0][0]/Q_ + U() * C1 * ( U()*q[0][0] + V()*q[0][1] + W()*q[0][2] )
                 -WakeEdgeData_->dS()*q[1][1]/Q_ + V() * C1 * ( U()*q[1][0] + V()*q[1][1] + W()*q[1][2] )
                 -WakeEdgeData_->dS()*q[2][2]/Q_ + W() * C1 * ( U()*q[2][0] + V()*q[2][1] + W()*q[2][2] );
                 
       }
       
       else {
          
          double C1, CoefX, CoefY, CoefZ;          
          double VecR[3], VecP[3], Dot;
          double pVecR_Velocity[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pVelocity[3][3], pVecP_pXYZ[3][3];

          // Subtract out free stream...
          
          VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
          VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
          VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
          
          pVecR_Velocity[0] = 1.;
          pVecR_Velocity[1] = 1.;
          pVecR_Velocity[2] = 1.;
                    
          // Determine component in thrust direction
          
          Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
          
          // Limiter... we only use velocities in minus the thrust direction
          
          if ( Dot < 0. ) {
             
             pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
             pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
             pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
      
          }
          
          else {
             
             Dot = 0.;

             pDot_pVecR[0] = 0.;
             pDot_pVecR[1] = 0.;
             pDot_pVecR[2] = 0.;
             
          }

          // Add back in free stream component
          
          VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
          VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
          VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
          
          pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
          pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
          pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                 
          // Chain rule
          
          pVecP_pVelocity[0][0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[0][1] = pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[0][2] = pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pVecP_pVelocity[1][0] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[1][1] = pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[1][2] = pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pVecP_pVelocity[2][0] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[2][1] = pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[2][2] = pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Velocity[2];

          // Up
          
          pVecP_pXYZ[0][0] = pVecP_pVelocity[0][0] * q[0][0]
                           + pVecP_pVelocity[0][1] * q[0][1]
                           + pVecP_pVelocity[0][2] * q[0][2];

          pVecP_pXYZ[1][0] = pVecP_pVelocity[0][0] * q[1][0]
                           + pVecP_pVelocity[0][1] * q[1][1]
                           + pVecP_pVelocity[0][2] * q[1][2];
                           
          pVecP_pXYZ[2][0] = pVecP_pVelocity[0][0] * q[2][0]
                           + pVecP_pVelocity[0][1] * q[2][1]
                           + pVecP_pVelocity[0][2] * q[2][2];
                           
          // Vp
          
          pVecP_pXYZ[0][1] = pVecP_pVelocity[1][0] * q[0][0]
                           + pVecP_pVelocity[1][1] * q[0][1]
                           + pVecP_pVelocity[1][2] * q[0][2];
                                           
          pVecP_pXYZ[1][1] = pVecP_pVelocity[1][0] * q[1][0]
                           + pVecP_pVelocity[1][1] * q[1][1]
                           + pVecP_pVelocity[1][2] * q[1][2];
                                            
          pVecP_pXYZ[2][1] = pVecP_pVelocity[1][0] * q[2][0]
                           + pVecP_pVelocity[1][1] * q[2][1]
                           + pVecP_pVelocity[1][2] * q[2][2];                           
                           
          // Wp
          
          pVecP_pXYZ[0][2] = pVecP_pVelocity[2][0] * q[0][0]
                           + pVecP_pVelocity[2][1] * q[0][1]
                           + pVecP_pVelocity[2][2] * q[0][2];
                                            
          pVecP_pXYZ[1][2] = pVecP_pVelocity[2][0] * q[1][0]
                           + pVecP_pVelocity[2][1] * q[1][1]
                           + pVecP_pVelocity[2][2] * q[1][2];
                                            
          pVecP_pXYZ[2][2] = pVecP_pVelocity[2][0] * q[2][0]
                           + pVecP_pVelocity[2][1] * q[2][1]
                           + pVecP_pVelocity[2][2] * q[2][2];    

          C1 = WakeEdgeData_->dS()/(Q_*Q_*Q_);
          
          CoefX = ( -WakeEdgeData_->dS()*pVecP_pXYZ[0][0]/Q_ + VecP[0] * C1 * ( VecP[0]*pVecP_pXYZ[0][0] + VecP[1]*pVecP_pXYZ[0][1] + VecP[2]*pVecP_pXYZ[0][2] ) )*WakeEdgeData_->RotorThrustVector(0)
                + ( -WakeEdgeData_->dS()*pVecP_pXYZ[0][1]/Q_ + VecP[1] * C1 * ( VecP[0]*pVecP_pXYZ[0][0] + VecP[1]*pVecP_pXYZ[0][1] + VecP[2]*pVecP_pXYZ[0][2] ) )*WakeEdgeData_->RotorThrustVector(1)
                + ( -WakeEdgeData_->dS()*pVecP_pXYZ[0][2]/Q_ + VecP[2] * C1 * ( VecP[0]*pVecP_pXYZ[0][0] + VecP[1]*pVecP_pXYZ[0][1] + VecP[2]*pVecP_pXYZ[0][2] ) )*WakeEdgeData_->RotorThrustVector(2);
                                                                                                                                                            
          CoefY = ( -WakeEdgeData_->dS()*pVecP_pXYZ[1][0]/Q_ + VecP[0] * C1 * ( VecP[0]*pVecP_pXYZ[1][0] + VecP[1]*pVecP_pXYZ[1][1] + VecP[2]*pVecP_pXYZ[1][2] ) )*WakeEdgeData_->RotorThrustVector(0)
                + ( -WakeEdgeData_->dS()*pVecP_pXYZ[1][1]/Q_ + VecP[1] * C1 * ( VecP[0]*pVecP_pXYZ[1][0] + VecP[1]*pVecP_pXYZ[1][1] + VecP[2]*pVecP_pXYZ[1][2] ) )*WakeEdgeData_->RotorThrustVector(1)
                + ( -WakeEdgeData_->dS()*pVecP_pXYZ[1][2]/Q_ + VecP[2] * C1 * ( VecP[0]*pVecP_pXYZ[1][0] + VecP[1]*pVecP_pXYZ[1][1] + VecP[2]*pVecP_pXYZ[1][2] ) )*WakeEdgeData_->RotorThrustVector(2);
                                                                                                                                                            
          CoefZ = ( -WakeEdgeData_->dS()*pVecP_pXYZ[2][0]/Q_ + VecP[0] * C1 * ( VecP[0]*pVecP_pXYZ[2][0] + VecP[1]*pVecP_pXYZ[2][1] + VecP[2]*pVecP_pXYZ[2][2] ) )*WakeEdgeData_->RotorThrustVector(0)
                + ( -WakeEdgeData_->dS()*pVecP_pXYZ[2][1]/Q_ + VecP[1] * C1 * ( VecP[0]*pVecP_pXYZ[2][0] + VecP[1]*pVecP_pXYZ[2][1] + VecP[2]*pVecP_pXYZ[2][2] ) )*WakeEdgeData_->RotorThrustVector(1)
                + ( -WakeEdgeData_->dS()*pVecP_pXYZ[2][2]/Q_ + VecP[2] * C1 * ( VecP[0]*pVecP_pXYZ[2][0] + VecP[1]*pVecP_pXYZ[2][1] + VecP[2]*pVecP_pXYZ[2][2] ) )*WakeEdgeData_->RotorThrustVector(2);

          Coef = CoefX * WakeEdgeData_->RotorThrustVector(0)
               + CoefY * WakeEdgeData_->RotorThrustVector(1)
               + CoefZ * WakeEdgeData_->RotorThrustVector(2);

       }
 
    }
    
    else {
       
       Coef = 0.;
       
    }
    
    return Coef;

}

/*##############################################################################
#                                                                              #
#               VSP_EDGE UnsteadyWakeTranposeMatrixMultiply                    #
#                                                                              #
##############################################################################*/

double VSP_EDGE::UnsteadyWakeTranposeMatrixMultiply(double q[3][3], double &DeltaTime)
{
    
    double Coef;
    
    if ( IsWakeEdge_ && Time_ >= MinValidTimeStep_ ) {

       if ( WakeEdgeData_->IsRotorWake() == 0 || WakeEdgeData_->IsRotorWake() == 2 ) {

          Coef = -DeltaTime*( q[0][0] + q[1][1] + q[2][2] );
          
       }
       
       else {
          
          double VecR[3], VecP[3], Dot;
          double pVecR_Velocity[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pVelocity[3][3], pVecP_pXYZ[3];

          // Subtract out free stream..
          
          VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
          VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
          VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
          
          pVecR_Velocity[0] = 1.;
          pVecR_Velocity[1] = 1.;
          pVecR_Velocity[2] = 1.;
                    
          // Determine component in thrust direction
          
          Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
          
          // Limiter... we only take corrections in minus the thrust direction
          
          if ( Dot < 0. ) {
             
             pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
             pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
             pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
      
          }
          
          else {
             
             Dot = 0.;

             pDot_pVecR[0] = 0.;
             pDot_pVecR[1] = 0.;
             pDot_pVecR[2] = 0.;
             
          }

          // Add back in free stream component
          
          VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
          VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
          VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
          
          pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
          pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
          pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                 
          // Chain rule
          
          pVecP_pVelocity[0][0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[0][1] = pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[0][2] = pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pVecP_pVelocity[1][0] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[1][1] = pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[1][2] = pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pVecP_pVelocity[2][0] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[2][1] = pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[2][2] = pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          // Residual[WakeEdgeData_->WakeResidualEquationNumberForX()] = -(dx - VecP[0]*DeltaTime);
          // Residual[WakeEdgeData_->WakeResidualEquationNumberForY()] = -(dy - VecP[1]*DeltaTime); 
          // Residual[WakeEdgeData_->WakeResidualEquationNumberForZ()] = -(dz - VecP[2]*DeltaTime);      
          
          pVecP_pXYZ[0] = pVecP_pVelocity[0][0] * q[0][0]
                        + pVecP_pVelocity[0][1] * q[0][1]
                        + pVecP_pVelocity[0][2] * q[0][2];
                                                         
          pVecP_pXYZ[1] = pVecP_pVelocity[1][0] * q[1][0]
                        + pVecP_pVelocity[1][1] * q[1][1]
                        + pVecP_pVelocity[1][2] * q[1][2];
                                                         
          pVecP_pXYZ[2] = pVecP_pVelocity[2][0] * q[2][0]
                        + pVecP_pVelocity[2][1] * q[2][1]
                        + pVecP_pVelocity[2][2] * q[2][2];
          
          Coef = -DeltaTime*( pVecP_pXYZ[0] + pVecP_pXYZ[1] + pVecP_pXYZ[2] );
                    
       }
               
    }
    
    else {
       
       Coef = 0.;
       
    }
    
    return Coef;

}

/*##############################################################################
#                                                                              #
#               VSP_EDGE WakeTranposeMatrixMultiplyMeshPerturbations           #
#                                                                              #
##############################################################################*/

void VSP_EDGE::WakeTranposeMatrixMultiplyMeshPerturbations(double *VecIn, double *VecOut)
{
    
    double Coef;
    
    if ( IsWakeEdge_ && WakeEdgeData_->dS() > 0. ) {
       
       if ( WakeEdgeData_->IsRotorWake() == 0 ) {

          VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()-3] += -VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()];  
                                                                                                                             
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()-3] += -VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()];  
                                                                                                                             
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()-3] += -VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];  
              
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()  ] +=  VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()];  
                                                                                                                             
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()  ] +=  VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()];  
                                                                                                                             
          VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()  ] +=  VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];   
          
       }
       
       else {
          
         double r1,r2,r3;
         
         r1 = WakeEdgeData_->RotorThrustVector(0);
         r2 = WakeEdgeData_->RotorThrustVector(1);
         r3 = WakeEdgeData_->RotorThrustVector(2);
         
         VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()-3] += -r1*r1*VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()]
                                                                      -r1*r2*VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()]
                                                                      -r1*r3*VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];

         VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()-3] += -r2*r1*VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()]
                                                                      -r2*r2*VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()]
                                                                      -r2*r3*VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];

         VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()-3] += -r3*r1*VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()]
                                                                      -r3*r2*VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()]
                                                                      -r3*r3*VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];


         VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()  ] +=  r1*r1*VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()]
                                                                      +r1*r2*VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()]
                                                                      +r1*r3*VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];
                                                                       
         VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()  ] +=  r2*r1*VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()]
                                                                      +r2*r2*VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()]
                                                                      +r2*r3*VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];
                                                                       
         VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()  ] +=  r3*r1*VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()]
                                                                      +r3*r2*VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()]
                                                                      +r3*r3*VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];
                 
         if ( ABS(WakeEdgeData_->RotorThrustVector(0)) <= 100.*Tolerance_1_ ) VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()  ] +=  VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()];  
         if ( ABS(WakeEdgeData_->RotorThrustVector(1)) <= 100.*Tolerance_1_ ) VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()  ] +=  VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()];  
         if ( ABS(WakeEdgeData_->RotorThrustVector(2)) <= 100.*Tolerance_1_ ) VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()  ] +=  VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];  
          
          
       }   

    }
    
    else {
      
       VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()] += VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()];  
       
       VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()] += VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()];  
                                                               
       VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()] += VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];  
             
    }

}

/*##############################################################################
#                                                                              #
#           VSP_EDGE UnsteadyWakeTranposeMatrixMultiplyMeshPerturbations       #
#                                                                              #
##############################################################################*/

void VSP_EDGE::UnsteadyWakeTranposeMatrixMultiplyMeshPerturbations(double *VecIn, double *VecOut)
{
    
    double Coef;
    
    if ( IsWakeEdge_ && Time_ >= MinValidTimeStep_ ) {

       VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()] += VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()];  
                                                                                                                       
       VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()] += VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()];  
                                                                                                                       
       VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()] += VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];      

    }
    
    else {
      
       VecOut[WakeEdgeData_->WakeResidualEquationNumberForX()] += VecIn[WakeEdgeData_->WakeResidualEquationNumberForX()];  
       
       VecOut[WakeEdgeData_->WakeResidualEquationNumberForY()] += VecIn[WakeEdgeData_->WakeResidualEquationNumberForY()];  
                                                               
       VecOut[WakeEdgeData_->WakeResidualEquationNumberForZ()] += VecIn[WakeEdgeData_->WakeResidualEquationNumberForZ()];  
             
    }

}

/*##############################################################################
#                                                                              #
#                     VSP_EDGE dInducedVelocity_dMesh_Transpose                #
#                                                                              #
##############################################################################*/

void VSP_EDGE::dInducedVelocity_dMesh_Transpose(double xyz_p[3],
                                                double &dq0_dx1,
                                                double &dq0_dy1,
                                                double &dq0_dz1,
                                                double &dq0_dx2,
                                                double &dq0_dy2,
                                                double &dq0_dz2,
                                                               
                                                double &dq1_dx1,
                                                double &dq1_dy1,
                                                double &dq1_dz1,
                                                double &dq1_dx2,
                                                double &dq1_dy2,
                                                double &dq1_dz2,
                                                               
                                                double &dq2_dx1,
                                                double &dq2_dy1,
                                                double &dq2_dz1,
                                                double &dq2_dx2,
                                                double &dq2_dy2,
                                                double &dq2_dz2,
                                                
                                                double &dq0_dxp,
                                                double &dq0_dyp,
                                                double &dq0_dzp,
                                                                   
                                                double &dq1_dxp,
                                                double &dq1_dyp,
                                                double &dq1_dzp,
                                                                     
                                                double &dq2_dxp,
                                                double &dq2_dyp,
                                                double &dq2_dzp,
                                                
                                                double &dq0_dMach,
                                                double &dq1_dMach,
                                                double &dq2_dMach)                              
                                                                                                                                                          
{

    GradientBoundVortex(Gamma_, xyz_p,
                        dq0_dx1,
                        dq0_dy1,
                        dq0_dz1,
                        dq0_dx2,
                        dq0_dy2,
                        dq0_dz2,
                        
                        dq1_dx1,
                        dq1_dy1,
                        dq1_dz1,
                        dq1_dx2,
                        dq1_dy2,
                        dq1_dz2,
                        
                        dq2_dx1,
                        dq2_dy1,
                        dq2_dz1,
                        dq2_dx2,
                        dq2_dy2,
                        dq2_dz2,
                        
                        dq0_dxp,
                        dq0_dyp,
                        dq0_dzp,
                           
                        dq1_dxp,
                        dq1_dyp,
                        dq1_dzp,
                             
                        dq2_dxp,
                        dq2_dyp,
                        dq2_dzp,                      
                        
                        dq0_dMach,
                        dq1_dMach,
                        dq2_dMach);                               
                                                               
}

/*##############################################################################
#                                                                              #
#               VSP_EDGE dInducedVelocity_dMesh_dMesh_Forward                  #
#                                                                              #
##############################################################################*/

void VSP_EDGE::dInducedVelocity_dMesh_dMesh_Forward(double &dq0_dx1,
                                                    double &dq0_dy1,
                                                    double &dq0_dz1,
                                                    double &dq0_dx2,
                                                    double &dq0_dy2,
                                                    double &dq0_dz2,
                                                                    
                                                    double &dq1_dx1,
                                                    double &dq1_dy1,
                                                    double &dq1_dz1,
                                                    double &dq1_dx2,
                                                    double &dq1_dy2,
                                                    double &dq1_dz2,
                                                                        
                                                    double &dq2_dx1,
                                                    double &dq2_dy1,
                                                    double &dq2_dz1,
                                                    double &dq2_dx2,
                                                    double &dq2_dy2,
                                                    double &dq2_dz2,
                                                                
                                                    double &dq0_dxp,
                                                    double &dq0_dyp,
                                                    double &dq0_dzp,
                                                                
                                                    double &dq1_dxp,
                                                    double &dq1_dyp,
                                                    double &dq1_dzp,
                                                                
                                                    double &dq2_dxp,
                                                    double &dq2_dyp,
                                                    double &dq2_dzp,    
                                                                                      
                                                    double Coef[3][9])
{

    if ( IsWakeEdge_ && WakeEdgeData_->dS() > 0. && !IsConcaveTrailingEdge_ && !IsFrozenTrailingEdge_) {

       // Faraway edge, x equation
       
       Coef[0][0] = ( -dq0_dx1*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dx1 + V()*dq1_dx1 + W()*dq2_dx1)/(Q_*Q_*Q_) );
       Coef[0][1] = ( -dq0_dy1*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dy1 + V()*dq1_dx1 + W()*dq2_dy1)/(Q_*Q_*Q_) );
       Coef[0][2] = ( -dq0_dz1*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dz1 + V()*dq1_dx1 + W()*dq2_dz1)/(Q_*Q_*Q_) );     
       
       Coef[0][3] = ( -dq0_dx2*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dx2 + V()*dq1_dx2 + W()*dq2_dx2)/(Q_*Q_*Q_) );
       Coef[0][4] = ( -dq0_dy2*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dy2 + V()*dq1_dx2 + W()*dq2_dy2)/(Q_*Q_*Q_) );
       Coef[0][5] = ( -dq0_dz2*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dz2 + V()*dq1_dx2 + W()*dq2_dz2)/(Q_*Q_*Q_) );     
                              
       // Faraway edge, y equation
       
       Coef[1][0] = ( -dq1_dx1*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dx1 + V()*dq1_dx1 + W()*dq2_dx1)/(Q_*Q_*Q_) );
       Coef[1][1] = ( -dq1_dy1*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dy1 + V()*dq1_dx1 + W()*dq2_dy1)/(Q_*Q_*Q_) );
       Coef[1][2] = ( -dq1_dz1*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dz1 + V()*dq1_dx1 + W()*dq2_dz1)/(Q_*Q_*Q_) );     
                                                        
       Coef[1][3] = ( -dq1_dx2*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dx2 + V()*dq1_dx2 + W()*dq2_dx2)/(Q_*Q_*Q_) );
       Coef[1][4] = ( -dq1_dy2*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dy2 + V()*dq1_dx2 + W()*dq2_dy2)/(Q_*Q_*Q_) );
       Coef[1][5] = ( -dq1_dz2*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dz2 + V()*dq1_dx2 + W()*dq2_dz2)/(Q_*Q_*Q_) );     
                
       // Faraway edge, z equation
       
       Coef[2][0] = ( -dq2_dx1*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dx1 + V()*dq1_dx1 + W()*dq2_dx1)/(Q_*Q_*Q_) );
       Coef[2][1] = ( -dq2_dy1*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dy1 + V()*dq1_dx1 + W()*dq2_dy1)/(Q_*Q_*Q_) );
       Coef[2][2] = ( -dq2_dz1*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dz1 + V()*dq1_dx1 + W()*dq2_dz1)/(Q_*Q_*Q_) );     
                                                   
       Coef[2][3] = ( -dq2_dx2*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dx2 + V()*dq1_dx2 + W()*dq2_dx2)/(Q_*Q_*Q_) );
       Coef[2][4] = ( -dq2_dy2*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dy2 + V()*dq1_dx2 + W()*dq2_dy2)/(Q_*Q_*Q_) );
       Coef[2][5] = ( -dq2_dz2*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dz2 + V()*dq1_dx2 + W()*dq2_dz2)/(Q_*Q_*Q_) );         
       
       // This edge, x equation
       
       Coef[0][6] = ( -dq0_dxp*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dxp + V()*dq1_dxp + W()*dq2_dxp)/(Q_*Q_*Q_) );
       Coef[0][7] = ( -dq0_dyp*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dyp + V()*dq1_dxp + W()*dq2_dyp)/(Q_*Q_*Q_) );
       Coef[0][8] = ( -dq0_dzp*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dzp + V()*dq1_dxp + W()*dq2_dzp)/(Q_*Q_*Q_) );     
                                                   
       // This edge, y equation
       
       Coef[1][6] = ( -dq1_dxp*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dxp + V()*dq1_dxp + W()*dq2_dxp)/(Q_*Q_*Q_) );
       Coef[1][7] = ( -dq1_dyp*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dyp + V()*dq1_dxp + W()*dq2_dyp)/(Q_*Q_*Q_) );
       Coef[1][8] = ( -dq1_dzp*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dzp + V()*dq1_dxp + W()*dq2_dzp)/(Q_*Q_*Q_) );     
                
       // This edge, z equation
       
       Coef[2][6] = ( -dq2_dxp*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dxp + V()*dq1_dxp + W()*dq2_dxp)/(Q_*Q_*Q_) );
       Coef[2][7] = ( -dq2_dyp*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dyp + V()*dq1_dxp + W()*dq2_dyp)/(Q_*Q_*Q_) );
       Coef[2][8] = ( -dq2_dzp*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dzp + V()*dq1_dxp + W()*dq2_dzp)/(Q_*Q_*Q_) );     

    }
    
    else {
       
       Coef[0][0] = 0.;
       Coef[0][1] = 0.;
       Coef[0][2] = 0.;  
                    
       Coef[0][3] = 0.;
       Coef[0][4] = 0.;
       Coef[0][5] = 0.;  
                    
       Coef[1][0] = 0.;
       Coef[1][1] = 0.;
       Coef[1][2] = 0.;  
                   
       Coef[1][3] = 0.;
       Coef[1][4] = 0.;
       Coef[1][5] = 0.;  
     
       Coef[2][0] = 0.;
       Coef[2][1] = 0.;
       Coef[2][2] = 0.;  
                   
       Coef[2][3] = 0.;
       Coef[2][4] = 0.;
       Coef[2][5] = 0.;      
   
       Coef[0][6] = 0.;
       Coef[0][7] = 0.;
       Coef[0][8] = 0.;  
              
       Coef[1][6] = 0.;
       Coef[1][7] = 0.;
       Coef[1][8] = 0.;  
                  
       Coef[2][6] = 0.;
       Coef[2][7] = 0.;
       Coef[2][8] = 0.;  
              
    }       
                  
}

/*##############################################################################
#                                                                              #
#               VSP_EDGE dInducedVelocity_dMesh_dMesh_Transpose                #
#                                                                              #
##############################################################################*/

void VSP_EDGE::dInducedVelocity_dMesh_dMesh_Transpose(double &dq0_dx1,
                                                      double &dq0_dy1,
                                                      double &dq0_dz1,
                                                      double &dq0_dx2,
                                                      double &dq0_dy2,
                                                      double &dq0_dz2,
                                                                      
                                                      double &dq1_dx1,
                                                      double &dq1_dy1,
                                                      double &dq1_dz1,
                                                      double &dq1_dx2,
                                                      double &dq1_dy2,
                                                      double &dq1_dz2,
                                                                          
                                                      double &dq2_dx1,
                                                      double &dq2_dy1,
                                                      double &dq2_dz1,
                                                      double &dq2_dx2,
                                                      double &dq2_dy2,
                                                      double &dq2_dz2,
                                                                  
                                                      double &dq0_dxp,
                                                      double &dq0_dyp,
                                                      double &dq0_dzp,
                                                                  
                                                      double &dq1_dxp,
                                                      double &dq1_dyp,
                                                      double &dq1_dzp,
                                                                  
                                                      double &dq2_dxp,
                                                      double &dq2_dyp,
                                                      double &dq2_dzp,    
                                                                                        
                                                      double Coef[9])
{

    if ( IsWakeEdge_ && WakeEdgeData_->dS() > 0. && !IsFrozenTrailingEdge_ && !IsConcaveTrailingEdge_ ) {

       if ( WakeEdgeData_->Direction() == 1 ) {
          
          if ( WakeEdgeData_->IsRotorWake() == 0 ) {
             
             // Faraway edge
             
             Coef[0] =     ( -dq0_dx1*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dx1 + V()*dq1_dx1 + W()*dq2_dx1)/(Q_*Q_*Q_) )*dxyz2_[0]
                     +     ( -dq1_dx1*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dx1 + V()*dq1_dx1 + W()*dq2_dx1)/(Q_*Q_*Q_) )*dxyz2_[1]
                     +     ( -dq2_dx1*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dx1 + V()*dq1_dx1 + W()*dq2_dx1)/(Q_*Q_*Q_) )*dxyz2_[2];                  
                                                                                                                                  
             Coef[1] =     ( -dq0_dy1*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dy1 + V()*dq1_dy1 + W()*dq2_dy1)/(Q_*Q_*Q_) )*dxyz2_[0]
                     +     ( -dq1_dy1*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dy1 + V()*dq1_dy1 + W()*dq2_dy1)/(Q_*Q_*Q_) )*dxyz2_[1]
                     +     ( -dq2_dy1*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dy1 + V()*dq1_dy1 + W()*dq2_dy1)/(Q_*Q_*Q_) )*dxyz2_[2];
                                                                                                                                              
             Coef[2] =     ( -dq0_dz1*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dz1 + V()*dq1_dz1 + W()*dq2_dz1)/(Q_*Q_*Q_) )*dxyz2_[0]
                     +     ( -dq1_dz1*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dz1 + V()*dq1_dz1 + W()*dq2_dz1)/(Q_*Q_*Q_) )*dxyz2_[1]          
                     +     ( -dq2_dz1*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dz1 + V()*dq1_dz1 + W()*dq2_dz1)/(Q_*Q_*Q_) )*dxyz2_[2];  
                     
             Coef[3] =     ( -dq0_dx2*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dx2 + V()*dq1_dx2 + W()*dq2_dx2)/(Q_*Q_*Q_) )*dxyz2_[0]
                     +     ( -dq1_dx2*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dx2 + V()*dq1_dx2 + W()*dq2_dx2)/(Q_*Q_*Q_) )*dxyz2_[1]
                     +     ( -dq2_dx2*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dx2 + V()*dq1_dx2 + W()*dq2_dx2)/(Q_*Q_*Q_) )*dxyz2_[2];
                                         
             Coef[4] =     ( -dq0_dy2*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dy2 + V()*dq1_dy2 + W()*dq2_dy2)/(Q_*Q_*Q_) )*dxyz2_[0]
                     +     ( -dq1_dy2*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dy2 + V()*dq1_dy2 + W()*dq2_dy2)/(Q_*Q_*Q_) )*dxyz2_[1]
                     +     ( -dq2_dy2*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dy2 + V()*dq1_dy2 + W()*dq2_dy2)/(Q_*Q_*Q_) )*dxyz2_[2]; 
                                                       
             Coef[5] =     ( -dq0_dz2*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dz2 + V()*dq1_dz2 + W()*dq2_dz2)/(Q_*Q_*Q_) )*dxyz2_[0]
                     +     ( -dq1_dz2*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dz2 + V()*dq1_dz2 + W()*dq2_dz2)/(Q_*Q_*Q_) )*dxyz2_[1]          
                     +     ( -dq2_dz2*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dz2 + V()*dq1_dz2 + W()*dq2_dz2)/(Q_*Q_*Q_) )*dxyz2_[2]; 
   
             // This edge
             
             Coef[6] = 0.5*( -dq0_dxp*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dxp + V()*dq1_dxp + W()*dq2_dxp)/(Q_*Q_*Q_) )*dxyz2_[0]
                     + 0.5*( -dq1_dxp*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dxp + V()*dq1_dxp + W()*dq2_dxp)/(Q_*Q_*Q_) )*dxyz2_[1]
                     + 0.5*( -dq2_dxp*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dxp + V()*dq1_dxp + W()*dq2_dxp)/(Q_*Q_*Q_) )*dxyz2_[2];                  
                                                                                                                                  
             Coef[7] = 0.5*( -dq0_dyp*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dyp + V()*dq1_dyp + W()*dq2_dyp)/(Q_*Q_*Q_) )*dxyz2_[0]
                     + 0.5*( -dq1_dyp*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dyp + V()*dq1_dyp + W()*dq2_dyp)/(Q_*Q_*Q_) )*dxyz2_[1]
                     + 0.5*( -dq2_dyp*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dyp + V()*dq1_dyp + W()*dq2_dyp)/(Q_*Q_*Q_) )*dxyz2_[2];
                                                                                                                                            
             Coef[8] = 0.5*( -dq0_dzp*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dzp + V()*dq1_dzp + W()*dq2_dzp)/(Q_*Q_*Q_) )*dxyz2_[0]
                     + 0.5*( -dq1_dzp*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dzp + V()*dq1_dzp + W()*dq2_dzp)/(Q_*Q_*Q_) )*dxyz2_[1]          
                     + 0.5*( -dq2_dzp*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dzp + V()*dq1_dzp + W()*dq2_dzp)/(Q_*Q_*Q_) )*dxyz2_[2];  
                   
          }
          
          else {

             double VecR[3], VecP[3], Dot, CoefX, CoefY, CoefZ;
             double pVecR_Velocity[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pVelocity[3][3], pVecP_pXYZ[3][3][3];
             
             // Subtract out free stream...
             
             VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
             VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
             VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
             
             pVecR_Velocity[0] = 1.;
             pVecR_Velocity[1] = 1.;
             pVecR_Velocity[2] = 1.;
                       
             // Determine component in thrust direction
             
             Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
             
             // Limiter... we only take corrections in minus the thrust direction
             
             if ( Dot < 0. ) {
                
                pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
                pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
                pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
             
             }
             
             else {
                
                Dot = 0.;
             
                pDot_pVecR[0] = 0.;
                pDot_pVecR[1] = 0.;
                pDot_pVecR[2] = 0.;
                
             }
             
             // Add back in free stream component
             
             VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
             VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
             VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
             
             pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
             pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
             pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                    
             // Chain rule
             
             pVecP_pVelocity[0][0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Velocity[0];
             pVecP_pVelocity[0][1] = pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Velocity[1];
             pVecP_pVelocity[0][2] = pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Velocity[2];
             
             pVecP_pVelocity[1][0] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Velocity[0];
             pVecP_pVelocity[1][1] = pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Velocity[1];
             pVecP_pVelocity[1][2] = pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Velocity[2];
             
             pVecP_pVelocity[2][0] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Velocity[0];
             pVecP_pVelocity[2][1] = pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Velocity[1];
             pVecP_pVelocity[2][2] = pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Velocity[2];
              
             // Up
             
             pVecP_pXYZ[0][0][0] = pVecP_pVelocity[0][0] * dq0_dx1
                                 + pVecP_pVelocity[0][1] * dq1_dx1
                                 + pVecP_pVelocity[0][2] * dq2_dx1;

             pVecP_pXYZ[0][1][0] = pVecP_pVelocity[0][0] * dq0_dy1
                                 + pVecP_pVelocity[0][1] * dq1_dy1
                                 + pVecP_pVelocity[0][2] * dq2_dy1;

             pVecP_pXYZ[0][2][0] = pVecP_pVelocity[0][0] * dq0_dz1
                                 + pVecP_pVelocity[0][1] * dq1_dz1
                                 + pVecP_pVelocity[0][2] * dq2_dz1;
                                 
             pVecP_pXYZ[0][0][1] = pVecP_pVelocity[0][0] * dq0_dx2
                                 + pVecP_pVelocity[0][1] * dq1_dx2
                                 + pVecP_pVelocity[0][2] * dq2_dx2;
                                                                
             pVecP_pXYZ[0][1][1] = pVecP_pVelocity[0][0] * dq0_dy2
                                 + pVecP_pVelocity[0][1] * dq1_dy2
                                 + pVecP_pVelocity[0][2] * dq2_dy2;
                                                                 
             pVecP_pXYZ[0][2][1] = pVecP_pVelocity[0][0] * dq0_dz2
                                 + pVecP_pVelocity[0][1] * dq1_dz2
                                 + pVecP_pVelocity[0][2] * dq2_dz2;                                                                               

             pVecP_pXYZ[0][0][2] = pVecP_pVelocity[0][0] * dq0_dxp
                                 + pVecP_pVelocity[0][1] * dq1_dxp
                                 + pVecP_pVelocity[0][2] * dq2_dxp;
                                                               
             pVecP_pXYZ[0][1][2] = pVecP_pVelocity[0][0] * dq0_dyp
                                 + pVecP_pVelocity[0][1] * dq1_dyp
                                 + pVecP_pVelocity[0][2] * dq2_dyp;
                                                               
             pVecP_pXYZ[0][2][2] = pVecP_pVelocity[0][0] * dq0_dzp
                                 + pVecP_pVelocity[0][1] * dq1_dzp
                                 + pVecP_pVelocity[0][2] * dq2_dzp;      
                                 
             // Vp
             
             pVecP_pXYZ[1][0][0] = pVecP_pVelocity[1][0] * dq0_dx1
                                 + pVecP_pVelocity[1][1] * dq1_dx1
                                 + pVecP_pVelocity[1][2] * dq2_dx1;
                                                  
             pVecP_pXYZ[1][1][0] = pVecP_pVelocity[1][0] * dq0_dy1
                                 + pVecP_pVelocity[1][1] * dq1_dy1
                                 + pVecP_pVelocity[1][2] * dq2_dy1;
                                                   
             pVecP_pXYZ[1][2][0] = pVecP_pVelocity[1][0] * dq0_dz1
                                 + pVecP_pVelocity[1][1] * dq1_dz1
                                 + pVecP_pVelocity[1][2] * dq2_dz1;
                                                  
             pVecP_pXYZ[1][0][1] = pVecP_pVelocity[1][0] * dq0_dx2
                                 + pVecP_pVelocity[1][1] * dq1_dx2
                                 + pVecP_pVelocity[1][2] * dq2_dx2;
                                                             
             pVecP_pXYZ[1][1][1] = pVecP_pVelocity[1][0] * dq0_dy2
                                 + pVecP_pVelocity[1][1] * dq1_dy2
                                 + pVecP_pVelocity[1][2] * dq2_dy2;
                                                             
             pVecP_pXYZ[1][2][1] = pVecP_pVelocity[1][0] * dq0_dz2
                                 + pVecP_pVelocity[1][1] * dq1_dz2
                                 + pVecP_pVelocity[1][2] * dq2_dz2;       

             pVecP_pXYZ[1][0][2] = pVecP_pVelocity[1][0] * dq0_dxp
                                 + pVecP_pVelocity[1][1] * dq1_dxp
                                 + pVecP_pVelocity[1][2] * dq2_dxp;
                                                                 
             pVecP_pXYZ[1][1][2] = pVecP_pVelocity[1][0] * dq0_dyp
                                 + pVecP_pVelocity[1][1] * dq1_dyp
                                 + pVecP_pVelocity[1][2] * dq2_dyp;
                                                                
             pVecP_pXYZ[1][2][2] = pVecP_pVelocity[1][0] * dq0_dzp
                                 + pVecP_pVelocity[1][1] * dq1_dzp
                                 + pVecP_pVelocity[1][2] * dq2_dzp;      
                                                                  
             // Wp
             
             pVecP_pXYZ[2][0][0] = pVecP_pVelocity[2][0] * dq0_dx1
                                 + pVecP_pVelocity[2][1] * dq1_dx1
                                 + pVecP_pVelocity[2][2] * dq2_dx1;
                                                  
             pVecP_pXYZ[2][1][0] = pVecP_pVelocity[2][0] * dq0_dy1
                                 + pVecP_pVelocity[2][1] * dq1_dy1
                                 + pVecP_pVelocity[2][2] * dq2_dy1;
                                                   
             pVecP_pXYZ[2][2][0] = pVecP_pVelocity[2][0] * dq0_dz1
                                 + pVecP_pVelocity[2][1] * dq1_dz1
                                 + pVecP_pVelocity[2][2] * dq2_dz1;
                                                  
             pVecP_pXYZ[2][0][1] = pVecP_pVelocity[2][0] * dq0_dx2
                                 + pVecP_pVelocity[2][1] * dq1_dx2
                                 + pVecP_pVelocity[2][2] * dq2_dx2;
                                                         
             pVecP_pXYZ[2][1][1] = pVecP_pVelocity[2][0] * dq0_dy2
                                 + pVecP_pVelocity[2][1] * dq1_dy2
                                 + pVecP_pVelocity[2][2] * dq2_dy2;
                                                          
             pVecP_pXYZ[2][2][1] = pVecP_pVelocity[2][0] * dq0_dz2
                                 + pVecP_pVelocity[2][1] * dq1_dz2
                                 + pVecP_pVelocity[2][2] * dq2_dz2;                                    

             pVecP_pXYZ[2][0][2] = pVecP_pVelocity[2][0] * dq0_dxp
                                 + pVecP_pVelocity[2][1] * dq1_dxp
                                 + pVecP_pVelocity[2][2] * dq2_dxp;
                                                                
             pVecP_pXYZ[2][1][2] = pVecP_pVelocity[2][0] * dq0_dyp
                                 + pVecP_pVelocity[2][1] * dq1_dyp
                                 + pVecP_pVelocity[2][2] * dq2_dyp;
                                                                
             pVecP_pXYZ[2][2][2] = pVecP_pVelocity[2][0] * dq0_dzp
                                 + pVecP_pVelocity[2][1] * dq1_dzp
                                 + pVecP_pVelocity[2][2] * dq2_dzp;   

             // Faraway edge
             
             CoefX   =     ( -pVecP_pXYZ[0][0][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][0][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][0][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(2);                  

             CoefY   =     ( -pVecP_pXYZ[0][0][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][0][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][0][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(2);                  

             CoefZ   =     ( -pVecP_pXYZ[0][0][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][0][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][0][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(2);                  

             Coef[0] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);                  
         
             CoefX   =     ( -pVecP_pXYZ[0][1][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][1][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][1][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(2);

             CoefY   =     ( -pVecP_pXYZ[0][1][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][1][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][1][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(2);

             CoefZ   =     ( -pVecP_pXYZ[0][1][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][1][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][1][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(2);
                                                    
             Coef[1] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);                                                                        

             CoefX   =     ( -pVecP_pXYZ[0][2][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][2][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(1)          
                     +     ( -pVecP_pXYZ[2][2][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(2);  

             CoefY   =     ( -pVecP_pXYZ[0][2][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][2][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(1)          
                     +     ( -pVecP_pXYZ[2][2][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(2);  

             CoefZ   =     ( -pVecP_pXYZ[0][2][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][2][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(1)          
                     +     ( -pVecP_pXYZ[2][2][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(2);  
                                                                          
             Coef[2] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    
                     
                                             
             CoefX   =     ( -pVecP_pXYZ[0][0][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][0][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][0][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(2);
                                              
             CoefY   =     ( -pVecP_pXYZ[0][0][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][0][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][0][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(2);
            
             CoefZ   =     ( -pVecP_pXYZ[0][0][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][0][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][0][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(2);
                                                                                                                                     
             Coef[3] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    
                     
             CoefX   =     ( -pVecP_pXYZ[0][1][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][1][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][1][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(2); 
                     
             CoefY   =     ( -pVecP_pXYZ[0][1][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][1][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][1][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(2); 

             CoefZ   =     ( -pVecP_pXYZ[0][1][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][1][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][1][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(2); 
                                                     
             Coef[4] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    
                     
             CoefX   =     ( -pVecP_pXYZ[0][2][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][2][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(1)          
                     +     ( -pVecP_pXYZ[2][2][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(2); 

             CoefY   =     ( -pVecP_pXYZ[0][2][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][2][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(1)          
                     +     ( -pVecP_pXYZ[2][2][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(2); 

             CoefZ   =     ( -pVecP_pXYZ[0][2][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][2][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(1)          
                     +     ( -pVecP_pXYZ[2][2][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(2); 
                                        
             Coef[5] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    
                     
             // This edge

             CoefX   = 0.5*( -pVecP_pXYZ[0][0][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][0][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(1)
                     + 0.5*( -pVecP_pXYZ[2][0][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(2);                  

             CoefY   = 0.5*( -pVecP_pXYZ[0][0][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][0][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(1)
                     + 0.5*( -pVecP_pXYZ[2][0][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(2);                  

             CoefZ   = 0.5*( -pVecP_pXYZ[0][0][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][0][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(1)
                     + 0.5*( -pVecP_pXYZ[2][0][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(2);                  
             
             Coef[6] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    

             CoefX   = 0.5*( -pVecP_pXYZ[0][1][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][1][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(1)
                     + 0.5*( -pVecP_pXYZ[2][1][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(2);

             CoefY   = 0.5*( -pVecP_pXYZ[0][1][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][1][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(1)
                     + 0.5*( -pVecP_pXYZ[2][1][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(2);

             CoefZ   = 0.5*( -pVecP_pXYZ[0][1][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][1][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(1)
                     + 0.5*( -pVecP_pXYZ[2][1][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(2);

             Coef[7] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    

             CoefX   = 0.5*( -pVecP_pXYZ[0][2][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][2][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(1)          
                     + 0.5*( -pVecP_pXYZ[2][2][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz2_[0]*WakeEdgeData_->RotorThrustVector(2);  

             CoefY   = 0.5*( -pVecP_pXYZ[0][2][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][2][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(1)          
                     + 0.5*( -pVecP_pXYZ[2][2][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz2_[1]*WakeEdgeData_->RotorThrustVector(2);  

             CoefZ   = 0.5*( -pVecP_pXYZ[0][2][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][2][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(1)          
                     + 0.5*( -pVecP_pXYZ[2][2][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz2_[2]*WakeEdgeData_->RotorThrustVector(2);  
                                                                                                                                      
             Coef[8] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    
          }
          
       }
       
       else {

          if ( WakeEdgeData_->IsRotorWake() == 0 ) {

             // Faraway edge
   
             Coef[0] =     ( -dq0_dx1*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dx1 + V()*dq1_dx1 + W()*dq2_dx1)/(Q_*Q_*Q_) )*dxyz1_[0]
                     +     ( -dq1_dx1*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dx1 + V()*dq1_dx1 + W()*dq2_dx1)/(Q_*Q_*Q_) )*dxyz1_[1]
                     +     ( -dq2_dx1*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dx1 + V()*dq1_dx1 + W()*dq2_dx1)/(Q_*Q_*Q_) )*dxyz1_[2];                  
                                                                                                                                                
             Coef[1] =     ( -dq0_dy1*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dy1 + V()*dq1_dy1 + W()*dq2_dy1)/(Q_*Q_*Q_) )*dxyz1_[0]
                     +     ( -dq1_dy1*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dy1 + V()*dq1_dy1 + W()*dq2_dy1)/(Q_*Q_*Q_) )*dxyz1_[1]
                     +     ( -dq2_dy1*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dy1 + V()*dq1_dy1 + W()*dq2_dy1)/(Q_*Q_*Q_) )*dxyz1_[2];
                                                                                                                                                 
             Coef[2] =     ( -dq0_dz1*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dz1 + V()*dq1_dz1 + W()*dq2_dz1)/(Q_*Q_*Q_) )*dxyz1_[0]
                     +     ( -dq1_dz1*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dz1 + V()*dq1_dz1 + W()*dq2_dz1)/(Q_*Q_*Q_) )*dxyz1_[1]          
                     +     ( -dq2_dz1*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dz1 + V()*dq1_dz1 + W()*dq2_dz1)/(Q_*Q_*Q_) )*dxyz1_[2];  
                                                                                                                                                 
             Coef[3] =     ( -dq0_dx2*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dx2 + V()*dq1_dx2 + W()*dq2_dx2)/(Q_*Q_*Q_) )*dxyz1_[0]
                     +     ( -dq1_dx2*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dx2 + V()*dq1_dx2 + W()*dq2_dx2)/(Q_*Q_*Q_) )*dxyz1_[1]
                     +     ( -dq2_dx2*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dx2 + V()*dq1_dx2 + W()*dq2_dx2)/(Q_*Q_*Q_) )*dxyz1_[2];
                                                                                                                                                 
             Coef[4] =     ( -dq0_dy2*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dy2 + V()*dq1_dy2 + W()*dq2_dy2)/(Q_*Q_*Q_) )*dxyz1_[0]
                     +     ( -dq1_dy2*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dy2 + V()*dq1_dy2 + W()*dq2_dy2)/(Q_*Q_*Q_) )*dxyz1_[1]
                     +     ( -dq2_dy2*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dy2 + V()*dq1_dy2 + W()*dq2_dy2)/(Q_*Q_*Q_) )*dxyz1_[2]; 
                                                                                                                                               
             Coef[5] =     ( -dq0_dz2*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dz2 + V()*dq1_dz2 + W()*dq2_dz2)/(Q_*Q_*Q_) )*dxyz1_[0]
                     +     ( -dq1_dz2*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dz2 + V()*dq1_dz2 + W()*dq2_dz2)/(Q_*Q_*Q_) )*dxyz1_[1]          
                     +     ( -dq2_dz2*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dz2 + V()*dq1_dz2 + W()*dq2_dz2)/(Q_*Q_*Q_) )*dxyz1_[2]; 
   
             // This edge
   
             Coef[6] = 0.5*( -dq0_dxp*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dxp + V()*dq1_dxp + W()*dq2_dxp)/(Q_*Q_*Q_) )*dxyz1_[0]
                     + 0.5*( -dq1_dxp*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dxp + V()*dq1_dxp + W()*dq2_dxp)/(Q_*Q_*Q_) )*dxyz1_[1]
                     + 0.5*( -dq2_dxp*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dxp + V()*dq1_dxp + W()*dq2_dxp)/(Q_*Q_*Q_) )*dxyz1_[2];                  
                                                                                                                                                
             Coef[7] = 0.5*( -dq0_dyp*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dyp + V()*dq1_dyp + W()*dq2_dyp)/(Q_*Q_*Q_) )*dxyz1_[0]
                     + 0.5*( -dq1_dyp*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dyp + V()*dq1_dyp + W()*dq2_dyp)/(Q_*Q_*Q_) )*dxyz1_[1]
                     + 0.5*( -dq2_dyp*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dyp + V()*dq1_dyp + W()*dq2_dyp)/(Q_*Q_*Q_) )*dxyz1_[2];
                                                                                                                                           
             Coef[8] = 0.5*( -dq0_dzp*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dzp + V()*dq1_dzp + W()*dq2_dzp)/(Q_*Q_*Q_) )*dxyz1_[0]
                     + 0.5*( -dq1_dzp*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dzp + V()*dq1_dzp + W()*dq2_dzp)/(Q_*Q_*Q_) )*dxyz1_[1]          
                     + 0.5*( -dq2_dzp*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dzp + V()*dq1_dzp + W()*dq2_dzp)/(Q_*Q_*Q_) )*dxyz1_[2];                    
   
          }

         else {

             double VecR[3], VecP[3], Dot, CoefX, CoefY, CoefZ;
             double pVecR_Velocity[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pVelocity[3][3], pVecP_pXYZ[3][3][3];
             
             // Subtract out free stream...
             
             VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
             VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
             VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
             
             pVecR_Velocity[0] = 1.;
             pVecR_Velocity[1] = 1.;
             pVecR_Velocity[2] = 1.;
                       
             // Determine component in thrust direction
             
             Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
             
             // Limiter... we only take corrections in minus the thrust direction
             
             if ( Dot < 0. ) {
                
                pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
                pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
                pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
             
             }
             
             else {
                
                Dot = 0.;
             
                pDot_pVecR[0] = 0.;
                pDot_pVecR[1] = 0.;
                pDot_pVecR[2] = 0.;
                
             }
             
             // Add back in free stream component
             
             VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
             VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
             VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
             
             pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
             pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
             pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                    
             // Chain rule
             
             pVecP_pVelocity[0][0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Velocity[0];
             pVecP_pVelocity[0][1] = pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Velocity[1];
             pVecP_pVelocity[0][2] = pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Velocity[2];
             
             pVecP_pVelocity[1][0] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Velocity[0];
             pVecP_pVelocity[1][1] = pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Velocity[1];
             pVecP_pVelocity[1][2] = pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Velocity[2];
             
             pVecP_pVelocity[2][0] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Velocity[0];
             pVecP_pVelocity[2][1] = pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Velocity[1];
             pVecP_pVelocity[2][2] = pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Velocity[2];
              
             // Up
             
             pVecP_pXYZ[0][0][0] = pVecP_pVelocity[0][0] * dq0_dx1
                                 + pVecP_pVelocity[0][1] * dq1_dx1
                                 + pVecP_pVelocity[0][2] * dq2_dx1;

             pVecP_pXYZ[0][1][0] = pVecP_pVelocity[0][0] * dq0_dy1
                                 + pVecP_pVelocity[0][1] * dq1_dy1
                                 + pVecP_pVelocity[0][2] * dq2_dy1;

             pVecP_pXYZ[0][2][0] = pVecP_pVelocity[0][0] * dq0_dz1
                                 + pVecP_pVelocity[0][1] * dq1_dz1
                                 + pVecP_pVelocity[0][2] * dq2_dz1;
                                 
             pVecP_pXYZ[0][0][1] = pVecP_pVelocity[0][0] * dq0_dx2
                                 + pVecP_pVelocity[0][1] * dq1_dx2
                                 + pVecP_pVelocity[0][2] * dq2_dx2;
                                                                
             pVecP_pXYZ[0][1][1] = pVecP_pVelocity[0][0] * dq0_dy2
                                 + pVecP_pVelocity[0][1] * dq1_dy2
                                 + pVecP_pVelocity[0][2] * dq2_dy2;
                                                                 
             pVecP_pXYZ[0][2][1] = pVecP_pVelocity[0][0] * dq0_dz2
                                 + pVecP_pVelocity[0][1] * dq1_dz2
                                 + pVecP_pVelocity[0][2] * dq2_dz2;                                                                               

             pVecP_pXYZ[0][0][2] = pVecP_pVelocity[0][0] * dq0_dxp
                                 + pVecP_pVelocity[0][1] * dq1_dxp
                                 + pVecP_pVelocity[0][2] * dq2_dxp;
                                                               
             pVecP_pXYZ[0][1][2] = pVecP_pVelocity[0][0] * dq0_dyp
                                 + pVecP_pVelocity[0][1] * dq1_dyp
                                 + pVecP_pVelocity[0][2] * dq2_dyp;
                                                               
             pVecP_pXYZ[0][2][2] = pVecP_pVelocity[0][0] * dq0_dzp
                                 + pVecP_pVelocity[0][1] * dq1_dzp
                                 + pVecP_pVelocity[0][2] * dq2_dzp;      
                                 
             // Up
             
             pVecP_pXYZ[1][0][0] = pVecP_pVelocity[1][0] * dq0_dx1
                                 + pVecP_pVelocity[1][1] * dq1_dx1
                                 + pVecP_pVelocity[1][2] * dq2_dx1;
                                                  
             pVecP_pXYZ[1][1][0] = pVecP_pVelocity[1][0] * dq0_dy1
                                 + pVecP_pVelocity[1][1] * dq1_dy1
                                 + pVecP_pVelocity[1][2] * dq2_dy1;
                                                   
             pVecP_pXYZ[1][2][0] = pVecP_pVelocity[1][0] * dq0_dz1
                                 + pVecP_pVelocity[1][1] * dq1_dz1
                                 + pVecP_pVelocity[1][2] * dq2_dz1;
                                                  
             pVecP_pXYZ[1][0][1] = pVecP_pVelocity[1][0] * dq0_dx2
                                 + pVecP_pVelocity[1][1] * dq1_dx2
                                 + pVecP_pVelocity[1][2] * dq2_dx2;
                                                             
             pVecP_pXYZ[1][1][1] = pVecP_pVelocity[1][0] * dq0_dy2
                                 + pVecP_pVelocity[1][1] * dq1_dy2
                                 + pVecP_pVelocity[1][2] * dq2_dy2;
                                                             
             pVecP_pXYZ[1][2][1] = pVecP_pVelocity[1][0] * dq0_dz2
                                 + pVecP_pVelocity[1][1] * dq1_dz2
                                 + pVecP_pVelocity[1][2] * dq2_dz2;       

             pVecP_pXYZ[1][0][2] = pVecP_pVelocity[1][0] * dq0_dxp
                                 + pVecP_pVelocity[1][1] * dq1_dxp
                                 + pVecP_pVelocity[1][2] * dq2_dxp;
                                                                 
             pVecP_pXYZ[1][1][2] = pVecP_pVelocity[1][0] * dq0_dyp
                                 + pVecP_pVelocity[1][1] * dq1_dyp
                                 + pVecP_pVelocity[1][2] * dq2_dyp;
                                                                
             pVecP_pXYZ[1][2][2] = pVecP_pVelocity[1][0] * dq0_dzp
                                 + pVecP_pVelocity[1][1] * dq1_dzp
                                 + pVecP_pVelocity[1][2] * dq2_dzp;      
                                                                  
             // Wp
             
             pVecP_pXYZ[2][0][0] = pVecP_pVelocity[2][0] * dq0_dx1
                                 + pVecP_pVelocity[2][1] * dq1_dx1
                                 + pVecP_pVelocity[2][2] * dq2_dx1;
                                                  
             pVecP_pXYZ[2][1][0] = pVecP_pVelocity[2][0] * dq0_dy1
                                 + pVecP_pVelocity[2][1] * dq1_dy1
                                 + pVecP_pVelocity[2][2] * dq2_dy1;
                                                   
             pVecP_pXYZ[2][2][0] = pVecP_pVelocity[2][0] * dq0_dz1
                                 + pVecP_pVelocity[2][1] * dq1_dz1
                                 + pVecP_pVelocity[2][2] * dq2_dz1;
                                                  
             pVecP_pXYZ[2][0][1] = pVecP_pVelocity[2][0] * dq0_dx2
                                 + pVecP_pVelocity[2][1] * dq1_dx2
                                 + pVecP_pVelocity[2][2] * dq2_dx2;
                                                         
             pVecP_pXYZ[2][1][1] = pVecP_pVelocity[2][0] * dq0_dy2
                                 + pVecP_pVelocity[2][1] * dq1_dy2
                                 + pVecP_pVelocity[2][2] * dq2_dy2;
                                                          
             pVecP_pXYZ[2][2][1] = pVecP_pVelocity[2][0] * dq0_dz2
                                 + pVecP_pVelocity[2][1] * dq1_dz2
                                 + pVecP_pVelocity[2][2] * dq2_dz2;                                    

             pVecP_pXYZ[2][0][2] = pVecP_pVelocity[2][0] * dq0_dxp
                                 + pVecP_pVelocity[2][1] * dq1_dxp
                                 + pVecP_pVelocity[2][2] * dq2_dxp;
                                                                
             pVecP_pXYZ[2][1][2] = pVecP_pVelocity[2][0] * dq0_dyp
                                 + pVecP_pVelocity[2][1] * dq1_dyp
                                 + pVecP_pVelocity[2][2] * dq2_dyp;
                                                                
             pVecP_pXYZ[2][2][2] = pVecP_pVelocity[2][0] * dq0_dzp
                                 + pVecP_pVelocity[2][1] * dq1_dzp
                                 + pVecP_pVelocity[2][2] * dq2_dzp;   

             // Faraway edge
             
             CoefX   =     ( -pVecP_pXYZ[0][0][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][0][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][0][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(2);                  
                                                                                                                                                                                                                  
             CoefY   =     ( -pVecP_pXYZ[0][0][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][0][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][0][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(2);                  
                                                                                                                                                                                                                 
             CoefZ   =     ( -pVecP_pXYZ[0][0][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][0][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][0][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(2);                  

             Coef[0] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);                  
         
             CoefX   =     ( -pVecP_pXYZ[0][1][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][1][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][1][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(2);
                                                                                                                                                                                                                
             CoefY   =     ( -pVecP_pXYZ[0][1][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][1][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][1][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(2);
                                                                                                                                                                                                                  
             CoefZ   =     ( -pVecP_pXYZ[0][1][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][1][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][1][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(2);
                                                    
             Coef[1] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);                                                                        

             CoefX   =     ( -pVecP_pXYZ[0][2][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][2][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(1)          
                     +     ( -pVecP_pXYZ[2][2][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(2);  
                                                                                                                                                                                                                 
             CoefY   =     ( -pVecP_pXYZ[0][2][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][2][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(1)          
                     +     ( -pVecP_pXYZ[2][2][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(2);  
                                                                                                                                                                                                                  
             CoefZ   =     ( -pVecP_pXYZ[0][2][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][2][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(1)          
                     +     ( -pVecP_pXYZ[2][2][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(2);  
                                                                          
             Coef[2] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    
                     
                                             
             CoefX   =     ( -pVecP_pXYZ[0][0][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][0][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][0][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(2);
                                                                                                                                                                                                                  
             CoefY   =     ( -pVecP_pXYZ[0][0][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][0][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][0][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(2);
                                                                                                                                                                                                                  
             CoefZ   =     ( -pVecP_pXYZ[0][0][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][0][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][0][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(2);
                                                                                                                                     
             Coef[3] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    
                     
             CoefX   =     ( -pVecP_pXYZ[0][1][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][1][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][1][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(2); 
                                                                                                                                                                                                                  
             CoefY   =     ( -pVecP_pXYZ[0][1][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][1][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][1][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(2); 
                                                                                                                                                                                                                  
             CoefZ   =     ( -pVecP_pXYZ[0][1][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][1][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(1)
                     +     ( -pVecP_pXYZ[2][1][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(2); 
                                                     
             Coef[4] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    
                     
             CoefX   =     ( -pVecP_pXYZ[0][2][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][2][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(1)          
                     +     ( -pVecP_pXYZ[2][2][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(2); 
                                                                                                                                                                                                                  
             CoefY   =     ( -pVecP_pXYZ[0][2][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][2][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(1)          
                     +     ( -pVecP_pXYZ[2][2][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(2); 
                                                                                                                                                                                                                  
             CoefZ   =     ( -pVecP_pXYZ[0][2][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(0)
                     +     ( -pVecP_pXYZ[1][2][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(1)          
                     +     ( -pVecP_pXYZ[2][2][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(2); 
                                        
             Coef[5] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    
                     
             // This edge

             CoefX   = 0.5*( -pVecP_pXYZ[0][0][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][0][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(1)
                     + 0.5*( -pVecP_pXYZ[2][0][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(2);                  
                                                                                                                                                                                                                 
             CoefY   = 0.5*( -pVecP_pXYZ[0][0][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][0][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(1)
                     + 0.5*( -pVecP_pXYZ[2][0][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(2);                  
                                                                                                                                                                                                                  
             CoefZ   = 0.5*( -pVecP_pXYZ[0][0][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][0][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(1)
                     + 0.5*( -pVecP_pXYZ[2][0][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(2);                  
             
             Coef[6] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    

             CoefX   = 0.5*( -pVecP_pXYZ[0][1][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][1][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(1)
                     + 0.5*( -pVecP_pXYZ[2][1][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(2);
                                                                                                                                                                                                                
             CoefY   = 0.5*( -pVecP_pXYZ[0][1][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][1][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(1)
                     + 0.5*( -pVecP_pXYZ[2][1][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(2);
                                                                                                                                                                                                                  
             CoefZ   = 0.5*( -pVecP_pXYZ[0][1][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][1][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(1)
                     + 0.5*( -pVecP_pXYZ[2][1][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(2);

             Coef[7] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);    

             CoefX   = 0.5*( -pVecP_pXYZ[0][2][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][2][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(1)          
                     + 0.5*( -pVecP_pXYZ[2][2][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz1_[0]*WakeEdgeData_->RotorThrustVector(2);  
                                                                                                                                                                                                                 
             CoefY   = 0.5*( -pVecP_pXYZ[0][2][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][2][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(1)          
                     + 0.5*( -pVecP_pXYZ[2][2][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz1_[1]*WakeEdgeData_->RotorThrustVector(2);  
                                                                                                                                                                                                                  
             CoefZ   = 0.5*( -pVecP_pXYZ[0][2][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(0)
                     + 0.5*( -pVecP_pXYZ[1][2][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(1)          
                     + 0.5*( -pVecP_pXYZ[2][2][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*dxyz1_[2]*WakeEdgeData_->RotorThrustVector(2);  
                                                                                                                                      
             Coef[8] = CoefX * WakeEdgeData_->RotorThrustVector(0)
                     + CoefY * WakeEdgeData_->RotorThrustVector(1)
                     + CoefZ * WakeEdgeData_->RotorThrustVector(2);  
          }
                    
       }               

    }     
    
    else {
       
       Coef[0] = 0.;
       Coef[1] = 0.;
       Coef[2] = 0.;
       Coef[3] = 0.;
       Coef[4] = 0.;
       Coef[5] = 0.;
       Coef[6] = 0.;
       Coef[7] = 0.;
       Coef[8] = 0.;
              
    }       
                  
}

/*##############################################################################
#                                                                              #
#                         VSP_EDGE PsiT_pR_pMesh                               #
#                                                                              #
##############################################################################*/

void VSP_EDGE::PsiT_pR_pMesh(double &dq0_dx1,
                             double &dq0_dy1,
                             double &dq0_dz1,
                             double &dq0_dx2,
                             double &dq0_dy2,
                             double &dq0_dz2,
                                             
                             double &dq1_dx1,
                             double &dq1_dy1,
                             double &dq1_dz1,
                             double &dq1_dx2,
                             double &dq1_dy2,
                             double &dq1_dz2,
                                                 
                             double &dq2_dx1,
                             double &dq2_dy1,
                             double &dq2_dz1,
                             double &dq2_dx2,
                             double &dq2_dy2,
                             double &dq2_dz2,
                                         
                             double &dq0_dxp,
                             double &dq0_dyp,
                             double &dq0_dzp,
                                         
                             double &dq1_dxp,
                             double &dq1_dyp,
                             double &dq1_dzp,
                                         
                             double &dq2_dxp,
                             double &dq2_dyp,
                             double &dq2_dzp,    
                             
                             double Psi[3],
                                                                                             
                             double Coef[9])
{

    if ( IsWakeEdge_ && WakeEdgeData_->dS() > 0. && !IsFrozenTrailingEdge_ && !IsConcaveTrailingEdge_ ) {

       if ( WakeEdgeData_->IsRotorWake() == 0 ) {
          
          // Faraway edge
          
          Coef[0] =     ( -dq0_dx1*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dx1 + V()*dq1_dx1 + W()*dq2_dx1)/(Q_*Q_*Q_) )*Psi[0]
                  +     ( -dq1_dx1*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dx1 + V()*dq1_dx1 + W()*dq2_dx1)/(Q_*Q_*Q_) )*Psi[1]
                  +     ( -dq2_dx1*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dx1 + V()*dq1_dx1 + W()*dq2_dx1)/(Q_*Q_*Q_) )*Psi[2];                  
                                                                                                                               
          Coef[1] =     ( -dq0_dy1*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dy1 + V()*dq1_dy1 + W()*dq2_dy1)/(Q_*Q_*Q_) )*Psi[0]
                  +     ( -dq1_dy1*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dy1 + V()*dq1_dy1 + W()*dq2_dy1)/(Q_*Q_*Q_) )*Psi[1]
                  +     ( -dq2_dy1*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dy1 + V()*dq1_dy1 + W()*dq2_dy1)/(Q_*Q_*Q_) )*Psi[2];
                                                                                                                                           
          Coef[2] =     ( -dq0_dz1*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dz1 + V()*dq1_dz1 + W()*dq2_dz1)/(Q_*Q_*Q_) )*Psi[0]
                  +     ( -dq1_dz1*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dz1 + V()*dq1_dz1 + W()*dq2_dz1)/(Q_*Q_*Q_) )*Psi[1]          
                  +     ( -dq2_dz1*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dz1 + V()*dq1_dz1 + W()*dq2_dz1)/(Q_*Q_*Q_) )*Psi[2];  
                  
          Coef[3] =     ( -dq0_dx2*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dx2 + V()*dq1_dx2 + W()*dq2_dx2)/(Q_*Q_*Q_) )*Psi[0]
                  +     ( -dq1_dx2*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dx2 + V()*dq1_dx2 + W()*dq2_dx2)/(Q_*Q_*Q_) )*Psi[1]
                  +     ( -dq2_dx2*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dx2 + V()*dq1_dx2 + W()*dq2_dx2)/(Q_*Q_*Q_) )*Psi[2];
                                      
          Coef[4] =     ( -dq0_dy2*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dy2 + V()*dq1_dy2 + W()*dq2_dy2)/(Q_*Q_*Q_) )*Psi[0]
                  +     ( -dq1_dy2*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dy2 + V()*dq1_dy2 + W()*dq2_dy2)/(Q_*Q_*Q_) )*Psi[1]
                  +     ( -dq2_dy2*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dy2 + V()*dq1_dy2 + W()*dq2_dy2)/(Q_*Q_*Q_) )*Psi[2]; 
                                                    
          Coef[5] =     ( -dq0_dz2*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dz2 + V()*dq1_dz2 + W()*dq2_dz2)/(Q_*Q_*Q_) )*Psi[0]
                  +     ( -dq1_dz2*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dz2 + V()*dq1_dz2 + W()*dq2_dz2)/(Q_*Q_*Q_) )*Psi[1]          
                  +     ( -dq2_dz2*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dz2 + V()*dq1_dz2 + W()*dq2_dz2)/(Q_*Q_*Q_) )*Psi[2]; 

          // This edge
          
          Coef[6] = 0.5*( -dq0_dxp*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dxp + V()*dq1_dxp + W()*dq2_dxp)/(Q_*Q_*Q_) )*Psi[0]
                  + 0.5*( -dq1_dxp*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dxp + V()*dq1_dxp + W()*dq2_dxp)/(Q_*Q_*Q_) )*Psi[1]
                  + 0.5*( -dq2_dxp*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dxp + V()*dq1_dxp + W()*dq2_dxp)/(Q_*Q_*Q_) )*Psi[2];                  
                                                                                                                               
          Coef[7] = 0.5*( -dq0_dyp*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dyp + V()*dq1_dyp + W()*dq2_dyp)/(Q_*Q_*Q_) )*Psi[0]
                  + 0.5*( -dq1_dyp*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dyp + V()*dq1_dyp + W()*dq2_dyp)/(Q_*Q_*Q_) )*Psi[1]
                  + 0.5*( -dq2_dyp*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dyp + V()*dq1_dyp + W()*dq2_dyp)/(Q_*Q_*Q_) )*Psi[2];
                                                                                                                                         
          Coef[8] = 0.5*( -dq0_dzp*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dq0_dzp + V()*dq1_dzp + W()*dq2_dzp)/(Q_*Q_*Q_) )*Psi[0]
                  + 0.5*( -dq1_dzp*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dq0_dzp + V()*dq1_dzp + W()*dq2_dzp)/(Q_*Q_*Q_) )*Psi[1]          
                  + 0.5*( -dq2_dzp*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dq0_dzp + V()*dq1_dzp + W()*dq2_dzp)/(Q_*Q_*Q_) )*Psi[2];  
                  
       }
       
       else {
          
          double VecR[3], VecP[3], Dot, CoefX, CoefY, CoefZ;
          double pVecR_Velocity[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pVelocity[3][3], pVecP_pXYZ[3][3][3];
          
          // Subtract out free stream...
          
          VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
          VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
          VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
          
          pVecR_Velocity[0] = 1.;
          pVecR_Velocity[1] = 1.;
          pVecR_Velocity[2] = 1.;
                    
          // Determine component in thrust direction
          
          Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
          
          // Limiter... we only take corrections in minus the thrust direction
          
          if ( Dot < 0. ) {
             
             pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
             pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
             pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
          
          }
          
          else {
             
             Dot = 0.;
          
             pDot_pVecR[0] = 0.;
             pDot_pVecR[1] = 0.;
             pDot_pVecR[2] = 0.;
             
          }
          
          // Add back in free stream component
          
          VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
          VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
          VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
          
          pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
          pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
          pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                 
          // Chain rule
          
          pVecP_pVelocity[0][0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[0][1] = pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[0][2] = pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pVecP_pVelocity[1][0] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[1][1] = pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[1][2] = pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pVecP_pVelocity[2][0] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[2][1] = pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[2][2] = pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Velocity[2];
           
          // Up
          
          pVecP_pXYZ[0][0][0] = pVecP_pVelocity[0][0] * dq0_dx1
                              + pVecP_pVelocity[0][1] * dq1_dx1
                              + pVecP_pVelocity[0][2] * dq2_dx1;

          pVecP_pXYZ[0][1][0] = pVecP_pVelocity[0][0] * dq0_dy1
                              + pVecP_pVelocity[0][1] * dq1_dy1
                              + pVecP_pVelocity[0][2] * dq2_dy1;

          pVecP_pXYZ[0][2][0] = pVecP_pVelocity[0][0] * dq0_dz1
                              + pVecP_pVelocity[0][1] * dq1_dz1
                              + pVecP_pVelocity[0][2] * dq2_dz1;
                              
          pVecP_pXYZ[0][0][1] = pVecP_pVelocity[0][0] * dq0_dx2
                              + pVecP_pVelocity[0][1] * dq1_dx2
                              + pVecP_pVelocity[0][2] * dq2_dx2;
                                                             
          pVecP_pXYZ[0][1][1] = pVecP_pVelocity[0][0] * dq0_dy2
                              + pVecP_pVelocity[0][1] * dq1_dy2
                              + pVecP_pVelocity[0][2] * dq2_dy2;
                                                              
          pVecP_pXYZ[0][2][1] = pVecP_pVelocity[0][0] * dq0_dz2
                              + pVecP_pVelocity[0][1] * dq1_dz2
                              + pVecP_pVelocity[0][2] * dq2_dz2;                                                                               

          pVecP_pXYZ[0][0][2] = pVecP_pVelocity[0][0] * dq0_dxp
                              + pVecP_pVelocity[0][1] * dq1_dxp
                              + pVecP_pVelocity[0][2] * dq2_dxp;
                                                            
          pVecP_pXYZ[0][1][2] = pVecP_pVelocity[0][0] * dq0_dyp
                              + pVecP_pVelocity[0][1] * dq1_dyp
                              + pVecP_pVelocity[0][2] * dq2_dyp;
                                                            
          pVecP_pXYZ[0][2][2] = pVecP_pVelocity[0][0] * dq0_dzp
                              + pVecP_pVelocity[0][1] * dq1_dzp
                              + pVecP_pVelocity[0][2] * dq2_dzp;      
                              
          // Vp
          
          pVecP_pXYZ[1][0][0] = pVecP_pVelocity[1][0] * dq0_dx1
                              + pVecP_pVelocity[1][1] * dq1_dx1
                              + pVecP_pVelocity[1][2] * dq2_dx1;
                                               
          pVecP_pXYZ[1][1][0] = pVecP_pVelocity[1][0] * dq0_dy1
                              + pVecP_pVelocity[1][1] * dq1_dy1
                              + pVecP_pVelocity[1][2] * dq2_dy1;
                                                
          pVecP_pXYZ[1][2][0] = pVecP_pVelocity[1][0] * dq0_dz1
                              + pVecP_pVelocity[1][1] * dq1_dz1
                              + pVecP_pVelocity[1][2] * dq2_dz1;
                                               
          pVecP_pXYZ[1][0][1] = pVecP_pVelocity[1][0] * dq0_dx2
                              + pVecP_pVelocity[1][1] * dq1_dx2
                              + pVecP_pVelocity[1][2] * dq2_dx2;
                                                          
          pVecP_pXYZ[1][1][1] = pVecP_pVelocity[1][0] * dq0_dy2
                              + pVecP_pVelocity[1][1] * dq1_dy2
                              + pVecP_pVelocity[1][2] * dq2_dy2;
                                                          
          pVecP_pXYZ[1][2][1] = pVecP_pVelocity[1][0] * dq0_dz2
                              + pVecP_pVelocity[1][1] * dq1_dz2
                              + pVecP_pVelocity[1][2] * dq2_dz2;       

          pVecP_pXYZ[1][0][2] = pVecP_pVelocity[1][0] * dq0_dxp
                              + pVecP_pVelocity[1][1] * dq1_dxp
                              + pVecP_pVelocity[1][2] * dq2_dxp;
                                                              
          pVecP_pXYZ[1][1][2] = pVecP_pVelocity[1][0] * dq0_dyp
                              + pVecP_pVelocity[1][1] * dq1_dyp
                              + pVecP_pVelocity[1][2] * dq2_dyp;
                                                             
          pVecP_pXYZ[1][2][2] = pVecP_pVelocity[1][0] * dq0_dzp
                              + pVecP_pVelocity[1][1] * dq1_dzp
                              + pVecP_pVelocity[1][2] * dq2_dzp;      
                                                               
          // Wp
          
          pVecP_pXYZ[2][0][0] = pVecP_pVelocity[2][0] * dq0_dx1
                              + pVecP_pVelocity[2][1] * dq1_dx1
                              + pVecP_pVelocity[2][2] * dq2_dx1;
                                               
          pVecP_pXYZ[2][1][0] = pVecP_pVelocity[2][0] * dq0_dy1
                              + pVecP_pVelocity[2][1] * dq1_dy1
                              + pVecP_pVelocity[2][2] * dq2_dy1;
                                                
          pVecP_pXYZ[2][2][0] = pVecP_pVelocity[2][0] * dq0_dz1
                              + pVecP_pVelocity[2][1] * dq1_dz1
                              + pVecP_pVelocity[2][2] * dq2_dz1;
                                               
          pVecP_pXYZ[2][0][1] = pVecP_pVelocity[2][0] * dq0_dx2
                              + pVecP_pVelocity[2][1] * dq1_dx2
                              + pVecP_pVelocity[2][2] * dq2_dx2;
                                                      
          pVecP_pXYZ[2][1][1] = pVecP_pVelocity[2][0] * dq0_dy2
                              + pVecP_pVelocity[2][1] * dq1_dy2
                              + pVecP_pVelocity[2][2] * dq2_dy2;
                                                       
          pVecP_pXYZ[2][2][1] = pVecP_pVelocity[2][0] * dq0_dz2
                              + pVecP_pVelocity[2][1] * dq1_dz2
                              + pVecP_pVelocity[2][2] * dq2_dz2;                                    

          pVecP_pXYZ[2][0][2] = pVecP_pVelocity[2][0] * dq0_dxp
                              + pVecP_pVelocity[2][1] * dq1_dxp
                              + pVecP_pVelocity[2][2] * dq2_dxp;
                                                             
          pVecP_pXYZ[2][1][2] = pVecP_pVelocity[2][0] * dq0_dyp
                              + pVecP_pVelocity[2][1] * dq1_dyp
                              + pVecP_pVelocity[2][2] * dq2_dyp;
                                                             
          pVecP_pXYZ[2][2][2] = pVecP_pVelocity[2][0] * dq0_dzp
                              + pVecP_pVelocity[2][1] * dq1_dzp
                              + pVecP_pVelocity[2][2] * dq2_dzp;   

          // Faraway edge
          
          CoefX   =     ( -pVecP_pXYZ[0][0][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(0)
                  +     ( -pVecP_pXYZ[1][0][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(1)
                  +     ( -pVecP_pXYZ[2][0][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][0] + VecP[1]*pVecP_pXYZ[1][0][0] + VecP[2]*pVecP_pXYZ[2][0][0])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(2);                  

          CoefY   =     ( -pVecP_pXYZ[0][1][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(0)
                  +     ( -pVecP_pXYZ[1][1][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(1)
                  +     ( -pVecP_pXYZ[2][1][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][0] + VecP[1]*pVecP_pXYZ[1][1][0] + VecP[2]*pVecP_pXYZ[2][1][0])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(2);                  

          CoefZ   =     ( -pVecP_pXYZ[0][2][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(0)
                  +     ( -pVecP_pXYZ[1][2][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(1)
                  +     ( -pVecP_pXYZ[2][2][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][0] + VecP[1]*pVecP_pXYZ[1][2][0] + VecP[2]*pVecP_pXYZ[2][2][0])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(2);                  

          Coef[0] = CoefX * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefX * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefX * WakeEdgeData_->RotorThrustVector(2) * Psi[2];                 
      
                                                 
          Coef[1] = CoefY * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefY * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefY * WakeEdgeData_->RotorThrustVector(2) * Psi[2];    
                                                                       
          Coef[2] = CoefZ * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefZ * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefZ * WakeEdgeData_->RotorThrustVector(2) * Psi[2];    
 
          CoefX   =     ( -pVecP_pXYZ[0][0][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(0)
                  +     ( -pVecP_pXYZ[1][0][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(1)
                  +     ( -pVecP_pXYZ[2][0][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][1] + VecP[1]*pVecP_pXYZ[1][0][1] + VecP[2]*pVecP_pXYZ[2][0][1])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(2);                  
                                                                                                                                                                                         
          CoefY   =     ( -pVecP_pXYZ[0][1][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(0)
                  +     ( -pVecP_pXYZ[1][1][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(1)
                  +     ( -pVecP_pXYZ[2][1][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][1] + VecP[1]*pVecP_pXYZ[1][1][1] + VecP[2]*pVecP_pXYZ[2][1][1])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(2);                  
                                                                                                                                                                                         
          CoefZ   =     ( -pVecP_pXYZ[0][2][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(0)
                  +     ( -pVecP_pXYZ[1][2][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(1)
                  +     ( -pVecP_pXYZ[2][2][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][1] + VecP[1]*pVecP_pXYZ[1][2][1] + VecP[2]*pVecP_pXYZ[2][2][1])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(2);                  

          Coef[3] = CoefX * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefX * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefX * WakeEdgeData_->RotorThrustVector(2) * Psi[2];                 
      
                                                 
          Coef[4] = CoefY * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefY * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefY * WakeEdgeData_->RotorThrustVector(2) * Psi[2];    
                                                                       
          Coef[5] = CoefZ * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefZ * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefZ * WakeEdgeData_->RotorThrustVector(2) * Psi[2];    
 
          // This edge

          CoefX   = 0.5*( -pVecP_pXYZ[0][0][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(0)
                  + 0.5*( -pVecP_pXYZ[1][0][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(1)
                  + 0.5*( -pVecP_pXYZ[2][0][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][0][2] + VecP[1]*pVecP_pXYZ[1][0][2] + VecP[2]*pVecP_pXYZ[2][0][2])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(2);                  
                                                                                                                                                                                
          CoefY   = 0.5*( -pVecP_pXYZ[0][1][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(0)
                  + 0.5*( -pVecP_pXYZ[1][1][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(1)
                  + 0.5*( -pVecP_pXYZ[2][1][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][1][2] + VecP[1]*pVecP_pXYZ[1][1][2] + VecP[2]*pVecP_pXYZ[2][1][2])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(2);                  
                                                                                                                                                                              
          CoefZ   = 0.5*( -pVecP_pXYZ[0][2][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(0)
                  + 0.5*( -pVecP_pXYZ[1][2][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(1)
                  + 0.5*( -pVecP_pXYZ[2][2][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*WakeEdgeData_->dS()*(VecP[0]*pVecP_pXYZ[0][2][2] + VecP[1]*pVecP_pXYZ[1][2][2] + VecP[2]*pVecP_pXYZ[2][2][2])/(Q_*Q_*Q_) )*WakeEdgeData_->RotorThrustVector(2);                  

          Coef[6] = CoefX * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefX * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefX * WakeEdgeData_->RotorThrustVector(2) * Psi[2];                 
      
                                                 
          Coef[7] = CoefY * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefY * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefY * WakeEdgeData_->RotorThrustVector(2) * Psi[2];    
                                                                       
          Coef[8] = CoefZ * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefZ * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefZ * WakeEdgeData_->RotorThrustVector(2) * Psi[2];    

       }
                
    }     
    
    else {
       
       Coef[0] = 0.;
       Coef[1] = 0.;
       Coef[2] = 0.;
       Coef[3] = 0.;
       Coef[4] = 0.;
       Coef[5] = 0.;
       Coef[6] = 0.;
       Coef[7] = 0.;
       Coef[8] = 0.;
              
    }       

}

/*##############################################################################
#                                                                              #
#                         VSP_EDGE PsiT_pR_pMesh                               #
#                                                                              #
##############################################################################*/

void VSP_EDGE::UnsteadyPsiT_pR_pMesh(double &dq0_dx1,
                                     double &dq0_dy1,
                                     double &dq0_dz1,
                                     double &dq0_dx2,
                                     double &dq0_dy2,
                                     double &dq0_dz2,
                                                     
                                     double &dq1_dx1,
                                     double &dq1_dy1,
                                     double &dq1_dz1,
                                     double &dq1_dx2,
                                     double &dq1_dy2,
                                     double &dq1_dz2,
                                                         
                                     double &dq2_dx1,
                                     double &dq2_dy1,
                                     double &dq2_dz1,
                                     double &dq2_dx2,
                                     double &dq2_dy2,
                                     double &dq2_dz2,
                                                 
                                     double &dq0_dxp,
                                     double &dq0_dyp,
                                     double &dq0_dzp,
                                                 
                                     double &dq1_dxp,
                                     double &dq1_dyp,
                                     double &dq1_dzp,
                                                 
                                     double &dq2_dxp,
                                     double &dq2_dyp,
                                     double &dq2_dzp,    
                                     
                                     double Psi[3],
                                     
                                     double &DeltaTime,
                                                                                                     
                                     double Coef[9])
{

    if ( IsWakeEdge_ && Time_ >= MinValidTimeStep_ && !IsFrozenTrailingEdge_ ) {

       if ( WakeEdgeData_->IsRotorWake() == 0 || WakeEdgeData_->IsRotorWake() == 2 ) {
          
          // Faraway edge
          
          Coef[0] =     ( -dq0_dx1*DeltaTime )*Psi[0]
                  +     ( -dq1_dx1*DeltaTime )*Psi[1]
                  +     ( -dq2_dx1*DeltaTime )*Psi[2];                  
                                                                                                                               
          Coef[1] =     ( -dq0_dy1*DeltaTime )*Psi[0]
                  +     ( -dq1_dy1*DeltaTime )*Psi[1]
                  +     ( -dq2_dy1*DeltaTime )*Psi[2];
                                                                                                                                           
          Coef[2] =     ( -dq0_dz1*DeltaTime )*Psi[0]
                  +     ( -dq1_dz1*DeltaTime )*Psi[1]          
                  +     ( -dq2_dz1*DeltaTime )*Psi[2];  
                                 
          Coef[3] =     ( -dq0_dx2*DeltaTime )*Psi[0]
                  +     ( -dq1_dx2*DeltaTime )*Psi[1]
                  +     ( -dq2_dx2*DeltaTime )*Psi[2];
                                 
          Coef[4] =     ( -dq0_dy2*DeltaTime )*Psi[0]
                  +     ( -dq1_dy2*DeltaTime )*Psi[1]
                  +     ( -dq2_dy2*DeltaTime )*Psi[2]; 
                                  
          Coef[5] =     ( -dq0_dz2*DeltaTime )*Psi[0]
                  +     ( -dq1_dz2*DeltaTime )*Psi[1]          
                  +     ( -dq2_dz2*DeltaTime )*Psi[2]; 

          // This edge
          
          Coef[6] = 0.5*( -dq0_dxp*DeltaTime )*Psi[0]
                  + 0.5*( -dq1_dxp*DeltaTime )*Psi[1]
                  + 0.5*( -dq2_dxp*DeltaTime )*Psi[2];                  
                                   
          Coef[7] = 0.5*( -dq0_dyp*DeltaTime )*Psi[0]
                  + 0.5*( -dq1_dyp*DeltaTime )*Psi[1]
                  + 0.5*( -dq2_dyp*DeltaTime )*Psi[2];
                                    
          Coef[8] = 0.5*( -dq0_dzp*DeltaTime )*Psi[0]
                  + 0.5*( -dq1_dzp*DeltaTime )*Psi[1]          
                  + 0.5*( -dq2_dzp*DeltaTime )*Psi[2];  
                  
       }
       
       else {
          
          double VecR[3], VecP[3], Dot, CoefX, CoefY, CoefZ;
          double pVecR_Velocity[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pVelocity[3][3], pVecP_pXYZ[3][3][3];
          
          // Subtract out free stream...
          
          VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
          VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
          VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
          
          pVecR_Velocity[0] = 1.;
          pVecR_Velocity[1] = 1.;
          pVecR_Velocity[2] = 1.;
                    
          // Determine component in thrust direction
          
          Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
          
          // Limiter... we only take corrections in minus the thrust direction
          
          if ( Dot < 0. ) {
             
             pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
             pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
             pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
          
          }
          
          else {
             
             Dot = 0.;
          
             pDot_pVecR[0] = 0.;
             pDot_pVecR[1] = 0.;
             pDot_pVecR[2] = 0.;
             
          }
          
          // Add back in free stream component
          
          VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
          VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
          VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
          
          pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
          pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
          pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                 
          // Chain rule
          
          pVecP_pVelocity[0][0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[0][1] = pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[0][2] = pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pVecP_pVelocity[1][0] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[1][1] = pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[1][2] = pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Velocity[2];
          
          pVecP_pVelocity[2][0] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Velocity[0];
          pVecP_pVelocity[2][1] = pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Velocity[1];
          pVecP_pVelocity[2][2] = pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Velocity[2];
           
          // Up
          
          pVecP_pXYZ[0][0][0] = pVecP_pVelocity[0][0] * dq0_dx1
                              + pVecP_pVelocity[0][1] * dq1_dx1
                              + pVecP_pVelocity[0][2] * dq2_dx1;

          pVecP_pXYZ[0][1][0] = pVecP_pVelocity[0][0] * dq0_dy1
                              + pVecP_pVelocity[0][1] * dq1_dy1
                              + pVecP_pVelocity[0][2] * dq2_dy1;

          pVecP_pXYZ[0][2][0] = pVecP_pVelocity[0][0] * dq0_dz1
                              + pVecP_pVelocity[0][1] * dq1_dz1
                              + pVecP_pVelocity[0][2] * dq2_dz1;
                              
          pVecP_pXYZ[0][0][1] = pVecP_pVelocity[0][0] * dq0_dx2
                              + pVecP_pVelocity[0][1] * dq1_dx2
                              + pVecP_pVelocity[0][2] * dq2_dx2;
                                                             
          pVecP_pXYZ[0][1][1] = pVecP_pVelocity[0][0] * dq0_dy2
                              + pVecP_pVelocity[0][1] * dq1_dy2
                              + pVecP_pVelocity[0][2] * dq2_dy2;
                                                              
          pVecP_pXYZ[0][2][1] = pVecP_pVelocity[0][0] * dq0_dz2
                              + pVecP_pVelocity[0][1] * dq1_dz2
                              + pVecP_pVelocity[0][2] * dq2_dz2;                                                                               

          pVecP_pXYZ[0][0][2] = pVecP_pVelocity[0][0] * dq0_dxp
                              + pVecP_pVelocity[0][1] * dq1_dxp
                              + pVecP_pVelocity[0][2] * dq2_dxp;
                                                            
          pVecP_pXYZ[0][1][2] = pVecP_pVelocity[0][0] * dq0_dyp
                              + pVecP_pVelocity[0][1] * dq1_dyp
                              + pVecP_pVelocity[0][2] * dq2_dyp;
                                                            
          pVecP_pXYZ[0][2][2] = pVecP_pVelocity[0][0] * dq0_dzp
                              + pVecP_pVelocity[0][1] * dq1_dzp
                              + pVecP_pVelocity[0][2] * dq2_dzp;      
                              
          // Vp
          
          pVecP_pXYZ[1][0][0] = pVecP_pVelocity[1][0] * dq0_dx1
                              + pVecP_pVelocity[1][1] * dq1_dx1
                              + pVecP_pVelocity[1][2] * dq2_dx1;
                                               
          pVecP_pXYZ[1][1][0] = pVecP_pVelocity[1][0] * dq0_dy1
                              + pVecP_pVelocity[1][1] * dq1_dy1
                              + pVecP_pVelocity[1][2] * dq2_dy1;
                                                
          pVecP_pXYZ[1][2][0] = pVecP_pVelocity[1][0] * dq0_dz1
                              + pVecP_pVelocity[1][1] * dq1_dz1
                              + pVecP_pVelocity[1][2] * dq2_dz1;
                                               
          pVecP_pXYZ[1][0][1] = pVecP_pVelocity[1][0] * dq0_dx2
                              + pVecP_pVelocity[1][1] * dq1_dx2
                              + pVecP_pVelocity[1][2] * dq2_dx2;
                                                          
          pVecP_pXYZ[1][1][1] = pVecP_pVelocity[1][0] * dq0_dy2
                              + pVecP_pVelocity[1][1] * dq1_dy2
                              + pVecP_pVelocity[1][2] * dq2_dy2;
                                                          
          pVecP_pXYZ[1][2][1] = pVecP_pVelocity[1][0] * dq0_dz2
                              + pVecP_pVelocity[1][1] * dq1_dz2
                              + pVecP_pVelocity[1][2] * dq2_dz2;       

          pVecP_pXYZ[1][0][2] = pVecP_pVelocity[1][0] * dq0_dxp
                              + pVecP_pVelocity[1][1] * dq1_dxp
                              + pVecP_pVelocity[1][2] * dq2_dxp;
                                                              
          pVecP_pXYZ[1][1][2] = pVecP_pVelocity[1][0] * dq0_dyp
                              + pVecP_pVelocity[1][1] * dq1_dyp
                              + pVecP_pVelocity[1][2] * dq2_dyp;
                                                             
          pVecP_pXYZ[1][2][2] = pVecP_pVelocity[1][0] * dq0_dzp
                              + pVecP_pVelocity[1][1] * dq1_dzp
                              + pVecP_pVelocity[1][2] * dq2_dzp;      
                                                               
          // Wp
          
          pVecP_pXYZ[2][0][0] = pVecP_pVelocity[2][0] * dq0_dx1
                              + pVecP_pVelocity[2][1] * dq1_dx1
                              + pVecP_pVelocity[2][2] * dq2_dx1;
                                               
          pVecP_pXYZ[2][1][0] = pVecP_pVelocity[2][0] * dq0_dy1
                              + pVecP_pVelocity[2][1] * dq1_dy1
                              + pVecP_pVelocity[2][2] * dq2_dy1;
                                                
          pVecP_pXYZ[2][2][0] = pVecP_pVelocity[2][0] * dq0_dz1
                              + pVecP_pVelocity[2][1] * dq1_dz1
                              + pVecP_pVelocity[2][2] * dq2_dz1;
                                               
          pVecP_pXYZ[2][0][1] = pVecP_pVelocity[2][0] * dq0_dx2
                              + pVecP_pVelocity[2][1] * dq1_dx2
                              + pVecP_pVelocity[2][2] * dq2_dx2;
                                                      
          pVecP_pXYZ[2][1][1] = pVecP_pVelocity[2][0] * dq0_dy2
                              + pVecP_pVelocity[2][1] * dq1_dy2
                              + pVecP_pVelocity[2][2] * dq2_dy2;
                                                       
          pVecP_pXYZ[2][2][1] = pVecP_pVelocity[2][0] * dq0_dz2
                              + pVecP_pVelocity[2][1] * dq1_dz2
                              + pVecP_pVelocity[2][2] * dq2_dz2;                                    

          pVecP_pXYZ[2][0][2] = pVecP_pVelocity[2][0] * dq0_dxp
                              + pVecP_pVelocity[2][1] * dq1_dxp
                              + pVecP_pVelocity[2][2] * dq2_dxp;
                                                             
          pVecP_pXYZ[2][1][2] = pVecP_pVelocity[2][0] * dq0_dyp
                              + pVecP_pVelocity[2][1] * dq1_dyp
                              + pVecP_pVelocity[2][2] * dq2_dyp;
                                                             
          pVecP_pXYZ[2][2][2] = pVecP_pVelocity[2][0] * dq0_dzp
                              + pVecP_pVelocity[2][1] * dq1_dzp
                              + pVecP_pVelocity[2][2] * dq2_dzp;   

          // Faraway edge
          
          CoefX   =     ( -pVecP_pXYZ[0][0][0]*DeltaTime )*WakeEdgeData_->RotorThrustVector(0)
                  +     ( -pVecP_pXYZ[1][0][0]*DeltaTime )*WakeEdgeData_->RotorThrustVector(1)
                  +     ( -pVecP_pXYZ[2][0][0]*DeltaTime )*WakeEdgeData_->RotorThrustVector(2);                  
                                          
          CoefY   =     ( -pVecP_pXYZ[0][1][0]*DeltaTime )*WakeEdgeData_->RotorThrustVector(0)
                  +     ( -pVecP_pXYZ[1][1][0]*DeltaTime )*WakeEdgeData_->RotorThrustVector(1)
                  +     ( -pVecP_pXYZ[2][1][0]*DeltaTime )*WakeEdgeData_->RotorThrustVector(2);                  
                                           
          CoefZ   =     ( -pVecP_pXYZ[0][2][0]*DeltaTime )*WakeEdgeData_->RotorThrustVector(0)
                  +     ( -pVecP_pXYZ[1][2][0]*DeltaTime )*WakeEdgeData_->RotorThrustVector(1)
                  +     ( -pVecP_pXYZ[2][2][0]*DeltaTime )*WakeEdgeData_->RotorThrustVector(2);                  

          Coef[0] = CoefX * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefX * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefX * WakeEdgeData_->RotorThrustVector(2) * Psi[2];                 
      
                                                 
          Coef[1] = CoefY * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefY * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefY * WakeEdgeData_->RotorThrustVector(2) * Psi[2];    
                                                                       
          Coef[2] = CoefZ * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefZ * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefZ * WakeEdgeData_->RotorThrustVector(2) * Psi[2];    
 
          CoefX   =     ( -pVecP_pXYZ[0][0][1]*DeltaTime )*WakeEdgeData_->RotorThrustVector(0)
                  +     ( -pVecP_pXYZ[1][0][1]*DeltaTime )*WakeEdgeData_->RotorThrustVector(1)
                  +     ( -pVecP_pXYZ[2][0][1]*DeltaTime )*WakeEdgeData_->RotorThrustVector(2);                  
                                         
          CoefY   =     ( -pVecP_pXYZ[0][1][1]*DeltaTime )*WakeEdgeData_->RotorThrustVector(0)
                  +     ( -pVecP_pXYZ[1][1][1]*DeltaTime )*WakeEdgeData_->RotorThrustVector(1)
                  +     ( -pVecP_pXYZ[2][1][1]*DeltaTime )*WakeEdgeData_->RotorThrustVector(2);                  
                                            
          CoefZ   =     ( -pVecP_pXYZ[0][2][1]*DeltaTime )*WakeEdgeData_->RotorThrustVector(0)
                  +     ( -pVecP_pXYZ[1][2][1]*DeltaTime )*WakeEdgeData_->RotorThrustVector(1)
                  +     ( -pVecP_pXYZ[2][2][1]*DeltaTime )*WakeEdgeData_->RotorThrustVector(2);                  

          Coef[3] = CoefX * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefX * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefX * WakeEdgeData_->RotorThrustVector(2) * Psi[2];                 
      
                                                 
          Coef[4] = CoefY * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefY * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefY * WakeEdgeData_->RotorThrustVector(2) * Psi[2];    
                                                                       
          Coef[5] = CoefZ * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefZ * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefZ * WakeEdgeData_->RotorThrustVector(2) * Psi[2];    
 
          // This edge

          CoefX   = 0.5*( -pVecP_pXYZ[0][0][2]*DeltaTime )*WakeEdgeData_->RotorThrustVector(0)
                  + 0.5*( -pVecP_pXYZ[1][0][2]*DeltaTime )*WakeEdgeData_->RotorThrustVector(1)
                  + 0.5*( -pVecP_pXYZ[2][0][2]*DeltaTime )*WakeEdgeData_->RotorThrustVector(2);                  
                                                         
          CoefY   = 0.5*( -pVecP_pXYZ[0][1][2]*DeltaTime )*WakeEdgeData_->RotorThrustVector(0)
                  + 0.5*( -pVecP_pXYZ[1][1][2]*DeltaTime )*WakeEdgeData_->RotorThrustVector(1)
                  + 0.5*( -pVecP_pXYZ[2][1][2]*DeltaTime )*WakeEdgeData_->RotorThrustVector(2);                  
                                                         
          CoefZ   = 0.5*( -pVecP_pXYZ[0][2][2]*DeltaTime )*WakeEdgeData_->RotorThrustVector(0)
                  + 0.5*( -pVecP_pXYZ[1][2][2]*DeltaTime )*WakeEdgeData_->RotorThrustVector(1)
                  + 0.5*( -pVecP_pXYZ[2][2][2]*DeltaTime )*WakeEdgeData_->RotorThrustVector(2);                  

          Coef[6] = CoefX * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefX * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefX * WakeEdgeData_->RotorThrustVector(2) * Psi[2];                 
      
                                                 
          Coef[7] = CoefY * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefY * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefY * WakeEdgeData_->RotorThrustVector(2) * Psi[2];    
                                                                       
          Coef[8] = CoefZ * WakeEdgeData_->RotorThrustVector(0) * Psi[0]
                  + CoefZ * WakeEdgeData_->RotorThrustVector(1) * Psi[1]
                  + CoefZ * WakeEdgeData_->RotorThrustVector(2) * Psi[2];    
     
       }          
                
    }     
    
    else {
       
       Coef[0] = 0.;
       Coef[1] = 0.;
       Coef[2] = 0.;
       Coef[3] = 0.;
       Coef[4] = 0.;
       Coef[5] = 0.;
       Coef[6] = 0.;
       Coef[7] = 0.;
       Coef[8] = 0.;
              
    }       

}

/*##############################################################################
#                                                                              #
#                       VSP_EDGE Wake_pResidual_pVinf                          #
#                                                                              #
##############################################################################*/

void VSP_EDGE::Wake_pResidual_pVinf(double pResidual_pVinf[3][3])
{
    
    double Vec[3];
    
    if ( IsWakeEdge_ && WakeEdgeData_->dS() > 0. && !IsFrozenTrailingEdge_ ) {

       if ( WakeEdgeData_->IsRotorWake() == 0 ) {
          
          pResidual_pVinf[0][0] = -WakeEdgeData_->dS()/Q_ + U()*U()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          pResidual_pVinf[0][1] =                           U()*V()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          pResidual_pVinf[0][2] =                           U()*W()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          
          pResidual_pVinf[1][0] =                           V()*U()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          pResidual_pVinf[1][1] = -WakeEdgeData_->dS()/Q_ + V()*V()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          pResidual_pVinf[1][2] =                           V()*W()*WakeEdgeData_->dS()/(Q_*Q_*Q_);  
          
          pResidual_pVinf[2][0] =                           W()*U()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          pResidual_pVinf[2][1] =                           W()*V()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          pResidual_pVinf[2][2] = -WakeEdgeData_->dS()/Q_ + W()*W()*WakeEdgeData_->dS()/(Q_*Q_*Q_);    
          
       }
       
       else {

          double VecR[3], VecP[3];
          double pVecR_Vinf[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pVinf[3][3];
          double pResX_pVinf[3], pResY_pVinf[3], pResZ_pVinf[3];
          double Dot, DotX, DotY, DotZ;
       
          double pUp_pGamma;                
          double pVp_pGamma;                
          double pWp_pGamma;
                         
          double pUp_pMesh;               
          double pVp_pMesh;               
          double pWp_pMesh;
                         
          double pUp_pStall;                
          double pVp_pStall;                
          double pWp_pStall;
       
          // Subtract out free stream...
          
          VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
          VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
          VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
          
          pVecR_Vinf[0] = -1.;
          pVecR_Vinf[1] = -1.;
          pVecR_Vinf[2] = -1.;
                    
          // Determine component in thrust direction
          
          Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
          
          // Limiter... we only take velocity components in minus the thrust direction
          
          if ( Dot < 0. ) {
             
             pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
             pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
             pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
       
          }
          
          else {
             
             Dot = 0.;
       
             pDot_pVecR[0] = 0.;
             pDot_pVecR[1] = 0.;
             pDot_pVecR[2] = 0.;
             
          }
       
          // Add back in free stream component
          
          VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
          VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
          VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
          
          pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
          pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
          pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                 
          // Chain rule
          
          pVecP_pVinf[0][0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Vinf[0];
          pVecP_pVinf[0][1] = pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Vinf[1];
          pVecP_pVinf[0][2] = pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Vinf[2];
                                                                   
          pVecP_pVinf[1][0] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Vinf[0];
          pVecP_pVinf[1][1] = pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Vinf[1];
          pVecP_pVinf[1][2] = pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Vinf[2];
                                                                 
          pVecP_pVinf[2][0] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Vinf[0];
          pVecP_pVinf[2][1] = pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Vinf[1];
          pVecP_pVinf[2][2] = pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Vinf[2];
 
          pResX_pVinf[0] = -pVecP_pVinf[0][0]*WakeEdgeData_->dS()/Q_ + VecP[0]*(VecP[0]*pVecP_pVinf[0][0] + VecP[1]*pVecP_pVinf[1][0] + VecP[2]*pVecP_pVinf[2][0] )*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          pResX_pVinf[1] = -pVecP_pVinf[0][1]*WakeEdgeData_->dS()/Q_ + VecP[0]*(VecP[0]*pVecP_pVinf[0][1] + VecP[1]*pVecP_pVinf[1][1] + VecP[2]*pVecP_pVinf[2][1] )*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          pResX_pVinf[2] = -pVecP_pVinf[0][2]*WakeEdgeData_->dS()/Q_ + VecP[0]*(VecP[0]*pVecP_pVinf[0][2] + VecP[1]*pVecP_pVinf[1][2] + VecP[2]*pVecP_pVinf[2][2] )*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          
          pResY_pVinf[0] = -pVecP_pVinf[1][0]*WakeEdgeData_->dS()/Q_ + VecP[1]*(VecP[0]*pVecP_pVinf[0][0] + VecP[1]*pVecP_pVinf[1][0] + VecP[2]*pVecP_pVinf[2][0] )*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          pResY_pVinf[1] = -pVecP_pVinf[1][1]*WakeEdgeData_->dS()/Q_ + VecP[1]*(VecP[0]*pVecP_pVinf[0][1] + VecP[1]*pVecP_pVinf[1][1] + VecP[2]*pVecP_pVinf[2][1] )*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          pResY_pVinf[2] = -pVecP_pVinf[1][2]*WakeEdgeData_->dS()/Q_ + VecP[1]*(VecP[0]*pVecP_pVinf[0][2] + VecP[1]*pVecP_pVinf[1][2] + VecP[2]*pVecP_pVinf[2][2] )*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          
          pResZ_pVinf[0] = -pVecP_pVinf[2][0]*WakeEdgeData_->dS()/Q_ + VecP[2]*(VecP[0]*pVecP_pVinf[0][0] + VecP[1]*pVecP_pVinf[1][0] + VecP[2]*pVecP_pVinf[2][0] )*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          pResZ_pVinf[1] = -pVecP_pVinf[2][1]*WakeEdgeData_->dS()/Q_ + VecP[2]*(VecP[0]*pVecP_pVinf[0][1] + VecP[1]*pVecP_pVinf[1][1] + VecP[2]*pVecP_pVinf[2][1] )*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          pResZ_pVinf[2] = -pVecP_pVinf[2][2]*WakeEdgeData_->dS()/Q_ + VecP[2]*(VecP[0]*pVecP_pVinf[0][2] + VecP[1]*pVecP_pVinf[1][2] + VecP[2]*pVecP_pVinf[2][2] )*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          
          DotX = pResX_pVinf[0]*WakeEdgeData_->RotorThrustVector(0) + pResY_pVinf[0]*WakeEdgeData_->RotorThrustVector(1)+ pResZ_pVinf[0]*WakeEdgeData_->RotorThrustVector(2);
          DotY = pResX_pVinf[1]*WakeEdgeData_->RotorThrustVector(0) + pResY_pVinf[1]*WakeEdgeData_->RotorThrustVector(1)+ pResZ_pVinf[1]*WakeEdgeData_->RotorThrustVector(2);
          DotZ = pResX_pVinf[2]*WakeEdgeData_->RotorThrustVector(0) + pResY_pVinf[2]*WakeEdgeData_->RotorThrustVector(1)+ pResZ_pVinf[2]*WakeEdgeData_->RotorThrustVector(2);

          pResidual_pVinf[0][0] = DotX*WakeEdgeData_->RotorThrustVector(0);
          pResidual_pVinf[0][1] = DotY*WakeEdgeData_->RotorThrustVector(0);
          pResidual_pVinf[0][2] = DotZ*WakeEdgeData_->RotorThrustVector(0);

          pResidual_pVinf[1][0] = DotX*WakeEdgeData_->RotorThrustVector(1);
          pResidual_pVinf[1][1] = DotY*WakeEdgeData_->RotorThrustVector(1);
          pResidual_pVinf[1][2] = DotZ*WakeEdgeData_->RotorThrustVector(1);
          
          pResidual_pVinf[2][0] = DotX*WakeEdgeData_->RotorThrustVector(2);
          pResidual_pVinf[2][1] = DotY*WakeEdgeData_->RotorThrustVector(2);
          pResidual_pVinf[2][2] = DotZ*WakeEdgeData_->RotorThrustVector(2);
 
       }
         
    }
        
    else {
    
       pResidual_pVinf[0][0] = 0.;
       pResidual_pVinf[0][1] = 0.;
       pResidual_pVinf[0][2] = 0.;
                             
       pResidual_pVinf[1][0] = 0.;
       pResidual_pVinf[1][1] = 0.;
       pResidual_pVinf[1][2] = 0.;  
                             
       pResidual_pVinf[2][0] = 0.;
       pResidual_pVinf[2][1] = 0.;
       pResidual_pVinf[2][2] = 0.;    
       
    }

}

/*##############################################################################
#                                                                              #
#                       VSP_EDGE Wake_pResidual_pVinf                          #
#                                                                              #
##############################################################################*/

void VSP_EDGE::Wake_pResidual_pVinfOld(double pResidual_pVinf[3][3])
{
    
    double Vec[3];
    
    if ( IsWakeEdge_ && WakeEdgeData_->dS() > 0. && !IsFrozenTrailingEdge_ ) {

       pResidual_pVinf[0][0] = -WakeEdgeData_->dS()/Q_ + U()*U()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
       pResidual_pVinf[0][1] =                           U()*V()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
       pResidual_pVinf[0][2] =                           U()*W()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
       
       pResidual_pVinf[1][0] =                           V()*U()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
       pResidual_pVinf[1][1] = -WakeEdgeData_->dS()/Q_ + V()*V()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
       pResidual_pVinf[1][2] =                           V()*W()*WakeEdgeData_->dS()/(Q_*Q_*Q_);  
       
       pResidual_pVinf[2][0] =                           W()*U()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
       pResidual_pVinf[2][1] =                           W()*V()*WakeEdgeData_->dS()/(Q_*Q_*Q_);
       pResidual_pVinf[2][2] = -WakeEdgeData_->dS()/Q_ + W()*W()*WakeEdgeData_->dS()/(Q_*Q_*Q_);    
          
    }
        
    else {
    
       pResidual_pVinf[0][0] = 0.;
       pResidual_pVinf[0][1] = 0.;
       pResidual_pVinf[0][2] = 0.;
                             
       pResidual_pVinf[1][0] = 0.;
       pResidual_pVinf[1][1] = 0.;
       pResidual_pVinf[1][2] = 0.;  
                             
       pResidual_pVinf[2][0] = 0.;
       pResidual_pVinf[2][1] = 0.;
       pResidual_pVinf[2][2] = 0.;    
       
    }

}

/*##############################################################################
#                                                                              #
#                 VSP_EDGE UnsteadyWake_pResidual_pVinf                        #
#                                                                              #
##############################################################################*/

void VSP_EDGE::UnsteadyWake_pResidual_pVinf(double pResidual_pVinf[3][3], double &DeltaTime)
{
    
    if ( IsWakeEdge_ && Time_ >= MinValidTimeStep_ && !IsFrozenTrailingEdge_ ) {

       if ( WakeEdgeData_->IsRotorWake() == 0 || WakeEdgeData_->IsRotorWake() == 2 ) {
          
          pResidual_pVinf[0][0] = -DeltaTime;
          pResidual_pVinf[0][1] = 0.;
          pResidual_pVinf[0][2] = 0.;
          
          pResidual_pVinf[1][0] = 0.;
          pResidual_pVinf[1][1] = -DeltaTime;
          pResidual_pVinf[1][2] = 0.;  
          
          pResidual_pVinf[2][0] = 0.;
          pResidual_pVinf[2][1] = 0.;
          pResidual_pVinf[2][2] = -DeltaTime;    
          
       }
         
       else {

          double VecR[3], VecP[3];
          double pVecR_Vinf[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pVinf[3][3];
          double Dot;

          // Subtract out free stream...
          
          VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
          VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
          VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
          
          pVecR_Vinf[0] = -1.;
          pVecR_Vinf[1] = -1.;
          pVecR_Vinf[2] = -1.;
                    
          // Determine component in thrust direction
          
          Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
          
          // Limiter... we only take velocity components in minus the thrust direction
          
          if ( Dot < 0. ) {
             
             pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
             pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
             pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
       
          }
          
          else {
             
             Dot = 0.;
       
             pDot_pVecR[0] = 0.;
             pDot_pVecR[1] = 0.;
             pDot_pVecR[2] = 0.;
             
          }
       
          // Add back in free stream component
          
          VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
          VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
          VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
          
          pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
          pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
          pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                 
          // Chain rule
          
          pVecP_pVinf[0][0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Vinf[0];
          pVecP_pVinf[0][1] = pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Vinf[1];
          pVecP_pVinf[0][2] = pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Vinf[2];
                                                                   
          pVecP_pVinf[1][0] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Vinf[0];
          pVecP_pVinf[1][1] = pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Vinf[1];
          pVecP_pVinf[1][2] = pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Vinf[2];
                                                                 
          pVecP_pVinf[2][0] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Vinf[0];
          pVecP_pVinf[2][1] = pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Vinf[1];
          pVecP_pVinf[2][2] = pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Vinf[2];

          pResidual_pVinf[0][0] = -DeltaTime * pVecP_pVinf[0][0];
          pResidual_pVinf[0][1] = -DeltaTime * pVecP_pVinf[0][1];
          pResidual_pVinf[0][2] = -DeltaTime * pVecP_pVinf[0][2];

          pResidual_pVinf[1][0] = -DeltaTime * pVecP_pVinf[1][0];
          pResidual_pVinf[1][1] = -DeltaTime * pVecP_pVinf[1][1];
          pResidual_pVinf[1][2] = -DeltaTime * pVecP_pVinf[1][2];
          
          pResidual_pVinf[2][0] = -DeltaTime * pVecP_pVinf[2][0];
          pResidual_pVinf[2][1] = -DeltaTime * pVecP_pVinf[2][1];
          pResidual_pVinf[2][2] = -DeltaTime * pVecP_pVinf[2][2];

       } 
                  
    }
        
    else {
    
       pResidual_pVinf[0][0] = 0.;
       pResidual_pVinf[0][1] = 0.;
       pResidual_pVinf[0][2] = 0.;
                             
       pResidual_pVinf[1][0] = 0.;
       pResidual_pVinf[1][1] = 0.;
       pResidual_pVinf[1][2] = 0.;  
                             
       pResidual_pVinf[2][0] = 0.;
       pResidual_pVinf[2][1] = 0.;
       pResidual_pVinf[2][2] = 0.;    
       
    }

}


/*##############################################################################
#                                                                              #
#                       VSP_EDGE Wake_pResidual_pMach                          #
#                                                                              #
##############################################################################*/

void VSP_EDGE::Wake_pResidual_pMach(double pResidual_pMach[3])
{
    
    if ( IsWakeEdge_ && WakeEdgeData_->dS() > 0. && !IsFrozenTrailingEdge_ ) {
    
       if ( WakeEdgeData_->IsRotorWake() == 0 ) {
    
          pResidual_pMach[0] = -dU_dMach()*WakeEdgeData_->dS()/Q_ + U()*WakeEdgeData_->dS()*(U()*dU_dMach() + V()*dV_dMach() + W()*dW_dMach())/(Q_*Q_*Q_);  
          
          pResidual_pMach[1] = -dV_dMach()*WakeEdgeData_->dS()/Q_ + V()*WakeEdgeData_->dS()*(U()*dU_dMach() + V()*dV_dMach() + W()*dW_dMach())/(Q_*Q_*Q_);  
                                                                 
          pResidual_pMach[2] = -dW_dMach()*WakeEdgeData_->dS()/Q_ + W()*WakeEdgeData_->dS()*(U()*dU_dMach() + V()*dV_dMach() + W()*dW_dMach())/(Q_*Q_*Q_);  
          
       }
       
       else {

          double VecR[3], VecP[3];
          double pVecR_Velocity[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pMach[3];
          double pResX_pMach, pResY_pMach, pResZ_pMach;
          double Dot, DotX, DotY, DotZ;
       
          double pUp_pGamma;                
          double pVp_pGamma;                
          double pWp_pGamma;
                         
          double pUp_pMesh;               
          double pVp_pMesh;               
          double pWp_pMesh;
                         
          double pUp_pStall;                
          double pVp_pStall;                
          double pWp_pStall;
       
          // Subtract out free stream...
          
          VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
          VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
          VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
          
          pVecR_Velocity[0] = 1.;
          pVecR_Velocity[1] = 1.;
          pVecR_Velocity[2] = 1.;
                    
          // Determine component in thrust direction
          
          Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
          
          // Limiter... we only take velocity components in minus the thrust direction
          
          if ( Dot < 0. ) {
             
             pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
             pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
             pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
       
          }
          
          else {
             
             Dot = 0.;
       
             pDot_pVecR[0] = 0.;
             pDot_pVecR[1] = 0.;
             pDot_pVecR[2] = 0.;
             
          }
       
          // Add back in free stream component
          
          VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
          VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
          VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
          
          pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
          pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
          pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                 
          // Chain rule
          
          pVecP_pMach[0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Velocity[0] * dU_dMach()
                         + pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Velocity[1] * dV_dMach()
                         + pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Velocity[2] * dW_dMach();
                                                     
          pVecP_pMach[1] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Velocity[0] * dU_dMach()
                         + pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Velocity[1] * dV_dMach()
                         + pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Velocity[2] * dW_dMach();
                                                  
          pVecP_pMach[2] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Velocity[0] * dU_dMach()
                         + pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Velocity[1] * dV_dMach()
                         + pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Velocity[2] * dW_dMach();
 
          pResX_pMach = -pVecP_pMach[0]*WakeEdgeData_->dS()/Q_ + VecP[0]*(VecP[0]*pVecP_pMach[0] + VecP[1]*pVecP_pMach[1] + VecP[2]*pVecP_pMach[2] )*WakeEdgeData_->dS()/(Q_*Q_*Q_);
      
          pResY_pMach = -pVecP_pMach[1]*WakeEdgeData_->dS()/Q_ + VecP[1]*(VecP[0]*pVecP_pMach[0] + VecP[1]*pVecP_pMach[1] + VecP[2]*pVecP_pMach[2] )*WakeEdgeData_->dS()/(Q_*Q_*Q_);
                
          pResZ_pMach = -pVecP_pMach[2]*WakeEdgeData_->dS()/Q_ + VecP[2]*(VecP[0]*pVecP_pMach[0] + VecP[1]*pVecP_pMach[1] + VecP[2]*pVecP_pMach[2] )*WakeEdgeData_->dS()/(Q_*Q_*Q_);
          
          DotX = pResX_pMach*WakeEdgeData_->RotorThrustVector(0) + pResY_pMach*WakeEdgeData_->RotorThrustVector(1)+ pResZ_pMach*WakeEdgeData_->RotorThrustVector(2);
          DotY = pResX_pMach*WakeEdgeData_->RotorThrustVector(0) + pResY_pMach*WakeEdgeData_->RotorThrustVector(1)+ pResZ_pMach*WakeEdgeData_->RotorThrustVector(2);
          DotZ = pResX_pMach*WakeEdgeData_->RotorThrustVector(0) + pResY_pMach*WakeEdgeData_->RotorThrustVector(1)+ pResZ_pMach*WakeEdgeData_->RotorThrustVector(2);

          pResidual_pMach[0] = DotX*WakeEdgeData_->RotorThrustVector(0);
          pResidual_pMach[1] = DotY*WakeEdgeData_->RotorThrustVector(1);
          pResidual_pMach[2] = DotZ*WakeEdgeData_->RotorThrustVector(2);

       }          
          
    }
    
    else {
    
       pResidual_pMach[0] = 0.;  
                           
       pResidual_pMach[1] = 0.;  
                                 
       pResidual_pMach[2] = 0.;  
       
    }

}

/*##############################################################################
#                                                                              #
#                 VSP_EDGE UnsteadyWake_pResidual_pMach                        #
#                                                                              #
##############################################################################*/

void VSP_EDGE::UnsteadyWake_pResidual_pMach(double pResidual_pMach[3], double &DeltaTime)
{
    
    if ( IsWakeEdge_ && Time_ >= MinValidTimeStep_ && !IsFrozenTrailingEdge_ ) {
       
       if ( WakeEdgeData_->IsRotorWake() == 0 || WakeEdgeData_->IsRotorWake() == 2 ) {
    
          pResidual_pMach[0] = -dU_dMach()*DeltaTime;  
                                        
          pResidual_pMach[1] = -dV_dMach()*DeltaTime;  
                                        
          pResidual_pMach[2] = -dW_dMach()*DeltaTime;  
          
       }
       
       else {

          double VecR[3], VecP[3];
          double pVecR_Velocity[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pMach[3];
          double Dot;

          // Subtract out free stream...
          
          VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
          VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
          VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
          
          pVecR_Velocity[0] = 1.;
          pVecR_Velocity[1] = 1.;
          pVecR_Velocity[2] = 1.;
                    
          // Determine component in thrust direction
          
          Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
          
          // Limiter... we only take velocity components in minus the thrust direction
          
          if ( Dot < 0. ) {
             
             pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
             pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
             pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
       
          }
          
          else {
             
             Dot = 0.;
       
             pDot_pVecR[0] = 0.;
             pDot_pVecR[1] = 0.;
             pDot_pVecR[2] = 0.;
             
          }
       
          // Add back in free stream component
          
          VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
          VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
          VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
          
          pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
          pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
          pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                 
          // Chain rule
          
          pVecP_pMach[0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Velocity[0] * dU_dMach()
                         + pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Velocity[1] * dV_dMach()
                         + pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Velocity[2] * dW_dMach();
                                                                    
          pVecP_pMach[1] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Velocity[0] * dU_dMach()
                         + pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Velocity[1] * dV_dMach()
                         + pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Velocity[2] * dW_dMach();
                                                                    
          pVecP_pMach[2] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Velocity[0] * dU_dMach()
                         + pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Velocity[1] * dV_dMach()
                         + pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Velocity[2] * dW_dMach();
          
          pResidual_pMach[0] = -DeltaTime * pVecP_pMach[0];
          pResidual_pMach[1] = -DeltaTime * pVecP_pMach[1];
          pResidual_pMach[2] = -DeltaTime * pVecP_pMach[2];

       }        

    }
    
    else {
    
       pResidual_pMach[0] = 0.;  
                           
       pResidual_pMach[1] = 0.;  
                                 
       pResidual_pMach[2] = 0.;  
       
    }

}

/*##############################################################################
#                                                                              #
#        VSP_EDGE dInducedVelocity_dMesh_dMesh_Unsteady_Transpose              #
#                                                                              #
##############################################################################*/

void VSP_EDGE::dInducedVelocity_dMesh_dMesh_Unsteady_Transpose(double &dq0_dx1,
                                                               double &dq0_dy1,
                                                               double &dq0_dz1,
                                                               double &dq0_dx2,
                                                               double &dq0_dy2,
                                                               double &dq0_dz2,
                                                                               
                                                               double &dq1_dx1,
                                                               double &dq1_dy1,
                                                               double &dq1_dz1,
                                                               double &dq1_dx2,
                                                               double &dq1_dy2,
                                                               double &dq1_dz2,
                                                                                   
                                                               double &dq2_dx1,
                                                               double &dq2_dy1,
                                                               double &dq2_dz1,
                                                               double &dq2_dx2,
                                                               double &dq2_dy2,
                                                               double &dq2_dz2,
                                                                           
                                                               double &dq0_dxp,
                                                               double &dq0_dyp,
                                                               double &dq0_dzp,
                                                                           
                                                               double &dq1_dxp,
                                                               double &dq1_dyp,
                                                               double &dq1_dzp,
                                                                           
                                                               double &dq2_dxp,
                                                               double &dq2_dyp,
                                                               double &dq2_dzp,    
                                                               
                                                               double &DeltaTime,
                                                                                                 
                                                               double Coef[9])
{

    if ( IsWakeEdge_ && Time_ >= MinValidTimeStep_ && !IsFrozenTrailingEdge_ ) {

       if ( WakeEdgeData_->Direction() == 1 ) {
         
          if ( WakeEdgeData_->IsRotorWake() == 0 || WakeEdgeData_->IsRotorWake() == 2 ) {
             
             // Faraway edge
             
             Coef[0] = -DeltaTime*dq0_dx1*dxyz2_[0]
                     + -DeltaTime*dq1_dx1*dxyz2_[1]
                     + -DeltaTime*dq2_dx1*dxyz2_[2];                  
   
             Coef[1] = -DeltaTime*dq0_dy1*dxyz2_[0]
                     + -DeltaTime*dq1_dy1*dxyz2_[1]
                     + -DeltaTime*dq2_dy1*dxyz2_[2];        
                     
             Coef[2] = -DeltaTime*dq0_dz1*dxyz2_[0]
                     + -DeltaTime*dq1_dz1*dxyz2_[1]
                     + -DeltaTime*dq2_dz1*dxyz2_[2];                          
   
             Coef[3] = -DeltaTime*dq0_dx2*dxyz2_[0]
                     + -DeltaTime*dq1_dx2*dxyz2_[1]
                     + -DeltaTime*dq2_dx2*dxyz2_[2];                  
                                       
             Coef[4] = -DeltaTime*dq0_dy2*dxyz2_[0]
                     + -DeltaTime*dq1_dy2*dxyz2_[1]
                     + -DeltaTime*dq2_dy2*dxyz2_[2];        
                                       
             Coef[5] = -DeltaTime*dq0_dz2*dxyz2_[0]
                     + -DeltaTime*dq1_dz2*dxyz2_[1]
                     + -DeltaTime*dq2_dz2*dxyz2_[2];                
      
             // This edge
                     
             Coef[6] = -0.5*DeltaTime*dq0_dxp*dxyz2_[0]
                     + -0.5*DeltaTime*dq1_dxp*dxyz2_[1]
                     + -0.5*DeltaTime*dq2_dxp*dxyz2_[2];                  
   
             Coef[7] = -0.5*DeltaTime*dq0_dyp*dxyz2_[0]
                     + -0.5*DeltaTime*dq1_dyp*dxyz2_[1]
                     + -0.5*DeltaTime*dq2_dyp*dxyz2_[2];         
                     
             Coef[8] = -0.5*DeltaTime*dq0_dzp*dxyz2_[0]
                     + -0.5*DeltaTime*dq1_dzp*dxyz2_[1]
                     + -0.5*DeltaTime*dq2_dzp*dxyz2_[2];      
                     
          }
          
          else {
             
             double VecR[3], VecP[3], Dot;
             double pVecR_Velocity[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pVelocity[3][3], pVecP_pStuff[3];
             
             // Subtract out free stream...
             
             VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
             VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
             VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
             
             pVecR_Velocity[0] = 1.;
             pVecR_Velocity[1] = 1.;
             pVecR_Velocity[2] = 1.;
                       
             // Determine component in thrust direction
             
             Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
             
             // Limiter... we only take corrections in minus the thrust direction
             
             if ( Dot < 0. ) {
                
                pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
                pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
                pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
             
             }
             
             else {
                
                Dot = 0.;
             
                pDot_pVecR[0] = 0.;
                pDot_pVecR[1] = 0.;
                pDot_pVecR[2] = 0.;
                
             }
             
             // Add back in free stream component
             
             VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
             VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
             VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
             
             pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
             pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
             pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                    
             // Chain rule
             
             pVecP_pVelocity[0][0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Velocity[0];
             pVecP_pVelocity[0][1] = pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Velocity[1];
             pVecP_pVelocity[0][2] = pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Velocity[2];
             
             pVecP_pVelocity[1][0] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Velocity[0];
             pVecP_pVelocity[1][1] = pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Velocity[1];
             pVecP_pVelocity[1][2] = pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Velocity[2];
             
             pVecP_pVelocity[2][0] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Velocity[0];
             pVecP_pVelocity[2][1] = pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Velocity[1];
             pVecP_pVelocity[2][2] = pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Velocity[2];

             // Faraway edge
             
             Coef[0]     = -DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dx1 + pVecP_pVelocity[0][1] * dq1_dx1 + pVecP_pVelocity[0][2] * dq2_dx1 ) * dxyz2_[0]
                         + -DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dx1 + pVecP_pVelocity[1][1] * dq1_dx1 + pVecP_pVelocity[1][2] * dq2_dx1 ) * dxyz2_[1]
                         + -DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dx1 + pVecP_pVelocity[2][1] * dq1_dx1 + pVecP_pVelocity[2][2] * dq2_dx1 ) * dxyz2_[2];     
                         
             Coef[1]     = -DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dy1 + pVecP_pVelocity[0][1] * dq1_dy1 + pVecP_pVelocity[0][2] * dq2_dy1 ) * dxyz2_[0]
                         + -DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dy1 + pVecP_pVelocity[1][1] * dq1_dy1 + pVecP_pVelocity[1][2] * dq2_dy1 ) * dxyz2_[1]
                         + -DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dy1 + pVecP_pVelocity[2][1] * dq1_dy1 + pVecP_pVelocity[2][2] * dq2_dy1 ) * dxyz2_[2];     
                         
             Coef[2]     = -DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dz1 + pVecP_pVelocity[0][1] * dq1_dz1 + pVecP_pVelocity[0][2] * dq2_dz1 ) * dxyz2_[0]
                         + -DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dz1 + pVecP_pVelocity[1][1] * dq1_dz1 + pVecP_pVelocity[1][2] * dq2_dz1 ) * dxyz2_[1]
                         + -DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dz1 + pVecP_pVelocity[2][1] * dq1_dz1 + pVecP_pVelocity[2][2] * dq2_dz1 ) * dxyz2_[2];     
                         
             Coef[3]     = -DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dx2 + pVecP_pVelocity[0][1] * dq1_dx2 + pVecP_pVelocity[0][2] * dq2_dx2 ) * dxyz2_[0]
                         + -DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dx2 + pVecP_pVelocity[1][1] * dq1_dx2 + pVecP_pVelocity[1][2] * dq2_dx2 ) * dxyz2_[1]
                         + -DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dx2 + pVecP_pVelocity[2][1] * dq1_dx2 + pVecP_pVelocity[2][2] * dq2_dx2 ) * dxyz2_[2];     
                                                                
             Coef[4]     = -DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dy2 + pVecP_pVelocity[0][1] * dq1_dy2 + pVecP_pVelocity[0][2] * dq2_dy2 ) * dxyz2_[0]
                         + -DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dy2 + pVecP_pVelocity[1][1] * dq1_dy2 + pVecP_pVelocity[1][2] * dq2_dy2 ) * dxyz2_[1]
                         + -DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dy2 + pVecP_pVelocity[2][1] * dq1_dy2 + pVecP_pVelocity[2][2] * dq2_dy2 ) * dxyz2_[2];     
                                                                
             Coef[5]     = -DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dz2 + pVecP_pVelocity[0][1] * dq1_dz2 + pVecP_pVelocity[0][2] * dq2_dz2 ) * dxyz2_[0]
                         + -DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dz2 + pVecP_pVelocity[1][1] * dq1_dz2 + pVecP_pVelocity[1][2] * dq2_dz2 ) * dxyz2_[1]
                         + -DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dz2 + pVecP_pVelocity[2][1] * dq1_dz2 + pVecP_pVelocity[2][2] * dq2_dz2 ) * dxyz2_[2];     

             // This edge
            
             Coef[6] = -0.5*DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dxp + pVecP_pVelocity[0][1] * dq1_dxp + pVecP_pVelocity[0][2] * dq2_dxp ) * dxyz2_[0]
                     + -0.5*DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dxp + pVecP_pVelocity[1][1] * dq1_dxp + pVecP_pVelocity[1][2] * dq2_dxp ) * dxyz2_[1]
                     + -0.5*DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dxp + pVecP_pVelocity[2][1] * dq1_dxp + pVecP_pVelocity[2][2] * dq2_dxp ) * dxyz2_[2];     
                                                     
             Coef[7] = -0.5*DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dyp + pVecP_pVelocity[0][1] * dq1_dyp + pVecP_pVelocity[0][2] * dq2_dyp ) * dxyz2_[0]
                     + -0.5*DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dyp + pVecP_pVelocity[1][1] * dq1_dyp + pVecP_pVelocity[1][2] * dq2_dyp ) * dxyz2_[1]
                     + -0.5*DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dyp + pVecP_pVelocity[2][1] * dq1_dyp + pVecP_pVelocity[2][2] * dq2_dyp ) * dxyz2_[2];     
                                                      
             Coef[8] = -0.5*DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dzp + pVecP_pVelocity[0][1] * dq1_dzp + pVecP_pVelocity[0][2] * dq2_dzp ) * dxyz2_[0]
                     + -0.5*DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dzp + pVecP_pVelocity[1][1] * dq1_dzp + pVecP_pVelocity[1][2] * dq2_dzp ) * dxyz2_[1]
                     + -0.5*DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dzp + pVecP_pVelocity[2][1] * dq1_dzp + pVecP_pVelocity[2][2] * dq2_dzp ) * dxyz2_[2];     
                                     
          }                     
 
       }
       
       else {

          if ( WakeEdgeData_->IsRotorWake() == 0 || WakeEdgeData_->IsRotorWake() == 2 ) {

             // Faraway edge
             
             Coef[0] = -DeltaTime*dq0_dx1*dxyz1_[0]
                     + -DeltaTime*dq1_dx1*dxyz1_[1]
                     + -DeltaTime*dq2_dx1*dxyz1_[2];                  
                                             
             Coef[1] = -DeltaTime*dq0_dy1*dxyz1_[0]
                     + -DeltaTime*dq1_dy1*dxyz1_[1]
                     + -DeltaTime*dq2_dy1*dxyz1_[2];        
                                              
             Coef[2] = -DeltaTime*dq0_dz1*dxyz1_[0]
                     + -DeltaTime*dq1_dz1*dxyz1_[1]
                     + -DeltaTime*dq2_dz1*dxyz1_[2];                          
                                              
             Coef[3] = -DeltaTime*dq0_dx2*dxyz1_[0]
                     + -DeltaTime*dq1_dx2*dxyz1_[1]
                     + -DeltaTime*dq2_dx2*dxyz1_[2];                  
                                              
             Coef[4] = -DeltaTime*dq0_dy2*dxyz1_[0]
                     + -DeltaTime*dq1_dy2*dxyz1_[1]
                     + -DeltaTime*dq2_dy2*dxyz1_[2];        
                                              
             Coef[5] = -DeltaTime*dq0_dz2*dxyz1_[0]
                     + -DeltaTime*dq1_dz2*dxyz1_[1]
                     + -DeltaTime*dq2_dz2*dxyz1_[2];                
      
             // This edge
                     
             Coef[6] = -0.5*DeltaTime*dq0_dxp*dxyz1_[0]
                     + -0.5*DeltaTime*dq1_dxp*dxyz1_[1]
                     + -0.5*DeltaTime*dq2_dxp*dxyz1_[2];                  
                                                 
             Coef[7] = -0.5*DeltaTime*dq0_dyp*dxyz1_[0]
                     + -0.5*DeltaTime*dq1_dyp*dxyz1_[1]
                     + -0.5*DeltaTime*dq2_dyp*dxyz1_[2];         
                                                 
             Coef[8] = -0.5*DeltaTime*dq0_dzp*dxyz1_[0]
                     + -0.5*DeltaTime*dq1_dzp*dxyz1_[1]
                     + -0.5*DeltaTime*dq2_dzp*dxyz1_[2];     
                     
          }
          
          else {
             
             double VecR[3], VecP[3], Dot;
             double pVecR_Velocity[3], pDot_pVecR[3], pVecP_pDot[3], pVecP_pVelocity[3][3], pVecP_pStuff[3];
             
             // Subtract out free stream...
             
             VecR[0] = Velocity_[0] - LocalFreeStreamVelocity(0);
             VecR[1] = Velocity_[1] - LocalFreeStreamVelocity(1);
             VecR[2] = Velocity_[2] - LocalFreeStreamVelocity(2);
             
             pVecR_Velocity[0] = 1.;
             pVecR_Velocity[1] = 1.;
             pVecR_Velocity[2] = 1.;
                       
             // Determine component in thrust direction
             
             Dot = vector_dot(VecR,WakeEdgeData_->RotorThrustVector());
             
             // Limiter... we only take corrections in minus the thrust direction
             
             if ( Dot < 0. ) {
                
                pDot_pVecR[0] = WakeEdgeData_->RotorThrustVector(0);
                pDot_pVecR[1] = WakeEdgeData_->RotorThrustVector(1);
                pDot_pVecR[2] = WakeEdgeData_->RotorThrustVector(2);
             
             }
             
             else {
                
                Dot = 0.;
             
                pDot_pVecR[0] = 0.;
                pDot_pVecR[1] = 0.;
                pDot_pVecR[2] = 0.;
                
             }
             
             // Add back in free stream component
             
             VecP[0] = LocalFreeStreamVelocity(0) + Dot * WakeEdgeData_->RotorThrustVector(0);
             VecP[1] = LocalFreeStreamVelocity(1) + Dot * WakeEdgeData_->RotorThrustVector(1);
             VecP[2] = LocalFreeStreamVelocity(2) + Dot * WakeEdgeData_->RotorThrustVector(2);      
             
             pVecP_pDot[0] = WakeEdgeData_->RotorThrustVector(0);
             pVecP_pDot[1] = WakeEdgeData_->RotorThrustVector(1);
             pVecP_pDot[2] = WakeEdgeData_->RotorThrustVector(2);
                    
             // Chain rule
             
             pVecP_pVelocity[0][0] = pVecP_pDot[0] * pDot_pVecR[0] * pVecR_Velocity[0];
             pVecP_pVelocity[0][1] = pVecP_pDot[0] * pDot_pVecR[1] * pVecR_Velocity[1];
             pVecP_pVelocity[0][2] = pVecP_pDot[0] * pDot_pVecR[2] * pVecR_Velocity[2];
             
             pVecP_pVelocity[1][0] = pVecP_pDot[1] * pDot_pVecR[0] * pVecR_Velocity[0];
             pVecP_pVelocity[1][1] = pVecP_pDot[1] * pDot_pVecR[1] * pVecR_Velocity[1];
             pVecP_pVelocity[1][2] = pVecP_pDot[1] * pDot_pVecR[2] * pVecR_Velocity[2];
             
             pVecP_pVelocity[2][0] = pVecP_pDot[2] * pDot_pVecR[0] * pVecR_Velocity[0];
             pVecP_pVelocity[2][1] = pVecP_pDot[2] * pDot_pVecR[1] * pVecR_Velocity[1];
             pVecP_pVelocity[2][2] = pVecP_pDot[2] * pDot_pVecR[2] * pVecR_Velocity[2];

             // Faraway edge
             
             Coef[0]     = -DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dx1 + pVecP_pVelocity[0][1] * dq1_dx1 + pVecP_pVelocity[0][2] * dq2_dx1 ) * dxyz1_[0]
                         + -DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dx1 + pVecP_pVelocity[1][1] * dq1_dx1 + pVecP_pVelocity[1][2] * dq2_dx1 ) * dxyz1_[1]
                         + -DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dx1 + pVecP_pVelocity[2][1] * dq1_dx1 + pVecP_pVelocity[2][2] * dq2_dx1 ) * dxyz1_[2];     
                                                                                                                                                    
             Coef[1]     = -DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dy1 + pVecP_pVelocity[0][1] * dq1_dy1 + pVecP_pVelocity[0][2] * dq2_dy1 ) * dxyz1_[0]
                         + -DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dy1 + pVecP_pVelocity[1][1] * dq1_dy1 + pVecP_pVelocity[1][2] * dq2_dy1 ) * dxyz1_[1]
                         + -DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dy1 + pVecP_pVelocity[2][1] * dq1_dy1 + pVecP_pVelocity[2][2] * dq2_dy1 ) * dxyz1_[2];     
                                                                                                                                                     
             Coef[2]     = -DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dz1 + pVecP_pVelocity[0][1] * dq1_dz1 + pVecP_pVelocity[0][2] * dq2_dz1 ) * dxyz1_[0]
                         + -DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dz1 + pVecP_pVelocity[1][1] * dq1_dz1 + pVecP_pVelocity[1][2] * dq2_dz1 ) * dxyz1_[1]
                         + -DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dz1 + pVecP_pVelocity[2][1] * dq1_dz1 + pVecP_pVelocity[2][2] * dq2_dz1 ) * dxyz1_[2];     
                                                                                                                                                      
             Coef[3]     = -DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dx2 + pVecP_pVelocity[0][1] * dq1_dx2 + pVecP_pVelocity[0][2] * dq2_dx2 ) * dxyz1_[0]
                         + -DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dx2 + pVecP_pVelocity[1][1] * dq1_dx2 + pVecP_pVelocity[1][2] * dq2_dx2 ) * dxyz1_[1]
                         + -DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dx2 + pVecP_pVelocity[2][1] * dq1_dx2 + pVecP_pVelocity[2][2] * dq2_dx2 ) * dxyz1_[2];     
                                                                                                                                                     
             Coef[4]     = -DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dy2 + pVecP_pVelocity[0][1] * dq1_dy2 + pVecP_pVelocity[0][2] * dq2_dy2 ) * dxyz1_[0]
                         + -DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dy2 + pVecP_pVelocity[1][1] * dq1_dy2 + pVecP_pVelocity[1][2] * dq2_dy2 ) * dxyz1_[1]
                         + -DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dy2 + pVecP_pVelocity[2][1] * dq1_dy2 + pVecP_pVelocity[2][2] * dq2_dy2 ) * dxyz1_[2];     
                                                                                                                                                    
             Coef[5]     = -DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dz2 + pVecP_pVelocity[0][1] * dq1_dz2 + pVecP_pVelocity[0][2] * dq2_dz2 ) * dxyz1_[0]
                         + -DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dz2 + pVecP_pVelocity[1][1] * dq1_dz2 + pVecP_pVelocity[1][2] * dq2_dz2 ) * dxyz1_[1]
                         + -DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dz2 + pVecP_pVelocity[2][1] * dq1_dz2 + pVecP_pVelocity[2][2] * dq2_dz2 ) * dxyz1_[2];     
                                                                                                                                                 
             // This edge                                                                                                                        
                                                                                                                                                 
             Coef[6] = -0.5*DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dxp + pVecP_pVelocity[0][1] * dq1_dxp + pVecP_pVelocity[0][2] * dq2_dxp ) * dxyz1_[0]
                     + -0.5*DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dxp + pVecP_pVelocity[1][1] * dq1_dxp + pVecP_pVelocity[1][2] * dq2_dxp ) * dxyz1_[1]
                     + -0.5*DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dxp + pVecP_pVelocity[2][1] * dq1_dxp + pVecP_pVelocity[2][2] * dq2_dxp ) * dxyz1_[2];     
                                                                                                                                              
             Coef[7] = -0.5*DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dyp + pVecP_pVelocity[0][1] * dq1_dyp + pVecP_pVelocity[0][2] * dq2_dyp ) * dxyz1_[0]
                     + -0.5*DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dyp + pVecP_pVelocity[1][1] * dq1_dyp + pVecP_pVelocity[1][2] * dq2_dyp ) * dxyz1_[1]
                     + -0.5*DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dyp + pVecP_pVelocity[2][1] * dq1_dyp + pVecP_pVelocity[2][2] * dq2_dyp ) * dxyz1_[2];     
                                                                                                                                            
             Coef[8] = -0.5*DeltaTime * ( pVecP_pVelocity[0][0] * dq0_dzp + pVecP_pVelocity[0][1] * dq1_dzp + pVecP_pVelocity[0][2] * dq2_dzp ) * dxyz1_[0]
                     + -0.5*DeltaTime * ( pVecP_pVelocity[1][0] * dq0_dzp + pVecP_pVelocity[1][1] * dq1_dzp + pVecP_pVelocity[1][2] * dq2_dzp ) * dxyz1_[1]
                     + -0.5*DeltaTime * ( pVecP_pVelocity[2][0] * dq0_dzp + pVecP_pVelocity[2][1] * dq1_dzp + pVecP_pVelocity[2][2] * dq2_dzp ) * dxyz1_[2];     
                                      
          }                     
                  
       }               

    }     
    
    else {
       
       Coef[0] = 0.;
       Coef[1] = 0.;
       Coef[2] = 0.;
       Coef[3] = 0.;
       Coef[4] = 0.;
       Coef[5] = 0.;
       Coef[6] = 0.;
       Coef[7] = 0.;
       Coef[8] = 0.;
              
    }       
                  
}

/*##############################################################################
#                                                                              #
#                  VSP_EDGE SizeAdjointEdgeList                                #
#                                                                              #
##############################################################################*/
    
void VSP_EDGE::SizeAdjointEdgeList(int NumberOfEdges)
{

    if ( AdjointEdgeList_ != NULL ) delete [] AdjointEdgeList_;

    if ( AdjointEdgeFactor_ != NULL ) delete [] AdjointEdgeFactor_;
    
    NumberOfAdjointEdges_ = NumberOfEdges;
    
    AdjointEdgeList_ = new int[NumberOfAdjointEdges_ + 1];

    AdjointEdgeFactor_ = new double[NumberOfAdjointEdges_ + 1];
    
    zero_int_array(AdjointEdgeList_, NumberOfAdjointEdges_);

    zero_double_array(AdjointEdgeFactor_, NumberOfAdjointEdges_);
          
}

#include "END_NAME_SPACE.H"
