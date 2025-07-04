#include "Vortex_Trail.H"

#include "START_NAME_SPACE.H"

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
  
    Verbose_ = 0;

    Wing_ = 0;
    
    TE_Edge_ = 0;
    
    TE_Node_ = 0;
    
    TE_Node_Region_Is_Concave_ = 0;
    
    IsFrozenTrailingEdge_ = 0;
    
    ComponentID_ = 0;

    S_[0] = NULL;
    
    S_[1] = NULL;
    
    NumberOfNodes_ = 0;

    NodeList_ = NULL;
    
    NodeList_Time_M_1_ = NULL;
    
    EdgeList_ = NULL;

    FreeStreamVelocity_[0] = 0.;
    FreeStreamVelocity_[1] = 0.;
    FreeStreamVelocity_[2] = 0.;
    
    Sigma_ = 0.;
    
    CoreSize_ = 0.;

    Tolerance_ = 1.e-6;
    
    SoverB_ = 0.;
        
    TimeAccurate_ = 0;
            
    ConvectType_ = 0;
    
    TimeAnalysisType_ = 0;
    
    Time_ = 0;

    WakeDampingIsOn_ = 0;
    
    Group_ = 0;
    
    IsARotor_ = 0;
    
    DoVortexStretching_ = 0;

    RotorOrigin_[0] = 0.;
    RotorOrigin_[1] = 0.;
    RotorOrigin_[2] = 0.;
        
    RotorThrustVector_[0] = -1.;
    RotorThrustVector_[1] =  0.;
    RotorThrustVector_[2] =  0.;

    FreeStreamDirection_[0] = 1.;
    FreeStreamDirection_[1] = 0.;
    FreeStreamDirection_[2] = 0.;
       
    DeltaTime_ = 0.;
    
    Vinf_ = 0.;

    RotorAnalysis_ = 0;
    
    BladeRPM_ = 0.;

    DoGroundEffectsAnalysis_ = 0;
    
    GlobalNode_ = NULL;

    WakeResidualEquationNumber_ = NULL;

    GlobalEdge_ = NULL;

    KuttaStallEquationNumber_ = 0;
    
    StallFactor_ = 1.;

    LocalChord_ = 4.;
    
    Cli_ = 0.;
    
    Cdi_ = 0.;
    
    Csi_ = 0.;
    
    Clo_ = 0.;
    
    Cdo_ = 0.;
    
    Cso_ = 0.;
    
    CFix_ = 0.;
       
    CFiy_ = 0.;
       
    CFiz_ = 0.;
       
    CFox_ = 0.;
       
    CFoy_ = 0.;
       
    CFoz_ = 0.;
    
    CMix_ = 0.;
    
    CMiy_ = 0.;
    
    CMiz_ = 0.;
    
    CMox_ = 0.;
    
    CMoy_ = 0.;
    
    CMoz_ = 0.;
    
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
                                   
    TE_Edge_                        = Trailing_Vortex.TE_Edge_;
                                       
    TE_Node_                        = Trailing_Vortex.TE_Node_;
    
    TE_Node_Region_Is_Concave_      = Trailing_Vortex.TE_Node_Region_Is_Concave_;
    
    IsFrozenTrailingEdge_           = Trailing_Vortex.IsFrozenTrailingEdge_;
                                   
    ComponentID_                    = Trailing_Vortex.ComponentID_;
                              
    NumberOfNodes_                  = Trailing_Vortex.NumberOfNodes_;
                        
    FreeStreamVelocity_[0]          = Trailing_Vortex.FreeStreamVelocity_[0];
    
    FreeStreamVelocity_[1]          = Trailing_Vortex.FreeStreamVelocity_[1];
    
    FreeStreamVelocity_[2]          = Trailing_Vortex.FreeStreamVelocity_[2];
    
    Sigma_                          = Trailing_Vortex.Sigma_;
    
    CoreSize_                       = Trailing_Vortex.CoreSize_;
    
    Tolerance_                      = Trailing_Vortex.Tolerance_;
    
    SoverB_                         = Trailing_Vortex.SoverB_;

    TimeAccurate_                   = Trailing_Vortex.TimeAccurate_;
                                                                   
    ConvectType_                    = Trailing_Vortex.ConvectType_;
                               
    TimeAnalysisType_               = Trailing_Vortex.TimeAnalysisType_;
    
    Time_                           = Trailing_Vortex.Time_;
    
    WakeDampingIsOn_                = Trailing_Vortex.WakeDampingIsOn_;

    IsARotor_                       = Trailing_Vortex.IsARotor_;
  
    RotorOrigin_[0]                 = Trailing_Vortex.RotorOrigin_[0];
    RotorOrigin_[1]                 = Trailing_Vortex.RotorOrigin_[1];
    RotorOrigin_[2]                 = Trailing_Vortex.RotorOrigin_[2];
  
    RotorThrustVector_[0]           = Trailing_Vortex.RotorThrustVector_[0];
    RotorThrustVector_[1]           = Trailing_Vortex.RotorThrustVector_[1];
    RotorThrustVector_[2]           = Trailing_Vortex.RotorThrustVector_[2];    
  
    FreeStreamDirection_[0]         = Trailing_Vortex.FreeStreamDirection_[0];
    FreeStreamDirection_[1]         = Trailing_Vortex.FreeStreamDirection_[1];
    FreeStreamDirection_[2]         = Trailing_Vortex.FreeStreamDirection_[2];    
                              
    DeltaTime_                       = Trailing_Vortex.DeltaTime_;
                                  
    Vinf_                           = Trailing_Vortex.Vinf_;
                  
    RotorAnalysis_                  = Trailing_Vortex.RotorAnalysis_;
                                   
    BladeRPM_                       = Trailing_Vortex.BladeRPM_;
                                 
    DoGroundEffectsAnalysis_        = Trailing_Vortex.DoGroundEffectsAnalysis_;

    S_[0] = new double[NumberOfNodes() + 1];
    
    S_[1] = new double[NumberOfNodes() + 1];
 
    NodeList_ = new VSP_NODE[NumberOfNodes() + 1];
    
    NodeList_Time_M_1_ = new VSP_NODE[NumberOfNodes() + 1];
 
    GlobalNode_ = new int[NumberOfNodes() + 1];
  
    WakeResidualEquationNumber_ = new int[3*NumberOfNodes() + 1];

    GlobalEdge_ = new int[NumberOfNodes() + 1];

    for ( i = 1 ; i <= NumberOfNodes() ; i++ ) {
     
       S_[0][i] = Trailing_Vortex.S_[0][i];
       
       S_[1][i] = Trailing_Vortex.S_[1][i];
       
       NodeList_[i] = Trailing_Vortex.NodeList_[i];
           
       GlobalNode_[i] = Trailing_Vortex.GlobalNode_[i];

       WakeResidualEquationNumber_[3*i-2] = Trailing_Vortex.WakeResidualEquationNumber_[3*i-2];
       WakeResidualEquationNumber_[3*i-1] = Trailing_Vortex.WakeResidualEquationNumber_[3*i-1];
       WakeResidualEquationNumber_[3*i  ] = Trailing_Vortex.WakeResidualEquationNumber_[3*i  ];

       GlobalEdge_[i] = Trailing_Vortex.GlobalEdge_[i];

    }

    KuttaStallEquationNumber_ = Trailing_Vortex.KuttaStallEquationNumber_;
    
    StallFactor_ = Trailing_Vortex.StallFactor_;
    
    LocalChord_ = Trailing_Vortex.LocalChord_;
     
    return *this;

}

