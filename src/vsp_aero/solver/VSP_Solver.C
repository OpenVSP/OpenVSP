//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Solver.H"

/*##############################################################################
#                                                                              #
#                                VSP_SOLVER constructor                        #
#                                                                              #
##############################################################################*/

VSP_SOLVER::VSP_SOLVER(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                   VSP_SOLVER init                            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::init(void)
{

    Verbose_ = 0;
    
    FirstTimeSetup_ = 1;
    
    FirstTimeSolve_ = 1;
    
    DoSymmetryPlaneSolve_ = 0;
    
    SetFarFieldDist_ = 0;
    
    NumberOfWakeTrailingNodes_ = 64; // Must be a power of 2
    
    SearchID_ = 0;
    
    SaveRestartFile_ = 0;
    
    JacobiRelaxationFactor_ = 0.90;
    
    DumpGeom_ = 0;
    
    ForceType_ = 0;
    
    AveragingIteration_ = 0;
    
    NumberOfAveragingSteps_ = 0;
    
    NoWakeIteration_ = 0;

}

/*##############################################################################
#                                                                              #
#                                  VSP_SOLVER Copy                             #
#                                                                              #
##############################################################################*/

VSP_SOLVER::VSP_SOLVER(const VSP_SOLVER &Solver)
{

    init();

    // Just * use the operator = code

    *this = Solver;

}

/*##############################################################################
#                                                                              #
#                                 VSP_SOLVER operator=                         #
#                                                                              #
##############################################################################*/

VSP_SOLVER& VSP_SOLVER::operator=(const VSP_SOLVER &MGSolver)
{

    printf("VSP_SOLVER operator= not implemented! \n");
    exit(1);
    
    return *this;

}

/*##############################################################################
#                                                                              #
#                              VSP_SOLVER destructor                           #
#                                                                              #
##############################################################################*/

VSP_SOLVER::~VSP_SOLVER(void)
{


}

/*##############################################################################
#                                                                              #
#                                   VSP_SOLVER Setup                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Setup(void)
{
 
    int i, j, NumberOfStations;
    double Area;
    
    // Copy over data
    
    MGLevel_ = 1;
    
    NumberOfMGLevels_ = VSPGeom().NumberOfGridLevels() - 1;
   
    // Size the lists and determine the size of the domain
    
    Xmin_ = 1.e9;
    Xmax_ = -Xmin_;

    Ymin_ = 1.e9;
    Ymax_ = -Ymin_;

    Zmin_ = 1.e9;
    Zmax_ = -Zmin_;
    
    NumberOfSurfaceVortexEdges_ = NumberOfTrailingVortexEdges_ = NumberOfVortexLoops_ = 0;
    
    Size_Lists();

    // Allocate space for the vortex edges and loops
  
    SurfaceVortexEdge_ = new VSP_EDGE*[NumberOfSurfaceVortexEdges_ + 1];
    
    VortexLoop_ = new VSP_LOOP*[NumberOfVortexLoops_ + 1];

    TrailingVortexEdge_ = new VORTEX_TRAIL[NumberOfTrailingVortexEdges_ + 1];
    
    Gamma_ = new double[NumberOfVortexLoops_ + 1];    
    
    Residual_ = new double[NumberOfVortexLoops_ + 1];    

    RightHandSide_ = new double[NumberOfVortexLoops_ + 1];     
 
    Diagonal_ = new double[NumberOfVortexLoops_ + 1];     
    
    GammaOld_= new double[NumberOfVortexLoops_ + 1];     
    
    Delta_= new double[NumberOfVortexLoops_ + 1];     
   
    zero_double_array(Gamma_,         NumberOfVortexLoops_); Gamma_[0]         = 0.;
    zero_double_array(Residual_,      NumberOfVortexLoops_); Residual_[0]      = 0.;
    zero_double_array(RightHandSide_, NumberOfVortexLoops_); RightHandSide_[0] = 0.;
    zero_double_array(Diagonal_,      NumberOfVortexLoops_); Diagonal_[0]      = 0.;
    zero_double_array(GammaOld_,      NumberOfVortexLoops_); GammaOld_[0]      = 0.;
    zero_double_array(Delta_,         NumberOfVortexLoops_); Delta_[0]         = 0.;
  
    LocalFreeStreamVelocity_ =  new double*[NumberOfVortexLoops_ + 1];  
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       LocalFreeStreamVelocity_[i] = new double[5];
       
    }    
    
    // Allocate space for span loading data
        
    Span_Cx_  = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cy_  = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cz_  = new double*[VSPGeom().NumberOfSurfaces() + 1];

    Span_Cn_  = new double*[VSPGeom().NumberOfSurfaces() + 1];

    Span_Cl_  = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cs_  = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cd_  = new double*[VSPGeom().NumberOfSurfaces() + 1];
    
    Span_Cmx_ = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmy_ = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmz_ = new double*[VSPGeom().NumberOfSurfaces() + 1];    
    
    Span_Yavg_ = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Area_ = new double*[VSPGeom().NumberOfSurfaces() + 1];
    
    Local_Vel_ = new double*[VSPGeom().NumberOfSurfaces() + 1];
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
        
       NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
 
       Span_Cx_[i]   = new double[NumberOfStations + 1];

       Span_Cy_[i]   = new double[NumberOfStations + 1];

       Span_Cz_[i]   = new double[NumberOfStations + 1];

       Span_Cmx_[i]  = new double[NumberOfStations + 1];

       Span_Cmy_[i]  = new double[NumberOfStations + 1];

       Span_Cmz_[i]  = new double[NumberOfStations + 1];
       
       Span_Cn_[i]   = new double[NumberOfStations + 1];
     
       Span_Cl_[i]   = new double[NumberOfStations + 1];

       Span_Cs_[i]   = new double[NumberOfStations + 1];
       
       Span_Cd_[i]   = new double[NumberOfStations + 1];
                     
       Span_Yavg_[i] = new double[NumberOfStations + 1];
       
       Span_Area_[i] = new double[NumberOfStations + 1];
       
       Local_Vel_[i] = new double[NumberOfStations + 1];
 
       zero_double_array(Span_Cx_[i],   NumberOfStations);
       zero_double_array(Span_Cy_[i],   NumberOfStations);
       zero_double_array(Span_Cz_[i],   NumberOfStations);      
 
       zero_double_array(Span_Cn_[i],   NumberOfStations);       

       zero_double_array(Span_Cl_[i],   NumberOfStations);
       zero_double_array(Span_Cs_[i],   NumberOfStations);
       zero_double_array(Span_Cd_[i],   NumberOfStations);      
       
       zero_double_array(Span_Cmx_[i],  NumberOfStations);
       zero_double_array(Span_Cmy_[i],  NumberOfStations);
       zero_double_array(Span_Cmz_[i],  NumberOfStations);       
        
       zero_double_array(Span_Yavg_[i], NumberOfStations);       
       zero_double_array(Span_Area_[i], NumberOfStations);
       
       zero_double_array(Local_Vel_[i], NumberOfStations);
     
    }    

    // Create vortex loop list

    Setup_VortexLoops();

    // Create vortex edge list
 
    Setup_VortexEdges();

    // Calculate average vortex edge spacing
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       Area = VortexLoop(SurfaceVortexEdge(j).VortexLoop1()).Area()
            + VortexLoop(SurfaceVortexEdge(j).VortexLoop2()).Area();
     
       SurfaceVortexEdge(j).LocalSpacing() = sqrt(0.5*(Area));
       
    }  
                   
    // Zero out stuff on this grid 

    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; j++ ) {

       VSPGeom().Grid(MGLevel_).LoopList(j).Gamma() = 0.;

    }
    
    // Create interaction list

    CreateSurfaceVorticesInteractionList();
    
}

/*##############################################################################
#                                                                              #
#                               VSP_SOLVER Size_Lists                          #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Size_Lists(void)
{
    
    int j;

    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; j++ ) {

       Xmin_ = MIN(Xmin_,VSPGeom().Grid(MGLevel_).LoopList(j).Xc());
       Xmax_ = MAX(Xmax_,VSPGeom().Grid(MGLevel_).LoopList(j).Xc());

       Ymin_ = MIN(Ymin_,VSPGeom().Grid(MGLevel_).LoopList(j).Yc());
       Ymax_ = MAX(Ymax_,VSPGeom().Grid(MGLevel_).LoopList(j).Yc());

       Zmin_ = MIN(Zmin_,VSPGeom().Grid(MGLevel_).LoopList(j).Zc());
       Zmax_ = MAX(Zmax_,VSPGeom().Grid(MGLevel_).LoopList(j).Zc());

    }

    // Allocate space for the vortex edges

    NumberOfSurfaceVortexEdges_  = VSPGeom().Grid(MGLevel_).NumberOfEdges();
    
    NumberOfTrailingVortexEdges_ = VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes();
    
    NumberOfVortexLoops_         = VSPGeom().Grid(MGLevel_).NumberOfLoops();
    
    printf("Number Of Trailing Vortices: %d \n",NumberOfTrailingVortexEdges_);

}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER Setup_VortexLoops                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Setup_VortexLoops(void)
{
    
    int j, Level;

    // Set up the Vortex Loops... 

    for ( Level = 1 ; Level <= NumberOfMGLevels_ ; Level++ ) {

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfLoops() ; j++ ) {
   
          VSPGeom().Grid(Level).LoopList(j).VortexLoop() = j;
          
       }
       
    }
       
    for ( j = 1 ; j <= VSPGeom().Grid(1).NumberOfLoops() ; j++ ) {

       VortexLoop_[j] = &(VSPGeom().Grid(1).LoopList(j));
       
    }
       
    // Set up a dummy loop at 0
    
    VortexLoop_[0] = new VSP_LOOP;
   
}    

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER Setup_VortexEdges                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Setup_VortexEdges(void)
{
    
    int j, Node1, Node2, LoopL, LoopR, VortexLoop1, VortexLoop2, Level;

    VSP_NODE VSP_Node1, VSP_Node2;
    
    // Setup the surface vortex edge data
    
    for ( Level = 1 ; Level <= NumberOfMGLevels_ ; Level++ ) {

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfEdges() ; j++ ) {
       
          // Pointer from grid edge to vortex edge
      
          VSPGeom().Grid(Level).EdgeList(j).VortexEdge() = j;
          
          // Pass in edge data and create edge cofficients
          
          Node1 = VSPGeom().Grid(Level).EdgeList(j).Node1();
          Node2 = VSPGeom().Grid(Level).EdgeList(j).Node2();

          VSPGeom().Grid(Level).EdgeList(j).Setup(VSPGeom().Grid(Level).NodeList(Node1),
                                                  VSPGeom().Grid(Level).NodeList(Node2));
           
          LoopL = VSPGeom().Grid(Level).EdgeList(j).LoopL();
          LoopR = VSPGeom().Grid(Level).EdgeList(j).LoopR();

          VortexLoop1 = VortexLoop2 = 0;

          if ( LoopL > 0 ) VortexLoop1 = VSPGeom().Grid(Level).LoopList(LoopL).VortexLoop();
          if ( LoopR > 0 ) VortexLoop2 = VSPGeom().Grid(Level).LoopList(LoopR).VortexLoop();

          // Keep track of the two bounding vortex loops

          VSPGeom().Grid(Level).EdgeList(j).VortexLoop1() = VortexLoop1;
          VSPGeom().Grid(Level).EdgeList(j).VortexLoop2() = VortexLoop2;                                    

       }
       
    }
    
    for ( j = 1 ; j <= VSPGeom().Grid(1).NumberOfEdges() ; j++ ) {
    
       SurfaceVortexEdge_[j] = &(VSPGeom().Grid(1).EdgeList(j));
       
    }
    
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER InitializeFreeStream                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::InitializeFreeStream(void)
{
 
    int i, j;
    double xyz[3], q[5], CA, SA, CB, SB, Rate_P, Rate_Q, Rate_R;
    VSP_NODE VSP_Node1, VSP_Node2;
    
    // Free stream velocity vector

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
    
    FreeStreamVelocity_[0] = CA*CB;
    FreeStreamVelocity_[1] =   -SB;
    FreeStreamVelocity_[2] = SA*CB;

    // Calculate local free stream conditions
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       LocalFreeStreamVelocity_[i][0] = FreeStreamVelocity_[0];
       LocalFreeStreamVelocity_[i][1] = FreeStreamVelocity_[1];
       LocalFreeStreamVelocity_[i][2] = FreeStreamVelocity_[2];
       LocalFreeStreamVelocity_[i][3] = 0.;  // Delta-P
       LocalFreeStreamVelocity_[i][4] = 1.;  // Local free stream velocity magnitude
       
    }        
       
    // Rotational rates... note minus signs come from difference in VSPAERO axes and 
    // the traditional stability axes. Also note... there's another sign floating around
    // since we want the obvserved free stream velocity, not the velocity of the body...
    
    Rate_P = -RotationalRate_[0] / Vinf_;
    Rate_Q =  RotationalRate_[1] / Vinf_;
    Rate_R = -RotationalRate_[2] / Vinf_;
       
    // Add in rotational velocities

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       xyz[0] = VortexLoop(i).Xc() - Xcg();            
       xyz[1] = VortexLoop(i).Yc() - Ycg();           
       xyz[2] = VortexLoop(i).Zc() - Zcg();      

       // P - Roll
       
       LocalFreeStreamVelocity_[i][0] += 0.;
       LocalFreeStreamVelocity_[i][1] += +xyz[2] * Rate_P;
       LocalFreeStreamVelocity_[i][2] += -xyz[1] * Rate_P;
        
       // Q - Pitch

       LocalFreeStreamVelocity_[i][0] += -xyz[2] * Rate_Q;
       LocalFreeStreamVelocity_[i][1] += 0.;
       LocalFreeStreamVelocity_[i][2] += +xyz[0] * Rate_Q;

       // R - Yaw
       
       LocalFreeStreamVelocity_[i][0] +=  xyz[1] * Rate_R;
       LocalFreeStreamVelocity_[i][1] += -xyz[0] * Rate_R;
       LocalFreeStreamVelocity_[i][2] += 0.;
    
    }  
    
    // Add in rotor induced velocities

    for ( j = 1 ; j <= NumberOfRotors_ ; j++ ) {
     
     RotorDisk(j).Density() = Density_;
 
     RotorDisk(j).Vinf(0) = FreeStreamVelocity_[0] * Vinf_;
     RotorDisk(j).Vinf(1) = FreeStreamVelocity_[1] * Vinf_;
     RotorDisk(j).Vinf(2) = FreeStreamVelocity_[2] * Vinf_;
     
    }
        
    for ( j = 1 ; j <= NumberOfRotors_ ; j++ ) {
     
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

          xyz[0] = VortexLoop(i).Xc();            
          xyz[1] = VortexLoop(i).Yc();           
          xyz[2] = VortexLoop(i).Zc();
       
          RotorDisk(j).Velocity(xyz, q);                   

          LocalFreeStreamVelocity_[i][0] += q[0] / Vinf_;
          LocalFreeStreamVelocity_[i][1] += q[1] / Vinf_;
          LocalFreeStreamVelocity_[i][2] += q[2] / Vinf_;
          LocalFreeStreamVelocity_[i][3] += q[3];
          LocalFreeStreamVelocity_[i][4] += q[4] / Vinf_;
          
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {

             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
            
             RotorDisk(j).Velocity(xyz, q);      
   
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
            
             LocalFreeStreamVelocity_[i][0] += q[0] / Vinf_;
             LocalFreeStreamVelocity_[i][1] += q[1] / Vinf_;
             LocalFreeStreamVelocity_[i][2] += q[2] / Vinf_;
             LocalFreeStreamVelocity_[i][3] += q[3];
             LocalFreeStreamVelocity_[i][4] += q[4] / Vinf_;
               
          }             
          
       }    
       
    }

}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER InitializeTrailingVortices                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::InitializeTrailingVortices(void)
{
 
    int i, j, NumEdges, Node1, Node2, Loop;
    double FarDist, *Sigma, *VecX, *VecY, *VecZ, Vec[3];
    double Scale_X, Scale_Y, Scale_Z;
    VSP_NODE VSP_Node1, VSP_Node2;
    
    // Trail initial wake, and the Trefftz flat wake with free stream
    
    if ( Mach_ < 1. ) {
     
       WakeAngle_[0] = FreeStreamVelocity_[0];
       WakeAngle_[1] = FreeStreamVelocity_[1];
       WakeAngle_[2] = FreeStreamVelocity_[2];
       
    }
    
    // Trail initial wake, and the Trefftz flat in x-direction... this is done more to guarantee that the
    // flat Trefftz wake lines within the Mach cone of the Trefftz integration line... as the final wake is 
    // allowed to move if user does wake relaxation...
    
    else {
     
       WakeAngle_[0] = 1.;
       WakeAngle_[1] = 0.;
       WakeAngle_[2] = 0.;
     
    }       

    // Determine how far to allow wakes to adapt... beyond this the wakes go straight off to
    // 'infinity' in the free stream direction
    
    Scale_X = Scale_Y = Scale_Z = 1.;
    
    if ( DoSymmetryPlaneSolve_ == SYM_X) Scale_X = 2.;
    if ( DoSymmetryPlaneSolve_ == SYM_Y) Scale_Y = 2.;
    if ( DoSymmetryPlaneSolve_ == SYM_Z) Scale_Z = 2.;
    
    FarDist = MAX3(3.*Scale_X*(Xmax_ - Xmin_), 2.*Scale_Y*(Ymax_-Ymin_), 2.*Scale_Z*(Zmax_-Zmin_));
    
    printf("Xmax_ - Xmin_: %f \n",Xmax_ - Xmin_);
    printf("Ymax_ - Ymin_: %f \n",Ymax_ - Ymin_);
    printf("Zmax_ - Zmin_: %f \n",Zmax_ - Zmin_);
    printf("\n");
    
    // Override far field distance
    
    if ( SetFarFieldDist_ ) FarDist = FarFieldDist_;

    printf("Wake FarDist set to: %f \n",FarDist);
    printf("\n");
 
    // Determine the minimum trailing edge spacing for edge kutta node

    Sigma = new double[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];

    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfNodes() ; j++ ) {

       Sigma[j] = 1.e9;
       
    }
  
    for ( i = 1 ; i <= VSPGeom().Grid(MGLevel_).NumberOfEdges() ; i++ ) {
     
       if ( VSPGeom().Grid(MGLevel_).EdgeList(i).IsTrailingEdge() ) {     

          Node1 = VSPGeom().Grid(MGLevel_).EdgeList(i).Node1();
          Node2 = VSPGeom().Grid(MGLevel_).EdgeList(i).Node2();
          
          Sigma[Node1] = MIN(Sigma[Node1], VSPGeom().Grid(MGLevel_).EdgeList(i).Length());
          Sigma[Node2] = MIN(Sigma[Node2], VSPGeom().Grid(MGLevel_).EdgeList(i).Length());
          
       }
       
    } 
    
    // Determine trailing edge angle
    
    VecX = new double[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    VecY = new double[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    VecZ = new double[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    
    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfNodes() ; j++ ) {

       VecX[j] = 0.;
       VecY[j] = 0.;
       VecZ[j] = 0.;
       
    }
    
    for ( i = 1 ; i <= VSPGeom().Grid(MGLevel_).NumberOfEdges() ; i++ ) {
     
       if ( VSPGeom().Grid(MGLevel_).EdgeList(i).IsTrailingEdge() ) { 
        
          // Determine trailing edge loop
          
          if ( VSPGeom().Grid(MGLevel_).EdgeList(i).Loop1() != 0 ) Loop = VSPGeom().Grid(MGLevel_).EdgeList(i).Loop1();
          if ( VSPGeom().Grid(MGLevel_).EdgeList(i).Loop2() != 0 ) Loop = VSPGeom().Grid(MGLevel_).EdgeList(i).Loop2();
          
          vector_cross(VSPGeom().Grid(MGLevel_).EdgeList(i).Vec(), VSPGeom().Grid(MGLevel_).LoopList(Loop).Normal(), Vec);

          Node1 = VSPGeom().Grid(MGLevel_).EdgeList(i).Node1();
          Node2 = VSPGeom().Grid(MGLevel_).EdgeList(i).Node2();
          
          if ( Loop == VSPGeom().Grid(MGLevel_).EdgeList(i).LoopL() ) {
           
             VecX[Node1] += Vec[0];
             VecY[Node1] += Vec[1];
             VecZ[Node1] += Vec[2];
             
             VecX[Node2] += Vec[0];
             VecY[Node2] += Vec[1];
             VecZ[Node2] += Vec[2];
             
          }
          
          else {
           
             VecX[Node1] -= Vec[0];
             VecY[Node1] -= Vec[1];
             VecZ[Node1] -= Vec[2];
             
             VecX[Node2] -= Vec[0];
             VecY[Node2] -= Vec[1];
             VecZ[Node2] -= Vec[2];
             
          }           

       }
       
    }    
       
    NumEdges = 0;
  
    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {
       
       NumEdges++;

       TrailingVortexEdge(NumEdges).Wing() = i;
      
       TrailingVortexEdge(NumEdges).Node() = VSPGeom().Grid(MGLevel_).KuttaNode(j);
                   
       // Pass in edge data and create edge cofficients
       
       VSP_Node1.x() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeX(j);
       VSP_Node1.y() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeY(j);
       VSP_Node1.z() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeZ(j);
              
       VSP_Node2.x() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeX(j) + WakeAngle_[0] * 1.e6;
       VSP_Node2.y() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeY(j) + WakeAngle_[1] * 1.e6;
       VSP_Node2.z() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeZ(j) + WakeAngle_[2] * 1.e6;
 
       // Set sigma
  
       TrailingVortexEdge(NumEdges).Sigma() = 0.1*Sigma[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
       
       // Set trailing edge direction 
       
       TrailingVortexEdge(NumEdges).TEVec(0) = VecX[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
       TrailingVortexEdge(NumEdges).TEVec(1) = VecY[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
       TrailingVortexEdge(NumEdges).TEVec(2) = VecZ[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
       
       // Create trailing wakes... specify number of sub vortices per trail

       TrailingVortexEdge(NumEdges).Setup(NumberOfWakeTrailingNodes_,FarDist,VSP_Node1,VSP_Node2);
        
    }
    
    delete [] Sigma;
    delete [] VecX;
    delete [] VecY;
    delete [] VecZ;

}

/*##############################################################################
#                                                                              #
#                               VSP_SOLVER Solve                               #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Solve(int Case)
{
 
    int i, Level;
    char StatusFileName[2000];
   
    // Initialize free stream
    
    InitializeFreeStream();
    
    // Initialize the wake trailing vortices
    
    InitializeTrailingVortices();
    
    // Update righthandside, Mach dependence 
        
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       RightHandSide_[i] = -vector_dot(VortexLoop(i).Normal(), LocalFreeStreamVelocity_[i]);

    }
    
    for ( Level = 1 ; Level <= NumberOfMGLevels_ ; Level++ ) {
 
       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {

          VSPGeom().Grid(Level).EdgeList(i).Mach() = Mach_;
          
       }
       
    }

    for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {

       TrailingVortexEdge(i).Mach() = Mach_;

    }
        
    // Do a restart
    
    if ( DoRestart_ == 1 ) {
     
       LoadRestartFile();
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

           VortexLoop(i).Gamma() = Gamma_[i];
    
        }
        
    }
    
    // Open status file
    
    if ( Case == 0 || Case == 1 ) {
       
       sprintf(StatusFileName,"%s.history",FileName_);
       
       if ( (StatusFile_ = fopen(StatusFileName, "w")) == NULL ) {
   
          printf("Could not open the history file for output! \n");
   
          exit(1);
   
       }    
       
    }

    // Header for history file
    
    if ( ABS(Case) > 0 ) {
       
       fprintf(StatusFile_,"\n\nSolver Case: %d \n\n",ABS(Case));
       
    }
    
                       //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789   
    fprintf(StatusFile_,"  Iter      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS \n");

    printf("Solving... \n\n");fflush(NULL);

    if ( DumpGeom_ ) WakeIterations_ = 0;
    
    for ( CurrentWakeIteration_ = 1 ; CurrentWakeIteration_ <= WakeIterations_ ; CurrentWakeIteration_++ ) {
   
       // Solve the linear system
       
       SolveLinearSystem();
 
       // Update wake locations

       UpdateWakeLocations();

       // Calculate forces

       CalculateForces();
      
       // Output status

       OutputStatusFile(0);
       
       printf("\n");
       
    }
    
    if ( ForceType_ == FORCE_AVERAGE ) OutputStatusFile(1);
     
    OutputZeroLiftDragToStatusFile();
    
    if ( Case <= 0 ) fclose(StatusFile_);

    // Calculate spanwise load distributions for lifting surfaces
    
    CalculateSpanWiseLoading();
        
    // Interpolate solution from grid 1 to 0
    
    InterpolateSolutionFromGrid(1);
           
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER SolveLinearSystem                             #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::SolveLinearSystem(void)
{
    
    // First time... calculate matrix diagonal

    if ( FirstTimeSolve_ ) {
    
       CalculateDiagonal();       
       
       FirstTimeSolve_ = 0;
       
    }

    // Solver the linear system

    Do_GMRES_Solve();    

    // Update the vortex strengths on the wake
 
    UpdateVortexEdgeStrengths(1);
    
    if ( SaveRestartFile_ ) WriteRestartFile();
    
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER CalculateDiagonal                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateDiagonal(void)
{

    int i, j;
    double q[4], Ws;
    
    zero_double_array(Diagonal_,NumberOfVortexLoops_);
     
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       // Loop 1
       
       SurfaceVortexEdge(j).Gamma() = 1.;
       
       i = SurfaceVortexEdge(j).VortexLoop1();

       SurfaceVortexEdge(j).InducedVelocity(VortexLoop(i).xyz_c(), q);

       Diagonal_[i] += vector_dot(VortexLoop(i).Normal(), q);
       
       // Loop 2
       
       SurfaceVortexEdge(j).Gamma() = -1.;
       
       i = SurfaceVortexEdge(j).VortexLoop2();
     
       SurfaceVortexEdge(j).InducedVelocity(VortexLoop(i).xyz_c(), q);
  
       Diagonal_[i] += vector_dot(VortexLoop(i).Normal(), q);
       
    }
 
    // If flow is supersonic add in generalized principart part of downwash
    
    if ( Mach_ > 1. ) {

       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

          Ws = SurfaceVortexEdge(j).GeneralizedPrincipalPartOfDownWash();
        
          // Loop 1
           
          if ( SurfaceVortexEdge(j).VortexLoop1IsDownWind() ) {
           
             i = SurfaceVortexEdge(j).VortexLoop1();
        
             Diagonal_[i] += Ws * SurfaceVortexEdge(j).VortexLoop1DownWindWeight();
       
          }

          // Loop 2

          if ( SurfaceVortexEdge(j).VortexLoop2IsDownWind() ) {
           
             i = SurfaceVortexEdge(j).VortexLoop2();

             Diagonal_[i] += Ws * SurfaceVortexEdge(j).VortexLoop2DownWindWeight();
       
          }

       }
       
    }
  
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       if ( ABS(Diagonal_[i]) == 0. ) {
        
          if ( VortexLoop(i).SurfaceType() == BODY_SURFACE ) printf("Loop: %d on body surface: %d has zero diagonal... Mach is: %lf \n",i,VortexLoop(i).BodyID(),Mach_);fflush(NULL);
          if ( VortexLoop(i).SurfaceType() == WING_SURFACE ) printf("Loop: %d on wing surface: %d has zero diagonal... Mach is: %lf \n",i,VortexLoop(i).WingID(),Mach_);fflush(NULL);
          
       }
     
       Diagonal_[i] = 1./Diagonal_[i];
        
    }    

}

/*##############################################################################
#                                                                              #
#                 VSP_SOLVER DoPreconditionedMatrixMultiply                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DoPreconditionedMatrixMultiply(double *vec_in, double *vec_out)
{
 
    DoMatrixMultiply(vec_in, vec_out);
    
    DoMatrixPrecondition(vec_out);
 
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER DoMatrixMultiply                            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DoMatrixMultiply(double *vec_in, double *vec_out)
{

    int i, j, Level;
    double xyz[3], q[4], Ws, Temp;
    VSP_EDGE *VortexEdge;
  
    zero_double_array(vec_out,NumberOfVortexLoops_);
    
    Gamma_[0] = 0.;
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       Gamma_[i] = vec_in[i];
      
    }

    UpdateVortexEdgeStrengths(1);
              
    // Surface vortex induced velocities

    for ( Level = 1 ; Level < NumberOfMGLevels_ ; Level++ ) {
       
       RestrictSolutionFromGrid(Level);
           
       UpdateVortexEdgeStrengths(Level+1);
  
    }

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
    
       Temp = 0.;

#pragma omp parallel for reduction(+:Temp) private(xyz,q,VortexEdge)            
       for ( j = 1 ; j <= NumberOfVortexEdgesForInteractionListEntry_[i] ; j++ ) {
        
          VortexEdge = SurfaceVortexEdgeInteractionList_[i][j];
       
          if ( !VortexEdge->IsTrailingEdge() ) {              

             // Calculate influence of this edge
             
             VortexEdge->InducedVelocity(VortexLoop(i).xyz_c(), q);
         
             Temp += vector_dot(VortexLoop(i).Normal(), q);
             
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {

               xyz[0] = VortexLoop(i).xyz_c()[0];
               xyz[1] = VortexLoop(i).xyz_c()[1];
               xyz[2] = VortexLoop(i).xyz_c()[2];
               
               if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1;
               if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1;
               if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1;
               
               VortexEdge->InducedVelocity(xyz, q);
         
               if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1;
               if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1;
               if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1;
     
               Temp += vector_dot(VortexLoop(i).Normal(), q);
               
             }             
             
          }
            
       }
       
       vec_out[i] = Temp;
       
    }

    // If flow is supersonic add in generalized principart part of downwash
    
    if ( Mach_ > 1. ) {

#pragma omp parallel for private(Ws,i)     
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

          if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {   
         
             Ws = SurfaceVortexEdge(j).GeneralizedPrincipalPartOfDownWash();
            
             // Loop 1
              
             if ( SurfaceVortexEdge(j).VortexLoop1IsDownWind() ) {
              
                i = SurfaceVortexEdge(j).VortexLoop1();
            
                vec_out[i] += Ws * SurfaceVortexEdge(j).Gamma() * SurfaceVortexEdge(j).VortexLoop1DownWindWeight();
          
             }

             // Loop 2

             if ( SurfaceVortexEdge(j).VortexLoop2IsDownWind() ) {
              
                i = SurfaceVortexEdge(j).VortexLoop2();

                vec_out[i] -= Ws * SurfaceVortexEdge(j).Gamma() * SurfaceVortexEdge(j).VortexLoop2DownWindWeight();
          
             }
             
          }

       }
       
    }    

    // Trailing vortex induced velocities
        
    if ( CurrentWakeIteration_ > NoWakeIteration_ ) {
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
        
          Temp = 0.;
          
#pragma omp parallel for reduction(+:Temp) private(xyz,q)          
          for ( j = 1 ; j <= NumberOfTrailingVortexEdges_ ; j++ ) {
   
             TrailingVortexEdge(j).InducedVelocity(VortexLoop(i).xyz_c(), q);
         
             Temp += vector_dot(VortexLoop(i).Normal(), q);
   
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {
                
               xyz[0] = VortexLoop(i).xyz_c()[0];
               xyz[1] = VortexLoop(i).xyz_c()[1];
               xyz[2] = VortexLoop(i).xyz_c()[2];
               
               if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1;
               if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1;
               if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1;
               
               TrailingVortexEdge(j).InducedVelocity(xyz, q);
         
               if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1;
               if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1;
               if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1;
               
               Temp += vector_dot(VortexLoop(i).Normal(), q);
               
             }   
   
          }
          
          vec_out[i] += Temp;
   
       }
       
    }

    vec_out[0] = 0.;

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER DoMatrixPrecondition                          #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DoMatrixPrecondition(double *vec_in)
{

   int i;
  
   // Precondition using Jacobi

#pragma omp parallel for    
   for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

      vec_in[i] *= JacobiRelaxationFactor_*Diagonal_[i];

   }

}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateVelocities                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateVelocities(void)
{

    int i, j, Level;
    double q[3], xyz[3], Ws, U, V, W;
    VSP_EDGE *VortexEdge;
    
    // Freestream component... includes rotor wash, and any rotational rates
    
#pragma omp parallel for
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
       VortexLoop(i).U() = LocalFreeStreamVelocity_[i][0];
       VortexLoop(i).V() = LocalFreeStreamVelocity_[i][1];
       VortexLoop(i).W() = LocalFreeStreamVelocity_[i][2];

    }
       
    // Update the vortex strengths

    UpdateVortexEdgeStrengths(1);
 
    // Surface vortex induced velocities

    for ( Level = 1 ; Level < NumberOfMGLevels_ ; Level++ ) {
        
       RestrictSolutionFromGrid(Level);
       
       UpdateVortexEdgeStrengths(Level+1);

    }

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
    
       U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(q,VortexEdge)            
       for ( j = 1 ; j <= NumberOfVortexEdgesForInteractionListEntry_[i] ; j++ ) {
        
          VortexEdge = SurfaceVortexEdgeInteractionList_[i][j];
       
          if ( !VortexEdge->IsTrailingEdge() ) {              

             VortexEdge->InducedVelocity(VortexLoop(i).xyz_c(), q);
         
             U += q[0];
             V += q[1];
             W += q[2];
          
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {
                
                xyz[0] = VortexLoop(i).xyz_c()[0];
                xyz[1] = VortexLoop(i).xyz_c()[1];
                xyz[2] = VortexLoop(i).xyz_c()[2];
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
               
                VortexEdge->InducedVelocity(xyz, q);        
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
               
                U += q[0];
                V += q[1];
                W += q[2];
               
             }                
                         
          }
          
       }
       
       VortexLoop(i).U() += U;
       VortexLoop(i).V() += V;
       VortexLoop(i).W() += W; 
       
    }
    
    // Trailing vortex induced velocities

    if ( CurrentWakeIteration_ > NoWakeIteration_ ) {
                    
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
        
          U = V = W = 0.;
          
#pragma omp parallel for reduction(+:U,V,W) private(q)          
          for ( j = 1 ; j <= NumberOfTrailingVortexEdges_ ; j++ ) {
   
             TrailingVortexEdge(j).InducedVelocity(VortexLoop(i).xyz_c(), q);
         
             U += q[0];
             V += q[1];
             W += q[2];
   
            // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {
                
                xyz[0] = VortexLoop(i).xyz_c()[0];
                xyz[1] = VortexLoop(i).xyz_c()[1];
                xyz[2] = VortexLoop(i).xyz_c()[2];
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
               
                TrailingVortexEdge(j).InducedVelocity(xyz, q); 
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
               
                U += q[0];
                V += q[1];
                W += q[2];
               
             }   
                
          }
          
          VortexLoop(i).U() += U;
          VortexLoop(i).V() += V;
          VortexLoop(i).W() += W;
            
       }
       
    }
    
    // If flow is supersonic add in generalized principart part of downwash
    
    if ( Mach_ > 1. ) {

#pragma omp parallel for private(i,j,Ws)
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

          Ws = SurfaceVortexEdge(j).GeneralizedPrincipalPartOfDownWash();
         
          // Loop 1
           
          if ( SurfaceVortexEdge(j).VortexLoop1IsDownWind() ) {
           
             i = SurfaceVortexEdge(j).VortexLoop1();
         
             Ws *= SurfaceVortexEdge(j).Gamma() * SurfaceVortexEdge(j).VortexLoop1DownWindWeight();
           
             VortexLoop(i).U() += Ws * VortexLoop(i).Normal()[0];
             VortexLoop(i).V() += Ws * VortexLoop(i).Normal()[1];
             VortexLoop(i).W() += Ws * VortexLoop(i).Normal()[2];
 
          }

          // Loop 2

          if ( SurfaceVortexEdge(j).VortexLoop2IsDownWind() ) {
           
             i = SurfaceVortexEdge(j).VortexLoop2();

             Ws *= SurfaceVortexEdge(j).Gamma() * SurfaceVortexEdge(j).VortexLoop2DownWindWeight();
             
             VortexLoop(i).U() -= Ws * VortexLoop(i).Normal()[0];
             VortexLoop(i).V() -= Ws * VortexLoop(i).Normal()[1];
             VortexLoop(i).W() -= Ws * VortexLoop(i).Normal()[2];
      
          }

       }
       
    }   
    
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER UpdateWakeLocations                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::UpdateWakeLocations(void)
{

    int i, j, k, Iter, IterMax;
    double xyz[3], q[5], U, V, W;
    double **Utmp, **Vtmp, **Wtmp, NodeDist;
    
    Utmp = new double*[NumberOfTrailingVortexEdges_ + 1];
    Vtmp = new double*[NumberOfTrailingVortexEdges_ + 1];
    Wtmp = new double*[NumberOfTrailingVortexEdges_ + 1];

    for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {
    
       Utmp[i] = new double[TrailingVortexEdge(i).NumberOfSubVortices() + 1];
       Vtmp[i] = new double[TrailingVortexEdge(i).NumberOfSubVortices() + 1];
       Wtmp[i] = new double[TrailingVortexEdge(i).NumberOfSubVortices() + 1];
       
       zero_double_array(Utmp[i], TrailingVortexEdge(i).NumberOfSubVortices());
       zero_double_array(Vtmp[i], TrailingVortexEdge(i).NumberOfSubVortices());
       zero_double_array(Wtmp[i], TrailingVortexEdge(i).NumberOfSubVortices());
    
    }

    // Initialize to free stream values

#pragma omp parallel for private(j)   
    for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {
     
       TrailingVortexEdge(i).FreeStreamVelocity(0) = FreeStreamVelocity_[0];
       TrailingVortexEdge(i).FreeStreamVelocity(1) = FreeStreamVelocity_[1];
       TrailingVortexEdge(i).FreeStreamVelocity(2) = FreeStreamVelocity_[2];
          
       for ( j = 1 ; j <= TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
   
          Utmp[i][j] = FreeStreamVelocity_[0];
          Vtmp[i][j] = FreeStreamVelocity_[1];
          Wtmp[i][j] = FreeStreamVelocity_[2];
          
       }

    }
    
    // Add in the rotor induced velocities
 
    for ( k = 1 ; k <= NumberOfRotors_ ; k++ ) {
     
#pragma omp parallel for private(j,xyz,q)           
       for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {
     
          for ( j = 1 ; j <= TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {

             xyz[0] = TrailingVortexEdge(i).xyz_c(j)[0]; 
             xyz[1] = TrailingVortexEdge(i).xyz_c(j)[1];        
             xyz[2] = TrailingVortexEdge(i).xyz_c(j)[2]; 
          
             RotorDisk(k).Velocity(xyz, q);                   

             Utmp[i][j] += q[0] / Vinf_;
             Vtmp[i][j] += q[1] / Vinf_;
             Wtmp[i][j] += q[2] / Vinf_;
             
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {
     
               if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
               if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
               if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
               
               RotorDisk(k).Velocity(xyz, q);        
      
               if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
               if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
               if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
               
               Utmp[i][j] += q[0] / Vinf_;
               Vtmp[i][j] += q[1] / Vinf_;
               Wtmp[i][j] += q[2] / Vinf_;
               
             }                

          }
          
       }    
       
    }
    
    // Wing surface vortex induced velocities

    for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {
               
       for ( j = 1 ; j <= TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
        
          xyz[0] = TrailingVortexEdge(i).xyz_c(j)[0]; 
          xyz[1] = TrailingVortexEdge(i).xyz_c(j)[1];        
          xyz[2] = TrailingVortexEdge(i).xyz_c(j)[2]; 
             
          CalculateSurfaceInducedVelocityAtPoint(xyz, q);

          Utmp[i][j] += q[0];
          Vtmp[i][j] += q[1];
          Wtmp[i][j] += q[2];
          
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {
  
            if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
            if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
            if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
            
            CalculateSurfaceInducedVelocityAtPoint(xyz, q);
   
            if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
            if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
            if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
            
            Utmp[i][j] += q[0];
            Vtmp[i][j] += q[1];
            Wtmp[i][j] += q[2];
            
          }
          
       }
       
    }

    // Iterate on wake shape
      
    IterMax = 5;
    
    for ( Iter = 1 ; Iter <= IterMax ; Iter++ ) {
 
       // Copy over freestream, surface induced, and rotor induced velocities... these have not changed

#pragma omp parallel for private(j)
       for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {

          for ( j = 1 ; j <= TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {

             TrailingVortexEdge(i).U(j) = Utmp[i][j];
             TrailingVortexEdge(i).V(j) = Vtmp[i][j];
             TrailingVortexEdge(i).W(j) = Wtmp[i][j];
             
          }
          
       }
       
       // Trailing vortex induced velocities

       if ( CurrentWakeIteration_ > NoWakeIteration_ ) {
          
          for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {
   
#pragma omp parallel for private(k,xyz,q,U,V,W)             
             for ( j = 1 ; j <= TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
   
                U = V = W = 0.;
                
                for ( k = 1 ; k <= NumberOfTrailingVortexEdges_ ; k++ ) {
              
                   NodeDist = sqrt( pow(TrailingVortexEdge(i).TE_Node().x() - TrailingVortexEdge(k).TE_Node().x(), 2.)
                                  + pow(TrailingVortexEdge(i).TE_Node().y() - TrailingVortexEdge(k).TE_Node().y(), 2.)
                                  + pow(TrailingVortexEdge(i).TE_Node().z() - TrailingVortexEdge(k).TE_Node().z(), 2.) );
   
                   if ( i != k && NodeDist > TrailingVortexEdge(i).Sigma() ) {
                 
                      TrailingVortexEdge(k).InducedVelocity(TrailingVortexEdge(i).xyz_c(j), q);
                      
                      U += q[0];
                      V += q[1];
                      W += q[2];
                      
                      // If there is a symmetry plane, calculate influence of the reflection
            
                      if ( DoSymmetryPlaneSolve_ ) {
            
                        xyz[0] = TrailingVortexEdge(i).xyz_c(j)[0];
                        xyz[1] = TrailingVortexEdge(i).xyz_c(j)[1];
                        xyz[2] = TrailingVortexEdge(i).xyz_c(j)[2];
                                 
                        if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                        if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                        if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
                        
                        TrailingVortexEdge(k).InducedVelocity(xyz, q);
               
                        if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                        if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                        if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
                        
                        U += q[0];
                        V += q[1];
                        W += q[2];
                        
                      }                   
                      
                   }
                      
                }
                
                TrailingVortexEdge(i).U(j) += U;
                TrailingVortexEdge(i).V(j) += V;
                TrailingVortexEdge(i).W(j) += W;
                
             }
           
          }
          
       }
 
       // Force last segment to free stream conditions
               
#pragma omp parallel for private(j)           
       for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {

          j = TrailingVortexEdge(i).NumberOfSubVortices() + 1;
      
          TrailingVortexEdge(i).U(j) = FreeStreamVelocity_[0];
          TrailingVortexEdge(i).V(j) = FreeStreamVelocity_[1];
          TrailingVortexEdge(i).W(j) = FreeStreamVelocity_[2];
          
       }       
       
       // Now update the trailing vortex locations
                 
#pragma omp parallel for        
       for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {

          TrailingVortexEdge(i).UpdateLocation();

       } 
       
    }

    for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {
    
       delete [] Utmp[i];
       delete [] Vtmp[i];
       delete [] Wtmp[i];

    }    

    delete [] Utmp;
    delete [] Vtmp;
    delete [] Wtmp;    
 
}

/*##############################################################################
#                                                                              #
#                           VSP_SOLVER Do_GMRES_Solve                          #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Do_GMRES_Solve(void)
{

    int i, Iters;
    double Dot, ResMax;

#pragma omp parallel for
    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       GammaOld_[i] = Gamma_[i];
       
       Delta_[i] = 0.;
       
    }

    CalculateResidual();

    Dot = 0.;
    
#pragma omp parallel for reduction(+:Dot)        
    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

       Dot += Residual_[i]*Residual_[i];
       
    }
  
    L2Residual_ = log10(sqrt(Dot/NumberOfVortexLoops_));
    
    DoMatrixPrecondition(Residual_);

    GMRES_Solver(NumberOfVortexLoops_+1,  // Number of Equations, 0 <= i < Neq
                 1,                       // Max number of outer iterations
                 300,                     // Max number of inner (restart) iterations
                 1,                       // Output flag, verbose = 0, or 1
                 Delta_,                  // Initial guess and solution vector
                 Residual_,               // Right hand side of Ax = b
                 0.001,                   // Maximum error tolerance <-----
                 0.1,                     // Residual reduction factor
                 ResMax,                  // Final log10 of residual reduction   
                 Iters);                  // Final iteration count      

    // Update solutlion vector

#pragma omp parallel for
    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

       Gamma_[i] = GammaOld_[i] + Delta_[i];

    }

    if ( Verbose_) printf("log10(ABS(L2Residual_)): %lf \n",L2Residual_);

}

/*##############################################################################
#                                                                              #
#                           VSP_SOLVER CalculateResidual                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateResidual(void)
{

    int i;
    
    // Do matrix multiply and calculate the residual

    DoMatrixMultiply(Gamma_,Residual_);

#pragma omp parallel for 
    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       Residual_[i] = RightHandSide_[i] - Residual_[i];

    }
    
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER GMRES_Solver                              #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::GMRES_Solver(int Neq,                   // Number of Equations, 0 <= i < Neq
                              int IterMax,               // Max number of outer iterations
                              int NumRestart,            // Max number of inner (restart) iterations
                              int Verbose,               // Output flag, verbose = 0, or 1
                              double *x,                 // Initial guess and solution vector
                              double *RightHandSide,     // Right hand side of Ax = b
                              double ErrorMax,           // Maximum error tolerance
                              double ErrorReduction,     // Residual reduction factor
                              double &ResFinal,          // Final log10 of residual reduction
                              int    &IterFinal)         // Final iteration count
{

    int i, j, k, Iter, Done, TotalIterations;

    double av, *c, Epsilon, *g, **h, Dot, Mu, *r;
    double rho, rho_zero, rho_tol, *s, **v, *y;
    
    Epsilon = 1.0e-03;
    
    TotalIterations = 0;

    // Allocate memory
    
    c = new double[NumRestart + 1];
    g = new double[NumRestart + 1];
    s = new double[NumRestart + 1];
    y = new double[NumRestart + 1];

    h = new double*[NumRestart + 1];

    for ( i = 0 ; i <= NumRestart ; i++ ) {

       h[i] = new double[NumRestart + 1];

    }

    v = new double*[NumRestart + 1];

    for ( i = 0 ; i <= NumRestart ; i++ ) {

       v[i] = new double[Neq + 1];

    }

    r = new double[Neq + 1];

    // Outer iterative loop
    
    Iter = 0;

    rho = 1.e9;

    rho_tol = 0.;

    Done = 0;
    
    while ( Iter < IterMax && ( ( rho > rho_tol || rho > ErrorMax ) && !Done ) ) {

      // Matrix Multiplication

      DoPreconditionedMatrixMultiply(x,r);

#pragma omp parallel for
      for ( i = 0; i < Neq; i++ ) {

        r[i] = RightHandSide[i] - r[i];
   
      }

      rho = sqrt(VectorDot(Neq,r,r));

      if ( Iter == 0 ) rho_zero = rho;
     
      if ( Verbose && Iter == 0 ) printf("Wake Iteration: %5d ... GMRES Iteration: %5d ... Reduction: %10.5f ...  Maximum: %10.5f \r",CurrentWakeIteration_, 0,log10(rho/rho_zero),log10(rho)); fflush(NULL);
      
      if ( Iter == 0 ) rho_tol = rho * ErrorReduction;
    
      for ( i = 0; i < Neq; i++ ) {
      
         v[0][i] = r[i] / rho;
      
      }
    
      g[0] = rho;

      for ( i = 1; i < NumRestart + 1; i++ ) {

        g[i] = 0.0;

      }
    
      for ( i = 0; i < NumRestart + 1; i++ ) {

         for ( j = 0; j < NumRestart; j++ ) {

            h[i][j] = 0.0;
        
         }

      }

      k = 0;

      while ( k < NumRestart && ( ( rho > rho_tol || rho > ErrorMax ) && !Done )  ) {

         // Matrix multiply
     
         DoPreconditionedMatrixMultiply(v[k], v[k+1]);

         av = sqrt(VectorDot(Neq,v[k+1],v[k+1]));
          
         for ( j = 0; j < k+1; j++ ) {

            h[j][k] = VectorDot( Neq, v[k+1], v[j] );

#pragma omp parallel for 
            for ( i = 0; i < Neq; i++ ) {
 
               v[k+1][i] = v[k+1][i] - h[j][k] * v[j][i];
 
            }

         }
      
         h[k+1][k] = sqrt ( VectorDot( Neq, v[k+1], v[k+1] ) );
    
         if ( ( av + Epsilon * h[k+1][k] ) == av ) {
           
            for ( j = 0; j < k+1; j++ )  {
 
               Dot = VectorDot( Neq, v[k+1], v[j] );
  
               h[j][k] = h[j][k] + Dot;
 
#pragma omp parallel for 
               for ( i = 0; i < Neq; i++ ) {
  
                  v[k+1][i] = v[k+1][i] - Dot * v[j][i];

               }
 
            }
 
            h[k+1][k] = sqrt ( VectorDot( Neq, v[k+1], v[k+1] ) );

         }
     
         if ( h[k+1][k] != 0.0 ) {

#pragma omp parallel for
            for ( i = 0; i < Neq; i++ )  {
 
               v[k+1][i] = v[k+1][i] / h[k+1][k];
 
            }

         }
     
         if ( 0 < k ) {

            for ( i = 0; i < k + 2; i++ ) {
 
               y[i] = h[i][k];
 
            }
 
            for ( j = 0; j < k; j++ ) {
 
               ApplyGivensRotation( c[j], s[j], j, y );
 
            }
 
            for ( i = 0; i < k + 2; i++ ) {
 
               h[i][k] = y[i];
 
            }

         }
     
         Mu = sqrt ( h[k][k] * h[k][k] + h[k+1][k] * h[k+1][k] );

         c[k] = h[k][k] / Mu;

         s[k] = -h[k+1][k] / Mu;

         h[k][k] = c[k] * h[k][k] - s[k] * h[k+1][k];

         h[k+1][k] = 0.0;

         ApplyGivensRotation( c[k], s[k], k, g );
     
         rho = fabs ( g[k+1] );

         TotalIterations = TotalIterations + 1;
    
          if ( Verbose ) printf("Wake Iteration: %5d ... GMRES Iteration: %5d ... Reduction: %10.5f ...  Maximum: %10.5f \r",CurrentWakeIteration_,TotalIterations,log10(rho/rho_zero),log10(rho)); fflush(NULL);
         
         if ( rho <= 1.e-5 ) Done = 1;

         k++;

      }
    
      k--;
    
      y[k] = g[k] / h[k][k];

      for ( i = k - 1; 0 <= i; i-- ) {

         y[i] = g[i];
 
         for ( j = i+1; j < k + 1; j++ ) {
 
            y[i] = y[i] - h[i][j] * y[j];
 
         }
 
         y[i] = y[i] / h[i][i];

      }

#pragma omp parallel for private(j)    
      for ( i = 0; i < Neq; i++ ) {

         for ( j = 0; j < k + 1; j++ ) {
 
            x[i] = x[i] + v[j][i] * y[j];
 
         }

       }

       Iter++;
    
    }

    IterFinal = TotalIterations;

    ResFinal = log10(rho/rho_zero);

    // Free up memory

    delete [] c;
    delete [] g;
    delete [] r;
    delete [] s;
    delete [] y;

    for ( i = 0 ; i <= NumRestart ; i++ ) {

       delete [] h[i];

    }

    delete [] h;

    for ( i = 0 ; i <= NumRestart ; i++ ) {

       delete [] v[i];

    }

    delete [] v;

    return;

}

/*##############################################################################
#                                                                              #
#                              VSP_SOLVER VectorDot                            #
#                                                                              #
##############################################################################*/

double VSP_SOLVER::VectorDot(int Neq, double *r, double *s) 
{

    int i;
    double dot;

    dot = 0.;

#pragma omp parallel for reduction(+:dot)
    for ( i = 0 ; i < Neq ; i++ ) {

       dot += r[i] * s[i];
    
    }

    return dot;

}

/*##############################################################################
#                                                                              #
#                        VSP_SOLVER  ApplyGivensRotation                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ApplyGivensRotation(double c, double s, int k, double *g)
{

  double g1, g2;

  g1 = c * g[k] - s * g[k+1];
  g2 = s * g[k] + c * g[k+1];

  g[k]   = g1;
  g[k+1] = g2;

  return;

}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateForces                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateForces(void)
{
   
    // Calculate velocities
 
    CalculateVelocities();

    // Calculate pressure forces by applying JK theorem to each edge
 
    CalculatePressureForces();
    
    // If subsonic, we do a modified trailing edge induced drag calculation
    
    if ( Mach_ < 1 ) {
       
       CalculateInducedDrag();
       
    }
    
    // Calculate 2D clmax limited forces and moments, as well as span wide
    // loading information
    
    CalculateCLmaxLimitedForces();
     
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER CalculatePressureForces                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculatePressureForces(void)
{

    int i, j, Loop1, Loop2;
    double Forces1[3], Forces2[3], Fx, Fy, Fz;
    double CA, SA, CB, SB;
    double Cx, Cy, Cz, Cmx, Cmy, Cmz;

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
        
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
       VortexLoop(i).Fx() = 0.;
       VortexLoop(i).Fy() = 0.;
       VortexLoop(i).Fz() = 0.;

    }

    // Loop over vortex edges and calculate forces via K-J theorem
    
    Cx = Cy = Cz = Cmx = Cmy = Cmz = 0.;

#pragma omp parallel for reduction(+:Cx, Cy, Cz, Cmx, Cmy, Cmz) private(Loop1, Loop2, Forces1, Forces2, Fx, Fy, Fz)
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
     
       SurfaceVortexEdge(j).Fx() = 0.;
       SurfaceVortexEdge(j).Fy() = 0.;
       SurfaceVortexEdge(j).Fz() = 0.;
          
       if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {
        
          Loop1 = SurfaceVortexEdge(j).VortexLoop1();
          
          if ( Loop1 != 0 ) {

             SurfaceVortexEdge(j).CalculateForces(VortexLoop(Loop1));
             
             Forces1[0] = SurfaceVortexEdge(j).Fx();
             Forces1[1] = SurfaceVortexEdge(j).Fy();
             Forces1[2] = SurfaceVortexEdge(j).Fz();
            
          }

          Loop2 = SurfaceVortexEdge(j).VortexLoop2();

          if ( Loop2 != 0 ) {
           
             SurfaceVortexEdge(j).CalculateForces(VortexLoop(Loop2));
             
             Forces2[0] = SurfaceVortexEdge(j).Fx();
             Forces2[1] = SurfaceVortexEdge(j).Fy();
             Forces2[2] = SurfaceVortexEdge(j).Fz();
             
          }       
          
          if ( Loop1 == 0 ) {
           
             Fx = Forces2[0];
             Fy = Forces2[1];
             Fz = Forces2[2];
             
          }
          
          else if ( Loop2 == 0 ) {
           
             Fx = Forces1[0];
             Fy = Forces1[1];
             Fz = Forces1[2];
             
          }           
           
          else {
         
             Fx = 0.5*( Forces1[0] + Forces2[0] );
             Fy = 0.5*( Forces1[1] + Forces2[1] );
             Fz = 0.5*( Forces1[2] + Forces2[2] );

          }

          // Edge forces
          
          SurfaceVortexEdge(j).Fx() = Fx;
          SurfaceVortexEdge(j).Fy() = Fy;
          SurfaceVortexEdge(j).Fz() = Fz;

          // Loop level forces
          
          if ( ( Loop1 != 0 && Loop2 != 0 ) || SurfaceVortexEdge(j).IsLeadingEdge() ) {
           
             VortexLoop(Loop1).Fx() += 0.5*Fx;
             VortexLoop(Loop1).Fy() += 0.5*Fy;
             VortexLoop(Loop1).Fz() += 0.5*Fz;

             VortexLoop(Loop2).Fx() += 0.5*Fx;
             VortexLoop(Loop2).Fy() += 0.5*Fy;
             VortexLoop(Loop2).Fz() += 0.5*Fz;
          
          }
          
          // Forces
          
          Cx += Fx;
          Cy += Fy;
          Cz += Fz;
          
          // Moments

          Cmx += Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll

          Cmy += Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                
          Cmz += Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw
          
       }
          
    }
    
    // Calculate normal force on each vortex loop
 
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
       VortexLoop(i).NormalForce() = -VortexLoop(i).Fx() * VortexLoop(i).Nx()
                                   + -VortexLoop(i).Fy() * VortexLoop(i).Ny()
                                   + -VortexLoop(i).Fz() * VortexLoop(i).Nz();
                                   
       VortexLoop(i).dCp() = VortexLoop(i).NormalForce() / VortexLoop(i).Area();                                    

    }  
        
    CFx_[0] = Cx;
    CFy_[0] = Cy;
    CFz_[0] = Cz;
    
    CMx_[0] = Cmx;
    CMy_[0] = Cmy;
    CMz_[0] = Cmz;

    CFx_[0] /= (0.5*Sref_);
    CFy_[0] /= (0.5*Sref_);
    CFz_[0] /= (0.5*Sref_);
    
    CMx_[0] /= (0.5*Sref_*Bref_); // Roll
    CMy_[0] /= (0.5*Sref_*Cref_); // Pitch
    CMz_[0] /= (0.5*Sref_*Bref_); // Yaw
 
    CL_[0] = (-CFx_[0] * SA + CFz_[0] * CA );
    CD_[0] = ( CFx_[0] * CA + CFz_[0] * SA ) * CB - CFy_[0] * SB;
    CS_[0] = ( CFx_[0] * CA + CFz_[0] * SA ) * SB + CFy_[0] * CB;
        
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateInducedDrag                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateInducedDrag(void)
{

    int i, j, k, Loop, Hits;
    double xyz[3], q[3], qtot[3];
    double CA, SA, CB, SB, u, v, w;

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
        
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
       VortexLoop(i).Fx() = 0.;
       VortexLoop(i).Fy() = 0.;
       VortexLoop(i).Fz() = 0.;

    }
   
    // Loop over vortex edges and calculate forces via K-J theorem, using only wake induced velocities applied at TE

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {

          // Calculate an averaged local velocity using the left/right loops
  
          u = 0.;
          v = 0.;
          w = 0.;
 
          Hits = 0;
          
          if ( SurfaceVortexEdge(j).VortexLoop1() != 0 ) {
           
             Loop = SurfaceVortexEdge(j).VortexLoop1();
             
             u += LocalFreeStreamVelocity_[Loop][0];
             v += LocalFreeStreamVelocity_[Loop][1];
             w += LocalFreeStreamVelocity_[Loop][2];

             Hits++;
          
          }
          
          if ( SurfaceVortexEdge(j).VortexLoop2() != 0 ) {
           
             Loop = SurfaceVortexEdge(j).VortexLoop2();
             
             u += LocalFreeStreamVelocity_[Loop][0];
             v += LocalFreeStreamVelocity_[Loop][1];
             w += LocalFreeStreamVelocity_[Loop][2];   

             Hits++;
             
          }

          u /= Hits;
          v /= Hits;
          w /= Hits;

          // Trailing vortices induced velocities... shift the current bound vortex to the 
          // 'trailing edge' of the trailing vortex.
 
#pragma omp parallel for reduction(+:u,v,w) private(xyz,q)        
          for ( k = 1 ; k <= NumberOfTrailingVortexEdges_ ; k++ ) {

             xyz[0] = TrailingVortexEdge(k).TE_Node().x();
             xyz[1] = SurfaceVortexEdge(j).Yc();
             xyz[2] = SurfaceVortexEdge(j).Zc();

             TrailingVortexEdge(k).InducedVelocity(xyz, q);

             u += q[0];
             v += q[1];
             w += q[2];
             
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {

               if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
               if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
               if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
               
               TrailingVortexEdge(k).InducedVelocity(xyz, q);
         
               if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
               if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
               if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
     
               u += q[0];
               v += q[1];
               w += q[2];
               
             }               

          }
          
          qtot[0] = u;
          qtot[1] = v;
          qtot[2] = w;

          SurfaceVortexEdge(j).CalculateTrefftzForces(qtot);
   
       }
       
    }
   
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER CalculateCLmaxLimitedForces                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateCLmaxLimitedForces(void)
{

    int i, j, k, Loop;
    int NumberOfStations, SpanStation, SurfaceID; 
    double Fx, Fy, Fz;
    double Length, Re, Cf, Cl, Cd, Cs, Ct, Cn, Cx, Cy, Cz;
    double Swet, SwetTotal, StallFact;
    double CA, SA, CB, SB, CMx, CMy, CMz, Cmx, Cmy, Cmz, Cl_2d, dCD;

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
    
    // Zero out spanwise loading arrays
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
      
       NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
 
       zero_double_array(Span_Cx_[i],   NumberOfStations);
       zero_double_array(Span_Cy_[i],   NumberOfStations);
       zero_double_array(Span_Cz_[i],   NumberOfStations);       
 
       zero_double_array(Span_Cn_[i],   NumberOfStations);       
 
       zero_double_array(Span_Cl_[i],   NumberOfStations);
       zero_double_array(Span_Cd_[i],   NumberOfStations);
       zero_double_array(Span_Cs_[i],   NumberOfStations);       
      
       zero_double_array(Span_Cmx_[i],  NumberOfStations);
       zero_double_array(Span_Cmy_[i],  NumberOfStations);
       zero_double_array(Span_Cmz_[i],  NumberOfStations);       
      
       zero_double_array(Span_Yavg_[i], NumberOfStations);       
       zero_double_array(Span_Area_[i], NumberOfStations);
       
       zero_double_array(Local_Vel_[i], NumberOfStations);
         
    }   
            
    // Loop over vortex edges and calculate forces via K-J theorem, using only wake induced velocities applied at TE
  
    Fx = Fy = Fz = CMx = CMy = CMz = 0.;
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {
          
          // Extract out forces
          
          if ( Mach_ < 1. ) {
             
             Fx = SurfaceVortexEdge(j).Trefftz_Fx();
             Fy = SurfaceVortexEdge(j).Trefftz_Fy();
             Fz = SurfaceVortexEdge(j).Trefftz_Fz();
             
          }
          
          else {
             
             Fx = -SurfaceVortexEdge(j).Fx();
             Fy = -SurfaceVortexEdge(j).Fy();
             Fz = -SurfaceVortexEdge(j).Fz();
             
          }

          // Sum up span wise loading
 
          for ( k = 1 ; k <= 2 ; k++ ) {
           
             if ( k == 1 ) Loop = SurfaceVortexEdge(j).Loop1();
             
             if ( k == 2 ) Loop = SurfaceVortexEdge(j).Loop2();
          
             // Wing Surface
             
             if ( VortexLoop(Loop).WingID() > 0 ) {
                
                SurfaceID = VortexLoop(Loop).SurfaceID();
                
                SpanStation = VortexLoop(Loop).SpanStation();

                // Chordwise integrated forces
                    
                    Span_Cx_[SurfaceID][SpanStation] -= 0.5*Fx;

                    Span_Cy_[SurfaceID][SpanStation] -= 0.5*Fy;

                    Span_Cz_[SurfaceID][SpanStation] -= 0.5*Fz;
                                  
                    Span_Cn_[SurfaceID][SpanStation] -= 0.5 * Fx * VortexLoop(Loop).Nx()
                                                      - 0.5 * Fy * VortexLoop(Loop).Ny()
                                                      - 0.5 * Fz * VortexLoop(Loop).Nz();

                // Chordwise integrated moments
   
                   Span_Cmx_[SurfaceID][SpanStation] -= 0.5 * Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - 0.5 * Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
               
                   Span_Cmy_[SurfaceID][SpanStation] -= 0.5 * Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - 0.5 * Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                         
                   Span_Cmz_[SurfaceID][SpanStation] -= 0.5 * Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - 0.5 * Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw      
                
             }
             
             // Body Surface
             
             else {
            
                SurfaceID = VortexLoop(Loop).SurfaceID();
                
                SpanStation = 1;

                // Do not included body forces... in theory, they should be zero
                
                /*
                   // Chordwise integrated forces
                       
                       Span_Cx_[SurfaceID][SpanStation] -= 0.5*SurfaceVortexEdge(j).Trefftz_Fx();
   
                       Span_Cy_[SurfaceID][SpanStation] -= 0.5*SurfaceVortexEdge(j).Trefftz_Fy();
   
                       Span_Cz_[SurfaceID][SpanStation] -= 0.5*SurfaceVortexEdge(j).Trefftz_Fz();
                                     
                       Span_Cn_[SurfaceID][SpanStation] -= 0.5 * SurfaceVortexEdge(j).Trefftz_Fx() * VortexLoop(Loop).Nx()
                                                         - 0.5 * SurfaceVortexEdge(j).Trefftz_Fy() * VortexLoop(Loop).Ny()
                                                         - 0.5 * SurfaceVortexEdge(j).Trefftz_Fz() * VortexLoop(Loop).Nz();

                */
                
                // Chordwise integrated moments
                       
                   Span_Cmx_[SurfaceID][SpanStation] -= 0.5 * Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - 0.5 * Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
               
                   Span_Cmy_[SurfaceID][SpanStation] -= 0.5 * Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - 0.5 * Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                         
                   Span_Cmz_[SurfaceID][SpanStation] -= 0.5 * Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - 0.5 * Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw

             }
             
          }       

       }
       
    }
    
    // Calculate span station areas and y values

    for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {

       if ( VortexLoop(j).WingID() > 0 ) {
           
          SurfaceID = VortexLoop(j).SurfaceID();
                
          SpanStation = VortexLoop(j).SpanStation();
          
       }
       
       else {
          
          SurfaceID = VortexLoop(j).SurfaceID();
                
          SpanStation = 1;
          
       }          
       
       // Average y span locaiton, and strip area
              
       Span_Yavg_[SurfaceID][SpanStation] += VortexLoop(j).Yc() * VortexLoop(j).Area();

       Span_Area_[SurfaceID][SpanStation] += VortexLoop(j).Area();
       
       Local_Vel_[SurfaceID][SpanStation] += LocalFreeStreamVelocity_[j][4] * VortexLoop(j).Area();

    }
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
       
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == WING_SURFACE ) {
           
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
          
       }
       
       else {
          
          NumberOfStations = 1;
          
       }     
              
       for ( j = 1 ; j <= NumberOfStations ; j++ ) {
          
          Span_Yavg_[i][j] /= Span_Area_[i][j];
          
          Local_Vel_[i][j] /= Span_Area_[i][j];
          
       }
       
    }    

    // Calculate maximum CL... imposing local 2D Clmax limits
    
    CL_[0] = CS_[0] = CD_[0] = CFx_[0] = CFy_[0] = CFz_[0]= CMx_[0] = CMy_[0] = CMz_[0] = SwetTotal = CDo_ = 0.;  

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == WING_SURFACE ) {
          
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();

       }
       
       else {
          
          NumberOfStations = 1;

       }   
       
       VSPGeom().VSP_Surface(i).CDo() = 0.;
           
       for ( k = 1 ; k <= NumberOfStations ; k++ ) {
         
          Cx = Span_Cx_[i][k] / (0.5*Span_Area_[i][k]);
          
          Cy = Span_Cy_[i][k] / (0.5*Span_Area_[i][k]);
          
          Cz = Span_Cz_[i][k] / (0.5*Span_Area_[i][k]);
          
          Cn = Span_Cn_[SurfaceID][SpanStation] / (0.5*Span_Area_[i][k]);
   
          Cl =   ( -Span_Cx_[i][k] * SA + Span_Cz_[i][k] * CA )                              / (0.5*Span_Area_[i][k]);
   
          Cd = ( (  Span_Cx_[i][k] * CA + Span_Cz_[i][k] * SA ) * CB - Span_Cy_[i][k] * SB ) / (0.5*Span_Area_[i][k]);
   
          Cs = ( (  Span_Cx_[i][k] * CA + Span_Cz_[i][k] * SA ) * SB + Span_Cy_[i][k] * CB ) / (0.5*Span_Area_[i][k]);

          Ct = sqrt( Cl*Cl + Cs*Cs );
          
          Cmx = Span_Cmx_[i][k] / (0.5*Span_Area_[i][k]*VSPGeom().VSP_Surface(i).LocalChord(k)); 

          Cmy = Span_Cmy_[i][k] / (0.5*Span_Area_[i][k]*VSPGeom().VSP_Surface(i).LocalChord(k)); 

          Cmz = Span_Cmz_[i][k] / (0.5*Span_Area_[i][k]*VSPGeom().VSP_Surface(i).LocalChord(k)); 
          
          // Adjust forces and moments for local 2d stall
          
          StallFact = 1.;
          
          if ( VSPGeom().VSP_Surface(i).SurfaceType() == WING_SURFACE && Clmax_2d_ > 0. ) {
          
             Cl_2d = Clmax_2d_ * pow(Local_Vel_[i][k], 2.);
          
             if ( ABS(Ct) > 0. ) StallFact = ABS(MIN(ABS(Ct), Cl_2d) / ABS(Ct));
             
          }
          
           Span_Cl_[i][k] = StallFact * Cl;
           Span_Cd_[i][k] = StallFact * Cd;
           Span_Cs_[i][k] = StallFact * Cs;
          
           Span_Cx_[i][k] = StallFact * Cx;
           Span_Cy_[i][k] = StallFact * Cy;
           Span_Cz_[i][k] = StallFact * Cz;
           
          Span_Cmx_[i][k] = StallFact * Cmx;
          Span_Cmy_[i][k] = StallFact * Cmy;
          Span_Cmz_[i][k] = StallFact * Cmz;
          
          // Integrate spanwise forces and moments
          
           CL_[0] += 0.5 *  Span_Cl_[i][k] * Span_Area_[i][k];
           CD_[0] += 0.5 *  Span_Cd_[i][k] * Span_Area_[i][k];
           CS_[0] += 0.5 *  Span_Cs_[i][k] * Span_Area_[i][k];
 
          CFx_[0] += 0.5 *  Span_Cx_[i][k] * Span_Area_[i][k];
          CFy_[0] += 0.5 *  Span_Cy_[i][k] * Span_Area_[i][k];
          CFz_[0] += 0.5 *  Span_Cz_[i][k] * Span_Area_[i][k];
          
          CMx_[0] += 0.5 * Span_Cmx_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
          CMy_[0] += 0.5 * Span_Cmy_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
          CMz_[0] += 0.5 * Span_Cmz_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
          
          // Spanwise viscous drag for wings
          
          if ( VSPGeom().VSP_Surface(i).SurfaceType() == WING_SURFACE ) {
          
             Length = VSPGeom().VSP_Surface(i).LocalChord(k);
             
             Re = ReCref_ * Local_Vel_[i][k] * Length / Cref_;
             
             Cf = 0.455 / pow(log10(Re),2.58);
           
             Swet = 2.*Span_Area_[i][k];
   
             // Add in 25% for thickness drag to wings
             
             Swet *= 1.25;
                    
             // Bump by 25% for miscellaneous
             
             dCD = 1.25 * Cf * pow(Local_Vel_[i][k], 2.) * Swet;
             
             // Save at component level
             
             VSPGeom().VSP_Surface(i).CDo() += dCD / Sref_;
             
             // Total drag
      
             CDo_ += dCD;
             
             SwetTotal += Swet;
             
          }
          
       }
         
    }    
    
     CL_[0] /= 0.5*Sref_;
     CD_[0] /= 0.5*Sref_;
     CS_[0] /= 0.5*Sref_;
    
    CFx_[0] /= 0.5*Sref_;
    CFy_[0] /= 0.5*Sref_;
    CFz_[0] /= 0.5*Sref_;

    CMx_[0] /= 0.5*Bref_*Sref_;
    CMy_[0] /= 0.5*Cref_*Sref_;
    CMz_[0] /= 0.5*Bref_*Sref_;
  
    // Loop over body surfaces and calculate skin friction drag
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == BODY_SURFACE ) {
          
          Length = VSPGeom().VSP_Surface(i).AverageChord();
          
          Re = ReCref_ * Length / Cref_;
          
          Cf = 0.455 / pow(log10(Re),2.58);
        
          Swet = VSPGeom().VSP_Surface(i).WettedArea();
          
          // Body is split into 4 parts, wetted area over account for...  also and in 10% for form drag
          
          Swet *= 0.25 * 1.10;
    
          // Bump by 25% for miscellaneous
          
          dCD = 1.25 * Cf * Swet;
          
          // Save at component level
          
          VSPGeom().VSP_Surface(i).CDo() = dCD / Sref_;
          
          // Total drag
   
          CDo_ += dCD;
          
          SwetTotal += Swet;
          
       }
 
    }
    
    CDo_ /= Sref_;
    
    // Check that user is not asking for averaged forces... AND not running enough iterations
    
    if ( ForceType_ == FORCE_AVERAGE && WakeIterations_ <= AveragingIteration_ ) {
       
       printf("\n");
       printf("Not enough wake iterations specified to do requested averaging! \n");
       printf("Force and moment averaging turned off! \n");
       printf("\n"); fflush(NULL);
       
       ForceType_ = 0;
       
    }       
    
    // If we are supersonic... then replace 
    
    // Running sum of forces and moments
    
    if ( ForceType_ == FORCE_AVERAGE && CurrentWakeIteration_ >= AveragingIteration_ ) {

           CL_[1] +=     CL_[0];
           CD_[1] +=     CD_[0];
           CS_[1] +=     CS_[0];
                     
          CFx_[1] +=    CFx_[0];
          CFy_[1] +=    CFy_[0];
          CFz_[1] +=    CFz_[0];
                     
          CMx_[1] +=    CMx_[0];
          CMy_[1] +=    CMy_[0];
          CMz_[1] +=    CMz_[0];
          
          NumberOfAveragingSteps_++;

    }       
    
    else {
       
        NumberOfAveragingSteps_ = 0;
 
           CL_[1] = 0.;
           CD_[1] = 0.;
           CS_[1] = 0.;
                     
          CFx_[1] = 0.;
          CFy_[1] = 0.;
          CFz_[1] = 0.;
                     
          CMx_[1] = 0.;
          CMy_[1] = 0.;     
    }
    
    // Calculate averages of forces and moments
    
    if ( ForceType_ == FORCE_AVERAGE && CurrentWakeIteration_ == WakeIterations_ ) {

           CL_[1] /= NumberOfAveragingSteps_;
           CD_[1] /= NumberOfAveragingSteps_;
           CS_[1] /= NumberOfAveragingSteps_;
                     
          CFx_[1] /= NumberOfAveragingSteps_;
          CFy_[1] /= NumberOfAveragingSteps_;
          CFz_[1] /= NumberOfAveragingSteps_;
                     
          CMx_[1] /= NumberOfAveragingSteps_;
          CMy_[1] /= NumberOfAveragingSteps_;     
          
    }       
    
}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CalculateSpanWiseLoading                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateSpanWiseLoading(void)
{
 
    int i, k, NumberOfStations;
    double TotalLift, CFx, CFy, CFz;
    double CL, CD, CS, CMx, CMy, CMz;
    char LoadFileName[2000];
    FILE *LoadFile;
    
    // Open the load file
    
    sprintf(LoadFileName,"%s.lod",FileName_);
    
    if ( (LoadFile = fopen(LoadFileName, "w")) == NULL ) {

       printf("Could not open the spanwise loading file for output! \n");

       exit(1);

    }

                   // 123456789 123456789 123456789 123456789 123456789 123456789 
    fprintf(LoadFile,"   Wing      Yavg     Chord       Cl        Cd        Cs \n");

    TotalLift = 0.;  

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == WING_SURFACE ) {
        
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
              
          for ( k = 1 ; k <= NumberOfStations ; k++ ) {

             fprintf(LoadFile,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
                     i,
                     Span_Yavg_[i][k],
                     VSPGeom().VSP_Surface(i).LocalChord(k),
                     Span_Cl_[i][k],
                     Span_Cd_[i][k],
                     Span_Cs_[i][k]);
            
             TotalLift += 0.5 * Span_Cl_[i][k] * Span_Area_[i][k];
      
          }
          
       }
                 
    }
    
    fprintf(LoadFile,"\n\n\n");

                   // 123456789 123456789012345678901234567890123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789   
    fprintf(LoadFile,"Comp      Component-Name                             Mach       AoA      Beta       CL        CDi       CS       CFx       CFy       CFz       Cmx       Cmy       Cmz \n");

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == WING_SURFACE ) {
        
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
           
          CL = CD = CS = CFx = CFy = CFz = CMx = CMy = CMz = 0.;
          
          for ( k = 1 ; k <= NumberOfStations ; k++ ) {

             CL  += 0.5 *  Span_Cl_[i][k] * Span_Area_[i][k];
             CD  += 0.5 *  Span_Cd_[i][k] * Span_Area_[i][k];
             CS  += 0.5 *  Span_Cs_[i][k] * Span_Area_[i][k];
             CFx += 0.5 *  Span_Cx_[i][k] * Span_Area_[i][k];
             CFy += 0.5 *  Span_Cy_[i][k] * Span_Area_[i][k];
             CFz += 0.5 *  Span_Cz_[i][k] * Span_Area_[i][k];
             CMx += 0.5 * Span_Cmx_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
             CMy += 0.5 * Span_Cmy_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
             CMz += 0.5 * Span_Cmz_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
      
          }
          
          CL /= 0.5*Sref_;
          CD /= 0.5*Sref_;
          CS /= 0.5*Sref_;

          CFx /= 0.5*Sref_;
          CFy /= 0.5*Sref_;
          CFz /= 0.5*Sref_;
          
          CMx /= 0.5*Bref_*Sref_;
          CMy /= 0.5*Cref_*Sref_;
          CMz /= 0.5*Bref_*Sref_;
          
          fprintf(LoadFile,"%-9d %-40s %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
                  i,
                  VSPGeom().VSP_Surface(i).ComponentName(),
                  Mach_,
                  AngleOfAttack_/TORAD,
                  AngleOfBeta_/TORAD,                 
                  CL,
                  CD,
                  CS,
                  CFx,
                  CFy,
                  CFz,
                  CMx,
                  CMy,
                  CMz);        
          
       }
            
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == BODY_SURFACE ) {
        
          k = 1;
    
          fprintf(LoadFile,"%-9d %-40s %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
                  i,
                  VSPGeom().VSP_Surface(i).ComponentName(),
                  Mach_,
                  AngleOfAttack_/TORAD,
                  AngleOfBeta_/TORAD,                                   
                  0.5 *   Span_Cl_[i][k] * Span_Area_[i][k] / (0.5*Sref_),
                  0.5 *   Span_Cd_[i][k] * Span_Area_[i][k] / (0.5*Sref_),
                  0.5 *   Span_Cs_[i][k] * Span_Area_[i][k] / (0.5*Sref_),
                  0.5 *   Span_Cx_[i][k] * Span_Area_[i][k] / (0.5*Sref_),
                  0.5 *   Span_Cy_[i][k] * Span_Area_[i][k] / (0.5*Sref_),
                  0.5 *   Span_Cz_[i][k] * Span_Area_[i][k] / (0.5*Sref_),
                  0.5 *  Span_Cmx_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k) / (0.5*Bref_*Sref_),
                  0.5 *  Span_Cmy_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k) / (0.5*Cref_*Sref_),
                  0.5 *  Span_Cmz_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k) / (0.5*Bref_*Sref_));
         
          TotalLift += 0.5 * Span_Cl_[i][k] * Span_Area_[i][k];

       }
                 
    }

    TotalLift /= 0.5*Sref_;

    fclose(LoadFile);

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutAerothermalDatabaseFiles                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutAerothermalDatabaseFiles(void)
{

    FILE *adb_file_;
    char adb_file_name[2000], DumChar[2000];
    int i, j, Node1, Node2, Node3, SurfaceType, SurfaceID;
    int i_size, c_size, f_size, DumInt, number_of_nodes, number_of_tris;
    int Level, NumberOfCoarseEdges, MaxLevels;
    int *FineGridNode, *TempArray;
    
    int num_Mach = 1;
    int num_Alpha = 1;
    
    float Sref = Sref_;
    float Cref = Cref_;
    float Bref = Bref_;
    float X_cg = XYZcg_[0];
    float Y_cg = XYZcg_[1];
    float Z_cg = XYZcg_[2];

    float FreeStreamPressure         = 1000;
    float DynamicPressure            = 100000.;

    float DumFloat;
    
    float Cp;
    
    float Area;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    sprintf(adb_file_name,"%s.adb",FileName_);
    
    if ( (adb_file_ = fopen(adb_file_name, "wb")) == NULL ) {

       printf("Could not open the aerothermal data base file for binary output! \n");

       exit(1);

    }
  
    // Write out coded id to allow us to determine endiannes of files

    DumInt = -123789456; // Version 1 of the ADB file

    fwrite(&DumInt, i_size, 1, adb_file_);

    // Write out header to aerodynamics file

    number_of_tris  = VSPGeom().Grid().NumberOfLoops();
    number_of_nodes = VSPGeom().Grid().NumberOfNodes();

    fwrite(&number_of_nodes, i_size, 1, adb_file_);
    fwrite(&number_of_tris,  i_size, 1, adb_file_);
    fwrite(&num_Mach,        i_size, 1, adb_file_);
    fwrite(&num_Alpha,       i_size, 1, adb_file_);
    fwrite(&Sref,            f_size, 1, adb_file_);
    fwrite(&Cref,            f_size, 1, adb_file_);
    fwrite(&Bref,            f_size, 1, adb_file_);
    fwrite(&X_cg,            f_size, 1, adb_file_);
    fwrite(&Y_cg,            f_size, 1, adb_file_);
    fwrite(&Z_cg,            f_size, 1, adb_file_);

    DumFloat = Mach_;

    fwrite(&DumFloat, f_size, 1, adb_file_);

    DumFloat = AngleOfAttack_;

    fwrite(&DumFloat, f_size, 1, adb_file_);

    DumFloat = AngleOfBeta_;

    fwrite(&DumFloat, f_size, 1, adb_file_);
    
    // Write out wing ID flags, names...

    DumInt = VSPGeom().NumberOfWings();
    
    fwrite(&DumInt, i_size, 1, adb_file_);

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == WING_SURFACE ) {
        
          fwrite(&(i), i_size, 1, adb_file_);
    
          sprintf(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
          
          fwrite(DumChar, c_size, 100, adb_file_);
          
       }
     
    }
    
    // Write out body ID flags, names...
 
    DumInt = VSPGeom().NumberOfBodies();
    
    fwrite(&DumInt, i_size, 1, adb_file_);

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == BODY_SURFACE ) {
        
          fwrite(&(i), i_size, 1, adb_file_);
    
          sprintf(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
          
          fwrite(DumChar, c_size, 100, adb_file_);
          
       }
     
    }
    
    // Write out triangulated surface mesh

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {

       Node1       = VSPGeom().Grid().LoopList(j).Node1();
       Node2       = VSPGeom().Grid().LoopList(j).Node2();
       Node3       = VSPGeom().Grid().LoopList(j).Node3();

       SurfaceType = VSPGeom().Grid().LoopList(j).SurfaceType();
       
       SurfaceID   = VSPGeom().Grid().LoopList(j).BodyID()
                   + VSPGeom().Grid().LoopList(j).WingID();

       Area        = VSPGeom().Grid().LoopList(j).Area();
       
       fwrite(&(Node1),       i_size, 1, adb_file_);
       fwrite(&(Node2),       i_size, 1, adb_file_);
       fwrite(&(Node3),       i_size, 1, adb_file_);
       fwrite(&(SurfaceType), i_size, 1, adb_file_);
       fwrite(&(SurfaceID),   i_size, 1, adb_file_);
       fwrite(&Area,          f_size, 1, adb_file_);

    }

    // Write out node data
    
    float x, y, z;

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfNodes() ; j++ ) {

       x = VSPGeom().Grid().NodeList(j).x();
       y = VSPGeom().Grid().NodeList(j).y();
       z = VSPGeom().Grid().NodeList(j).z();
       
       fwrite(&(x), f_size, 1, adb_file_);
       fwrite(&(y), f_size, 1, adb_file_);
       fwrite(&(z), f_size, 1, adb_file_);
       
    }

    // Write out aero data base file
 
    fwrite(&(FreeStreamPressure), f_size, 1, adb_file_); // Freestream static pressure
    fwrite(&(DynamicPressure),    f_size, 1, adb_file_); // Freestream dynamic pressure
    
    // Loop over surfaces and write out solution

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {

       // Write out the data base
         
       Cp = VSPGeom().Grid().LoopList(j).dCp();
       
  //     Cp = VSPGeom().Grid().LoopList(j).Gamma();
  
       fwrite(&Cp, f_size, 1, adb_file_); // Wall or Edge Pressure, Pa

    }

    // Write out wake shape
    
    fwrite(&NumberOfTrailingVortexEdges_, i_size, 1, adb_file_);
    
    for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {

       TrailingVortexEdge(i).WriteToFile(adb_file_);

    }     

    // Write out the rotor data
    
    fwrite(&NumberOfRotors_, i_size, 1, adb_file_);
    
    for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
     
       RotorDisk(i).Write_Binary_STP_Data(adb_file_);
     
    }
    
    // Write out wing coarse grid edges
        
    MaxLevels = VSPGeom().NumberOfGridLevels();

    fwrite(&MaxLevels, i_size, 1, adb_file_); 
   
    // Loop over each level

    FineGridNode = new int[VSPGeom().Grid().NumberOfNodes() + 1];
 
    TempArray = new int[VSPGeom().Grid().NumberOfNodes() + 1];

    for ( Level = 1 ; Level < MaxLevels ; Level++ ) {

       NumberOfCoarseEdges = VSPGeom().Grid(Level).NumberOfEdges();
 
       fwrite(&NumberOfCoarseEdges, i_size, 1, adb_file_); 

       // Perm array... points to finest grid from whatever level we are on... initialize to level 0
       
       if ( Level == 1 ) {

          for ( j = 1 ; j <= VSPGeom().Grid().NumberOfNodes() ; j++ ) { 
           
             FineGridNode[j] = j;
             
             TempArray[j] = 0;
             
          }
          
       }

       // Only update permutation array if this level actually exists on the grid

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) { 
        
          TempArray[j] = FineGridNode[VSPGeom().Grid(Level).NodeList(j).FineGridNode()];
          
       }
       
       for ( j = 1 ; j <= VSPGeom().Grid().NumberOfNodes() ; j++ ) { 
        
          FineGridNode[j] = TempArray[j];
          
          TempArray[j] = 0;
          
       }

       // Create pointer from current level to finest level
       
       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfEdges() ; j++ ) {

         SurfaceID = VSPGeom().Grid(Level).EdgeList(j).Body()
                   + VSPGeom().Grid(Level).EdgeList(j).Wing();

          Node1 = VSPGeom().Grid(Level).EdgeList(j).Node1();
          Node2 = VSPGeom().Grid(Level).EdgeList(j).Node2();  
                        
          Node1 = FineGridNode[Node1];             
          Node2 = FineGridNode[Node2];

          fwrite(&SurfaceID, i_size, 1, adb_file_);
          
          fwrite(&Node1, i_size, 1, adb_file_);
          fwrite(&Node2, i_size, 1, adb_file_);

       }
  
    }
    
    fclose(adb_file_);

    delete [] FineGridNode;
    delete [] TempArray;
    
}

