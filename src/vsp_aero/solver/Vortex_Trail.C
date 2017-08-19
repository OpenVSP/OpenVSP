#include "Vortex_Trail.H"

/*##############################################################################
#                                                                              #
#                      Allocate space for statics                              #
#                                                                              #
##############################################################################*/

    double VORTEX_TRAIL::Mach_ = 0.;

/*##############################################################################
#                                                                              #
#                              VORTEX_TRAIL constructor                        #
#                                                                              #
##############################################################################*/

VORTEX_TRAIL::VORTEX_TRAIL(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                 VORTEX_TRAIL init                            #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::init(void)
{

    Mach_ = 0.;
  
    Verbose_ = 1;

    NumberOfLevels_ = 0;
        
    VortexEdgeList_ = NULL;
    
    VortexEdgeVelocity_ = NULL;
    
    NumberOfSubVortices_ = NULL;
    
    S_ = NULL;
    
    NodeList_ = NULL;
    
    Tolerance_ = 1.e-6;
    
    Evaluate_ = 0;
    
    TimeAccurate_ = 0;
    
    NumberofExactShiftPoints_ = 0;
    
    ConvectType_ = 0;
    
    TimeStep_ = 0.;
    
    Vinf_ = 0.;
    
    Gamma_ = NULL;
    
    GammaNew_ = NULL;
    
    GammaSave_ = NULL;

    MaxConvectedDistance_ = 1.e12;
            
}

/*##############################################################################
#                                                                              #
#                                VORTEX_TRAIL Copy                             #
#                                                                              #
##############################################################################*/

VORTEX_TRAIL::VORTEX_TRAIL(const VORTEX_TRAIL &Solver_)
{

    init();

    // Just * use the operator = code

    *this = Solver_;

}

/*##############################################################################
#                                                                              #
#                               VORTEX_TRAIL operator=                         #
#                                                                              #
##############################################################################*/

VORTEX_TRAIL& VORTEX_TRAIL::operator=(const VORTEX_TRAIL &Trailing_Vortex)
{

    int i, Level;
    
    // Verbose mode
    
    Verbose_ = Trailing_Vortex.Verbose_;
        
    // Wing and edge, or trailing node this vortex belongs to
    
    Wing_ = Trailing_Vortex.Wing_;
    
    Node_ = Trailing_Vortex.Node_;
    
    // List of trailing vortices
    
    NumberOfLevels_ = Trailing_Vortex.NumberOfLevels_;
    
    NumberOfSubVortices_ = new int[NumberOfLevels_ + 1];

    VortexEdgeList_ = new VSP_EDGE*[NumberOfLevels_ + 1];
                
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
     
       NumberOfSubVortices_[Level] = Trailing_Vortex.NumberOfSubVortices_[Level];

       VortexEdgeList_[Level] = new VSP_EDGE[NumberOfSubVortices_[Level] + 2];

    }
 
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
     
       for ( i = 1 ; i <= NumberOfSubVortices_[Level] + 1 ; i++ ) {
        
          VortexEdgeList_[Level][i] = Trailing_Vortex.VortexEdgeList_[Level][i];
          
       }

    }
    
    Gamma_     = new double[NumberOfSubVortices() + 5];
    
    GammaNew_  = new double[NumberOfSubVortices() + 5];
    
    GammaSave_ = new double[NumberOfSubVortices() + 5];    
  
    VortexEdgeVelocity_ = new double*[NumberOfSubVortices() + 3];

    for ( i = 1 ; i <= NumberOfSubVortices() + 2 ; i++ ) {
     
      VortexEdgeVelocity_[i] = new double[6];
      
      VortexEdgeVelocity_[i][0] = Trailing_Vortex.VortexEdgeVelocity_[i][0];
      VortexEdgeVelocity_[i][1] = Trailing_Vortex.VortexEdgeVelocity_[i][1];
      VortexEdgeVelocity_[i][2] = Trailing_Vortex.VortexEdgeVelocity_[i][2];
     
    }    

    FreeStreamVelocity_[0] = Trailing_Vortex.FreeStreamVelocity_[0];
    FreeStreamVelocity_[1] = Trailing_Vortex.FreeStreamVelocity_[1];
    FreeStreamVelocity_[2] = Trailing_Vortex.FreeStreamVelocity_[2];

    Mach_ = Trailing_Vortex.Mach_;
    
    Sigma_ = Trailing_Vortex.Sigma_;
    
    Tolerance_ = Trailing_Vortex.Tolerance_;
    
    TEVec_[0] = Trailing_Vortex.TEVec_[0];
    TEVec_[1] = Trailing_Vortex.TEVec_[1];
    TEVec_[2] = Trailing_Vortex.TEVec_[2];

    for ( i = 1 ; i <= NumberOfSubVortices() + 4 ; i++ ) {
       
       Gamma_[i]     = Trailing_Vortex.Gamma_[i];
       GammaNew_[i]  = Trailing_Vortex.GammaNew_[i];
       GammaSave_[i] = Trailing_Vortex.GammaSave_[i];
       
    }
    
    NumberOfNodes_ = Trailing_Vortex.NumberOfNodes_;

    Length_ = Trailing_Vortex.Length_;
    
    FarDist_ = Trailing_Vortex.FarDist_;

    TE_Node_ = Trailing_Vortex.TE_Node_;
    
    S_ = new double[NumberOfSubVortices() + 3];
 
    NodeList_ = new VSP_NODE[NumberOfSubVortices() + 3];
    
    for ( i = 1 ; i <= NumberOfSubVortices() + 2 ; i++ ) {
     
       S_[i] = Trailing_Vortex.S_[i];
    
       NodeList_[i] = Trailing_Vortex.NodeList_[i];
       
    }
    
    Evaluate_ = Trailing_Vortex.Evaluate_;
    
    NumberofExactShiftPoints_ = Trailing_Vortex.NumberofExactShiftPoints_;

    ConvectType_ = Trailing_Vortex.ConvectType_;

    TimeAccurate_ = Trailing_Vortex.TimeAccurate_;
    
    TimeStep_ = Trailing_Vortex.TimeStep_;
    
    Vinf_ = Trailing_Vortex.Vinf_;
    
    return *this;

}