/*##############################################################################
#                                                                              #
#                            VORTEX_TRAIL destructor                           #
#                                                                              #
##############################################################################*/

VORTEX_TRAIL::~VORTEX_TRAIL(void)
{

    if ( S_[0] != NULL ) delete [] S_[0];
    
    if ( S_[1] != NULL ) delete [] S_[1];
    
    if ( NodeList_ != NULL ) delete [] NodeList_;
    
    if ( NodeList_Time_M_1_ != NULL ) delete [] NodeList_Time_M_1_;
    
    if ( GlobalNode_ != NULL ) delete [] GlobalNode_;
    
    if ( GlobalEdge_ != NULL ) delete [] GlobalEdge_;
    
    if ( EdgeList_ != NULL ) delete [] EdgeList_;
       
    if ( WakeResidualEquationNumber_ != NULL ) delete [] WakeResidualEquationNumber_;

    S_[0] = NULL;
  
    S_[1] = NULL;
    
    NodeList_ = NULL;
    
    NodeList_Time_M_1_ = NULL;

    GlobalEdge_ = NULL;
  
}

/*##############################################################################
#                                                                              #
#                              VORTEX_TRAIL Setup                              #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::Setup(int NumWakeNodes, double FarDist, VSP_NODE &Node1, VSP_NODE &Node2)
{

    int i, j, k, m, Level, Done;
    double Vec[3], WakeDir[3];
    double Length, Dot, Theta, DTheta, DS, DT, Omega, Time, Angle;
    VSP_NODE NodeA, NodeB;
    QUAT Quat, InvQuat, Vec1, Vec2;
    
    NumberOfNodes_ = NumWakeNodes;
    
    FarDist = FarDist;   

    S_[0] = new double[NumberOfNodes() + 1];

    S_[1] = new double[NumberOfNodes() + 1];
    
    NodeList_ = new VSP_NODE[NumberOfNodes() + 1];
    
    NodeList_Time_M_1_ = new VSP_NODE[NumberOfNodes() + 1];

    GlobalNode_ = new int[NumberOfNodes() + 1];
    
    GlobalEdge_ = new int[NumberOfNodes() + 1];
    
    WakeResidualEquationNumber_ = new int[3*NumberOfNodes() + 1];

    EdgeList_ = new VSP_EDGE*[NumberOfNodes() + 1];

    for ( i = 1 ; i <= NumberOfNodes() ; i++ ) {
       
       EdgeList_[i] = NULL;
       
    }

    // Node 2 is the far down stream point... basically 'infinity'
    
    Vec[0] = Node2.x() - Node1.x();  
    Vec[1] = Node2.y() - Node1.y();
    Vec[2] = Node2.z() - Node1.z();

    Length = sqrt(vector_dot(Vec,Vec));
    
    Vec[0] /= Length;
    Vec[1] /= Length;
    Vec[2] /= Length;
 
    // Create node spacing out along vortex line

    DT = 0.;

    if ( RotorAnalysis_ == 0 ) {
      
       DS = FarDist/ (NumberOfNodes() - 2);
       
       for ( i = 1 ; i <= NumberOfNodes_ - 1 ; i++ ) {
 
          S_[0][i] = (i-1)*DS;
          S_[1][i] = (i-1)*DS;          
                   
       }
       
    }
    
    else {
       
       Omega = BladeRPM_ * 2. * PI / 60.;
       
       DT = (18.*PI/180.)/ABS(Omega);

       DS = DT * Vinf_;
       
       Dot = vector_dot(RotorThrustVector_, FreeStreamDirection_);

       for ( i = 1 ; i <= NumberOfNodes() - 1 ; i++ ) {

          S_[0][i] = (i-1)*DS;
          S_[1][i] = (i-1)*DS;

       }       
       
    }

    S_[0][NumberOfNodes()] = Length - FarDist;
    S_[1][NumberOfNodes()] = Length - FarDist;

    // Modify the trailing wake spacing for time accurate analysis... 

    if ( TimeAccurate_ ) {
       
       S_[0][1] = 0.;
       S_[1][1] = 0.;

       for ( j = 2 ; j <= NumberOfNodes() ; j++ ) {
          
          S_[0][j] = S_[0][j-1] + Vinf_ * DeltaTime_;
          S_[1][j] = S_[1][j-1] + Vinf_ * DeltaTime_;

       }

    }    
    
    // Simple free stream, planar wakes

    if ( RotorAnalysis_ == 0 ) {

       for ( i = 1 ; i <= NumberOfNodes_  ; i++ ) {
   
          NodeList_[i].x() = Node1.x() + Vec[0]*S_[0][i];
          NodeList_[i].y() = Node1.y() + Vec[1]*S_[0][i];
          NodeList_[i].z() = Node1.z() + Vec[2]*S_[0][i];

       }  
       
    }
    
    // Helical wake
    
    else {  

       S_[0][NumberOfNodes()] = S_[0][NumberOfNodes() - 1] + (S_[0][NumberOfNodes() - 1] - S_[0][NumberOfNodes() - 2]);
       S_[1][NumberOfNodes()] = S_[1][NumberOfNodes() - 1] + (S_[1][NumberOfNodes() - 1] - S_[1][NumberOfNodes() - 2]);
 
       Omega = BladeRPM_ * 2. * PI / 60.;

       // This sets the intial shape of the wake for a steady state case
       
       WakeDir[0] = -0.1*RotorThrustVector_[0] + 0.9*FreeStreamDirection_[0]; 
       WakeDir[1] = -0.1*RotorThrustVector_[1] + 0.9*FreeStreamDirection_[1]; 
       WakeDir[2] = -0.1*RotorThrustVector_[2] + 0.9*FreeStreamDirection_[2];
      
       Dot = sqrt(vector_dot(WakeDir,WakeDir)); 
        
       WakeDir[0] /= Dot;
       WakeDir[1] /= Dot;
       WakeDir[2] /= Dot;
        
       for ( i = 1 ; i <= NumberOfNodes_ - 1 ; i++ ) {
   
          Time = S_[0][i] / Vinf_;

          // Subtract out the rotor origin
          
          Vec1(0) = Node1.x() - RotorOrigin_[0];
          Vec1(1) = Node1.y() - RotorOrigin_[1];
          Vec1(2) = Node1.z() - RotorOrigin_[2];   
  
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

       }  
       
       // Last wake node still off at infinity...
       
       NodeList_[NumberOfNodes_].x() = Node2.x();
       NodeList_[NumberOfNodes_].y() = Node2.y();
       NodeList_[NumberOfNodes_].z() = Node2.z();
                 
       S_[0][1] = S_[1][1] = 0.;
       
       for ( i = 2 ; i <= NumberOfNodes()  ; i++ ) {
   
          S_[0][i] = S_[0][i-1] + sqrt( pow(NodeList_[i].x()-NodeList_[i-1].x(),2.)
                                      + pow(NodeList_[i].y()-NodeList_[i-1].y(),2.)
                                      + pow(NodeList_[i].z()-NodeList_[i-1].z(),2.) );
                                      
          S_[1][i] = S_[0][i];                            

       }  
                     
    }
    
}

/*##############################################################################
#                                                                              #
#                              VORTEX_TRAIL Setup                              #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::SizeList(int NumWakeNodes)
{

    int i;
    
    NumberOfNodes_ = NumWakeNodes;
    
    S_[0] = new double[NumberOfNodes() + 1];

    S_[1] = new double[NumberOfNodes() + 1];

    NodeList_ = new VSP_NODE[NumberOfNodes() + 1];
    
    NodeList_Time_M_1_ = new VSP_NODE[NumberOfNodes() + 1];

    GlobalNode_ = new int[NumberOfNodes() + 1];
    
    WakeResidualEquationNumber_ = new int[3*NumberOfNodes() + 1];

    GlobalEdge_ = new int[NumberOfNodes() + 1];

    EdgeList_ = new VSP_EDGE*[NumberOfNodes() + 1];

    for ( i = 1 ; i <= NumberOfNodes() ; i++ ) {
       
       EdgeList_[i] = NULL;
       
    }
     
}

/*##############################################################################
#                                                                              #
#                     VORTEX_TRAIL UpdateWakeSpacing                           #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::UpdateWakeSpacing(void)
{

    int i;
    
    S_[0][1] = S_[1][1] = 0.;
    
    for ( i = 2 ; i <= NumberOfNodes() ; i++ ) {
    
       S_[0][i] = S_[0][i-1] + sqrt( pow(NodeList_[i].x()-NodeList_[i-1].x(),2.)
                                   + pow(NodeList_[i].y()-NodeList_[i-1].y(),2.)
                                   + pow(NodeList_[i].z()-NodeList_[i-1].z(),2.) );
                                   
       S_[1][i] = S_[0][i];                            
    
    }  
    
    if ( EdgeList_ != NULL ) {
       
       for ( i = 1 ; i <= NumberOfNodes() - 2 ; i++ ) {
          
          if ( EdgeList_[i] != NULL && EdgeList_[i]->IsWakeEdge() && EdgeList_[i]->WakeEdgeData().dS() > 0. ) EdgeList_[i]->WakeEdgeData().dS() = dS(i);
    
       }
       
    }
        
}

/*##############################################################################
#                                                                              #
#                    VORTEX_TRAIL UpdateWakeLocation                           #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::UpdateWakeLocation(double WakeRelax, double &ResMax, double &ResL2)
{
 
    int i;
    double Vec[3], Mag, dS, Residual[3], Relax;
    double Vec3[3], Dir[3], Dot, Dot1, Dot2, VecTemp[3];

    // Calculate the residual, and update the wakes

    ResL2 = ResMax = 0.;
     
    Relax = 0.85*WakeRelax;

    for ( i = 1 ; i <= NumberOfNodes() - 2 ; i++ ) {
    
       Vec[0] = EdgeList_[i]->Velocity()[0];
       Vec[1] = EdgeList_[i]->Velocity()[1];
       Vec[2] = EdgeList_[i]->Velocity()[2];

       Mag = sqrt(vector_dot(Vec,Vec));

       if ( Mag > 0. ) {
          
          Vec[0] /= Mag;
          Vec[1] /= Mag;
          Vec[2] /= Mag;
          
       }
       
       dS = S_[0][i+1] - S_[0][i];

       // Only take velocities in the free stream and rotor wash directions
 
       if ( RotorAnalysis_ ) {
    
          Vec[0] = EdgeList_[i]->Velocity()[0];
          Vec[1] = EdgeList_[i]->Velocity()[1];
          Vec[2] = EdgeList_[i]->Velocity()[2];
      
          VecTemp[0] = Vec[0] - EdgeList_[i]->LocalFreeStreamVelocity(0);
          VecTemp[1] = Vec[1] - EdgeList_[i]->LocalFreeStreamVelocity(1);
          VecTemp[2] = Vec[2] - EdgeList_[i]->LocalFreeStreamVelocity(2);
      
          Dot2 = vector_dot(RotorThrustVector_,VecTemp);
      
          Dot2 = MIN(Dot2,0.);
      
          Vec[0] = EdgeList_[i]->LocalFreeStreamVelocity(0) + Dot2*RotorThrustVector_[0];
          Vec[1] = EdgeList_[i]->LocalFreeStreamVelocity(1) + Dot2*RotorThrustVector_[1];
          Vec[2] = EdgeList_[i]->LocalFreeStreamVelocity(2) + Dot2*RotorThrustVector_[2];
          
          Mag = sqrt(vector_dot(Vec,Vec));
          
          if ( Mag > 0. ) {
             
             Vec[0] /= Mag;
             Vec[1] /= Mag;
             Vec[2] /= Mag;
             
          }     
                             
       }       
               
       Residual[0] = NodeList_[i+1].x() - NodeList_[i].x() - Vec[0]*dS;
       Residual[1] = NodeList_[i+1].y() - NodeList_[i].y() - Vec[1]*dS;
       Residual[2] = NodeList_[i+1].z() - NodeList_[i].z() - Vec[2]*dS;

       // For rotor analysis we project the residual into a subspace that 
       // only allows for motion in the downwash + free stream directions
       
       if ( RotorAnalysis_ ) {
          
          Dir[0] = -RotorThrustVector_[0];
          Dir[1] = -RotorThrustVector_[1];
          Dir[2] = -RotorThrustVector_[2];
          
          Mag = sqrt(vector_dot(Dir,Dir));
          
          Dir[0] /= Mag;
          Dir[1] /= Mag;
          Dir[2] /= Mag;
                    
          Dot = vector_dot(Residual, Dir);

          Residual[0] = Dot * Dir[0];
          Residual[1] = Dot * Dir[1];
          Residual[2] = Dot * Dir[2];         
           
       }

       ResMax = MAX(ResMax,ABS(Residual[0]));
       ResMax = MAX(ResMax,ABS(Residual[1]));
       ResMax = MAX(ResMax,ABS(Residual[2]));
       
       ResL2 += Residual[0]*Residual[0]
              + Residual[1]*Residual[1]
              + Residual[2]*Residual[2];
       
       // Update
       
       NodeList_[i+1].x() -= Relax*Residual[0];
       NodeList_[i+1].y() -= Relax*Residual[1];
       NodeList_[i+1].z() -= Relax*Residual[2];
 
    }
    
//    if ( RotorAnalysis_ ) UpdateWakeSpacing();

}

/*##############################################################################
#                                                                              #
#                    VORTEX_TRAIL UpdateWakeLocation                           #
#                                                                              #
##############################################################################*/