/*##############################################################################
#                                                                              #
#                        VSP_SOLVER WriteRestartFile                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteRestartFile(void)
{
    
    int i, d_size;
    char FileNameWithExt[2000];
    FILE *RestartFile;

    d_size = sizeof(double);
    
    // Open restart file
    
    sprintf(FileNameWithExt,"%s.restart",FileName_);
    
    if ( (RestartFile = fopen(FileNameWithExt, "wb")) == NULL ) {

       printf("Could not open the restart file for output! \n");

       exit(1);

    }   
    
    // Write out the vortex strengths
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       fwrite(&(Gamma_[i]), d_size, 1, RestartFile);
       
    }    
    
    fclose(RestartFile);
    
    // Write out the wake shape
  
}

/*##############################################################################
#                                                                              #
#                        VSP_SOLVER LoadRestartFile                            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::LoadRestartFile(void)
{

    int i, d_size;
    char FileNameWithExt[2000];
    FILE *RestartFile;
    
    d_size = sizeof(double);
    
    // Open status file
    
    sprintf(FileNameWithExt,"%s.restart",FileName_);
    
    if ( (RestartFile = fopen(FileNameWithExt, "rb")) == NULL ) {

       printf("Could not open the restart file for output! \n");

       exit(1);

    }   
    
    // Write out the vortex strengths
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       fread(&(Gamma_[i]), d_size, 1, RestartFile);
       
    }    
    
    fclose(RestartFile);
    
    // Read in the wake shape
  
}

/*##############################################################################
#                                                                              #
#            VSP_SOLVER CreateSurfaceVorticesInteractionList                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateSurfaceVorticesInteractionList(void)
{
 
    int i, j, k, NumberOfEdges, TotalHits, Included, Edge, DoCheck;
    double xyz[3], SpeedRatio;
   
    // Allocate space for final interaction lists
    
    NumberOfVortexEdgesForInteractionListEntry_ = new int[NumberOfVortexLoops_ + 1];

    SurfaceVortexEdgeInteractionList_ = new VSP_EDGE**[NumberOfVortexLoops_ + 1];

    DoCheck = 0;
    
    TotalHits = 0;
    
    SpeedRatio = 0.;
    
    printf("Creating interaction lists... \n\n");fflush(NULL);
    
    if ( 0&&Mach_ > 1. ) {
       
       NumberOfVortexEdgesForInteractionListEntry_[1] = NumberOfSurfaceVortexEdges_;  
       
       SurfaceVortexEdgeInteractionList_[1] = new VSP_EDGE*[NumberOfSurfaceVortexEdges_ + 1]; 
       
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
          
          SurfaceVortexEdgeInteractionList_[1][j] = SurfaceVortexEdge_[j];
          
       }
       
       for ( k = 2 ; k <= NumberOfVortexLoops_ ; k++ ) {
       
          NumberOfVortexEdgesForInteractionListEntry_[k] = NumberOfSurfaceVortexEdges_;
           
          SurfaceVortexEdgeInteractionList_[k] = SurfaceVortexEdgeInteractionList_[1];
          
       }
       
    }
    
    else {
    
       for ( k = 1 ; k <= NumberOfVortexLoops_ ; k++ ) {
        
          printf("%d / %d \r",k,NumberOfVortexLoops_);fflush(NULL);
   
          xyz[0] = VortexLoop(k).Xc();
          xyz[1] = VortexLoop(k).Yc();
          xyz[2] = VortexLoop(k).Zc();
   
          SurfaceVortexEdgeInteractionList_[k] = CreateInteractionList(xyz, NumberOfEdges);
                                
          NumberOfVortexEdgesForInteractionListEntry_[k] = NumberOfEdges;    
          
          TotalHits += NumberOfEdges;
          
          SpeedRatio += ((double)NumberOfEdges)/NumberOfSurfaceVortexEdges_;
          
          // Check if list includes vortex loop k
   
          if ( DoCheck ) {
           
             Included = 0;
             
             for ( j = 1 ; j <= NumberOfEdges ; j++ ) {
              
                for ( i = 1 ; i <= VortexLoop(k).NumberOfEdges() ; i++ ) {
                            
                   Edge = VortexLoop(k).Edge(i);
                   
                   if ( SurfaceVortexEdgeInteractionList_[k][j] == SurfaceVortexEdge_[Edge] ) Included++;
                   
                }
   
             }
             
             if ( Included != VortexLoop(k).NumberOfEdges() ) printf("\n\n\nIncluded was: %d out of: %d \n",Included,VortexLoop(k).NumberOfEdges());
             
          }
          
       }
       
       SpeedRatio = NumberOfVortexLoops_/SpeedRatio;
       
       if ( Verbose_ ) printf("\nSpeed Up Ratio: %lf \n\n\n",SpeedRatio);fflush(NULL);
       
    }
  
}

/*##############################################################################
#                                                                              #
#            VSP_SOLVER CalculateSurfaceInducedVelocityAtPoint                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateSurfaceInducedVelocityAtPoint(double xyz[3], double q[3])
{
 
    int j, NumberOfEdges;
    double U, V, W, dq[3];
    VSP_EDGE **InteractionList, *VortexEdge;
     
    // Create interaction list for this xyz location

    InteractionList = CreateInteractionList(xyz, NumberOfEdges);

    U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(VortexEdge,dq)
    for ( j = 1 ; j <= NumberOfEdges ; j++ ) {
     
       VortexEdge = InteractionList[j];
    
       if ( !VortexEdge->IsTrailingEdge() ) {              

          VortexEdge->InducedVelocity(xyz, dq);
      
          U += dq[0];
          V += dq[1];
          W += dq[2];
          
       }
         
    }
    
    q[0] = U;
    q[1] = V;
    q[2] = W;    
    
    delete [] InteractionList;
 
}

/*##############################################################################
#                                                                              #
#            VSP_SOLVER CalculateWingSurfaceInducedVelocityAtPoint             #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateWingSurfaceInducedVelocityAtPoint(double xyz[3], double q[3])
{
 
    int j, NumberOfEdges;
    double U, V, W, dq[3];
    VSP_EDGE **InteractionList, *VortexEdge;
     
    // Create interaction list for this xyz location

    InteractionList = CreateInteractionList(xyz, NumberOfEdges);

    U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(VortexEdge,dq)
    for ( j = 1 ; j <= NumberOfEdges ; j++ ) {
     
       VortexEdge = InteractionList[j];
    
       if ( !VortexEdge->IsTrailingEdge() && VortexEdge->Wing() ) {              

          VortexEdge->InducedVelocity(xyz, dq);
      
          U += dq[0];
          V += dq[1];
          W += dq[2];
          
       }
         
    }
    
    q[0] = U;
    q[1] = V;
    q[2] = W;    
    
    delete [] InteractionList;
 
}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CreateInteractionList                          #
#                                                                              #
##############################################################################*/

