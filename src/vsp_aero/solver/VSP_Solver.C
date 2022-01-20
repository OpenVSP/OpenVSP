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

    Clo_2d_ = 0.;
        
    Clmax_2d_ = -1.;
         
    NumberOfKelvinConstraints_ = 0;
    
    LoadDeformationFile_ = 0;
    
    Write2DFEMFile_ = 0;
    
    TimeAccurate_ = 0;
    
    QuasiTimeAccurate_ = 0;
    
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

    SPRINTF(CaseString_,"No Comment");
    
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
    
    GMRESTightConvergence_ = 0;
    
    SpanLoadingData_ = 0;
    
    NumberOfSpanLoadDataSets_ = 0;
    
    SpanLoadData_ = NULL;
    
    PanelSpanWiseLoading_ = 1;
    
    AveragingHasStarted_ = 0;

    NumberOfAveragingSets_ = 0;
    
    DoAdjointSolve_ = 0;
    
    AdjointSolve_ = 0;
    
    CalculateGradients_ = 0;
    
    CurrentOptFunction_ = OPT_CD_CL_CM;
    
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

    PRINTF("VSP_SOLVER operator= not implemented! \n");
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
    int NumSteps_1, NumSteps_2, NumberOfTimeSamples, ThereIsARotor;
    int Found, UserNumberOfTimeSteps, *ComponentInThisGroup;
    VSPAERO_DOUBLE Area, Scale_X, Scale_Y, Scale_Z, FarDist, Period, S[3], Mag;
    VSPAERO_DOUBLE MinRotorDiameter, dt1, dt2, TimeSetByFastestRotor, StreamDist;
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
    
    PRINTF("Xmax_ - Xmin_: %f \n",Xmax_ - Xmin_);
    PRINTF("Ymax_ - Ymin_: %f \n",Ymax_ - Ymin_);
    PRINTF("Zmax_ - Zmin_: %f \n",Zmax_ - Zmin_);
    PRINTF("\n");
    
    // Override far field distance
    
    if ( SetFarFieldDist_ ) {
       
       FarDist = FarFieldDist_;
       
    }
    
    else {
       
       FarFieldDist_ = FarDist;
       
    }
    
    PRINTF("Wake FarDist set to: %f \n",FarDist);
    
    // Distance along free stream direction
    
    StreamDist = (Xmax_ - Xmin_);

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
    
    PRINTF("Number Of Trailing Vortices: %d \n",NumberOfTrailingVortexEdges_);
    
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
   
          SPRINTF(GroupFileName,"%s.groups",FileName_);
          
          if ( (GroupFile = fopen(GroupFileName, "r")) == NULL ) {
      
             PRINTF("Could not open the group file for input! \n");
      
             exit(1);
      
          }
    
          // Load in the data
          
          fscanf(GroupFile,"%d\n",&NumberOfComponentGroups_);
          
          PRINTF("There are %d component groups \n",NumberOfComponentGroups_);
          
          ComponentGroupList_ = new COMPONENT_GROUP[NumberOfComponentGroups_ + 1];
          
          for ( i = 1 ; i <= NumberOfComponentGroups_ ; i++ ) {
             
             fgets(DumChar,80,GroupFile);
             
             PRINTF("DumChar: %s \n",DumChar);fflush(NULL);
             
             ComponentGroupList_[i].LoadData(GroupFile);
             
          }
          
          fclose(GroupFile);
          
       }
       
       else {
          
          NumberOfComponentGroups_ = 1;
          
          ComponentGroupList_ = new COMPONENT_GROUP[NumberOfComponentGroups_ + 1];
       
          SPRINTF(ComponentGroupList_[1].GroupName(),"%s",FileName_);
          
          ComponentGroupList_[1].GeometryIsDynamic() = 2;

          // Determine angular rates and reduced frequency for damping analyses

          TimeStep_ = (FarFieldDist_/8.) / ( Vinf_ );
       
          Unsteady_AngleRate_ = 2. * PI / (TimeStep_ * NumberOfTimeSteps_);

//djk
// 2d wing test case, set Vinf_ = 1.
//TimeStep_ = 0.25;
//Unsteady_AngleRate_ =  0.666667 ;
//NumberOfTimeSteps_ = 512;          
//Unsteady_AngleMax_ = 5.;

//djk
// 2d wing test case, set Vinf_ = 100.
//TimeStep_ = 0.0025;
//Unsteady_AngleRate_ =  66.666667;
//NumberOfTimeSteps_ = 512;          
//Unsteady_AngleMax_ = 5.;

          if ( TimeAnalysisType_ == P_ANALYSIS) ReducedFrequency_ = 0.5 * Unsteady_AngleRate_ * Bref_ / Vinf_;
          if ( TimeAnalysisType_ == Q_ANALYSIS) ReducedFrequency_ = 0.5 * Unsteady_AngleRate_ * Cref_ / Vinf_;
          if ( TimeAnalysisType_ == R_ANALYSIS) ReducedFrequency_ = 0.5 * Unsteady_AngleRate_ * Bref_ / Vinf_;

          ComponentGroupList_[1].GeometryIsARotor() = 0;
          
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
             
             PRINTF("Unknown time analysis type! \n");fflush(NULL);
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
       
       SPRINTF(ComponentGroupList_[1].GroupName(),"%s",FileName_);

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
    
    PRINTF("NumberOfComponentGroups_: %d \n",NumberOfComponentGroups_);
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
          
          if ( ComponentGroupList_[c].ComponentList(j) > VSPGeom().NumberOfComponents() ) {
             
             PRINTF("Component Group: %d lists non-existant Component: %d \n", c, ComponentGroupList_[c].ComponentList(j));
             
             fflush(NULL);exit(1);
          
          }    

       }
       
    }

    // Determine the number of lifting surfaces per component group
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
  
       CompSurfs = 0;

       for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
          
          // VLM
          
          if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {

             for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
             
                if ( ComponentGroupList_[c].ComponentList(j) == VSPGeom().VSP_Surface(i).ComponentID() ) {
                
                   if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) CompSurfs++;
                
                }
                
             }
             
          }
          
          // Panel or VSPGEOM/VLM
          
          else {

             Found = 0;
             
             i = 1;
             
             while ( i <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() && !Found ) {
                
                if ( ComponentGroupList_[c].ComponentList(j) == VSPGeom().Grid(MGLevel_).ComponentIDForKuttaNode(i) ) Found = 1;
             
                i++;
                
             }   
             
             if ( Found ) CompSurfs++;
             
          }

       }
       
       PRINTF("Found %d lifting surfaces for component group: %d \n",CompSurfs, c);
       
       if ( CompSurfs > 0 ) ComponentGroupList_[c].GeometryHasWings() = 1;

       ComponentGroupList_[c].SizeSpanLoadingList(CompSurfs);

       CompSurfs = 0;

       for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
          
          // VLM
          
          if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {

             for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
                
                if ( ComponentGroupList_[c].ComponentList(j) == VSPGeom().VSP_Surface(i).ComponentID() ) {
                                      
                   if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
                   
                      CompSurfs++;
                   
                      ComponentGroupList_[c].SpanLoadData(CompSurfs).SurfaceID() = i;
                   
                      ComponentGroupList_[c].SpanLoadData(CompSurfs).SetNumberOfSpanStations(VSPGeom().VSP_Surface(i).NumberOfSpanStations());   
                   
                   }
                     
                }
                
             }
             
          }
          
          // VLM VSPGEOM, or any panel

          else {
           
             Found = 0;
             
             i = 1;
             
             while ( i <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() && !Found ) {
                
                if ( ComponentGroupList_[c].ComponentList(j) == VSPGeom().Grid(MGLevel_).ComponentIDForKuttaNode(i) ) Found = VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(i);
             
                i++;
                
             }   
             
             if ( Found ) {
                
                CompSurfs++;
             
                ComponentGroupList_[c].SpanLoadData(CompSurfs).SurfaceID() = Found;
             
                // Do this later...   ComponentGroupList_[c].SpanLoadData(CompSurfs).SetNumberOfSpanStations(...);   
             
             }                         
                                     
          }
            
       }
   
    }

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
          
          if ( ComponentGroupList_[c].ComponentList(j) > VSPGeom().NumberOfComponents() ) {
             
             PRINTF("Component Group: %d lists non-existant Component: %d \n", c, ComponentGroupList_[c].ComponentList(j));
             
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

    PRINTF("AllComponentsAreFixed_: %d \n",AllComponentsAreFixed_);
    
    // If all components are not fixed, check that there is indeed relative motion
    
    if ( AllComponentsAreFixed_ == 0 && NumberOfComponentGroups_ > 1 ) ThereIsRelativeComponentMotion_ = 1;

    // Unsteady analysis
    
    if ( TimeAccurate_ ) {

       // Determine the slowest and fastest rotor
       
       WopWopOmegaMax_ = -1.e9;
       WopWopOmegaMin_ =  1.e9;
       
       ThereIsARotor = 0;
       
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
          if ( ComponentGroupList_[c].GeometryIsARotor() ) {

             if ( ABS(ComponentGroupList_[c].Omega()) < WopWopOmegaMin_ ) {
       
                WopWopOmegaMin_ = ABS(ComponentGroupList_[c].Omega());
                
             }
                       
             if ( ABS(ComponentGroupList_[c].Omega()) > WopWopOmegaMax_ ) {
       
                WopWopOmegaMax_ = ABS(ComponentGroupList_[c].Omega());
                
             }
             
             ThereIsARotor = 1;
                       
          }
          
       }
       
       // Determine the smallest rotor
       
       MinRotorDiameter = 1.e9;
       
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
          if ( ComponentGroupList_[c].GeometryIsARotor() ) {

             if ( ComponentGroupList_[c].RotorDiameter() < MinRotorDiameter ) {
       
                MinRotorDiameter = ComponentGroupList_[c].RotorDiameter();
                
             }
                       
          }
          
       }
            
       // Calculate a default time step if user did not set it
      
       if ( TimeStep_ <= 0. ) {

          TimeSetByFastestRotor = 0.;
          
          if ( WopWopOmegaMax_ > 0. ) {
             
             TimeSetByFastestRotor = ABS(TimeStep_);

             // Time step based on rotation rate
             
             TimeStep_ = 18. * TORAD / ( ABS(TimeStep_)*WopWopOmegaMax_); // 18 degrees per step... so 20 steps per rev
             
          }

          else {
             
             TimeStep_ = 0.1*Cref_;
             
          }
          
       }
       
       // Calculate the number of time steps if user did not set it
       
       if ( NumberOfTimeSteps_ < 0 ) {
          
          UserNumberOfTimeSteps = NumberOfTimeSteps_;
          
          if ( QuasiTimeAccurate_ && NumberOfTimeSteps_ == -1 ) NumberOfTimeSteps_ = -3;
                    
          // Slowest rotor, at least 2 rotations... but only if there's a spread in speeds from min to max omega
      
          NumSteps_1 = 0;
          
          if ( WopWopOmegaMax_ != WopWopOmegaMin_ ) {
             
             Period = 2.*PI / WopWopOmegaMin_;
             
             NumSteps_1 = FLOAT( 2.*Period / TimeStep_ ) + 1;
             
          }
          
          // Fastest rotor does ABS(NumberOfTimeSteps_) revolutions
          
          NumSteps_2 = 0;
 
 //         if ( TimeSetByFastestRotor > 0. ) NumSteps_2 = ceil(TimeSetByFastestRotor*ABS(NumberOfTimeSteps_)*20); // 20 steps per rev, @ 18 degrees per step
 
          if ( TimeSetByFastestRotor > 0. ) {
             
             NumSteps_2 = (int) FLOAT( TimeSetByFastestRotor*ABS(NumberOfTimeSteps_)*20 );
             
             if ( (VSPAERO_DOUBLE) NumSteps_2 < FLOAT( TimeSetByFastestRotor*ABS(NumberOfTimeSteps_)*20 ) ) {
                
                NumSteps_2++;
                
             }
                 
          } 
             
          if ( NumSteps_1 > NumSteps_2 ) {
             
             NumberOfTimeSteps_ = NumSteps_1;
          
             PRINTF("Setting number of time steps to %d based on slowest rotor making 2 rotations. \n",NumberOfTimeSteps_);
             
          }
          
          else {
             
             PRINTF("Setting number of time steps to %d based on fastest rotor making %d rotations. \n",NumSteps_2,ABS(NumberOfTimeSteps_));
             
             NumberOfTimeSteps_ = NumSteps_2;
          
          }
             
       }
       
       // If user did not specify the number of wake nodes, set it greater than the number of time steps
      
       if ( NumberOfWakeTrailingNodes_ < 0 ) {
          
          NumberOfWakeTrailingNodes_ = 64;
          
          while ( 2*NumberOfWakeTrailingNodes_ < NumberOfTimeSteps_ ) {
             
             NumberOfWakeTrailingNodes_ *= 2;
             
          }
          
          if ( QuasiTimeAccurate_ && NumberOfWakeTrailingNodes_ > NumberOfTimeSteps_ ) {
             
             NumberOfWakeTrailingNodes_ /= 2;
             
          }
          
          PRINTF("Setting wake number of nodes to: %d \n",NumberOfWakeTrailingNodes_);
          
       }       
       
       // Determine average start times for each lifting surface, and number of time samples
       
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

          if ( ComponentGroupList_[c].GeometryIsARotor() ) {
             
             Period = 2.*PI / ABS(ComponentGroupList_[c].Omega());
             
          }
          
          else {
          
             if ( ThereIsARotor ) {
                
                Period = 2.*PI / ABS(WopWopOmegaMin_);
                
             }
             
             else {
                
                Period = 0.3333 * TimeStep_ * NumberOfTimeSteps_;

             }
             
          }          
          
          ComponentGroupList_[c].StartAveragingTime() = NumberOfTimeSteps_ * TimeStep_ - Period - TimeStep_;
          
          NumberOfTimeSamples = FLOAT( ( NumberOfTimeSteps_ * TimeStep_ - ComponentGroupList_[c].StartAveragingTime() ) / TimeStep_ );
          
          if ( NoiseAnalysis_ ) NumberOfTimeSamples = 181;
          
          for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfLiftingSurfaces() ; i++ ) {
          
             ComponentGroupList_[c].SpanLoadData(i).SetNumberOfNumberOfTimeSamples(NumberOfTimeSamples);   
             
          }
          
       }       

       PRINTF("Used FarFieldDist_ of: %f to calculate time step \n",FarFieldDist_);
           
       PRINTF("TimeAnalysisType_: %d \n",TimeAnalysisType_);
       
       PRINTF("TimeStep_: %f \n",TimeStep_);

       PRINTF("ReducedFrequency_: %f \n",ReducedFrequency_);
       
       PRINTF("Unsteady_AngleRate_: %f \n",Unsteady_AngleRate_);

    }

    // Size span load arrays 

    if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {

       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

          for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfLiftingSurfaces() ; i++ ) {
          
             ComponentGroupList_[c].SpanLoadData(i).SizeSpanLoadingList();
             
          }
          
       }

    }   
    
    // Initialize some ref data for each component
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
    
       ComponentGroupList_[c].Density() = Density_;
       ComponentGroupList_[c].Vref() = Vref_;
       ComponentGroupList_[c].Sref() = Sref_;
       ComponentGroupList_[c].Bref() = Bref_;
       ComponentGroupList_[c].Cref() = Cref_; 

    }   
   
    // VLM model
    
    if ( ModelType_ == VLM_MODEL ) {
       
       PRINTF("Model type: VLM \n");
 
       NumberOfKelvinConstraints_ = 0;

       LoopIsOnBaseRegion_ = new int[NumberOfVortexLoops_ + 1];

       zero_int_array(LoopIsOnBaseRegion_, NumberOfVortexLoops_);
    
    }
    
    // Panel Model
    
    else if ( ModelType_ == PANEL_MODEL ) {
       
       PRINTF("Model type: PANEL \n");       
       
       DetermineNumberOfKelvinConstrains();
    
    }
    
    // Uknown model
    
    else {
       
       PRINTF("Unknown Model Type! \n");fflush(NULL);
       
       exit(1);
       
    }

    NumberOfEquations_ = NumberOfVortexLoops_ + NumberOfKelvinConstraints_;

    // Allocate space for the vortex edges and loops
  
    SurfaceVortexEdge_ = new VSP_EDGE*[NumberOfSurfaceVortexEdges_ + 1];

    TrailingVortexEdge_ = new VORTEX_TRAIL[NumberOfTrailingVortexEdges_ + 1];
    
    VortexLoop_ = new VSP_LOOP*[NumberOfVortexLoops_ + 1];
  
    UnsteadyTrailingWakeVelocity_ =  new VSPAERO_DOUBLE*[NumberOfVortexLoops_ + 1];  
    
    LocalBodySurfaceVelocityForLoop_ = new VSPAERO_DOUBLE*[NumberOfVortexLoops_ + 1];  

    LocalBodySurfaceVelocityForEdge_ = new VSPAERO_DOUBLE*[NumberOfSurfaceVortexEdges_ + 1];          
        
    Gamma_[0] = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];    
    Gamma_[1] = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];    
    Gamma_[2] = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];    

    Diagonal_ = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];     

    Delta_= new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];     
   
    zero_double_array(Gamma_[0], NumberOfVortexLoops_); Gamma_[0][0] = 0.;   
    zero_double_array(Gamma_[1], NumberOfVortexLoops_); Gamma_[1][0] = 0.;   
    zero_double_array(Gamma_[2], NumberOfVortexLoops_); Gamma_[2][0] = 0.;   

    zero_double_array(Diagonal_, NumberOfVortexLoops_); Diagonal_[0] = 0.;    
    zero_double_array(Delta_,    NumberOfVortexLoops_);    Delta_[0] = 0.;
   
    Residual_ = new VSPAERO_DOUBLE[NumberOfEquations_ + 1];    

    RightHandSide_ = new VSPAERO_DOUBLE[NumberOfEquations_ + 1];     
     
    MatrixVecTemp_ = new VSPAERO_DOUBLE[NumberOfEquations_ + 1];     
   
    zero_double_array(Residual_,      NumberOfEquations_); Residual_[0]      = 0.;
    zero_double_array(RightHandSide_, NumberOfEquations_); RightHandSide_[0] = 0.;
    zero_double_array(MatrixVecTemp_, NumberOfEquations_); RightHandSide_[0] = 0.;
        
    if ( NoiseAnalysis_ || DoAdjointSolve_ || CalculateGradients_ ) {
       
       NumberOfNoiseInterpolationPoints_ = 9;
       
       for ( i = 0 ; i < NumberOfNoiseInterpolationPoints_ ; i++ ) {
          
          GammaNoise_[i] = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];    
    
          FxNoise_[i] = new VSPAERO_DOUBLE[NumberOfSurfaceVortexEdges_ + 1];    
          FyNoise_[i] = new VSPAERO_DOUBLE[NumberOfSurfaceVortexEdges_ + 1];    
          FzNoise_[i] = new VSPAERO_DOUBLE[NumberOfSurfaceVortexEdges_ + 1];    
   
          dCpUnsteadyNoise_[i] = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];    
            
          UNoise_[i] = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];    
          VNoise_[i] = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];    
          WNoise_[i] = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];    
   
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

       UnsteadyTrailingWakeVelocity_[i] = new VSPAERO_DOUBLE[3];
       
       LocalBodySurfaceVelocityForLoop_[i] = new VSPAERO_DOUBLE[3];
       
       // Zero out trailing wake velocities
       
       UnsteadyTrailingWakeVelocity_[i][0] = 0.;
       UnsteadyTrailingWakeVelocity_[i][1] = 0.;
       UnsteadyTrailingWakeVelocity_[i][2] = 0.;
       
       // Zero out local body surface velocities
       
       LocalBodySurfaceVelocityForLoop_[i][0] = 0.;
       LocalBodySurfaceVelocityForLoop_[i][1] = 0.;
       LocalBodySurfaceVelocityForLoop_[i][2] = 0.;         

    }
    
    for ( i = 1 ; i <= NumberOfSurfaceVortexEdges_ ; i++ ) {
       
       LocalBodySurfaceVelocityForEdge_[i] = new VSPAERO_DOUBLE[3];
       
       // Zero out local body surface velocities
       
       LocalBodySurfaceVelocityForEdge_[i][0] = 0.;
       LocalBodySurfaceVelocityForEdge_[i][1] = 0.;
       LocalBodySurfaceVelocityForEdge_[i][2] = 0.;       
       
    }           
    
    // Allocate space for span loading data ... for VLM Model

    if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {
                
       // Mark those wings that are on rotors
            
       ComponentInThisGroup = new int[VSPGeom().NumberOfComponents() + 1];
       
       zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());
       
       for ( i = 1 ; i <= NumberOfComponentGroups_ ; i++ ) {
          
          if ( ComponentGroupList_[i].GeometryIsARotor() ) {
       
             for ( j = 1 ; j <= ComponentGroupList_[i].NumberOfComponents() ; j++ ) {
             
                ComponentInThisGroup[ComponentGroupList_[i].ComponentList(j)] = 1;
                
             }
       
          }
          
       }
       
       // Size the span loading data
     
       SpanLoadingData_ = 1;
       
       StartOfSpanLoadDataSets_ = 1;
              
       NumberOfSpanLoadDataSets_ = VSPGeom().NumberOfSurfaces();
    
       SpanLoadData_ = new SPAN_LOAD_DATA[NumberOfSpanLoadDataSets_ + 1];
       
       for ( i = 0 ; i <= NumberOfSpanLoadDataSets_ ; i++ ) {

          NumberOfStations = 1;
          
          if ( i > 0 ) NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
          
          SpanLoadData(i).Size(NumberOfStations);
          
          // Mark those on rotors
          
          if ( ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()] ) {
             
             SpanLoadData(i).IsARotor() = 1;
             
             PRINTF("marking ! \n");fflush(NULL);
             
          }
          
       }
       
       for ( i = 1 ; i <= NumberOfSpanLoadDataSets_ ; i++ ) { 
          
          for ( k = 1 ; k <= SpanLoadData(i).NumberOfSpanStations() ; k++ ) {

             // Root chord vector and length
             
             SpanLoadData(i).Span_Chord(k) = VSPGeom().VSP_Surface(i).LocalChord(k);
             
             S[0] = VSPGeom().VSP_Surface(i).xTE(k) - VSPGeom().VSP_Surface(i).xLE(k);
             S[1] = VSPGeom().VSP_Surface(i).yTE(k) - VSPGeom().VSP_Surface(i).yLE(k);
             S[2] = VSPGeom().VSP_Surface(i).zTE(k) - VSPGeom().VSP_Surface(i).zLE(k);
             
             Mag = sqrt(vector_dot(S,S));
             
             S[0] /= Mag;
             S[1] /= Mag;
             S[2] /= Mag;     
             
             SpanLoadData(i).Span_Svec(k)[0] = S[0];  
             SpanLoadData(i).Span_Svec(k)[1] = S[1];  
             SpanLoadData(i).Span_Svec(k)[2] = S[2];  
             
             // LE location
             
             SpanLoadData(i).Span_XLE(k) = VSPGeom().VSP_Surface(i).xLE(k),
             SpanLoadData(i).Span_XLE(k) = VSPGeom().VSP_Surface(i).yLE(k),
             SpanLoadData(i).Span_XLE(k) = VSPGeom().VSP_Surface(i).zLE(k),
                          
             // TE location
             
             SpanLoadData(i).Span_XTE(k) = VSPGeom().VSP_Surface(i).xTE(k);
             SpanLoadData(i).Span_YTE(k) = VSPGeom().VSP_Surface(i).yTE(k);                      
             SpanLoadData(i).Span_ZTE(k) = VSPGeom().VSP_Surface(i).zTE(k);                   
             
             // Surface Normal vector

             SpanLoadData(i).Span_Nvec(k)[0] = VSPGeom().VSP_Surface(i).NxQC(k); 
             SpanLoadData(i).Span_Nvec(k)[1] = VSPGeom().VSP_Surface(i).NyQC(k); 
             SpanLoadData(i).Span_Nvec(k)[2] = VSPGeom().VSP_Surface(i).NzQC(k);              

             // Span location
             
             SpanLoadData(i).Span_S(k) = VSPGeom().VSP_Surface(i).s(k);
             
          }
          
          SpanLoadData(i).Root_LE(0) = VSPGeom().VSP_Surface(i).Root_LE(0);
          SpanLoadData(i).Root_LE(1) = VSPGeom().VSP_Surface(i).Root_LE(1);
          SpanLoadData(i).Root_LE(2) = VSPGeom().VSP_Surface(i).Root_LE(2);
          
          SpanLoadData(i).Root_TE(0) = VSPGeom().VSP_Surface(i).Root_TE(0);
          SpanLoadData(i).Root_TE(1) = VSPGeom().VSP_Surface(i).Root_TE(1);
          SpanLoadData(i).Root_TE(2) = VSPGeom().VSP_Surface(i).Root_TE(2);
          
          SpanLoadData(i).Root_QC(0) = VSPGeom().VSP_Surface(i).Root_QC(0);
          SpanLoadData(i).Root_QC(1) = VSPGeom().VSP_Surface(i).Root_QC(1);
          SpanLoadData(i).Root_QC(2) = VSPGeom().VSP_Surface(i).Root_QC(2);
          
          SpanLoadData(i).Tip_LE(0) = VSPGeom().VSP_Surface(i).Tip_LE(0);
          SpanLoadData(i).Tip_LE(1) = VSPGeom().VSP_Surface(i).Tip_LE(1);
          SpanLoadData(i).Tip_LE(2) = VSPGeom().VSP_Surface(i).Tip_LE(2);
                         
          SpanLoadData(i).Tip_TE(0) = VSPGeom().VSP_Surface(i).Tip_TE(0);
          SpanLoadData(i).Tip_TE(1) = VSPGeom().VSP_Surface(i).Tip_TE(1);
          SpanLoadData(i).Tip_TE(2) = VSPGeom().VSP_Surface(i).Tip_TE(2);
                
          SpanLoadData(i).Tip_QC(0) = VSPGeom().VSP_Surface(i).Tip_QC(0);
          SpanLoadData(i).Tip_QC(1) = VSPGeom().VSP_Surface(i).Tip_QC(1);
          SpanLoadData(i).Tip_QC(2) = VSPGeom().VSP_Surface(i).Tip_QC(2);
       
       }
       
       delete [] ComponentInThisGroup;
    
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
    VSPAERO_DOUBLE Vec1[3], Vec2[3], Dot;
        
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

    PRINTF("Setting up Kelvin constraints... and there are: %10d Vortex Sheets \n", NumberOfVortexSheets_);
             
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

          PRINTF("Looking for node: %d \n",Node);fflush(NULL);
          
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
            
             PRINTF("Error in determining number of Kelvin regions for a periodic wake surface! \n");
             PRINTF("Looking for node: %d \n",Node);
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
                     
                      PRINTF("wtf... how did we jump to another Kelvin Group... \n"); fflush(NULL);
                      exit(1);
                      
                   }
                   
                   if ( LoopInKelvinConstraintGroup_[Loop2] == KelvinGroup ) {
                      
                      LoopInKelvinConstraintGroup_[Loop2] = -KelvinGroup;
    
                      if ( Loop2 != Loop ) LoopStack[++StackSize] = Loop2;
              
                   }    
                  
                   else if ( LoopInKelvinConstraintGroup_[Loop2] != -KelvinGroup ){
                     
                      PRINTF("wtf... how did we jump to another Kelvin Group... \n"); fflush(NULL);
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
                              
             PRINTF("Base region found for vortex sheet system: %d \n",k);fflush(NULL);
                                        
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

    PRINTF("There are %d Kelvin constraints \n",NumberOfKelvinConstraints_);
    
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
    VSPAERO_DOUBLE xyz[3], q[5], CA, SA, CB, SB, Rate_P, Rate_Q, Rate_R;
    VSPAERO_DOUBLE gamma, f1, gm1, gm2, gm3;
    VSP_NODE VSP_Node1, VSP_Node2;
   
    // Limit lower value of Mach number
    
    if ( Mach_ <= 0. ) Mach_ = 0.001;

    // Set Mach number for the edge class... it is static across all edge instances

    VSPGeom().Grid(1).EdgeList(1).SetMach(Mach_);    
    
    // Set multi-pole far away ratio
    
    FarAway_ = 10.;
    
    if ( Mach_ >= 1. ) FarAway_ = 9999999.;

    // Turn on KT correction

    if ( KarmanTsienCorrection_  && ModelType_ == VLM_MODEL ) {

       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

          // if ( Mach_ * 1.125 < 0.9 ) SurfaceVortexEdge(j).KTFact() = 1.125;

          SurfaceVortexEdge(j).KTFact() = MAX(1.,MIN(1.125, 0.90/Mach_));

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
       
          LocalBodySurfaceVelocityForLoop_[i][0] = 0.;
          LocalBodySurfaceVelocityForLoop_[i][1] = 0.;
          LocalBodySurfaceVelocityForLoop_[i][2] = 0.;            
       
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
     
       if ( ABS(Vinf_) > 1.e-6 ) PRINTF("Reducing free stream velocity to: %f \n",Vinf_);
   
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

       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(0) -= LocalBodySurfaceVelocityForLoop_[i][0];
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(1) -= LocalBodySurfaceVelocityForLoop_[i][1];
       VSPGeom().Grid(1).LoopList(i).LocalFreeStreamVelocity(2) -= LocalBodySurfaceVelocityForLoop_[i][2];
     
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
    VSPAERO_DOUBLE Fact;

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
 
    int i, j, k, c, jj, kk, p, q, Node, NumEdges, Node1, Node2, Node3, Loop;
    int NodeA, NodeB, Found, Done;
    int NumberOfSheets, NumberOfKuttaNodes, Hits, MaxNumberOfCommonTEs;
    int *ComponentInThisGroup, NumberOfStations, *ComponentGroup;
    int *NumberOfVortexSheetsForComponent, *MaxNumberOfVortexSheetsForComponent, **VortexSheetListForComponent;
    int MaxNumberOfComponents, ComponentID, Edge;
    VSPAERO_DOUBLE FarDist, *Sigma, *VecX, *VecY, *VecZ, VecTe[3], Dot, Dist;
    VSPAERO_DOUBLE Scale_X, Scale_Y, Scale_Z, WakeDist, xyz_te[3], *MaxDistance;
    VSPAERO_DOUBLE Vec0[3], Vec1[3], Vec2[3], Vec3[3], Mag, dt;
    VSPAERO_DOUBLE VecS[3], VecT[3], VecN[3], S, S1, S2, Uc, U1, U2;
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
    
    PRINTF("Xmax_ - Xmin_: %f \n",Xmax_ - Xmin_);
    PRINTF("Ymax_ - Ymin_: %f \n",Ymax_ - Ymin_);
    PRINTF("Zmax_ - Zmin_: %f \n",Zmax_ - Zmin_);
    PRINTF("\n");
    
    // Override far field distance
    
    if ( SetFarFieldDist_ ) {
       
       FarDist = FarFieldDist_;
       
    }
    
    else {
       
       FarFieldDist_ = FarDist;
       
    }
    
    PRINTF("Wake FarDist set to: %f \n",FarDist);
    PRINTF("\n");
       
    // Set intial wake start time
    
    WakeStartingTime_ = 0;
    
    if ( TimeAccurate_ && StartFromSteadyState_ ) WakeStartingTime_ = NumberOfWakeTrailingNodes_;

    // Determine the minimum trailing edge spacing for edge kutta node

    Sigma = new VSPAERO_DOUBLE[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    
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
    
    VecX = new VSPAERO_DOUBLE[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    VecY = new VSPAERO_DOUBLE[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    VecZ = new VSPAERO_DOUBLE[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    
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
    
    PRINTF("There are: %10d Vortex Sheets \n", NumberOfVortexSheets_);
    
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
        
    PRINTF("Creating vortex sheet data... \n"); fflush(NULL);

    // Create copy of vortex sheet data for each CPU
    
    for ( c = 0 ; c < NumberOfThreads_ ; c++ ) {
       
       i = 0;
       
       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
          
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
             
             PRINTF("Warning ... zero kutta nodes for sheet: %d \n",k);
             fflush(NULL);
             
          }
          
       }
       
       // Mark those vortex sheets that come off rotors for time accurate cases
       
       if ( TimeAccurate_ ) { 
       
          // Mark any unsteady rotor components
          
          ComponentInThisGroup = new int[VSPGeom().NumberOfComponents() + 1];

          zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());
       
          for ( i = 1 ; i <= NumberOfComponentGroups_ ; i++ ) {
             
              if ( ComponentGroupList_[i].GeometryIsARotor() ) {
       
                for ( j = 1 ; j <= ComponentGroupList_[i].NumberOfComponents() ; j++ ) {
                
                   ComponentInThisGroup[ComponentGroupList_[i].ComponentList(j)] = 1;
                   
                }
       
             }
             
          }
                    
       }
       
       dt = 0.;
       
       if ( QuasiTimeAccurate_  ) {
          
          if ( Vinf_ > 0. ) {
             
             dt = 1.5*FarDist / ( Vinf_ * NumberOfTimeSteps_ );
             
          }
          
          PRINTF("dt: %f ... TimeStep_: %f \n",dt,TimeStep_);
          
          dt = MAX(dt,TimeStep_);
             
       }                          
           
       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

          NumEdges = 0;
          
          VortexSheet(c,k).TimeAccurate() = TimeAccurate_;
          
          VortexSheet(c,k).QuasiTimeAccurate() = QuasiTimeAccurate_;
          
          VortexSheet(c,k).TimeAnalysisType() = TimeAnalysisType_;
     
          VortexSheet(c,k).Vinf() = SGN(Vinf_)*MAX(0.000001,ABS(Vinf_));
       
          VortexSheet(c,k).TimeStep() = TimeStep_;  

          VortexSheet(c,k).FarAwayRatio() = FarAway_;
          
          VortexSheet(c,k).DoGroundEffectsAnalysis() = DoGroundEffectsAnalysis();
          
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
                
                VortexSheet(c,k).TrailingVortex(NumEdges).TimeAccurate() = TimeAccurate_;
                
                VortexSheet(c,k).TrailingVortex(NumEdges).QuasiTimeAccurate() = QuasiTimeAccurate_;
                
                VortexSheet(c,k).TrailingVortex(NumEdges).TimeAnalysisType() = TimeAnalysisType_;
   
                VortexSheet(c,k).TrailingVortex(NumEdges).RotorAnalysis() = 0;
                                
                VortexSheet(c,k).TrailingVortex(NumEdges).FarAwayRatio() = FarAway_;
                
                VortexSheet(c,k).TrailingVortex(NumEdges).DoGroundEffectsAnalysis() = DoGroundEffectsAnalysis();
  
                if ( RotorAnalysis_ ) VortexSheet(c,k).TrailingVortex(NumEdges).RotorAnalysis() = 1;
                   
                VortexSheet(c,k).TrailingVortex(NumEdges).Vinf() = MAX(0.000001,Vinf_);;
   
                VortexSheet(c,k).TrailingVortex(NumEdges).BladeRPM() = BladeRPM_;
                
                VortexSheet(c,k).TrailingVortex(NumEdges).TimeStep() = TimeStep_;   
       
                // Pointer to the wing this trailing vortex leaves from
         
                VortexSheet(c,k).TrailingVortex(NumEdges).Wing() = k;
                
                // Flag if the vortex sheet is periodic (eg would be a nacelle)
                
                VortexSheet(c,k).IsPeriodic() = VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNodeIsPeriodic(j);
   
                // Pointer to the kutta node
                
                VortexSheet(c,k).TrailingVortex(NumEdges).Node() = VSPGeom().Grid(MGLevel_).KuttaNode(j);
                
                // Location along span of this kutta node S over Span
                
                VortexSheet(c,k).TrailingVortex(NumEdges).SoverB() = VSPGeom().Grid(MGLevel_).KuttaNodeSoverB(j);
                
                // Component ID
               
                VortexSheet(c,k).TrailingVortex(NumEdges).ComponentID() = VSPGeom().Grid(MGLevel_).ComponentIDForKuttaNode(j);
                
                // Check for unsteady rotor components
                
                if ( TimeAccurate_ ) {
                   
                   if ( ComponentInThisGroup[VortexSheet(c,k).TrailingVortex(NumEdges).ComponentID()] ) {
                      
                      VortexSheet(c,k).IsARotor() = 1;
                      
                      VortexSheet(c,k).TrailingVortex(NumEdges).IsARotor() = 1;
                      
                   }
                   
                }
                   
                if ( QuasiTimeAccurate_ && !VortexSheet(c,k).IsARotor() ) {
                   
                   VortexSheet(c,k).TimeStep() = VortexSheet(c,k).TrailingVortex(NumEdges).TimeStep() = dt;
                   
                }                             
                                                                    
                // Pass in edge data and create edge cofficients
                
                VSP_Node1.x() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeX(j);
                VSP_Node1.y() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeY(j);
                VSP_Node1.z() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeZ(j);
   
                VSP_Node2.x() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeX(j) + WakeAngle_[0] * 1.e6;
                VSP_Node2.y() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeY(j) + WakeAngle_[1] * 1.e6;
                VSP_Node2.z() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeZ(j) + WakeAngle_[2] * 1.e6;
          
                // Set sigma
 
                VortexSheet(c,k).TrailingVortex(NumEdges).Sigma() = 0.5*Sigma[VSPGeom().Grid(MGLevel_).KuttaNode(j)];

                // Set trailing edge direction 
                
                VortexSheet(c,k).TrailingVortex(NumEdges).TEVec(0) = VecX[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
                VortexSheet(c,k).TrailingVortex(NumEdges).TEVec(1) = VecY[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
                VortexSheet(c,k).TrailingVortex(NumEdges).TEVec(2) = VecZ[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
               
                // Create trailing wakes... specify number of sub vortices per trail
      
                WakeDist = MAX(VSP_Node1.x() + 0.5*FarDist, Xmax_ + 0.25*FarDist) - VSP_Node1.x();
     
                if ( QuasiTimeAccurate_ && !VortexSheet(c,k).IsARotor() ) {
                   
                   VortexSheet(c,k).TrailingVortex(NumEdges).Setup(MIN(NumberOfWakeTrailingNodes_,64),WakeDist,VSP_Node1,VSP_Node2);
                   
                }    
                
                else {
                   
                   VortexSheet(c,k).TrailingVortex(NumEdges).Setup(NumberOfWakeTrailingNodes_,WakeDist,VSP_Node1,VSP_Node2);                   
                   
                }
                                     

             }
                
          }
      
          VortexSheet(c,k).SetupVortexSheets();
          
          if ( c == 0 ) {

             if ( VortexSheet(c,k).IsPeriodic() ) {
                
                PRINTF("There are: %10d kutta nodes for vortex sheet: %10d     <----- Periodic Wake \n",VortexSheet(c,k).NumberOfTrailingVortices(),k); fflush(NULL);
                
             }
             
             else {
                
                PRINTF("There are: %10d kutta nodes for vortex sheet: %10d  \n",VortexSheet(c,k).NumberOfTrailingVortices(),k); fflush(NULL);
                
             }
             
          }
   
       }
       
       if ( TimeAccurate_ ) delete [] ComponentInThisGroup;
       
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
             
             xyz_te[0] = VortexSheet(k).TrailingVortex(j).TE_Node().x();
             xyz_te[1] = VortexSheet(k).TrailingVortex(j).TE_Node().y();
             xyz_te[2] = VortexSheet(k).TrailingVortex(j).TE_Node().z();
             
             for ( kk = k + 1 ; kk <= NumberOfVortexSheets_ ; kk++ ) {

                if ( inside_box( VortexSheet(kk).TEBox(), xyz_te ) ) {
         
                   for ( jj = 1 ; jj <= VortexSheet(kk).NumberOfTrailingVortices() ; jj++ ) {
                      
                      Dist = sqrt( SQR(xyz_te[0] - VortexSheet(kk).TrailingVortex(jj).TE_Node().x())
                                 + SQR(xyz_te[1] - VortexSheet(kk).TrailingVortex(jj).TE_Node().y())
                                 + SQR(xyz_te[2] - VortexSheet(kk).TrailingVortex(jj).TE_Node().z()) );
                                 
                      if ( Dist <= 0.1*VortexSheet( k).TrailingVortex(j ).Sigma() ||
                           Dist <= 0.1*VortexSheet(kk).TrailingVortex(jj).Sigma() ) {
                              
                         if ( !TimeAccurate_ || ComponentGroup[VortexSheet(k).TrailingVortex(j).ComponentID()] == ComponentGroup[VortexSheet(kk).TrailingVortex(jj).ComponentID()] ) {
                              
                            PRINTF("Found a TE node that is common between vortex sheets: %d and %d \n",k,kk);
                            
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

    // Allocate space for span loading data ... for Panel Model

    if ( ( ModelType_ == PANEL_MODEL || ( ModelType_ == VLM_MODEL && SurfaceType_ == VSPGEOM_SURFACE ) ) && !SpanLoadingData_ && PanelSpanWiseLoading_ && !DumpGeom_ ) {

       SpanLoadingData_ = 1;

       StartOfSpanLoadDataSets_ = 0;
       
       NumberOfSpanLoadDataSets_ = NumberOfVortexSheets_;
       
       SpanLoadData_ = new SPAN_LOAD_DATA[NumberOfSpanLoadDataSets_ + 1];
       
       for ( i = 0 ; i <= NumberOfSpanLoadDataSets_ ; i++ ) {
       
          NumberOfStations = 1;
          
          if ( i > 0 ) NumberOfStations = VortexSheet(i).NumberOfTrailingVortices() - 1;
          
          SpanLoadData(i).Size(NumberOfStations);
          
       }
              
       MaxDistance = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];
           
       for ( p = 1 ; p <= NumberOfVortexLoops_ ; p++ ) {
       
          VortexLoop(p).SpanStation() = 0;
          
          VortexLoop(p).VortexSheet() = 0;
       
          MaxDistance[p] = 1.e9;
          
       }
       
       // Create a loop to vortex sheet list
       
       MaxNumberOfComponents = 0;
       
      //for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
      //
      //   for ( j = 1 ; j <= VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
      //      
      //      MaxNumberOfComponents = MAX(MaxNumberOfComponents,VSPGeom().Grid(1).NodeList(Node1).ComponentID());
      //
      //   }
      //   
      //}

       for ( k = 1 ; k <= VSPGeom().Grid(1).NumberOfNodes() ; k++ ) {
       
          MaxNumberOfComponents = MAX(MaxNumberOfComponents,VSPGeom().Grid(1).NodeList(k).ComponentID());

       }
                     
       NumberOfVortexSheetsForComponent = new int[MaxNumberOfComponents + 1];
       
       MaxNumberOfVortexSheetsForComponent = new int[MaxNumberOfComponents + 1];
              
       VortexSheetListForComponent = new int*[MaxNumberOfComponents + 1];
       
       zero_int_array(NumberOfVortexSheetsForComponent, MaxNumberOfComponents);
       
       for ( i = 1 ; i <= MaxNumberOfComponents ; i++ ) {
          
          MaxNumberOfVortexSheetsForComponent[i] = 5;
          
          VortexSheetListForComponent[i] = new int[MaxNumberOfVortexSheetsForComponent[i] + 1];
          
       }
       
       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
          for ( j = 1 ; j <= VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
             
             Node1 = VortexSheet(k).TrailingVortex(j).Node();
             
             ComponentID = VSPGeom().Grid(1).NodeList(Node1).ComponentID();

             if ( NumberOfVortexSheetsForComponent[ComponentID] == 0 ) {
                
                NumberOfVortexSheetsForComponent[ComponentID] = 1;
                
                VortexSheetListForComponent[ComponentID][1] = k;
 
             }
             
             else {
             
                i = 1;
                
                Found = 0;
                
                while ( i <= NumberOfVortexSheetsForComponent[ComponentID] && !Found ) {
            
                   if ( VortexSheetListForComponent[ComponentID][i] == k ) Found = 1;
                   
                   i++;
                   
                }
                
                if ( !Found ) {
                   
                   if ( NumberOfVortexSheetsForComponent[ComponentID] + 1 <= MaxNumberOfVortexSheetsForComponent[ComponentID] ) {
                      
                      NumberOfVortexSheetsForComponent[ComponentID]++;
                      
                      VortexSheetListForComponent[ComponentID][NumberOfVortexSheetsForComponent[ComponentID]] = k;
                      
                   }
                   
                   else {
                      
                      VortexSheetListForComponent[ComponentID] = resize_int_array(VortexSheetListForComponent[ComponentID], 
                                                                                  MaxNumberOfVortexSheetsForComponent[ComponentID], 
                                                                                  MaxNumberOfVortexSheetsForComponent[ComponentID] + 5);
                                       
                      MaxNumberOfVortexSheetsForComponent[ComponentID] += 5;
                     
                      NumberOfVortexSheetsForComponent[ComponentID]++;
                      
                      VortexSheetListForComponent[ComponentID][NumberOfVortexSheetsForComponent[ComponentID]] = k;      
                      
                   }
                   
                }
                
             }                               
                               
          }
          
       }
       
       // Trailing edge to vortex sheet list

       for ( p = 1 ; p <= NumberOfSurfaceVortexEdges_ ; p++ ) {

          if ( SurfaceVortexEdge(p).IsTrailingEdge() ) {  
             
             Loop = SurfaceVortexEdge(p).Loop1();
             
             ComponentID = VortexLoop(Loop).ComponentID();
            
             Found = 0;

             i = 1;

             while ( i <= NumberOfVortexSheetsForComponent[ComponentID] && !Found ) {
                
             k = VortexSheetListForComponent[ComponentID][i];
          
                Node1 = MIN(SurfaceVortexEdge(p).Node1(), SurfaceVortexEdge(p).Node2());
                Node2 = MAX(SurfaceVortexEdge(p).Node1(), SurfaceVortexEdge(p).Node2());
                                
                j = 1;
   
                while ( j < VortexSheet(k).NumberOfTrailingVortices() && Found < 2 ) {
                   
                   NodeA = MIN(VortexSheet(k).TrailingVortex(j).Node(),VortexSheet(k).TrailingVortex(j+1).Node());
                   NodeB = MAX(VortexSheet(k).TrailingVortex(j).Node(),VortexSheet(k).TrailingVortex(j+1).Node());

                   // Find this edge
                   
                   if ( Node1 == NodeA && Node2 == NodeB ) {
                      
                      VortexSheet(k).TrailingVortex(j).Edge() = p;
                   
                      Found = 2;
                      
                   }
                   
                   else if ( Node1 == NodeA || Node1 == NodeB ) {
                      
                      Found = 1;
                      
                   }
                   
                   else if ( Node2 == NodeA || Node2 == NodeB ) {
                   
                      Found = 1;
                      
                   }
                   
                   j++;
                   
                }
                
                i++;
                
             }
             
             if ( Found == 0 ) {
                
                PRINTF("Could not find matching edge! \n");
                
                PRINTF("Working on vortex sheet: %d --> Node1, Node2: %d %d \n",k,Node1,Node2);
                
                fflush(NULL); exit(1);
             
             }             
             
          }
          
       }
       
       PRINTF("MaxNumberOfComponents: %d \n",MaxNumberOfComponents);

       // Loop over all vortex loops on a lifting surface and associate them with a vortex sheet
       
       for ( p = 1 ; p <= NumberOfVortexLoops_ ; p++ ) {
          
          ComponentID = VortexLoop(p).ComponentID();
          
          q = 1;
          
          Done = 0;

          while ( q <= NumberOfVortexSheetsForComponent[ComponentID] && !Done ) {
                 
             k = VortexSheetListForComponent[ComponentID][q];
             
             // If this loop is on a lifting surface, determine which 'span station' it sits in
             
             if ( k > 0 ) {

                Uc = 0.;
                
                for ( i = 1 ; i <= VortexLoop(p).NumberOfNodes() ; i++ ) {
                   
                   Uc += VortexLoop(p).U_Node(i);
                   
                }
                
                Uc /= VortexLoop(p).NumberOfNodes();
                               
                j = 1;
                
                Done = 0;
                                
                while ( j < VortexSheet(k).NumberOfTrailingVortices() && !Done ) { 
                
                   Edge = VortexSheet(k).TrailingVortex(j).Edge();
                   
                   if ( VortexLoop(p).ComponentID() == SurfaceVortexEdge(Edge).ComponentID() ) {
                   
                      Node1 = SurfaceVortexEdge(Edge).Node1();
                      Node2 = SurfaceVortexEdge(Edge).Node2();
                      
                      Loop = SurfaceVortexEdge(Edge).Loop1();
                      
                      Found = 0;
                      
                      i = 1;
                      
                      while ( i <= VortexLoop(Loop).NumberOfNodes() && Found < 2 ) {
                                                                   
                         if ( VortexLoop(Loop).Node(i) == Node1 ) { U1 = VortexLoop(Loop).U_Node(i) ; Found++; };
                         if ( VortexLoop(Loop).Node(i) == Node2 ) { U2 = VortexLoop(Loop).U_Node(i) ; Found++; };
                         
                         i++;
                         
                      }
                      
                      if ( q == NumberOfVortexSheetsForComponent[ComponentID] && Found != 2 ) {
                                            
                         PRINTF("Error in determining loop edge for U1-2 values! \n");
                         fflush(NULL); exit(1);
                         
                      }
                      
                      if ( Uc >= MIN(U1,U2) && Uc <= MAX(U1,U2) ) {
                       
                         VortexLoop(p).SpanStation() = j;
                     
                         VortexLoop(p).VortexSheet() = k;      
                         
                         Done = 1;
                                        
                      }
                      
                   }
                   
                   j++;           
   
                }
                
                if ( !Done ) {
                   
                   VortexLoop(p).SpanStation() = 0;
                   
                   VortexLoop(p).VortexSheet() = 0; 
                   
                }
                
             }
             
             q++;
             
          }
          
       }
       
       delete [] VortexSheetListForComponent;

       delete [] MaxDistance;

       // Estimate the local chord
      
       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
          for ( j = 1 ; j < VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
  
             Node1 = VortexSheet(k).TrailingVortex(j  ).Node();
             Node2 = VortexSheet(k).TrailingVortex(j+1).Node();
              
             Vec1[0] = 0.5*( VSPGeom().Grid(1).NodeList(Node1).x() + VSPGeom().Grid(1).NodeList(Node2).x() );
             Vec1[1] = 0.5*( VSPGeom().Grid(1).NodeList(Node1).y() + VSPGeom().Grid(1).NodeList(Node2).y() );
             Vec1[2] = 0.5*( VSPGeom().Grid(1).NodeList(Node1).z() + VSPGeom().Grid(1).NodeList(Node2).z() );
             
             if ( j == 1 ) {
                
                S = 0.;
                
                Vec0[0] = Vec1[0];
                Vec0[1] = Vec1[1];
                Vec0[2] = Vec1[2];
                
             }
             
             else {
                
                Vec0[0] -= Vec1[0];
                Vec0[1] -= Vec1[1];
                Vec0[2] -= Vec1[2];
                
                S += sqrt(vector_dot(Vec0,Vec0));
                
                Vec0[0] = Vec1[0];
                Vec0[1] = Vec1[1];
                Vec0[2] = Vec1[2];                
                
             }
             
             for ( p = 1 ; p <= NumberOfVortexLoops_ ; p++ ) {
                
                if ( VortexLoop(p).VortexSheet() == k && VortexLoop(p).SpanStation() == j ) {
                   
                   Vec2[0] = VortexLoop(p).Xc() - Vec1[0]; // XLE - XTE
                   Vec2[1] = VortexLoop(p).Yc() - Vec1[1]; // YLE - YTE
                   Vec2[2] = VortexLoop(p).Zc() - Vec1[2]; // ZLE - ZTE
          
                   Mag = sqrt(vector_dot(Vec2,Vec2));
                   
                   if ( Mag >= SpanLoadData(k).Span_Chord(j) ) {
                      
                      // Store chord and direction vector
                
                      SpanLoadData(k).Span_Chord(j) = Mag;
                      
                      SpanLoadData(k).Span_Svec(j)[0] = -Vec2[0] / Mag;
                      SpanLoadData(k).Span_Svec(j)[1] = -Vec2[1] / Mag;
                      SpanLoadData(k).Span_Svec(j)[2] = -Vec2[2] / Mag;    
                      
                      SpanLoadData(k).Span_S(j) = S;

                      // TE location

                      SpanLoadData(k).Span_XTE(j) = Vec1[0];
                      SpanLoadData(k).Span_YTE(j) = Vec1[1];                      
                      SpanLoadData(k).Span_ZTE(j) = Vec1[2];                      
                      
                      // Calculate normal to strip
                      
                      VecS[0] = SpanLoadData(k).Span_Svec(j)[0];
                      VecS[1] = SpanLoadData(k).Span_Svec(j)[1];
                      VecS[2] = SpanLoadData(k).Span_Svec(j)[2];

                      VecT[0] = VSPGeom().Grid(1).NodeList(Node2).x() - VSPGeom().Grid(1).NodeList(Node1).x();
                      VecT[1] = VSPGeom().Grid(1).NodeList(Node2).y() - VSPGeom().Grid(1).NodeList(Node1).y();
                      VecT[2] = VSPGeom().Grid(1).NodeList(Node2).z() - VSPGeom().Grid(1).NodeList(Node1).z();

                      vector_cross(VecS, VecT, VecN);
                      
                      Mag = sqrt(vector_dot(VecN,VecN));
                      
                      SpanLoadData(k).Span_Nvec(j)[0] = VecN[0] / Mag;
                      SpanLoadData(k).Span_Nvec(j)[1] = VecN[1] / Mag; 
                      SpanLoadData(k).Span_Nvec(j)[2] = VecN[2] / Mag;     

                   }
                   
                }
                
             }
             
          }
          
       }
     
       // Create Span location data
       
       for ( k = 0 ; k <= NumberOfSpanLoadDataSets_ ; k++ ) {
       
          if ( SpanLoadData(k).NumberOfSpanStations() > 1 ) {
                
             S1 = SpanLoadData(k).Span_S(1);
             S2 = SpanLoadData(k).Span_S(SpanLoadData(k).NumberOfSpanStations());

             if ( S1 != S2 ) {
                             
               for ( j = 1 ; j <= SpanLoadData(k).NumberOfSpanStations() ; j++ ) {
                  
                  SpanLoadData(k).Span_S(j) = ( SpanLoadData(k).Span_S(j) - S1 )/(S2 - S1);
                  
               }
               
             }
             
             else {
                
               for ( j = 1 ; j <= SpanLoadData(k).NumberOfSpanStations() ; j++ ) {
                  
                  SpanLoadData(k).Span_S(j) = 0.;
                                    
               }
               
             }                
             
          }
          
          else {
             
             SpanLoadData(k).Span_S(1) = 0.;
             
             SpanLoadData(k).Span_Chord(1) = 1.;
             
          }
          
       }  

       // Accumulate areas, etc
              
       for ( p = 1 ; p <= NumberOfVortexLoops_ ; p++ ) {
          
          k = VortexLoop(p).VortexSheet();
          
          j = VortexLoop(p).SpanStation();
          
          if ( k == 0 && j == 0 ) j = VortexLoop(p).SpanStation() = 1;
         
          SpanLoadData(k).Span_Area(j) += VortexLoop(p).Area();
      
       }

       if ( Verbose_ ) {
          
          for ( k = 0 ; k <= NumberOfSpanLoadDataSets_ ; k++ ) {
          
             for ( j = 1 ; j <= SpanLoadData(k).NumberOfSpanStations() ; j++ ) {
          
                Node1 = VortexSheet(k).TrailingVortex(j  ).Node();
                Node2 = VortexSheet(k).TrailingVortex(j+1).Node();
                 
                Vec1[0] = SpanLoadData(k).Span_XTE(j);
                Vec1[1] = SpanLoadData(k).Span_YTE(j);
                Vec1[2] = SpanLoadData(k).Span_ZTE(j);
                
                PRINTF("Vortex Sheet: %d --> j: %d --> Span XYZ: %f %f %f --> Chord: %f \n",
                k,
                j,
                Vec1[0],
                Vec1[1],
                Vec1[2],                
                SpanLoadData(k).Span_Chord(j));
                
             }
             
          }
          
       }
       
       // Allocate space for span load data in the group data sets
    
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
        
          for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfLiftingSurfaces() ; i++ ) {
          
             j = ComponentGroupList_[c].SpanLoadData(i).SurfaceID();
          
             ComponentGroupList_[c].SpanLoadData(i).SetNumberOfSpanStations(SpanLoadData(j).NumberOfSpanStations());
          
          }
      
       }
       
       // Size span load arrays ... their size is a function of the number of span stations, and rotation period for unsteady flows
       
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
                  
          for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfLiftingSurfaces() ; i++ ) {
          
             ComponentGroupList_[c].SpanLoadData(i).SizeSpanLoadingList();
             
          }

       }          
             
    }
    
    else {
       
       StartOfSpanLoadDataSets_ = 0;
    
    }
    
    PRINTF("Done creating vortex sheet data... \n");fflush(NULL);
   
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

PRINTF("Adding edge at: %f %f %f with MinCoreSize: %f \n",SurfaceVortexEdge(p).Xc(),SurfaceVortexEdge(p).Yc(),SurfaceVortexEdge(p).Zc(),MinCoreSize[Node]);

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
 
    int c, i, j, k;
    char StatusFileName[2000], LoadFileName[2000], ADBFileName[2000];
    char GroupFileName[2000], RotorFileName[2000];
 
    // Zero out solution
   
    zero_double_array(Gamma_[0], NumberOfVortexLoops_); Gamma_[0][0] = 0.;   
    zero_double_array(Gamma_[1], NumberOfVortexLoops_); Gamma_[1][0] = 0.;
    zero_double_array(Gamma_[2], NumberOfVortexLoops_); Gamma_[2][0] = 0.;
    zero_double_array(Delta_,    NumberOfVortexLoops_);    Delta_[0] = 0.;
        
    CurrentTime_ = 0.;
    
    AveragingHasStarted_ = 0;  
   
    NumberOfAveragingSets_ = 0;
    
    CFx_[2] = 0.;
    CFy_[2] = 0.;
    CFz_[2] = 0.;
             
    CMx_[2] = 0.;
    CMy_[2] = 0.;
    CMz_[2] = 0.;
          
    CL_[2] = 0.; 
    CD_[2] = 0.; 
    CS_[2] = 0.; 
          
    CDo_[2] = 0.;  
    
    CDTrefftz_[2] = 0.;  
        
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
             
        CL_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
        CD_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
        CS_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];    
       CFx_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CFy_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CFz_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CMx_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CMy_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CMz_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];     
        
    }

    // Update geometry location and interaction lists for moving geoemtries

    CurrentTime_ = CurrentNoiseTime_ = 0.;
           
    if ( TimeAccurate_ && !StartFromSteadyState_ ) UpdateGeometryLocation(1);
             
    // Initialize free stream
    
    InitializeFreeStream();
    
    // Recalculate interaction lists if Mach crossed over Mach = 1
    
    if ( ( LastMach_ < 0.                 ) || 
         ( Mach_ >= 1. && LastMach_ <  1. ) ||
         ( Mach_ <  1. && LastMach_ >= 1. ) ) {
  
       if ( LastMach_ > 0. ) PRINTF("Updating interaction lists due to subsonic / supersonic Mach change \n");
       
       if ( !DumpGeom_ ) CreateSurfaceVorticesInteractionList(0);

    }
  
    LastMach_ = Mach_;

    // Initialize the wake trailing vortices

    InitializeTrailingVortices();
    
    ZeroVortexState();

    // Zero out span load data
 
    if ( !DumpGeom_ ) {
       
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
   
          for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfLiftingSurfaces() ; j++ ) {
          
             ComponentGroupList_[c].SpanLoadData(j).ZeroForcesAndMoments();
             
          }
          
       }
       
    }
          
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
       
       SPRINTF(StatusFileName,"%s.history",FileName_);
       
       if ( (StatusFile_ = fopen(StatusFileName, "w")) == NULL ) {
   
          PRINTF("Could not open the history file for output! \n");
   
          exit(1);
   
       }    
       
    }

    // Header for history file
    
    if ( ABS(Case) > 0 ) {
       
       // Write out generic header
       
       WriteCaseHeader(StatusFile_);
       
       // Status update to user
       
       FPRINTF(StatusFile_,"\n\nSolver Case: %d \n\n",ABS(Case));
       
    }
    
    if ( !TimeAccurate_ ) {

                          //123456789 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123          
       FPRINTF(StatusFile_,"  Iter         Mach           AoA          Beta            CL           CDo           CDi         CDtot            CS           L/D             E           CFx           CFy           CFz           CMx           CMy           CMz       CDtrefftz      T/QS \n");
   
    }
    
    else {
       
       if ( TimeAnalysisType_ == HEAVE_ANALYSIS ) {
                 
                             //1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123    
          FPRINTF(StatusFile_,"     Time          Mach           AoA          Beta            CL           CDo           CDi         CDtot            CS           L/D             E           CFx            CFy          CFz           CMx           CMy           CMz       CDtrefftz      T/QS             H    \n");

       }
       
       else if ( TimeAnalysisType_ == P_ANALYSIS || TimeAnalysisType_ == Q_ANALYSIS ||  TimeAnalysisType_ == R_ANALYSIS ) {
         
                             //1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123
          FPRINTF(StatusFile_,"     Time          Mach           AoA          Beta            CL           CDo           CDi         CDtot            CS           L/D             E           CFx            CFy          CFz           CMx           CMy           CMz       CDtrefftz       T/QS        UnstdyAng  \n");

       }
       
       else {

                             //1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123
          FPRINTF(StatusFile_,"     Time          Mach           AoA          Beta            CL           CDo           CDi         CDtot            CS           L/D             E           CFx            CFy          CFz           CMx           CMy           CMz       CDtrefftz       T/QS\n");
      
       }
   
    }

    // Open the adb and case list files the first time only
    
    if ( Case == 0 || Case == 1 ) {

       SPRINTF(ADBFileName,"%s.adb",FileName_);
       
       if ( (ADBFile_ = fopen(ADBFileName, "wb")) == NULL ) {
   
          PRINTF("Could not open the aerothermal data base file for binary output! \n");
   
          exit(1);
   
       }
       
       SPRINTF(ADBFileName,"%s.adb.cases",FileName_);
       
       if ( (ADBCaseListFile_ = fopen(ADBFileName, "w")) == NULL ) {
   
          PRINTF("Could not open the aerothermal data base case list file for output! \n");
   
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
       
       SPRINTF(GroupFileName,"%s.group.%d",FileName_,c);
    
       if ( (GroupFile_[c] = fopen(GroupFileName, "w")) == NULL ) {
    
          PRINTF("Could not open the %s group coefficient file! \n",GroupFileName);
    
          exit(1);
    
       }

                            //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890    
       FPRINTF(GroupFile_[c],"Force Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
       FPRINTF(GroupFile_[c],"   Time        Cx         Cy         Cz        Cxo        Cyo        Czo        Cxi        Cyi        Czi        Cmx        Cmy        Cmz        Cmxo       Cmyo       Cmzo       Cmxi       Cmyi       Cmzi        CL         CD         CS        CLo        CDo        CSo        CLi        CDi        CSi \n");
                          
       // Create rotor file
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          k++;
          
          SPRINTF(RotorFileName,"%s.rotor.%d",FileName_,k);
    
          if ( (RotorFile_[k] = fopen(RotorFileName, "w")) == NULL ) {
      
             PRINTF("Could not open the %s rotor coefficient file! \n",RotorFileName);
      
             exit(1);
      
          }

 
                               //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890
          FPRINTF(RotorFile_[k],"                                                                                                                                        ------------ Propeller Coefficients _----------       -------- Rotor Coefficients _-------\n\n");                        
          FPRINTF(RotorFile_[k]," Time       Diameter     RPM       Thrust    Thrusto    Thrusti     Power      Powero     Poweri     Moment     Momento    Momenti      J          CT         CQ         CP        EtaP       CT_H       CQ_H       CP_H       FOM        Angle \n");
 
       
       }
   
    }     

    // Write out ADB Geometry
    
    if ( Case == 0 || Case == 1 ) {

       WriteOutAerothermalDatabaseHeader();

       WriteOutAerothermalDatabaseGeometry();

    }

    PRINTF("Solving... Mach: %f ... Alpha: %f ... Beta: %f \n\n",Mach_,AngleOfAttack_/TORAD,AngleOfBeta_/TORAD);fflush(NULL);

    // Header for command line status

    if ( !TimeAccurate_ ) {

             //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789          
       PRINTF("  Iter      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz   CDTrefftz     T/QS \n");
   
    }
    
    if ( DumpGeom_ ) WakeIterations_ = 0;
    
    if ( TimeAccurate_ && !StartFromSteadyState_ ) WakeIterations_ = 1;
  
    // Solve at the each time step... or single solve if just a steady state solution

    if ( !TimeAccurate_ ) NumberOfTimeSteps_ = 1;
    
    // If time accurate, write out current t=0 solution state
    
    if ( TimeAccurate_ ) {
       
        SPRINTF(CaseString_,"Time: %-f ...",0.);
       
        WriteOutAerothermalDatabaseSolution();
    
    }

#ifdef AUTODIFF

    // Turn off auto diff recording

    adept::Stack *AutoDiffStack;
    
    AutoDiffStack = adept::active_stack();
        
    AutoDiffStack->pause_recording();

#endif

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
   
             if ( !StartFromSteadyState_ || ( StartFromSteadyState_ && Time_ > 1 ) ) {
                
                if ( !QuasiTimeAccurate_ ) CalculateUnsteadyWakeVelocities();
                
             }
                
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

          OutputStatusFile(0);
          
          // Write out group data, and any rotor data
  
          if ( !TimeAccurate_ ) OutputForcesAndMomentsForGroup(0);   
        
          PRINTF("\n");
   
       }
  
       if ( TimeAccurate_ && StartFromSteadyState_ ) WakeIterations_ = 1;

       // Write out ADB Solution for time accurate cases
       
       if ( TimeAccurate_ ) {
          
          if ( TimeAnalysisType_ == P_ANALYSIS ) {
             
             SPRINTF(CaseString_,"T: %-f, P: %-f",CurrentTime_, Unsteady_Angle_/TORAD);
             
          }
             
          else if ( TimeAnalysisType_ == Q_ANALYSIS ) {
             
             SPRINTF(CaseString_,"T: %-f, Q: %-f",CurrentTime_, Unsteady_Angle_/TORAD);

             
          }
          
          else if ( TimeAnalysisType_ == R_ANALYSIS ) {
             
             SPRINTF(CaseString_,"T: %-f, R: %-f",CurrentTime_, Unsteady_Angle_/TORAD);
             
          }
                
          else {
          
             SPRINTF(CaseString_,"Time: %-f ...",CurrentTime_);
             
          }
          
          WriteOutAerothermalDatabaseGeometry();

          InterpolateSolutionFromGrid(1);
          
          WriteOutAerothermalDatabaseSolution();

          // Write out group data, and any rotor data
  
          OutputForcesAndMomentsForGroup(0);    
          
       }
   
    }

    // Output status file... time averaged quantities

    if ( TimeAccurate_ ) OutputStatusFile(1);  
    
    // Output zero lift data to the status file
    
    OutputZeroLiftDragToStatusFile();

    // Open the load file the first time only
    
    if ( Case == 0 || Case == 1 ) {
    
       SPRINTF(LoadFileName,"%s.lod",FileName_);
       
       if ( (LoadFile_ = fopen(LoadFileName, "w")) == NULL ) {
   
          PRINTF("Could not open the spanwise loading file for output! \n");
   
          exit(1);
   
       }
       
    }         
    
    // Calculate spanwise load distributions for lifting surfaces
 
    if ( !DumpGeom_ && NumberOfSpanLoadDataSets_ > 0 ) CalculateSpanWiseLoading();
    
    // Write out FEM loading file
 
    if ( !DumpGeom_ ) CreateFEMLoadFile(Case);

    // Interpolate solution from grid 1 to 0
 
    InterpolateSolutionFromGrid(1);

    // Output and survey point results
    
    if ( NumberofSurveyPoints_ > 0 ) CalculateVelocitySurvey();
 
    // Write out ADB Solution

    if ( !TimeAccurate_ ) WriteOutAerothermalDatabaseSolution();
    
    // Write out 2d FEM geometry and solution if requested
    
    if ( Write2DFEMFile_ )  {
       
       if ( Case == 0 || Case == 1 ) WriteFEM2DGeometry();
       
       if ( !DumpGeom_ ) WriteFEM2DSolution();
       
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
#                   VSP_SOLVER WriteOutSteadyStateNoiseFiles                   #
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
             
        CL_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
        CD_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
        CS_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];    
       CFx_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CFy_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CFz_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CMx_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CMy_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CMz_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];       
 
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

    SPRINTF(StatusFileName,"%s.noise.history",FileName_);
    
    if ( (StatusFile_ = fopen(StatusFileName, "w")) == NULL ) {
   
       PRINTF("Could not open the history file for output! \n");
   
       exit(1);
   
    }    

    // Write out generic header
    
    WriteCaseHeader(StatusFile_);

    // Status update to user
    
    FPRINTF(StatusFile_,"\n\nSolver Case: %d \n\n",ABS(Case));


                       //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
    FPRINTF(StatusFile_,"  Time      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz   CDTrefftz     T/QS    \n");

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
       
       SPRINTF(GroupFileName,"%s.noise.group.%d",FileName_,c);
 
       if ( (GroupFile_[c] = fopen(GroupFileName, "w")) == NULL ) {
   
          PRINTF("Could not open the %s group coefficient file! \n",GroupFileName);
   
          exit(1);
   
       }

                            //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890    
       FPRINTF(GroupFile_[c],"Force Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
       FPRINTF(GroupFile_[c],"   Time        Cx         Cy         Cz        Cxo        Cyo        Czo        Cxi        Cyi        Czi        Cmx        Cmy        Cmz        Cmxo       Cmyo       Cmzo       Cmxi       Cmyi       Cmzi        CL         CD         CS        CLo        CDo        CLi        CDi \n");
                          
       // Create rotor file
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          k++;
          
          SPRINTF(RotorFileName,"%s.noise.rotor.%d",FileName_,k);
    
          if ( (RotorFile_[k] = fopen(RotorFileName, "w")) == NULL ) {
      
             PRINTF("Could not open the %s rotor coefficient file! \n",RotorFileName);
      
             exit(1);
      
          }

                               //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890
          FPRINTF(RotorFile_[k],"Rotor Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
          FPRINTF(RotorFile_[k],"   Time     Diameter     RPM       Thrust     Thrusto    Thrusti    Moment     Momento    Momenti      J          CT         CQ        EtaP       Angle \n");
                             
       }

    }     
  
    // Open the input adb file
    
    if ( Case == 0 || Case == 1 ) {

       SPRINTF(ADBFileName,"%s.adb",FileName_);
       
       if ( (InputADBFile_ = fopen(ADBFileName, "rb")) == NULL ) {
   
          PRINTF("Could not open the aerothermal data base file for binary input! \n");
   
          exit(1);
   
       }

       // Read in the header
       
       ReadInAerothermalDatabaseHeader();

    }
    
    // Open the output adb file
    
    if ( Case == 0 || Case == 1 ) {

       SPRINTF(ADBFileName,"%s.noise.adb",FileName_);
       
       if ( (ADBFile_ = fopen(ADBFileName, "wb")) == NULL ) {
   
          PRINTF("Could not open the aerothermal data base file for binary output! \n");
   
          exit(1);
   
       }

       WriteOutAerothermalDatabaseHeader();

    }

    // Open the case file
    
    SPRINTF(ADBFileName,"%s.noise.adb.cases",FileName_);
    
    if ( (ADBCaseListFile_ = fopen(ADBFileName, "w")) == NULL ) {

       PRINTF("Could not open the aerothermal data base case list file for output! \n");

       exit(1);

    }       

    // Setup PSU-WopWop data and write out file headers

    SetupPSUWopWopData();

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) WriteOutPSUWopWopFileHeadersForGroup(c);

    if ( TimeAccurate_ && !StartFromSteadyState_ ) WakeIterations_ = 1;
    
    // Step through the entire ADB file until we get to the end

    PRINTF("Stepping through solution and writing out sound files... \n\n");fflush(NULL);

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

       OutputStatusFile(0);

       // Write out any rotor coefficients
       
       OutputForcesAndMomentsForGroup(0);
       
       // Write out geometry and current solution

       SPRINTF(CaseString_,"Time: %-f ...",CurrentNoiseTime_);

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
    VSPAERO_DOUBLE Time_0, Time_1, Time_2, Epsilon, EvaluationTime, Period;
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
             
        CL_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
        CD_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
        CS_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];    
       CFx_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CFy_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CFz_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CMx_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CMy_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CMz_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];       
 
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

    SPRINTF(StatusFileName,"%s.noise.history",FileName_);
    
    if ( (StatusFile_ = fopen(StatusFileName, "w")) == NULL ) {
   
       PRINTF("Could not open the history file for output! \n");
   
       exit(1);
   
    }    

    // Write out generic header
    
    WriteCaseHeader(StatusFile_);

    // Status update to user
    
    FPRINTF(StatusFile_,"\n\nSolver Case: %d \n\n",ABS(Case));


                       //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
    FPRINTF(StatusFile_,"  Time      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz   CDTrefftz     T/QS   \n");

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
       
       SPRINTF(GroupFileName,"%s.noise.group.%d",FileName_,c);
 
       if ( (GroupFile_[c] = fopen(GroupFileName, "w")) == NULL ) {
   
          PRINTF("Could not open the %s group coefficient file! \n",GroupFileName);
   
          exit(1);
   
       }

                            //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890  
       FPRINTF(GroupFile_[c],"Force Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
       FPRINTF(GroupFile_[c],"   Time        Cx         Cy         Cz        Cxo        Cyo        Czo        Cxi        Cyi        Czi        Cmx        Cmy        Cmz        Cmxo       Cmyo       Cmzo       Cmxi       Cmyi       Cmzi        CL         CD        CLo        CDo        CLi        CDi \n");
                          
       // Create rotor file
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          k++;
          
          SPRINTF(RotorFileName,"%s.noise.rotor.%d",FileName_,k);
    
          if ( (RotorFile_[k] = fopen(RotorFileName, "w")) == NULL ) {
      
             PRINTF("Could not open the %s rotor coefficient file! \n",RotorFileName);
      
             exit(1);
      
          }

                               //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890  
          FPRINTF(RotorFile_[k],"Rotor Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
          FPRINTF(RotorFile_[k],"   Time     Diameter     RPM       Thrust     Thrusto    Thrusti    Moment     Momento    Momenti      J          CT         CQ        EtaP       Angle \n");
                             
       }

    }     
  
    // Open the input adb file

    SPRINTF(ADBFileName,"%s.adb",FileName_);
    
    if ( (InputADBFile_ = fopen(ADBFileName, "rb")) == NULL ) {
    
       PRINTF("Could not open the aerothermal data base file for binary input! \n");
    
       exit(1);
    
    }

    // Open the output adb file

    WopWopWriteOutADBFile_ = 1;    

    if ( WopWopWriteOutADBFile_ ) {

       SPRINTF(ADBFileName,"%s.noise.adb",FileName_);
       
       if ( (ADBFile_ = fopen(ADBFileName, "wb")) == NULL ) {
   
          PRINTF("Could not open the aerothermal data base file for binary output! \n");
   
          exit(1);
   
       }

       WriteOutAerothermalDatabaseHeader();

    }

    // Open the case file
    
    SPRINTF(ADBFileName,"%s.noise.adb.cases",FileName_);
    
    if ( (ADBCaseListFile_ = fopen(ADBFileName, "w")) == NULL ) {

       PRINTF("Could not open the aerothermal data base case list file for output! \n");

       exit(1);

    }       
    
    // Set up PSU-WopWop data
           
    SetupPSUWopWopData();
   
    // Loop over all the component groups and write out the PSU-WopWop data files

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       for ( i = 1 ; i <= NumberOfComponentGroups_ ; i++ ) {
          
          ComponentGroupList_[i].ZeroAverageForcesAndMoments();
          
       }
              
       PRINTF("\nWorking on group: %d \n",c);fflush(NULL);

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

       PRINTF("VSPAERO    Time Step: %f \n",TimeStep_);
       PRINTF("PSU-WopWop Time Step: %f \n",NoiseTimeStep_);

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
   
       if ( Verbose_ ) PRINTF("Stepping through solution and writing out sound files... \n\n");fflush(NULL);
   
       if ( NoiseTimeStep_ > TimeStep_ ) {
          
          PRINTF("Noise Time Step must be <= VSPAERO Solver Time Step! \n");
          exit(1);
          
       }
                 
       for ( NoiseTime_ = 1 ; NoiseTime_ <= NumberOfNoiseTimeSteps_ ; NoiseTime_++ ) {
          
          CurrentNoiseTime_ = (NoiseTime_-1)*NoiseTimeStep_;
          
          EvaluationTime = CurrentNoiseTime_ + NoiseTimeShift_;
      
          PRINTF("Noise Time Step: %d ... Time: %f \r",NoiseTime_, CurrentNoiseTime_); fflush(NULL);
          
          // Find solver time interval that bounds the noise time
          
          Found = 0;
       
          // Check if we are still in the current noise interval
      
          if ( EvaluationTime >= Time_0 - Epsilon && EvaluationTime <= Time_1 + Epsilon ) {
             
             // Nothing to do, current solution interval still bounds noise time
             
             if ( Verbose_ ) PRINTF("Current solver interval of %e to %e is bounding... for noise time of: %e ... which is evaluated at: %e \n",CurrentTime_ - TimeStep_, CurrentTime_,CurrentNoiseTime_, EvaluationTime);
             
             Found = 1;
             
          }
          
          else if ( NewTime == NumberOfTimeSteps_ - 1 && EvaluationTime >= NumberOfTimeSteps_ * TimeStep_ ) {
             
             Found = 1;
          
          }
          
          // Otherwise, we need to search for the new interval
          
          else {
   
             if ( Verbose_ ) PRINTF("Searching for interval containing noise time of: %e ... which is evaluated at: %e \n",CurrentNoiseTime_, EvaluationTime);
             
             NewTime = Time_ + 1;
             
             while ( !Found && NewTime <= NumberOfTimeSteps_ + 1 ) {
   
                Time_0 = (NewTime-2)*TimeStep_;
                Time_1 = (NewTime-1)*TimeStep_;
   
                if ( Verbose_ ) PRINTF("Trying Time: %d ... with interval: %e to %e \n",NewTime, Time_0, Time_1 );
         
                // Check if we are inside new interval
                
                if ( EvaluationTime >= Time_0 - Epsilon && EvaluationTime <= Time_1 + Epsilon ) {
                   
                   Found = 1;
   
                   if ( Verbose_ ) PRINTF("Moving to solver time interval: %d ... time: %e to %e \n",NewTime, Time_0, Time_1 );
                   
                   Time_ = NewTime;
                   
                   CurrentTime_ = Time_1;
                                   
                }
   
                
                else {
                   
                   NewTime++;
                   
                }
                
                // Read in the next ADB Solution
                  
                if ( Verbose_ ) PRINTF("Reading in next geometry set... \n");
                
                ReadInAerothermalDatabaseGeometry();
                
                if ( Verbose_ ) PRINTF("Reading in next solution set... \n");
                
                ReadInAerothermalDatabaseSolution(-1);
                          
             }
          
          }
          
          if ( !Found ) {
             
             PRINTF("Noise time is outside of solver solution time domain! \n");
             PRINTF("Noise time: %f \n",EvaluationTime);
             PRINTF("Solution time domain: %f to %f \n",0.,NumberOfTimeSteps_*TimeStep_);
             
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
   
          OutputStatusFile(0);
   
          // Write out any rotor coefficients
          
          OutputForcesAndMomentsForGroup(c);
          
          // Write out geometry and current solution
   
          SPRINTF(CaseString_,"Time: %-f ...",CurrentNoiseTime_);
   
          if ( WopWopWriteOutADBFile_ && c == 1 ) WriteOutAerothermalDatabaseGeometry();
   
          InterpolateSolutionFromGrid(1);
          
          if ( WopWopWriteOutADBFile_ && c == 1 ) WriteOutAerothermalDatabaseSolution();
                        
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
    VSPAERO_DOUBLE Normal[3];

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
   
    int i, v, cpu;
    
    // Calculate preconditioners
  
    if ( ( !TimeAccurate_ && CurrentWakeIteration_ == 1 && !DumpGeom_ ) || ( TimeAccurate_ && Time_ == 1 ) ) {

       for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
       
          for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
          
             VortexSheet(cpu,v).TurnWakeDampingOn();
             
          }    
       
       }   
       
       if ( Preconditioner_ != MATCON ) CalculateDiagonal();       
     
       if ( Preconditioner_ == SSOR   ) CalculateNeighborCoefs();

       if ( Preconditioner_ == MATCON ) CreateMatrixPreconditioners();

       for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
       
          for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
          
             VortexSheet(cpu,v).TurnWakeDampingOff();
             
          }    
       
       }    
          
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
    VSPAERO_DOUBLE q[4], Ws;
    
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
        
          if ( VortexLoop(i).SurfaceType() == DEGEN_BODY_SURFACE ) PRINTF("Loop: %d on body surface: %d has zero diagonal... \n",i,VortexLoop(i).DegenBodyID());fflush(NULL);
          if ( VortexLoop(i).SurfaceType() == DEGEN_WING_SURFACE ) PRINTF("Loop: %d on wing surface: %d has zero diagonal... \n",i,VortexLoop(i).DegenWingID());fflush(NULL);
          if ( VortexLoop(i).SurfaceType() == CART3D_SURFACE     ) PRINTF("Loop: %d on wing surface: %d has zero diagonal... \n",i,VortexLoop(i).Cart3dID());fflush(NULL);
         
          PRINTF("Area: %e \n",VortexLoop(i).Area());
          
          for ( j = 1 ; j <= VortexLoop(i).NumberOfEdges() ; j++ ) {
             
             Edge = VortexLoop(i).Edge(j);
             
             Node1 = SurfaceVortexEdge(j).Node1();
             Node2 = SurfaceVortexEdge(j).Node2();
             
             PRINTF("Edge: %d --> Node 1: %d @ %lf %lf %lf ... Node 2: %d @ %lf %lf %lf \n",
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
    VSPAERO_DOUBLE q[4];

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
    VSPAERO_DOUBLE q[3], *Diagonal, Ws, Tolerance, Distance, NormalDistance, Vec[3], Ratio;

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
       
       Diagonal = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];
       
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
    
    PRINTF("Creating matrix preconditioners data structure... \n");
    
    if ( VSPGeom().NumberOfGridLevels() == 1 ) {
       
       PRINTF("Error... mesh too coarse, or something else failed. Stopping in CreateMatrixPreconditionersDataStructure! \n");
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
       
       PRINTF("Starting at level: %d \n",Level);
       
       while ( !Done && Level >= 1 ) {
   
          Loops = 0;
   
          for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {
     
             NumLoops[i] = CalculateNumberOfFineLoops(Level, VSPGeom().Grid(Level).LoopList(i), LoopList);
    
             Loops = MAX(Loops,NumLoops[i]);
   
          }
          
          PRINTF("Level: %d has MaxLoops of: %d \n",Level,Loops);
          
          if ( Loops <= 1.25*TargetLoops ) Done = 1;
   
          Level--;
          
       }
       
       Level++;
       
       PRINTF("Starting at level: %d \n",Level); fflush(NULL);
       
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
             
             if ( Verbose_ ) PRINTF("Preconditioning Matrix %d contains %d fine loops \n",p,Loops); fflush(NULL);
      
             MatrixPreconditionerList_[p].Size(Loops);
   
             k = 0;
             
             for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {
                
                if ( LoopList[j] ) MatrixPreconditionerList_[p].VortexLoopList(++k) = j;
   
             }
             
             if ( k != Loops ) {
                
                PRINTF("Error in creating preconditioning matrix data structure! \n");
                PRINTF("k: %d ... Loops: %d \n",k,Loops);
                
                fflush(NULL);
                
                exit(1);
                
             }
      
             zero_int_array(LoopList, NumberOfVortexLoops_);
             
             Loops = 0;
             
          }
   
          i++;
           
       }
       
       AvgLoops /= p;
       
       PRINTF("Created: %d Matrix preconditioners \n",p);
       PRINTF("Min matrix: %d Loops \n",MinLoops);
       PRINTF("Max matrix: %d Loops \n",MaxLoops);
       PRINTF("Avg matrix: %d Loops \n",AvgLoops);
   
       PRINTF("\n");
       
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

void VSP_SOLVER::DoPreconditionedMatrixMultiply(VSPAERO_DOUBLE *vec_in, VSPAERO_DOUBLE *vec_out)
{

    if ( !AdjointSolve_ ) {

       DoMatrixMultiply(vec_in,vec_out);
       
       DoMatrixPrecondition(vec_out);
       
    }
    
    else {

       Optimization_DoAdjointMatrixMultiply(vec_in,vec_out);
 
    }

}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER DoMatrixMultiply                            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DoMatrixMultiply(VSPAERO_DOUBLE *vec_in, VSPAERO_DOUBLE *vec_out)
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

void VSP_SOLVER::MatrixMultiply(VSPAERO_DOUBLE *vec_in, VSPAERO_DOUBLE *vec_out)
{

    int i, j, k, v, Level, Loop, LoopType, MaxLoopTypes, NumberOfSheets, cpu;
    VSPAERO_DOUBLE xyz[3], q[4], Ws, U, V, W;
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

#pragma omp parallel for reduction(+:U,V,W) private(j,Level,Loop,xyz,q,VortexEdge) schedule(dynamic) if (DO_PARALLEL_LOOP)
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

    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
    
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
       
          VortexSheet(cpu,v).TurnWakeDampingOn();
          
       }    
    
    }   
          
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

    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
    
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
       
          VortexSheet(cpu,v).TurnWakeDampingOff();
          
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

void VSP_SOLVER::MatrixTransposeMultiply(VSPAERO_DOUBLE *vec_in, VSPAERO_DOUBLE *vec_out)
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

void VSP_SOLVER::DoMatrixPrecondition(VSPAERO_DOUBLE *vec_in)
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
       
       PRINTF("Unknown preconditioner! \n");fflush(NULL);
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
    VSPAERO_DOUBLE q[3], xyz[3], Ws, U, V, W;
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

#pragma omp parallel for reduction(+:U,V,W) private(j,Level,Loop,q,VortexEdge,xyz) schedule(dynamic) if (DO_PARALLEL_LOOP)          
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

    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
    
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
       
          VortexSheet(cpu,v).TurnWakeDampingOn();
          
       }    

    }    
       
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
    
    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
    
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
       
          VortexSheet(cpu,v).TurnWakeDampingOff();
          
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
#                       VSP_SOLVER CalculateEdgeVelocities                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateEdgeVelocities(void)
{

    int i, j, k, m, p, t, v, w, cpu, NumberOfSheets, Level;
    VSPAERO_DOUBLE xyz[3], xyz_te[3], q[5], U, V, W, Delta, MaxDelta, CoreWidth;
    VSPAERO_DOUBLE Rate_P, Rate_Q, Rate_R;
    VORTEX_SHEET_ENTRY *VortexSheetList;

    // Initialize to free stream values

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       SurfaceVortexEdge(j).U() = FreeStreamVelocity_[0] - LocalBodySurfaceVelocityForEdge_[j][0];
       SurfaceVortexEdge(j).V() = FreeStreamVelocity_[1] - LocalBodySurfaceVelocityForEdge_[j][1];
       SurfaceVortexEdge(j).W() = FreeStreamVelocity_[2] - LocalBodySurfaceVelocityForEdge_[j][2];
       
       SurfaceVortexEdge(j).DownWash_U() = 0.;
       SurfaceVortexEdge(j).DownWash_V() = 0.;
       SurfaceVortexEdge(j).DownWash_W() = 0.;
       
    }
    
    // Rotational rates... note these rates are wrt to the body stability
    // axes... so we have to convert them to equivalent freestream velocities...
    // in the VSPAERO axes system with has X and Z pointing in the opposite
    // directions
    
    Rate_P = RotationalRate_[0];
    Rate_Q = RotationalRate_[1];
    Rate_R = RotationalRate_[2];

    // Add in rotational velocities

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       xyz[0] = SurfaceVortexEdge(j).Xc() - Xcg();
       xyz[1] = SurfaceVortexEdge(j).Yc() - Ycg();       
       xyz[2] = SurfaceVortexEdge(j).Zc() - Zcg();
       
       // P - Roll
       
       SurfaceVortexEdge(j).U() += 0.;
       SurfaceVortexEdge(j).V() += -xyz[2] * Rate_P;
       SurfaceVortexEdge(j).W() += +xyz[1] * Rate_P;
        
       // Q - Pitch
       
       SurfaceVortexEdge(j).U() += -xyz[2] * Rate_Q;
       SurfaceVortexEdge(j).V() += 0.;
       SurfaceVortexEdge(j).W() += +xyz[0] * Rate_Q;
       
       // R - Yaw
       
       SurfaceVortexEdge(j).U() += -xyz[1] * Rate_R;
       SurfaceVortexEdge(j).V() += +xyz[0] * Rate_R;
       SurfaceVortexEdge(j).W() += 0.;

    }  
        
    // Add in the rotor induced velocities

    for ( k = 1 ; k <= NumberOfRotors_ ; k++ ) {

#pragma omp parallel for private(j,xyz,q)
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
     
          xyz[0] = SurfaceVortexEdge(j).Xc(); 
          xyz[1] = SurfaceVortexEdge(j).Yc();        
          xyz[2] = SurfaceVortexEdge(j).Zc(); 
       
          RotorDisk(k).Velocity(xyz, q);                   

          SurfaceVortexEdge(j).U() += q[0];
          SurfaceVortexEdge(j).V() += q[1];
          SurfaceVortexEdge(j).W() += q[2];
       
          // If there is ground effects, z plane...
          
          if ( DoGroundEffectsAnalysis() ) {
  
             xyz[0] = SurfaceVortexEdge(j).Xc();
             xyz[1] = SurfaceVortexEdge(j).Yc();       
             xyz[2] = SurfaceVortexEdge(j).Zc();
                  
             xyz[2] *= -1.;
            
             RotorDisk(k).Velocity(xyz, q);        
   
             q[2] *= -1.;
            
             SurfaceVortexEdge(j).U() += q[0];
             SurfaceVortexEdge(j).V() += q[1];
             SurfaceVortexEdge(j).W() += q[2];

             SurfaceVortexEdge(j).DownWash_U() += q[0];
             SurfaceVortexEdge(j).DownWash_V() += q[1];
             SurfaceVortexEdge(j).DownWash_W() += q[2];
                         
          }     
                          
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {
  
             xyz[0] = SurfaceVortexEdge(j).Xc();
             xyz[1] = SurfaceVortexEdge(j).Yc();      
             xyz[2] = SurfaceVortexEdge(j).Zc();
  
             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
            
             RotorDisk(k).Velocity(xyz, q);        
   
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
            
             SurfaceVortexEdge(j).U() += q[0];
             SurfaceVortexEdge(j).V() += q[1];
             SurfaceVortexEdge(j).W() += q[2];
             
             SurfaceVortexEdge(j).DownWash_U() += q[0];
             SurfaceVortexEdge(j).DownWash_V() += q[1];
             SurfaceVortexEdge(j).DownWash_W() += q[2];             
            
             // If there is ground effects, z plane...
             
             if ( DoGroundEffectsAnalysis() ) {

                xyz[2] *= -1.;
               
                RotorDisk(k).Velocity(xyz, q);        
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;            
                                                      q[2] *= -1.;
               
                SurfaceVortexEdge(j).U() += q[0];
                SurfaceVortexEdge(j).V() += q[1];
                SurfaceVortexEdge(j).W() += q[2];
                
                SurfaceVortexEdge(j).DownWash_U() += q[0];
                SurfaceVortexEdge(j).DownWash_V() += q[1];
                SurfaceVortexEdge(j).DownWash_W() += q[2];                
               
             }  
             
          }
 
       }
       
    }
         
    // Wing surface vortex induced velocities

#pragma omp parallel for private(j,xyz,q)
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
    
       xyz[0] = SurfaceVortexEdge(j).Xc(); 
       xyz[1] = SurfaceVortexEdge(j).Yc();        
       xyz[2] = SurfaceVortexEdge(j).Zc(); 
          
       CalculateSurfaceInducedVelocityAtPoint(xyz, q);

       SurfaceVortexEdge(j).U() += q[0];
       SurfaceVortexEdge(j).V() += q[1];
       SurfaceVortexEdge(j).W() += q[2];
       
       SurfaceVortexEdge(j).DownWash_U() += q[0];
       SurfaceVortexEdge(j).DownWash_V() += q[1];
       SurfaceVortexEdge(j).DownWash_W() += q[2];       
   
       // If there is ground effects, z plane ...
       
       if ( DoGroundEffectsAnalysis() ) {

          xyz[0] = SurfaceVortexEdge(j).Xc();
          xyz[1] = SurfaceVortexEdge(j).Yc();      
          xyz[2] = SurfaceVortexEdge(j).Zc();
               
          xyz[2] *= -1.;
         
          CalculateSurfaceInducedVelocityAtPoint(xyz, q);

          q[2] *= -1.;
         
          SurfaceVortexEdge(j).U() += q[0];
          SurfaceVortexEdge(j).V() += q[1];
          SurfaceVortexEdge(j).W() += q[2];
          
          SurfaceVortexEdge(j).DownWash_U() += q[0];
          SurfaceVortexEdge(j).DownWash_V() += q[1];
          SurfaceVortexEdge(j).DownWash_W() += q[2];                 
          
       }
                    
       // If there is a symmetry plane, calculate influence of the reflection
       
       if ( DoSymmetryPlaneSolve_ ) {

          xyz[0] = SurfaceVortexEdge(j).Xc();
          xyz[1] = SurfaceVortexEdge(j).Yc();       
          xyz[2] = SurfaceVortexEdge(j).Zc();
       
          if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
          if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
          if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
         
          CalculateSurfaceInducedVelocityAtPoint(xyz, q);

          if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
          if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
          if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
         
          SurfaceVortexEdge(j).U() += q[0];
          SurfaceVortexEdge(j).V() += q[1];
          SurfaceVortexEdge(j).W() += q[2];
          
          SurfaceVortexEdge(j).DownWash_U() += q[0];
          SurfaceVortexEdge(j).DownWash_V() += q[1];
          SurfaceVortexEdge(j).DownWash_W() += q[2];         
                    
          // If there is ground effects, z plane ...
          
          if ( DoGroundEffectsAnalysis() ) {

             xyz[2] *= -1.;
            
             CalculateSurfaceInducedVelocityAtPoint(xyz, q);
   
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;         
                                                   q[2] *= -1.;
            
             SurfaceVortexEdge(j).U() += q[0];
             SurfaceVortexEdge(j).V() += q[1];
             SurfaceVortexEdge(j).W() += q[2];
             
             SurfaceVortexEdge(j).DownWash_U() += q[0];
             SurfaceVortexEdge(j).DownWash_V() += q[1];
             SurfaceVortexEdge(j).DownWash_W() += q[2];         
                       
          }
                       
       }
       
    }

    // Copy over vortex sheet data for parallel runs
   
    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {

       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
          VortexSheet_[cpu][k] += VortexSheet_[0][k];
    
       }  
       
    }   

    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {

       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
          VortexSheet(cpu,k).TurnWakeDampingOn();
    
       }  
       
    }   
    
    // Wake vortex induced velocities

    cpu = 0;
    
    for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {

#pragma omp parallel for private(cpu, j, xyz, q, U, V, W)
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

#ifdef VSPAERO_OPENMP    
       cpu = omp_get_thread_num();
#else
       cpu = 0;
#endif  
       
          xyz[0] = SurfaceVortexEdge(j).Xc(); 
          xyz[1] = SurfaceVortexEdge(j).Yc();        
          xyz[2] = SurfaceVortexEdge(j).Zc();       
          
          VortexSheet(cpu,v).InducedVelocity(xyz, q);
          
          U = q[0];
          V = q[1];
          W = q[2];
          
          // If there is ground effects, z plane ...
          
          if ( DoGroundEffectsAnalysis() ) {
          
             xyz[0] = SurfaceVortexEdge(j).Xc(); 
             xyz[1] = SurfaceVortexEdge(j).Yc();        
             xyz[2] = SurfaceVortexEdge(j).Zc();               
          
             xyz[2] *= -1.;
          
             VortexSheet(cpu,v).InducedVelocity(xyz, q);
          
             q[2] *= -1.;
          
             U += q[0];
             V += q[1];
             W += q[2];
          
          }  
                            
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {
          
             xyz[0] = SurfaceVortexEdge(j).Xc();
             xyz[1] = SurfaceVortexEdge(j).Yc();
             xyz[2] = SurfaceVortexEdge(j).Zc();          
          
             if ( DoSymmetryPlaneSolve_ == SYM_X ) { xyz[0] *= -1.; };
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) { xyz[1] *= -1.; };
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) { xyz[2] *= -1.; };
          
             VortexSheet(cpu,v).InducedVelocity(xyz, q);
          
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
          
             U += q[0];
             V += q[1];
             W += q[2];
             
             // If there is ground effects, z plane ...
          
             if ( DoGroundEffectsAnalysis() ) {
          
                xyz[2] *= -1.; 
             
                VortexSheet(cpu,v).InducedVelocity(xyz, q);
           
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;                
                                                      q[2] *= -1.;
             
                U += q[0];
                V += q[1];
                W += q[2];
             
             }                        
          
          }        
          
          SurfaceVortexEdge(j).U() += U;
          SurfaceVortexEdge(j).V() += V;
          SurfaceVortexEdge(j).W() += W;      
          
          SurfaceVortexEdge(j).DownWash_U() += q[0];
          SurfaceVortexEdge(j).DownWash_V() += q[1];
          SurfaceVortexEdge(j).DownWash_W() += q[2];                                    

       }

    }
 
    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
    
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
       
          VortexSheet(cpu,v).TurnWakeDampingOff();
          
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
    VSPAERO_DOUBLE xyz[3], xyz_te[3], q[5], U, V, W, Delta, MaxDelta, CoreWidth;
    VSPAERO_DOUBLE Rate_P, Rate_Q, Rate_R;
    VORTEX_SHEET_ENTRY *VortexSheetList;

    // Initialize to free stream values

    for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {

       VortexSheet(m).ZeroEdgeVelocities();
 
       for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
        
          VortexSheet(m).TrailingVortex(i).FreeStreamVelocity(0) = FreeStreamVelocity_[0];
          VortexSheet(m).TrailingVortex(i).FreeStreamVelocity(1) = FreeStreamVelocity_[1];
          VortexSheet(m).TrailingVortex(i).FreeStreamVelocity(2) = FreeStreamVelocity_[2];
             
          for ( j = 1 ; j <= VortexSheet(m).TrailingVortex(i).NumberOfSubVortices() ; j++ ) {
      
             VortexSheet(m).TrailingVortex(i).U(j) = FreeStreamVelocity_[0];
             VortexSheet(m).TrailingVortex(i).V(j) = FreeStreamVelocity_[1];
             VortexSheet(m).TrailingVortex(i).W(j) = FreeStreamVelocity_[2];
             
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
         
          for ( j = 1 ; j <= VortexSheet(m).TrailingVortex(i).NumberOfSubVortices() ; j++ ) {

             xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0] - Xcg();
             xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1] - Ycg();       
             xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2] - Zcg();

             // P - Roll
             
             VortexSheet(m).TrailingVortex(i).U(j) += 0.;
             VortexSheet(m).TrailingVortex(i).V(j) += -xyz[2] * Rate_P;
             VortexSheet(m).TrailingVortex(i).W(j) += +xyz[1] * Rate_P;
              
             // Q - Pitch
      
             VortexSheet(m).TrailingVortex(i).U(j) += -xyz[2] * Rate_Q;
             VortexSheet(m).TrailingVortex(i).V(j) += 0.;
             VortexSheet(m).TrailingVortex(i).W(j) += +xyz[0] * Rate_Q;
      
             // R - Yaw
             
             VortexSheet(m).TrailingVortex(i).U(j) += -xyz[1] * Rate_R;
             VortexSheet(m).TrailingVortex(i).V(j) += +xyz[0] * Rate_R;
             VortexSheet(m).TrailingVortex(i).W(j) += 0.;
             
          }
          
       }
    
    }  
        
    // Add in the rotor induced velocities

    for ( k = 1 ; k <= NumberOfRotors_ ; k++ ) {
     
       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     

          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
            
             for ( j = 1 ; j <= VortexSheet(m).TrailingVortex(i).NumberOfSubVortices() ; j++ ) {
   
                xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0]; 
                xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1];        
                xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2]; 
             
                RotorDisk(k).Velocity(xyz, q);                   
   
                VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                VortexSheet(m).TrailingVortex(i).W(j) += q[2];
             
                // If there is ground effects, z plane...
                
                if ( DoGroundEffectsAnalysis() ) {
        
                   xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0]; 
                   xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1];        
                   xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2]; 
                        
                   xyz[2] *= -1.;
                  
                   RotorDisk(k).Velocity(xyz, q);        
         
                   q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                   VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                   VortexSheet(m).TrailingVortex(i).W(j) += q[2];
                  
                }     
                                
                // If there is a symmetry plane, calculate influence of the reflection
                
                if ( DoSymmetryPlaneSolve_ ) {
        
                   xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0]; 
                   xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1];        
                   xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2]; 
        
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
                  
                   RotorDisk(k).Velocity(xyz, q);        
         
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                   VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                   VortexSheet(m).TrailingVortex(i).W(j) += q[2];
                  
                   // If there is ground effects, z plane...
                   
                   if ( DoGroundEffectsAnalysis() ) {
   
                      xyz[2] *= -1.;
                     
                      RotorDisk(k).Velocity(xyz, q);        
            
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;            
                                                            q[2] *= -1.;
                     
                      VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                      VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                      VortexSheet(m).TrailingVortex(i).W(j) += q[2];
                     
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
                         
          for ( j = 1 ; j <= VortexSheet(m).TrailingVortex(i).NumberOfSubVortices() ; j++ ) {
             
             xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0]; 
             xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1];        
             xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2]; 
                
             CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
   
             VortexSheet(m).TrailingVortex(i).U(j) += q[0];
             VortexSheet(m).TrailingVortex(i).V(j) += q[1];
             VortexSheet(m).TrailingVortex(i).W(j) += q[2];
         
             // If there is ground effects, z plane ...
             
             if ( DoGroundEffectsAnalysis() ) {
     
                xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0]; 
                xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1];        
                xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2]; 
                     
                xyz[2] *= -1.;
               
                CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
      
                q[2] *= -1.;
               
                VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                VortexSheet(m).TrailingVortex(i).W(j) += q[2];
                
             }
                          
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {
     
                xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0]; 
                xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1];        
                xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2]; 
             
                if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
               
                CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
               
                VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                VortexSheet(m).TrailingVortex(i).W(j) += q[2];
                
                // If there is ground effects, z plane ...
                
                if ( DoGroundEffectsAnalysis() ) {
 
                   xyz[2] *= -1.;
                  
                   CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
         
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;         
                                                         q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                   VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                   VortexSheet(m).TrailingVortex(i).W(j) += q[2];
                   
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

    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {

       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
          VortexSheet(cpu,k).TurnWakeDampingOn();
    
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
 
             xyz[0] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[0];
             xyz[1] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[1];
             xyz[2] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[2];                

             xyz_te[0] = VortexSheet(w).TrailingVortex(t).TE_Node().x();
             xyz_te[1] = VortexSheet(w).TrailingVortex(t).TE_Node().y();
             xyz_te[2] = VortexSheet(w).TrailingVortex(t).TE_Node().z();

             VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, xyz_te, q);
             
             U = q[0];
             V = q[1];
             W = q[2];

             // If there is ground effects, z plane ...
   
             if ( DoGroundEffectsAnalysis() ) {

                xyz[0] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[0];
                xyz[1] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[1];
                xyz[2] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[2];                

                xyz_te[0] = VortexSheet(w).TrailingVortex(t).TE_Node().x();
                xyz_te[1] = VortexSheet(w).TrailingVortex(t).TE_Node().y();
                xyz_te[2] = VortexSheet(w).TrailingVortex(t).TE_Node().z();
                 
                xyz[2] *= -1.; xyz_te[2] *= -1.;
               
                VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, xyz_te, q);
       
                q[2] *= -1.;
               
                U += q[0];
                V += q[1];
                W += q[2];
               
             }  
                                
             // If there is a symmetry plane, calculate influence of the reflection
   
             if ( DoSymmetryPlaneSolve_ ) {

                xyz[0] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[0];
                xyz[1] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[1];
                xyz[2] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[2];                

                xyz_te[0] = VortexSheet(w).TrailingVortex(t).TE_Node().x();
                xyz_te[1] = VortexSheet(w).TrailingVortex(t).TE_Node().y();
                xyz_te[2] = VortexSheet(w).TrailingVortex(t).TE_Node().z();
                 
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
             
             VortexSheet(w).TrailingVortex(t).U(Level,j) += U;
             VortexSheet(w).TrailingVortex(t).V(Level,j) += V;
             VortexSheet(w).TrailingVortex(t).W(Level,j) += W;                           
 
          }
          
       }

    }

    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {

       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
          VortexSheet(cpu,k).TurnWakeDampingOff();
    
       }  
       
    }   
    
    for ( w = 1 ; w <= NumberOfVortexSheets_ ; w++ ) {
      
       VortexSheet(w).ProlongateEdgeVelocities();
       
    }

    // Force last segment to free stream conditions
            
    for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     
           
       for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {

          j = VortexSheet(m).TrailingVortex(i).NumberOfSubVortices() + 1;
   
          VortexSheet(m).TrailingVortex(i).U(j) = FreeStreamVelocity_[0];
          VortexSheet(m).TrailingVortex(i).V(j) = FreeStreamVelocity_[1];
          VortexSheet(m).TrailingVortex(i).W(j) = FreeStreamVelocity_[2];
          
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
             
             for ( p = 1 ; p <= VortexSheet(v).TrailingVortex(i).NumberOfSubVortices() ; p++ ) {
                
                VortexSheet(v).TrailingVortex(i).U(p) += VortexSheet(w).TrailingVortex(j).U(p);
                VortexSheet(v).TrailingVortex(i).V(p) += VortexSheet(w).TrailingVortex(j).V(p);
                VortexSheet(v).TrailingVortex(i).W(p) += VortexSheet(w).TrailingVortex(j).W(p);
                
             }
             
          }
          
       }
       
       // Average velocities
       
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
          
          for ( k = 1 ; k <= VortexSheet(v).NumberOfCommonTE() ; k++ ) {
             
             w = VortexSheet(v).CommonTEList(k).Sheet_j();
             
             i = VortexSheet(v).CommonTEList(k).TEVortex_i();
             j = VortexSheet(v).CommonTEList(k).TEVortex_j();
             
             for ( p = 1 ; p <= VortexSheet(v).TrailingVortex(i).NumberOfSubVortices() ; p++ ) {
                
                VortexSheet(v).TrailingVortex(i).U(p) /= ( VortexSheet(v).NumberOfCommonNodesForTE(i) + 1);
                VortexSheet(v).TrailingVortex(i).V(p) /= ( VortexSheet(v).NumberOfCommonNodesForTE(i) + 1);
                VortexSheet(v).TrailingVortex(i).W(p) /= ( VortexSheet(v).NumberOfCommonNodesForTE(i) + 1);
                
             }
             
          }
          
       }       
  
       // Distribute velocities
       
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
          
          for ( k = 1 ; k <= VortexSheet(v).NumberOfCommonTE() ; k++ ) {
             
             w = VortexSheet(v).CommonTEList(k).Sheet_j();
             
             i = VortexSheet(v).CommonTEList(k).TEVortex_i();
             j = VortexSheet(v).CommonTEList(k).TEVortex_j();
             
             for ( p = 1 ; p <= VortexSheet(v).TrailingVortex(i).NumberOfSubVortices() ; p++ ) {
                
                VortexSheet(w).TrailingVortex(j).U(p) = VortexSheet(v).TrailingVortex(i).U(p);
                VortexSheet(w).TrailingVortex(j).V(p) = VortexSheet(v).TrailingVortex(i).V(p);
                VortexSheet(w).TrailingVortex(j).W(p) = VortexSheet(v).TrailingVortex(i).W(p);
                 
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

    if ( Verbose_ ) PRINTF("MaxDelta: %f \n",log10(MaxDelta)); 
     
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
    VSPAERO_DOUBLE xyz[3], q[5], U, V, W;
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

    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
    
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
       
          VortexSheet(cpu,v).TurnWakeDampingOn();
          
       }    

    }    
    
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
           
             xyz[0] = VortexLoop(Loop).xyz_c()[0];
             xyz[1] = VortexLoop(Loop).xyz_c()[1];
             xyz[2] = VortexLoop(Loop).xyz_c()[2];
             
             xyz[2] *= -1.;
        
             VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, q);
             
             q[2] *= -1.;
             
             U += q[0];
             V += q[1];
             W += q[2];
             
          }   
          
          // If there is a symmetry plane, calculate influence of the reflection
        
          if ( DoSymmetryPlaneSolve_ ) {
           
             xyz[0] = VortexLoop(Loop).xyz_c()[0];
             xyz[1] = VortexLoop(Loop).xyz_c()[1];
             xyz[2] = VortexLoop(Loop).xyz_c()[2];
             
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
    
    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
    
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
       
          VortexSheet(cpu,v).TurnWakeDampingOff();
          
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
    VSPAERO_DOUBLE Hdot, RotVec[3];
    QUAT Quat, InvQuat, Vec1, Vec2;

    // Unsteady heaving analysis
    
    if ( TimeAnalysisType_ == HEAVE_ANALYSIS ) {
    
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
        
          Unsteady_H_ = Unsteady_HMax_ * sin ( Unsteady_AngleRate_*CurrentTime_ );     
          
          Hdot = Unsteady_HMax_ * cos ( Unsteady_AngleRate_*CurrentTime_) * Unsteady_AngleRate_;   
 
          // Calculate body velocities
          
          LocalBodySurfaceVelocityForLoop_[i][0] = 0.;
          LocalBodySurfaceVelocityForLoop_[i][1] = 0.;
          LocalBodySurfaceVelocityForLoop_[i][2] = Hdot;
        
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
PRINTF("AngleOfAttackZero_: %f \n",AngleOfAttackZero_);          
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
          
          LocalBodySurfaceVelocityForLoop_[i][0] = ( Vec2(0) - Vec1(0) ) / TimeStep_;
          LocalBodySurfaceVelocityForLoop_[i][1] = ( Vec2(1) - Vec1(1) ) / TimeStep_;
          LocalBodySurfaceVelocityForLoop_[i][2] = ( Vec2(2) - Vec1(2) ) / TimeStep_;
                    
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

    int i, j, k, m, c, p, v, w, t, NumberOfSheets, jMax, Level;
    int *ComponentInThisGroup, cpu, Node, Found;
    VSPAERO_DOUBLE OVec[3], TVec[3], RVec[3], CoreWidth;
    VSPAERO_DOUBLE xyz[3], xyz_te[3], q[5], U, V, W;
    VSPAERO_DOUBLE TimeStep, CurrentTime;
    QUAT Quat, InvQuat, Vec1, Vec2, DQuatDt, Omega, BodyVelocity, WQuat;
    VORTEX_SHEET_ENTRY *VortexSheetList;

    // If a full run, calculate velocities on trailing wakes
    
    if ( !NoiseAnalysis_ && !DoStartUp ) {

       // Initialize to free stream values
   
       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {
   
          VortexSheet(m).ZeroEdgeVelocities();

          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
           
             VortexSheet(m).TrailingVortex(i).FreeStreamVelocity(0) = FreeStreamVelocity_[0];
             VortexSheet(m).TrailingVortex(i).FreeStreamVelocity(1) = FreeStreamVelocity_[1];
             VortexSheet(m).TrailingVortex(i).FreeStreamVelocity(2) = FreeStreamVelocity_[2];
                
             for ( j = 1 ; j <= VortexSheet(m).TrailingVortex(i).NumberOfSubVortices() ; j++ ) {
         
                VortexSheet(m).TrailingVortex(i).U(j) = FreeStreamVelocity_[0];
                VortexSheet(m).TrailingVortex(i).V(j) = FreeStreamVelocity_[1];
                VortexSheet(m).TrailingVortex(i).W(j) = FreeStreamVelocity_[2];
                
             }
      
          }
          
       }
       
       // Add in the rotor induced velocities
    
       for ( k = 1 ; k <= NumberOfRotors_ ; k++ ) {
        
          for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     
   
             for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
   
                jMax = VortexSheet(m).TrailingVortex(i).NumberOfSubVortices();
       
                if ( TimeAccurate_ ) {
                   
                   jMax = MIN(VortexSheet(m).TrailingVortex(i).NumberOfSubVortices(), WakeStartingTime_ + Time_ + 1);
      
                }
             
                for ( j = 1 ; j <= jMax ; j++ ) {
   
                   xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0]; 
                   xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1];        
                   xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2]; 
                
                   RotorDisk(k).Velocity(xyz, q);                   
      
                   VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                   VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                   VortexSheet(m).TrailingVortex(i).W(j) += q[2];
            
                   // If there is ground effects, z plane...
                   
                   if ( DoGroundEffectsAnalysis() ) {
           
                      xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0]; 
                      xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1];        
                      xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2]; 
                           
                      xyz[2] *= -1.;
                     
                      RotorDisk(k).Velocity(xyz, q);        
            
                      q[2] *= -1.;
                     
                      VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                      VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                      VortexSheet(m).TrailingVortex(i).W(j) += q[2];
                     
                   }     
                                   
                   // If there is a symmetry plane, calculate influence of the reflection
                   
                   if ( DoSymmetryPlaneSolve_ ) {
           
                      xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0]; 
                      xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1];        
                      xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2]; 
           
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
                     
                      RotorDisk(k).Velocity(xyz, q);        
            
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
                     
                      VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                      VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                      VortexSheet(m).TrailingVortex(i).W(j) += q[2];
                     
                      // If there is ground effects, z plane...
                      
                      if ( DoGroundEffectsAnalysis() ) {
      
                         xyz[2] *= -1.;
                        
                         RotorDisk(k).Velocity(xyz, q);        
               
                         if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                         if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;            
                                                               q[2] *= -1.;
                        
                         VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                         VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                         VortexSheet(m).TrailingVortex(i).W(j) += q[2];
                        
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
             
             jMax = VortexSheet(m).TrailingVortex(i).NumberOfSubVortices();
    
             if ( TimeAccurate_ ) {
                
                jMax = MIN(VortexSheet(m).TrailingVortex(i).NumberOfSubVortices(), WakeStartingTime_ + Time_ + 1);
   
             }
   
             for ( j = 1 ; j <= jMax ; j++ ) {
          
                xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0]; 
                xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1];        
                xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2]; 

                CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
      
                VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                VortexSheet(m).TrailingVortex(i).W(j) += q[2];
            
                // If there is ground effects, z plane ...
                
                if ( DoGroundEffectsAnalysis() ) {
        
                   xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0]; 
                   xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1];        
                   xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2]; 
                        
                   xyz[2] *= -1.;
                  
                   CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
         
                   q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                   VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                   VortexSheet(m).TrailingVortex(i).W(j) += q[2];
                   
                }
                             
                // If there is a symmetry plane, calculate influence of the reflection
                
                if ( DoSymmetryPlaneSolve_ ) {
        
                   xyz[0] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[0]; 
                   xyz[1] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[1];        
                   xyz[2] = VortexSheet(m).TrailingVortex(i).xyz_c(j)[2]; 
                
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
                  
                   CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
         
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                   VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                   VortexSheet(m).TrailingVortex(i).W(j) += q[2];
                   
                   // If there is ground effects, z plane ...
                   
                   if ( DoGroundEffectsAnalysis() ) {
    
                      xyz[2] *= -1.;
                     
                      CalculateSurfaceInducedVelocityAtPoint(xyz, q, CoreWidth);
            
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;         
                                                            q[2] *= -1.;
                     
                      VortexSheet(m).TrailingVortex(i).U(j) += q[0];
                      VortexSheet(m).TrailingVortex(i).V(j) += q[1];
                      VortexSheet(m).TrailingVortex(i).W(j) += q[2];
                      
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
                
          for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
          
             for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
             
                VortexSheet(cpu,v).TurnWakeDampingOn();
                
             }    
         
          }    
                 
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
                   
                xyz_te[0] = VortexSheet(w).TrailingVortex(t).TE_Node().x();
                xyz_te[1] = VortexSheet(w).TrailingVortex(t).TE_Node().y();
                xyz_te[2] = VortexSheet(w).TrailingVortex(t).TE_Node().z();
                   
                for ( i = 1 ; i <= VortexSheetVortexToVortexSet_[v].NumberOfVortexSheetInteractionEdges(p) ; i++ ) {
   
                   Level           = VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p)[i].Level();
                   
                   j               = VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p)[i].Edge();
                   
                   NumberOfSheets  = VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p)[i].NumberOfVortexSheets();
                   
                   VortexSheetList = VortexSheetVortexToVortexSet_[v].VortexSheetInteractionTrailingVortexList(p)[i].VortexSheetList_;
                   
                   xyz[0] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[0];
                   xyz[1] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[1];
                   xyz[2] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[2];                
      
                   VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, xyz_te, q);
                   
                   U = q[0];
                   V = q[1];
                   W = q[2];
   
                   // If there is ground effects, z plane ...
         
                   if ( DoGroundEffectsAnalysis() ) {
      
                      xyz[0] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[0];
                      xyz[1] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[1];
                      xyz[2] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[2];                
      
                      xyz_te[0] = VortexSheet(w).TrailingVortex(t).TE_Node().x();
                      xyz_te[1] = VortexSheet(w).TrailingVortex(t).TE_Node().y();
                      xyz_te[2] = VortexSheet(w).TrailingVortex(t).TE_Node().z();
                       
                      xyz[2] *= -1.; xyz_te[2] *= -1.;
                     
                      VortexSheet(cpu,v).InducedVelocity(NumberOfSheets, VortexSheetList, xyz, xyz_te, q);
             
                      q[2] *= -1.;
                     
                      U += q[0];
                      V += q[1];
                      W += q[2];
                     
                   }  
                                      
                   // If there is a symmetry plane, calculate influence of the reflection
         
                   if ( DoSymmetryPlaneSolve_ ) {
      
                      xyz[0] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[0];
                      xyz[1] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[1];
                      xyz[2] = VortexSheet(w).TrailingVortex(t).xyz_c(j)[2];                
      
                      xyz_te[0] = VortexSheet(w).TrailingVortex(t).TE_Node().x();
                      xyz_te[1] = VortexSheet(w).TrailingVortex(t).TE_Node().y();
                      xyz_te[2] = VortexSheet(w).TrailingVortex(t).TE_Node().z();
                       
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
      
                   VortexSheet(w).TrailingVortex(t).U(Level,j) += U;
                   VortexSheet(w).TrailingVortex(t).V(Level,j) += V;
                   VortexSheet(w).TrailingVortex(t).W(Level,j) += W;
          
                }
                
             }
   
          }
          
          for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
          
             for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
             
                VortexSheet(cpu,v).TurnWakeDampingOff();
                
             }    
         
          }        
          
       }

       for ( w = 1 ; w <= NumberOfVortexSheets_ ; w++ ) {
       
          VortexSheet(w).ProlongateEdgeVelocities();
        
       }

       // Force last segments to free stream conditions
   
       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     
              
          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
   
             jMax = VortexSheet(m).TrailingVortex(i).NumberOfSubVortices();
    
             if ( TimeAccurate_ ) {
                
                jMax = MIN(VortexSheet(m).TrailingVortex(i).NumberOfSubVortices(), WakeStartingTime_ + Time_ + 2);
   
             }
             
             for ( j = jMax ; j <= VortexSheet(m).TrailingVortex(i).NumberOfSubVortices() ; j++ ) {
         
                VortexSheet(m).TrailingVortex(i).U(j) = FreeStreamVelocity_[0];
                VortexSheet(m).TrailingVortex(i).V(j) = FreeStreamVelocity_[1];
                VortexSheet(m).TrailingVortex(i).W(j) = FreeStreamVelocity_[2];
                
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
             
             for ( p = 1 ; p <= VortexSheet(v).TrailingVortex(i).NumberOfSubVortices() ; p++ ) {
                
                VortexSheet(v).TrailingVortex(i).U(p) += VortexSheet(w).TrailingVortex(j).U(p);
                VortexSheet(v).TrailingVortex(i).V(p) += VortexSheet(w).TrailingVortex(j).V(p);
                VortexSheet(v).TrailingVortex(i).W(p) += VortexSheet(w).TrailingVortex(j).W(p);
                
             }
             
          }
          
       }
       
       // Average velocities
       
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
          
          for ( k = 1 ; k <= VortexSheet(v).NumberOfCommonTE() ; k++ ) {
             
             w = VortexSheet(v).CommonTEList(k).Sheet_j();
             
             i = VortexSheet(v).CommonTEList(k).TEVortex_i();
             j = VortexSheet(v).CommonTEList(k).TEVortex_j();
             
             for ( p = 1 ; p <= VortexSheet(v).TrailingVortex(i).NumberOfSubVortices() ; p++ ) {
                
                VortexSheet(v).TrailingVortex(i).U(p) /= ( VortexSheet(v).NumberOfCommonNodesForTE(i) + 1);
                VortexSheet(v).TrailingVortex(i).V(p) /= ( VortexSheet(v).NumberOfCommonNodesForTE(i) + 1);
                VortexSheet(v).TrailingVortex(i).W(p) /= ( VortexSheet(v).NumberOfCommonNodesForTE(i) + 1);
                
             }
             
          }
          
       }       
  
       // Distribute velocities
       
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {
          
          for ( k = 1 ; k <= VortexSheet(v).NumberOfCommonTE() ; k++ ) {
             
             w = VortexSheet(v).CommonTEList(k).Sheet_j();
             
             i = VortexSheet(v).CommonTEList(k).TEVortex_i();
             j = VortexSheet(v).CommonTEList(k).TEVortex_j();
             
             for ( p = 1 ; p <= VortexSheet(v).TrailingVortex(i).NumberOfSubVortices() ; p++ ) {
                
                VortexSheet(w).TrailingVortex(j).U(p) = VortexSheet(v).TrailingVortex(i).U(p);
                VortexSheet(w).TrailingVortex(j).V(p) = VortexSheet(v).TrailingVortex(i).V(p);
                VortexSheet(w).TrailingVortex(j).W(p) = VortexSheet(v).TrailingVortex(i).W(p);
                 
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

                LocalBodySurfaceVelocityForLoop_[i][0] = BodyVelocity(0) + ComponentGroupList_[c].Velocity(0);
                LocalBodySurfaceVelocityForLoop_[i][1] = BodyVelocity(1) + ComponentGroupList_[c].Velocity(1);
                LocalBodySurfaceVelocityForLoop_[i][2] = BodyVelocity(2) + ComponentGroupList_[c].Velocity(2);
             
             }
      
          }

          // Calculate surface velocity for edge centroids
          
          for ( i = 1 ; i <= NumberOfSurfaceVortexEdges_ ; i++ ) {
             
             if ( ComponentInThisGroup[SurfaceVortexEdge(i).ComponentID()] ) {
 
                Vec1(0) = SurfaceVortexEdge(i).Xc() - OVec[0];
                Vec1(1) = SurfaceVortexEdge(i).Yc() - OVec[1];
                Vec1(2) = SurfaceVortexEdge(i).Zc() - OVec[2];
            
                // Body point location after rotation
                
                Vec2 = Quat * Vec1 * InvQuat;
         
                // Body point velocity
                
                BodyVelocity = WQuat * Vec2;
           
                LocalBodySurfaceVelocityForEdge_[i][0] = BodyVelocity(0) + ComponentGroupList_[c].Velocity(0);
                LocalBodySurfaceVelocityForEdge_[i][1] = BodyVelocity(1) + ComponentGroupList_[c].Velocity(1);
                LocalBodySurfaceVelocityForEdge_[i][2] = BodyVelocity(2) + ComponentGroupList_[c].Velocity(2);
             
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
             
             // Update the Span Loading data
             
             if ( ModelType_ == VLM_MODEL ) {
             
                for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
        
                   if ( ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()] ) SpanLoadData(i).UpdateGeometryLocation(TVec,OVec,Quat,InvQuat);
        
                }    
               
             }
             
             else {
                
                for ( i = 1 ; i <= NumberOfSpanLoadDataSets_ ; i++ ) {
                
                   Found = 0;
                   
                   j = 1;
                   
                   while ( j <= SpanLoadData(i).NumberOfSpanStations() && !Found ) {
                         
                      Node = VortexSheet(i).TrailingVortex(j).Node();
                      
                      if ( ComponentInThisGroup[VSPGeom().Grid(1).NodeList(Node).ComponentID()] ) Found = 1;
                     
                      j++;
                      
                   }
                   
                   if ( Found ) SpanLoadData(i).UpdateGeometryLocation(TVec,OVec,Quat,InvQuat);
        
                }    
                                
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

    int i, j, c, n, Level, *ComponentInThisGroup, Found, Node;
    VSPAERO_DOUBLE OVec[3], TVec[3], RVec[3];
    VSPAERO_DOUBLE TimeStep, CurrentTime;
    QUAT Quat, InvQuat, Vec1, Vec2, DQuatDt, Omega, BodyVelocity, WQuat;

    if ( !NoiseAnalysis_ ) {
       
       PRINTF("The ResetGeometry routine only really works for noise analyses! \n");
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
         
             // Calculate surface velocity for loop centroids
             
             for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
                
                if ( ComponentInThisGroup[VortexLoop(i).ComponentID()] ) {
    
                   Vec1(0) = VortexLoop(i).Xc() - OVec[0];
                   Vec1(1) = VortexLoop(i).Yc() - OVec[1];
                   Vec1(2) = VortexLoop(i).Zc() - OVec[2];
               
                   // Body point location after rotation
                   
                   Vec2 = Quat * Vec1 * InvQuat;
            
                   // Body point velocity
                   
                   BodyVelocity = WQuat * Vec2;
              
                   LocalBodySurfaceVelocityForLoop_[i][0] = BodyVelocity(0) + ComponentGroupList_[c].Velocity(0);
                   LocalBodySurfaceVelocityForLoop_[i][1] = BodyVelocity(1) + ComponentGroupList_[c].Velocity(1);
                   LocalBodySurfaceVelocityForLoop_[i][2] = BodyVelocity(2) + ComponentGroupList_[c].Velocity(2);
                
                }
         
             }

             // Calculate surface velocity for edge centroids
             
             for ( i = 1 ; i <= NumberOfSurfaceVortexEdges_ ; i++ ) {
                
                if ( ComponentInThisGroup[SurfaceVortexEdge(i).ComponentID()] ) {
    
                   Vec1(0) = SurfaceVortexEdge(i).Xc() - OVec[0];
                   Vec1(1) = SurfaceVortexEdge(i).Yc() - OVec[1];
                   Vec1(2) = SurfaceVortexEdge(i).Zc() - OVec[2];
               
                   // Body point location after rotation
                   
                   Vec2 = Quat * Vec1 * InvQuat;
            
                   // Body point velocity
                   
                   BodyVelocity = WQuat * Vec2;
              
                   LocalBodySurfaceVelocityForEdge_[i][0] = BodyVelocity(0) + ComponentGroupList_[c].Velocity(0);
                   LocalBodySurfaceVelocityForEdge_[i][1] = BodyVelocity(1) + ComponentGroupList_[c].Velocity(1);
                   LocalBodySurfaceVelocityForEdge_[i][2] = BodyVelocity(2) + ComponentGroupList_[c].Velocity(2);
                
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

             // Update the Span Loading data
             
             if ( ModelType_ == VLM_MODEL ) {
             
                for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
        
                   if ( ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()] ) SpanLoadData(i).UpdateGeometryLocation(TVec,OVec,Quat,InvQuat);
        
                }    
                
             }
             
             else {
                
                for ( i = 1 ; i <= NumberOfSpanLoadDataSets_ ; i++ ) {
                
                   Found = 0;
                   
                   j = 1;
                   
                   while ( j <= SpanLoadData(i).NumberOfSpanStations() && !Found ) {
                         
                      Node = VortexSheet(i).TrailingVortex(j).Node();
                      
                      if ( ComponentInThisGroup[VSPGeom().Grid(1).NodeList(Node).ComponentID()] ) Found = 1;
                     
                      j++;
                      
                   }
                   
                   if ( Found ) SpanLoadData(i).UpdateGeometryLocation(TVec,OVec,Quat,InvQuat);
        
                }    
                                
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
    VSPAERO_DOUBLE ResMax, ResRed, ResFin;

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

    if ( GMRESTightConvergence_ ) {

       ResMax = 0.1*Vref_;
       ResRed = 0.001;       
       
    }
 
    // Use preconditioned GMRES to solve the linear system
    
    AdjointSolve_ = 0;
     
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
                 
    AdjointSolve_ = 0;                 

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
       
       VSPAERO_DOUBLE KelvinSum = 0.;
   
       for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
          KelvinSum += Gamma(i);
   
       }
       
       PRINTF("\n\nKelvin Sum: %f \n",KelvinSum);
       
    }
    
    if ( Verbose_) PRINTF("log10(ABS(L2Residual_)): %lf \n",L2Residual_);

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER Optimization_AdjointSolve                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Optimization_AdjointSolve(void)
{

    int i, d_size;    
    char PsiFileName[2000];
    FILE *PsiFile;

    d_size = sizeof(double);
    
    // Calculate dF_dGamma
    
    Optimization_Calculate_pF_pGamma();
       
    // Reverse mode for (dR_dGamma)^T + GMRES to solve for Psi

    Optimization_GMRES_AdjointSolve();
    
    // Write out Psi for later use
    
    SPRINTF(PsiFileName,"%s.psi",FileName_);
    
    if ( (PsiFile = fopen(PsiFileName, "wb")) == NULL ) {
    
       PRINTF("Could not open the Psi file for output! \n");
    
       exit(1);
    
    }
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       FWRITE(&(Psi_[i]), d_size, 1, PsiFile);
       
    }
    
    fclose(PsiFile);

}

/*##############################################################################
#                                                                              #
#         VSP_SOLVER Optimization_CalculateGradientOfDesignFunction            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Optimization_CalculateGradientOfDesignFunction(void)
{

    int i, d_size;    
    char PsiFileName[2000];
    FILE *PsiFile;

    d_size = sizeof(double);

    // Read in Psi from previous adjoint solve
    
    SPRINTF(PsiFileName,"%s.psi",FileName_);
    
    if ( (PsiFile = fopen(PsiFileName, "rb")) == NULL ) {
    
       PRINTF("Could not open the Psi file for input! \n");
    
       exit(1);
    
    }

    Psi_ = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       FREAD(&(Psi_[i]), d_size, 1, PsiFile);
       
    }
    
    fclose(PsiFile);

    // Calculate pR_pMesh
    
    Optimization_Calculate_pR_pMesh();
        
    // Calculate pF_pMesh
    
    Optimization_Calculate_pF_pMesh();

    // Calculate final gradient
    
    Optimization_Calculate_Total_Gradient();
    
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER Optimization_Solve                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Optimization_Solve(int Case)
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
             
        CL_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
        CD_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
        CS_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];    
       CFx_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CFy_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CFz_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CMx_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CMy_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];
       CMz_Unsteady_ = new VSPAERO_DOUBLE[NumberOfTimeSteps_ + 1];       
 
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

    SPRINTF(StatusFileName,"%s.optimization.history",FileName_);
    
    if ( (StatusFile_ = fopen(StatusFileName, "w")) == NULL ) {
   
       PRINTF("Could not open the history file for output! \n");
   
       exit(1);
   
    }    

    // Write out generic header
    
    WriteCaseHeader(StatusFile_);

    // Status update to user
    
    FPRINTF(StatusFile_,"\n\nSolver Case: %d \n\n",ABS(Case));


                       //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
    FPRINTF(StatusFile_,"  Time      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz   CDTrefftz     T/QS    \n");

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
       
       SPRINTF(GroupFileName,"%s.optimization.group.%d",FileName_,c);
 
       if ( (GroupFile_[c] = fopen(GroupFileName, "w")) == NULL ) {
   
          PRINTF("Could not open the %s group coefficient file! \n",GroupFileName);
   
          exit(1);
   
       }

                            //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890    
       FPRINTF(GroupFile_[c],"Force Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
       FPRINTF(GroupFile_[c],"   Time        Cx         Cy         Cz        Cxo        Cyo        Czo        Cxi        Cyi        Czi        Cmx        Cmy        Cmz        Cmxo       Cmyo       Cmzo       Cmxi       Cmyi       Cmzi        CL         CD         CS        CLo        CDo        CLi        CDi \n");
                          
       // Create rotor file
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          k++;
          
          SPRINTF(RotorFileName,"%s.optimization.rotor.%d",FileName_,k);
    
          if ( (RotorFile_[k] = fopen(RotorFileName, "w")) == NULL ) {
      
             PRINTF("Could not open the %s rotor coefficient file! \n",RotorFileName);
      
             exit(1);
      
          }

                               //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890
          FPRINTF(RotorFile_[k],"Rotor Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
          FPRINTF(RotorFile_[k],"   Time     Diameter     RPM       Thrust     Thrusto    Thrusti    Moment     Momento    Momenti      J          CT         CQ        EtaP       Angle \n");
                             
       }

    }     
  
    // Open the input adb file
    
    if ( Case == 0 || Case == 1 ) {

       SPRINTF(ADBFileName,"%s.adb",FileName_);
       
       if ( (InputADBFile_ = fopen(ADBFileName, "rb")) == NULL ) {
   
          PRINTF("Could not open the aerothermal data base file for binary input! \n");
   
          exit(1);
   
       }

       // Read in the header
       
       ReadInAerothermalDatabaseHeader();

    }
    
    // Open the output adb file
    
    if ( Case == 0 || Case == 1 ) {

       SPRINTF(ADBFileName,"%s.optimization.adb",FileName_);
       
       if ( (ADBFile_ = fopen(ADBFileName, "wb")) == NULL ) {
   
          PRINTF("Could not open the aerothermal data base file for binary output! \n");
   
          exit(1);
   
       }

       WriteOutAerothermalDatabaseHeader();

    }

    // Open the case file
    
    SPRINTF(ADBFileName,"%s.optimization.adb.cases",FileName_);
    
    if ( (ADBCaseListFile_ = fopen(ADBFileName, "w")) == NULL ) {

       PRINTF("Could not open the aerothermal data base case list file for output! \n");

       exit(1);

    }       

#ifdef AUTODIFF

    // Turn off auto diff recording

    adept::Stack *AutoDiffStack;
    
    AutoDiffStack = adept::active_stack();
        
    AutoDiffStack->pause_recording();

#endif

    if ( TimeAccurate_ && !StartFromSteadyState_ ) WakeIterations_ = 1;
    
    // Step through the entire ADB file until we get to the end

    PRINTF("Reading in solution... \n\n");fflush(NULL);

    NumberOfTimeSteps_ = 0;

    NoiseTimeStep_ = TimeStep_ = 1.;
  
    for ( Time_ = 0 ;Time_ <= NumberOfTimeSteps_ ; Time_++ ) {
 
       CurrentTime_ = CurrentNoiseTime_ = 0.;
       
       // Read in the next ADB Solution
         
       ReadInAerothermalDatabaseGeometry();
    
       ReadInAerothermalDatabaseSolution(0);

       InterpolateExistingSolution(CurrentNoiseTime_);
                 
       UpdateVortexEdgeStrengths(1, ALL_WAKE_GAMMAS);
          
       UpdateWakeVortexInteractionLists();
          
       // Initialize the free stream conditions
                  
       InitializeFreeStream();
    
       // Calculate forces

       CalculateForces();
   
       // Output status

       OutputStatusFile(0);

       // Write out any rotor coefficients
       
       OutputForcesAndMomentsForGroup(0);
       
       // Write out geometry and current solution

       SPRINTF(CaseString_,"Time: %-f ...",CurrentNoiseTime_);

       WriteOutAerothermalDatabaseGeometry();

       InterpolateSolutionFromGrid(1);
       
       WriteOutAerothermalDatabaseSolution();
 
    }
    
    // Close up files
    
    fclose(StatusFile_);    
    fclose(InputADBFile_);
    fclose(ADBFile_);
    fclose(ADBCaseListFile_);

    // Close any rotor coefficient files

    k = 0;
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       fclose(GroupFile_[c]);
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) fclose(RotorFile_[++k]);
          
    }

    // Solve the adjoint eqation

    if ( DoAdjointSolve_ ) {

       PRINTF("\n\n\nSolving adjoint ... \n");
       
       Optimization_AdjointSolve();
       
      // for testing... CreateAdjointMatrix();
       
    }
    
    // Solve for the functional gradients
    
    if ( CalculateGradients_ ) {
       
       PRINTF("\n\n\nCalculating functional gradients ... \n");
       
       Optimization_CalculateGradientOfDesignFunction();
       
    }
        
}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER Optimization_Calculate_Total_Gradient                #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Optimization_Calculate_Total_Gradient(void)
{

    int i;
    char GradientFileName[2000];
    FILE *GRADFile;

    // Open the gradient file
    
    SPRINTF(GradientFileName,"%s.gradient",FileName_);
    
    if ( (GRADFile = fopen(GradientFileName, "w")) == NULL ) {
   
       PRINTF("Could not open the gradient output file! \n");
   
       exit(1);
   
    }

    // Calculate the total gradient
    
    dF_dMesh_ = new VSPAERO_DOUBLE[3*VSPGeom().Grid(0).NumberOfNodes() + 1];
    
    for ( i = 1 ; i <= 3*VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {
       
       dF_dMesh_[i] = pF_pMesh_[i] - pR_pMesh_[i];

    }
    
    // Output the gradient data

    FPRINTF(GRADFile,"%d \n",VSPGeom().Grid(0).NumberOfNodes());

                    //1234567890    1234567890 1234567890 1234567890    1234567890 1234567890 1234567890    
    
    FPRINTF(GRADFile,"   Node            X          Y          Z            dFdX       dFdY       dFdZ \n");
    
    for ( i = 1 ; i <= VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {
       
       FPRINTF(GRADFile,"%10d   %10.6e %10.6e %10.6e   %10.6e %10.6e %10.6e \n",
        i,
        VSPGeom().Grid(0).NodeList(i).x(),
        VSPGeom().Grid(0).NodeList(i).y(),
        VSPGeom().Grid(0).NodeList(i).z(),
        dF_dMesh_[3*i-2],
        dF_dMesh_[3*i-1],
        dF_dMesh_[3*i  ]);
        
    }
        
    fclose(GRADFile);

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER Optimization_Calculate_pF_pMesh               #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Optimization_Calculate_pF_pMesh(void)
{

#ifdef AUTODIFF

    int i;
 
    VSPAERO_DOUBLE *Function = &(OptimizationFunction_[CurrentOptFunction_]);

    adept::Stack *AutoDiffStack;
    
    AutoDiffStack = adept::active_stack();

    AutoDiffStack->continue_recording();
 
    CalculateForces();

    AutoDiffStack->pause_recording();    

    AutoDiffStack->clear_gradients();
    
    Function->set_gradient(1.);

    for ( i = 1 ; i <= VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {

       AutoDiffStack->independent(VSPGeom().Grid(0).NodeList(i).x());
       AutoDiffStack->independent(VSPGeom().Grid(0).NodeList(i).y());
       AutoDiffStack->independent(VSPGeom().Grid(0).NodeList(i).z());
             
    }

    AutoDiffStack->dependent(*Function);
    
    AutoDiffStack->compute_adjoint();

    pF_pMesh_ = new VSPAERO_DOUBLE[3*VSPGeom().Grid(0).NumberOfNodes() + 1];

    for ( i = 1 ; i <= VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {
       
       pF_pMesh_[3*i-2] = VSPGeom().Grid(0).NodeList(i).x().get_gradient();
       pF_pMesh_[3*i-1] = VSPGeom().Grid(0).NodeList(i).y().get_gradient();
       pF_pMesh_[3*i  ] = VSPGeom().Grid(0).NodeList(i).z().get_gradient();

    }

#endif
 
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER Optimization_Calculate_dR_dMesh               #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Optimization_Calculate_pR_pMesh(void)
{

#ifdef AUTODIFF

    int i, j;

    adept::Stack *AutoDiffStack;
    
    AutoDiffStack = adept::active_stack();
    
    AutoDiffStack->continue_recording();

    CalculateResidual();
    
    AutoDiffStack->pause_recording();
    
    AutoDiffStack->clear_gradients();

    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

       Residual_[i].set_gradient(Psi_[i].value());
       
    }
     
    for ( i = 1 ; i <= VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {

       AutoDiffStack->independent(VSPGeom().Grid(0).NodeList(i).x());
       AutoDiffStack->independent(VSPGeom().Grid(0).NodeList(i).y());
       AutoDiffStack->independent(VSPGeom().Grid(0).NodeList(i).z());
             
    }

    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

       AutoDiffStack->dependent(Residual_[i]);
       
    }

    AutoDiffStack->compute_adjoint();

    pR_pMesh_ = new VSPAERO_DOUBLE[3*VSPGeom().Grid(0).NumberOfNodes() + 1];

    for ( i = 1 ; i <= VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {
       
       pR_pMesh_[3*i-2] = VSPGeom().Grid(0).NodeList(i).x().get_gradient();
       pR_pMesh_[3*i-1] = VSPGeom().Grid(0).NodeList(i).y().get_gradient();
       pR_pMesh_[3*i  ] = VSPGeom().Grid(0).NodeList(i).z().get_gradient();
  
    }

#endif
 
}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER Optimization_Calculate_pF_pGamma               #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Optimization_Calculate_pF_pGamma(void)
{

#ifdef AUTODIFF

    int i;
    
    // Drag is the objective function
    
    VSPAERO_DOUBLE *Function = &(OptimizationFunction_[CurrentOptFunction_]);

    adept::Stack *AutoDiffStack;
    
    AutoDiffStack = adept::active_stack();
 
    AutoDiffStack->continue_recording();
 
    AutoDiffStack->new_recording();
   
    CalculateForces();

    AutoDiffStack->pause_recording();    
    
    AutoDiffStack->clear_gradients();
    
    Function->set_gradient(1.);

    AutoDiffStack->independent(Gamma_[0], NumberOfVortexLoops_ + 1);
    
    AutoDiffStack->dependent(*Function);
    
    AutoDiffStack->compute_adjoint();
    
    pF_pGamma_ = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       pF_pGamma_[i] = Gamma_[0][i].get_gradient();
  
    }
    
    AutoDiffStack->pause_recording();    
        
#endif
 
}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER Optimization_GMRES_AdjointSolve                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Optimization_GMRES_AdjointSolve(void)
{

#ifdef AUTODIFF

    int i, Iters;
    VSPAERO_DOUBLE ResMax, ResRed, ResFin;

    // Create adjoint matrix data
 
    adept::Stack *AutoDiffStack;
    
    AutoDiffStack = adept::active_stack();
    
    AutoDiffStack->continue_recording();

    AutoDiffStack->new_recording();

    CalculateResidual();
    
    AutoDiffStack->pause_recording();
 
    // GMRES solve

    Psi_ = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];
    
    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

       Psi_[i] = 0.;
       
    }
    
    ResMax = 0.001;
    ResRed = 0.001;

    // Use preconditioned GMRES to solve the linear system
     
    AdjointSolve_ = 1;
     
    GMRES_Solver(NumberOfVortexLoops_+1,  // Number of Equations, 0 <= i < Neq
                 3,                       // Max number of outer iterations
                 500,                     // Max number of inner (restart) iterations
                 1,                       // Output flag, verbose = 0, or 1
                 Psi_,                    // Initial guess and solution vector
                 pF_pGamma_,              // Right hand side of Ax = b
                 ResMax,                  // Maximum error tolerance
                 ResRed,                  // Residual reduction factor
                 ResFin,                  // Final log10 of residual reduction   
                 Iters);                  // Final iteration count      
      
#endif
 
}

/*##############################################################################
#                                                                              #
#                 VSP_SOLVER DoPreconditionedMatrixMultiply                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Optimization_DoAdjointMatrixMultiply(VSPAERO_DOUBLE *vec_in, VSPAERO_DOUBLE *vec_out)
{

#ifdef AUTODIFF

    adept::Stack *AutoDiffStack;
    
    AutoDiffStack = adept::active_stack();

    int i;
    
    AutoDiffStack->clear_gradients();
    
    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

       Residual_[i].set_gradient(vec_in[i].value());
       
    }

    AutoDiffStack->independent(Gamma_[0], NumberOfVortexLoops_ + 1);
  
    AutoDiffStack->dependent(Residual_, NumberOfVortexLoops_ + 1);
     
    AutoDiffStack->compute_adjoint();
 
    vec_out[0] = 0.;

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
      
       vec_out[i] = Gamma_[0][i].get_gradient();

    }

#endif

}

/*##############################################################################
#                                                                              #
#                           VSP_SOLVER CreateAdjointMatrix                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateAdjointMatrix(void)
{

    int i, j, k;
    int d_size;
    VSPAERO_DOUBLE Dot, *VecIn, *VecOut, *VecOutAutoDiff, *SaveGamma;
    char AdjointFileName[2000];
    FILE *AdjointFile;

    d_size = sizeof(double);
    
    VecIn = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];
    
    VecOut = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];
    
    VecOutAutoDiff = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];

    SaveGamma = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];

    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

       SaveGamma[i] = Gamma(i);
       
       Gamma(i) = 0.;

    }

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       VecIn[i] = 1.;
       
       VecOut[i] = 0.;
       
       VecOutAutoDiff[i] = 0.;
       
    }
 
    // Allocate space for the matrix
    
    AdjointMatrix_.size(NumberOfVortexLoops_, NumberOfVortexLoops_);
    
    // Create the matrix

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       PRINTF("Working on column %d of %d \r",i,NumberOfVortexLoops_); fflush(NULL);
       
       Gamma(i-1) = 0.;
       Gamma(i  ) = 1.;
       
       MatrixMultiply(Gamma_[0], Residual_);

       for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {
     
          AdjointMatrix_(i,j) = Residual_[j];
          
          PRINTF("AdjointMatrix_(%d,%d): %f \n",i,j,AdjointMatrix_(i,j));

       }

    }
    
    PRINTF("\n\n\n");
    
    // Do matrix-vector product

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
    
      Dot = 0.;
   
#pragma omp parallel for reduction(+:Dot) if (DO_PARALLEL_LOOP)
      for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {
         
         Dot += AdjointMatrix_(i,j) * VecIn[j];
         
      }
      
      VecOut[i] = Dot;
    
    }
    
    // Now do it with automatric differetiation
    
    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

      Gamma(i) = SaveGamma[i];

    }
        
    Optimization_DoAdjointMatrixMultiply(VecIn,VecOutAutoDiff);
    
    // Compare results
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       PRINTF("VecOut, VecOutAutoDiff: %f %f \n",VecOut[i],VecOutAutoDiff[i]);
       
    }
    
    // Write out the matrix

    if ( 0 ) {
       
       SPRINTF(AdjointFileName,"%s.adjoint",FileName_);
       
       if ( (AdjointFile = fopen(AdjointFileName, "wb")) == NULL ) {
      
          PRINTF("Could not open the adjoint matrix file for output! \n");
      
          exit(1);
      
       }
       
       for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {
          
          for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
             
             FWRITE(&(AdjointMatrix_(i,j)), d_size, 1, AdjointFile);
             
          }
          
       }
       
       fclose(AdjointFile);
       
    }
       
}

/*##############################################################################
#                                                                              #
#                           VSP_SOLVER CalculateResidual                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateResidual(void)
{

    int i, k;
    VSPAERO_DOUBLE Dot;

    // VLM Model
   
    if ( ModelType_ == VLM_MODEL ) {
          
       MatrixMultiply(Gamma_[0], Residual_);

       for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
          Residual_[i] = RightHandSide_[i] - Residual_[i];

       }
    
       // Add in unsteady terms
    
       if ( TimeAccurate_ && !QuasiTimeAccurate_ ) {

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
    
       if ( TimeAccurate_ && !QuasiTimeAccurate_ ) {

          // Time dependent wake terms
          
          for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
             MatrixVecTemp_[i] -= vector_dot(VortexLoop(i).Normal(), UnsteadyTrailingWakeVelocity_[i]);
          
          }                 
       
       }      
       
       // Kelvin constraint
      
       for ( k = 1 ; k <= NumberOfKelvinConstraints_ ; k++ ) {
          
          MatrixVecTemp_[NumberOfVortexLoops_ + k] = 0.;
          
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

void VSP_SOLVER::GMRES_Solver(int Neq,                       // Number of Equations, 0 <= i < Neq
                              int IterMax,                   // Max number of outer iterations
                              int NumRestart,                // Max number of inner (restart) iterations
                              int Verbose,                   // Output flag, verbose = 0, or 1
                              VSPAERO_DOUBLE *x,             // Initial guess and solution vector
                              VSPAERO_DOUBLE *RightHandSide, // Right hand side of Ax = b
                              VSPAERO_DOUBLE ErrorMax,       // Maximum error tolerance
                              VSPAERO_DOUBLE ErrorReduction, // Residual reduction factor
                              VSPAERO_DOUBLE &ResFinal,      // Final log10 of residual reduction
                              int    &IterFinal)             // Final iteration count
{

    int i, j, k, Iter, Done, TotalIterations;

    VSPAERO_DOUBLE av, *c, Epsilon, *g, **h, Dot, Mu, *r;
    VSPAERO_DOUBLE rho, rho_zero, rho_tol = 0, rho_ratio, *s, **v, *y, NowTime = 0;
    
    Epsilon = 1.0e-03;
    
    TotalIterations = 0;

    // Allocate memory
    
    c = new VSPAERO_DOUBLE[NumRestart + 1];
    g = new VSPAERO_DOUBLE[NumRestart + 1];
    s = new VSPAERO_DOUBLE[NumRestart + 1];
    y = new VSPAERO_DOUBLE[NumRestart + 1];

    h = new VSPAERO_DOUBLE*[NumRestart + 1];

    for ( i = 0 ; i <= NumRestart ; i++ ) {

       h[i] = new VSPAERO_DOUBLE[NumRestart + 1];

    }

    v = new VSPAERO_DOUBLE*[NumRestart + 1];

    for ( i = 0 ; i <= NumRestart ; i++ ) {

       v[i] = new VSPAERO_DOUBLE[Neq + 1];

    }

    r = new VSPAERO_DOUBLE[Neq + 1] ();

    // Check for case were we come in converged already
    
    for ( i = 0; i < Neq; i++ ) {

      r[i] = RightHandSide[i] - r[i];
   
    }

    rho = sqrt(VectorDot(Neq,r,r));

    if ( rho <=rho_tol && rho <= ErrorMax ) return;
          
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
    
      rho_ratio = rho / rho_zero;
    
      if ( Verbose && Iter == 0 && !TimeAccurate_ ) PRINTF("Wake Iter: %5d / %-5d ... GMRES Iter: %5d ... Red: %10.5f / %-10.5f ...  Max: %10.5f / %-10.5f \r",CurrentWakeIteration_, WakeIterations_, 0,FLOAT(log10(rho_ratio)),FLOAT(log10(ErrorReduction)), FLOAT(log10(rho)), FLOAT(log10(ErrorMax))); fflush(NULL);
      if ( Verbose && Iter == 0 &&  TimeAccurate_ ) PRINTF("TStep: %-5d / %-5d ... Time: %10.5f ... GMRES Iter: %5d ... Red: %10.5f / %-10.5f ...  Max: %-10.5f / %10.5f \r",Time_,NumberOfTimeSteps_,CurrentTime_, 0,log10(rho_ratio),log10(ErrorReduction), log10(rho), log10(ErrorMax)); fflush(NULL);
          
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

         rho_ratio = rho / rho_zero;

//         if ( Verbose && !TimeAccurate_) PRINTF("Red: %10.5f / %-10.5f ...  Max: %10.5f / %-10.5f  \n",log10(rho_ratio),log10(ErrorReduction), log10(rho), log10(ErrorMax)); fflush(NULL);

         TotalIterations = TotalIterations + 1;
         
         NowTime = myclock();
    
         if ( Verbose && !TimeAccurate_) PRINTF("Wake Iter: %5d / %-5d ... GMRES Iter: %5d ... Red: %10.5f / %-10.5f ...  Max: %10.5f / %-10.5f  \r",CurrentWakeIteration_,WakeIterations_,TotalIterations,FLOAT(log10(rho_ratio)),FLOAT(log10(ErrorReduction)), FLOAT(log10(rho)), FLOAT(log10(ErrorMax))); fflush(NULL);
         if ( Verbose &&  TimeAccurate_) PRINTF("TStep: %5d / %-5d ... Time: %10.5f ... GMRES Iter: %5d ... Red: %10.5f / %-10.5f ...  Max: %10.5f / %-10.5f ... STime: %10.5f ... TotTime: %10.5f \r",Time_,NumberOfTimeSteps_,CurrentTime_,TotalIterations,log10(rho_ratio),log10(ErrorReduction), log10(rho), log10(ErrorMax), NowTime - StartSolveTime_, NowTime - StartTime_ ); fflush(NULL);

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

    if ( Verbose && !TimeAccurate_) SPRINTF(ConvergenceLine_,"Wake Iter: %5d / %-5d ... GMRES Iter: %5d ... Red: %10.5f / %-10.5f ...  Max: %10.5f / %-10.5f",CurrentWakeIteration_,WakeIterations_,TotalIterations,log10(rho/rho_zero),log10(ErrorReduction), log10(rho), log10(ErrorMax)); fflush(NULL);
    if ( Verbose &&  TimeAccurate_) SPRINTF(ConvergenceLine_,"TStep: %5d / %-5d ... Time: %10.5f ... GMRES Iter: %5d ... Red: %10.5f / %-10.5f ...  Max: %10.5f / %-10.5f ... STime: %10.5f ... TotTime: %10.5f",Time_,NumberOfTimeSteps_,CurrentTime_,TotalIterations,log10(rho/rho_zero),log10(ErrorReduction), log10(rho), log10(ErrorMax), NowTime - StartSolveTime_, NowTime - StartTime_ ); fflush(NULL);

    return;

}

/*##############################################################################
#                                                                              #
#                              VSP_SOLVER VectorDot                            #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE VSP_SOLVER::VectorDot(int Neq, VSPAERO_DOUBLE *r, VSPAERO_DOUBLE *s) 
{

    int i;
    VSPAERO_DOUBLE dot;

    dot = 0.;

#pragma omp parallel for reduction(+:dot) if (DO_PARALLEL_LOOP)
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

void VSP_SOLVER::ApplyGivensRotation(VSPAERO_DOUBLE c, VSPAERO_DOUBLE s, int k, VSPAERO_DOUBLE *g)
{

  VSPAERO_DOUBLE g1, g2;

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

    VSPAERO_DOUBLE CLReq;
   
    // If a full run, calculate induced drag and surface velocities
    
    if ( !NoiseAnalysis_ ) {
         
       // Modified trailing edge induced drag calculation

       CalculateTrefftzForces();    

       // Calculate surface velocities
   
       CalculateVelocities();
       
       CalculateEdgeVelocities();
       
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

    IntegrateForcesAndMoments();
 
    // Calculate 2D clmax limited forces and moments, as well as span wide
    // loading information
    
    if ( ModelType_ == VLM_MODEL || (ModelType_ == PANEL_MODEL && PanelSpanWiseLoading_ ) ) {

       CalculateCLmaxLimitedForces();

    }
    
    // Keep track of optimization functionals
    
    OptimizationFunction_[OPT_CL] = CL_[0];    
    OptimizationFunction_[OPT_CD] = CD_[0];
    OptimizationFunction_[OPT_CS] = CS_[0];

    OptimizationFunction_[OPT_CMx] = CMx_[0];
    OptimizationFunction_[OPT_CMy] = CMy_[0];
    OptimizationFunction_[OPT_CMz] = CMz_[0];
    
    // L/D with an attemp to hold CL constant
    
    CLReq = 0.1800;
    
    OptimizationFunction_[OPT_CD_CL_CM] = 100.*pow(CL_[0]-CLReq,2.) + ABS(CD_[0]);

    // D with an attempt to hold CL constant, and drive CM to zero

    CLReq = 0.1800;

    OptimizationFunction_[OPT_CD_CL_CM] = 100.*pow(CL_[0]-CLReq,2.) + ABS(CD_[0]) + 5.*ABS(CMy_[0]);
            
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
    
    CalculateCLmaxLimitedForces();   
   
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateTrefftzForces                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateTrefftzForces(void)
{

    int j, p, v, cpu;
    VSPAERO_DOUBLE xyz[3], q[3], qtot[3];

    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
    
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
       
          VortexSheet(cpu,v).TurnWakeDampingOff();
          
       }    
   
    }   
          
    // Loop over vortex edges and calculate forces via K-J theorem, using only wake induced velocities applied at TE

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {

          // Trailing vortices induced velocities... shift the current bound vortex to the 
          // 'trailing edge' of the trailing vortex.
          
          qtot[0] = qtot[1] = qtot[2] = 0.;
          
          qtot[0] = FreeStreamVelocity_[0];
          qtot[1] = FreeStreamVelocity_[1];
          qtot[2] = FreeStreamVelocity_[2];
   
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

    for ( cpu = 1 ; cpu < NumberOfThreads_ ; cpu++ ) {
    
       for ( v = 1 ; v <= NumberOfVortexSheets_ ; v++ ) {     
       
          VortexSheet(cpu,v).TurnWakeDampingOff();
          
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
    VSPAERO_DOUBLE Fx, Fy, Fz, Fxloc, Fyloc, Fzloc, ids1, ids2, wgt1, wgt2;

    // Loop over vortex edges and calculate forces via K-J theorem

#pragma omp parallel for 
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       SurfaceVortexEdge(j).CalculateForces();
               
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
    VSPAERO_DOUBLE DeltaCp, DGammaDt, TimeStep;

    // Choose the time step...
    
    TimeStep = TimeStep_;

    if ( NoiseAnalysis_ ) TimeStep = NoiseTimeStep_;

      
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
     
       SurfaceVortexEdge(j).Unsteady_Fx() = 0.;
       SurfaceVortexEdge(j).Unsteady_Fy() = 0.;
       SurfaceVortexEdge(j).Unsteady_Fz() = 0.;
 
    }
    
    if ( !QuasiTimeAccurate_ ) {
           
#pragma omp parallel for private(DeltaCp,DGammaDt,Edge,j)             
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
          DGammaDt = ( 3.*Gamma(i) - 4.*GammaNM1(i) + GammaNM2(i) ) / (2.*TimeStep);

          DeltaCp = -DGammaDt; // No, there's no 2 here... we divide by 1/2 later... And Vinf is out as we scale the system by it as well

          VortexLoop(i).dCp_Unsteady() = DeltaCp;
  
          for ( j = 1 ; j <= VortexLoop(i).NumberOfEdges() ; j++ ) {
             
             Edge = VortexLoop(i).Edge(j);
             
             SurfaceVortexEdge(Edge).Unsteady_Fx() -= DeltaCp * VortexLoop(i).Area() * VortexLoop(i).Nx() / VortexLoop(i).NumberOfEdges();
             SurfaceVortexEdge(Edge).Unsteady_Fy() -= DeltaCp * VortexLoop(i).Area() * VortexLoop(i).Ny() / VortexLoop(i).NumberOfEdges();
             SurfaceVortexEdge(Edge).Unsteady_Fz() -= DeltaCp * VortexLoop(i).Area() * VortexLoop(i).Nz() / VortexLoop(i).NumberOfEdges();
             
          }
          
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
    VSPAERO_DOUBLE Fx, Fy, Fz, Wgt1, Wgt2;

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
// djk... reconsider this for vspgeom files?     
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

    int i, j, Loop1, Loop2, Edge, Node, Hits, *OnBoundary, BoundaryLoop;
    VSPAERO_DOUBLE Dot, KTFact, Normal[3], Area1, Area2, wgt1, wgt2;
    VSPAERO_DOUBLE Cp, CpCrit, LocalMach, KTMach;
    VSPAERO_DOUBLE *NodalCp, *NodalArea, Area, NewCp, Relax;      
    
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

       if ( VortexLoop(i).VortexSheet() == 0 && VortexLoop(i).dCp() > QMax_*QMax_*SQR(Vref_) ) VortexLoop(i).dCp() = QMax_*QMax_*SQR(Vref_);

       VortexLoop(i).dCp() = 1. - VortexLoop(i).dCp(); 

       // Limit Cp

       Cp = VortexLoop(i).dCp();

       LocalMach = Mach_;
       
       if ( Machref_ > 0. && Vref_ > 0. ) {
          
          LocalMach = Machref_*VortexLoop(i).LocalFreeStreamVelocity(4)/Vref_;
        
       }
       
       LocalMach = MIN(LocalMach, 0.80);
   
       CpCrit = -2.*(1.-LocalMach*LocalMach)/(LocalMach*LocalMach*(1.4+1.));
       
       CpCrit *= 2.5;

       if ( VortexLoop(i).VortexSheet() == 0 ) VortexLoop(i).dCp() = MAX(Cp,CpCrit);     
                                     
    }

    // Clean up solution near intersections

    OnBoundary = new int[NumberOfSurfaceNodes_ + 1];
           
    zero_int_array(OnBoundary, NumberOfSurfaceNodes_);
           
    NodalCp    = new VSPAERO_DOUBLE[NumberOfSurfaceNodes_ + 1];       
    NodalArea  = new VSPAERO_DOUBLE[NumberOfSurfaceNodes_ + 1];
     
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
       
       PRINTF("%s ... KTRes: %10.5f \r",ConvergenceLine_,KTResidual_[1]);

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

    PRINTF("Starting... Creating vorticity gradient data structure .. \n"); fflush(NULL);

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

    PRINTF("Finished... Creating vorticity gradient data structure .. \n"); fflush(NULL);

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

    PRINTF("Starting... Creating vorticity gradient data structure .. \n"); fflush(NULL);

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

    PRINTF("Finished ... Creating vorticity gradient data structure .. \n"); fflush(NULL);

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER CalculateLeastSquaresCoefficients                  #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateLeastSquaresCoefficients(int Loop1)
{
   
    int k, Loop2, NumberOfEquations;
    VSPAERO_DOUBLE  Wgt, dx, dy, dz, Area1, Area2;
    
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
    VSPAERO_DOUBLE Wgt, Area1, Area2, dx, dy, dz;
    VSPAERO_DOUBLE Fact, *dV, *Denom, *Res, *Dif, *Sum, ResMax, ResMax0, Delta, Eps, Wgt1, Wgt2, dVAvg;
    
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

    dV = new VSPAERO_DOUBLE[NumberOfSurfaceNodes_ + 1];
    
    Denom = new VSPAERO_DOUBLE[NumberOfSurfaceNodes_ + 1];
    
    FixedNode = new int[NumberOfSurfaceNodes_ + 1];
    
    Res = new VSPAERO_DOUBLE[NumberOfSurfaceNodes_ + 1];
    
    Dif = new VSPAERO_DOUBLE[NumberOfSurfaceNodes_ + 1];
    
    Sum = new VSPAERO_DOUBLE[NumberOfSurfaceNodes_ + 1];

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
             
             //PRINTF("Iter: %d ... Vorticity Resmax: %f \n",Iter,log10(ResMax/ResMax0));
             
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

void VSP_SOLVER::IntegrateForcesAndMoments(void)
{

    int i, j, c, Loop1, Loop2, LoadCase, *ComponentInThisGroup;
    VSPAERO_DOUBLE Fx, Fy, Fz, Wgt1, Wgt2, LocalVel, LocalMach, LPGFact;
    VSPAERO_DOUBLE CA, SA, CB, SB;
    VSPAERO_DOUBLE Cxi, Cyi, Czi, CDi;
    VSPAERO_DOUBLE Cx2, Cy2, Cz2, Cmx2, Cmy2, Cmz2;
    VSPAERO_DOUBLE ComponentCg[3];

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);

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
       
          Fx = SurfaceVortexEdge(j).Trefftz_Fx();
          Fy = SurfaceVortexEdge(j).Trefftz_Fy();
          Fz = SurfaceVortexEdge(j).Trefftz_Fz();
       
          Cxi += Fx;
          Cyi += Fy;
          Czi += Fz;

          // If this is an unsteady case, keep track of component group induced drag
      
          c = ComponentInThisGroup[SurfaceVortexEdge(j).ComponentID()];

          if ( c > 0) {
          
             ComponentGroupList_[c].CD() += ( Fx * CA + Fz * SA ) * CB - Fy * SB;

          }

       }

       if ( !SurfaceVortexEdge(j).IsTrailingEdge()  ) {

          // Sum up forces and moments from each edge

          Fx = SurfaceVortexEdge(j).Fx() + SurfaceVortexEdge(j).Unsteady_Fx();
          Fy = SurfaceVortexEdge(j).Fy() + SurfaceVortexEdge(j).Unsteady_Fy();
          Fz = SurfaceVortexEdge(j).Fz() + SurfaceVortexEdge(j).Unsteady_Fz();

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
    CD_[LoadCase] = ( CFx_[LoadCase] * CA + CFz_[LoadCase] * SA ) * CB - CFy_[LoadCase] * SB;
    CS_[LoadCase] = ( CFx_[LoadCase] * CA + CFz_[LoadCase] * SA ) * SB + CFy_[LoadCase] * CB;

    // Non dimensonalize
    
     CL_[LoadCase] /= (0.5*Sref_*Vref_*Vref_);   
     CD_[LoadCase] /= (0.5*Sref_*Vref_*Vref_);   
     CS_[LoadCase] /= (0.5*Sref_*Vref_*Vref_);   

    CFx_[LoadCase] /= (0.5*Sref_*Vref_*Vref_);
    CFy_[LoadCase] /= (0.5*Sref_*Vref_*Vref_);
    CFz_[LoadCase] /= (0.5*Sref_*Vref_*Vref_);
    
    CMx_[LoadCase] /= (0.5*Sref_*Bref_*Vref_*Vref_); // Roll
    CMy_[LoadCase] /= (0.5*Sref_*Cref_*Vref_*Vref_); // Pitch
    CMz_[LoadCase] /= (0.5*Sref_*Bref_*Vref_*Vref_); // Yaw
    
    CDTrefftz_[LoadCase] = CDi / (0.5*Sref_*Vref_*Vref_);

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
                  
       CDTrefftz_[LoadCase] *= 2.;

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

void VSP_SOLVER::CalculateCLmaxLimitedForces(void)
{

    int i, j, k, c, Node, t, Loop, Loop1, Loop2, LoadCase, *ComponentInThisGroup;
    int NumberOfStations, SpanStation, SurfaceID, NumSurfs; 
    VSPAERO_DOUBLE Fx, Fy, Fz, Fxi, Fyi, Fzi, Wgt, Wgti;
    VSPAERO_DOUBLE Length, Re, Cf, Cdi, Cn, Cx, Cy, Cz;
    VSPAERO_DOUBLE Swet, StallFact, CvCl, FRatio, FFactor;
    VSPAERO_DOUBLE CA, SA, CB, SB, Cmx, Cmy, Cmz, Cl_2d, dCD, CLv;
    VSPAERO_DOUBLE Mag, xyzLE[3], xyzTE[3], S[3];
    VSPAERO_DOUBLE ViscousForce, dF[3], dM[3], Dot, xyz_mid[3], Vec[3], ComponentCg[3];
    VSPAERO_DOUBLE Cl, Cd, Cs, Ct, Chord, Cd_Airfoil;
    VSPAERO_DOUBLE nvec[3], svec[3], Wgt1, Wgt2, LPGFact, U, V, W, LocalVel, LocalMach, Fact;
    VSPAERO_DOUBLE Thrust, Moment, Diameter, RPM, J, CT, CQ, CP, EtaP, CT_h, CQ_h, CP_h, FOM;
    VSPAERO_DOUBLE c1, c2, c3, CpMinLoc, LocalCp, StallRatio, CnFact;

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);

    // Zero out spanwise loading arrays
  
    for ( i = StartOfSpanLoadDataSets_  ; i <= NumberOfSpanLoadDataSets_ ; i++ ) { 

       SpanLoadData(i).ZeroForcesAndMoments();
         
    }   

    // Zero out any component group forces and moments
         
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       ComponentGroupList_[c].CL() = 0.;
       ComponentGroupList_[c].CD() = 0.;
       ComponentGroupList_[c].CS() = 0.;

       ComponentGroupList_[c].CLo() = 0.;
       ComponentGroupList_[c].CDo() = 0.;
       ComponentGroupList_[c].CSo() = 0.;
                    
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
    
    ComponentInThisGroup = new int[VSPGeom().NumberOfComponents() + 1];
    
    zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());     
           
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {  
       
       for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfComponents() ; i++ ) {
          
          ComponentInThisGroup[ComponentGroupList_[c].ComponentList(i)] = c;
          
       }
       
    }       

   // Calculate span station areas and y values

    for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {

       if ( VortexLoop(j).DegenWingID() > 0 || VortexLoop(j).VortexSheet() > 0 ) {
          
          if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {
             
             SurfaceID = VortexLoop(j).SurfaceID();
           
             SpanStation = VortexLoop(j).SpanStation();
             
          }
          
          else {
          
             SurfaceID = VortexLoop(j).VortexSheet();
              
             SpanStation = VortexLoop(j).SpanStation();
              
          }
          
       }
       
       else {
          
          if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {
             
             SurfaceID = VortexLoop(j).SurfaceID();
           
             SpanStation = 1;
             
          }
          
          else {
          
             SurfaceID = 0;
          
             SpanStation = 1;
              
          }             
          
       }
   
       // Average span location, and strip area

       SpanLoadData(SurfaceID).Span_Xavg(SpanStation) += VortexLoop(j).Xc() * VortexLoop(j).Area();
       SpanLoadData(SurfaceID).Span_Yavg(SpanStation) += VortexLoop(j).Yc() * VortexLoop(j).Area();
       SpanLoadData(SurfaceID).Span_Zavg(SpanStation) += VortexLoop(j).Zc() * VortexLoop(j).Area();

       SpanLoadData(SurfaceID).Span_Area(SpanStation) += VortexLoop(j).Area();
       
       // Average local velocity

       U = VortexLoop(j).U();
       V = VortexLoop(j).V();
       W = VortexLoop(j).W();

       SpanLoadData(SurfaceID).Span_Local_Velocity(SpanStation)[0] += U * VortexLoop(j).Area() / Vref_;
       SpanLoadData(SurfaceID).Span_Local_Velocity(SpanStation)[1] += V * VortexLoop(j).Area() / Vref_;
       SpanLoadData(SurfaceID).Span_Local_Velocity(SpanStation)[2] += W * VortexLoop(j).Area() / Vref_;

    }

    // Calculate span areas and local velocities

    for ( i = StartOfSpanLoadDataSets_ ; i <= NumberOfSpanLoadDataSets_ ; i++ ) { 
      
       if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {
          
          NumberOfStations = 1;
          
          if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
             
             NumberOfStations = SpanLoadData(i).NumberOfSpanStations();
       
          }

       }
       
       else {
                              
          NumberOfStations = SpanLoadData(i).NumberOfSpanStations();

       }
 
       for ( j = 1 ; j <= NumberOfStations ; j++ ) {
   
          if ( SpanLoadData(i).Span_Area(j) > 0. ) {
           
             SpanLoadData(i).Span_Xavg(j) /= SpanLoadData(i).Span_Area(j);
             SpanLoadData(i).Span_Yavg(j) /= SpanLoadData(i).Span_Area(j);
             SpanLoadData(i).Span_Zavg(j) /= SpanLoadData(i).Span_Area(j);

             SpanLoadData(i).Span_Local_Velocity(j)[0] /= SpanLoadData(i).Span_Area(j);
             SpanLoadData(i).Span_Local_Velocity(j)[1] /= SpanLoadData(i).Span_Area(j);
             SpanLoadData(i).Span_Local_Velocity(j)[2] /= SpanLoadData(i).Span_Area(j);
             SpanLoadData(i).Span_Local_Velocity(j)[3] /= SpanLoadData(i).Span_Area(j);
             
             SpanLoadData(i).Span_Local_Velocity(j)[3] = sqrt( SpanLoadData(i).Span_Local_Velocity(j)[0]*SpanLoadData(i).Span_Local_Velocity(j)[0]
                                                             + SpanLoadData(i).Span_Local_Velocity(j)[1]*SpanLoadData(i).Span_Local_Velocity(j)[1]
                                                             + SpanLoadData(i).Span_Local_Velocity(j)[2]*SpanLoadData(i).Span_Local_Velocity(j)[2] );
             
             if ( ModelType_ == PANEL_MODEL ) SpanLoadData(i).Span_Area(j) *= 0.5; // I just want the strip planform area
             
          }
          
          // Local Velocity
          
          LocalVel = SpanLoadData(i).Span_Local_Velocity(j)[3];
    
          // Local Re number, note that Local_Vel is scaled by Vref_
          
          Re = ReCref_ * LocalVel * SpanLoadData(i).Span_Chord(j) / Cref_;
          
          // Calculate Local Mach
          
          if ( Mach_ < 1. ) {
             
             if ( Machref_ > 0. ) {
               
                LocalMach = Machref_*ABS(LocalVel);
                
                LocalMach = MIN(LocalMach,0.999);
                
             }
             
             else {
               
                LocalMach = Mach_*ABS(LocalVel);
                
                LocalMach = MIN(LocalMach,0.999);
                
             }
             
             LocalMach = MIN(LocalMach, 0.90);
             
          }
                 
          // Calculate min attainable Cp
          
          CpMinLoc = -999.;
                    
          if ( LocalMach < 1. && Re > 0. ) {
             
             // From "Method for the Prediction of Wing Maximum Lift"
             // by Walter O. Valarezo* and Vincent D. Chinf
             // This is the Cpmin value when the flow goes sonic
             
             CpMinLoc = (pow((1. + 0.2*LocalMach*LocalMach)/1.2,3.5) - 1.)/(0.7*LocalMach*LocalMach);
           
             // Carlson's Cp min limit correlation modification, based on local Re
             
             Re /= 1.e6;
             
             c1 = Re;
             c2 = Re + pow(10.,(4.-3.*LocalMach));
             c3 = 0.05 + 0.35*SQR(1.-LocalMach);
             
             CpMinLoc *= pow(c1/c2,c3);
          
          }

          SpanLoadData(i).Span_CpMin(j) = CpMinLoc;
        
       }
       
    }   
    
    // Loop over vortex edges and calculate forces via K-J theorem, using only wake induced velocities applied at TE

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       Loop1 = SurfaceVortexEdge(j).Loop1();
       Loop2 = SurfaceVortexEdge(j).Loop2();
 
       // Extract out forces

       Fx = Fy = Fz = Fxi = Fyi = Fzi = 0.;
       
       if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {

          Fxi = SurfaceVortexEdge(j).Trefftz_Fx();
          Fyi = SurfaceVortexEdge(j).Trefftz_Fy();
          Fzi = SurfaceVortexEdge(j).Trefftz_Fz();
          
       }
       
       if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {
          
          Fx = SurfaceVortexEdge(j).Fx() + SurfaceVortexEdge(j).Unsteady_Fx();
          Fy = SurfaceVortexEdge(j).Fy() + SurfaceVortexEdge(j).Unsteady_Fy();
          Fz = SurfaceVortexEdge(j).Fz() + SurfaceVortexEdge(j).Unsteady_Fz();

       }

       // Calculate local free stream velocity
       
       Wgt1 = VortexLoop(Loop1).Area()/( VortexLoop(Loop1).Area() + VortexLoop(Loop2).Area() );
       
       Wgt2 = 1. - Wgt1;

       U = Wgt1 * VortexLoop(Loop1).LocalFreeStreamVelocity(0) + Wgt2 * VortexLoop(Loop2).LocalFreeStreamVelocity(0);
       V = Wgt1 * VortexLoop(Loop1).LocalFreeStreamVelocity(1) + Wgt2 * VortexLoop(Loop2).LocalFreeStreamVelocity(1);
       W = Wgt1 * VortexLoop(Loop1).LocalFreeStreamVelocity(2) + Wgt2 * VortexLoop(Loop2).LocalFreeStreamVelocity(2);
              
       LocalVel = sqrt( U*U + V*V + W*W );

       LocalCp =  Wgt1*VortexLoop(Loop1).dCp() + Wgt2*VortexLoop(Loop2).dCp();
  
       LocalCp /= pow(LocalVel/Vref_,2.);
       
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

          Wgt = 0.;
        
          Wgti = VortexLoop(Loop).Area() / ( VortexLoop(SurfaceVortexEdge(j).Loop1()).Area() + VortexLoop(SurfaceVortexEdge(j).Loop2()).Area() );

          if ( ( SurfaceVortexEdge(j).VortexLoop1() != 0 && SurfaceVortexEdge(j).VortexLoop2() != 0 ) || SurfaceVortexEdge(j).IsLeadingEdge() || SurfaceType_ == VSPGEOM_SURFACE ) Wgt = Wgti;

          // Wing Surface
          
          if ( VortexLoop(Loop).DegenWingID() > 0 || VortexLoop(Loop).VortexSheet() > 0 ) {
             
             if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {
                
                SurfaceID = VortexLoop(Loop).SurfaceID();
              
                SpanStation = VortexLoop(Loop).SpanStation();
                
             }
             
             else {
             
                SurfaceID = VortexLoop(Loop).VortexSheet();
             
                SpanStation = VortexLoop(Loop).SpanStation();

             }
             
             // Check for stall

             if ( Clmax_2d_ < -998. ) {
       
                if ( ( ModelType_ == VLM_MODEL ) && ABS(LocalCp) >= ABS(SpanLoadData(SurfaceID).Span_CpMin(SpanStation) - VSPGeom().Grid(1).LoopList(Loop).LocalFreeStreamVelocity(3)) ) {
                   
                   StallRatio = ABS(LocalCp)/ABS(SpanLoadData(SurfaceID).Span_CpMin(SpanStation) - VSPGeom().Grid(1).LoopList(Loop).LocalFreeStreamVelocity(3));
                   
                   StallRatio = pow(StallRatio,1.5);
                   
                   SpanLoadData(SurfaceID).StallFact(SpanStation) += StallRatio*VortexLoop(Loop).Area();
            
                }
                
                if ( ( ModelType_ == PANEL_MODEL ) && LocalCp <= SpanLoadData(SurfaceID).Span_CpMin(SpanStation) - VSPGeom().Grid(1).LoopList(Loop).LocalFreeStreamVelocity(3)  ) {
                   
                   StallRatio = ABS(LocalCp)/ABS(SpanLoadData(SurfaceID).Span_CpMin(SpanStation) - VSPGeom().Grid(1).LoopList(Loop).LocalFreeStreamVelocity(3));
                   
                   StallRatio = pow(StallRatio,1.5);
            
                   SpanLoadData(SurfaceID).StallFact(SpanStation) += VortexLoop(Loop).Area();

                }
       
             }             
        
             // Forces
             
             SpanLoadData(SurfaceID).Span_Cx(SpanStation) += Wgt*Fx;
             SpanLoadData(SurfaceID).Span_Cy(SpanStation) += Wgt*Fy;
             SpanLoadData(SurfaceID).Span_Cz(SpanStation) += Wgt*Fz;

             // Chordwise integrated induced forces

             SpanLoadData(SurfaceID).Span_Cxi(SpanStation) += Wgti*Fxi;
             SpanLoadData(SurfaceID).Span_Cyi(SpanStation) += Wgti*Fyi;
             SpanLoadData(SurfaceID).Span_Czi(SpanStation) += Wgti*Fzi;

             // Chordwise integrated moments

             SpanLoadData(SurfaceID).Span_Cmx(SpanStation) += Wgt * Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Wgt * Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
             SpanLoadData(SurfaceID).Span_Cmy(SpanStation) += Wgt * Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Wgt * Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
             SpanLoadData(SurfaceID).Span_Cmz(SpanStation) += Wgt * Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Wgt * Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw    

          }
          
          // Body Surface
          
          else {
         
             if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {
                
                SurfaceID = VortexLoop(Loop).SurfaceID();
              
                SpanStation = 1;
                
             }
             
             else {
             
                SurfaceID = 0;
             
                SpanStation = 1;
                
             }

             // Chordwise integrated forces
             
             if ( ModelType_ != VLM_MODEL ) {
                                
                // Forces
                
                SpanLoadData(SurfaceID).Span_Cx(SpanStation) += Wgt*Fx;                      
                SpanLoadData(SurfaceID).Span_Cy(SpanStation) += Wgt*Fy;                    
                SpanLoadData(SurfaceID).Span_Cz(SpanStation) += Wgt*Fz;                        
                    
                // Chordwise integrated induced forces

                SpanLoadData(SurfaceID).Span_Cxi(SpanStation) += Wgti*Fxi;                      
                SpanLoadData(SurfaceID).Span_Cyi(SpanStation) += Wgti*Fyi;                    
                SpanLoadData(SurfaceID).Span_Czi(SpanStation) += Wgti*Fzi;                        
                                                   
             }
             
             // Chordwise integrated moments

             SpanLoadData(SurfaceID).Span_Cmx(SpanStation) += Wgt * Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Wgt * Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll                
             SpanLoadData(SurfaceID).Span_Cmy(SpanStation) += Wgt * Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Wgt * Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch              
             SpanLoadData(SurfaceID).Span_Cmz(SpanStation) += Wgt * Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Wgt * Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw
   
          }
          
       }      

    }
 
    // Non-dimensionalize

    for ( i = StartOfSpanLoadDataSets_ ; i <= NumberOfSpanLoadDataSets_ ; i++ ) { 
      
       if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {
          
          NumberOfStations = 1;
          
          if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
             
             NumberOfStations = SpanLoadData(i).NumberOfSpanStations();
       
          }

       }
       
       else {
                              
          NumberOfStations = SpanLoadData(i).NumberOfSpanStations();

       }

       for ( k = 1 ; k <= NumberOfStations ; k++ ) {
          
          if ( SpanLoadData(i).Span_Area(k) > 0. ) {
     
             SpanLoadData(i).Span_Cx(k) /= (0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_);
             SpanLoadData(i).Span_Cy(k) /= (0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_);
             SpanLoadData(i).Span_Cz(k) /= (0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_);

             SpanLoadData(i).Span_Cxi(k) /= (0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_);
             SpanLoadData(i).Span_Cyi(k) /= (0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_);
             SpanLoadData(i).Span_Czi(k) /= (0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_);
               
             Chord = SpanLoadData(i).Span_Chord(k);

             SpanLoadData(i).Span_Cmx(k) /= (0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_*Chord);  // Roll
             SpanLoadData(i).Span_Cmy(k) /= (0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_*Chord);  // Pitch
             SpanLoadData(i).Span_Cmz(k) /= (0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_*Chord);  // Yaw

             SpanLoadData(i).StallFact(k) /= SpanLoadData(i).Span_Area(k);

          }
        
       }
       
    }
        
    // Calculate maximum CL... imposing local 2D Clmax limits

    LoadCase = 0;

    CL_[LoadCase] = CS_[LoadCase] = CD_[LoadCase] = 0.;
    
    CFx_[LoadCase] = CFy_[LoadCase] = CFz_[LoadCase] = 0.;
    
    CMx_[LoadCase] = CMy_[LoadCase] = CMz_[LoadCase] = 0.;
    
    CDo_[LoadCase] = CMxo_ = CMyo_ = CMzo_ = 0.;  
    
    CDTrefftz_[LoadCase] = 0;

    for ( i = StartOfSpanLoadDataSets_ ; i <= NumberOfSpanLoadDataSets_ ; i++ ) { 
      
       if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {
          
          NumberOfStations = 1;
          
          if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
             
             NumberOfStations = SpanLoadData(i).NumberOfSpanStations();
       
          }

       }
       
       else {
                              
          NumberOfStations = SpanLoadData(i).NumberOfSpanStations();

       }
       
       VSPGeom().VSP_Surface(i).CDo() = 0.;
      
       for ( k = 1 ; k <= NumberOfStations ; k++ ) {
         
          Cx = SpanLoadData(i).Span_Cx(k);
          Cy = SpanLoadData(i).Span_Cy(k);
          Cz = SpanLoadData(i).Span_Cz(k);
           
          Cl =  (-Cx * SA +  Cz * CA );
          Cd =  ( Cx * CA +  Cz * SA ) * CB -  Cy * SB;   
          Cs =  ( Cx * CA +  Cz * SA ) * SB +  Cy * CB;

          Cdi = ( SpanLoadData(i).Span_Cxi(k) * CA + SpanLoadData(i).Span_Czi(k) * SA ) * CB - SpanLoadData(i).Span_Cyi(k) * SB;

          Cn = SpanLoadData(i).Span_Cn(k) = Cx * SpanLoadData(i).Span_Nvec(k)[0] + Cy * SpanLoadData(i).Span_Nvec(k)[1] + Cz * SpanLoadData(i).Span_Nvec(k)[2];
   
          Cmx = SpanLoadData(i).Span_Cmx(k);
          Cmy = SpanLoadData(i).Span_Cmy(k);
          Cmz = SpanLoadData(i).Span_Cmz(k);          

          // Cdi and Cd the same for supersonic cases
          
          if ( Mach_ >= 1. ) Cdi = Cd;
      
          // Adjust forces and moments for local 2d stall
          
          StallFact = 1.;

          if ( NumberOfStations > 1 && Clmax_2d_ > 0. ) {
     
             Cl_2d = Clmax_2d_ * pow(SpanLoadData(i).Span_Local_Velocity(k)[3], 2.);
          
             if ( ABS(Cn) > 0. ) StallFact = ABS(MIN(ABS(Cn), Cl_2d) / ABS(Cn));
             
          }
          
          else if ( Mach_ < 1. && NumberOfStations > 1 && Clmax_2d_ < -998. ) {
             
              if ( SpanLoadData(i).StallFact(k) > 0. && ABS(Cn) >= 1.25 ) {

                StallFact = 1. - MIN(1.,SpanLoadData(i).StallFact(k));
                
                SpanLoadData(i).IsStalled(k) = 1;
               
             }
             
          }
                 
          SpanLoadData(i).Span_Cl(k) = StallFact * Cl;
          SpanLoadData(i).Span_Cd(k) =             Cd + 0.25*(1.-StallFact)*ABS(Cn);
          SpanLoadData(i).Span_Cs(k) = StallFact * Cs;
          
          SpanLoadData(i).Span_Cx(k) = StallFact * Cx;
          SpanLoadData(i).Span_Cy(k) = StallFact * Cy;
          SpanLoadData(i).Span_Cz(k) = StallFact * Cz;       
    
          SpanLoadData(i).Span_Cn(k) = StallFact * Cn;
            
          SpanLoadData(i).Span_Cmx(k) = StallFact * Cmx;
          SpanLoadData(i).Span_Cmy(k) = StallFact * Cmy;
          SpanLoadData(i).Span_Cmz(k) = StallFact * Cmz;        

          //   spanwise forces and moments
          
          if ( !SpanLoadData(i).IsARotor() ) {
             
             // For VLM, don't book keep forces on non-lifting (body) panels
// djk             
             if ( 1||NumberOfStations > 1 ) {

                 CL_[LoadCase] += 0.5 * SpanLoadData(i).Span_Cl(k) * SpanLoadData(i).Span_Area(k);
                 CD_[LoadCase] += 0.5 * SpanLoadData(i).Span_Cd(k) * SpanLoadData(i).Span_Area(k);
                 CS_[LoadCase] += 0.5 * SpanLoadData(i).Span_Cs(k) * SpanLoadData(i).Span_Area(k);
                                                              
                CFx_[LoadCase] += 0.5 * SpanLoadData(i).Span_Cx(k) * SpanLoadData(i).Span_Area(k);
                CFy_[LoadCase] += 0.5 * SpanLoadData(i).Span_Cy(k) * SpanLoadData(i).Span_Area(k);
                CFz_[LoadCase] += 0.5 * SpanLoadData(i).Span_Cz(k) * SpanLoadData(i).Span_Area(k);
                
             }
             
             Chord = SpanLoadData(i).Span_Chord(k);
             
             CMx_[LoadCase] += 0.5 * SpanLoadData(i).Span_Cmx(k) * SpanLoadData(i).Span_Area(k) * Chord;
             CMy_[LoadCase] += 0.5 * SpanLoadData(i).Span_Cmy(k) * SpanLoadData(i).Span_Area(k) * Chord;
             CMz_[LoadCase] += 0.5 * SpanLoadData(i).Span_Cmz(k) * SpanLoadData(i).Span_Area(k) * Chord;
             
             CDTrefftz_[LoadCase] += 0.5 * Cdi * SpanLoadData(i).Span_Area(k);
             
          }
          
          // If this is an unsteady case, keep track of component group forces and moments
   
          if ( i > 0 ) {
             
             if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {
             
                c = ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()];
                
             }
             
             else {
                
                c = 0;
                
                j = 1;
                
                while ( j <= SpanLoadData(i).NumberOfSpanStations() && c == 0 ) {
                      
                   Node = VortexSheet(i).TrailingVortex(j).Node();
                   
                   c = ComponentInThisGroup[VSPGeom().Grid(1).NodeList(Node).ComponentID()];
                
                   j++;
                   
                }                
                
             }

             if ( c > 0) {
                
                // Lift, drag, side force

                Cx = SpanLoadData(i).Span_Cx(k);
                Cy = SpanLoadData(i).Span_Cy(k);
                Cz = SpanLoadData(i).Span_Cz(k);

                Cl =  (-Cx * SA +  Cz * CA );
                Cd =  ( Cx * CA +  Cz * SA ) * CB -  Cy * SB;   
                Cs =  ( Cx * CA +  Cz * SA ) * SB +  Cy * CB;
                                                       
                ComponentGroupList_[c].CL() += 0.5 * Cl * SpanLoadData(i).Span_Area(k);
                ComponentGroupList_[c].CD() += 0.5 * Cd * SpanLoadData(i).Span_Area(k);
                ComponentGroupList_[c].CS() += 0.5 * Cs * SpanLoadData(i).Span_Area(k);
                                                   
                // Forces

                dF[0] = 0.5 * SpanLoadData(i).Span_Cx(k) * SpanLoadData(i).Span_Area(k);
                dF[1] = 0.5 * SpanLoadData(i).Span_Cy(k) * SpanLoadData(i).Span_Area(k);
                dF[2] = 0.5 * SpanLoadData(i).Span_Cz(k) * SpanLoadData(i).Span_Area(k);
                              
                ComponentGroupList_[c].Cx() += dF[0];
                ComponentGroupList_[c].Cy() += dF[1];
                ComponentGroupList_[c].Cz() += dF[2];

                // Moments, about component CG

                Chord = SpanLoadData(i).Span_Chord(k);
                     
                ComponentCg[0] = ComponentGroupList_[c].OVec(0);
                ComponentCg[1] = ComponentGroupList_[c].OVec(1);
                ComponentCg[2] = ComponentGroupList_[c].OVec(2);
 
                dM[0] = 0.5 * SpanLoadData(i).Span_Cmx(k) * SpanLoadData(i).Span_Area(k) * Chord;
                dM[1] = 0.5 * SpanLoadData(i).Span_Cmy(k) * SpanLoadData(i).Span_Area(k) * Chord;
                dM[2] = 0.5 * SpanLoadData(i).Span_Cmz(k) * SpanLoadData(i).Span_Area(k) * Chord;

                dM[0] += dF[2] * ( XYZcg_[1] - ComponentCg[1] ) - dF[1] * ( XYZcg_[2] - ComponentCg[2] );
                dM[1] -= dF[2] * ( XYZcg_[0] - ComponentCg[0] ) - dF[0] * ( XYZcg_[2] - ComponentCg[2] );
                dM[2] += dF[1] * ( XYZcg_[0] - ComponentCg[0] ) - dF[0] * ( XYZcg_[1] - ComponentCg[1] );
                
                ComponentGroupList_[c].Cmx() += dM[0];
                ComponentGroupList_[c].Cmy() += dM[1];
                ComponentGroupList_[c].Cmz() += dM[2];

             }
             
          }
  
          // Spanwise viscous drag for wings
          
          if ( NumberOfStations > 1 ) {

             S[0] = SpanLoadData(i).Span_Svec(k)[0];
             S[1] = SpanLoadData(i).Span_Svec(k)[1];
             S[2] = SpanLoadData(i).Span_Svec(k)[2];
      
             // Alignment of strip with local velocity
             
             Dot = 0.;
             
             if ( SpanLoadData(i).Span_Local_Velocity(k)[3] > 0. ) {
                
                Dot = ( S[0]*SpanLoadData(i).Span_Local_Velocity(k)[0] + S[1]*SpanLoadData(i).Span_Local_Velocity(k)[1] + S[2]*SpanLoadData(i).Span_Local_Velocity(k)[2] )/SpanLoadData(i).Span_Local_Velocity(k)[3];
             
             }      

             xyz_mid[0] = SpanLoadData(i).Span_XTE(k) - 0.5*SpanLoadData(i).Span_Chord(k)*S[0];
             xyz_mid[1] = SpanLoadData(i).Span_YTE(k) - 0.5*SpanLoadData(i).Span_Chord(k)*S[1];
             xyz_mid[2] = SpanLoadData(i).Span_ZTE(k) - 0.5*SpanLoadData(i).Span_Chord(k)*S[2];                          
 
             LocalVel = SpanLoadData(i).Span_Local_Velocity(k)[3];

             // Chord
    
             Length = SpanLoadData(i).Span_Chord(k);
        
             // Local Re number, note that Local_Vel is scaled by Vref_
             
             Re = ReCref_ * LocalVel * Length / Cref_;

             Cf = 1.037 / pow(log10(Re),2.58);
      
             // Calculate flat plate viscous force... note density is booked kept as '1'
            
             ViscousForce = 0.5 * Cf * pow(LocalVel * Vref_, 2.) * SpanLoadData(i).Span_Area(k);

             // Calculate 2D drag due to lift, simple fit to NACA 0012 data
             
             CLv = 0.;

             if ( LocalVel > 0. ) CLv = SpanLoadData(i).Span_Cn(k) / pow(LocalVel, 2.);

             if ( Mach_ < 1. ) {
                
                if ( Machref_ > 0. ) {
                   
                   LocalMach = Machref_*ABS(LocalVel);
                    
                }
                
                else {
                   
                   LocalMach = Mach_*ABS(LocalVel);
                   
                
                }
                
                LocalMach = MIN(LocalMach,0.999);
                
             }

             Fact = 1.;

             if ( LocalMach >= 0.6) Fact = 1. + pow(MIN(LocalMach,1.) - 0.6,2.)/2.;    
                          
             // Curve fit, NACA 0012 and 2412 using numbers split between smooth and standard roughness data

             CvCl = 0.00625 + 0.01*ABS(Clo_2d_);

             ViscousForce += 0.5*Fact*CvCl*pow(CLv-Clo_2d_, 2.) * pow(LocalVel * Vref_, 2.) * SpanLoadData(i).Span_Area(k);
    
             // Crude reverse flow model
             
             if ( Dot <= 0. ) ViscousForce *= 2.;
                 
             // Vector components - along local velocity

             dF[0] = dF[1] = dF[2] = 0.;
             
             if ( SpanLoadData(i).Span_Local_Velocity(k)[3] > 0. ) {
             
                dF[0] = ViscousForce * SpanLoadData(i).Span_Local_Velocity(k)[0] / SpanLoadData(i).Span_Local_Velocity(k)[3];
                dF[1] = ViscousForce * SpanLoadData(i).Span_Local_Velocity(k)[1] / SpanLoadData(i).Span_Local_Velocity(k)[3];
                dF[2] = ViscousForce * SpanLoadData(i).Span_Local_Velocity(k)[2] / SpanLoadData(i).Span_Local_Velocity(k)[3];
                
             }
                         
             // Drag component
             
             dCD = 0.;
             
             if ( Vinf_ > 0. ) dCD = vector_dot(dF,FreeStreamVelocity_)/Vinf_;
             
             // Non-dimensionalize
             
             dCD /= 0.5*Sref_*Vref_*Vref_;

             // Save at component level
             
             VSPGeom().VSP_Surface(i).CDo() += dCD;
             
             // Total drag
      
             if ( !SpanLoadData(i).IsARotor() ) CDo_[LoadCase] += dCD;

             // Forces

             SpanLoadData(i).Span_Cxo(k) = dF[0]/(0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_);
             SpanLoadData(i).Span_Cyo(k) = dF[1]/(0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_);  
             SpanLoadData(i).Span_Czo(k) = dF[2]/(0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_);
                 
             if ( !SpanLoadData(i).IsARotor() ) {
                                 
                CFxo_ += dF[0];
                CFyo_ += dF[1];
                CFzo_ += dF[2];
             
             }
             
             // Moments
   
             dM[0] = dF[2] * ( xyz_mid[1] - XYZcg_[1] ) - dF[1] * ( xyz_mid[2] - XYZcg_[2] );   // Roll
             dM[1] = dF[0] * ( xyz_mid[2] - XYZcg_[2] ) - dF[2] * ( xyz_mid[0] - XYZcg_[0] );   // Pitch
             dM[2] = dF[1] * ( xyz_mid[0] - XYZcg_[0] ) - dF[0] * ( xyz_mid[1] - XYZcg_[1] );   // Yaw

             Chord = SpanLoadData(i).Span_Chord(k);
        
             SpanLoadData(i).Span_Cmxo(k) += dM[0]/(0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_*Chord);
             SpanLoadData(i).Span_Cmyo(k) += dM[1]/(0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_*Chord);    
             SpanLoadData(i).Span_Cmzo(k) += dM[2]/(0.5*SpanLoadData(i).Span_Area(k)*Vref_*Vref_*Chord);  

             CMxo_ += dM[0];             
             CMyo_ += dM[1];                     
             CMzo_ += dM[2];
  
             // If this is an unsteady case, keep track of component group forces and moments
 
             if ( i > 0 ) {
    
                if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {
                
                   c = ComponentInThisGroup[VSPGeom().VSP_Surface(i).ComponentID()];
                   
                }
                
                else {
                   
                   c = 0;
                   
                   j = 1;
                   
                   while ( j <= SpanLoadData(i).NumberOfSpanStations() && c == 0 ) {
                         
                      Node = VortexSheet(i).TrailingVortex(j).Node();
                      
                      c = ComponentInThisGroup[VSPGeom().Grid(1).NodeList(Node).ComponentID()];
                   
                      j++;
                      
                   }                
                   
                }
                             
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

    // Clip pressures over entire vehicle, straight inviscid limits

    for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {

       // Calculate local velocity
       
       LocalCp = VortexLoop(j).dCp();
 
       LocalVel = VortexLoop(j).LocalFreeStreamVelocity(4);
 
       LocalCp /= pow(LocalVel/Vref_,2.);
              
       if ( Mach_ < 1. ) {
          
          if ( Machref_ > 0. ) {
             
             LocalMach = Machref_*ABS(LocalVel)/Vref_;
             
             LocalMach = MIN(LocalMach,0.999);
             
          }
          
          else {
             
             LocalMach = Mach_*ABS(LocalVel)/Vref_;
             
             LocalMach = MIN(LocalMach,0.999);
             
          }
          
       }
       
       CpMinLoc = -999.;
       
       if ( LocalMach < 1. ) {
             
          CpMinLoc = -1./(0.7*LocalMach*LocalMach);
          
       }
             
       if ( ( ModelType_ == VLM_MODEL   ) && ABS(LocalCp) >= ABS(CpMinLoc - VSPGeom().Grid(1).LoopList(j).LocalFreeStreamVelocity(3)) ) {
          
          VortexLoop(j).dCp() = SGN(VortexLoop(j).dCp()) * ABS(CpMinLoc - VSPGeom().Grid(1).LoopList(j).LocalFreeStreamVelocity(3));
       
       }
       
       if ( ( ModelType_ == PANEL_MODEL ) && LocalCp <= CpMinLoc - VSPGeom().Grid(1).LoopList(j).LocalFreeStreamVelocity(3) ) {
          
          VortexLoop(j).dCp() = CpMinLoc - VSPGeom().Grid(1).LoopList(j).LocalFreeStreamVelocity(3);
       
       }
       
    }
    
    // Clip pressures... over lifting surfaces used Carlson's correlation
             
    if ( Clmax_2d_ < -998. ) {

       for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {
   
          // Wing Surface
 
          if ( VortexLoop(j).DegenWingID() > 0 || VortexLoop(j).VortexSheet() > 0 ) {
                          
             if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {
                
                SurfaceID = VortexLoop(j).SurfaceID();
              
                SpanStation = VortexLoop(j).SpanStation();
                
             }
             
             else {
             
                SurfaceID = VortexLoop(j).VortexSheet();
             
                SpanStation = VortexLoop(j).SpanStation();
      
             }
             
             LocalCp = VortexLoop(j).dCp();
             
             // Check for stall
      
             if ( ( ModelType_ == VLM_MODEL   ) && ABS(LocalCp) >= ABS(SpanLoadData(SurfaceID).Span_CpMin(SpanStation) - VSPGeom().Grid(1).LoopList(j).LocalFreeStreamVelocity(3)) ) {
                
                VortexLoop(j).dCp() = SGN(VortexLoop(j).dCp()) * ABS(SpanLoadData(SurfaceID).Span_CpMin(SpanStation));
         
             }

             if ( ( ModelType_ == PANEL_MODEL ) && LocalCp <= SpanLoadData(SurfaceID).Span_CpMin(SpanStation) - VSPGeom().Grid(1).LoopList(j).LocalFreeStreamVelocity(3)  ) {
                              
                VortexLoop(j).dCp() = SpanLoadData(SurfaceID).Span_CpMin(SpanStation) - VSPGeom().Grid(1).LoopList(j).LocalFreeStreamVelocity(3);

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
 
    CDTrefftz_[LoadCase] /= 0.5*Sref_;
 
    // Keep track of component group forces and moments
 
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       ComponentGroupList_[c].Cx() /= 0.5*Sref_; 
       ComponentGroupList_[c].Cy() /= 0.5*Sref_; 
       ComponentGroupList_[c].Cz() /= 0.5*Sref_; 

       ComponentGroupList_[c].Cmx() /= 0.5*Bref_*Sref_;
       ComponentGroupList_[c].Cmy() /= 0.5*Cref_*Sref_;
       ComponentGroupList_[c].Cmz() /= 0.5*Bref_*Sref_;

       // Now calculate CL, CD, CS
       
       ComponentGroupList_[c].CL() /= 0.5*Sref_;
       ComponentGroupList_[c].CD() /= 0.5*Sref_;
       ComponentGroupList_[c].CS() /= 0.5*Sref_;

       ComponentGroupList_[c].CLo() = (-ComponentGroupList_[c].Cxo() * SA + ComponentGroupList_[c].Czo() * CA );
       ComponentGroupList_[c].CDo() = ( ComponentGroupList_[c].Cxo() * CA + ComponentGroupList_[c].Czo() * SA ) * CB - ComponentGroupList_[c].Cyo() * SB;
       ComponentGroupList_[c].CSo() = ( ComponentGroupList_[c].Cxo() * CA + ComponentGroupList_[c].Czo() * SA ) * SB + ComponentGroupList_[c].Cyo() * CB;
 
       // Store time history of span loading for reporting and averaging later
       
       if ( ( TimeAccurate_ && CurrentTime_ >= ComponentGroupList_[c].StartAveragingTime() ) || ( !TimeAccurate_ && CurrentWakeIteration_ >= WakeIterations_ ) ) {

          for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfLiftingSurfaces() ; j++ ) {
          
             AveragingHasStarted_ = 1;
          
             i = ComponentGroupList_[c].SpanLoadData(j).SurfaceID();
          
             t = ++(ComponentGroupList_[c].SpanLoadData(j).ActualTimeSamples());

             if ( ComponentGroupList_[c].GeometryIsARotor() ) {

                for ( k = 1 ; k <= ComponentGroupList_[c].SpanLoadData(j).NumberOfSpanStations() ; k++ ) {

                   // Time and angular location
              
                   ComponentGroupList_[c].SpanLoadData(j).Time(t,k) = CurrentTime_;

                   ComponentGroupList_[c].SpanLoadData(j).RotationAngle(t,k) = ComponentGroupList_[c].TotalRotationAngle();
  
                   // Flat plate normal
       
                   nvec[0] = SpanLoadData(i).Span_Nvec(k)[0];
                   nvec[1] = SpanLoadData(i).Span_Nvec(k)[1];
                   nvec[2] = SpanLoadData(i).Span_Nvec(k)[2];
                   
                   // Vector from leading to trailing edge
           
                   Chord = SpanLoadData(i).Span_Chord(k);
       
                   svec[0] = SpanLoadData(i).Span_Svec(k)[0];
                   svec[1] = SpanLoadData(i).Span_Svec(k)[1];
                   svec[2] = SpanLoadData(i).Span_Svec(k)[2];
         
                   Diameter = ComponentGroupList_[c].RotorDiameter();
                   
                   RPM = ComponentGroupList_[c].Omega() * 60 / ( 2.*PI );
                                    
                   // Quarter chord location
                   
                   ComponentGroupList_[c].SpanLoadData(j).X_QC(t,k) = SpanLoadData(i).Span_XLE(j) + 0.25*Chord*svec[0];
                   ComponentGroupList_[c].SpanLoadData(j).Y_QC(t,k) = SpanLoadData(i).Span_YLE(j) + 0.25*Chord*svec[1];
                   ComponentGroupList_[c].SpanLoadData(j).Z_QC(t,k) = SpanLoadData(i).Span_ZLE(j) + 0.25*Chord*svec[2];
           
                   // Viscous Forces

                   dF[0] = SpanLoadData(i).Span_Cxo(k) * 0.5 * Density_ * Vref_ * Vref_ * SpanLoadData(i).Span_Area(k);
                   dF[1] = SpanLoadData(i).Span_Cyo(k) * 0.5 * Density_ * Vref_ * Vref_ * SpanLoadData(i).Span_Area(k);
                   dF[2] = SpanLoadData(i).Span_Czo(k) * 0.5 * Density_ * Vref_ * Vref_ * SpanLoadData(i).Span_Area(k);
                                
                   Thrust = vector_dot(dF, ComponentGroupList_[c].RVec());
                   
                   // Viscous Moments

                   ComponentCg[0] = ComponentGroupList_[c].OVec(0);
                   ComponentCg[1] = ComponentGroupList_[c].OVec(1);
                   ComponentCg[2] = ComponentGroupList_[c].OVec(2);

                   dM[0] = SpanLoadData(i).Span_Cmxo(k) * 0.5 * Density_ * Vref_ * Vref_ * SpanLoadData(i).Span_Area(k) * Chord;
                   dM[1] = SpanLoadData(i).Span_Cmyo(k) * 0.5 * Density_ * Vref_ * Vref_ * SpanLoadData(i).Span_Area(k) * Chord;
                   dM[2] = SpanLoadData(i).Span_Cmzo(k) * 0.5 * Density_ * Vref_ * Vref_ * SpanLoadData(i).Span_Area(k) * Chord;   
    
                   dM[0] -= dF[2] * ( ComponentCg[1] - XYZcg_[1] ) - dF[1] * ( ComponentCg[2] - XYZcg_[2] );
                   dM[1] -= dF[0] * ( ComponentCg[2] - XYZcg_[2] ) - dF[2] * ( ComponentCg[0] - XYZcg_[0] );
                   dM[2] -= dF[1] * ( ComponentCg[0] - XYZcg_[0] ) - dF[0] * ( ComponentCg[1] - XYZcg_[1] );
     
                   Moment = -vector_dot(dM, ComponentGroupList_[c].RVec());     
     
                   CalculateRotorCoefficientsFromForces(Thrust, Moment, Diameter, RPM, J, CT, CQ, CP, EtaP, CT_h, CQ_h, CP_h, FOM);    
            
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cno(t,k) = vector_dot(nvec, dF) * CT_h / Thrust;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cso(t,k) = vector_dot(svec, dF) * CT_h / Thrust;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cto(t,k) = CT_h;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cqo(t,k) = CQ_h;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cpo(t,k) = CP_h;

                   // Inviscid forces

                   dF[0] = SpanLoadData(i).Span_Cx(k) * 0.5 * Density_ * Vref_ * Vref_ * SpanLoadData(i).Span_Area(k);
                   dF[1] = SpanLoadData(i).Span_Cy(k) * 0.5 * Density_ * Vref_ * Vref_ * SpanLoadData(i).Span_Area(k);
                   dF[2] = SpanLoadData(i).Span_Cz(k) * 0.5 * Density_ * Vref_ * Vref_ * SpanLoadData(i).Span_Area(k);
                                        
                   Thrust = vector_dot(dF, ComponentGroupList_[c].RVec());
           
                   // Inviscid Moments
   
                   ComponentCg[0] = ComponentGroupList_[c].OVec(0);
                   ComponentCg[1] = ComponentGroupList_[c].OVec(1);
                   ComponentCg[2] = ComponentGroupList_[c].OVec(2);
                   
                   dM[0] = SpanLoadData(i).Span_Cmx(k) * 0.5 * Density_ * Vref_ * Vref_ * SpanLoadData(i).Span_Area(k) * Chord;
                   dM[1] = SpanLoadData(i).Span_Cmy(k) * 0.5 * Density_ * Vref_ * Vref_ * SpanLoadData(i).Span_Area(k) * Chord;
                   dM[2] = SpanLoadData(i).Span_Cmz(k) * 0.5 * Density_ * Vref_ * Vref_ * SpanLoadData(i).Span_Area(k) * Chord;   
    
                   dM[0] -= dF[2] * ( ComponentCg[1] - XYZcg_[1] ) - dF[1] * ( ComponentCg[2] - XYZcg_[2] );
                   dM[1] -= dF[0] * ( ComponentCg[2] - XYZcg_[2] ) - dF[2] * ( ComponentCg[0] - XYZcg_[0] );
                   dM[2] -= dF[1] * ( ComponentCg[0] - XYZcg_[0] ) - dF[0] * ( ComponentCg[1] - XYZcg_[1] );
     
                   Moment = -vector_dot(dM, ComponentGroupList_[c].RVec());     
                  
                   CalculateRotorCoefficientsFromForces(Thrust, Moment, Diameter, RPM, J, CT, CQ, CP, EtaP, CT_h, CQ_h, CP_h, FOM);    
   
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cn(t,k) = vector_dot(nvec, dF) * CT_h / Thrust;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cs(t,k) = vector_dot(svec, dF) * CT_h / Thrust;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Ct(t,k) = CT_h;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cq(t,k) = CQ_h;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cp(t,k) = CP_h;
                                    
                   ComponentGroupList_[c].SpanLoadData(j).Span_Area(t,k)  = SpanLoadData(i).Span_Area(k);
                   ComponentGroupList_[c].SpanLoadData(j).Span_Chord(t,k) = Chord;
                   ComponentGroupList_[c].SpanLoadData(j).Span_S(t,k)     = SpanLoadData(i).Span_S(k);
                                                      
                   ComponentGroupList_[c].SpanLoadData(j).Local_Velocity(t,k) = SpanLoadData(i).Span_Local_Velocity(k)[3];            
                                    
                }
                
             }
           
             // Just a wing, or also a rotor blade... keep track of wingy things
             
             if ( ComponentGroupList_[c].GeometryHasWings() ) {
  
                for ( k = 1 ; k <= ComponentGroupList_[c].SpanLoadData(j).NumberOfSpanStations() ; k++ ) {

                   // Time and angular location
              
                   ComponentGroupList_[c].SpanLoadData(j).Time(t,k) = CurrentTime_;
     
                   // Flat plate normal
       
                   nvec[0] = SpanLoadData(i).Span_Nvec(k)[0];
                   nvec[1] = SpanLoadData(i).Span_Nvec(k)[1];
                   nvec[2] = SpanLoadData(i).Span_Nvec(k)[2];
                   
                   // Vector from leading to trailing edge
           
                   Chord = SpanLoadData(i).Span_Chord(k);
       
                   svec[0] = SpanLoadData(i).Span_Svec(k)[0];
                   svec[1] = SpanLoadData(i).Span_Svec(k)[1];
                   svec[2] = SpanLoadData(i).Span_Svec(k)[2];

                   // Quarter chord location
                   
                   ComponentGroupList_[c].SpanLoadData(j).X_QC(t,k) = SpanLoadData(i).Span_XLE(j) + 0.25*Chord*svec[0];
                   ComponentGroupList_[c].SpanLoadData(j).Y_QC(t,k) = SpanLoadData(i).Span_YLE(j) + 0.25*Chord*svec[1];
                   ComponentGroupList_[c].SpanLoadData(j).Z_QC(t,k) = SpanLoadData(i).Span_ZLE(j) + 0.25*Chord*svec[2];

                   Cx = SpanLoadData(i).Span_Cxo(k) + SpanLoadData(i).Span_Cx(k);
                   Cy = SpanLoadData(i).Span_Cyo(k) + SpanLoadData(i).Span_Cy(k);
                   Cz = SpanLoadData(i).Span_Czo(k) + SpanLoadData(i).Span_Cz(k);
                   
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cx(t,k) = Cx;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cy(t,k) = Cy;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cz(t,k) = Cz;
                                            
                   Cl =  (-Cx * SA +  Cz * CA );
                   Cd =  ( Cx * CA +  Cz * SA ) * CB -  Cy * SB;   
                   Cs =  ( Cx * CA +  Cz * SA ) * SB +  Cy * CB;
       
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cl(t,k) = Cl;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cd(t,k) = Cd;
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cw(t,k) = Cs;
                   
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cmx(t,k) = SpanLoadData(i).Span_Cmxo(k) + SpanLoadData(i).Span_Cmx(k);
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cmy(t,k) = SpanLoadData(i).Span_Cmyo(k) + SpanLoadData(i).Span_Cmy(k);
                   ComponentGroupList_[c].SpanLoadData(j).Span_Cmz(t,k) = SpanLoadData(i).Span_Cmzo(k) + SpanLoadData(i).Span_Cmz(k);                      
                                                          
                   ComponentGroupList_[c].SpanLoadData(j).Span_Area(t,k)  = SpanLoadData(i).Span_Area(k);
                   ComponentGroupList_[c].SpanLoadData(j).Span_Chord(t,k) = Chord;
                   ComponentGroupList_[c].SpanLoadData(j).Span_S(t,k)     = SpanLoadData(i).Span_S(k);
                                                      
                   ComponentGroupList_[c].SpanLoadData(j).Local_Velocity(t,k) = SpanLoadData(i).Span_Local_Velocity(k)[3];            
                                    
                }
                
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

       CDTrefftz_[LoadCase] *= 2.; 
       
    }
        
    if ( DoSymmetryPlaneSolve_ == SYM_X ) CMy_[LoadCase] = CMz_[LoadCase] = CFx_[LoadCase] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Y ) CMx_[LoadCase] = CMz_[LoadCase] = CFy_[LoadCase] = CS_[LoadCase] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Z ) CMx_[LoadCase] = CMy_[LoadCase] = CFz_[LoadCase] = 0.;
              
    // VLM Model - Loop over body surfaces and calculate skin friction drag
    
    if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE) {
    
       for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
        
          if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
             
             Length = VSPGeom().VSP_Surface(i).AverageChord();
            
             Re = ReCref_ * Length / Cref_;
    
             Cf = 0.455 / pow(log10(Re),2.58);
            
             Swet = VSPGeom().VSP_Surface(i).WettedArea();
             
             // Body is split into 4 parts, wetted area over accounted for... hence the 0.25
 
             dCD = 0.25 * Cf * Swet / Sref_;
             
             // Bump for form drag
   
             FRatio = Length * Length * PI / Swet;
             
             FFactor = 1. + 60./pow(FRatio,3.) + FRatio/400.;

             dCD *= FFactor;
             
             if ( DoSymmetryPlaneSolve_ ) dCD *= 2.; // We are only doing half the longitudinal
                                                     // The lateral parts are added in below with
                                                     // another 2x 
   
             // Save at component level
   
             VSPGeom().VSP_Surface(i).CDo() = dCD;
             
             // Total drag
      
             CDo_[LoadCase] += dCD;
   
          }
    
       }
       
    }
    
    // Panel model, everything not-a-wing is lumped into span station 0
    
    else {
     
       Re = ReCref_;
       
       Cf = 0.455 / pow(log10(Re),2.58);
       
       Swet = SpanLoadData(0).Span_Area(1);
       
       // Bump by 25% for miscellaneous
       
       dCD = 1.25 * Cf * Swet / ( 0.5*Sref_ );
       
       // Total drag
       
       CDo_[LoadCase] += dCD;

    }       
       
    // Adjust for symmetry
    
    if ( DoSymmetryPlaneSolve_  ) {
       
       CDo_[LoadCase] *= 2.;
       
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

    // Update averaging for steady and quasi steady cases
    
    if ( TimeAccurate_ && AveragingHasStarted_ ) {
       
       NumberOfAveragingSets_++;
       
       CFx_[2] += CFx_[0];
       CFy_[2] += CFy_[0];
       CFz_[2] += CFz_[0];
                  
       CMx_[2] += CMx_[0];
       CMy_[2] += CMy_[0];
       CMz_[2] += CMz_[0];
                  
        CL_[2] +=  CL_[0]; 
        CD_[2] +=  CD_[0]; 
        CS_[2] +=  CS_[0]; 
        
       CDo_[2] += CDo_[0];
       
       CDTrefftz_[2] += CDTrefftz_[0];
       
       
    }

}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER SurfaceVortexEdgeIsBetweenPlanes               #
#                                                                              #
##############################################################################*/

int VSP_SOLVER::SurfaceVortexEdgeIsBetweenPlanes(VSPAERO_DOUBLE *Normal1, VSPAERO_DOUBLE *Normal1Point,
                                                 VSPAERO_DOUBLE *Normal2, VSPAERO_DOUBLE *Normal2Point,
                                                 int Edge, VSPAERO_DOUBLE &Weight) {
  

    int Type1, Type2;
    int N1_P1 = 0, N1_P2 = 0, N2_P1 = 0, N2_P2 = 0;
    VSPAERO_DOUBLE Point1[3], Point2[3], S1, S2;
    
    // Define the line
    
    Point1[0] = SurfaceVortexEdge(Edge).X1();
    Point1[1] = SurfaceVortexEdge(Edge).Y1();
    Point1[2] = SurfaceVortexEdge(Edge).Z1();

    Point2[0] = SurfaceVortexEdge(Edge).X2();
    Point2[1] = SurfaceVortexEdge(Edge).Y2();
    Point2[2] = SurfaceVortexEdge(Edge).Z2();
 
    // Check if any point of this segment is between the two planes
 
    if ( PointIsOnRightSideOfPlane(Normal1, Normal1Point, Point1) >= 0. ) N1_P1 = 1;
    if ( PointIsOnRightSideOfPlane(Normal1, Normal1Point, Point2) >= 0. ) N1_P2 = 1;
    
    if ( PointIsOnRightSideOfPlane(Normal2, Normal2Point, Point1) <= 0. ) N2_P1 = 1;
    if ( PointIsOnRightSideOfPlane(Normal2, Normal2Point, Point2) <= 0. ) N2_P2 = 1;    
    
    // Points are between two planes, we are done
    
    if ( N1_P1 == 1 && N1_P2 == 1 && N2_P1 == 1 && N2_P2 == 1 ) { Weight = 1. ; return 1; };
                                                                
    // Points are to the left of both planes                    
                                                                
    if ( N1_P1 == 0 && N1_P2 == 0 && N2_P1 == 1 && N2_P2 == 1 ) { Weight = 0. ; return 0; };
                                                                
    // Points are to the right of both planes                   
                                                                
    if ( N1_P1 == 1 && N1_P2 == 1 && N2_P1 == 0 && N2_P2 == 0 ) { Weight = 0. ; return 0; };
    
    // Otherwise, check for intersections

    Type1 = PlaneSegmentIntersection(Normal1, Normal1Point, Point1, SurfaceVortexEdge(Edge).Vec(), S1);
       
    // Segment lines in the plane of the plane   

    if ( Type1 == 0 ) { Weight = 1. ; return 1; };
       
    // Segement is parallel with plane, but does not intersect it
    
    if ( Type1 == -1 ) { Weight = 1. ; return 1; };
    
    Type2 = PlaneSegmentIntersection(Normal2, Normal2Point, Point1, SurfaceVortexEdge(Edge).Vec(), S2);
    
    // Segment lines in the plane of the plane   

    if ( Type2 == 0 ) { Weight = 1. ; return 1; };
       
    // Segement is parallel with plane, but does not intersect it
    
    if ( Type2 == -1 ) { Weight = 1. ; return 1; };    
    
    // If we made it this far... then the line is cut by both planes
    
    S1 = MIN(1.,MAX(S1,0.));
    
    S2 = MIN(1.,MAX(S2,0.));
    
    Weight = ABS(S2-S1);
    
    return 1;

}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CalculateSpanWiseLoading                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateSpanWiseLoading(void)
{
 
    int i, k, NumSurfs, NumberOfStations = 0;
    VSPAERO_DOUBLE TotalLift, CFx, CFy, CFz;
    VSPAERO_DOUBLE CL, CD, CS, CMx, CMy, CMz;
    char DumChar[2000];
    
    // Write out generic header
    
    WriteCaseHeader(LoadFile_);
    
    // Write out column labels
    
                    // 123456789 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 
    FPRINTF(LoadFile_, "   Wing       S            Xavg          Yavg           Zavg          Chord        V/Vref          Cl            Cd            Cs            Cx            Cy           Cz            Cmx           Cmy           Cmz \n");

    TotalLift = 0.;  

    for ( i = StartOfSpanLoadDataSets_ ; i <= NumberOfSpanLoadDataSets_ ; i++ ) { 
      
       if ( ModelType_ == VLM_MODEL && SurfaceType_ != VSPGEOM_SURFACE ) {
          
          NumberOfStations = 1;
          
          if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
             
             NumberOfStations = SpanLoadData(i).NumberOfSpanStations();
       
          }

       }
       
       else {
          
          if ( ModelType_ == PANEL_MODEL || SurfaceType_ == VSPGEOM_SURFACE ) {
          
             NumberOfStations = SpanLoadData(i).NumberOfSpanStations();
             
          }

       }
     
       if ( NumberOfStations > 1 ) {
        
          for ( k = 1 ; k <= NumberOfStations ; k++ ) {

              FPRINTF(LoadFile_, "%9d % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E \n",
                     i,
                     SpanLoadData(i).Span_S(k),                    
                     SpanLoadData(i).Span_Xavg(k),
                     SpanLoadData(i).Span_Yavg(k),
                     SpanLoadData(i).Span_Zavg(k),                     
                     SpanLoadData(i).Span_Chord(k),
                     SpanLoadData(i).Span_Local_Velocity(k)[3],
                     SpanLoadData(i).Span_Cl(k),
                     SpanLoadData(i).Span_Cd(k),
                     SpanLoadData(i).Span_Cs(k),
                     SpanLoadData(i).Span_Cx(k),
                     SpanLoadData(i).Span_Cy(k),
                     SpanLoadData(i).Span_Cz(k),
                     SpanLoadData(i).Span_Cmx(k),
                     SpanLoadData(i).Span_Cmy(k),
                     SpanLoadData(i).Span_Cmz(k));
            
             TotalLift += 0.5 * SpanLoadData(i).Span_Cl(k) * SpanLoadData(i).Span_Area(k);

          }
          
       }
       
    }
    
    
    FPRINTF(LoadFile_,"\n\n\n");

                    // 1234567890123 123456789012345678901234567890123456789 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123   
    FPRINTF(LoadFile_, "Comp         Component-Name                             Mach           AoA          Beta            CL             CDi           CS            CFx           CFy           CFz         Cmx           Cmy           Cmz \n");

    for ( i = StartOfSpanLoadDataSets_ ; i <= NumberOfSpanLoadDataSets_ ; i++ ) { 
          
       NumberOfStations = SpanLoadData(i).NumberOfSpanStations();
  
       if ( NumberOfStations > 1 ) {
          
          CL = CD = CS = CFx = CFy = CFz = CMx = CMy = CMz = 0.;
          
          for ( k = 1 ; k <= NumberOfStations ; k++ ) {

             CL  += 0.5 *  SpanLoadData(i).Span_Cl(k) * SpanLoadData(i).Span_Area(k);
             CD  += 0.5 *  SpanLoadData(i).Span_Cd(k) * SpanLoadData(i).Span_Area(k);
             CS  += 0.5 *  SpanLoadData(i).Span_Cs(k) * SpanLoadData(i).Span_Area(k);
             CFx += 0.5 *  SpanLoadData(i).Span_Cx(k) * SpanLoadData(i).Span_Area(k);
             CFy += 0.5 *  SpanLoadData(i).Span_Cy(k) * SpanLoadData(i).Span_Area(k);
             CFz += 0.5 *  SpanLoadData(i).Span_Cz(k) * SpanLoadData(i).Span_Area(k);
             
             CMx += 0.5 * SpanLoadData(i).Span_Cmx(k) * SpanLoadData(i).Span_Area(k) * SpanLoadData(i).Span_Chord(k);
             CMy += 0.5 * SpanLoadData(i).Span_Cmy(k) * SpanLoadData(i).Span_Area(k) * SpanLoadData(i).Span_Chord(k);
             CMz += 0.5 * SpanLoadData(i).Span_Cmz(k) * SpanLoadData(i).Span_Area(k) * SpanLoadData(i).Span_Chord(k);
      
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
          
          if ( ModelType_ == VLM_MODEL ) {
   
             if ( SurfaceType_ != VSPGEOM_SURFACE ) {
                
                SPRINTF(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
                
             }
             
             else {
                
                SPRINTF(DumChar,"%s",VSPGeom().VSP_Surface(1).ComponentName());
                
             }                
             
          }
          
          else {
             
             SPRINTF(DumChar,"Wake-Sheet-%-d",i);
             
          }
           
          FPRINTF(LoadFile_, "%-9d %-40s % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E \n",
                  i,
                  DumChar,
                  Mach_,
                  FLOAT(AngleOfAttack_/TORAD),
                  FLOAT(AngleOfBeta_/TORAD),                 
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
       
       else {
        
          k = 1;
          
          if ( ModelType_ == VLM_MODEL ) {
       
             if ( SurfaceType_ != VSPGEOM_SURFACE ) {
                
                SPRINTF(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());

             }
             
             else {
                
                SPRINTF(DumChar,"%s",VSPGeom().VSP_Surface(1).ComponentName());
                
             }                  
             
          }
          
          else {
             
             SPRINTF(DumChar,"Not-A-Wing");
             
          }

          FPRINTF(LoadFile_,"%-9d %-40s % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E \n",
                  i,
                  DumChar,
                  Mach_,
                  FLOAT(AngleOfAttack_/TORAD),
                  FLOAT(AngleOfBeta_/TORAD),                                   
                  FLOAT(0.5 *   SpanLoadData(i).Span_Cl(k) * SpanLoadData(i).Span_Area(k) / (0.5*Sref_)),
                  FLOAT(0.5 *   SpanLoadData(i).Span_Cd(k) * SpanLoadData(i).Span_Area(k) / (0.5*Sref_)),
                  FLOAT(0.5 *   SpanLoadData(i).Span_Cs(k) * SpanLoadData(i).Span_Area(k) / (0.5*Sref_)),
                  FLOAT(0.5 *   SpanLoadData(i).Span_Cx(k) * SpanLoadData(i).Span_Area(k) / (0.5*Sref_)),
                  FLOAT(0.5 *   SpanLoadData(i).Span_Cy(k) * SpanLoadData(i).Span_Area(k) / (0.5*Sref_)),
                  FLOAT(0.5 *   SpanLoadData(i).Span_Cz(k) * SpanLoadData(i).Span_Area(k) / (0.5*Sref_)),
                  FLOAT(0.5 *  SpanLoadData(i).Span_Cmx(k) * SpanLoadData(i).Span_Area(k) * SpanLoadData(i).Span_Chord(k) / (0.5*Bref_*Sref_)),
                  FLOAT(0.5 *  SpanLoadData(i).Span_Cmy(k) * SpanLoadData(i).Span_Area(k) * SpanLoadData(i).Span_Chord(k) / (0.5*Cref_*Sref_)),
                  FLOAT(0.5 *  SpanLoadData(i).Span_Cmz(k) * SpanLoadData(i).Span_Area(k) * SpanLoadData(i).Span_Chord(k) / (0.5*Bref_*Sref_)));
         
          TotalLift += 0.5 * SpanLoadData(i).Span_Cl(k) * SpanLoadData(i).Span_Area(k);

       }
                 
    }
    
    FPRINTF(LoadFile_,"\n\n\n");

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
    
       SPRINTF(LoadFileName,"%s.fem",FileName_);
       
       if ( (FEMLoadFile_ = fopen(LoadFileName, "w")) == NULL ) {
   
          PRINTF("Could not open the fem load file for output! \n");
   
          exit(1);
   
       }
       
    }
    
    WriteCaseHeader(FEMLoadFile_);
       
    // Write out FEM beam load file for VLM model
    
    if ( ModelType_ == VLM_MODEL ) CreateFEMLoadFileFromVLMSolve(Case);

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
    VSPAERO_DOUBLE Vec[3], VecQC[3], VecQC_Def[3], RVec[3], Force[3], Moment[3], Chord;    

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( 1||VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
        
          FPRINTF(FEMLoadFile_,"Wing Surface: %d \n",i);
          FPRINTF(FEMLoadFile_,"SpanStations: %d \n",VSPGeom().VSP_Surface(i).NumberOfSpanStations());
          FPRINTF(FEMLoadFile_,"\n");
          
          //                    123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789  
          FPRINTF(FEMLoadFile_,"   Wing    XLE_ORIG  YLE_ORIG  ZLE_ORIG  XTE_ORIG  YTE_ORIG  ZTE_ORIG  XQC_ORIG  YQC_ORIG  ZQC_ORIG  S_ORIG     Area     Chord     XLE_DEF   YLE_DEF   ZLE_DEF   XTE_DEF   YTE_DEF   ZTE_DEF   XQC_DEF   YQC_DEF   ZQC_DEF    S_DEF       Cl        Cd        Cs        Cx        Cy        Cz       Cmx       Cmy       Cmz \n");

          NumberOfStations = SpanLoadData(i).NumberOfSpanStations();
              
          for ( k = 1 ; k <= NumberOfStations ; k++ ) {

             // Calculate local deformed quarter chord location
             
             Vec[0] = SpanLoadData(i).Span_XTE_Def(k) - SpanLoadData(i).Span_XLE_Def(k);
             Vec[1] = SpanLoadData(i).Span_YTE_Def(k) - SpanLoadData(i).Span_YLE_Def(k);
             Vec[2] = SpanLoadData(i).Span_ZTE_Def(k) - SpanLoadData(i).Span_ZLE_Def(k);
                  
             Chord = sqrt(vector_dot(Vec,Vec));
         
             Vec[0] /= Chord;
             Vec[1] /= Chord;
             Vec[2] /= Chord;

             VecQC_Def[0] = SpanLoadData(i).Span_XLE_Def(k) + 0.25*Chord*Vec[0];
             VecQC_Def[1] = SpanLoadData(i).Span_YLE_Def(k) + 0.25*Chord*Vec[1];
             VecQC_Def[2] = SpanLoadData(i).Span_ZLE_Def(k) + 0.25*Chord*Vec[2];
                       
             // Calculate local undeformed quarter chord location

             Vec[0] = SpanLoadData(i).Span_XTE(k) - SpanLoadData(i).Span_XLE(k);
             Vec[1] = SpanLoadData(i).Span_YTE(k) - SpanLoadData(i).Span_YLE(k);
             Vec[2] = SpanLoadData(i).Span_ZTE(k) - SpanLoadData(i).Span_ZLE(k);
             
             Chord = sqrt(vector_dot(Vec,Vec));

             Vec[0] /= Chord;
             Vec[1] /= Chord;
             Vec[2] /= Chord;

             VecQC[0] = SpanLoadData(i).Span_XLE(k) + 0.25*Chord*Vec[0];
             VecQC[1] = SpanLoadData(i).Span_YLE(k) + 0.25*Chord*Vec[1];
             VecQC[2] = SpanLoadData(i).Span_ZLE(k) + 0.25*Chord*Vec[2];
  
             // Transfer moments to the deformed, quarter chord location
          
             RVec[0] = XYZcg_[0] - VecQC_Def[0];
             RVec[1] = XYZcg_[1] - VecQC_Def[1];
             RVec[2] = XYZcg_[2] - VecQC_Def[2];
        
             Force[0] = SpanLoadData(i).Span_Cx(k);
             Force[1] = SpanLoadData(i).Span_Cy(k);
             Force[2] = SpanLoadData(i).Span_Cz(k);
             
             vector_cross(RVec,Force,Moment);
             
             // Add in the moments from the CG... YES ADD IN, we calculated the moments due to ref change above...

             Moment[0] += SpanLoadData(i).Span_Cmx(k) * SpanLoadData(i).Span_Chord(k);
             Moment[1] += SpanLoadData(i).Span_Cmy(k) * SpanLoadData(i).Span_Chord(k);
             Moment[2] += SpanLoadData(i).Span_Cmz(k) * SpanLoadData(i).Span_Chord(k);
                     
             // Re-nondimensionalize
             
             Moment[0] /= SpanLoadData(i).Span_Chord(k);
             Moment[1] /= SpanLoadData(i).Span_Chord(k);
             Moment[2] /= SpanLoadData(i).Span_Chord(k);

             // Note... all forces and moments are referenced to the local chord and local quarter chord
             // Ie... these are '2D' coefficients

             FPRINTF(FEMLoadFile_,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
                     i,
                     SpanLoadData(i).Span_XLE(k),
                     SpanLoadData(i).Span_YLE(k),
                     SpanLoadData(i).Span_ZLE(k),
                     SpanLoadData(i).Span_XTE(k),
                     SpanLoadData(i).Span_YTE(k),
                     SpanLoadData(i).Span_ZTE(k),    
                     VecQC[0],
                     VecQC[1],
                     VecQC[2],                                      
                     SpanLoadData(i).Span_S(k),
                     SpanLoadData(i).Span_Area(k),
                     SpanLoadData(i).Span_Chord(k),         
                     SpanLoadData(i).Span_XLE_Def(k),
                     SpanLoadData(i).Span_YLE_Def(k),
                     SpanLoadData(i).Span_ZLE_Def(k),
                     SpanLoadData(i).Span_XTE_Def(k),
                     SpanLoadData(i).Span_YTE_Def(k),
                     SpanLoadData(i).Span_ZTE_Def(k),
                     VecQC_Def[0],
                     VecQC_Def[1],
                     VecQC_Def[2],
                     SpanLoadData(i).Span_S_Def(k),
                     SpanLoadData(i).Span_Cl(k),
                     SpanLoadData(i).Span_Cd(k),
                     SpanLoadData(i).Span_Cs(k),
                     SpanLoadData(i).Span_Cx(k),
                     SpanLoadData(i).Span_Cy(k),
                     SpanLoadData(i).Span_Cz(k),
                     Moment[0],
                     Moment[1],
                     Moment[2]);
         
          }
          
          FPRINTF(FEMLoadFile_,"\n");
          FPRINTF(FEMLoadFile_,"   Planform:\n");
          FPRINTF(FEMLoadFile_,"\n");

          FPRINTF(FEMLoadFile_,"   Root LE: %9.5lf %9.5lf %9.5lf \n",SpanLoadData(i).Root_LE(0),SpanLoadData(i).Root_LE(1),SpanLoadData(i).Root_LE(2));
          FPRINTF(FEMLoadFile_,"   Root TE: %9.5lf %9.5lf %9.5lf \n",SpanLoadData(i).Root_TE(0),SpanLoadData(i).Root_TE(1),SpanLoadData(i).Root_TE(2));
          FPRINTF(FEMLoadFile_,"   Root QC: %9.5lf %9.5lf %9.5lf \n",SpanLoadData(i).Root_QC(0),SpanLoadData(i).Root_QC(1),SpanLoadData(i).Root_QC(2));

          FPRINTF(FEMLoadFile_,"\n");

          FPRINTF(FEMLoadFile_,"   Tip LE:  %9.5lf %9.5lf %9.5lf \n",SpanLoadData(i).Tip_LE(0),SpanLoadData(i).Tip_LE(1),SpanLoadData(i).Tip_LE(2));
          FPRINTF(FEMLoadFile_,"   Tip TE:  %9.5lf %9.5lf %9.5lf \n",SpanLoadData(i).Tip_TE(0),SpanLoadData(i).Tip_TE(1),SpanLoadData(i).Tip_TE(2));
          FPRINTF(FEMLoadFile_,"   Tip QC:  %9.5lf %9.5lf %9.5lf \n",SpanLoadData(i).Tip_QC(0),SpanLoadData(i).Tip_QC(1),SpanLoadData(i).Tip_QC(2));
                      
       }
                 
    }
    
    FPRINTF(FEMLoadFile_,"\n\n");
    FPRINTF(FEMLoadFile_,"Total Forces and Moments \n");
    FPRINTF(FEMLoadFile_,"\n\n");
     
    FPRINTF(FEMLoadFile_,"Total CL:  %lf \n", CL_[0]);
    FPRINTF(FEMLoadFile_,"Total CD:  %lf \n", CD_[0]);
    FPRINTF(FEMLoadFile_,"Total CS:  %lf \n", CS_[0]);
    
    FPRINTF(FEMLoadFile_,"Total CFx: %lf \n", CFx_[0]);
    FPRINTF(FEMLoadFile_,"Total CFy: %lf \n", CFy_[0]);
    FPRINTF(FEMLoadFile_,"Total CFz: %lf \n", CFz_[0]);

    FPRINTF(FEMLoadFile_,"Total CMx: %lf \n", CMx_[0]);
    FPRINTF(FEMLoadFile_,"Total CMy: %lf \n", CMy_[0]);
    FPRINTF(FEMLoadFile_,"Total CMz: %lf \n", CMz_[0]);

}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER CreateFEMLoadFileFromPanelSolve                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateFEMLoadFileFromPanelSolve(int Case)
{
 
    int i, j, k, Node, Node1, Node2, *OnVortexSheet;
    VSPAERO_DOUBLE *Fx, *Fy, *Fz, fx, fy, fz, Cl, Cd, Cs;
    VSPAERO_DOUBLE CA, SA, CB, SB;

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
        
    // Mark nodes on trailing edge of each lifting surface
    
    OnVortexSheet = new int[VSPGeom().Grid(1).NumberOfNodes() + 1];
    
    zero_int_array(OnVortexSheet, VSPGeom().Grid(1).NumberOfNodes());
    
    Fx  = new VSPAERO_DOUBLE[VSPGeom().Grid(1).NumberOfNodes() + 1];
    Fy  = new VSPAERO_DOUBLE[VSPGeom().Grid(1).NumberOfNodes() + 1];
    Fz  = new VSPAERO_DOUBLE[VSPGeom().Grid(1).NumberOfNodes() + 1];
    
    zero_double_array(Fx, VSPGeom().Grid(1).NumberOfNodes());
    zero_double_array(Fy, VSPGeom().Grid(1).NumberOfNodes());
    zero_double_array(Fz, VSPGeom().Grid(1).NumberOfNodes());
        
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          Node = VortexSheet(k).TrailingVortex(i).Node();

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
    FPRINTF(FEMLoadFile_,"   Wing       X         Y         Z         Cx        Cy        Cz        CL        CD        CS \n");

    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          Node = VortexSheet(k).TrailingVortex(i).Node();
          
          // Nodal forces, non-dimensionalized by Sref
          
          fx = Fx[Node] / (0.5*Sref_*Vref_*Vref_);
          fy = Fy[Node] / (0.5*Sref_*Vref_*Vref_);
          fz = Fz[Node] / (0.5*Sref_*Vref_*Vref_);
          
          // Nodal force coefficients, again non-dimensionalized by Sref
          
          Cl =   ( -fx * SA + fz * CA );
          Cd = ( (  fx * CA + fz * SA ) * CB - fy * SB );
          Cs = ( (  fx * CA + fz * SA ) * SB + fy * CB );

          FPRINTF(FEMLoadFile_,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf\n",
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
    
    FPRINTF(FEMLoadFile_,"\n");
    FPRINTF(FEMLoadFile_,"Note: Force coefficients are NOT 2D - they are the full 3D forces, non-dimensionalized by Q * Sref \n");

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
    
    SPRINTF(LoadFileName,"%s.fem2d",FileName_);
    
    if ( (FEM2DLoadFile_ = fopen(LoadFileName, "w")) == NULL ) {

       PRINTF("Could not open the fem load file for output! \n");

       exit(1);

    }
    
    // Write out header 

    number_of_tris  = VSPGeom().Grid().NumberOfLoops();
    number_of_nodes = VSPGeom().Grid().NumberOfNodes();

    FPRINTF(FEM2DLoadFile_,"NumberOfNodes:%d \n",number_of_nodes);
    FPRINTF(FEM2DLoadFile_,"NumberOfTris: %d \n",number_of_tris);

    // Write out node data
    
    FPRINTF(FEM2DLoadFile_,"Nodal data: \n");
    FPRINTF(FEM2DLoadFile_,"X, Y, Z \n");

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfNodes() ; j++ ) {

       FPRINTF(FEM2DLoadFile_,"%f %f %f \n",
        VSPGeom().Grid().NodeList(j).x(),
        VSPGeom().Grid().NodeList(j).y(),
        VSPGeom().Grid().NodeList(j).z());
       
    }
        
    // Write out triangulated surface mesh
    
    FPRINTF(FEM2DLoadFile_,"Tri data: \n");
    FPRINTF(FEM2DLoadFile_,"Node1, Node2, Node3, SurfType, SurfID, Area, Nx, Ny, Nz \n");

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {

       SurfaceID   = VSPGeom().Grid().LoopList(j).DegenBodyID()
                   + VSPGeom().Grid().LoopList(j).DegenWingID()
                   + VSPGeom().Grid().LoopList(j).Cart3dID();
                 
       FPRINTF(FEM2DLoadFile_,"%d %d %d %d %d %f %f %f %f \n",
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

    FPRINTF(FEM2DLoadFile_,"\n");
    
    WriteCaseHeader(FEM2DLoadFile_);
            
    // Write out solution

    FPRINTF(FEM2DLoadFile_,"Solution Data\n");    
    FPRINTF(FEM2DLoadFile_,"Tri, DeltaCp_or_Cp \n");

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {

       FPRINTF(FEM2DLoadFile_,"%d %f \n", j, VSPGeom().Grid().LoopList(j).dCp()/(0.5*Vref_*Vref_));

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
    VSPAERO_DOUBLE xyz[3], q[5];
    VSPAERO_DOUBLE *U, *V, *W;
    char SurveyFileName[2000];
    FILE *SurveyFile;
    
    U = new VSPAERO_DOUBLE[NumberofSurveyPoints_ + 1];
    V = new VSPAERO_DOUBLE[NumberofSurveyPoints_ + 1];
    W = new VSPAERO_DOUBLE[NumberofSurveyPoints_ + 1];

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
   
             VortexSheet(p).TrailingVortex(k).InducedVelocity(xyz, q);
                
             U[i] += q[0];
             V[i] += q[1];
             W[i] += q[2];
               
             // If ground effects... add in ground effects ... z plane
   
             if ( DoGroundEffectsAnalysis() ) {
   
                xyz[0] = SurveyPointList(i).x();
                xyz[1] = SurveyPointList(i).y();
                xyz[2] = SurveyPointList(i).z();
                        
                xyz[2] *= -1.;
               
                VortexSheet(p).TrailingVortex(k).InducedVelocity(xyz, q);
      
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
               
                VortexSheet(p).TrailingVortex(k).InducedVelocity(xyz, q);
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
               
                U[i] += q[0];
                V[i] += q[1];
                W[i] += q[2];
                
                // If ground effects... add in ground effects ... z plane
      
                if ( DoGroundEffectsAnalysis() ) {
  
                   xyz[2] *= -1.;
                  
                   VortexSheet(p).TrailingVortex(k).InducedVelocity(xyz, q);
         
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
    
    SPRINTF(SurveyFileName,"%s.svy",FileName_);
    
    if ( (SurveyFile = fopen(SurveyFileName, "w")) == NULL ) {

       PRINTF("Could not open the survey file for output! \n");

       exit(1);

    }    
                       //0123456789x0123456789x0123456789x   0123456789x0123456789x0123456789x 
    FPRINTF(SurveyFile, "     X          Y          Z             U          V          W \n");

    for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {

       xyz[0] = SurveyPointList(i).x();
       xyz[1] = SurveyPointList(i).y();
       xyz[2] = SurveyPointList(i).z();
       
       FPRINTF(SurveyFile, "%10.5f %10.5f%10.5f    %10.5f %10.5f %10.5f \n",
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
    
    float Sref = FLOAT( Sref_ );
    float Cref = FLOAT( Cref_ );
    float Bref = FLOAT( Bref_ );
    float X_cg = FLOAT( XYZcg_[0] );
    float Y_cg = FLOAT( XYZcg_[1] );
    float Z_cg = FLOAT( XYZcg_[2] );

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Write out coded id to allow us to determine endiannes of files

    DumInt = -123789456; // Version 2 of the ADB file

    FWRITE(&DumInt, i_size, 1, ADBFile_);
    
    // Write out model type... VLM or PANEL
    
    FWRITE(&ModelType_, i_size, 1, ADBFile_);

    // Write out symmetry flag
    
    FWRITE(&DoSymmetryPlaneSolve_, i_size, 1, ADBFile_);

    // Write out unsteady analysis flag
    
    if ( TimeAccurate_) {
    
       DumInt = 1;
    
       FWRITE(&DumInt, i_size, 1, ADBFile_);    
       
    }
    
    else {
       
       DumInt = 0;
       
       FWRITE(&DumInt, i_size, 1, ADBFile_);    
       
    }       
    
    // Write out header to aerodynamics file

    number_of_tris  = VSPGeom().Grid().NumberOfLoops();
    number_of_nodes = VSPGeom().Grid().NumberOfNodes();
    
    FWRITE(&(NumberOfVortexLoops_),      i_size, 1, ADBFile_);
    FWRITE(&number_of_nodes,             i_size, 1, ADBFile_);
    FWRITE(&number_of_tris,              i_size, 1, ADBFile_);
    FWRITE(&NumberOfSurfaceVortexEdges_, i_size, 1, ADBFile_);

    FWRITE(&Sref,                   f_size, 1, ADBFile_);
    FWRITE(&Cref,                   f_size, 1, ADBFile_);
    FWRITE(&Bref,                   f_size, 1, ADBFile_);
    FWRITE(&X_cg,                   f_size, 1, ADBFile_);
    FWRITE(&Y_cg,                   f_size, 1, ADBFile_);
    FWRITE(&Z_cg,                   f_size, 1, ADBFile_);
    
    // Write out wing ID flags, names...

    DumInt = VSPGeom().NumberOfDegenWings();
    
    FWRITE(&DumInt, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
        
          FWRITE(&(i), i_size, 1, ADBFile_);
    
          SPRINTF(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
          
          FWRITE(DumChar, c_size, 100, ADBFile_);
          
          ComponentID = VSPGeom().VSP_Surface(i).ComponentID();
         
          FWRITE(&ComponentID, i_size, 1, ADBFile_);       
       
       }
     
    }
    
    // Write out body ID flags, names...
 
    DumInt = VSPGeom().NumberOfDegenBodies();
    
    FWRITE(&DumInt, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
        
          FWRITE(&(i), i_size, 1, ADBFile_);
    
          SPRINTF(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
          
          FWRITE(DumChar, c_size, 100, ADBFile_);
          
          ComponentID = VSPGeom().VSP_Surface(i).ComponentID();
          
          FWRITE(&ComponentID, i_size, 1, ADBFile_);              
      
       }
     
    }
    
    // Write out Cart3d ID flags, names...
 
    DumInt = VSPGeom().NumberOfCart3dSurfaces();

    FWRITE(&DumInt, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= VSPGeom().NumberOfCart3dSurfaces() ; i++ ) { 
     
       FWRITE(&(i), i_size, 1, ADBFile_);
 
       SPRINTF(DumChar,"Surface_%d",i);
       
       FWRITE(DumChar, c_size, 100, ADBFile_);

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

    FREAD(&DumInt, i_size, 1, InputADBFile_);
    
    // Read in model type... VLM or PANEL
    
    FREAD(&DumInt, i_size, 1, InputADBFile_);

    // Read in symmetry flag
    
    FREAD(&DumInt, i_size, 1, InputADBFile_);

    // Read in unsteady analysis flag
    
    FREAD(&TimeAccurate_, i_size, 1, InputADBFile_);    

    // Read in header to aerodynamics file

    FREAD(&DumInt,   i_size, 1, InputADBFile_);
    FREAD(&DumInt,   i_size, 1, InputADBFile_);
    FREAD(&DumInt,   i_size, 1, InputADBFile_);
    FREAD(&DumInt,   i_size, 1, InputADBFile_);    
    FREAD(&DumFloat, f_size, 1, InputADBFile_);
    FREAD(&DumFloat, f_size, 1, InputADBFile_);
    FREAD(&DumFloat, f_size, 1, InputADBFile_);
    FREAD(&DumFloat, f_size, 1, InputADBFile_);
    FREAD(&DumFloat, f_size, 1, InputADBFile_);
    FREAD(&DumFloat, f_size, 1, InputADBFile_);
    
    // Read in wing ID flags, names...

    FREAD(&Wings, i_size, 1, InputADBFile_);

    for ( i = 1 ; i <= Wings ; i++ ) { 

       FREAD(&DumInt, i_size, 1,   InputADBFile_);
       
       FREAD(DumChar, c_size, 100, InputADBFile_);
       
       FREAD(&DumInt, i_size, 1,   InputADBFile_);       

    }
    
    // Read in body ID flags, names...

    FREAD(&Bodies, i_size, 1, InputADBFile_);

    for ( i = 1 ; i <= Bodies ; i++ ) { 

       FREAD(&DumInt, i_size, 1,   InputADBFile_);

       FREAD(DumChar, c_size, 100, InputADBFile_);

       FREAD(&DumInt, i_size, 1,   InputADBFile_);              

    }
    
    // Read in Cart3d ID flags, names...

    FREAD(&Surfs, i_size, 1, InputADBFile_);

    for ( i = 1 ; i <=Surfs ; i++ ) { 
     
       FREAD(&DumInt, i_size, 1,   InputADBFile_);

       FREAD(DumChar, c_size, 100, InputADBFile_);

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

       Area        = FLOAT( VSPGeom().Grid().LoopList(j).Area() );
       
       FWRITE(&(Node1),       i_size, 1, ADBFile_);
       FWRITE(&(Node2),       i_size, 1, ADBFile_);
       FWRITE(&(Node3),       i_size, 1, ADBFile_);
       FWRITE(&(SurfaceType), i_size, 1, ADBFile_);
       FWRITE(&(SurfaceID),   i_size, 1, ADBFile_);
       FWRITE(&Area,          f_size, 1, ADBFile_);

    }

    // Write out node data

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfNodes() ; j++ ) {

       x = FLOAT( VSPGeom().Grid().NodeList(j).x() );
       y = FLOAT( VSPGeom().Grid().NodeList(j).y() );
       z = FLOAT( VSPGeom().Grid().NodeList(j).z() );
       
       FWRITE(&(x), f_size, 1, ADBFile_);
       FWRITE(&(y), f_size, 1, ADBFile_);
       FWRITE(&(z), f_size, 1, ADBFile_);
       
    }

    // Write out the rotor data
    
    FWRITE(&NumberOfRotors_, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
     
       RotorDisk(i).Write_Binary_STP_Data(ADBFile_);
     
    }
    
    // Write out the edges for each grid level
        
    MaxLevels = VSPGeom().NumberOfGridLevels();

    FWRITE(&MaxLevels, i_size, 1, ADBFile_); 
   
    // Loop over each level

    for ( Level = 1 ; Level <= MaxLevels ; Level++ ) {

       NumberOfCoarseNodes = VSPGeom().Grid(Level).NumberOfNodes();

       NumberOfCoarseEdges = VSPGeom().Grid(Level).NumberOfEdges();
 
       FWRITE(&NumberOfCoarseNodes, i_size, 1, ADBFile_); 

       FWRITE(&NumberOfCoarseEdges, i_size, 1, ADBFile_); 

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

          x = FLOAT( VSPGeom().Grid(Level).NodeList(j).x() );
          y = FLOAT( VSPGeom().Grid(Level).NodeList(j).y() );
          z = FLOAT( VSPGeom().Grid(Level).NodeList(j).z() );
          
          FWRITE(&(x), f_size, 1, ADBFile_);
          FWRITE(&(y), f_size, 1, ADBFile_);
          FWRITE(&(z), f_size, 1, ADBFile_);         
     
       }
       
       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfEdges() ; j++ ) {

          SurfaceID = VSPGeom().Grid(Level).EdgeList(j).DegenBody()
                    + VSPGeom().Grid(Level).EdgeList(j).DegenWing()
                    + VSPGeom().Grid(Level).EdgeList(j).Cart3DSurface();
                    
          if ( VSPGeom().Grid(Level).EdgeList(j).Loop1() == VSPGeom().Grid(Level).EdgeList(j).Loop2() ) SurfaceID *= -1;         

          Node1 = VSPGeom().Grid(Level).EdgeList(j).Node1();
          Node2 = VSPGeom().Grid(Level).EdgeList(j).Node2();  

          FWRITE(&SurfaceID, i_size, 1, ADBFile_);
          
          FWRITE(&Node1, i_size, 1, ADBFile_);
          FWRITE(&Node2, i_size, 1, ADBFile_);

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
    
    FWRITE(&NumberOfKuttaTE, i_size, 1, ADBFile_);
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
 
       if ( VSPGeom().Grid(Level).EdgeList(i).IsTrailingEdge() ) {     

          FWRITE(&i, i_size, 1, ADBFile_); 
          
       }
       
    }
    
    // Write out kutta nodes
    
    NumberOfKuttaNodes = 0;
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
            
       NumberOfKuttaNodes += VortexSheet(k).NumberOfTrailingVortices();
       
    }

    FWRITE(&NumberOfKuttaNodes, i_size, 1, ADBFile_);
 
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
            
       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          Node1 = VortexSheet(k).TrailingVortex(i).Node();
  
          FWRITE(&Node1, i_size, 1, ADBFile_); 

       }
    
    }
    
    // Write out control surfaces
    
    NumberOfControlSurfaces = 0;
    
    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          NumberOfControlSurfaces += VSPGeom().VSP_Surface(j).NumberOfControlSurfaces();
          
       }
 
    }

    FWRITE(&NumberOfControlSurfaces, i_size, 1, ADBFile_); 
    
    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          for ( k = 1 ; k <= VSPGeom().VSP_Surface(j).NumberOfControlSurfaces() ; k++ ) {
             
             p = VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfNodes();
             
             FWRITE(&p, i_size, 1, ADBFile_);
       
             for ( p = 1 ; p <= VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfNodes() ; p++ ) {
        
                x = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node(p)[0] );
                y = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node(p)[1] );
                z = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node(p)[2] );
                
                FWRITE(&x, f_size, 1, ADBFile_); 
                FWRITE(&y, f_size, 1, ADBFile_); 
                FWRITE(&z, f_size, 1, ADBFile_); 
             
             }
             
             // Hinge data
             
             x = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_1(0) );
             y = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_1(1) );
             z = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_1(2) );   

             FWRITE(&x, f_size, 1, ADBFile_); 
             FWRITE(&y, f_size, 1, ADBFile_); 
             FWRITE(&z, f_size, 1, ADBFile_);     
             
             x = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_2(0) );
             y = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_2(1) );
             z = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_2(2) );   
             
             FWRITE(&x, f_size, 1, ADBFile_); 
             FWRITE(&y, f_size, 1, ADBFile_); 
             FWRITE(&z, f_size, 1, ADBFile_);       
             
             x = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).HingeVec(0) );
             y = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).HingeVec(1) );
             z = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).HingeVec(2) );   
             
             FWRITE(&x, f_size, 1, ADBFile_); 
             FWRITE(&y, f_size, 1, ADBFile_); 
             FWRITE(&z, f_size, 1, ADBFile_);                   
             
             // Affected loops
             
             NumberOfControlLoops = 0;
             
             for ( p = 1 ; p <= VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfLoops() ; p++ ) {
                
                Loop = VSPGeom().VSP_Surface(j).ControlSurface(k).LoopList(p);
                
                NumberOfControlLoops += VSPGeom().Grid(1).LoopList(Loop).NumberOfFineGridLoops();
                
             }
             
             FWRITE(&NumberOfControlLoops, i_size, 1, ADBFile_);
             
             for ( p = 1 ; p <= VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfLoops() ; p++ ) {
                
                Loop = VSPGeom().VSP_Surface(j).ControlSurface(k).LoopList(p);
                
                for ( r = 1 ; r <= VSPGeom().Grid(1).LoopList(Loop).NumberOfFineGridLoops() ; r++ ) {
                   
                   FWRITE(&(VSPGeom().Grid(1).LoopList(Loop).FineGridLoop(r)), i_size, 1, ADBFile_);
                   
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

       FREAD(&DumInt,   i_size, 1, InputADBFile_);
       FREAD(&DumInt,   i_size, 1, InputADBFile_);
       FREAD(&DumInt,   i_size, 1, InputADBFile_);
       FREAD(&DumInt,   i_size, 1, InputADBFile_);
       FREAD(&DumInt,   i_size, 1, InputADBFile_);
       FREAD(&DumFloat, f_size, 1, InputADBFile_);

    }

    // Read in node data

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfNodes() ; j++ ) {

       FREAD(&DumFloat, f_size, 1, InputADBFile_);
       FREAD(&DumFloat, f_size, 1, InputADBFile_);
       FREAD(&DumFloat, f_size, 1, InputADBFile_);
       
    }

    // Read in the rotor data
    
    FREAD(&NumberOfRotors, i_size, 1, InputADBFile_);

    for ( i = 1 ; i <= NumberOfRotors ; i++ ) {
     
       RotorDisk(i).Skip_Read_Binary_STP_Data(InputADBFile_);
     
    }
    
    // Read in the edges for each grid level

    FREAD(&MaxLevels, i_size, 1, InputADBFile_); 
   
    // Loop over each level

    for ( Level = 1 ; Level <= MaxLevels ; Level++ ) {

       FREAD(&DumInt, i_size, 1, InputADBFile_); 

       FREAD(&DumInt, i_size, 1, InputADBFile_); 

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

          FREAD(&DumFloat, f_size, 1, InputADBFile_);
          FREAD(&DumFloat, f_size, 1, InputADBFile_);
          FREAD(&DumFloat, f_size, 1, InputADBFile_);         
     
       }
       
       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfEdges() ; j++ ) {

          FREAD(&DumInt, i_size, 1, InputADBFile_);
          
          FREAD(&DumInt, i_size, 1, InputADBFile_);
          FREAD(&DumInt, i_size, 1, InputADBFile_);

       }
  
    }
    
    // Read in kutta edges

    FREAD(&NumberOfKuttaTE, i_size, 1, InputADBFile_);
    
    for ( i = 1 ; i <= NumberOfKuttaTE ; i++ ) {
 
       FREAD(&DumInt, i_size, 1, InputADBFile_); 

    }
    
    // Read in kutta nodes

    FREAD(&NumberOfKuttaNodes, i_size, 1, InputADBFile_);

    for ( k = 1 ; k <= NumberOfKuttaNodes ; k++ ) {

       FREAD(&DumInt, i_size, 1, InputADBFile_); 

    }
    
    // Read in control surfaces

    FREAD(&NumberOfControlSurfaces, i_size, 1, InputADBFile_); 
    
    for ( j = 1 ; j <= NumberOfControlSurfaces ; j++ ) {

       FWRITE(&j, i_size, 1, InputADBFile_);
   
       for ( p = 1 ; p <= j ; p++ ) {
   
          FREAD(&DumFloat, f_size, 1, InputADBFile_); 
          FREAD(&DumFloat, f_size, 1, InputADBFile_); 
          FREAD(&DumFloat, f_size, 1, InputADBFile_); 
       
       }
       
       // Hinge data
       
       FREAD(&DumFloat, f_size, 1, InputADBFile_); 
       FREAD(&DumFloat, f_size, 1, InputADBFile_); 
       FREAD(&DumFloat, f_size, 1, InputADBFile_);     
       
       FREAD(&DumFloat, f_size, 1, InputADBFile_); 
       FREAD(&DumFloat, f_size, 1, InputADBFile_); 
       FREAD(&DumFloat, f_size, 1, InputADBFile_);       
   
       FREAD(&DumFloat, f_size, 1, InputADBFile_); 
       FREAD(&DumFloat, f_size, 1, InputADBFile_); 
       FREAD(&DumFloat, f_size, 1, InputADBFile_);                   
       
       // Affected loops
   
       FREAD(&j, i_size, 1, InputADBFile_);
       
       for ( p = 1 ; p <= j ; p++ ) {
   
          FREAD(&DumInt, i_size, 1, InputADBFile_);
   
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

    int i, j, k, NumTrailVortices;
    int i_size, c_size, f_size, d_size;

    float DumFloat;
    
    float Cp, Cp_Unsteady, Gamma;

    // Write out case data to adb case file
    
    FPRINTF(ADBCaseListFile_,"%10.7f %10.7f %10.7f    %-200s \n",Mach_, AngleOfAttack_/TORAD, AngleOfBeta_/TORAD, CaseString_);
    
    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
    d_size = sizeof(double);

    // Write out Mach, Alpha, Beta

    DumFloat = FLOAT( Mach_ );

    FWRITE(&DumFloat, f_size, 1, ADBFile_);

    DumFloat = FLOAT( AngleOfAttack_ );

    FWRITE(&DumFloat, f_size, 1, ADBFile_);

    DumFloat = FLOAT( AngleOfBeta_ );

    FWRITE(&DumFloat, f_size, 1, ADBFile_);    

    // Write out min and min and max Cp
    
    DumFloat = FLOAT( CpMin_ );
    
    FWRITE(&(DumFloat), f_size, 1, ADBFile_);
    
    DumFloat = FLOAT( CpMax_ );
    
    FWRITE(&(DumFloat), f_size, 1, ADBFile_);
        
    // Write out the vortex strengths, and both the steady and unsteady Cp on the computational mesh

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       FWRITE(&(Gamma_[0][i]                ), d_size, 1, ADBFile_);
       FWRITE(&(VortexLoop(i).dCp_Unsteady()), d_size, 1, ADBFile_);
           
    }   
      
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       FWRITE(&(SurfaceVortexEdge(j).Fx()), d_size, 1, ADBFile_);
       FWRITE(&(SurfaceVortexEdge(j).Fy()), d_size, 1, ADBFile_);
       FWRITE(&(SurfaceVortexEdge(j).Fz()), d_size, 1, ADBFile_);
         
    }

    // Write out surface velocities on the computational mesh
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       FWRITE(&(VortexLoop(i).U()), d_size, 1, ADBFile_);
       FWRITE(&(VortexLoop(i).V()), d_size, 1, ADBFile_);
       FWRITE(&(VortexLoop(i).W()), d_size, 1, ADBFile_);

    }    
           
    // Write out solution on the input tri mesh

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {
  
       Gamma       = FLOAT( VSPGeom().Grid().LoopList(j).Gamma() );
       Cp          = FLOAT( VSPGeom().Grid().LoopList(j).dCp() );
       Cp_Unsteady = FLOAT( VSPGeom().Grid().LoopList(j).dCp_Unsteady() );
   
       FWRITE(&Cp,          f_size, 1, ADBFile_); // Delta Cp, or CP
       FWRITE(&Cp_Unsteady, f_size, 1, ADBFile_); // Unsteady Delta Cp, or Cp
       FWRITE(&Gamma,       f_size, 1, ADBFile_); // Circulation strength

    }

    // Write out wake shape

    NumTrailVortices = 0;
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
      
       NumTrailVortices += VortexSheet(k).NumberOfTrailingVortices();

    }    
      
    FWRITE(&NumTrailVortices, i_size, 1, ADBFile_);

    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
           
       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          VortexSheet(k).TrailingVortex(i).WriteToFile(ADBFile_);

       }
       
    }     

    // Write out control surface deflection angles

    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          for ( k = 1 ; k <= VSPGeom().VSP_Surface(j).NumberOfControlSurfaces() ; k++ ) {

             DumFloat = FLOAT( VSPGeom().VSP_Surface(j).ControlSurface(k).DeflectionAngle() );

             FWRITE(&(DumFloat), f_size, 1, ADBFile_); 

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

    FREAD(&DumFloat, f_size, 1, InputADBFile_);

    FREAD(&DumFloat, f_size, 1, InputADBFile_);

    FREAD(&DumFloat, f_size, 1, InputADBFile_);    

    // Read in min and min and max Cp

    FREAD(&DumFloat, f_size, 1, InputADBFile_);

    FREAD(&DumFloat, f_size, 1, InputADBFile_);
       
    // Read the vortex strengths and unsteady Cp on the computational mesh

    // This will be N - TimeCase
    
    if ( TimeCase >= 0 && TimeCase <= 5 ) {
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
          FREAD(&(      GammaNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
          FREAD(&(dCpUnsteadyNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
     
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
   
          FREAD(&(      GammaNoise_[0][i]), d_size, 1, InputADBFile_);
          FREAD(&(dCpUnsteadyNoise_[0][i]), d_size, 1, InputADBFile_);
     
       }  
       
    }    
    
    else {
       
       PRINTF("Unknown ADB read case: %d \n",TimeCase);
       fflush(NULL);
       exit(1);
       
    }
    
    // Read in the edge forces on the computational mesh

    // This will be N - TimeCase
    
    if ( TimeCase >= 0 && TimeCase <= 5 ) {
       
       for ( i = 1 ; i <= NumberOfSurfaceVortexEdges_ ; i++ ) {
          
          FREAD(&(FxNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
          FREAD(&(FyNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
          FREAD(&(FzNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
            
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
          
          FREAD(&(FxNoise_[0][i]), d_size, 1, InputADBFile_);
          FREAD(&(FyNoise_[0][i]), d_size, 1, InputADBFile_);
          FREAD(&(FzNoise_[0][i]), d_size, 1, InputADBFile_);
            
       }
       
    }    
    
    else {
       
       PRINTF("Unknown ADB read case: %d \n",TimeCase);
       fflush(NULL);
       exit(1);
       
    }    
    
    // Read in surface velocities on the computational mesh

    // This will be N - TimeCase
    
    if ( TimeCase >= 0 && TimeCase <= 5 ) {
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
          FREAD(&(UNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
          FREAD(&(VNoise_[TimeCase][i]), d_size, 1, InputADBFile_);
          FREAD(&(WNoise_[TimeCase][i]), d_size, 1, InputADBFile_);

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
   
          FREAD(&(UNoise_[0][i]), d_size, 1, InputADBFile_);
          FREAD(&(VNoise_[0][i]), d_size, 1, InputADBFile_);
          FREAD(&(WNoise_[0][i]), d_size, 1, InputADBFile_);
     
       }  
       
    }    
    
    else {
       
       PRINTF("Unknown ADB read case: %d \n",TimeCase);
       fflush(NULL);
       exit(1);
       
    }
      
    // Loop over surfaces and read in solution

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {
  
       FREAD(&Cp,          f_size, 1, InputADBFile_); // Delta Cp, or CP
       FREAD(&Cp_Unsteady, f_size, 1, InputADBFile_); // Unsteady Delta Cp, or Cp
       FREAD(&Gamma,       f_size, 1, InputADBFile_); // Circulation strength

    }

    // Read in wake shape
    
    FREAD(&DumInt, i_size, 1, InputADBFile_);
 
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
      
       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          if ( DoAdjointSolve_ || CalculateGradients_ ) {
             
             VortexSheet(k).TrailingVortex(i).ReadInFile(InputADBFile_);
             
          }
          
          else {
             
             VortexSheet(k).TrailingVortex(i).SkipReadInFile(InputADBFile_);
             
          }

       }
       
    }     
    
    // Read in control surface deflection angles

    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          for ( k = 1 ; k <= VSPGeom().VSP_Surface(j).NumberOfControlSurfaces() ; k++ ) {

             FREAD(&DumFloat, f_size, 1, InputADBFile_); 

          }
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER InterpolateExistingSolution                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::InterpolateExistingSolution(VSPAERO_DOUBLE Time)
{
   
    int i, j, Edge, MaxSize;
    VSPAERO_DOUBLE *Result, DeltaCp;
    
    MaxSize = MAX(NumberOfVortexLoops_, NumberOfSurfaceVortexEdges_);
    
    Result = new VSPAERO_DOUBLE[MaxSize + 1];
    
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

void VSP_SOLVER::InterpolateInTime(VSPAERO_DOUBLE Time, VSPAERO_DOUBLE **ArrayIn, VSPAERO_DOUBLE *ArrayOut, int NumValues)
{
   
   int i;
   VSPAERO_DOUBLE InterpTime[6], Wgt[5];
   VSPAERO_DOUBLE m0, m1, n0, n1, p0, p1, p, t;
   
   // Steady state case
   
   if ( SteadyStateNoise_ || DoAdjointSolve_ || CalculateGradients_ ) {

      for ( i = 1 ; i <= NumValues ; i++ ) {
         
         ArrayOut[i] = ArrayIn[0][i];
 
      }
      
   }      
   
   else {
      
      // Otherwise we have to interpolate in time
         
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
            
            PRINTF("Unknown type of interpolation method in noise routines! \n");
            fflush(NULL);
            exit(1);
            
         }
   
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
    
    SPRINTF(FileNameWithExt,"%s.restart",FileName_);
    
    if ( (RestartFile = fopen(FileNameWithExt, "wb")) == NULL ) {

       PRINTF("Could not open the restart file for output! \n");

       exit(1);

    }   
    
    // Write out the vortex strengths
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       FWRITE(&(Gamma_[0][i]), d_size, 1, RestartFile);
       FWRITE(&(Gamma_[1][i]), d_size, 1, RestartFile);
       FWRITE(&(Gamma_[2][i]), d_size, 1, RestartFile);
       
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
    
    SPRINTF(FileNameWithExt,"%s.restart",FileName_);
    
    if ( (RestartFile = fopen(FileNameWithExt, "rb")) == NULL ) {

       PRINTF("Could not open the restart file for output! \n");

       exit(1);

    }   
    
    // Write out the vortex strengths
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       FREAD(&(Gamma_[0][i]), d_size, 1, RestartFile);
       FREAD(&(Gamma_[1][i]), d_size, 1, RestartFile);
       FREAD(&(Gamma_[2][i]), d_size, 1, RestartFile);
       
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
    
    VSPAERO_DOUBLE xyz[3], Vec[3], Distance, Test;
    
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
    
    if ( LoopType == FIXED_LOOPS ) PRINTF("Creating interaction lists... \n\n");fflush(NULL);

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
       
       PRINTF("Unknown type of loop in CreateSurfaceVorticesInteractionList! \n");
       exit(1);
       
    }
    
    // Forward sweep
    
    if ( LoopType == FIXED_LOOPS ) PRINTF("Forward sweep... \n");
               
    NumberOfInteractionLoops_[LoopType] = NumberOfVortexLoops_;

#pragma omp parallel for reduction(+:TotalHits,SpeedRatio) private(xyz,TempInteractionList,NumberOfEdges,i) schedule(dynamic) if (DO_PARALLEL_LOOP)
    for ( k = 1 ; k <= NumberOfVortexLoops_ ; k++ ) {
     
       if ( LoopType == FIXED_LOOPS && (k/1000)*1000 == k ) PRINTF("%d / %d \r",k,NumberOfVortexLoops_);fflush(NULL);

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

    if ( LoopType == FIXED_LOOPS ) PRINTF("The Forward Speed Up Ratio: %Lf \n",SpeedRatio);
    
    // Backwards sweep
    
    if ( LoopType == FIXED_LOOPS ) PRINTF("Backward sweep... \n");

    CommonEdgeList = new LOOP_ENTRY*[NumberOfThreads_ + 1] ();
    
    EdgeIsCommon = new int*[NumberOfThreads_ + 1] ();

    for ( cpu = 0 ; cpu < NumberOfThreads_ ; cpu++ ) {
       
       CommonEdgeList[cpu] = new LOOP_ENTRY[NumberOfVortexLoops_ + 1];
       
       EdgeIsCommon[cpu] = new int[MaxInteractionEdges + 1];
       
       zero_int_array(EdgeIsCommon[cpu], MaxInteractionEdges);
       
    }
    
    MaxLevels = VSPGeom().NumberOfGridLevels();

    NewHits = 0;
 
    LoopOffSet = 0;
    
    for ( Level = 2 ; Level <= MaxLevels ; Level++ ) {

       if ( LoopType == FIXED_LOOPS ) PRINTF("Working on level %d of %d \r",Level,MaxLevels);fflush(NULL);

#pragma omp parallel for reduction(+:NewHits) private(cpu,CurrentLoop,i,j,CommonEdges,TestEdge,TotalFound,Found,Done,xyz,Vec,Test,Distance,k,p,NumberOfEdges,TempInteractionList) schedule(dynamic) if (DO_PARALLEL_LOOP)
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

                 EdgeIsCommon[cpu][ABS(InteractionLoopList_[LoopType][CurrentLoop].SurfaceVortexEdgeInteractionList(j)->VortexEdge())] = 0;
   
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

    if ( LoopType == FIXED_LOOPS ) PRINTF("\nTotal Speed Up Ratio: %Lf \n\n\n",SpeedRatio);fflush(NULL);

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
       
       PRINTF("Error in cleaning up interaction list! \n"); fflush(NULL);
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
    
    VSPAERO_DOUBLE xyz[3], Vec[3], Distance, Test;
    
    long double SpeedRatio;
    
    VORTEX_SHEET_LOOP_INTERACTION_ENTRY *VortexsheetInteractionLoopList;
    VORTEX_SHEET_ENTRY *TempInteractionList;
    VORTEX_SHEET_LIST *CommonSheetList;
      
    if ( Verbose_ ) PRINTF("Creating vortex sheet to surface interaction lists... \n\n");fflush(NULL);
    
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
       
       if ( Verbose_ && (k/1000)*1000 == k ) PRINTF("%d / %d \r",k,NumberOfVortexLoops_);fflush(NULL);

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

    if ( Verbose_ ) PRINTF("Surface/Wake: Forward Speed Up Ratio: %Lf \n",SpeedRatio);
    
    // Backwards sweep
    
    if ( Verbose_ ) PRINTF("Surface/Wake Backward sweep... \n");

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

    if ( Verbose_ ) PRINTF("\nSurface/Wake: Total Speed Up Ratio: %Lf \n\n\n",SpeedRatio);fflush(NULL);

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
       
       PRINTF("Error in cleaning up interaction list! \n"); fflush(NULL);
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
    VSPAERO_DOUBLE xyz[3], Distance, Test, SpeedRatio;
    VORTEX_SHEET_LOOP_INTERACTION_ENTRY *VortexSheetInteractionEdgeList;
    VORTEX_SHEET_ENTRY *TempInteractionList;
    VORTEX_SHEET_LIST *CommonSheetList;

#ifdef VSPAERO_OPENMP    
    cpu = omp_get_thread_num();
#else
    cpu = 0;
#endif  

    if ( Verbose_ ) PRINTF("Creating vortex sheet to vortex sheet interaction lists... \n\n");fflush(NULL);
    
    // Allocate space for final interaction lists

    MaxInteractionEdges = VortexSheet(w).TrailingVortex(t).TotalNumberOfSubVortices();

    if ( Verbose_ ) PRINTF("MaxInteractionEdges: %d \n",MaxInteractionEdges);

    VortexSheetInteractionEdgeList = new VORTEX_SHEET_LOOP_INTERACTION_ENTRY[MaxInteractionEdges + 1];

    DoCheck = 0;
    
    TotalHits = 0;
    
    FullEval = 0;

    // Forward sweep
    
    NumSubVortices = VortexSheet(w).TrailingVortex(t).NumberOfSubVortices();

    if ( TimeAccurate_ ) NumSubVortices = MIN(WakeStartingTime_ + Time_ + 1, VortexSheet(w).TrailingVortex(t).NumberOfSubVortices());

    for ( k = 1 ; k <= NumSubVortices ; k++ ) {

       xyz[0] = VortexSheet(w).TrailingVortex(t).VortexEdge(k).Xc();
       xyz[1] = VortexSheet(w).TrailingVortex(t).VortexEdge(k).Yc();
       xyz[2] = VortexSheet(w).TrailingVortex(t).VortexEdge(k).Zc();

       if ( Verbose_ && (k/1000)*1000 == k ) PRINTF("%d / %d \r",k,NumberOfVortexLoops_);fflush(NULL);

       TempInteractionList = VortexSheet(cpu,v).CreateInteractionSheetList(xyz, NumberOfSheets); 

       // Save the sorted list
       
       VortexSheetInteractionEdgeList[k].Level() = 1;             
             
       VortexSheetInteractionEdgeList[k].TrailingVortex() = t;

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

    FullEval = VortexSheet(w).TrailingVortex(t).NumberOfSubVortices() * ( VortexSheet(v).NumberOfTrailingVortices() - 1);

    NumberOfVortexSheetInteractionEdges = VortexSheet(w).TrailingVortex(t).NumberOfSubVortices();

    SpeedRatio = (VSPAERO_DOUBLE) TotalHits;
   
    SpeedRatio = FullEval / SpeedRatio;

    if ( Verbose_ ) PRINTF("Wake/Wake: Forward Speed Up Ratio: %f \n",SpeedRatio);
  
    // Backwards sweep
 
    if ( Verbose_ ) PRINTF("Wake/Wake: Backward sweep... \n");

    CommonSheetList = new VORTEX_SHEET_LIST[MaxInteractionEdges + 1];

    int MaxLevels = VortexSheet(w).TrailingVortex(t).NumberOfLevels();

    int NewHits = 0;
 
    LoopOffSet = 0;
    
    if ( Verbose_ ) PRINTF("MaxLevels: %d \n",MaxLevels);

    for ( Level = 2 ; Level <= MaxLevels ; Level++ ) {
       
       NumSubVortices = VortexSheet(w).TrailingVortex(t).NumberOfSubVortices(Level);

       if ( TimeAccurate_ ) NumSubVortices = MIN( WakeStartingTime_ + Time_ + 1, VortexSheet(w).TrailingVortex(t).NumberOfSubVortices(Level));

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
                
                Distance = MIN(CommonSheetList[1].Sheet[CommonSheetList[1].NextSheet  ].Distance,
                               CommonSheetList[2].Sheet[CommonSheetList[2].NextSheet-1].Distance);

                // FarAway x approximate distance between fine grid trailing vortex edge centroids
                
                Test = 0.5 * FarAway_ * VortexSheet(w).TrailingVortex(t).VortexEdge(Level,Loop).Length();

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

       LoopOffSet += VortexSheet(w).TrailingVortex(t).NumberOfSubVortices(Level-1);

    }

    SpeedRatio *= (VSPAERO_DOUBLE) TotalHits / (VSPAERO_DOUBLE) NewHits;

    if ( Verbose_ ) PRINTF("\n Wake/Wake: Total Speed Up Ratio: %lf  \n\n\n",SpeedRatio);fflush(NULL);

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
       
       PRINTF("Error in cleaning up interaction list! \n"); fflush(NULL);
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

void VSP_SOLVER::CalculateSurfaceInducedVelocityAtPoint(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[3])
{
 
    int j, NumberOfEdges;
    VSPAERO_DOUBLE U, V, W, dq[3];
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

void VSP_SOLVER::CalculateSurfaceInducedVelocityAtPoint(int ComponentID, int pLoop, VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[3])
{
 
    int j, NumberOfEdges;
    VSPAERO_DOUBLE U, V, W, dq[3];
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

void VSP_SOLVER::CalculateSurfaceInducedVelocityAtPoint(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE CoreWidth)
{
 
    int j, NumberOfEdges;
    VSPAERO_DOUBLE U, V, W, dq[3];
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

void VSP_SOLVER::AdjustNearSurfaceVelocities(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[3], int NumberOfEdges, VSP_EDGE **InteractionList)
{
 
    int j;
    VSPAERO_DOUBLE Vec1[3], Vec2[3], qVec[3], Normal[3], Dot, Distance, Ratio, Angle;
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

void VSP_SOLVER::CalculateWingSurfaceInducedVelocityAtPoint(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[3])
{
 
    int j, NumberOfEdges;
    VSPAERO_DOUBLE U, V, W, dq[3];
    VSP_EDGE **InteractionList, *VortexEdge;
     
    // Create interaction list for this xyz location

    InteractionList = CreateInteractionList(0, 0, ALL_LOOPS, xyz, NumberOfEdges);

    U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(VortexEdge,dq) if (DO_PARALLEL_LOOP)
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

VSP_EDGE **VSP_SOLVER::CreateInteractionList(int ComponentID, int pLoop, int InteractionType, VSPAERO_DOUBLE xyz[3], int &NumberOfInteractionEdges)
{

    int i, j, cpu, CoarseGridEdge, FineGridEdge, Level, Loop, LoopComponentID;
    int DoAllLoops, NoRelativeMotion, RelativeMotion;
    int StackSize, MoveDownLevel, Next, AddEdges, NumberOfUsedEdges;
    VSPAERO_DOUBLE Distance, Test, NormalDistance, Vec[3], Tolerance, Ratio, OverLap;

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
 
                OverLap = calculate_box_overlap(VSPGeom().BBoxForComponent(ComponentID), VSPGeom().BBoxForComponent(VSPGeom().Grid(Level).LoopList(Loop).ComponentID()));

                if ( OverLap > 0. ) {

                   if ( ABS(NormalDistance) <= Tolerance ) AddEdges = 0;

                }
                
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
 
    NumberOfInteractionEdges = 0;

    // Add in all the coarsest edges

    Level = VSPGeom().NumberOfGridLevels();

    for ( i = VSPGeom().Grid(Level).NumberOfEdges() ; i >= 1 ; i-- ) {
       
       if ( EdgeIsUsed_[cpu][Level][i] == SearchID_[cpu] ) TempInteractionList_[cpu][++NumberOfInteractionEdges] = &(VSPGeom().Grid(Level).EdgeList(i)); 
       
    }

    // Add in the finer grid edges
    
    for ( Level = VSPGeom().NumberOfGridLevels() - 1 ; Level >= 1 ; Level-- ) {
    
       for ( i = VSPGeom().Grid(Level).NumberOfEdges() ; i >= 1 ; i-- ) {

         // Add those edges that are used at this level
         
         if ( EdgeIsUsed_[cpu][Level][i] == SearchID_[cpu] ) {

             CoarseGridEdge = VSPGeom().Grid(Level).EdgeList(i).CoarseGridEdge();
             
             // Coarse edge was not added, so add this one in
                
             if ( EdgeIsUsed_[cpu][Level+1][CoarseGridEdge] != SearchID_[cpu] ) {
             
                TempInteractionList_[cpu][++NumberOfInteractionEdges] = &(VSPGeom().Grid(Level).EdgeList(i));    
               
             }

          }    
          
       }   
       
    }

//PRINTF("NumberOfInteractionEdges: %d \n",NumberOfInteractionEdges);fflush(NULL);exit(1);

/* old way... slower, but a bit more strict
 * 
    // Zero out fine grid edges that might be marked as well
    
    NumberOfInteractionEdges = 0;

    for ( Level = VSPGeom().NumberOfGridLevels() ; Level >= 1 ; Level-- ) {
       
       for ( i = VSPGeom().Grid(Level).NumberOfEdges() ; i >= 1 ; i-- ) {
          
          if ( EdgeIsUsed_[cpu][Level][i] == SearchID_[cpu] ) {

             // On coarsest mesh, so just add it in
             
             if ( Level == VSPGeom().NumberOfGridLevels() ) {
         
                TempInteractionList_[cpu][++NumberOfInteractionEdges] = &(VSPGeom().Grid(Level).EdgeList(i));    
          
          PRINTF("NumberOfInteractionEdges: %d \n",NumberOfInteractionEdges);
          
             }  
             
             // Not on the coarsest mesh, must check if any coarser mesh edges already added in...
             
             else {
                
                CoarseGridEdge = VSPGeom().Grid(Level).EdgeList(i).CoarseGridEdge();
                
                // Coarse edge was not added, so add this one in
                   
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
                
                // Coarse edge was added so buffer it up a level
                   
                if ( EdgeIsUsed_[cpu][Level+1][CoarseGridEdge] == SearchID_[cpu] ) {
                   
               //    EdgeIsUsed_[cpu][Level][i] = SearchID_[cpu];
                      
                }
                
             }
             
          }
          
       }
       
    }              

PRINTF("NumberOfInteractionEdges: %d \n",NumberOfInteractionEdges);fflush(NULL);exit(1);
*/

     
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
#                        VSP_SOLVER NodeIsInsideLoop                           #
#                                                                              #
##############################################################################*/

int VSP_SOLVER::NodeIsInsideLoop(VSP_LOOP &Loop, VSPAERO_DOUBLE xyz[3])
{
   
   int i, Edge;
   
   for ( i = 1 ; i <= Loop.NumberOfEdges() ; i++ ) {
  
      Edge = Loop.Edge(i);
      
   }   
   
   return 1;
   
}

/*##############################################################################
#                                                                              #
#                   VSP_SOLVER ProlongateSolutionFromGrid                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ProlongateSolutionFromGrid(int Level)
{
 
    int i_c, i_f, g_c, g_f;
    VSPAERO_DOUBLE Fact;
   
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
    VSPAERO_DOUBLE Fact;

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
    VSPAERO_DOUBLE Fact, Area1, Area2, wgt1, wgt2;

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
    VSPAERO_DOUBLE Fact, *dCp, *Denom, *Res, *Dif, *Sum, Delta, Eps, ResMax, ResMax0;
    VSPAERO_DOUBLE Wgt1, Wgt2, CpAvg;
    
    g_c = Level;
    g_f = Level - 1;
    
    // Smooth pressure
    
    dCp = new VSPAERO_DOUBLE[VSPGeom().Grid(g_c).NumberOfNodes() + 1];
    Denom = new VSPAERO_DOUBLE[VSPGeom().Grid(g_c).NumberOfNodes() + 1];
    
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

    Res = new VSPAERO_DOUBLE[VSPGeom().Grid(g_c).NumberOfNodes() + 1];

    Dif = new VSPAERO_DOUBLE[VSPGeom().Grid(g_c).NumberOfNodes() + 1];

    Sum = new VSPAERO_DOUBLE[VSPGeom().Grid(g_c).NumberOfNodes() + 1];

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
          
          //PRINTF("Iter: %d ... Resmax: %f \n",Iter,log10(ResMax/ResMax0));
          
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
   
             Node1 = VortexSheet(k).TrailingVortex(i).Node();

             VortexSheet(k).TrailingVortex(i).Gamma() = VSPGeom().Grid(Level).NodeList(Node1).dGamma();          

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
      
                VortexSheet(k).TrailingVortex(i).Gamma() = 0.;
   
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

void VSP_SOLVER::OutputStatusFile(int Case)
{

    int i;
    VSPAERO_DOUBLE E, AR, ToQS, Time;
    
    AR = Bref_ * Bref_ / Sref_;

    ToQS = 0.;
    
    for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
     
       if ( Verbose_ ) PRINTF("RotorDisk(%d).RotorThrust(): %f \n",i,RotorDisk(i).RotorThrust());
            
       ToQS += RotorDisk(i).RotorThrust() / ( 0.5 * Density_ * Vref_ * Vref_ * Sref_);
       
    }    
 
    E = (CL() * CL() /(PI * AR)) / CD() ;
 
    i = CurrentWakeIteration_;

    if ( !TimeAccurate_ ) {
       
        FPRINTF(StatusFile_, "%9d % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E\n",
               i,
               Mach_,
               FLOAT(AngleOfAttack_/TORAD),
               FLOAT(AngleOfBeta_/TORAD),
               CL(),
               CDo(),
               CD(),
               FLOAT(CDo() + CD()),
               CS(),            
               FLOAT(CL()/(CDo() + CD())),
               E,
               CFx(),
               CFy(),
               CFz(),
               CMx(),
               CMy(),
               CMz(),
               CDTrefftz_[0],
               ToQS);

       PRINTF("%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf",
               i,
               Mach_,
               FLOAT(AngleOfAttack_/TORAD),
               FLOAT(AngleOfBeta_/TORAD),
               CL(),
               CDo(),
               CD(),
               FLOAT(CDo() + CD()),
               CS(),            
               FLOAT(CL()/(CDo() + CD())),
               E,
               CFx(),
               CFy(),
               CFz(),
               CMx(),
               CMy(),
               CMz(),
               CDTrefftz_[0],
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
          
           FPRINTF(StatusFile_, "% 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E\n",
                  CurrentTime_,
                  Mach_,
                  FLOAT(AngleOfAttack_/TORAD),
                  FLOAT(AngleOfBeta_/TORAD),
                  CL(),
                  CDo(),
                  CD(),
                  FLOAT(CDo() + CD()),
                  CS(),            
                  FLOAT(CL()/(CDo() + CD())),
                  E,
                  CFx(),
                  CFy(),
                  CFz(),
                  CMx(),
                  CMy(),
                  CMz(),
                  CDTrefftz_[0],
                  ToQS,
                  Unsteady_H_);
                  
       }
       
       else if ( TimeAnalysisType_ == P_ANALYSIS || TimeAnalysisType_ == Q_ANALYSIS || TimeAnalysisType_ == R_ANALYSIS ) {
          
            FPRINTF(StatusFile_, "%9.5lf % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E\n",
                  CurrentTime_,
                  Mach_,
                  FLOAT(AngleOfAttack_/TORAD),
                  FLOAT(AngleOfBeta_/TORAD),
                  CL(),
                  CDo(),
                  CD(),
                  FLOAT(CDo() + CD()),
                  CS(),            
                  FLOAT(CL()/(CDo() + CD())),
                  E,
                  CFx(),
                  CFy(),
                  CFz(),
                  CMx(),
                  CMy(),
                  CMz(),
                  CDTrefftz_[0],
                  ToQS,
                  FLOAT(Unsteady_Angle_*180./3.14159));                    
                                    
       }        
       
       else {
          
          Time = CurrentTime_;
          
          if ( NoiseAnalysis_ ) Time = CurrentNoiseTime_;
          
          // Unsteady results for this time step
          
          if ( Case == 0 ) {
          
              FPRINTF(StatusFile_, "%9.5lf % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E\n",
                     Time,
                     Mach_,
                     FLOAT(AngleOfAttack_/TORAD),
                     FLOAT(AngleOfBeta_/TORAD),
                     CL(),
                     CDo(),
                     CD(),
                     FLOAT(CDo() + CD()),
                     CS(),            
                     FLOAT(CL()/(CDo() + CD())),
                     E,
                     CFx(),
                     CFy(),
                     CFz(),
                     CMx(),
                     CMy(),
                     CMz(),
                     CDTrefftz_[0],
                     ToQS);      
                     
          }
          
          else { 
               
             CFx_[2] /= NumberOfAveragingSets_;
             CFy_[2] /= NumberOfAveragingSets_;
             CFz_[2] /= NumberOfAveragingSets_;
                                          
             CMx_[2] /= NumberOfAveragingSets_;
             CMy_[2] /= NumberOfAveragingSets_;
             CMz_[2] /= NumberOfAveragingSets_;
                                           
              CL_[2] /= NumberOfAveragingSets_;
              CD_[2] /= NumberOfAveragingSets_;
              CS_[2] /= NumberOfAveragingSets_;
                                           
             CDo_[2] /= NumberOfAveragingSets_;       
       
             E = (CL(2) * CL(2) /(PI * AR)) / CD(2) ;
             
             FPRINTF(StatusFile_,"\n\n\n");
             FPRINTF(StatusFile_,"Average Data: \n\n");
                              
             FPRINTF(StatusFile_,"%9.5lf % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E % 9E\n",
                     Time,
                     Mach_,
                     FLOAT(AngleOfAttack_/TORAD),
                     FLOAT(AngleOfBeta_/TORAD),
                     CL(2),
                     CDo(2),
                     CD(2),
                     FLOAT(CDo(2) + CD(2)),
                     CS(2),            
                     FLOAT(CL(2)/(CDo(2) + CD(2))),
                     E,
                     CFx(2),
                     CFy(2),
                     CFz(2),
                     CMx(2),
                     CMy(2),
                     CMz(2),
                     CDTrefftz_[2],
                     ToQS);      
                     
          }
                         
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
    
    FPRINTF(StatusFile_,"\n");
    FPRINTF(StatusFile_,"\n");
    FPRINTF(StatusFile_,"\n");    
    FPRINTF(StatusFile_,"Skin Friction Drag Break Out:\n");    
    FPRINTF(StatusFile_,"\n");   
    FPRINTF(StatusFile_,"\n");       
                       //1234567890123456789012345678901234567890: 123456789
    FPRINTF(StatusFile_,"Surface                                      CDo \n");
    FPRINTF(StatusFile_,"\n");
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       FPRINTF(StatusFile_,"%-40s  %9.5lf \n",
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
    
    FPRINTF(fid,"***************************************************************************************************************************************************************************************** \n");
    FPRINTF(fid,"\n");
    
    //          123456789012345678901234567890123456789
    FPRINTF(fid,headerFormatStr, "# Name", "Value   ", "  Units");
    FPRINTF(fid,dataFormatStr, "Sref_", Sref(), "Lunit^2");
    FPRINTF(fid,dataFormatStr, "Cref_", Cref(), "Lunit");
    FPRINTF(fid,dataFormatStr, "Bref_", Bref(), "Lunit");
    FPRINTF(fid,dataFormatStr, "Xcg_", Xcg(), "Lunit");
    FPRINTF(fid,dataFormatStr, "Ycg_", Ycg(), "Lunit");
    FPRINTF(fid,dataFormatStr, "Zcg_", Zcg(), "Lunit");
    FPRINTF(fid,dataFormatStr, "Mach_", Mach(), "no_unit");
    FPRINTF(fid,dataFormatStr, "AoA_", FLOAT(AngleOfAttack()/TORAD), "deg");
    FPRINTF(fid,dataFormatStr, "Beta_", FLOAT(AngleOfBeta()/TORAD), "deg");
    FPRINTF(fid,dataFormatStr, "Rho_", Density(), "Munit/Lunit^3");
    FPRINTF(fid,dataFormatStr, "Vinf_", Vinf(), "Lunit/Tunit");
    FPRINTF(fid,dataFormatStr, "Roll__Rate", RotationalRate_p(), "rad/Tunit");
    FPRINTF(fid,dataFormatStr, "Pitch_Rate", RotationalRate_q(), "rad/Tunit");
    FPRINTF(fid,dataFormatStr, "Yaw___Rate", RotationalRate_r(), "rad/Tunit");
    /*
    char control_name[20];
    for ( int n = 1 ; n <= NumberOfControlGroups_ ; n++ ) {
        //                    1234567890123456789
        SPRINTF(control_name,"Control_Group_%-5d",n);
        FPRINTF(fid,dataFormatStr, control_name, Delta_Control_, "deg");
    }
    */
    
    FPRINTF(fid,"\n");
}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER CalculateLeadingEdgeSuctionFraction                #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE VSP_SOLVER::CalculateLeadingEdgeSuctionFraction(VSPAERO_DOUBLE Mach, VSPAERO_DOUBLE ToC, VSPAERO_DOUBLE RoC, VSPAERO_DOUBLE EtaToC, VSPAERO_DOUBLE AoA, VSPAERO_DOUBLE Sweep)
{
   
   VSPAERO_DOUBLE Rin, RoCn, ToCn, Machn, Betan, Ctn;
   VSPAERO_DOUBLE e1, e2, e3, Ptt, k, Kt;
   
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
#                VSP_SOLVER OutputForcesAndMomentsForGroup                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::OutputForcesAndMomentsForGroup(int Group)
{
   
    int c, k, i, j, t;
    VSPAERO_DOUBLE Thrust, Thrusto, Thrusti, Moment, Momento, Momenti, Power, Powero, Poweri;
    VSPAERO_DOUBLE CT, CQ, CP, J, EtaP, CT_h, CQ_h, CP_h, FOM, Diameter, RPM, Vec[3], Time;
    VSPAERO_DOUBLE Omega, Radius, TipVelocity, Area;

    // Loop over component groups, calculate rotor coefficients if any are
    // flagged as being rotors by the user.

    k = 0;
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
        
       // Write out group data

       Time = CurrentTime_;
       
       if ( NoiseAnalysis_ ) Time = CurrentNoiseTime_ + NoiseTimeShift_;

       if ( !TimeAccurate_ ) Time = CurrentWakeIteration_;
                 
       if ( Group == 0 || c == Group ) {
          
          FPRINTF(GroupFile_[c],"%10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f \n",
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
                  ComponentGroupList_[c].CS() + ComponentGroupList_[c].CSo(),
                  ComponentGroupList_[c].CLo(),
                  ComponentGroupList_[c].CDo(),   
                  ComponentGroupList_[c].CSo(),                     
                  ComponentGroupList_[c].CL(),                  
                  ComponentGroupList_[c].CD(),
                  ComponentGroupList_[c].CS());       
                  
 
          Time = CurrentTime_;
           
          if ( NoiseAnalysis_ ) Time = CurrentNoiseTime_ + NoiseTimeShift_;
          
          if ( !TimeAccurate_ ) Time = CurrentWakeIteration_;
              
          // Store time history of last rotation
          
          if ( ( TimeAccurate_ && CurrentTime_ >= ComponentGroupList_[c].StartAveragingTime() ) || ( !TimeAccurate_ && CurrentWakeIteration_ >= WakeIterations_ ) ) ComponentGroupList_[c].UpdateAverageForcesAndMoments();
          
          // If this is the last time step, write out final averaged forces
             
          if ( ( TimeAccurate_ && Time_ == NumberOfTimeSteps_ ) ) ComponentGroupList_[c].CalculateAverageForcesAndMoments();
                                  
          // Write out wing data
                  
          if ( ComponentGroupList_[c].GeometryHasWings() ) {

             // If this is the last time step, write out final averaged forces
                
             if ( ( TimeAccurate_ && Time_ == NumberOfTimeSteps_ ) || ( !TimeAccurate_ && CurrentWakeIteration_ >= WakeIterations_ ) ) {
                
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
                
                FPRINTF(GroupFile_[c],"\n\n");
                FPRINTF(GroupFile_[c],"Average over last full revolution: \n");
                FPRINTF(GroupFile_[c],"\n");
 
                FPRINTF(GroupFile_[c],"%10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f \n",
                        Time,
                        ComponentGroupList_[c].CxAvg() + ComponentGroupList_[c].CxoAvg(),
                        ComponentGroupList_[c].CyAvg() + ComponentGroupList_[c].CyoAvg(),
                        ComponentGroupList_[c].CzAvg() + ComponentGroupList_[c].CzoAvg(),
                        ComponentGroupList_[c].CxoAvg(),
                        ComponentGroupList_[c].CyoAvg(),
                        ComponentGroupList_[c].CzoAvg(),
                        ComponentGroupList_[c].CxAvg(),
                        ComponentGroupList_[c].CyAvg(),
                        ComponentGroupList_[c].CzAvg(),                                    
                        ComponentGroupList_[c].CmxAvg() + ComponentGroupList_[c].CmxoAvg(),
                        ComponentGroupList_[c].CmyAvg() + ComponentGroupList_[c].CmyoAvg(),
                        ComponentGroupList_[c].CmzAvg() + ComponentGroupList_[c].CmzoAvg(),
                        ComponentGroupList_[c].CmxoAvg(),
                        ComponentGroupList_[c].CmyoAvg(),
                        ComponentGroupList_[c].CmzoAvg(),
                        ComponentGroupList_[c].CmxAvg(),
                        ComponentGroupList_[c].CmyAvg(),
                        ComponentGroupList_[c].CmzAvg(),                                    
                        ComponentGroupList_[c].CLAvg() + ComponentGroupList_[c].CLoAvg(),
                        ComponentGroupList_[c].CDAvg() + ComponentGroupList_[c].CDoAvg(),
                        ComponentGroupList_[c].CSAvg() + ComponentGroupList_[c].CSoAvg(),                        
                        ComponentGroupList_[c].CLoAvg(),
                        ComponentGroupList_[c].CDoAvg(),   
                        ComponentGroupList_[c].CSoAvg(),                           
                        ComponentGroupList_[c].CLAvg(),                  
                        ComponentGroupList_[c].CDAvg(),
                        ComponentGroupList_[c].CSAvg());       
                                                  
                // Write out spanwise blade loading data, again averaged over one revolution
                
                for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfLiftingSurfaces() ; i++ ) {

                   FPRINTF(GroupFile_[c],"\n\n\n");
                   FPRINTF(GroupFile_[c],"Average loading for wing surface: %d --> VSP Surface: %d ... NOTE: values are per area! \n",i,ComponentGroupList_[c].SpanLoadData(i).SurfaceID());
                   FPRINTF(GroupFile_[c],"\n");
                   
                                    //    123456789 123456789 123456789 123456789 123456789 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123
                   FPRINTF(GroupFile_[c],"  Station     S       Chord     Area      V/Vref        Cx            Cy            Cz            Cl            Cd            Cs           Cmx           Cmy           Cmz \n");
                   
                   ComponentGroupList_[c].SpanLoadData(i).CalculateAverageForcesAndMoments();
                   
                   // Write out averaged span loading data
                                        
                   for ( j = 1 ; j <= ComponentGroupList_[c].SpanLoadData(i).NumberOfSpanStations() ; j++ ) {

                     FPRINTF(GroupFile_[c],"%9d %9.5lf %9.5lf %9.5lf %9.5lf %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le \n",
                             j,
                             ComponentGroupList_[c].SpanLoadData(i).Span_S(j),                    
                             ComponentGroupList_[c].SpanLoadData(i).Span_Chord(j),
                             ComponentGroupList_[c].SpanLoadData(i).Span_Area(j),
                             
                             ComponentGroupList_[c].SpanLoadData(i).Local_Velocity(j),
            
                             ComponentGroupList_[c].SpanLoadData(i).Span_Cx(j),
                             ComponentGroupList_[c].SpanLoadData(i).Span_Cy(j),
                             ComponentGroupList_[c].SpanLoadData(i).Span_Cz(j),
                             
                             ComponentGroupList_[c].SpanLoadData(i).Span_Cl(j),
                             ComponentGroupList_[c].SpanLoadData(i).Span_Cd(j),
                             ComponentGroupList_[c].SpanLoadData(i).Span_Cw(j),

                             ComponentGroupList_[c].SpanLoadData(i).Span_Cmx(j),
                             ComponentGroupList_[c].SpanLoadData(i).Span_Cmy(j),
                             ComponentGroupList_[c].SpanLoadData(i).Span_Cmz(j));


                   }
                   
                }

                // Write out spanwise blade loading data, as a function of time, over about 1 period of rotation 
                
                for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfLiftingSurfaces() ; i++ ) {

                   FPRINTF(GroupFile_[c],"\n\n\n");
                   FPRINTF(GroupFile_[c],"Time history of loading for wing surface: %d --> VSP Surface: %d ... NOTE: values per area! \n",i,ComponentGroupList_[c].SpanLoadData(i).SurfaceID());
                   FPRINTF(GroupFile_[c],"\n");
                                         
                   ComponentGroupList_[c].SpanLoadData(i).CalculateAverageForcesAndMoments();
                   
                   // Write out averaged span loading data

                   for ( t = 1 ; t <= ComponentGroupList_[c].SpanLoadData(i).ActualTimeSamples() ; t++ ) {

                                       //    123456789 123456789 123456789 123456789 123456789 123456789 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123
                      FPRINTF(GroupFile_[c],"  Station    Time       S       Chord     Area      V/Vref        Cx            Cy            Cz            Cl            Cd            Cs           Cmx           Cmy           Cmz \n");
                   
                                            
                      for ( j = 1 ; j <= ComponentGroupList_[c].SpanLoadData(i).NumberOfSpanStations() ; j++ ) {

                         FPRINTF(GroupFile_[c],"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le \n",
                                 j,
                                 ComponentGroupList_[c].SpanLoadData(i).Time(t,j),
                                 ComponentGroupList_[c].SpanLoadData(i).Span_S(t,j),                    
                                 ComponentGroupList_[c].SpanLoadData(i).Span_Chord(t,j),
                                 ComponentGroupList_[c].SpanLoadData(i).Span_Area(t,j),
                                 
                                 ComponentGroupList_[c].SpanLoadData(i).Local_Velocity(t,j),
                        
                                 ComponentGroupList_[c].SpanLoadData(i).Span_Cx(t,j),
                                 ComponentGroupList_[c].SpanLoadData(i).Span_Cy(t,j),
                                 ComponentGroupList_[c].SpanLoadData(i).Span_Cz(t,j),
                                 
                                 ComponentGroupList_[c].SpanLoadData(i).Span_Cl(t,j),
                                 ComponentGroupList_[c].SpanLoadData(i).Span_Cd(t,j),
                                 ComponentGroupList_[c].SpanLoadData(i).Span_Cw(t,j),
                        
                                 ComponentGroupList_[c].SpanLoadData(i).Span_Cmx(t,j),
                                 ComponentGroupList_[c].SpanLoadData(i).Span_Cmy(t,j),
                                 ComponentGroupList_[c].SpanLoadData(i).Span_Cmz(t,j));
                        
                        
                      }
    
                   }
                   
                }
                
             }               
             
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
             
             // Power
             
             Powero = Momento * ComponentGroupList_[c].Omega();
             
             Poweri = Momenti * ComponentGroupList_[c].Omega();
             
             Power = Powero + Poweri;
             
             CalculateRotorCoefficientsFromForces(Thrust, Moment, Diameter, RPM, J, CT, CQ, CP, EtaP, CT_h, CQ_h, CP_h, FOM);
             
             Time = CurrentTime_;
          
             if ( NoiseAnalysis_ ) Time = CurrentNoiseTime_ + NoiseTimeShift_;
             
             if ( !TimeAccurate_ ) Time = CurrentWakeIteration_;
                                  //  1      2      3      4      5      6      7      8      9     10     11     12     13     14     15     16     17     18     19   
             FPRINTF(RotorFile_[k],"%10.5f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f \n",
                      Time,
                      Diameter,
                      RPM,
                      Thrust,
                      Thrusto,
                      Thrusti,
                      Power,
                      Powero,
                      Poweri,
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
                 
             if ( ( TimeAccurate_ && Time_ == NumberOfTimeSteps_ ) ) {
                                
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
                
                FPRINTF(RotorFile_[k],"\n\n");
                FPRINTF(RotorFile_[k],"Average over last full revolution: \n");
                FPRINTF(RotorFile_[k],"\n");
                                                 //  1      2      3      4      5      6      7      8      9     10     11     12     13     14     15     16     17     18     19     20     21      
                FPRINTF(RotorFile_[k],"           %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f \n",
                        Diameter,
                        RPM,
                        Thrust,
                        Thrusto,
                        Thrusti,
                        Power,
                        Powero,
                        Poweri,                        
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
                
                for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfLiftingSurfaces() ; i++ ) {

                   FPRINTF(RotorFile_[k],"\n\n\n");
                   FPRINTF(RotorFile_[k],"Average loading for rotor blade: %d --> VSP Surface: %d ... NOTE: values are per station, not per area! \n",i,ComponentGroupList_[c].SpanLoadData(i).SurfaceID());
                   FPRINTF(RotorFile_[k],"\n");
                   
                                    //    123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123
                   FPRINTF(RotorFile_[k],"  Station     S       Chord     Area      V/Vref   Diameter    RPM      TipVel       CNo_H         CSo_H         CTo_H         CQo_H         CPo_H         CN_H          CS_H          CT_H          CQ_H          CP_H \n");
                   
                   ComponentGroupList_[c].SpanLoadData(i).CalculateAverageForcesAndMoments();
                   
                   // Write out averaged span loading data

                   Omega = 2. * PI * RPM / 60.;
                   
                   Radius = 0.5 * Diameter;
                   
                   Area = PI * Radius * Radius;
                   
                   TipVelocity = Omega * Radius;
                                        
                   for ( j = 1 ; j <= ComponentGroupList_[c].SpanLoadData(i).NumberOfSpanStations() ; j++ ) {

                     FPRINTF(RotorFile_[k],"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le \n",
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

                // Write out spanwise blade loading data, as a function of time, over about 1 period of rotation 
                
                for ( i = 1 ; i <= ComponentGroupList_[c].NumberOfLiftingSurfaces() ; i++ ) {

                   FPRINTF(RotorFile_[k],"\n\n\n");
                   FPRINTF(RotorFile_[k],"Time history of loading for rotor blade: %d --> VSP Surface: %d ... NOTE: values are per station, not per area! \n",i,ComponentGroupList_[c].SpanLoadData(i).SurfaceID());
                   FPRINTF(RotorFile_[k],"\n");
                                         
                   ComponentGroupList_[c].SpanLoadData(i).CalculateAverageForcesAndMoments();
                   
                   // Write out averaged span loading data

                   Omega = 2. * PI * RPM / 60.;
                   
                   Radius = 0.5 * Diameter;
                   
                   Area = PI * Radius * Radius;
                   
                   TipVelocity = Omega * Radius;
                   
                   for ( t = 1 ; t <= ComponentGroupList_[c].SpanLoadData(i).ActualTimeSamples() ; t++ ) {

                                       //    123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123 1234567890123                         
                      FPRINTF(RotorFile_[k],"  Station    Time       Angle     Xqc       Yqc       Zqc       S       Chord     Area      V/Vref   Diameter    RPM      TipVel       CNo_H         CSo_H         CTo_H         CQo_H         CPo_H         CN_H          CS_H          CT_H          CQ_H          CP_H \n");
                                           
                      for ( j = 1 ; j <= ComponentGroupList_[c].SpanLoadData(i).NumberOfSpanStations() ; j++ ) {

                        FPRINTF(RotorFile_[k],"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le %13.6le \n",
                                j,
                                ComponentGroupList_[c].SpanLoadData(i).Time(t,j),
                                ComponentGroupList_[c].SpanLoadData(i).RotationAngle(t,j),  
                                ComponentGroupList_[c].SpanLoadData(i).X_QC(t,j),  
                                ComponentGroupList_[c].SpanLoadData(i).Y_QC(t,j),  
                                ComponentGroupList_[c].SpanLoadData(i).Z_QC(t,j),  
                                
                                ComponentGroupList_[c].SpanLoadData(i).Span_S(t,j),                    
                                ComponentGroupList_[c].SpanLoadData(i).Span_Chord(t,j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Area(t,j),
                                
                                ComponentGroupList_[c].SpanLoadData(i).Local_Velocity(j),
                                
                                Diameter,
                                RPM,
                                TipVelocity,
                                
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cno(t,j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cso(t,j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cto(t,j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cqo(t,j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cpo(t,j),

                                ComponentGroupList_[c].SpanLoadData(i).Span_Cn(t,j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cs(t,j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Ct(t,j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cq(t,j),
                                ComponentGroupList_[c].SpanLoadData(i).Span_Cp(t,j));

   
                      }
                   
                   }
                   
                }
                
             }             
                                      
          }
         
       }
      
    }

}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER CalculateRotorCoefficients                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateRotorCoefficientsFromForces(VSPAERO_DOUBLE Thrust, VSPAERO_DOUBLE Moment, VSPAERO_DOUBLE Diameter, VSPAERO_DOUBLE RPM,
                                                      VSPAERO_DOUBLE &J, VSPAERO_DOUBLE &CT, VSPAERO_DOUBLE &CQ, VSPAERO_DOUBLE &CP, VSPAERO_DOUBLE &EtaP,
                                                      VSPAERO_DOUBLE &CT_h, VSPAERO_DOUBLE &CQ_h, VSPAERO_DOUBLE &CP_h, VSPAERO_DOUBLE &FOM)
{
   
    VSPAERO_DOUBLE n, Omega, Radius, Area, TipVelocity;

    // Revs per second
    
    n = fabs( RPM / 60. );
    
    // Advance ratio
    
    J = Vinf_ / (n * Diameter);

    // Propeller coefficients
    
    CT = Thrust / ( Density_ * n * n * pow(Diameter, 4.) );
    CQ = Moment / ( Density_ * n * n * pow(Diameter, 5.) );
    CP = 2.*PI *CQ;
        
    EtaP = J * CT / (2.*PI*CQ);
    
    // Rotor (Helicopter) coefficients
    
    Omega = fabs( 2. * PI * RPM / 60. );
    
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
             
             PRINTF("Found: %d wing surfaces in group: %d \n",NumberOfWingSurfaces, c);
             
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
             
             PRINTF("Found: %d body surfaces in group: %d \n",NumberOfBodySurfaces, c);
             
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

      PRINTF("wtf!... Time Step is still less than zero! \n");fflush(NULL);exit(1);
      
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

   WopWopNumberOfTimeSteps_ = FLOAT( 4 * ( WopWopObserverTime_ / NoiseTimeStep_ + 1) );
   
   PowerOfTwo = 1;
   
   while ( 2*PowerOfTwo < WopWopNumberOfTimeSteps_ ) PowerOfTwo *= 2;

   WopWopNumberOfTimeSteps_ = PowerOfTwo;

   PRINTF("PSU-WopWop Setup: \n");
   PRINTF("Total Solution time:      %f \n",NumberOfTimeSteps_ * TimeStep_);
   PRINTF("WopWopOmegaMin:           %f \n",WopWopOmegaMin_);
   PRINTF("Longest Period:           %f \n",WopWopLongestPeriod_);
   PRINTF("NumberOfNoiseTimeSteps_:  %d \n",NumberOfNoiseTimeSteps_);
   PRINTF("NoiseTimeShift_:          %f \n",NoiseTimeShift_);
   PRINTF("WopWopObserverTime_:      %f \n",WopWopObserverTime_);
   PRINTF("WopWopNumberOfTimeSteps_: %d \n",WopWopNumberOfTimeSteps_);
   PRINTF("\n");

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

       PRINTF("Could not open the PSUWopWop Case File output! \n");

       exit(1);

    }    
    
    SPRINTF(NameListFile,"%s.nam",FileName_);
    
    FPRINTF(WopWopCaseFile,"&caseName\n");
    FPRINTF(WopWopCaseFile,"   globalFolderName = \'./\' \n");
    FPRINTF(WopWopCaseFile,"   caseNameFile = \'%s\' \n",NameListFile);
    FPRINTF(WopWopCaseFile,"/ \n");        
    
    fclose(WopWopCaseFile);
    
    // Actual namelist file
    
    if ( (PSUWopWopNameListFile_ = fopen(NameListFile, "w")) == NULL ) {

       PRINTF("Could not open the PSUWopWop Namelist File output! \n");

       exit(1);

    } 
    
    // EnvironmentIn namelist
        
    FPRINTF(PSUWopWopNameListFile_,"!\n");    
    FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    FPRINTF(PSUWopWopNameListFile_,"!\n");
    
    FPRINTF(PSUWopWopNameListFile_,"&EnvironmentIn\n");
    FPRINTF(PSUWopWopNameListFile_,"   nbSourceContainers     = 1       \n");    
    FPRINTF(PSUWopWopNameListFile_,"   nbObserverContainers   = 1       \n");    
    FPRINTF(PSUWopWopNameListFile_,"   ASCIIOutputFlag        = .true.  \n");
    FPRINTF(PSUWopWopNameListFile_,"   spectrumFlag           = .true.  \n");     
    FPRINTF(PSUWopWopNameListFile_,"   SPLdBFlag              = .true.  \n");     
    FPRINTF(PSUWopWopNameListFile_,"   SPLdBAFlag             = .true.  \n");     
    FPRINTF(PSUWopWopNameListFile_,"   OASPLdBFlag            = .true.  \n");     
    FPRINTF(PSUWopWopNameListFile_,"   OASPLdBAFlag           = .true.  \n");     
    FPRINTF(PSUWopWopNameListFile_,"   acousticPressureFlag   = .true.  \n");     
    FPRINTF(PSUWopWopNameListFile_,"   pressureGradient1AFlag = .true.  \n");     
    FPRINTF(PSUWopWopNameListFile_,"   thicknessNoiseFlag     = .true.  \n");    
    FPRINTF(PSUWopWopNameListFile_,"   loadingNoiseFlag       = .true.  \n");    
    FPRINTF(PSUWopWopNameListFile_,"   totalNoiseFlag         = .true.  \n");    
    FPRINTF(PSUWopWopNameListFile_,"   sigmaFlag              = .false. \n");    
    FPRINTF(PSUWopWopNameListFile_,"   loadingSigmaFlag       = .false. \n");    
    FPRINTF(PSUWopWopNameListFile_,"   machSigmaFlag          = .false. \n");    
    FPRINTF(PSUWopWopNameListFile_,"   totalNoiseSigmaFlag    = .false. \n");     
    FPRINTF(PSUWopWopNameListFile_,"   audioFlag              = .true.  \n");
    FPRINTF(PSUWopWopNameListFile_,"   debugLevel = 5                   \n");    
 
    FPRINTF(PSUWopWopNameListFile_,"/ \n");  
       
    // EnvironmentConstants namelist

    FPRINTF(PSUWopWopNameListFile_,"!\n");      
    FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    FPRINTF(PSUWopWopNameListFile_,"!\n");
        
    FPRINTF(PSUWopWopNameListFile_,"&EnvironmentConstants \n");
    
    // Note these are english unit versions of psu-WopWop's default values
    
    FPRINTF(PSUWopWopNameListFile_,"   rho         = %e \n",Density_*WopWopDensityConversion_);  // Density, kg/m^3

    FPRINTF(WopWopCaseFile,"/ \n");    
           
    // ObserverIn namelist
       
    FPRINTF(PSUWopWopNameListFile_,"!\n");      
    FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    FPRINTF(PSUWopWopNameListFile_,"!\n");
           
    FPRINTF(PSUWopWopNameListFile_,"&ObserverIn  \n");
    FPRINTF(PSUWopWopNameListFile_,"  Title  = \'Observer_1\' \n");
    FPRINTF(PSUWopWopNameListFile_,"  tMin = 0.0 \n");
 
    if ( WopWopFlyBy_ ) {
       
       FPRINTF(PSUWopWopNameListFile_,"  tMax = %f  \n",24.);
       
    }
    
    else {
       
       FPRINTF(PSUWopWopNameListFile_,"  tMax = %f  \n",5./(BladeRPM_/60.));
                     
    }

    // Fly by
    
    if ( WopWopFlyBy_ ) {
       
       FPRINTF(PSUWopWopNameListFile_,"  nt                = 262144 \n");
       FPRINTF(PSUWopWopNameListFile_,"  xloc              = 0.0   \n");
       FPRINTF(PSUWopWopNameListFile_,"  yloc              = 0.0   \n");
       FPRINTF(PSUWopWopNameListFile_,"  zloc              = 0.0   \n");
       FPRINTF(PSUWopWopNameListFile_,"  highPassFrequency = 10.0  \n");
 
       FPRINTF(PSUWopWopNameListFile_,"/ \n");        
   
    }
    
    // Fixed to aircraft
    
    else {
       
       FPRINTF(PSUWopWopNameListFile_,"  nt                = %d \n",WopWopNumberOfTimeSteps_);
       FPRINTF(PSUWopWopNameListFile_,"  highPassFrequency = 10.0 \n");
       FPRINTF(PSUWopWopNameListFile_,"  attachedTo        = \'Aircraft\' \n");
       FPRINTF(PSUWopWopNameListFile_,"  nbBaseObsContFrame = 1  \n");

       FPRINTF(PSUWopWopNameListFile_,"/ \n");        
                 
    }

    // Observer CB
    
    if ( !WopWopFlyBy_ ) {

       FPRINTF(PSUWopWopNameListFile_,"!\n");      
       FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
       FPRINTF(PSUWopWopNameListFile_,"!\n");
       
       FPRINTF(PSUWopWopNameListFile_,"&CB  \n");
       FPRINTF(PSUWopWopNameListFile_,"  Title  = \'Observer\' \n");
       FPRINTF(PSUWopWopNameListFile_,"  translationType=\'TimeIndependent\' \n"); 
       FPRINTF(PSUWopWopNameListFile_,"  TranslationValue = 0.0, -500., 0. \n");                

       FPRINTF(PSUWopWopNameListFile_,"/ \n");        
       
    }

    // ContainerIn namelist - Aircraft

    FPRINTF(PSUWopWopNameListFile_,"!\n");      
    FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
    FPRINTF(PSUWopWopNameListFile_,"!\n");
        
    FPRINTF(PSUWopWopNameListFile_,"&ContainerIn  \n");
    FPRINTF(PSUWopWopNameListFile_,"   Title        = \'Aircraft\' \n");
    FPRINTF(PSUWopWopNameListFile_,"   nbContainer  = %d \n",WopWopNumberOfRotors_);
    FPRINTF(PSUWopWopNameListFile_,"   nbBase = 1 \n");
    FPRINTF(PSUWopWopNameListFile_,"   dtau = %e \n",WopWopdTau_);
    
    FPRINTF(PSUWopWopNameListFile_,"/ \n");   
    
    // Change of Base namelist for forward motion of aircraft

    FPRINTF(PSUWopWopNameListFile_,"&CB \n");
    FPRINTF(PSUWopWopNameListFile_,"   Title           = \'Velocity\' \n");
    FPRINTF(PSUWopWopNameListFile_,"   translationType = \'KnownFunction\' \n");
    FPRINTF(PSUWopWopNameListFile_,"   AH = 0.0, 0.0, 0.0 \n");
    FPRINTF(PSUWopWopNameListFile_,"   VH = %f,  0.0, 0.0 \n",-Vinf_*WopWopLengthConversion_);
    FPRINTF(PSUWopWopNameListFile_,"   Y0 = %f, %f, %f    \n",0.0, 0.0, 0.0);
    FPRINTF(PSUWopWopNameListFile_,"/ \n");        

    // Loop over each rotor

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          i = ComponentGroupList_[c].WopWop().RotorID();

          // ContainerIn for current rotor
      
          FPRINTF(PSUWopWopNameListFile_,"!\n");      
          FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          FPRINTF(PSUWopWopNameListFile_,"!\n");
       
          FPRINTF(PSUWopWopNameListFile_,"      &ContainerIn  \n");
          FPRINTF(PSUWopWopNameListFile_,"         Title        = \'Rotor_%d\' \n",i);
          FPRINTF(PSUWopWopNameListFile_,"         nbContainer  = %d \n",2*ComponentGroupList_[c].WopWop().NumberOfBlades());
          FPRINTF(PSUWopWopNameListFile_,"         BPMNoiseFlag           = .false. \n");
          FPRINTF(PSUWopWopNameListFile_,"         PeggNoiseFlag          = .false. \n");
   
          if ( SteadyStateNoise_ ) {
             
              FPRINTF(PSUWopWopNameListFile_,"         nbBase = 1 \n");
      
          }
          
          FPRINTF(PSUWopWopNameListFile_,"      / \n");     
   
          // Change of Base namelist if we are not time accurate, and let WopWop do the unsteady stuff
   
          FPRINTF(PSUWopWopNameListFile_,"!\n");      
          FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          FPRINTF(PSUWopWopNameListFile_,"!\n");
                 
          if ( SteadyStateNoise_ ) {
            
             FPRINTF(PSUWopWopNameListFile_,"      &CB \n");
             FPRINTF(PSUWopWopNameListFile_,"         Title     = \'Rotation\' \n");
             FPRINTF(PSUWopWopNameListFile_,"         rotation  = .true. \n");
             FPRINTF(PSUWopWopNameListFile_,"         AngleType = \'KnownFunction\' \n");
             FPRINTF(PSUWopWopNameListFile_,"         Omega     =  %f \n",-BladeRPM_*2.*PI/60.);
             FPRINTF(PSUWopWopNameListFile_,"         AxisValue = 1.0,0.0,0.0 \n");
             FPRINTF(PSUWopWopNameListFile_,"      / \n");        
             
          }
   
          // Broad band noise: BPM namelist
   
          FPRINTF(PSUWopWopNameListFile_,"!\n");      
          FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          FPRINTF(PSUWopWopNameListFile_,"!\n");
          
          FPRINTF(PSUWopWopNameListFile_,"      &BPMIn \n");
          FPRINTF(PSUWopWopNameListFile_,"         BPMNoiseFile = %s.PSUWopWop.BPM.Rotor.%d.dat \n",FileName_,i);
          FPRINTF(PSUWopWopNameListFile_,"         nSect           = %d \n",ComponentGroupList_[c].WopWop().NumberOfBladesSections());
          FPRINTF(PSUWopWopNameListFile_,"         uniformBlade    = 0 \n");
          FPRINTF(PSUWopWopNameListFile_,"         BLtrip          = 0 \n");
          FPRINTF(PSUWopWopNameListFile_,"         sectChordFlag   = 'FILEVALUE' \n");
          FPRINTF(PSUWopWopNameListFile_,"         sectLengthFlag  = 'COMPUTE'   \n");
          FPRINTF(PSUWopWopNameListFile_,"         TEThicknessFlag = 'FILEVALUE' \n");
          FPRINTF(PSUWopWopNameListFile_,"         TEflowAngleFlag = 'FILEVALUE' \n");
          FPRINTF(PSUWopWopNameListFile_,"         TipLCSFlag      = 'FILEVALUE' \n");
          FPRINTF(PSUWopWopNameListFile_,"         SectAOAFlag     = 'FILEVALUE' \n");
          FPRINTF(PSUWopWopNameListFile_,"         SectAOAFlag     = 'FILEVALUE' \n");
          FPRINTF(PSUWopWopNameListFile_,"         LBLVSnoise      = .true.  \n");
          FPRINTF(PSUWopWopNameListFile_,"         TBLTEnoise      = .true.  \n");
          FPRINTF(PSUWopWopNameListFile_,"         bluntNoise      = .true.  \n");
          FPRINTF(PSUWopWopNameListFile_,"         bladeTipNoise   = .true.  \n");
          FPRINTF(PSUWopWopNameListFile_,"      / \n");
    
          // Broad band noise: PeggIn namelist
          
          FPRINTF(PSUWopWopNameListFile_,"!\n");      
          FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
          FPRINTF(PSUWopWopNameListFile_,"!\n");
                 
          WriteOutPSUWopWopPeggNamelist();
          
          // Write out containers for each of the blades... one for loading, one for thickness
          
          for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {
          
             SPRINTF(PatchLoadingGeometryName  ,"%s.PSUWopWop.Loading.Geometry.Rotor.%d.Blade.%d.dat",  FileName_,i,j);
             SPRINTF(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Rotor.%d.Blade.%d.dat",FileName_,i,j);
             SPRINTF(WopWopFileName,          "%s.PSUWopWop.Loading.Rotor.%d.Blade.%d.dat",           FileName_,i,j);
               
             // ContainerIn for Aero loading geometry and load file
      
             FPRINTF(PSUWopWopNameListFile_,"!\n");      
             FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             FPRINTF(PSUWopWopNameListFile_,"!\n");
         
             FPRINTF(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             FPRINTF(PSUWopWopNameListFile_,"               Title        = \'Blade_%d_For_Rotor_%d_Loading\' \n",j,i);
             FPRINTF(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             FPRINTF(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchLoadingGeometryName);
             FPRINTF(PSUWopWopNameListFile_,"               patchLoadingFile   = %s \n",WopWopFileName);
             FPRINTF(PSUWopWopNameListFile_,"            / \n");     
          
             FPRINTF(PSUWopWopNameListFile_,"!\n");      
             FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             FPRINTF(PSUWopWopNameListFile_,"!\n");
    
             // ContainerIn for Aero thickness model
             
             FPRINTF(PSUWopWopNameListFile_,"            &ContainerIn  \n");
             FPRINTF(PSUWopWopNameListFile_,"               Title        = \'Blade_%d_For_Rotor_%d_Thickness\' \n",j,i);
             FPRINTF(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
             FPRINTF(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
             FPRINTF(PSUWopWopNameListFile_,"            / \n");               
                
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
   
    int c, i, j, Case;
    char NameListFile[2000];
    char PatchThicknessGeometryName[2000], PatchLoadingGeometryName[2000], WopWopFileName[2000];
    char pressureFileName[2000];
    char SPLFileName[2000];
    char OASPLFileName[2000];
    char phaseFileName[2000];
    char complexPressureFileName[2000];
    char audioFileName[2000];
    
    FILE *WopWopCaseFile;      

    // Cases namelist file
    
    if ( (WopWopCaseFile = fopen("cases.nam", "w")) == NULL ) {

       PRINTF("Could not open the PSUWopWop Case File output! \n");

       exit(1);

    }    
    
    // Baseline case, with all rotors

    FPRINTF(WopWopCaseFile,"! \n");
    FPRINTF(WopWopCaseFile,"!Remove the comment delimiter from those cases below you wish to run, beyond the default all up case \n");
    FPRINTF(WopWopCaseFile,"! \n");
    
    SPRINTF(NameListFile,"%s.nam",FileName_);
    
    FPRINTF(WopWopCaseFile,"&caseName\n");
    FPRINTF(WopWopCaseFile,"   globalFolderName = \'./\' \n");
    FPRINTF(WopWopCaseFile,"   caseNameFile = \'%s\' \n",NameListFile);
    FPRINTF(WopWopCaseFile,"/ \n");        
    
    // Case with no rotors

    SPRINTF(NameListFile,"%s.NoRotors.nam",FileName_);
    
    FPRINTF(WopWopCaseFile,"!&caseName\n");
    FPRINTF(WopWopCaseFile,"!   globalFolderName = \'./\' \n");
    FPRINTF(WopWopCaseFile,"!   caseNameFile = \'%s\' \n",NameListFile);
    FPRINTF(WopWopCaseFile,"!/ \n");       
    
    // Cases with a single rotor included 
        
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          i = ComponentGroupList_[c].WopWop().RotorID();
          
          SPRINTF(NameListFile,"%s.Rotor.%d.nam",FileName_,i);
          
          FPRINTF(WopWopCaseFile,"!&caseName\n");
          FPRINTF(WopWopCaseFile,"!   globalFolderName = \'./\' \n");
          FPRINTF(WopWopCaseFile,"!   caseNameFile = \'%s\' \n",NameListFile);
          FPRINTF(WopWopCaseFile,"!/ \n");      
          
       }
       
    }        
                    
    fclose(WopWopCaseFile);
    
    // Create multiple psuwopwop cases, body/wings alone, bodies/wings + single rotors, and full up case
    
    for ( Case = -1 ; Case <= WopWopNumberOfRotors_ ; Case++ ) {

       if ( Case == -1 ) SPRINTF(NameListFile,"%s.NoRotors.nam",FileName_);
       if ( Case ==  0 ) SPRINTF(NameListFile,"%s.nam",FileName_);
       if ( Case >   0 ) SPRINTF(NameListFile,"%s.Rotor.%d.nam",FileName_,Case);
       
       // Actual namelist file
       
       if ( (PSUWopWopNameListFile_ = fopen(NameListFile, "w")) == NULL ) {
   
          PRINTF("Could not open the PSUWopWop Namelist File output! \n");
   
          exit(1);
   
       } 
       
       // EnvironmentIn namelist
           
       FPRINTF(PSUWopWopNameListFile_,"!\n");    
       FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
       FPRINTF(PSUWopWopNameListFile_,"!\n");
       
       FPRINTF(PSUWopWopNameListFile_,"&EnvironmentIn\n");
       FPRINTF(PSUWopWopNameListFile_,"   nbSourceContainers     = 1       \n");    
       FPRINTF(PSUWopWopNameListFile_,"   nbObserverContainers   = 1       \n");    
       FPRINTF(PSUWopWopNameListFile_,"   ASCIIOutputFlag        = .true.  \n");
       FPRINTF(PSUWopWopNameListFile_,"   spectrumFlag           = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   SPLdBFlag              = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   SPLdBAFlag             = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   OASPLdBFlag            = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   OASPLdBAFlag           = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   acousticPressureFlag   = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   pressureGradient1AFlag = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   thicknessNoiseFlag     = .true.  \n");    
       FPRINTF(PSUWopWopNameListFile_,"   loadingNoiseFlag       = .true.  \n");    
       FPRINTF(PSUWopWopNameListFile_,"   totalNoiseFlag         = .true.  \n");    
       FPRINTF(PSUWopWopNameListFile_,"   sigmaFlag              = .false. \n");    
       FPRINTF(PSUWopWopNameListFile_,"   loadingSigmaFlag       = .false. \n");    
       FPRINTF(PSUWopWopNameListFile_,"   machSigmaFlag          = .false. \n");    
       FPRINTF(PSUWopWopNameListFile_,"   totalNoiseSigmaFlag    = .false. \n");     
       FPRINTF(PSUWopWopNameListFile_,"   audioFlag              = .true.  \n");
       FPRINTF(PSUWopWopNameListFile_,"   debugLevel = 5                   \n");    
       
       if ( Case == -1 ) {
          
          SPRINTF(pressureFileName,       "pressure.NoRotors");
          SPRINTF(SPLFileName,            "spl.NoRotors");
          SPRINTF(OASPLFileName,          "OASPL.NoRotors.");
          SPRINTF(phaseFileName,          "phase.NoRotors.");
          SPRINTF(complexPressureFileName,"complexPressure.NoRotors");
          SPRINTF(audioFileName,          "sigma.NoRotors");
          
       }
     
       if ( Case == 0 ) {
          
          SPRINTF(pressureFileName,       "pressure.AllRotors");
          SPRINTF(SPLFileName,            "spl.AllRotors");
          SPRINTF(OASPLFileName,          "OASPL.AllRotors.");
          SPRINTF(phaseFileName,          "phase.AllRotors.");
          SPRINTF(complexPressureFileName,"complexPressure.AllRotors");
          SPRINTF(audioFileName,          "sigma.AllRotors");
          
       }
    
       if ( Case > 0 ) {
          
          SPRINTF(pressureFileName,       "pressure.Rotor.%d",Case);
          SPRINTF(SPLFileName,            "spl.Rotor.%d",Case);
          SPRINTF(OASPLFileName,          "OASPL.Rotor.%d.",Case);
          SPRINTF(phaseFileName,          "phase.Rotor.%d.",Case);
          SPRINTF(complexPressureFileName,"complexPressure.Rotor.%d",Case);
          SPRINTF(audioFileName,          "sigma.Rotor.%d",Case);
          
       }

       FPRINTF(PSUWopWopNameListFile_,"   pressureFileName        = %s \n",pressureFileName);
       FPRINTF(PSUWopWopNameListFile_,"   SPLFileName             = %s \n",SPLFileName);           
       FPRINTF(PSUWopWopNameListFile_,"   OASPLFileName           = %s \n",OASPLFileName);         
       FPRINTF(PSUWopWopNameListFile_,"   phaseFileName           = %s \n",phaseFileName);         
       FPRINTF(PSUWopWopNameListFile_,"   complexPressureFileName = %s \n",complexPressureFileName);
       FPRINTF(PSUWopWopNameListFile_,"   audioFileName           = %s \n",audioFileName);         
          
       FPRINTF(PSUWopWopNameListFile_,"/ \n");  
          
       // EnvironmentConstants namelist
   
       FPRINTF(PSUWopWopNameListFile_,"!\n");      
       FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
       FPRINTF(PSUWopWopNameListFile_,"!\n");
           
       FPRINTF(PSUWopWopNameListFile_,"&EnvironmentConstants \n");
       
       // Note these are english unit versions of psu-WopWop's default values
       
       FPRINTF(PSUWopWopNameListFile_,"   rho         = %e \n",Density_*WopWopDensityConversion_);  // Density, kg/m^3
   
       FPRINTF(WopWopCaseFile,"/ \n");    
              
       // ObserverIn namelist
          
       FPRINTF(PSUWopWopNameListFile_,"!\n");      
       FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
       FPRINTF(PSUWopWopNameListFile_,"!\n");
              
       FPRINTF(PSUWopWopNameListFile_,"&ObserverIn  \n");
       FPRINTF(PSUWopWopNameListFile_,"  Title             = \'Observer_1\' \n");
       FPRINTF(PSUWopWopNameListFile_,"  tMin              =  0.0 \n");
       FPRINTF(PSUWopWopNameListFile_,"  tMax              = 30.0 \n");
       FPRINTF(PSUWopWopNameListFile_,"  highPassFrequency = %f \n",10.);
       FPRINTF(PSUWopWopNameListFile_,"  nt                = 32768 \n");
   
       // Fixed fly by location
   
       FPRINTF(PSUWopWopNameListFile_,"  xLoc              = %f \n",0.);
       FPRINTF(PSUWopWopNameListFile_,"  yLoc              = %f \n",0.);
       FPRINTF(PSUWopWopNameListFile_,"  zLoc              = %f \n",0.);
   
       // OASPLdB inputs
       
       FPRINTF(PSUWopWopNameListFile_,"  segmentSize       =  1.0  \n");
       FPRINTF(PSUWopWopNameListFile_,"  segmentStepSize   =  0.1  \n");
   
       FPRINTF(PSUWopWopNameListFile_,"/ \n");        
   
       // ContainerIn namelist - Aircraft
   
       FPRINTF(PSUWopWopNameListFile_,"!\n");      
       FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
       FPRINTF(PSUWopWopNameListFile_,"!\n");
           
       FPRINTF(PSUWopWopNameListFile_,"&ContainerIn  \n");
       FPRINTF(PSUWopWopNameListFile_,"   Title        = \'Aircraft\' \n");
       
       // Just wings/bodies
       
       if ( Case == -1 ) {
          
          FPRINTF(PSUWopWopNameListFile_,"   nbContainer  = %d \n",                    0 + WopWopNumberOfWings_ + WopWopNumberOfBodies_);
       
       }
       
       // All rotors + wings/bodies
       
       else if ( Case == 0 ) {
          
          FPRINTF(PSUWopWopNameListFile_,"   nbContainer  = %d \n",WopWopNumberOfRotors_ + WopWopNumberOfWings_ + WopWopNumberOfBodies_);
          
       }
       
       // Wings/bodies + 1 of the rotors
       
       else {
          
          FPRINTF(PSUWopWopNameListFile_,"   nbContainer  = %d \n",                    1 + WopWopNumberOfWings_ + WopWopNumberOfBodies_);
          
       }
       
       FPRINTF(PSUWopWopNameListFile_,"   nbBase = 2 \n");
       FPRINTF(PSUWopWopNameListFile_,"   dtau = %e \n",WopWopdTau_);
       
       FPRINTF(PSUWopWopNameListFile_,"/ \n");   
           
   
       // Change of Base namelist for attitude of aircraft
   
       FPRINTF(PSUWopWopNameListFile_,"&CB \n");
       FPRINTF(PSUWopWopNameListFile_,"   Title           = \'Euler Angle Rotation\' \n");
       FPRINTF(PSUWopWopNameListFile_,"   translationType = \'TimeIndependent\' \n");
       FPRINTF(PSUWopWopNameListFile_,"   angleValue      = %f \n",AngleOfAttack_);
       FPRINTF(PSUWopWopNameListFile_,"   axisValue       = %f, %f, %f \n",0.,1.,0.);
       FPRINTF(PSUWopWopNameListFile_,"/ \n");   
               
       // Change of Base namelist for forward motion of aircraft
   
       FPRINTF(PSUWopWopNameListFile_,"&CB \n");
       FPRINTF(PSUWopWopNameListFile_,"   Title           = \'Velocity\' \n");
       FPRINTF(PSUWopWopNameListFile_,"   translationType = \'KnownFunction\' \n");
       FPRINTF(PSUWopWopNameListFile_,"   AH = 0.0, 0.0, 0.0 \n");
       FPRINTF(PSUWopWopNameListFile_,"   VH = %f,  0.0, 0.0 \n",-Vinf_*WopWopLengthConversion_);
       FPRINTF(PSUWopWopNameListFile_,"   Y0 = %f, %f, %f    \n",Vinf_*WopWopLengthConversion_*15., 0.0, 50.0);
       FPRINTF(PSUWopWopNameListFile_,"/ \n");        
   
       // Loop over each rotor
       
       if ( Case >= 0 ) {
          
          for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
      
             if ( ComponentGroupList_[c].GeometryIsARotor() ) {
                
                i = ComponentGroupList_[c].WopWop().RotorID();
                
                if ( Case == 0 || Case == i ) {
      
                   // ContainerIn for current rotor
               
                   FPRINTF(PSUWopWopNameListFile_,"!\n");      
                   FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                   FPRINTF(PSUWopWopNameListFile_,"!\n");
                
                   FPRINTF(PSUWopWopNameListFile_,"      &ContainerIn  \n");
                   FPRINTF(PSUWopWopNameListFile_,"         Title        = \'Rotor_%d\' \n",i);
                   FPRINTF(PSUWopWopNameListFile_,"         nbContainer  = %d \n",2*ComponentGroupList_[c].WopWop().NumberOfBlades());
                   FPRINTF(PSUWopWopNameListFile_,"         BPMNoiseFlag           = .false. \n");
                   FPRINTF(PSUWopWopNameListFile_,"         PeggNoiseFlag          = .false. \n");
            
                   // Possible change of base for steady state case where PSUWOPWOP takes care of the blade rotation
                   
                   if ( SteadyStateNoise_ ) {
                      
                      FPRINTF(PSUWopWopNameListFile_,"         nbBase = 1 \n");
               
                   }
                   
                   FPRINTF(PSUWopWopNameListFile_,"      / \n");     
            
                   // Change of Base namelist if we are not time accurate, and let WopWop do the unsteady stuff
                 
                   if ( SteadyStateNoise_ ) {
         
                      FPRINTF(PSUWopWopNameListFile_,"!\n");      
                      FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                      FPRINTF(PSUWopWopNameListFile_,"!\n");
                     
                      FPRINTF(PSUWopWopNameListFile_,"      &CB \n");
                      FPRINTF(PSUWopWopNameListFile_,"         Title     = \'Rotation\' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         rotation  = .true. \n");
                      FPRINTF(PSUWopWopNameListFile_,"         AngleType = \'KnownFunction\' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         Omega     =  %f \n",-BladeRPM_*2.*PI/60.);
                      FPRINTF(PSUWopWopNameListFile_,"         AxisValue = 1.0,0.0,0.0 \n");
                      FPRINTF(PSUWopWopNameListFile_,"      / \n");        
                      
                   }
            
                   if ( SteadyStateNoise_ ) {
                      
                      // Broad band noise: BPM namelist
               
                      FPRINTF(PSUWopWopNameListFile_,"!\n");      
                      FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                      FPRINTF(PSUWopWopNameListFile_,"!\n");
                      
                      FPRINTF(PSUWopWopNameListFile_,"      &BPMIn \n");
                      FPRINTF(PSUWopWopNameListFile_,"         BPMNoiseFile = %s.PSUWopWop.BPM.Rotor.%d.dat \n",FileName_,i);
                      FPRINTF(PSUWopWopNameListFile_,"         nSect           = %d \n",ComponentGroupList_[c].WopWop().NumberOfBladesSections());
                      FPRINTF(PSUWopWopNameListFile_,"         uniformBlade    = 0 \n");
                      FPRINTF(PSUWopWopNameListFile_,"         BLtrip          = 0 \n");
                      FPRINTF(PSUWopWopNameListFile_,"         sectChordFlag   = 'FILEVALUE' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         sectLengthFlag  = 'COMPUTE'   \n");
                      FPRINTF(PSUWopWopNameListFile_,"         TEThicknessFlag = 'FILEVALUE' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         TEflowAngleFlag = 'FILEVALUE' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         TipLCSFlag      = 'FILEVALUE' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         SectAOAFlag     = 'FILEVALUE' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         SectAOAFlag     = 'FILEVALUE' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         LBLVSnoise      = .true.  \n");
                      FPRINTF(PSUWopWopNameListFile_,"         TBLTEnoise      = .true.  \n");
                      FPRINTF(PSUWopWopNameListFile_,"         bluntNoise      = .true.  \n");
                      FPRINTF(PSUWopWopNameListFile_,"         bladeTipNoise   = .true.  \n");
                      FPRINTF(PSUWopWopNameListFile_,"      / \n");
                
                      // Broad band noise: PeggIn namelist
                      
                      FPRINTF(PSUWopWopNameListFile_,"!\n");      
                      FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                      FPRINTF(PSUWopWopNameListFile_,"!\n");
                             
                      WriteOutPSUWopWopPeggNamelist();
                      
                   }
                   
                   // Write out containers for each of the blades... one for loading, one for thickness
                   
                   for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {
                   
                      SPRINTF(PatchLoadingGeometryName  ,"%s.PSUWopWop.Loading.Geometry.Rotor.%d.Blade.%d.dat",  FileName_,i,j);
                      SPRINTF(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Rotor.%d.Blade.%d.dat",FileName_,i,j);
                      SPRINTF(WopWopFileName,            "%s.PSUWopWop.Loading.Rotor.%d.Blade.%d.dat",           FileName_,i,j);
                        
                      // ContainerIn for Aero loading geometry and load file
               
                      FPRINTF(PSUWopWopNameListFile_,"!\n");      
                      FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                      FPRINTF(PSUWopWopNameListFile_,"!\n");
                  
                      FPRINTF(PSUWopWopNameListFile_,"            &ContainerIn  \n");
                      FPRINTF(PSUWopWopNameListFile_,"               Title        = \'Blade_%d_For_Rotor_%d_Loading\' \n",j,i);
                      FPRINTF(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
                      FPRINTF(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchLoadingGeometryName);
                      FPRINTF(PSUWopWopNameListFile_,"               patchLoadingFile   = %s \n",WopWopFileName);
                      FPRINTF(PSUWopWopNameListFile_,"            / \n");     
                   
                      FPRINTF(PSUWopWopNameListFile_,"!\n");      
                      FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                      FPRINTF(PSUWopWopNameListFile_,"!\n");
             
                      // ContainerIn for Aero thickness model
                      
                      FPRINTF(PSUWopWopNameListFile_,"            &ContainerIn  \n");
                      FPRINTF(PSUWopWopNameListFile_,"               Title        = \'Blade_%d_For_Rotor_%d_Thickness\' \n",j,i);
                      FPRINTF(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
                      FPRINTF(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
                      FPRINTF(PSUWopWopNameListFile_,"            / \n");               
                         
                   }
                   
                }
                   
             }
             
          }
          
       }
   
       // Loop over each wing
       
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
   
          if ( ComponentGroupList_[c].GeometryHasWings() ) {
             
             i = ComponentGroupList_[c].WopWop().WingID();
   
             // ContainerIn for current wing
         
             FPRINTF(PSUWopWopNameListFile_,"!\n");      
             FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             FPRINTF(PSUWopWopNameListFile_,"!\n");
          
             FPRINTF(PSUWopWopNameListFile_,"      &ContainerIn  \n");
             FPRINTF(PSUWopWopNameListFile_,"         Title        = \'Wing_%d\' \n",i);
             FPRINTF(PSUWopWopNameListFile_,"         nbContainer  = %d \n",2*ComponentGroupList_[c].WopWop().NumberOfWingSurfaces());
             FPRINTF(PSUWopWopNameListFile_,"      / \n");     
      
             // Write out containers for each of the wing surfaces... one for loading, one for thickness
             
             for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {
             
                SPRINTF(PatchLoadingGeometryName  ,"%s.PSUWopWop.Loading.Geometry.Wing.%d.Surface.%d.dat",  FileName_,i,j);
                SPRINTF(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Wing.%d.Surface.%d.dat",FileName_,i,j);
                SPRINTF(WopWopFileName,            "%s.PSUWopWop.Loading.Wing.%d.Surface.%d.dat",           FileName_,i,j);
                  
                // ContainerIn for Aero loading geometry and load file
         
                FPRINTF(PSUWopWopNameListFile_,"!\n");      
                FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                FPRINTF(PSUWopWopNameListFile_,"!\n");
            
                FPRINTF(PSUWopWopNameListFile_,"            &ContainerIn  \n");
                FPRINTF(PSUWopWopNameListFile_,"               Title        = \'Surface_%d_For_Wing_%d_Loading\' \n",j,i);
                FPRINTF(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
                FPRINTF(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchLoadingGeometryName);
                FPRINTF(PSUWopWopNameListFile_,"               patchLoadingFile   = %s \n",WopWopFileName);
                FPRINTF(PSUWopWopNameListFile_,"            / \n");     
             
                FPRINTF(PSUWopWopNameListFile_,"!\n");      
                FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                FPRINTF(PSUWopWopNameListFile_,"!\n");
       
                // ContainerIn for Aero thickness model
                
                FPRINTF(PSUWopWopNameListFile_,"            &ContainerIn  \n");
                FPRINTF(PSUWopWopNameListFile_,"               Title        = \'Surface_%d_For_Wing_%d_Thickness\' \n",j,i);
                FPRINTF(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
                FPRINTF(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
                FPRINTF(PSUWopWopNameListFile_,"            / \n");               
                   
             }
                
          }
          
       }
   
       // Loop over each body
       
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
   
          if ( ComponentGroupList_[c].GeometryHasBodies() ) {
             
             i = ComponentGroupList_[c].WopWop().BodyID();
   
             // ContainerIn for current body
         
             FPRINTF(PSUWopWopNameListFile_,"!\n");      
             FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             FPRINTF(PSUWopWopNameListFile_,"!\n");
          
             FPRINTF(PSUWopWopNameListFile_,"      &ContainerIn  \n");
             FPRINTF(PSUWopWopNameListFile_,"         Title        = \'Body_%d\' \n",i);
             FPRINTF(PSUWopWopNameListFile_,"         nbContainer  = %d \n",ComponentGroupList_[c].WopWop().NumberOfBodySurfaces());
             FPRINTF(PSUWopWopNameListFile_,"      / \n");     
      
             // Write out containers for each of the bodies... just the thickness
             
             for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBodySurfaces() ; j++ ) {
             
                SPRINTF(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Body.%d.Surface.%d.dat",FileName_,i,j);
   
                FPRINTF(PSUWopWopNameListFile_,"!\n");      
                FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                FPRINTF(PSUWopWopNameListFile_,"!\n");
       
                // ContainerIn for Aero thickness model
                
                FPRINTF(PSUWopWopNameListFile_,"            &ContainerIn  \n");
                FPRINTF(PSUWopWopNameListFile_,"               Title        = \'Surface_%d_For_Body_%d_Thickness\' \n",j,i);
                FPRINTF(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
                FPRINTF(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
                FPRINTF(PSUWopWopNameListFile_,"            / \n");               
                   
             }
                
          }
          
       }
       
       fclose(PSUWopWopNameListFile_);
       
    }
                
}

/*##############################################################################
#                                                                              #
#      VSP_SOLVER WriteOutPSUWopWopCaseAndNameListFilesForFootPrint            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCaseAndNameListFilesForFootPrint(void)
{
   
    int c, i, j, Case;
    char NameListFile[2000];
    char PatchThicknessGeometryName[2000], PatchLoadingGeometryName[2000], WopWopFileName[2000];
    char pressureFileName[2000];
    char SPLFileName[2000];
    char OASPLFileName[2000];
    char phaseFileName[2000];
    char complexPressureFileName[2000];
    char audioFileName[2000];
    FILE *WopWopCaseFile;      

    // Cases namelist file
    
    if ( (WopWopCaseFile = fopen("cases.nam", "w")) == NULL ) {

       PRINTF("Could not open the PSUWopWop Case File output! \n");

       exit(1);

    }    
    
    // Baseline case, with all rotors

    FPRINTF(WopWopCaseFile,"! \n");
    FPRINTF(WopWopCaseFile,"!Remove the comment delimiter from those cases below you wish to run, beyond the default all up case \n");
    FPRINTF(WopWopCaseFile,"! \n");
   
    SPRINTF(NameListFile,"%s.nam",FileName_);
    
    FPRINTF(WopWopCaseFile,"&caseName\n");
    FPRINTF(WopWopCaseFile,"   globalFolderName = \'./\' \n");
    FPRINTF(WopWopCaseFile,"   caseNameFile = \'%s\' \n",NameListFile);
    FPRINTF(WopWopCaseFile,"/ \n");        
    
    // Case with no rotors

    SPRINTF(NameListFile,"%s.NoRotors.nam",FileName_);
    
    FPRINTF(WopWopCaseFile,"!&caseName\n");
    FPRINTF(WopWopCaseFile,"!   globalFolderName = \'./\' \n");
    FPRINTF(WopWopCaseFile,"!   caseNameFile = \'%s\' \n",NameListFile);
    FPRINTF(WopWopCaseFile,"!/ \n");       
    
    // Cases with a single rotor included 
        
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          i = ComponentGroupList_[c].WopWop().RotorID();
          
          SPRINTF(NameListFile,"%s.Rotor.%d.nam",FileName_,i);
          
          FPRINTF(WopWopCaseFile,"!&caseName\n");
          FPRINTF(WopWopCaseFile,"!   globalFolderName = \'./\' \n");
          FPRINTF(WopWopCaseFile,"!   caseNameFile = \'%s\' \n",NameListFile);
          FPRINTF(WopWopCaseFile,"!/ \n");      
          
       }
       
    }          
                    
    fclose(WopWopCaseFile);

    // Create multiple psuwopwop cases, body/wings alone, bodies/wings + single rotors, and full up case
    
    for ( Case = -1 ; Case <= WopWopNumberOfRotors_ ; Case++ ) {

       if ( Case == -1 ) SPRINTF(NameListFile,"%s.NoRotors.nam",FileName_);
       if ( Case ==  0 ) SPRINTF(NameListFile,"%s.nam",FileName_);
       if ( Case >   0 ) SPRINTF(NameListFile,"%s.Rotor.%d.nam",FileName_,Case);

       // Actual namelist file
       
       if ( (PSUWopWopNameListFile_ = fopen(NameListFile, "w")) == NULL ) {
   
          PRINTF("Could not open the PSUWopWop Namelist File output! \n");
   
          exit(1);
   
       } 
                  
       // EnvironmentIn namelist
           
       FPRINTF(PSUWopWopNameListFile_,"!\n");    
       FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
       FPRINTF(PSUWopWopNameListFile_,"!\n");
       
       FPRINTF(PSUWopWopNameListFile_,"&EnvironmentIn\n");
       FPRINTF(PSUWopWopNameListFile_,"   nbSourceContainers     = 1       \n");    
       FPRINTF(PSUWopWopNameListFile_,"   nbObserverContainers   = 1       \n");    
       FPRINTF(PSUWopWopNameListFile_,"   ASCIIOutputFlag        = .true.  \n");
       FPRINTF(PSUWopWopNameListFile_,"   spectrumFlag           = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   SPLdBFlag              = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   SPLdBAFlag             = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   OASPLdBFlag            = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   OASPLdBAFlag           = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   acousticPressureFlag   = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   pressureGradient1AFlag = .true.  \n");     
       FPRINTF(PSUWopWopNameListFile_,"   thicknessNoiseFlag     = .true.  \n");    
       FPRINTF(PSUWopWopNameListFile_,"   loadingNoiseFlag       = .true.  \n");    
       FPRINTF(PSUWopWopNameListFile_,"   totalNoiseFlag         = .true.  \n");    
       FPRINTF(PSUWopWopNameListFile_,"   sigmaFlag              = .false. \n");    
       FPRINTF(PSUWopWopNameListFile_,"   loadingSigmaFlag       = .false. \n");    
       FPRINTF(PSUWopWopNameListFile_,"   machSigmaFlag          = .false. \n");    
       FPRINTF(PSUWopWopNameListFile_,"   totalNoiseSigmaFlag    = .false. \n");     
       FPRINTF(PSUWopWopNameListFile_,"   audioFlag              = .true.  \n");
       FPRINTF(PSUWopWopNameListFile_,"   debugLevel = 5                   \n");    

       if ( Case == -1 ) {
          
          SPRINTF(pressureFileName,       "pressure.NoRotors");
          SPRINTF(SPLFileName,            "spl.NoRotors");
          SPRINTF(OASPLFileName,          "OASPL.NoRotors.");
          SPRINTF(phaseFileName,          "phase.NoRotors.");
          SPRINTF(complexPressureFileName,"complexPressure.NoRotors");
          SPRINTF(audioFileName,          "sigma.NoRotors");
          
       }
     
       if ( Case == 0 ) {
          
          SPRINTF(pressureFileName,       "pressure.AllRotors");
          SPRINTF(SPLFileName,            "spl.AllRotors");
          SPRINTF(OASPLFileName,          "OASPL.AllRotors.");
          SPRINTF(phaseFileName,          "phase.AllRotors.");
          SPRINTF(complexPressureFileName,"complexPressure.AllRotors");
          SPRINTF(audioFileName,          "sigma.AllRotors");
          
       }
    
       if ( Case > 0 ) {
          
          SPRINTF(pressureFileName,       "pressure.Rotor.%d",Case);
          SPRINTF(SPLFileName,            "spl.Rotor.%d",Case);
          SPRINTF(OASPLFileName,          "OASPL.Rotor.%d.",Case);
          SPRINTF(phaseFileName,          "phase.Rotor.%d.",Case);
          SPRINTF(complexPressureFileName,"complexPressure.Rotor.%d",Case);
          SPRINTF(audioFileName,          "sigma.Rotor.%d",Case);
          
       }

       FPRINTF(PSUWopWopNameListFile_,"   pressureFileName        = %s \n",pressureFileName);
       FPRINTF(PSUWopWopNameListFile_,"   SPLFileName             = %s \n",SPLFileName);           
       FPRINTF(PSUWopWopNameListFile_,"   OASPLFileName           = %s \n",OASPLFileName);         
       FPRINTF(PSUWopWopNameListFile_,"   phaseFileName           = %s \n",phaseFileName);         
       FPRINTF(PSUWopWopNameListFile_,"   complexPressureFileName = %s \n",complexPressureFileName);
       FPRINTF(PSUWopWopNameListFile_,"   audioFileName           = %s \n",audioFileName);         
           
       FPRINTF(PSUWopWopNameListFile_,"/ \n");  
          
       // EnvironmentConstants namelist
   
       FPRINTF(PSUWopWopNameListFile_,"!\n");      
       FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
       FPRINTF(PSUWopWopNameListFile_,"!\n");
           
       FPRINTF(PSUWopWopNameListFile_,"&EnvironmentConstants \n");
       
       // Note these are english unit versions of psu-WopWop's default values
       
       FPRINTF(PSUWopWopNameListFile_,"   rho         = %e \n",Density_*WopWopDensityConversion_);  // Density, kg/m^3
   
       FPRINTF(WopWopCaseFile,"/ \n");    
              
       // ObserverIn namelist
          
       FPRINTF(PSUWopWopNameListFile_,"!\n");      
       FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
       FPRINTF(PSUWopWopNameListFile_,"!\n");
              
       FPRINTF(PSUWopWopNameListFile_,"&ObserverIn  \n");
       FPRINTF(PSUWopWopNameListFile_,"  Title  = \'Observer_1\' \n");
       FPRINTF(PSUWopWopNameListFile_,"  tMin = 0.0 \n");
       FPRINTF(PSUWopWopNameListFile_,"  tMax = %f  \n",WopWopObserverTime_);
   
       // Fixed to aircraft
          
       FPRINTF(PSUWopWopNameListFile_,"  nt                = %d \n",WopWopNumberOfTimeSteps_);
       FPRINTF(PSUWopWopNameListFile_,"  highPassFrequency = %f \n",10.);
       FPRINTF(PSUWopWopNameListFile_,"  attachedTo        = \'Aircraft\' \n");
       FPRINTF(PSUWopWopNameListFile_,"  nbBaseObsContFrame = 1  \n");
       
       FPRINTF(PSUWopWopNameListFile_,"/ \n");        
                    
       // Observer CB
   
       FPRINTF(PSUWopWopNameListFile_,"!\n");      
       FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
       FPRINTF(PSUWopWopNameListFile_,"!\n");
       
       FPRINTF(PSUWopWopNameListFile_,"&CB  \n");
       FPRINTF(PSUWopWopNameListFile_,"  Title  = \'Observer\' \n");
       FPRINTF(PSUWopWopNameListFile_,"  translationType=\'TimeIndependent\' \n"); 
       FPRINTF(PSUWopWopNameListFile_,"  TranslationValue = 0.0, -500., 0. \n");                
   
       FPRINTF(PSUWopWopNameListFile_,"/ \n");        
          
       // ContainerIn namelist - Aircraft
   
       FPRINTF(PSUWopWopNameListFile_,"!\n");      
       FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
       FPRINTF(PSUWopWopNameListFile_,"!\n");
           
       FPRINTF(PSUWopWopNameListFile_,"&ContainerIn  \n");
       FPRINTF(PSUWopWopNameListFile_,"   Title        = \'Aircraft\' \n");
       FPRINTF(PSUWopWopNameListFile_,"   nbContainer  = %d \n",WopWopNumberOfRotors_ + WopWopNumberOfWings_ + WopWopNumberOfBodies_);
       FPRINTF(PSUWopWopNameListFile_,"   nbBase = 1 \n");
       FPRINTF(PSUWopWopNameListFile_,"   dtau = %e \n",WopWopdTau_);
       
       FPRINTF(PSUWopWopNameListFile_,"/ \n");   
       
       // Change of Base namelist for forward motion of aircraft
   
       FPRINTF(PSUWopWopNameListFile_,"&CB \n");
       FPRINTF(PSUWopWopNameListFile_,"   Title           = \'Velocity\' \n");
       FPRINTF(PSUWopWopNameListFile_,"   translationType = \'KnownFunction\' \n");
       FPRINTF(PSUWopWopNameListFile_,"   AH = 0.0, 0.0, 0.0 \n");
       FPRINTF(PSUWopWopNameListFile_,"   VH = %f,  0.0, 0.0 \n",-Vinf_*WopWopLengthConversion_);
       FPRINTF(PSUWopWopNameListFile_,"   Y0 = %f, %f, %f    \n",0.0, 0.0, 0.0);
       FPRINTF(PSUWopWopNameListFile_,"/ \n");        

       // Loop over each rotor
       
       if ( Case >= 0 ) {
          
          for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
      
             if ( ComponentGroupList_[c].GeometryIsARotor() ) {
                
                i = ComponentGroupList_[c].WopWop().RotorID();
                
                if ( Case == 0 || Case == i ) {

                   // ContainerIn for current rotor
               
                   FPRINTF(PSUWopWopNameListFile_,"!\n");      
                   FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                   FPRINTF(PSUWopWopNameListFile_,"!\n");
                
                   FPRINTF(PSUWopWopNameListFile_,"      &ContainerIn  \n");
                   FPRINTF(PSUWopWopNameListFile_,"         Title        = \'Rotor_%d\' \n",i);
                   FPRINTF(PSUWopWopNameListFile_,"         nbContainer  = %d \n",2*ComponentGroupList_[c].WopWop().NumberOfBlades());
                   FPRINTF(PSUWopWopNameListFile_,"         BPMNoiseFlag           = .false. \n");
                   FPRINTF(PSUWopWopNameListFile_,"         PeggNoiseFlag          = .false. \n");
            
                   // Possible change of base for steady state case where PSUWOPWOP takes care of the blade rotation
                   
                   if ( SteadyStateNoise_ ) {
                      
                      FPRINTF(PSUWopWopNameListFile_,"         nbBase = 1 \n");
               
                   }
                   
                   FPRINTF(PSUWopWopNameListFile_,"      / \n");     
            
                   // Change of Base namelist if we are not time accurate, and let WopWop do the unsteady stuff
                 
                   if ( SteadyStateNoise_ ) {
         
                      FPRINTF(PSUWopWopNameListFile_,"!\n");      
                      FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                      FPRINTF(PSUWopWopNameListFile_,"!\n");
                     
                      FPRINTF(PSUWopWopNameListFile_,"      &CB \n");
                      FPRINTF(PSUWopWopNameListFile_,"         Title     = \'Rotation\' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         rotation  = .true. \n");
                      FPRINTF(PSUWopWopNameListFile_,"         AngleType = \'KnownFunction\' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         Omega     =  %f \n",-BladeRPM_*2.*PI/60.);
                      FPRINTF(PSUWopWopNameListFile_,"         AxisValue = 1.0,0.0,0.0 \n");
                      FPRINTF(PSUWopWopNameListFile_,"      / \n");        
                      
                   }
            
                   if ( SteadyStateNoise_ ) {
                      
                      // Broad band noise: BPM namelist
               
                      FPRINTF(PSUWopWopNameListFile_,"!\n");      
                      FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                      FPRINTF(PSUWopWopNameListFile_,"!\n");
                      
                      FPRINTF(PSUWopWopNameListFile_,"      &BPMIn \n");
                      FPRINTF(PSUWopWopNameListFile_,"         BPMNoiseFile = %s.PSUWopWop.BPM.Rotor.%d.dat \n",FileName_,i);
                      FPRINTF(PSUWopWopNameListFile_,"         nSect           = %d \n",ComponentGroupList_[c].WopWop().NumberOfBladesSections());
                      FPRINTF(PSUWopWopNameListFile_,"         uniformBlade    = 0 \n");
                      FPRINTF(PSUWopWopNameListFile_,"         BLtrip          = 0 \n");
                      FPRINTF(PSUWopWopNameListFile_,"         sectChordFlag   = 'FILEVALUE' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         sectLengthFlag  = 'COMPUTE'   \n");
                      FPRINTF(PSUWopWopNameListFile_,"         TEThicknessFlag = 'FILEVALUE' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         TEflowAngleFlag = 'FILEVALUE' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         TipLCSFlag      = 'FILEVALUE' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         SectAOAFlag     = 'FILEVALUE' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         SectAOAFlag     = 'FILEVALUE' \n");
                      FPRINTF(PSUWopWopNameListFile_,"         LBLVSnoise      = .true.  \n");
                      FPRINTF(PSUWopWopNameListFile_,"         TBLTEnoise      = .true.  \n");
                      FPRINTF(PSUWopWopNameListFile_,"         bluntNoise      = .true.  \n");
                      FPRINTF(PSUWopWopNameListFile_,"         bladeTipNoise   = .true.  \n");
                      FPRINTF(PSUWopWopNameListFile_,"      / \n");
                
                      // Broad band noise: PeggIn namelist
                      
                      FPRINTF(PSUWopWopNameListFile_,"!\n");      
                      FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                      FPRINTF(PSUWopWopNameListFile_,"!\n");
                             
                      WriteOutPSUWopWopPeggNamelist();
                      
                   }
                   
                   // Write out containers for each of the blades... one for loading, one for thickness
                   
                   for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {
                   
                      SPRINTF(PatchLoadingGeometryName  ,"%s.PSUWopWop.Loading.Geometry.Rotor.%d.Blade.%d.dat",  FileName_,i,j);
                      SPRINTF(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Rotor.%d.Blade.%d.dat",FileName_,i,j);
                      SPRINTF(WopWopFileName,            "%s.PSUWopWop.Loading.Rotor.%d.Blade.%d.dat",           FileName_,i,j);
                        
                      // ContainerIn for Aero loading geometry and load file
               
                      FPRINTF(PSUWopWopNameListFile_,"!\n");      
                      FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                      FPRINTF(PSUWopWopNameListFile_,"!\n");
                  
                      FPRINTF(PSUWopWopNameListFile_,"            &ContainerIn  \n");
                      FPRINTF(PSUWopWopNameListFile_,"               Title        = \'Blade_%d_For_Rotor_%d_Loading\' \n",j,i);
                      FPRINTF(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
                      FPRINTF(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchLoadingGeometryName);
                      FPRINTF(PSUWopWopNameListFile_,"               patchLoadingFile   = %s \n",WopWopFileName);
                      FPRINTF(PSUWopWopNameListFile_,"            / \n");     
                   
                      FPRINTF(PSUWopWopNameListFile_,"!\n");      
                      FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                      FPRINTF(PSUWopWopNameListFile_,"!\n");
             
                      // ContainerIn for Aero thickness model
                      
                      FPRINTF(PSUWopWopNameListFile_,"            &ContainerIn  \n");
                      FPRINTF(PSUWopWopNameListFile_,"               Title        = \'Blade_%d_For_Rotor_%d_Thickness\' \n",j,i);
                      FPRINTF(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
                      FPRINTF(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
                      FPRINTF(PSUWopWopNameListFile_,"            / \n");               
                         
                   }
                      
                }
                
             }
             
          }
          
       }
   
       // Loop over each wing
       
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
   
          if ( ComponentGroupList_[c].GeometryHasWings() ) {
             
             i = ComponentGroupList_[c].WopWop().WingID();
   
             // ContainerIn for current wing
         
             FPRINTF(PSUWopWopNameListFile_,"!\n");      
             FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             FPRINTF(PSUWopWopNameListFile_,"!\n");
          
             FPRINTF(PSUWopWopNameListFile_,"      &ContainerIn  \n");
             FPRINTF(PSUWopWopNameListFile_,"         Title        = \'Wing_%d\' \n",i);
             FPRINTF(PSUWopWopNameListFile_,"         nbContainer  = %d \n",2*ComponentGroupList_[c].WopWop().NumberOfWingSurfaces());
             FPRINTF(PSUWopWopNameListFile_,"      / \n");     
      
             // Write out containers for each of the wing surfaces... one for loading, one for thickness
             
             for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {
             
                SPRINTF(PatchLoadingGeometryName  ,"%s.PSUWopWop.Loading.Geometry.Wing.%d.Surface.%d.dat",  FileName_,i,j);
                SPRINTF(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Wing.%d.Surface.%d.dat",FileName_,i,j);
                SPRINTF(WopWopFileName,            "%s.PSUWopWop.Loading.Wing.%d.Surface.%d.dat",           FileName_,i,j);
                  
                // ContainerIn for Aero loading geometry and load file
         
                FPRINTF(PSUWopWopNameListFile_,"!\n");      
                FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                FPRINTF(PSUWopWopNameListFile_,"!\n");
            
                FPRINTF(PSUWopWopNameListFile_,"            &ContainerIn  \n");
                FPRINTF(PSUWopWopNameListFile_,"               Title        = \'Surface_%d_For_Wing_%d_Loading\' \n",j,i);
                FPRINTF(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
                FPRINTF(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchLoadingGeometryName);
                FPRINTF(PSUWopWopNameListFile_,"               patchLoadingFile   = %s \n",WopWopFileName);
                FPRINTF(PSUWopWopNameListFile_,"            / \n");     
             
                FPRINTF(PSUWopWopNameListFile_,"!\n");      
                FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                FPRINTF(PSUWopWopNameListFile_,"!\n");
       
                // ContainerIn for Aero thickness model
                
                FPRINTF(PSUWopWopNameListFile_,"            &ContainerIn  \n");
                FPRINTF(PSUWopWopNameListFile_,"               Title        = \'Surface_%d_For_Wing_%d_Thickness\' \n",j,i);
                FPRINTF(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
                FPRINTF(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
                FPRINTF(PSUWopWopNameListFile_,"            / \n");               
                   
             }
                
          }
          
       }
   
       // Loop over each body
       
       for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
   
          if ( ComponentGroupList_[c].GeometryHasBodies() ) {
             
             i = ComponentGroupList_[c].WopWop().BodyID();
   
             // ContainerIn for current body
         
             FPRINTF(PSUWopWopNameListFile_,"!\n");      
             FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
             FPRINTF(PSUWopWopNameListFile_,"!\n");
          
             FPRINTF(PSUWopWopNameListFile_,"      &ContainerIn  \n");
             FPRINTF(PSUWopWopNameListFile_,"         Title        = \'Body_%d\' \n",i);
             FPRINTF(PSUWopWopNameListFile_,"         nbContainer  = %d \n",ComponentGroupList_[c].WopWop().NumberOfBodySurfaces());
             FPRINTF(PSUWopWopNameListFile_,"      / \n");     
      
             // Write out containers for each of the bodies... just the thickness
             
             for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBodySurfaces() ; j++ ) {
             
                SPRINTF(PatchThicknessGeometryName,"%s.PSUWopWop.Thickness.Geometry.Body.%d.Surface.%d.dat",FileName_,i,j);
   
                FPRINTF(PSUWopWopNameListFile_,"!\n");      
                FPRINTF(PSUWopWopNameListFile_,"!************************************************************************************************************************ \n");
                FPRINTF(PSUWopWopNameListFile_,"!\n");
       
                // ContainerIn for Aero thickness model
                
                FPRINTF(PSUWopWopNameListFile_,"            &ContainerIn  \n");
                FPRINTF(PSUWopWopNameListFile_,"               Title        = \'Surface_%d_For_Body_%d_Thickness\' \n",j,i);
                FPRINTF(PSUWopWopNameListFile_,"               nbContainer  = 0 \n");
                FPRINTF(PSUWopWopNameListFile_,"               patchGeometryFile  = %s \n",PatchThicknessGeometryName);
                FPRINTF(PSUWopWopNameListFile_,"            / \n");               
                   
             }
                
          }
          
       }
       
       fclose(PSUWopWopNameListFile_);
       
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
          
          SPRINTF(PatchGeometryName,"%s.PSUWopWop.Thickness.Geometry.Rotor.%d.Blade.%d.dat",FileName_,i,j);
          
          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().OpenThicknessGeometryFileForBlade(j,PatchGeometryName);
          
          // Geometry File Header
          
          DumInt = 42; FWRITE(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number, line 2
          
          // Units... this is just a comment
          
          SPRINTF(HeaderName,"SI");
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
      
          FWRITE(HeaderName, c_size, 32, WopFile); // Units
         
          // Comment line
          
          SPRINTF(DumChar,"ThicknessFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          FWRITE(DumChar, c_size, 1024, WopFile); // Comment lLine

          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid

          if ( SteadyStateNoise_ ) {
             
             DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Constant data

          }
          
          else {
            
             FWRITE(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic

          }
                       
          DumInt =  1;        FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - node based normals
          DumInt =  1;        FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =  0;        FWRITE(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
          DumInt =  0;        FWRITE(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          
          // Surface for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);

          SPRINTF(HeaderName,"VSPAERO_WING_%d",k);
         
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
              
          FWRITE(HeaderName, c_size, 32, WopFile);
          
          NumberI = VSPGeom().VSP_Surface(k).Surface_NumI();
          NumberJ = VSPGeom().VSP_Surface(k).Surface_NumJ();

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
   
                Period = FLOAT (ComponentGroupList_[c].Period());
          
                FWRITE(&(Period), f_size, 1, WopFile); // Period
    
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
      
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }
                       
          FWRITE(&(NumberI), i_size, 1, WopFile); // Number in I
          FWRITE(&(NumberJ), i_size, 1, WopFile); // Number in J

       }
       
    }
       
    // Loop over wings and surfaces

     if ( ComponentGroupList_[c].GeometryHasWings() ) {
       
       i = ComponentGroupList_[c].WopWop().WingID();

       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {
          
          SPRINTF(PatchGeometryName,"%s.PSUWopWop.Thickness.Geometry.Wing.%d.Surface.%d.dat",FileName_,i,j);
          
          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().OpenThicknessGeometryFileForWingSurface(j,PatchGeometryName);
          
          // Geometry File Header
          
          DumInt = 42; FWRITE(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number, line 2
          
          // Units... this is just a comment
          
          SPRINTF(HeaderName,"SI");
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
      
          FWRITE(HeaderName, c_size, 32, WopFile); // Units
         
          // Comment line
          
          SPRINTF(DumChar,"ThicknessFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          FWRITE(DumChar, c_size, 1024, WopFile); // Comment lLine

          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid

          if ( SteadyStateNoise_ ) {
             
             DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Constant data

          }
          
          else {
             
             FWRITE(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic

          }
                       
          DumInt =  1;        FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - node based normals
          DumInt =  1;        FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =  0;        FWRITE(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
          DumInt =  0;        FWRITE(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          
          // Surface for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);

          SPRINTF(HeaderName,"VSPAERO_WING_%d",k);
         
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
              
          FWRITE(HeaderName, c_size, 32, WopFile);
          
          NumberI = VSPGeom().VSP_Surface(k).Surface_NumI();
          NumberJ = VSPGeom().VSP_Surface(k).Surface_NumJ();

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
   
                Period = FLOAT(WopWopLongestPeriod_);
          
                FWRITE(&(Period), f_size, 1, WopFile); // Period
    
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
      
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }
                       
          FWRITE(&(NumberI), i_size, 1, WopFile); // Number in I
          FWRITE(&(NumberJ), i_size, 1, WopFile); // Number in J

       }
       
    }

    // Loop over body surfaces

    if ( ComponentGroupList_[c].GeometryHasBodies() ) {
       
       i = ComponentGroupList_[c].WopWop().BodyID();

       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBodySurfaces() ; j++ ) {
          
          SPRINTF(PatchGeometryName,"%s.PSUWopWop.Thickness.Geometry.Body.%d.Surface.%d.dat",FileName_,i,j);
    
          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().OpenThicknessGeometryFileForBodySurface(j,PatchGeometryName);
          
          // Geometry File Header
          
          DumInt = 42; FWRITE(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number, line 2
          
          // Units... this is just a comment
          
          SPRINTF(HeaderName,"SI");
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
      
          FWRITE(HeaderName, c_size, 32, WopFile); // Units
         
          // Comment line
          
          SPRINTF(DumChar,"ThicknessFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          FWRITE(DumChar, c_size, 1024, WopFile); // Comment lLine

          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid

          if ( SteadyStateNoise_ ) {
             
             DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Constant data

          }
          
          else {
             
             FWRITE(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic

          }
                       
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - node based normals
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
          DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          
          // Surface for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBody(j);

          SPRINTF(HeaderName,"VSPAERO_BODY_%d",k);
         
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
              
          FWRITE(HeaderName, c_size, 32, WopFile);
          
          NumberI = VSPGeom().VSP_Surface(k).Surface_NumI();
          NumberJ = VSPGeom().VSP_Surface(k).Surface_NumJ();

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
   
                Period = FLOAT (WopWopLongestPeriod_);
          
                FWRITE(&(Period), f_size, 1, WopFile); // Period
    
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
      
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }
                       
          FWRITE(&(NumberI), i_size, 1, WopFile); // Number in I
          FWRITE(&(NumberJ), i_size, 1, WopFile); // Number in J

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
    VSPAERO_DOUBLE Translation[3];
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
      
          Time = FLOAT (CurrentNoiseTime_);
       
          if ( !SteadyStateNoise_ ) FWRITE(&Time, f_size, 1, WopFile);             
                          
          // X node values
    
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {

                x = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_x(m,n) + Translation[0] ) );
                
                FWRITE(&(x), f_size, 1, WopFile);
           
             }
             
          }
          
          // Y node values
          
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
  
                y = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_y(m,n) + Translation[1] ) );
     
                FWRITE(&(y), f_size, 1, WopFile);
                
             }
             
          }

          // z node values
          
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
    
                z = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_z(m,n) + Translation[2] ) );
      
                FWRITE(&(z), f_size, 1, WopFile);                
                
             }
             
          } 
          
          // X Normal values
           
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = FLOAT ( -VSPGeom().VSP_Surface(k).Surface_Nx(m,n) );
        
                FWRITE(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          } 
          
          // Y Normal values
              
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = FLOAT ( -VSPGeom().VSP_Surface(k).Surface_Ny(m,n) );
           
                FWRITE(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          }  
          
          // Z Normal values
              
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = FLOAT ( -VSPGeom().VSP_Surface(k).Surface_Nz(m,n) );
                
                FWRITE(&(DumFloat), f_size, 1, WopFile);
                
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
      
          Time = FLOAT (CurrentNoiseTime_);
       
          if ( !SteadyStateNoise_ ) FWRITE(&Time, f_size, 1, WopFile);             
                          
          // X node values
    
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
                
                x = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_x(m,n) + Translation[0] ) );

                FWRITE(&(x), f_size, 1, WopFile);
           
             }
             
          }
          
          // Y node values
          
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {

                y = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_y(m,n) + Translation[1] ) );
     
                FWRITE(&(y), f_size, 1, WopFile);
                
             }
             
          }

          // z node values
          
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
   
                z = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_z(m,n) + Translation[2] ) );
      
                FWRITE(&(z), f_size, 1, WopFile);                
                
             }
             
          } 
          
          // X Normal values
           
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = FLOAT ( -VSPGeom().VSP_Surface(k).Surface_Nx(m,n) );
        
                FWRITE(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          } 
          
          // Y Normal values
              
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = FLOAT ( -VSPGeom().VSP_Surface(k).Surface_Ny(m,n) );
           
                FWRITE(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          }  
          
          // Z Normal values
              
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = FLOAT ( -VSPGeom().VSP_Surface(k).Surface_Nz(m,n) );
                
                FWRITE(&(DumFloat), f_size, 1, WopFile);
                
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
      
          Time = FLOAT (CurrentNoiseTime_);
       
          if ( !SteadyStateNoise_ ) FWRITE(&Time, f_size, 1, WopFile);             
                          
          // X node values
    
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {

                x = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_x(m,n) + Translation[0] ) );
                
                FWRITE(&(x), f_size, 1, WopFile);
           
             }
             
          }
          
          // Y node values
          
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
   
                y = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_y(m,n) + Translation[1] ) );
     
                FWRITE(&(y), f_size, 1, WopFile);
                
             }
             
          }

          // z node values
          
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
  
                z = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_z(m,n) + Translation[2] ) );
      
                FWRITE(&(z), f_size, 1, WopFile);                
                
             }
             
          } 
          
          // X Normal values
           
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = FLOAT ( -VSPGeom().VSP_Surface(k).Surface_Nx(m,n) );
        
                FWRITE(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          } 
          
          // Y Normal values
              
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = FLOAT ( -VSPGeom().VSP_Surface(k).Surface_Ny(m,n) );
           
                FWRITE(&(DumFloat), f_size, 1, WopFile);
                
             }
             
          }  
          
          // Z Normal values
              
          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
             
             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
          
                DumFloat = FLOAT ( -VSPGeom().VSP_Surface(k).Surface_Nz(m,n) );
                
                FWRITE(&(DumFloat), f_size, 1, WopFile);
                
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
          
          SPRINTF(PatchGeometryName,"%s.PSUWopWop.Loading.Geometry.Rotor.%d.Blade.%d.dat",FileName_,i,j);

          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().OpenLoadingGeometryFileForBlade(j,PatchGeometryName);

          // Geometry File Header
          
          DumInt = 42; FWRITE(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number, line 2
          
          // Force units... this is just a comment
          
          SPRINTF(HeaderName,"Pa\n");
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
      
          FWRITE(HeaderName, c_size, 32, WopFile); // Units
         
          // Comment line
          
          SPRINTF(DumChar,"CompactGeometryFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          FWRITE(DumChar, c_size, 1024, WopFile); // Comment lLine

          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid

          if ( SteadyStateNoise_ ) {
             
             DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Constant data

          }
          
          else {
             
             FWRITE(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic


          }
                       
          DumInt =  1;        FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - node based normals
          DumInt =  1;        FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =  0;        FWRITE(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
          DumInt =  0;        FWRITE(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          
          // Write out surface inforamation for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);
          
          SPRINTF(HeaderName,"Blade_%d_Rotor_%d",i,j);
         
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
              
          FWRITE(HeaderName, c_size, 32, WopFile);

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

                Period = FLOAT ( ComponentGroupList_[c].Period() );
                
                FWRITE(&(Period), f_size, 1, WopFile); // Period
                
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
                
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }
                     
          NumberI = 1;
          NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
                                 
          FWRITE(&(NumberI),            i_size, 1, WopFile); // Number in I
          FWRITE(&(NumberJ),            i_size, 1, WopFile); // Number in J

       }
       
    }

    // Loop over wings and surfaces
 
    if ( ComponentGroupList_[c].GeometryHasWings() ) {
       
       i = ComponentGroupList_[c].WopWop().WingID();
    
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ;  j++ ) {
          
          SPRINTF(PatchGeometryName,"%s.PSUWopWop.Loading.Geometry.Wing.%d.Surface.%d.dat",FileName_,i,j);

          // Thickness Geometry 

          WopFile = ComponentGroupList_[c].WopWop().OpenLoadingGeometryFileForWingSurface(j,PatchGeometryName);

          // Geometry File Header
          
          DumInt = 42; FWRITE(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number, line 2
          
          // Force units... this is just a comment
          
          SPRINTF(HeaderName,"Pa\n");
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
      
          FWRITE(HeaderName, c_size, 32, WopFile); // Units
         
          // Comment line
          
          SPRINTF(DumChar,"CompactGeometryFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          FWRITE(DumChar, c_size, 1024, WopFile); // Comment lLine

          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid

          if ( SteadyStateNoise_ ) {
             
             DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Constant data

          }
          
          else {
             
             FWRITE(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic


          }
                       
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - node based normals
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
          DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          
          // Write out surface inforamation for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);
          
          SPRINTF(HeaderName,"Blade_%d_Rotor_%d",i,j);
         
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
              
          FWRITE(HeaderName, c_size, 32, WopFile);

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

                Period = FLOAT ( WopWopLongestPeriod_ );
                
                FWRITE(&(Period), f_size, 1, WopFile); // Period
                
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
                
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }
                     
          NumberI = 1;
          NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
                                 
          FWRITE(&(NumberI),            i_size, 1, WopFile); // Number in I
          FWRITE(&(NumberJ),            i_size, 1, WopFile); // Number in J

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
    float x, y, z, Time, Translation[3];
    FILE *WopFile;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Vehicle translation vector
    
    Translation[0] = Translation[1] = Translation[2] = 0.;
    
    if ( !SteadyStateNoise_ && WopWopPeriodicity_ == WOPWOP_APERIODIC ) {
           
       Translation[0] = FLOAT ( -FreeStreamVelocity_[0] * CurrentNoiseTime_ );
       Translation[1] = FLOAT ( -FreeStreamVelocity_[1] * CurrentNoiseTime_ );
       Translation[2] = FLOAT ( -FreeStreamVelocity_[2] * CurrentNoiseTime_ );
       
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
      
          Time = FLOAT ( CurrentNoiseTime_ );
        
          if ( !SteadyStateNoise_ ) FWRITE(&Time, f_size, 1, WopFile);         

          // X node values

          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             x = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).xTE(m) + 0.75*(VSPGeom().VSP_Surface(k).xLE(m) - VSPGeom().VSP_Surface(k).xTE(m)) + Translation[0] ) );

             FWRITE(&(x), f_size, 1, WopFile);
        
          }

          // Y node values
          
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             y = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).yTE(m) + 0.75*(VSPGeom().VSP_Surface(k).yLE(m) - VSPGeom().VSP_Surface(k).yTE(m)) + Translation[1] ) );

             FWRITE(&(y), f_size, 1, WopFile);
                     
          }

          // z node values
          
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             z = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).zTE(m) + 0.75*(VSPGeom().VSP_Surface(k).zLE(m) - VSPGeom().VSP_Surface(k).zTE(m)) + Translation[2] ) );

             FWRITE(&(z), f_size, 1, WopFile);
        
          }

          // X Normal values
           
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             x = FLOAT ( VSPGeom().VSP_Surface(k).NxQC(m) );

             FWRITE(&(x), f_size, 1, WopFile);
        
          }
          
          // Y Normal values
              
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             y = FLOAT ( VSPGeom().VSP_Surface(k).NyQC(m) );

             FWRITE(&(y), f_size, 1, WopFile);
        
          }
          
          // Z Normal values
              
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             z = FLOAT ( VSPGeom().VSP_Surface(k).NzQC(m) );

             FWRITE(&(z), f_size, 1, WopFile);
        
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
      
          Time = FLOAT ( CurrentNoiseTime_ );
        
          if ( !SteadyStateNoise_ ) FWRITE(&Time, f_size, 1, WopFile);         

          // X node values

          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             x = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).xTE(m) + 0.75*(VSPGeom().VSP_Surface(k).xLE(m) - VSPGeom().VSP_Surface(k).xTE(m)) + Translation[0] ) );

             FWRITE(&(x), f_size, 1, WopFile);
        
          }

          // Y node values
          
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             y = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).yTE(m) + 0.75*(VSPGeom().VSP_Surface(k).yLE(m) - VSPGeom().VSP_Surface(k).yTE(m)) + Translation[1] ) );

             FWRITE(&(y), f_size, 1, WopFile);
                     
          }

          // z node values
          
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             z = FLOAT ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).zTE(m) + 0.75*(VSPGeom().VSP_Surface(k).zLE(m) - VSPGeom().VSP_Surface(k).zTE(m)) + Translation[2] ) );

             FWRITE(&(z), f_size, 1, WopFile);
        
          }

          // X Normal values
           
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             x = FLOAT ( VSPGeom().VSP_Surface(k).NxQC(m) );

             FWRITE(&(x), f_size, 1, WopFile);
        
          }
          
          // Y Normal values
              
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             y = FLOAT ( VSPGeom().VSP_Surface(k).NyQC(m) );

             FWRITE(&(y), f_size, 1, WopFile);
        
          }
          
          // Z Normal values
              
          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {

             z = FLOAT ( VSPGeom().VSP_Surface(k).NzQC(m) );

             FWRITE(&(z), f_size, 1, WopFile);
        
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

          SPRINTF(WopWopFileName, "%s.PSUWopWop.Loading.Rotor.%d.Blade.%d.dat",FileName_,i,j);

          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().OpenLoadingFileForBlade(j, WopWopFileName);

          // Write out surface information for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);

          // Load File Header
          
          DumInt = 42; FWRITE(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number, line 2
      
          FWRITE(DumChar, c_size, 1024, WopFile); // Comment Line   
              
          DumInt = 2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 indicates this is a loading file
          DumInt = 1; FWRITE(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt = 1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid
          
          if ( SteadyStateNoise_ ) {
             
             DumInt = 1; FWRITE(&(DumInt), i_size, 1, WopFile); // Constant data
             
          }
          
          else {
             
             FWRITE(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic
             
          }
                
          DumInt =   1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - node centered normals
          DumInt =   2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 - data is loading vector
          
          if ( SteadyStateNoise_ ) {
             
             DumInt = 2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 - Rotating frame
        
          }           
                      
          else {      
                      
             DumInt = 1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - Ground reference fixed frame
             
          }
                    
          DumInt =   1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =   0; FWRITE(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          DumInt =   0; FWRITE(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          DumInt =   1; FWRITE(&(DumInt), i_size, 1, WopFile); // Number of zones with data
          DumInt =  -1; FWRITE(&(DumInt), i_size, 1, WopFile); //negative since compact loading

          SPRINTF(HeaderName,"Blade_%d_Rotor_%d",i,j);
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
                  
          FWRITE(HeaderName, c_size, 32, WopFile); // Patch name
   
          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

                ComponentGroupList_[c].Period();
                
                FWRITE(&(Period), f_size, 1, WopFile); // Period
                
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
                
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }

          NumberI = 1;
          NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
         
          FWRITE(&(NumberI), i_size, 1, WopFile); // NI
          FWRITE(&(NumberJ), i_size, 1, WopFile); // NJ = NumberOfSpanStations

       }
       
    }

    // Loop over wings and surfaces

    if ( ComponentGroupList_[c].GeometryHasWings() ) {
       
       i = ComponentGroupList_[c].WopWop().WingID();
    
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {

          SPRINTF(WopWopFileName, "%s.PSUWopWop.Loading.Wing.%d.Surface.%d.dat",FileName_,i,j);

          // Thickness Geometry file
          
          WopFile = ComponentGroupList_[c].WopWop().OpenLoadingFileForWingSurface(j, WopWopFileName);

          // Write out surface information for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);

          // Load File Header
          
          DumInt = 42; FWRITE(&(DumInt), i_size, 1, WopFile); // Magic Number
          DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number
          DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number, line 2
      
          FWRITE(DumChar, c_size, 1024, WopFile); // Comment Line   
              
          DumInt = 2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 indicates this is a loading file
          DumInt = 1; FWRITE(&(DumInt), i_size, 1, WopFile); // Number of zones
          DumInt = 1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid
          
          if ( SteadyStateNoise_ ) {
             
             DumInt = 1; FWRITE(&(DumInt), i_size, 1, WopFile); // Constant data
             
          }
          
          else {
             
             FWRITE(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic
             
          }
                
          DumInt =   1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - node centered normals
          DumInt =   2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 - data is loading vector
          
          if ( SteadyStateNoise_ ) {
             
             DumInt = 2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 - Rotating frame
        
          }           
                      
          else {      
                      
             DumInt = 1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - Ground reference fixed frame
             
          }
                    
          DumInt =   1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
          DumInt =   0; FWRITE(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          DumInt =   0; FWRITE(&(DumInt), i_size, 1, WopFile); // 0, reserved 
          DumInt =   1; FWRITE(&(DumInt), i_size, 1, WopFile); // Number of zones with data
          DumInt =  -1; FWRITE(&(DumInt), i_size, 1, WopFile); //negative since compact loading

          SPRINTF(HeaderName,"Wing_%d_Surface_%d",i,j);
          
          Length = strlen(HeaderName);
          memset(&HeaderName[Length], ' ', 32 - Length);
                  
          FWRITE(HeaderName, c_size, 32, WopFile); // Patch name
   
          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

                Period = FLOAT( WopWopLongestPeriod_ );
                
                FWRITE(&(Period), f_size, 1, WopFile); // Period
                
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
                
             }
             
             else {
                
                FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

             }
             
          }

          NumberI = 1;
          NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
         
          FWRITE(&(NumberI), i_size, 1, WopFile); // NI
          FWRITE(&(NumberJ), i_size, 1, WopFile); // NJ = NumberOfSpanStations

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
    
    DynP = FLOAT( 0.5 * Density_ * Vinf_ * Vinf_ * WopWopPressureConversion_ );

    // Loop over rotors and blades

    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
       
       i = ComponentGroupList_[c].WopWop().RotorID();
     
       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ;  j++ ) {
  
          // Loading file
          
          WopFile = ComponentGroupList_[c].WopWop().LoadingFileForBlade(j);

          // Write out surface inforamation for this blade
          
          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);
          
          // Current time
      
          Time = FLOAT( CurrentNoiseTime_ );
     
          if ( !SteadyStateNoise_ ) FWRITE(&Time, f_size, 1, WopFile);

          NumberOfStations = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
              
          // X force per length
              
          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
             
             DumFloat = FLOAT( DynP * SpanLoadData(k).Span_Cx(m) * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );

             FWRITE(&(DumFloat), f_size, 1, WopFile);
    
          }
          
          // Y force per length
              
          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
             
             DumFloat = FLOAT( DynP * SpanLoadData(k).Span_Cy(m) * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );

             FWRITE(&(DumFloat), f_size, 1, WopFile);
         
          }
          
          // Z force per length
              
          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
             
             DumFloat = FLOAT( DynP * SpanLoadData(k).Span_Cz(m) * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );

             FWRITE(&(DumFloat), f_size, 1, WopFile);
    
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
      
          Time = FLOAT( CurrentNoiseTime_ );
     
          if ( !SteadyStateNoise_ ) FWRITE(&Time, f_size, 1, WopFile);

          NumberOfStations = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
              
          // X force per length
              
          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
             
             DumFloat = FLOAT( DynP * SpanLoadData(k).Span_Cx(m) * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );

             FWRITE(&(DumFloat), f_size, 1, WopFile);
    
          }
          
          // Y force per length
              
          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
             
             DumFloat = FLOAT( DynP * SpanLoadData(k).Span_Cy(m) * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );

             FWRITE(&(DumFloat), f_size, 1, WopFile);
         
          }
          
          // Z force per length
              
          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
             
             DumFloat = FLOAT( DynP * SpanLoadData(k).Span_Cz(m) * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );

             FWRITE(&(DumFloat), f_size, 1, WopFile);
    
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
    VSPAERO_DOUBLE x1, y1, z1, x2, y2, z2;
    VSPAERO_DOUBLE BladeArea, BladeRadius = 0, Thrust, BladeCL, RotationSpeed;
       
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

             BladeArea += SpanLoadData(k).Span_Area(i);

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
        
    FPRINTF(PSUWopWopNameListFile_,"      &PeggIn\n");

    FPRINTF(PSUWopWopNameListFile_,"         TotalBladeAreaFlag = \'USERVALUE\' \n");
    FPRINTF(PSUWopWopNameListFile_,"         TotalBladeArea     = %lf \n",BladeArea*WopWopAreaConversion_);
                                                               
    FPRINTF(PSUWopWopNameListFile_,"         BladeRadiusFlag    = \'USERVALUE\' \n");
    FPRINTF(PSUWopWopNameListFile_,"         BladeRadius        = %lf \n",BladeRadius*WopWopLengthConversion_);
                                                               
    FPRINTF(PSUWopWopNameListFile_,"         RotSpeedFlag       = \'USERVALUE\' \n");
    FPRINTF(PSUWopWopNameListFile_,"         RotSpeed           = %lf \n",RotationSpeed);
                                                               
    FPRINTF(PSUWopWopNameListFile_,"         CLBarFlag          = \'USERVALUE\' \n");
    FPRINTF(PSUWopWopNameListFile_,"         CLBar              = %lf \n",BladeCL);
                                                               
    FPRINTF(PSUWopWopNameListFile_,"         TotalThrustFlag    = \'USERVALUE\' \n");
    FPRINTF(PSUWopWopNameListFile_,"         TotalThrust        = %lf \n",Thrust*WopWopForceConversion_);
                                                               
    FPRINTF(PSUWopWopNameListFile_,"         HubAxisFlag        = \'USERVALUE\' \n");
    FPRINTF(PSUWopWopNameListFile_,"         HubAxis            = %f, %f, %f \n",-1., 0., 0.);

    FPRINTF(PSUWopWopNameListFile_,"      / \n");        

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

       SPRINTF(WopWopFileName,"%s.PSUWopWop.BPM.Rotor.%d.dat",FileName_,i);

       // BPM file
       
       WopFile = ComponentGroupList_[c].WopWop().OpenBPMFile(WopWopFileName);

       // Write out surface inforamation for this blade
       
       k = ComponentGroupList_[c].WopWop().SurfaceForBlade(1);
    
       // BPM File Header
       
       NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
    
       DumInt = 42; FWRITE(&(DumInt ), i_size, 1, WopFile); // Magic Number
                    FWRITE(&(NumberJ), i_size, 1, WopFile); // Number of sections definining the blade
       DumInt =  0; FWRITE(&(DumInt ), i_size, 1, WopFile); // Non-uniform blade sections
       DumInt =  1; FWRITE(&(DumInt ), i_size, 1, WopFile); // Blade section chord data provided
       DumInt =  0; FWRITE(&(DumInt ), i_size, 1, WopFile); // Blade section length data not provided
       DumInt =  1; FWRITE(&(DumInt ), i_size, 1, WopFile); // Blade section TE thickness provided
       DumInt =  1; FWRITE(&(DumInt ), i_size, 1, WopFile); // Blade section TE flow angle provided
       DumInt =  1; FWRITE(&(DumInt ), i_size, 1, WopFile); // Blade section effective AoA provided
       DumInt =  1; FWRITE(&(DumInt ), i_size, 1, WopFile); // Blade section tip lift curve slope provided
       DumInt =  1; FWRITE(&(DumInt ), i_size, 1, WopFile); // Blade section free stream velocity provided
   
       if ( !TimeAccurate_ ) {
          
          DumInt = 1; FWRITE(&(DumInt), i_size, 1, WopFile); // constant data
                   
       }           
                   
       else {      
                   
          DumInt = 3; FWRITE(&(DumInt), i_size, 1, WopFile); // aperiodic data
          
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
          
          DumFloat = FLOAT( VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );

          FWRITE(&(DumFloat), f_size, 1, WopFile);
   
          // Span
          
          DumFloat = FLOAT( SpanLoadData(k).Span_Area(m) / VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );

// djk... let psu-wopwop calculate this
          
     //     FWRITE(&(DumFloat), f_size, 1, WopFile);
          
          // Blade section TE thickness
          
          DumFloat = FLOAT( 0.0005 * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );
                
          FWRITE(&(DumFloat), f_size, 1, WopFile);
          
          // Blade section TE flow angle, radians
          
          DumFloat = 0.244;
            
          FWRITE(&(DumFloat), f_size, 1, WopFile);             
   
       }
    
       for ( m = 1 ; m <= NumberOfStations ; m++ ) {
          
          // Blade section effective AoA
          
          DumFloat = 0.0;
           
          FWRITE(&(DumFloat), f_size, 1, WopFile);        
   
          // Blade section tip lift curve slope
          
          DumFloat = 1.0;
                      
          FWRITE(&(DumFloat), f_size, 1, WopFile); 
          
          // Blade section free stream speed
          
          DumFloat = FLOAT( Vinf_ * WopWopLengthConversion_ );
            
          FWRITE(&(DumFloat), f_size, 1, WopFile);     

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
    
    SPRINTF(PatchGeometryName, "%s.PSUWopWop.Loading.Geometry.dat",FileName_);
    
    // Geometry file
    
    if ( (WopFile = fopen(PatchGeometryName, "wb")) == NULL ) {

       PRINTF("Could not open the PSUWopWop Case File output! \n");

       exit(1);

    }    
    
    // Geometry File Header
    
    DumInt = 42; FWRITE(&(DumInt), i_size, 1, WopFile); // Magic Number
    DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number
    DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number, line 2
  
    // Force units... this is just a comment
    
    SPRINTF(HeaderName,"Pascals\n");
    
    Length = strlen(HeaderName);
    memset(&HeaderName[Length], ' ', 32 - Length);

    FWRITE(HeaderName, c_size, 32, WopFile); // Units
   
    // Comment line
    
    SPRINTF(DumChar,"VSPAERO Created PSUWopWop Loading Geometry File");

    Length = strlen(DumChar);
    memset(&DumChar[Length], ' ', 1024 - Length);
        
    FWRITE(DumChar, c_size, 1024, WopFile); // Comment lLine
        
    DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
    DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1, single, zone for now
    DumInt =  2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 indicates this is an unstructured grid
    
    if ( SteadyStateNoise_ ) {
    
       DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - Constant data
       
    }
    
    else {
       
       FWRITE(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic
       
    }       
       
    DumInt =  2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 - faced centered normals
    DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
    DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
    DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // 0, reserved 
    
    SPRINTF(HeaderName,"Zone_1_Geom_Uns");
    
    Length = strlen(HeaderName);
    memset(&HeaderName[Length], ' ', 32 - Length);
        
    FWRITE(HeaderName, c_size, 32, WopFile);
    
    Level = 0;
    
    NumberOfNodes = VSPGeom().Grid(Level).NumberOfNodes();
    NumberOfLoops = VSPGeom().Grid(Level).NumberOfLoops();
    
    if ( !SteadyStateNoise_ ) {
         
       if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

          Period = FLOAT( WopWopLongestPeriod_ );
          
          FWRITE(&(Period), f_size, 1, WopFile); // Period
          
          FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
          
       }
       
       else {
          
          FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

       }
       
    }
    
    FWRITE(&(NumberOfNodes),      i_size, 1, WopFile); // Number of nodes
    FWRITE(&(NumberOfLoops),      i_size, 1, WopFile); // Number of loops
    
    // Write out loop connnectivity

    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {
       
       DumInt = VSPGeom().Grid(Level).LoopList(i).NumberOfNodes();
  
       FWRITE(&(DumInt), i_size, 1, WopFile);
       
       for ( j = 1 ; j <= VSPGeom().Grid(Level).LoopList(i).NumberOfNodes() ; j++ ) {
          
          Node = VSPGeom().Grid(Level).LoopList(i).Node(j);

          FWRITE(&Node, i_size, 1, WopFile);
          
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
    VSPAERO_DOUBLE Translation[3];
    float DumFloat, x, y, z, Time;
    FILE *WopFile = 0;

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
    
    Time = FLOAT( CurrentNoiseTime_ );
 
    if ( !SteadyStateNoise_ ) FWRITE(&Time, f_size, 1, WopFile);

    Level = 0;
       
    // X node values
       
    for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

       x = FLOAT( WopWopLengthConversion_ * ( VSPGeom().Grid(Level).NodeList(j).x() + Translation[0] ) );
       
       FWRITE(&(x), f_size, 1, WopFile);

    }
    
    // Y node values
    
    for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

       y = FLOAT( WopWopLengthConversion_ * ( VSPGeom().Grid(Level).NodeList(j).y() + Translation[1] ) );

       FWRITE(&(y), f_size, 1, WopFile);
  
    }
    
    // Z node values
    
    for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

       z = FLOAT( WopWopLengthConversion_ * ( VSPGeom().Grid(Level).NodeList(j).z() + Translation[2] ) );

       FWRITE(&(z), f_size, 1, WopFile);         
  
    }        

    // X Normal values
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       DumFloat = FLOAT( WopWopAreaConversion_ * ( VSPGeom().Grid(Level).LoopList(i).Nx() * VSPGeom().Grid(Level).LoopList(i).Area()) );

       FWRITE(&DumFloat, f_size, 1, WopFile);

    }    
    
    // Y Normal values
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       DumFloat = FLOAT( WopWopAreaConversion_ * ( VSPGeom().Grid(Level).LoopList(i).Ny() * VSPGeom().Grid(Level).LoopList(i).Area()) );

       FWRITE(&DumFloat, f_size, 1, WopFile);

    }    
    
    // Z Normal values
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       DumFloat = FLOAT( WopWopAreaConversion_ * ( VSPGeom().Grid(Level).LoopList(i).Nz() * VSPGeom().Grid(Level).LoopList(i).Area()) );

       FWRITE(&DumFloat, f_size, 1, WopFile);

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
    
    SPRINTF(WopWopFileName, "%s.PSUWopWop.Loading.dat",FileName_);
    
    if ( (WopFile = fopen(WopWopFileName, "wb")) == NULL ) {

       PRINTF("Could not open the PSUWopWop Case File output! \n");

       exit(1);

    }        
    
    // Load File Header
    
    DumInt = 42; FWRITE(&(DumInt), i_size, 1, WopFile); // Magic Number
    DumInt =  1; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number
    DumInt =  0; FWRITE(&(DumInt), i_size, 1, WopFile); // Version number, line 2

    FWRITE(DumChar, c_size, 1024, WopFile); // Comment lLine
        
    DumInt =   2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 indicates this is a loading file
    DumInt =   1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1, single, zone for now
    DumInt =   2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 indicates this is an unstructured grid
 
    if ( !SteadyStateNoise_ ) {

       DumInt = 1; FWRITE(&(DumInt), i_size, 1, WopFile); // Constant data
       
    }
    
    else {
       
       FWRITE(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... periodic, or aperiodic
       
    }
  
    DumInt =   2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 - faced centered normals
    DumInt =   2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 - data is loading vector
    
    if ( SteadyStateNoise_ ) {
       
       DumInt = 2; FWRITE(&(DumInt), i_size, 1, WopFile); // 2 - Rotating frame
                
    }           
                
    else {      
                
       DumInt = 1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - Ground reference fixed frame
       
    }
    
    DumInt =   1; FWRITE(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
    DumInt =   0; FWRITE(&(DumInt), i_size, 1, WopFile); // 0, reserved 
    DumInt =   0; FWRITE(&(DumInt), i_size, 1, WopFile); // 0, reserved 
    DumInt =   1; FWRITE(&(DumInt), i_size, 1, WopFile); // Number of zones with data
    DumInt =  -1; FWRITE(&(DumInt), i_size, 1, WopFile); // Zone list, negative since VLM

    SPRINTF(HeaderName,"Zone_1_Load_Uns");
    
    Length = strlen(HeaderName);
    memset(&HeaderName[Length], ' ', 32 - Length);
            
    FWRITE(HeaderName, c_size, 32, WopFile); // Patch name
 
    Level = 0;
    
    NumberOfLoops = VSPGeom().Grid(Level).NumberOfLoops();
     
    if ( !SteadyStateNoise_ ) {
         
       if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

          Period = FLOAT( WopWopLongestPeriod_ );
          
          FWRITE(&(Period), f_size, 1, WopFile); // Period
          
          FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
          
       }
       
       else {
          
          FWRITE(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    

       }
       
    }
    
    FWRITE(&(NumberOfLoops),      i_size, 1, WopFile); // Number of loops
       
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

    Time = FLOAT( CurrentNoiseTime_ );

    if ( !SteadyStateNoise_ ) FWRITE(&Time, f_size, 1, WopFile);

    // Forces
    
    DynP = FLOAT( 0.5 * Density_ * Vinf_ * Vinf_ * WopWopPressureConversion_ );

    Level = 0;

    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       Fx = FLOAT( -DynP * VSPGeom().Grid(Level).LoopList(i).dCp() * VSPGeom().Grid(Level).LoopList(i).Nx() );

       FWRITE(&Fx, f_size, 1, WopFile);
     
    }  
     
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       Fy = FLOAT( -DynP * VSPGeom().Grid(Level).LoopList(i).dCp() * VSPGeom().Grid(Level).LoopList(i).Ny() );

       FWRITE(&Fy, f_size, 1, WopFile);
    
    }   
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       Fz = FLOAT( -DynP * VSPGeom().Grid(Level).LoopList(i).dCp() * VSPGeom().Grid(Level).LoopList(i).Nz() );

       FWRITE(&Fz, f_size, 1, WopFile);
    
    }    

}
 
 
