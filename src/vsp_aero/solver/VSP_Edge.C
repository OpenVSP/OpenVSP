//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Edge.H"

/*##############################################################################
#                                                                              #
#                      Allocate space for statics                              #
#                                                                              #
##############################################################################*/

    double VSP_EDGE::Mach_           = 0.;
    double VSP_EDGE::Kappa_          = 2.;

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
        
    Child1_ = Child2_ = NULL;
    
    Node1_ = 0;
    Node2_ = 0;

    Loop1_ = 0;
    Loop2_ = 0;
    
    LoopL_ = 0;
    LoopR_ = 0;
    
    VortexLoop1_ = 0;
    VortexLoop2_ = 0;
    
    VortexEdge_ = 0;
    
    EdgeType_ = 0;
    IsTrailingEdge_ = 0;
    IsLeadingEdge_ = 0;
    IsBoundaryEdge_ = 0;
    
    CoarseGridEdge_ = 0;
    FineGridEdge_ = 0;
    Level_ = 0.;

    EdgeWasUsedForLoop_ = 0;
    
    S_ = 0.;
    T_ = 0.;
    Sigma_ = 0.;
    Length_ = 0.;
    
                 Forces_[0] =              Forces_[1] =              Forces_[2] = 0.;
         Trefftz_Forces_[0] =      Trefftz_Forces_[1] =      Trefftz_Forces_[2] = 0.;
        Unsteady_Forces_[0] =     Unsteady_Forces_[1] =     Unsteady_Forces_[2] = 0.;
          InducedForces_[0] =       InducedForces_[1] =       InducedForces_[2] = 0.;
            LocalForces_[0] =         LocalForces_[1] =         LocalForces_[2] = 0.;
    Unsteady_LocalForces[0] = Unsteady_LocalForces[1] = Unsteady_LocalForces[2] = 0.;
     
    Verbose_ = 0;
 
    DegenWing_ = 0;
    DegenBody_ = 0;

    X1_ = 0.;
    Y1_ = 0.;
    Z1_ = 0.;
    
    X2_ = 0.;
    Y2_ = 0.;
    Z2_ = 0.;

    XYZc_[0] = XYZc_[1] = XYZc_[2] = 0.;
    
    Vec_[0] = Vec_[1] = Vec_[2] = 0.;

    u_ = v_ = w_ = 0.;
    
    Tolerance_1_ = 0.;
    Tolerance_2_ = 0.;
    Tolerance_4_ = 0.;

    LocalSpacing_ = 0.;

    VortexLoop1IsDownWind_ = 0;
    VortexLoop2IsDownWind_ = 0;
    
    VortexLoop1DownWindWeight_ = 0.;
    VortexLoop2DownWindWeight_ = 0.;

    Gamma_ = 0.;   

    ThicknessToChord_ = 0.;   
    
    LocationOfMaxThickness_ = 0.;   
    
    RadiusToChord_ = 0.;        
    
    CoreWidth_ = 0.;        
    
    MinCoreWidth_ = 0.;    
    
    WakeNode_ = 0;              
    
    KTFact_ = 1.;  
    
    Normal_[0] = Normal_[1] = Normal_[2] = 0.;
  
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
    
    VortexLoop1IsDownWind_ = VSPEdge.VortexLoop1IsDownWind_;
    VortexLoop2IsDownWind_ = VSPEdge.VortexLoop2IsDownWind_;
    
    VortexLoop1DownWindWeight_ = VSPEdge.VortexLoop1DownWindWeight_;
    VortexLoop2DownWindWeight_ = VSPEdge.VortexLoop2DownWindWeight_;
    
    // Vortex loop and edge data
    
    VortexLoop1_    = VSPEdge.VortexLoop1_;   
    VortexLoop2_    = VSPEdge.VortexLoop2_;   
    
    VortexEdge_     = VSPEdge.VortexEdge_;    
    
    // Edge type
    
    EdgeType_       = VSPEdge.EdgeType_;     
    IsTrailingEdge_ = VSPEdge.IsTrailingEdge_;
    IsLeadingEdge_  = VSPEdge.IsLeadingEdge_; 
    IsBoundaryEdge_ = VSPEdge.IsBoundaryEdge_;
    ComponentID_    = VSPEdge.ComponentID_;

    // Multi-Grid stuff
    
    CoarseGridEdge_     = VSPEdge.CoarseGridEdge_;     
    FineGridEdge_       = VSPEdge.FineGridEdge_;     
    Level_              = VSPEdge.Level_;     
    EdgeWasUsedForLoop_ = VSPEdge.EdgeWasUsedForLoop_;     
     
    // Surface type
    
    DegenWing_      = VSPEdge.DegenWing_;
    DegenBody_      = VSPEdge.DegenBody_;
    Cart3DSurface_  = VSPEdge.Cart3DSurface_;
    
    // XYZ of end points
    
    X1_ = VSPEdge.X1_;
    Y1_ = VSPEdge.Y1_;
    Z1_ = VSPEdge.Z1_;

    X2_ = VSPEdge.X2_;
    Y2_ = VSPEdge.Y2_;
    Z2_ = VSPEdge.Z2_;
    
    // Center of filament

    VortexCentroid_ = VSPEdge.VortexCentroid_;

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
    
    Sigma_ = VSPEdge.Sigma_;

    Length_ = VSPEdge.Length_;
    
    S_ = VSPEdge.S_;
    T_ = VSPEdge.T_;
    
    LocalSpacing_ = VSPEdge.LocalSpacing_;

    Mach_ = VSPEdge.Mach_;
    
    // Tolerances
    
    Tolerance_1_ = VSPEdge.Tolerance_1_;
    Tolerance_2_ = VSPEdge.Tolerance_2_;
    Tolerance_4_ = VSPEdge.Tolerance_4_;
    
    // Children

    Child1_ = VSPEdge.Child1_;
    Child2_ = VSPEdge.Child2_;
    
    // Edge coefs
    
    EdgeCoef_[0] = VSPEdge.EdgeCoef_[0];
    EdgeCoef_[1] = VSPEdge.EdgeCoef_[1];
    
    // Circulation strength
    
    Gamma_ = VSPEdge.Gamma_;
    
    // Airfoil information                                  
  
    ThicknessToChord_ = VSPEdge.ThicknessToChord_;
    
    LocationOfMaxThickness_ = VSPEdge.LocationOfMaxThickness_;
    
    RadiusToChord_ = VSPEdge.RadiusToChord_;    
    
    CoreWidth_ = VSPEdge.CoreWidth_;
    
    MinCoreWidth_ = VSPEdge.MinCoreWidth_;
    
    // KT correction
    
    KTFact_ = VSPEdge.KTFact_;

    // Normal vector
    
    Normal_[0] = VSPEdge.Normal_[0];
    Normal_[1] = VSPEdge.Normal_[1];
    Normal_[2] = VSPEdge.Normal_[2];
    
    // Forces
    
    for ( i = 0 ; i <= 2 ; i++ ) {
       
                    Forces_[i] = VSPEdge.Forces_[i];
               LocalForces_[i] = VSPEdge.LocalForces_[i];
            Trefftz_Forces_[i] = VSPEdge.Trefftz_Forces_[i];
           Unsteady_Forces_[i] = VSPEdge.Unsteady_Forces_[i];
             InducedForces_[i] = VSPEdge.InducedForces_[i];       
       Unsteady_LocalForces[i] = VSPEdge.Unsteady_LocalForces[i];
       
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

   // Nothing to do...

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
    
    // Get center of filament

    VortexCentroid().x() = XYZc_[0] = 0.5*( X1_ + X2_ );
    VortexCentroid().y() = XYZc_[1] = 0.5*( Y1_ + Y2_ );
    VortexCentroid().z() = XYZc_[2] = 0.5*( Z1_ + Z2_ );
    
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
    
    // Tolerances
    
    Tolerance_1_ = 1.e-7;
    Tolerance_2_ = Tolerance_1_ * Tolerance_1_;
    Tolerance_4_ = Tolerance_2_ * Tolerance_2_;

    // Zero out forces
    
    Forces_[0] = Forces_[1] = Forces_[2] = 0.;

}