VSP_EDGE **VSP_SOLVER::CreateInteractionList(double xyz[3], int &NumberOfInteractionEdges)
{

    int i, j, Level, Loop;
    int Level_1, Level_2, Used, i_1, i_2;
    int StackSize, MoveDownLevel, Next, Found;
    double Distance, FarAway, Mu, TanMu, Ratio_1, Ratio_2;
    VSP_EDGE **InteractionEdgeList;
    
    // Mach angle
    
    TanMu = 1.e9;
    
    if ( Mach_ > 1. ) {
       
       Mu = asin(1./Mach_);
       
       TanMu = sin(Mu);
       
    }
       
    // Allocate space if this is the first time through
    
    if ( FirstTimeSetup_ ) {
  
       // Allocate space for temporary interaction lists
       
       EdgeIsUsed_ = new int*[VSPGeom().NumberOfGridLevels() + 1];
      
       for ( Level = VSPGeom().NumberOfGridLevels() - 1 ; Level >= 1  ; Level-- ) {
        
          EdgeIsUsed_[Level] = new int[VSPGeom().Grid(Level).NumberOfEdges() + 1];
        
          zero_int_array(EdgeIsUsed_[Level], VSPGeom().Grid(Level).NumberOfEdges());
          
       }
                    
       MaxStackSize_ = 0;

       // Loop over all grid levels for this surface
       
       for ( Level = VSPGeom().NumberOfGridLevels() - 1 ; Level >= 1  ; Level-- ) {

          // Loop over all vortex loops
          
         MaxStackSize_ += VSPGeom().Grid(Level).NumberOfLoops();
         
       }

       LoopStackList_ = new STACK_ENTRY[MaxStackSize_ + 1];
       
       FirstTimeSetup_ = 0;
       
    }

    // Define faraway criteria... how far away we need to be from a loop to treat it as faraway
    // Ratio of distance to maximum loop size
    
    FarAway = 3.0;
     
    // Insert loops on coarsest level into stack
    
    Level = VSPGeom().NumberOfGridLevels() - 1;
 
    StackSize = 0;
    
    for ( Loop = 1 ; Loop <= VSPGeom().Grid(Level).NumberOfLoops() ; Loop++ ) {
     
       StackSize++;
       
       LoopStackList_[StackSize].Level = Level;
       LoopStackList_[StackSize].Loop  = Loop;

    }
    
    // Update the search ID value... reset things after we done all the loops
    
    SearchID_++;
    
    if ( SearchID_ > NumberOfVortexLoops_ ) {
     
       for ( Level = 1 ; Level < VSPGeom().NumberOfGridLevels() ; Level++ ) {
      
          zero_int_array(EdgeIsUsed_[Level], VSPGeom().Grid(Level).NumberOfEdges()); 
          
       }
       
       SearchID_ = 1;
       
    }
    
    // Now loop over stack and begin AGMP process

    Next = 1;
    
    while ( Next <= StackSize ) {
     
       Level = LoopStackList_[Next].Level;
       Loop  = LoopStackList_[Next].Loop;

       // If we are far enough away from this loop, add it's edges to the interaction list
             
       MoveDownLevel = 0;

       Distance = sqrt( pow(xyz[0] - VSPGeom().Grid(Level).LoopList(Loop).Xc(),2.)
                      + pow(xyz[1] - VSPGeom().Grid(Level).LoopList(Loop).Yc(),2.)
                      + pow(xyz[2] - VSPGeom().Grid(Level).LoopList(Loop).Zc(),2.) );
       
       Ratio_1 = Distance / VSPGeom().Grid(Level).LoopList(Loop).Length();
       
       Ratio_2 = TanMu * Distance / VSPGeom().Grid(Level).LoopList(Loop).Length();
     
       if ( Level == 1 || ( MIN(Ratio_1,Ratio_2) >= FarAway && !inside_box(VSPGeom().Grid(Level).LoopList(Loop).BoundBox(), xyz) ) ) {
      
          // Add these edges to the list
          
          for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfEdges() ; i++ ) {
    
             j = VSPGeom().Grid(Level).LoopList(Loop).Edge(i);
             
             EdgeIsUsed_[Level][j] = SearchID_;
             
          }
          
       }
       
       // If xyz too close to this loop, move down a level
       
       else {
        
          MoveDownLevel = 1;
          
       }

       if ( MoveDownLevel ) {
        
          if ( Level > 1 ) {
         
             for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
               
                StackSize++;
                
                if ( StackSize > MaxStackSize_ ) {
                   
                  printf("stack size must be resized! \n");fflush(NULL);
                  exit(1);
                    
                }
                  
                LoopStackList_[StackSize].Level = Level - 1;
                LoopStackList_[StackSize].Loop  = VSPGeom().Grid(Level).LoopList(Loop).FineGridLoop(i);
     
             }   
             
          }

       }
       
       // Move onto next entry in the stack

       Next++;
       
    }
           
    // Pack all unique edges... move up from coarse to fine grids
    
    NumberOfInteractionEdges = 0;
    
    for ( Level = VSPGeom().NumberOfGridLevels() - 1 ; Level >= 1  ; Level-- ) {
     
       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
        
          // This edge was marked as being used
          
          if ( EdgeIsUsed_[Level][i] == SearchID_ ) {
          
             // Check that the edge is not already used on a coarser grid
             
             if ( Level + 1 < VSPGeom().NumberOfGridLevels() ) {
              
                i_1 = i ; Level_1 = Level;
              
                Level_2 = Level + 1;
                
                Used = 0;
                
                // Loop down grid levels to check if it's used on a coarser grid
                
                while ( Level_2 < VSPGeom().NumberOfGridLevels() && !Used ) {
              
                  i_2 = VSPGeom().Grid(Level_1).EdgeList(i_1).CourseGridEdge();

                  if ( i_2 > 0 && EdgeIsUsed_[Level_2][i_2] == SearchID_ ) {
                   
                    Used = 1;
                    
                  }
                  
                  Level_1 = Level_2;
                  
                  Level_2 = Level_1 + 1;
                  
                  i_1 = i_2;
                  
                }
           
                // If not used, then add it to the list
                
                if ( !Used ) {
                 
                   NumberOfInteractionEdges++;
                   
                }
                
                // Edge was used on a coarser grid, so remove it from the list
                
                else {
                 
                   EdgeIsUsed_[Level][i] = 0;
                   
                }
           
             }
             
             // This is the coarset grid, so we definitely use this edge
             
             else {
              
                NumberOfInteractionEdges++;
                
             }
                           
          }
          
       }

    }
    
    // Force any trailing edges to be evaluated at the finest grid level
    
    for ( Level = VSPGeom().NumberOfGridLevels() - 1 ; Level >= 1  ; Level-- ) {
     
       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
        
          // This edge was marked as being used
          
          if ( EdgeIsUsed_[Level][i] == SearchID_ ) {
           
             // If this edge is on trailing edge, force it to be evaluated on the finest grid
             
             if ( VSPGeom().Grid(Level).EdgeList(i).IsTrailingEdge() ) {
              
                if ( Level != 1  ) {

                   // Determine the fine grid edge this coarse edge comes from
                   
                   i_1 = i ; Level_1 = Level;
                       
                   Found = 0;
                   
                   // Loop down grid levels to check if it's used on a coarser grid
       
                   while ( Level_1 >= 2 && !Found ) {
                    
                     i_2 = VSPGeom().Grid(Level_1).EdgeList(i_1).FineGridEdge();
                         
                     if ( Level_1 == 2 ) Found = i_2;

                     Level_1--;
                     
                     i_1 = i_2;
                     
                   }
                  
                   // Zero out this coarse grid edge as being used
                   
                   EdgeIsUsed_[Level][    i] = 0;
                   
                   // Replace with the fine grid version
                         
                   EdgeIsUsed_[    1][Found] = SearchID_;
                   
                }
                
             }
             
          }
          
       }
       
    }

    InteractionEdgeList = new VSP_EDGE*[NumberOfInteractionEdges + 1];
    
    NumberOfInteractionEdges = 0;
     
    // Create the final interaction list
    
    for ( Level = VSPGeom().NumberOfGridLevels() - 1 ; Level >= 1  ; Level-- ) {
        
       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
        
          if ( EdgeIsUsed_[Level][i] == SearchID_ ) {
           
             NumberOfInteractionEdges++;
           
             InteractionEdgeList[NumberOfInteractionEdges] = &(VSPGeom().Grid(Level).EdgeList(i));
   
          }
          
       }
     
    }      
    
    return InteractionEdgeList;
    
}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CalculateMPVelocity                            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateMPVelocity(void)
{

    int i, j, Level;
    double q[4], U, V, W;
    VSP_EDGE *VortexEdge;
    
    for ( Level = 1 ; Level < NumberOfMGLevels_ ; Level++ ) {
        
       RestrictSolutionFromGrid(Level);
       
       UpdateVortexEdgeStrengths(Level+1);
   
    }
    
    i = 111;
    
    U = V = W = 0.;

double xyz[3];
xyz[0] = 10.;
xyz[1] =  1.;
xyz[2] = 10.;

    printf("NumberOfVortexEdgesForInteractionListEntry_[i]: %d \n",NumberOfVortexEdgesForInteractionListEntry_[i]);
    
    for ( j = 1 ; j <= NumberOfVortexEdgesForInteractionListEntry_[i] ; j++ ) {
     
       VortexEdge = SurfaceVortexEdgeInteractionList_[i][j];
    
       if ( !VortexEdge->IsTrailingEdge() ) {              

          VortexEdge->InducedVelocity(xyz, q);
      
          U += q[0];
          V += q[1];
          W += q[2];
          
       }
         
    }
    
    printf("New U,V,W: %lf %lf %lf \n",U,V,W);
 
}