double VORTEX_TRAIL::UpdateWakeLocationOld(double WakeRelax)
{
 
    int i;
    double Vec[3], Mag, dS, Residual[3], ResMax, Relax, Angle, Omega;
    double Dot1, Dot2, Vec3[3];

    // Calculate the residual, and update the wakes

    ResMax = 0.;
     
    Relax = 0.85*WakeRelax;

    for ( i = 1 ; i <= NumberOfNodes() - 1 ; i++ ) {
    
       Vec[0] = EdgeList_[i]->Velocity()[0];
       Vec[1] = EdgeList_[i]->Velocity()[1];
       Vec[2] = EdgeList_[i]->Velocity()[2];

       Mag = sqrt(vector_dot(Vec,Vec));

       if ( Mag > 0. ) {
          
          Vec[0] /= Mag;
          Vec[1] /= Mag;
          Vec[2] /= Mag;
          
       }
     
       dS = S_[0][i+1] - S_[0][i];
                    
       Residual[0] = NodeList_[i+1].x() - NodeList_[i].x() - Vec[0]*dS;
       Residual[1] = NodeList_[i+1].y() - NodeList_[i].y() - Vec[1]*dS;
       Residual[2] = NodeList_[i+1].z() - NodeList_[i].z() - Vec[2]*dS;

       // For rotor analysis we project the residual into a subspace that 
       // only allows for motion in the downwash + free stream directions
       
       if ( RotorAnalysis_ ) {

          // Component in direction of free stream
          
          Dot1 = vector_dot(Residual, FreeStreamDirection_);
          
//printf("Residual: %f %f %f \n",
//Residual[0],
//Residual[1],
//Residual[2]);                  
          // Apply limiter
       
          Dot1 = MIN(Dot1,0.);
       
          // Subtract out free stream contribution
       
          Residual[0] -= Dot1*FreeStreamDirection_[0];  
          Residual[1] -= Dot1*FreeStreamDirection_[1];  
          Residual[2] -= Dot1*FreeStreamDirection_[2];   
          
          // Calculate portion in direction of thrust vector
          
          Dot2 = vector_dot(Residual, RotorThrustVector_);
          
          // Apply limiter
          
          Dot2 = MAX(Dot2,0.);
          
          // Add back in free limited free stream direction, 
          // and limited rotor thrust direction contributions
                
          Residual[0] = Dot1*FreeStreamDirection_[0] + Dot2*RotorThrustVector_[0];
          Residual[1] = Dot1*FreeStreamDirection_[1] + Dot2*RotorThrustVector_[1];  
          Residual[2] = Dot1*FreeStreamDirection_[2] + Dot2*RotorThrustVector_[2];   
               
       }
    
       ResMax = MAX(ResMax,ABS(Residual[0]));
       ResMax = MAX(ResMax,ABS(Residual[1]));
       ResMax = MAX(ResMax,ABS(Residual[2]));

//printf("j: %d --> Explicit dx,dy,dz,dS: %f %f %f %f --> Vec: %f %f %f --> Dot1: %f ... Dot2:%f ... ResMax: %f \n",
//i,
//NodeList_[i+1].x() - NodeList_[i].x(),
//NodeList_[i+1].y() - NodeList_[i].y(),
//NodeList_[i+1].z() - NodeList_[i].z(),
//dS,
//Vec[0],Vec[1],Vec[2],Dot1,Dot2,ResMax);

       NodeList_[i+1].x() -= Relax*Residual[0];
       NodeList_[i+1].y() -= Relax*Residual[1];
       NodeList_[i+1].z() -= Relax*Residual[2];
       
    }
    
    if ( RotorAnalysis_ ) UpdateWakeSpacing();
  
    return ResMax;

}