/*##############################################################################
#                                                                              #
#                            VSP_EDGE SetMach                                  #
#                                                                              #
##############################################################################*/

void VSP_EDGE::SetMach(VSPAERO_DOUBLE Mach) {

#ifdef AUTODIFF

    Mach_ = Mach.value();

#else

    Mach_ = Mach;

#endif

    if ( Mach_ < 1. ) {
   
       Kappa_ = 2.;
   
    }
   
     else {
   
       Kappa_ = 1.;
   
    }
    
}

/*##############################################################################
#                                                                              #
#                          VSP_EDGE BoundVortex                                #
#                                                                              #
##############################################################################*/

void VSP_EDGE::InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]) {

    CoreWidth_ = 0.;
    
    NewBoundVortex(xyz_p, q);
    
}

/*##############################################################################
#                                                                              #
#                          VSP_EDGE BoundVortex                                #
#                                                                              #
##############################################################################*/

void VSP_EDGE::InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE CoreWidth) {

    // Offset distance for vortex core model
    // This is my implementation of Scully's model...
    // The core width comes from the calling routine... I assume
    // it knows what it's doing. This adjustment is meant to
    // stabilize vortex wake to wake and wake to body interactions.
        
    CoreWidth_ = CoreWidth;

    NewBoundVortex(xyz_p, q);
    
}

