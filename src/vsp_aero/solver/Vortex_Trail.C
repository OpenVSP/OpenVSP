//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

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
    
    VortexEdgeList_ = NULL;
    
    VortexEdgeVelocity_ = NULL;
    
    S_ = NULL;
    
    NodeList_ = NULL;
    
    Tolerance_ = 1.e-6;

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
  
    VortexEdgeVelocity_ = new double*[NumberOfSubVortices() + 3];

    for ( i = 1 ; i <= NumberOfSubVortices() + 2 ; i++ ) {
     
      VortexEdgeVelocity_[i] = new double[3];
      
      VortexEdgeVelocity_[i][0] = Trailing_Vortex.VortexEdgeVelocity_[i][0];
      VortexEdgeVelocity_[i][1] = Trailing_Vortex.VortexEdgeVelocity_[i][1];
      VortexEdgeVelocity_[i][2] = Trailing_Vortex.VortexEdgeVelocity_[i][2];
     
    }    

    FreeStreamVelocity_[0] = Trailing_Vortex.FreeStreamVelocity_[0];
    FreeStreamVelocity_[1] = Trailing_Vortex.FreeStreamVelocity_[1];
    FreeStreamVelocity_[2] = Trailing_Vortex.FreeStreamVelocity_[2];
    
    // Mach number
    
    Mach_ = Trailing_Vortex.Mach_;
    
    // Pointers to circulation strengths
    
    VortexLoop1_ = Trailing_Vortex.VortexLoop1_;
    VortexLoop2_ = Trailing_Vortex.VortexLoop2_;
    
    Gamma_ = Trailing_Vortex.Gamma_;

    Length_ = Trailing_Vortex.Length_;
    
    FarDist_ = Trailing_Vortex.FarDist_;

    TE_Node_ = Trailing_Vortex.TE_Node_;
    
    S_ = new double[NumberOfSubVortices() + 3];
 
    NodeList_ = new VSP_NODE[NumberOfSubVortices() + 3];
    
    for ( i = 1 ; i <= NumberOfSubVortices() + 2 ; i++ ) {
     
       S_[i] = Trailing_Vortex.S_[i];
    
       NodeList_[i] = Trailing_Vortex.NodeList_[i];
       
    }
    
    return *this;

}

/*##############################################################################
#                                                                              #
#                            VORTEX_TRAIL destructor                           #
#                                                                              #
##############################################################################*/

VORTEX_TRAIL::~VORTEX_TRAIL(void)
{
 

}