/*##############################################################################
#                                                                              #
#                      VORTEX_TRAIL StoreWakeLocation                          #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::StoreWakeLocation(void)
{
    
    int i;
    
    for ( i = 2 ; i <= NumberOfNodes() ; i++ ) {
   
       NodeList_Time_M_1_[i].x() = NodeList_[i-1].x();
       NodeList_Time_M_1_[i].y() = NodeList_[i-1].y();
       NodeList_Time_M_1_[i].z() = NodeList_[i-1].z();
       
    }   

    if ( EdgeList_ != NULL ) {
       
       for ( i = 1 ; i <= NumberOfNodes() - 1 ; i++ ) {
          
          if ( EdgeList_[i] != NULL && EdgeList_[i]->IsWakeEdge() ) {
             
             EdgeList_[i]->StoreWakeLocation();
             
          }
    
       }
       
    }
        
}

/*##############################################################################
#                                                                              #
#                 VORTEX_TRAIL UpdateUnsteadyWakeLocation                      #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::UpdateUnsteadyWakeLocation(double WakeRelax, double &ResMax, double &ResL2)
{
 
    int i, iMax;
    double dx, dy, dz;
    double Vec[3], Dot;
    
    ResMax = ResL2 = 0.;
    
    iMax = MIN(Time_,  NumberOfNodes() - 1);

    for ( i = 2 ; i <= iMax + 1 ; i++ ) {
       
       // Full, unsteady time step using full velocity field

       if ( IsARotor_ == 0 || IsARotor_ == 2 ) {

          Vec[0] = EdgeList_[i-1]->Velocity()[0];
          Vec[1] = EdgeList_[i-1]->Velocity()[1];
          Vec[2] = EdgeList_[i-1]->Velocity()[2];
      
       }
       
       // Project induced velocity onto the thrust direction... ie we solve
       // an approximate subset of the full 3dof equations... yes it's cheating,
       // but it works ;-) 
       
       else {
     
          // Subtract out free stream.. this should probably be more detailed
          //... since there could be other 'free stream' stuff

          Vec[0] = EdgeList_[i-1]->Velocity()[0] - EdgeList_[i-1]->LocalFreeStreamVelocity(0);
          Vec[1] = EdgeList_[i-1]->Velocity()[1] - EdgeList_[i-1]->LocalFreeStreamVelocity(1);
          Vec[2] = EdgeList_[i-1]->Velocity()[2] - EdgeList_[i-1]->LocalFreeStreamVelocity(2);

          // Determine component in thrust direction

          Dot = vector_dot(Vec,EdgeList_[i-1]->WakeEdgeData().RotorThrustVector());

          // Limiter... we only take corrections in minus the thrust direction
          
          Dot = MIN(Dot,0.);
          
          // Add back in free stream to velocities determined above
          
          Vec[0] = EdgeList_[i-1]->LocalFreeStreamVelocity(0) + Dot * EdgeList_[i-1]->WakeEdgeData().RotorThrustVector(0);
          Vec[1] = EdgeList_[i-1]->LocalFreeStreamVelocity(1) + Dot * EdgeList_[i-1]->WakeEdgeData().RotorThrustVector(1);
          Vec[2] = EdgeList_[i-1]->LocalFreeStreamVelocity(2) + Dot * EdgeList_[i-1]->WakeEdgeData().RotorThrustVector(2);
          
       }
       
       // Take a time step

       dx = NodeList_Time_M_1_[i].x() + Vec[0] * DeltaTime_ - NodeList_[i].x();
       dy = NodeList_Time_M_1_[i].y() + Vec[1] * DeltaTime_ - NodeList_[i].y();
       dz = NodeList_Time_M_1_[i].z() + Vec[2] * DeltaTime_ - NodeList_[i].z();
 
       NodeList_[i].x() += WakeRelax * dx;
       NodeList_[i].y() += WakeRelax * dy;
       NodeList_[i].z() += WakeRelax * dz;       
       
       ResMax = MAX(ResMax, ABS(dx));
       ResMax = MAX(ResMax, ABS(dy));
       ResMax = MAX(ResMax, ABS(dz));
       
       ResL2 += dx*dx + dy*dy + dz*dz;
       
    }

}

/*##############################################################################
#                                                                              #
#                            VORTEX_TRAIL GammaScale                           #
#                                                                              #
##############################################################################*/

