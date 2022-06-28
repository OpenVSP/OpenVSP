#include "Vortex_Trail.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                      Allocate space for statics                              #
#                                                                              #
##############################################################################*/

    double VORTEX_TRAIL::FarAway_ = 5.;


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
  
    Verbose_ = 1;

    NumberOfLevels_ = 0;
        
    VortexEdgeList_ = NULL;
    
    VortexEdgeVelocity_ = NULL;
    
    NumberOfSubVortices_ = NULL;
    
    TotalNumberOfSubVortices_ = 0;
    
    S_[0] = NULL;
    
    S_[1] = NULL;
    
    NodeList_ = NULL;
    
    Tolerance_ = 1.e-6;
    
    Evaluate_ = 0;
    
    TimeAccurate_ = 0;
            
    ConvectType_ = 0;
    
    TimeStep_ = 0.;
    
    Vinf_ = 0.;

    Gamma_ = NULL;
    
    GammaNew_ = NULL;
    
    GammaSave_ = NULL;
    
    WakeAge_ = NULL;
    
    a_ = NULL;
    
    b_ = NULL;
    
    c_ = NULL;
    
    d_ = NULL;
    
    r_ = NULL;    

    dx_ = NULL;
    
    dy_ = NULL;
    
    dz_ = NULL;
    
    CurrentTimeStep_ = 0;
    
    DoGroundEffectsAnalysis_ = 0;
    
    ComponentID_ = 0;

    CoreSize_ = 0.;
    
    Searched_ = 0;
    
    Search_ = NULL;
    
    WakeDampingIsOn_ = 0;
 
    IsARotor_ = 0;
    
    RotorOrigin_[0] = 0.;
    RotorOrigin_[1] = 0.;
    RotorOrigin_[2] = 0.;
    
    RotorThrustVector_[0] = -1.;
    RotorThrustVector_[1] = 0.;
    RotorThrustVector_[2] = 0.;

    FreeStreamDirection_[0] = 1.;
    FreeStreamDirection_[1] = 0.;
    FreeStreamDirection_[2] = 0.;
    
}

/*##############################################################################
#                                                                              #
#                                VORTEX_TRAIL Copy                             #
#                                                                              #
##############################################################################*/

VORTEX_TRAIL::VORTEX_TRAIL(const VORTEX_TRAIL &Trailing_Vortex)
{

    init();

    // Just * use the operator = code

    *this = Trailing_Vortex;

}

/*##############################################################################
#                                                                              #
#                               VORTEX_TRAIL operator=                         #
#                                                                              #
##############################################################################*/

VORTEX_TRAIL& VORTEX_TRAIL::operator=(const VORTEX_TRAIL &Trailing_Vortex)
{

    int i, Level;
 
    Verbose_                        = Trailing_Vortex.Verbose_;

    Wing_                           = Trailing_Vortex.Wing_;
                                   
    Edge_                           = Trailing_Vortex.Edge_;
                                       
    Node_                           = Trailing_Vortex.Node_;
                                   
    ComponentID_                    = Trailing_Vortex.ComponentID_;
                                
    Length_                         = Trailing_Vortex.Length_;
                                   
    FarDist_                        = Trailing_Vortex.FarDist_;
                                   
    NumberOfNodes_                  = Trailing_Vortex.NumberOfNodes_;
                                   
    TE_Node_                        = Trailing_Vortex.TE_Node_;
                                   
    NumberOfLevels_                 = Trailing_Vortex.NumberOfLevels_;

    FreeStreamVelocity_[0]          = Trailing_Vortex.FreeStreamVelocity_[0];
    
    FreeStreamVelocity_[1]          = Trailing_Vortex.FreeStreamVelocity_[1];
    
    FreeStreamVelocity_[2]          = Trailing_Vortex.FreeStreamVelocity_[2];

    LocalVelocityAtTrailingEdge_[0] = Trailing_Vortex.LocalVelocityAtTrailingEdge_[0];
    
    LocalVelocityAtTrailingEdge_[1] = Trailing_Vortex.LocalVelocityAtTrailingEdge_[1];
    
    LocalVelocityAtTrailingEdge_[2] = Trailing_Vortex.LocalVelocityAtTrailingEdge_[2];

    Sigma_                          = Trailing_Vortex.Sigma_;

    CoreSize_                       = Trailing_Vortex.CoreSize_;

    Evaluate_                       = Trailing_Vortex.Evaluate_;
    
    Tolerance_                      = Trailing_Vortex.Tolerance_;
    
    TEVec_[0]                       = Trailing_Vortex.TEVec_[0];
    
    TEVec_[1]                       = Trailing_Vortex.TEVec_[1];
    
    TEVec_[2]                       = Trailing_Vortex.TEVec_[2];
    
    Evaluate_                       = Trailing_Vortex.Evaluate_;

    TimeAccurate_                   = Trailing_Vortex.TimeAccurate_;
                                                                   
    ConvectType_                    = Trailing_Vortex.ConvectType_;
                              
    TimeAnalysisType_               = Trailing_Vortex.TimeAnalysisType_;
                          
    TimeStep_                       = Trailing_Vortex.TimeStep_;
                                  
    Vinf_                           = Trailing_Vortex.Vinf_;

    CurrentTimeStep_                = Trailing_Vortex.CurrentTimeStep_;
                           
    RotorAnalysis_                  = Trailing_Vortex.RotorAnalysis_;
                                   
    BladeRPM_                       = Trailing_Vortex.BladeRPM_;
                                   
    EvaluatedLength_                = Trailing_Vortex.EvaluatedLength_;
                                   
    DoGroundEffectsAnalysis_        = Trailing_Vortex.DoGroundEffectsAnalysis_;
    
    TotalNumberOfSubVortices_       = Trailing_Vortex.TotalNumberOfSubVortices_;
    
    WakeDampingIsOn_                = Trailing_Vortex.WakeDampingIsOn_;

    IsARotor_                       = Trailing_Vortex.IsARotor_;
    
    // List of trailing vortices
    
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
    
    Gamma_     = new VSPAERO_DOUBLE[NumberOfSubVortices() + 5];
    
    GammaNew_  = new VSPAERO_DOUBLE[NumberOfSubVortices() + 5];
    
    GammaSave_ = new VSPAERO_DOUBLE[NumberOfSubVortices() + 5];    
    
    WakeAge_   = new VSPAERO_DOUBLE[NumberOfSubVortices() + 5];    
  
    for ( i = 1 ; i <= NumberOfSubVortices() + 4 ; i++ ) {
       
       Gamma_[i]     = Trailing_Vortex.Gamma_[i];
       GammaNew_[i]  = Trailing_Vortex.GammaNew_[i];
       GammaSave_[i] = Trailing_Vortex.GammaSave_[i];       
       WakeAge_[i]   = Trailing_Vortex.WakeAge_[i]; 
       
    }
    
    VortexEdgeVelocity_ = new VSPAERO_DOUBLE**[NumberOfLevels_ + 1];
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
      
       VortexEdgeVelocity_[Level] = new VSPAERO_DOUBLE*[NumberOfSubVortices_[Level] + 3];

       for ( i = 1 ; i <= NumberOfSubVortices_[Level] + 2 ; i++ ) {
        
         VortexEdgeVelocity_[Level][i] = new VSPAERO_DOUBLE[3];

       }    
       
    }

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
    
       for ( i = 1 ; i <= NumberOfSubVortices_[Level] + 2 ; i++ ) {
        
         VortexEdgeVelocity_[Level][i][0] = Trailing_Vortex.VortexEdgeVelocity_[Level][i][0];
         VortexEdgeVelocity_[Level][i][1] = Trailing_Vortex.VortexEdgeVelocity_[Level][i][1];
         VortexEdgeVelocity_[Level][i][2] = Trailing_Vortex.VortexEdgeVelocity_[Level][i][2];
        
       }    
       
    }
       
       
    S_[0] = new VSPAERO_DOUBLE[NumberOfSubVortices() + 3];
    S_[1] = new VSPAERO_DOUBLE[NumberOfSubVortices() + 3];
 
    NodeList_ = new VSP_NODE[NumberOfSubVortices() + 3];
    
    for ( i = 1 ; i <= NumberOfSubVortices() + 2 ; i++ ) {
     
       S_[0][i] = Trailing_Vortex.S_[0][i];
       
       S_[1][i] = Trailing_Vortex.S_[1][i];
    
       NodeList_[i] = Trailing_Vortex.NodeList_[i];
 
    }
    
    CreateSearchTree_();
    
    return *this;

}

