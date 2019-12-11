//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Solver.H"

/*##############################################################################
#                                                                              #
#                      Allocate space for statics                              #
#                                                                              #
##############################################################################*/

    double VSP_SOLVER::FarAway_ = 5.;
    
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
 
    DoSymmetryPlaneSolve_ = 0;
    
    SetFarFieldDist_ = 0;
    
    NumberOfWakeTrailingNodes_ = 64; // Must be a power of 2
    
    SearchID_ = NULL;
    
    SaveRestartFile_ = 0;
    
    JacobiRelaxationFactor_ = 0.25;
    
    DumpGeom_ = 0;

    NoWakeIteration_ = 0;
    
    MaxTurningAngle_ = -1.;
    
    Clmax_2d_ = -1.;
     
    CDo_ = 0.;
    
    NumberOfKelvinConstraints_ = 0;
    
    LoadDeformationFile_ = 0;
    
    Write2DFEMFile_ = 0;
    
    TimeAccurate_ = 0;
    
    StartFromSteadyState_ = 0;
    
    TimeStep_ = 0.;
    
    TimeAnalysisType_ = 0;

    NumberOfTimeSteps_ = 1;
    
    ReducedFrequency_ = 0.0;
    
    Unsteady_AngleRate_ = 0.;
    
    Unsteady_Angle_ = 0.;
    
    Unsteady_AngleMax_ = 0.;
    
    Unsteady_H_ = 0.;
    
    Unsteady_HMax_ = 0.;
    
    Preconditioner_ = MATCON;

    sprintf(CaseString_,"No Comment");
    
    VortexSheet_ = NULL;
    
     CL_Unsteady_ = NULL;
     CD_Unsteady_ = NULL;
     CS_Unsteady_ = NULL;
    CFx_Unsteady_ = NULL;
    CFy_Unsteady_ = NULL;
    CFz_Unsteady_ = NULL;
    CMx_Unsteady_ = NULL;
    CMy_Unsteady_ = NULL;
    CMz_Unsteady_ = NULL;
 
    AngleOfAttackZero_ = 0.;
    
    AngleOfBetaZero_ = 0.;   
    
    RotorAnalysis_ = 0;
    
    BladeRPM_ = 0.;
    
    NumberOfSurfaceNodes_ = 0;
    
    AllComponentsAreFixed_ = 1;
    
    ThereIsRelativeComponentMotion_ = 0;
    
    InteractionLoopList_[0] = NULL;
    
    InteractionLoopList_[1] = NULL;
    
    NumberOfInteractionLoops_[0] = 0;
    
    NumberOfInteractionLoops_[1] = 0;

    NumberOfVortexSheetInteractionLoops_ = NULL;
    
    VortexSheetInteractionLoopList_ = NULL;
    
    NumberOfVortexSheetInteractionEdges_ = NULL;
    
    NumberOfVortexSheetInteractionEdges_ = NULL;
    
    VortexSheetVortexToVortexSet_ = NULL;
    
    RotorFile_ = NULL;

    GroupFile_ = NULL;
    
    DoHoverRampFreeStream_ = 0;
    
    HoverRampFreeStreamVelocity_ = 0.;
    
    OriginalVinfHoverRamp_ = 0.;

    NoiseAnalysis_ = 0;
    
    SteadyStateNoise_ = 0;
    
    WopWopEnglishUnits_ = 0;
    
    WopWopFlyBy_ = 1;
    
    WopWopPeriodicity_ = WOPWOP_PERIODIC;

    WopWopWriteOutADBFile_ = 0;
    
    WopWopUserStartTime_ = 0.;
    
    WopWopUserFinishTime_ = 0.;

    NoiseInterpolation_ = NOISE_QUINTIC_HERMITE_INTERPOLATION;
    
    KarmanTsienCorrection_ = 0.;
    
    KTResidual_[1] = 0.;
    
    LastMach_ = -1.;
    
    KelvinLambda_ = 1.;

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
 
    int c, i, j, k, cpu, NumberOfStations, MaxEdges, Level, Hits, CompSurfs;
    int NumSteps_1, NumSteps_2;
    double Area, Scale_X, Scale_Y, Scale_Z, FarDist, Period;
    char GroupFileName[2000], DumChar[2000];
    FILE *GroupFile;
    
    // Save a copy of free stream velocity 
    
    OriginalVinfHoverRamp_ = Vinf_;
    
    if ( DoHoverRampFreeStream_ ) {
       
       Vinf_ = HoverRampFreeStreamVelocity_;
       
    }
        
    // Copy over data
    
    MGLevel_ = 1;
    
    NumberOfMGLevels_ = VSPGeom().NumberOfGridLevels();

    // Size the lists and determine the size of the domain
    
    Xmin_ = 1.e9;
    Xmax_ = -Xmin_;

    Ymin_ = 1.e9;
    Ymax_ = -Ymin_;

    Zmin_ = 1.e9;
    Zmax_ = -Zmin_;

    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfNodes() ; j++ ) {

       Xmin_ = MIN(Xmin_,VSPGeom().Grid(MGLevel_).NodeList(j).x());
       Xmax_ = MAX(Xmax_,VSPGeom().Grid(MGLevel_).NodeList(j).x());

       Ymin_ = MIN(Ymin_,VSPGeom().Grid(MGLevel_).NodeList(j).y());
       Ymax_ = MAX(Ymax_,VSPGeom().Grid(MGLevel_).NodeList(j).y());

       Zmin_ = MIN(Zmin_,VSPGeom().Grid(MGLevel_).NodeList(j).z());
       Zmax_ = MAX(Zmax_,VSPGeom().Grid(MGLevel_).NodeList(j).z());

    }
    
    // Determine how far to allow wakes to adapt... beyond this the wakes go straight off to
    // 'infinity' in the free stream direction
    
    Scale_X = Scale_Y = Scale_Z = 1.;
    
    if ( DoSymmetryPlaneSolve_ == SYM_X ) Scale_X = 2.;
    if ( DoSymmetryPlaneSolve_ == SYM_Y ) Scale_Y = 2.;
    if ( DoSymmetryPlaneSolve_ == SYM_Z ) Scale_Z = 2.;
    
    FarDist = MAX3(Scale_X*(Xmax_ - Xmin_), Scale_Y*(Ymax_-Ymin_), Scale_Z*(Zmax_-Zmin_));
    
    printf("Xmax_ - Xmin_: %f \n",Xmax_ - Xmin_);
    printf("Ymax_ - Ymin_: %f \n",Ymax_ - Ymin_);
    printf("Zmax_ - Zmin_: %f \n",Zmax_ - Zmin_);
    printf("\n");
    
    // Override far field distance
    
    if ( SetFarFieldDist_ ) {
       
       FarDist = FarFieldDist_;
       
    }
    
    else {
       
       FarFieldDist_ = FarDist;
       
    }
    
    printf("Wake FarDist set to: %f \n",FarDist);

    // Find average lifting chord
    
    AverageVehicleChord_ = 0;
    
    Hits = 0;
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 

       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
          
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();

          for ( k = 1 ; k <= NumberOfStations ; k++ ) {
    
             // Chord
             
             AverageVehicleChord_ += VSPGeom().VSP_Surface(i).LocalChord(k);
             
             Hits++;
             
          }
          
       }
       
    }
                 
    AverageVehicleChord_ /= Hits;

    // Allocate space for the vortex edges
    
    NumberOfSurfaceNodes_        = VSPGeom().Grid(MGLevel_).NumberOfNodes();

    NumberOfSurfaceVortexEdges_  = VSPGeom().Grid(MGLevel_).NumberOfEdges();
    
    NumberOfTrailingVortexEdges_ = VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes();
    
    NumberOfVortexLoops_         = VSPGeom().Grid(MGLevel_).NumberOfLoops();
    
    printf("Number Of Trailing Vortices: %d \n",NumberOfTrailingVortexEdges_);
    
    // Allocate space for component level data
    
    GeometryComponentIsFixed_ = new int[VSPGeom().NumberOfComponents() + 1];
    
    GeometryGroupID_ = new int[VSPGeom().NumberOfComponents() + 1];
            
    // Set all geometry as fixed by default
            
    for ( i = 1 ; i <= VSPGeom().NumberOfComponents() ; i++ ) {
       
       GeometryComponentIsFixed_[i] = 1;
       
       GeometryGroupID_[i] = 0;
       
    }
    
    // If this is an unsteady, path following analysis read in the 
    // group data
    
    NumberOfComponentGroups_ = 0;
    
    if ( TimeAccurate_ ) {
       
       if ( TimeAnalysisType_ != P_ANALYSIS &&
            TimeAnalysisType_ != Q_ANALYSIS &&
            TimeAnalysisType_ != R_ANALYSIS  ) {

          // Open the group file
   
          sprintf(GroupFileName,"%s.groups",FileName_);
          
          if ( (GroupFile = fopen(GroupFileName, "r")) == NULL ) {
      
             printf("Could not open the group file for input! \n");
      
             exit(1);
      
          }
    
          // Load in the data
          
          fscanf(GroupFile,"%d\n",&NumberOfComponentGroups_);
          
          printf("There are %d component groups \n",NumberOfComponentGroups_);
          
          ComponentGroupList_ = new COMPONENT_GROUP[NumberOfComponentGroups_ + 1];
          
          for ( i = 1 ; i <= NumberOfComponentGroups_ ; i++ ) {
             
             fgets(DumChar,80,GroupFile);
             
             ComponentGroupList_[i].LoadData(GroupFile);
             
          }
          
          fclose(GroupFile);
          
       }
       
       else {
          
          NumberOfComponentGroups_ = 1;
          
          ComponentGroupList_ = new COMPONENT_GROUP[NumberOfComponentGroups_ + 1];
       
          sprintf(ComponentGroupList_[1].GroupName(),"%s",FileName_);
          
          ComponentGroupList_[1].GeometryIsDynamic() = 2;

          // Determine angular rates and reduced frequency for damping analyses

          TimeStep_ = (FarFieldDist_/8.) / ( Vinf_ );
             
          Unsteady_AngleRate_ = 2. * PI / (TimeStep_ * NumberOfTimeSteps_);

//djk
// 2d wing test case
//TimeStep_ = 0.25;
//Unsteady_AngleRate_ =  0.666667 ;
//NumberOfTimeSteps_ = 512;          
//Unsteady_AngleMax_ = 5.;

//TimeStep_ = 1.0;
//Unsteady_AngleRate_ =  0.333333 ;
//NumberOfTimeSteps_ = 512;          
//Unsteady_AngleMax_ = 5.;

//TimeStep_ = 2.5;
//Unsteady_AngleRate_ =  0.0666667 ;
//NumberOfTimeSteps_ = 512;          
//Unsteady_AngleMax_ = 5.;


          if ( TimeAnalysisType_ == P_ANALYSIS) ReducedFrequency_ = 0.5 * Unsteady_AngleRate_ * Bref_ / Vinf_;
          if ( TimeAnalysisType_ == Q_ANALYSIS) ReducedFrequency_ = 0.5 * Unsteady_AngleRate_ * Cref_ / Vinf_;
          if ( TimeAnalysisType_ == R_ANALYSIS) ReducedFrequency_ = 0.5 * Unsteady_AngleRate_ * Bref_ / Vinf_;

          ComponentGroupList_[1].Omega() = Unsteady_AngleRate_;
          ComponentGroupList_[1].AngleMax() = Unsteady_AngleMax_;

          ComponentGroupList_[1].OVec(0) = XYZcg_[0];
          ComponentGroupList_[1].OVec(1) = XYZcg_[1];
          ComponentGroupList_[1].OVec(2) = XYZcg_[2];
          
          // Roll analysis
          
          if ( TimeAnalysisType_ == P_ANALYSIS ) {

            ComponentGroupList_[1].RVec(0) = -1.;
            ComponentGroupList_[1].RVec(1) =  0.;
            ComponentGroupList_[1].RVec(2) =  0.;
            
          }
          
          // Pitch analysis
          
          else if ( TimeAnalysisType_ == Q_ANALYSIS ) {
             
            ComponentGroupList_[1].RVec(0) =  0.;
            ComponentGroupList_[1].RVec(1) =  1.;
            ComponentGroupList_[1].RVec(2) =  0.;
            
          }
          
          // Yaw analysis
          
          else if ( TimeAnalysisType_ == R_ANALYSIS ) {
             
            ComponentGroupList_[1].RVec(0) =  0.;
            ComponentGroupList_[1].RVec(1) =  0.;
            ComponentGroupList_[1].RVec(2) = -1.;
            
          }
          
          else {
             
             printf("Unknown time analysis type! \n");fflush(NULL);
             exit(1);
             
          }
                    
          ComponentGroupList_[1].SizeList(VSPGeom().NumberOfComponents());
          
          for ( j = 1 ; j <= VSPGeom().NumberOfComponents() ; j++ ) {
             
             ComponentGroupList_[1].ComponentList(j) = j;
             
          }

       }

    }
    
    // Steady state (or possible a steady state rotor) calculation
    
    else {
       
       NumberOfComponentGroups_ = 1;
       
       ComponentGroupList_ = new COMPONENT_GROUP[NumberOfComponentGroups_ + 1];
       
       sprintf(ComponentGroupList_[1].GroupName(),"%s",FileName_);

       if ( RotorAnalysis_ ) {
      
          ComponentGroupList_[1].GeometryIsARotor() = 1;
          ComponentGroupList_[1].Omega() = BladeRPM_ * 2. * PI / 60.;
          ComponentGroupList_[1].RotorDiameter() = Bref_;
          
       }
       
       ComponentGroupList_[1].AngleMax() = 0.;
       
       ComponentGroupList_[1].OVec(0) = XYZcg_[0];
       ComponentGroupList_[1].OVec(1) = XYZcg_[1];
       ComponentGroupList_[1].OVec(2) = XYZcg_[2];

       ComponentGroupList_[1].RVec(0) = -1.;
       ComponentGroupList_[1].RVec(1) =  0.;
       ComponentGroupList_[1].RVec(2) =  0.;
                   
       ComponentGroupList_[1].SizeList(VSPGeom().NumberOfComponents());
       
       for ( j = 1 ; j <= VSPGeom().NumberOfComponents() ; j++ ) {
          
          ComponentGroupList_[1].ComponentList(j) = j;
          
       }
   
    }       

    // Check that components listed by user seem valid
    
    printf("NumberOfComponentGroups_: %d \n",NumberOfComponentGroups_);
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
          
          if ( ComponentGroupList_[c].ComponentList(j) > VSPGeom().NumberOfComponents() ) {
             
             printf("Component Group: %d lists non-existant Component: %d \n", c, ComponentGroupList_[c].ComponentList(j));
             
             fflush(NULL);exit(1);
          
          }    

       }
       
    }

    // Determine the number of lifting surfaces per component
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
          
       CompSurfs = 0;

       for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {

          for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
 
             if ( ComponentGroupList_[c].ComponentList(j) == VSPGeom().VSP_Surface(i).ComponentID() ) {
             
                if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) CompSurfs++;
             
             }
             
          }

       }
       
       printf("Found %d surfaces for component group: %d \n",CompSurfs, c);
       
       ComponentGroupList_[c].SizeSpanLoadingList(CompSurfs);

       CompSurfs = 0;

       for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
          
          for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
             
             if ( ComponentGroupList_[c].ComponentList(j) == VSPGeom().VSP_Surface(i).ComponentID() ) {
                
                if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
                
                   CompSurfs++;
                
                   ComponentGroupList_[c].SpanLoadData(CompSurfs).SurfaceID() = i;
                
                   ComponentGroupList_[c].SpanLoadData(CompSurfs).SizeList(VSPGeom().VSP_Surface(i).NumberOfSpanStations());
                   
                }
                                                
             }
             
          }

       }
              
    }

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
          
          if ( ComponentGroupList_[c].ComponentList(j) > VSPGeom().NumberOfComponents() ) {
             
             printf("Component Group: %d lists non-existant Component: %d \n", c, ComponentGroupList_[c].ComponentList(j));
             
             fflush(NULL);exit(1);
          
          }    

       }
       
    }
    
    // Determine which component groups are fixed
       
    AllComponentsAreFixed_ = 0;
            
    for ( i = 1 ; i <= NumberOfComponentGroups_ ; i++ ) {
       
       if ( !ComponentGroupList_[i].GeometryIsFixed() ) {
       
          for ( j = 1 ; j <= ComponentGroupList_[i].NumberOfComponents() ; j++ ) {
          
             GeometryComponentIsFixed_[ComponentGroupList_[i].ComponentList(j)] = 0;

          }

       }
       
       else {
          
          AllComponentsAreFixed_++;
          
       }
 
       for ( j = 1 ; j <= ComponentGroupList_[i].NumberOfComponents() ; j++ ) {

          GeometryGroupID_[ComponentGroupList_[i].ComponentList(j)] = i;
          
       }
              
    }

    if ( AllComponentsAreFixed_ == NumberOfComponentGroups_ ) {
       
       AllComponentsAreFixed_ = 1;
       
    }
    
    else {
        
       AllComponentsAreFixed_ = 0;
       
    }

    printf("AllComponentsAreFixed_: %d \n",AllComponentsAreFixed_);
    
    // If all components are not fixed, check that there is indeed relative motion
    
    if ( AllComponentsAreFixed_ == 0 && NumberOfComponentGroups_ > 1 ) ThereIsRelativeComponentMotion_ = 1;

    // Unsteady analysis
    
    if ( TimeAccurate_ ) {

       // Calculate a default time step if user did not set it
       
       if ( TimeStep_ <= 0. ) {
          
          TimeStep_ = 0.5*SigmaAvg_;
       
          // Determine the slowest and fastest rotor
          
          WopWopOmegaMax_ = -1.e9;
          WopWopOmegaMin_ =  1.e9;
          
          for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
          
             if ( ComponentGroupList_[c].GeometryIsARotor() ) {

                if ( ABS(ComponentGroupList_[c].Omega()) < WopWopOmegaMin_ ) {
          
                   WopWopOmegaMin_ = ABS(ComponentGroupList_[c].Omega());
                   
                }
                          
                if ( ABS(ComponentGroupList_[c].Omega()) > WopWopOmegaMax_ ) {
          
                   WopWopOmegaMax_ = ABS(ComponentGroupList_[c].Omega());
                   
                }
                          
             }
             
          }
          
          if ( WopWopOmegaMax_ > 0. ) {

             TimeStep_ = 15. * TORAD / WopWopOmegaMax_;
             
             printf("Using a time step of: %f based on fastest rotor \n",TimeStep_);
             
          }
          
       }
       
       // Calculate the number of time steps if user did not set it
       
       if ( NumberOfTimeSteps_ < 0 ) {
          
          // Slowest rotor, at least 1 rotation
          
          Period = 2.*PI / WopWopOmegaMin_;
          
          NumSteps_1 = 2.*Period / TimeStep_ + 1;
          
          // Fastest rotor does ABS(NumberOfTimeSteps_) revolutions
          
          NumSteps_2 = ABS(NumberOfTimeSteps_)*24;
          
          if ( NumSteps_1 > NumSteps_2 ) {
             
             NumberOfTimeSteps_ = NumSteps_1;
          
             printf("Setting number of time steps to %d based on slowest rotor making 2 rotations. \n",NumberOfTimeSteps_);
             
          }
          
          else {
             
             NumberOfTimeSteps_ = NumSteps_2;
          
             printf("Setting number of time steps to %d based on fastest rotor making %d rotations. \n",NumberOfTimeSteps_,ABS(NumberOfTimeSteps_));
             
          }
          
       }
       
       // Determine average start times for each rotor ... one rotor revolution
       
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
          if ( ComponentGroupList_[c].GeometryIsARotor() ) {
             
             Period = 2.*PI / ABS(ComponentGroupList_[c].Omega());
             
             ComponentGroupList_[c].StartAverageTime() = NumberOfTimeSteps_ * TimeStep_ - Period - TimeStep_;
       
          }
          
       }       

       printf("Used FarFieldDist_ of: %f to calculate time step \n",FarFieldDist_);
           
       printf("TimeAnalysisType_: %d \n",TimeAnalysisType_);
       
       printf("TimeStep_: %f \n",TimeStep_);

       printf("ReducedFrequency_: %f \n",ReducedFrequency_);
       
       printf("Unsteady_AngleRate_: %f \n",Unsteady_AngleRate_);

    }

    // VLM model
    
    if ( ModelType_ == VLM_MODEL ) {
       
       NumberOfKelvinConstraints_ = 0;

       LoopIsOnBaseRegion_ = new int[NumberOfVortexLoops_ + 1];

       zero_int_array(LoopIsOnBaseRegion_, NumberOfVortexLoops_);
    
    }
    
    // Panel Model
    
    else if ( ModelType_ == PANEL_MODEL ) {
       
       DetermineNumberOfKelvinConstrains();
    
    }
    
    // Uknown model
    
    else {
       
       printf("Unknown Model Type! \n");fflush(NULL);
       
       exit(1);
       
    }
    
    NumberOfEquations_ = NumberOfVortexLoops_ + NumberOfKelvinConstraints_;

    // Allocate space for the vortex edges and loops
  
    SurfaceVortexEdge_ = new VSP_EDGE*[NumberOfSurfaceVortexEdges_ + 1];

    TrailingVortexEdge_ = new VORTEX_TRAIL[NumberOfTrailingVortexEdges_ + 1];
    
    VortexLoop_ = new VSP_LOOP*[NumberOfVortexLoops_ + 1];
  
    UnsteadyTrailingWakeVelocity_ =  new double*[NumberOfVortexLoops_ + 1];  
    
    LocalBodySurfaceVelocity_ =  new double*[NumberOfVortexLoops_ + 1];  
        
    Gamma_[0] = new double[NumberOfVortexLoops_ + 1];    
    Gamma_[1] = new double[NumberOfVortexLoops_ + 1];    
    Gamma_[2] = new double[NumberOfVortexLoops_ + 1];    

    Diagonal_ = new double[NumberOfVortexLoops_ + 1];     

    Delta_= new double[NumberOfVortexLoops_ + 1];     
   
    zero_double_array(Gamma_[0], NumberOfVortexLoops_); Gamma_[0][0] = 0.;   
    zero_double_array(Gamma_[1], NumberOfVortexLoops_); Gamma_[1][0] = 0.;   
    zero_double_array(Gamma_[2], NumberOfVortexLoops_); Gamma_[2][0] = 0.;   

    zero_double_array(Diagonal_, NumberOfVortexLoops_); Diagonal_[0] = 0.;    
    zero_double_array(Delta_,    NumberOfVortexLoops_);    Delta_[0] = 0.;
   
    Residual_ = new double[NumberOfEquations_ + 1];    

    RightHandSide_ = new double[NumberOfEquations_ + 1];     
     
    MatrixVecTemp_ = new double[NumberOfEquations_ + 1];     
   
    zero_double_array(Residual_,      NumberOfEquations_); Residual_[0]      = 0.;
    zero_double_array(RightHandSide_, NumberOfEquations_); RightHandSide_[0] = 0.;
    zero_double_array(MatrixVecTemp_, NumberOfEquations_); RightHandSide_[0] = 0.;
         
    if ( NoiseAnalysis_ ) {
       
       NumberOfNoiseInterpolationPoints_ = 9;
       
       for ( i = 0 ; i < NumberOfNoiseInterpolationPoints_ ; i++ ) {
          
          GammaNoise_[i] = new double[NumberOfVortexLoops_ + 1];    
    
          FxNoise_[i] = new double[NumberOfSurfaceVortexEdges_ + 1];    
          FyNoise_[i] = new double[NumberOfSurfaceVortexEdges_ + 1];    
          FzNoise_[i] = new double[NumberOfSurfaceVortexEdges_ + 1];    
   
          dCpUnsteadyNoise_[i] = new double[NumberOfVortexLoops_ + 1];    
            
          UNoise_[i] = new double[NumberOfVortexLoops_ + 1];    
          VNoise_[i] = new double[NumberOfVortexLoops_ + 1];    
          WNoise_[i] = new double[NumberOfVortexLoops_ + 1];    
   
          zero_double_array(GammaNoise_[i], NumberOfVortexLoops_); GammaNoise_[i][0] = 0.;   
   
          zero_double_array(FxNoise_[i], NumberOfSurfaceVortexEdges_); FxNoise_[i][0] = 0.;   
          zero_double_array(FyNoise_[i], NumberOfSurfaceVortexEdges_); FyNoise_[i][0] = 0.;   
          zero_double_array(FzNoise_[i], NumberOfSurfaceVortexEdges_); FzNoise_[i][0] = 0.;   
   
          zero_double_array(dCpUnsteadyNoise_[i], NumberOfVortexLoops_); dCpUnsteadyNoise_[i][0] = 0.;   
   
          zero_double_array(UNoise_[i], NumberOfVortexLoops_); UNoise_[i][0] = 0.;   
          zero_double_array(VNoise_[i], NumberOfVortexLoops_); VNoise_[i][0] = 0.;   
          zero_double_array(WNoise_[i], NumberOfVortexLoops_); WNoise_[i][0] = 0.;   
       
       }

    }
  
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       UnsteadyTrailingWakeVelocity_[i] = new double[3];
       
       LocalBodySurfaceVelocity_[i] = new double[3];
       
       // Zero out trailing wake velocities
       
       UnsteadyTrailingWakeVelocity_[i][0] = 0.;
       UnsteadyTrailingWakeVelocity_[i][1] = 0.;
       UnsteadyTrailingWakeVelocity_[i][2] = 0.;
       
       // Zero out local body surface velocities
       
       LocalBodySurfaceVelocity_[i][0] = 0.;
       LocalBodySurfaceVelocity_[i][1] = 0.;
       LocalBodySurfaceVelocity_[i][2] = 0.;         

    }      
    
    // Allocate space for span loading data
            
    Span_Cx_      = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cy_      = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cz_      = new double*[VSPGeom().NumberOfSurfaces() + 1];

    Span_Cxo_     = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cyo_     = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Czo_     = new double*[VSPGeom().NumberOfSurfaces() + 1];
                   
    Span_Cxi_     = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cyi_     = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Czi_     = new double*[VSPGeom().NumberOfSurfaces() + 1];

    Span_Cmx_     = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmy_     = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmz_     = new double*[VSPGeom().NumberOfSurfaces() + 1];    
               
    Span_Cmxo_    = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmyo_    = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmzo_    = new double*[VSPGeom().NumberOfSurfaces() + 1];    
               
    Span_Cmxi_    = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmyi_    = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmzi_    = new double*[VSPGeom().NumberOfSurfaces() + 1];    
                              
    Span_Cn_      = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cl_      = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cs_      = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cd_      = new double*[VSPGeom().NumberOfSurfaces() + 1];
               
    Span_Cmx_     = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmy_     = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmz_     = new double*[VSPGeom().NumberOfSurfaces() + 1];    
               
    Span_Yavg_    = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Area_    = new double*[VSPGeom().NumberOfSurfaces() + 1];
    
    Local_Vel_[0] = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Local_Vel_[1] = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Local_Vel_[2] = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Local_Vel_[3] = new double*[VSPGeom().NumberOfSurfaces() + 1];
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
        
       NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
 
       Span_Cx_[i]       = new double[NumberOfStations + 1];                        
       Span_Cy_[i]       = new double[NumberOfStations + 1];                        
       Span_Cz_[i]       = new double[NumberOfStations + 1];

       Span_Cxo_[i]      = new double[NumberOfStations + 1];                        
       Span_Cyo_[i]      = new double[NumberOfStations + 1];                        
       Span_Czo_[i]      = new double[NumberOfStations + 1];
                               
       Span_Cxi_[i]      = new double[NumberOfStations + 1];                        
       Span_Cyi_[i]      = new double[NumberOfStations + 1];                        
       Span_Czi_[i]      = new double[NumberOfStations + 1];

       Span_Cmx_[i]      = new double[NumberOfStations + 1];                         
       Span_Cmy_[i]      = new double[NumberOfStations + 1];                         
       Span_Cmz_[i]      = new double[NumberOfStations + 1];

       Span_Cmxo_[i]     = new double[NumberOfStations + 1];                         
       Span_Cmyo_[i]     = new double[NumberOfStations + 1];                         
       Span_Cmzo_[i]     = new double[NumberOfStations + 1];
       
       Span_Cmxi_[i]     = new double[NumberOfStations + 1];                         
       Span_Cmyi_[i]     = new double[NumberOfStations + 1];                         
       Span_Cmzi_[i]     = new double[NumberOfStations + 1];
                                       
       Span_Cn_[i]       = new double[NumberOfStations + 1];                         
       Span_Cl_[i]       = new double[NumberOfStations + 1];                         
       Span_Cs_[i]       = new double[NumberOfStations + 1];                         
       Span_Cd_[i]       = new double[NumberOfStations + 1];
            
       Span_Yavg_[i]     = new double[NumberOfStations + 1];                         
       Span_Area_[i]     = new double[NumberOfStations + 1];
       
       Local_Vel_[0][i]  = new double[NumberOfStations + 1];
       Local_Vel_[1][i]  = new double[NumberOfStations + 1];
       Local_Vel_[2][i]  = new double[NumberOfStations + 1];
       Local_Vel_[3][i]  = new double[NumberOfStations + 1];
 
       zero_double_array(Span_Cx_[i],       NumberOfStations);
       zero_double_array(Span_Cy_[i],       NumberOfStations);
       zero_double_array(Span_Cz_[i],       NumberOfStations);      
                                            
       zero_double_array(Span_Cxi_[i],      NumberOfStations);
       zero_double_array(Span_Cyi_[i],      NumberOfStations);
       zero_double_array(Span_Czi_[i],      NumberOfStations);      
                                            
       zero_double_array(Span_Cn_[i],       NumberOfStations);       
                                            
       zero_double_array(Span_Cl_[i],       NumberOfStations);
       zero_double_array(Span_Cs_[i],       NumberOfStations);
       zero_double_array(Span_Cd_[i],       NumberOfStations); 
                                            
       zero_double_array(Span_Cmx_[i],      NumberOfStations);
       zero_double_array(Span_Cmy_[i],      NumberOfStations);
       zero_double_array(Span_Cmz_[i],      NumberOfStations);       
                                            
       zero_double_array(Span_Yavg_[i],     NumberOfStations);       
       zero_double_array(Span_Area_[i],     NumberOfStations);
       
       zero_double_array(Local_Vel_[0][i],  NumberOfStations);
       zero_double_array(Local_Vel_[1][i],  NumberOfStations);
       zero_double_array(Local_Vel_[2][i],  NumberOfStations);
       zero_double_array(Local_Vel_[3][i],  NumberOfStations);
     
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
    
    // Allocate space if this is the first time through
    
    if ( FirstTimeSetup_ ) {
       
       StartTime_ = myclock();
  
       // Determine the maximum number of edges across all grid levels
       
       MaxEdges  = 0;
       
       for ( Level = 1 ; Level <= VSPGeom().NumberOfGridLevels() ; Level++ ) {
          
          MaxEdges += VSPGeom().Grid(Level).NumberOfEdges();
          
       }
       
       // Allocate space for temporary interaction lists... one per processor
       
#ifdef VSPAERO_OPENMP

       NumberOfThreads_ = omp_get_max_threads();

#else
       NumberOfThreads_ = 1;

#endif       

       SearchID_ = new int[NumberOfThreads_];

       EdgeIsUsed_ = new int**[NumberOfThreads_];
       
       TempInteractionList_ = new VSP_EDGE**[NumberOfThreads_];
       
       LoopStackList_ = new STACK_ENTRY*[NumberOfThreads_];
       
       // Now size the list per processor
       
       for ( cpu = 0 ; cpu < NumberOfThreads_ ; cpu++ ) {
          
          SearchID_[cpu] = 0;

          EdgeIsUsed_[cpu] = new int*[VSPGeom().NumberOfGridLevels() + 1];
          
          for ( Level = VSPGeom().NumberOfGridLevels() ; Level >= 1  ; Level-- ) {
           
             EdgeIsUsed_[cpu][Level] = new int[VSPGeom().Grid(Level).NumberOfEdges() + 1];
           
             zero_int_array(EdgeIsUsed_[cpu][Level], VSPGeom().Grid(Level).NumberOfEdges());
             
          }
   
          // Temporary interaction list
          
          TempInteractionList_[cpu] = new VSP_EDGE*[NumberOfSurfaceVortexEdges_ + 1];
                       
          MaxStackSize_ = 0;
   
          // Loop over all grid levels for this surface
          
          for ( Level = VSPGeom().NumberOfGridLevels() ; Level >= 1  ; Level-- ) {
   
             // Loop over all vortex loops
             
             MaxStackSize_ += VSPGeom().Grid(Level).NumberOfLoops();
            
          }
   
          LoopStackList_[cpu] = new STACK_ENTRY[MaxStackSize_ + 1];
          
       }
       
       // Create Matrix preconditioner
       
       if ( Preconditioner_ == MATCON ) CreateMatrixPreconditionersDataStructure();

       FirstTimeSetup_ = 0;
       
    }
    
    // If panel solver, or unsteady 
    
    if ( !DumpGeom_ && ModelType_ == PANEL_MODEL ) CreateVorticityGradientDataStructure();
    
}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER DetermineNumberOfKelvinConstrains                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DetermineNumberOfKelvinConstrains(void)
{
   
    int j, k, p, n, Edge, Node, Loop, Loop1, Loop2, Next, StackSize, Done, FoundOne;
    int Node1, Node2, *LoopStack, NotFlipped, KelvinGroup, Wing;
    double Vec1[3], Vec2[3], Dot;
        
    MGLevel_ = 1;
    
    // Traverse the loops, edge to edge, and determine the number of 
    // independent bodies... this is the number of Kelvin constraints
    // we have to impose
 
    LoopStack = new int[NumberOfVortexLoops_ + 1];
 
    LoopIsOnBaseRegion_ = new int[NumberOfVortexLoops_ + 1];

    LoopInKelvinConstraintGroup_ = new int[NumberOfVortexLoops_ + 1];
 
    zero_int_array(LoopStack, NumberOfVortexLoops_);
    
    zero_int_array(LoopIsOnBaseRegion_, NumberOfVortexLoops_);
    
    zero_int_array(LoopInKelvinConstraintGroup_, NumberOfVortexLoops_);

    NumberOfKelvinConstraints_ = 1;

    StackSize = Next = 1;
    
    LoopStack[Next] = 1;
    
    LoopInKelvinConstraintGroup_[1] = NumberOfKelvinConstraints_;
    
    Done = 0;
    
    while ( !Done ) {
       
       while ( Next <= StackSize ) {
          
          Loop = LoopStack[Next];
          
          for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).LoopList(Loop).NumberOfEdges() ; j++ ) {
             
             Edge = VSPGeom().Grid(MGLevel_).LoopList(Loop).Edge(j);
             
             Loop1 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Loop1();
             
             Loop2 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Loop2();
             
             if ( LoopInKelvinConstraintGroup_[Loop1] == 0 ) {
                             
                LoopInKelvinConstraintGroup_[Loop1] = NumberOfKelvinConstraints_;
                
                LoopStack[++StackSize] = Loop1;
                
             }
             
             if ( LoopInKelvinConstraintGroup_[Loop2] == 0 ) {
                
                LoopInKelvinConstraintGroup_[Loop2] = NumberOfKelvinConstraints_;

                LoopStack[++StackSize] = Loop2;
        
             }          
              
          }
          
          Next++;   
  
       }   

       FoundOne = 0;
       
       j = 1;
       
       while ( j <= VSPGeom().Grid(MGLevel_).NumberOfLoops() && !FoundOne ) {
          
          if ( LoopInKelvinConstraintGroup_[j] == 0 ) {
             
             StackSize = Next = 1;
             
             LoopStack[Next] = j;
    
             LoopInKelvinConstraintGroup_[j] = ++NumberOfKelvinConstraints_;
             
             FoundOne = 1;
             
          }
          
          j++;
          
       }
       
       if ( !FoundOne ) Done = 1;
       
    }
    
    // Determine the number of vortex sheets
    
    NumberOfVortexSheets_ = 0;
    
    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {

       NumberOfVortexSheets_ = MAX(NumberOfVortexSheets_, VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j));
       
    }   

    printf("There are: %10d Vortex Sheets \n", NumberOfVortexSheets_);
             
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       // Find a node on this vortex sheet
       
       FoundOne = 0;
       
       j = 1;
            
       while ( !FoundOne && j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ) {
   
          Wing = VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j);
        
          if ( Wing == k && VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNodeIsPeriodic(j) ) {
  
             Node = VSPGeom().Grid(MGLevel_).KuttaNode(j);;
             
             FoundOne = 1;
             
          }
          
          j++;
          
       }

       if ( FoundOne ) {

          printf("Looking for node: %d \n",Node);fflush(NULL);
          
          // Grab a loop that contains this kutta node
         
          FoundOne = 0;
         
          n = 1;
         
          while ( !FoundOne && n <= NumberOfVortexLoops_ ) {
            
             p = 1;
                        
             while ( p <= VSPGeom().Grid(MGLevel_).LoopList(n).NumberOfEdges() && !FoundOne ) {
      
                Edge = VSPGeom().Grid(MGLevel_).LoopList(n).Edge(p);
               
                Node1 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Node1();
                Node2 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Node2();
               
                if ( Node == Node1 || Node == Node2 ) {
                  
                   Loop = n;
                  
                   FoundOne = 1;
                  
                }
               
                p++;
               
             }
            
             n++;
            
          }
         
          if ( !FoundOne ) {
            
             printf("Error in determining number of Kelvin regions for a periodic wake surface! \n");
             printf("Looking for node: %d \n",Node);
             fflush(NULL);
             exit(1);
            
          }
         
          // Go edge, by edge and see if we cover the entire kelvin region
         
          zero_int_array(LoopStack, NumberOfVortexLoops_);
         
          StackSize = Next = 1;
          
          LoopStack[Next] = Loop;
         
          KelvinGroup = LoopInKelvinConstraintGroup_[Loop];

          while ( Next <= StackSize ) {
             
             Loop = LoopStack[Next];
             
             for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).LoopList(Loop).NumberOfEdges() ; p++ ) {
                
                Edge = VSPGeom().Grid(MGLevel_).LoopList(Loop).Edge(p);
               
                if ( !VSPGeom().Grid(MGLevel_).EdgeList(Edge).IsTrailingEdge() ) {
               
                   Loop1 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Loop1();
                   
                   Loop2 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Loop2();

                   if ( LoopInKelvinConstraintGroup_[Loop1] == KelvinGroup ) {
                      
                      LoopInKelvinConstraintGroup_[Loop1] = -KelvinGroup;
            
                      if ( Loop1 != Loop ) LoopStack[++StackSize] = Loop1;
                      
                   }
                  
                   else if ( LoopInKelvinConstraintGroup_[Loop1] != -KelvinGroup ){
                     
                      printf("wtf... how did we jump to another Kelvin Group... \n"); fflush(NULL);
                      exit(1);
                      
                   }
                   
                   if ( LoopInKelvinConstraintGroup_[Loop2] == KelvinGroup ) {
                      
                      LoopInKelvinConstraintGroup_[Loop2] = -KelvinGroup;
    
                      if ( Loop2 != Loop ) LoopStack[++StackSize] = Loop2;
              
                   }    
                  
                   else if ( LoopInKelvinConstraintGroup_[Loop2] != -KelvinGroup ){
                     
                      printf("wtf... how did we jump to another Kelvin Group... \n"); fflush(NULL);
                      exit(1);
                     
                   }    
                  
                }                       
                 
             }
             
             Next++;   
     
          }
      
          // Check and see if there are any loops in this Kelvin group that were not flipped
         
          NotFlipped = 0;

          for ( p = 1 ; p <= NumberOfVortexLoops_ ; p++ ) {                  
                  
             if ( LoopInKelvinConstraintGroup_[p] == KelvinGroup ) {
               
                NotFlipped++;
               
             }
            
          }
            
          // If not all were flipped... then there is a base region
         
          if ( NotFlipped > 0 ) {
                              
             printf("Base region found for vortex sheet system: %d \n",k);fflush(NULL);
                                        
             // Determine which region ... + or - ... is the base region
            
             Vec1[0] = Vec1[1] = Vec1[2] = 0.;
            
             Vec2[0] = Vec2[1] = Vec2[2] = 0.;

             for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                if ( LoopInKelvinConstraintGroup_[p] ==  KelvinGroup ) {
                  
                   Vec1[0] += VSPGeom().Grid(MGLevel_).LoopList(p).Nx();
                   Vec1[1] += VSPGeom().Grid(MGLevel_).LoopList(p).Ny();
                   Vec1[2] += VSPGeom().Grid(MGLevel_).LoopList(p).Nz();
                  
                }
               
                if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                  
                   Vec2[0] += VSPGeom().Grid(MGLevel_).LoopList(p).Nx();
                   Vec2[1] += VSPGeom().Grid(MGLevel_).LoopList(p).Ny();
                   Vec2[2] += VSPGeom().Grid(MGLevel_).LoopList(p).Nz();
                  
                }                     
               
             }
            
             Dot = sqrt(vector_dot(Vec1,Vec1));
            
             Vec1[0] /= Dot; 
             Vec1[1] /= Dot; 
             Vec1[2] /= Dot; 
            
             Dot = sqrt(vector_dot(Vec2,Vec2));
            
             Vec2[0] /= Dot; 
             Vec2[1] /= Dot; 
             Vec2[2] /= Dot;
            
             // Remove those loops in the base region from the Kelvin group, and add them
             // to the base region list
            
             if ( Vec1[0] > Vec2[0] ) {
           
                for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                   if ( LoopInKelvinConstraintGroup_[p] == KelvinGroup ) {
                     
                      LoopIsOnBaseRegion_[p] = 1;
                     
                      LoopInKelvinConstraintGroup_[p] = KelvinGroup;
              
                   }
                  
                }
               
                for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                   if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                  
                      LoopInKelvinConstraintGroup_[p] = KelvinGroup;
              
                   }
                  
                }                     
               
             }
            
             else {
       
                for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                   if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                     
                      LoopIsOnBaseRegion_[p] = 1;
                     
                      LoopInKelvinConstraintGroup_[p] = KelvinGroup;
                     
                   }
                  
                }
               
                for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                   if ( LoopInKelvinConstraintGroup_[p] == KelvinGroup ) {
                   
                      LoopInKelvinConstraintGroup_[p] = KelvinGroup;
              
                   }
                  
                }                        
               
             }  
                   
          }
          
          else {
             
             for ( p = 1 ; p <= NumberOfVortexLoops_ ; p++ ) {                  
                  
                if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                  
                   LoopInKelvinConstraintGroup_[p] = KelvinGroup;
                  
                }
            
             }
          
          }
         
       }                 
       
    }

    printf("There are %d Kelvin constraints \n",NumberOfKelvinConstraints_);
    
    delete [] LoopStack; 
 
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
    
    // Local VortexLoop list points to finest grid vortex loop list
   
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
    
    int j, k, Node1, Node2, LoopL, LoopR, VortexLoop1, VortexLoop2, Level;

    VSP_NODE VSP_Node1, VSP_Node2;
    
    // Setup the surface vortex edge data
    
    k = 0;
    
    for ( Level = 1 ; Level <= NumberOfMGLevels_ ; Level++ ) {

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfEdges() ; j++ ) {
       
          // Pointer from grid edge to vortex edge
   
          VSPGeom().Grid(Level).EdgeList(j).VortexEdge() = ++k;
          
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
 
    int i, j, Level;
    double xyz[3], q[5], CA, SA, CB, SB, Rate_P, Rate_Q, Rate_R;
    double gamma, f1, gm1, gm2, gm3;
    VSP_NODE VSP_Node1, VSP_Node2;
   
    // Limit lower value of Mach number
    
    if ( Mach_ <= 0. ) Mach_ = 0.001;

    // Set Mach number for the edge class... it is static across all edge instances

    VSPGeom().Grid(1).EdgeList(1).SetMach(Mach_);    
    
    // Set multi-pole far away ratio
    
    FarAway_ = 5.;
    
    if ( Mach_ >= 1. ) FarAway_ = 9999999.;

    // Turn on KT correction
    
    if ( KarmanTsienCorrection_  && ModelType_ == VLM_MODEL ) {
       
      for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
          SurfaceVortexEdge(j).KTFact() = 1.125;
          
       }
       
       RestrictKTFactorFromGrid(1);
       
    }
    
    // Limits on max velocity, and min/max pressures

    gamma = 1.4;

    gm1 = gamma - 1.;

    gm2 = 0.5*gm1*SQR(Mach_);

    gm3 = 1./gm1;

    QMax_ = 0.75*sqrt( 1./gm2 + 1. );    
    
    CpMin_ = -2./(gamma*Mach_*Mach_);

    CpMin_ = MAX(-15.,CpMin_);

    if ( Mach_ < 1. ) {
       
  //     CpMin_ = CpMin_ * ( pow(1./(1.+pow(10.,4.-3.*Mach_)),0.05+0.35*SQR(1.-Mach_)) );
       
    }

    f1 = 1. + 0.5*(gamma-1.)*Mach_*Mach_;

    CpMax_ = 2.*( pow(f1,(gamma)/(gamma-1)) - 1. ) / ( 1.4*Mach_*Mach_ );
 
    DCpMax_ = CpMax_ - CpMin_;

    // Base pressure
    
    if ( Mach_ <= 1.1 ) {

       CpBase_ = -0.20 - 0.25/40.*pow(Mach_ + 1., 4.);

    }

    else if ( Mach_ > 1.1 && Mach_ <= 4. ) {

       CpBase_ = -1.418/pow(Mach_+1.,1.95);

    }

    else {

	    CpBase_ = -1./(Mach_*Mach_);

    }    
    
    // Zero out loop level local free stream velocities
    
    for ( Level = 0 ; Level <= NumberOfMGLevels_ ; Level++ ) {

       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

          VSPGeom().Grid(Level).LoopList(i).LocalFreeStreamVelocity(0) = 0.;
          VSPGeom().Grid(Level).LoopList(i).LocalFreeStreamVelocity(1) = 0.;   
          VSPGeom().Grid(Level).LoopList(i).LocalFreeStreamVelocity(2) = 0.;   
          VSPGeom().Grid(Level).LoopList(i).LocalFreeStreamVelocity(3) = 0.;   
          VSPGeom().Grid(Level).LoopList(i).LocalFreeStreamVelocity(4) = 0.;   
          
       }
       
    }
        
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
            
       if ( !TimeAccurate_ ) {
                       
          // Zero out trailing wake velocities
       
          UnsteadyTrailingWakeVelocity_[i][0] = 0.;
          UnsteadyTrailingWakeVelocity_[i][1] = 0.;
          UnsteadyTrailingWakeVelocity_[i][2] = 0.;
       
          // Zero out local body surface velocities
       
          LocalBodySurfaceVelocity_[i][0] = 0.;
          LocalBodySurfaceVelocity_[i][1] = 0.;
          LocalBodySurfaceVelocity_[i][2] = 0.;            
       
       } 
       
    }

    // Calculate body motion velocities
    
    if ( TimeAccurate_ && ( TimeAnalysisType_ == P_ANALYSIS || TimeAnalysisType_ == Q_ANALYSIS || TimeAnalysisType_ == R_ANALYSIS ) ) {
       
       Unsteady_Angle_ = ComponentGroupList_[1].Angle();
       
    }

    // Free stream velocity vector

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
    
    // If hover ramp is on... we gradually reduce the free stream velocity to zero

    if ( DoHoverRampFreeStream_ > 0 && Time_ > DoHoverRampFreeStream_ ) {

       Vinf_ *= 0.90;
       
       Vinf_ = MAX(OriginalVinfHoverRamp_, Vinf_);
     
       if ( ABS(Vinf_) > 1.e-6 ) printf("Reducing free stream velocity to: %f \n",Vinf_);
   
    } 

    FreeStreamVelocity_[0] = CA*CB * Vinf_;
    FreeStreamVelocity_[1] =   -SB * Vinf_;
    FreeStreamVelocity_[2] = SA*CB * Vinf_;

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
 
       // Calculate local free stream conditions
       
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(0) = FreeStreamVelocity_[0];
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(1) = FreeStreamVelocity_[1];
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(2) = FreeStreamVelocity_[2];
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(3) = 0.; // Delta-P
             
    }        
       
    // Rotational rates... note these rates are wrt to the body stability
    // axes... so we have to convert them to equivalent freestream velocities...
    // in the VSPAERO axes system with has X and Z pointing in the opposite
    // directions
    
    Rate_P = RotationalRate_[0];
    Rate_Q = RotationalRate_[1];
    Rate_R = RotationalRate_[2];
       
    // Add in rotational velocities

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       xyz[0] = VortexLoop(i).Xc() - Xcg();            
       xyz[1] = VortexLoop(i).Yc() - Ycg();           
       xyz[2] = VortexLoop(i).Zc() - Zcg();      

       // P - Roll

       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(0) += 0.;
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(1) += -xyz[2] * Rate_P;
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(2) += +xyz[1] * Rate_P;      
        
       // Q - Pitch

       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(0) += -xyz[2] * Rate_Q;
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(1) += 0.;
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(2) += +xyz[0] * Rate_Q;    

       // R - Yaw

       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(0) += -xyz[1] * Rate_R;
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(1) += +xyz[0] * Rate_R;
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(2) += 0.;    
           
    }  
    
    // Add in rotor induced velocities

    for ( j = 1 ; j <= NumberOfRotors_ ; j++ ) {
     
       RotorDisk(j).Density() = Density_;
 
       RotorDisk(j).Vinf(0) = FreeStreamVelocity_[0];
       RotorDisk(j).Vinf(1) = FreeStreamVelocity_[1];
       RotorDisk(j).Vinf(2) = FreeStreamVelocity_[2];
     
    }
        
    for ( j = 1 ; j <= NumberOfRotors_ ; j++ ) {
     
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

          xyz[0] = VortexLoop(i).Xc();            
          xyz[1] = VortexLoop(i).Yc();           
          xyz[2] = VortexLoop(i).Zc();
       
          RotorDisk(j).Velocity(xyz, q);                   

          VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(0) += q[0];
          VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(1) += q[1];
          VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(2) += q[2];
          VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(3) += q[3];
          VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(4) += q[4];
                         
          // If there is a ground effects, z - plane
          
          if ( DoGroundEffectsAnalysis() ) {

             xyz[0] = VortexLoop(i).Xc();            
             xyz[1] = VortexLoop(i).Yc();           
             xyz[2] = VortexLoop(i).Zc();
          
             xyz[2] *= -1.;
            
             RotorDisk(j).Velocity(xyz, q);      
   
             q[2] *= -1.;
             
             VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(0) += q[0];
             VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(1) += q[1];
             VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(2) += q[2];
             VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(3) += q[3];
             VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(4) += q[4];      
                    
           }      
                    
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {

             xyz[0] = VortexLoop(i).Xc();            
             xyz[1] = VortexLoop(i).Yc();           
             xyz[2] = VortexLoop(i).Zc();
             
             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
            
             RotorDisk(j).Velocity(xyz, q);      
   
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
             
             VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(0) += q[0];
             VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(1) += q[1];
             VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(2) += q[2];
             VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(3) += q[3];
             VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(4) += q[4];                 
             
             // If there is a ground effects, z - plane
             
             if ( DoGroundEffectsAnalysis() ) {
  
                xyz[2] *= -1.;
               
                RotorDisk(j).Velocity(xyz, q);      
      
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;      
                                                      q[2] *= -1.;

                VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(0) += q[0];
                VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(1) += q[1];
                VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(2) += q[2];
                VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(3) += q[3];
                VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(4) += q[4];                    
                  
             }    
               
          }             
          
       }    
       
    }

    // Add in surface velocity terms
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(0) -= LocalBodySurfaceVelocity_[i][0];
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(1) -= LocalBodySurfaceVelocity_[i][1];
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(2) -= LocalBodySurfaceVelocity_[i][2];
     
    }       
    
    // Update velocity magnitude
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
 
       // Calculate local free stream conditions

       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(4) = sqrt( SQR(VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(0))
                                                                      + SQR(VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(1))
                                                                      + SQR(VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(2)) );
        
    }   
    
    // Restrict the local free stream velocities onto the coarser grids
    
    RestrictFreeStreamVelocity();
         
}
 
/*##############################################################################
#                                                                              #
#                   VSP_SOLVER RestrictFreeStreamVelocity                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::RestrictFreeStreamVelocity(void) 
{
 
    int Level, i_c, i_f;
    double Fact;

    for ( Level = 1 ; Level < NumberOfMGLevels_ ; Level++ ) {

       for ( i_f = 1 ; i_f <= VSPGeom().Grid(Level).NumberOfLoops() ; i_f++ ) {
  
          i_c = VSPGeom().Grid(Level).LoopList(i_f).CoarseGridLoop();
          
          Fact = VSPGeom().Grid(Level).LoopList(i_f).Area() / VSPGeom().Grid(Level).LoopList(i_c).Area();
          
          VSPGeom().Grid(Level+1).LoopList(i_c).LocalFreeStreamVelocity(0) += Fact*VSPGeom().Grid(Level).LoopList(i_f).LocalFreeStreamVelocity(0);
          VSPGeom().Grid(Level+1).LoopList(i_c).LocalFreeStreamVelocity(1) += Fact*VSPGeom().Grid(Level).LoopList(i_f).LocalFreeStreamVelocity(1);
          VSPGeom().Grid(Level+1).LoopList(i_c).LocalFreeStreamVelocity(2) += Fact*VSPGeom().Grid(Level).LoopList(i_f).LocalFreeStreamVelocity(2);

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
 
    int i, j, k, c, jj, kk, Node, NumEdges, Node1, Node2, Loop, *ComponentGroup;
    int NumberOfSheets, NumberOfKuttaNodes, Hits, MaxNumberOfCommonTEs;
    double FarDist, *Sigma, *VecX, *VecY, *VecZ, VecTe[3], Dot, Dist;
    double Scale_X, Scale_Y, Scale_Z, WakeDist, xyz_te[3];
    VSP_NODE VSP_Node1, VSP_Node2;
    
    // Initial wake in the free stream direction

    if ( Vinf_ > 0. ) {
       
       WakeAngle_[0] = FreeStreamVelocity_[0] / Vinf_;
       WakeAngle_[1] = FreeStreamVelocity_[1] / Vinf_;
       WakeAngle_[2] = FreeStreamVelocity_[2] / Vinf_;
       
    }
    
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
    
    FarDist = MAX3(Scale_X*(Xmax_ - Xmin_), Scale_Y*(Ymax_-Ymin_), Scale_Z*(Zmax_-Zmin_));
    
    printf("Xmax_ - Xmin_: %f \n",Xmax_ - Xmin_);
    printf("Ymax_ - Ymin_: %f \n",Ymax_ - Ymin_);
    printf("Zmax_ - Zmin_: %f \n",Zmax_ - Zmin_);
    printf("\n");
    
    // Override far field distance
    
    if ( SetFarFieldDist_ ) {
       
       FarDist = FarFieldDist_;
       
    }
    
    else {
       
       FarFieldDist_ = FarDist;
       
    }
    
    printf("Wake FarDist set to: %f \n",FarDist);
    printf("\n");
       
    // Set intial wake start time
    
    WakeStartingTime_ = 0;
    
    if ( TimeAccurate_ && StartFromSteadyState_ ) WakeStartingTime_ = NumberOfWakeTrailingNodes_;

    // Determine the minimum trailing edge spacing for edge kutta node

    Sigma = new double[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    
    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfNodes() ; j++ ) {

       Sigma[j] = 1.e9;

    }
  
    Hits = 0;
    
    SigmaAvg_ = 0.;
    
    for ( i = 1 ; i <= VSPGeom().Grid(MGLevel_).NumberOfEdges() ; i++ ) {
     
       if ( VSPGeom().Grid(MGLevel_).EdgeList(i).IsTrailingEdge() ) {     

          Node1 = VSPGeom().Grid(MGLevel_).EdgeList(i).Node1();
          Node2 = VSPGeom().Grid(MGLevel_).EdgeList(i).Node2();
          
          Sigma[Node1] = MIN(Sigma[Node1], VSPGeom().Grid(MGLevel_).EdgeList(i).Length());
          Sigma[Node2] = MIN(Sigma[Node2], VSPGeom().Grid(MGLevel_).EdgeList(i).Length());
          
          SigmaAvg_ += VSPGeom().Grid(MGLevel_).EdgeList(i).Length();
          
          Hits++;
        
       }
       
    } 
    
    // Average trailing edge spacing
    
    SigmaAvg_ /= Hits;

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
          
          VecTe[0] = VecTe[1] = VecTe[2] = 0.;
          
          for ( j = 1 ; j <= 2 ; j++ ) {
             
             if ( j == 1 ) Loop = VSPGeom().Grid(MGLevel_).EdgeList(i).Loop1();
             
             if ( j == 2 ) Loop = VSPGeom().Grid(MGLevel_).EdgeList(i).Loop2();
             
             if ( Loop != 0 ) {
             
                // Edge vector crossed into normal... gives TE direction
                
                vector_cross(VSPGeom().Grid(MGLevel_).EdgeList(i).Vec(), VSPGeom().Grid(MGLevel_).LoopList(Loop).Normal(), VecTe);
 
                Node1 = VSPGeom().Grid(MGLevel_).EdgeList(i).Node1();
                Node2 = VSPGeom().Grid(MGLevel_).EdgeList(i).Node2();
           
                if ( Loop == VSPGeom().Grid(MGLevel_).EdgeList(i).LoopL() ) {
                 
                   VecX[Node1] += VecTe[0];
                   VecY[Node1] += VecTe[1];
                   VecZ[Node1] += VecTe[2];
                   
                   VecX[Node2] += VecTe[0];
                   VecY[Node2] += VecTe[1];
                   VecZ[Node2] += VecTe[2];
                   
                }
                
                else {
                 
                   VecX[Node1] -= VecTe[0];
                   VecY[Node1] -= VecTe[1];
                   VecZ[Node1] -= VecTe[2];
                               
                   VecX[Node2] -= VecTe[0];
                   VecY[Node2] -= VecTe[1];
                   VecZ[Node2] -= VecTe[2];
                   
                }    
                
             }       
             
          }

       }
       
    }    
    
    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {

       Node = VSPGeom().Grid(MGLevel_).KuttaNode(j);
       
       VecTe[0] = VecX[Node];
       VecTe[1] = VecY[Node];
       VecTe[2] = VecZ[Node];
       
       Dot = sqrt(vector_dot(VecTe,VecTe));
       
       if ( Dot > 0. ) {
          
          VecX[Node] /= Dot;
          VecY[Node] /= Dot;
          VecZ[Node] /= Dot;
          
       }
          
    }          
              
    // Determine the number of vortex sheets
    
    NumberOfSheets = 0;
    
    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {

       NumberOfSheets = MAX(NumberOfSheets, VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j));
       
    }    
    
    NumberOfVortexSheets_ = 0;
    
    for ( k = 1 ; k <= NumberOfSheets ; k++ ) {
       
       NumberOfKuttaNodes = 0;
       
       for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {
          
          if ( VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j) == k ) NumberOfKuttaNodes++;
          
       }
       
       if ( NumberOfKuttaNodes > 1 ) NumberOfVortexSheets_++;
       
    }
    
    printf("There are: %10d Vortex Sheets \n", NumberOfVortexSheets_);
    
    if ( VortexSheet_ != NULL ) {

       for ( i = 0 ; i < NumberOfThreads_ ; i++ ) {
          
          delete [] VortexSheet_[i];
          
       }
       
       delete [] VortexSheet_;

    }

    VortexSheet_ = new VORTEX_SHEET*[NumberOfThreads_];
    
    for ( i = 0 ; i < NumberOfThreads_ ; i++ ) {
       
       VortexSheet_[i] = new VORTEX_SHEET[NumberOfVortexSheets_ + 1];

    }   
    
    printf("Creating vortex sheet data... \n"); fflush(NULL);

    // Create copy of vortex sheet data for each CPU
    
    for ( c = 0 ; c < NumberOfThreads_ ; c++ ) {
       
       i = 0;
       
       for ( k = 1 ; k <= NumberOfSheets ; k++ ) {
          
          NumberOfKuttaNodes = 0;
          
          for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {
             
             if ( VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j) == k ) NumberOfKuttaNodes++;
             
          }
          
          if ( NumberOfKuttaNodes > 1 ){
             
             i++;
             
             VortexSheet(c,i).SizeTrailingVortexList(NumberOfKuttaNodes);
             
             VortexSheet(c,i).WingSurface() = k;
             
          }
          
          else {
             
             printf("Warning ... zero kutta nodes for sheet: %d \n",k);
             fflush(NULL);
             
          }
          
       }

       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

          NumEdges = 0;
          
          VortexSheet(c,k).TimeAccurate() = TimeAccurate_;
          
          VortexSheet(c,k).TimeAnalysisType() = TimeAnalysisType_;
//djk          
          VortexSheet(c,k).Vinf() = SGN(Vinf_)*MAX(0.000001,ABS(Vinf_));
       
          VortexSheet(c,k).TimeStep() = TimeStep_;       
          
          VortexSheet(c,k).FarAwayRatio() = FarAway_;
          
          if ( Vinf_ > 0. ) {

             VortexSheet(c,k).FreeStreamVelocity(0) = FreeStreamVelocity_[0]/Vinf_;
             VortexSheet(c,k).FreeStreamVelocity(1) = FreeStreamVelocity_[1]/Vinf_;
             VortexSheet(c,k).FreeStreamVelocity(2) = FreeStreamVelocity_[2]/Vinf_;
         
          }
          
          else {
             
             VortexSheet(c,k).FreeStreamVelocity(0) = 0.;
             VortexSheet(c,k).FreeStreamVelocity(1) = 0.;
             VortexSheet(c,k).FreeStreamVelocity(2) = 0.; 
             
          }          
   
          for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {
             
             if ( VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j) == VortexSheet(c,k).WingSurface() ) {
             
                NumEdges++;
                
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).TimeAccurate() = TimeAccurate_;
                
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).TimeAnalysisType() = TimeAnalysisType_;
   
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).RotorAnalysis() = 0;
                                
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).FarAwayRatio() = FarAway_;
  
                if ( RotorAnalysis_ ) VortexSheet(c,k).TrailingVortexEdge(NumEdges).RotorAnalysis() = 1;
                   
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).Vinf() = MAX(0.000001,Vinf_);;
   
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).BladeRPM() = BladeRPM_;
                
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).TimeStep() = TimeStep_;                
                             
                // Pointer to the wing this trailing vortex leaves from
         
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).Wing() = k;
                
                // Flag if the vortex sheet is periodic (eg would be a nacelle)
                
                VortexSheet(c,k).IsPeriodic() = VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNodeIsPeriodic(j);
   
                // Pointer to the kutta node
                
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).Node() = VSPGeom().Grid(MGLevel_).KuttaNode(j);
                
                // Location along span of this kutta node S over Span
                
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).SoverB() = VSPGeom().Grid(MGLevel_).KuttaNodeSoverB(j);
                
                // Component ID
               
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).ComponentID() = VSPGeom().Grid(MGLevel_).ComponentIDForKuttaNode(j);
                        
                // Pass in edge data and create edge cofficients
                
                VSP_Node1.x() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeX(j);
                VSP_Node1.y() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeY(j);
                VSP_Node1.z() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeZ(j);
   
                VSP_Node2.x() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeX(j) + WakeAngle_[0] * 1.e6;
                VSP_Node2.y() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeY(j) + WakeAngle_[1] * 1.e6;
                VSP_Node2.z() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeZ(j) + WakeAngle_[2] * 1.e6;
          
                // Set sigma
   
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).Sigma() = 0.5*Sigma[VSPGeom().Grid(MGLevel_).KuttaNode(j)];

                // Set trailing edge direction 
                
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).TEVec(0) = VecX[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).TEVec(1) = VecY[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).TEVec(2) = VecZ[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
               
                // Create trailing wakes... specify number of sub vortices per trail
      
                WakeDist = MAX(VSP_Node1.x() + 0.5*FarDist, Xmax_ + 0.25*FarDist) - VSP_Node1.x();
     
                VortexSheet(c,k).TrailingVortexEdge(NumEdges).Setup(NumberOfWakeTrailingNodes_,WakeDist,VSP_Node1,VSP_Node2);

             }
                
          }
      
          VortexSheet(c,k).SetupVortexSheets();
          
          if ( c == 0 ) {

             if ( VortexSheet(c,k).IsPeriodic() ) {
                
                printf("There are: %10d kutta nodes for vortex sheet: %10d     <----- Periodic Wake \n",VortexSheet(c,k).NumberOfTrailingVortices(),k); fflush(NULL);
                
             }
             
             else {
                
                printf("There are: %10d kutta nodes for vortex sheet: %10d  \n",VortexSheet(c,k).NumberOfTrailingVortices(),k); fflush(NULL);
                
             }
             
          }
   
       }
       
    }
    
    // For VLM mode loop over trailing edges and see if any overlap from wing to wing
        
    if ( ModelType_ == VLM_MODEL ) {
       
       ComponentGroup = new int[VSPGeom().NumberOfComponents() + 1];
       
       zero_int_array(ComponentGroup, VSPGeom().NumberOfComponents());
       
       if ( TimeAccurate_ ) {

          for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

             for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
         
                ComponentGroup[ComponentGroupList_[c].ComponentList(j)] = c;
                
             }
             
          }
          
       }
      
       int MaxNumberOfCommonTEs = 1000;
       
       int NumberOfCommonTEs = 0;
       
       COMMON_VORTEX_SHEET *CommonTEList = new COMMON_VORTEX_SHEET[MaxNumberOfCommonTEs + 1];
              
       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
          
          for ( j = 1 ; j <= VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
             
             xyz_te[0] = VortexSheet(k).TrailingVortexEdge(j).TE_Node().x();
             xyz_te[1] = VortexSheet(k).TrailingVortexEdge(j).TE_Node().y();
             xyz_te[2] = VortexSheet(k).TrailingVortexEdge(j).TE_Node().z();
             
             for ( kk = k + 1 ; kk <= NumberOfVortexSheets_ ; kk++ ) {

                if ( inside_box( VortexSheet(kk).TEBox(), xyz_te ) ) {
         
                   for ( jj = 1 ; jj <= VortexSheet(kk).NumberOfTrailingVortices() ; jj++ ) {
                      
                      Dist = sqrt( SQR(xyz_te[0] - VortexSheet(kk).TrailingVortexEdge(jj).TE_Node().x())
                                 + SQR(xyz_te[1] - VortexSheet(kk).TrailingVortexEdge(jj).TE_Node().y())
                                 + SQR(xyz_te[2] - VortexSheet(kk).TrailingVortexEdge(jj).TE_Node().z()) );
                                 
                      if ( Dist <= 0.1*VortexSheet( k).TrailingVortexEdge(j ).Sigma() ||
                           Dist <= 0.1*VortexSheet(kk).TrailingVortexEdge(jj).Sigma() ) {
                              
                         if ( !TimeAccurate_ || ComponentGroup[VortexSheet(k).TrailingVortexEdge(j).ComponentID()] == ComponentGroup[VortexSheet(kk).TrailingVortexEdge(jj).ComponentID()] ) {
                              
                            printf("Found a TE node that is common between vortex sheets: %d and %d \n",k,kk);
                            
                            NumberOfCommonTEs++;
                            
                            if ( NumberOfCommonTEs > MaxNumberOfCommonTEs ) {
                               
                               COMMON_VORTEX_SHEET *Temp;
                               
                               MaxNumberOfCommonTEs *= 1.5;
                               
                               Temp =  new COMMON_VORTEX_SHEET[MaxNumberOfCommonTEs + 1];
                            
                               for ( i = 1 ; i <= NumberOfCommonTEs - 1 ; i++ ) {
   
                                  Temp[i].Sheet_i()    = CommonTEList[i].Sheet_i();
                                  Temp[i].Sheet_j()    = CommonTEList[i].Sheet_j();
                                                              
                                  Temp[i].TEVortex_i() = CommonTEList[i].TEVortex_i();
                                  Temp[i].TEVortex_j() = CommonTEList[i].TEVortex_j();     
                                  
                               }
                               
                               delete [] CommonTEList;
                               
                               CommonTEList= Temp;
                               
                            }                    
   
                            CommonTEList[NumberOfCommonTEs].Sheet_i() = k;
                            CommonTEList[NumberOfCommonTEs].Sheet_j() = kk;
                            
                            CommonTEList[NumberOfCommonTEs].TEVortex_i() = j;
                            CommonTEList[NumberOfCommonTEs].TEVortex_j() = jj;        
                            
                         }                 
                         
                      }
                      
                   }
                   
                }

             }
             
          }
          
       }
       
       delete [] ComponentGroup;
       
       // Pack the data
       
       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
          
          j = 0;

          for ( i = 1 ; i <= NumberOfCommonTEs ; i++ ) {
             
             if ( CommonTEList[i].Sheet_i() == k ) {
   
                j++;
                
             }
             
          }
          
          if ( j > 0 ) {
             
             VortexSheet(k).SizeCommonTEList(j);
             
             j = 0;
             
             for ( i = 1 ; i <= NumberOfCommonTEs ; i++ ) {
                
                if ( CommonTEList[i].Sheet_i() == k ) {
                   
                   j++;
                   
                   VortexSheet(k).CommonTEList(j).Sheet_i() = CommonTEList[i].Sheet_i();
                   VortexSheet(k).CommonTEList(j).Sheet_j() = CommonTEList[i].Sheet_j();
                               
                   VortexSheet(k).CommonTEList(j).TEVortex_i() = CommonTEList[i].TEVortex_i();
                   VortexSheet(k).CommonTEList(j).TEVortex_j() = CommonTEList[i].TEVortex_j();   
                   
                   VortexSheet(k).NumberOfCommonNodesForTE(CommonTEList[i].TEVortex_i())++;
                                      
                }
                
             }   
             
          }
          
       }          
       
       delete [] CommonTEList;
              
    }

    printf("Done creating vortex sheet data... \n");fflush(NULL);
    
    
/* Test code... for tip vortex stuff
 * 
     int , Next, *NextNode, NumberOfTeNodes, *NodeIsUsed; 

    // Create a node to edge list
   
    int *NumberOfEdgesForNode_, **NodeToEdgeList_;
    
    NumberOfEdgesForNode_ = new int[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    
    zero_int_array(NumberOfEdgesForNode_, VSPGeom().Grid(MGLevel_).NumberOfNodes());
    
    for ( i = 1 ; i <= VSPGeom().Grid(MGLevel_).NumberOfEdges() ; i++ ) {
 
       NumberOfEdgesForNode_[VSPGeom().Grid(MGLevel_).EdgeList(i).Node1()]++;
       NumberOfEdgesForNode_[VSPGeom().Grid(MGLevel_).EdgeList(i).Node2()]++;
 
    }
   
    NodeToEdgeList_ = new int*[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    
    for ( i = 1 ; i <= VSPGeom().Grid(MGLevel_).NumberOfNodes() ; i++ ) {
       
       NodeToEdgeList_[i] = new int[NumberOfEdgesForNode_[i] + 1];
       
    }

    zero_int_array(NumberOfEdgesForNode_, VSPGeom().Grid(MGLevel_).NumberOfNodes());
    
    for ( i = 1 ; i <= VSPGeom().Grid(MGLevel_).NumberOfEdges() ; i++ ) {

       Node = VSPGeom().Grid(MGLevel_).EdgeList(i).Node1();
       
       NumberOfEdgesForNode_[Node]++;
       
       NodeToEdgeList_[Node][NumberOfEdgesForNode_[Node]] = i;
       
       Node = VSPGeom().Grid(MGLevel_).EdgeList(i).Node2();
       
       NumberOfEdgesForNode_[Node]++;
       
       NodeToEdgeList_[Node][NumberOfEdgesForNode_[Node]] = i;          

    } 
        
    // Loop over edges attached to the root and tip of the vortex sheets and 
    // apply the vortex core model to them
    
    NextNode = new int[NumberOfSurfaceNodes_ + 1];
    
    NodeIsUsed = new int[NumberOfSurfaceNodes_ + 1];

          for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {

             if ( VSPGeom().Grid(MGLevel_).KuttaNodeIsOnWingTip(j) ) {
                
                zero_int_array(NodeIsUsed, NumberOfSurfaceNodes_);
                
                Node = VSPGeom().Grid(MGLevel_).KuttaNode(j);
                
                NodeIsUsed[Node] = 1;
                
                VecTe[0] = VecX[Node];
                VecTe[1] = VecY[Node];
                VecTe[2] = VecZ[Node];
       
                NumberOfTeNodes = 1;
                
                NextNode[NumberOfTeNodes] = Node;
                
                Next = 1;
                
                while ( Next <= NumberOfTeNodes ) {
                   
                   n = NextNode[Next];
                   
                   NodeIsUsed[n] = 1;
                   
                   for ( k = 1 ; k <= NumberOfEdgesForNode_[n] ; k++ ) {
                  
                      p = NodeToEdgeList_[n][k];
            
                      if ( ABS(vector_dot(VecTe,SurfaceVortexEdge(p).Vec())) > 0.90 ) {

printf("Adding edge at: %f %f %f with MinCoreSize: %f \n",SurfaceVortexEdge(p).Xc(),SurfaceVortexEdge(p).Yc(),SurfaceVortexEdge(p).Zc(),MinCoreSize[Node]);

                         SurfaceVortexEdge(p).MinCoreWidth() = 0.1*MinCoreSize[Node];

                         SurfaceVortexEdge(p).MinCoreWidth() = 0.00;
                    
                         if ( NodeIsUsed[SurfaceVortexEdge(p).Node1()] == 0 ) {
                            
                            Distance = sqrt( pow(VSPGeom().Grid(MGLevel_).NodeList(Node).x() - VSPGeom().Grid(MGLevel_).NodeList(SurfaceVortexEdge(p).Node1()).x(),2.)
                                           + pow(VSPGeom().Grid(MGLevel_).NodeList(Node).y() - VSPGeom().Grid(MGLevel_).NodeList(SurfaceVortexEdge(p).Node1()).y(),2.)
                                           + pow(VSPGeom().Grid(MGLevel_).NodeList(Node).z() - VSPGeom().Grid(MGLevel_).NodeList(SurfaceVortexEdge(p).Node1()).z(),2.) );
                            
                            if ( MinCoreSize[Node] > 0. && Distance/MinCoreSize[Node] < 5. ) {
                               
                         //      NextNode[++NumberOfTeNodes] = SurfaceVortexEdge(p).Node1();
                                 
                         //      NodeIsUsed[SurfaceVortexEdge(p).Node1()] = 1;
                               
                            }
                            
                         }                         
      
                         if ( NodeIsUsed[SurfaceVortexEdge(p).Node2()] == 0 ) {
                            
                            Distance = sqrt( pow(VSPGeom().Grid(MGLevel_).NodeList(Node).x() - VSPGeom().Grid(MGLevel_).NodeList(SurfaceVortexEdge(p).Node2()).x(),2.)
                                           + pow(VSPGeom().Grid(MGLevel_).NodeList(Node).y() - VSPGeom().Grid(MGLevel_).NodeList(SurfaceVortexEdge(p).Node2()).y(),2.)
                                           + pow(VSPGeom().Grid(MGLevel_).NodeList(Node).z() - VSPGeom().Grid(MGLevel_).NodeList(SurfaceVortexEdge(p).Node2()).z(),2.) );
                            
                            if ( MinCoreSize[Node] > 0. && Distance/MinCoreSize[Node] < 5. ) {
                               
                        //       NextNode[++NumberOfTeNodes] = SurfaceVortexEdge(p).Node2();
                             
                        //       NodeIsUsed[SurfaceVortexEdge(p).Node2()] = 1;
                               
                            }
                            
                         }    
                                            
                      }
                      
                   }
                   
                   Next++;
                    
                }      
                
             }
             
          }

    delete [] NodeIsUsed;
    * 
   for ( i = 1 ; i <= VSPGeom().Grid(MGLevel_).NumberOfNodes() ; i++ ) {
       
       if ( NumberOfEdgesForNode_[i] > 0 ) delete [] NodeToEdgeList_[i];
       
    }
    
    delete [] NodeToEdgeList_;    
    delete [] NumberOfEdgesForNode_;
        * 

 */   
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
 
    int c, i, k;
    char StatusFileName[2000], LoadFileName[2000], ADBFileName[2000];
    char GroupFileName[2000], RotorFileName[2000];
   
    // Zero out solution
   
    zero_double_array(Gamma_[0], NumberOfVortexLoops_); Gamma_[0][0] = 0.;   
    zero_double_array(Gamma_[1], NumberOfVortexLoops_); Gamma_[1][0] = 0.;
    zero_double_array(Gamma_[2], NumberOfVortexLoops_); Gamma_[2][0] = 0.;
    zero_double_array(Delta_,    NumberOfVortexLoops_);    Delta_[0] = 0.;
        
    CurrentTime_ = 0.;
       
    // Keep track of unsteady forces and moments
    
    if ( TimeAccurate_ ) {
       
       if (  CL_Unsteady_ != NULL ) delete []  CL_Unsteady_;    
       if (  CD_Unsteady_ != NULL ) delete []  CD_Unsteady_;      
       if (  CS_Unsteady_ != NULL ) delete []  CS_Unsteady_;       
       if ( CFx_Unsteady_ != NULL ) delete [] CFx_Unsteady_;      
       if ( CFy_Unsteady_ != NULL ) delete [] CFy_Unsteady_;       
       if ( CFz_Unsteady_ != NULL ) delete [] CFz_Unsteady_;       
       if ( CMx_Unsteady_ != NULL ) delete [] CMx_Unsteady_;       
       if ( CMy_Unsteady_ != NULL ) delete [] CMy_Unsteady_;       
       if ( CMz_Unsteady_ != NULL ) delete [] CMz_Unsteady_;       
             
        CL_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
        CD_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
        CS_Unsteady_ = new double[NumberOfTimeSteps_ + 1];    
       CFx_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CFy_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CFz_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMx_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMy_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMz_Unsteady_ = new double[NumberOfTimeSteps_ + 1];       
 
    }

    // Update geometry location and interaction lists for moving geoemtries

    CurrentTime_ = CurrentNoiseTime_ = 0.;
           
    if ( TimeAccurate_ && !StartFromSteadyState_ ) UpdateGeometryLocation(1);
             
    // Initialize free stream
    
    InitializeFreeStream();
    
    // Recalculate interaction lists if Mach crossed over Mach = 1
    
    if ( LastMach_ < 0. || ( Mach_ >= 1. && LastMach_ <  1. ) || ( Mach_ <  1. && LastMach_ >= 1 ) ) {
       
       printf("Updating interaction lists due to subsonic / supersonic Mach change \n");
       
       if ( !DumpGeom_ ) CreateSurfaceVorticesInteractionList(0);

    }
  
    LastMach_ = Mach_;

    // Initialize the wake trailing vortices

    InitializeTrailingVortices();
    
    ZeroVortexState();

    // Calculate the right hand side
    
    CalculateRightHandSide();
        
    // Do a restart
    
    if ( DoRestart_ == 1 ) {
  
       LoadRestartFile();
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

           VortexLoop(i).Gamma() = Gamma(i);
    
       }
        
    }
    
    else {
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

           VortexLoop(i).Gamma() = Gamma(i) = 0.;
    
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
       
       // Write out generic header
       
       WriteCaseHeader(StatusFile_);
       
       // Status update to user
       
       fprintf(StatusFile_,"\n\nSolver Case: %d \n\n",ABS(Case));
       
    }
    
    if ( !TimeAccurate_ ) {

                          //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789          
       fprintf(StatusFile_,"  Iter      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS \n");
   
    }
    
    else {
       
       if ( TimeAnalysisType_ == HEAVE_ANALYSIS ) {
                 
                             //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789     
          fprintf(StatusFile_,"  Time      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS      H       CL_Un     CDi_Un    CS_Un     CFx_Un     CFy_Un    CFz_Un   CMx_Un    CMy_Un    CMz_Un \n");

       }
       
       else if ( TimeAnalysisType_ == P_ANALYSIS || TimeAnalysisType_ == Q_ANALYSIS ||  TimeAnalysisType_ == R_ANALYSIS ) {
         
                             //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789          
          fprintf(StatusFile_,"  Time      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS  UnstdyAng   CL_Un     CDi_Un    CS_Un     CFx_Un     CFy_Un    CFz_Un   CMx_Un    CMy_Un    CMz_Un \n");

       }
       
       else {

                             //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
          fprintf(StatusFile_,"  Time      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS    CL_Un     CDi_Un    CS_Un     CFx_Un     CFy_Un    CFz_Un   CMx_Un    CMy_Un    CMz_Un \n");
      
       }
   
    }

    // Open the adb and case list files the first time only
    
    if ( Case == 0 || Case == 1 ) {

       sprintf(ADBFileName,"%s.adb",FileName_);
       
       if ( (ADBFile_ = fopen(ADBFileName, "wb")) == NULL ) {
   
          printf("Could not open the aerothermal data base file for binary output! \n");
   
          exit(1);
   
       }
       
       sprintf(ADBFileName,"%s.adb.cases",FileName_);
       
       if ( (ADBCaseListFile_ = fopen(ADBFileName, "w")) == NULL ) {
   
          printf("Could not open the aerothermal data base case list file for output! \n");
   
          exit(1);
   
       }       
       
    }    
    
    // Set up group and rotor files

    GroupFile_ = new FILE*[NumberOfComponentGroups_ + 1];
        
    k = 0;
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) k++;
       
    }
    
    k++;

    RotorFile_ = new FILE*[k + 1];
    
    k = 0;
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
    
       // Create group file
       
       sprintf(GroupFileName,"%s.group.%d",FileName_,c);
    
       if ( (GroupFile_[c] = fopen(GroupFileName, "w")) == NULL ) {
    
          printf("Could not open the %s group coefficient file! \n",GroupFileName);
    
          exit(1);
    
       }

                            //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890  
       fprintf(GroupFile_[c],"Force Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
       fprintf(GroupFile_[c],"   Time        Cx         Cy         Cz        Cxo        Cyo        Czo        Cxi        Cyi        Czi        Cmx        Cmy        Cmz        Cmxo       Cmyo       Cmzo       Cmxi       Cmyi       Cmzi        CL         CD        CLo        CDo        CLi        CDi \n");
                          
       // Create rotor file
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          k++;
          
          sprintf(RotorFileName,"%s.rotor.%d",FileName_,k);
    
          if ( (RotorFile_[k] = fopen(RotorFileName, "w")) == NULL ) {
      
             printf("Could not open the %s rotor coefficient file! \n",RotorFileName);
      
             exit(1);
      
          }

 
                               //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890
          fprintf(RotorFile_[k],"                                                                                                       ------------ Propeller Coefficients _----------       -------- Rotor Coefficients _-------\n\n");                        
          fprintf(RotorFile_[k]," Time       Diameter     RPM       Thrust    Thrusto    Thrusti     Moment     Momento    Momenti      J          CT         CQ         CP        EtaP       CT_H       CQ_H       CP_H       FOM        Angle \n");
 
       }
   
    }     

    // Write out ADB Geometry
    
    if ( Case == 0 || Case == 1 ) {

       WriteOutAerothermalDatabaseHeader();

       WriteOutAerothermalDatabaseGeometry();

    }

    printf("Solving... \n\n");fflush(NULL);

    if ( DumpGeom_ ) WakeIterations_ = 0;
    
    if ( TimeAccurate_ && !StartFromSteadyState_ ) WakeIterations_ = 1;
  
    // Solve at the each time step... or single solve if just a steady state solution

    if ( !TimeAccurate_ ) NumberOfTimeSteps_ = 1;
    
    // If time accurate, write out current t=0 solution state
    
    if ( TimeAccurate_ ) {
       
        sprintf(CaseString_,"Time: %-f ...",0.);
       
        WriteOutAerothermalDatabaseSolution();
    
    }
    
    for ( Time_ = 1 ; Time_ <= NumberOfTimeSteps_ ; Time_++ ) {

       CurrentTime_ = Time_*TimeStep_;
       
       StartSolveTime_ = myclock();
       
       if ( !TimeAccurate_ ) CurrentTime_ = 0.;
       
       // Inner iteration on wake shape

       for ( CurrentWakeIteration_ = 1 ; CurrentWakeIteration_ <= WakeIterations_ ; CurrentWakeIteration_++ ) {

          if ( TimeAccurate_ ) UpdateWakeConvectedDistance();

          UpdateWakeVortexInteractionLists();

          if ( TimeAccurate_ ) {
   
             // Update geometry location and interaction lists for moving geoemtries

             if ( !StartFromSteadyState_ || ( StartFromSteadyState_ && Time_ > 1 ) ) UpdateGeometryLocation(0);

             if ( !AllComponentsAreFixed_ && ThereIsRelativeComponentMotion_ ) CreateSurfaceVorticesInteractionList(1);

             // Update free stream for unsteady cases
                                        
             InitializeFreeStream();
             
             // Update right hand side
             
             CalculateRightHandSide();

             // Calculate the unsteady wake velocities
   
             if ( !StartFromSteadyState_ || ( StartFromSteadyState_ && Time_ > 1 ) ) CalculateUnsteadyWakeVelocities();
                   
          }          

          // Solve the linear system

          SolveLinearSystem();
  
          // If time accurate we save the trailing vorticity state
     
          if ( TimeAccurate_ ) SaveVortexState();

          // Update wake locations

          if ( ( WakeIterations_ > 1                                  ) || 
               ( TimeAccurate_ && StartFromSteadyState_ && Time_ == 1 ) ) UpdateWakeLocations();

          // Calculate forces

          CalculateForces();
     
          // Output status

          OutputStatusFile();
          
          // Write out group data, and any rotor data
  
          if ( !TimeAccurate_ ) CalculateRotorCoefficientsForGroup(0);   
        
          printf("\n");
          
       }

       if ( TimeAccurate_ && StartFromSteadyState_ ) WakeIterations_ = 1;

       // Write out ADB Solution for time accurate cases
       
       if ( TimeAccurate_ ) {
          
          if ( TimeAnalysisType_ == P_ANALYSIS ) {
             
             sprintf(CaseString_,"T: %-f, P: %-f",CurrentTime_, Unsteady_Angle_/TORAD);
             
          }
             
          else if ( TimeAnalysisType_ == Q_ANALYSIS ) {
             
             sprintf(CaseString_,"T: %-f, Q: %-f",CurrentTime_, Unsteady_Angle_/TORAD);

             
          }
          
          else if ( TimeAnalysisType_ == R_ANALYSIS ) {
             
             sprintf(CaseString_,"T: %-f, R: %-f",CurrentTime_, Unsteady_Angle_/TORAD);
             
          }
                
          else {
          
             sprintf(CaseString_,"Time: %-f ...",CurrentTime_);
             
          }
          
          WriteOutAerothermalDatabaseGeometry();

          InterpolateSolutionFromGrid(1);
          
          WriteOutAerothermalDatabaseSolution();

          // Write out group data, and any rotor data
  
          CalculateRotorCoefficientsForGroup(0);    
          
       }
   
    }

    OutputZeroLiftDragToStatusFile();

    // Open the load file the first time only
    
    if ( Case == 0 || Case == 1 ) {
    
       sprintf(LoadFileName,"%s.lod",FileName_);
       
       if ( (LoadFile_ = fopen(LoadFileName, "w")) == NULL ) {
   
          printf("Could not open the spanwise loading file for output! \n");
   
          exit(1);
   
       }
       
    }       
    
    // Calculate spanwise load distributions for lifting surfaces
 
    CalculateSpanWiseLoading();
    
    // Write out FEM loading file
 
    CreateFEMLoadFile(Case);

    // Interpolate solution from grid 1 to 0
 
    InterpolateSolutionFromGrid(1);

    // Output and survey point results
    
    if ( NumberofSurveyPoints_ > 0 ) CalculateVelocitySurvey();
 
    // Write out ADB Solution

    if ( !TimeAccurate_ ) WriteOutAerothermalDatabaseSolution();
    
    // Write out 2d FEM geometry and solution if requested
    
    if ( Write2DFEMFile_ )  {
       
       if ( Case == 0 || Case == 1 ) WriteFEM2DGeometry();
       
       WriteFEM2DSolution();
       
    }          

    // Close up files
    
    if ( Case <= 0                    ) fclose(StatusFile_);
    if ( Case <= 0                    ) fclose(LoadFile_);
    if ( Case <= 0                    ) fclose(ADBFile_);
    if ( Case <= 0                    ) fclose(ADBCaseListFile_);
    if ( Case <= 0                    ) fclose(FEMLoadFile_);
    if ( Case <= 0 && Write2DFEMFile_ ) fclose(FEM2DLoadFile_);

    // Close any rotor coefficient files

    k = 0;
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       fclose(GroupFile_[c]);
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) fclose(RotorFile_[++k]);
          
    }

    if ( RotorFile_ != NULL ) delete [] RotorFile_;
 
}

/*##############################################################################
#                                                                              #
#                   VSP_SOLVER WriteOutNoiseFiles                              #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutNoiseFiles(int Case)
{
 
    if ( SteadyStateNoise_ ) {
       
       WriteOutSteadyStateNoiseFiles(Case);
       
    }
    
    else {
       
       WriteOutTimeAccurateNoiseFiles(Case);
       
       
    }

}

/*##############################################################################
#                                                                              #
#                   VSP_SOLVER WriteOutSteadyStateNoiseFiles                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutSteadyStateNoiseFiles(int Case)
{
 
    int c, i, k;
    char StatusFileName[2000], ADBFileName[2000];
    char GroupFileName[2000], RotorFileName[2000];
   
    // Zero out solution
   
    zero_double_array(Gamma_[0], NumberOfVortexLoops_); Gamma_[0][0] = 0.;   
    zero_double_array(Gamma_[1], NumberOfVortexLoops_); Gamma_[1][0] = 0.;
    zero_double_array(Gamma_[2], NumberOfVortexLoops_); Gamma_[2][0] = 0.;
    zero_double_array(Delta_,    NumberOfVortexLoops_);    Delta_[0] = 0.;
        
    CurrentTime_ = 0.;
       
    // Keep track of unsteady forces and moments
    
    if ( TimeAccurate_ ) {
       
       if (  CL_Unsteady_ != NULL ) delete []  CL_Unsteady_;    
       if (  CD_Unsteady_ != NULL ) delete []  CD_Unsteady_;      
       if (  CS_Unsteady_ != NULL ) delete []  CS_Unsteady_;       
       if ( CFx_Unsteady_ != NULL ) delete [] CFx_Unsteady_;      
       if ( CFy_Unsteady_ != NULL ) delete [] CFy_Unsteady_;       
       if ( CFz_Unsteady_ != NULL ) delete [] CFz_Unsteady_;       
       if ( CMx_Unsteady_ != NULL ) delete [] CMx_Unsteady_;       
       if ( CMy_Unsteady_ != NULL ) delete [] CMy_Unsteady_;       
       if ( CMz_Unsteady_ != NULL ) delete [] CMz_Unsteady_;       
             
        CL_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
        CD_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
        CS_Unsteady_ = new double[NumberOfTimeSteps_ + 1];    
       CFx_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CFy_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CFz_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMx_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMy_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMz_Unsteady_ = new double[NumberOfTimeSteps_ + 1];       
 
    }

    // Update geometry location... really just the surface velocities
             
    CurrentTime_ = CurrentNoiseTime_ = 0.;
             
    if ( TimeAccurate_ && !StartFromSteadyState_ ) UpdateGeometryLocation(1);
                 
    // Initialize free stream
    
    InitializeFreeStream();

    // Create interaction list for fixed components

    CreateSurfaceVorticesInteractionList(0);

    // Initialize the wake trailing vortices

    InitializeTrailingVortices();
    
    ZeroVortexState();

    // Calculate the right hand side
    
    CalculateRightHandSide();

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

        VortexLoop(i).Gamma() = Gamma(i) = 0.;
 
    }

    // Open status file

    sprintf(StatusFileName,"%s.noise.history",FileName_);
    
    if ( (StatusFile_ = fopen(StatusFileName, "w")) == NULL ) {
   
       printf("Could not open the history file for output! \n");
   
       exit(1);
   
    }    

    // Write out generic header
    
    WriteCaseHeader(StatusFile_);

    // Status update to user
    
    fprintf(StatusFile_,"\n\nSolver Case: %d \n\n",ABS(Case));


                       //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
    fprintf(StatusFile_,"  Time      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS    CL_Un     CDi_Un    CS_Un     CFx_Un     CFy_Un    CFz_Un   CMx_Un    CMy_Un    CMz_Un \n");

    // If a rotor or unsteady path following case, open any required rotor files
    
    GroupFile_ = new FILE*[NumberOfComponentGroups_ + 1];
        
    k = 0;
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) k++;
       
    }
    
    k++;

    RotorFile_ = new FILE*[k + 1];
    
    k = 0;
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
  
       // Create group file
       
       sprintf(GroupFileName,"%s.noise.group.%d",FileName_,c);
 
       if ( (GroupFile_[c] = fopen(GroupFileName, "w")) == NULL ) {
   
          printf("Could not open the %s group coefficient file! \n",GroupFileName);
   
          exit(1);
   
       }

                            //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890  
       fprintf(GroupFile_[c],"Force Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
       fprintf(GroupFile_[c],"   Time        Cx         Cy         Cz        Cxo        Cyo        Czo        Cxi        Cyi        Czi        Cmx        Cmy        Cmz        Cmxo       Cmyo       Cmzo       Cmxi       Cmyi       Cmzi        CL         CD        CLo        CDo        CLi        CDi \n");
                          
       // Create rotor file
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          k++;
          
          sprintf(RotorFileName,"%s.noise.rotor.%d",FileName_,k);
    
          if ( (RotorFile_[k] = fopen(RotorFileName, "w")) == NULL ) {
      
             printf("Could not open the %s rotor coefficient file! \n",RotorFileName);
      
             exit(1);
      
          }

                               //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890  
          fprintf(RotorFile_[k],"Rotor Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
          fprintf(RotorFile_[k],"   Time     Diameter     RPM       Thrust     Thrusto    Thrusti    Moment     Momento    Momenti      J          CT         CQ        EtaP       Angle \n");
                             
       }

    }     
  
    // Open the input adb file
    
    if ( Case == 0 || Case == 1 ) {

       sprintf(ADBFileName,"%s.adb",FileName_);
       
       if ( (InputADBFile_ = fopen(ADBFileName, "rb")) == NULL ) {
   
          printf("Could not open the aerothermal data base file for binary input! \n");
   
          exit(1);
   
       }

       // Read in the header
       
       ReadInAerothermalDatabaseHeader();

    }
    
    // Open the output adb file
    
    if ( Case == 0 || Case == 1 ) {

       sprintf(ADBFileName,"%s.noise.adb",FileName_);
       
       if ( (ADBFile_ = fopen(ADBFileName, "wb")) == NULL ) {
   
          printf("Could not open the aerothermal data base file for binary output! \n");
   
          exit(1);
   
       }

       WriteOutAerothermalDatabaseHeader();

    }

    // Open the case file
    
    sprintf(ADBFileName,"%s.noise.adb.cases",FileName_);
    
    if ( (ADBCaseListFile_ = fopen(ADBFileName, "w")) == NULL ) {

       printf("Could not open the aerothermal data base case list file for output! \n");

       exit(1);

    }       

    // Setup PSU-WopWop data and write out file headers

    SetupPSUWopWopData();

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) WriteOutPSUWopWopFileHeadersForGroup(c);

    if ( TimeAccurate_ && !StartFromSteadyState_ ) WakeIterations_ = 1;
    
    // Step through the entire ADB file until we get to the end

    printf("Stepping through solution and writing out sound files... \n\n");fflush(NULL);

    WopWopComponentGroupStart_ = 1;
    
    WopWopComponentGroupEnd_ = NumberOfComponentGroups_;
    
    NoiseTimeStep_ = TimeStep_;
    
    for ( Time_ = 0 ;Time_ <= NumberOfTimeSteps_ ; Time_++ ) {
 
       CurrentTime_ = CurrentNoiseTime_ = Time_ * TimeStep_;
       
       // Read in the next ADB Solution
         
       ReadInAerothermalDatabaseGeometry();
       
       ReadInAerothermalDatabaseSolution(0);

       InterpolateExistingSolution(CurrentNoiseTime_);
                 
       UpdateVortexEdgeStrengths(1, ALL_WAKE_GAMMAS);
          
       // Initialize the free stream conditions
                  
       InitializeFreeStream();
    
       // Calculate forces

       CalculateForces();
   
       // Output status

       OutputStatusFile();

       // Write out any rotor coefficients
       
       CalculateRotorCoefficientsForGroup(0);
       
       // Write out geometry and current solution

       sprintf(CaseString_,"Time: %-f ...",CurrentNoiseTime_);

       WriteOutAerothermalDatabaseGeometry();

       InterpolateSolutionFromGrid(1);
       
       WriteOutAerothermalDatabaseSolution();
                     
       // Write out PSU WopWop Files for unsteady analysis

       if ( Time_ == NumberOfTimeSteps_ - 1 ) { for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) WriteOutPSUWopWopUnsteadyDataForGroup(c); };

       // Update geometry location and interaction lists for moving geoemtries
       
       UpdateGeometryLocation(0);     
         
    }

    // PSU WopWop Namelist file 
 
    WriteOutPSUWopWopCaseAndNameListFiles();

    // Close up files
    
    fclose(StatusFile_);    
    fclose(InputADBFile_);
    fclose(ADBFile_);
    fclose(ADBCaseListFile_);
    
    // Close PSU WopWop Files

    fclose(PSUWopWopNameListFile_);
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryIsARotor() ) {

          ComponentGroupList_[c].WopWop().CloseFiles();
          
       }
       
    }

    // Close any rotor coefficient files

    k = 0;
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       fclose(GroupFile_[c]);
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) fclose(RotorFile_[++k]);
          
    }
    
}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER WriteOutTimeAccurateNoiseFiles                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutTimeAccurateNoiseFiles(int Case)
{
 
    int c, i, k, Found, NewTime;
    double Time_0, Time_1, Time_2, Epsilon, EvaluationTime, Period;
    char StatusFileName[2000], ADBFileName[2000];
    char GroupFileName[2000], RotorFileName[2000];
   
    // Zero out solution
   
    zero_double_array(Gamma_[0], NumberOfVortexLoops_); Gamma_[0][0] = 0.;   
    zero_double_array(Gamma_[1], NumberOfVortexLoops_); Gamma_[1][0] = 0.;
    zero_double_array(Gamma_[2], NumberOfVortexLoops_); Gamma_[2][0] = 0.;
    zero_double_array(Delta_,    NumberOfVortexLoops_);    Delta_[0] = 0.;
        
    CurrentTime_ = 0.;
       
    // Keep track of unsteady forces and moments
    
    if ( TimeAccurate_ ) {
       
       if (  CL_Unsteady_ != NULL ) delete []  CL_Unsteady_;    
       if (  CD_Unsteady_ != NULL ) delete []  CD_Unsteady_;      
       if (  CS_Unsteady_ != NULL ) delete []  CS_Unsteady_;       
       if ( CFx_Unsteady_ != NULL ) delete [] CFx_Unsteady_;      
       if ( CFy_Unsteady_ != NULL ) delete [] CFy_Unsteady_;       
       if ( CFz_Unsteady_ != NULL ) delete [] CFz_Unsteady_;       
       if ( CMx_Unsteady_ != NULL ) delete [] CMx_Unsteady_;       
       if ( CMy_Unsteady_ != NULL ) delete [] CMy_Unsteady_;       
       if ( CMz_Unsteady_ != NULL ) delete [] CMz_Unsteady_;       
             
        CL_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
        CD_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
        CS_Unsteady_ = new double[NumberOfTimeSteps_ + 1];    
       CFx_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CFy_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CFz_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMx_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMy_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMz_Unsteady_ = new double[NumberOfTimeSteps_ + 1];       
 
    }

    // Update geometry location... really just the surface velocities
             
    CurrentTime_ = CurrentNoiseTime_ = 0.;
             
    if ( TimeAccurate_ && !StartFromSteadyState_ ) UpdateGeometryLocation(1);
                   
    // Initialize free stream
    
    InitializeFreeStream();

    // Initialize the wake trailing vortices

    InitializeTrailingVortices();
    
    ZeroVortexState();

    // Create interaction list for fixed components

    CreateSurfaceVorticesInteractionList(0);
    
    // Calculate the right hand side
    
    CalculateRightHandSide();

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

        VortexLoop(i).Gamma() = Gamma(i) = 0.;
 
    }

    // Open status file

    sprintf(StatusFileName,"%s.noise.history",FileName_);
    
    if ( (StatusFile_ = fopen(StatusFileName, "w")) == NULL ) {
   
       printf("Could not open the history file for output! \n");
   
       exit(1);
   
    }    

    // Write out generic header
    
    WriteCaseHeader(StatusFile_);

    // Status update to user
    
    fprintf(StatusFile_,"\n\nSolver Case: %d \n\n",ABS(Case));


                       //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
    fprintf(StatusFile_,"  Time      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS    CL_Un     CDi_Un    CS_Un     CFx_Un     CFy_Un    CFz_Un   CMx_Un    CMy_Un    CMz_Un \n");

    // If a rotor or unsteady path following case, open any required rotor files
    
    GroupFile_ = new FILE*[NumberOfComponentGroups_ + 1];
        
    k = 0;
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) k++;
       
    }
    
    k++;

    RotorFile_ = new FILE*[k + 1];
    
    k = 0;
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
  
       // Create group file
       
       sprintf(GroupFileName,"%s.noise.group.%d",FileName_,c);
 
       if ( (GroupFile_[c] = fopen(GroupFileName, "w")) == NULL ) {
   
          printf("Could not open the %s group coefficient file! \n",GroupFileName);
   
          exit(1);
   
       }

                            //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890  
       fprintf(GroupFile_[c],"Force Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
       fprintf(GroupFile_[c],"   Time        Cx         Cy         Cz        Cxo        Cyo        Czo        Cxi        Cyi        Czi        Cmx        Cmy        Cmz        Cmxo       Cmyo       Cmzo       Cmxi       Cmyi       Cmzi        CL         CD        CLo        CDo        CLi        CDi \n");
                          
       // Create rotor file
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          k++;
          
          sprintf(RotorFileName,"%s.noise.rotor.%d",FileName_,k);
    
          if ( (RotorFile_[k] = fopen(RotorFileName, "w")) == NULL ) {
      
             printf("Could not open the %s rotor coefficient file! \n",RotorFileName);
      
             exit(1);
      
          }

                               //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890  
          fprintf(RotorFile_[k],"Rotor Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
          fprintf(RotorFile_[k],"   Time     Diameter     RPM       Thrust     Thrusto    Thrusti    Moment     Momento    Momenti      J          CT         CQ        EtaP       Angle \n");
                             
       }

    }     
  
    // Open the input adb file

    sprintf(ADBFileName,"%s.adb",FileName_);
    
    if ( (InputADBFile_ = fopen(ADBFileName, "rb")) == NULL ) {
    
       printf("Could not open the aerothermal data base file for binary input! \n");
    
       exit(1);
    
    }

    // Open the output adb file

    WopWopWriteOutADBFile_ = 1;    

    if ( WopWopWriteOutADBFile_ ) {

       sprintf(ADBFileName,"%s.noise.adb",FileName_);
       
       if ( (ADBFile_ = fopen(ADBFileName, "wb")) == NULL ) {
   
          printf("Could not open the aerothermal data base file for binary output! \n");
   
          exit(1);
   
       }

       WriteOutAerothermalDatabaseHeader();

    }

    // Open the case file
    
    sprintf(ADBFileName,"%s.noise.adb.cases",FileName_);
    
    if ( (ADBCaseListFile_ = fopen(ADBFileName, "w")) == NULL ) {

       printf("Could not open the aerothermal data base case list file for output! \n");

       exit(1);

    }       
    
    // Set up PSU-WopWop data
           
    SetupPSUWopWopData();
   
    // Loop over all the component groups and write out the PSU-WopWop data files

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       printf("\nWorking on group: %d \n",c);fflush(NULL);

       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
       
          Period = 2.*PI/ABS(ComponentGroupList_[c].Omega());
          
       }
       
       else {

          Period = 2.*PI/WopWopOmegaMin_;
          
       }
   
       NoiseTimeStep_ = Period / (NumberOfNoiseTimeSteps_ - 1);
   
       // Rewind ADB file
       
       rewind(InputADBFile_);

       // Write out PSU-WopWopfile headers

       WriteOutPSUWopWopFileHeadersForGroup(c);

       // Read in the header
       
       ReadInAerothermalDatabaseHeader();
              
       // Calculate noise start and stop times, time step

       printf("VSPAERO    Time Step: %f \n",TimeStep_);
       printf("PSU-WopWop Time Step: %f \n",NoiseTimeStep_);

       if ( TimeAccurate_ && !StartFromSteadyState_ ) WakeIterations_ = 1;
       
       // Read in the initial state, t-4dt --> t = 0
       
       ReadInAerothermalDatabaseGeometry();
       
       ReadInAerothermalDatabaseSolution(5);
       
       // Read in the initial state, t-3dt --> t = dt
       
       ReadInAerothermalDatabaseGeometry();
   
       ReadInAerothermalDatabaseSolution(4);
       
       // Read in the initial state, t-2dt --> t = 2dt
       
       ReadInAerothermalDatabaseGeometry();
   
       ReadInAerothermalDatabaseSolution(3);
           
       // Read in the initial state, t-dt --> t = 3dt
       
       ReadInAerothermalDatabaseGeometry();
   
       ReadInAerothermalDatabaseSolution(2);
   
       // Read in state after fist time step, t-0 -->  --> t = 4dt
       
       ReadInAerothermalDatabaseGeometry();
   
       ReadInAerothermalDatabaseSolution(1);    

       // Read in state after fist time step, t+dt -->  --> t = 5dt
       
       ReadInAerothermalDatabaseGeometry();
   
       ReadInAerothermalDatabaseSolution(0);    
      
       Time_ = 5;
   
       Time_0 =                (Time_-2)*TimeStep_;
       
       Time_1 = CurrentTime_ = (Time_-1)*TimeStep_;
       
       Time_2 =                (Time_  )*TimeStep_;
   
       Epsilon = TimeStep_/1000.;     
       
       NewTime = 0;

       // Move over each time step and read in solution from adb ... and write out corresponding noise files
   
       if ( Verbose_ ) printf("Stepping through solution and writing out sound files... \n\n");fflush(NULL);
   
       if ( NoiseTimeStep_ > TimeStep_ ) {
          
          printf("Noise Time Step must be <= VSPAERO Solver Time Step! \n");
          exit(1);
          
       }
   
       for ( NoiseTime_ = 1 ; NoiseTime_ <= NumberOfNoiseTimeSteps_ ; NoiseTime_++ ) {
          
          CurrentNoiseTime_ = (NoiseTime_-1)*NoiseTimeStep_;
          
          EvaluationTime = CurrentNoiseTime_ + NoiseTimeShift_;
      
          printf("Noise Time Step: %d ... Time: %f \r",NoiseTime_, CurrentNoiseTime_); fflush(NULL);
          
          // Find solver time interval that bounds the noise time
          
          Found = 0;
       
          // Check if we are still in the current noise interval
      
          if ( EvaluationTime >= Time_0 - Epsilon && EvaluationTime <= Time_1 + Epsilon ) {
             
             // Nothing to do, current solution interval still bounds noise time
             
             if ( Verbose_ ) printf("Current solver interval of %e to %e is bounding... for noise time of: %e ... which is evaluated at: %e \n",CurrentTime_ - TimeStep_, CurrentTime_,CurrentNoiseTime_, EvaluationTime);
             
             Found = 1;
             
          }
          
          else if ( NewTime == NumberOfTimeSteps_ - 1 && EvaluationTime >= NumberOfTimeSteps_ * TimeStep_ ) {
             
             Found = 1;
          
          }
          
          // Otherwise, we need to search for the new interval
          
          else {
   
             if ( Verbose_ ) printf("Searching for interval containing noise time of: %e ... which is evaluated at: %e \n",CurrentNoiseTime_, EvaluationTime);
             
             NewTime = Time_ + 1;
             
             while ( !Found && NewTime <= NumberOfTimeSteps_ + 1 ) {
   
                Time_0 = (NewTime-2)*TimeStep_;
                Time_1 = (NewTime-1)*TimeStep_;
   
                if ( Verbose_ ) printf("Trying Time: %d ... with interval: %e to %e \n",NewTime, Time_0, Time_1 );
         
                // Check if we are inside new interval
                
                if ( EvaluationTime >= Time_0 - Epsilon && EvaluationTime <= Time_1 + Epsilon ) {
                   
                   Found = 1;
   
                   if ( Verbose_ ) printf("Moving to solver time interval: %d ... time: %e to %e \n",NewTime, Time_0, Time_1 );
                   
                   Time_ = NewTime;
                   
                   CurrentTime_ = Time_1;
                                   
                }
   
                
                else {
                   
                   NewTime++;
                   
                }
                
                // Read in the next ADB Solution
                  
                if ( Verbose_ ) printf("Reading in next geometry set... \n");
                
                ReadInAerothermalDatabaseGeometry();
                
                if ( Verbose_ ) printf("Reading in next solution set... \n");
                
                ReadInAerothermalDatabaseSolution(-1);
                          
             }
          
          }
          
          if ( !Found ) {
             
             printf("Noise time is outside of solver solution time domain! \n");
             printf("Noise time: %f \n",EvaluationTime);
             printf("Solution time domain: %f to %f \n",0.,NumberOfTimeSteps_*TimeStep_);
             
             fflush(NULL); 
             exit(1);
             
          }
          
          // Interpolate solution
          
          else {

             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
                
                if ( NoiseTime_ == 1                        ) SavePeriodicNoiseSolution();
                
                if ( CurrentNoiseTime_ >= Period - TimeStep_ - Epsilon ) RestorePeriodicNoiseSolution();
                
             }
                
             InterpolateExistingSolution(EvaluationTime);
  
             UpdateVortexEdgeStrengths(1, ALL_WAKE_GAMMAS);
   
          }
         
          // Initialize the free stream conditions
                     
          InitializeFreeStream();
       
          // Calculate forces
   
          CalculateForces();
     
          // Output status
   
          OutputStatusFile();
   
          // Write out any rotor coefficients
          
          CalculateRotorCoefficientsForGroup(c);
          
          // Write out geometry and current solution
   
          sprintf(CaseString_,"Time: %-f ...",CurrentNoiseTime_);
   
          if ( WopWopWriteOutADBFile_ ) WriteOutAerothermalDatabaseGeometry();
   
          InterpolateSolutionFromGrid(1);
          
          if ( WopWopWriteOutADBFile_ ) WriteOutAerothermalDatabaseSolution();
                        
          // Write out PSU WopWop Files for unsteady analysis

          WriteOutPSUWopWopUnsteadyDataForGroup(c);
  
          // Update geometry location and interaction lists for moving geoemtries
          
          UpdateGeometryLocation(0);    
             
       }
       
       // Reset the geometry
       
       ResetGeometry();
       
    }

    // PSU WopWop Namelist file 
 
    WriteOutPSUWopWopCaseAndNameListFiles();

    // Close up files
    
    fclose(StatusFile_);    
    fclose(InputADBFile_);
    fclose(ADBCaseListFile_);
    
    if ( WopWopWriteOutADBFile_ ) fclose(ADBFile_);
    
    // Close PSU WopWop Files

    fclose(PSUWopWopNameListFile_);
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryIsARotor() ) {

          ComponentGroupList_[c].WopWop().CloseFiles();
          
       }
       
    }

    // Close any rotor coefficient files

    k = 0;
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       fclose(GroupFile_[c]);
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) fclose(RotorFile_[++k]);
          
    }
    
}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER UpdateWakeConvectedDistance                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::UpdateWakeConvectedDistance(void)
{
   
    int k;
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       VortexSheet(k).CurrentTimeStep() = WakeStartingTime_ + Time_;

       VortexSheet(k).UpdateConvectedDistance();
       
    }
       
}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER CalculateRightHandSide                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateRightHandSide(void)
{
 
    int i, j, k, p, Loop;
    double Normal[3];

    // Update righthandside, Mach dependence 

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       RightHandSide_[i] = -vector_dot(VortexLoop(i).Normal(), VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity());

    }
    
    // Modify righthandside for control surface deflections
    
    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          for ( k = 1 ; k <= VSPGeom().VSP_Surface(j).NumberOfControlSurfaces() ; k++ ) {
       
             for ( p = 1 ; p <= VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfLoops() ; p++ ) {
             
                Loop = VSPGeom().VSP_Surface(j).ControlSurface(k).LoopList(p);
          
                Normal[0] = VortexLoop(Loop).Normal()[0];
                Normal[1] = VortexLoop(Loop).Normal()[1];
                Normal[2] = VortexLoop(Loop).Normal()[2];

                VSPGeom().VSP_Surface(j).ControlSurface(k).RotateNormal(Normal);
            
                RightHandSide_[Loop] = -vector_dot(Normal, VSPGeom().Grid(1).LoopList(Loop).LocalFreeStreamVelocity());
                                
                
             }
             
          }
          
       }
       
    }
    
    if ( ModelType_ == PANEL_MODEL ) {
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
          
          if ( LoopIsOnBaseRegion_[i] ) RightHandSide_[i] = 0.;
          
       }       
       
    }    
 
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER SolveLinearSystem                             #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::SolveLinearSystem(void)
{
   
    int i;
    
    // Calculate preconditioners
  
    if ( CurrentWakeIteration_ == 1 && !DumpGeom_ ) {

       if ( Preconditioner_ != MATCON ) CalculateDiagonal();       
     
       if ( Preconditioner_ == SSOR   ) CalculateNeighborCoefs();

       if ( Preconditioner_ == MATCON ) CreateMatrixPreconditioners();

       for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
          
          GammaNM2(i) = GammaNM1(i) = GammaNM1(i) = Gamma(i) = Delta_[i] = 0.;
          
       }
       
    }

    // Solver the linear system

    Do_GMRES_Solve();    
    
    // Update the vortex strengths on the wake

    UpdateVortexEdgeStrengths(1, ALL_WAKE_GAMMAS);

    if ( SaveRestartFile_ ) WriteRestartFile();
    
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER CalculateDiagonal                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateDiagonal(void)
{

    int i, j, Edge, Node1, Node2;
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
        
          if ( VortexLoop(i).SurfaceType() == DEGEN_BODY_SURFACE ) printf("Loop: %d on body surface: %d has zero diagonal... \n",i,VortexLoop(i).DegenBodyID());fflush(NULL);
          if ( VortexLoop(i).SurfaceType() == DEGEN_WING_SURFACE ) printf("Loop: %d on wing surface: %d has zero diagonal... \n",i,VortexLoop(i).DegenWingID());fflush(NULL);
          if ( VortexLoop(i).SurfaceType() == CART3D_SURFACE     ) printf("Loop: %d on wing surface: %d has zero diagonal... \n",i,VortexLoop(i).Cart3dID());fflush(NULL);
         
          printf("Area: %e \n",VortexLoop(i).Area());
          
          for ( j = 1 ; j <= VortexLoop(i).NumberOfEdges() ; j++ ) {
             
             Edge = VortexLoop(i).Edge(j);
             
             Node1 = SurfaceVortexEdge(j).Node1();
             Node2 = SurfaceVortexEdge(j).Node2();
             
             printf("Edge: %d --> Node 1: %d @ %lf %lf %lf ... Node 2: %d @ %lf %lf %lf \n",
                    Edge,
                    Node1,
                    VSPGeom().Grid(MGLevel_).NodeList(Node1).x(),
                    VSPGeom().Grid(MGLevel_).NodeList(Node1).y(),
                    VSPGeom().Grid(MGLevel_).NodeList(Node1).z(),
                    Node2,
                    VSPGeom().Grid(MGLevel_).NodeList(Node2).x(),
                    VSPGeom().Grid(MGLevel_).NodeList(Node2).y(),
                    VSPGeom().Grid(MGLevel_).NodeList(Node2).z());
          }                    
                    
       }
     
       Diagonal_[i] = 1./Diagonal_[i];
        
    }  
    
    if ( ModelType_ == PANEL_MODEL ) {
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
          
          if ( LoopIsOnBaseRegion_[i] ) Diagonal_[i] = 1.;
          
       }       
       
    }      
    
    // Find smallest diagonal coefficient
    
    MaxDiagonal_ = -1.e9;

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       MaxDiagonal_ = MAX(MaxDiagonal_,Diagonal_[i]);

    }    
 
}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CalculateNeighborCoefs                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateNeighborCoefs(void)
{

    int j, k, Edge, Loop1, Loop2;
    double q[4];

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       SurfaceVortexEdge(j).EdgeCoef(0) = 0.;
       SurfaceVortexEdge(j).EdgeCoef(1) = 0.;

    }
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       Loop1 = SurfaceVortexEdge(j).VortexLoop1();
       Loop2 = SurfaceVortexEdge(j).VortexLoop2();
       
       for ( k = 1 ; k <= VortexLoop(Loop2).NumberOfEdges() ; k++ ) {
          
          Edge = VortexLoop(Loop2).Edge(k);
             
          SurfaceVortexEdge(Edge).Gamma() = 1.;
          
          if ( SurfaceVortexEdge(Edge).VortexLoop1() != Loop2 ) SurfaceVortexEdge(Edge).Gamma() = -1.;
          
          SurfaceVortexEdge(Edge).InducedVelocity(VortexLoop(Loop1).xyz_c(), q);
          
          SurfaceVortexEdge(j).EdgeCoef(0) += vector_dot(VortexLoop(Loop1).Normal(), q);
          
       }
       
       Loop1 = SurfaceVortexEdge(j).VortexLoop2();
       Loop2 = SurfaceVortexEdge(j).VortexLoop1();
              
       for ( k = 1 ; k <= VortexLoop(Loop2).NumberOfEdges() ; k++ ) {
          
          Edge = VortexLoop(Loop2).Edge(k);
             
          SurfaceVortexEdge(Edge).Gamma() = 1.;
          
          if ( SurfaceVortexEdge(Edge).VortexLoop1() != Loop2 ) SurfaceVortexEdge(Edge).Gamma() = -1.;
          
          SurfaceVortexEdge(Edge).InducedVelocity(VortexLoop(Loop1).xyz_c(), q);
          
          SurfaceVortexEdge(j).EdgeCoef(1) += vector_dot(VortexLoop(Loop1).Normal(), q);
          
       }
       
    }       

    // Scale the coefficients... note that Diagonal_ holds the inverse 
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       SurfaceVortexEdge(j).EdgeCoef(0) *= Diagonal_[SurfaceVortexEdge(j).VortexLoop1()];
       SurfaceVortexEdge(j).EdgeCoef(1) *= Diagonal_[SurfaceVortexEdge(j).VortexLoop2()];

    }    
    
    // Zero out boundaries
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       if ( SurfaceVortexEdge(j).VortexLoop1() == 0 ) SurfaceVortexEdge(j).EdgeCoef(1) = 0.;
       if ( SurfaceVortexEdge(j).VortexLoop2() == 0 ) SurfaceVortexEdge(j).EdgeCoef(0) = 0.;

    }      
    
    // Zero out trailing edges
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {
          
          SurfaceVortexEdge(j).EdgeCoef(0) = 0.;
          SurfaceVortexEdge(j).EdgeCoef(1) = 0.;
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CreateMatrixPreconditioners                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateMatrixPreconditioners(void)
{

    int i, j, k, m, Neq, iLoop, jLoop, Edge, DoIt;
    double q[3], *Diagonal, Ws, Tolerance, Distance, NormalDistance, Vec[3], Ratio;

    for ( k = 1 ; k <= NumberOfMatrixPreconditioners_ ; k++ ) {
       
       Neq = MatrixPreconditionerList_[k].NumberOfVortexLoops();
       
       for ( i = 1 ; i <= Neq ; i++ ) {
          
          iLoop = MatrixPreconditionerList_[k].VortexLoopList(i);

          if ( !LoopIsOnBaseRegion_[iLoop] ) {

             for ( j = 1 ; j <= Neq ; j++ ) {
                
                jLoop = MatrixPreconditionerList_[k].VortexLoopList(j);
                
                MatrixPreconditionerList_[k].A()(i,j) = 0.;
      
                DoIt = 1;

/* Testing...                      
                // Modify the matrix for sharp TEs
         
                if ( iLoop != jLoop && ModelType_ == PANEL_MODEL ) {

                   Tolerance = 0.25*VortexLoop(iLoop).RefLength();
                
                   Vec[0] = VortexLoop(iLoop).Xc() - VortexLoop(jLoop).Xc();
                   Vec[1] = VortexLoop(iLoop).Yc() - VortexLoop(jLoop).Yc();
                   Vec[2] = VortexLoop(iLoop).Zc() - VortexLoop(jLoop).Zc();
                   
                   Distance = sqrt(vector_dot(Vec,Vec));
                   
                   Ratio = Distance / VortexLoop(iLoop).RefLength();
       
                   if ( Ratio <= 2. ) {
                
                      if ( vector_dot(VortexLoop(iLoop).Normal(), VortexLoop(jLoop).Normal()) < 0. ) {
                   
                         // Calculate normal distance
      
                         NormalDistance = ABS(vector_dot(Vec,VortexLoop(jLoop).Normal()));
           
                         if ( ABS(NormalDistance) <= Tolerance ) DoIt = 0;    

                      }
                      
                   }
       
                }
 */
                 
                if ( DoIt ) {

                   for ( m = 1 ; m <= VortexLoop(jLoop).NumberOfEdges() ; m++ ) {
                      
                      Edge = VortexLoop(jLoop).Edge(m);
                      
                      if ( !SurfaceVortexEdge(Edge).IsTrailingEdge() ) {
                         
                         SurfaceVortexEdge(Edge).Gamma() = 1.;
                         
                         if ( SurfaceVortexEdge(Edge).VortexLoop1() != jLoop ) SurfaceVortexEdge(Edge).Gamma() = -1.;
                         
                         SurfaceVortexEdge(Edge).InducedVelocity(VortexLoop(iLoop).xyz_c(), q);
          
                         MatrixPreconditionerList_[k].A()(i,j) += vector_dot(VortexLoop(iLoop).Normal(), q);
                         
                         SurfaceVortexEdge(Edge).Gamma() = 0.;
     
                      }
       
                   }
                   
                }
    
             }
             
          }
          
          else {
  
             for ( j = 1 ; j <= Neq ; j++ ) {
                
                MatrixPreconditionerList_[k].A()(i,j) = 0.;
                
             }
             
             MatrixPreconditionerList_[k].A()(i,i) = 1.;
                
          }

       }

    }
    
    // If flow is supersonic add in generalized principal part of downwash
    
    if ( Mach_ > 1. ) {
       
       Diagonal = new double[NumberOfVortexLoops_ + 1];
       
       zero_double_array(Diagonal, NumberOfVortexLoops_);

       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

          Ws = SurfaceVortexEdge(j).GeneralizedPrincipalPartOfDownWash();
        
          // Loop 1
           
          if ( SurfaceVortexEdge(j).VortexLoop1IsDownWind() ) {
           
             i = SurfaceVortexEdge(j).VortexLoop1();
        
             Diagonal[i] += Ws * SurfaceVortexEdge(j).VortexLoop1DownWindWeight();
       
          }

          // Loop 2

          if ( SurfaceVortexEdge(j).VortexLoop2IsDownWind() ) {
           
             i = SurfaceVortexEdge(j).VortexLoop2();

             Diagonal[i] += Ws * SurfaceVortexEdge(j).VortexLoop2DownWindWeight();
       
          }

       }
       
       for ( k = 1 ; k <= NumberOfMatrixPreconditioners_ ; k++ ) {
          
          Neq = MatrixPreconditionerList_[k].NumberOfVortexLoops();
          
          for ( i = 1 ; i <= Neq ; i++ ) {
             
             iLoop = MatrixPreconditionerList_[k].VortexLoopList(i);

             if ( !LoopIsOnBaseRegion_[iLoop] ) {
   
                MatrixPreconditionerList_[k].A()(i,i) += Diagonal[iLoop];
                
             }
          
          }

       }
   
       delete [] Diagonal;
       
    }

    // Form LU decomposition
    
#pragma omp parallel for schedule(dynamic)  
    for ( k = 1 ; k <= NumberOfMatrixPreconditioners_ ; k++ ) {
       
       MatrixPreconditionerList_[k].LU();
       
    }
                       
}

/*##############################################################################
#                                                                              #
#           VSP_SOLVER CreateMatrixPreconditionersDataStructure                #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateMatrixPreconditionersDataStructure(void)
{

    int i, j, k, p, Done, Loops, Level, *LoopList, *NumLoops;
    int TargetLoops, MinLoops, MaxLoops, AvgLoops;
    
    printf("Creating matrix preconditioners data structure... \n");
    
    if ( VSPGeom().NumberOfGridLevels() == 1 ) {
       
       printf("Error... mesh too coarse, or something else failed. Stopping in CreateMatrixPreconditionersDataStructure! \n");
       fflush(NULL);
       exit(1);
       
    }

    else {
       
       LoopList = new int[NumberOfVortexLoops_ + 1];
       
       NumLoops = new int[NumberOfVortexLoops_ + 1];
       
       zero_int_array(LoopList, NumberOfVortexLoops_);
       
       zero_int_array(NumLoops, NumberOfVortexLoops_);
       
       // Determine level to start at
       
       TargetLoops = MIN(500, NumberOfVortexLoops_/5);

       Done = 0;
       
       Level = VSPGeom().NumberOfGridLevels();
       
       printf("Starting at level: %d \n",Level);
       
       while ( !Done && Level >= 1 ) {
   
          Loops = 0;
   
          for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {
     
             NumLoops[i] = CalculateNumberOfFineLoops(Level, VSPGeom().Grid(Level).LoopList(i), LoopList);
    
             Loops = MAX(Loops,NumLoops[i]);
   
          }
          
          printf("Level: %d has MaxLoops of: %d \n",Level,Loops);
          
          if ( Loops <= 1.25*TargetLoops ) Done = 1;
   
          Level--;
          
       }
       
       Level++;
       
       printf("Starting at level: %d \n",Level); fflush(NULL);
       
       // Now group vortex loops into groups, one each per matrix preconditioner
   
       MatrixPreconditionerList_ = new MATPRECON[VSPGeom().Grid(Level).NumberOfLoops() + 1];
   
       i = 1; 
   
       Done = Loops = p = 0;
       
       zero_int_array(LoopList, NumberOfVortexLoops_);
       
       MinLoops = 1.e9;
       MaxLoops = -MinLoops;
       AvgLoops = 0;
       
       while ( i <= VSPGeom().Grid(Level).NumberOfLoops() && !Done ) {
     
          Loops += CalculateNumberOfFineLoops(Level, VSPGeom().Grid(Level).LoopList(i), LoopList);
   
          if ( ( Loops >= TargetLoops || Loops + NumLoops[i+1] > 1.25*TargetLoops ) || i == VSPGeom().Grid(Level).NumberOfLoops() ) {
   
             MinLoops = MIN(MinLoops,Loops);
             MaxLoops = MAX(MaxLoops,Loops);
             AvgLoops += Loops;
             
             p++;
             
             if ( Verbose_ ) printf("Preconditioning Matrix %d contains %d fine loops \n",p,Loops); fflush(NULL);
      
             MatrixPreconditionerList_[p].Size(Loops);
   
             k = 0;
             
             for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {
                
                if ( LoopList[j] ) MatrixPreconditionerList_[p].VortexLoopList(++k) = j;
   
             }
             
             if ( k != Loops ) {
                
                printf("Error in creating preconditioning matrix data structure! \n");
                printf("k: %d ... Loops: %d \n",k,Loops);
                
                fflush(NULL);
                
                exit(1);
                
             }
      
             zero_int_array(LoopList, NumberOfVortexLoops_);
             
             Loops = 0;
             
          }
   
          i++;
           
       }
       
       AvgLoops /= p;
       
       printf("Created: %d Matrix preconditioners \n",p);
       printf("Min matrix: %d Loops \n",MinLoops);
       printf("Max matrix: %d Loops \n",MaxLoops);
       printf("Avg matrix: %d Loops \n",AvgLoops);
   
       printf("\n");
       
       NumberOfMatrixPreconditioners_ = p;
       
       delete [] LoopList;
       delete [] NumLoops;
       
    }

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER CalculateNumberOfFineLoops                    #
#                                                                              #
##############################################################################*/

int VSP_SOLVER::CalculateNumberOfFineLoops(int Level, VSP_LOOP &Loop, int *LoopList)
{

    int i, FineLoops;

    FineLoops = 0;

    if ( Level == 2 ) {

       for ( i = 1 ; i <= Loop.NumberOfFineGridLoops() ; i++ ) {
    
          FineLoops++;
          
          LoopList[Loop.FineGridLoop(i)] = 1;

       }
       
    }
    
    else {
  
       for ( i = 1 ; i <= Loop.NumberOfFineGridLoops() ; i++ ) {
    
          FineLoops += CalculateNumberOfFineLoops(Level-1,VSPGeom().Grid(Level-1).LoopList(Loop.FineGridLoop(i)),LoopList);

       }
       
    }
    
    return FineLoops;

}

/*##############################################################################
#                                                                              #
#                 VSP_SOLVER DoPreconditionedMatrixMultiply                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DoPreconditionedMatrixMultiply(double *vec_in, double *vec_out)
{

    DoMatrixMultiply(vec_in,vec_out);

    DoMatrixPrecondition(vec_out);

}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER DoMatrixMultiply                            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DoMatrixMultiply(double *vec_in, double *vec_out)
{

    if ( ModelType_ == VLM_MODEL ) {
      
       MatrixMultiply(vec_in, vec_out);
      
    }
   
    else if ( ModelType_ == PANEL_MODEL ) {

       MatrixMultiply(vec_in, MatrixVecTemp_);
    
       MatrixTransposeMultiply(MatrixVecTemp_, vec_out);

    }

}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER MatrixMultiply                            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::MatrixMultiply(double *vec_in, double *vec_out)
{

    int i, j, k, v, Level, Loop, LoopType, MaxLoopTypes, NumberOfSheets, cpu;
    double xyz[3], q[4], Ws, U, V, W;
    VSP_EDGE *VortexEdge;
    VORTEX_SHEET_ENTRY *VortexSheetList;

    zero_double_array(vec_out,NumberOfVortexLoops_);
    
    Gamma(0) = 0.;
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       Gamma(i) = vec_in[i];
       
    }

    // Restrict the current solution to the coarser grids

    UpdateVortexEdgeStrengths(1, IMPLICIT_WAKE_GAMMAS);
     
    for ( Level = 1 ; Level < NumberOfMGLevels_ ; Level++ ) {
       
       RestrictSolutionFromGrid(Level);
           
       UpdateVortexEdgeStrengths(Level+1, IMPLICIT_WAKE_GAMMAS);
  
    }

    // Surface vortex induced velocities 

    ZeroLoopVelocities();

    MaxLoopTypes = 0;
    
    if ( !AllComponentsAreFixed_ ) MaxLoopTypes = 1;

    for ( LoopType = 0 ; LoopType <= MaxLoopTypes ; LoopType++ ) {

#pragma omp parallel for reduction(+:U,V,W) private(j,Level,Loop,xyz,q,VortexEdge) schedule(dynamic)
       for ( i = 1 ; i <= NumberOfInteractionLoops_[LoopType] ; i++ ) {
       
          Level = InteractionLoopList_[LoopType][i].Level();
          
          Loop  = InteractionLoopList_[LoopType][i].Loop();
        
          U = V = W = 0.;

          for ( j = 1 ; j <= InteractionLoopList_[LoopType][i].NumberOfVortexEdges() ; j++ ) {
    
             VortexEdge = InteractionLoopList_[LoopType][i].SurfaceVortexEdgeInteractionList(j);
   
             // Calculate influence of this edge
  
             VortexEdge->InducedVelocity(VSPGeom().Grid(Level).LoopList(Loop).xyz_c(), q);
     
             U += q[0];
             V += q[1];
             W += q[2];
           
             // If there is ground effects, z plane...
             
             if ( DoGroundEffectsAnalysis() ) {
   
                xyz[0] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[0];
                xyz[1] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[1];
                xyz[2] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[2];
               
                xyz[2] *= -1.;
               
                VortexEdge->InducedVelocity(xyz, q);
         
                q[2] *= -1.;
     
                U += q[0];
                V += q[1];
                W += q[2];
               
             }    
                          
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {
   
                xyz[0] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[0];
                xyz[1] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[1];
                xyz[2] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[2];
               
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
                  
                if ( DoGroundEffectsAnalysis() ) {
   
                   xyz[2] *= -1.;
                  
                   VortexEdge->InducedVelocity(xyz, q);
            
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                         q[2] *= -1.;
   
                   U += q[0];
                   V += q[1];
                   W += q[2];
                  
                }                   
               
             }             
   
          }

          VSPGeom().Grid(Level).LoopList(Loop).U() += U;
          VSPGeom().Grid(Level).LoopList(Loop).V() += V;   
          VSPGeom().Grid(Level).LoopList(Loop).W() += W;

       }   
       
    }

    // Copy over vortex sheet data for parallel runs
    
    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {

       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
          VortexSheet_[cpu][k] += VortexSheet_[0][k];
    
       }  
       
    }   

    // Trailing vortex induced velocities

    for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {

#pragma omp parallel for private(cpu, Level, Loop, NumberOfSheets, VortexSheetList, xyz, q, U, V, W) schedule(dynamic)
       for ( i = 1 ; i <= NumberOfVortexSheetInteractionLoops_[v] ; i++ ) {
        
#ifdef VSPAERO_OPENMP    
          cpu = omp_get_thread_num();
#else
          cpu = 0;
#endif  
      
          Level = VortexSheetInteractionLoopList_[v][i].Level();

          Loop  = VortexSheetInteractionLoopList_[v][i].Loop();
          
          NumberOfSheets = VortexSheetInteractionLoopList_[v][i].NumberOfVortexSheets();
          
          VortexSheetList = VortexSheetInteractionLoopList_[v][i].VortexSheetList_;
          
          VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, VSPGeom().Grid(Level).LoopList(Loop).xyz_c(), q);
          
          U = q[0];
          V = q[1];
          W = q[2];

          // If there is ground effects, z plane...
        
          if ( DoGroundEffectsAnalysis() ) {
           
             xyz[0] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[0];
             xyz[1] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[1];
             xyz[2] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[2];
             
             xyz[2] *= -1.;
        
             VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, q);
             
             q[2] *= -1.;
             
             U += q[0];
             V += q[1];
             W += q[2];
             
          }   
          
          // If there is a symmetry plane, calculate influence of the reflection
        
          if ( DoSymmetryPlaneSolve_ ) {
           
             xyz[0] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[0];
             xyz[1] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[1];
             xyz[2] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[2];
             
             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
             
             VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, q);
             
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
             
             U += q[0];
             V += q[1];
             W += q[2];
            
             // If there is ground effects, z plane...
           
             if ( DoGroundEffectsAnalysis() ) {
  
                xyz[2] *= -1.;
             
                VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, q);
    
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                      q[2] *= -1.;
             
                U += q[0];
                V += q[1];
                W += q[2];
             
             }   
         
          }   
          
          VSPGeom().Grid(Level).LoopList(Loop).U() += U;
          VSPGeom().Grid(Level).LoopList(Loop).V() += V;   
          VSPGeom().Grid(Level).LoopList(Loop).W() += W;
          
       }
          
    }

    ProlongateVelocity();
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       vec_out[i] = vector_dot(VortexLoop(i).Normal(), VortexLoop(i).Velocity());
       
    }

    // If flow is supersonic add in generalized principal part of downwash
    
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
      
    vec_out[0] = vec_in[0];

    // Kelvin constraint
    
    if ( ModelType_ == PANEL_MODEL ) {

       for ( k = 1 ; k <= NumberOfKelvinConstraints_ ; k++ ) {
          
          vec_out[NumberOfVortexLoops_ + k] = 0.;
          
       }
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
  
          vec_out[NumberOfVortexLoops_ + LoopInKelvinConstraintGroup_[i]] += KelvinLambda_*vec_in[i];
      
       }

    }

    // Base region
    
    if ( ModelType_ == PANEL_MODEL ) {
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
          
          if ( LoopIsOnBaseRegion_[i] ) vec_out[i] = vec_in[i];
          
       }       
       
    }

}

/*##############################################################################
#                                                                              #
#                      VSP_SOLVER ZeroLoopVelocities                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ZeroLoopVelocities(void) 
{
 
    int Level, i;

    for ( Level = 1 ; Level <= NumberOfMGLevels_ ; Level++ ) {

       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

          VSPGeom().Grid(Level).LoopList(i).U() = 0.;
          VSPGeom().Grid(Level).LoopList(i).V() = 0.;
          VSPGeom().Grid(Level).LoopList(i).W() = 0.;
          
          VSPGeom().Grid(Level).LoopList(i).DownWash_U() = 0.;
          VSPGeom().Grid(Level).LoopList(i).DownWash_V() = 0.;
          VSPGeom().Grid(Level).LoopList(i).DownWash_W() = 0.;

       }
       
    }          
            
}

/*##############################################################################
#                                                                              #
#                      VSP_SOLVER ProlongateVelocity                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ProlongateVelocity(void) 
{
 
    int Level, i_c, i_f, j;

    for ( Level = NumberOfMGLevels_ ; Level > 1; Level-- ) {

       for ( i_c = 1 ; i_c <= VSPGeom().Grid(Level).NumberOfLoops() ; i_c++ ) {
         
          for ( j = 1 ; j <= VSPGeom().Grid(Level).LoopList(i_c).NumberOfFineGridLoops() ; j++ ) {
             
             i_f = VSPGeom().Grid(Level).LoopList(i_c).FineGridLoop(j);

             VSPGeom().Grid(Level-1).LoopList(i_f).U() += VSPGeom().Grid(Level).LoopList(i_c).U();
             VSPGeom().Grid(Level-1).LoopList(i_f).V() += VSPGeom().Grid(Level).LoopList(i_c).V();
             VSPGeom().Grid(Level-1).LoopList(i_f).W() += VSPGeom().Grid(Level).LoopList(i_c).W();

             VSPGeom().Grid(Level-1).LoopList(i_f).DownWash_U() += VSPGeom().Grid(Level).LoopList(i_c).DownWash_U();
             VSPGeom().Grid(Level-1).LoopList(i_f).DownWash_V() += VSPGeom().Grid(Level).LoopList(i_c).DownWash_V();
             VSPGeom().Grid(Level-1).LoopList(i_f).DownWash_W() += VSPGeom().Grid(Level).LoopList(i_c).DownWash_W();

          }
          
       }
       
    }          
                  
}

/*##############################################################################
#                                                                              #
#                   VSP_SOLVER ProlongateUnsteadyVelocity                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ProlongateUnsteadyVelocity(void) 
{
 
    int Level, i, i_c, i_f, j;

    for ( Level = NumberOfMGLevels_ ; Level > 1; Level-- ) {

       for ( i_c = 1 ; i_c <= VSPGeom().Grid(Level).NumberOfLoops() ; i_c++ ) {
         
          for ( j = 1 ; j <= VSPGeom().Grid(Level).LoopList(i_c).NumberOfFineGridLoops() ; j++ ) {
             
             i_f = VSPGeom().Grid(Level).LoopList(i_c).FineGridLoop(j);

             VSPGeom().Grid(Level-1).LoopList(i_f).U() += VSPGeom().Grid(Level).LoopList(i_c).U();
             VSPGeom().Grid(Level-1).LoopList(i_f).V() += VSPGeom().Grid(Level).LoopList(i_c).V();
             VSPGeom().Grid(Level-1).LoopList(i_f).W() += VSPGeom().Grid(Level).LoopList(i_c).W();

          }
          
       }
       
    }          
          
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       UnsteadyTrailingWakeVelocity_[i][0] = VortexLoop(i).U();
       UnsteadyTrailingWakeVelocity_[i][1] = VortexLoop(i).V();
       UnsteadyTrailingWakeVelocity_[i][2] = VortexLoop(i).W();

    }

}

/*##############################################################################
#                                                                              #
#                   VSP_SOLVER MatrixTransposeMultiply                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::MatrixTransposeMultiply(double *vec_in, double *vec_out)
{

    int i;
    
    vec_out[0] = vec_in[0];
    
    // Kelvin constraint

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       vec_out[i] = vec_in[i] + KelvinLambda_*vec_in[NumberOfVortexLoops_ + LoopInKelvinConstraintGroup_[i]];
   
    }

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER DoMatrixPrecondition                          #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DoMatrixPrecondition(double *vec_in)
{

    int i, j, k;

    // Precondition using Jacobi

    if ( Preconditioner_ == JACOBI ) {
       
#pragma omp parallel for    
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
  
          vec_in[i] *= JacobiRelaxationFactor_*Diagonal_[i];
   
       }
       
    }

    // Edge SSOR

    else if ( Preconditioner_ == SSOR ) {

#pragma omp parallel for    
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
         vec_in[i] *= (2.-JacobiRelaxationFactor_)*Diagonal_[i] / JacobiRelaxationFactor_;

       }
   
#pragma omp parallel for       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
           vec_in[i] *= JacobiRelaxationFactor_;
   
       }

       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
          
          if ( SurfaceVortexEdge(j).VortexLoop2() < SurfaceVortexEdge(j).VortexLoop1() ) { 
             
             vec_in[SurfaceVortexEdge(j).VortexLoop1()] -= SurfaceVortexEdge(j).EdgeCoef(0) * vec_in[SurfaceVortexEdge(j).VortexLoop2()];
   
          }
          
          else {
             
             vec_in[SurfaceVortexEdge(j).VortexLoop2()] -= SurfaceVortexEdge(j).EdgeCoef(1) * vec_in[SurfaceVortexEdge(j).VortexLoop1()];
     
          }
   
       }    

#pragma omp parallel for            
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
           vec_in[i] *= JacobiRelaxationFactor_;
   
       }

       for ( j = NumberOfSurfaceVortexEdges_ ; j >= 1 ; j-- ) {
          
          if ( SurfaceVortexEdge(j).VortexLoop2() > SurfaceVortexEdge(j).VortexLoop1() ) { 
             
             vec_in[SurfaceVortexEdge(j).VortexLoop1()] -= SurfaceVortexEdge(j).EdgeCoef(0) * vec_in[SurfaceVortexEdge(j).VortexLoop2()];
             
          }
          
          else {
             
             vec_in[SurfaceVortexEdge(j).VortexLoop2()] -= SurfaceVortexEdge(j).EdgeCoef(1) * vec_in[SurfaceVortexEdge(j).VortexLoop1()];
             
          }
          
       }     

    }
    
    // Matrix precondtioner
    
    else if ( Preconditioner_ == MATCON ) {

#pragma omp parallel for private(i) schedule(dynamic)                   
       for ( k = 1 ; k <= NumberOfMatrixPreconditioners_ ; k++ ) {

          for ( i = 1 ; i <= MatrixPreconditionerList_[k].NumberOfVortexLoops() ; i++ ) {

             MatrixPreconditionerList_[k].x(i) = vec_in[MatrixPreconditionerList_[k].VortexLoopList(i)];
             
          }
          
          MatrixPreconditionerList_[k].Solve();
          
          for ( i = 1 ; i <= MatrixPreconditionerList_[k].NumberOfVortexLoops() ; i++ ) {
      
             vec_in[MatrixPreconditionerList_[k].VortexLoopList(i)] = MatrixPreconditionerList_[k].x(i);
 
          }          
          
       }

    }

    else {
       
       printf("Unknown preconditioner! \n");fflush(NULL);
       exit(1);
       
    }
 
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateVelocities                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateVelocities(void)
{

    int i, j, k, v, Level, Loop, LoopType, MaxLoopTypes, cpu, NumberOfSheets;
    double q[3], xyz[3], Ws, U, V, W;
    VSP_EDGE *VortexEdge;
    VORTEX_SHEET_ENTRY *VortexSheetList;
    
    // Freestream component... includes rotor wash, and any rotational rates
    
    ZeroLoopVelocities();
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       VortexLoop(i).U() = VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(0);
       VortexLoop(i).V() = VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(1);
       VortexLoop(i).W() = VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(2); 
    
    }
       
    // Update the vortex strengths

    UpdateVortexEdgeStrengths(1, ALL_WAKE_GAMMAS);
    
    for ( Level = 1 ; Level < NumberOfMGLevels_ ; Level++ ) {
        
       RestrictSolutionFromGrid(Level);
       
       UpdateVortexEdgeStrengths(Level+1, ALL_WAKE_GAMMAS);

    }

    // Surface vortex induced velocities

    MaxLoopTypes = 0;
    
    if ( !AllComponentsAreFixed_ ) MaxLoopTypes = 1;

    for ( LoopType = 0 ; LoopType <= MaxLoopTypes ; LoopType++ ) {

#pragma omp parallel for reduction(+:U,V,W) private(j,Level,Loop,q,VortexEdge,xyz) schedule(dynamic)          
       for ( i = 1 ; i <= NumberOfInteractionLoops_[LoopType] ; i++ ) {
              
          Level = InteractionLoopList_[LoopType][i].Level();
          
          Loop  = InteractionLoopList_[LoopType][i].Loop();    
       
          U = V = W = 0.;

          for ( j = 1 ; j <= InteractionLoopList_[LoopType][i].NumberOfVortexEdges() ; j++ ) {
    
             VortexEdge = InteractionLoopList_[LoopType][i].SurfaceVortexEdgeInteractionList(j);
   
             VortexEdge->InducedVelocity(VSPGeom().Grid(Level).LoopList(Loop).xyz_c(), q);
         
             U += q[0];
             V += q[1];
             W += q[2];
          
             // If there is ground effects, z plane...
             
             if ( DoGroundEffectsAnalysis() ) {
                
                xyz[0] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[0];
                xyz[1] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[1];
                xyz[2] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[2];
      
                xyz[2] *= -1.;
               
                VortexEdge->InducedVelocity(xyz, q);        
      
                q[2] *= -1.;
               
                U += q[0];
                V += q[1];
                W += q[2];
               
             }     
                       
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {
                
                xyz[0] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[0];
                xyz[1] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[1];
                xyz[2] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[2];
      
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
                
                // If there is ground effects, z plane...
                
                if ( DoGroundEffectsAnalysis() ) {
      
                   xyz[2] *= -1.;
                  
                   VortexEdge->InducedVelocity(xyz, q);        
         
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;         
                                                         q[2] *= -1.;
                  
                   U += q[0];
                   V += q[1];
                   W += q[2];
                  
                }                     
               
             }                
   
          }
           
          VSPGeom().Grid(Level).LoopList(Loop).U() += U;
          VSPGeom().Grid(Level).LoopList(Loop).V() += V;   
          VSPGeom().Grid(Level).LoopList(Loop).W() += W;

       }
       
    }

    // Copy over vortex sheet data for parallel runs
  
    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {

       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
          VortexSheet_[cpu][k] += VortexSheet_[0][k];
    
       }  
       
    }   

    // Trailing vortex induced velocities

    for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {

#pragma omp parallel for private(cpu, Level, Loop, NumberOfSheets, VortexSheetList, xyz, q, U, V, W)
       for ( i = 1 ; i <= NumberOfVortexSheetInteractionLoops_[v] ; i++ ) {

#ifdef VSPAERO_OPENMP    
       cpu = omp_get_thread_num();
#else
       cpu = 0;
#endif  

          Level = VortexSheetInteractionLoopList_[v][i].Level();

          Loop  = VortexSheetInteractionLoopList_[v][i].Loop();
          
          NumberOfSheets = VortexSheetInteractionLoopList_[v][i].NumberOfVortexSheets();
          
          VortexSheetList = VortexSheetInteractionLoopList_[v][i].VortexSheetList_;
          
          VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, VSPGeom().Grid(Level).LoopList(Loop).xyz_c(), q);
          
          U = q[0];
          V = q[1];
          W = q[2];

          // If there is ground effects, z plane...
        
          if ( DoGroundEffectsAnalysis() ) {
           
             xyz[0] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[0];
             xyz[1] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[1];
             xyz[2] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[2];
             
             xyz[2] *= -1.;
        
             VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, q);
             
             q[2] *= -1.;
             
             U += q[0];
             V += q[1];
             W += q[2];
             
          }   
          
          // If there is a symmetry plane, calculate influence of the reflection
        
          if ( DoSymmetryPlaneSolve_ ) {
           
             xyz[0] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[0];
             xyz[1] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[1];
             xyz[2] = VSPGeom().Grid(Level).LoopList(Loop).xyz_c()[2];
             
             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
             
             VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, q);
             
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
             
             U += q[0];
             V += q[1];
             W += q[2];
            
             // If there is ground effects, z plane...
           
             if ( DoGroundEffectsAnalysis() ) {
  
                xyz[2] *= -1.;
             
                VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, q);
    
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                      q[2] *= -1.;
             
                U += q[0];
                V += q[1];
                W += q[2];
             
             }   
         
          }   
      
          VSPGeom().Grid(Level).LoopList(Loop).U() += U;
          VSPGeom().Grid(Level).LoopList(Loop).V() += V;   
          VSPGeom().Grid(Level).LoopList(Loop).W() += W;  
          
          VSPGeom().Grid(Level).LoopList(Loop).DownWash_U() += U;
          VSPGeom().Grid(Level).LoopList(Loop).DownWash_V() += V;   
          VSPGeom().Grid(Level).LoopList(Loop).DownWash_W() += W;                      
  
       }
          
    }
     
    ProlongateVelocity();
        
    // If flow is supersonic add in generalized principal part part of downwash
    
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

    int i, j, k, m, p, t, v, w, cpu, NumberOfSheets, Level;
    double xyz[3], xyz_te[3], q[5], U, V, W, Delta, MaxDelta, CoreWidth;
    double Rate_P, Rate_Q, Rate_R;
    VORTEX_SHEET_ENTRY *VortexSheetList;

    // Initialize to free stream values

    for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {

       VortexSheet(m).ZeroEdgeVelocities();
 
       for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
        
          VortexSheet(m).TrailingVortexEdge(i).FreeStreamVelocity(0) = FreeStreamVelocity_[0];
          VortexSheet(m).TrailingVortexEdge(i).FreeStreamVelocity(1) = FreeStreamVelocity_[1];
          VortexSheet(m).TrailingVortexEdge(i).FreeStreamVelocity(2) = FreeStreamVelocity_[2];
             
          for ( j = 1 ; j <= VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
      
             VortexSheet(m).TrailingVortexEdge(i).U(j) = FreeStreamVelocity_[0];
             VortexSheet(m).TrailingVortexEdge(i).V(j) = FreeStreamVelocity_[1];
             VortexSheet(m).TrailingVortexEdge(i).W(j) = FreeStreamVelocity_[2];
             
          }
   
       }
       
    }
    
    // Rotational rates... note these rates are wrt to the body stability
    // axes... so we have to convert them to equivalent freestream velocities...
    // in the VSPAERO axes system with has X and Z pointing in the opposite
    // directions
    
    Rate_P = RotationalRate_[0];
    Rate_Q = RotationalRate_[1];
    Rate_R = RotationalRate_[2];

    // Add in rotational velocities

    for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     

       for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
         
          for ( j = 1 ; j <= VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {

             xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0] - Xcg();
             xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1] - Ycg();       
             xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2] - Zcg();

             // P - Roll
             
             VortexSheet(m).TrailingVortexEdge(i).U(j) += 0.;
             VortexSheet(m).TrailingVortexEdge(i).V(j) += -xyz[2] * Rate_P;
             VortexSheet(m).TrailingVortexEdge(i).W(j) += +xyz[1] * Rate_P;
              
             // Q - Pitch
      
             VortexSheet(m).TrailingVortexEdge(i).U(j) += -xyz[2] * Rate_Q;
             VortexSheet(m).TrailingVortexEdge(i).V(j) += 0.;
             VortexSheet(m).TrailingVortexEdge(i).W(j) += +xyz[0] * Rate_Q;
      
             // R - Yaw
             
             VortexSheet(m).TrailingVortexEdge(i).U(j) += -xyz[1] * Rate_R;
             VortexSheet(m).TrailingVortexEdge(i).V(j) += +xyz[0] * Rate_R;
             VortexSheet(m).TrailingVortexEdge(i).W(j) += 0.;
             
          }
          
       }
    
    }  
        
    // Add in the rotor induced velocities

    for ( k = 1 ; k <= NumberOfRotors_ ; k++ ) {
     
       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     

          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
            
             for ( j = 1 ; j <= VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
   
                xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
             
                RotorDisk(k).Velocity(xyz, q);                   
   
                VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
             
                // If there is ground effects, z plane...
                
                if ( DoGroundEffectsAnalysis() ) {
        
                   xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                   xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                   xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
                        
                   xyz[2] *= -1.;
                  
                   RotorDisk(k).Velocity(xyz, q);        
         
                   q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                   VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                   VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
                  
                }     
                                
                // If there is a symmetry plane, calculate influence of the reflection
                
                if ( DoSymmetryPlaneSolve_ ) {
        
                   xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                   xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                   xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
        
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
                  
                   RotorDisk(k).Velocity(xyz, q);        
         
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                   VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                   VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
                  
                   // If there is ground effects, z plane...
                   
                   if ( DoGroundEffectsAnalysis() ) {
   
                      xyz[2] *= -1.;
                     
                      RotorDisk(k).Velocity(xyz, q);        
            
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;            
                                                            q[2] *= -1.;
                     
                      VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                      VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                      VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
                     
                   }  
                                  
                }                
   
             }
             
          }    

       }
       
    }
         
    // Wing surface vortex induced velocities

    for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     

#pragma omp parallel for private(CoreWidth,j,xyz,q) schedule(dynamic)                                                
       for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
          
          CoreWidth = VortexSheet(m).CoreSize();
                         
          for ( j = 1 ; j <= VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
             
             xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
             xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
             xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
                
             CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
   
             VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
             VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
             VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
         
             // If there is ground effects, z plane ...
             
             if ( DoGroundEffectsAnalysis() ) {
     
                xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
                     
                xyz[2] *= -1.;
               
                CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
      
                q[2] *= -1.;
               
                VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
                
             }
                          
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {
     
                xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
             
                if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
               
                CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
               
                VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
                
                // If there is ground effects, z plane ...
                
                if ( DoGroundEffectsAnalysis() ) {
 
                   xyz[2] *= -1.;
                  
                   CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
         
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;         
                                                         q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                   VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                   VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
                   
                }
                             
             }
             
          }
          
       }
       
    }

    // Copy over vortex sheet data for parallel runs
   
    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {

       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
          VortexSheet_[cpu][k] += VortexSheet_[0][k];
    
       }  
       
    }   

    // Wake vortex to vortex interactions
  
    for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {

#pragma omp parallel for private(cpu,Level,w,t,i,j,NumberOfSheets,VortexSheetList,xyz,xyz_te,q,U,V,W) schedule(dynamic)                            
       for ( p = 1 ; p <= VortexSheetVortexToVortexSet_[v].NumberOfSets() ; p++ ) { 

#ifdef VSPAERO_OPENMP    
          cpu = omp_get_thread_num();
#else
          cpu = 0;
#endif          
          w = VortexSheetVortexToVortexSet_[v].VortexW(p);
          
          t = VortexSheetVortexToVortexSet_[v].TrailingVortexT(p);

          for ( i = 1 ; i <= VortexSheetVortexToVortexSet_[v].NumberOfVortexSheetInteractionEdges(p) ; i++ ) {

             Level           = VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p)[i].Level();
             
             j               = VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p)[i].Edge();
             
             NumberOfSheets  = VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p)[i].NumberOfVortexSheets();
             
             VortexSheetList = VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p)[i].VortexSheetList_;
 
             xyz[0] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[0];
             xyz[1] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[1];
             xyz[2] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[2];                

             xyz_te[0] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().x();
             xyz_te[1] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().y();
             xyz_te[2] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().z();

             VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, xyz_te, q);
             
             U = q[0];
             V = q[1];
             W = q[2];

             // If there is ground effects, z plane ...
   
             if ( DoGroundEffectsAnalysis() ) {

                xyz[0] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[0];
                xyz[1] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[1];
                xyz[2] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[2];                

                xyz_te[0] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().x();
                xyz_te[1] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().y();
                xyz_te[2] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().z();
                 
                xyz[2] *= -1.; xyz_te[2] *= -1.;
               
                VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, xyz_te, q);
       
                q[2] *= -1.;
               
                U += q[0];
                V += q[1];
                W += q[2];
               
             }  
                                
             // If there is a symmetry plane, calculate influence of the reflection
   
             if ( DoSymmetryPlaneSolve_ ) {

                xyz[0] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[0];
                xyz[1] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[1];
                xyz[2] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[2];                

                xyz_te[0] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().x();
                xyz_te[1] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().y();
                xyz_te[2] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().z();
                 
                if ( DoSymmetryPlaneSolve_ == SYM_X ) { xyz[0] *= -1.; xyz_te[0] *= -1.; };
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) { xyz[1] *= -1.; xyz_te[1] *= -1.; };
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) { xyz[2] *= -1.; xyz_te[2] *= -1.; };
               
                VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, xyz_te, q);
       
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
               
                U += q[0];
                V += q[1];
                W += q[2];
                
                // If there is ground effects, z plane ...
      
                if ( DoGroundEffectsAnalysis() ) {

                   xyz[2] *= -1.; xyz_te[2] *= -1.;
                  
                   VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, xyz_te, q);
          
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;                
                                                         q[2] *= -1.;
                  
                   U += q[0];
                   V += q[1];
                   W += q[2];
                  
                }                        
               
             }        
             
             VortexSheet(w).TrailingVortexEdge(t).U(Level,j) += U;
             VortexSheet(w).TrailingVortexEdge(t).V(Level,j) += V;
             VortexSheet(w).TrailingVortexEdge(t).W(Level,j) += W;                           
 
          }
          
       }

    }

    for ( w = 1 ; w <= NumberOfVortexSheets_ ; w++ ) {
      
       VortexSheet(w).ProlongateEdgeVelocities();
       
    }

    // Force last segment to free stream conditions
            
    for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     
           
       for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {

          j = VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() + 1;
   
          VortexSheet(m).TrailingVortexEdge(i).U(j) = FreeStreamVelocity_[0];
          VortexSheet(m).TrailingVortexEdge(i).V(j) = FreeStreamVelocity_[1];
          VortexSheet(m).TrailingVortexEdge(i).W(j) = FreeStreamVelocity_[2];
          
       }
       
    }       

    // For any trailing vortices leaving 'identical' trailing edges to have the same velocities
    
    if ( ModelType_ == VLM_MODEL ) {

       // Agglomerate velocities
       
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
          
          for ( k = 1 ; k <= VortexSheet(v).NumberOfCommonTE() ; k++ ) {
             
             w = VortexSheet(v).CommonTEList(k).Sheet_j();
             
             i = VortexSheet(v).CommonTEList(k).TEVortex_i();
             j = VortexSheet(v).CommonTEList(k).TEVortex_j();
             
             for ( p = 1 ; p <= VortexSheet(v).TrailingVortexEdge(i).NumberOfSubVortices() ; p++ ) {
                
                VortexSheet(v).TrailingVortexEdge(i).U(p) += VortexSheet(w).TrailingVortexEdge(j).U(p);
                VortexSheet(v).TrailingVortexEdge(i).V(p) += VortexSheet(w).TrailingVortexEdge(j).V(p);
                VortexSheet(v).TrailingVortexEdge(i).W(p) += VortexSheet(w).TrailingVortexEdge(j).W(p);
                
             }
             
          }
          
       }
       
       // Average velocities
       
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
          
          for ( k = 1 ; k <= VortexSheet(v).NumberOfCommonTE() ; k++ ) {
             
             w = VortexSheet(v).CommonTEList(k).Sheet_j();
             
             i = VortexSheet(v).CommonTEList(k).TEVortex_i();
             j = VortexSheet(v).CommonTEList(k).TEVortex_j();
             
             for ( p = 1 ; p <= VortexSheet(v).TrailingVortexEdge(i).NumberOfSubVortices() ; p++ ) {
                
                VortexSheet(v).TrailingVortexEdge(i).U(p) /= ( VortexSheet(v).NumberOfCommonNodesForTE(i) + 1);
                VortexSheet(v).TrailingVortexEdge(i).V(p) /= ( VortexSheet(v).NumberOfCommonNodesForTE(i) + 1);
                VortexSheet(v).TrailingVortexEdge(i).W(p) /= ( VortexSheet(v).NumberOfCommonNodesForTE(i) + 1);
                
             }
             
          }
          
       }       
  
       // Distribute velocities
       
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
          
          for ( k = 1 ; k <= VortexSheet(v).NumberOfCommonTE() ; k++ ) {
             
             w = VortexSheet(v).CommonTEList(k).Sheet_j();
             
             i = VortexSheet(v).CommonTEList(k).TEVortex_i();
             j = VortexSheet(v).CommonTEList(k).TEVortex_j();
             
             for ( p = 1 ; p <= VortexSheet(v).TrailingVortexEdge(i).NumberOfSubVortices() ; p++ ) {
                
                VortexSheet(w).TrailingVortexEdge(j).U(p) = VortexSheet(v).TrailingVortexEdge(i).U(p);
                VortexSheet(w).TrailingVortexEdge(j).V(p) = VortexSheet(v).TrailingVortexEdge(i).V(p);
                VortexSheet(w).TrailingVortexEdge(j).W(p) = VortexSheet(v).TrailingVortexEdge(i).W(p);
                 
             }
             
          }
          
       }  
              
    }
      
    // Now update the location of the wake
              
    MaxDelta = 0.;
              
    for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     

       if ( DoGroundEffectsAnalysis() ) VortexSheet(m).DoGroundEffectsAnalysis() = 1;

       Delta = VortexSheet(m).UpdateWakeLocation();
       
       MaxDelta = MAX(MaxDelta,Delta);

    }

    if ( Verbose_ ) printf("MaxDelta: %f \n",log10(MaxDelta)); 
     
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER SaveVortexState                             #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::SaveVortexState(void)
{
   
    int k;
   
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
       VortexSheet(k).SaveVortexState();
   
    }
   
}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER CalculateUnsteadyWakeVelocities                  #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateUnsteadyWakeVelocities(void)
{

    int i, k, v, cpu, NumberOfSheets, Level, Loop;
    double xyz[3], q[5], U, V, W;
    VORTEX_SHEET_ENTRY *VortexSheetList;

    // Update vortex strengths

    UpdateVortexEdgeStrengths(1, EXPLICIT_WAKE_GAMMAS);

    // Copy over vortex sheet data for parallel runs
   
    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {

       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
          VortexSheet_[cpu][k] += VortexSheet_[0][k];
    
       }  
       
    }   

    // Trailing vortex induced velocities

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       UnsteadyTrailingWakeVelocity_[i][0] = 0.;
       UnsteadyTrailingWakeVelocity_[i][1] = 0.;
       UnsteadyTrailingWakeVelocity_[i][2] = 0.;
             
    }
    
    ZeroLoopVelocities();

    for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {

       for ( i = 1 ; i <= NumberOfVortexSheetInteractionLoops_[v] ; i++ ) {
        
          cpu = 0;

          Level = VortexSheetInteractionLoopList_[v][i].Level();

          Loop  = VortexSheetInteractionLoopList_[v][i].Loop();
          
          NumberOfSheets = VortexSheetInteractionLoopList_[v][i].NumberOfVortexSheets();
          
          VortexSheetList = VortexSheetInteractionLoopList_[v][i].VortexSheetList_;
          
          VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, VSPGeom().Grid(Level).LoopList(Loop).xyz_c(), q);
          
          U = q[0];
          V = q[1];
          W = q[2];

          // If there is ground effects, z plane...
        
          if ( DoGroundEffectsAnalysis() ) {
           
             xyz[0] = VortexLoop(i).xyz_c()[0];
             xyz[1] = VortexLoop(i).xyz_c()[1];
             xyz[2] = VortexLoop(i).xyz_c()[2];
             
             xyz[2] *= -1.;
        
             VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, q);
             
             q[2] *= -1.;
             
             U += q[0];
             V += q[1];
             W += q[2];
             
          }   
          
          // If there is a symmetry plane, calculate influence of the reflection
        
          if ( DoSymmetryPlaneSolve_ ) {
           
             xyz[0] = VortexLoop(i).xyz_c()[0];
             xyz[1] = VortexLoop(i).xyz_c()[1];
             xyz[2] = VortexLoop(i).xyz_c()[2];
             
             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
             
             VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, q);
             
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
             
             U += q[0];
             V += q[1];
             W += q[2];
            
             // If there is ground effects, z plane...
           
             if ( DoGroundEffectsAnalysis() ) {
  
                xyz[2] *= -1.;
             
                VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, q);
    
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                      q[2] *= -1.;
             
                U += q[0];
                V += q[1];
                W += q[2];
             
             }   
         
          }   
          
          VSPGeom().Grid(Level).LoopList(Loop).U() += U;
          VSPGeom().Grid(Level).LoopList(Loop).V() += V;   
          VSPGeom().Grid(Level).LoopList(Loop).W() += W;          
          
       }
          
    }
    
    ProlongateUnsteadyVelocity();

    // Update vortex strengths
    
    UpdateVortexEdgeStrengths(1, ALL_WAKE_GAMMAS);
        
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateSurfaceMotion                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateSurfaceMotion(void)
{

    int i;
    double Hdot, RotVec[3];
    QUAT Quat, InvQuat, Vec1, Vec2;

    // Unsteady heaving analysis
    
    if ( TimeAnalysisType_ == HEAVE_ANALYSIS ) {
    
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
        
          Unsteady_H_ = Unsteady_HMax_ * sin ( Unsteady_AngleRate_*CurrentTime_ );     
          
          Hdot = Unsteady_HMax_ * cos ( Unsteady_AngleRate_*CurrentTime_) * Unsteady_AngleRate_;   
 
          // Calculate body velocities
          
          LocalBodySurfaceVelocity_[i][0] = 0.;
          LocalBodySurfaceVelocity_[i][1] = 0.;
          LocalBodySurfaceVelocity_[i][2] = Hdot;
        
       }
       
    }
    
    // Unsteady Pitch, Roll, and Yaw Analysis 
    
    else if ( TimeAnalysisType_ == P_ANALYSIS ||
              TimeAnalysisType_ == Q_ANALYSIS || 
              TimeAnalysisType_ == R_ANALYSIS ) {
    
       if ( TimeAnalysisType_ == P_ANALYSIS ) {
          
          RotVec[0] = -1.;
          RotVec[1] =  0.;
          RotVec[2] =  0.;

       }
       
       else if ( TimeAnalysisType_ == Q_ANALYSIS ) { 
          
          RotVec[0] =  0.;
          RotVec[1] =  1.;
          RotVec[2] =  0.;

       }          
             
       else if ( TimeAnalysisType_ == R_ANALYSIS ) { 
          
          RotVec[0] =  0.;
          RotVec[1] =  0.;
          RotVec[2] = -1.;
  
       }   

       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
          // Surface location at t - dt
//djk          

          Unsteady_Angle_ = AngleOfAttackZero_ + Unsteady_AngleMax_ * TORAD * sin ( Unsteady_AngleRate_*(CurrentTime_ - TimeStep_) - 0.5*PI);  

          Quat.FormRotationQuat(RotVec,Unsteady_Angle_);
   
          InvQuat = Quat;
   
          InvQuat.FormInverse();
       
          // Rotate

          Vec1(0) = VortexLoop(i).Xc() - Xcg();
          Vec1(1) = VortexLoop(i).Yc() - Ycg();
          Vec1(2) = VortexLoop(i).Zc() - Zcg();
   
          Vec1 = Quat * Vec1 * InvQuat;

          Vec1(0) += Xcg();
          Vec1(1) += Ycg();
          Vec1(2) += Zcg();
printf("AngleOfAttackZero_: %f \n",AngleOfAttackZero_);          
Unsteady_AngleRate_ = Unsteady_AngleMax_ = 0.;                  
          // Surface location at t
//djk          
          Unsteady_Angle_ = AngleOfAttackZero_ + Unsteady_AngleMax_ * TORAD * sin ( Unsteady_AngleRate_*CurrentTime_ - 0.5*PI);  
          
          Quat.FormRotationQuat(RotVec,Unsteady_Angle_);
   
          InvQuat = Quat;
   
          InvQuat.FormInverse();
       
          // Rotate

          Vec2(0) = VortexLoop(i).Xc() - Xcg();
          Vec2(1) = VortexLoop(i).Yc() - Ycg();
          Vec2(2) = VortexLoop(i).Zc() - Zcg();
   
          Vec2 = Quat * Vec2 * InvQuat;

          Vec2(0) += Xcg();
          Vec2(1) += Ycg();
          Vec2(2) += Zcg();

          // Calculate body velocities
          
          LocalBodySurfaceVelocity_[i][0] = ( Vec2(0) - Vec1(0) ) / TimeStep_;
          LocalBodySurfaceVelocity_[i][1] = ( Vec2(1) - Vec1(1) ) / TimeStep_;
          LocalBodySurfaceVelocity_[i][2] = ( Vec2(2) - Vec1(2) ) / TimeStep_;
                    
          // Angle at time t
//djk          
          Unsteady_Angle_ = AngleOfAttackZero_ + Unsteady_AngleMax_ * TORAD * sin ( Unsteady_AngleRate_*(CurrentTime_) - 0.5*PI);  
           
       }
       
    }
    
    else {
              
       // Do nothing... 
       
    }

}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER UpdateGeometryLocation                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::UpdateGeometryLocation(int DoStartUp)
{

    int i, j, k, m, c, p, v, w, t, NumberOfSheets, jMax, Level, *ComponentInThisGroup, cpu;
    double OVec[3], TVec[3], RVec[3], CoreWidth;
    double xyz[3], xyz_te[3], q[5], U, V, W;
    double TimeStep, CurrentTime;
    QUAT Quat, InvQuat, Vec1, Vec2, DQuatDt, Omega, BodyVelocity, WQuat;
    VORTEX_SHEET_ENTRY *VortexSheetList;

    // If a full run, calculate velocities on trailing wakes
    
    if ( !NoiseAnalysis_ && !DoStartUp ) {

       // Initialize to free stream values
   
       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {
   
          VortexSheet(m).ZeroEdgeVelocities();

          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
           
             VortexSheet(m).TrailingVortexEdge(i).FreeStreamVelocity(0) = FreeStreamVelocity_[0];
             VortexSheet(m).TrailingVortexEdge(i).FreeStreamVelocity(1) = FreeStreamVelocity_[1];
             VortexSheet(m).TrailingVortexEdge(i).FreeStreamVelocity(2) = FreeStreamVelocity_[2];
                
             for ( j = 1 ; j <= VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
         
                VortexSheet(m).TrailingVortexEdge(i).U(j) = FreeStreamVelocity_[0];
                VortexSheet(m).TrailingVortexEdge(i).V(j) = FreeStreamVelocity_[1];
                VortexSheet(m).TrailingVortexEdge(i).W(j) = FreeStreamVelocity_[2];
                
             }
      
          }
          
       }
       
       // Add in the rotor induced velocities
    
       for ( k = 1 ; k <= NumberOfRotors_ ; k++ ) {
        
          for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     
   
             for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
   
                jMax = VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices();
       
                if ( TimeAccurate_ ) {
                   
                   jMax = MIN(VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices(), WakeStartingTime_ + Time_ + 1);
      
                }
             
                for ( j = 1 ; j <= jMax ; j++ ) {
   
                   xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                   xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                   xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
                
                   RotorDisk(k).Velocity(xyz, q);                   
      
                   VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                   VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                   VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
            
                   // If there is ground effects, z plane...
                   
                   if ( DoGroundEffectsAnalysis() ) {
           
                      xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                      xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                      xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
                           
                      xyz[2] *= -1.;
                     
                      RotorDisk(k).Velocity(xyz, q);        
            
                      q[2] *= -1.;
                     
                      VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                      VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                      VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
                     
                   }     
                                   
                   // If there is a symmetry plane, calculate influence of the reflection
                   
                   if ( DoSymmetryPlaneSolve_ ) {
           
                      xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                      xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                      xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
           
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
                     
                      RotorDisk(k).Velocity(xyz, q);        
            
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
                     
                      VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                      VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                      VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
                     
                      // If there is ground effects, z plane...
                      
                      if ( DoGroundEffectsAnalysis() ) {
      
                         xyz[2] *= -1.;
                        
                         RotorDisk(k).Velocity(xyz, q);        
               
                         if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                         if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;            
                                                               q[2] *= -1.;
                        
                         VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                         VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                         VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
                        
                      }  
                                     
                   }                
      
                }
                
             }    
   
          }
          
       }
   
       // Wing and body surface vortex induced velocities

       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     

          CoreWidth = VortexSheet(m).CoreSize();

#pragma omp parallel for private(j,jMax,xyz,q) schedule(dynamic)                                                              
          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
             
             jMax = VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices();
    
             if ( TimeAccurate_ ) {
                
                jMax = MIN(VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices(), WakeStartingTime_ + Time_ + 1);
   
             }
   
             for ( j = 1 ; j <= jMax ; j++ ) {
          
                xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 

                CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
      
                VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
            
                // If there is ground effects, z plane ...
                
                if ( DoGroundEffectsAnalysis() ) {
        
                   xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                   xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                   xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
                        
                   xyz[2] *= -1.;
                  
                   CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
         
                   q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                   VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                   VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
                   
                }
                             
                // If there is a symmetry plane, calculate influence of the reflection
                
                if ( DoSymmetryPlaneSolve_ ) {
        
                   xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                   xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                   xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
                
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
                  
                   CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
         
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                   VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                   VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
                   
                   // If there is ground effects, z plane ...
                   
                   if ( DoGroundEffectsAnalysis() ) {
    
                      xyz[2] *= -1.;
                     
                      CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
            
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;         
                                                            q[2] *= -1.;
                     
                      VortexSheet(m).TrailingVortexEdge(i).U(j) += q[0];
                      VortexSheet(m).TrailingVortexEdge(i).V(j) += q[1];
                      VortexSheet(m).TrailingVortexEdge(i).W(j) += q[2];
                      
                   }
                                
                }
                
             }
             
          }
          
       }        
    
       // Copy over vortex sheet data for parallel runs

       for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
   
          for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
          
             VortexSheet_[cpu][k] += VortexSheet_[0][k];
       
          }  
          
       }   
 
       // Wake vortex to vortex interactions... only do this for truly time accurate solutions
       // ... for stab/control cases we ignore the wake-wake interactions for both speed
       // and stability since we are likely pushing the time step a bit.

       if ( TimeAccurate_ && TimeAnalysisType_ == 0 ) {
          
          for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
   
#pragma omp parallel for private(cpu,Level,w,t,i,j,NumberOfSheets,VortexSheetList,xyz,xyz_te,q,U,V,W) schedule(dynamic)                       
             for ( p = 1 ; p <= VortexSheetVortexToVortexSet_[v].NumberOfSets() ; p++ ) { 
   
#ifdef VSPAERO_OPENMP    
                cpu = omp_get_thread_num();
#else
                cpu = 0;
#endif    
                w = VortexSheetVortexToVortexSet_[v].VortexW(p);
                
                t = VortexSheetVortexToVortexSet_[v].TrailingVortexT(p);
                   
                xyz_te[0] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().x();
                xyz_te[1] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().y();
                xyz_te[2] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().z();
                   
                for ( i = 1 ; i <= VortexSheetVortexToVortexSet_[v].NumberOfVortexSheetInteractionEdges(p) ; i++ ) {
   
                   Level           = VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p)[i].Level();
                   
                   j               = VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p)[i].Edge();
                   
                   NumberOfSheets  = VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p)[i].NumberOfVortexSheets();
                   
                   VortexSheetList = VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p)[i].VortexSheetList_;
                   
                   xyz[0] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[0];
                   xyz[1] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[1];
                   xyz[2] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[2];                
      
                   VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, xyz_te, q);
                   
                   U = q[0];
                   V = q[1];
                   W = q[2];
   
                   // If there is ground effects, z plane ...
         
                   if ( DoGroundEffectsAnalysis() ) {
      
                      xyz[0] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[0];
                      xyz[1] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[1];
                      xyz[2] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[2];                
      
                      xyz_te[0] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().x();
                      xyz_te[1] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().y();
                      xyz_te[2] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().z();
                       
                      xyz[2] *= -1.; xyz_te[2] *= -1.;
                     
                      VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, xyz_te, q);
             
                      q[2] *= -1.;
                     
                      U += q[0];
                      V += q[1];
                      W += q[2];
                     
                   }  
                                      
                   // If there is a symmetry plane, calculate influence of the reflection
         
                   if ( DoSymmetryPlaneSolve_ ) {
      
                      xyz[0] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[0];
                      xyz[1] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[1];
                      xyz[2] = VortexSheet(w).TrailingVortexEdge(t).xyz_c(j)[2];                
      
                      xyz_te[0] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().x();
                      xyz_te[1] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().y();
                      xyz_te[2] = VortexSheet(w).TrailingVortexEdge(t).TE_Node().z();
                       
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) { xyz[0] *= -1.; xyz_te[0] *= -1.; };
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) { xyz[1] *= -1.; xyz_te[1] *= -1.; };
                      if ( DoSymmetryPlaneSolve_ == SYM_Z ) { xyz[2] *= -1.; xyz_te[2] *= -1.; };
                     
                      VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, xyz_te, q);
             
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
                     
                      U += q[0];
                      V += q[1];
                      W += q[2];
                      
                      // If there is ground effects, z plane ...
            
                      if ( DoGroundEffectsAnalysis() ) {
      
                         xyz[2] *= -1.; xyz_te[2] *= -1.;
                        
                         VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, xyz_te, q);
                
                         if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                         if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;                
                                                               q[2] *= -1.;
                        
                         U += q[0];
                         V += q[1];
                         W += q[2];
                        
                      }                        
                     
                   }                   
      
                   VortexSheet(w).TrailingVortexEdge(t).U(Level,j) += U;
                   VortexSheet(w).TrailingVortexEdge(t).V(Level,j) += V;
                   VortexSheet(w).TrailingVortexEdge(t).W(Level,j) += W;
          
                }
                
             }
   
          }
          
       }

       for ( w = 1 ; w <= NumberOfVortexSheets_ ; w++ ) {
       
          VortexSheet(w).ProlongateEdgeVelocities();
        
       }

       // Force last segments to free stream conditions
   
       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     
              
          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
   
             jMax = VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices();
    
             if ( TimeAccurate_ ) {
                
                jMax = MIN(VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices(), WakeStartingTime_ + Time_ + 2);
   
             }
             
             for ( j = jMax ; j <= VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
         
                VortexSheet(m).TrailingVortexEdge(i).U(j) = FreeStreamVelocity_[0];
                VortexSheet(m).TrailingVortexEdge(i).V(j) = FreeStreamVelocity_[1];
                VortexSheet(m).TrailingVortexEdge(i).W(j) = FreeStreamVelocity_[2];
                
             }
             
          }
          
       }  
       
    }
    
    // For any trailing vortices leaving 'identical' trailing edges to have the same velocities
    
    if ( ModelType_ == VLM_MODEL ) {

       // Agglomerate velocities
       
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
          
          for ( k = 1 ; k <= VortexSheet(v).NumberOfCommonTE() ; k++ ) {
             
             w = VortexSheet(v).CommonTEList(k).Sheet_j();
             
             i = VortexSheet(v).CommonTEList(k).TEVortex_i();
             j = VortexSheet(v).CommonTEList(k).TEVortex_j();
             
             for ( p = 1 ; p <= VortexSheet(v).TrailingVortexEdge(i).NumberOfSubVortices() ; p++ ) {
                
                VortexSheet(v).TrailingVortexEdge(i).U(p) += VortexSheet(w).TrailingVortexEdge(j).U(p);
                VortexSheet(v).TrailingVortexEdge(i).V(p) += VortexSheet(w).TrailingVortexEdge(j).V(p);
                VortexSheet(v).TrailingVortexEdge(i).W(p) += VortexSheet(w).TrailingVortexEdge(j).W(p);
                
             }
             
          }
          
       }
       
       // Average velocities
       
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
          
          for ( k = 1 ; k <= VortexSheet(v).NumberOfCommonTE() ; k++ ) {
             
             w = VortexSheet(v).CommonTEList(k).Sheet_j();
             
             i = VortexSheet(v).CommonTEList(k).TEVortex_i();
             j = VortexSheet(v).CommonTEList(k).TEVortex_j();
             
             for ( p = 1 ; p <= VortexSheet(v).TrailingVortexEdge(i).NumberOfSubVortices() ; p++ ) {
                
                VortexSheet(v).TrailingVortexEdge(i).U(p) /= ( VortexSheet(v).NumberOfCommonNodesForTE(i) + 1);
                VortexSheet(v).TrailingVortexEdge(i).V(p) /= ( VortexSheet(v).NumberOfCommonNodesForTE(i) + 1);
                VortexSheet(v).TrailingVortexEdge(i).W(p) /= ( VortexSheet(v).NumberOfCommonNodesForTE(i) + 1);
                
             }
             
          }
          
       }       
  
       // Distribute velocities
       
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
          
          for ( k = 1 ; k <= VortexSheet(v).NumberOfCommonTE() ; k++ ) {
             
             w = VortexSheet(v).CommonTEList(k).Sheet_j();
             
             i = VortexSheet(v).CommonTEList(k).TEVortex_i();
             j = VortexSheet(v).CommonTEList(k).TEVortex_j();
             
             for ( p = 1 ; p <= VortexSheet(v).TrailingVortexEdge(i).NumberOfSubVortices() ; p++ ) {
                
                VortexSheet(w).TrailingVortexEdge(j).U(p) = VortexSheet(v).TrailingVortexEdge(i).U(p);
                VortexSheet(w).TrailingVortexEdge(j).V(p) = VortexSheet(v).TrailingVortexEdge(i).V(p);
                VortexSheet(w).TrailingVortexEdge(j).W(p) = VortexSheet(v).TrailingVortexEdge(i).W(p);
                 
             }
             
          }
          
       }  
              
    }

    // Choose the time step...
    
    TimeStep = TimeStep_;
    
    CurrentTime = CurrentTime_;
    
    if ( NoiseAnalysis_ ) {
       
       TimeStep = NoiseTimeStep_;
       
       CurrentTime = CurrentNoiseTime_ + NoiseTimeShift_;       
    
    }
     
    // Loop over any unsteady component groups and update geometry
    
    ComponentInThisGroup = new int[VSPGeom().NumberOfComponents() + 1];

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       ComponentGroupList_[c].Update(TimeStep, CurrentTime);

       OVec[0] = ComponentGroupList_[c].OVec(0); // Rotation origin
       OVec[1] = ComponentGroupList_[c].OVec(1);
       OVec[2] = ComponentGroupList_[c].OVec(2);
       
       RVec[0] = ComponentGroupList_[c].RVec(0); // Rotation vector
       RVec[1] = ComponentGroupList_[c].RVec(1);
       RVec[2] = ComponentGroupList_[c].RVec(2);
       
       TVec[0] = ComponentGroupList_[c].TVec(0); // Translation vector
       TVec[1] = ComponentGroupList_[c].TVec(1);
       TVec[2] = ComponentGroupList_[c].TVec(2);
       
       Quat = ComponentGroupList_[c].Quat();
       
       InvQuat = ComponentGroupList_[c].InvQuat();
       
       WQuat = ComponentGroupList_[c].WQuat();
       
       zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());
       
       for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {

          ComponentInThisGroup[ComponentGroupList_[c].ComponentList(j)] = 1;
          
       }
                 
       // If component group is not fixed... update it's location
                 
       if ( !ComponentGroupList_[c].GeometryIsFixed() ) {
   
          // Calculate surface velocity
          
          for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
             
             if ( ComponentInThisGroup[VortexLoop(i).ComponentID()] ) {
 
                Vec1(0) = VortexLoop(i).Xc() - OVec[0];
                Vec1(1) = VortexLoop(i).Yc() - OVec[1];
                Vec1(2) = VortexLoop(i).Zc() - OVec[2];
            
                // Body point location after rotation
                
                Vec2 = Quat * Vec1 * InvQuat;
         
                // Body point velocity
                
                BodyVelocity = WQuat * Vec2;
           
                LocalBodySurfaceVelocity_[i][0] = BodyVelocity(0) + ComponentGroupList_[c].Velocity(0);
                LocalBodySurfaceVelocity_[i][1] = BodyVelocity(1) + ComponentGroupList_[c].Velocity(1);
                LocalBodySurfaceVelocity_[i][2] = BodyVelocity(2) + ComponentGroupList_[c].Velocity(2);
             
             }
      
          }
  
          if ( !DoStartUp ) {
             
             // Update grids
             
             for ( Level = 0 ; Level <= NumberOfMGLevels_ ; Level++ ) {
         
                VSPGeom().Grid(Level).UpdateGeometryLocation(TVec,OVec,Quat,InvQuat,ComponentInThisGroup);
         
             }      
   
             // Update acuator disk locations
          
             for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
              
                RotorDisk(i).UpdateGeometryLocation(TVec,OVec,Quat,InvQuat);
                
             }
             
             // Update the wire frame
             
             for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
            
                if ( ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()] ) VSPGeom().VSP_Surface(i).UpdateGeometryLocation(TVec,OVec,Quat,InvQuat);
                
             }
             
             // Update the least squares cofficients for the panel solver
             
             if ( ModelType_ == PANEL_MODEL ) {
                
                for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
                   
                   if ( ComponentInThisGroup[VortexLoop(i).ComponentID()] ) CalculateLeastSquaresCoefficients(i);
                   
                }
                
             }
             
          }
          
       }
      
       if ( !DoStartUp ) {
          
          // Update vortex sheet motion over this time step
      
          if ( !NoiseAnalysis_ ) {
             
             for ( i = 1 ; i <= NumberOfVortexSheets_ ; i++ ) {
         
                VortexSheet(i).UpdateGeometryLocation(TVec,OVec,Quat,InvQuat,ComponentInThisGroup);
         
             }     
             
          }
          
          // Update component group rotation location if there was a translation
          
          ComponentGroupList_[c].OVec(0) += ComponentGroupList_[c].TVec(0);
          ComponentGroupList_[c].OVec(1) += ComponentGroupList_[c].TVec(1);
          ComponentGroupList_[c].OVec(2) += ComponentGroupList_[c].TVec(2);
       
       }

    }

    delete [] ComponentInThisGroup;

    // Note to self... CG is currently fixed to the original location... 
    // so global moments would be affected if we really wanted the moments about some
    // point that moves with the geometry
    
    // Update CG ?  

}

/*##############################################################################
#                                                                              #
#                           VSP_SOLVER ResetGeometry                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ResetGeometry(void)
{

    int i, j, c, n, Level, *ComponentInThisGroup;
    double OVec[3], TVec[3], RVec[3];
    double TimeStep, CurrentTime;
    QUAT Quat, InvQuat, Vec1, Vec2, DQuatDt, Omega, BodyVelocity, WQuat;

    if ( !NoiseAnalysis_ ) {
       
       printf("The ResetGeometry routine only really works for noise analyses! \n");
       fflush(NULL);
       exit(1);
       
    }

    ComponentInThisGroup = new int[VSPGeom().NumberOfComponents() + 1];

    // Loop over any unsteady component groups and update geometry

    TimeStep = -NoiseTimeStep_;
       
    CurrentTime = CurrentNoiseTime_ + NoiseTimeShift_;       

    for ( n = 1 ; n <= NumberOfNoiseTimeSteps_ ; n++ ) {
  
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
   
          ComponentGroupList_[c].Update(TimeStep, CurrentTime);
   
          OVec[0] = ComponentGroupList_[c].OVec(0); // Rotation origin
          OVec[1] = ComponentGroupList_[c].OVec(1);
          OVec[2] = ComponentGroupList_[c].OVec(2);
          
          RVec[0] = ComponentGroupList_[c].RVec(0); // Rotation vector
          RVec[1] = ComponentGroupList_[c].RVec(1);
          RVec[2] = ComponentGroupList_[c].RVec(2);
          
          TVec[0] = ComponentGroupList_[c].TVec(0); // Translation vector
          TVec[1] = ComponentGroupList_[c].TVec(1);
          TVec[2] = ComponentGroupList_[c].TVec(2);
          
          Quat = ComponentGroupList_[c].Quat();
          
          InvQuat = ComponentGroupList_[c].InvQuat();
          
          WQuat = ComponentGroupList_[c].WQuat();
          
          zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());
          
          for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
   
             ComponentInThisGroup[ComponentGroupList_[c].ComponentList(j)] = 1;
             
          }
                    
          // If component group is not fixed... update it's location
                    
          if ( !ComponentGroupList_[c].GeometryIsFixed() ) {
         
             // Calculate surface velocity
             
             for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
                
                if ( ComponentInThisGroup[VortexLoop(i).ComponentID()] ) {
    
                   Vec1(0) = VortexLoop(i).Xc() - OVec[0];
                   Vec1(1) = VortexLoop(i).Yc() - OVec[1];
                   Vec1(2) = VortexLoop(i).Zc() - OVec[2];
               
                   // Body point location after rotation
                   
                   Vec2 = Quat * Vec1 * InvQuat;
            
                   // Body point velocity
                   
                   BodyVelocity = WQuat * Vec2;
              
                   LocalBodySurfaceVelocity_[i][0] = BodyVelocity(0) + ComponentGroupList_[c].Velocity(0);
                   LocalBodySurfaceVelocity_[i][1] = BodyVelocity(1) + ComponentGroupList_[c].Velocity(1);
                   LocalBodySurfaceVelocity_[i][2] = BodyVelocity(2) + ComponentGroupList_[c].Velocity(2);
                
                }
         
             }
   
             // Update grids
             
             for ( Level = 0 ; Level <= NumberOfMGLevels_ ; Level++ ) {
         
                VSPGeom().Grid(Level).UpdateGeometryLocation(TVec,OVec,Quat,InvQuat,ComponentInThisGroup);
         
             }      
   
             // Update acuator disk locations
          
             for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
              
                RotorDisk(i).UpdateGeometryLocation(TVec,OVec,Quat,InvQuat);
                
             }
             
             // Update the wire frame
             
             for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
            
                if ( ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()] ) VSPGeom().VSP_Surface(i).UpdateGeometryLocation(TVec,OVec,Quat,InvQuat);
                
             }
             
             // Update the least squares cofficients for the panel solver
             
             if ( ModelType_ == PANEL_MODEL ) {
                
                for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
                   
                   if ( ComponentInThisGroup[VortexLoop(i).ComponentID()] ) CalculateLeastSquaresCoefficients(i);
                   
                }
                
             }
   
          }
   
          // Update component group rotation location if there was a translation
          
          ComponentGroupList_[c].OVec(0) += ComponentGroupList_[c].TVec(0);
          ComponentGroupList_[c].OVec(1) += ComponentGroupList_[c].TVec(1);
          ComponentGroupList_[c].OVec(2) += ComponentGroupList_[c].TVec(2);
   
       }
       
       CurrentTime -= TimeStep;
    
    }
    
    delete [] ComponentInThisGroup;   
   
}

/*##############################################################################
#                                                                              #
#                           VSP_SOLVER Do_GMRES_Solve                          #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Do_GMRES_Solve(void)
{

    int i, Iters;
    double ResMax, ResRed, ResFin;

    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       GammaNM2(i) = GammaNM1(i);
     
       GammaNM1(i) = Gamma(i);
       
       Delta_[i] = 0.;
       
    }

    // Calculate the initial, preconditioned, residual

    CalculateResidual();
   
    DoMatrixPrecondition(Residual_);

    // Convergence criteria

    ResMax = 0.1*Vref_;
    ResRed = 0.1;
 
    // Use preconditioned GMRES to solve the linear system
     
    GMRES_Solver(NumberOfVortexLoops_+1,  // Number of Equations, 0 <= i < Neq
                 3,                       // Max number of outer iterations
                 500,                     // Max number of inner (restart) iterations
                 1,                       // Output flag, verbose = 0, or 1
                 Delta_,                  // Initial guess and solution vector
                 Residual_,               // Right hand side of Ax = b
                 ResMax,                  // Maximum error tolerance
                 ResRed,                  // Residual reduction factor
                 ResFin,                  // Final log10 of residual reduction   
                 Iters);                  // Final iteration count      

    // Update solution vector

    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

       Gamma(i) = GammaNM1(i) + Delta_[i];

    }

    // Update previous time steps for start up from free stream case
    
    if ( StartFromSteadyState_ && Time_ == 1 ) {
       
       for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
          GammaNM1(i) = GammaNM2(i) = Gamma(i);
          
       }
    
    }
    
    if ( Verbose_ ) {
       
       double KelvinSum = 0.;
   
       for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
          KelvinSum += Gamma(i);
   
       }
       
       printf("\n\nKelvin Sum: %f \n",KelvinSum);
       
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

    int i, k;
    double Dot;

    // VLM Model
   
    if ( ModelType_ == VLM_MODEL ) {
          
       MatrixMultiply(Gamma_[0], Residual_);

       for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
          Residual_[i] = RightHandSide_[i] - Residual_[i];

       }
    
       // Add in unsteady terms
    
       if ( TimeAccurate_ ) {

          // Time dependent wake terms
       
          for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

             Residual_[i] -= vector_dot(VortexLoop(i).Normal(), UnsteadyTrailingWakeVelocity_[i]);

          }
       
       }
      
       Dot = 0.;
    
       for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

          Dot += Residual_[i]*Residual_[i];
       
       }

       L2Residual_ = log10(sqrt(Dot/NumberOfVortexLoops_));
      
    }
   
    // Panel model
   
    else if ( ModelType_ == PANEL_MODEL ) {

       MatrixMultiply(Gamma_[0], Residual_);

       for ( i = 0 ; i <= NumberOfVortexLoops_  ; i++ ) {
    
          MatrixVecTemp_[i] = RightHandSide_[i] - Residual_[i];

       }     

       // Add in unsteady terms
    
       if ( TimeAccurate_ ) {

          // Time dependent wake terms
          
          for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
             MatrixVecTemp_[i] -= vector_dot(VortexLoop(i).Normal(), UnsteadyTrailingWakeVelocity_[i]);
          
          }                 
       
       }      
       
       Dot = 0.;
    
       for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

          Dot += MatrixVecTemp_[i]*MatrixVecTemp_[i];
        
       }
       
       L2Residual_ = log10(sqrt(Dot/NumberOfVortexLoops_));

       MatrixTransposeMultiply(MatrixVecTemp_, Residual_);
  
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
    double rho, rho_zero, rho_tol, *s, **v, *y, NowTime;
    
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

    rho_zero = rho;

    rho_tol = 0.;

    Done = 0;

    while ( Iter < IterMax && ( ( rho > rho_tol || rho > ErrorMax ) && !Done ) ) {

      // Matrix Multiplication

      DoPreconditionedMatrixMultiply(x,r);

      for ( i = 0; i < Neq; i++ ) {

        r[i] = RightHandSide[i] - r[i];
   
      }

      rho = sqrt(VectorDot(Neq,r,r));

      if ( Iter == 0 ) rho_zero = rho;

      if ( Iter == 0 ) rho_tol = rho * ErrorReduction;
    
      if ( Verbose && Iter == 0 && !TimeAccurate_ ) printf("Wake Iter: %5d / %-5d ... GMRES Iter: %5d ... Red: %10.5f / %-10.5f ...  Max: %10.5f / %-10.5f \r",CurrentWakeIteration_, WakeIterations_, 0,log10(rho/rho_zero),log10(ErrorReduction), log10(rho), log10(ErrorMax)); fflush(NULL);
      if ( Verbose && Iter == 0 &&  TimeAccurate_ ) printf("TStep: %-5d / %-5d ... Time: %10.5f ... GMRES Iter: %5d ... Red: %10.5f / %-10.5f ...  Max: %-10.5f / %10.5f \r",Time_,NumberOfTimeSteps_,CurrentTime_, 0,log10(rho/rho_zero),log10(ErrorReduction), log10(rho), log10(ErrorMax)); fflush(NULL);
          
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

            for ( i = 0; i < Neq; i++ ) {
 
               v[k+1][i] = v[k+1][i] - h[j][k] * v[j][i];
 
            }

         }

   
         h[k+1][k] = sqrt ( VectorDot( Neq, v[k+1], v[k+1] ) );
    
         if ( ( av + Epsilon * h[k+1][k] ) == av ) {
           
            for ( j = 0; j < k+1; j++ )  {
 
               Dot = VectorDot( Neq, v[k+1], v[j] );
  
               h[j][k] = h[j][k] + Dot;

               for ( i = 0; i < Neq; i++ ) {
  
                  v[k+1][i] = v[k+1][i] - Dot * v[j][i];

               }
 
            }

            h[k+1][k] = sqrt ( VectorDot( Neq, v[k+1], v[k+1] ) );

         }
     
         if ( h[k+1][k] != 0.0 ) {

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
         
         NowTime = myclock();
    
         if ( Verbose && !TimeAccurate_) printf("Wake Iter: %5d / %-5d ... GMRES Iter: %5d ... Red: %10.5f / %-10.5f ...  Max: %10.5f / %-10.5f  \r",CurrentWakeIteration_,WakeIterations_,TotalIterations,log10(rho/rho_zero),log10(ErrorReduction), log10(rho), log10(ErrorMax)); fflush(NULL);
         if ( Verbose &&  TimeAccurate_) printf("TStep: %5d / %-5d ... Time: %10.5f ... GMRES Iter: %5d ... Red: %10.5f / %-10.5f ...  Max: %10.5f / %-10.5f ... STime: %10.5f ... TotTime: %10.5f \r",Time_,NumberOfTimeSteps_,CurrentTime_,TotalIterations,log10(rho/rho_zero),log10(ErrorReduction), log10(rho), log10(ErrorMax), NowTime - StartSolveTime_, NowTime - StartTime_ ); fflush(NULL);

         if ( rho <= ErrorMax && rho <= rho_tol ) Done = 1;

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

    if ( Verbose && !TimeAccurate_) sprintf(ConvergenceLine_,"Wake Iter: %5d / %-5d ... GMRES Iter: %5d ... Red: %10.5f / %-10.5f ...  Max: %10.5f / %-10.5f",CurrentWakeIteration_,WakeIterations_,TotalIterations,log10(rho/rho_zero),log10(ErrorReduction), log10(rho), log10(ErrorMax)); fflush(NULL);
    if ( Verbose &&  TimeAccurate_) sprintf(ConvergenceLine_,"TStep: %5d / %-5d ... Time: %10.5f ... GMRES Iter: %5d ... Red: %10.5f / %-10.5f ...  Max: %10.5f / %-10.5f ... STime: %10.5f ... TotTime: %10.5f",Time_,NumberOfTimeSteps_,CurrentTime_,TotalIterations,log10(rho/rho_zero),log10(ErrorReduction), log10(rho), log10(ErrorMax), NowTime - StartSolveTime_, NowTime - StartTime_ ); fflush(NULL);

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
   
    // If a full run, calculate induced drag and surface velocities
    
    if ( !NoiseAnalysis_ ) {
         
       // Modified trailing edge induced drag calculation

       CalculateTrefftzForces();    

       // Calculate surface velocities
   
       CalculateVelocities();
       
    }

    // Calculate vorticity gradient
    
    if ( ModelType_ == PANEL_MODEL ) CalculateVorticityGradient();
    
    if ( TimeAccurate_ && !NoiseAnalysis_ ) CalculateUnsteadyForces();

    // Calculate forces by applying JK theorem to each edge

    if ( !NoiseAnalysis_ ) CalculateKuttaJukowskiForces();

    // Calculate Delta-Cps, or surface pressures
    
    if ( ModelType_ == VLM_MODEL ) CalculateDeltaCPs();
      
    if ( ModelType_ == PANEL_MODEL ) CalculateSurfacePressures();

    // Integrate forces and moments
    
    if ( TimeAccurate_ ) IntegrateForcesAndMoments(1);

    IntegrateForcesAndMoments(0);
 
    // Calculate 2D clmax limited forces and moments, as well as span wide
    // loading information
    
    if ( ModelType_ == VLM_MODEL ) {
       
       if ( TimeAccurate_ ) CalculateCLmaxLimitedForces(1);
     
       CalculateCLmaxLimitedForces(0);

    }

}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER ReCalculateForces                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ReCalculateForces(void)
{
   
    // Calculate 2D clmax limited forces and moments, as well as span wide
    // loading information... this routine is likely to be called after 
    // simply changing the value of ReCref_ - so user is really looking for
    // the change in viscous drag for the new value of ReCref_
    
    if ( ModelType_ == VLM_MODEL ) CalculateCLmaxLimitedForces(0);   
   
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateTrefftzForces                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateTrefftzForces(void)
{

    int j, p;
    double xyz[3], q[3], qtot[3];

    // Loop over vortex edges and calculate forces via K-J theorem, using only wake induced velocities applied at TE

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {

          // Trailing vortices induced velocities... shift the current bound vortex to the 
          // 'trailing edge' of the trailing vortex.
          
          qtot[0] = qtot[1] = qtot[2] = 0.;
   
          for ( p = 1 ; p <= NumberOfVortexSheets_ ; p++ ) {
   
             // Evaluation location
   
             xyz[0] = SurfaceVortexEdge(j).Xc();
             xyz[1] = SurfaceVortexEdge(j).Yc();
             xyz[2] = SurfaceVortexEdge(j).Zc();
   
             VortexSheet(0,p).InducedKuttaVelocity(xyz, q);
   
             qtot[0] += q[0];
             qtot[1] += q[1];
             qtot[2] += q[2];
   
             // If there is ground effects, z plane ...
             
             if ( DoGroundEffectsAnalysis() ) {
   
                xyz[0] = SurfaceVortexEdge(j).Xc();
                xyz[1] = SurfaceVortexEdge(j).Yc();
                xyz[2] = SurfaceVortexEdge(j).Zc();
   
                xyz[2] *= -1.;
               
                VortexSheet(0,p).InducedKuttaVelocity(xyz, q);
         
                q[2] *= -1.;
     
                qtot[0] += q[0];
                qtot[1] += q[1];
                qtot[2] += q[2];
               
             }
             
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {
   
                xyz[0] = SurfaceVortexEdge(j).Xc();
                xyz[1] = SurfaceVortexEdge(j).Yc();
                xyz[2] = SurfaceVortexEdge(j).Zc();
             
                if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
               
                VortexSheet(0,p).InducedKuttaVelocity(xyz, q);
         
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
     
                qtot[0] += q[0];
                qtot[1] += q[1];
                qtot[2] += q[2];
               
                // If there is ground effects, z plane ...
                
                if ( DoGroundEffectsAnalysis() ) {
   
                   xyz[2] *= -1.;
                  
                   VortexSheet(0,p).InducedKuttaVelocity(xyz, q);
            
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;         
                                                         q[2] *= -1.;
        
                   qtot[0] += q[0];
                   qtot[1] += q[1];
                   qtot[2] += q[2];
                  
                }
                 
             }
           
          }
          
          SurfaceVortexEdge(j).CalculateTrefftzForces(qtot);

       }
       
    }

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER CalculateKuttaJukowskiForces                  #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateKuttaJukowskiForces(void)
{

    int j, Loop1, Loop2;
    double Fx, Fy, Fz, ids1, ids2, wgt1, wgt2;

    // Loop over vortex edges and calculate forces via K-J theorem

#pragma omp parallel for private(Loop1, Loop2, Fx, Fy, Fz, ids1, ids2, wgt1, wgt2)
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       Loop1 = SurfaceVortexEdge(j).VortexLoop1();
       Loop2 = SurfaceVortexEdge(j).VortexLoop2(); 
       
       if ( Loop1 == 0 ) {

          if ( ModelType_ == VLM_MODEL ) SurfaceVortexEdge(j).CalculateForces(VortexLoop(Loop2));

       }
       
       else if ( Loop2 == 0 ) {
          
          if ( ModelType_ == VLM_MODEL ) SurfaceVortexEdge(j).CalculateForces(VortexLoop(Loop1));

       }
       
       else {
          
          ids1 = 1./( pow(SurfaceVortexEdge(j).Xc() - VortexLoop(Loop1).Xc(),2.)
                    + pow(SurfaceVortexEdge(j).Yc() - VortexLoop(Loop1).Yc(),2.)
                    + pow(SurfaceVortexEdge(j).Zc() - VortexLoop(Loop1).Zc(),2.) );
          
          ids2 = 1./( pow(SurfaceVortexEdge(j).Xc() - VortexLoop(Loop2).Xc(),2.)
                    + pow(SurfaceVortexEdge(j).Yc() - VortexLoop(Loop2).Yc(),2.)
                    + pow(SurfaceVortexEdge(j).Zc() - VortexLoop(Loop2).Zc(),2.) );
  
          wgt1 = ids1 / (ids1 + ids2);
          wgt2 = 1. - wgt1;
      
          Fx = Fy = Fz = 0.; 
   
          SurfaceVortexEdge(j).CalculateForces(VortexLoop(Loop1));
          
          Fx += wgt1*SurfaceVortexEdge(j).Fx();
          Fy += wgt1*SurfaceVortexEdge(j).Fy();
          Fz += wgt1*SurfaceVortexEdge(j).Fz();
          
          SurfaceVortexEdge(j).CalculateForces(VortexLoop(Loop2));
          
          Fx += wgt2*SurfaceVortexEdge(j).Fx();
          Fy += wgt2*SurfaceVortexEdge(j).Fy();
          Fz += wgt2*SurfaceVortexEdge(j).Fz();
                  
          // Edge forces
          
          SurfaceVortexEdge(j).Fx() = Fx;
          SurfaceVortexEdge(j).Fy() = Fy;
          SurfaceVortexEdge(j).Fz() = Fz;
 
       }
               
    }
                 
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER CalculateUnsteadyForces                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateUnsteadyForces(void)
{

    int i, j, Edge;
    double DeltaCp, DGammaDt, TimeStep;

    // Choose the time step...
    
    TimeStep = TimeStep_;

    if ( NoiseAnalysis_ ) TimeStep = NoiseTimeStep_;

      
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
     
       SurfaceVortexEdge(j).Unsteady_Fx() = 0.;
       SurfaceVortexEdge(j).Unsteady_Fy() = 0.;
       SurfaceVortexEdge(j).Unsteady_Fz() = 0.;
 
    }
    
#pragma omp parallel for private(DeltaCp,DGammaDt,Edge,j)             
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

    //   DGammaDt = ( 3.*Gamma(i) - 4.*GammaNM1(i) + GammaNM2(i) ) / (2.*TimeStep);
    //   DGammaDt = ( Gamma(i) - GammaNM1(i) ) / (TimeStep);
    
         DGammaDt = ( Gamma(i) - GammaNM2(i) ) / (2.*TimeStep);

       DeltaCp = -DGammaDt / ( Vref_ * Vref_ );

       VortexLoop(i).dCp_Unsteady() = DeltaCp;

       for ( j = 1 ; j <= VortexLoop(i).NumberOfEdges() ; j++ ) {
          
          Edge = VortexLoop(i).Edge(j);
          
          SurfaceVortexEdge(Edge).Unsteady_Fx() -= DeltaCp * VortexLoop(i).Area() * VortexLoop(i).Nx()/VortexLoop(i).NumberOfEdges();
          SurfaceVortexEdge(Edge).Unsteady_Fy() -= DeltaCp * VortexLoop(i).Area() * VortexLoop(i).Ny()/VortexLoop(i).NumberOfEdges();
          SurfaceVortexEdge(Edge).Unsteady_Fz() -= DeltaCp * VortexLoop(i).Area() * VortexLoop(i).Nz()/VortexLoop(i).NumberOfEdges();
          
       }
       
    }
                 
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER CalculateDeltaCPs                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateDeltaCPs(void)
{

    int i, j, Loop1, Loop2;
    double Fx, Fy, Fz, Wgt1, Wgt2;

    // Loop over vortex edges and calculate forces via K-J theorem
 
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
       VortexLoop(i).Fx() = 0.;
       VortexLoop(i).Fy() = 0.;
       VortexLoop(i).Fz() = 0.;

    }
     
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       Loop1 = SurfaceVortexEdge(j).VortexLoop1();
       Loop2 = SurfaceVortexEdge(j).VortexLoop2();

       // Edge forces

       Fx = SurfaceVortexEdge(j).Fx() + SurfaceVortexEdge(j).Unsteady_Fx();
       Fy = SurfaceVortexEdge(j).Fy() + SurfaceVortexEdge(j).Unsteady_Fy();
       Fz = SurfaceVortexEdge(j).Fz() + SurfaceVortexEdge(j).Unsteady_Fz();
          
       // Loop level forces
       
       if ( ( Loop1 != 0 && Loop2 != 0 ) || SurfaceVortexEdge(j).IsLeadingEdge() ) {

          Wgt1 = VortexLoop(Loop1).Area()/(VortexLoop(Loop1).Area() + VortexLoop(Loop2).Area());    

          Wgt2 = 1 - Wgt1;
                  
          VortexLoop(Loop1).Fx() += Wgt1*Fx;
          VortexLoop(Loop1).Fy() += Wgt1*Fy;
          VortexLoop(Loop1).Fz() += Wgt1*Fz;

          VortexLoop(Loop2).Fx() += Wgt2*Fx;
          VortexLoop(Loop2).Fy() += Wgt2*Fy;
          VortexLoop(Loop2).Fz() += Wgt2*Fz;
       
       }

    }
    
    // Calculate normal force on each vortex loop

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
       VortexLoop(i).NormalForce() = -VortexLoop(i).Fx() * VortexLoop(i).Nx()
                                   + -VortexLoop(i).Fy() * VortexLoop(i).Ny()
                                   + -VortexLoop(i).Fz() * VortexLoop(i).Nz();
                                   
       VortexLoop(i).dCp() = VortexLoop(i).NormalForce() / VortexLoop(i).Area();      
              
       VortexLoop(i).dCp() /= Vref_*Vref_;

    }  
   
}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER CalculateSurfacePressures                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateSurfacePressures(void)
{

    int i, j, k, Loop1, Loop2, Edge, Node, Hits, *OnBoundary, BoundaryLoop;
    double Dot, KTFact, Normal[3], Area1, Area2, wgt1, wgt2;
    double Cp, CpCrit, LocalMach, KTMach;
    double *NodalCp, *NodalArea, Area, NewCp, Relax;      
    
    // Add in vorticity gradient and zero out any residual normal component
        
    for ( Loop1 = 1 ; Loop1 <= NumberOfVortexLoops_ ; Loop1++ ) {
       
       VortexLoop(Loop1).U() -= 0.5*VorticityGradient_[Loop1].dv_dx();
       VortexLoop(Loop1).V() -= 0.5*VorticityGradient_[Loop1].dv_dy();
       VortexLoop(Loop1).W() -= 0.5*VorticityGradient_[Loop1].dv_dz();

       Normal[0] = VortexLoop(Loop1).Nx();
       Normal[1] = VortexLoop(Loop1).Ny();
       Normal[2] = VortexLoop(Loop1).Nz();
         
       Dot = vector_dot(Normal, VortexLoop(Loop1).Velocity());

       VortexLoop(Loop1).U() -= Dot * Normal[0];
       VortexLoop(Loop1).V() -= Dot * Normal[1];
       VortexLoop(Loop1).W() -= Dot * Normal[2];
      
    }       
        
    // Calculate Cp
         
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
 
       VortexLoop(i).dCp() =  ( pow(VortexLoop(i).U(),2.)
                              + pow(VortexLoop(i).V(),2.)
                              + pow(VortexLoop(i).W(),2.) ) / SQR(Vref_);

       if ( VortexLoop(i).dCp() > QMax_*QMax_*SQR(Vref_) ) VortexLoop(i).dCp() = QMax_*QMax_*SQR(Vref_);

       VortexLoop(i).dCp() = 1. - VortexLoop(i).dCp(); 

       // Limit Cp

       Cp = VortexLoop(i).dCp();

       LocalMach = Mach_;
       
       if ( Mach_ > 0. && Vinf_ > 0. ) {
          
          LocalMach = Mach_*VortexLoop(i).LocalFreeStreamVelocity(4)/Vinf_;
          
       }
       
       LocalMach = MIN(LocalMach, 0.80);
   
       CpCrit = -2.*(1.-LocalMach*LocalMach)/(LocalMach*LocalMach*(1.4+1.));
       
       CpCrit *= 2.5;

       VortexLoop(i).dCp() = MAX(Cp,CpCrit);     
                                     
    }

    // Clean up solution near intersections

    OnBoundary = new int[NumberOfSurfaceNodes_ + 1];
           
    zero_int_array(OnBoundary, NumberOfSurfaceNodes_);
           
    NodalCp    = new double[NumberOfSurfaceNodes_ + 1];       
    NodalArea  = new double[NumberOfSurfaceNodes_ + 1];
     
    zero_double_array(NodalCp, NumberOfSurfaceNodes_);
    zero_double_array(NodalArea, NumberOfSurfaceNodes_);

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       for ( j = 1 ; j <= VortexLoop(i).NumberOfNodes() ; j++ ) {
          
          Node = VortexLoop(i).Node(j);
       
          NodalCp[Node] += VortexLoop(i).Area() * VortexLoop(i).dCp();
          
          NodalArea[Node] += VortexLoop(i).Area();
          
       }
       
    }
    
    for ( i = 1 ; i <= NumberOfSurfaceNodes_ ; i++ ) {
       
       NodalCp[i] /= NodalArea[i];
       
       
    }
                  
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       for ( j = 1 ; j <= VortexLoop(i).NumberOfEdges() ; j++ ) {
      
          Edge =  VortexLoop(i).Edge(j);
          
          Loop1 = SurfaceVortexEdge(Edge).Loop1();
          Loop2 = SurfaceVortexEdge(Edge).Loop2();
       
          if ( VortexLoop(Loop1).SurfaceID() != VortexLoop(Loop2).SurfaceID() ) {
             
             OnBoundary[SurfaceVortexEdge(Edge).Node1()] = 1;
             OnBoundary[SurfaceVortexEdge(Edge).Node2()] = 1;
                           
          }
             
       }
       
    }
 
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       BoundaryLoop = 0;
       
       for ( j = 1 ; j <= VortexLoop(i).NumberOfNodes() ; j++ ) {
       
          if ( OnBoundary[VortexLoop(i).Node(j)] ) BoundaryLoop = 1;
             
       }
             
       if ( BoundaryLoop ) {
  
          NewCp = Area = 0.;
          
          Hits = 0;
          
          for ( j = 1 ; j <= VortexLoop(i).NumberOfNodes() ; j++ ) {
       
             Node = VortexLoop(i).Node(j);
          
             if ( !OnBoundary[Node] ) {
                
                NewCp += NodalArea[Node] * NodalCp[Node];
                
                Area += NodalArea[Node];
                
                Hits++;
                
             }
             
          }
     
          if ( Hits > 0 ) VortexLoop(i).dCp() = NewCp / Area;
       
       } 
 
    }
   
    delete [] NodalArea;

    delete [] OnBoundary;
    
    delete [] NodalCp;

    // Karman-Tsien correction
    
    KTResidual_[1] = 0.;
    
    Relax = 0.75;

    if ( KarmanTsienCorrection_ && Mach_ > 0. ) {
        
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
    
          // Cp
   
          Cp = VortexLoop(i).dCp();
   
          LocalMach = Mach_;
          
          if ( Mach_ > 0. && Vinf_ > 0. ) {
             
             LocalMach = Mach_*VortexLoop(i).LocalFreeStreamVelocity(4)/Vinf_;
             
          }
          
          LocalMach = MIN(LocalMach, MAX(Mach_,0.95));
      
          CpCrit = -2.*(1.-LocalMach*LocalMach)/(LocalMach*LocalMach*(1.4+1.));

          // Karman-Tsien correction
       
          KTMach = VortexLoop(i).KTFact()*LocalMach;
          
          KTFact = sqrt( (1.-LocalMach*LocalMach)/(1.-KTMach*KTMach) );
          
          Cp /= VortexLoop(i).KTFact();
    
          KTFact = 1. - 0.25*LocalMach*LocalMach*MAX(Cp,CpCrit);
          
          // Laitone KTFact = 1. - 0.5*(LocalMach*LocalMach*( 1. + 0.2*LocalMach*LocalMach )/sqrt(1.-LocalMach*LocalMach))*MAX(Cp,CpCrit);
                
          if ( KTFact > 0. ) {
          
             if ( KTFact*KTFact + Mach_*Mach_ - 1. > 0. ) {
                
                KTMach = sqrt( (KTFact*KTFact + LocalMach*LocalMach - 1.)/(KTFact*KTFact) );
             
                KTMach = MIN(0.95,KTMach);
      
                KTFact = KTMach/LocalMach;
                
             }
             
             else {
                
                KTMach = 0.;
                
                KTFact = KTMach/LocalMach;
                
             }
             
          }
          
          else {
             
             KTFact = 1.;
             
          }
    
          KTResidual_[1] += pow(KTFact-VortexLoop(i).KTFact(),2.);

          VortexLoop(i).KTFact() = (1.-Relax)*VortexLoop(i).KTFact() + Relax*KTFact;
          
       }
       
       // Calculate convergence of KT correction and apply to edges

       KTResidual_[1] /= NumberOfVortexLoops_;
  
       KTResidual_[1] = sqrt(KTResidual_[1]);
       
       if ( CurrentWakeIteration_ == 1 ) KTResidual_[0] = KTResidual_[1];
       
       KTResidual_[1] /= KTResidual_[0];
       
       KTResidual_[1] = log10(KTResidual_[1]);
       
       printf("%s ... KTRes: %10.5f \r",ConvergenceLine_,KTResidual_[1]);

       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
          
          if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {

             Loop1 = SurfaceVortexEdge(j).Loop1();
             Loop2 = SurfaceVortexEdge(j).Loop2();
             
             Area1 = VortexLoop(Loop1).Area();
             Area2 = VortexLoop(Loop2).Area();
             
             wgt2 = Area2 / ( Area1 + Area2 );
             wgt1 = 1. - wgt2;
             
             SurfaceVortexEdge(j).KTFact() = wgt1 * VortexLoop(Loop1).KTFact() + wgt2 * VortexLoop(Loop2).KTFact();
             
          }
          
       }

       RestrictKTFactorFromGrid(1);

    }
  
    // Add in delta Cp due to rotors, and unsteady correction

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
 
       // Rotor Delta Cp
    
       VortexLoop(i).dCp() += VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(3);
       
       // Unsteady correction
       
       if ( TimeAccurate_ ) {
          
          VortexLoop(i).dCp() += VortexLoop(i).dCp_Unsteady();
          
       }
        
    }
    
    // Enforce base pressures
  
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       if ( LoopIsOnBaseRegion_[i] ) VortexLoop(i).dCp() = CpBase_;

    }
            
}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER CreateVorticityGradientDataStructure                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateVorticityGradientDataStructure(void)
{

    int i, j, k, Loop, Loop1, Loop2, Loop3, StartLoop, Level;
    int *NeighborLoop, *NextLoop;
    int NumberOfLoops, NewNumberOfLoops, NumberOfEquations;
    int *NumberOfLoopsForNode, **NodeToLoopList, Node;

    printf("Starting... Creating vorticity gradient data structure .. \n"); fflush(NULL);

    // Create a node to loop tri list
    
    NumberOfLoopsForNode = new int[NumberOfSurfaceNodes_ + 1];
    
    NodeToLoopList = new int*[NumberOfSurfaceNodes_ + 1];
    
    zero_int_array(NumberOfLoopsForNode, NumberOfSurfaceNodes_);
    
    for ( Loop = 1 ; Loop <= NumberOfVortexLoops_ ; Loop++ ) {
       
       for ( j = 1 ; j <= VortexLoop(Loop).NumberOfNodes() ; j++ ) {
          
          Node = VortexLoop(Loop).Node(j);
          
          NumberOfLoopsForNode[Node]++;
          
       }
       
    }
    
    for ( i = 1 ; i <= NumberOfSurfaceNodes_ ; i++ ) {
       
       NodeToLoopList[i] = new int[NumberOfLoopsForNode[i] + 1];
       
       
    }
    
    zero_int_array(NumberOfLoopsForNode, NumberOfSurfaceNodes_);
    
    for ( Loop = 1 ; Loop <= NumberOfVortexLoops_ ; Loop++ ) {
       
       for ( j = 1 ; j <= VortexLoop(Loop).NumberOfNodes() ; j++ ) {
          
          Node = VortexLoop(Loop).Node(j);
       
          NumberOfLoopsForNode[Node]++;
          
          NodeToLoopList[Node][NumberOfLoopsForNode[Node]] = Loop;
          
          
       }
       
    }    

    VorticityGradient_ = new GRADIENT[NumberOfVortexLoops_ + 1];
    
    NeighborLoop = new int[NumberOfVortexLoops_ + 1];

    NextLoop = new int[NumberOfVortexLoops_ + 1];
    
    zero_int_array(NextLoop, NumberOfVortexLoops_);

    for ( Loop1 = 1 ; Loop1 <= NumberOfVortexLoops_ ; Loop1++ ) {
       
       NumberOfLoops = 0;
       
       NeighborLoop[Loop1] = Loop1;
       
       NextLoop[++NumberOfLoops] = Loop1;
        
       NewNumberOfLoops = NumberOfLoops;

       // Buffer out 3 levels ish
       
       StartLoop = 1;
        
       for ( Level = 1 ; Level <= 3 ; Level++ ) {

          for ( k = StartLoop ; k <= NumberOfLoops ; k++ ) {
   
             Loop2 = NextLoop[k];
             
             // Add in neighboring loops
             
             for ( i = 1 ; i <= VortexLoop(Loop2).NumberOfNodes() ; i++ ) {
                
                Node = VortexLoop(Loop2).Node(i);
                
                for ( j = 1 ; j <= NumberOfLoopsForNode[Node] ; j++ ) {
                   
                   Loop3 = NodeToLoopList[Node][j];
                   
                   if ( NeighborLoop[Loop3] != Loop1 ) {
                      
                      NeighborLoop[Loop3] = Loop1;
                      
                      NextLoop[++NewNumberOfLoops] = Loop3;
                      
                   }
                   
                }
                
             }
             
          }
          
          StartLoop += NumberOfLoops;
          
          NumberOfLoops = NewNumberOfLoops;
          
       }
       
       NumberOfLoops--; // Don't count center loop in list
 
       NumberOfEquations = NumberOfLoops + 1;
       
       VorticityGradient_[Loop1].SizeList(NumberOfLoops, NumberOfEquations, 3);
      
       for ( k = 2 ; k <= NumberOfLoops ; k++ ) {
          
          VorticityGradient_[Loop1].LoopList(k) = Loop2 = NextLoop[k];
          
       }

       CalculateLeastSquaresCoefficients(Loop1); 

    }  

    delete [] NeighborLoop;

    for ( i = 1 ; i <= NumberOfSurfaceNodes_ ; i++ ) {
       
       if ( NumberOfLoopsForNode[i] > 0 ) delete [] NodeToLoopList[i];
       
    }    
    
    delete [] NodeToLoopList;
    delete [] NumberOfLoopsForNode;

    printf("Finished... Creating vorticity gradient data structure .. \n"); fflush(NULL);

}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER CreateVorticityGradientDataStructure                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateVorticityGradientDataStructureOld(void)
{

    int i, k, Loop1, Loop2, Loop3, Next, Edge, IsNeighbor, Level;
    int *NodeIsMarked, *NeighborLoop, *NextLoop, *TouchedLoop;
    int NumberOfLoops, NumberOfEquations;

    printf("Starting... Creating vorticity gradient data structure .. \n"); fflush(NULL);

    VorticityGradient_ = new GRADIENT[NumberOfVortexLoops_ + 1];
    
    NodeIsMarked = new int[NumberOfSurfaceNodes_ + 1];

    NeighborLoop = new int[NumberOfVortexLoops_ + 1];
    
    TouchedLoop = new int[NumberOfVortexLoops_ + 1];
    
    NextLoop = new int[NumberOfVortexLoops_ + 1];
    
    zero_int_array(NextLoop, NumberOfVortexLoops_);

    for ( Loop1 = 1 ; Loop1 <= NumberOfVortexLoops_ ; Loop1++ ) {
       
       zero_int_array(NodeIsMarked, NumberOfSurfaceNodes_);
       
       zero_int_array(NeighborLoop, NumberOfVortexLoops_);
       
       zero_int_array(TouchedLoop, NumberOfVortexLoops_);
    
       TouchedLoop[Loop1] = 1;
       
       for ( i = 1 ; i <= VortexLoop(Loop1).NumberOfNodes() ; i++ ) {
        
          NodeIsMarked[VortexLoop(Loop1).Node(i)] = 1;
          
       }
    
       // Find first level of loops
        
       // Insert neighbor loops
       
       NumberOfLoops = 0;
              
       for ( i = 1 ; i <= VortexLoop(Loop1).NumberOfEdges() ; i++ ) {
          
          Edge = VortexLoop(Loop1).Edge(i);
          
          Loop2 = SurfaceVortexEdge(Edge).Loop1() + SurfaceVortexEdge(Edge).Loop2() - Loop1;

          NextLoop[++NumberOfLoops] = Loop2;

       }
        
       for ( Level = 1 ; Level <= 3 ; Level++ ) {

          Next = 1;

          while ( Next <= NumberOfLoops ) {
   
             Loop2 = NextLoop[Next];
             
             if ( Loop2 != Loop1 && NeighborLoop[Loop2] == 0 ) {
                       
                // Check if loop shares any nodes with Loop1
                
                IsNeighbor = 0;
                
                for ( i = 1 ; i <= VortexLoop(Loop2).NumberOfNodes() ; i++ ) {
                   
                   if ( NodeIsMarked[VortexLoop(Loop2).Node(i)] == Level ) IsNeighbor = 1;
                   
                }
                
                // If it shares at least one node then look at it's neighbors
               
                if ( IsNeighbor && TouchedLoop[Loop2] == 0 ) {
                  
                   // Add this loop to the neighbor list
                          
                   NeighborLoop[Loop2] = 1;
                   
                   TouchedLoop[Loop2] = 1;
                   
                   // Mark any new nodes on this loop as Level + 1
                   
                   for ( i = 1 ; i <= VortexLoop(Loop2).NumberOfNodes() ; i++ ) {
                   
                      if ( NodeIsMarked[VortexLoop(Loop2).Node(i)] == 0 ) NodeIsMarked[VortexLoop(Loop2).Node(i)] = Level + 1;
       
                   }
                
                   // Now add in Loop2's neighbors that are new
                      
                   for ( i = 1 ; i <= VortexLoop(Loop2).NumberOfEdges() ; i++ ) {
                   
                      Edge = VortexLoop(Loop2).Edge(i);
                      
                      Loop3 = SurfaceVortexEdge(Edge).Loop1() + SurfaceVortexEdge(Edge).Loop2() - Loop2;
            
                      if ( NeighborLoop[Loop3] == 0 && TouchedLoop[Loop3] == 0 ) {
                
                         NextLoop[++NumberOfLoops] = Loop3;                  
                         
                      }
                   
                   }
     
                }
                
             }
             
             Next++;
             
          }
          
       }

       NumberOfLoops = 0;
       
       for ( Loop2 = 1 ; Loop2 <= NumberOfVortexLoops_ ; Loop2++ ) {
   
          if ( NeighborLoop[Loop2] ) NumberOfLoops++;
          
       }

       NumberOfEquations = NumberOfLoops + 1;
       
       VorticityGradient_[Loop1].SizeList(NumberOfLoops, NumberOfEquations, 3);
      
       k = 0;
          
       for ( Loop2 = 1 ; Loop2 <= NumberOfVortexLoops_ ; Loop2++ ) {
          
          if ( NeighborLoop[Loop2] ) {
             
             k++;
             
             VorticityGradient_[Loop1].LoopList(k) = Loop2;
  
          }  

       }
       
       CalculateLeastSquaresCoefficients(Loop1); 

    }  

    delete [] NodeIsMarked;
    delete [] NeighborLoop;
    delete [] TouchedLoop;

    printf("Finished ... Creating vorticity gradient data structure .. \n"); fflush(NULL);

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER CalculateLeastSquaresCoefficients                  #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateLeastSquaresCoefficients(int Loop1)
{
   
    int k, Loop2, NumberOfEquations;
    double  Wgt, dx, dy, dz, Area1, Area2;
    
    MATRIX *A;
    
    NumberOfEquations = VorticityGradient_[Loop1].NumberOfEquations();
    
    A = new MATRIX;
     
    // Linear fit

    A->size(NumberOfEquations,3);

    for ( k = 1 ; k <= VorticityGradient_[Loop1].NumberOfLoops() ; k++ ) {
          
       Loop2 = VorticityGradient_[Loop1].LoopList(k);

       dx = VortexLoop(Loop2).Xc() - VortexLoop(Loop1).Xc();
       dy = VortexLoop(Loop2).Yc() - VortexLoop(Loop1).Yc();
       dz = VortexLoop(Loop2).Zc() - VortexLoop(Loop1).Zc();
         
       Area1 = VortexLoop(Loop1).Area();
       
       Area2 = VortexLoop(Loop2).Area();
       
       // Weight is proportional to dot product of normals... 

       Wgt = MAX(vector_dot(VortexLoop(Loop1).Normal(), VortexLoop(Loop2).Normal()),0.000001);

       // Weight is proportional to tri area
       
       Wgt *= MIN(pow(Area2/Area1,2.),1.);
       
       // Weight is inversely proportional to distance
       
       Wgt *= 1./sqrt(dx*dx + dy*dy + dz*dz);             

       (*A)(k,1) = dx*Wgt;
       (*A)(k,2) = dy*Wgt;
       (*A)(k,3) = dz*Wgt;

    }

    
    (*A)(NumberOfEquations,1) = VortexLoop(Loop1).Nx();
    (*A)(NumberOfEquations,2) = VortexLoop(Loop1).Ny();
    (*A)(NumberOfEquations,3) = VortexLoop(Loop1).Nz();

    VorticityGradient_[Loop1].At() = (*A).transpose();

    VorticityGradient_[Loop1].AA() = VorticityGradient_[Loop1].At() * (*A);
    
    VorticityGradient_[Loop1].LU();
    
    delete A; 
  
}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER CalculateVorticityGradient                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateVorticityGradient(void)
{

    int i, j, k, Loop, Loop1, Loop2, Edge, Node1, Node2, *FixedNode;
    int Iter, Done, NodeHits;
    double Wgt, Area1, Area2, dx, dy, dz;
    double Fact, *dV, *Denom, *Res, *Dif, *Sum, ResMax, ResMax0, Delta, Eps, Wgt1, Wgt2, dVAvg;
    
    for ( Loop1 = 1 ; Loop1 <= NumberOfVortexLoops_ ; Loop1++ ) {
       
       for ( k = 1 ; k <= VorticityGradient_[Loop1].NumberOfLoops() ; k++ ) {
          
          Loop2 = VorticityGradient_[Loop1].LoopList(k);

          Area1 = VortexLoop(Loop1).Area();
          
          Area2 = VortexLoop(Loop2).Area();
          
          dx = VortexLoop(Loop2).Xc() - VortexLoop(Loop1).Xc();
          dy = VortexLoop(Loop2).Yc() - VortexLoop(Loop1).Yc();
          dz = VortexLoop(Loop2).Zc() - VortexLoop(Loop1).Zc();          

          Wgt = MAX(vector_dot(VortexLoop(Loop1).Normal(), VortexLoop(Loop2).Normal()),0.000001);

          Wgt *= MIN(pow(Area2/Area1,2.),1.);

          Wgt *= 1./sqrt(dx*dx + dy*dy + dz*dz);    
             
          VorticityGradient_[Loop1].b(k) = ( VortexLoop(Loop2).Gamma() - VortexLoop(Loop1).Gamma() ) * Wgt;    

       }
       
       VorticityGradient_[Loop1].b(VorticityGradient_[Loop1].NumberOfEquations()) = 0.;
    
       VorticityGradient_[Loop1].Solve();
    
    }   
    
    // Zero out gradient at trailing edges

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {

          Loop1 = SurfaceVortexEdge(j).LoopL();
          Loop2 = SurfaceVortexEdge(j).LoopR();

          VorticityGradient_[Loop1].dv_dx() = 0.;
          VorticityGradient_[Loop1].dv_dy() = 0.;
          VorticityGradient_[Loop1].dv_dz() = 0.;
                                              
          VorticityGradient_[Loop2].dv_dx() = 0.;
          VorticityGradient_[Loop2].dv_dy() = 0.;
          VorticityGradient_[Loop2].dv_dz() = 0.;          

       }

    }   
    
    // Smooth gradient

    dV = new double[NumberOfSurfaceNodes_ + 1];
    
    Denom = new double[NumberOfSurfaceNodes_ + 1];
    
    FixedNode = new int[NumberOfSurfaceNodes_ + 1];
    
    Res = new double[NumberOfSurfaceNodes_ + 1];
    
    Dif = new double[NumberOfSurfaceNodes_ + 1];
    
    Sum = new double[NumberOfSurfaceNodes_ + 1];

    for ( i = 1 ; i <= 3 ; i++ ) {

       zero_double_array(dV, NumberOfSurfaceNodes_);

       zero_double_array(Denom, NumberOfSurfaceNodes_);

       for ( Loop = 1 ; Loop <= NumberOfVortexLoops_ ; Loop++ ) {
    
          for ( j = 1 ; j <= VortexLoop(Loop).NumberOfEdges() ; j++ ) {
             
             Edge = VortexLoop(Loop).Edge(j);
             
             Node1 = SurfaceVortexEdge(Edge).Node1();
             Node2 = SurfaceVortexEdge(Edge).Node2();
             
             if ( i == 1 ) {
                
                dV[Node1] += VorticityGradient_[Loop].dv_dx() * VortexLoop(Loop).Area();
                dV[Node2] += VorticityGradient_[Loop].dv_dx() * VortexLoop(Loop).Area();
                
             }
             
             else if ( i == 2 ) {
                
                dV[Node1] += VorticityGradient_[Loop].dv_dy() * VortexLoop(Loop).Area();
                dV[Node2] += VorticityGradient_[Loop].dv_dy() * VortexLoop(Loop).Area();
                
             }
             
             else {

                dV[Node1] += VorticityGradient_[Loop].dv_dz() * VortexLoop(Loop).Area();
                dV[Node2] += VorticityGradient_[Loop].dv_dz() * VortexLoop(Loop).Area();
                
             }                                   
             
             Denom[Node1] += VortexLoop(Loop).Area();
             Denom[Node2] += VortexLoop(Loop).Area();
             
          }
          
       }
       
       for ( j = 1 ; j <= NumberOfSurfaceNodes_ ; j++ ) {
          
          dV[j] /= Denom[j];
          
       }

       // Enforce kutta condition on trailing edge

       zero_int_array(FixedNode, NumberOfSurfaceNodes_);

       for ( j = 1 ; j <= VSPGeom().Grid(1).NumberOfKuttaNodes() ; j++ ) {

           dV[VSPGeom().Grid(1).KuttaNode(j)] = 0.;
           
           FixedNode[VSPGeom().Grid(1).KuttaNode(j)] = 1;
      
       }

       zero_double_array(Sum, NumberOfSurfaceNodes_);

       // Count edge hits per node
       
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
          // Edge to node pointers
       
          Node1 = SurfaceVortexEdge(j).Node1();
          Node2 = SurfaceVortexEdge(j).Node2();

          Sum[Node1] += 1.;
          Sum[Node2] += 1.;
         
       }

       // Loop over and smooth all residuals
       
       for ( j = 1 ; j <= NumberOfSurfaceNodes_ ; j++ ) {
       
          Res[j] = dV[j];
       
          Dif[j] = 0.;
       
       }

       // Do a few iterations of smoothing
       
       Iter = 1;
       
       Done = 0;
       
       while ( !Done && Iter <= 250 ) {
           
          ResMax = 0.;
       
          // Loop over the edges and scatter fluxes
       
          for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
           
              // Edge to node pointers
           
              Node1 = SurfaceVortexEdge(j).Node1();
              Node2 = SurfaceVortexEdge(j).Node2();

              Delta = Res[Node1] - Res[Node2];
       
              Dif[Node1] -= Delta;
              Dif[Node2] += Delta;
       
          }
       
          Eps = 0.5;
          
          for ( j = 1 ; j <= NumberOfSurfaceNodes_ ; j++ ) {
       
             if ( !FixedNode[j] ) {
       
                Fact = Eps*Sum[j];
                
                Delta = ( dV[j] + Fact*Res[j] + Eps*Dif[j] )/( 1. + Fact ) - Res[j];
                
                ResMax += Delta*Delta;
       
                Res[j] += Delta;
       
             }
       
             Dif[j] = 0.;
       
          }
          
          ResMax = sqrt(ResMax/NumberOfSurfaceNodes_);
          
          if ( Iter == 1 ) ResMax0 = ResMax;
          
          if ( ResMax0 != 0 ) {
             
             //printf("Iter: %d ... Vorticity Resmax: %f \n",Iter,log10(ResMax/ResMax0));
             
             if ( log10(ResMax/ResMax0) <= -2. ) Done = 1;
             
          }  
          
          Iter++;     
                 
       }

       // Update nodal values with smoothed result
       
       for ( j = 1 ; j <= NumberOfSurfaceNodes_ ; j++ ) {
      
          dV[j] = Res[j];
      
       }           
      
       // Finally, update loop values
      
       for ( Loop = 1 ; Loop <= NumberOfVortexLoops_ ; Loop++ ) {
          
          NodeHits = 0;
          
          dVAvg = 0.;
      
          for ( j = 1 ; j <= VortexLoop(Loop).NumberOfEdges() ; j++ ) {
       
             Edge = VortexLoop(Loop).Edge(j);
             
             Node1 = SurfaceVortexEdge(Edge).Node1();
             Node2 = SurfaceVortexEdge(Edge).Node2();
             
             Wgt1 = Wgt2 = 0.;

             if ( Sum[Node1] > 0. ) { Wgt1 = 1.; NodeHits++; }
             if ( Sum[Node2] > 0. ) { Wgt2 = 1.; NodeHits++; }

             dVAvg += Wgt1*dV[Node1] + Wgt2*dV[Node2];
             
          }

          if ( i == 1 && NodeHits > 0 ) VorticityGradient_[Loop].dv_dx() = dVAvg / NodeHits;
          if ( i == 2 && NodeHits > 0 ) VorticityGradient_[Loop].dv_dy() = dVAvg / NodeHits;
          if ( i == 3 && NodeHits > 0 ) VorticityGradient_[Loop].dv_dz() = dVAvg / NodeHits;
          
       }

    }
                
    delete [] dV;
    delete [] Denom;
    delete [] FixedNode;
    delete [] Res;
    delete [] Dif;
    delete [] Sum;

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER IntegrateForcesAndMoments                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::IntegrateForcesAndMoments(int UnsteadyEvaluation)
{

    int i, j, c, Loop1, Loop2, LoadCase, *ComponentInThisGroup;
    double Fx, Fy, Fz, Wgt1, Wgt2, LocalVel, LocalMach, LPGFact;
    double CA, SA, CB, SB;
    double SteadyComponent;
    double Cxi, Cyi, Czi, CDi;
    double Cx2, Cy2, Cz2, Cmx2, Cmy2, Cmz2;
    double ComponentCg[3];

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);

    SteadyComponent = 1.;
    
    if ( UnsteadyEvaluation == 1 ) SteadyComponent = 0.;

    // Zero out component group forces and moments
 
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       // Invisicid component
       
       ComponentGroupList_[c].Cx()  = 0.;
       ComponentGroupList_[c].Cy()  = 0.;
       ComponentGroupList_[c].Cz()  = 0.;
                                    
       ComponentGroupList_[c].Cmx() = 0.;
       ComponentGroupList_[c].Cmy() = 0.;
       ComponentGroupList_[c].Cmz() = 0.;

       ComponentGroupList_[c].CL()  = 0.;
       ComponentGroupList_[c].CD()  = 0.;
       ComponentGroupList_[c].CS()  = 0.;

       // Viscous component
       
       ComponentGroupList_[c].Cxo()  = 0.;
       ComponentGroupList_[c].Cyo()  = 0.;
       ComponentGroupList_[c].Czo()  = 0.;
                                    
       ComponentGroupList_[c].Cmxo() = 0.;
       ComponentGroupList_[c].Cmyo() = 0.;
       ComponentGroupList_[c].Cmzo() = 0.;

       ComponentGroupList_[c].CLo()  = 0.;
       ComponentGroupList_[c].CDo()  = 0.;
       ComponentGroupList_[c].CSo()  = 0.;
       
    }
     
    ComponentInThisGroup = new int[VSPGeom().NumberOfComponents() + 1];
    
    zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());     
           
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {  
       
       for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfComponents() ; i++ ) {
          
          ComponentInThisGroup[ComponentGroupList_[c].ComponentList(i)] = c;
          
       }
       
    }

    // Loop over vortex edges and integrate the forces / moments

    Cxi = Cyi = Czi = 0.;
    
    Cx2 = Cy2 = Cz2 = Cmx2 = Cmy2 = Cmz2 = 0.;
      
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       Loop1 = SurfaceVortexEdge(j).LoopL();
       Loop2 = SurfaceVortexEdge(j).LoopR();
       
       if ( Loop1 == 0 ) Loop1 = Loop2;
       if ( Loop2 == 0 ) Loop2 = Loop1;

       // Calculate local velocity
       
       Wgt1 = VortexLoop(Loop1).Area()/( VortexLoop(Loop1).Area() + VortexLoop(Loop2).Area() );
       
       Wgt2 = 1. - Wgt1;

       LocalVel = Wgt1*VortexLoop(Loop1).LocalFreeStreamVelocity(4) + Wgt2*VortexLoop(Loop2).LocalFreeStreamVelocity(4);

       // Trailing edge theorem, for induced drag
       
       if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {

          // Sum up forces from each trailing edge element, this includes the unsteady component from the unsteady wake...
       
          Fx = SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fx();
          Fy = SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fy();
          Fz = SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fz();
       
          Cxi += Fx;
          Cyi += Fy;
          Czi += Fz;

          // If this is an unsteady case, keep track of component group induced drag
      
          c = ComponentInThisGroup[SurfaceVortexEdge(j).ComponentID()];

          if ( c > 0) {
          
             ComponentGroupList_[c].CD() += ( Fx * CA + Fz * SA ) * CB - Fy * SB;

          }

       }

       if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {

          // Sum up forces and moments from each edge

          Fx = SteadyComponent*SurfaceVortexEdge(j).Fx() + SurfaceVortexEdge(j).Unsteady_Fx();
          Fy = SteadyComponent*SurfaceVortexEdge(j).Fy() + SurfaceVortexEdge(j).Unsteady_Fy();
          Fz = SteadyComponent*SurfaceVortexEdge(j).Fz() + SurfaceVortexEdge(j).Unsteady_Fz();

          // Apply local Prandtl Glauert correction
          
          LPGFact = 1.;
          
          if ( Mach_ < 1. ) {
             
             if ( Machref_ > 0. ) {
                
                LocalMach = Machref_*ABS(LocalVel)/Vref_;
                
                LocalMach = MIN(LocalMach,0.999);
                
                LPGFact = sqrt(1.-pow(Mach_,2.))/sqrt(1.-pow(LocalMach,2.));
                
             }
             
             else {
                
                LocalMach = Mach_*ABS(LocalVel)/Vref_;
                
                LocalMach = MIN(LocalMach,0.999);
                
                LPGFact = sqrt(1.-pow(Mach_,2.))/sqrt(1.-pow(LocalMach,2.));
                
             }
             
          }
   
          Fx *= LPGFact;     
          Fy *= LPGFact;     
          Fz *= LPGFact;     

          Cx2 += Fx;
          Cy2 += Fy;
          Cz2 += Fz;
          
          Cmx2 += Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
          Cmy2 += Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
          Cmz2 += Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw
          
          // If this is an unsteady case, keep track of component group forces and moments
    
          c = ComponentInThisGroup[SurfaceVortexEdge(j).ComponentID()];

          if ( c > 0) {
               
             ComponentGroupList_[c].Cx() += Fx;
             ComponentGroupList_[c].Cy() += Fy;
             ComponentGroupList_[c].Cz() += Fz;
             
             ComponentCg[0] = ComponentGroupList_[c].OVec(0);
             ComponentCg[1] = ComponentGroupList_[c].OVec(1);
             ComponentCg[2] = ComponentGroupList_[c].OVec(2);
          
             ComponentGroupList_[c].Cmx() += Fz * ( SurfaceVortexEdge(j).Yc() - ComponentCg[1] ) - Fy * ( SurfaceVortexEdge(j).Zc() - ComponentCg[2] );   // Roll
             ComponentGroupList_[c].Cmy() += Fx * ( SurfaceVortexEdge(j).Zc() - ComponentCg[2] ) - Fz * ( SurfaceVortexEdge(j).Xc() - ComponentCg[0] );   // Pitch
             ComponentGroupList_[c].Cmz() += Fy * ( SurfaceVortexEdge(j).Xc() - ComponentCg[0] ) - Fx * ( SurfaceVortexEdge(j).Yc() - ComponentCg[1] );   // Yaw

          }
                    
       }             
  
    }

    LoadCase = 0;
    
    if ( UnsteadyEvaluation ) LoadCase = 1;
    
    // Store and non-dimensionalize the forces and moments
    
    CFx_[LoadCase] = Cx2;
    CFy_[LoadCase] = Cy2;
    CFz_[LoadCase] = Cz2;
    
    CMx_[LoadCase] = Cmx2;
    CMy_[LoadCase] = Cmy2;
    CMz_[LoadCase] = Cmz2;

    // Calculate induced drag from the trailing edge analysis
    
    CDi = ( ( Cxi * CA + Czi * SA ) * CB - Cyi * SB);

    // Now calculate CL, CD, CS
 
    CL_[LoadCase] = (-CFx_[LoadCase] * SA + CFz_[LoadCase] * CA );
    CD_[LoadCase] = CDi;
    CS_[LoadCase] = ( CFx_[LoadCase] * CA + CFz_[LoadCase] * SA ) * SB + CFy_[LoadCase] * CB;

    // Non dimensonalize
    
     CDi           /= ( 0.5*Sref_*Vref_*Vref_ );
    
     CL_[LoadCase] /= (0.5*Sref_*Vref_*Vref_);   
     CD_[LoadCase] /= (0.5*Sref_*Vref_*Vref_);   
     CS_[LoadCase] /= (0.5*Sref_*Vref_*Vref_);   

    CFx_[LoadCase] /= (0.5*Sref_*Vref_*Vref_);
    CFy_[LoadCase] /= (0.5*Sref_*Vref_*Vref_);
    CFz_[LoadCase] /= (0.5*Sref_*Vref_*Vref_);
    
    CMx_[LoadCase] /= (0.5*Sref_*Bref_*Vref_*Vref_); // Roll
    CMy_[LoadCase] /= (0.5*Sref_*Cref_*Vref_*Vref_); // Pitch
    CMz_[LoadCase] /= (0.5*Sref_*Bref_*Vref_*Vref_); // Yaw

    // If flow is supersonic... use overall surface integration
    
    if ( Mach_ >= 1. ) CDi = ( ( Cx2 * CA + Cz2 * SA ) * CB - Cy2 * SB) / ( 0.5*Sref_*Vref_*Vref_ );

    // Adjust for symmetry
  
    if ( DoSymmetryPlaneSolve_ ) {
      
       CFx_[LoadCase] *= 2.;
       CFy_[LoadCase] *= 2.; 
       CFz_[LoadCase] *= 2.; 
   
       CMx_[LoadCase] *= 2.; 
       CMy_[LoadCase] *= 2.; 
       CMz_[LoadCase] *= 2.; 

        CL_[LoadCase] *= 2.; 
        CD_[LoadCase] *= 2.; 
        CS_[LoadCase] *= 2.; 
              
        CDi *= 2.;
       
    }
   
    if ( DoSymmetryPlaneSolve_ == SYM_X ) CMy_[LoadCase] = CMz_[LoadCase] = CFx_[LoadCase] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Y ) CMx_[LoadCase] = CMz_[LoadCase] = CFy_[LoadCase] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Z ) CMx_[LoadCase] = CMy_[LoadCase] = CFz_[LoadCase] = 0.;
    
    // Non-dimensionalize the component forces and moments

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       // Non-dimensionalize 

       ComponentGroupList_[c].Cx() /= 0.5*Sref_*Vref_*Vref_; 
       ComponentGroupList_[c].Cy() /= 0.5*Sref_*Vref_*Vref_; 
       ComponentGroupList_[c].Cz() /= 0.5*Sref_*Vref_*Vref_; 

       ComponentGroupList_[c].Cmx() /= 0.5*Bref_*Sref_*Vref_*Vref_;
       ComponentGroupList_[c].Cmy() /= 0.5*Cref_*Sref_*Vref_*Vref_;
       ComponentGroupList_[c].Cmz() /= 0.5*Bref_*Sref_*Vref_*Vref_;

       // Now calculate CL, CD, CS
      
       ComponentGroupList_[c].CL() = (-ComponentGroupList_[c].Cx() * SA + ComponentGroupList_[c].Cz() * CA );
       ComponentGroupList_[c].CS() = ( ComponentGroupList_[c].Cx() * CA + ComponentGroupList_[c].Cz() * SA ) * SB + ComponentGroupList_[c].Cy() * CB;

       ComponentGroupList_[c].CD() /= 0.5*Cref_*Sref_*Vref_*Vref_;
          
    }
    
    delete [] ComponentInThisGroup;

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER CalculateCLmaxLimitedForces                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateCLmaxLimitedForces(int UnsteadyEvaluation)
{

    int i, j, k, c, Loop, Loop1, Loop2, LoadCase, *ComponentInThisGroup;
    int NumberOfStations, SpanStation, SurfaceID; 
    double Fx, Fy, Fz, Fxi, Fyi, Fzi, Wgt;
    double Length, Re, Cf, Cdi, Cn, Cx, Cy, Cz;
    double Swet, StallFact;
    double CA, SA, CB, SB, Cmx, Cmy, Cmz, Cl_2d, dCD, CLv;
    double Mag, SteadyComponent, xyzLE[3], xyzTE[3], S[3];
    double ViscousForce, dF[3], dM[3], Dot, xyz_mid[3], ComponentCg[3];
    double Cl, Cd, Cs, Ct;
    double nvec[3], svec[3], Wgt1, Wgt2, LPGFact, LocalVel, LocalMach;
    double Thrust, Moment, Diameter, RPM, J, CT, CQ, CP, EtaP, CT_h, CQ_h, CP_h, FOM;

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
    
    SteadyComponent = 1.;
    
    if ( UnsteadyEvaluation == 1 ) SteadyComponent = 0.;
        
    // Zero out spanwise loading arrays
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
      
       NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
 
       zero_double_array(Span_Cx_[i],      NumberOfStations);
       zero_double_array(Span_Cy_[i],      NumberOfStations);
       zero_double_array(Span_Cz_[i],      NumberOfStations);       
                                          
       zero_double_array(Span_Cxo_[i],     NumberOfStations);
       zero_double_array(Span_Cyo_[i],     NumberOfStations);
       zero_double_array(Span_Czo_[i],     NumberOfStations);       
                                          
       zero_double_array(Span_Cxi_[i],     NumberOfStations);
       zero_double_array(Span_Cyi_[i],     NumberOfStations);
       zero_double_array(Span_Czi_[i],     NumberOfStations);     
                                          
       zero_double_array(Span_Cmxo_[i],    NumberOfStations);
       zero_double_array(Span_Cmyo_[i],    NumberOfStations);
       zero_double_array(Span_Cmzo_[i],    NumberOfStations);      
                                          
       zero_double_array(Span_Cmxi_[i],    NumberOfStations);
       zero_double_array(Span_Cmyi_[i],    NumberOfStations);
       zero_double_array(Span_Cmzi_[i],    NumberOfStations);      
                                          
       zero_double_array(Span_Cmx_[i],     NumberOfStations);
       zero_double_array(Span_Cmy_[i],     NumberOfStations);
       zero_double_array(Span_Cmz_[i],     NumberOfStations);      
                                          
       zero_double_array(Span_Cn_[i],      NumberOfStations);       
       zero_double_array(Span_Cl_[i],      NumberOfStations);
       zero_double_array(Span_Cd_[i],      NumberOfStations);
       zero_double_array(Span_Cs_[i],      NumberOfStations);       
                                          
       zero_double_array(Span_Yavg_[i],    NumberOfStations);       
       zero_double_array(Span_Area_[i],    NumberOfStations);
       
       zero_double_array(Local_Vel_[0][i], NumberOfStations);
       zero_double_array(Local_Vel_[1][i], NumberOfStations);
       zero_double_array(Local_Vel_[2][i], NumberOfStations);
       zero_double_array(Local_Vel_[3][i], NumberOfStations);
         
    }   
    
    // If this is an unsteady case, zero out any component group forces and moments
    
    if ( UnsteadyEvaluation == 0 ) {
        
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
   
          ComponentGroupList_[c].Cxo() = 0.;
          ComponentGroupList_[c].Cyo() = 0.;
          ComponentGroupList_[c].Czo() = 0.;
          
          ComponentGroupList_[c].Cx() = 0.;
          ComponentGroupList_[c].Cy() = 0.;
          ComponentGroupList_[c].Cz() = 0.;
   
          ComponentGroupList_[c].Cmxo() = 0.;
          ComponentGroupList_[c].Cmyo() = 0.;
          ComponentGroupList_[c].Cmzo() = 0.;
   
          ComponentGroupList_[c].Cmx() = 0.;
          ComponentGroupList_[c].Cmy() = 0.;
          ComponentGroupList_[c].Cmz() = 0.;
                            
       }
       
    }
    
    ComponentInThisGroup = new int[VSPGeom().NumberOfComponents() + 1];
    
    zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());     
           
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {  
       
       for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfComponents() ; i++ ) {
          
          ComponentInThisGroup[ComponentGroupList_[c].ComponentList(i)] = c;
          
       }
       
    }       

    // Loop over vortex edges and calculate forces via K-J theorem, using only wake induced velocities applied at TE

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       Loop1 = SurfaceVortexEdge(j).LoopL();
       Loop2 = SurfaceVortexEdge(j).LoopR();
                 
       if ( Loop1 == 0 ) Loop1 = Loop2;
       if ( Loop2 == 0 ) Loop2 = Loop1;
                 
       // Extract out forces

       Fx = Fy = Fz = Fxi = Fyi = Fzi = 0.;
       
       if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {

          Fxi = SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fx();
          Fyi = SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fy();
          Fzi = SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fz();
          
       }
       
       else {
          
          Fx  = SteadyComponent*SurfaceVortexEdge(j).Fx() + SurfaceVortexEdge(j).Unsteady_Fx();
          Fy  = SteadyComponent*SurfaceVortexEdge(j).Fy() + SurfaceVortexEdge(j).Unsteady_Fy();
          Fz  = SteadyComponent*SurfaceVortexEdge(j).Fz() + SurfaceVortexEdge(j).Unsteady_Fz();
          
       }
       
       // Calculate local velocity
       
       Wgt1 = VortexLoop(Loop1).Area()/( VortexLoop(Loop1).Area() + VortexLoop(Loop2).Area() );
       
       Wgt2 = 1. - Wgt1;

       LocalVel = Wgt1*VortexLoop(Loop1).LocalFreeStreamVelocity(4) + Wgt2*VortexLoop(Loop2).LocalFreeStreamVelocity(4);

       LPGFact = 1.;
       
       if ( Mach_ < 1. ) {
          
          if ( Machref_ > 0. ) {
             
             LocalMach = Machref_*ABS(LocalVel)/Vref_;
             
             LocalMach = MIN(LocalMach,0.999);
             
             LPGFact = sqrt(1.-pow(Mach_,2.))/sqrt(1.-pow(LocalMach,2.));
             
          }
          
          else {
             
             LocalMach = Mach_*ABS(LocalVel)/Vref_;
             
             LocalMach = MIN(LocalMach,0.999);
             
             LPGFact = sqrt(1.-pow(Mach_,2.))/sqrt(1.-pow(LocalMach,2.));
             
          }
          
       }

       Fx *= LPGFact;     
       Fy *= LPGFact;     
       Fz *= LPGFact;     
          
       // Sum up span wise loading

       for ( k = 1 ; k <= 2 ; k++ ) {
        
          if ( k == 1 ) Loop = SurfaceVortexEdge(j).Loop1();
          
          if ( k == 2 ) Loop = SurfaceVortexEdge(j).Loop2();
         
          Wgt = VortexLoop(Loop).Area() / ( VortexLoop(SurfaceVortexEdge(j).Loop1()).Area() + VortexLoop(SurfaceVortexEdge(j).Loop2()).Area() );

          // Wing Surface
          
          if ( VortexLoop(Loop).DegenWingID() > 0 ) {
             
             SurfaceID = VortexLoop(Loop).SurfaceID();
             
             SpanStation = VortexLoop(Loop).SpanStation();
             
             // Chordwise integrated forces

             Span_Cx_[SurfaceID][SpanStation] += Wgt*Fx;

             Span_Cy_[SurfaceID][SpanStation] += Wgt*Fy;

             Span_Cz_[SurfaceID][SpanStation] += Wgt*Fz;
                                            
             Span_Cn_[SurfaceID][SpanStation] += Wgt * Fx * VortexLoop(Loop).Nx()
                                               + Wgt * Fy * VortexLoop(Loop).Ny()
                                               + Wgt * Fz * VortexLoop(Loop).Nz();
                                                               
             // Chordwise integrated induced forces
             
             Span_Cxi_[SurfaceID][SpanStation] += Wgt*Fxi;

             Span_Cyi_[SurfaceID][SpanStation] += Wgt*Fyi;

             Span_Czi_[SurfaceID][SpanStation] += Wgt*Fzi;                

             // Chordwise integrated moments

             Span_Cmx_[SurfaceID][SpanStation] += Wgt * Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Wgt * Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
         
             Span_Cmy_[SurfaceID][SpanStation] += Wgt * Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Wgt * Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                   
             Span_Cmz_[SurfaceID][SpanStation] += Wgt * Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Wgt * Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw    

             
          }
          
          // Body Surface
          
          else {
         
             SurfaceID = VortexLoop(Loop).SurfaceID();
             
             SpanStation = 1;
        
             // Chordwise integrated forces
             
             if ( Mach_ < 1. ) {
                    
                Span_Cx_[SurfaceID][SpanStation] += Wgt*Fx;

                Span_Cy_[SurfaceID][SpanStation] += Wgt*Fy;

                Span_Cz_[SurfaceID][SpanStation] += Wgt*Fz;
                               
                Span_Cn_[SurfaceID][SpanStation] += Wgt * Fx * VortexLoop(Loop).Nx()
                                                  + Wgt * Fy * VortexLoop(Loop).Ny()
                                                  + Wgt * Fz * VortexLoop(Loop).Nz();
                                                  
                // Chordwise integrated induced forces
                
                Span_Cxi_[SurfaceID][SpanStation] += Wgt*Fxi;
   
                Span_Cyi_[SurfaceID][SpanStation] += Wgt*Fyi;
   
                Span_Czi_[SurfaceID][SpanStation] += Wgt*Fzi;                
                                                   
             }
             
             // Chordwise integrated moments
                    
             Span_Cmx_[SurfaceID][SpanStation] += Wgt * Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Wgt * Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
            
             Span_Cmy_[SurfaceID][SpanStation] += Wgt * Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Wgt * Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                      
             Span_Cmz_[SurfaceID][SpanStation] += Wgt * Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Wgt * Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw

          }
          
       }      

    }
    
    // Calculate span station areas and y values

    for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {

       if ( VortexLoop(j).DegenWingID() > 0 ) {
           
          SurfaceID = VortexLoop(j).SurfaceID();
                
          SpanStation = VortexLoop(j).SpanStation();
          
       }
       
       else {
          
          SurfaceID = VortexLoop(j).SurfaceID();
                
          SpanStation = 1;
          
       }          

       // Average y span location, and strip area
              
       Span_Yavg_[SurfaceID][SpanStation] += VortexLoop(j).Yc() * VortexLoop(j).Area();

       Span_Area_[SurfaceID][SpanStation] += VortexLoop(j).Area();

       Local_Vel_[0][SurfaceID][SpanStation] += VSPGeom().Grid(1).LoopList(j).LocalFreeStreamVelocity(0) * VortexLoop(j).Area() / Vref_;
       Local_Vel_[1][SurfaceID][SpanStation] += VSPGeom().Grid(1).LoopList(j).LocalFreeStreamVelocity(1) * VortexLoop(j).Area() / Vref_;
       Local_Vel_[2][SurfaceID][SpanStation] += VSPGeom().Grid(1).LoopList(j).LocalFreeStreamVelocity(2) * VortexLoop(j).Area() / Vref_;
       Local_Vel_[3][SurfaceID][SpanStation] += VSPGeom().Grid(1).LoopList(j).LocalFreeStreamVelocity(4) * VortexLoop(j).Area() / Vref_; // Yes, it's 4

    }
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
       
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
           
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
          
       }
       
       else {
          
          NumberOfStations = 1;
          
       }     
              
       for ( j = 1 ; j <= NumberOfStations ; j++ ) {
          
          Span_Yavg_[i][j] /= Span_Area_[i][j];
          
          Local_Vel_[0][i][j] /= Span_Area_[i][j];
          Local_Vel_[1][i][j] /= Span_Area_[i][j];
          Local_Vel_[2][i][j] /= Span_Area_[i][j];
          Local_Vel_[3][i][j] /= Span_Area_[i][j];
          
       }
       
    }    

    // Non-dimensionalize
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
          
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();

       }
       
       else {
          
          NumberOfStations = 1;

       }   

       for ( k = 1 ; k <= NumberOfStations ; k++ ) {

          Span_Cx_[i][k] /= (0.5*Span_Area_[i][k]*Vref_*Vref_);
          
          Span_Cy_[i][k] /= (0.5*Span_Area_[i][k]*Vref_*Vref_);
          
          Span_Cz_[i][k] /= (0.5*Span_Area_[i][k]*Vref_*Vref_);

          
          Span_Cmx_[i][k] /= (0.5*Span_Area_[i][k]*Vref_*Vref_*VSPGeom().VSP_Surface(i).LocalChord(k));  // Roll
                                           
          Span_Cmy_[i][k] /= (0.5*Span_Area_[i][k]*Vref_*Vref_*VSPGeom().VSP_Surface(i).LocalChord(k));  // Pitch
                                           
          Span_Cmz_[i][k] /= (0.5*Span_Area_[i][k]*Vref_*Vref_*VSPGeom().VSP_Surface(i).LocalChord(k));  // Yaw

          
          Span_Cn_[i][k] /= (0.5*Span_Area_[i][k]*Vref_*Vref_);

          Span_Cxi_[i][k] /= (0.5*Span_Area_[i][k]*Vref_*Vref_);
                                            
          Span_Cyi_[i][k] /= (0.5*Span_Area_[i][k]*Vref_*Vref_);          
                                            
          Span_Czi_[i][k] /= (0.5*Span_Area_[i][k]*Vref_*Vref_);  
        
       }
       
    }
          
    // Calculate maximum CL... imposing local 2D Clmax limits
        
    LoadCase = 0;
    
    if ( UnsteadyEvaluation ) LoadCase = 1;

    CL_[LoadCase] = CS_[LoadCase] = CD_[LoadCase] = CFx_[LoadCase] = CFy_[LoadCase] = CFz_[LoadCase] = CMx_[LoadCase] = CMy_[LoadCase] = CMz_[LoadCase] = CDo_ = CMxo_ = CMyo_ = CMzo_ = 0.;  

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
          
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();

       }
       
       else {
          
          NumberOfStations = 1;

       }   
       
       VSPGeom().VSP_Surface(i).CDo() = 0.;
           
       for ( k = 1 ; k <= NumberOfStations ; k++ ) {
         
          Cx = Span_Cx_[i][k];
          
          Cy = Span_Cy_[i][k];
          
          Cz = Span_Cz_[i][k];
          
          Cn = Span_Cn_[i][k];
                    
          Cl =  (  -Span_Cx_[i][k] * SA +  Span_Cz_[i][k] * CA );
   
          Cd =  (   Span_Cx_[i][k] * CA +  Span_Cz_[i][k] * SA ) * CB -  Span_Cy_[i][k] * SB;
   
          Cs =  (   Span_Cx_[i][k] * CA +  Span_Cz_[i][k] * SA ) * SB +  Span_Cy_[i][k] * CB;

          Cdi = (  Span_Cxi_[i][k] * CA + Span_Czi_[i][k] * SA ) * CB - Span_Cyi_[i][k] * SB;

          Ct = sqrt( Cl*Cl + Cs*Cs );
          
          Cmx = Span_Cmx_[i][k]; 

          Cmy = Span_Cmy_[i][k]; 

          Cmz = Span_Cmz_[i][k]; 
          
          // Cdi and Cd the same for supersonic cases
          
          if ( Mach_ >= 1. ) Cdi = Cd;
      
          // Adjust forces and moments for local 2d stall
          
          StallFact = 1.;
          
          if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE && Clmax_2d_ > 0. ) {
          
             Cl_2d = Clmax_2d_ * pow(Local_Vel_[3][i][k], 2.);
          
             if ( ABS(Ct) > 0. ) StallFact = ABS(MIN(ABS(Ct), Cl_2d) / ABS(Ct));
             
          }

          Span_Cl_[i][k] = StallFact * Cl;
          Span_Cd_[i][k] = StallFact * Cdi;
          Span_Cs_[i][k] = StallFact * Cs;
          
          Span_Cx_[i][k] = StallFact * Cx;
          Span_Cy_[i][k] = StallFact * Cy;
          Span_Cz_[i][k] = StallFact * Cz;
          
          Span_Cn_[i][k] = StallFact * Cn;
           
          Span_Cmx_[i][k] = StallFact * Cmx;
          Span_Cmy_[i][k] = StallFact * Cmy;
          Span_Cmz_[i][k] = StallFact * Cmz;

          // Integrate spanwise forces and moments
          
           CL_[LoadCase] += 0.5 *  Span_Cl_[i][k] * Span_Area_[i][k];
           CD_[LoadCase] += 0.5 *  Span_Cd_[i][k] * Span_Area_[i][k];
           CS_[LoadCase] += 0.5 *  Span_Cs_[i][k] * Span_Area_[i][k];
 
          CFx_[LoadCase] += 0.5 *  Span_Cx_[i][k] * Span_Area_[i][k];
          CFy_[LoadCase] += 0.5 *  Span_Cy_[i][k] * Span_Area_[i][k];
          CFz_[LoadCase] += 0.5 *  Span_Cz_[i][k] * Span_Area_[i][k];

          CMx_[LoadCase] += 0.5 * Span_Cmx_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
          CMy_[LoadCase] += 0.5 * Span_Cmy_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
          CMz_[LoadCase] += 0.5 * Span_Cmz_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
          
          // If this is an unsteady case, keep track of component group forces and moments
      
          if ( UnsteadyEvaluation == 0 ) {
             
             c = ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()];
   
             if ( c > 0) {
                
                // Forces
                
                dF[0] = 0.5 * Span_Cx_[i][k] * Span_Area_[i][k];
                dF[1] = 0.5 * Span_Cy_[i][k] * Span_Area_[i][k];
                dF[2] = 0.5 * Span_Cz_[i][k] * Span_Area_[i][k];
                  
                ComponentGroupList_[c].Cx() += dF[0];
                ComponentGroupList_[c].Cy() += dF[1];
                ComponentGroupList_[c].Cz() += dF[2];
   
                // Moments, about component CG
                
                ComponentCg[0] = ComponentGroupList_[c].OVec(0);
                ComponentCg[1] = ComponentGroupList_[c].OVec(1);
                ComponentCg[2] = ComponentGroupList_[c].OVec(2);
   
                dM[0] = 0.5 * Span_Cmx_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
                dM[1] = 0.5 * Span_Cmy_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
                dM[2] = 0.5 * Span_Cmz_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
                
                dM[0] -= dF[2] * ( ComponentCg[1] - XYZcg_[1] ) - dF[1] * ( ComponentCg[2] - XYZcg_[2] );
                dM[1] -= dF[0] * ( ComponentCg[2] - XYZcg_[2] ) - dF[2] * ( ComponentCg[0] - XYZcg_[0] );
                dM[2] -= dF[1] * ( ComponentCg[0] - XYZcg_[0] ) - dF[0] * ( ComponentCg[1] - XYZcg_[1] );
                
                ComponentGroupList_[c].Cmx() += dM[0];
                ComponentGroupList_[c].Cmy() += dM[1];
                ComponentGroupList_[c].Cmz() += dM[2];
   
             }
             
          }
  
          // Spanwise viscous drag for wings
          
          if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {

             // Root LE, TE locations
   
             xyzLE[0] = VSPGeom().VSP_Surface(i).xLE(k);
             xyzLE[1] = VSPGeom().VSP_Surface(i).yLE(k);
             xyzLE[2] = VSPGeom().VSP_Surface(i).zLE(k);
             
             xyzTE[0] = VSPGeom().VSP_Surface(i).xTE(k);
             xyzTE[1] = VSPGeom().VSP_Surface(i).yTE(k);
             xyzTE[2] = VSPGeom().VSP_Surface(i).zTE(k);         
             
             // Mid point
             
             xyz_mid[0] = 0.5*( xyzLE[0] + xyzTE[0] );
             xyz_mid[1] = 0.5*( xyzLE[1] + xyzTE[1] );
             xyz_mid[2] = 0.5*( xyzLE[2] + xyzTE[2] );
            
             // Root chord vector and length
             
             S[0] = xyzTE[0] - xyzLE[0];
             S[1] = xyzTE[1] - xyzLE[1];
             S[2] = xyzTE[2] - xyzLE[2];
            
             Mag = sqrt(vector_dot(S,S));
             
             S[0] /= Mag;
             S[1] /= Mag;
             S[2] /= Mag;
           
             // Alignment of strip with local velocity

             Dot = 0.;
             
             if ( Local_Vel_[3][i][k] > 0. ) {
                
                Dot = ( S[0]*Local_Vel_[0][i][k] + S[1]*Local_Vel_[1][i][k] + S[2]*Local_Vel_[2][i][k] )/Local_Vel_[3][i][k];
             }
                     
             LocalVel = Local_Vel_[3][i][k];
             
             // Chord
             
             Length = VSPGeom().VSP_Surface(i).LocalChord(k);
       
             // Local Re number, note that Local_Vel is scaled by Vref_
             
             Re = ReCref_ * LocalVel * Vref_ * Length / Cref_;
             
             Re = MAX(Re,1.e6);
             
             Cf = 0.61 / pow(log10(Re),2.58);

             // Calculate flat plate viscous force... note density is booked kept as '1'
            
             ViscousForce = 0.5 * Cf * pow(LocalVel * Vref_, 2.) * Span_Area_[i][k];

             // Calculate 2D drag due to lift, simple fit to NACA 0012 data
             
             CLv = Span_Cn_[i][k] / pow(LocalVel, 2.);

             ViscousForce += 0.5*0.00625*pow(CLv, 2.) * pow(LocalVel * Vref_, 2.) * Span_Area_[i][k];

             dF[0] = ViscousForce * S[0] * Dot;
             dF[1] = ViscousForce * S[1] * Dot;
             dF[2] = ViscousForce * S[2] * Dot;
             
             // Drag component
             
             dCD = 0.;
             
             if ( Vinf_ > 0. ) dCD = vector_dot(dF,FreeStreamVelocity_)/Vinf_;
             
             // Non-dimensionalize
             
             dCD /= 0.5*Sref_*Vref_*Vref_;

             // Save at component level
             
             VSPGeom().VSP_Surface(i).CDo() += dCD;
             
             // Total drag
      
             CDo_ += dCD;
             
             // Forces

             Span_Cxo_[i][k] = dF[0]/(0.5*Span_Area_[i][k]*Vref_*Vref_);
             Span_Cyo_[i][k] = dF[1]/(0.5*Span_Area_[i][k]*Vref_*Vref_);  
             Span_Czo_[i][k] = dF[2]/(0.5*Span_Area_[i][k]*Vref_*Vref_);
                    
             CFxo_ += dF[0];
             CFyo_ += dF[1];
             CFzo_ += dF[2];
             
             // Moments
          
             dM[0] = dF[2] * ( xyz_mid[1] - XYZcg_[1] ) - dF[1] * ( xyz_mid[2] - XYZcg_[2] );   // Roll
             dM[1] = dF[0] * ( xyz_mid[2] - XYZcg_[2] ) - dF[2] * ( xyz_mid[0] - XYZcg_[0] );   // Pitch
             dM[2] = dF[1] * ( xyz_mid[0] - XYZcg_[0] ) - dF[0] * ( xyz_mid[1] - XYZcg_[1] );   // Yaw

             Span_Cmxo_[i][k] = dM[0]/(0.5*Span_Area_[i][k]*Vref_*Vref_*VSPGeom().VSP_Surface(i).LocalChord(k));
             Span_Cmyo_[i][k] = dM[1]/(0.5*Span_Area_[i][k]*Vref_*Vref_*VSPGeom().VSP_Surface(i).LocalChord(k));    
             Span_Cmzo_[i][k] = dM[2]/(0.5*Span_Area_[i][k]*Vref_*Vref_*VSPGeom().VSP_Surface(i).LocalChord(k));  
              
             CMxo_ += dM[0];             
             CMyo_ += dM[1];                     
             CMzo_ += dM[2];
  
             // If this is an unsteady case, keep track of component group forces and moments
 
             if ( UnsteadyEvaluation == 0 ) {
    
                c = ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()];
                
                if ( c > 0 ) {
                   
                   // Forces
                     
                   ComponentGroupList_[c].Cxo() += dF[0]/(0.5*Sref_*Vref_*Vref_);
                   ComponentGroupList_[c].Cyo() += dF[1]/(0.5*Sref_*Vref_*Vref_);
                   ComponentGroupList_[c].Czo() += dF[2]/(0.5*Sref_*Vref_*Vref_);
                
                   // Moments about group CG
                   
                   ComponentCg[0] = ComponentGroupList_[c].OVec(0);
                   ComponentCg[1] = ComponentGroupList_[c].OVec(1);
                   ComponentCg[2] = ComponentGroupList_[c].OVec(2);
                
                   ComponentGroupList_[c].Cmxo() += ( dF[2] * ( xyz_mid[1] - ComponentCg[1] ) - dF[1] * ( xyz_mid[2] - ComponentCg[2] ) )/(0.5*Bref_*Sref_*Vref_*Vref_);
                   ComponentGroupList_[c].Cmyo() += ( dF[0] * ( xyz_mid[2] - ComponentCg[2] ) - dF[2] * ( xyz_mid[0] - ComponentCg[0] ) )/(0.5*Cref_*Sref_*Vref_*Vref_);
                   ComponentGroupList_[c].Cmzo() += ( dF[1] * ( xyz_mid[0] - ComponentCg[0] ) - dF[0] * ( xyz_mid[1] - ComponentCg[1] ) )/(0.5*Bref_*Sref_*Vref_*Vref_);
                                                   
                }      
                
             }                            
                               
          }
          
       }
         
    }    

    CL_[LoadCase] /= 0.5*Sref_;
    CD_[LoadCase] /= 0.5*Sref_;
    CS_[LoadCase] /= 0.5*Sref_;
    
    CFx_[LoadCase] /= 0.5*Sref_;
    CFy_[LoadCase] /= 0.5*Sref_;
    CFz_[LoadCase] /= 0.5*Sref_;

    CMx_[LoadCase] /= 0.5*Bref_*Sref_;
    CMy_[LoadCase] /= 0.5*Cref_*Sref_;
    CMz_[LoadCase] /= 0.5*Bref_*Sref_;
    
    // If this is an unsteady case, keep track of component group forces and moments

    if ( UnsteadyEvaluation == 0 ) {
   
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
   
          ComponentGroupList_[c].Cx() /= 0.5*Sref_; 
          ComponentGroupList_[c].Cy() /= 0.5*Sref_; 
          ComponentGroupList_[c].Cz() /= 0.5*Sref_; 
   
          ComponentGroupList_[c].Cmx() /= 0.5*Bref_*Sref_;
          ComponentGroupList_[c].Cmy() /= 0.5*Cref_*Sref_;
          ComponentGroupList_[c].Cmz() /= 0.5*Bref_*Sref_;
   
          // Now calculate CL, CD, CS
          
          ComponentGroupList_[c].CL() = (-ComponentGroupList_[c].Cx() * SA + ComponentGroupList_[c].Cz() * CA );
          ComponentGroupList_[c].CD() = ( ComponentGroupList_[c].Cx() * CA + ComponentGroupList_[c].Cz() * SA ) * CB - ComponentGroupList_[c].Cy() * SB;
          ComponentGroupList_[c].CS() = ( ComponentGroupList_[c].Cx() * CA + ComponentGroupList_[c].Cz() * SA ) * SB + ComponentGroupList_[c].Cy() * CB;
   
          ComponentGroupList_[c].CLo() = (-ComponentGroupList_[c].Cxo() * SA + ComponentGroupList_[c].Czo() * CA );
          ComponentGroupList_[c].CDo() = ( ComponentGroupList_[c].Cxo() * CA + ComponentGroupList_[c].Czo() * SA ) * CB - ComponentGroupList_[c].Cyo() * SB;
          ComponentGroupList_[c].CSo() = ( ComponentGroupList_[c].Cxo() * CA + ComponentGroupList_[c].Czo() * SA ) * SB + ComponentGroupList_[c].Cyo() * CB;
    
          // Store time history of span loading for averaging later
          
          if ( (  TimeAccurate_ && CurrentTime_ >= ComponentGroupList_[c].StartAverageTime() ) || ( !TimeAccurate_ && CurrentWakeIteration_ >= WakeIterations_ ) ) {
          
             for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfSurfaces() ; j++ ) {
       
                i = ComponentGroupList_[c].SpanLoadData(j).SurfaceID();
              
                for ( k = 1 ; k <= ComponentGroupList_[c].SpanLoadData(j).NumberOfSpanStations() ; k++ ) {
    
                   // Flat plate normal
                   
                   nvec[0] = VSPGeom().VSP_Surface(i).NxQC(k);
                   nvec[1] = VSPGeom().VSP_Surface(i).NyQC(k);
                   nvec[2] = VSPGeom().VSP_Surface(i).NzQC(k);
                   
                   // Vector from leading to trailing edge
                   
                   svec[0] = VSPGeom().VSP_Surface(i).xTE(k) - VSPGeom().VSP_Surface(i).xLE(k);
                   svec[1] = VSPGeom().VSP_Surface(i).yTE(k) - VSPGeom().VSP_Surface(i).yLE(k);
                   svec[2] = VSPGeom().VSP_Surface(i).zTE(k) - VSPGeom().VSP_Surface(i).zLE(k);
                   
                   Mag = sqrt(vector_dot(svec,svec));
                   
                   svec[0] /= Mag;
                   svec[1] /= Mag;
                   svec[2] /= Mag;
         
                   Diameter = ComponentGroupList_[c].RotorDiameter();
                   
                   RPM = ComponentGroupList_[c].Omega() * 60 / ( 2.*PI );
           
                   // Forces
                   
                   dF[0] = Span_Cxo_[i][k] * 0.5 * Density_ * Vref_ * Vref_ * Span_Area_[i][k];
                   dF[1] = Span_Cyo_[i][k] * 0.5 * Density_ * Vref_ * Vref_ * Span_Area_[i][k];
                   dF[2] = Span_Czo_[i][k] * 0.5 * Density_ * Vref_ * Vref_ * Span_Area_[i][k];
                   
                   Thrust = vector_dot(dF, ComponentGroupList_[c].RVec());
                   
                   // Viscous Moments
   
                   ComponentCg[0] = ComponentGroupList_[c].OVec(0);
                   ComponentCg[1] = ComponentGroupList_[c].OVec(1);
                   ComponentCg[2] = ComponentGroupList_[c].OVec(2);
                   
                   dM[0] = Span_Cmxo_[i][k] * 0.5 * Density_ * Vref_ * Vref_ * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
                   dM[1] = Span_Cmyo_[i][k] * 0.5 * Density_ * Vref_ * Vref_ * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
                   dM[2] = Span_Cmzo_[i][k] * 0.5 * Density_ * Vref_ * Vref_ * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);   
    
                   dM[0] -= dF[2] * ( ComponentCg[1] - XYZcg_[1] ) - dF[1] * ( ComponentCg[2] - XYZcg_[2] );
                   dM[1] -= dF[0] * ( ComponentCg[2] - XYZcg_[2] ) - dF[2] * ( ComponentCg[0] - XYZcg_[0] );
                   dM[2] -= dF[1] * ( ComponentCg[0] - XYZcg_[0] ) - dF[0] * ( ComponentCg[1] - XYZcg_[1] );
     
                   Moment = -vector_dot(dM, ComponentGroupList_[c].RVec());     
                   
                   CalculateRotorCoefficientsFromForces(Thrust, Moment, Diameter, RPM, J, CT, CQ, CP, EtaP, CT_h, CQ_h, CP_h, FOM);    
                 
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cno(k) += vector_dot(nvec, dF) * CT_h / Thrust;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cso(k) += vector_dot(svec, dF) * CT_h / Thrust;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cto(k) += CT_h;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cqo(k) += CQ_h;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cpo(k) += CP_h;
   
                   // Inviscid forces
               
                   dF[0] = Span_Cx_[i][k] * 0.5 * Density_ * Vref_ * Vref_ * Span_Area_[i][k];
                   dF[1] = Span_Cy_[i][k] * 0.5 * Density_ * Vref_ * Vref_ * Span_Area_[i][k];
                   dF[2] = Span_Cz_[i][k] * 0.5 * Density_ * Vref_ * Vref_ * Span_Area_[i][k];
                   
                   Thrust = vector_dot(dF, ComponentGroupList_[c].RVec());
           
                   // Inviscid Moments
   
                   ComponentCg[0] = ComponentGroupList_[c].OVec(0);
                   ComponentCg[1] = ComponentGroupList_[c].OVec(1);
                   ComponentCg[2] = ComponentGroupList_[c].OVec(2);
                   
                   dM[0] = Span_Cmx_[i][k] * 0.5 * Density_ * Vref_ * Vref_ * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
                   dM[1] = Span_Cmy_[i][k] * 0.5 * Density_ * Vref_ * Vref_ * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
                   dM[2] = Span_Cmz_[i][k] * 0.5 * Density_ * Vref_ * Vref_ * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);   
    
                   dM[0] -= dF[2] * ( ComponentCg[1] - XYZcg_[1] ) - dF[1] * ( ComponentCg[2] - XYZcg_[2] );
                   dM[1] -= dF[0] * ( ComponentCg[2] - XYZcg_[2] ) - dF[2] * ( ComponentCg[0] - XYZcg_[0] );
                   dM[2] -= dF[1] * ( ComponentCg[0] - XYZcg_[0] ) - dF[0] * ( ComponentCg[1] - XYZcg_[1] );
     
                   Moment = -vector_dot(dM, ComponentGroupList_[c].RVec());     
                   
                   CalculateRotorCoefficientsFromForces(Thrust, Moment, Diameter, RPM, J, CT, CQ, CP, EtaP, CT_h, CQ_h, CP_h, FOM);    
                 
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cn(k) += vector_dot(nvec, dF) * CT_h / Thrust;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cs(k) += vector_dot(svec, dF) * CT_h / Thrust;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Ct(k) += CT_h;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cq(k) += CQ_h;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cp(k) += CP_h;
   
                   ComponentGroupList_[c].SpanLoadData(j).Span_Area(k)  += Span_Area_[i][k];
                   ComponentGroupList_[c].SpanLoadData(j).Span_Chord(k) += VSPGeom().VSP_Surface(i).LocalChord(k);
                   ComponentGroupList_[c].SpanLoadData(j).Span_S(k)     += VSPGeom().VSP_Surface(i).s(k);
                   
                   ComponentGroupList_[c].SpanLoadData(j).Local_Velocity(k) += Local_Vel_[3][i][k];
                                     
                }
   
                ComponentGroupList_[c].SpanLoadData(j).NumberOfTimeSamples()++;
       
             }
             
          }
             
       }
       
    }
     
    // Adjust for symmetry
    
    if ( DoSymmetryPlaneSolve_  ) {
       
       CFx_[LoadCase] *= 2.;
       CFy_[LoadCase] *= 2.; 
       CFz_[LoadCase] *= 2.; 
   
       CMx_[LoadCase] *= 2.; 
       CMy_[LoadCase] *= 2.; 
       CMz_[LoadCase] *= 2.; 
       
       CL_[LoadCase] *= 2.; 
       CD_[LoadCase] *= 2.; 
       CS_[LoadCase] *= 2.; 
       
    }
        
    if ( DoSymmetryPlaneSolve_ == SYM_X ) CMy_[LoadCase] = CMz_[LoadCase] = CFx_[LoadCase] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Y ) CMx_[LoadCase] = CMz_[LoadCase] = CFy_[LoadCase] = CS_[LoadCase] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Z ) CMx_[LoadCase] = CMy_[LoadCase] = CFz_[LoadCase] = 0.;
              
    // Loop over body surfaces and calculate skin friction drag
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
          
          Length = VSPGeom().VSP_Surface(i).AverageChord();
          
          Re = ReCref_ * Length / Cref_;
          
          Cf = 0.455 / pow(log10(Re),2.58);
        
          Swet = VSPGeom().VSP_Surface(i).WettedArea();
          
          // Body is split into 4 parts, wetted area over accounted for...  also add in 10% for form drag
          
          Swet *= 0.25 * 1.10;
          
          // If symmetry then centerline body panels were booted
          
          if ( DoSymmetryPlaneSolve_  ) Swet *= 2.;          
    
          // Bump by 25% for miscellaneous
          
          dCD = 1.25 * Cf * Swet / ( 0.5*Sref_ );

          // Save at component level

          VSPGeom().VSP_Surface(i).CDo() = dCD;
          
          // Total drag
   
          CDo_ += dCD;

       }
 
    }
    
    // Adjust for symmetry
    
    if ( DoSymmetryPlaneSolve_  ) {
       
       CDo_ *= 2.;
       
       CMxo_ = 0.;
       
       if ( DoSymmetryPlaneSolve_ == SYM_Y ) CFyo_ = 0.;
       if ( DoSymmetryPlaneSolve_ == SYM_Z ) CFzo_ = 0.;
    
       
    }

    CFxo_ /= 0.5*Sref_*Vref_*Vref_;
    CFyo_ /= 0.5*Sref_*Vref_*Vref_;
    CFzo_ /= 0.5*Sref_*Vref_*Vref_;
    
    CMxo_ /= 0.5*Bref_*Sref_*Vref_*Vref_;
    CMyo_ /= 0.5*Cref_*Sref_*Vref_*Vref_;
    CMzo_ /= 0.5*Bref_*Sref_*Vref_*Vref_;

    delete [] ComponentInThisGroup;

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
    
    // Write out generic header
    
    WriteCaseHeader(LoadFile_);
    
    // Write out column labels
    
                    // 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 
    fprintf(LoadFile_,"   Wing       S        Yavg     Chord     V/Vref      Cl        Cd        Cs        Cx        Cy       Cz        Cmx       Cmy       Cmz \n");

    TotalLift = 0.;  

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
        
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
              
          for ( k = 1 ; k <= NumberOfStations ; k++ ) {

             fprintf(LoadFile_,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
                     i,
                     VSPGeom().VSP_Surface(i).s(k),                    
                     Span_Yavg_[i][k],
                     VSPGeom().VSP_Surface(i).LocalChord(k),
                     Local_Vel_[3][i][k],
                     Span_Cl_[i][k],
                     Span_Cd_[i][k],
                     Span_Cs_[i][k],
                     Span_Cx_[i][k],
                     Span_Cy_[i][k],
                     Span_Cz_[i][k],
                     Span_Cmx_[i][k],
                     Span_Cmy_[i][k],
                     Span_Cmz_[i][k]);
            
             TotalLift += 0.5 * Span_Cl_[i][k] * Span_Area_[i][k];
      
          }
          
       }
                 
    }
    
    fprintf(LoadFile_,"\n\n\n");

                    // 123456789 123456789012345678901234567890123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789   
    fprintf(LoadFile_,"Comp      Component-Name                             Mach       AoA      Beta       CL        CDi       CS       CFx       CFy       CFz       Cmx       Cmy       Cmz \n");

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
        
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
          
          CMx /= 0.5*Sref_*Bref_;
          CMy /= 0.5*Sref_*Cref_;
          CMz /= 0.5*Sref_*Bref_;
          
          fprintf(LoadFile_,"%-9d %-40s %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
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
            
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
        
          k = 1;
    
          fprintf(LoadFile_,"%-9d %-40s %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
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
    
    fprintf(LoadFile_,"\n\n\n");

    TotalLift /= 0.5*Sref_*Vref_*Vref_;
               
}

/*##############################################################################
#                                                                              #
#                          VSP_SOLVER CreateFEMLoadFile                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateFEMLoadFile(int Case)
{
   
    char LoadFileName[2000];
   
    if ( Case == 0 || Case == 1 ) {
       
       // Open the fem load file
    
       sprintf(LoadFileName,"%s.fem",FileName_);
       
       if ( (FEMLoadFile_ = fopen(LoadFileName, "w")) == NULL ) {
   
          printf("Could not open the fem load file for output! \n");
   
          exit(1);
   
       }
       
    }
    
    WriteCaseHeader(FEMLoadFile_);
       
    // Write out FEM beam load file for VLM model
    
    if ( ModelType_ == VLM_MODEL ) {
       
      CreateFEMLoadFileFromVLMSolve(Case);
       
    }
    
    // Write out FEM beam load file for Panel model
    
    else if ( ModelType_ == PANEL_MODEL ) {
       
       CreateFEMLoadFileFromPanelSolve(Case);
       
    }
      
}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER CreateFEMLoadFileFromVLMSolve                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateFEMLoadFileFromVLMSolve(int Case)
{
 
    int i, k, NumberOfStations;
    double Vec[3], VecQC[3], VecQC_Def[3], RVec[3], Force[3], Moment[3], Chord;    

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
        
          fprintf(FEMLoadFile_,"Wing Surface: %d \n",i);
          fprintf(FEMLoadFile_,"SpanStations: %d \n",VSPGeom().VSP_Surface(i).NumberOfSpanStations());
          fprintf(FEMLoadFile_,"\n");
          
          //                    123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789  
          fprintf(FEMLoadFile_,"   Wing    XLE_ORIG  YLE_ORIG  ZLE_ORIG  XTE_ORIG  YTE_ORIG  ZTE_ORIG  XQC_ORIG  YQC_ORIG  ZQC_ORIG  S_ORIG     Area     Chord     XLE_DEF   YLE_DEF   ZLE_DEF   XTE_DEF   YTE_DEF   ZTE_DEF   XQC_DEF   YQC_DEF   ZQC_DEF    S_DEF       Cl        Cd        Cs        Cx        Cy        Cz       Cmx       Cmy       Cmz \n");

          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
              
          for ( k = 1 ; k <= NumberOfStations ; k++ ) {

             // Calculate local deformed quarter chord location
             
             Vec[0] = VSPGeom().VSP_Surface(i).xTE_Def(k) - VSPGeom().VSP_Surface(i).xLE_Def(k);
             Vec[1] = VSPGeom().VSP_Surface(i).yTE_Def(k) - VSPGeom().VSP_Surface(i).yLE_Def(k);
             Vec[2] = VSPGeom().VSP_Surface(i).zTE_Def(k) - VSPGeom().VSP_Surface(i).zLE_Def(k);
             
             Chord = sqrt(vector_dot(Vec,Vec));
         
             Vec[0] /= Chord;
             Vec[1] /= Chord;
             Vec[2] /= Chord;

             VecQC_Def[0] = VSPGeom().VSP_Surface(i).xLE_Def(k) + 0.25*Chord*Vec[0];
             VecQC_Def[1] = VSPGeom().VSP_Surface(i).yLE_Def(k) + 0.25*Chord*Vec[1];
             VecQC_Def[2] = VSPGeom().VSP_Surface(i).zLE_Def(k) + 0.25*Chord*Vec[2];
             
             // Calculate local undeformed quarter chord location
            
             Vec[0] = VSPGeom().VSP_Surface(i).xTE(k) - VSPGeom().VSP_Surface(i).xLE(k);
             Vec[1] = VSPGeom().VSP_Surface(i).yTE(k) - VSPGeom().VSP_Surface(i).yLE(k);
             Vec[2] = VSPGeom().VSP_Surface(i).zTE(k) - VSPGeom().VSP_Surface(i).zLE(k);
             
             Chord = sqrt(vector_dot(Vec,Vec));

             Vec[0] /= Chord;
             Vec[1] /= Chord;
             Vec[2] /= Chord;

             VecQC[0] = VSPGeom().VSP_Surface(i).xLE(k) + 0.25*Chord*Vec[0];
             VecQC[1] = VSPGeom().VSP_Surface(i).yLE(k) + 0.25*Chord*Vec[1];
             VecQC[2] = VSPGeom().VSP_Surface(i).zLE(k) + 0.25*Chord*Vec[2];
  
             // Transfer moments to the deformed, quarter chord location
          
             RVec[0] = XYZcg_[0] - VecQC_Def[0];
             RVec[1] = XYZcg_[1] - VecQC_Def[1];
             RVec[2] = XYZcg_[2] - VecQC_Def[2];
        
             Force[0] = Span_Cx_[i][k];
             Force[1] = Span_Cy_[i][k];
             Force[2] = Span_Cz_[i][k];
             
             vector_cross(RVec,Force,Moment);
             
             // Add in the moments from the CG

             Moment[0] += Span_Cmx_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
             Moment[1] += Span_Cmy_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
             Moment[2] += Span_Cmz_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
             
             // Re-nondimensionalize
             
             Moment[0] /= ( VSPGeom().VSP_Surface(i).LocalChord(k) );
             Moment[1] /= ( VSPGeom().VSP_Surface(i).LocalChord(k) );
             Moment[2] /= ( VSPGeom().VSP_Surface(i).LocalChord(k) );

             // Note... all forces and moments are referenced to the local chord and local quarter chord
             // Ie... these are '2D' coefficients

             fprintf(FEMLoadFile_,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
                     i,
                     VSPGeom().VSP_Surface(i).xLE(k),
                     VSPGeom().VSP_Surface(i).yLE(k),
                     VSPGeom().VSP_Surface(i).zLE(k),
                     VSPGeom().VSP_Surface(i).xTE(k),
                     VSPGeom().VSP_Surface(i).yTE(k),
                     VSPGeom().VSP_Surface(i).zTE(k),    
                     VecQC[0],
                     VecQC[1],
                     VecQC[2],                                      
                     VSPGeom().VSP_Surface(i).s(k),       
                     Span_Area_[i][k],
                     VSPGeom().VSP_Surface(i).LocalChord(k),              
                     VSPGeom().VSP_Surface(i).xLE_Def(k),
                     VSPGeom().VSP_Surface(i).yLE_Def(k),
                     VSPGeom().VSP_Surface(i).zLE_Def(k),
                     VSPGeom().VSP_Surface(i).xTE_Def(k),
                     VSPGeom().VSP_Surface(i).yTE_Def(k),
                     VSPGeom().VSP_Surface(i).zTE_Def(k),
                     VecQC_Def[0],
                     VecQC_Def[1],
                     VecQC_Def[2],
                     VSPGeom().VSP_Surface(i).s_Def(k),       
                     Span_Cl_[i][k],
                     Span_Cd_[i][k],
                     Span_Cs_[i][k],
                     Span_Cx_[i][k],
                     Span_Cy_[i][k],
                     Span_Cz_[i][k],
                     Moment[0],
                     Moment[1],
                     Moment[2]);
         
          }
          
          fprintf(FEMLoadFile_,"\n");
          fprintf(FEMLoadFile_,"   Planform:\n");
          fprintf(FEMLoadFile_,"\n");

          fprintf(FEMLoadFile_,"   Root LE: %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Root_LE(0),VSPGeom().VSP_Surface(i).Root_LE(1),VSPGeom().VSP_Surface(i).Root_LE(2));
          fprintf(FEMLoadFile_,"   Root TE: %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Root_TE(0),VSPGeom().VSP_Surface(i).Root_TE(1),VSPGeom().VSP_Surface(i).Root_TE(2));
          fprintf(FEMLoadFile_,"   Root QC: %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Root_QC(0),VSPGeom().VSP_Surface(i).Root_QC(1),VSPGeom().VSP_Surface(i).Root_QC(2));

          fprintf(FEMLoadFile_,"\n");

          fprintf(FEMLoadFile_,"   Tip LE:  %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Tip_LE(0),VSPGeom().VSP_Surface(i).Tip_LE(1),VSPGeom().VSP_Surface(i).Tip_LE(2));
          fprintf(FEMLoadFile_,"   Tip TE:  %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Tip_TE(0),VSPGeom().VSP_Surface(i).Tip_TE(1),VSPGeom().VSP_Surface(i).Tip_TE(2));
          fprintf(FEMLoadFile_,"   Tip QC:  %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Tip_QC(0),VSPGeom().VSP_Surface(i).Tip_QC(1),VSPGeom().VSP_Surface(i).Tip_QC(2));
                      
       }
                 
    }
    
    fprintf(FEMLoadFile_,"\n\n");
    fprintf(FEMLoadFile_,"Total Forces and Moments \n");
    fprintf(FEMLoadFile_,"\n\n");
     
    fprintf(FEMLoadFile_,"Total CL:  %lf \n", CL_[0]);
    fprintf(FEMLoadFile_,"Total CD:  %lf \n", CD_[0]);
    fprintf(FEMLoadFile_,"Total CS:  %lf \n", CS_[0]);
    
    fprintf(FEMLoadFile_,"Total CFx: %lf \n", CFx_[0]);
    fprintf(FEMLoadFile_,"Total CFy: %lf \n", CFy_[0]);
    fprintf(FEMLoadFile_,"Total CFz: %lf \n", CFz_[0]);

    fprintf(FEMLoadFile_,"Total CMx: %lf \n", CMx_[0]);
    fprintf(FEMLoadFile_,"Total CMy: %lf \n", CMy_[0]);
    fprintf(FEMLoadFile_,"Total CMz: %lf \n", CMz_[0]);

}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER CreateFEMLoadFileFromPanelSolve                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateFEMLoadFileFromPanelSolve(int Case)
{
 
    int i, j, k, Node, Node1, Node2, *OnVortexSheet;
    double *Fx, *Fy, *Fz, fx, fy, fz, Cl, Cd, Cs;
    double CA, SA, CB, SB;

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
        
    // Mark nodes on trailing edge of each lifting surface
    
    OnVortexSheet = new int[VSPGeom().Grid(1).NumberOfNodes() + 1];
    
    zero_int_array(OnVortexSheet, VSPGeom().Grid(1).NumberOfNodes());
    
    Fx  = new double[VSPGeom().Grid(1).NumberOfNodes() + 1];
    Fy  = new double[VSPGeom().Grid(1).NumberOfNodes() + 1];
    Fz  = new double[VSPGeom().Grid(1).NumberOfNodes() + 1];
    
    zero_double_array(Fx, VSPGeom().Grid(1).NumberOfNodes());
    zero_double_array(Fy, VSPGeom().Grid(1).NumberOfNodes());
    zero_double_array(Fz, VSPGeom().Grid(1).NumberOfNodes());
        
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          Node = VortexSheet(k).TrailingVortexEdge(i).Node();

          OnVortexSheet[Node] = k;     

       }
       
    }
   
    // Calculate forces for each lifting surface
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
             
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
     
          if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {
             
             Node1 = SurfaceVortexEdge(j).Node1();
             Node2 = SurfaceVortexEdge(j).Node2();
             
             if ( OnVortexSheet[Node1] == k && OnVortexSheet[Node2] == k ) {

                fx = SurfaceVortexEdge(j).Fx() + SurfaceVortexEdge(j).Unsteady_Fx();
                fy = SurfaceVortexEdge(j).Fy() + SurfaceVortexEdge(j).Unsteady_Fy();
                fz = SurfaceVortexEdge(j).Fz() + SurfaceVortexEdge(j).Unsteady_Fz();
                
                Fx[Node1] += 0.5*fx;
                Fy[Node1] += 0.5*fy;
                Fz[Node1] += 0.5*fz;
                
                Fx[Node2] += 0.5*fx;
                Fy[Node2] += 0.5*fy;
                Fz[Node2] += 0.5*fz;                
                
             }
             
          }
          
       }
       
    }
    
    // Write out data
    
    //                    123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 
    fprintf(FEMLoadFile_,"   Wing       X         Y         Z         Cx        Cy        Cz        CL        CD        CS \n");

    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          Node = VortexSheet(k).TrailingVortexEdge(i).Node();
          
          // Nodal forces, non-dimensionalized by Sref
          
          fx = Fx[Node] / (0.5*Sref_*Vref_*Vref_);
          fy = Fy[Node] / (0.5*Sref_*Vref_*Vref_);
          fz = Fz[Node] / (0.5*Sref_*Vref_*Vref_);
          
          // Nodal force coefficients, again non-dimensionalized by Sref
          
          Cl =   ( -fx * SA + fz * CA );
          Cd = ( (  fx * CA + fz * SA ) * CB - fy * SB );
          Cs = ( (  fx * CA + fz * SA ) * SB + fy * CB );

          fprintf(FEMLoadFile_,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf\n",
          k,   
          VSPGeom().Grid(1).NodeList(Node).x(),
          VSPGeom().Grid(1).NodeList(Node).y(),
          VSPGeom().Grid(1).NodeList(Node).z(),
          fx,
          fy,
          fz,
          Cl,
          Cd,
          Cs);
          
       }
       
    }
    
    fprintf(FEMLoadFile_,"\n");
    fprintf(FEMLoadFile_,"Note: Force coefficients are NOT 2D - they are the full 3D forces, non-dimensionalized by Q * Sref \n");

}         
          
/*##############################################################################
#                                                                              #
#                         VSP_SOLVER WriteFEM2DGeometry                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteFEM2DGeometry(void)
{

    int j, SurfaceID;
    int number_of_nodes, number_of_tris;
    char LoadFileName[2000];

    // Open the fem load file
    
    sprintf(LoadFileName,"%s.fem2d",FileName_);
    
    if ( (FEM2DLoadFile_ = fopen(LoadFileName, "w")) == NULL ) {

       printf("Could not open the fem load file for output! \n");

       exit(1);

    }
    
    // Write out header 

    number_of_tris  = VSPGeom().Grid().NumberOfLoops();
    number_of_nodes = VSPGeom().Grid().NumberOfNodes();

    fprintf(FEM2DLoadFile_,"NumberOfNodes:%d \n",number_of_nodes);
    fprintf(FEM2DLoadFile_,"NumberOfTris: %d \n",number_of_tris);

    // Write out node data
    
    fprintf(FEM2DLoadFile_,"Nodal data: \n");
    fprintf(FEM2DLoadFile_,"X, Y, Z \n");

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfNodes() ; j++ ) {

       fprintf(FEM2DLoadFile_,"%f %f %f \n",
        VSPGeom().Grid().NodeList(j).x(),
        VSPGeom().Grid().NodeList(j).y(),
        VSPGeom().Grid().NodeList(j).z());
       
    }
        
    // Write out triangulated surface mesh
    
    fprintf(FEM2DLoadFile_,"Tri data: \n");
    fprintf(FEM2DLoadFile_,"Node1, Node2, Node3, SurfType, SurfID, Area, Nx, Ny, Nz \n");

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {

       SurfaceID   = VSPGeom().Grid().LoopList(j).DegenBodyID()
                   + VSPGeom().Grid().LoopList(j).DegenWingID()
                   + VSPGeom().Grid().LoopList(j).Cart3dID();
                 
       fprintf(FEM2DLoadFile_,"%d %d %d %d %d %f %f %f %f \n",
         VSPGeom().Grid().LoopList(j).Node1(), 
         VSPGeom().Grid().LoopList(j).Node2(), 
         VSPGeom().Grid().LoopList(j).Node3(), 
         VSPGeom().Grid().LoopList(j).SurfaceType(),
         SurfaceID, 
         VSPGeom().Grid().LoopList(j).Area(),
         VSPGeom().Grid().LoopList(j).Nx(),
         VSPGeom().Grid().LoopList(j).Ny(),
         VSPGeom().Grid().LoopList(j).Nz());

    }

}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER WriteFEM2DSolution                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteFEM2DSolution(void)
{

    int j;

    fprintf(FEM2DLoadFile_,"\n");
    
    WriteCaseHeader(FEM2DLoadFile_);
            
    // Write out solution

    fprintf(FEM2DLoadFile_,"Solution Data\n");    
    fprintf(FEM2DLoadFile_,"Tri, DeltaCp_or_Cp \n");

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {

       fprintf(FEM2DLoadFile_,"%d %f \n", j, VSPGeom().Grid().LoopList(j).dCp()/(0.5*Vref_*Vref_));

    }

}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER CalculateVelocitySurvey                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateVelocitySurvey(void)
{

    int i, k, p;
    double xyz[3], q[5];
    double *U, *V, *W;
    char SurveyFileName[2000];
    FILE *SurveyFile;
    
    U = new double[NumberofSurveyPoints_ + 1];
    V = new double[NumberofSurveyPoints_ + 1];
    W = new double[NumberofSurveyPoints_ + 1];

    zero_double_array(U, NumberofSurveyPoints_);
    zero_double_array(V, NumberofSurveyPoints_);
    zero_double_array(W, NumberofSurveyPoints_);

    // Initialize to free stream values

#pragma omp parallel for  
    for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {

       U[i] = FreeStreamVelocity_[0];
       V[i] = FreeStreamVelocity_[1];
       W[i] = FreeStreamVelocity_[2];

    }
    
    // Add in the rotor induced velocities
 
    for ( k = 1 ; k <= NumberOfRotors_ ; k++ ) {
    
//#pragma omp parallel for private(xyz,q)           
       for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {
    
          xyz[0] = SurveyPointList(i).x();
          xyz[1] = SurveyPointList(i).y();
          xyz[2] = SurveyPointList(i).z();
   
          RotorDisk(k).Velocity(xyz, q);                   
   
          U[i] += q[0];
          V[i] += q[1];
          W[i] += q[2];
          
          // If ground effects... add in ground effects ... z plane
          
          if ( DoGroundEffectsAnalysis() ) {
   
             xyz[0] = SurveyPointList(i).x();
             xyz[1] = SurveyPointList(i).y();
             xyz[2] = SurveyPointList(i).z();
                
             xyz[2] *= -1.;
            
             RotorDisk(k).Velocity(xyz, q);        
   
             q[2] *= -1.;
            
             U[i] += q[0];
             V[i] += q[1];
             W[i] += q[2];
   
          }             
          
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {
   
             xyz[0] = SurveyPointList(i).x();
             xyz[1] = SurveyPointList(i).y();
             xyz[2] = SurveyPointList(i).z();
             
             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
             
             RotorDisk(k).Velocity(xyz, q);        
   
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
            
             U[i] += q[0];
             V[i] += q[1];
             W[i] += q[2];
             
             // If ground effects... add in ground effects ... z plane
             
             if ( DoGroundEffectsAnalysis() ) {

                xyz[2] *= -1.;
               
                RotorDisk(k).Velocity(xyz, q);        
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                      q[2] *= -1.;
               
                U[i] += q[0];
                V[i] += q[1];
                W[i] += q[2];
   
             }
               
          }            
         
       }    
       
    }

    // Wing surface vortex induced velocities

    for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {
  
       xyz[0] = SurveyPointList(i).x();
       xyz[1] = SurveyPointList(i).y();
       xyz[2] = SurveyPointList(i).z();
    
       CalculateSurfaceInducedVelocityAtPoint(xyz, q);

       U[i] += q[0];
       V[i] += q[1];
       W[i] += q[2];
       
       // If ground effects... add in ground effects ... z plane
       
       if ( DoGroundEffectsAnalysis() ) {

         xyz[0] = SurveyPointList(i).x();
         xyz[1] = SurveyPointList(i).y();
         xyz[2] = SurveyPointList(i).z();
       
         xyz[2] *= -1.;
         
         CalculateSurfaceInducedVelocityAtPoint(xyz, q);

         q[2] *= -1.;
         
         U[i] += q[0];
         V[i] += q[1];
         W[i] += q[2];
         
       }
              
       // If there is a symmetry plane, calculate influence of the reflection
       
       if ( DoSymmetryPlaneSolve_ ) {

          xyz[0] = SurveyPointList(i).x();
          xyz[1] = SurveyPointList(i).y();
          xyz[2] = SurveyPointList(i).z();
         
          if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
          if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
          if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
         
          CalculateSurfaceInducedVelocityAtPoint(xyz, q);

          if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
          if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
          if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
         
          U[i] += q[0];
          V[i] += q[1];
          W[i] += q[2];

          // If ground effects... add in ground effects ... z plane
          
          if ( DoGroundEffectsAnalysis() ) {

            xyz[2] *= -1.;
            
            CalculateSurfaceInducedVelocityAtPoint(xyz, q);
   
            if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
            if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                  q[2] *= -1.;
            
            U[i] += q[0];
            V[i] += q[1];
            W[i] += q[2];
            
          }
                
       }
       
    }

    // Wake induced velocities

    for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {

       for ( p = 1 ; p <= NumberOfVortexSheets_ ; p++ ) {
       
          for ( k = 1 ; k <= VortexSheet(p).NumberOfTrailingVortices() ; k++ ) {
   
             xyz[0] = SurveyPointList(i).x();
             xyz[1] = SurveyPointList(i).y();
             xyz[2] = SurveyPointList(i).z();
   
             VortexSheet(p).TrailingVortexEdge(k).InducedVelocity(xyz, q);
                
             U[i] += q[0];
             V[i] += q[1];
             W[i] += q[2];
               
             // If ground effects... add in ground effects ... z plane
   
             if ( DoGroundEffectsAnalysis() ) {
   
                xyz[0] = SurveyPointList(i).x();
                xyz[1] = SurveyPointList(i).y();
                xyz[2] = SurveyPointList(i).z();
                        
                xyz[2] *= -1.;
               
                VortexSheet(p).TrailingVortexEdge(k).InducedVelocity(xyz, q);
      
                q[2] *= -1.;
               
                U[i] += q[0];
                V[i] += q[1];
                W[i] += q[2];
               
             }      
                             
             // If there is a symmetry plane, calculate influence of the reflection
   
             if ( DoSymmetryPlaneSolve_ ) {
   
                xyz[0] = SurveyPointList(i).x();
                xyz[1] = SurveyPointList(i).y();
                xyz[2] = SurveyPointList(i).z();
                        
                if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
               
                VortexSheet(p).TrailingVortexEdge(k).InducedVelocity(xyz, q);
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
               
                U[i] += q[0];
                V[i] += q[1];
                W[i] += q[2];
                
                // If ground effects... add in ground effects ... z plane
      
                if ( DoGroundEffectsAnalysis() ) {
  
                   xyz[2] *= -1.;
                  
                   VortexSheet(p).TrailingVortexEdge(k).InducedVelocity(xyz, q);
         
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                         q[2] *= -1.;
                  
                   U[i] += q[0];
                   V[i] += q[1];
                   W[i] += q[2];
                  
                }                      
               
             }                   
   
          }
          
       }

    }
    
    // Write out the velocity survey
    
    sprintf(SurveyFileName,"%s.svy",FileName_);
    
    if ( (SurveyFile = fopen(SurveyFileName, "w")) == NULL ) {

       printf("Could not open the survey file for output! \n");

       exit(1);

    }    
                       //0123456789x0123456789x0123456789x   0123456789x0123456789x0123456789x 
    fprintf(SurveyFile, "     X          Y          Z             U          V          W \n");

    for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {

       xyz[0] = SurveyPointList(i).x();
       xyz[1] = SurveyPointList(i).y();
       xyz[2] = SurveyPointList(i).z();
       
       fprintf(SurveyFile, "%10.5f %10.5f%10.5f    %10.5f %10.5f %10.5f \n",
               SurveyPointList(i).x(),
               SurveyPointList(i).y(),
               SurveyPointList(i).z(),
               U[i],
               V[i],
               W[i]);
     
    }    
    
    fclose(SurveyFile);
    
    delete [] U;
    delete [] V;
    delete [] W;
 
}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER WriteOutAerothermalDatabaseHeader                #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutAerothermalDatabaseHeader(void)
{
   
    char DumChar[2000];
    int i, number_of_nodes, number_of_tris;
    int i_size, c_size, f_size, DumInt, ComponentID;
    
    float Sref = Sref_;
    float Cref = Cref_;
    float Bref = Bref_;
    float X_cg = XYZcg_[0];
    float Y_cg = XYZcg_[1];
    float Z_cg = XYZcg_[2];

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Write out coded id to allow us to determine endiannes of files

    DumInt = -123789456; // Version 2 of the ADB file

    fwrite(&DumInt, i_size, 1, ADBFile_);
    
    // Write out model type... VLM or PANEL
    
    fwrite(&ModelType_, i_size, 1, ADBFile_);

    // Write out symmetry flag
    
    fwrite(&DoSymmetryPlaneSolve_, i_size, 1, ADBFile_);

    // Write out unsteady analysis flag
    
    if ( TimeAccurate_) {
    
       DumInt = 1;
    
       fwrite(&DumInt, i_size, 1, ADBFile_);    
       
    }
    
    else {
       
       DumInt = 0;
       
       fwrite(&DumInt, i_size, 1, ADBFile_);    
       
    }       
    
    // Write out header to aerodynamics file

    number_of_tris  = VSPGeom().Grid().NumberOfLoops();
    number_of_nodes = VSPGeom().Grid().NumberOfNodes();
    
    fwrite(&(NumberOfVortexLoops_),      i_size, 1, ADBFile_);
    fwrite(&number_of_nodes,             i_size, 1, ADBFile_);
    fwrite(&number_of_tris,              i_size, 1, ADBFile_);
    fwrite(&NumberOfSurfaceVortexEdges_, i_size, 1, ADBFile_);

    fwrite(&Sref,                   f_size, 1, ADBFile_);
    fwrite(&Cref,                   f_size, 1, ADBFile_);
    fwrite(&Bref,                   f_size, 1, ADBFile_);
    fwrite(&X_cg,                   f_size, 1, ADBFile_);
    fwrite(&Y_cg,                   f_size, 1, ADBFile_);
    fwrite(&Z_cg,                   f_size, 1, ADBFile_);
    
    // Write out wing ID flags, names...

    DumInt = VSPGeom().NumberOfDegenWings();
    
    fwrite(&DumInt, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
        
          fwrite(&(i), i_size, 1, ADBFile_);
    
          sprintf(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
          
          fwrite(DumChar, c_size, 100, ADBFile_);
          
          ComponentID = VSPGeom().VSP_Surface(i).ComponentID();
         
          fwrite(&ComponentID, i_size, 1, ADBFile_);       
       
       }
     
    }
    
    // Write out body ID flags, names...
 
    DumInt = VSPGeom().NumberOfDegenBodies();
    
    fwrite(&DumInt, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
        
          fwrite(&(i), i_size, 1, ADBFile_);
    
          sprintf(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
          
          fwrite(DumChar, c_size, 100, ADBFile_);
          
          ComponentID = VSPGeom().VSP_Surface(i).ComponentID();
          
          fwrite(&ComponentID, i_size, 1, ADBFile_);              
      
       }
     
    }
    
    // Write out Cart3d ID flags, names...
 
    DumInt = VSPGeom().NumberOfCart3dSurfaces();

    fwrite(&DumInt, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= VSPGeom().NumberOfCart3dSurfaces() ; i++ ) { 
     
       fwrite(&(i), i_size, 1, ADBFile_);
 
       sprintf(DumChar,"Surface_%d",i);
       
       fwrite(DumChar, c_size, 100, ADBFile_);

    }    
    
}   

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER ReadInAerothermalDatabaseHeader                  #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ReadInAerothermalDatabaseHeader(void)
{
   
    int i, Wings, Bodies, Surfs;
    int i_size, c_size, f_size;
    
    int DumInt;
    float DumFloat;
    char DumChar[2000];

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Read in endiannes of files

    fread(&DumInt, i_size, 1, InputADBFile_);
    
    // Read in model type... VLM or PANEL
    
    fread(&DumInt, i_size, 1, InputADBFile_);

    // Read in symmetry flag
    
    fread(&DumInt, i_size, 1, InputADBFile_);

    // Read in unsteady analysis flag
    
    fread(&TimeAccurate_, i_size, 1, InputADBFile_);    

    // Read in header to aerodynamics file

    fread(&DumInt,   i_size, 1, InputADBFile_);
    fread(&DumInt,   i_size, 1, InputADBFile_);
    fread(&DumInt,   i_size, 1, InputADBFile_);
    fread(&DumInt,   i_size, 1, InputADBFile_);    
    fread(&DumFloat, f_size, 1, InputADBFile_);
    fread(&DumFloat, f_size, 1, InputADBFile_);
    fread(&DumFloat, f_size, 1, InputADBFile_);
    fread(&DumFloat, f_size, 1, InputADBFile_);
    fread(&DumFloat, f_size, 1, InputADBFile_);
    fread(&DumFloat, f_size, 1, InputADBFile_);
    
    // Read in wing ID flags, names...

    fread(&Wings, i_size, 1, InputADBFile_);

    for ( i = 1 ; i <= Wings ; i++ ) { 

       fread(&DumInt, i_size, 1,   InputADBFile_);
       
       fread(DumChar, c_size, 100, InputADBFile_);
       
       fread(&DumInt, i_size, 1,   InputADBFile_);       

    }
    
    // Read in body ID flags, names...

    fread(&Bodies, i_size, 1, InputADBFile_);

    for ( i = 1 ; i <= Bodies ; i++ ) { 

       fread(&DumInt, i_size, 1,   InputADBFile_);

       fread(DumChar, c_size, 100, InputADBFile_);

       fread(&DumInt, i_size, 1,   InputADBFile_);              

    }
    
    // Read in Cart3d ID flags, names...

    fread(&Surfs, i_size, 1, InputADBFile_);

    for ( i = 1 ; i <=Surfs ; i++ ) { 
     
       fread(&DumInt, i_size, 1,   InputADBFile_);

       fread(DumChar, c_size, 100, InputADBFile_);

    }    
    
}   
 
/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutAerothermalDatabaseGeometry                #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutAerothermalDatabaseGeometry(void)
{

    int i, j, k, p, r, Node1, Node2, Node3, SurfaceType, SurfaceID;
    int i_size, c_size, f_size;
    int Level, NumberOfCoarseEdges, NumberOfCoarseNodes, MaxLevels;
    int NumberOfKuttaTE, NumberOfKuttaNodes, NumberOfControlLoops;
    int NumberOfControlSurfaces, Loop;
    float Area;
    float x, y, z;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Write out triangulated surface mesh

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {

       Node1       = VSPGeom().Grid().LoopList(j).Node1();
       Node2       = VSPGeom().Grid().LoopList(j).Node2();
       Node3       = VSPGeom().Grid().LoopList(j).Node3();

       SurfaceType = VSPGeom().Grid().LoopList(j).SurfaceType();
       
       SurfaceID   = VSPGeom().Grid().LoopList(j).DegenBodyID()
                   + VSPGeom().Grid().LoopList(j).DegenWingID()
                   + VSPGeom().Grid().LoopList(j).Cart3dID();

       Area        = VSPGeom().Grid().LoopList(j).Area();
       
       fwrite(&(Node1),       i_size, 1, ADBFile_);
       fwrite(&(Node2),       i_size, 1, ADBFile_);
       fwrite(&(Node3),       i_size, 1, ADBFile_);
       fwrite(&(SurfaceType), i_size, 1, ADBFile_);
       fwrite(&(SurfaceID),   i_size, 1, ADBFile_);
       fwrite(&Area,          f_size, 1, ADBFile_);

    }

    // Write out node data

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfNodes() ; j++ ) {

       x = VSPGeom().Grid().NodeList(j).x();
       y = VSPGeom().Grid().NodeList(j).y();
       z = VSPGeom().Grid().NodeList(j).z();
       
       fwrite(&(x), f_size, 1, ADBFile_);
       fwrite(&(y), f_size, 1, ADBFile_);
       fwrite(&(z), f_size, 1, ADBFile_);
       
    }

    // Write out the rotor data
    
    fwrite(&NumberOfRotors_, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
     
       RotorDisk(i).Write_Binary_STP_Data(ADBFile_);
     
    }
    
    // Write out the edges for each grid level
        
    MaxLevels = VSPGeom().NumberOfGridLevels();

    fwrite(&MaxLevels, i_size, 1, ADBFile_); 
   
    // Loop over each level

    for ( Level = 1 ; Level <= MaxLevels ; Level++ ) {

       NumberOfCoarseNodes = VSPGeom().Grid(Level).NumberOfNodes();

       NumberOfCoarseEdges = VSPGeom().Grid(Level).NumberOfEdges();
 
       fwrite(&NumberOfCoarseNodes, i_size, 1, ADBFile_); 

       fwrite(&NumberOfCoarseEdges, i_size, 1, ADBFile_); 

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

          x = VSPGeom().Grid(Level).NodeList(j).x();
          y = VSPGeom().Grid(Level).NodeList(j).y();
          z = VSPGeom().Grid(Level).NodeList(j).z();
          
          fwrite(&(x), f_size, 1, ADBFile_);
          fwrite(&(y), f_size, 1, ADBFile_);
          fwrite(&(z), f_size, 1, ADBFile_);         
     
       }
       
       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfEdges() ; j++ ) {

          SurfaceID = VSPGeom().Grid(Level).EdgeList(j).DegenBody()
                    + VSPGeom().Grid(Level).EdgeList(j).DegenWing()
                    + VSPGeom().Grid(Level).EdgeList(j).Cart3DSurface();
                    
          if ( VSPGeom().Grid(Level).EdgeList(j).Loop1() == VSPGeom().Grid(Level).EdgeList(j).Loop2() )  SurfaceID *= -1;         

          Node1 = VSPGeom().Grid(Level).EdgeList(j).Node1();
          Node2 = VSPGeom().Grid(Level).EdgeList(j).Node2();  

          fwrite(&SurfaceID, i_size, 1, ADBFile_);
          
          fwrite(&Node1, i_size, 1, ADBFile_);
          fwrite(&Node2, i_size, 1, ADBFile_);

       }
  
    }
    
    // Write out kutta edges
    
    Level = 1;
    
    NumberOfKuttaTE = 0;
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
 
       if ( VSPGeom().Grid(Level).EdgeList(i).IsTrailingEdge() ) {     
          
          NumberOfKuttaTE++;
          
       }
       
    }
    
    fwrite(&NumberOfKuttaTE, i_size, 1, ADBFile_);
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
 
       if ( VSPGeom().Grid(Level).EdgeList(i).IsTrailingEdge() ) {     

          fwrite(&i, i_size, 1, ADBFile_); 
          
       }
       
    }
    
    // Write out kutta nodes
    
    NumberOfKuttaNodes = 0;
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
            
       NumberOfKuttaNodes += VortexSheet(k).NumberOfTrailingVortices();
       
    }

    fwrite(&NumberOfKuttaNodes, i_size, 1, ADBFile_);
 
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
            
       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          Node1 = VortexSheet(k).TrailingVortexEdge(i).Node();
  
          fwrite(&Node1, i_size, 1, ADBFile_); 

       }
    
    }
    
    // Write out control surfaces
    
    NumberOfControlSurfaces = 0;
    
    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          NumberOfControlSurfaces += VSPGeom().VSP_Surface(j).NumberOfControlSurfaces();
          
       }
 
    }

    fwrite(&NumberOfControlSurfaces, i_size, 1, ADBFile_); 
    
    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          for ( k = 1 ; k <= VSPGeom().VSP_Surface(j).NumberOfControlSurfaces() ; k++ ) {
             
             p = VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfNodes();
             
             fwrite(&p, i_size, 1, ADBFile_);
       
             for ( p = 1 ; p <= VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfNodes() ; p++ ) {
        
                x = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node(p)[0];
                y = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node(p)[1];
                z = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node(p)[2];
                
                fwrite(&x, f_size, 1, ADBFile_); 
                fwrite(&y, f_size, 1, ADBFile_); 
                fwrite(&z, f_size, 1, ADBFile_); 
             
             }
             
             // Hinge data
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_1(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_1(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_1(2);   
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_);     
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_2(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_2(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_2(2);   
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_);       
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeVec(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeVec(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeVec(2);   
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_);                   
             
             // Affected loops
             
             NumberOfControlLoops = 0;
             
             for ( p = 1 ; p <= VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfLoops() ; p++ ) {
                
                Loop = VSPGeom().VSP_Surface(j).ControlSurface(k).LoopList(p);
                
                NumberOfControlLoops += VSPGeom().Grid(1).LoopList(Loop).NumberOfFineGridLoops();
                
             }
             
             fwrite(&NumberOfControlLoops, i_size, 1, ADBFile_);
             
             for ( p = 1 ; p <= VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfLoops() ; p++ ) {
                
                Loop = VSPGeom().VSP_Surface(j).ControlSurface(k).LoopList(p);
                
                for ( r = 1 ; r <= VSPGeom().Grid(1).LoopList(Loop).NumberOfFineGridLoops() ; r++ ) {
                   
                   fwrite(&(VSPGeom().Grid(1).LoopList(Loop).FineGridLoop(r)), i_size, 1, ADBFile_);
                   
                }
          
             }             
                                                                 
          }
          
       }
       
    }    

}

/*##############################################################################
#                                                                              #
#                 VSP_SOLVER ReadInAerothermalDatabaseGeometry                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ReadInAerothermalDatabaseGeometry(void)
{

    int i, j, k, p, Level, MaxLevels, NumberOfKuttaTE, NumberOfKuttaNodes, NumberOfControlSurfaces, NumberOfRotors;
    int i_size, c_size, f_size;
    
    int DumInt;
    float DumFloat;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Read in triangulated surface mesh

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {

       fread(&DumInt,   i_size, 1, InputADBFile_);
       fread(&DumInt,   i_size, 1, InputADBFile_);
       fread(&DumInt,   i_size, 1, InputADBFile_);
       fread(&DumInt,   i_size, 1, InputADBFile_);
       fread(&DumInt,   i_size, 1, InputADBFile_);
       fread(&DumFloat, f_size, 1, InputADBFile_);

    }

    // Read in node data

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfNodes() ; j++ ) {

       fread(&DumFloat, f_size, 1, InputADBFile_);
       fread(&DumFloat, f_size, 1, InputADBFile_);
       fread(&DumFloat, f_size, 1, InputADBFile_);
       
    }

    // Read in the rotor data
    
    fread(&NumberOfRotors, i_size, 1, InputADBFile_);

    for ( i = 1 ; i <= NumberOfRotors ; i++ ) {
     
       RotorDisk(i).Skip_Read_Binary_STP_Data(InputADBFile_);
     
    }
    
    // Read in the edges for each grid level

    fread(&MaxLevels, i_size, 1, InputADBFile_); 
   
    // Loop over each level

    for ( Level = 1 ; Level <= MaxLevels ; Level++ ) {

       fread(&DumInt, i_size, 1, InputADBFile_); 

       fread(&DumInt, i_size, 1, InputADBFile_); 

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

          fread(&DumFloat, f_size, 1, InputADBFile_);
          fread(&DumFloat, f_size, 1, InputADBFile_);
          fread(&DumFloat, f_size, 1, InputADBFile_);         
     
       }
       
       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfEdges() ; j++ ) {

          fread(&DumInt, i_size, 1, InputADBFile_);
          
          fread(&DumInt, i_size, 1, InputADBFile_);
          fread(&DumInt, i_size, 1, InputADBFile_);

       }
  
    }
    
    // Read in kutta edges

    fread(&NumberOfKuttaTE, i_size, 1, InputADBFile_);
    
    for ( i = 1 ; i <= NumberOfKuttaTE ; i++ ) {
 
       fread(&DumInt, i_size, 1, InputADBFile_); 

    }
    
    // Read in kutta nodes

    fread(&NumberOfKuttaNodes, i_size, 1, InputADBFile_);

    for ( k = 1 ; k <= NumberOfKuttaNodes ; k++ ) {

       fread(&DumInt, i_size, 1, InputADBFile_); 

    }
    
    // Read in control surfaces

    fread(&NumberOfControlSurfaces, i_size, 1, InputADBFile_); 
    
    for ( j = 1 ; j <= NumberOfControlSurfaces ; j++ ) {

       fwrite(&j, i_size, 1, InputADBFile_);
   
       for ( p = 1 ; p <= j ; p++ ) {
   
          fread(&DumFloat, f_size, 1, InputADBFile_); 
          fread(&DumFloat, f_size, 1, InputADBFile_); 
          fread(&DumFloat, f_size, 1, InputADBFile_); 
       
       }
       
       // Hinge data
       
       fread(&DumFloat, f_size, 1, InputADBFile_); 
       fread(&DumFloat, f_size, 1, InputADBFile_); 
       fread(&DumFloat, f_size, 1, InputADBFile_);     
       
       fread(&DumFloat, f_size, 1, InputADBFile_); 
       fread(&DumFloat, f_size, 1, InputADBFile_); 
       fread(&DumFloat, f_size, 1, InputADBFile_);       
   
       fread(&DumFloat, f_size, 1, InputADBFile_); 
       fread(&DumFloat, f_size, 1, InputADBFile_); 
       fread(&DumFloat, f_size, 1, InputADBFile_);                   
       
       // Affected loops
   
       fread(&j, i_size, 1, InputADBFile_);
       
       for ( p = 1 ; p <= j ; p++ ) {
   
          fread(&DumInt, i_size, 1, InputADBFile_);
   
       }             
          
    }    

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutAerothermalDatabaseSolution                #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutAerothermalDatabaseSolution(void)
{

    int i, j, k;
    int i_size, c_size, f_size, d_size;

    float DumFloat;
    
    float Cp, Cp_Unsteady, Gamma;

    // Write out case data to adb case file
    
    fprintf(ADBCaseListFile_,"%10.7f %10.7f %10.7f    %-200s \n",Mach_, AngleOfAttack_/TORAD, AngleOfBeta_/TORAD, CaseString_);
    
    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
    d_size = sizeof(double);

    // Write out Mach, Alpha, Beta

    DumFloat = Mach_;

    fwrite(&DumFloat, f_size, 1, ADBFile_);

    DumFloat = AngleOfAttack_;

    fwrite(&DumFloat, f_size, 1, ADBFile_);

    DumFloat = AngleOfBeta_;

    fwrite(&DumFloat, f_size, 1, ADBFile_);    

    // Write out min and min and max Cp
    
    DumFloat = CpMin_;
    
    fwrite(&(DumFloat), f_size, 1, ADBFile_);
    
    DumFloat = CpMax_;
    
    fwrite(&(DumFloat), f_size, 1, ADBFile_);
        
    // Write out the vortex strengths, and both the steady and unsteady Cp on the computational mesh

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       fwrite(&(Gamma_[0][i]                ), d_size, 1, ADBFile_);
       fwrite(&(VortexLoop(i).dCp_Unsteady()), d_size, 1, ADBFile_);
           
    }   
        
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       fwrite(&(SurfaceVortexEdge(j).Fx()), d_size, 1, ADBFile_);
       fwrite(&(SurfaceVortexEdge(j).Fy()), d_size, 1, ADBFile_);
       fwrite(&(SurfaceVortexEdge(j).Fz()), d_size, 1, ADBFile_);
         
    }

    // Write out surface velocities on the computational mesh
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       fwrite(&(VortexLoop(i).U()), d_size, 1, ADBFile_);
       fwrite(&(VortexLoop(i).V()), d_size, 1, ADBFile_);
       fwrite(&(VortexLoop(i).W()), d_size, 1, ADBFile_);

    }    
            
    // Write out solution on the input tri mesh

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {
  
       Gamma       = VSPGeom().Grid().LoopList(j).Gamma();
       Cp          = VSPGeom().Grid().LoopList(j).dCp();
       Cp_Unsteady = VSPGeom().Grid().LoopList(j).dCp_Unsteady();
    
       fwrite(&Cp,          f_size, 1, ADBFile_); // Delta Cp, or CP
       fwrite(&Cp_Unsteady, f_size, 1, ADBFile_); // Unsteady Delta Cp, or Cp
       fwrite(&Gamma,       f_size, 1, ADBFile_); // Circulation strength

    }

    // Write out wake shape
   
    fwrite(&NumberOfTrailingVortexEdges_, i_size, 1, ADBFile_);
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
      
       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          VortexSheet(k).TrailingVortexEdge(i).WriteToFile(ADBFile_);

       }
       
    }     
    
    // Write out control surface deflection angles

    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          for ( k = 1 ; k <= VSPGeom().VSP_Surface(j).NumberOfControlSurfaces() ; k++ ) {

             DumFloat = VSPGeom().VSP_Surface(j).ControlSurface(k).DeflectionAngle();

             fwrite(&(DumFloat), f_size, 1, ADBFile_); 

          }
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                 VSP_SOLVER ReadInAerothermalDatabaseSolution                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ReadInAerothermalDatabaseSolution(int TimeCase)
{

    int i, j, k;
    int i_size, c_size, f_size, d_size;

    int DumInt;
    float DumFloat;
    float Cp, Cp_Unsteady, Gamma;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
    d_size = sizeof(double);

    // Read in Mach, Alpha, Beta

    fread(&DumFloat, f_size, 1, InputADBFile_);

    fread(&DumFloat, f_size, 1, InputADBFile_);

    fread(&DumFloat, f_size, 1, InputADBFile_);    

    // Read in min and min and max Cp

    fread(&DumFloat, f_size, 1, InputADBFile_);

    fread(&DumFloat, f_size, 1, InputADBFile_);
       
    // Read the vortex strengths and unsteady Cp on the computational mesh

    // This will be N - TimeCase
    
    if ( TimeCase >= 0 && TimeCase <= 5 ) {
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
          fread(&(      GammaNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
          fread(&(dCpUnsteadyNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
     
       }  
       
    }
 
    // This will be N, but shift current to N-1, and N-1 to N-2
    
    else if ( TimeCase == -1 ) {

       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
                GammaNoise_[5][i] =       GammaNoise_[4][i];
                GammaNoise_[4][i] =       GammaNoise_[3][i];
                GammaNoise_[3][i] =       GammaNoise_[2][i];
                GammaNoise_[2][i] =       GammaNoise_[1][i];
                GammaNoise_[1][i] =       GammaNoise_[0][i];
                
          dCpUnsteadyNoise_[5][i] = dCpUnsteadyNoise_[4][i];                                
          dCpUnsteadyNoise_[4][i] = dCpUnsteadyNoise_[3][i];                
          dCpUnsteadyNoise_[3][i] = dCpUnsteadyNoise_[2][i];
          dCpUnsteadyNoise_[2][i] = dCpUnsteadyNoise_[1][i];
          dCpUnsteadyNoise_[1][i] = dCpUnsteadyNoise_[0][i];
     
       }  
              
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
          fread(&(      GammaNoise_[0][i]), d_size, 1, InputADBFile_);
          fread(&(dCpUnsteadyNoise_[0][i]), d_size, 1, InputADBFile_);
     
       }  
       
    }    
    
    else {
       
       printf("Unknown ADB read case: %d \n",TimeCase);
       fflush(NULL);
       exit(1);
       
    }
    
    // Read in the edge forces on the computational mesh

    // This will be N - TimeCase
    
    if ( TimeCase >= 0 && TimeCase <= 5 ) {
       
       for ( i = 1 ; i <= NumberOfSurfaceVortexEdges_ ; i++ ) {
          
          fread(&(FxNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
          fread(&(FyNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
          fread(&(FzNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
            
       }
       
    }

    // This will be N, but shift current to N-1
    
    else if ( TimeCase == -1 ) {

       for ( i = 1 ; i <= NumberOfSurfaceVortexEdges_ ; i++ ) {

          FxNoise_[5][i] = FxNoise_[4][i];
          FyNoise_[5][i] = FyNoise_[4][i];
          FzNoise_[5][i] = FzNoise_[4][i];

          FxNoise_[4][i] = FxNoise_[3][i];
          FyNoise_[4][i] = FyNoise_[3][i];
          FzNoise_[4][i] = FzNoise_[3][i];

          FxNoise_[3][i] = FxNoise_[2][i];
          FyNoise_[3][i] = FyNoise_[2][i];
          FzNoise_[3][i] = FzNoise_[2][i];

          FxNoise_[2][i] = FxNoise_[1][i];
          FyNoise_[2][i] = FyNoise_[1][i];
          FzNoise_[2][i] = FzNoise_[1][i];
             
          FxNoise_[1][i] = FxNoise_[0][i];
          FyNoise_[1][i] = FyNoise_[0][i];
          FzNoise_[1][i] = FzNoise_[0][i];
     
       }  
              
       for ( i = 1 ; i <= NumberOfSurfaceVortexEdges_ ; i++ ) {
          
          fread(&(FxNoise_[0][i]), d_size, 1, InputADBFile_);
          fread(&(FyNoise_[0][i]), d_size, 1, InputADBFile_);
          fread(&(FzNoise_[0][i]), d_size, 1, InputADBFile_);
            
       }
       
    }    
    
    else {
       
       printf("Unknown ADB read case: %d \n",TimeCase);
       fflush(NULL);
       exit(1);
       
    }    
    
    // Read in surface velocities on the computational mesh

     // This will be N - TimeCase
    
    if ( TimeCase >= 0 && TimeCase <= 5 ) {
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
          fread(&(UNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
          fread(&(VNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
          fread(&(WNoise_[TimeCase][i]), d_size, 1, InputADBFile_);

       }  
       
    }
    
    // This will be N, but shift current to N-1
    
    else if ( TimeCase == -1 ) {

       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

          UNoise_[5][i] = UNoise_[4][i];
          VNoise_[5][i] = VNoise_[4][i];
          WNoise_[5][i] = WNoise_[4][i];
          
          UNoise_[4][i] = UNoise_[3][i];
          VNoise_[4][i] = VNoise_[3][i];
          WNoise_[4][i] = WNoise_[3][i];

          UNoise_[3][i] = UNoise_[2][i];
          VNoise_[3][i] = VNoise_[2][i];
          WNoise_[3][i] = WNoise_[2][i];

          UNoise_[2][i] = UNoise_[1][i];
          VNoise_[2][i] = VNoise_[1][i];
          WNoise_[2][i] = WNoise_[1][i];
             
          UNoise_[1][i] = UNoise_[0][i];
          VNoise_[1][i] = VNoise_[0][i];
          WNoise_[1][i] = WNoise_[0][i];
     
       }  
              
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
          fread(&(UNoise_[0][i]), d_size, 1, InputADBFile_);
          fread(&(VNoise_[0][i]), d_size, 1, InputADBFile_);
          fread(&(WNoise_[0][i]), d_size, 1, InputADBFile_);
     
       }  
       
    }    
    
    else {
       
       printf("Unknown ADB read case: %d \n",TimeCase);
       fflush(NULL);
       exit(1);
       
    }
      
    // Loop over surfaces and read in solution

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {
  
       fread(&Cp,          f_size, 1, InputADBFile_); // Delta Cp, or CP
       fread(&Cp_Unsteady, f_size, 1, InputADBFile_); // Unsteady Delta Cp, or Cp
       fread(&Gamma,       f_size, 1, InputADBFile_); // Circulation strength

    }

    // Read in wake shape
    
    fread(&DumInt, i_size, 1, InputADBFile_);
 
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
      
       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          VortexSheet(k).TrailingVortexEdge(i).SkipReadInFile(InputADBFile_);

       }
       
    }     
    
    // Read in control surface deflection angles

    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          for ( k = 1 ; k <= VSPGeom().VSP_Surface(j).NumberOfControlSurfaces() ; k++ ) {

             fread(&DumFloat, f_size, 1, InputADBFile_); 

          }
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER InterpolateExistingSolution                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::InterpolateExistingSolution(double Time)
{
   
    int i, j, Edge, MaxSize;
    double *Result, DeltaCp;
    
    MaxSize = MAX(NumberOfVortexLoops_, NumberOfSurfaceVortexEdges_);
    
    Result = new double[MaxSize + 1];
    
    // Gamma

    InterpolateInTime(Time, GammaNoise_, Result, NumberOfVortexLoops_);
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) Gamma_[0][i] = Result[i];

    // Unsteady Cp and forces
   
    InterpolateInTime(Time, dCpUnsteadyNoise_, Result, NumberOfVortexLoops_);
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) VortexLoop(i).dCp_Unsteady() = Result[i];

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
     
       SurfaceVortexEdge(j).Unsteady_Fx() = 0.;
       SurfaceVortexEdge(j).Unsteady_Fy() = 0.;
       SurfaceVortexEdge(j).Unsteady_Fz() = 0.;
    
    }
     
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       DeltaCp = VortexLoop(i).dCp_Unsteady();
    
       for ( j = 1 ; j <= VortexLoop(i).NumberOfEdges() ; j++ ) {
          
          Edge = VortexLoop(i).Edge(j);
          
          SurfaceVortexEdge(Edge).Unsteady_Fx() -= DeltaCp * VortexLoop(i).Area() * VortexLoop(i).Nx()/VortexLoop(i).NumberOfEdges();
          SurfaceVortexEdge(Edge).Unsteady_Fy() -= DeltaCp * VortexLoop(i).Area() * VortexLoop(i).Ny()/VortexLoop(i).NumberOfEdges();
          SurfaceVortexEdge(Edge).Unsteady_Fz() -= DeltaCp * VortexLoop(i).Area() * VortexLoop(i).Nz()/VortexLoop(i).NumberOfEdges();
          
       }
      
    }

    // Unsteady velocities

    InterpolateInTime(Time, UNoise_, Result, NumberOfVortexLoops_);
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) VortexLoop(i).U() = Result[i];
    
    InterpolateInTime(Time, VNoise_, Result, NumberOfVortexLoops_);
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) VortexLoop(i).V() = Result[i];
    
    InterpolateInTime(Time, WNoise_, Result, NumberOfVortexLoops_);
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) VortexLoop(i).W() = Result[i];

    // Unsteady edge forces

    InterpolateInTime(Time, FxNoise_, Result, NumberOfSurfaceVortexEdges_);
    
    for ( i = 1 ; i <= NumberOfSurfaceVortexEdges_ ; i++ ) SurfaceVortexEdge(i).Fx() = Result[i];

    InterpolateInTime(Time, FyNoise_, Result, NumberOfSurfaceVortexEdges_);
    
    for ( i = 1 ; i <= NumberOfSurfaceVortexEdges_ ; i++ ) SurfaceVortexEdge(i).Fy() = Result[i];

    InterpolateInTime(Time, FzNoise_, Result, NumberOfSurfaceVortexEdges_);
    
    for ( i = 1 ; i <= NumberOfSurfaceVortexEdges_ ; i++ ) SurfaceVortexEdge(i).Fz() = Result[i];
    
    delete [] Result;
           
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER InterpolateTimeEdgeForces                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::InterpolateInTime(double Time, double **ArrayIn, double *ArrayOut, int NumValues)
{
   
   int i;
   double InterpTime[6], Wgt[5];
   double m0, m1, n0, n1, p0, p1, p, t;
   
   InterpTime[0] = CurrentTime_ +    TimeStep_;
   InterpTime[1] = CurrentTime_               ;
   InterpTime[2] = CurrentTime_ -    TimeStep_;
   InterpTime[3] = CurrentTime_ - 2.*TimeStep_;
   InterpTime[4] = CurrentTime_ - 3.*TimeStep_;
   InterpTime[5] = CurrentTime_ - 4.*TimeStep_;

   if ( Time > InterpTime[1] ) {
      
      for ( i = 1 ; i <= NumValues ; i++ ) {
         
         ArrayOut[i] = ArrayIn[1][i];
            
      }

   }
   
   else if ( Time < InterpTime[2] ) {
      
      for ( i = 1 ; i <= NumValues ; i++ ) {
         
         ArrayOut[i] = ArrayIn[2][i];
           
      }

   }
   
   else {

      // Linear interpolation
      
      if ( NoiseInterpolation_ == NOISE_LINEAR_INTERPOLATION ) {
  
         Wgt[0] = ( Time - InterpTime[2] ) / TimeStep_;
         Wgt[1] = 1. - Wgt[0];
   
         for ( i = 1 ; i <= NumValues ; i++ ) {
            
            ArrayOut[i] = Wgt[1] * ArrayIn[2][i] + Wgt[0] * ArrayIn[1][i];
    
         }
         
      }
      
      // Quadratic interpolation
      
      else if ( NoiseInterpolation_ == NOISE_QUADRATIC_INTERPOLATION ) {
         
         Wgt[2] = 0.5*( Time - InterpTime[2] ) * ( Time - InterpTime[1] ) / ( TimeStep_ * TimeStep_ );
         Wgt[1] =    -( Time - InterpTime[3] ) * ( Time - InterpTime[1] ) / ( TimeStep_ * TimeStep_ );
         Wgt[0] = 0.5*( Time - InterpTime[3] ) * ( Time - InterpTime[2] ) / ( TimeStep_ * TimeStep_ );

         for ( i = 1 ; i <= NumValues ; i++ ) {
            
            ArrayOut[i] = Wgt[2] * ArrayIn[3][i] + Wgt[1] * ArrayIn[2][i] + Wgt[0] * ArrayIn[1][i];
    
         }
                  
      }

      // Cubic interpolation
      
      // 0 3
      // 1 2
      // 2 1
      // 3 0
      
      else if ( NoiseInterpolation_ == NOISE_CUBIC_INTERPOLATION ) {
         
         Wgt[3] = -( Time - InterpTime[3] ) * ( Time - InterpTime[2] ) * ( Time - InterpTime[1] ) / ( TimeStep_ * TimeStep_ * TimeStep_ )/6.;
         Wgt[2] =  ( Time - InterpTime[4] ) * ( Time - InterpTime[2] ) * ( Time - InterpTime[1] ) / ( TimeStep_ * TimeStep_ * TimeStep_ )/2.;
         Wgt[1] = -( Time - InterpTime[4] ) * ( Time - InterpTime[3] ) * ( Time - InterpTime[1] ) / ( TimeStep_ * TimeStep_ * TimeStep_ )/2.;
         Wgt[0] =  ( Time - InterpTime[4] ) * ( Time - InterpTime[3] ) * ( Time - InterpTime[2] ) / ( TimeStep_ * TimeStep_ * TimeStep_ )/6.;

         for ( i = 1 ; i <= NumValues ; i++ ) {
            
            ArrayOut[i] = Wgt[3] * ArrayIn[4][i] + Wgt[2] * ArrayIn[3][i] + Wgt[1] * ArrayIn[2][i] + Wgt[0] * ArrayIn[1][i];
    
         }
                  
      }

      // Quartic interpolation
      
      else if ( NoiseInterpolation_ == NOISE_QUARTIC_INTERPOLATION ) {
         
         Wgt[4] =  ( Time - InterpTime[4] ) * ( Time - InterpTime[3] ) * ( Time - InterpTime[2] ) * ( Time - InterpTime[1] ) / ( TimeStep_ * TimeStep_ * TimeStep_ * TimeStep_ )/24.;  //  (4-3)*(4-2)*(4-1)*(4-0)    ( 1)( 2)( 3)( 4) =  24
         Wgt[3] = -( Time - InterpTime[5] ) * ( Time - InterpTime[3] ) * ( Time - InterpTime[2] ) * ( Time - InterpTime[1] ) / ( TimeStep_ * TimeStep_ * TimeStep_ * TimeStep_ )/6.;   //  (3-4)*(3-2)*(3-1)*(3-0)    (-1)( 1)( 2)( 3) = -6
         Wgt[2] =  ( Time - InterpTime[5] ) * ( Time - InterpTime[4] ) * ( Time - InterpTime[2] ) * ( Time - InterpTime[1] ) / ( TimeStep_ * TimeStep_ * TimeStep_ * TimeStep_ )/4.;   //  (2-4)*(2-3)*(2-1)*(2-0)    (-2)(-1)( 1)( 2) =  4
         Wgt[1] = -( Time - InterpTime[5] ) * ( Time - InterpTime[4] ) * ( Time - InterpTime[3] ) * ( Time - InterpTime[1] ) / ( TimeStep_ * TimeStep_ * TimeStep_ * TimeStep_ )/6.;   //  (1-4)*(1-3)*(1-2)*(1-0)    (-3)(-2)(-1)( 1) = -6
         Wgt[0] =  ( Time - InterpTime[5] ) * ( Time - InterpTime[4] ) * ( Time - InterpTime[3] ) * ( Time - InterpTime[2] ) / ( TimeStep_ * TimeStep_ * TimeStep_ * TimeStep_ )/24.;  //  (0-4)*(0-3)*(0-2)*(0-1)    (-4)(-3)(-2)(-1) =  24

         for ( i = 1 ; i <= NumValues ; i++ ) {
            
            ArrayOut[i] = Wgt[4] * ArrayIn[5][i] + Wgt[3] * ArrayIn[4][i] + Wgt[2] * ArrayIn[3][i] + Wgt[1] * ArrayIn[2][i] + Wgt[0] * ArrayIn[1][i];
    
         }
                  
      }

      // Cubic Hermite Spline
      
      else if ( NoiseInterpolation_ == NOISE_CUBIC_HERMITE_INTERPOLATION ) {
         
         for ( i = 1 ; i <= NumValues ; i++ ) {
            
            // Slopes
            
            m1 = 0.5*( ArrayIn[0][i] - ArrayIn[2][i] );
            m0 = 0.5*( ArrayIn[1][i] - ArrayIn[3][i] );
 
            // End values
            
            p1 = ArrayIn[1][i];
            p0 = ArrayIn[2][i];
            
            // 0-1 interval value
            
            t = ( Time - InterpTime[2] ) / TimeStep_;
            
            // Final interpolation
            
            p = ( 2.*t*t*t - 3.*t*t + 1)*p0 + (t*t*t - 2.*t*t + t)*m0 + (-2.*t*t*t + 3*t*t)*p1 + (t*t*t - t*t)*m1;
                           
            ArrayOut[i] = p;
    
         }
                  
      }

      // Quintic Hermite Spline
      
      else if ( NoiseInterpolation_ == NOISE_QUINTIC_HERMITE_INTERPOLATION ) {
         
         for ( i = 1 ; i <= NumValues ; i++ ) {
            
            // Slopes
            
            m1 = 0.5*( ArrayIn[0][i] - ArrayIn[2][i] );
            m0 = 0.5*( ArrayIn[1][i] - ArrayIn[3][i] );
            
            // 2nd derivatives

            n1 = ( ArrayIn[0][i] - 2.*ArrayIn[1][i] + ArrayIn[2][i] );
            n0 = ( ArrayIn[1][i] - 2.*ArrayIn[2][i] + ArrayIn[3][i] );

            // End values
            
            p1 = ArrayIn[1][i];
            p0 = ArrayIn[2][i];
            
            // 0-1 interval value
            
            t = ( Time - InterpTime[2] ) / TimeStep_;
            
            // Final interpolation
            
            p = ( 1.0                       - 10.0*t*t*t + 15.0*t*t*t*t - 6.0*t*t*t*t*t )*p0
              + (           t               -  6.0*t*t*t +  8.0*t*t*t*t - 3.0*t*t*t*t*t )*m0
              + (                   0.5*t*t -  1.5*t*t*t +  1.5*t*t*t*t - 0.5*t*t*t*t*t )*n0
              + (                              0.5*t*t*t -  1.0*t*t*t*t + 0.5*t*t*t*t*t )*n1
              + (                           -  4.0*t*t*t +  7.0*t*t*t*t - 3.0*t*t*t*t*t )*m1
              + (                             10.0*t*t*t - 15.0*t*t*t*t + 6.0*t*t*t*t*t )*p1;
                           
            ArrayOut[i] = p;
    
         }
                  
      }
                              
      else {
         
         printf("Unknown type of interpolation method in noise routines! \n");
         fflush(NULL);
         exit(1);
         
      }

   }

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER SavePeriodicNoiseSolution                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::SavePeriodicNoiseSolution(void)
{
   
    int i;
    
    // Save Gamma
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
          
       GammaNoise_[6][i] = GammaNoise_[0][i];
       GammaNoise_[7][i] = GammaNoise_[1][i];
       GammaNoise_[8][i] = GammaNoise_[2][i];

    }
    
    // Save the unsteady Cp
  
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
          
       dCpUnsteadyNoise_[6][i] = dCpUnsteadyNoise_[0][i];
       dCpUnsteadyNoise_[7][i] = dCpUnsteadyNoise_[1][i];
       dCpUnsteadyNoise_[8][i] = dCpUnsteadyNoise_[2][i];

    }
    
    // Save velocities
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
    
       UNoise_[6][i] = UNoise_[0][i];
       VNoise_[6][i] = VNoise_[0][i];        
       WNoise_[6][i] = WNoise_[0][i];           

       UNoise_[7][i] = UNoise_[1][i];
       VNoise_[7][i] = VNoise_[1][i];        
       WNoise_[7][i] = WNoise_[1][i];           

       UNoise_[8][i] = UNoise_[2][i];
       VNoise_[8][i] = VNoise_[2][i];        
       WNoise_[8][i] = WNoise_[2][i];           
    
    }    
    
    // Save edge forces

    for ( i = 1 ; i <= NumberOfSurfaceVortexEdges_ ; i++ ) {
       
       FxNoise_[6][i] = FxNoise_[0][i]; 
       FyNoise_[6][i] = FyNoise_[0][i]; 
       FzNoise_[6][i] = FzNoise_[0][i]; 

       FxNoise_[7][i] = FxNoise_[1][i]; 
       FyNoise_[7][i] = FyNoise_[1][i]; 
       FzNoise_[7][i] = FzNoise_[1][i]; 

       FxNoise_[8][i] = FxNoise_[2][i]; 
       FyNoise_[8][i] = FyNoise_[2][i]; 
       FzNoise_[8][i] = FzNoise_[2][i]; 
         
    }
      
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER RestorePeriodicNoiseSolution                  #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::RestorePeriodicNoiseSolution(void)
{
   
    int i;
    
    // Restore Gamma
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
          
       GammaNoise_[0][i] = GammaNoise_[6][i];
       GammaNoise_[1][i] = GammaNoise_[7][i];
       GammaNoise_[2][i] = GammaNoise_[8][i];

    }
    
    // Restore the unsteady Cp
  
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
          
       dCpUnsteadyNoise_[0][i] = dCpUnsteadyNoise_[6][i];
       dCpUnsteadyNoise_[1][i] = dCpUnsteadyNoise_[7][i];
       dCpUnsteadyNoise_[2][i] = dCpUnsteadyNoise_[8][i];

    }
    
    // Restore velocities
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
    
       UNoise_[0][i] = UNoise_[6][i];
       VNoise_[0][i] = VNoise_[6][i];        
       WNoise_[0][i] = WNoise_[6][i];           

       UNoise_[1][i] = UNoise_[7][i];
       VNoise_[1][i] = VNoise_[7][i];        
       WNoise_[1][i] = WNoise_[7][i];           

       UNoise_[2][i] = UNoise_[8][i];
       VNoise_[2][i] = VNoise_[8][i];        
       WNoise_[2][i] = WNoise_[8][i];           
    
    }    
    
    // Restore edge forces

    for ( i = 1 ; i <= NumberOfSurfaceVortexEdges_ ; i++ ) {
       
       FxNoise_[0][i] = FxNoise_[6][i]; 
       FyNoise_[0][i] = FyNoise_[6][i]; 
       FzNoise_[0][i] = FzNoise_[6][i]; 
                                 
       FxNoise_[1][i] = FxNoise_[7][i]; 
       FyNoise_[1][i] = FyNoise_[7][i]; 
       FzNoise_[1][i] = FzNoise_[7][i]; 
                                 
       FxNoise_[2][i] = FxNoise_[8][i]; 
       FyNoise_[2][i] = FyNoise_[8][i]; 
       FzNoise_[2][i] = FzNoise_[8][i]; 
         
    }
      
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

       fwrite(&(Gamma_[0][i]), d_size, 1, RestartFile);
       fwrite(&(Gamma_[1][i]), d_size, 1, RestartFile);
       fwrite(&(Gamma_[2][i]), d_size, 1, RestartFile);
       
    }    
    
    fclose(RestartFile);

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
     
       fread(&(Gamma_[0][i]), d_size, 1, RestartFile);
       fread(&(Gamma_[1][i]), d_size, 1, RestartFile);
       fread(&(Gamma_[2][i]), d_size, 1, RestartFile);
       
    }    
    
    fclose(RestartFile);
    
    // Read in the wake shape
  
}

/*##############################################################################
#                                                                              #
#            VSP_SOLVER CreateSurfaceVorticesInteractionList                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateSurfaceVorticesInteractionList(int LoopType)
{
 
    int i, j, k, p, cpu, Level, Loop, NumberOfEdges, CurrentLoop;
    int TestEdge, MaxInteractionLoops, MaxInteractionEdges, LoopOffSet, InteractionType;
    int Done, Found, TotalFound, CommonEdges, MaxLevels, **EdgeIsCommon;

    long long int TotalHits, NewHits;
    
    double xyz[3], Vec[3], Distance, Test;
    
    long double SpeedRatio;
    
    VSP_EDGE **TempInteractionList;
    LOOP_ENTRY **CommonEdgeList;
      
    // Allocate space for final interaction lists

    MaxInteractionLoops = MaxInteractionEdges = 0;
    
    for ( Level = 1 ; Level <= VSPGeom().NumberOfGridLevels() ; Level++ ) {
       
       MaxInteractionLoops += VSPGeom().Grid(Level).NumberOfLoops();
       
    }

    for ( Level = 1 ; Level <= VSPGeom().NumberOfGridLevels() ; Level++ ) {

       MaxInteractionEdges += VSPGeom().Grid(Level).NumberOfEdges();
       
    }
          
              
    if ( InteractionLoopList_[LoopType] != 0 ) delete [] InteractionLoopList_[LoopType];
    
    NumberOfInteractionLoops_[LoopType] = 0;

    InteractionLoopList_[LoopType] = new LOOP_INTERACTION_ENTRY[MaxInteractionLoops + 1];

    TotalHits = 0;
    
    SpeedRatio = 0.;
    
    if ( LoopType == FIXED_LOOPS ) printf("Creating interaction lists... \n\n");fflush(NULL);

    // Define which type of loops we are looking at...

    if ( LoopType == FIXED_LOOPS ) {
       
       if ( AllComponentsAreFixed_ ) {
          
          InteractionType = ALL_LOOPS;
          
       }
       
       else {
          
          InteractionType = NO_RELATIVE_MOTION;

       }
       
    }
    
    else if ( LoopType == MOVING_LOOPS ) {
       
       InteractionType = RELATIVE_MOTION;       
       
    }
    
    else {
       
       printf("Unknown type of loop in CreateSurfaceVorticesInteractionList! \n");
       exit(1);
       
    }
    
    // Forward sweep
    
    if ( LoopType == FIXED_LOOPS ) printf("Forward sweep... \n");
        
    NumberOfInteractionLoops_[LoopType] = NumberOfVortexLoops_;

#pragma omp parallel for reduction(+:TotalHits,SpeedRatio) private(xyz,TempInteractionList,NumberOfEdges,i) schedule(dynamic)
    for ( k = 1 ; k <= NumberOfVortexLoops_ ; k++ ) {
     
       if ( LoopType == FIXED_LOOPS && (k/1000)*1000 == k ) printf("%d / %d \r",k,NumberOfVortexLoops_);fflush(NULL);

       xyz[0] = VortexLoop(k).Xc();
       xyz[1] = VortexLoop(k).Yc();
       xyz[2] = VortexLoop(k).Zc();

       TempInteractionList = CreateInteractionList(VortexLoop(k).ComponentID(), k, InteractionType, xyz, NumberOfEdges);

       // Save the sorted list
      
       InteractionLoopList_[LoopType][k].Level() = 1;
      
       InteractionLoopList_[LoopType][k].Loop() = k;

       InteractionLoopList_[LoopType][k].SizeList(NumberOfEdges);
       
       for ( i = 1 ; i <= InteractionLoopList_[LoopType][k].NumberOfVortexEdges() ; i++ ) {

          InteractionLoopList_[LoopType][k].SurfaceVortexEdgeInteractionList()[i] = TempInteractionList[i];

       }       

       TotalHits += NumberOfEdges;
       
       SpeedRatio += (long double) (NumberOfEdges);

    }
    
    SpeedRatio = ( long double ) NumberOfVortexLoops_ / SpeedRatio;
    
    SpeedRatio *= NumberOfSurfaceVortexEdges_;

    if ( LoopType == FIXED_LOOPS ) printf("The Forward Speed Up Ratio: %Lf \n",SpeedRatio);
    
    // Backwards sweep
    
    if ( LoopType == FIXED_LOOPS ) printf("Backward sweep... \n");

    CommonEdgeList = new LOOP_ENTRY*[NumberOfThreads_ + 1];
    
    EdgeIsCommon = new int*[NumberOfThreads_ + 1];

    for ( cpu = 0 ; cpu < NumberOfThreads_ ; cpu++ ) {
       
       CommonEdgeList[cpu] = new LOOP_ENTRY[NumberOfVortexLoops_ + 1];
       
       EdgeIsCommon[cpu] = new int[MaxInteractionEdges + 1];
       
       zero_int_array(EdgeIsCommon[cpu], MaxInteractionEdges);
       
    }
    
    MaxLevels = VSPGeom().NumberOfGridLevels();

    NewHits = 0;
 
    LoopOffSet = 0;
    
    for ( Level = 2 ; Level <= MaxLevels ; Level++ ) {

#pragma omp parallel for reduction(+:NewHits) private(cpu,CurrentLoop,i,j,CommonEdges,TestEdge,TotalFound,Found,Done,xyz,Vec,Test,Distance,k,p,NumberOfEdges,TempInteractionList) schedule(dynamic)
       for ( Loop = 1 ; Loop <= VSPGeom().Grid(Level).NumberOfLoops() ; Loop++ ) {
       
#ifdef VSPAERO_OPENMP    
          cpu = omp_get_thread_num();
#else
          cpu = 0;
#endif         

          CurrentLoop = NumberOfInteractionLoops_[LoopType] + Loop;

          InteractionLoopList_[LoopType][CurrentLoop].Level() = Level;
          
          InteractionLoopList_[LoopType][CurrentLoop].Loop() = Loop;

          // Find common part of lists
          
          for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
             
             j = VSPGeom().Grid(Level).LoopList(Loop).FineGridLoop(i) + LoopOffSet;
             
             CommonEdgeList[cpu][i].NextEdge = 1;
     
             CommonEdgeList[cpu][i].Edge = InteractionLoopList_[LoopType][j].SurfaceVortexEdgeInteractionList();
             
             CommonEdgeList[cpu][i].NumberOfVortexEdges = InteractionLoopList_[LoopType][j].NumberOfVortexEdges();
             
          }

          CommonEdges = 0;
              
          while ( CommonEdgeList[cpu][1].NextEdge <= CommonEdgeList[cpu][1].NumberOfVortexEdges ) {
   
             TestEdge = CommonEdgeList[cpu][1].Edge[CommonEdgeList[cpu][1].NextEdge]->VortexEdge();
             
             TotalFound = 0;
             
             for ( i = 2 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
      
                Found = Done = 0;
                
                while ( CommonEdgeList[cpu][i].NextEdge <= CommonEdgeList[cpu][i].NumberOfVortexEdges && !Found && !Done ) {

                   // Found a common source loop
                   
                   if ( TestEdge == CommonEdgeList[cpu][i].Edge[CommonEdgeList[cpu][i].NextEdge]->VortexEdge() ) {
                      
                      Found = 1;
                      
                      CommonEdgeList[cpu][i].NextEdge++;
                      
                   }
               
                   // Not on this level
                   
                   else if ( CommonEdgeList[cpu][i].Edge[CommonEdgeList[cpu][i].NextEdge]->VortexEdge() < TestEdge ) {
                      
                      CommonEdgeList[cpu][i].NextEdge++;
                      
                   }
                   
                   // Nothing else to check
                   
                   else {
                      
                      Done = 1;
                      
                   }
                   
                }
                
                if ( Found ) TotalFound++;
              
             }
         
             if ( TotalFound > 0 && TotalFound == VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() - 1 ) {

                // Mid point of common source edge
                
                xyz[0] = CommonEdgeList[cpu][1].Edge[CommonEdgeList[cpu][1].NextEdge]->Xc();
                xyz[1] = CommonEdgeList[cpu][1].Edge[CommonEdgeList[cpu][1].NextEdge]->Yc();
                xyz[2] = CommonEdgeList[cpu][1].Edge[CommonEdgeList[cpu][1].NextEdge]->Zc();
                
                // Centroid of agglomerated loop for all the fine grid loops
                
                Vec[0] = xyz[0] - VSPGeom().Grid(Level).LoopList(Loop).Xc();
                Vec[1] = xyz[1] - VSPGeom().Grid(Level).LoopList(Loop).Yc();
                Vec[2] = xyz[2] - VSPGeom().Grid(Level).LoopList(Loop).Zc();
                      
                // Distance from centroid to source loop
                
                Distance = sqrt( SQR(Vec[0]) + SQR(Vec[1]) + SQR(Vec[2]) );
                
                // Find maximum distance from agglomerated loop centroid to fine grid loop centroids

                Test = 0.;
                
                for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {

                    // Centroid of agglomerated loop for all the fine grid loops
                    
                    Vec[0] = VSPGeom().Grid(Level).LoopList(Loop).Xc();
                    Vec[1] = VSPGeom().Grid(Level).LoopList(Loop).Yc();
                    Vec[2] = VSPGeom().Grid(Level).LoopList(Loop).Zc();
                
                    // Centroid of the j'th fine grid loop
                    
                    j = VSPGeom().Grid(Level).LoopList(Loop).FineGridLoop(i);
               
                    // Distance between fine grid and agglomerated grid centroids
                    
                    Vec[0] -= VSPGeom().Grid(Level-1).LoopList(j).Xc();
                    Vec[1] -= VSPGeom().Grid(Level-1).LoopList(j).Yc();
                    Vec[2] -= VSPGeom().Grid(Level-1).LoopList(j).Zc();
                
                    Test = MAX(Test,sqrt( SQR(Vec[0]) + SQR(Vec[1]) + SQR(Vec[2]) ));
                    
                }
            
                Test *= FarAway_;
                
                if ( Test <= Distance ) {
 
                   CommonEdges++;

                   EdgeIsCommon[cpu][CommonEdgeList[cpu][1].Edge[CommonEdgeList[cpu][1].NextEdge]->VortexEdge()] = 1;
                  
                }
               
             }
       
             CommonEdgeList[cpu][1].NextEdge++;
    
          }
    
          // Create a list of the common edges, and trim up the remaining edge lists
         
          if ( CommonEdges > 0 ) {
 
             // Create the common list
             
             InteractionLoopList_[LoopType][CurrentLoop].Level() = Level;
             
             InteractionLoopList_[LoopType][CurrentLoop].Loop() = Loop;
     
             InteractionLoopList_[LoopType][CurrentLoop].SizeList(CommonEdges);
             
             i = 1;
             
             j = 0;
        
             while ( j < CommonEdges && i <= CommonEdgeList[cpu][1].NumberOfVortexEdges ) {
         
               if ( EdgeIsCommon[cpu][CommonEdgeList[cpu][1].Edge[i]->VortexEdge()] == 1 ) {
              
                    InteractionLoopList_[LoopType][CurrentLoop].SurfaceVortexEdgeInteractionList()[++j] = CommonEdgeList[cpu][1].Edge[i];
                    
                }
                
                i++;
                
             }
             
             // Trim out the common edges from the initial lists
             
             for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
                
                j = VSPGeom().Grid(Level).LoopList(Loop).FineGridLoop(i) + LoopOffSet;
                
                NumberOfEdges = InteractionLoopList_[LoopType][j].NumberOfVortexEdges() - CommonEdges;
              
                // There are non-common edges remaining
                
                if ( NumberOfEdges > 0 ) {
                   
                   TempInteractionList = new VSP_EDGE*[NumberOfEdges + 1];
       
                   k = 0;
             
                   p = 1;
              
                   while ( k < NumberOfEdges && p <= InteractionLoopList_[LoopType][j].NumberOfVortexEdges() ) {

                      if ( EdgeIsCommon[cpu][InteractionLoopList_[LoopType][j].SurfaceVortexEdgeInteractionList(p)->VortexEdge()] == 0 ) {
 
                         TempInteractionList[++k] = InteractionLoopList_[LoopType][j].SurfaceVortexEdgeInteractionList(p);
                         
                      }
                      
                      p++;

                   }
 
                   InteractionLoopList_[LoopType][j].UseList(NumberOfEdges, TempInteractionList);
 
                }
                
                // There are no non-common edges remaining...
                
                else {

                   InteractionLoopList_[LoopType][j].Level() = 0;
                   
                   InteractionLoopList_[LoopType][j].Loop() = 0;

                   InteractionLoopList_[LoopType][j].DeleteList();
                 
                }
              
             }
                   
             // Unmark the common edges
             
             for ( j = 1 ; j <= InteractionLoopList_[LoopType][CurrentLoop].NumberOfVortexEdges() ; j++ ) {

                 EdgeIsCommon[cpu][ABS(InteractionLoopList_[LoopType][CurrentLoop].SurfaceVortexEdgeInteractionList(j)->VortexEdge())] =0;
   
             }

          }
     
          for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {

             NewHits += CommonEdgeList[cpu][i].NumberOfVortexEdges - CommonEdges;
             
          }
          
          if ( Level == MaxLevels ) NewHits += CommonEdges;
        
       }
 
       NumberOfInteractionLoops_[LoopType] += VSPGeom().Grid(Level).NumberOfLoops();
       
       LoopOffSet += VSPGeom().Grid(Level-1).NumberOfLoops();
       
    }
    
    SpeedRatio *= (long double) TotalHits / ( double) NewHits;

    if ( LoopType == FIXED_LOOPS ) printf("\nTotal Speed Up Ratio: %Lf \n\n\n",SpeedRatio);fflush(NULL);

    for ( cpu = 0 ; cpu < NumberOfThreads_ ; cpu++ ) {
       
       delete [] CommonEdgeList[cpu];
       
    }
    
    delete [] CommonEdgeList;

    for ( cpu = 0 ; cpu < NumberOfThreads_ ; cpu++ ) {
       
       delete [] EdgeIsCommon[cpu];
       
    }
    
    delete [] EdgeIsCommon;
        
    // Delete any zero length lists
    
    int NumberOfActualLoops = 0;
    
    for ( i = 1 ; i <= NumberOfInteractionLoops_[LoopType] ; i++ ) {
       
       if ( InteractionLoopList_[LoopType][i].NumberOfVortexEdges() > 0 ) {
          
          NumberOfActualLoops++;
          
       }
       
    }

    LOOP_INTERACTION_ENTRY *TempList;
    
    TempList = new LOOP_INTERACTION_ENTRY[NumberOfActualLoops + 1];
    
    j = 0;
    
    for ( i = 1 ; i <= NumberOfInteractionLoops_[LoopType] ; i++ ) {
       
       if ( InteractionLoopList_[LoopType][i].NumberOfVortexEdges() > 0 ) {
          
          TempList[++j] = InteractionLoopList_[LoopType][i];
          
       }
       
    }
    
    if ( j != NumberOfActualLoops ) {
       
       printf("Error in cleaning up interaction list! \n"); fflush(NULL);
       exit(1);
       
    }
    
    delete [] InteractionLoopList_[LoopType];
    
    NumberOfInteractionLoops_[LoopType] = NumberOfActualLoops;
    
    InteractionLoopList_[LoopType] = TempList;

}

/*##############################################################################
#                                                                              #
#                 VSP_SOLVER UpdateWakeVortexInteractionLists                  #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::UpdateWakeVortexInteractionLists(void)
{
   
    int v, w, t, p, q, k, cpu;
    
    // Vortex to surface interaction lists
    
    if ( NumberOfVortexSheetInteractionLoops_ != NULL ) {
       
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {

          delete [] VortexSheetInteractionLoopList_[v];

       }
       
       delete [] VortexSheetInteractionLoopList_;
          
       delete [] NumberOfVortexSheetInteractionLoops_;
       
    }
    
    NumberOfVortexSheetInteractionLoops_ = new int[NumberOfVortexSheets_ + 1];
    
    VortexSheetInteractionLoopList_ = new VORTEX_SHEET_LOOP_INTERACTION_ENTRY*[NumberOfVortexSheets_ + 1];

    for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
       
       VortexSheetInteractionLoopList_[v] = CreateVortexSheetInteractionList(v, NumberOfVortexSheetInteractionLoops_[v]);

    }

    // Vortex to Vortex interaction lists
    
    if ( VortexSheetVortexToVortexSet_ != NULL ) {
       
       delete [] VortexSheetVortexToVortexSet_;
       
    }
       
    VortexSheetVortexToVortexSet_ = new VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET[NumberOfVortexSheets_ + 1];

    for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
       
       p = 0;
       
       for ( w = 1 ; w <= NumberOfVortexSheets_ ; w++ ) {
 
          for ( t = 1 ; t <= VortexSheet(w).NumberOfTrailingVortices() ; t++ ) {
             
             p++;
             
          }
          
       }

       VortexSheetVortexToVortexSet_[v].SizeList(p);
       
    }

    // Copy over vortex sheet data for parallel runs
    
    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {

       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
          VortexSheet_[cpu][k] += VortexSheet_[0][k];
    
       }  
       
    } 

    for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {

       p = q = 0;
       
       for ( w = 1 ; w <= NumberOfVortexSheets_ ; w++ ) {
#pragma omp parallel for private(p) schedule(dynamic)
          for ( t = 1 ; t <= VortexSheet(w).NumberOfTrailingVortices() ; t++ ) {

             p = q + t;

             VortexSheetVortexToVortexSet_[v].VortexW(p) = w;
        
             VortexSheetVortexToVortexSet_[v].TrailingVortexT(p) = t;        
             
             VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p) = CreateVortexTrailInteractionList(v, w, t, VortexSheetVortexToVortexSet_[v].NumberOfVortexSheetInteractionEdges(p));   

          }
          
          q += VortexSheet(w).NumberOfTrailingVortices();

       }

    }

}

/*##############################################################################
#                                                                              #
#               VSP_SOLVER CreateVortexSheetInteractionList                    #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_LOOP_INTERACTION_ENTRY* VSP_SOLVER::CreateVortexSheetInteractionList(int v, int &NumberOfVortexSheetInteractionLoops)
{
 
    int i, j, k,  p, Level, Loop, FullEval, DoCheck;
    int TestSheet, MaxInteractionLoops, LoopOffSet;
    int Done, Found, TotalFound, CommonSheets, NumberOfSheets, MaxLevels;

    long long int NewHits, TotalHits;
    
    double xyz[3], Vec[3], Distance, Test;
    
    long double SpeedRatio;
    
    VORTEX_SHEET_LOOP_INTERACTION_ENTRY *VortexsheetInteractionLoopList;
    VORTEX_SHEET_ENTRY *TempInteractionList;
    VORTEX_SHEET_LIST *CommonSheetList;
      
    if ( Verbose_ ) printf("Creating vortex sheet to surface interaction lists... \n\n");fflush(NULL);
    
    // Allocate space for final interaction lists

    MaxInteractionLoops  = 0;
    
    for ( Level = 1 ; Level <= VSPGeom().NumberOfGridLevels() ; Level++ ) {
       
       MaxInteractionLoops += VSPGeom().Grid(Level).NumberOfLoops();
       
    }
    
    MaxInteractionLoops *= NumberOfVortexSheets_;

    NumberOfVortexSheetInteractionLoops = 0;

    VortexsheetInteractionLoopList = new VORTEX_SHEET_LOOP_INTERACTION_ENTRY[MaxInteractionLoops + 1];

    DoCheck = 0;
    
    TotalHits = 0;
    
    FullEval = 0;

    // Forward sweep

    for ( k = 1 ; k <= NumberOfVortexLoops_ ; k++ ) {
       
       if ( Verbose_ && (k/1000)*1000 == k ) printf("%d / %d \r",k,NumberOfVortexLoops_);fflush(NULL);

       xyz[0] = VortexLoop(k).Xc();
       xyz[1] = VortexLoop(k).Yc();
       xyz[2] = VortexLoop(k).Zc();

       TempInteractionList = VortexSheet(v).CreateInteractionSheetList(xyz, NumberOfSheets); 

       // Save the sorted list

       VortexsheetInteractionLoopList[k].Level() = 1;
      
       VortexsheetInteractionLoopList[k].Loop() = k;

       VortexsheetInteractionLoopList[k].SizeList(NumberOfSheets);
       
       for ( i = 1 ; i <= VortexsheetInteractionLoopList[k].NumberOfVortexSheets() ; i++ ) {

          VortexsheetInteractionLoopList[k].VortexSheetList(i).Level    = TempInteractionList[i].Level;
          VortexsheetInteractionLoopList[k].VortexSheetList(i).Sheet    = TempInteractionList[i].Sheet;
          VortexsheetInteractionLoopList[k].VortexSheetList(i).SheetID  = TempInteractionList[i].SheetID;
          VortexsheetInteractionLoopList[k].VortexSheetList(i).Distance = TempInteractionList[i].Distance;

       }
       
       delete [] TempInteractionList; 

       TotalHits += NumberOfSheets;

    }
    
    FullEval += NumberOfVortexLoops_ * (VortexSheet(v).NumberOfTrailingVortices() - 1);

    NumberOfVortexSheetInteractionLoops = NumberOfVortexLoops_;

    SpeedRatio = (long double) TotalHits;
    
    SpeedRatio = FullEval / SpeedRatio;

    if ( Verbose_ ) printf("Surface/Wake: Forward Speed Up Ratio: %Lf \n",SpeedRatio);
    
    // Backwards sweep
    
    if ( Verbose_ ) printf("Surface/Wake Backward sweep... \n");

    CommonSheetList = new VORTEX_SHEET_LIST[NumberOfVortexLoops_ + 1];

    MaxLevels = VSPGeom().NumberOfGridLevels();

    NewHits = 0;
 
    LoopOffSet = 0;
    
    for ( Level = 2 ; Level <= MaxLevels ; Level++ ) {

       for ( Loop = 1 ; Loop <= VSPGeom().Grid(Level).NumberOfLoops() ; Loop++ ) {
          
          NumberOfVortexSheetInteractionLoops++;
        
          VortexsheetInteractionLoopList[NumberOfVortexSheetInteractionLoops].Level() = Level;
          
          VortexsheetInteractionLoopList[NumberOfVortexSheetInteractionLoops].Loop() = Loop;

          // Find common part of lists
          
          for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
             
             j = VSPGeom().Grid(Level).LoopList(Loop).FineGridLoop(i) + LoopOffSet;
             
             CommonSheetList[i].NextSheet = 1;
     
             CommonSheetList[i].Sheet = VortexsheetInteractionLoopList[j].VortexSheetList_;
             
             CommonSheetList[i].NumberOfSheets = VortexsheetInteractionLoopList[j].NumberOfVortexSheets();
             
          }
             
          CommonSheets = 0;
              
          while ( CommonSheetList[1].NextSheet <= CommonSheetList[1].NumberOfSheets ) {
   
             TestSheet = CommonSheetList[1].Sheet[CommonSheetList[1].NextSheet].SheetID;
             
             TotalFound = 0;
             
             for ( i = 2 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
      
                Found = Done = 0;
                
                while ( CommonSheetList[i].NextSheet <= CommonSheetList[i].NumberOfSheets && !Found && !Done ) {

                   if ( TestSheet == CommonSheetList[i].Sheet[CommonSheetList[i].NextSheet].SheetID ) {
                      
                      Found = 1;
                      
                      CommonSheetList[i].NextSheet++;
                      
                   }
               
                   else if ( CommonSheetList[i].Sheet[CommonSheetList[i].NextSheet].SheetID < TestSheet ) {
                      
                      CommonSheetList[i].NextSheet++;
                      
                   }
                   
                   else {
                      
                      Done = 1;
                      
                   }
                   
                }
                
                if ( Found ) TotalFound++;
              
             }
             
             // If we found the same sheet in all the fine grid loops...
            
             if ( TotalFound > 0 && TotalFound == VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() - 1 ) {

                Distance = CommonSheetList[1].Sheet[CommonSheetList[1].NextSheet].Distance;

                for ( i = 2 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {

                   Distance = MIN(CommonSheetList[i].Sheet[CommonSheetList[i].NextSheet-1].Distance, Distance);

                }
               
                // Find maximum distance from agglomerated loop centroid to fine grid loop centroids
       
                Test = 0.;
                
                for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {

                    // Centroid of agglomerated loop for all the fine grid loops
                    
                    Vec[0] = VSPGeom().Grid(Level).LoopList(Loop).Xc();
                    Vec[1] = VSPGeom().Grid(Level).LoopList(Loop).Yc();
                    Vec[2] = VSPGeom().Grid(Level).LoopList(Loop).Zc();
                
                    // Centroid of the j'th fine grid loop
                    
                    j = VSPGeom().Grid(Level).LoopList(Loop).FineGridLoop(i);
               
                    // Distance between fine grid and agglomerated grid centroids
                    
                    Vec[0] -= VSPGeom().Grid(Level-1).LoopList(j).Xc();
                    Vec[1] -= VSPGeom().Grid(Level-1).LoopList(j).Yc();
                    Vec[2] -= VSPGeom().Grid(Level-1).LoopList(j).Zc();
                
                    Test = MAX(Test,sqrt( SQR(Vec[0]) + SQR(Vec[1]) + SQR(Vec[2]) ));
                    
                }
              
                Test *= FarAway_;
                
                if ( Test <= Distance ) {
 
                   CommonSheets++;

                   CommonSheetList[1].Sheet[CommonSheetList[1].NextSheet].SheetID *= -1;
          
                   for ( i = 2 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
                 
                      CommonSheetList[i].Sheet[CommonSheetList[i].NextSheet-1].SheetID  *= -1;

                   }
                 
                }
               
             }
          
             CommonSheetList[1].NextSheet++;
    
          }
         
          // Create a list of the common sheets, and trim up the remaining edge lists
         
          if ( CommonSheets > 0 ) {

             // Create the common list
             
             VortexsheetInteractionLoopList[NumberOfVortexSheetInteractionLoops].Level() = Level;
             
             VortexsheetInteractionLoopList[NumberOfVortexSheetInteractionLoops].Loop() = Loop;
     
             VortexsheetInteractionLoopList[NumberOfVortexSheetInteractionLoops].SizeList(CommonSheets);
             
             i = 1;
             
             j = 0;
             
             while ( j < CommonSheets && i <= CommonSheetList[1].NumberOfSheets ) {
    
                if ( CommonSheetList[1].Sheet[i].SheetID < 0 ) {
                 
                    j++;
                    
                    VortexsheetInteractionLoopList[NumberOfVortexSheetInteractionLoops].VortexSheetList_[j] = CommonSheetList[1].Sheet[i];
                    
                    VortexsheetInteractionLoopList[NumberOfVortexSheetInteractionLoops].VortexSheetList_[j].SheetID *= -1;
                }
                
                i++;
                
             }
            
             // Trim out the common edges from the initial lists
             
             for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
                
                j = VSPGeom().Grid(Level).LoopList(Loop).FineGridLoop(i) + LoopOffSet;
                
                NumberOfSheets = VortexsheetInteractionLoopList[j].NumberOfVortexSheets() - CommonSheets;
              
                // There are non-common edges remaining
                
                if ( NumberOfSheets > 0 ) {
                 
                   TempInteractionList = new VORTEX_SHEET_ENTRY[NumberOfSheets + 1];
       
                   k = 0;
             
                   p = 1;
              
                   while ( k < NumberOfSheets && p <= VortexsheetInteractionLoopList[j].NumberOfVortexSheets() ) {

                      if ( VortexsheetInteractionLoopList[j].VortexSheetList(p).SheetID > 0 ) {
   
                         TempInteractionList[++k] = VortexsheetInteractionLoopList[j].VortexSheetList(p);

                      }
                      
                      p++;

                   }
 
                   VortexsheetInteractionLoopList[j].UseList(NumberOfSheets, TempInteractionList);
 
                }
                
                // There are no non-common edges remaining...
                
                else {

                   VortexsheetInteractionLoopList[j].Level() = 0;
                   
                   VortexsheetInteractionLoopList[j].Loop() = 0;

                   VortexsheetInteractionLoopList[j].DeleteList();
                 
                }
              
             }

          }
          
          for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {

             NewHits += CommonSheetList[i].NumberOfSheets - CommonSheets;
             
          }
          
          if ( Level == MaxLevels ) NewHits += CommonSheets;

        
       }

       LoopOffSet += VSPGeom().Grid(Level-1).NumberOfLoops();
       
    }
  
    SpeedRatio *= (long double) TotalHits / (double) NewHits;

    if ( Verbose_ ) printf("\nSurface/Wake: Total Speed Up Ratio: %Lf \n\n\n",SpeedRatio);fflush(NULL);

    delete [] CommonSheetList;
    
    // Delete any zero length lists
    
    int NumberOfActualLoops = 0;
    
    for ( i = 1 ; i <= NumberOfVortexSheetInteractionLoops ; i++ ) {
       
       if ( VortexsheetInteractionLoopList[i].NumberOfVortexSheets() > 0 ) {
          
          NumberOfActualLoops++;
          
       }
       
    }

    VORTEX_SHEET_LOOP_INTERACTION_ENTRY *TempList;
    
    TempList = new VORTEX_SHEET_LOOP_INTERACTION_ENTRY[NumberOfActualLoops + 1];
    
    j = 0;
    
    for ( i = 1 ; i <= NumberOfVortexSheetInteractionLoops ; i++ ) {
       
       if ( VortexsheetInteractionLoopList[i].NumberOfVortexSheets() > 0 ) {
          
          TempList[++j] = VortexsheetInteractionLoopList[i];
          
       }
       
    }
    
    if ( j != NumberOfActualLoops ) {
       
       printf("Error in cleaning up interaction list! \n"); fflush(NULL);
       exit(1);
       
    }
    
    delete [] VortexsheetInteractionLoopList;
    
    NumberOfVortexSheetInteractionLoops = NumberOfActualLoops;
    
    VortexsheetInteractionLoopList = TempList;
    
    return VortexsheetInteractionLoopList;

}

/*##############################################################################
#                                                                              #
#               VSP_SOLVER CreateVortexTrailInteractionList                    #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_LOOP_INTERACTION_ENTRY* VSP_SOLVER::CreateVortexTrailInteractionList(int v, int w, int t, int &NumberOfVortexSheetInteractionEdges)
{
 
    int i, j, k, p, Level, Loop, TotalHits, FullEval, DoCheck, cpu;
    int TestSheet, MaxInteractionEdges, LoopOffSet;
    int Done, Found, CommonSheets, NumberOfSheets, NumSubVortices;
    double xyz[3], Distance, Test, SpeedRatio;
    VORTEX_SHEET_LOOP_INTERACTION_ENTRY *VortexSheetInteractionEdgeList;
    VORTEX_SHEET_ENTRY *TempInteractionList;
    VORTEX_SHEET_LIST *CommonSheetList;

#ifdef VSPAERO_OPENMP    
    cpu = omp_get_thread_num();
#else
    cpu = 0;
#endif  

    if ( Verbose_ ) printf("Creating vortex sheet to vortex sheet interaction lists... \n\n");fflush(NULL);
    
    // Allocate space for final interaction lists

    MaxInteractionEdges = VortexSheet(w).TrailingVortexEdge(t).TotalNumberOfSubVortices();

    if ( Verbose_ ) printf("MaxInteractionEdges: %d \n",MaxInteractionEdges);

    VortexSheetInteractionEdgeList = new VORTEX_SHEET_LOOP_INTERACTION_ENTRY[MaxInteractionEdges + 1];

    DoCheck = 0;
    
    TotalHits = 0;
    
    FullEval = 0;

    // Forward sweep
    
    NumSubVortices = VortexSheet(w).TrailingVortexEdge(t).NumberOfSubVortices();

    if ( TimeAccurate_ ) NumSubVortices = MIN(WakeStartingTime_ + Time_ + 1, VortexSheet(w).TrailingVortexEdge(t).NumberOfSubVortices());

    for ( k = 1 ; k <= NumSubVortices ; k++ ) {

       xyz[0] = VortexSheet(w).TrailingVortexEdge(t).VortexEdge(k).Xc();
       xyz[1] = VortexSheet(w).TrailingVortexEdge(t).VortexEdge(k).Yc();
       xyz[2] = VortexSheet(w).TrailingVortexEdge(t).VortexEdge(k).Zc();

       if ( Verbose_ && (k/1000)*1000 == k ) printf("%d / %d \r",k,NumberOfVortexLoops_);fflush(NULL);

       TempInteractionList = VortexSheet(cpu,v).CreateInteractionSheetList(xyz, NumberOfSheets); 

       // Save the sorted list
       
       VortexSheetInteractionEdgeList[k].Level() = 1;             
             
       VortexSheetInteractionEdgeList[k].TrailingVortexEdge() = t;

       VortexSheetInteractionEdgeList[k].Edge() = k;

       VortexSheetInteractionEdgeList[k].SizeList(NumberOfSheets);
       
       for ( i = 1 ; i <= VortexSheetInteractionEdgeList[k].NumberOfVortexSheets() ; i++ ) {

          VortexSheetInteractionEdgeList[k].VortexSheetList(i).Level    = TempInteractionList[i].Level;
          VortexSheetInteractionEdgeList[k].VortexSheetList(i).Sheet    = TempInteractionList[i].Sheet;
          VortexSheetInteractionEdgeList[k].VortexSheetList(i).SheetID  = TempInteractionList[i].SheetID;
          VortexSheetInteractionEdgeList[k].VortexSheetList(i).Distance = TempInteractionList[i].Distance;

       }       

       delete [] TempInteractionList; 

       TotalHits += NumberOfSheets;
       
    }

    FullEval = VortexSheet(w).TrailingVortexEdge(t).NumberOfSubVortices() * ( VortexSheet(v).NumberOfTrailingVortices() - 1);

    NumberOfVortexSheetInteractionEdges = VortexSheet(w).TrailingVortexEdge(t).NumberOfSubVortices();

    SpeedRatio = (double) TotalHits;
   
    SpeedRatio = FullEval / SpeedRatio;

    if ( Verbose_ ) printf("Wake/Wake: Forward Speed Up Ratio: %f \n",SpeedRatio);
  
    // Backwards sweep
 
    if ( Verbose_ ) printf("Wake/Wake: Backward sweep... \n");

    CommonSheetList = new VORTEX_SHEET_LIST[MaxInteractionEdges + 1];

    int MaxLevels = VortexSheet(w).TrailingVortexEdge(t).NumberOfLevels();

    int NewHits = 0;
 
    LoopOffSet = 0;
    
    if ( Verbose_ ) printf("MaxLevels: %d \n",MaxLevels);

    for ( Level = 2 ; Level <= MaxLevels ; Level++ ) {
       
       NumSubVortices = VortexSheet(w).TrailingVortexEdge(t).NumberOfSubVortices(Level);

       if ( TimeAccurate_ ) NumSubVortices = MIN( WakeStartingTime_ + Time_ + 1, VortexSheet(w).TrailingVortexEdge(t).NumberOfSubVortices(Level));

       for ( Loop = 1 ; Loop <= NumSubVortices ; Loop++ ) {
          
          NumberOfVortexSheetInteractionEdges++;
        
          VortexSheetInteractionEdgeList[NumberOfVortexSheetInteractionEdges].Level() = Level;
          
          VortexSheetInteractionEdgeList[NumberOfVortexSheetInteractionEdges].Loop() = Loop;

          // Find common part of lists
    
          for ( i = 1 ; i <= 2 ; i++ ) {

             j = 2 * Loop - 2 + i + LoopOffSet;
      
             CommonSheetList[i].NextSheet = 1;
     
             CommonSheetList[i].Sheet = VortexSheetInteractionEdgeList[j].VortexSheetList_;
             
             CommonSheetList[i].NumberOfSheets = VortexSheetInteractionEdgeList[j].NumberOfVortexSheets();
             
          }
             
          CommonSheets = 0;
              
          while ( CommonSheetList[1].NextSheet <= CommonSheetList[1].NumberOfSheets ) {

             TestSheet = CommonSheetList[1].Sheet[CommonSheetList[1].NextSheet].SheetID;

             Found = Done = 0;
             
             while ( CommonSheetList[2].NextSheet <= CommonSheetList[2].NumberOfSheets && !Found && !Done ) {

                if ( TestSheet == CommonSheetList[2].Sheet[CommonSheetList[2].NextSheet].SheetID ) {
                   
                   Found = 1;
                   
                   CommonSheetList[2].NextSheet++;
                   
                }
            
                else if ( CommonSheetList[2].Sheet[CommonSheetList[2].NextSheet].SheetID < TestSheet ) {
                   
                   CommonSheetList[2].NextSheet++;
                   
                }
                
                else {
                   
                   Done = 1;
                   
                }
                
             }
        
             // If we found the same sheet in both lists
            
             if ( Found ) {

                // Distance to the source vortex sheet
                
                 Distance = MIN( CommonSheetList[1].Sheet[CommonSheetList[1].NextSheet].Distance,
                                 CommonSheetList[2].Sheet[CommonSheetList[2].NextSheet - 1].Distance );

                // FarAway x approximate distance between fine grid trailing vortex edge centroids
                
                Test = 0.5 * FarAway_ * VortexSheet(w).TrailingVortexEdge(t).VortexEdge(Level,Loop).Length();

                 if ( Test <= Distance ) {

                   CommonSheets++;

                   CommonSheetList[1].Sheet[CommonSheetList[1].NextSheet  ].SheetID *= -1;
          
                   CommonSheetList[2].Sheet[CommonSheetList[2].NextSheet-1].SheetID *= -1;

                }
               
             }
       
             CommonSheetList[1].NextSheet++;
    
          }
        
          // Create a list of the common sheets, and trim up the remaining edge lists
         
          if ( CommonSheets > 0 ) {

             // Create the common list
             
             VortexSheetInteractionEdgeList[NumberOfVortexSheetInteractionEdges].Level() = Level;
             
             VortexSheetInteractionEdgeList[NumberOfVortexSheetInteractionEdges].Loop() = Loop;
     
             VortexSheetInteractionEdgeList[NumberOfVortexSheetInteractionEdges].SizeList(CommonSheets);
    
             i = 1;
             
             j = 0;
             
             while ( j < CommonSheets && i <= CommonSheetList[1].NumberOfSheets ) {
    
                if ( CommonSheetList[1].Sheet[i].SheetID < 0 ) {
                 
                    j++;
                    
                    VortexSheetInteractionEdgeList[NumberOfVortexSheetInteractionEdges].VortexSheetList_[j] = CommonSheetList[1].Sheet[i];
                    
                    VortexSheetInteractionEdgeList[NumberOfVortexSheetInteractionEdges].VortexSheetList_[j].SheetID *= -1;
                }
                
                i++;
                
             }
          
             // Trim out the common edges from the initial lists
             
             for ( i = 1 ; i <= 2 ; i++ ) {
             
                j = 2 * Loop - 2 + i + LoopOffSet;
     
                NumberOfSheets = VortexSheetInteractionEdgeList[j].NumberOfVortexSheets() - CommonSheets;
           
                // There are non-common edges remaining
          
                if ( NumberOfSheets > 0 ) {
   
                   TempInteractionList = new VORTEX_SHEET_ENTRY[NumberOfSheets + 1];           
      
                   k = 0;
             
                   p = 1;
              
                   while ( k < NumberOfSheets && p <= VortexSheetInteractionEdgeList[j].NumberOfVortexSheets() ) {

                      if ( VortexSheetInteractionEdgeList[j].VortexSheetList(p).SheetID > 0 ) {

                         TempInteractionList[++k] = VortexSheetInteractionEdgeList[j].VortexSheetList(p);

                      }
                      
                      p++;

                   }
 
                   VortexSheetInteractionEdgeList[j].UseList(NumberOfSheets, TempInteractionList);
 
                }
                
                // There are no non-common edges remaining...
                
                else {

                   VortexSheetInteractionEdgeList[j].Level() = 0;
                   
                   VortexSheetInteractionEdgeList[j].Loop() = 0;

                   VortexSheetInteractionEdgeList[j].DeleteList();
                 
                }
              
             }

          }

          for ( i = 1 ; i <= 2 ; i++ ) {

             NewHits += CommonSheetList[i].NumberOfSheets - CommonSheets;
             
          }
          
          if ( Level == MaxLevels ) NewHits += CommonSheets;

        
       }

       LoopOffSet += VortexSheet(w).TrailingVortexEdge(t).NumberOfSubVortices(Level-1);

    }

    SpeedRatio *= (double) TotalHits / (double) NewHits;

    if ( Verbose_ ) printf("\n Wake/Wake: Total Speed Up Ratio: %lf  \n\n\n",SpeedRatio);fflush(NULL);

    delete [] CommonSheetList;
    
    // Delete any zero length lists
    
    int NumberOfActualLoops = 0;
    
    for ( i = 1 ; i <= NumberOfVortexSheetInteractionEdges ; i++ ) {
       
       if ( VortexSheetInteractionEdgeList[i].NumberOfVortexSheets() > 0 ) {
          
          NumberOfActualLoops++;
          
       }
       
    }

    VORTEX_SHEET_LOOP_INTERACTION_ENTRY *TempList;
    
    TempList = new VORTEX_SHEET_LOOP_INTERACTION_ENTRY[NumberOfActualLoops + 1];
    
    j = 0;
    
    for ( i = 1 ; i <= NumberOfVortexSheetInteractionEdges ; i++ ) {
       
       if ( VortexSheetInteractionEdgeList[i].NumberOfVortexSheets() > 0 ) {
          
          TempList[++j] = VortexSheetInteractionEdgeList[i];
          
       }
       
    }
    
    if ( j != NumberOfActualLoops ) {
       
       printf("Error in cleaning up interaction list! \n"); fflush(NULL);
       exit(1);
       
    }
    
    delete [] VortexSheetInteractionEdgeList;
    
    NumberOfVortexSheetInteractionEdges = NumberOfActualLoops;
    
    VortexSheetInteractionEdgeList = TempList;
  
    return VortexSheetInteractionEdgeList;

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

    InteractionList = CreateInteractionList(0, 0, ALL_LOOPS, xyz, NumberOfEdges);

    U = V = W = 0.;

    for ( j = 1 ; j <= NumberOfEdges ; j++ ) {
     
       VortexEdge = InteractionList[j];

       VortexEdge->InducedVelocity(xyz, dq);
   
       U += dq[0];
       V += dq[1];
       W += dq[2];

    }
    
    q[0] = U;
    q[1] = V;
    q[2] = W;    

}

/*##############################################################################
#                                                                              #
#            VSP_SOLVER CalculateSurfaceInducedVelocityAtPoint                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateSurfaceInducedVelocityAtPoint(int ComponentID, int pLoop, double xyz[3], double q[3])
{
 
    int j, NumberOfEdges;
    double U, V, W, dq[3];
    VSP_EDGE **InteractionList, *VortexEdge;
     
    // Create interaction list for this xyz location

    InteractionList = CreateInteractionList(ComponentID, pLoop, ALL_LOOPS, xyz, NumberOfEdges);

    U = V = W = 0.;

    for ( j = 1 ; j <= NumberOfEdges ; j++ ) {
     
       VortexEdge = InteractionList[j];

       VortexEdge->InducedVelocity(xyz, dq);
   
       U += dq[0];
       V += dq[1];
       W += dq[2];

    }
    
    q[0] = U;
    q[1] = V;
    q[2] = W;    

}

/*##############################################################################
#                                                                              #
#            VSP_SOLVER CalculateSurfaceInducedVelocityAtPoint                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateSurfaceInducedVelocityAtPoint(double xyz[3], double q[3], double CoreWidth)
{
 
    int j, NumberOfEdges;
    double U, V, W, dq[3];
    VSP_EDGE **InteractionList, *VortexEdge;
     
    // Create interaction list for this xyz location

    InteractionList = CreateInteractionList(0, 0, ALL_LOOPS, xyz, NumberOfEdges);

    U = V = W = 0.;
 
    for ( j = 1 ; j <= NumberOfEdges ; j++ ) {
     
       // Calculate induced velocity from this edge
       
       VortexEdge = InteractionList[j];

       VortexEdge->InducedVelocity(xyz, dq, CoreWidth);
       
       // Add in induced velocity

       U += dq[0];
       V += dq[1];
       W += dq[2];

    }

    q[0] = U;
    q[1] = V;
    q[2] = W;    
    
    // Testing... AdjustNearSurfaceVelocities(xyz, q, NumberOfEdges, InteractionList);
           
}

/*##############################################################################
#                                                                              #
#                 VSP_SOLVER AdjustNearSurfaceVelocities                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::AdjustNearSurfaceVelocities(double xyz[3], double q[3], int NumberOfEdges, VSP_EDGE **InteractionList)
{
 
    int j;
    double Vec1[3], Vec2[3], qVec[3], Normal[3], Dot, Distance, Ratio, Angle;
    VSP_EDGE *VortexEdge;
 
    // Adjust velocities near panel model surfaces
    
    if ( ModelType_ == PANEL_MODEL && !TimeAccurate_ ) {
 
       qVec[0] = q[0];
       qVec[1] = q[1];
       qVec[2] = q[2];
       
       if ( DoSymmetryPlaneSolve_ == SYM_X && xyz[0] < 0. ) qVec[0] *= -1.;
       if ( DoSymmetryPlaneSolve_ == SYM_Y && xyz[1] < 0. ) qVec[1] *= -1.;
       if ( DoSymmetryPlaneSolve_ == SYM_Z && xyz[2] < 0. ) qVec[2] *= -1.;

       for ( j = 1 ; j <= NumberOfEdges ; j++ ) {
          
          VortexEdge = InteractionList[j];
          
          // Are we on the finest grid level
  
          if ( VortexEdge->Level() <= 1 ) {

             // Calculate distance to surface

             Vec1[0] = xyz[0];
             Vec1[1] = xyz[1];
             Vec1[2] = xyz[2];

             Vec2[0] = VortexEdge->Xc();
             Vec2[1] = VortexEdge->Yc();
             Vec2[2] = VortexEdge->Zc();

             if ( DoSymmetryPlaneSolve_ == SYM_X && xyz[0] < 0. ) Vec1[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y && xyz[1] < 0. ) Vec1[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z && xyz[2] < 0. ) Vec1[2] *= -1.;

             if ( DoSymmetryPlaneSolve_ == SYM_X && xyz[0] < 0. ) Vec2[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y && xyz[1] < 0. ) Vec2[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z && xyz[2] < 0. ) Vec2[2] *= -1.;
 
             Vec1[0] -= Vec2[0];
             Vec1[1] -= Vec2[1];
             Vec1[2] -= Vec2[2];
             
             // Calculate angle
             
             Distance = sqrt(vector_dot(Vec1,Vec1));

             Normal[0] = VortexEdge->Normal()[0];
             Normal[1] = VortexEdge->Normal()[1];
             Normal[2] = VortexEdge->Normal()[2];
             
             if ( DoSymmetryPlaneSolve_ == SYM_X && xyz[0] < 0. ) Normal[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y && xyz[1] < 0. ) Normal[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z && xyz[2] < 0. ) Normal[2] *= -1.;
             
             Dot = vector_dot(Vec1,Normal);
             
             Dot /= Distance;
             
             Dot = MAX(-1.,MIN(Dot,1.));
             
             Angle = acos(Dot);
             
             if ( Dot > 0. && Angle <= 0.5*PI ) {
             
                // Ratio of distance to surface edge length
                
                Ratio = Distance / VortexEdge->Length();
                
                // Component of velocity in direction of surface normal
                             
                Dot = vector_dot(Normal, qVec);
                
                // Does velocity pull us into body
         
                if ( Dot < 0. && Ratio < 1. ) {
   
                   // Damp out velocity near wall
    
                   Dot *= exp(-5.*Ratio);

                   qVec[0] -= Dot * Normal[0];
                   qVec[1] -= Dot * Normal[1];
                   qVec[2] -= Dot * Normal[2];
                      
                }
   
             }
             
          }
          
       }

       if ( DoSymmetryPlaneSolve_ == SYM_X && xyz[0] < 0. ) qVec[0] *= -1.;
       if ( DoSymmetryPlaneSolve_ == SYM_Y && xyz[1] < 0. ) qVec[1] *= -1.;
       if ( DoSymmetryPlaneSolve_ == SYM_Z && xyz[2] < 0. ) qVec[2] *= -1.;

       q[0] = qVec[0];
       q[1] = qVec[1];
       q[2] = qVec[2];
       
    } 

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

    InteractionList = CreateInteractionList(0, 0, ALL_LOOPS, xyz, NumberOfEdges);

    U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(VortexEdge,dq)
    for ( j = 1 ; j <= NumberOfEdges ; j++ ) {
     
       VortexEdge = InteractionList[j];
    
       if ( VortexEdge->DegenWing() ) {              

          VortexEdge->InducedVelocity(xyz, dq);
      
          U += dq[0];
          V += dq[1];
          W += dq[2];
          
       }
         
    }
    
    q[0] = U;
    q[1] = V;
    q[2] = W;    

}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CreateInteractionList                          #
#                                                                              #
##############################################################################*/

VSP_EDGE **VSP_SOLVER::CreateInteractionList(int ComponentID, int pLoop, int InteractionType, double xyz[3], int &NumberOfInteractionEdges)
{

    int i, j, cpu, CoarseGridEdge, FineGridEdge, Level, Loop, LoopComponentID;
    int DoAllLoops, NoRelativeMotion, RelativeMotion;
    int StackSize, MoveDownLevel, Next, AddEdges, NumberOfUsedEdges;
    double Distance, Test, NormalDistance, Vec[3], Tolerance, Ratio;

    // Grab the current cpu thread id

#ifdef VSPAERO_OPENMP    
    cpu = omp_get_thread_num();
#else
    cpu = 0;
#endif    
  
    // Insert loops on coarsest level into stack
    
    Level = VSPGeom().NumberOfGridLevels();
 
    StackSize = 0;

    for ( Loop = 1 ; Loop <= VSPGeom().Grid(Level).NumberOfLoops() ; Loop++ ) {
     
       StackSize++;
       
       LoopStackList_[cpu][StackSize].Level = Level;
       LoopStackList_[cpu][StackSize].Loop  = Loop;

    }
      
    // Update the search ID value... reset things after we have done all the loops
    
    SearchID_[cpu]++;
    
    if ( SearchID_[cpu] > NumberOfVortexLoops_ ) {
    
       for ( Level = 1 ; Level <= VSPGeom().NumberOfGridLevels() ; Level++ ) {
      
          zero_int_array(EdgeIsUsed_[cpu][Level], VSPGeom().Grid(Level).NumberOfEdges()); 
          
       }
       
       SearchID_[cpu] = 1;

    }
    
    // Interaction types
    
    DoAllLoops = RelativeMotion = NoRelativeMotion = 0;
    
    if ( InteractionType == ALL_LOOPS          ) DoAllLoops       = 1;
    if ( InteractionType ==    RELATIVE_MOTION )   RelativeMotion = 1;
    if ( InteractionType == NO_RELATIVE_MOTION ) NoRelativeMotion = 1;
    
    // Now loop over stack and begin AGMP process

    NumberOfUsedEdges = 0;

    Next = 1;
        
    while ( Next <= StackSize ) {
     
       Level = LoopStackList_[cpu][Next].Level;
       Loop  = LoopStackList_[cpu][Next].Loop;
       
       LoopComponentID = VSPGeom().Grid(Level).LoopList(Loop).ComponentID();
     
       if ( ( DoAllLoops                                                                                                        ) || 
            ( NoRelativeMotion && (  GeometryComponentIsFixed_[ComponentID] && GeometryComponentIsFixed_[LoopComponentID]  )    ) ||
            ( NoRelativeMotion && (  GeometryGroupID_[ComponentID] == GeometryGroupID_[LoopComponentID]                    )    ) ||
            (   RelativeMotion && ( !GeometryComponentIsFixed_[ComponentID] || !GeometryComponentIsFixed_[LoopComponentID] ) &&
                                  (  GeometryGroupID_[ComponentID] != GeometryGroupID_[LoopComponentID]                    )    ) ) {
               
          // If we are far enough away from this loop, add it's edges to the interaction list
                
          MoveDownLevel = 0;
   
          Vec[0] = xyz[0] - VSPGeom().Grid(Level).LoopList(Loop).Xc();
          Vec[1] = xyz[1] - VSPGeom().Grid(Level).LoopList(Loop).Yc();
          Vec[2] = xyz[2] - VSPGeom().Grid(Level).LoopList(Loop).Zc();
                
          Distance = sqrt( SQR(Vec[0]) + SQR(Vec[1]) + SQR(Vec[2]) );
                  
          Test = FarAway_ * ( VSPGeom().Grid(Level).LoopList(Loop).Length() + VSPGeom().Grid(Level).LoopList(Loop).CentroidOffSet() );
   
          if ( Level == 1 || ( Test <= Distance && !inside_box(VSPGeom().Grid(Level).LoopList(Loop).BoundBox(), xyz) ) ) {
   
             AddEdges = 1;
         
             // Check for nearly planar, and close, panels on different surfaces - VLM
   
             Ratio = Distance / ( VSPGeom().Grid(Level).LoopList(Loop).Length() + VSPGeom().Grid(Level).LoopList(Loop).CentroidOffSet() );
           
             if ( ComponentID > 0 && ComponentID != VSPGeom().Grid(Level).LoopList(Loop).ComponentID() && Ratio <= 2. ) {
             
                // Calculate normal distance
   
                NormalDistance = ABS(vector_dot(Vec,VSPGeom().Grid(Level).LoopList(Loop).Normal()));
        
                // Tolerance
                
                Tolerance = sqrt(VSPGeom().Grid(Level).LoopList(Loop).Area());
         
                if ( ABS(NormalDistance) <= Tolerance ) AddEdges = 0;

             }
             
             // Check for near planar, and close, panels with opposite pointing normals - typical trailing edges
             
             Ratio = Distance / VSPGeom().Grid(Level).LoopList(Loop).Length();

             if ( ComponentID > 0 && ModelType_ == PANEL_MODEL && Ratio <= 1. ) {
   
                if ( Level == 1 && pLoop != Loop ) {
                   
                   if ( vector_dot(VortexLoop(pLoop).Normal(), VSPGeom().Grid(Level).LoopList(Loop).Normal()) < 0. ) {
                
                      // Calculate normal distance
      
                      NormalDistance = ABS(vector_dot(Vec,VSPGeom().Grid(Level).LoopList(Loop).Normal()));
          
                      // Tolerance

                      Tolerance = 0.25*VSPGeom().Grid(Level).LoopList(Loop).RefLength();
                      
                      if ( ABS(NormalDistance) <= Tolerance ) {
                        
                   // test code...     AddEdges = 0;     
                         
                      }
                    
                   }
                   
                }
                 
             }
             
             // Add these edges to the list
             
             if ( AddEdges ) {
 
                for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfEdges() ; i++ ) {
          
                   j = VSPGeom().Grid(Level).LoopList(Loop).Edge(i);
                   
                   if ( EdgeIsUsed_[cpu][Level][j] != SearchID_[cpu] ) {
                      
                      if ( !VSPGeom().Grid(Level).EdgeList(j).IsTrailingEdge() ) {
                      
                         EdgeIsUsed_[cpu][Level][j] = SearchID_[cpu];

                      }
                     
                   }
                   
                }
                
             }
             
          }
          
          // If xyz too close to this loop, move down a level
          
          else {
   
             if ( Level > 1 ) {
            
                for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
   
                   StackSize++;
       
                   LoopStackList_[cpu][StackSize].Level = Level - 1;
                   
                   LoopStackList_[cpu][StackSize].Loop  = VSPGeom().Grid(Level).LoopList(Loop).FineGridLoop(i);
   
                }   
                
             }
                       
          }
          
       }
   
       // Move onto next entry in the stack

       Next++;
       
    }
/*
 * Old way
 * 
    // Zero out fine grid edges that might be marked as well
  
    NumberOfInteractionEdges = 0;

    for ( Level = VSPGeom().NumberOfGridLevels() ; Level >= 1 ; Level-- ) {
       
       for ( i = VSPGeom().Grid(Level).NumberOfEdges() ; i >= 1 ; i-- ) {
          
          if ( EdgeIsUsed_[cpu][Level][i] == SearchID_[cpu] ) {

             // Add in the edge if it's not represented on a coarser grid already
             // And zero out finer grid contribution
             
             if ( Level > 1 ) {
                
                FineGridEdge = VSPGeom().Grid(Level).EdgeList(i).FineGridEdge();
             
                EdgeIsUsed_[cpu][Level-1][FineGridEdge] = 0;
                
                if ( EdgeIsUsed_[cpu][Level][i] == SearchID_[cpu] ) {
                   
                   TempInteractionList_[cpu][++NumberOfInteractionEdges] = &(VSPGeom().Grid(Level).EdgeList(i));

                   EdgeIsUsed_[cpu][Level][i] = 0;
                   
                }
               
             }
             
             // On the finest grid, so just add in the edge
             
             else {
                   
                if ( EdgeIsUsed_[cpu][Level][i] == SearchID_[cpu] ) {
         
                   EdgeIsUsed_[cpu][Level][i] = 0;
                   
                   TempInteractionList_[cpu][++NumberOfInteractionEdges] = &(VSPGeom().Grid(Level).EdgeList(i));    
                  
                }
                
             }
             
          }
          
       }
       
    }
*/

    // Zero out fine grid edges that might be marked as well
    
    NumberOfInteractionEdges = 0;

    for ( Level = VSPGeom().NumberOfGridLevels() ; Level >= 1 ; Level-- ) {
       
       for ( i = VSPGeom().Grid(Level).NumberOfEdges() ; i >= 1 ; i-- ) {
          
          if ( EdgeIsUsed_[cpu][Level][i] == SearchID_[cpu] ) {

             // On coarsest mesh, so just add it in
             
             if ( Level == VSPGeom().NumberOfGridLevels() ) {
         
                TempInteractionList_[cpu][++NumberOfInteractionEdges] = &(VSPGeom().Grid(Level).EdgeList(i));    
          
             }  
             
             // Not on the coarsest mesh, must check if any coarser mesh edges already added in...
             
             else {
                
                CoarseGridEdge = VSPGeom().Grid(Level).EdgeList(i).CoarseGridEdge();
                
                // Coarse edge was not added, so this one in
                   
                if ( EdgeIsUsed_[cpu][Level+1][CoarseGridEdge] != SearchID_[cpu] ) {
    
                   TempInteractionList_[cpu][++NumberOfInteractionEdges] = &(VSPGeom().Grid(Level).EdgeList(i));    
                  
                }
                
             }             
             
          }
          
          // Edge not marked, but still need to check if a coarser edge was added...
          
          else {
             
             // Not on the coarsest mesh, must check if any coarser mesh edges already added in...
             
             if ( Level != VSPGeom().NumberOfGridLevels() ) {
         
                CoarseGridEdge = VSPGeom().Grid(Level).EdgeList(i).CoarseGridEdge();
                
                // Coarse edge was not added, so this one in
                   
                if ( EdgeIsUsed_[cpu][Level+1][CoarseGridEdge] == SearchID_[cpu] ) {
                   
                   EdgeIsUsed_[cpu][Level][i] = SearchID_[cpu];
   
                }
                
             }
             
          }
          
       }
       
    }              
        
    // Reverse order of the list
    
    VSP_EDGE *TempEdge;
    
    for ( i = 1 ; i <= NumberOfInteractionEdges / 2 ; i++ ) {
       
       TempEdge = TempInteractionList_[cpu][i];
       
       TempInteractionList_[cpu][i] = TempInteractionList_[cpu][NumberOfInteractionEdges - i + 1];
       
       TempInteractionList_[cpu][NumberOfInteractionEdges - i + 1] = TempEdge;
       
    }
    
    return TempInteractionList_[cpu];
    
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
#                   VSP_SOLVER RestrictKTFactorFromGrid                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::RestrictKTFactorFromGrid(int Level)
{
 
    int i_c, i_f, g_c, g_f, Loop1, Loop2;
    double Fact, Area1, Area2, wgt1, wgt2;

    // Restrict solution from Level i, to level i+1
    
    g_f = Level;
    g_c = Level + 1;
    
    // Zero out stuff on the coarsest grid
    
    for ( i_c = 1 ; i_c <= VSPGeom().Grid(g_c).NumberOfLoops() ; i_c++ ) {

       VSPGeom().Grid(g_c).LoopList(i_c).KTFact() = 0.;

    }
  
    // Restrict the solution to the coarse grid
 
    for ( i_f = 1 ; i_f <= VSPGeom().Grid(g_f).NumberOfLoops() ; i_f++ ) {

       i_c = VSPGeom().Grid(g_f).LoopList(i_f).CoarseGridLoop();
       
       Fact = VSPGeom().Grid(g_f).LoopList(i_f).Area()
            / VSPGeom().Grid(g_c).LoopList(i_c).Area();

       VSPGeom().Grid(g_c).LoopList(i_c).KTFact() += Fact*VSPGeom().Grid(g_f).LoopList(i_f).KTFact();
  
    }
    
    // Update KT Factor on the coarse grid edges

    for ( i_c = 1 ; i_c <= VSPGeom().Grid(g_c).NumberOfEdges() ; i_c++ ) {
    
       Loop1 = VSPGeom().Grid(g_c).EdgeList(i_c).Loop1();
       Loop2 = VSPGeom().Grid(g_c).EdgeList(i_c).Loop2();
       
       Area1 = VSPGeom().Grid(g_c).LoopList(Loop1).Area();
       Area2 = VSPGeom().Grid(g_c).LoopList(Loop1).Area();
       
       wgt2 = Area2 / ( Area1 + Area2 );
       wgt1 = 1. - wgt2;
       
       VSPGeom().Grid(g_c).EdgeList(i_c).KTFact() = wgt1 *  VSPGeom().Grid(g_c).LoopList(Loop1).KTFact() + wgt2 *  VSPGeom().Grid(g_c).LoopList(Loop2).KTFact();
       
    }
       
}


/*##############################################################################
#                                                                              #
#                       VSP_SOLVER InterpolateSolutionFromGrid                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::InterpolateSolutionFromGrid(int Level)
{
 
    int i, j, i_c, i_f, g_c, g_f, Edge, Iter, Node, Node1, Node2, Done;
    int *FixedNode,  NodeHits;
    double Fact, *dCp, *Denom, *Res, *Dif, *Sum, Delta, Eps, ResMax, ResMax0;
    double Wgt1, Wgt2, CpAvg;
    
    g_c = Level;
    g_f = Level - 1;
    
    // Smooth pressure
    
    dCp = new double[VSPGeom().Grid(g_c).NumberOfNodes() + 1];
    Denom = new double[VSPGeom().Grid(g_c).NumberOfNodes() + 1];
    
    zero_double_array(dCp, VSPGeom().Grid(g_c).NumberOfNodes());
    zero_double_array(Denom, VSPGeom().Grid(g_c).NumberOfNodes());
    
    // Create nodal value of CP

    for ( i_c = 1 ; i_c <= VSPGeom().Grid(g_c).NumberOfLoops() ; i_c++ ) {

       for ( j = 1 ; j <= VSPGeom().Grid(g_c).LoopList(i_c).NumberOfEdges() ; j++ ) {
        
          Edge = VSPGeom().Grid(g_c).LoopList(i_c).Edge(j);
          
          Node1 = VSPGeom().Grid(g_c).EdgeList(Edge).Node1();
          Node2 = VSPGeom().Grid(g_c).EdgeList(Edge).Node2();
          
          dCp[Node1] += VSPGeom().Grid(g_c).LoopList(i_c).dCp() * VSPGeom().Grid(g_c).LoopList(i_c).Area();
          dCp[Node2] += VSPGeom().Grid(g_c).LoopList(i_c).dCp() * VSPGeom().Grid(g_c).LoopList(i_c).Area();
                               
          Denom[Node1] += VSPGeom().Grid(g_c).LoopList(i_c).Area();
          Denom[Node2] += VSPGeom().Grid(g_c).LoopList(i_c).Area();
          
       }

    }

    for ( j = 1 ; j <= VSPGeom().Grid(g_c).NumberOfNodes() ; j++ ) {
       
       dCp[j] /= Denom[j];
       
    }

    // Enforce kutta condition on trailing edge
  
    FixedNode = new int[VSPGeom().Grid(g_c).NumberOfNodes() + 1];
    
    zero_int_array(FixedNode, VSPGeom().Grid(g_c).NumberOfNodes());

    if ( ModelType_ == VLM_MODEL ) {
       
       for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfKuttaNodes() ; i++ ) {
   
          Node = VSPGeom().Grid(g_c).KuttaNode(i);
   
          dCp[Node] = 0.;
          
          FixedNode[Node] = 1.;
   
       }
       
    }
    
    // Panel Model
    
    else if ( ModelType_ == PANEL_MODEL ) {
       
       for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfKuttaNodes() ; i++ ) {
   
          Node = VSPGeom().Grid(g_c).KuttaNode(i);

          FixedNode[Node] = 1;
   
       }
       
       for ( i_c = 1 ; i_c <= VSPGeom().Grid(g_c).NumberOfLoops() ; i_c++ ) {
        
          for ( j = 1 ; j <= VSPGeom().Grid(g_c).LoopList(i_c).NumberOfEdges() ; j++ ) {
           
             Edge = VSPGeom().Grid(g_c).LoopList(i_c).Edge(j);
             
             Node1 = VSPGeom().Grid(g_c).EdgeList(Edge).Node1();
             Node2 = VSPGeom().Grid(g_c).EdgeList(Edge).Node2();
         
             if ( LoopIsOnBaseRegion_[i_c] ) {
                
                FixedNode[Node1] = 1;
                
                dCp[Node1] = CpBase_;
                
             }
       
             if ( LoopIsOnBaseRegion_[i_c] ) {
                
                FixedNode[Node2] = 1;
                
                dCp[Node2] = CpBase_;
                
             }
             
          }
          
       }       
    }

    Res = new double[VSPGeom().Grid(g_c).NumberOfNodes() + 1];

    Dif = new double[VSPGeom().Grid(g_c).NumberOfNodes() + 1];

    Sum = new double[VSPGeom().Grid(g_c).NumberOfNodes() + 1];

    zero_double_array(Sum,VSPGeom().Grid(g_c).NumberOfNodes());

    // Count edge hits per node

    for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfEdges() ; i++ ) {

       // Edge to node pointers

       Node1 = VSPGeom().Grid(g_c).EdgeList(i).Node1();
       Node2 = VSPGeom().Grid(g_c).EdgeList(i).Node2();

       Sum[Node1] += 1.;
       Sum[Node2] += 1.;
      
    }

    // Loop over and smooth all residuals

    for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfNodes() ; i++ ) {

       Res[i] = dCp[i];

       Dif[i] = 0.;

    }

    // Do a few iterations of smoothing

    Iter = 1;
    
    Done = 0;
    
    while ( !Done && Iter <= 250 ) {
       
       ResMax = 0.;

       // Loop over the edges and scatter fluxes

       for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfEdges() ; i++ ) {

           // Edge to node pointers

           Node1 = VSPGeom().Grid(g_c).EdgeList(i).Node1();
           Node2 = VSPGeom().Grid(g_c).EdgeList(i).Node2();
     
           Delta = Res[Node1] - Res[Node2];

           Dif[Node1] -= Delta;
           Dif[Node2] += Delta;

       }

       Eps = 1.;
       
       for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfNodes() ; i++ ) {

          if ( !FixedNode[i] ) {

             Fact = Eps*Sum[i];
             
             Delta = ( dCp[i] + Fact*Res[i] + Eps*Dif[i] )/( 1. + Fact ) - Res[i];
             
             ResMax += Delta*Delta;

             Res[i] += Delta;

          }

          Dif[i] = 0.;

       }
       
       ResMax = sqrt(ResMax/VSPGeom().Grid(g_c).NumberOfNodes());
       
       if ( Iter == 1 ) ResMax0 = ResMax;
       
       if ( ResMax0 != 0 ) {
          
          //printf("Iter: %d ... Resmax: %f \n",Iter,log10(ResMax/ResMax0));
          
          if ( log10(ResMax/ResMax0) <= -2. ) Done = 1;
          
       }  
       
       Iter++;     
              
    }

    // Update nodal values with smoothed result
    
    for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfNodes() ; i++ ) {

       dCp[i] = Res[i];

    }           
 
    // Finally, update loop values

    for ( i_c = 1 ; i_c <= VSPGeom().Grid(g_c).NumberOfLoops() ; i_c++ ) {
       
       NodeHits = 0;
       
       CpAvg = 0.;

       for ( j = 1 ; j <= VSPGeom().Grid(g_c).LoopList(i_c).NumberOfEdges() ; j++ ) {
    
          Edge = VSPGeom().Grid(g_c).LoopList(i_c).Edge(j);
          
          Node1 = VSPGeom().Grid(g_c).EdgeList(Edge).Node1();
          Node2 = VSPGeom().Grid(g_c).EdgeList(Edge).Node2();
          
          Wgt1 = Wgt2 = 0.;
          
          if ( ModelType_ == VLM_MODEL || Sum[Node1] > 0. ) { Wgt1 = 1.; NodeHits++; }
          if ( ModelType_ == VLM_MODEL || Sum[Node2] > 0. ) { Wgt2 = 1.; NodeHits++; }
          
          CpAvg += Wgt1*dCp[Node1] + Wgt2*dCp[Node2];
          
       }

       if ( NodeHits > 0 ) VSPGeom().Grid(g_c).LoopList(i_c).dCp() = CpAvg / NodeHits;
       
    }
    
    // Enforce base pressures
    
    if ( ModelType_ == PANEL_MODEL ) {
       
       // Base
       
       for ( i_c = 1 ; i_c <= NumberOfVortexLoops_ ; i_c++ ) {
          
          if ( LoopIsOnBaseRegion_[i_c] ) VSPGeom().Grid(g_c).LoopList(i_c).dCp() = CpBase_;
         
       }            
    
    }
    
    // Prolongate solution from coarse to fine grid

#pragma omp parallel for private(i_c)
    for ( i_f = 1 ; i_f <= VSPGeom().Grid(g_f).NumberOfLoops() ; i_f++ ) {

       i_c = VSPGeom().Grid(g_f).LoopList(i_f).CoarseGridLoop();

       VSPGeom().Grid(g_f).LoopList(i_f).dCp()          = VSPGeom().Grid(g_c).LoopList(i_c).dCp();
       
       VSPGeom().Grid(g_f).LoopList(i_f).dCp_Unsteady() = VSPGeom().Grid(g_c).LoopList(i_c).dCp_Unsteady();

       // Scale Cp with local velocities

       VSPGeom().Grid(g_f).LoopList(i_f).Gamma()        = VSPGeom().Grid(g_c).LoopList(i_c).Gamma();

    }  
    
    delete [] dCp;
    delete [] Denom;

}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER UpdateVortexEdgeStrengths                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::UpdateVortexEdgeStrengths(int Level, int UpdateType)
{

    int i, j, k, Node1, Node2;

    // Copy current value over to vortex loops
    
    if ( Level == 1 ) {
     
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
        
          VortexLoop(i).Gamma() = Gamma(i);

       }
  
    }
    
    // Calculate delta-gammas for each surface vortex edge
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {

       VSPGeom().Grid(Level).EdgeList(i).Gamma() = VSPGeom().Grid(Level).LoopList(VSPGeom().Grid(Level).EdgeList(i).VortexLoop1()).Gamma()
                                                 - VSPGeom().Grid(Level).LoopList(VSPGeom().Grid(Level).EdgeList(i).VortexLoop2()).Gamma();
                  
    }    

    // Calculate node wise delta gammas on finest grid

    if ( Level == 1 ) {

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

          VSPGeom().Grid(Level).NodeList(j).dGamma() = 0.;
          
       }

       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
    
          if ( VSPGeom().Grid(Level).EdgeList(i).IsTrailingEdge() ) {     

             Node1 = VSPGeom().Grid(Level).EdgeList(i).Node1();
             Node2 = VSPGeom().Grid(Level).EdgeList(i).Node2();
                          
             VSPGeom().Grid(Level).NodeList(Node1).dGamma() += VSPGeom().Grid(Level).EdgeList(i).Gamma();
             VSPGeom().Grid(Level).NodeList(Node2).dGamma() -= VSPGeom().Grid(Level).EdgeList(i).Gamma();
        
          }
          
       }

       // Calculate delta-gammas for each trailing vortex edge
       
       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

          for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {
   
             Node1 = VortexSheet(k).TrailingVortexEdge(i).Node();

             VortexSheet(k).TrailingVortexEdge(i).Gamma() = VSPGeom().Grid(Level).NodeList(Node1).dGamma();          

          }
  
          if ( StartFromSteadyState_ && Time_ == 1 ) {
   
             VortexSheet(k).UpdateVortexStrengths(-UpdateType);
             
          }
          
          else {
             
             VortexSheet(k).UpdateVortexStrengths(UpdateType);
             
          }             
          
       }

    }
  
}

/*##############################################################################
#                                                                              #
#                          VSP_SOLVER ZeroVortexState                          #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ZeroVortexState(void)
{

    int i, j, k, Level;

    // Copy current value over to vortex loops
    
    for ( Level = 1 ; Level <= NumberOfMGLevels_ ; Level++ ) {
       
       if ( Level == 1 ) {
   
          for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
           
             VortexLoop(i).Gamma() = 0.;
   
          }
     
       }
       
       // Calculate delta-gammas for each surface vortex edge
       
       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
   
          VSPGeom().Grid(Level).EdgeList(i).Gamma() = 0.;
                     
       }    
   
       // Calculate node wise delta gammas on finest grid
   
       if ( Level == 1 ) {
   
          for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {
   
             VSPGeom().Grid(Level).NodeList(j).dGamma() = 0.;
             
          }
 
          // Calculate delta-gammas for each trailing vortex edge
          
          for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
   
             for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {
      
                VortexSheet(k).TrailingVortexEdge(i).Gamma() = 0.;
   
             }

             VortexSheet(k).UpdateVortexStrengths(IMPLICIT_WAKE_GAMMAS);
             VortexSheet(k).UpdateVortexStrengths(EXPLICIT_WAKE_GAMMAS);
             VortexSheet(k).UpdateVortexStrengths(ALL_WAKE_GAMMAS);
             
          }
     
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                      VSP_SOLVER OutputStatusFile                             #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::OutputStatusFile(void)
{

    int i;
    double E, AR, ToQS, Time;
    
    AR = Bref_ * Bref_ / Sref_;

    ToQS = 0.;
    
    for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
     
       if ( Verbose_ ) printf("RotorDisk(%d).RotorThrust(): %f \n",i,RotorDisk(i).RotorThrust());
            
       ToQS += RotorDisk(i).RotorThrust() / ( 0.5 * Density_ * Vref_ * Vref_ * Sref_);
       
    }    
 
    E = (CL() * CL() /(PI * AR)) / CD() ;
 
    i = CurrentWakeIteration_;

    if ( !TimeAccurate_ ) {
       
       fprintf(StatusFile_,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf\n",
               i,
               Mach_,
               AngleOfAttack_/TORAD,
               AngleOfBeta_/TORAD,
               CL(),
               CDo(),
               CD(),
               CDo() + CD(),
               CS(),            
               CL()/(CDo() + CD()),
               E,
               CFx(),
               CFy(),
               CFz(),
               CMx(),
               CMy(),
               CMz(),
               ToQS);
               
    }
    
    else {
       
        CL_Unsteady_[Time_] = CL();
        CD_Unsteady_[Time_] = CD();
        CS_Unsteady_[Time_] = CS();
       CFx_Unsteady_[Time_] = CFx();
       CFy_Unsteady_[Time_] = CFy();
       CFz_Unsteady_[Time_] = CFz();
       CMx_Unsteady_[Time_] = CMx();
       CMy_Unsteady_[Time_] = CMy();
       CMz_Unsteady_[Time_] = CMz();

       if ( TimeAnalysisType_ == HEAVE_ANALYSIS ) {
          
          fprintf(StatusFile_,"%9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf\n",
                  CurrentTime_,
                  Mach_,
                  AngleOfAttack_/TORAD,
                  AngleOfBeta_/TORAD,
                  CL(),
                  CDo(),
                  CD(),
                  CDo() + CD(),
                  CS(),            
                  CL()/(CDo() + CD()),
                  E,
                  CFx(),
                  CFy(),
                  CFz(),
                  CMx(),
                  CMy(),
                  CMz(),
                  ToQS,
                  Unsteady_H_,
                  CL(1),
                  CD(1),
                  CS(1),
                  CFx(1),
                  CFy(1),
                  CFz(1),
                  CMx(1),
                  CMy(1),
                  CMz(1));
                  
       }
       
       else if ( TimeAnalysisType_ == P_ANALYSIS || TimeAnalysisType_ == Q_ANALYSIS || TimeAnalysisType_ == R_ANALYSIS ) {
          
          fprintf(StatusFile_,"%9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf\n",
                  CurrentTime_,
                  Mach_,
                  AngleOfAttack_/TORAD,
                  AngleOfBeta_/TORAD,
                  CL(),
                  CDo(),
                  CD(),
                  CDo() + CD(),
                  CS(),            
                  CL()/(CDo() + CD()),
                  E,
                  CFx(),
                  CFy(),
                  CFz(),
                  CMx(),
                  CMy(),
                  CMz(),
                  ToQS,
                  Unsteady_Angle_*180./3.14159,
                  CL(1),
                  CD(1),
                  CS(1),
                  CFx(1),
                  CFy(1),
                  CFz(1),
                  CMx(1),
                  CMy(1),
                  CMz(1));                  
                  
       }        
       
       else {
          
          Time = CurrentTime_;
          
          if ( NoiseAnalysis_ ) Time = CurrentNoiseTime_;
          
          fprintf(StatusFile_,"%9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf\n",
                  Time,
                  Mach_,
                  AngleOfAttack_/TORAD,
                  AngleOfBeta_/TORAD,
                  CL(),
                  CDo(),
                  CD(),
                  CDo() + CD(),
                  CS(),            
                  CL()/(CDo() + CD()),
                  E,
                  CFx(),
                  CFy(),
                  CFz(),
                  CMx(),
                  CMy(),
                  CMz(),
                  ToQS,
                  CL(1),
                  CD(1),
                  CS(1),
                  CFx(1),
                  CFy(1),
                  CFz(1),
                  CMx(1),
                  CMy(1),
                  CMz(1));                  
               
       }            
               
    }       

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
    fprintf(StatusFile_,"Skin Friction Drag Break Out:\n");    
    fprintf(StatusFile_,"\n");   
    fprintf(StatusFile_,"\n");       
                       //1234567890123456789012345678901234567890: 123456789
    fprintf(StatusFile_,"Surface                                      CDo \n");
    fprintf(StatusFile_,"\n");
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       fprintf(StatusFile_,"%-40s  %9.5lf \n",
               VSPGeom().VSP_Surface(i).ComponentName(),
               VSPGeom().VSP_Surface(i).CDo());

    } 
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER WriteCaseHeader                               #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteCaseHeader(FILE *fid)
{
    char headerFormatStr[] = "%-20s %12s %-20s\n";
    char dataFormatStr[] =   "%-20s %12.7lf %-20s\n";
    
    fprintf(fid,"***************************************************************************************************************************************************************************************** \n");
    fprintf(fid,"\n");
    
    //          123456789012345678901234567890123456789
    fprintf(fid,headerFormatStr, "# Name", "Value   ", "  Units");
    fprintf(fid,dataFormatStr, "Sref_", Sref(), "Lunit^2");
    fprintf(fid,dataFormatStr, "Cref_", Cref(), "Lunit");
    fprintf(fid,dataFormatStr, "Bref_", Bref(), "Lunit");
    fprintf(fid,dataFormatStr, "Xcg_", Xcg(), "Lunit");
    fprintf(fid,dataFormatStr, "Ycg_", Ycg(), "Lunit");
    fprintf(fid,dataFormatStr, "Zcg_", Zcg(), "Lunit");
    fprintf(fid,dataFormatStr, "Mach_", Mach(), "no_unit");
    fprintf(fid,dataFormatStr, "AoA_", AngleOfAttack()/TORAD, "deg");
    fprintf(fid,dataFormatStr, "Beta_", AngleOfBeta()/TORAD, "deg");
    fprintf(fid,dataFormatStr, "Rho_", Density(), "Munit/Lunit^3");
    fprintf(fid,dataFormatStr, "Vinf_", Vinf(), "Lunit/Tunit");
    fprintf(fid,dataFormatStr, "Roll__Rate", RotationalRate_p(), "rad/Tunit");
    fprintf(fid,dataFormatStr, "Pitch_Rate", RotationalRate_q(), "rad/Tunit");
    fprintf(fid,dataFormatStr, "Yaw___Rate", RotationalRate_r(), "rad/Tunit");
    /*
    char control_name[20];
    for ( int n = 1 ; n <= NumberOfControlGroups_ ; n++ ) {
        //                    1234567890123456789
        sprintf(control_name,"Control_Group_%-5d",n);
        fprintf(fid,dataFormatStr, control_name, Delta_Control_, "deg");
    }
    */
    
    fprintf(fid,"\n");
}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER CalculateLeadingEdgeSuctionFraction                #
#                                                                              #
##############################################################################*/

double VSP_SOLVER::CalculateLeadingEdgeSuctionFraction(double Mach, double ToC, double RoC, double EtaToC, double AoA, double Sweep)
{
   
   double Rin, RoCn, ToCn, Machn, Betan, Ctn;
   double e1, e2, e3, Ptt, k, Kt;
   
   // Normal RoC
   
   RoCn = RoC / pow(cos(Sweep),2.);
   
   // Normal ToC
   
   ToCn = ToC / cos(Sweep);   

   // Normal Leading edge radius index
   
   Rin = RoCn * EtaToC / pow(ToCn,2.);
   
   // Exponents
   
   e1 =  0.40*pow(Rin,0.16) - 0.7;
   e2 =  1.60*pow(Rin,0.10) - 3.0;
   e3 = -0.32*pow(Rin,0.10) - 0.3;
   
   // Normal Mach
   
   Machn = Mach * cos(Sweep);
   
   // Catch supersonic case
   
   if ( Machn >= 1. ) return 0.;   
   
   // Betan
   
   Betan = sqrt(1. - pow(Machn,2.));
   
   // Ctn
   
   Ctn = 2.*PI*pow(sin(AoA),2.) / ( Betan * pow(cos(Sweep),3.) );
   
   // Ptt
   
   Ptt = Ctn * Betan * pow( ToCn * pow(EtaToC/0.5,e1) / 0.09, e2);
   
   // k
   
   k = ( 0.14*(1. - (1. - sqrt(Rin)*pow(Machn,5.))) + 0.11*sqrt(Rin))*pow((1.-Machn)/Machn,0.48*(1.+pow(Rin,0.3)));
   
   // Kt

   Kt = (1. + sqrt(pow(ToCn,1.2))) * k * pow(Ptt,e3);
   
   Kt = MAX(0.,MIN(Kt,1.));
   
   return Kt;
 
}

/*##############################################################################
#                                                                              #
#            VSP_SOLVER CalculateRotorCoefficientsForGroup                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateRotorCoefficientsForGroup(int Group)
{
   
    int c, k, i, j;
    double Thrust, Thrusto, Thrusti, Moment, Momento, Momenti;
    double CT, CQ, CP, J, EtaP, CT_h, CQ_h, CP_h, FOM, Diameter, RPM, Vec[3], Time;
    double Omega, Radius, TipVelocity, Area;

    // Single rotor analysis
   
    if (0&& RotorAnalysis_ ) {
      
       Diameter = Bref_;
      
       RPM = BladeRPM_;
   
       // Thrust
    
       Thrusto = -CFxo_ * 0.5 * Density_ * Vref_ * Vref_ * Sref_;    
       Thrusti = -CFx() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
       Thrust = Thrusto + Thrusti;
    
       // Torque
     
       Momento = CMxo_ * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Bref_;   
       Momenti = CMx() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Bref_;   
       Moment = Momento + Momenti;
       
       // Calculate rotor coefficients
      
       CalculateRotorCoefficientsFromForces(Thrust, Moment, Diameter, RPM, J, CT, CQ, CP, EtaP, CT_h, CQ_h, CP_h, FOM);

       fprintf(RotorFile_[1],"%10.3f %10.3f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f \n",
               Diameter,
               RPM,
               Thrust,
               Thrusto,
               Thrusti,
               Moment,
               Momento,
               Momenti,
               J,
               CT,
               CQ,
               CP,
               EtaP,
               CT_h,
               CQ_h,
               CP_h,
               FOM);
               
        // Write out spanwise blade loading data, again averaged over one revolution
        
        for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfSurfaces() ; i++ ) {
        
           fprintf(RotorFile_[k],"\n\n\n");
           fprintf(RotorFile_[k],"Steady state loading for rotor blade: %d --> VSP Surface: %d ... NOTE: values are per station, not per area! \n",i,ComponentGroupList_[c].SpanLoadData(i).SurfaceID());
           fprintf(RotorFile_[k],"\n");
           
                            //    123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123
           fprintf(RotorFile_[k],"  Station     S       Chord     Area      V/Vref   Diameter    RPM      TipVel       CNo_H         CSo_H         CTo_H         CQo_H         CPo_H         CN_H          CS_H          CT_H          CQ_H          CP_H \n");
           
           ComponentGroupList_[c].SpanLoadData(i).CalculateAverageForcesAndMoments();
           
           // Write out averaged span loading data
        
           Omega = 2. * PI * RPM / 60.;
           
           Radius = 0.5 * Diameter;
           
           Area = PI * Radius * Radius;
           
           TipVelocity = Omega * Radius;
                                
           for ( j = 1 ; j <= ComponentGroupList_[c].SpanLoadData(i).NumberOfSpanStations() ; j++ ) {
        
             fprintf(RotorFile_[k],"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le \n",
                     j,
                     ComponentGroupList_[c].SpanLoadData(i).Span_S(j),                    
                     ComponentGroupList_[c].SpanLoadData(i).Span_Chord(j),
                     ComponentGroupList_[c].SpanLoadData(i).Span_Area(j),
                     
                     ComponentGroupList_[c].SpanLoadData(i).Local_Velocity(j),
                     
                     Diameter,
                     RPM,
                     TipVelocity,
                     
                     ComponentGroupList_[c].SpanLoadData(i).Span_Cno(j),
                     ComponentGroupList_[c].SpanLoadData(i).Span_Cso(j),
                     ComponentGroupList_[c].SpanLoadData(i).Span_Cto(j),
                     ComponentGroupList_[c].SpanLoadData(i).Span_Cqo(j),
                     ComponentGroupList_[c].SpanLoadData(i).Span_Cpo(j),
        
                     ComponentGroupList_[c].SpanLoadData(i).Span_Cn(j),
                     ComponentGroupList_[c].SpanLoadData(i).Span_Cs(j),
                     ComponentGroupList_[c].SpanLoadData(i).Span_Ct(j),
                     ComponentGroupList_[c].SpanLoadData(i).Span_Cq(j),
                     ComponentGroupList_[c].SpanLoadData(i).Span_Cp(j));
        
        
           }
           
        }
                                  
    }
   
  //  else if ( TimeAccurate_ ) {
  
else if ( 1 ) {      
   
       // Loop over component groups, calculate rotor coefficients if any are
       // flagged as being rotors by the user.

       k = 0;
       
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
           
          // Write out group data

          Time = CurrentTime_;
          
          if ( NoiseAnalysis_ ) Time = CurrentNoiseTime_ + NoiseTimeShift_;

          if ( !TimeAccurate_ ) Time = CurrentWakeIteration_;
                    
          if ( Group == 0 || c == Group ) {
             
             fprintf(GroupFile_[c],"%10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f \n",
                     Time,
                     ComponentGroupList_[c].Cx() + ComponentGroupList_[c].Cxo(),
                     ComponentGroupList_[c].Cy() + ComponentGroupList_[c].Cyo(),
                     ComponentGroupList_[c].Cz() + ComponentGroupList_[c].Czo(),
                     ComponentGroupList_[c].Cxo(),
                     ComponentGroupList_[c].Cyo(),
                     ComponentGroupList_[c].Czo(),
                     ComponentGroupList_[c].Cx(),
                     ComponentGroupList_[c].Cy(),
                     ComponentGroupList_[c].Cz(),                                    
                     ComponentGroupList_[c].Cmx() + ComponentGroupList_[c].Cmxo(),
                     ComponentGroupList_[c].Cmy() + ComponentGroupList_[c].Cmyo(),
                     ComponentGroupList_[c].Cmz() + ComponentGroupList_[c].Cmzo(),
                     ComponentGroupList_[c].Cmxo(),
                     ComponentGroupList_[c].Cmyo(),
                     ComponentGroupList_[c].Cmzo(),
                     ComponentGroupList_[c].Cmx(),
                     ComponentGroupList_[c].Cmy(),
                     ComponentGroupList_[c].Cmz(),                                    
                     ComponentGroupList_[c].CL() + ComponentGroupList_[c].CLo(),
                     ComponentGroupList_[c].CD() + ComponentGroupList_[c].CDo(),
                     ComponentGroupList_[c].CLo(),
                     ComponentGroupList_[c].CDo(),   
                     ComponentGroupList_[c].CL(),                  
                     ComponentGroupList_[c].CD());             
                     
          }     
                                              
          // Write out rotor data
          
          if ( ComponentGroupList_[c].GeometryIsARotor() ) {
           
             k++;
              
             Diameter = ComponentGroupList_[c].RotorDiameter();
             
             RPM = ComponentGroupList_[c].Omega() * 60 / ( 2.*PI );
             
             // Forces

             Vec[0] = ComponentGroupList_[c].Cxo() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
             Vec[1] = ComponentGroupList_[c].Cyo() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
             Vec[2] = ComponentGroupList_[c].Czo() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
             
             Thrusto = vector_dot(Vec, ComponentGroupList_[c].RVec());
             
             Vec[0] = ComponentGroupList_[c].Cx() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
             Vec[1] = ComponentGroupList_[c].Cy() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
             Vec[2] = ComponentGroupList_[c].Cz() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
             
             Thrusti = vector_dot(Vec, ComponentGroupList_[c].RVec());
                 
             Thrust = Thrusto + Thrusti;
             
             // Moments
   
             Vec[0] = ComponentGroupList_[c].Cmxo() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Bref_;
             Vec[1] = ComponentGroupList_[c].Cmyo() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Cref_;
             Vec[2] = ComponentGroupList_[c].Cmzo() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Bref_;   
            
             Momento = -vector_dot(Vec, ComponentGroupList_[c].RVec());     
                
             Vec[0] = ComponentGroupList_[c].Cmx() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Bref_;
             Vec[1] = ComponentGroupList_[c].Cmy() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Cref_;
             Vec[2] = ComponentGroupList_[c].Cmz() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Bref_;   
             
             Momenti = -vector_dot(Vec, ComponentGroupList_[c].RVec());     
             
             Moment = Momento + Momenti;
             
             CalculateRotorCoefficientsFromForces(Thrust, Moment, Diameter, RPM, J, CT, CQ, CP, EtaP, CT_h, CQ_h, CP_h, FOM);
             
             Time = CurrentTime_;
          
             if ( NoiseAnalysis_ ) Time = CurrentNoiseTime_ + NoiseTimeShift_;
             
             if ( !TimeAccurate_ ) Time = CurrentWakeIteration_;
             
             if ( Group == 0 || c == Group ) {
                                      //  1      2      3      4      5      6      7      8      9     10     11     12     13     14     15     16     17     18     19   
                fprintf(RotorFile_[k],"%10.5f %10.3f %10.3f %10.3f %10.3f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f \n",
                        Time,
                        Diameter,
                        RPM,
                        Thrust,
                        Thrusto,
                        Thrusti,
                        Moment,
                        Momento,
                        Momenti,
                        J,
                        CT,
                        CQ,
                        CP,
                        EtaP,
                        CT_h,
                        CQ_h,
                        CP_h,
                        FOM,
                        ComponentGroupList_[c].TotalRotationAngle()/TORAD);
                        
             }
             
             // Average forces and moment data
             
             if ( Group == 0 ) {
                
                // Store time history of last rotation
                
                if ( ( TimeAccurate_ && CurrentTime_ >= ComponentGroupList_[c].StartAverageTime() ) || ( !TimeAccurate_ && CurrentWakeIteration_ >= WakeIterations_ ) ) ComponentGroupList_[c].UpdateAverageForcesAndMoments();
     
                // If this is the last time step, write out final averaged forces
                
                if ( ( TimeAccurate_ && Time_ == NumberOfTimeSteps_ ) || ( !TimeAccurate_ && CurrentWakeIteration_ >= WakeIterations_ ) ) {
                   
                   ComponentGroupList_[c].CalculateAverageForcesAndMoments();
                   
                   Diameter = ComponentGroupList_[c].RotorDiameter();
                   
                   RPM = ComponentGroupList_[c].Omega() * 60 / ( 2.*PI );
                   
                   // Forces
                   
                   Vec[0] = ComponentGroupList_[c].CxoAvg() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
                   Vec[1] = ComponentGroupList_[c].CyoAvg() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
                   Vec[2] = ComponentGroupList_[c].CzoAvg() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
                   
                   Thrusto = vector_dot(Vec, ComponentGroupList_[c].RVec());
                   
                   Vec[0] = ComponentGroupList_[c].CxAvg() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
                   Vec[1] = ComponentGroupList_[c].CyAvg() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
                   Vec[2] = ComponentGroupList_[c].CzAvg() * 0.5 * Density_ * Vref_ * Vref_ * Sref_;
                   
                   Thrusti = vector_dot(Vec, ComponentGroupList_[c].RVec());
                       
                   Thrust = Thrusto + Thrusti;
                   
                   // Moments
                   
                   Vec[0] = ComponentGroupList_[c].CmxoAvg() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Bref_;
                   Vec[1] = ComponentGroupList_[c].CmyoAvg() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Cref_;
                   Vec[2] = ComponentGroupList_[c].CmzoAvg() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Bref_;   
                   
                   Momento = -vector_dot(Vec, ComponentGroupList_[c].RVec());     
                      
                   Vec[0] = ComponentGroupList_[c].CmxAvg() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Bref_;
                   Vec[1] = ComponentGroupList_[c].CmyAvg() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Cref_;
                   Vec[2] = ComponentGroupList_[c].CmzAvg() * 0.5 * Density_ * Vref_ * Vref_ * Sref_ * Bref_;   
                   
                   Momenti = -vector_dot(Vec, ComponentGroupList_[c].RVec());     
                   
                   Moment = Momento + Momenti;
                   
                   CalculateRotorCoefficientsFromForces(Thrust, Moment, Diameter, RPM, J, CT, CQ, CP, EtaP, CT_h, CQ_h, CP_h, FOM);                   
                   
                   fprintf(RotorFile_[k],"\n\n");
                   fprintf(RotorFile_[k],"Average over last full revolution: \n");
                   fprintf(RotorFile_[k],"\n");
                                                    //  1      2      3      4      5      6      7      8      9     10     11     12     13     14     15     16     17     18  
                   fprintf(RotorFile_[k],"           %10.3f %10.3f %10.3f %10.3f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f \n",
                           Diameter,
                           RPM,
                           Thrust,
                           Thrusto,
                           Thrusti,
                           Moment,
                           Momento,
                           Momenti,
                           J,
                           CT,
                           CQ,
                           CP,
                           EtaP,
                           CT_h,
                           CQ_h,
                           CP_h,
                           FOM,
                           ComponentGroupList_[c].TotalRotationAngle()/TORAD);
                           
                   // Write out spanwise blade loading data, again averaged over one revolution
                   
                   for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfSurfaces() ; i++ ) {
 
                      fprintf(RotorFile_[k],"\n\n\n");
                      fprintf(RotorFile_[k],"Average loading for rotor blade: %d --> VSP Surface: %d ... NOTE: values are per station, not per area! \n",i,ComponentGroupList_[c].SpanLoadData(i).SurfaceID());
                      fprintf(RotorFile_[k],"\n");
                      
                                       //    123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123
                      fprintf(RotorFile_[k],"  Station     S       Chord     Area      V/Vref   Diameter    RPM      TipVel       CNo_H         CSo_H         CTo_H         CQo_H         CPo_H         CN_H          CS_H          CT_H          CQ_H          CP_H \n");
                      
                      ComponentGroupList_[c].SpanLoadData(i).CalculateAverageForcesAndMoments();
                      
                      // Write out averaged span loading data

                      Omega = 2. * PI * RPM / 60.;
                      
                      Radius = 0.5 * Diameter;
                      
                      Area = PI * Radius * Radius;
                      
                      TipVelocity = Omega * Radius;
                                           
                      for ( j = 1 ; j <= ComponentGroupList_[c].SpanLoadData(i).NumberOfSpanStations() ; j++ ) {

                        fprintf(RotorFile_[k],"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le \n",
                                j,
                                ComponentGroupList_[c].SpanLoadData(i).Span_S(j),                    
                                ComponentGroupList_[c].SpanLoadData(i).Span_Chord(j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Area(j),
                                
                                ComponentGroupList_[c].SpanLoadData(i).Local_Velocity(j),
                                
                                Diameter,
                                RPM,
                                TipVelocity,
                                
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cno(j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cso(j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cto(j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cqo(j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cpo(j),

                                ComponentGroupList_[c].SpanLoadData(i).Span_Cn(j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cs(j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Ct(j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cq(j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cp(j));

   
                      }
                      
                   }

                }
                
             }
                                      
          }
         
       }
      
    }
   
    else {
      
       printf("Unkown type of rotor analysis! \n");
       fflush(NULL);
       exit(1);
      
    }      

}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER CalculateRotorCoefficients                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateRotorCoefficientsFromForces(double Thrust, double Moment, double Diameter, double RPM,
                                                      double &J, double &CT, double &CQ, double &CP, double &EtaP,
                                                      double &CT_h, double &CQ_h, double &CP_h, double &FOM)
{
   
    double n, Omega, Radius, Area, TipVelocity;

    // Revs per second
    
    n = RPM / 60.;
    
    // Advance ratio
    
    J = Vinf_ / (n * Diameter);

    // Propeller coefficients
    
    CT = Thrust / ( Density_ * n * n * pow(Diameter, 4.) );
    CQ = Moment / ( Density_ * n * n * pow(Diameter, 5.) );
    CP = 2.*PI *CQ;
    
    EtaP = J * CT / (2.*PI*CQ);
    
    // Rotor (Helicopter) coefficients
    
    Omega = 2. * PI * RPM / 60.;
    
    Radius = 0.5 * Diameter;
    
    Area = PI * Radius * Radius;
    
    TipVelocity = Omega * Radius;
    
    CT_h = Thrust / ( Density_ * Area * pow(TipVelocity, 2.) );
    CQ_h = Moment / ( Density_ * Area * pow(TipVelocity, 2.) * Radius );
    CP_h = CQ_h;
    
    FOM = 0.;
    
    if ( CT_h > 0. ) FOM = pow(CT_h, 1.5)/(sqrt(2.)*CP_h);

}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER SetupPSUWopWopData                          #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::SetupPSUWopWopData(void)
{

    int c, i, j, k, NumberOfBlades, NumberOfWingSurfaces, NumberOfBodySurfaces;
    int LastComponentID, *ComponentInThisGroup, PowerOfTwo;
    
    // Unit conversions

    if ( WopWopEnglishUnits_ ) {
       
       WopWopDensityConversion_  = 515.379;
       
       WopWopLengthConversion_   = 0.3048;
       
       WopWopAreaConversion_     = WopWopLengthConversion_ * WopWopLengthConversion_;
       
       WopWopPressureConversion_ = 47.880172;
       
       WopWopForceConversion_    = 4.4482216;
       
    }
    
    else {
       
       WopWopDensityConversion_  = 1.;
       
       WopWopLengthConversion_   = 1.;
       
       WopWopAreaConversion_     = 1.;
       
       WopWopPressureConversion_ = 1.;     
       
       WopWopForceConversion_    = 1.;
       
    }  
    
    // Time steps
    
    if ( !TimeAccurate_ ) {

       WopWopdTau_ = (1./(BladeRPM_/60.))/128.;
       
       if ( !WopWopFlyBy_ ) WopWopdTau_ /= 10.;

    }
    
    else {
       
       WopWopdTau_ = NoiseTimeStep_;
       
    }    
    
    // Determine the number of rotors and blades per rotor

    ComponentInThisGroup = new int[VSPGeom().NumberOfComponents() + 1];
   
    WopWopNumberOfRotors_ = WopWopNumberOfWings_ = WopWopNumberOfBodies_ = WopWopNumberOfTotalBlades_ = WopWopNumberOfTotalWingSurfaces_ = WopWopNumberOfTtoalBodySurfaces_ = 0;
   
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       // Rotor
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          WopWopNumberOfRotors_++;
          
          zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());

          for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
          
             ComponentInThisGroup[ComponentGroupList_[c].ComponentList(j)] = 1;
             
          }
                 
          // Determine number of blades
                    
          NumberOfBlades = 0;
       
          for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
          
             if ( ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()] ) NumberOfBlades++;
             
          }

          ComponentGroupList_[c].WopWop().SizeBladeList(NumberOfBlades);
          
          ComponentGroupList_[c].WopWop().RotorID() = WopWopNumberOfRotors_;

          k = 0;
          
          for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
          
             if ( ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()] ) {
                
                ComponentGroupList_[c].WopWop().SurfaceForBlade(++k) = i;
             
                ComponentGroupList_[c].WopWop().NumberOfBladesSections() = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
         
             }
             
          }
          
          WopWopNumberOfTotalBlades_ += NumberOfBlades;
          
       }
       
    }
    
    // Check for possible wing surfaces

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       if ( !ComponentGroupList_[c].GeometryIsARotor() ) {
          
          zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());

          for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
          
             ComponentInThisGroup[ComponentGroupList_[c].ComponentList(j)] = 1;
             
          }
                 
          // Determine number of blades
                    
          NumberOfWingSurfaces = 0;
       
          for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
          
             if ( ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()] && VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) NumberOfWingSurfaces++;
             
          }
          
          if ( NumberOfWingSurfaces > 0 ) {
             
             printf("Found: %d wing surfaces in group: %d \n",NumberOfWingSurfaces, c);
             
             WopWopNumberOfWings_++;

             ComponentGroupList_[c].WopWop().SizeWingSurfaceList(NumberOfWingSurfaces);
             
             ComponentGroupList_[c].WopWop().WingID() = WopWopNumberOfWings_;
             
             ComponentGroupList_[c].GeometryHasWings() = 1;
   
             k = 0;
             
             for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
             
                if ( ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()] && VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
                   
                   ComponentGroupList_[c].WopWop().SurfaceForWing(++k) = i;
                
                   ComponentGroupList_[c].WopWop().NumberOfWingSections() = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
            
                }
                
             }
             
             WopWopNumberOfTotalWingSurfaces_ += NumberOfWingSurfaces;
             
          }          
          
       }
       
    }

    // Check for possible body surfaces

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       if ( !ComponentGroupList_[c].GeometryIsARotor() ) {
          
          zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());

          for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
          
             ComponentInThisGroup[ComponentGroupList_[c].ComponentList(j)] = 1;
             
          }
                 
          // Determine number of blades
                    
          NumberOfBodySurfaces = 0;
       
          LastComponentID = -999;
          
          for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
          
             if ( ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()] && VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
                
                if ( VSPGeom().VSP_Surface(i).ComponentID() != LastComponentID ) {
                   
                   NumberOfBodySurfaces++;
                   
                   LastComponentID = VSPGeom().VSP_Surface(i).ComponentID();
                   
                }
                
             }
             
          }
          
          if ( NumberOfBodySurfaces > 0 ) {
             
             printf("Found: %d body surfaces in group: %d \n",NumberOfBodySurfaces, c);
             
             WopWopNumberOfBodies_++;

             ComponentGroupList_[c].WopWop().SizeBodySurfaceList(NumberOfBodySurfaces);
             
             ComponentGroupList_[c].WopWop().BodyID() = WopWopNumberOfBodies_;
             
             ComponentGroupList_[c].GeometryHasBodies() = 1;
   
             LastComponentID = -999;
    
             k = 0;
             
             for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
             
                if ( ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()] && VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
                   
                   if ( VSPGeom().VSP_Surface(i).ComponentID() != LastComponentID ) {
                      
                      LastComponentID = VSPGeom().VSP_Surface(i).ComponentID();
                   
                      ComponentGroupList_[c].WopWop().SurfaceForBody(++k) = i;
                
                      ComponentGroupList_[c].WopWop().NumberOfBodySections() = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
                      
                   }
            
                }
                
             }
             
             WopWopNumberOfTtoalBodySurfaces_ += NumberOfWingSurfaces;
             
          }          
          
       }
       
    }
    
    delete [] ComponentInThisGroup;
    
    // Determine the slowest and fastest rotor
    
    WopWopOmegaMin_ =  1.e9;
    WopWopOmegaMax_ = -1.e9;
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          if ( ABS(ComponentGroupList_[c].Omega()) < WopWopOmegaMin_ ) {
          
             WopWopLongestPeriodGroup_ = c;
          
             WopWopOmegaMin_ = ABS(ComponentGroupList_[c].Omega());
             
          }

          if ( ABS(ComponentGroupList_[c].Omega()) > WopWopOmegaMax_ ) {

             WopWopOmegaMax_ = ABS(ComponentGroupList_[c].Omega());
             
          }
                    
       }
       
   }
   
   if ( TimeStep_ < 0. ) {

      TimeStep_ = 15. * TORAD / WopWopOmegaMax_;
      
      printf("Using a time step of: %f based on fastest rotor \n",TimeStep_);
      
   }   
   
   // 2 degrees per step
   
   NumberOfNoiseTimeSteps_ = 181;

   // Calculate start point of periodic noise analysis, this is
   // driven by the longest period motion
   
   WopWopLongestPeriod_ = 2.*PI/WopWopOmegaMin_;
   
   NoiseTimeStep_ = WopWopLongestPeriod_ / (NumberOfNoiseTimeSteps_ - 1);
   
   NoiseTimeShift_ = NumberOfTimeSteps_ * TimeStep_ - WopWopLongestPeriod_- TimeStep_;

   // Calculate observer time, make it a power of 2

   WopWopObserverTime_ = 5.*WopWopLongestPeriod_;

   WopWopNumberOfTimeSteps_ = 4 * ( WopWopObserverTime_ / NoiseTimeStep_ + 1);
   
   PowerOfTwo = 1;
   
   while ( 2*PowerOfTwo < WopWopNumberOfTimeSteps_ ) PowerOfTwo *= 2;

   WopWopNumberOfTimeSteps_ = PowerOfTwo;

   printf("PSU-WopWop Setup: \n");
   printf("Total Solution time:      %f \n",NumberOfTimeSteps_ * TimeStep_);
   printf("WopWopOmegaMin:           %f \n",WopWopOmegaMin_);
   printf("Longest Period:           %f \n",WopWopLongestPeriod_);
   printf("NumberOfNoiseTimeSteps_:  %d \n",NumberOfNoiseTimeSteps_);
   printf("NoiseTimeShift_:          %f \n",NoiseTimeShift_);
   printf("WopWopObserverTime_:      %f \n",WopWopObserverTime_);
   printf("WopWopNumberOfTimeSteps_: %d \n",WopWopNumberOfTimeSteps_);
   printf("\n");

}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER WriteOutPSUWopWopCaseAndNameListFiles                #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCaseAndNameListFiles(void)
{
  
  
   if ( WopWopFlyBy_ ) {
      
      WriteOutPSUWopWopCaseAndNameListFilesForFlyBy();
      
   }
   
   else {
      
      WriteOutPSUWopWopCaseAndNameListFilesForFootPrint();
      
   }
    
}

/*##############################################################################
#                                                                              #
#        VSP_SOLVER WriteOutPSUWopWopCaseAndNameListFilesForFlyBy              #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCaseAndNameListFilesForFlyByOld(void)
{
   
    int c, i, j;
    char NameListFile[2000];
    char PatchThicknessGeometryName[2000], PatchLoadingGeometryName[2000], WopWopFileName[2000];
    FILE *WopWopCaseFile;      

    // Cases namelist file
    
    if ( (WopWopCaseFile = fopen("cases.nam", "w")) == NULL ) {

       printf("Could not open the PSUWopWop Case File output! \n");

       exit(1);

    }    
    
    sprintf(NameListFile,"%s.nam",FileName_);
    
    fprintf(WopWopCaseFile,"&caseName\n");
    fprintf(WopWopCaseFile,"   globalFolderName = \'./\' \n");
    fprintf(WopWopCaseFile,"   caseNameFile = \'%s\' \n",NameListFile);
    fprintf(WopWopCaseFile,"/ \n");        
    
    fclose(WopWopCaseFile);
    
    // Actual namelist file
    
    if ( (PSUWopWopNameListFile_ = fopen(NameListFile, "w")) == NULL ) {

       printf("Could not open the PSUWopWop Namelist File output! \n");

       exit(1);

    } 
    
    // EnvironmentIn namelist
        
    fprintf(PSUWopWopNameListFile_,"!\n");    
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
    
    fprintf(PSUWopWopNameListFile_,"&EnvironmentIn\n");
    fprintf(PSUWopWopNameListFile_,"   nbSourceContainers     = 1       \n");    
    fprintf(PSUWopWopNameListFile_,"   nbObserverContainers   = 1       \n");    
    fprintf(PSUWopWopNameListFile_,"   ASCIIOutputFlag        = .true.  \n");
    fprintf(PSUWopWopNameListFile_,"   spectrumFlag           = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   SPLdBFlag              = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   SPLdBAFlag             = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   OASPLdBFlag            = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   OASPLdBAFlag           = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   acousticPressureFlag   = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   pressureGradient1AFlag = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   thicknessNoiseFlag     = .true.  \n");    
    fprintf(PSUWopWopNameListFile_,"   loadingNoiseFlag       = .true.  \n");    
    fprintf(PSUWopWopNameListFile_,"   totalNoiseFlag         = .true.  \n");    
    fprintf(PSUWopWopNameListFile_,"   sigmaFlag              = .false. \n");    
    fprintf(PSUWopWopNameListFile_,"   loadingSigmaFlag       = .false. \n");    
    fprintf(PSUWopWopNameListFile_,"   machSigmaFlag          = .false. \n");    
    fprintf(PSUWopWopNameListFile_,"   totalNoiseSigmaFlag    = .false. \n");     
    fprintf(PSUWopWopNameListFile_,"   audioFlag              = .true.  \n");
    fprintf(PSUWopWopNameListFile_,"   debugLevel = 5                   \n");    
 
    fprintf(PSUWopWopNameListFile_,"/ \n");  
       
    // EnvironmentConstants namelist

    fprintf(PSUWopWopNameListFile_,"!\n");      
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
        
    fprintf(PSUWopWopNameListFile_,"&EnvironmentConstants \n");
    
    // Note these are english unit versions of psu-WopWop's default values
    
    fprintf(PSUWopWopNameListFile_,"   rho         = %e \n",Density_*WopWopDensityConversion_);  // Density, kg/m^3

    fprintf(WopWopCaseFile,"/ \n");    
           
    // ObserverIn namelist
       
    fprintf(PSUWopWopNameListFile_,"!\n");      
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
           
    fprintf(PSUWopWopNameListFile_,"&ObserverIn  \n");
    fprintf(PSUWopWopNameListFile_,"  Title  = \'Observer_1\' \n");
    fprintf(PSUWopWopNameListFile_,"  tMin = 0.0 \n");
 
    if ( WopWopFlyBy_ ) {
       
       fprintf(PSUWopWopNameListFile_,"  tMax = %f  \n",24.);
       
    }
    
    else {
       
       fprintf(PSUWopWopNameListFile_,"  tMax = %f  \n",5./(BladeRPM_/60.));
                     
    }

    // Fly by
    
    if ( WopWopFlyBy_ ) {
       
       fprintf(PSUWopWopNameListFile_,"  nt                = 262144 \n");
       fprintf(PSUWopWopNameListFile_,"  xloc              = 0.0   \n");
       fprintf(PSUWopWopNameListFile_,"  yloc              = 0.0   \n");
       fprintf(PSUWopWopNameListFile_,"  zloc              = 0.0   \n");
       fprintf(PSUWopWopNameListFile_,"  highPassFrequency = 10.0  \n");
 
       fprintf(PSUWopWopNameListFile_,"/ \n");        
   
    }
    
    // Fixed to aircraft
    
    else {
       
       fprintf(PSUWopWopNameListFile_,"  nt                = %d \n",WopWopNumberOfTimeSteps_);
       fprintf(PSUWopWopNameListFile_,"  highPassFrequency = 10.0 \n");
       fprintf(PSUWopWopNameListFile_,"  attachedTo        = \'Aircraft\' \n");
       fprintf(PSUWopWopNameListFile_,"  nbBaseObsContFrame = 1  \n");

       fprintf(PSUWopWopNameListFile_,"/ \n");        
                 
    }

    // Observer CB
    
    if ( !WopWopFlyBy_ ) {

       fprintf(PSUWopWopNameListFile_,"!\n");      
       fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
       fprintf(PSUWopWopNameListFile_,"!\n");
       
       fprintf(PSUWopWopNameListFile_,"&CB  \n");
       fprintf(PSUWopWopNameListFile_,"  Title  = \'Observer\' \n");
       fprintf(PSUWopWopNameListFile_,"  translationType=\'TimeIndependent\' \n"); 
       fprintf(PSUWopWopNameListFile_,"  TranslationValue = 0.0, -500., 0. \n");                

       fprintf(PSUWopWopNameListFile_,"/ \n");        
       
    }

    // ContainerIn namelist - Aircraft

    fprintf(PSUWopWopNameListFile_,"!\n");      
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
        
    fprintf(PSUWopWopNameListFile_,"&ContainerIn  \n");
    fprintf(PSUWopWopNameListFile_,"   Title        = \'Aircraft\' \n");
    fprintf(PSUWopWopNameListFile_,"   nbContainer  = %d \n",WopWopNumberOfRotors_);
    fprintf(PSUWopWopNameListFile_,"   nbBase = 1 \n");
    fprintf(PSUWopWopNameListFile_,"   dtau = %e \n",WopWopdTau_);
    
    fprintf(PSUWopWopNameListFile_,"/ \n");   
    
    // Change of Base namelist for forward motion of aircraft

    fprintf(PSUWopWopNameListFile_,"&CB \n");
    fprintf(PSUWopWopNameListFile_,"   Title           = \'Velocity\' \n");
    fprintf(PSUWopWopNameListFile_,"   translationType = \'KnownFunction\' \n");
    fprintf(PSUWopWopNameListFile_,"   AH = 0.0, 0.0, 0.0 \n");
    fprintf(PSUWopWopNameListFile_,"   VH = %f,  0.0, 0.0 \n",-Vinf_*WopWopLengthConversion_);
    fprintf(PSUWopWopNameListFile_,"   Y0 = %f, %f, %f    \n",0.0, 0.0, 0.0);
    fprintf(PSUWopWopNameListFile_,"/ \n");        

    // Loop over each rotor

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          i = ComponentGroupList_[c].WopWop().RotorID();

          // ContainerIn for current rotor
      
          fprintf(PSUWopWopNameListFile_,"!\n");      
          fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          fprintf(PSUWopWopNameListFile_,"!\n");
       
          fprintf(PSUWopWopNameListFile_,"      &ContainerIn  \n");
          fprintf(PSUWopWopNameListFile_,"         Title        = \'Rotor_%d\' \n",i);
          fprintf(PSUWopWopNameListFile_,"         nbContainer  = %d \n",2*ComponentGroupList_[c].WopWop().NumberOfBlades());
          fprintf(PSUWopWopNameListFile_,"         BPMNoiseFlag           = .false. \n");
          fprintf(PSUWopWopNameListFile_,"         PeggNoiseFlag          = .false. \n");
   
          if ( SteadyStateNoise_ ) {
             
              fprintf(PSUWopWopNameListFile_,"         nbBase = 1 \n");
      
          }
          
          fprintf(PSUWopWopNameListFile_,"      / \n");     
   
          // Change of Base namelist if we are not time accurate, and let WopWop do the unsteady stuff
   
          fprintf(PSUWopWopNameListFile_,"!\n");      
          fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          fprintf(PSUWopWopNameListFile_,"!\n");
                 
          if ( SteadyStateNoise_ ) {
            
             fprintf(PSUWopWopNameListFile_,"      &CB \n");
             fprintf(PSUWopWopNameListFile_,"         Title     = \'Rotation\' \n");
             fprintf(PSUWopWopNameListFile_,"         rotation  = .true. \n");
             fprintf(PSUWopWopNameListFile_,"         AngleType = \'KnownFunction\' \n");
             fprintf(PSUWopWopNameListFile_,"         Omega     =  %f \n",-BladeRPM_*2.*PI/60.);
             fprintf(PSUWopWopNameListFile_,"         AxisValue = 1.0,0.0,0.0 \n");
             fprintf(PSUWopWopNameListFile_,"      / \n");        
             
          }
   
          // Broad band noise: BPM namelist
   
          fprintf(PSUWopWopNameListFile_,"!\n");      
          fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          fprintf(PSUWopWopNameListFile_,"!\n");
          
          fprintf(PSUWopWopNameListFile_,"      &BPMIn \n");
          fprintf(PSUWopWopNameListFile_,"         BPMNoiseFile = %s.PSUWopWop.BPM.Rotor.%d.dat \n",FileName_,i);
          fprintf(PSUWopWopNameListFile_,"         nSect           = %d \n",ComponentGroupList_[c].WopWop().NumberOfBladesSections());
          fprintf(PSUWopWopNameListFile_,"         uniformBlade    = 0 \n");
          fprintf(PSUWopWopNameListFile_,"         BLtrip          = 0 \n");
          fprintf(PSUWopWopNameListFile_,"         sectChordFlag   = 'FILEVALUE' \n");
          fprintf(PSUWopWopNameListFile_,"         sectLengthFlag  = 'COMPUTE'   \n");
          fprintf(PSUWopWopNameListFile_,"         TEThicknessFlag = 'FILEVALUE' \n");
          fprintf(PSUWopWopNameListFile_,"         TEflowAngleFlag = 'FILEVALUE' \n");
          fprintf(PSUWopWopNameListFile_,"         TipLCSFlag      = 'FILEVALUE' \n");
          fprintf(PSUWopWopNameListFile_,"         SectAOAFlag     = 'FILEVALUE' \n");
          fprintf(PSUWopWopNameListFile_,"         SectAOAFlag     = 'FILEVALUE' \n");
          fprintf(PSUWopWopNameListFile_,"         LBLVSnoise      = .true.  \n");
          fprintf(PSUWopWopNameListFile_,"         TBLTEnoise      = .true.  \n");
          fprintf(PSUWopWopNameListFile_,"         bluntNoise      = .true.  \n");
          fprintf(PSUWopWopNameListFile_,"         bladeTipNoise   = .true.  \n");
          fprintf(PSUWopWopNameListFile_,"      / \n");
    
          // Broad band noise: PeggIn namelist
          
          fprintf(PSUWopWopNameListFile_,"!\n");      
          fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          fprintf(PSUWopWopNameListFile_,"!\n");
                 
          WriteOutPSUWopWopPeggNamelist();
          
          // Write out containers for each of the blades... one for loading, one for thickness
          
          for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {
          
             sprintf(PatchLoadingGeometryName  ,"%s.PSUWopWop.Loading.Geometry.Rotor.%d.Blade.%d.dat",  FileName_,i,j);
             sprintf(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Rotor.%d.Blade.%d.dat",FileName_,i,j);
             sprintf(WopWopFileName,          "%s.PSUWopWop.Loading.Rotor.%d.Blade.%d.dat",           FileName_,i,j);
               
             // ContainerIn for Aero loading geometry and load file
      
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
         
             fprintf(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             fprintf(PSUWopWopNameListFile_,"               Title        = \'Blade_%d_For_Rotor_%d_Loading\' \n",j,i);
             fprintf(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             fprintf(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchLoadingGeometryName);
             fprintf(PSUWopWopNameListFile_,"               patchLoadingFile   = %s \n",WopWopFileName);
             fprintf(PSUWopWopNameListFile_,"            / \n");     
          
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
    
             // ContainerIn for Aero thickness model
             
             fprintf(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             fprintf(PSUWopWopNameListFile_,"               Title        = \'Blade_%d_For_Rotor_%d_Thickness\' \n",j,i);
             fprintf(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             fprintf(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
             fprintf(PSUWopWopNameListFile_,"            / \n");               
                
          }
       
       }
          
    }
        
}

/*##############################################################################
#                                                                              #
#        VSP_SOLVER WriteOutPSUWopWopCaseAndNameListFilesForFlyBy              #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCaseAndNameListFilesForFlyBy(void)
{
   
    int c, i, j;
    char NameListFile[2000];
    char PatchThicknessGeometryName[2000], PatchLoadingGeometryName[2000], WopWopFileName[2000];
    FILE *WopWopCaseFile;      

    // Cases namelist file
    
    if ( (WopWopCaseFile = fopen("cases.nam", "w")) == NULL ) {

       printf("Could not open the PSUWopWop Case File output! \n");

       exit(1);

    }    
    
    sprintf(NameListFile,"%s.nam",FileName_);
    
    fprintf(WopWopCaseFile,"&caseName\n");
    fprintf(WopWopCaseFile,"   globalFolderName = \'./\' \n");
    fprintf(WopWopCaseFile,"   caseNameFile = \'%s\' \n",NameListFile);
    fprintf(WopWopCaseFile,"/ \n");        
    
    fclose(WopWopCaseFile);
    
    // Actual namelist file
    
    if ( (PSUWopWopNameListFile_ = fopen(NameListFile, "w")) == NULL ) {

       printf("Could not open the PSUWopWop Namelist File output! \n");

       exit(1);

    } 
    
    // EnvironmentIn namelist
        
    fprintf(PSUWopWopNameListFile_,"!\n");    
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
    
    fprintf(PSUWopWopNameListFile_,"&EnvironmentIn\n");
    fprintf(PSUWopWopNameListFile_,"   nbSourceContainers     = 1       \n");    
    fprintf(PSUWopWopNameListFile_,"   nbObserverContainers   = 1       \n");    
    fprintf(PSUWopWopNameListFile_,"   ASCIIOutputFlag        = .true.  \n");
    fprintf(PSUWopWopNameListFile_,"   spectrumFlag           = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   SPLdBFlag              = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   SPLdBAFlag             = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   OASPLdBFlag            = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   OASPLdBAFlag           = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   acousticPressureFlag   = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   pressureGradient1AFlag = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   thicknessNoiseFlag     = .true.  \n");    
    fprintf(PSUWopWopNameListFile_,"   loadingNoiseFlag       = .true.  \n");    
    fprintf(PSUWopWopNameListFile_,"   totalNoiseFlag         = .true.  \n");    
    fprintf(PSUWopWopNameListFile_,"   sigmaFlag              = .false. \n");    
    fprintf(PSUWopWopNameListFile_,"   loadingSigmaFlag       = .false. \n");    
    fprintf(PSUWopWopNameListFile_,"   machSigmaFlag          = .false. \n");    
    fprintf(PSUWopWopNameListFile_,"   totalNoiseSigmaFlag    = .false. \n");     
    fprintf(PSUWopWopNameListFile_,"   audioFlag              = .true.  \n");
    fprintf(PSUWopWopNameListFile_,"   debugLevel = 5                   \n");    
 
    fprintf(PSUWopWopNameListFile_,"/ \n");  
       
    // EnvironmentConstants namelist

    fprintf(PSUWopWopNameListFile_,"!\n");      
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
        
    fprintf(PSUWopWopNameListFile_,"&EnvironmentConstants \n");
    
    // Note these are english unit versions of psu-WopWop's default values
    
    fprintf(PSUWopWopNameListFile_,"   rho         = %e \n",Density_*WopWopDensityConversion_);  // Density, kg/m^3

    fprintf(WopWopCaseFile,"/ \n");    
           
    // ObserverIn namelist
       
    fprintf(PSUWopWopNameListFile_,"!\n");      
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
           
    fprintf(PSUWopWopNameListFile_,"&ObserverIn  \n");
    fprintf(PSUWopWopNameListFile_,"  Title             = \'Observer_1\' \n");
    fprintf(PSUWopWopNameListFile_,"  tMin              =  0.0 \n");
    fprintf(PSUWopWopNameListFile_,"  tMax              = 30.0 \n");
    fprintf(PSUWopWopNameListFile_,"  highPassFrequency = %f \n",10.);
    fprintf(PSUWopWopNameListFile_,"  nt                = 32768 \n");

    // Fixed fly by location

    fprintf(PSUWopWopNameListFile_,"  xLoc              = %f \n",0.);
    fprintf(PSUWopWopNameListFile_,"  yLoc              = %f \n",0.);
    fprintf(PSUWopWopNameListFile_,"  zLoc              = %f \n",0.);

    // OASPLdB inputs
    
    fprintf(PSUWopWopNameListFile_,"  segmentSize       =  1.0  \n");
    fprintf(PSUWopWopNameListFile_,"  segmentStepSize   =  0.1  \n");

    fprintf(PSUWopWopNameListFile_,"/ \n");        

    // ContainerIn namelist - Aircraft

    fprintf(PSUWopWopNameListFile_,"!\n");      
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
        
    fprintf(PSUWopWopNameListFile_,"&ContainerIn  \n");
    fprintf(PSUWopWopNameListFile_,"   Title        = \'Aircraft\' \n");
    fprintf(PSUWopWopNameListFile_,"   nbContainer  = %d \n",WopWopNumberOfRotors_ + WopWopNumberOfWings_ + WopWopNumberOfBodies_);
    fprintf(PSUWopWopNameListFile_,"   nbBase = 2 \n");
    fprintf(PSUWopWopNameListFile_,"   dtau = %e \n",WopWopdTau_);
    
    fprintf(PSUWopWopNameListFile_,"/ \n");   
        

    // Change of Base namelist for attitude of aircraft

    fprintf(PSUWopWopNameListFile_,"&CB \n");
    fprintf(PSUWopWopNameListFile_,"   Title           = \'Euler Angle Rotation\' \n");
    fprintf(PSUWopWopNameListFile_,"   translationType = \'TimeIndependent\' \n");
    fprintf(PSUWopWopNameListFile_,"   angleValue      = %f \n",AngleOfAttack_);
    fprintf(PSUWopWopNameListFile_,"   axisValue       = %f, %f, %f \n",0.,1.,0.);
    fprintf(PSUWopWopNameListFile_,"/ \n");   
            
    // Change of Base namelist for forward motion of aircraft

    fprintf(PSUWopWopNameListFile_,"&CB \n");
    fprintf(PSUWopWopNameListFile_,"   Title           = \'Velocity\' \n");
    fprintf(PSUWopWopNameListFile_,"   translationType = \'KnownFunction\' \n");
    fprintf(PSUWopWopNameListFile_,"   AH = 0.0, 0.0, 0.0 \n");
    fprintf(PSUWopWopNameListFile_,"   VH = %f,  0.0, 0.0 \n",-Vinf_*WopWopLengthConversion_);
    fprintf(PSUWopWopNameListFile_,"   Y0 = %f, %f, %f    \n",Vinf_*WopWopLengthConversion_*15., 0.0, 50.0);
    fprintf(PSUWopWopNameListFile_,"/ \n");        

    // Loop over each rotor
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          i = ComponentGroupList_[c].WopWop().RotorID();

          // ContainerIn for current rotor
      
          fprintf(PSUWopWopNameListFile_,"!\n");      
          fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          fprintf(PSUWopWopNameListFile_,"!\n");
       
          fprintf(PSUWopWopNameListFile_,"      &ContainerIn  \n");
          fprintf(PSUWopWopNameListFile_,"         Title        = \'Rotor_%d\' \n",i);
          fprintf(PSUWopWopNameListFile_,"         nbContainer  = %d \n",2*ComponentGroupList_[c].WopWop().NumberOfBlades());
          fprintf(PSUWopWopNameListFile_,"         BPMNoiseFlag           = .false. \n");
          fprintf(PSUWopWopNameListFile_,"         PeggNoiseFlag          = .false. \n");
   
          // Possible change of base for steady state case where PSUWOPWOP takes care of the blade rotation
          
          if ( SteadyStateNoise_ ) {
             
             fprintf(PSUWopWopNameListFile_,"         nbBase = 1 \n");
      
          }
          
          fprintf(PSUWopWopNameListFile_,"      / \n");     
   
          // Change of Base namelist if we are not time accurate, and let WopWop do the unsteady stuff
        
          if ( SteadyStateNoise_ ) {

             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
            
             fprintf(PSUWopWopNameListFile_,"      &CB \n");
             fprintf(PSUWopWopNameListFile_,"         Title     = \'Rotation\' \n");
             fprintf(PSUWopWopNameListFile_,"         rotation  = .true. \n");
             fprintf(PSUWopWopNameListFile_,"         AngleType = \'KnownFunction\' \n");
             fprintf(PSUWopWopNameListFile_,"         Omega     =  %f \n",-BladeRPM_*2.*PI/60.);
             fprintf(PSUWopWopNameListFile_,"         AxisValue = 1.0,0.0,0.0 \n");
             fprintf(PSUWopWopNameListFile_,"      / \n");        
             
          }
   
          if ( SteadyStateNoise_ ) {
             
             // Broad band noise: BPM namelist
      
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
             
             fprintf(PSUWopWopNameListFile_,"      &BPMIn \n");
             fprintf(PSUWopWopNameListFile_,"         BPMNoiseFile = %s.PSUWopWop.BPM.Rotor.%d.dat \n",FileName_,i);
             fprintf(PSUWopWopNameListFile_,"         nSect           = %d \n",ComponentGroupList_[c].WopWop().NumberOfBladesSections());
             fprintf(PSUWopWopNameListFile_,"         uniformBlade    = 0 \n");
             fprintf(PSUWopWopNameListFile_,"         BLtrip          = 0 \n");
             fprintf(PSUWopWopNameListFile_,"         sectChordFlag   = 'FILEVALUE' \n");
             fprintf(PSUWopWopNameListFile_,"         sectLengthFlag  = 'COMPUTE'   \n");
             fprintf(PSUWopWopNameListFile_,"         TEThicknessFlag = 'FILEVALUE' \n");
             fprintf(PSUWopWopNameListFile_,"         TEflowAngleFlag = 'FILEVALUE' \n");
             fprintf(PSUWopWopNameListFile_,"         TipLCSFlag      = 'FILEVALUE' \n");
             fprintf(PSUWopWopNameListFile_,"         SectAOAFlag     = 'FILEVALUE' \n");
             fprintf(PSUWopWopNameListFile_,"         SectAOAFlag     = 'FILEVALUE' \n");
             fprintf(PSUWopWopNameListFile_,"         LBLVSnoise      = .true.  \n");
             fprintf(PSUWopWopNameListFile_,"         TBLTEnoise      = .true.  \n");
             fprintf(PSUWopWopNameListFile_,"         bluntNoise      = .true.  \n");
             fprintf(PSUWopWopNameListFile_,"         bladeTipNoise   = .true.  \n");
             fprintf(PSUWopWopNameListFile_,"      / \n");
       
             // Broad band noise: PeggIn namelist
             
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
                    
             WriteOutPSUWopWopPeggNamelist();
             
          }
          
          // Write out containers for each of the blades... one for loading, one for thickness
          
          for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {
          
             sprintf(PatchLoadingGeometryName  ,"%s.PSUWopWop.Loading.Geometry.Rotor.%d.Blade.%d.dat",  FileName_,i,j);
             sprintf(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Rotor.%d.Blade.%d.dat",FileName_,i,j);
             sprintf(WopWopFileName,            "%s.PSUWopWop.Loading.Rotor.%d.Blade.%d.dat",           FileName_,i,j);
               
             // ContainerIn for Aero loading geometry and load file
      
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
         
             fprintf(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             fprintf(PSUWopWopNameListFile_,"               Title        = \'Blade_%d_For_Rotor_%d_Loading\' \n",j,i);
             fprintf(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             fprintf(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchLoadingGeometryName);
             fprintf(PSUWopWopNameListFile_,"               patchLoadingFile   = %s \n",WopWopFileName);
             fprintf(PSUWopWopNameListFile_,"            / \n");     
          
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
    
             // ContainerIn for Aero thickness model
             
             fprintf(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             fprintf(PSUWopWopNameListFile_,"               Title        = \'Blade_%d_For_Rotor_%d_Thickness\' \n",j,i);
             fprintf(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             fprintf(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
             fprintf(PSUWopWopNameListFile_,"            / \n");               
                
          }
             
       }
       
    }

    // Loop over each wing
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryHasWings() ) {
          
          i = ComponentGroupList_[c].WopWop().WingID();

          // ContainerIn for current wing
      
          fprintf(PSUWopWopNameListFile_,"!\n");      
          fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          fprintf(PSUWopWopNameListFile_,"!\n");
       
          fprintf(PSUWopWopNameListFile_,"      &ContainerIn  \n");
          fprintf(PSUWopWopNameListFile_,"         Title        = \'Wing_%d\' \n",i);
          fprintf(PSUWopWopNameListFile_,"         nbContainer  = %d \n",2*ComponentGroupList_[c].WopWop().NumberOfWingSurfaces());
          fprintf(PSUWopWopNameListFile_,"      / \n");     
   
          // Write out containers for each of the wing surfaces... one for loading, one for thickness
          
          for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {
          
             sprintf(PatchLoadingGeometryName  ,"%s.PSUWopWop.Loading.Geometry.Wing.%d.Surface.%d.dat",  FileName_,i,j);
             sprintf(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Wing.%d.Surface.%d.dat",FileName_,i,j);
             sprintf(WopWopFileName,            "%s.PSUWopWop.Loading.Wing.%d.Surface.%d.dat",           FileName_,i,j);
               
             // ContainerIn for Aero loading geometry and load file
      
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
         
             fprintf(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             fprintf(PSUWopWopNameListFile_,"               Title        = \'Surface_%d_For_Wing_%d_Loading\' \n",j,i);
             fprintf(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             fprintf(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchLoadingGeometryName);
             fprintf(PSUWopWopNameListFile_,"               patchLoadingFile   = %s \n",WopWopFileName);
             fprintf(PSUWopWopNameListFile_,"            / \n");     
          
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
    
             // ContainerIn for Aero thickness model
             
             fprintf(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             fprintf(PSUWopWopNameListFile_,"               Title        = \'Surface_%d_For_Wing_%d_Thickness\' \n",j,i);
             fprintf(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             fprintf(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
             fprintf(PSUWopWopNameListFile_,"            / \n");               
                
          }
             
       }
       
    }

    // Loop over each body
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryHasBodies() ) {
          
          i = ComponentGroupList_[c].WopWop().BodyID();

          // ContainerIn for current body
      
          fprintf(PSUWopWopNameListFile_,"!\n");      
          fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          fprintf(PSUWopWopNameListFile_,"!\n");
       
          fprintf(PSUWopWopNameListFile_,"      &ContainerIn  \n");
          fprintf(PSUWopWopNameListFile_,"         Title        = \'Body_%d\' \n",i);
          fprintf(PSUWopWopNameListFile_,"         nbContainer  = %d \n",ComponentGroupList_[c].WopWop().NumberOfBodySurfaces());
          fprintf(PSUWopWopNameListFile_,"      / \n");     
   
          // Write out containers for each of the bodies... just the thickness
          
          for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBodySurfaces() ; j++ ) {
          
             sprintf(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Body.%d.Surface.%d.dat",FileName_,i,j);

             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
    
             // ContainerIn for Aero thickness model
             
             fprintf(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             fprintf(PSUWopWopNameListFile_,"               Title        = \'Surface_%d_For_Body_%d_Thickness\' \n",j,i);
             fprintf(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             fprintf(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
             fprintf(PSUWopWopNameListFile_,"            / \n");               
                
          }
             
       }
       
    }
                
}

/*##############################################################################
#                                                                              #
#      VSP_SOLVER WriteOutPSUWopWopCaseAndNameListFilesForFootPrint            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCaseAndNameListFilesForFootPrint(void)
{
   
    int c, i, j;
    char NameListFile[2000];
    char PatchThicknessGeometryName[2000], PatchLoadingGeometryName[2000], WopWopFileName[2000];
    FILE *WopWopCaseFile;      

    // Cases namelist file
    
    if ( (WopWopCaseFile = fopen("cases.nam", "w")) == NULL ) {

       printf("Could not open the PSUWopWop Case File output! \n");

       exit(1);

    }    
    
    sprintf(NameListFile,"%s.nam",FileName_);
    
    fprintf(WopWopCaseFile,"&caseName\n");
    fprintf(WopWopCaseFile,"   globalFolderName = \'./\' \n");
    fprintf(WopWopCaseFile,"   caseNameFile = \'%s\' \n",NameListFile);
    fprintf(WopWopCaseFile,"/ \n");        
    
    fclose(WopWopCaseFile);
    
    // Actual namelist file
    
    if ( (PSUWopWopNameListFile_ = fopen(NameListFile, "w")) == NULL ) {

       printf("Could not open the PSUWopWop Namelist File output! \n");

       exit(1);

    } 
    
    // EnvironmentIn namelist
        
    fprintf(PSUWopWopNameListFile_,"!\n");    
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
    
    fprintf(PSUWopWopNameListFile_,"&EnvironmentIn\n");
    fprintf(PSUWopWopNameListFile_,"   nbSourceContainers     = 1       \n");    
    fprintf(PSUWopWopNameListFile_,"   nbObserverContainers   = 1       \n");    
    fprintf(PSUWopWopNameListFile_,"   ASCIIOutputFlag        = .true.  \n");
    fprintf(PSUWopWopNameListFile_,"   spectrumFlag           = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   SPLdBFlag              = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   SPLdBAFlag             = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   OASPLdBFlag            = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   OASPLdBAFlag           = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   acousticPressureFlag   = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   pressureGradient1AFlag = .true.  \n");     
    fprintf(PSUWopWopNameListFile_,"   thicknessNoiseFlag     = .true.  \n");    
    fprintf(PSUWopWopNameListFile_,"   loadingNoiseFlag       = .true.  \n");    
    fprintf(PSUWopWopNameListFile_,"   totalNoiseFlag         = .true.  \n");    
    fprintf(PSUWopWopNameListFile_,"   sigmaFlag              = .false. \n");    
    fprintf(PSUWopWopNameListFile_,"   loadingSigmaFlag       = .false. \n");    
    fprintf(PSUWopWopNameListFile_,"   machSigmaFlag          = .false. \n");    
    fprintf(PSUWopWopNameListFile_,"   totalNoiseSigmaFlag    = .false. \n");     
    fprintf(PSUWopWopNameListFile_,"   audioFlag              = .true.  \n");
    fprintf(PSUWopWopNameListFile_,"   debugLevel = 5                   \n");    
 
    fprintf(PSUWopWopNameListFile_,"/ \n");  
       
    // EnvironmentConstants namelist

    fprintf(PSUWopWopNameListFile_,"!\n");      
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
        
    fprintf(PSUWopWopNameListFile_,"&EnvironmentConstants \n");
    
    // Note these are english unit versions of psu-WopWop's default values
    
    fprintf(PSUWopWopNameListFile_,"   rho         = %e \n",Density_*WopWopDensityConversion_);  // Density, kg/m^3

    fprintf(WopWopCaseFile,"/ \n");    
           
    // ObserverIn namelist
       
    fprintf(PSUWopWopNameListFile_,"!\n");      
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
           
    fprintf(PSUWopWopNameListFile_,"&ObserverIn  \n");
    fprintf(PSUWopWopNameListFile_,"  Title  = \'Observer_1\' \n");
    fprintf(PSUWopWopNameListFile_,"  tMin = 0.0 \n");
    fprintf(PSUWopWopNameListFile_,"  tMax = %f  \n",WopWopObserverTime_);

    // Fixed to aircraft
       
    fprintf(PSUWopWopNameListFile_,"  nt                = %d \n",WopWopNumberOfTimeSteps_);
    fprintf(PSUWopWopNameListFile_,"  highPassFrequency = %f \n",10.);
    fprintf(PSUWopWopNameListFile_,"  attachedTo        = \'Aircraft\' \n");
    fprintf(PSUWopWopNameListFile_,"  nbBaseObsContFrame = 1  \n");
    
    fprintf(PSUWopWopNameListFile_,"/ \n");        
                 
    // Observer CB

    fprintf(PSUWopWopNameListFile_,"!\n");      
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
    
    fprintf(PSUWopWopNameListFile_,"&CB  \n");
    fprintf(PSUWopWopNameListFile_,"  Title  = \'Observer\' \n");
    fprintf(PSUWopWopNameListFile_,"  translationType=\'TimeIndependent\' \n"); 
    fprintf(PSUWopWopNameListFile_,"  TranslationValue = 0.0, -500., 0. \n");                

    fprintf(PSUWopWopNameListFile_,"/ \n");        
       
    // ContainerIn namelist - Aircraft

    fprintf(PSUWopWopNameListFile_,"!\n");      
    fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    fprintf(PSUWopWopNameListFile_,"!\n");
        
    fprintf(PSUWopWopNameListFile_,"&ContainerIn  \n");
    fprintf(PSUWopWopNameListFile_,"   Title        = \'Aircraft\' \n");
    fprintf(PSUWopWopNameListFile_,"   nbContainer  = %d \n",WopWopNumberOfRotors_ + WopWopNumberOfWings_ + WopWopNumberOfBodies_);
    fprintf(PSUWopWopNameListFile_,"   nbBase = 1 \n");
    fprintf(PSUWopWopNameListFile_,"   dtau = %e \n",WopWopdTau_);
    
    fprintf(PSUWopWopNameListFile_,"/ \n");   
    
    // Change of Base namelist for forward motion of aircraft

    fprintf(PSUWopWopNameListFile_,"&CB \n");
    fprintf(PSUWopWopNameListFile_,"   Title           = \'Velocity\' \n");
    fprintf(PSUWopWopNameListFile_,"   translationType = \'KnownFunction\' \n");
    fprintf(PSUWopWopNameListFile_,"   AH = 0.0, 0.0, 0.0 \n");
    fprintf(PSUWopWopNameListFile_,"   VH = %f,  0.0, 0.0 \n",-Vinf_*WopWopLengthConversion_);
    fprintf(PSUWopWopNameListFile_,"   Y0 = %f, %f, %f    \n",0.0, 0.0, 0.0);
    fprintf(PSUWopWopNameListFile_,"/ \n");        

    // Loop over each rotor
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          i = ComponentGroupList_[c].WopWop().RotorID();

          // ContainerIn for current rotor
      
          fprintf(PSUWopWopNameListFile_,"!\n");      
          fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          fprintf(PSUWopWopNameListFile_,"!\n");
       
          fprintf(PSUWopWopNameListFile_,"      &ContainerIn  \n");
          fprintf(PSUWopWopNameListFile_,"         Title        = \'Rotor_%d\' \n",i);
          fprintf(PSUWopWopNameListFile_,"         nbContainer  = %d \n",2*ComponentGroupList_[c].WopWop().NumberOfBlades());
          fprintf(PSUWopWopNameListFile_,"         BPMNoiseFlag           = .false. \n");
          fprintf(PSUWopWopNameListFile_,"         PeggNoiseFlag          = .false. \n");
   
          // Possible change of base for steady state case where PSUWOPWOP takes care of the blade rotation
          
          if ( SteadyStateNoise_ ) {
             
             fprintf(PSUWopWopNameListFile_,"         nbBase = 1 \n");
      
          }
          
          fprintf(PSUWopWopNameListFile_,"      / \n");     
   
          // Change of Base namelist if we are not time accurate, and let WopWop do the unsteady stuff
        
          if ( SteadyStateNoise_ ) {

             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
            
             fprintf(PSUWopWopNameListFile_,"      &CB \n");
             fprintf(PSUWopWopNameListFile_,"         Title     = \'Rotation\' \n");
             fprintf(PSUWopWopNameListFile_,"         rotation  = .true. \n");
             fprintf(PSUWopWopNameListFile_,"         AngleType = \'KnownFunction\' \n");
             fprintf(PSUWopWopNameListFile_,"         Omega     =  %f \n",-BladeRPM_*2.*PI/60.);
             fprintf(PSUWopWopNameListFile_,"         AxisValue = 1.0,0.0,0.0 \n");
             fprintf(PSUWopWopNameListFile_,"      / \n");        
             
          }
   
          if ( SteadyStateNoise_ ) {
             
             // Broad band noise: BPM namelist
      
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
             
             fprintf(PSUWopWopNameListFile_,"      &BPMIn \n");
             fprintf(PSUWopWopNameListFile_,"         BPMNoiseFile = %s.PSUWopWop.BPM.Rotor.%d.dat \n",FileName_,i);
             fprintf(PSUWopWopNameListFile_,"         nSect           = %d \n",ComponentGroupList_[c].WopWop().NumberOfBladesSections());
             fprintf(PSUWopWopNameListFile_,"         uniformBlade    = 0 \n");
             fprintf(PSUWopWopNameListFile_,"         BLtrip          = 0 \n");
             fprintf(PSUWopWopNameListFile_,"         sectChordFlag   = 'FILEVALUE' \n");
             fprintf(PSUWopWopNameListFile_,"         sectLengthFlag  = 'COMPUTE'   \n");
             fprintf(PSUWopWopNameListFile_,"         TEThicknessFlag = 'FILEVALUE' \n");
             fprintf(PSUWopWopNameListFile_,"         TEflowAngleFlag = 'FILEVALUE' \n");
             fprintf(PSUWopWopNameListFile_,"         TipLCSFlag      = 'FILEVALUE' \n");
             fprintf(PSUWopWopNameListFile_,"         SectAOAFlag     = 'FILEVALUE' \n");
             fprintf(PSUWopWopNameListFile_,"         SectAOAFlag     = 'FILEVALUE' \n");
             fprintf(PSUWopWopNameListFile_,"         LBLVSnoise      = .true.  \n");
             fprintf(PSUWopWopNameListFile_,"         TBLTEnoise      = .true.  \n");
             fprintf(PSUWopWopNameListFile_,"         bluntNoise      = .true.  \n");
             fprintf(PSUWopWopNameListFile_,"         bladeTipNoise   = .true.  \n");
             fprintf(PSUWopWopNameListFile_,"      / \n");
       
             // Broad band noise: PeggIn namelist
             
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
                    
             WriteOutPSUWopWopPeggNamelist();
             
          }
          
          // Write out containers for each of the blades... one for loading, one for thickness
          
          for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {
          
             sprintf(PatchLoadingGeometryName  ,"%s.PSUWopWop.Loading.Geometry.Rotor.%d.Blade.%d.dat",  FileName_,i,j);
             sprintf(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Rotor.%d.Blade.%d.dat",FileName_,i,j);
             sprintf(WopWopFileName,            "%s.PSUWopWop.Loading.Rotor.%d.Blade.%d.dat",           FileName_,i,j);
               
             // ContainerIn for Aero loading geometry and load file
      
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
         
             fprintf(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             fprintf(PSUWopWopNameListFile_,"               Title        = \'Blade_%d_For_Rotor_%d_Loading\' \n",j,i);
             fprintf(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             fprintf(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchLoadingGeometryName);
             fprintf(PSUWopWopNameListFile_,"               patchLoadingFile   = %s \n",WopWopFileName);
             fprintf(PSUWopWopNameListFile_,"            / \n");     
          
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
    
             // ContainerIn for Aero thickness model
             
             fprintf(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             fprintf(PSUWopWopNameListFile_,"               Title        = \'Blade_%d_For_Rotor_%d_Thickness\' \n",j,i);
             fprintf(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             fprintf(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
             fprintf(PSUWopWopNameListFile_,"            / \n");               
                
          }
             
       }
       
    }

    // Loop over each wing
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryHasWings() ) {
          
          i = ComponentGroupList_[c].WopWop().WingID();

          // ContainerIn for current wing
      
          fprintf(PSUWopWopNameListFile_,"!\n");      
          fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          fprintf(PSUWopWopNameListFile_,"!\n");
       
          fprintf(PSUWopWopNameListFile_,"      &ContainerIn  \n");
          fprintf(PSUWopWopNameListFile_,"         Title        = \'Wing_%d\' \n",i);
          fprintf(PSUWopWopNameListFile_,"         nbContainer  = %d \n",2*ComponentGroupList_[c].WopWop().NumberOfWingSurfaces());
          fprintf(PSUWopWopNameListFile_,"      / \n");     
   
          // Write out containers for each of the wing surfaces... one for loading, one for thickness
          
          for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {
          
             sprintf(PatchLoadingGeometryName  ,"%s.PSUWopWop.Loading.Geometry.Wing.%d.Surface.%d.dat",  FileName_,i,j);
             sprintf(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Wing.%d.Surface.%d.dat",FileName_,i,j);
             sprintf(WopWopFileName,            "%s.PSUWopWop.Loading.Wing.%d.Surface.%d.dat",           FileName_,i,j);
               
             // ContainerIn for Aero loading geometry and load file
      
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
         
             fprintf(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             fprintf(PSUWopWopNameListFile_,"               Title        = \'Surface_%d_For_Wing_%d_Loading\' \n",j,i);
             fprintf(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             fprintf(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchLoadingGeometryName);
             fprintf(PSUWopWopNameListFile_,"               patchLoadingFile   = %s \n",WopWopFileName);
             fprintf(PSUWopWopNameListFile_,"            / \n");     
          
             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
    
             // ContainerIn for Aero thickness model
             
             fprintf(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             fprintf(PSUWopWopNameListFile_,"               Title        = \'Surface_%d_For_Wing_%d_Thickness\' \n",j,i);
             fprintf(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             fprintf(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
             fprintf(PSUWopWopNameListFile_,"            / \n");               
                
          }
             
       }
       
    }

    // Loop over each body
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryHasBodies() ) {
          
          i = ComponentGroupList_[c].WopWop().BodyID();

          // ContainerIn for current body
      
          fprintf(PSUWopWopNameListFile_,"!\n");      
          fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          fprintf(PSUWopWopNameListFile_,"!\n");
       
          fprintf(PSUWopWopNameListFile_,"      &ContainerIn  \n");
          fprintf(PSUWopWopNameListFile_,"         Title        = \'Body_%d\' \n",i);
          fprintf(PSUWopWopNameListFile_,"         nbContainer  = %d \n",ComponentGroupList_[c].WopWop().NumberOfBodySurfaces());
          fprintf(PSUWopWopNameListFile_,"      / \n");     
   
          // Write out containers for each of the bodies... just the thickness
          
          for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBodySurfaces() ; j++ ) {
          
             sprintf(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Body.%d.Surface.%d.dat",FileName_,i,j);

             fprintf(PSUWopWopNameListFile_,"!\n");      
             fprintf(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             fprintf(PSUWopWopNameListFile_,"!\n");
    
             // ContainerIn for Aero thickness model
             
             fprintf(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             fprintf(PSUWopWopNameListFile_,"               Title        = \'Surface_%d_For_Body_%d_Thickness\' \n",j,i);
             fprintf(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             fprintf(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
             fprintf(PSUWopWopNameListFile_,"            / \n");               
                
          }
             
       }
       
    }
                
}

/*##############################################################################
#                                                                              #
#                 VSP_SOLVER WriteOutPSUWopWopFileHeaders                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopFileHeadersForGroup(int c)
{

    WriteOutPSUWopWopCompactGeometryHeaderForGroup(c);
    
    WriteOutPSUWopWopCompactLoadingHeaderForGroup(c);

    WriteOutPSUWopWopThicknessGeometryHeaderForGroup(c);
    
    WriteOutPSUWopWopBPMHeaderForGroup(c);
     
}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER WriteOutPSUWopWopUnsteadyData                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopUnsteadyDataForGroup(int c)
{

    WriteOutPSUWopWopCompactGeometryDataForGroup(c);
    
    WriteOutPSUWopWopCompactLoadingDataForGroup(c);
    
    WriteOutPSUWopWopThicknessGeometryDataForGroup(c);
    
    WriteOutPSUWopWopBPMDataForGroup(c);
       
}

/*##############################################################################
#                                                                              #
#             VSP_SOLVER WriteOutPSUWopWopThicknessGeometryHeader              #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopThicknessGeometryHeaderForGroup(int c)
{

    char HeaderName[32], DumChar[2000];
    char PatchGeometryName[2000];
    int i, j, k, i_size, c_size, f_size, DumInt;
    int NumberI, NumberJ, Length;
    float Period;
    FILE *WopFile;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
    
    // Loop over rotors and blades

    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
       
       i = ComponentGroupList_[c].WopWop().RotorID();

       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {
          
          sprintf(PatchGeometryName,"%s.PSUWopWop.Thickness.Geometry.Rotor.%d.Blade.%d.dat",FileName_,i,j);
          
          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().OpenThicknessGeometryFileForBlade(j,PatchGeometryName);
          
          // Geometry File Header
          
          DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
          
          // Units... this is just a comment
          
          sprintf(HeaderName,"SI");
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
      
          fwrite(&(HeaderName), c_size, 32, WopFile); // Units
         
          // Comment line
          
          sprintf(DumChar,"ThicknessFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          fwrite(&(DumChar), c_size, 1024, WopFile); // Comment lLine

          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid

          if ( SteadyStateNoise_ ) {
             
             DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Constant data

          }
          
          else {
            
             fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic

          }
                       
          DumInt =  1;        fwrite(&(DumInt), i_size, 1, WopFile); // 1 - node based normals
          DumInt =  1;        fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =  0;        fwrite(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
          DumInt =  0;        fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          
          // Surface for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);

          sprintf(HeaderName,"VSPAERO_WING_%d",k);
         
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
              
          fwrite(&(HeaderName), c_size, 32, WopFile);
          
          NumberI = VSPGeom().VSP_Surface(k).Surface_NumI();
          NumberJ = VSPGeom().VSP_Surface(k).Surface_NumJ();

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
   
                Period = ComponentGroupList_[c].Period();
          
                fwrite(&(Period), f_size, 1, WopFile); // Period
    
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
      
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }
                       
          fwrite(&(NumberI), i_size, 1, WopFile); // Number in I
          fwrite(&(NumberJ), i_size, 1, WopFile); // Number in J

       }
       
    }
       
    // Loop over wings and surfaces

     if ( ComponentGroupList_[c].GeometryHasWings() ) {
       
       i = ComponentGroupList_[c].WopWop().WingID();

       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {
          
          sprintf(PatchGeometryName,"%s.PSUWopWop.Thickness.Geometry.Wing.%d.Surface.%d.dat",FileName_,i,j);
          
          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().OpenThicknessGeometryFileForWingSurface(j,PatchGeometryName);
          
          // Geometry File Header
          
          DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
          
          // Units... this is just a comment
          
          sprintf(HeaderName,"SI");
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
      
          fwrite(&(HeaderName), c_size, 32, WopFile); // Units
         
          // Comment line
          
          sprintf(DumChar,"ThicknessFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          fwrite(&(DumChar), c_size, 1024, WopFile); // Comment lLine

          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid

          if ( SteadyStateNoise_ ) {
             
             DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Constant data

          }
          
          else {
             
             fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic

          }
                       
          DumInt =  1;        fwrite(&(DumInt), i_size, 1, WopFile); // 1 - node based normals
          DumInt =  1;        fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =  0;        fwrite(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
          DumInt =  0;        fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          
          // Surface for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);

          sprintf(HeaderName,"VSPAERO_WING_%d",k);
         
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
              
          fwrite(&(HeaderName), c_size, 32, WopFile);
          
          NumberI = VSPGeom().VSP_Surface(k).Surface_NumI();
          NumberJ = VSPGeom().VSP_Surface(k).Surface_NumJ();

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
   
                Period = WopWopLongestPeriod_;
          
                fwrite(&(Period), f_size, 1, WopFile); // Period
    
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
      
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }
                       
          fwrite(&(NumberI), i_size, 1, WopFile); // Number in I
          fwrite(&(NumberJ), i_size, 1, WopFile); // Number in J

       }
       
    }

    // Loop over body surfaces

    if ( ComponentGroupList_[c].GeometryHasBodies() ) {
       
       i = ComponentGroupList_[c].WopWop().BodyID();

       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBodySurfaces() ; j++ ) {
          
          sprintf(PatchGeometryName,"%s.PSUWopWop.Thickness.Geometry.Body.%d.Surface.%d.dat",FileName_,i,j);
    
          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().OpenThicknessGeometryFileForBodySurface(j,PatchGeometryName);
          
          // Geometry File Header
          
          DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
          
          // Units... this is just a comment
          
          sprintf(HeaderName,"SI");
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
      
          fwrite(&(HeaderName), c_size, 32, WopFile); // Units
         
          // Comment line
          
          sprintf(DumChar,"ThicknessFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          fwrite(&(DumChar), c_size, 1024, WopFile); // Comment lLine

          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid

          if ( SteadyStateNoise_ ) {
             
             DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Constant data

          }
          
          else {
             
             fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic

          }
                       
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - node based normals
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          
          // Surface for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBody(j);

          sprintf(HeaderName,"VSPAERO_BODY_%d",k);
         
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
              
          fwrite(&(HeaderName), c_size, 32, WopFile);
          
          NumberI = VSPGeom().VSP_Surface(k).Surface_NumI();
          NumberJ = VSPGeom().VSP_Surface(k).Surface_NumJ();

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
   
                Period = WopWopLongestPeriod_;
          
                fwrite(&(Period), f_size, 1, WopFile); // Period
    
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
      
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }
                       
          fwrite(&(NumberI), i_size, 1, WopFile); // Number in I
          fwrite(&(NumberJ), i_size, 1, WopFile); // Number in J

       }
       
    }

}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER WriteOutPSUWopWopThicknessGeometryData               #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopThicknessGeometryDataForGroup(int c)
{
    
    int i, j, k, m, n;
    int i_size, c_size, f_size;
    double Translation[3];
    float DumFloat, x, y, z, Time;
    FILE *WopFile;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
    
    // Vehicle translation vector
    
    Translation[0] = Translation[1] = Translation[2] = 0.;
    
    if ( !SteadyStateNoise_ && WopWopPeriodicity_ == WOPWOP_APERIODIC ) {
           
       Translation[0] = -FreeStreamVelocity_[0] * CurrentNoiseTime_;
       Translation[1] = -FreeStreamVelocity_[1] * CurrentNoiseTime_;
       Translation[2] = -FreeStreamVelocity_[2] * CurrentNoiseTime_;
       
    }

    // Loop over rotors and blades

    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
       
       i = ComponentGroupList_[c].WopWop().RotorID();
    
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {

          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().ThicknessGeometryFileForBlade(j);
          
          // Surface for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);

          // Current time
      
          Time = CurrentNoiseTime_;
       
          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);             
                          
          // X node values
    
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {

                x = VSPGeom().VSP_Surface(k).Surface_x(m,n) + Translation[0];
                
                x *= WopWopLengthConversion_;

                fwrite(&(x), f_size, 1, WopFile);
           
             }
             
          }
          
          // Y node values
          
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                y = VSPGeom().VSP_Surface(k).Surface_y(m,n) + Translation[1];
                
                y *= WopWopLengthConversion_;
     
                fwrite(&(y), f_size, 1, WopFile);
                
             }
             
          }

          // z node values
          
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                z = VSPGeom().VSP_Surface(k).Surface_z(m,n) + Translation[2];
                
                z *= WopWopLengthConversion_;
      
                fwrite(&(z), f_size, 1, WopFile);                
                
             }
             
          } 
          
          // X Normal values
           
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = -VSPGeom().VSP_Surface(k).Surface_Nx(m,n);
        
                fwrite(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          } 
          
          // Y Normal values
              
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = -VSPGeom().VSP_Surface(k).Surface_Ny(m,n);
           
                fwrite(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          }  
          
          // Z Normal values
              
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = -VSPGeom().VSP_Surface(k).Surface_Nz(m,n);
                
                fwrite(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          }   
          
       }
       
    } 

    // Loop over wings and surfaces

    if ( ComponentGroupList_[c].GeometryHasWings() ) {
       
       i = ComponentGroupList_[c].WopWop().WingID();
    
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {

          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().ThicknessGeometryFileForWingSurface(j);
          
          // Surface for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);

          // Current time
      
          Time = CurrentNoiseTime_;
       
          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);             
                          
          // X node values
    
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {

                x = VSPGeom().VSP_Surface(k).Surface_x(m,n) + Translation[0];
                
                x *= WopWopLengthConversion_;

                fwrite(&(x), f_size, 1, WopFile);
           
             }
             
          }
          
          // Y node values
          
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                y = VSPGeom().VSP_Surface(k).Surface_y(m,n) + Translation[1];
                
                y *= WopWopLengthConversion_;
     
                fwrite(&(y), f_size, 1, WopFile);
                
             }
             
          }

          // z node values
          
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                z = VSPGeom().VSP_Surface(k).Surface_z(m,n) + Translation[2];
                
                z *= WopWopLengthConversion_;
      
                fwrite(&(z), f_size, 1, WopFile);                
                
             }
             
          } 
          
          // X Normal values
           
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = -VSPGeom().VSP_Surface(k).Surface_Nx(m,n);
        
                fwrite(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          } 
          
          // Y Normal values
              
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = -VSPGeom().VSP_Surface(k).Surface_Ny(m,n);
           
                fwrite(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          }  
          
          // Z Normal values
              
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = -VSPGeom().VSP_Surface(k).Surface_Nz(m,n);
                
                fwrite(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          }   
          
       }
       
    } 

    // Loop over body surfaces

    if ( ComponentGroupList_[c].GeometryHasBodies() ) {
       
       i = ComponentGroupList_[c].WopWop().BodyID();
    
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBodySurfaces() ; j++ ) {

          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().ThicknessGeometryFileForBodySurface(j);
          
          // Surface for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBody(j);

          // Current time
      
          Time = CurrentNoiseTime_;
       
          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);             
                          
          // X node values
    
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {

                x = VSPGeom().VSP_Surface(k).Surface_x(m,n) + Translation[0];
                
                x *= WopWopLengthConversion_;

                fwrite(&(x), f_size, 1, WopFile);
           
             }
             
          }
          
          // Y node values
          
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                y = VSPGeom().VSP_Surface(k).Surface_y(m,n) + Translation[1];
                
                y *= WopWopLengthConversion_;
     
                fwrite(&(y), f_size, 1, WopFile);
                
             }
             
          }

          // z node values
          
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                z = VSPGeom().VSP_Surface(k).Surface_z(m,n) + Translation[2];
                
                z *= WopWopLengthConversion_;
      
                fwrite(&(z), f_size, 1, WopFile);                
                
             }
             
          } 
          
          // X Normal values
           
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = -VSPGeom().VSP_Surface(k).Surface_Nx(m,n);
        
                fwrite(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          } 
          
          // Y Normal values
              
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = -VSPGeom().VSP_Surface(k).Surface_Ny(m,n);
           
                fwrite(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          }  
          
          // Z Normal values
              
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = -VSPGeom().VSP_Surface(k).Surface_Nz(m,n);
                
                fwrite(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          }   
          
       }
       
    } 
  
}

/*##############################################################################
#                                                                              #
#             VSP_SOLVER WriteOutPSUWopWopCompactGeometryHeader                #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCompactGeometryHeaderForGroup(int c)
{

    char HeaderName[32], DumChar[2000];
    char PatchGeometryName[2000];
    int i, j, k, i_size, c_size, f_size, DumInt;
    int NumberI, NumberJ, Length;
    float Period;
    FILE *WopFile;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Loop over rotors and blades

    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
       
       i = ComponentGroupList_[c].WopWop().RotorID();
    
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ;  j++ ) {
          
          sprintf(PatchGeometryName,"%s.PSUWopWop.Loading.Geometry.Rotor.%d.Blade.%d.dat",FileName_,i,j);

          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().OpenLoadingGeometryFileForBlade(j,PatchGeometryName);

          // Geometry File Header
          
          DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
          
          // Force units... this is just a comment
          
          sprintf(HeaderName,"Pa\n");
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
      
          fwrite(&(HeaderName), c_size, 32, WopFile); // Units
         
          // Comment line
          
          sprintf(DumChar,"CompactGeometryFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          fwrite(&(DumChar), c_size, 1024, WopFile); // Comment lLine

          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid

          if ( SteadyStateNoise_ ) {
             
             DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Constant data

          }
          
          else {
             
             fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic


          }
                       
          DumInt =  1;        fwrite(&(DumInt), i_size, 1, WopFile); // 1 - node based normals
          DumInt =  1;        fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =  0;        fwrite(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
          DumInt =  0;        fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          
          // Write out surface inforamation for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);
          
          sprintf(HeaderName,"Blade_%d_Rotor_%d",i,j);
         
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
              
          fwrite(&(HeaderName), c_size, 32, WopFile);

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

                Period = ComponentGroupList_[c].Period();
                
                fwrite(&(Period), f_size, 1, WopFile); // Period
                
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
                
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }
                     
          NumberI = 1;
          NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
                                 
          fwrite(&(NumberI),            i_size, 1, WopFile); // Number in I
          fwrite(&(NumberJ),            i_size, 1, WopFile); // Number in J

       }
       
    }

    // Loop over wings and surfaces
 
    if ( ComponentGroupList_[c].GeometryHasWings() ) {
       
       i = ComponentGroupList_[c].WopWop().WingID();
    
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ;  j++ ) {
          
          sprintf(PatchGeometryName,"%s.PSUWopWop.Loading.Geometry.Wing.%d.Surface.%d.dat",FileName_,i,j);

          // Thickness Geometry 

          WopFile = ComponentGroupList_[c].WopWop().OpenLoadingGeometryFileForWingSurface(j,PatchGeometryName);

          // Geometry File Header
          
          DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
          
          // Force units... this is just a comment
          
          sprintf(HeaderName,"Pa\n");
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
      
          fwrite(&(HeaderName), c_size, 32, WopFile); // Units
         
          // Comment line
          
          sprintf(DumChar,"CompactGeometryFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          fwrite(&(DumChar), c_size, 1024, WopFile); // Comment lLine

          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid

          if ( SteadyStateNoise_ ) {
             
             DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Constant data

          }
          
          else {
             
             fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic


          }
                       
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - node based normals
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          
          // Write out surface inforamation for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);
          
          sprintf(HeaderName,"Blade_%d_Rotor_%d",i,j);
         
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
              
          fwrite(&(HeaderName), c_size, 32, WopFile);

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

                Period = WopWopLongestPeriod_;
                
                fwrite(&(Period), f_size, 1, WopFile); // Period
                
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
                
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }
                     
          NumberI = 1;
          NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
                                 
          fwrite(&(NumberI),            i_size, 1, WopFile); // Number in I
          fwrite(&(NumberJ),            i_size, 1, WopFile); // Number in J

       }
       
    }

}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER WriteOutPSUWopWopCompactGeometryData                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCompactGeometryDataForGroup(int c)
{
    
    int i, j, k, m;
    int i_size, c_size, f_size, NumberOfSpanStations;
    float x, y, z, Time,Translation[3];
    FILE *WopFile;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Vehicle translation vector
    
    Translation[0] = Translation[1] = Translation[2] = 0.;
    
    if ( !SteadyStateNoise_ && WopWopPeriodicity_ == WOPWOP_APERIODIC ) {
           
       Translation[0] = -FreeStreamVelocity_[0] * CurrentNoiseTime_;
       Translation[1] = -FreeStreamVelocity_[1] * CurrentNoiseTime_;
       Translation[2] = -FreeStreamVelocity_[2] * CurrentNoiseTime_;
       
    }
    
    // Loop over rotors and blades

    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
       
       i = ComponentGroupList_[c].WopWop().RotorID();
    
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {

          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().LoadingGeometryFileForBlade(j);

          // Write out surface information for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);
     
          NumberOfSpanStations = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
     
          // Current time
      
          Time = CurrentNoiseTime_;
        
          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);         

          // X node values

          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             x = VSPGeom().VSP_Surface(k).xTE(m) + 0.75*(VSPGeom().VSP_Surface(k).xLE(m) - VSPGeom().VSP_Surface(k).xTE(m)) + Translation[0];
             
             x *= WopWopLengthConversion_;
 
             fwrite(&(x), f_size, 1, WopFile);
        
          }

          // Y node values
          
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             y = VSPGeom().VSP_Surface(k).yTE(m) + 0.75*(VSPGeom().VSP_Surface(k).yLE(m) - VSPGeom().VSP_Surface(k).yTE(m)) + Translation[1];
           
             y *= WopWopLengthConversion_;

             fwrite(&(y), f_size, 1, WopFile);
                     
          }

          // z node values
          
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
             
             z = VSPGeom().VSP_Surface(k).zTE(m) + 0.75*(VSPGeom().VSP_Surface(k).zLE(m) - VSPGeom().VSP_Surface(k).zTE(m)) + Translation[2];
             
             z *= WopWopLengthConversion_;

             fwrite(&(z), f_size, 1, WopFile);
        
          }

          // X Normal values
           
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             x = VSPGeom().VSP_Surface(k).NxQC(m);

             fwrite(&(x), f_size, 1, WopFile);
        
          }
          
          // Y Normal values
              
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             y = VSPGeom().VSP_Surface(k).NyQC(m);

             fwrite(&(y), f_size, 1, WopFile);
        
          }
          
          // Z Normal values
              
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             z = VSPGeom().VSP_Surface(k).NzQC(m);

             fwrite(&(z), f_size, 1, WopFile);
        
          }
          
       }
       
    }  

    // Loop over wings and surfaces
  
    if ( ComponentGroupList_[c].GeometryHasWings() ) {
       
       i = ComponentGroupList_[c].WopWop().WingID();
    
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {

          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().LoadingGeometryFileForWingSurface(j);

          // Write out surface information for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);
     
          NumberOfSpanStations = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
     
          // Current time
      
          Time = CurrentNoiseTime_;
        
          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);         

          // X node values

          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             x = VSPGeom().VSP_Surface(k).xTE(m) + 0.75*(VSPGeom().VSP_Surface(k).xLE(m) - VSPGeom().VSP_Surface(k).xTE(m)) + Translation[0];
             
             x *= WopWopLengthConversion_;
 
             fwrite(&(x), f_size, 1, WopFile);
        
          }

          // Y node values
          
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             y = VSPGeom().VSP_Surface(k).yTE(m) + 0.75*(VSPGeom().VSP_Surface(k).yLE(m) - VSPGeom().VSP_Surface(k).yTE(m)) + Translation[1];
           
             y *= WopWopLengthConversion_;

             fwrite(&(y), f_size, 1, WopFile);
                     
          }

          // z node values
          
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
             
             z = VSPGeom().VSP_Surface(k).zTE(m) + 0.75*(VSPGeom().VSP_Surface(k).zLE(m) - VSPGeom().VSP_Surface(k).zTE(m)) + Translation[2];
             
             z *= WopWopLengthConversion_;

             fwrite(&(z), f_size, 1, WopFile);
        
          }

          // X Normal values
           
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             x = VSPGeom().VSP_Surface(k).NxQC(m);

             fwrite(&(x), f_size, 1, WopFile);
        
          }
          
          // Y Normal values
              
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             y = VSPGeom().VSP_Surface(k).NyQC(m);

             fwrite(&(y), f_size, 1, WopFile);
        
          }
          
          // Z Normal values
              
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             z = VSPGeom().VSP_Surface(k).NzQC(m);

             fwrite(&(z), f_size, 1, WopFile);
        
          }
          
       }
       
    }  
 
}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutPSUWopWopCompactLoadingHeader              #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCompactLoadingHeaderForGroup(int c)
{

    char HeaderName[32], DumChar[2000];
    char WopWopFileName[2000];
    int i, j, k, i_size, c_size, f_size, DumInt;
    int NumberI, NumberJ, Length;
    float Period;
    FILE *WopFile;
       
    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Loop over rotors and blades

    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
       
       i = ComponentGroupList_[c].WopWop().RotorID();
    
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {

          sprintf(WopWopFileName, "%s.PSUWopWop.Loading.Rotor.%d.Blade.%d.dat",FileName_,i,j);

          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().OpenLoadingFileForBlade(j, WopWopFileName);

          // Write out surface information for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);

          // Load File Header
          
          DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
      
          fwrite(&(DumChar), c_size, 1024, WopFile); // Comment Line   
              
          DumInt = 2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 indicates this is a loading file
          DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid
          
          if ( SteadyStateNoise_ ) {
             
             DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // Constant data
             
          }
          
          else {
             
             fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic
             
          }
                
          DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - node centered normals
          DumInt =   2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - data is loading vector
          
          if ( SteadyStateNoise_ ) {
             
             DumInt = 2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - Rotating frame
        
          }           
                      
          else {      
                      
             DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - Ground reference fixed frame
             
          }
                    
          DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =   0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          DumInt =   0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones with data
          DumInt =  -1; fwrite(&(DumInt), i_size, 1, WopFile); //negative since compact loading

          sprintf(HeaderName,"Blade_%d_Rotor_%d",i,j);
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
                  
          fwrite(&(HeaderName), c_size, 32, WopFile); // Patch name
   
          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

                ComponentGroupList_[c].Period();
                
                fwrite(&(Period), f_size, 1, WopFile); // Period
                
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
                
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }

          NumberI = 1;
          NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
         
          fwrite(&(NumberI), i_size, 1, WopFile); // NI
          fwrite(&(NumberJ), i_size, 1, WopFile); // NJ = NumberOfSpanStations

       }
       
    }

    // Loop over wings and surfaces

    if ( ComponentGroupList_[c].GeometryHasWings() ) {
       
       i = ComponentGroupList_[c].WopWop().WingID();
    
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {

          sprintf(WopWopFileName, "%s.PSUWopWop.Loading.Wing.%d.Surface.%d.dat",FileName_,i,j);

          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().OpenLoadingFileForWingSurface(j, WopWopFileName);

          // Write out surface information for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);

          // Load File Header
          
          DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
      
          fwrite(&(DumChar), c_size, 1024, WopFile); // Comment Line   
              
          DumInt = 2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 indicates this is a loading file
          DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid
          
          if ( SteadyStateNoise_ ) {
             
             DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // Constant data
             
          }
          
          else {
             
             fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic
             
          }
                
          DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - node centered normals
          DumInt =   2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - data is loading vector
          
          if ( SteadyStateNoise_ ) {
             
             DumInt = 2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - Rotating frame
        
          }           
                      
          else {      
                      
             DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - Ground reference fixed frame
             
          }
                    
          DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =   0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          DumInt =   0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones with data
          DumInt =  -1; fwrite(&(DumInt), i_size, 1, WopFile); //negative since compact loading

          sprintf(HeaderName,"Wing_%d_Surface_%d",i,j);
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
                  
          fwrite(&(HeaderName), c_size, 32, WopFile); // Patch name
   
          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

                Period = WopWopLongestPeriod_;
                
                fwrite(&(Period), f_size, 1, WopFile); // Period
                
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
                
                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }

          NumberI = 1;
          NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
         
          fwrite(&(NumberI), i_size, 1, WopFile); // NI
          fwrite(&(NumberJ), i_size, 1, WopFile); // NJ = NumberOfSpanStations

       }
       
    }

}

/*##############################################################################
#                                                                              #
#           VSP_SOLVER WriteOutPSUWopWopCompactLoadingData                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCompactLoadingDataForGroup(int c)
{
    
    int i, j, k, m, NumberOfStations;
    int i_size, c_size, f_size;
    float DumFloat, DynP, Time;
    FILE *WopFile;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
    
    // Forces
    
    DynP = 0.5 * Density_ * Vinf_ * Vinf_ * WopWopPressureConversion_;

    // Loop over rotors and blades

    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
       
       i = ComponentGroupList_[c].WopWop().RotorID();
     
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ;  j++ ) {
  
          // Loading file
          
          WopFile = ComponentGroupList_[c].WopWop().LoadingFileForBlade(j);

          // Write out surface inforamation for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);
          
          // Current time
      
          Time = CurrentNoiseTime_;
     
          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);

          NumberOfStations = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
              
          // X force per length
              
          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
             
             DumFloat = DynP * Span_Cx_[k][m] * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_;

             fwrite(&(DumFloat), f_size, 1, WopFile);
    
          }
          
          // Y force per length
              
          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
             
             DumFloat = DynP * Span_Cy_[k][m] * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_;

             fwrite(&(DumFloat), f_size, 1, WopFile);
         
          }
          
          // Z force per length
              
          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
             
             DumFloat = DynP * Span_Cz_[k][m] * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_;

             fwrite(&(DumFloat), f_size, 1, WopFile);
    
          }                    
 
       }
                 
    }

    // Loop over wings and surfaces

    if ( ComponentGroupList_[c].GeometryHasWings() ) {
       
       i = ComponentGroupList_[c].WopWop().WingID();
     
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ;  j++ ) {
  
          // Loading file
          
          WopFile = ComponentGroupList_[c].WopWop().LoadingFileForWingSurface(j);

          // Write out surface inforamation for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);
          
          // Current time
      
          Time = CurrentNoiseTime_;
     
          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);

          NumberOfStations = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
              
          // X force per length
              
          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
             
             DumFloat = DynP * Span_Cx_[k][m] * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_;

             fwrite(&(DumFloat), f_size, 1, WopFile);
    
          }
          
          // Y force per length
              
          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
             
             DumFloat = DynP * Span_Cy_[k][m] * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_;

             fwrite(&(DumFloat), f_size, 1, WopFile);
         
          }
          
          // Z force per length
              
          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
             
             DumFloat = DynP * Span_Cz_[k][m] * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_;

             fwrite(&(DumFloat), f_size, 1, WopFile);
    
          }                    
      
       }
                 
    }

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutPSUWopWopPeggNamelist                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopPeggNamelist(void)
{

    int i, k, i_size, c_size, f_size;
    double x1, y1, z1, x2, y2, z2;
    float BladeArea, BladeRadius, Thrust, BladeCL, RotationSpeed;
       
    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
            
    // Calculate total blade area
        
    BladeArea = 0.;

    for ( k = 1 ; k <= VSPGeom().NumberOfSurfaces() ; k++ ) {
         
       // Wing   
            
       if ( VSPGeom().VSP_Surface(k).SurfaceType() == DEGEN_WING_SURFACE ) {
      
          for ( i = 1 ; i < VSPGeom().VSP_Surface(k).NumberOfSpanStations() ; i++ ) {

             BladeArea += Span_Area_[k][i];

          }
          
       }
       
    }

    // Calculate blade radius
    
    for ( k = 1 ; k <= VSPGeom().NumberOfSurfaces() ; k++ ) {
         
       // Wing   
            
       if ( VSPGeom().VSP_Surface(k).SurfaceType() == DEGEN_WING_SURFACE ) {
          
          i = 1;

          x1 = VSPGeom().VSP_Surface(k).xTE(i) + 0.75*(VSPGeom().VSP_Surface(k).xLE(i) - VSPGeom().VSP_Surface(k).xTE(i));
          y1 = VSPGeom().VSP_Surface(k).yTE(i) + 0.75*(VSPGeom().VSP_Surface(k).yLE(i) - VSPGeom().VSP_Surface(k).yTE(i));
          z1 = VSPGeom().VSP_Surface(k).zTE(i) + 0.75*(VSPGeom().VSP_Surface(k).zLE(i) - VSPGeom().VSP_Surface(k).zTE(i));
          
          i = VSPGeom().VSP_Surface(k).NumberOfSpanStations();

          x2 = VSPGeom().VSP_Surface(k).xTE(i) + 0.75*(VSPGeom().VSP_Surface(k).xLE(i) - VSPGeom().VSP_Surface(k).xTE(i));
          y2 = VSPGeom().VSP_Surface(k).yTE(i) + 0.75*(VSPGeom().VSP_Surface(k).yLE(i) - VSPGeom().VSP_Surface(k).yTE(i));
          z2 = VSPGeom().VSP_Surface(k).zTE(i) + 0.75*(VSPGeom().VSP_Surface(k).zLE(i) - VSPGeom().VSP_Surface(k).zTE(i));
          
          BladeRadius = sqrt( SQR(x2-x1) + SQR(y2-y1) + SQR(z2-z1) );

       }
       
    }
    
    // Calculate averge lift coefficient
  
    RotationSpeed = BladeRPM_ * 2. * PI / 60.;

    Thrust = -0.5*Density_*Sref_*SQR(Vinf_)*CFx_[0];
    
    BladeCL = 6.*Thrust/ (Density_ * BladeArea * SQR(RotationSpeed));

    // PeggIn namelist
        
    fprintf(PSUWopWopNameListFile_,"      &PeggIn\n");

    fprintf(PSUWopWopNameListFile_,"         TotalBladeAreaFlag = \'USERVALUE\' \n");
    fprintf(PSUWopWopNameListFile_,"         TotalBladeArea     = %f \n",BladeArea*WopWopAreaConversion_);
                                                               
    fprintf(PSUWopWopNameListFile_,"         BladeRadiusFlag    = \'USERVALUE\' \n");
    fprintf(PSUWopWopNameListFile_,"         BladeRadius        = %f \n",BladeRadius*WopWopLengthConversion_);
                                                               
    fprintf(PSUWopWopNameListFile_,"         RotSpeedFlag       = \'USERVALUE\' \n");
    fprintf(PSUWopWopNameListFile_,"         RotSpeed           = %f \n",RotationSpeed);
                                                               
    fprintf(PSUWopWopNameListFile_,"         CLBarFlag          = \'USERVALUE\' \n");
    fprintf(PSUWopWopNameListFile_,"         CLBar              = %f \n",BladeCL);
                                                               
    fprintf(PSUWopWopNameListFile_,"         TotalThrustFlag    = \'USERVALUE\' \n");
    fprintf(PSUWopWopNameListFile_,"         TotalThrust        = %f \n",Thrust*WopWopForceConversion_);
                                                               
    fprintf(PSUWopWopNameListFile_,"         HubAxisFlag        = \'USERVALUE\' \n");
    fprintf(PSUWopWopNameListFile_,"         HubAxis            = %f, %f, %f \n",-1., 0., 0.);

    fprintf(PSUWopWopNameListFile_,"      / \n");        

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutPSUWopWopBPMHeader                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopBPMHeaderForGroup(int c)
{

    char WopWopFileName[2000];
    int i, k, i_size, c_size, f_size, DumInt, NumberJ;
    FILE *WopFile;
       
    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Loop over rotors and blades

    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
       
       i = ComponentGroupList_[c].WopWop().RotorID();

       sprintf(WopWopFileName,"%s.PSUWopWop.BPM.Rotor.%d.dat",FileName_,i);

       // BPM file
       
       WopFile = ComponentGroupList_[c].WopWop().OpenBPMFile(WopWopFileName);

       // Write out surface inforamation for this blade
       
       k = ComponentGroupList_[c].WopWop().SurfaceForBlade(1);
    
       // BPM File Header
       
       NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
    
       DumInt = 42; fwrite(&(DumInt ), i_size, 1, WopFile); // Magic Number
                    fwrite(&(NumberJ), i_size, 1, WopFile); // Number of sections definining the blade
       DumInt =  0; fwrite(&(DumInt ), i_size, 1, WopFile); // Non-uniform blade sections
       DumInt =  1; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section chord data provided
       DumInt =  0; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section length data not provided
       DumInt =  1; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section TE thickness provided
       DumInt =  1; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section TE flow angle provided
       DumInt =  1; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section effective AoA provided
       DumInt =  1; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section tip lift curve slope provided
       DumInt =  1; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section free stream velocity provided
   
       if ( !TimeAccurate_ ) {
          
          DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // constant data
                   
       }           
                   
       else {      
                   
          DumInt = 3; fwrite(&(DumInt), i_size, 1, WopFile); // aperiodic data
          
       }

    }

}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER WriteOutPSUWopWopBPMData                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopBPMDataForGroup(int c)
{

    int i, j, k, m, i_size, c_size, f_size;
    int NumberOfStations;
    float DumFloat; 
    FILE *WopFile;
       
    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
          
    // Loop over rotors

    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
       
       i = ComponentGroupList_[c].WopWop().RotorID();
    
       // WopWop and BPM assume all the blades are the same
       
       j = 1;

       // BPM file
       
       WopFile = ComponentGroupList_[c].WopWop().BPMFile();

       // Write out surface inforamation for this blade
       
       k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);
   
       NumberOfStations = VSPGeom().VSP_Surface(k).NumberOfSpanStations();

       for ( m = 1 ; m <= NumberOfStations ; m++ ) {

          // Chord
          
          DumFloat = VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_;

          fwrite(&(DumFloat), f_size, 1, WopFile);
   
          // Span
          
          DumFloat = Span_Area_[k][m] / VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ ;

// djk... let psu-wopwop calculate this
          
     //     fwrite(&(DumFloat), f_size, 1, WopFile);
          
          // Blade section TE thickness
          
          DumFloat = 0.0005 * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_;
                
          fwrite(&(DumFloat), f_size, 1, WopFile);
          
          // Blade section TE flow angle, radians
          
          DumFloat = 0.244;
            
          fwrite(&(DumFloat), f_size, 1, WopFile);             
   
       }
    
       for ( m = 1 ; m <= NumberOfStations ; m++ ) {
          
          // Blade section effective AoA
          
          DumFloat = 0.0;
           
          fwrite(&(DumFloat), f_size, 1, WopFile);        
   
          // Blade section tip lift curve slope
          
          DumFloat = 1.0;
                      
          fwrite(&(DumFloat), f_size, 1, WopFile); 
          
          // Blade section free stream speed
          
          DumFloat = Vinf_ * WopWopLengthConversion_;
            
          fwrite(&(DumFloat), f_size, 1, WopFile);     

       }

    }

}

/*##############################################################################
#                                                                              #
#             VSP_SOLVER WriteOutPSUSWopWopLoadingGeometryHeader               #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopLoadingGeometryHeaderForGroup(int c)
{

    char HeaderName[32], DumChar[2000];
    char PatchGeometryName[2000];
    int i, j, Node, Level, i_size, c_size, f_size, DumInt;
    int NumberOfNodes, NumberOfLoops, Length;
    float Period;
    FILE *WopFile;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
    
    sprintf(PatchGeometryName, "%s.PSUWopWop.Loading.Geometry.dat",FileName_);
    
    // Geometry file
    
    if ( (WopFile = fopen(PatchGeometryName, "wb")) == NULL ) {

       printf("Could not open the PSUWopWop Case File output! \n");

       exit(1);

    }    
    
    // Geometry File Header
    
    DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
    DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
    DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
  
    // Force units... this is just a comment
    
    sprintf(HeaderName,"Pascals\n");
    
    Length = strlen(HeaderName);
    memset(&HeaderName[Length], ' ', 32 - Length);

    fwrite(&(HeaderName), c_size, 32, WopFile); // Units
   
    // Comment line
    
    sprintf(DumChar,"VSPAERO Created PSUWopWop Loading Geometry File");

    Length = strlen(DumChar);
    memset(&DumChar[Length], ' ', 1024 - Length);
        
    fwrite(&(DumChar), c_size, 1024, WopFile); // Comment lLine
        
    DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
    DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1, single, zone for now
    DumInt =  2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 indicates this is an unstructured grid
    
    if ( SteadyStateNoise_ ) {
    
       DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - Constant data
       
    }
    
    else {
       
       fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic
       
    }       
       
    DumInt =  2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - faced centered normals
    DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
    DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
    DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
    
    sprintf(HeaderName,"Zone_1_Geom_Uns");
    
    Length = strlen(HeaderName);
    memset(&HeaderName[Length], ' ', 32 - Length);
        
    fwrite(&(HeaderName), c_size, 32, WopFile);
    
    Level = 0;
    
    NumberOfNodes = VSPGeom().Grid(Level).NumberOfNodes();
    NumberOfLoops = VSPGeom().Grid(Level).NumberOfLoops();
    
    if ( !SteadyStateNoise_ ) {
         
       if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

          Period = WopWopLongestPeriod_;
          
          fwrite(&(Period), f_size, 1, WopFile); // Period
          
          fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
          
       }
       
       else {
          
          fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

       }
       
    }
    
    fwrite(&(NumberOfNodes),      i_size, 1, WopFile); // Number of nodes
    fwrite(&(NumberOfLoops),      i_size, 1, WopFile); // Number of loops
    
    // Write out loop connnectivity

    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {
       
       DumInt = VSPGeom().Grid(Level).LoopList(i).NumberOfNodes();
  
       fwrite(&(DumInt), i_size, 1, WopFile);
       
       for ( j = 1 ; j <= VSPGeom().Grid(Level).LoopList(i).NumberOfNodes() ; j++ ) {
          
          Node = VSPGeom().Grid(Level).LoopList(i).Node(j);

          fwrite(&Node, i_size, 1, WopFile);
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER WriteOutPSUWopWopLoadingGeometryData                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopLoadingGeometryDataForGroup(int c)
{
    
    int i, j, Level;
    int i_size, c_size, f_size;
    double Translation[3];
    float DumFloat, x, y, z, Time;
    FILE *WopFile;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
    
    // Vehicle translation vector
    
    Translation[0] = Translation[1] = Translation[2] = 0.;
    
    if ( !SteadyStateNoise_ && WopWopPeriodicity_ == WOPWOP_APERIODIC ) {
           
       Translation[0] = -FreeStreamVelocity_[0] * CurrentNoiseTime_;
       Translation[1] = -FreeStreamVelocity_[1] * CurrentNoiseTime_;
       Translation[2] = -FreeStreamVelocity_[2] * CurrentNoiseTime_;
       
    }
    
    // Current time
    
    Time = CurrentNoiseTime_;
 
    if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);

    Level = 0;
       
    // X node values
       
    for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

       x = VSPGeom().Grid(Level).NodeList(j).x() + Translation[0];
       
       x *= WopWopLengthConversion_;
       
       fwrite(&(x), f_size, 1, WopFile);

    }
    
    // Y node values
    
    for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

       y = VSPGeom().Grid(Level).NodeList(j).y() + Translation[1];
       
       y *= WopWopLengthConversion_;

       fwrite(&(y), f_size, 1, WopFile);
  
    }
    
    // Z node values
    
    for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

       z = VSPGeom().Grid(Level).NodeList(j).z() + Translation[2];
       
       z *= WopWopLengthConversion_;

       fwrite(&(z), f_size, 1, WopFile);         
  
    }        

    // X Normal values
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {
 
       DumFloat = VSPGeom().Grid(Level).LoopList(i).Nx() * VSPGeom().Grid(Level).LoopList(i).Area();
       
       DumFloat *= WopWopAreaConversion_;

       fwrite(&DumFloat, f_size, 1, WopFile);

    }    
    
    // Y Normal values
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       DumFloat = VSPGeom().Grid(Level).LoopList(i).Ny() * VSPGeom().Grid(Level).LoopList(i).Area();

       DumFloat *= WopWopAreaConversion_;

       fwrite(&DumFloat, f_size, 1, WopFile);

    }    
    
    // Z Normal values
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       DumFloat = VSPGeom().Grid(Level).LoopList(i).Nz() * VSPGeom().Grid(Level).LoopList(i).Area();
       
       DumFloat *= WopWopAreaConversion_;

       fwrite(&DumFloat, f_size, 1, WopFile);

    }               

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutPSUWopWopLoadingHeader                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopLoadingHeaderForGroup(int c)
{

    char HeaderName[32], DumChar[2000];
    char WopWopFileName[2000];
    int Level, i_size, c_size, f_size, DumInt;
    int NumberOfLoops, Length;
    float Period;
    FILE *WopFile;
       
    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
            
    // Load file
    
    sprintf(WopWopFileName, "%s.PSUWopWop.Loading.dat",FileName_);
    
    if ( (WopFile = fopen(WopWopFileName, "wb")) == NULL ) {

       printf("Could not open the PSUWopWop Case File output! \n");

       exit(1);

    }        
    
    // Load File Header
    
    DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
    DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
    DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2

    fwrite(&(DumChar), c_size, 1024, WopFile); // Comment lLine
        
    DumInt =   2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 indicates this is a loading file
    DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // 1, single, zone for now
    DumInt =   2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 indicates this is an unstructured grid
 
    if ( !SteadyStateNoise_ ) {

       DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // Constant data
       
    }
    
    else {
       
       fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... periodic, or aperiodic
       
    }
  
    DumInt =   2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - faced centered normals
    DumInt =   2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - data is loading vector
    
    if ( SteadyStateNoise_ ) {
       
       DumInt = 2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - Rotating frame
                
    }           
                
    else {      
                
       DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - Ground reference fixed frame
       
    }
    
    DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
    DumInt =   0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
    DumInt =   0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
    DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones with data
    DumInt =  -1; fwrite(&(DumInt), i_size, 1, WopFile); // Zone list, negative since VLM

    sprintf(HeaderName,"Zone_1_Load_Uns");
    
    Length = strlen(HeaderName);
    memset(&HeaderName[Length], ' ', 32 - Length);
            
    fwrite(&(HeaderName), c_size, 32, WopFile); // Patch name
 
    Level = 0;
    
    NumberOfLoops = VSPGeom().Grid(Level).NumberOfLoops();
     
    if ( !SteadyStateNoise_ ) {
         
       if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

          Period = WopWopLongestPeriod_;
          
          fwrite(&(Period), f_size, 1, WopFile); // Period
          
          fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
          
       }
       
       else {
          
          fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

       }
       
    }
    
    fwrite(&(NumberOfLoops),      i_size, 1, WopFile); // Number of loops
       
}

    
/*##############################################################################
#                                                                              #
#                  VSP_SOLVER WriteOutPSUWopWopLoadingData                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopLoadingDataForGroup(int c)
{
    
    int i, Level;
    int i_size, c_size, f_size;
    float Time, Fx, Fy, Fz, DynP;
    FILE *WopFile;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
    
    // Current time

    Time = CurrentNoiseTime_;

    if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);

    // Forces
    
    DynP = 0.5 * Density_ * Vinf_ * Vinf_ * WopWopPressureConversion_;

    Level = 0;

    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       Fx = -DynP * VSPGeom().Grid(Level).LoopList(i).dCp() * VSPGeom().Grid(Level).LoopList(i).Nx();

       fwrite(&Fx, f_size, 1, WopFile);
     
    }  
     
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       Fy = -DynP * VSPGeom().Grid(Level).LoopList(i).dCp() * VSPGeom().Grid(Level).LoopList(i).Ny();

       fwrite(&Fy, f_size, 1, WopFile);
    
    }   
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       Fz = -DynP * VSPGeom().Grid(Level).LoopList(i).dCp() * VSPGeom().Grid(Level).LoopList(i).Nz();

       fwrite(&Fz, f_size, 1, WopFile);
    
    }    

}
 
 