/*##############################################################################
#                                                                              #
#                   VSP_SOLVER ProlongateSolutionFromGrid                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ProlongateSolutionFromGrid(int Level)
{
 
    int i_c, i_f, g_c, g_f;
    double Fact;
   
    g_c = Level;
    g_f = Level - 1;
    
    // Subtract out initial restricted solution from coarse the coarse grid
    // solution - this ends up with the correction being stored.

    for ( i_f = 1 ; i_f <= VSPGeom().Grid(g_f).NumberOfLoops() ; i_f++ ) {

       i_c = VSPGeom().Grid(g_f).LoopList(i_f).CoarseGridLoop();
       
       Fact = VSPGeom().Grid(g_f).LoopList(i_f).Area() 
            / VSPGeom().Grid(g_c).LoopList(i_c).Area();
  
       VSPGeom().Grid(g_c).LoopList(i_c).Gamma() -= Fact * VSPGeom().Grid(g_f).LoopList(i_f).Gamma();
       
    }

    // Prolongate correction from coarse to fine grid - direct injection

    for ( i_f = 1 ; i_f <= VSPGeom().Grid(g_f).NumberOfLoops() ; i_f++ ) {

       i_c = VSPGeom().Grid(g_f).LoopList(i_f).CoarseGridLoop();
  
       VSPGeom().Grid(g_f).LoopList(i_f).Gamma() += VSPGeom().Grid(g_c).LoopList(i_c).Gamma();
       
    }

}

/*##############################################################################
#                                                                              #
#                   VSP_SOLVER RestrictSolutionFromGrid                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::RestrictSolutionFromGrid(int Level)
{
 
    int i_c, i_f, g_c, g_f;
    double Fact;
    
    // Restrict solution from Level i, to level i+1
    
    g_f = Level;
    g_c = Level + 1;
    
    // Zero out stuff on the coarsest grid
    
#pragma omp parallel for      
    for ( i_c = 1 ; i_c <= VSPGeom().Grid(g_c).NumberOfLoops() ; i_c++ ) {

       VSPGeom().Grid(g_c).LoopList(i_c).Gamma() = 0.;

    }
  
    // Restrict the solution to the coarse grid
 
    for ( i_f = 1 ; i_f <= VSPGeom().Grid(g_f).NumberOfLoops() ; i_f++ ) {

       i_c = VSPGeom().Grid(g_f).LoopList(i_f).CoarseGridLoop();
       
       Fact = VSPGeom().Grid(g_f).LoopList(i_f).Area()
            / VSPGeom().Grid(g_c).LoopList(i_c).Area();

       VSPGeom().Grid(g_c).LoopList(i_c).Gamma() += Fact*VSPGeom().Grid(g_f).LoopList(i_f).Gamma();
  
    }

}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER InterpolateSolutionFromGrid                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::InterpolateSolutionFromGrid(int Level)
{
 
    int j, i_c, i_f, g_c, g_f, Loop1, Loop2, Edge, Iter;
    double Area1, Area2, Fact, *Smoothed_dCp, *Denom;
    
    g_c = Level;
    g_f = Level - 1;
    
    // Prolongate solution from course grid at Level, to finer grid and Level - 1

#pragma omp parallel for private(i_c)
    for ( i_f = 1 ; i_f <= VSPGeom().Grid(g_f).NumberOfLoops() ; i_f++ ) {

       i_c = VSPGeom().Grid(g_f).LoopList(i_f).CoarseGridLoop();

       VSPGeom().Grid(g_f).LoopList(i_f).dCp()   = VSPGeom().Grid(g_c).LoopList(i_c).dCp();

       VSPGeom().Grid(g_f).LoopList(i_f).Gamma() = VSPGeom().Grid(g_c).LoopList(i_c).Gamma();

    }
    
    // Smooth pressure
    
    Smoothed_dCp = new double[VSPGeom().Grid(g_f).NumberOfLoops() + 1];
    Denom = new double[VSPGeom().Grid(g_f).NumberOfLoops() + 1];
    
    for ( Iter = 1 ; Iter <= 5 ; Iter++ ) {
     
       zero_double_array(Smoothed_dCp, VSPGeom().Grid(g_f).NumberOfLoops());
       zero_double_array(Denom,        VSPGeom().Grid(g_f).NumberOfLoops());
       
       for ( i_f = 1 ; i_f <= VSPGeom().Grid(g_f).NumberOfLoops() ; i_f++ ) {
        
          for ( j = 1 ; j <= VSPGeom().Grid(g_f).LoopList(i_f).NumberOfEdges() ; j++ ) {
           
             Edge = VSPGeom().Grid(g_f).LoopList(i_f).Edge(j);
             
             Loop1 = VSPGeom().Grid(g_f).EdgeList(Edge).Loop1();
             Loop2 = VSPGeom().Grid(g_f).EdgeList(Edge).Loop2();
             
             Area1 = VSPGeom().Grid(g_f).LoopList(Loop1).Area();
             Area2 = VSPGeom().Grid(g_f).LoopList(Loop1).Area();
             
             Fact = Area1 * VSPGeom().Grid(g_f).LoopList(Loop1).dCp()
                  + Area2 * VSPGeom().Grid(g_f).LoopList(Loop2).dCp();
            
             Smoothed_dCp[Loop1] += Fact;
             Smoothed_dCp[Loop2] += Fact;
                                  
             Denom[Loop1] += Area1 + Area2;                               
             Denom[Loop2] += Area1 + Area2;      
             
          }
          
       }
       
       for ( i_f = 1 ; i_f <= VSPGeom().Grid(g_f).NumberOfLoops() ; i_f++ ) {
       
          VSPGeom().Grid(g_f).LoopList(i_f).dCp() = Smoothed_dCp[i_f] / Denom[i_f];
          
       }
          
    }
    
    delete [] Smoothed_dCp;
    delete [] Denom;

}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER UpdateVortexEdgeStrengths                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::UpdateVortexEdgeStrengths(int Level)
{

    int i, j, Node1, Node2;

    // Copy current value over to vortex loops
    
    if ( Level == 1 ) {
     
#pragma omp parallel for    
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
        
          VortexLoop(i).Gamma() = Gamma_[i];

       }
       
    }
    
    // Calculate delta-gammas for each surface vortex edge
    
#pragma omp parallel for       
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {

       VSPGeom().Grid(Level).EdgeList(i).Gamma() = VSPGeom().Grid(Level).LoopList(VSPGeom().Grid(Level).EdgeList(i).VortexLoop1()).Gamma()
                                                 - VSPGeom().Grid(Level).LoopList(VSPGeom().Grid(Level).EdgeList(i).VortexLoop2()).Gamma();
                         
    }    

    // Calculate node wise delta gammas on finest grid

    if ( Level == 1 ) {

#pragma omp parallel for
       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

          VSPGeom().Grid(Level).NodeList(j).dGamma() = 0.;
          
       }

#pragma omp parallel for private(Node1,Node2)        
       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
        
          if ( VSPGeom().Grid(Level).EdgeList(i).IsTrailingEdge() ) {     

             Node1 = VSPGeom().Grid(Level).EdgeList(i).Node1();
             Node2 = VSPGeom().Grid(Level).EdgeList(i).Node2();
             
             VSPGeom().Grid(Level).NodeList(Node1).dGamma() += VSPGeom().Grid(Level).EdgeList(i).Gamma();
             VSPGeom().Grid(Level).NodeList(Node2).dGamma() -= VSPGeom().Grid(Level).EdgeList(i).Gamma();
             
          }
          
       }

       // Calculate delta-gammas for each trailing vortex edge
    
#pragma omp parallel for private(Node1)          
       for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {

          Node1 = TrailingVortexEdge(i).Node();
          
          TrailingVortexEdge(i).Gamma() = VSPGeom().Grid(Level).NodeList(Node1).dGamma();
 
       }
       
    }
    
}

/*##############################################################################
#                                                                              #
#                      VSP_SOLVER OutputStatusFile                             #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::OutputStatusFile(int Type)
{

    int i;
    double E, AR, ToQS;
    
    AR = Bref_ * Bref_ / Sref_;

    ToQS = 0.;
    
    for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
     
       ToQS += RotorDisk(i).RotorThrust() / ( 0.5 * Density_ * Vinf_ * Vinf_ * Sref_);
       
    }    
 
    E = (CL(Type) *CL(Type) /(PI * AR)) / CD(Type) ;
 
    i = CurrentWakeIteration_;
    
    if ( Type == 1 ) {
       
       i = 99999;
       
       fprintf(StatusFile_,"\n\n\n");
       
    }
    
    fprintf(StatusFile_,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf\n",
            i,
            Mach_,
            AngleOfAttack_/TORAD,
            AngleOfBeta_/TORAD,
            CL(Type),
            CDo(),
            CD(Type),
            CDo() + CD(Type),
            CS(Type),            
            CL(Type)/(CDo() + CD(Type)),
            E,
            CFx(Type),
            CFy(Type),
            CFz(Type),
            CMx(Type),
            CMy(Type),
            CMz(Type),
            ToQS);

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER OutputZeroLiftDragToStatusFile                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::OutputZeroLiftDragToStatusFile(void)
{
 
    int i;
    
    fprintf(StatusFile_,"\n");
    fprintf(StatusFile_,"\n");
    fprintf(StatusFile_,"\n");    
    fprintf(StatusFile_,"Skin Fiction Drag Break Out:\n");    
    fprintf(StatusFile_,"\n");   
    fprintf(StatusFile_,"\n");       
                       //1234567890123456789012345678901234567890: 123456789
    fprintf(StatusFile_,"Surface                                      CDo \n");
    fprintf(StatusFile_,"\n");
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       fprintf(StatusFile_,"%-40s: %9.5lf \n",
               VSPGeom().VSP_Surface(i).ComponentName(),
               VSPGeom().VSP_Surface(i).CDo());

    } 
}