/*##############################################################################
#                                                                              #
#                               VORTEX_TRAIL operator+=                        #
#                                                                              #
##############################################################################*/

VORTEX_TRAIL& VORTEX_TRAIL::operator+=(const VORTEX_TRAIL &Trailing_Vortex)
{

    int i, j, k, m, Level;
    VSP_NODE NodeA, NodeB;
    
    Verbose_                        = Trailing_Vortex.Verbose_;

    Wing_                           = Trailing_Vortex.Wing_;
                                   
    Edge_                           = Trailing_Vortex.Edge_;
                                   
    Node_                           = Trailing_Vortex.Node_;
                                   
    ComponentID_                    = Trailing_Vortex.ComponentID_;
                                   
    Length_                         = Trailing_Vortex.Length_;
                                   
    FarDist_                        = Trailing_Vortex.FarDist_;
                                   
    NumberOfNodes_                  = Trailing_Vortex.NumberOfNodes_;
                                   
    TE_Node_                        = Trailing_Vortex.TE_Node_;
                                   
    NumberOfLevels_                 = Trailing_Vortex.NumberOfLevels_;

    FreeStreamVelocity_[0]          = Trailing_Vortex.FreeStreamVelocity_[0];
    
    FreeStreamVelocity_[1]          = Trailing_Vortex.FreeStreamVelocity_[1];
    
    FreeStreamVelocity_[2]          = Trailing_Vortex.FreeStreamVelocity_[2];

    LocalVelocityAtTrailingEdge_[0] = Trailing_Vortex.LocalVelocityAtTrailingEdge_[0];
    
    LocalVelocityAtTrailingEdge_[1] = Trailing_Vortex.LocalVelocityAtTrailingEdge_[1];
    
    LocalVelocityAtTrailingEdge_[2] = Trailing_Vortex.LocalVelocityAtTrailingEdge_[2];

    Sigma_                          = Trailing_Vortex.Sigma_;

    CoreSize_                       = Trailing_Vortex.CoreSize_;

    Evaluate_                       = Trailing_Vortex.Evaluate_;
    
    Tolerance_                      = Trailing_Vortex.Tolerance_;
    
    TEVec_[0]                       = Trailing_Vortex.TEVec_[0];
    
    TEVec_[1]                       = Trailing_Vortex.TEVec_[1];
    
    TEVec_[2]                       = Trailing_Vortex.TEVec_[2];
    
    Evaluate_                       = Trailing_Vortex.Evaluate_;

    TimeAccurate_                   = Trailing_Vortex.TimeAccurate_;
                                                                                                                              
    ConvectType_                    = Trailing_Vortex.ConvectType_;
                              
    TimeAnalysisType_               = Trailing_Vortex.TimeAnalysisType_;
                          
    TimeStep_                       = Trailing_Vortex.TimeStep_;
                                  
    Vinf_                           = Trailing_Vortex.Vinf_;
   
    CurrentTimeStep_                = Trailing_Vortex.CurrentTimeStep_;
                           
    RotorAnalysis_                  = Trailing_Vortex.RotorAnalysis_;
                                   
    BladeRPM_                       = Trailing_Vortex.BladeRPM_;
                                   
    EvaluatedLength_                = Trailing_Vortex.EvaluatedLength_;
                                   
    DoGroundEffectsAnalysis_        = Trailing_Vortex.DoGroundEffectsAnalysis_;
    
    WakeDampingIsOn_                = Trailing_Vortex.WakeDampingIsOn_;
    
    IsARotor_                       = Trailing_Vortex.IsARotor_;

    // Copy node locations
    
    for ( i = 1 ; i <= NumberOfSubVortices() + 2 ; i++ ) {

       NodeList_[i].x() = Trailing_Vortex.NodeList_[i].x();
       NodeList_[i].y() = Trailing_Vortex.NodeList_[i].y();
       NodeList_[i].z() = Trailing_Vortex.NodeList_[i].z();

    }

    // Update the agglomerated trailing wake approximations
    
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
       
          VortexEdgeList(Level)[j].WakeNode() = i + m;
          
          VortexEdgeList(Level)[j].S() = S_[0][i];
          
          VortexEdgeList(Level)[j].T() = WakeAge_[i];
             
          VortexEdgeList(Level)[j].Setup(NodeA, NodeB);          

          // Edge length
          
          if ( Level == 1 ) {
             
             VortexEdgeList(Level)[j].ReferenceLength() = VortexEdgeList(Level)[j].Length();
             
          }
          
          else {
             
             VortexEdgeList(Level)[j].ReferenceLength() = VortexEdgeList(Level-1)[k  ].ReferenceLength()
                                                        + VortexEdgeList(Level-1)[k+1].ReferenceLength();
             
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
       
       VortexEdgeList(Level)[j+1].WakeNode() = NumberOfSubVortices()+2;
       
       VortexEdgeList(Level)[j+1].S() = S_[0][NumberOfSubVortices()+1];

       VortexEdgeList(Level)[j+1].Setup(NodeA, NodeB);
                    
       m *= 2;
       
    }  
    
    CreateSearchTree_();

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

    }

    if ( VortexEdgeList_ != NULL ) delete [] VortexEdgeList_;

    if ( Gamma_          != NULL ) delete [] Gamma_;

    if ( GammaNew_       != NULL ) delete [] GammaNew_;

    if ( GammaSave_      != NULL ) delete [] GammaSave_;
    
    if ( WakeAge_        != NULL ) delete [] WakeAge_;
    
    if ( a_ != NULL ) delete [] a_;

    if ( b_ != NULL ) delete [] b_;

    if ( c_ != NULL ) delete [] c_;

    if ( d_ != NULL ) delete [] d_;

    if ( r_ != NULL ) delete [] r_;
    
    if ( dx_ != NULL ) delete [] dx_;

    if ( dy_ != NULL ) delete [] dy_;

    if ( dz_ != NULL ) delete [] dz_;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       for ( i = 1 ; i <= NumberOfSubVortices_[Level] + 2 ; i++ ) {
        
         if ( VortexEdgeVelocity_[Level][i] != NULL ) delete [] VortexEdgeVelocity_[Level][i];

       }
       
       if ( VortexEdgeVelocity_[Level] != NULL ) delete [] VortexEdgeVelocity_[Level]; 
       
    }
    
    if ( VortexEdgeVelocity_ != NULL ) delete [] VortexEdgeVelocity_;

    if ( S_[0] != NULL ) delete [] S_[0];

    if ( S_[1] != NULL ) delete [] S_[1];
 
    if ( NodeList_ != NULL ) delete [] NodeList_;
  
    if ( Search_ != NULL ) delete Search_;

    if ( NumberOfSubVortices_ != NULL ) delete [] NumberOfSubVortices_;
   
    VortexEdgeList_ = NULL;
    
    Gamma_ = NULL;
    
    GammaNew_ = NULL;
    
    GammaSave_ = NULL;
    
    WakeAge_ = NULL;

    a_ = NULL;
    
    b_ = NULL;
    
    c_ = NULL;
    
    d_ = NULL;
    
    r_ = NULL;    
    
    dx_ = NULL;
    
    dy_ = NULL;
    
    dz_ = NULL;
    
    VortexEdgeVelocity_ = NULL;
    
    S_[0] = NULL;

    S_[1] = NULL;
    
    NodeList_ = NULL;
    
    Search_ = NULL;
    
    NumberOfSubVortices_ = NULL;
    
}