/*##############################################################################
#                                                                              #
#                            VORTEX_TRAIL destructor                           #
#                                                                              #
##############################################################################*/

VORTEX_TRAIL::~VORTEX_TRAIL(void)
{

    int i, Level;

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       if ( VortexEdgeList_[Level] != NULL ) delete [] VortexEdgeList_[Level];
    
       VortexEdgeList_[Level] = NULL;
 
    }

    if ( VortexEdgeList_ != NULL ) delete [] VortexEdgeList_;

    if ( Gamma_          != NULL ) delete [] Gamma_;

    if ( GammaNew_       != NULL ) delete [] GammaNew_;

    if ( GammaSave_      != NULL ) delete [] GammaSave_;

    if ( NumberOfSubVortices_ != NULL ) { 
       
       if ( NumberOfSubVortices() != 0 ) {
          
          for ( i = 1 ; i <= NumberOfSubVortices() + 2 ; i++ ) {
        
            if ( VortexEdgeVelocity_[i] != NULL ) delete [] VortexEdgeVelocity_[i];
      
          }    
          
       }
       
    }

    if ( VortexEdgeVelocity_ != NULL ) delete [] VortexEdgeVelocity_;

    if ( S_ != NULL ) delete [] S_;

    if ( NodeList_ != NULL ) delete [] NodeList_;

}