/*##############################################################################
#                                                                              #
#                          VSP_EDGE BoundVortex                                #
#                                                                              #
##############################################################################*/

void VSP_EDGE::NewBoundVortex(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3])
{

    int NoInfluence;
    VSPAERO_DOUBLE Xp, Yp, Zp;
    VSPAERO_DOUBLE U2, U4;
    VSPAERO_DOUBLE V2, V4;
    VSPAERO_DOUBLE W2, W4;
    VSPAERO_DOUBLE C_Gamma;
    VSPAERO_DOUBLE a, b, c, d, dx, dy, dz;
    VSPAERO_DOUBLE s1, s2, F, F1, F2;

    Beta2_ = 1. - SQR(KTFact_*Mach_);

    // Constants
    
    Xp = xyz_p[0];
    Yp = xyz_p[1];
    Zp = xyz_p[2];
    
    dx = X1_ - Xp;
    dy = Y1_ - Yp;
    dz = Z1_ - Zp;

    // Integral constants
    
    a = dx*dx + Beta2_*( dy*dy + dz*dz );    
    b = 2.*( u_*dx + Beta2_*( v_*dy + w_*dz ) );
    c = u_*u_ + Beta2_ * ( v_*v_ + w_*w_ );
    d = 4.*a*c - b*b;
 
    // Leading coefficient for velocity integrals
    
    C_Gamma = Gamma_ * Beta2_ / (2.*PI*Kappa_);
    
    // Determine integration limits
    
    NoInfluence = 0;
    
    s1 = 0.;
    s2 = 1.;
       
    if ( Mach_ > 1. ) {
     
       // Obvious case of no influence

       if ( Xp < X1_ && Xp < X2_ ) {
        
          NoInfluence = 1;
          
       }
       
    }

    if ( !NoInfluence ) {
     
       // F function evaluated at node 1

       F1 = 0.;

       if ( Mach_ < 1. || ( Xp >= X1_ && 0.99*SQR(X1_-Xp) + Beta2_*( SQR(Y1_-Yp) + SQR(Z1_-Zp) ) > 0. ) ) {

           F1 = Fint(a,b,c,d,s1);
       
       }

       // F function evaluated at node 2

       F2 = 0.;
       
       if ( Mach_ < 1. || ( Xp >= X2_ && 0.99*SQR(X2_-Xp) + Beta2_*( SQR(Y2_-Yp) + SQR(Z2_-Zp) ) > 0. ) ) {
      
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

}

/*##############################################################################
#                                                                              #
#                          VSP_EDGE BoundVortex                                #
#                                                                              #
##############################################################################*/

void VSP_EDGE::OldBoundVortex(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3])
{

    int NoInfluence;
    VSPAERO_DOUBLE Beta_2;
    VSPAERO_DOUBLE Xp, Yp, Zp;
    VSPAERO_DOUBLE Up, U1, U2, U3, U4;
    VSPAERO_DOUBLE Vp, V1, V2, V3, V4;
    VSPAERO_DOUBLE Wp, W1, W2, W3, W4;
    VSPAERO_DOUBLE Kappa, C_Gamma;
    VSPAERO_DOUBLE a, b, c, d, u, v, w, dx, dy, dz;
    VSPAERO_DOUBLE s1, s2, F, F1, F2, G, G1, G2, Arg1, Arg2, Eps;

    Eps = 0.99;
    
    Beta_2 = 1. - SQR(Mach_);

    if ( Beta_2 > 0. ) {

       Kappa = 2.;

    }

    else {

       Kappa = 1.;

    }

    // Direction vector of the edge 

    u = Vec_[0] * Length_;
    v = Vec_[1] * Length_;
    w = Vec_[2] * Length_;

    // Constants
    
    Xp = xyz_p[0];
    Yp = xyz_p[1];
    Zp = xyz_p[2];
    
    dx = X1_ - Xp;
    dy = Y1_ - Yp;
    dz = Z1_ - Zp;

    // Integral constants
    
    a = dx*dx + Beta_2*( dy*dy + dz*dz );    
    b = 2.*( u*dx + Beta_2*( v*dy + w*dz ) );
    c = u*u + Beta_2 * ( v*v + w*w );
    d = 4.*a*c - b*b;
 
    // Leading coefficient for velocity integrals
    
    C_Gamma = Beta_2 * Gamma_ / (2.*PI*Kappa);
    
    // Determine integration limits
    
    NoInfluence = 0;
    
    s1 = 0.;
    s2 = 1.;
       
    if ( Mach_ > 1. ) {
     
       // Obvious case of no influence

       if ( Xp < X1_ && Xp < X2_ ) {
        
          NoInfluence = 1;
          
       }
       
    }

    if ( !NoInfluence ) {
     
       // F and G functions evaluated at node 1
       
       Arg1 = SQR(X1_-Xp);
       
       Arg2 = Beta_2*( SQR(Y1_-Yp) + SQR(Z1_-Zp) );
       
       F1 = G1 = 0.;

       if ( Mach_ < 1. || ( Xp >= X1_ && Eps*Arg1 + Arg2 > 0. ) ) {

           F1 = Fint(a,b,c,d,s1);
           G1 = Gint(a,b,c,d,s1);
          
       }

       // F and G functions evaluated at node 2

       Arg1 = SQR(X2_-Xp);
     
       Arg2 = Beta_2*( SQR(Y2_-Yp) + SQR(Z2_-Zp) );
       
       F2 = G2 = 0.;
       
       if ( Mach_ < 1. || ( Xp >= X2_ && Eps*Arg1 + Arg2 > 0. ) ) {
      
           F2 = Fint(a,b,c,d,s2);
           G2 = Gint(a,b,c,d,s2);
          
  
       }
       
       // Evalulate integrals
       
       F = F2 - F1;
       G = G2 - G1;
       
       // U Velocity

       U1 =  v * w      * G;
       U2 =  v *     dz * F;
       U3 = -v * w      * G;
       U4 =     -w * dy * F;
       
       Up = -C_Gamma*( U1 + U2 + U3 + U4 );

       // V Velocity
       
       V1 =  u * w      * G;
       V2 =  u *     dz * F;
       V3 = -u * w      * G;
       V4 =     -w * dx * F;
       
       Vp = C_Gamma*( V1 + V2 + V3 + V4 );

       // W Velocity
       
       W1 =  u * v      * G;
       W2 =  u *     dy * F;
       W3 = -u * v      * G;
       W4 =     -v * dx * F;
       
       Wp = -C_Gamma*( W1 + W2 + W3 + W4 );
       
    }
    
    else {
     
       Up = Vp = Wp = 0.;
       
    }

    // Return velocities
    
    q[0] = Up;
    q[1] = Vp;
    q[2] = Wp;

}

/*##############################################################################
#                                                                              #
#                          VSP_EDGE Fint                                       #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE VSP_EDGE::Fint(VSPAERO_DOUBLE &a, VSPAERO_DOUBLE &b, VSPAERO_DOUBLE &c, VSPAERO_DOUBLE &d, VSPAERO_DOUBLE &s)
{
 
    VSPAERO_DOUBLE R, F, Denom;

    R = a + b*s + c*s*s;

    if ( ABS(d) < Tolerance_2_ || R < Tolerance_1_ ) return 0.;

    Denom = d * sqrt(R);

    F = 2.*(2.*c*s + b)*Denom/(Denom*Denom + CoreWidth_*CoreWidth_ + MinCoreWidth_*MinCoreWidth_);

    return F;
 
}

/*##############################################################################
#                                                                              #
#                          VSP_EDGE Gint                                       #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE VSP_EDGE::Gint(VSPAERO_DOUBLE &a, VSPAERO_DOUBLE &b, VSPAERO_DOUBLE &c, VSPAERO_DOUBLE &d, VSPAERO_DOUBLE &s)
{
   
    VSPAERO_DOUBLE R, G, Denom;
    
    R = a + b*s + c*s*s;

    if ( ABS(d) < Tolerance_2_ || R < Tolerance_1_ ) return 0.;

    Denom = d * sqrt(R);

    G = -2.*(2.*a+b*s)*Denom/(Denom*Denom + CoreWidth_*CoreWidth_ + MinCoreWidth_*MinCoreWidth_);
    
    return G;
 
} 

/*##############################################################################
#                                                                              #
#                     VSP_EDGE FindLineConicIntersection                       #
#                                                                              #
##############################################################################*/

void VSP_EDGE::FindLineConicIntersection(VSPAERO_DOUBLE &Xp, VSPAERO_DOUBLE &Yp, VSPAERO_DOUBLE &Zp,
                                         VSPAERO_DOUBLE &X1, VSPAERO_DOUBLE &Y1, VSPAERO_DOUBLE &Z1,
                                         VSPAERO_DOUBLE &Xd, VSPAERO_DOUBLE &Yd, VSPAERO_DOUBLE &Zd,
                                         VSPAERO_DOUBLE &t1, VSPAERO_DOUBLE &t2)
{
 
    int Solved;
    VSPAERO_DOUBLE Xi1, Xi2, Xi, Yi, Zi, Xo, Yo, Zo, Normal[3], Vec[3], Dot;
    VSPAERO_DOUBLE A, B, C, Aq, Bq, Cq;
    VSPAERO_DOUBLE Tol, Delta, v1, v2;
     
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
              
                t1 = t1;
                t2 = 1.;
          
             }
             
             else {
              
                t1 = 0.;
                t2 = t2;
      
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

VSPAERO_DOUBLE VSP_EDGE::GeneralizedPrincipalPartOfDownWash(void)
{
 
    VSPAERO_DOUBLE Beta_2, T, Ws, Mag, Theta, Arg;
    
    Beta_2 = 1. - SQR(Mach_);
    
    Mag = MAX(MIN(Vec_[0],1.),-1.);

    Theta = 0.5*PI - acos(Mag);
    
    T = tan(Theta);

    Arg = -Beta_2 - T*T;
  
    Ws = 0.;

    if ( Mach_ > 1. && Arg > 0. ) Ws = 0.50*sqrt(Arg)*cos(Theta)/LocalSpacing_;
    
//    Ws *= MAX(1.,pow(2./Mach_,2.));

    return Ws;
    
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

void VSP_EDGE::CalculateTrefftzForces(VSPAERO_DOUBLE WakeInducedVelocity[3])
{

    vector_cross(WakeInducedVelocity, Vec_, Trefftz_Forces_);
   
    Trefftz_Forces_[0] *= -Length_*Gamma_;
    Trefftz_Forces_[1] *= -Length_*Gamma_;
    Trefftz_Forces_[2] *= -Length_*Gamma_;

}

/*##############################################################################
#                                                                              #
#                      VSP_EDGE UpdateGeometryLocation                         #
#                                                                              #
##############################################################################*/

void VSP_EDGE::UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat)
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
    
    // Update centroid node

    VortexCentroid().x() = XYZc_[0];
    VortexCentroid().y() = XYZc_[1];
    VortexCentroid().z() = XYZc_[2];   
    
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
     