/*##############################################################################
#                                                                              #
#                              VORTEX_TRAIL Setup                              #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::Setup(int NumSubVortices, VSPAERO_DOUBLE FarDist, VSP_NODE &Node1, VSP_NODE &Node2)
{

    int i, j, k, m, Level, Done;
    VSPAERO_DOUBLE Vec[3], WakeDir[3], Dot, Theta, DTheta, DS, DT, Omega, Time, Angle, Radius;
    VSP_NODE NodeA, NodeB;
    QUAT Quat, InvQuat, Vec1, Vec2;
    
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
    
       PRINTF("Number of trailing wake nodes must be a power of 2! \n"); fflush(NULL);
       PRINTF("NumberOfLevels_: %d \n",NumberOfLevels_);
       PRINTF("2^NumberOfLevels_: %f \n",pow((VSPAERO_DOUBLE)2,NumberOfLevels_));
        
       exit(1);
       
    }
    
    NumberOfSubVortices_ = new int[NumberOfLevels_ + 1];

    VortexEdgeList_ = new VSP_EDGE*[NumberOfLevels_ + 1];
    
    VortexEdgeVelocity_ = new VSPAERO_DOUBLE**[NumberOfLevels_ + 1];

    k = NumSubVortices;
    
    TotalNumberOfSubVortices_ = 0;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       NumberOfSubVortices_[Level] = k;
       
       TotalNumberOfSubVortices_ += k;
  
       VortexEdgeList_[Level] = new VSP_EDGE[k + 2];
       
       VortexEdgeVelocity_[Level] = new VSPAERO_DOUBLE*[NumberOfSubVortices_[Level] + 3];

       for ( i = 1 ; i <= NumberOfSubVortices_[Level] + 2 ; i++ ) {
        
          VortexEdgeVelocity_[Level][i] = new VSPAERO_DOUBLE[3];
        
       }
    
       k /= 2;
       
    }


    S_[0] = new VSPAERO_DOUBLE[NumberOfSubVortices() + 3];

    S_[1] = new VSPAERO_DOUBLE[NumberOfSubVortices() + 3];

    NodeList_ = new VSP_NODE[NumberOfSubVortices() + 3];

    if ( WakeAge_ != NULL ) delete [] WakeAge_;
    
    WakeAge_ = new VSPAERO_DOUBLE[NumberOfSubVortices() + 5];
    
    zero_double_array(WakeAge_, NumberOfSubVortices() + 4);    
    
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

    if ( !RotorAnalysis_ ) {
       
       DTheta = 0.5*PI / NumberOfSubVortices();
       
       for ( i = 1 ; i <= NumberOfNodes_ - 1 ; i++ ) {
        
          Theta = (i-1)*DTheta;
          
          S_[0][i] = FarDist*(1.-cos(Theta));
          S_[1][i] = FarDist*(1.-cos(Theta));
         
       }
       
    }
    
    else {
       
       Omega = BladeRPM_ * 2. * PI / 60.;
       
       DT = (18.*PI/180.)/ABS(Omega);

       DS = DT * Vinf_;
       
       Dot = vector_dot(RotorThrustVector_, FreeStreamDirection_);
       
  //     DS *= MAX(0.1,ABS(Dot));
       
       for ( i = 1 ; i <= NumberOfNodes_ - 1 ; i++ ) {

          S_[0][i] = (i-1)*DS;
          S_[1][i] = (i-1)*DS;
         
       }       
       
    }

    S_[0][NumberOfNodes_] = Length_ - FarDist;
    S_[1][NumberOfNodes_] = Length_ - FarDist;

    // Modify the trailing wake spacing for time accurate analysis... 

    if ( TimeAccurate_ ) {
       
       S_[0][1] = 0.;
       S_[1][1] = 0.;

       for ( j = 2 ; j <= NumberOfNodes_ ; j++ ) {
          
          S_[0][j] = S_[0][j-1] + Vinf_ * TimeStep_;
          S_[1][j] = S_[1][j-1] + Vinf_ * TimeStep_;

       }

    }    
    
    // Simple free stream, planar wakes
    
    if ( !RotorAnalysis_ ) {
       
       for ( i = 1 ; i <= NumberOfNodes_  ; i++ ) {
   
          NodeList_[i].x() = TE_Node_.x() + Vec[0]*S_[0][i];
          NodeList_[i].y() = TE_Node_.y() + Vec[1]*S_[0][i];
          NodeList_[i].z() = TE_Node_.z() + Vec[2]*S_[0][i];
       
       }  
       
    }
    
    // Helical wake
    
    else {  

       S_[0][NumberOfNodes_] = S_[0][NumberOfNodes_ - 1] + (S_[0][NumberOfNodes_ - 1] - S_[0][NumberOfNodes_ - 2]);
       S_[1][NumberOfNodes_] = S_[1][NumberOfNodes_ - 1] + (S_[1][NumberOfNodes_ - 1] - S_[1][NumberOfNodes_ - 2]);
 
       Omega = BladeRPM_ * 2. * PI / 60.;
       
       WakeAge_[0] = 0.;
       
       WakeDir[0] = -0.1*RotorThrustVector_[0] + 0.9*FreeStreamDirection_[0]; 
       WakeDir[1] = -0.1*RotorThrustVector_[1] + 0.9*FreeStreamDirection_[1]; 
       WakeDir[2] = -0.1*RotorThrustVector_[2] + 0.9*FreeStreamDirection_[2];
       
       Dot = sqrt(vector_dot(WakeDir,WakeDir)); 
        
       WakeDir[0] /= Dot;
       WakeDir[1] /= Dot;
       WakeDir[2] /= Dot;
        
       for ( i = 1 ; i <= NumberOfNodes_  ; i++ ) {
   
          Time = S_[0][i] / Vinf_;

      //    Radius = sqrt( TE_Node_.y()*TE_Node_.y() + TE_Node_.z()*TE_Node_.z() );
      //    
      //    // Vector from rotor origin to TE
      //    
      //    Vec[0] = TE_Node_.x() - RotorOrigin_[0];
      //    Vec[1] = TE_Node_.y() - RotorOrigin_[1];
      //    Vec[2] = TE_Node_.z() - RotorOrigin_[2];
      //    
      //    // Subtract out component in direction of rotor thrust
      //    
      //  Dot = vector_dot(Vec,RotorThrustVector_);
      //  
      //  Vec[0] -= Dot * RotorThrustVector_[0];
      //  Vec[1] -= Dot * RotorThrustVector_[1];
      //  Vec[2] -= Dot * RotorThrustVector_[2];
      //  
      //  // Radius is what's left over...
      //  
      //  Radius = sqrt(vector_dot(Vec,Vec));
      //  
      //  // Angle 
      //  
      //  Theta = atan2(TE_Node_.y(), TE_Node_.z());
      //  
      //  Theta = asin(TE_Node_.y() / Radius);
      //  
      //  PRINTF("sin --> Theta: %f \n",Theta);
      //  
      //  Theta = acos(TE_Node_.z() / Radius);
      //  
      //  PRINTF("cos --> Theta: %f \n",Theta);
      //  
      //  NodeList_[i].x() = TE_Node_.x() + S_[0][i];
      //  NodeList_[i].y() = Radius * sin(Omega * Time + Theta);
      //  NodeList_[i].z() = Radius * cos(Omega * Time + Theta);
          
          // Subtract out the rotor origin
          
          Vec1(0) = TE_Node_.x() - RotorOrigin_[0];
          Vec1(1) = TE_Node_.y() - RotorOrigin_[1];
          Vec1(2) = TE_Node_.z() - RotorOrigin_[2];    
          
          // Quaternion for this rotation, and it's inverse      
          
          Angle = -Omega * Time;
          
          Quat.FormRotationQuat(RotorThrustVector_,Angle);
          
          InvQuat = Quat;
          
          InvQuat.FormInverse();   
          
          // Apply the quat transformation, gotta love quats... really, you do.

          Vec2 = Quat * Vec1 * InvQuat;
          
          // Add back in the origin, and translate in the free stream / thrust direction and update xyz...
          // Note... this is just a first guess at the rotor shape and location.

          NodeList_[i].x() = Vec2(0) + RotorOrigin_[0] + S_[0][i]*WakeDir[0];
          NodeList_[i].y() = Vec2(1) + RotorOrigin_[1] + S_[0][i]*WakeDir[1];
          NodeList_[i].z() = Vec2(2) + RotorOrigin_[2] + S_[0][i]*WakeDir[2];
  
          WakeAge_[i] = i * DT;
          
       }  

       S_[0][1] = S_[1][1] = 0.;
       
       for ( i = 2 ; i <= NumberOfNodes_  ; i++ ) {
   
          S_[0][i] = S_[0][i-1] + sqrt( pow(NodeList_[i].x()-NodeList_[i-1].x(),2.)
                                      + pow(NodeList_[i].y()-NodeList_[i-1].y(),2.)
                                      + pow(NodeList_[i].z()-NodeList_[i-1].z(),2.) );
                                      
          S_[1][i] = S_[0][i];                            

       }  
                     
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
 
          VortexEdgeList(Level)[j].IsTrailingEdge() = 0;
          
          VortexEdgeList(Level)[j].Sigma() = Sigma_;
          
          VortexEdgeList(Level)[j].WakeNode() = i + m;
          
          VortexEdgeList(Level)[j].S() = S_[0][i];
      
          VortexEdgeList(Level)[j].Setup(NodeA, NodeB);
          
          // Edge length
          
          if ( Level == 1 ) {
             
             VortexEdgeList(Level)[j].ReferenceLength() = VortexEdgeList(Level)[j].Length();
             
          }
          
          else {
             
             VortexEdgeList(Level)[j].ReferenceLength() = VortexEdgeList(Level-1)[k  ].ReferenceLength()
                                                        + VortexEdgeList(Level-1)[k+1].ReferenceLength();
             
          }
          
          // Children
   
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
 
       VortexEdgeList(Level)[j+1].IsTrailingEdge() = 0;
       
       VortexEdgeList(Level)[j+1].Sigma() = Sigma_;
       
       VortexEdgeList(Level)[j+1].WakeNode() = NumberOfSubVortices()+2;
       
       VortexEdgeList(Level)[j+1].S() = S_[0][NumberOfSubVortices()+1];
       
       VortexEdgeList(Level)[j+1].Setup(NodeA, NodeB);
                              
       m *= 2;
       
    }     
    
    // Gamma arrays
    
    if (     Gamma_ != NULL ) delete [] Gamma_;
    
    if (  GammaNew_ != NULL ) delete [] GammaNew_;
    
    if ( GammaSave_ != NULL ) delete [] GammaSave_ ;
        
    Gamma_     = new VSPAERO_DOUBLE[NumberOfSubVortices() + 5];
    
    GammaNew_  = new VSPAERO_DOUBLE[NumberOfSubVortices() + 5];
    
    GammaSave_ = new VSPAERO_DOUBLE[NumberOfSubVortices() + 5];
    
    zero_double_array(Gamma_,     NumberOfSubVortices() + 4);
    
    zero_double_array(GammaNew_,  NumberOfSubVortices() + 4);
    
    zero_double_array(GammaSave_, NumberOfSubVortices() + 4);
    
    // Residual smoothing arrays

    a_ = new VSPAERO_DOUBLE[NumberOfSubVortices() + 3];
    b_ = new VSPAERO_DOUBLE[NumberOfSubVortices() + 3];
    c_ = new VSPAERO_DOUBLE[NumberOfSubVortices() + 3];
    d_ = new VSPAERO_DOUBLE[NumberOfSubVortices() + 3];
    r_ = new VSPAERO_DOUBLE[NumberOfSubVortices() + 3];    
    
    // Correction arrays
    
    dx_ = new VSPAERO_DOUBLE[NumberOfSubVortices() + 3];
    dy_ = new VSPAERO_DOUBLE[NumberOfSubVortices() + 3];
    dz_ = new VSPAERO_DOUBLE[NumberOfSubVortices() + 3];    
    
    // Search Tree

    CreateSearchTree_();

}

/*##############################################################################
#                                                                              #
#                         VORTEX_TRAIL CreateSearchTree_                       #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::CreateSearchTree_(void)
{
   
    int n;

    n = NumberOfSubVortices();
 
    if ( TimeAccurate_ ) n = MIN( CurrentTimeStep_ + 1, NumberOfSubVortices() );

    if ( Search_ != NULL ) delete Search_;
    
    Search_ = new SEARCH;
    
    Search_->CreateSearchTree(*this, n);

}

/*##############################################################################
#                                                                              #
#                         VORTEX_TRAIL InducedVelocity                         #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3])
{

    VSPAERO_DOUBLE Radius, LocalRadius, Solidity, Vec1[3], Dot, Wgt, Ratio;
   
    CoreSize_ = 0.;
    
    if ( IsARotor_ && WakeDampingIsOn_ ) CoreSize_ = Sigma_;
    
    InducedVelocity_(xyz_p, q);
     
}

/*##############################################################################
#                                                                              #
#                         VORTEX_TRAIL InducedVelocity                         #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE CoreSize)
{

    VSPAERO_DOUBLE Radius, LocalRadius, Solidity, Vec1[3], Dot, Wgt;
    
    CoreSize_ = MAX(CoreSize, 2.*Sigma_);

    InducedVelocity_(xyz_p, q);
    
}

/*##############################################################################
#                                                                              #
#                         VORTEX_TRAIL InducedVelocity_                        #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::InducedVelocity_(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3])
{
 
   int i, Level;
   VSPAERO_DOUBLE dq[3];

   // Update the vortex strengths for all of the sub vortex elements
   // This has to be here... since the wake agglomeration routine, 
   // at the vortex sheet level, may change gamma
   
   UpdateGamma();
 
   // Start at the coarsest level
      
   Level = NumberOfLevels_;

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

   if ( !TimeAccurate_ ) {

      VortexEdgeList(Level)[i].InducedVelocity(xyz_p, dq, CoreSize_);

      q[0] += dq[0];
      q[1] += dq[1];
      q[2] += dq[2];
      
   }

//djk
   // Uncomment below to make things 2D
  // q[0] = q[1] = q[2] = 0.;

}

/*##############################################################################
#                                                                              #
#                 VORTEX_TRAIL CalculateVelocityForSubVortex                   #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::CalculateVelocityForSubVortex(VSP_EDGE &VortexEdge, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3])
{
 
   VSPAERO_DOUBLE dq[3], Dist, Ratio, CoreWidth;

   Dist = sqrt( SQR(VortexEdge.Xc() - xyz_p[0]) 
              + SQR(VortexEdge.Yc() - xyz_p[1]) 
              + SQR(VortexEdge.Zc() - xyz_p[2]) );
                 
   Ratio = Dist / VortexEdge.ReferenceLength();

   if ( !VortexEdge.ThereAreChildren() || Ratio >= FarAway_ ) {

      CoreWidth = sqrt(CoreSize_*CoreSize_ + 5.*0.001*ABS(VortexEdge.Gamma())*VortexEdge.T());

      VortexEdge.InducedVelocity(xyz_p, dq, CoreWidth);

      q[0] += dq[0];
      q[1] += dq[1];
      q[2] += dq[2];     
 
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
 
   int i, Level, NumSubVortices;
   VSPAERO_DOUBLE Wgt1, Wgt2;

   if ( !TimeAccurate_ ) {

      for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
       
         NumSubVortices =  NumberOfSubVortices(Level);
                
         for ( i = 1 ; i <= NumSubVortices + 1 ; i++ ) {
             
            // Initialize each subvortex

            VortexEdgeList(Level)[i].Gamma() = Gamma_[0];

         }
         
      }
      
   }
   
   else {
      
      // Initialize the first level
      
      Level = 1;
      
      NumSubVortices = NumberOfSubVortices(Level);
      
      if ( TimeAccurate_ ) NumSubVortices = MIN( CurrentTimeStep_ + 1, NumberOfSubVortices(Level) );
   
      for ( i = 1 ; i <= NumSubVortices + 1 ; i++ ) {

         VortexEdgeList(Level)[i].Gamma() = Gamma_[i];
         
      }
                  
      for ( Level = 2 ; Level <= NumberOfLevels_ ; Level++ ) {

         NumSubVortices = NumberOfSubVortices(Level);

         for ( i = 1 ; i <= NumSubVortices ; i++ ) {

            // Initialize each subvortex and coarser level... agglomerate the vorticity
            
            Wgt1 = VortexEdgeList(Level)[i].Child1().S()/( VortexEdgeList(Level)[i].Child1().S() + VortexEdgeList(Level)[i].Child2().S() );
            Wgt2 = 1. - Wgt1;
  
            VortexEdgeList(Level)[i].Gamma() = Wgt1*VortexEdgeList(Level)[i].Child1().Gamma() + Wgt2*VortexEdgeList(Level)[i].Child2().Gamma();

         }

         VortexEdgeList(Level)[NumberOfSubVortices(Level) + 1].Gamma() = Gamma_[NumberOfSubVortices(1) + 1];

      }      
      
   }

}
  
/*##############################################################################
#                                                                              #
#                    VORTEX_TRAIL UpdateWakeLocation                           #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE VORTEX_TRAIL::UpdateWakeLocation(void)
{
 
    int i, j, m, Level;
    VSPAERO_DOUBLE Vec[3], Mag, dx, dy, dz, dS, MaxDelta, Relax, Angle, Omega;
    VSPAERO_DOUBLE Dot1, Dot3, Vec3[3];
    VSP_NODE NodeA, NodeB, NodeTemp;
    QUAT OmegaVec, Quat, InvQuat, WQuat, Vec1, Vec2, RotationalVelocity;

    if ( RotorAnalysis_ ) {
    
       // Quaternion for this rotation, and it's inverse      
       
       Angle = 0.;
       
       Quat.FormRotationQuat(RotorThrustVector_,Angle);
       
       InvQuat = Quat;
       
       InvQuat.FormInverse();   
       
       // Angular rate

       Omega = -BladeRPM_ * 2. * PI / 60.;
       
       OmegaVec(0) = Omega * RotorThrustVector_[0];
       OmegaVec(1) = Omega * RotorThrustVector_[1];
       OmegaVec(2) = Omega * RotorThrustVector_[2];
       OmegaVec(3) = 0.;
           
       // Quaternion rates
     
       WQuat = OmegaVec * Quat * InvQuat;
           
    }

    // Align wake with streamlines
    
    NodeList_[1].x() = TE_Node_.x();
    NodeList_[1].y() = TE_Node_.y(); 
    NodeList_[1].z() = TE_Node_.z();

    MaxDelta = 0.;
    
    Relax = 0.85;

    WakeAge_[1] = 0.;

    for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
    
       Vec[0] = VortexEdgeVelocity(i)[0];
       Vec[1] = VortexEdgeVelocity(i)[1];
       Vec[2] = VortexEdgeVelocity(i)[2];

       RotationalVelocity(0) = RotationalVelocity(1) = RotationalVelocity(2) = 0.;
       
       if ( RotorAnalysis_ ) {
          
          // Subtract out the rotor origin
          
          Vec1(0) = NodeList_[i+1].x() - RotorOrigin_[0];
          Vec1(1) = NodeList_[i+1].y() - RotorOrigin_[1];
          Vec1(2) = NodeList_[i+1].z() - RotorOrigin_[2];    
          
          Vec2 = Quat * Vec1 * InvQuat;

          // Rotation velocity
    
          RotationalVelocity = WQuat * Vec2;
                            
       }
       
       if ( i > 1 && i < NumberOfSubVortices() + 1 ) {
          
          Vec[0] = 0.25*( VortexEdgeVelocity(i-1)[0] + 2.*VortexEdgeVelocity(i)[0] + VortexEdgeVelocity(i+1)[0] ) + RotationalVelocity(0);
          Vec[1] = 0.25*( VortexEdgeVelocity(i-1)[1] + 2.*VortexEdgeVelocity(i)[1] + VortexEdgeVelocity(i+1)[1] ) + RotationalVelocity(1);
          Vec[2] = 0.25*( VortexEdgeVelocity(i-1)[2] + 2.*VortexEdgeVelocity(i)[2] + VortexEdgeVelocity(i+1)[2] ) + RotationalVelocity(2);

       }
       
       else {
          
          Vec[0] = VortexEdgeVelocity(i)[0] + RotationalVelocity(0);
          Vec[1] = VortexEdgeVelocity(i)[1] + RotationalVelocity(1);
          Vec[2] = VortexEdgeVelocity(i)[2] + RotationalVelocity(2);      
       
       }     

       Mag = sqrt(vector_dot(Vec,Vec));
   
       Vec[0] /= Mag;
       Vec[1] /= Mag;
       Vec[2] /= Mag;
       
       dS = S_[0][i+1] - S_[0][i];
        
       dx = NodeList_[i].x() + Vec[0]*dS - NodeList_[i+1].x();
       dy = NodeList_[i].y() + Vec[1]*dS - NodeList_[i+1].y();
       dz = NodeList_[i].z() + Vec[2]*dS - NodeList_[i+1].z();

       if ( RotorAnalysis_ ) {
          
          Vec[0] = dx;
          Vec[1] = dy;
          Vec[2] = dz;
          
          // Component in the thrust direction
          
          Dot1 = vector_dot(Vec,RotorThrustVector_);
          
          // Component in direction of the vector from the rotor to the free stram direction
          
          Vec3[0] = FreeStreamDirection_[0] + RotorThrustVector_[0];
          Vec3[1] = FreeStreamDirection_[1] + RotorThrustVector_[1];
          Vec3[2] = FreeStreamDirection_[2] + RotorThrustVector_[2];
          
          Dot3 = vector_dot(FreeStreamDirection_, RotorThrustVector_);
          
          Vec3[0] = FreeStreamDirection_[0] - Dot3 * RotorThrustVector_[0]; 
          Vec3[1] = FreeStreamDirection_[1] - Dot3 * RotorThrustVector_[1]; 
          Vec3[2] = FreeStreamDirection_[2] - Dot3 * RotorThrustVector_[2]; 
          
          Dot3 = sqrt(vector_dot(Vec3, Vec3));
          
          if ( Dot3 > 0. ) {

             Vec3[0] /= Dot3;
             Vec3[1] /= Dot3;
             Vec3[2] /= Dot3;
             
          }

          Dot3 = vector_dot(Vec, Vec3);
      
          dx = Dot1 * RotorThrustVector_[0] + Dot3 * Vec3[0];
          dy = Dot1 * RotorThrustVector_[1] + Dot3 * Vec3[1];
          dz = Dot1 * RotorThrustVector_[2] + Dot3 * Vec3[2];
    
       }

       NodeList_[i+1].x() += Relax*dx;
       NodeList_[i+1].y() += Relax*dy;
       NodeList_[i+1].z() += Relax*dz;

       MaxDelta = MAX((dx*dx + dy*dy + dz*dz)/(S_[0][i+1]*S_[0][i+1]), MaxDelta);
       
       if ( DoGroundEffectsAnalysis_ ) NodeList_[i+1].z() = MAX(NodeList_[i+1].z(), 0.);
       
    }

    MaxDelta = sqrt(MaxDelta);
 
    //SmoothWake();    

    // Update the agglomerated trailing wake approximations
    
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
          
          VortexEdgeList(Level)[j].WakeNode() = i + m;
          
          VortexEdgeList(Level)[j].S() = S_[0][i];
          
          VortexEdgeList(Level)[j].T() = 0.5*(WakeAge_[i] + WakeAge_[i+m]);

          VortexEdgeList(Level)[j].Setup(NodeA, NodeB);          
          
       }

       // Final vortex edge trails off to infinity...
       
       NodeA.x() = NodeList_[NumberOfSubVortices()+1].x();
       NodeA.y() = NodeList_[NumberOfSubVortices()+1].y();
       NodeA.z() = NodeList_[NumberOfSubVortices()+1].z();
       
       NodeB.x() = NodeList_[NumberOfSubVortices()+2].x();
       NodeB.y() = NodeList_[NumberOfSubVortices()+2].y();
       NodeB.z() = NodeList_[NumberOfSubVortices()+2].z();
       
       VortexEdgeList(Level)[j+1].WakeNode() = NumberOfSubVortices()+2;
       
       VortexEdgeList(Level)[j+1].S() = S_[0][NumberOfSubVortices()+1];

       VortexEdgeList(Level)[j+1].Setup(NodeA, NodeB);
          
       m *= 2;
       
    }

    CreateSearchTree_();
  
    return MaxDelta;

}

/*##############################################################################
#                                                                              #
#                        VORTEX_TRAIL SaveVortexState                          #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::SaveVortexState(void)
{
   
    int i, n;

    n = MIN( CurrentTimeStep_ + 1, NumberOfSubVortices() + 1);
           
    // Make a copy of the circulation 
        
    for ( i = 0 ; i <= n ; i++ ) {
       
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
 
    int i, n;
    VSPAERO_DOUBLE Fact, Distance;

    // Set convection type
    
    ConvectType_ = ConvectType;
 
    // Just convecting the implicit portion of the wake
    
    if ( ConvectType_ == IMPLICIT_WAKE_GAMMAS ) {
       
       // Zero out everything, except the first time step loop = 0
             
       for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
       
          Gamma_[i] = 0.;
       
       }
       
       Gamma_[1] = Gamma_[0];
           
    }
    
    // Just convect the explicit portion of the wake
    
    else if ( ConvectType_ == EXPLICIT_WAKE_GAMMAS || ConvectType_ == ALL_WAKE_GAMMAS ) {

       for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
       
          Gamma_[i] = GammaSave_[i];

       }
       
       if ( ConvectType_ == EXPLICIT_WAKE_GAMMAS ) Gamma_[0] = 0.;

       // Exact shift of gamma

       for ( i = NumberOfSubVortices() + 1 ; i >= 1; i-- ) {
          
          Gamma_[i] = Gamma_[i-1];
   
       }
            
       // Don't let errors from upwind scheme propagate further than
       // physically possible

       n = MIN( CurrentTimeStep_ + 1, NumberOfSubVortices() + 2);
   
       for ( i = n ; i <= NumberOfSubVortices() + 1 ; i++ ) {
 
          Gamma_[i] = 0.;

       }    
       
       // Do a weighted update for final trailing vortex
       
       if ( n >= NumberOfSubVortices() + 1 ) {

          i = NumberOfSubVortices() + 1;

          Distance = Vinf_ * TimeStep_ * CurrentTimeStep_;

          if ( S_[0][i] <= Distance ) {
             
             Fact = ( Distance - S_[0][i] ) / ( S_[0][i+1] - S_[0][i] );
Fact = 0.;
             Gamma_[i] *= Fact;
          
          }       
          
       }

    }
    
    else {
       
       PRINTF("Unknown convection option! \n");
       
       exit(1);
       
    }
    
    // Calculate wake age
    
    for ( i = 1 ; i <= NumberOfSubVortices() + 1 ; i++ ) {
       
       WakeAge_[i] = TimeStep_ * i;
      
    }
  
}

/*##############################################################################
#                                                                              #
#                        VORTEX_TRAIL LimitVelocity                            #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::LimitVelocity(VSPAERO_DOUBLE q[3])
{
 
   VSPAERO_DOUBLE Dot;
   
   // No flipping around ...

   Dot = vector_dot(FreeStreamVelocity_, q);

   if ( Dot < 0. ) {
    
      q[0] = FreeStreamVelocity_[0];
      q[1] = FreeStreamVelocity_[1];
      q[2] = FreeStreamVelocity_[2];
      
   }

}

/*##############################################################################
#                                                                              #
#                        VORTEX_TRAIL SmoothVelocity                           #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::SmoothVelocity(VSPAERO_DOUBLE *Velocity)
{

    VSPAERO_DOUBLE *a, *b, *c, *d, *r, Eps;
    int i, NumberOfNodes;

    // Allocate space for the tridiagonal array

    NumberOfNodes = NumberOfSubVortices();
    
    a = new VSPAERO_DOUBLE[NumberOfNodes + 1];
    b = new VSPAERO_DOUBLE[NumberOfNodes + 1];
    c = new VSPAERO_DOUBLE[NumberOfNodes + 1];
    d = new VSPAERO_DOUBLE[NumberOfNodes + 1];
    r = new VSPAERO_DOUBLE[NumberOfNodes + 1];
    
    // Loop over x,y, and then z values
    
    for ( i = 1 ; i <= NumberOfNodes ; i++ ) {
     
       r[i] = Velocity[i];
  
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

    Eps = 0.5;

    for ( i = 2 ; i < NumberOfNodes ; i++ ) {

       a[i] = -0.5*Eps;
       b[i] = 1. + Eps;
       c[i] = -0.5*Eps;
       d[i] = r[i];
       
       if ( S_[0][i] <= 0.25*Sigma_ ) {
          
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

    VSPAERO_DOUBLE Eps;
    int i, Case, NumberOfNodes;

    // Allocate space for the tridiagonal array

    NumberOfNodes = MIN(CurrentTimeStep_ + 1, NumberOfSubVortices() + 1);

    // Loop over x,y, and then z values
    
    for ( Case = 1 ; Case <= 3 ; Case++ ) {

       for ( i = 1 ; i <= NumberOfNodes ; i++ ) {
        
          if ( Case == 1 ) r_[i] = dx_[i];
          if ( Case == 2 ) r_[i] = dy_[i];
          if ( Case == 3 ) r_[i] = dz_[i];
          
       }
    
       // BC at trailing edge
       
       a_[1] = 0.;
       b_[1] = 1.;
       c_[1] = 0.;
       d_[1] = r_[1];
       
       // BC at far wake

       a_[NumberOfNodes] =  0.;
       b_[NumberOfNodes] =  1.;
       c_[NumberOfNodes] =  0.;
       d_[NumberOfNodes] =  r_[NumberOfNodes];   

       // Implicit residual smoothing

       for ( i = 2 ; i < NumberOfNodes ; i++ ) {

          Eps = 1. + 0.001*S_[0][i]/TimeStep_;

          a_[i] = -0.5*Eps;
          b_[i] = 1. + Eps;
          c_[i] = -0.5*Eps;
          d_[i] = r_[i];

       }

       // Forward elimination

       for ( i = 2 ; i <= NumberOfNodes ; i++ ) {

          b_[i] = b_[i] - c_[i-1]*a_[i]/b_[i-1];
          d_[i] = d_[i] - d_[i-1]*a_[i]/b_[i-1];

       }

       // Backwards substitution

       r_[NumberOfNodes] = d_[NumberOfNodes]/b_[NumberOfNodes];

       for ( i = NumberOfNodes - 1 ; i >= 1 ; i-- ) {

          r_[i] = ( d_[i] - c_[i]*r_[i+1] )/b_[i];

       }
       
       // Update solution
       
       for ( i = 1 ; i <= NumberOfNodes ; i++ ) {
        
          if ( Case == 1 ) dx_[i] = r_[i];
          if ( Case == 2 ) dy_[i] = r_[i];
          if ( Case == 3 ) dz_[i] = r_[i];
        
       }
        
    }

}

/*##############################################################################
#                                                                              #
#                        VORTEX_TRAIL SmoothWake                               #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::SmoothWake(void)
{

    VSPAERO_DOUBLE Eps;
    int i, Case, NumberOfNodes;

    // Allocate space for the tridiagonal array

    NumberOfNodes = NumberOfSubVortices() + 1;

    if ( TimeAccurate_ ) NumberOfNodes = MIN( CurrentTimeStep_ + 1, NumberOfSubVortices() + 1);

    // Loop over x,y, and then z values
    
    for ( Case = 1 ; Case <= 3 ; Case++ ) {

       for ( i = 1 ; i <= NumberOfNodes ; i++ ) {

          if ( Case == 1 ) r_[i] = NodeList_[i].x();
          if ( Case == 2 ) r_[i] = NodeList_[i].y();
          if ( Case == 3 ) r_[i] = NodeList_[i].z();
          
       }

       // BC at trailing edge
       
       a_[1] = 0.;
       b_[1] = 1.;
       c_[1] = 0.;
       d_[1] = r_[1];
       
       // BC at far wake

       a_[NumberOfNodes] =  0.;
       b_[NumberOfNodes] =  1.;
       c_[NumberOfNodes] =  0.;
       d_[NumberOfNodes] =  r_[NumberOfNodes];   

       // General node... implicity smoothing

       Eps = 0.25;

       for ( i = 2 ; i < NumberOfNodes ; i++ ) {
 
          a_[i] = -0.5*Eps;
          b_[i] = 1. + Eps;
          c_[i] = -0.5*Eps;
          d_[i] = r_[i];

       }

       // Forward elimination

       for ( i = 2 ; i <= NumberOfNodes ; i++ ) {

          b_[i] = b_[i] - c_[i-1]*a_[i]/b_[i-1];
          d_[i] = d_[i] - d_[i-1]*a_[i]/b_[i-1];

       }

       // Backwards substitution

       r_[NumberOfNodes] = d_[NumberOfNodes]/b_[NumberOfNodes];

       for ( i = NumberOfNodes - 1 ; i >= 1 ; i-- ) {

          r_[i] = ( d_[i] - c_[i]*r_[i+1] )/b_[i];

       }
       
       // Update solution
       
       for ( i = 1 ; i <= NumberOfNodes ; i++ ) {
        
          if ( Case == 1 ) NodeList_[i].x() = r_[i];
          if ( Case == 2 ) NodeList_[i].y() = r_[i];
          if ( Case == 3 ) NodeList_[i].z() = r_[i];
          
          if ( Case == 3 && DoGroundEffectsAnalysis_ ) NodeList_[i].z() = MAX(NodeList_[i].z(), 0.);    
        
       }
        
    }
    
}

/*##############################################################################
#                                                                              #
#                    VORTEX_TRAIL UpdateGeometryLocation                       #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat)
{
 
    int i, j, k, m, Level, NumMaxSubVortices;
    VSPAERO_DOUBLE U, V, W, dS;
    VSP_NODE NodeA, NodeB;
    QUAT Vec;

    // Update location
    
    Vec(0) = TE_Node_.x() - OVec[0];
    Vec(1) = TE_Node_.y() - OVec[1];
    Vec(2) = TE_Node_.z() - OVec[2];

    Vec = Quat * Vec * InvQuat;

    dx_[1] = 0.;
    dy_[1] = 0.;
    dz_[1] = 0.;

    TE_Node_.x() = Vec(0) + OVec[0] + TVec[0];
    TE_Node_.y() = Vec(1) + OVec[1] + TVec[1];
    TE_Node_.z() = Vec(2) + OVec[2] + TVec[2];    
    
    // Keep track of final vortex edge that goes of to infinity
        
    i = NumberOfSubVortices() + 2;
    
    dx_[i] = NodeList_[i].x() - NodeList_[i-1].x();
    dy_[i] = NodeList_[i].y() - NodeList_[i-1].y();
    dz_[i] = NodeList_[i].z() - NodeList_[i-1].z();

    // Now do a time step and convect.. Forward Euler
    
    NumMaxSubVortices = MIN(CurrentTimeStep_, NumberOfSubVortices());

    for ( i = 2 ; i <= NumMaxSubVortices + 1 ; i++ ) {

       U = VortexEdgeVelocity_[1][i-1][0];
       V = VortexEdgeVelocity_[1][i-1][1];
       W = VortexEdgeVelocity_[1][i-1][2];

       dx_[i] = U * TimeStep_;
       dy_[i] = V * TimeStep_;
       dz_[i] = W * TimeStep_;       

    } 

    // Implicit residual smoothing  
    
    Smooth();
    
    // Update x,y,z .. and shift in time along array

    for ( i = NumMaxSubVortices + 1 ; i >= 2 ; i-- ) {
   
       NodeList_[i].x() = NodeList_[i-1].x() + dx_[i];
       NodeList_[i].y() = NodeList_[i-1].y() + dy_[i];
       NodeList_[i].z() = NodeList_[i-1].z() + dz_[i];
   
       if ( DoGroundEffectsAnalysis_ ) NodeList_[i].z() = MAX(NodeList_[i].z(), 0.);    
 
    }  

    NodeList_[1].x() = TE_Node_.x();
    NodeList_[1].y() = TE_Node_.y(); 
    NodeList_[1].z() = TE_Node_.z();    
        
    if ( DoGroundEffectsAnalysis_ ) NodeList_[1].z() = MAX(NodeList_[1].z(), 0.);  

    // Update wake to infinity for quasi-steady cases, otherwise this is just another node
    
    i = NumberOfSubVortices() + 2;
    
    NodeList_[i].x() = NodeList_[i-1].x() + dx_[i];
    NodeList_[i].y() = NodeList_[i-1].y() + dy_[i];
    NodeList_[i].z() = NodeList_[i-1].z() + dz_[i];

    if ( DoGroundEffectsAnalysis_ ) NodeList_[i].z() = MAX(NodeList_[i].z(), 0.);  

    for ( i = 0 ; i <= NumMaxSubVortices + 2 ; i++ ) {

       S_[1][i] = S_[0][i];
            
    }   
 
    S_[0][1] = 0.;
    
    for ( i = 2 ; i <= NumMaxSubVortices + 2 ; i++ ) {

       dS = sqrt( (NodeList_[i].x()-NodeList_[i-1].x())*(NodeList_[i].x()-NodeList_[i-1].x())
                + (NodeList_[i].y()-NodeList_[i-1].y())*(NodeList_[i].y()-NodeList_[i-1].y())
                + (NodeList_[i].z()-NodeList_[i-1].z())*(NodeList_[i].z()-NodeList_[i-1].z()) );

       S_[0][i] = S_[0][i-1] + dS;
    
    }      
 
    // Update the agglomerated trailing wake approximations
    
    m = 1;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       j = 0;

       k = 1;
       
       for ( i = 1 ; i <= NumMaxSubVortices ; i+=m ) {
        
          j++;
          
          NodeA.x() = NodeList_[i  ].x();
          NodeA.y() = NodeList_[i  ].y();
          NodeA.z() = NodeList_[i  ].z();
          
          NodeB.x() = NodeList_[i+m].x();
          NodeB.y() = NodeList_[i+m].y();
          NodeB.z() = NodeList_[i+m].z();
          
          VortexEdgeList(Level)[j].WakeNode() = i + m;
          
          VortexEdgeList(Level)[j].S() = S_[0][i];

          VortexEdgeList(Level)[j].T() = 0.5*( WakeAge_[i] + WakeAge_[i+m] );
                                      
          VortexEdgeList(Level)[j].Setup(NodeA, NodeB);          
        
          // Edge length
          
          if ( Level == 1 ) {
             
             VortexEdgeList(Level)[j].ReferenceLength() = VortexEdgeList(Level)[j].Length();
             
          }
          
          else {
             
             VortexEdgeList(Level)[j].ReferenceLength() = VortexEdgeList(Level-1)[k  ].ReferenceLength()
                                                        + VortexEdgeList(Level-1)[k+1].ReferenceLength();
   
          }
          
          k += 2;
                    
       }

       // Final vortex edge trails off to infinity...
       
       if ( !TimeAccurate_ ) {

          NodeA.x() = NodeList_[NumberOfSubVortices()+1].x();
          NodeA.y() = NodeList_[NumberOfSubVortices()+1].y();
          NodeA.z() = NodeList_[NumberOfSubVortices()+1].z();
          
          NodeB.x() = NodeList_[NumberOfSubVortices()+2].x();
          NodeB.y() = NodeList_[NumberOfSubVortices()+2].y();
          NodeB.z() = NodeList_[NumberOfSubVortices()+2].z();
          
          VortexEdgeList(Level)[j+1].WakeNode() = NumberOfSubVortices()+2;
          
          VortexEdgeList(Level)[j+1].S() = S_[0][NumberOfSubVortices()+1];
          
          VortexEdgeList(Level)[j+1].Setup(NodeA, NodeB);
       
       }
       
       m *= 2;
       
    }    
    
    CreateSearchTree_();

}

/*##############################################################################
#                                                                              #
#                            VORTEX_TRAIL GammaScale                           #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE VORTEX_TRAIL::GammaScale(int i)
{
   
   VSPAERO_DOUBLE dS_old, dS_new, Scale;
   
   return 1;
   
   if ( i == 1 ) return 1.;
   
   dS_new = S_[0][i+1] - S_[0][i  ]; // New spacing
   dS_old = S_[1][i  ] - S_[1][i-1]; // Old spacing

   Scale = 1.;
   
   if ( dS_new > 0. ) Scale = dS_old / dS_new;
   
   Scale = MAX(0.,MIN(Scale,1.));
   
   return Scale;

}

/*##############################################################################
#                                                                              #
#                     VORTEX_TRAIL ZeroEdgeVelocities                          #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::ZeroEdgeVelocities(void)
{
   
    int i, Level;
 
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       for ( i = 1 ; i <= NumberOfSubVortices_[Level] + 2 ; i++ ) {
             
          VortexEdgeVelocity_[Level][i][0] = 0.;          
          VortexEdgeVelocity_[Level][i][1] = 0.;          
          VortexEdgeVelocity_[Level][i][2] = 0.;
          
       }
       
    }          
          
}

/*##############################################################################
#                                                                              #
#                     VORTEX_TRAIL ProlongateEdgeVelocities                    #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::ProlongateEdgeVelocities(void)
{
 
    int i, j, Level;
    
    // Prolongate velocity from coarse to fine grid 

    for ( Level = NumberOfLevels_ ; Level > 1 ; Level-- ) {

       for ( i = 1 ; i <= NumberOfSubVortices_[Level] ; i++ ) {
          
          j = 2 * i - 1;
             
          VortexEdgeVelocity_[Level-1][j][0] += VortexEdgeVelocity_[Level][i][0];          
          VortexEdgeVelocity_[Level-1][j][1] += VortexEdgeVelocity_[Level][i][1];         
          VortexEdgeVelocity_[Level-1][j][2] += VortexEdgeVelocity_[Level][i][2];

          j = 2 * i;
             
          VortexEdgeVelocity_[Level-1][j][0] += VortexEdgeVelocity_[Level][i][0];            
          VortexEdgeVelocity_[Level-1][j][1] += VortexEdgeVelocity_[Level][i][1];            
          VortexEdgeVelocity_[Level-1][j][2] += VortexEdgeVelocity_[Level][i][2]; 
                    
       }
       
    }          
              
}

/*##############################################################################
#                                                                              #
#                          VORTEX_TRAIL WriteToFile                            #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::WriteToFile(FILE *adb_file)
{
 
    int i, n, Done, i_size, c_size, f_size;
    float x, y, z, s;
    
    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    n = NumberOfSubVortices() + 2;
    
    Done = 0;
    
    if ( TimeAccurate_ ) n = MIN( CurrentTimeStep_ + 1, NumberOfSubVortices() + 2);

    FWRITE(&(Node_), i_size, 1, adb_file);  // Kutta node
        
    s = FLOAT (SoverB_);
    
    FWRITE(&(s), f_size, 1, adb_file); // S over B (span) 
    
    FWRITE(&(n), i_size, 1, adb_file); // Number of subvorices
    
    x = FLOAT (NodeList_[1].x());
    y = FLOAT (NodeList_[1].y());
    z = FLOAT (NodeList_[1].z());
     
    FWRITE(&(x), f_size, 1, adb_file);
    FWRITE(&(y), f_size, 1, adb_file);
    FWRITE(&(z), f_size, 1, adb_file);

    for ( i = 2 ; i <= n ; i++ ) {

       x = FLOAT (NodeList_[i].x());
       y = FLOAT (NodeList_[i].y());
       z = FLOAT (NodeList_[i].z());

       FWRITE(&(x), f_size, 1, adb_file);
       FWRITE(&(y), f_size, 1, adb_file);
       FWRITE(&(z), f_size, 1, adb_file);

    }

}

/*##############################################################################
#                                                                              #
#                          VORTEX_TRAIL WriteToFile                            #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::ReadInFile(FILE *adb_file)
{
 
    int i, n, Done, i_size, c_size, f_size, DumInt;
    float x, y, z, s;
    
    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    n = NumberOfSubVortices() + 2;
    
    Done = 0;
   
    FREAD(&(DumInt), i_size, 1, adb_file);     

    FREAD(&(s), f_size, 1, adb_file);
    
    FREAD(&(n), i_size, 1, adb_file);

    FREAD(&(x), f_size, 1, adb_file);
    FREAD(&(y), f_size, 1, adb_file);
    FREAD(&(z), f_size, 1, adb_file);

    for ( i = 2 ; i <= n ; i++ ) {

       FREAD(&(x), f_size, 1, adb_file);
       FREAD(&(y), f_size, 1, adb_file);
       FREAD(&(z), f_size, 1, adb_file);

       NodeList_[i].x() = x;
       NodeList_[i].y() = y;
       NodeList_[i].z() = z;
    
    }

}

/*##############################################################################
#                                                                              #
#                          VORTEX_TRAIL SkipReadInFile                         #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::SkipReadInFile(FILE *adb_file)
{
 
    int i, n, i_size, c_size, f_size;

    int DumInt;
    float DumFloat;
      
    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    FREAD(&DumInt, i_size, 1, adb_file);     
    
    FREAD(&(DumFloat), f_size, 1, adb_file);
     
    FREAD(&n, i_size, 1, adb_file);

    FREAD(&DumFloat, f_size, 1, adb_file);
    FREAD(&DumFloat, f_size, 1, adb_file);
    FREAD(&DumFloat, f_size, 1, adb_file);

    for ( i = 2 ; i <= n ; i++ ) {

       FREAD(&DumFloat, f_size, 1, adb_file);
       FREAD(&DumFloat, f_size, 1, adb_file);
       FREAD(&DumFloat, f_size, 1, adb_file);

    }

}

#include "END_NAME_SPACE.H"