/*##############################################################################
#                                                                              #
#                              VORTEX_TRAIL Setup                              #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::Setup(int NumSubVortices, double FarDist, VSP_NODE &Node1, VSP_NODE &Node2)
{

    int i, j, k, m, Level, Done;
    double Vec[3], Theta, DTheta;
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
       printf("2^NumberOfLevels_: %f \n",pow(2,NumberOfLevels_));
        
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
     
      VortexEdgeVelocity_[i] = new double[3];
     
    }
    
    // Calculate length of trailing vortex, and unit vector in it's direction
    
    TE_Node_.x() = Node1.x();
    TE_Node_.y() = Node1.y();
    TE_Node_.z() = Node1.z();
    
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
          
          VortexEdgeList(Level)[j].Wing() = Wing_;
          
          VortexEdgeList(Level)[j].Node() = Node_;
          
          VortexEdgeList(Level)[j].IsTrailingEdge() = 0;
          
          VortexEdgeList(Level)[j].Sigma() = Sigma_;
      
          VortexEdgeList(Level)[j].Setup(NodeA, NodeB);

//printf("i,j,k, m: %d %d %d %d ..... i, i+m: %d %d \n",i,j,k,m,i,i+m);
          
    //      if ( Level == 1 ) printf("Level: %d ... i: %d ... j: %d \n",Level,i,j);
          
          if ( Level > 1 ) {
          
//             printf("Level: %d ... j: %d ... k, k+1 : %d %d \n",Level,j,k,k+1);

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
     
       VortexEdgeList(Level)[j+1].Wing() = Wing_;
       
       VortexEdgeList(Level)[j+1].Node() = Node_;
       
       VortexEdgeList(Level)[j+1].IsTrailingEdge() = 0;
       
       VortexEdgeList(Level)[j+1].Sigma() = Sigma_;
       
       VortexEdgeList(Level)[j+1].Setup(NodeA, NodeB);
          
       m *= 2;
       
    }     
   
}

/*##############################################################################
#                                                                              #
#                         VORTEX_TRAIL SimpleVortex                            #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::InducedVelocity(double xyz_p[3], double q[3])
{
 
   int i, Level;
   double dq[3], Rho, Fact;
   double Vec1[3], Vec2[3], Radius;

   // Update the vortex strengths for all of the sub vortex elements
   
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
              
   if ( Radius > Tolerance_ ) {
    
      VortexEdgeList(Level)[i].InducedVelocity(xyz_p, dq);

      Rho = Radius / VortexEdgeList(Level)[i].Sigma();
      
      Fact = pow(Rho,3.)/pow(Rho*Rho + sqrt(Tolerance_),1.5);

      q[0] += Fact*dq[0];
      q[1] += Fact*dq[1];
      q[2] += Fact*dq[2];
      
   }

}

/*##############################################################################
#                                                                              #
#                 VORTEX_TRAIL CalculateVelocityForSubVortex                   #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::CalculateVelocityForSubVortex(VSP_EDGE &VortexEdge, double xyz_p[3], double q[3])
{
 
   double Vec[3], dq[3], Dot, Dist, Radius, Rho, Fact, Ratio, FarAway;

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
      
      if ( Radius > Tolerance_ ) {
 
         VortexEdge.InducedVelocity(xyz_p, dq);

         Rho = Radius / VortexEdge.Sigma();

         Fact = pow(Rho,3.)/pow(Rho*Rho + sqrt(Tolerance_),1.5);
  
         q[0] += Fact*dq[0];
         q[1] += Fact*dq[1];
         q[2] += Fact*dq[2];                 

      }
        
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

   for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
    
      for ( i = 1 ; i <= NumberOfSubVortices(Level) + 1 ; i++ ) {
       
         // Initialize each subvortex
         
         VortexEdgeList(Level)[i].Gamma() = Gamma_;
         
         VortexEdgeList(Level)[i].Mach() = Mach_;

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
        
        if ( S_[i] <= 0.25*Sigma_ ) {
         
           U[i] = TEVec_[0];
           V[i] = TEVec_[1];
           W[i] = TEVec_[2];
           
        }
        
    }
    
    // Smooth the velocity... currently not reliable
    
    //SmoothVelocity(U);
    //SmoothVelocity(V);
    //SmoothVelocity(W);
 
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

/*
    Level = 1;
    
    for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
    
       NodeA.x() = NodeList_[i  ].x();
       NodeA.y() = NodeList_[i  ].y();
       NodeA.z() = NodeList_[i  ].z();
       
       NodeB.x() = NodeList_[i+1].x();
       NodeB.y() = NodeList_[i+1].y();
       NodeB.z() = NodeList_[i+1].z();

       VortexEdgeList(Level)[i].Setup(NodeA, NodeB);   
       
    }     
*/

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
    
    // Calculate lengths 
 /*   
    double Length;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       Length = 0.;
       
       for ( i = 1 ; i <= NumberOfSubVortices(Level) ; i++ ) {
        
          Length += VortexEdgeList(Level)[i].Length();
          
       }
       
       printf("Level: %d --> Length: %lf \n",Level,Length);
       
    } */
      
    delete [] U;
    delete [] V;
    delete [] W;

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
                  
          Eps = 2.*Ds;
                   
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
 
    int i, n, i_size, c_size, f_size;
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
    
    fwrite(&(n), i_size, 1, adb_file);
    
    fwrite(&(x), f_size, 1, adb_file);
    fwrite(&(y), f_size, 1, adb_file);
    fwrite(&(z), f_size, 1, adb_file);

    for ( i = 1 ; i <= NumberOfSubVortices() + 1; i++ ) {

       x = NodeList_[i+1].x();
       y = NodeList_[i+1].y();
       z = NodeList_[i+1].z();
    
       fwrite(&(x), f_size, 1, adb_file);
       fwrite(&(y), f_size, 1, adb_file);
       fwrite(&(z), f_size, 1, adb_file);

    }

}