double VORTEX_TRAIL::GammaScale(int i)
{
   
//   double dS_old, dS_new, Scale;
//   
//   return 1;
//   
//   if ( i == 1 ) return 1.;
//   
//   dS_new = S_[0][i+1] - S_[0][i  ]; // New spacing
//   dS_old = S_[1][i  ] - S_[1][i-1]; // Old spacing
//
//   Scale = 1.;
//   
//   if ( dS_new > 0. ) Scale = dS_old / dS_new;
//   
// //  Scale = MAX(0.,MIN(Scale,1.));
//   
//   return Scale;

     return 1;

}

/*##############################################################################
#                                                                              #
#                          VORTEX_TRAIL WriteToFile                            #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::WriteToFile(FILE *adb_file)
{
 
    int i, n, i_size, c_size, d_size;
    double x, y, z, s;
    
    // Sizeof int and double

    i_size = sizeof(int);
    c_size = sizeof(char);
    d_size = sizeof(double);

    n = NumberOfNodes();

    if ( TimeAccurate_ ) n = MIN( Time_ + 1, NumberOfNodes());

    if ( TE_Node_Region_Is_Concave_ ) n = 1;
    
    fwrite(&(TE_Node_), i_size, 1, adb_file);  // Kutta node
        
    s = double (SoverB_);
    
    fwrite(&(s), d_size, 1, adb_file); // S over B (span) 
    
    fwrite(&(n), i_size, 1, adb_file); // Number of nodes

    for ( i = 1 ; i <= n ; i++ ) {

       x = double (NodeList_[i].x());
       y = double (NodeList_[i].y());
       z = double (NodeList_[i].z());

       fwrite(&(x), d_size, 1, adb_file);
       fwrite(&(y), d_size, 1, adb_file);
       fwrite(&(z), d_size, 1, adb_file);

    }

}

/*##############################################################################
#                                                                              #
#                          VORTEX_TRAIL ReadInFile                             #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::ReadInFile(FILE *adb_file)
{
 
    int i, j, k, m, n, NumMaxNodes, Level, i_size, c_size, d_size, DumInt;
    double x, y, z, s;
    double dS;
    VSP_NODE NodeA, NodeB;
        
    // Sizeof int and double

    i_size = sizeof(int);
    c_size = sizeof(char);
    d_size = sizeof(double);

    fread(&(DumInt), i_size, 1, adb_file);     

    fread(&(s), d_size, 1, adb_file);
    
    fread(&(n), i_size, 1, adb_file);

    for ( i = 1 ; i <= n ; i++ ) {

       fread(&(x), d_size, 1, adb_file);
       fread(&(y), d_size, 1, adb_file);
       fread(&(z), d_size, 1, adb_file);

       NodeList_[i].x() = x;
       NodeList_[i].y() = y;
       NodeList_[i].z() = z;
 
    }

    NumMaxNodes = NumberOfNodes();
    
    if ( TimeAccurate_ ) NumMaxNodes = MIN(Time_, NumberOfNodes());

    for ( i = 0 ; i <= NumMaxNodes ; i++ ) {

       S_[1][i] = S_[0][i];
            
    }   
 
    S_[0][1] = 0.;
    
    for ( i = 2 ; i <= NumMaxNodes ; i++ ) {

       dS = sqrt( (NodeList_[i].x()-NodeList_[i-1].x())*(NodeList_[i].x()-NodeList_[i-1].x())
                + (NodeList_[i].y()-NodeList_[i-1].y())*(NodeList_[i].y()-NodeList_[i-1].y())
                + (NodeList_[i].z()-NodeList_[i-1].z())*(NodeList_[i].z()-NodeList_[i-1].z()) );

       S_[0][i] = S_[0][i-1] + dS;
    
    }      
 
}

/*##############################################################################
#                                                                              #
#                          VORTEX_TRAIL SkipReadInFile                         #
#                                                                              #
##############################################################################*/

void VORTEX_TRAIL::SkipReadInFile(FILE *adb_file)
{
 
    int i, n, i_size, c_size, d_size;

    int DumInt;
    double DumDouble;
      
    // Sizeof int and double

    i_size = sizeof(int);
    c_size = sizeof(char);
    d_size = sizeof(double);

    fread(&DumInt, i_size, 1, adb_file);     
    
    fread(&(DumDouble), d_size, 1, adb_file);
     
    fread(&n, i_size, 1, adb_file);

    for ( i = 1 ; i <= n ; i++ ) {

       fread(&DumDouble, d_size, 1, adb_file);
       fread(&DumDouble, d_size, 1, adb_file);
       fread(&DumDouble, d_size, 1, adb_file);

    }

}

#include "END_NAME_SPACE.H"