/*##############################################################################
#                                                                              #
#                              VORTEX_TRAIL Setup                              #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::Setup(int NumSubVortices, double FarDist, VSP_NODE &Node1, VSP_NODE &Node2)
{

    int i, j, k, m, Level, Done;
    double Vec[3], Theta, DTheta, Ds, Eps;
    VSP_NODE NodeA, NodeB;
    
    FarDist_ = FarDist;   
 
    k = 1;
    
    Done = 0;
    
    NumberOfLevels_ = 1;
    
    while ( !Done ) {
     
       k *= 2;

       if ( k >= NumSubVortices ) {
        
          Done = 1;
          
       }
       
       else {
       
          NumberOfLevels_++;
          
       }
       
    }
    
    if ( k != NumSubVortices ) {
    
       printf("Number of trailing wake nodes must be a power of 2! \n"); fflush(NULL);
       printf("NumberOfLevels_: %d \n",NumberOfLevels_);
       printf("2^NumberOfLevels_: %f \n",pow((double)2,NumberOfLevels_));
        
       exit(1);
       
    }
    
    NumberOfSubVortices_ = new int[NumberOfLevels_ + 1];

    VortexEdgeList_ = new VSP_EDGE*[NumberOfLevels_ + 1];
      
    k = NumSubVortices;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       NumberOfSubVortices_[Level] = k;
  
       VortexEdgeList_[Level] = new VSP_EDGE[k + 2];
       
       k /= 2;
       
    }
    
    VortexEdgeVelocity_ = new double*[NumberOfSubVortices() + 3];
    
    S_ = new double[NumberOfSubVortices() + 3];
    
    NodeList_ = new VSP_NODE[NumberOfSubVortices() + 3];
    
    for ( i = 1 ; i <= NumberOfSubVortices() + 2 ; i++ ) {
     
       VortexEdgeVelocity_[i] = new double[6];
     
    }
    
    // Calculate length of trailing vortex, and unit vector in it's direction
    
    TE_Node_.x() = Node1.x();
    TE_Node_.y() = Node1.y();
    TE_Node_.z() = Node1.z();
    
    // Node 2 is the far down stream point... basically 'infinity'
    
    Vec[0] = Node2.x() - Node1.x();  
    Vec[1] = Node2.y() - Node1.y();
    Vec[2] = Node2.z() - Node1.z();

    Length_ = sqrt(vector_dot(Vec,Vec));
    
    Vec[0] /= Length_;
    Vec[1] /= Length_;
    Vec[2] /= Length_;
    
    // Create node spacing out along vortex line
   
    NumberOfNodes_ = NumberOfSubVortices() + 2;

    DTheta = 0.5*PI / NumberOfSubVortices();

    for ( i = 1 ; i <= NumberOfNodes_ - 1 ; i++ ) {
     
       Theta = (i-1)*DTheta;
       
       S_[i] = FarDist*(1.-cos(Theta));

    }

    S_[NumberOfNodes_] = Length_ - FarDist;

    // Modify the trailing wake spacing for time accurate analysis... 

    if ( TimeAccurate_ ) {
       
       for ( j = 2 ; j <= NumberOfNodes_ ; j++ ) {
          
          Ds = S_[j] - S_[j-1];

          if ( Ds < TimeStep_ * Vinf_ ) NumberofExactShiftPoints_ = j;
          
       }
       
       NumberofExactShiftPoints_ = MAX(32, NumberofExactShiftPoints_);

       for ( j = 2 ; j <= NumberofExactShiftPoints_ ; j++ ) {
                 
          Eps = (j-1) * TimeStep_ * Vinf_ - S_[j];
          
          for ( i = j ; i <= NumberOfNodes_ ; i++ ) {
           
             S_[i] += Eps;
      
          }   
          
       }    
       
    }    
    
    for ( i = 1 ; i <= NumberOfNodes_  ; i++ ) {

       NodeList_[i].x() = TE_Node_.x() + Vec[0]*S_[i];
       NodeList_[i].y() = TE_Node_.y() + Vec[1]*S_[i];
       NodeList_[i].z() = TE_Node_.z() + Vec[2]*S_[i];
       
    }    

    // Now merge vortices to create course grids
        
    m = 1;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       j = 0;

       k = 1;
       
       for ( i = 1 ; i <= NumberOfSubVortices() ; i+=m ) {
        
          j++;
          
          NodeA.x() = NodeList_[i  ].x();
          NodeA.y() = NodeList_[i  ].y();
          NodeA.z() = NodeList_[i  ].z();
          
          NodeB.x() = NodeList_[i+m].x();
          NodeB.y() = NodeList_[i+m].y();
          NodeB.z() = NodeList_[i+m].z();
          
          VortexEdgeList(Level)[j].DegenWing() = Wing_;
          
          VortexEdgeList(Level)[j].Node() = Node_;
          
          VortexEdgeList(Level)[j].IsTrailingEdge() = 0;
          
          VortexEdgeList(Level)[j].Sigma() = Sigma_;
          
          VortexEdgeList(Level)[j].S() = S_[i];
      
          VortexEdgeList(Level)[j].Setup(NodeA, NodeB);
   
          if ( Level > 1 ) {

             VortexEdgeList(Level)[j].SetupChildren(VortexEdgeList_[Level-1][k  ],
                                                    VortexEdgeList_[Level-1][k+1]);
  
          }
          
          k += 2;
          
       }

       // Final vortex edge trails off to infinity...
       
       NodeA.x() = NodeList_[NumberOfSubVortices()+1].x();
       NodeA.y() = NodeList_[NumberOfSubVortices()+1].y();
       NodeA.z() = NodeList_[NumberOfSubVortices()+1].z();
       
       NodeB.x() = NodeList_[NumberOfSubVortices()+2].x();
       NodeB.y() = NodeList_[NumberOfSubVortices()+2].y();
       NodeB.z() = NodeList_[NumberOfSubVortices()+2].z();
     
       VortexEdgeList(Level)[j+1].DegenWing() = Wing_;
       
       VortexEdgeList(Level)[j+1].Node() = Node_;
       
       VortexEdgeList(Level)[j+1].IsTrailingEdge() = 0;
       
       VortexEdgeList(Level)[j+1].Sigma() = Sigma_;
       
       VortexEdgeList(Level)[j+1].Setup(NodeA, NodeB);
          
       m *= 2;
       
    }     
    
    // Gamma arrays
    
    if (     Gamma_ != NULL ) delete [] Gamma_;
    
    if (  GammaNew_ != NULL ) delete [] GammaNew_;
    
    if ( GammaSave_ != NULL ) delete [] GammaSave_ ;
        
    Gamma_     = new double[NumberOfSubVortices() + 5];
    
    GammaNew_  = new double[NumberOfSubVortices() + 5];
    
    GammaSave_ = new double[NumberOfSubVortices() + 5];
    
    zero_double_array(Gamma_,     NumberOfSubVortices() + 4);
    
    zero_double_array(GammaNew_,  NumberOfSubVortices() + 4);
    
    zero_double_array(GammaSave_, NumberOfSubVortices() + 4);

}

/*##############################################################################
#                                                                              #
#                         VORTEX_TRAIL SimpleVortex                            #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::InducedVelocity(double xyz_p[3], double q[3])
{
 
   int i, Level;
   double dq[3], Fact;
   double Vec1[3], Vec2[3], Radius;

   // Update the vortex strengths for all of the sub vortex elements
   // This has to be here... since the wake agglomeration routine, 
   // at the vortex sheet level, may change gamma
   
   UpdateGamma();
 
   // Start at the coarsest level
      
   Level = NumberOfLevels_;
   
   // if ( Mach_ >= 1. ) Level = 1;

   q[0] = q[1] = q[2] = 0.;
  
   for ( i = 1 ; i <= NumberOfSubVortices(Level) ; i++ ) {
  
      dq[0] = dq[1] = dq[2] = 0.;
      
      CalculateVelocityForSubVortex(VortexEdgeList(Level)[i], xyz_p, dq);
      
      q[0] += dq[0];
      q[1] += dq[1];
      q[2] += dq[2];
     
   }

   // Add in final vortex that goes off to infinity...
   
   Level = 1;
   
   i = NumberOfSubVortices() + 1;

   Vec1[0] = xyz_p[0] - VortexEdgeList(Level)[i].X1();
   Vec1[1] = xyz_p[1] - VortexEdgeList(Level)[i].Y1();
   Vec1[2] = xyz_p[2] - VortexEdgeList(Level)[i].Z1();

   vector_cross(VortexEdgeList(Level)[i].Vec(), Vec1, Vec2);
   
   Radius = sqrt(vector_dot(Vec2,Vec2));
    
   VortexEdgeList(Level)[i].InducedVelocity(xyz_p, dq);

   Fact = MIN(Radius/pow(VortexEdgeList(Level)[i].Sigma(),2.),1.);

   q[0] += Fact*dq[0];
   q[1] += Fact*dq[1];
   q[2] += Fact*dq[2];

   // Uncomment below to make things 2D
   //q[0] = q[1] = q[2] = 0.;
 
}

/*##############################################################################
#                                                                              #
#                 VORTEX_TRAIL CalculateVelocityForSubVortex                   #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::CalculateVelocityForSubVortex(VSP_EDGE &VortexEdge, double xyz_p[3], double q[3])
{
 
   double Vec[3], dq[3], Dot, Dist, Radius, Fact, Ratio, FarAway;

   FarAway = 3.;
   
   Dist = sqrt( SQR(VortexEdge.Xc() - xyz_p[0]) 
              + SQR(VortexEdge.Yc() - xyz_p[1]) 
              + SQR(VortexEdge.Zc() - xyz_p[2]) );
                 
   Ratio = Dist / VortexEdge.Length();
  
   // If faraway, evalulate at this level
 
   if ( !VortexEdge.ThereAreChildren() || Ratio >= FarAway ) {
    
      Vec[0] = xyz_p[0] - VortexEdge.X1();
      Vec[1] = xyz_p[1] - VortexEdge.Y1();
      Vec[2] = xyz_p[2] - VortexEdge.Z1();
   
      Dot = vector_dot(Vec, VortexEdge.Vec());
      
      Vec[0] -= Dot * VortexEdge.Vec()[0];
      Vec[1] -= Dot * VortexEdge.Vec()[1];
      Vec[2] -= Dot * VortexEdge.Vec()[2];
      
      Radius = sqrt(vector_dot(Vec,Vec));
      
      VortexEdge.InducedVelocity(xyz_p, dq);

      Fact = MIN(Radius/pow(VortexEdge.Sigma(),2.),1.);

      q[0] += Fact*dq[0];
      q[1] += Fact*dq[1];
      q[2] += Fact*dq[2];                 
        
   }
   
   // Otherwise, move up a level and evaluate things with the 2 children
   
   else {

      CalculateVelocityForSubVortex(VortexEdge.Child1(), xyz_p, q);
      
      CalculateVelocityForSubVortex(VortexEdge.Child2(), xyz_p, q);
   
   }
 
}

/*##############################################################################
#                                                                              #
#                        VORTEX_TRAIL UpdateGamma                              #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::UpdateGamma(void)
{
 
   int i, Level;
   double time1, time2;

   if ( !TimeAccurate_ ) {

      for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
       
         for ( i = 1 ; i <= NumberOfSubVortices(Level) + 1 ; i++ ) {
             
            // Initialize each subvortex

            VortexEdgeList(Level)[i].Gamma() = Gamma_[0];

            VortexEdgeList(Level)[i].Mach() = Mach_;

         }
         
      }
      
   }
   
   else {
      
      // Initialize the first level
      
      Level = 1;
        
      for ( i = 1 ; i <= NumberOfSubVortices(Level) + 1 ; i++ ) {

         VortexEdgeList(Level)[i].Gamma() = Gamma_[i];

         VortexEdgeList(Level)[i].Mach() = Mach_;
            
      }
                  
      for ( Level = 2 ; Level <= NumberOfLevels_ ; Level++ ) {
       
         for ( i = 1 ; i <= NumberOfSubVortices(Level) ; i++ ) {

            // Initialize each subvortex and coarser level... agglomerate the vorticity
 
            VortexEdgeList(Level)[i].Gamma() = 0.5*(VortexEdgeList(Level)[i].Child1().Gamma() + VortexEdgeList(Level)[i].Child2().Gamma());
        
            VortexEdgeList(Level)[i].Mach() = Mach_;

         }

         VortexEdgeList(Level)[NumberOfSubVortices(Level) + 1].Gamma() = Gamma_[NumberOfSubVortices(1) + 1];
     
         VortexEdgeList(Level)[NumberOfSubVortices(Level) + 1].Mach() = Mach_;

      }      
      
   }

}
  
/*##############################################################################
#                                                                              #
#                        VORTEX_TRAIL UpdateLocation                           #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::UpdateLocation(void)
{
 
    int i, j, m, Level;
    double *U, *V, *W, Vec[3], Mag, dx, dy, dz, dS;
    VSP_NODE NodeA, NodeB, NodeTemp;
   
    //  velocities to be monotonic in nature
    
    U = new double[NumberOfSubVortices() + 2];
    V = new double[NumberOfSubVortices() + 2];
    W = new double[NumberOfSubVortices() + 2];
   
    for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
     
        LimitVelocity(VortexEdgeVelocity(i));

        U[i] = VortexEdgeVelocity(i)[0];
        V[i] = VortexEdgeVelocity(i)[1];
        W[i] = VortexEdgeVelocity(i)[2];
        
        if ( S_[i] <= 0.25*Sigma_  ) {
         
           U[i] = TEVec_[0];
           V[i] = TEVec_[1];
           W[i] = TEVec_[2];
           
        }

    }
 
    NodeList_[1].x() = TE_Node_.x();
    NodeList_[1].y() = TE_Node_.y(); 
    NodeList_[1].z() = TE_Node_.z();

    for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {

        Vec[0] = U[i];
        Vec[1] = V[i];
        Vec[2] = W[i];
        
        Mag = sqrt(vector_dot(Vec,Vec));

        Vec[0] /= Mag;
        Vec[1] /= Mag;
        Vec[2] /= Mag;
        
        dS = S_[i+1] - S_[i];

        dx = NodeList_[i].x() + Vec[0]*dS - NodeList_[i+1].x();
        dy = NodeList_[i].y() + Vec[1]*dS - NodeList_[i+1].y();
        dz = NodeList_[i].z() + Vec[2]*dS - NodeList_[i+1].z();

        NodeList_[i+1].x() += 0.5*dx;
        NodeList_[i+1].y() += 0.5*dy;
        NodeList_[i+1].z() += 0.5*dz;

    }
    
    // Smooth (x,y,z)
    
    Smooth(); 
    
    // Update trailing vortex shape
    
    m = 1;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       j = 0;

       for ( i = 1 ; i <= NumberOfSubVortices() ; i+=m ) {
        
          j++;
          
          NodeA.x() = NodeList_[i  ].x();
          NodeA.y() = NodeList_[i  ].y();
          NodeA.z() = NodeList_[i  ].z();
          
          NodeB.x() = NodeList_[i+m].x();
          NodeB.y() = NodeList_[i+m].y();
          NodeB.z() = NodeList_[i+m].z();
                    
          VortexEdgeList(Level)[j].Setup(NodeA, NodeB);          
          
       }

       // Final vortex edge trails off to infinity...
       
       NodeA.x() = NodeList_[NumberOfSubVortices()+1].x();
       NodeA.y() = NodeList_[NumberOfSubVortices()+1].y();
       NodeA.z() = NodeList_[NumberOfSubVortices()+1].z();
       
       NodeB.x() = NodeList_[NumberOfSubVortices()+2].x();
       NodeB.y() = NodeList_[NumberOfSubVortices()+2].y();
       NodeB.z() = NodeList_[NumberOfSubVortices()+2].z();

       VortexEdgeList(Level)[j+1].Setup(NodeA, NodeB);
          
       m *= 2;
       
    }
      
    delete [] U;
    delete [] V;
    delete [] W;

}

/*##############################################################################
#                                                                              #
#                        VORTEX_TRAIL SaveVortexState                          #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::SaveVortexState(void)
{
   
    int i;
    
    for ( i = 0 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
       
       GammaSave_[i] = Gamma_[i];
       
    }    
    
}

/*##############################################################################
#                                                                              #
#                        VORTEX_TRAIL ConvectWakeVorticity                     #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::ConvectWakeVorticity(int ConvectType)
{
 
    int i;
    double dS, CFL, Fact;

    // Set convection type
    
    ConvectType_ = ConvectType;
  
    // Convect vorticity down stream along streamlines
    
    zero_double_array(GammaNew_, NumberOfSubVortices() + 2);
    
    // Just convecting the implicit portion of the wake
    
    if ( ConvectType_ == IMPLICIT_WAKE_GAMMAS ) {
       
       // Zero out everything except trailing edge ( j = 0) vorticity
             
       for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
       
          Gamma_[i] = 0.;
       
       }
               
    }
    
    // Just convecting the explicit portion of the wake
    
    else if ( ConvectType_ == EXPLICIT_WAKE_GAMMAS ) {
  
       for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
       
          Gamma_[i] = GammaSave_[i];
       
       }
       
       Gamma_[0] = 0.;
                            
    }
    
    // Convect and use the entire trailing wake
    
    else if ( ConvectType_ == ALL_WAKE_GAMMAS ) {

       for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
       
          Gamma_[i] = GammaSave_[i];
       
       }

    }
    
    else {
       
       printf("Unknown convection option! \n");
       
       exit(1);
       
    }
   
    // Exact shift
        
    for ( i = 1 ; i <= NumberofExactShiftPoints_ ; i++ ) {
       
       GammaNew_[i] = Gamma_[i-1];
       
    }
   
    // Integrate rest of wake

    for ( i = NumberofExactShiftPoints_ + 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {

        dS = S_[i] - S_[i-1];

        CFL = TimeStep_ / dS;
        
        // First order upwind
        
        if (1|| i < 2 ) {
        
           GammaNew_[i] = (Gamma_[i] + CFL * GammaNew_[i-1])/(1. + CFL);
           
        }
        
        // Second order upwind
        
        else {
           
           GammaNew_[i] = (Gamma_[i] + 0.5*CFL * (4.*GammaNew_[i-1] - GammaNew_[i-2]))/(1. + 1.5*CFL);

        }

    }

    // Update gamma
    
    for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
       
       Gamma_[i] = GammaNew_[i];

    }
    
    // Don't let errors from upwind scheme propagate further than
    // physically possible
          
    for ( i = 2 ; i <= NumberOfSubVortices() + 1 ; i++ ) {

       if ( S_[i] > MaxConvectedDistance_ ) Gamma_[i] = 0.;

    }    
    
    // Do a weighted update for final trailing vortex
   
    i = NumberOfSubVortices() + 1;
    
    if ( S_[i] <= MaxConvectedDistance_ ) {
       
       Fact = ( MaxConvectedDistance_ - S_[i] ) / ( S_[i+1] - S_[i] );
       
      Gamma_[i] = Fact * GammaNew_[i];
       
    }

}

/*##############################################################################
#                                                                              #
#                        VORTEX_TRAIL ConvectWakeVorticity                     #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::ConvectWakeVorticityOld(int ConvectType)
{
 
    int i;
    double dS, CFL, Temp, Fact;
  
    // Convect vorticity down stream along streamlines
    
    zero_double_array(GammaNew_, NumberOfSubVortices() + 2);
    
    // Just convecting the implicit portion of the wake
    
    if ( ConvectType == IMPLICIT_WAKE_GAMMAS ) {
       
       Temp = Gamma_[1];
      
       for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
       
          Gamma_[i] = 0.;
       
       }
       
       Gamma_[0] = Temp;
           
    }
    
    // Just convecting the explicit portion of the wake
    
    else if ( ConvectType == EXPLICIT_WAKE_GAMMAS ) {
  
       for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
       
          Gamma_[i] = GammaSave_[i];
       
       }
       
       Gamma_[0] = 0.;
                     
    }
    
    // Convect and use the entire trailing wake
    
    else if ( ConvectType == ALL_WAKE_GAMMAS ) {
       
       Temp = Gamma_[1];
              
       for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
       
          Gamma_[i] = GammaSave_[i];
       
       }
       
       Gamma_[0] = Temp;
       
    }
    
    else {
       
       printf("Unknown convection option! \n");
       
       exit(1);
       
    }
    
    // Exact shift
    
    GammaNew_[1] = Gamma_[0];
    
    for ( i = 1 ; i <= NumberofExactShiftPoints_ ; i++ ) {
       
       GammaNew_[i] = Gamma_[i-1];
       
    }
    
    // Integrate rest of wake
    
    for ( i = NumberofExactShiftPoints_ + 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {

        dS = S_[i] - S_[i-1];

        CFL = TimeStep_ / dS;
        
        // First order upwind
        
        if ( i < 2 ) {
        
           GammaNew_[i] = (Gamma_[i] + CFL * GammaNew_[i-1])/(1. + CFL);
           
        }
        
        // Second order upwind
        
        else {
           
           GammaNew_[i] = (Gamma_[i] + 0.5*CFL * (4.*GammaNew_[i-1] - GammaNew_[i-2]))/(1. + 1.5*CFL);

        }

    }

    // Update gamma
    
    for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
       
       Gamma_[i] = GammaNew_[i];
       
       // Don't let errors from upwind scheme propagate further than
       // physically possible
       
       if ( S_[i] > MaxConvectedDistance_ ) Gamma_[i] = 0.;

    }
    
    // Do a weighted update for final trailing vortex
   
    i = NumberOfSubVortices() + 1;
    
    if ( S_[i] <= MaxConvectedDistance_ ) {
       
       Fact = ( S_[i] - MaxConvectedDistance_ ) / ( S_[i+1] - S_[i] );
       
       Gamma_[i] = Fact * GammaNew_[i];
       
    }

}

/*##############################################################################
#                                                                              #
#                        VORTEX_TRAIL LimitVelocity                            #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::LimitVelocity(double q[3])
{
 
   double Dot, Eps, Vec1[3];
   
   // Take vector dot of local velocity and q

   Dot = vector_dot(FreeStreamVelocity_, q) / vector_dot(q, q);

   if ( Dot < 0. ) {
    
      Eps = 0.;
      
      Dot = 1.;
      
   }
   
   else {
    
      Eps = 1.;
      
   }
   
   Vec1[0] = q[0] - Dot*FreeStreamVelocity_[0];
   Vec1[1] = q[1] - Dot*FreeStreamVelocity_[1];
   Vec1[2] = q[2] - Dot*FreeStreamVelocity_[2];
   
   q[0] = Dot*FreeStreamVelocity_[0] + Eps * Vec1[0];
   q[1] = Dot*FreeStreamVelocity_[1] + Eps * Vec1[1];
   q[2] = Dot*FreeStreamVelocity_[2] + Eps * Vec1[2];
   
}

/*##############################################################################
#                                                                              #
#                        VORTEX_TRAIL SmoothVelocity                           #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::SmoothVelocity(double *Velocity)
{

    double *a, *b, *c, *d, *r, Eps, Ds;
    int i, NumberOfNodes;

    // Allocate space for the tridiagonal array

    NumberOfNodes = NumberOfSubVortices();
    
    a = new double[NumberOfNodes + 1];
    b = new double[NumberOfNodes + 1];
    c = new double[NumberOfNodes + 1];
    d = new double[NumberOfNodes + 1];
    r = new double[NumberOfNodes + 1];
    
    // Loop over x,y, and then z values
    
    for ( i = 1 ; i <= NumberOfNodes ; i++ ) {
     
       r[i] =  Velocity[i];
  
    }
 
    // BC at trailing edge
    
    a[1] = 0.;
    b[1] = 1.;
    c[1] = 0.;
    d[1] = r[1];
    
    // BC at far wake

    a[NumberOfNodes] =  0.;
    b[NumberOfNodes] =  1.;
    c[NumberOfNodes] =  0.;
    d[NumberOfNodes] =  r[NumberOfNodes];   

    // General node... implicity smoothing

    for ( i = 2 ; i < NumberOfNodes ; i++ ) {

       Ds = sqrt( pow(NodeList_[i].x() - NodeList_[i-1].x(),2.)
                + pow(NodeList_[i].y() - NodeList_[i-1].y(),2.)
                + pow(NodeList_[i].z() - NodeList_[i-1].z(),2.) );
               
       Eps = Ds;

       a[i] = -0.5*Eps/(Ds*Ds);
       b[i] = 1. + Eps/(Ds*Ds);
       c[i] = -0.5*Eps/(Ds*Ds);
       d[i] = r[i];
       
       if ( S_[i] <= 0.25*Sigma_ ) {
          
          a[i] = 0.;
          b[i] = 1.;
          c[i] = 0.;
          
       }
         
    }

    // Forward elimination

    for ( i = 2 ; i <= NumberOfNodes ; i++ ) {

       b[i] = b[i] - c[i-1]*a[i]/b[i-1];
       d[i] = d[i] - d[i-1]*a[i]/b[i-1];

    }

    // Backwards substitution

    r[NumberOfNodes] = d[NumberOfNodes]/b[NumberOfNodes];

    for ( i = NumberOfNodes - 1 ; i >= 1 ; i-- ) {

       r[i] = ( d[i] - c[i]*r[i+1] )/b[i];

    }
    
    // Update solution
    
    for ( i = 1 ; i <= NumberOfNodes ; i++ ) {
     
      Velocity[i] = r[i];

    }

    // Free up space

    delete [] a;
    delete [] b;
    delete [] c;
    delete [] d;
    delete [] r;

}

/*##############################################################################
#                                                                              #
#                        VORTEX_TRAIL Smooth                                   #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::Smooth(void)
{

    double *a, *b, *c, *d, *r, Eps, Ds;
     int i, Case, NumberOfNodes;

    // Allocate space for the tridiagonal array

    NumberOfNodes = NumberOfSubVortices();
    
    a = new double[NumberOfNodes + 1];
    b = new double[NumberOfNodes + 1];
    c = new double[NumberOfNodes + 1];
    d = new double[NumberOfNodes + 1];
    r = new double[NumberOfNodes + 1];
    
    // Loop over x,y, and then z values
    
    for ( Case = 1 ; Case <= 3 ; Case++ ) {

       for ( i = 1 ; i <= NumberOfNodes ; i++ ) {
        
          if ( Case == 1 ) r[i] =  NodeList_[i].x();
          if ( Case == 2 ) r[i] =  NodeList_[i].y();
          if ( Case == 3 ) r[i] =  NodeList_[i].z();
          
       }
    
       // BC at trailing edge
       
       a[1] = 0.;
       b[1] = 1.;
       c[1] = 0.;
       d[1] = r[1];
       
       // BC at far wake

       a[NumberOfNodes] =  0.;
       b[NumberOfNodes] =  1.;
       c[NumberOfNodes] =  0.;
       d[NumberOfNodes] =  r[NumberOfNodes];   

       // General node... implicity smoothing

       for ( i = 2 ; i < NumberOfNodes ; i++ ) {

          Ds = sqrt( pow(NodeList_[i].x() - NodeList_[i-1].x(),2.)
                   + pow(NodeList_[i].y() - NodeList_[i-1].y(),2.)
                   + pow(NodeList_[i].z() - NodeList_[i-1].z(),2.) );
                  
          Eps = 1.00*Ds;
                   
          a[i] = -0.5*Eps/(Ds*Ds);
          b[i] = 1. + Eps/(Ds*Ds);
          c[i] = -0.5*Eps/(Ds*Ds);
          d[i] = r[i];

       }

       // Forward elimination

       for ( i = 2 ; i <= NumberOfNodes ; i++ ) {

          b[i] = b[i] - c[i-1]*a[i]/b[i-1];
          d[i] = d[i] - d[i-1]*a[i]/b[i-1];

       }

       // Backwards substitution

       r[NumberOfNodes] = d[NumberOfNodes]/b[NumberOfNodes];

       for ( i = NumberOfNodes - 1 ; i >= 1 ; i-- ) {

          r[i] = ( d[i] - c[i]*r[i+1] )/b[i];

       }
       
       // Update solution
       
       for ( i = 1 ; i <= NumberOfNodes ; i++ ) {
        
         if ( Case == 1 ) NodeList_[i].x() = r[i];
         if ( Case == 2 ) NodeList_[i].y() = r[i];
         if ( Case == 3 ) NodeList_[i].z() = r[i];
         
        }
        
    }

    // Free up space

    delete [] a;
    delete [] b;
    delete [] c;
    delete [] d;
    delete [] r;
    
}

/*##############################################################################
#                                                                              #
#                          VORTEX_TRAIL WriteToFile                            #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::WriteToFile(FILE *adb_file)
{
 
    int i, n, Done, i_size, c_size, f_size;
    float x, y, z;
    VSP_NODE NodeA, NodeB;
    
    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
     
    // Write out trailing wake, up to, but not including portion that trails
    // off to infinity...
    
    NodeA.x() = TE_Node_.x();
    NodeA.y() = TE_Node_.y();
    NodeA.z() = TE_Node_.z();
    
    x = NodeList_[1].x();
    y = NodeList_[1].y();
    z = NodeList_[1].z();
    
    n = NumberOfSubVortices() + 2;
    
    Done = 0;
    
    if ( TimeAccurate_ ) {
       
       i = 1;
       
       while ( i <= NumberOfSubVortices() + 1 && !Done ) {
          
          if ( S_[i] > MaxConvectedDistance_ ) Done = 1;
          
          i++;
          
       }
       
       n = i;
       
    }
     
    fwrite(&(n), i_size, 1, adb_file);
    
    fwrite(&(x), f_size, 1, adb_file);
    fwrite(&(y), f_size, 1, adb_file);
    fwrite(&(z), f_size, 1, adb_file);

    for ( i = 1 ; i <= n - 1 ; i++ ) {

       x = NodeList_[i+1].x();
       y = NodeList_[i+1].y();
       z = NodeList_[i+1].z();
    
       fwrite(&(x), f_size, 1, adb_file);
       fwrite(&(y), f_size, 1, adb_file);
       fwrite(&(z), f_size, 1, adb_file);

    }

}


