//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#ifndef WIN32
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "VSP_Solver.H"
#include "ControlSurfaceGroup.H"

#define INTEGER_VALUE 0
#define double_VALUE  1

using namespace VSPAERO_SOLVER;

#define VER_MAJOR 7
#define VER_MINOR 2
#define VER_PATCH 0

char *FileName;

double Sref_;
double Cref_;
double Bref_;
double Xcg_;
double Ycg_;
double Zcg_;
double Cl_;
double Cd_;
double Cm_;
double Mach_;
double AoA_;
double Beta_;
double Rho_;
double Vinf_;
double Vref_;
double Machref_;
double ReCref_;
double Clo2D_;
double MaxTurningAngle_;
double FarDist_;
double TimeStep_;
double ReducedFrequency_;
double UnsteadyAngleMax_;
double UnsteadyHMax_;     
double HeightAboveGround_;
double BladeRPM_;
double WakeRelax_;
double FarAway_;
double ForwardGMRESConvergenceFactor_;
double AdjointGMRESConvergenceFactor_;
double NonLinearConvergenceFactor_;
double CoreSizeFactor_;

int AdjointSolutionForceType_;
int NumberOfAdjointForceAndMomentCases_;
int AdjointSolveForThisForceMomentCase_[7];
int NumberOfAdjointComponentListSettings_;
int UserAdjointComponentList_[10000];

#define MAXRUNCASES 100000

// Number of Machs, AoAs, and Betas

int NumberOfMachs_;
int NumberOfAoAs_;
int NumberOfBetas_;
int NumberOfReCrefs_;

// Mach, AoA, and Beta Lists

double   MachList_[MAXRUNCASES];
double    AoAList_[MAXRUNCASES];
double   BetaList_[MAXRUNCASES];
double ReCrefList_[MAXRUNCASES];

// Control surfaces

int NumberOfControlGroups_;
CONTROL_SURFACE_GROUP *ControlSurfaceGroup_;

// Sability and control Mach, AoA, and Beta Lists

double Stab_MachList_[MAXRUNCASES];
double  Stab_AoAList_[MAXRUNCASES];
double Stab_BetaList_[MAXRUNCASES];

// Stability Rotational rates list

double RotationalRate_pList_[MAXRUNCASES];
double RotationalRate_qList_[MAXRUNCASES];
double RotationalRate_rList_[MAXRUNCASES];

// Deltas for calculating derivaties

double Delta_AoA_;
double Delta_Beta_;
double Delta_Mach_;
double Delta_P_;
double Delta_Q_;
double Delta_R_;
double Delta_Control_;

// Raw stability data

double CFxForCase[MAXRUNCASES];
double CFyForCase[MAXRUNCASES];
double CFzForCase[MAXRUNCASES];

double CMxForCase[MAXRUNCASES];
double CMyForCase[MAXRUNCASES];
double CMzForCase[MAXRUNCASES];

double CLForCase[MAXRUNCASES];
double CDForCase[MAXRUNCASES];
double CSForCase[MAXRUNCASES];

double CDtForCase[MAXRUNCASES];

double CMlForCase[MAXRUNCASES];
double CMmForCase[MAXRUNCASES];
double CMnForCase[MAXRUNCASES];

double CDoForCase[MAXRUNCASES];

double OptimizationFunctionForCase[MAXRUNCASES];

// Stability derivatives

double dCFx_wrt[MAXRUNCASES];
double dCFy_wrt[MAXRUNCASES];
double dCFz_wrt[MAXRUNCASES];

double dCMx_wrt[MAXRUNCASES];
double dCMy_wrt[MAXRUNCASES];
double dCMz_wrt[MAXRUNCASES];

double dCL_wrt[MAXRUNCASES];
double dCD_wrt[MAXRUNCASES];
double dCS_wrt[MAXRUNCASES];

double dCMl_wrt[MAXRUNCASES];
double dCMm_wrt[MAXRUNCASES];
double dCMn_wrt[MAXRUNCASES];

FILE *StabFile;
FILE *VorviewFlt;

int WakeIterations_                  = 0;
int NumberOfRotors_                  = 0;
int NumStabCases_                    = 7;
int NumberOfThreads_                 = 1;
int StabControlRun_                  = 0;
int SetFreeStream_                   = 0;
int SaveRestartFile_                 = 0;
int DoRestartRun_                    = 0;
int SetFarDist_                      = 0;
int Symmetry_                        = 0;
int NumberOfWakeNodes_               = 0;
int DumpGeom_                        = 0;
int ImplicitWake_                    = 0;
int ImplicitWakeStartIteration_      = 0;
int NumberofSurveyPoints_            = 0;
int NumberOfSurveyTimeSteps_         = 0;
int LoadFEMDeformation_              = 0;
int DoGroundEffectsAnalysis_         = 0;
int Write2DFEMFile_                  = 0;
int WriteTecplotFile_                = 0;
int DoUnsteadyAnalysis_              = 0;
int StartFromSteadyState_            = 0;
int UnsteadyAnalysisType_            = 0;
int NumberOfTimeSteps_               = 0;
int StartAveragingTimeStep_          = 1;
int CreateNoiseFiles_                = 0;
int NoPanelSpanWiseLoading_          = 0;
int DoAdjointSolve_                  = 0;
int OptimizationSolve_               = 0;
int OptimizationFunction_            = 0;
int SetupHighLiftFile_               = 0;
int RestartAndInterrogateSolution_   = 0;
int NumberOfQuadTrees_               = 0;
int NumberOfInlets_                  = 0;
int NumberOfNozzles_                 = 0;
int DoFiniteDiffTest                 = 0;
int StallModelIsOn                   = 0;
int SolveOnMGLevel_                  = 0;
int UpdateMatrixPreconditioner_      = 0;
int UseWakeNodeMatrixPreconditioner_ = 0;
int SymmetryFlag_                    = 0;
int FreezeMultiPoleAtIteration_      = 100000;
int FreezeWakeAtIteration_           = 100000;
int FreezeWakeRootVortices_          = 0;
int QuadTreeBufferLevels_            = 0;
int VSPAERO_InputFileFormatOutput_   = 0;

// Prototypes

int main(int argc, char **argv);
void PrintUsageHelp();
void ParseInput(int argc, char *argv[]);
void LoadCaseFile(int ReadFlag);
void ApplyControlDeflections(void);
void Solve(void);
void FiniteDifference_StabilityAndControlSolve(void);
void CalculateStabilityDerivatives(void);
void WriteOutVorviewFLTFile(void);
void UnsteadyStabilityAndControlSolve(void);
void Noise(void);
void CalculateAerodynamicCenter(void);
void FiniteDiffTestSolve(void);
void PRINT_STAB_LINE(double F1,
                     double F2,
                     double F3,
                     double F4,
                     double F5,
                     double F6,
                     double F7,
                     double F8);
void WriteOutAdjointStabilityDerivatives(void);
                     
int SearchForIntegerVariable(FILE *File, const char *VariableName, int &Value);
int SearchForCharacterVariable(FILE *File, const char *VariableName, char *Variable);
int SearchForFloatVariable(FILE *File, const char *VariableName, double &Value);
int SearchForFloatVariableList(FILE *File, const char *VariableName, double *Value, int &NumberOfEntries);
int SearchForIntVariableList(FILE *File, const char *VariableName, int *Value, int &NumberOfEntries);

VSP_SOLVER VSP_VLM_;
VSP_SOLVER &VSP_VLM(void) { return VSP_VLM_; };

// The code...

/*##############################################################################
#                                                                              #
#                                 main                                         #
#                                                                              #
##############################################################################*/

int main(int argc, char **argv)
{

    int i;
    double TotalTime;
    
    TotalTime = myclock();
    
    // Grab the file name
    
    FileName = argv[argc-1];
         
    // Load in the case file
    
    LoadCaseFile(1);
        
    // Parse the input

    ParseInput(argc,argv);

    // Loop over input file reads and echo to user what we look for...

    if ( VSPAERO_InputFileFormatOutput_ ) { LoadCaseFile(0); fflush(NULL) ; exit(1) ; };

    // Output a header

    printf("VSPAERO v.%d.%d.%d --- Compiled on: %s at %s PST \n", VER_MAJOR, VER_MINOR, VER_PATCH, __DATE__, __TIME__);
    printf("\n\n\n\n");
    
#ifdef VSPAERO_OPENMP
    printf("Initializing OPENMP for %d threads \n",NumberOfThreads_);
   
    omp_set_num_threads(NumberOfThreads_);
    
    NumberOfThreads_ = omp_get_max_threads();

    printf("NumberOfThreads_: %d \n",NumberOfThreads_);

#ifdef _OPENMP
    printf("_OPENMP Version string: %d\n", _OPENMP);
#endif

#else
    NumberOfThreads_ = 1;
    printf("Single threaded build.\n");
#endif

    // Read in FEM deformation file
    
//broken    if ( LoadFEMDeformation_ ) VSP_VLM().LoadFEMDeformation() = 1;
    
    // Do ground effects analysis
    
    if ( DoGroundEffectsAnalysis_ ) {
       
       VSP_VLM().DoGroundEffectsAnalysis() = 1;
       
       VSP_VLM().VehicleRotationAngleVector(0) = 0.;
       VSP_VLM().VehicleRotationAngleVector(1) = -AoAList_[1];
       VSP_VLM().VehicleRotationAngleVector(2) = 0.;
       
       VSP_VLM().VehicleRotationAxisLocation(0) = Xcg_;
       VSP_VLM().VehicleRotationAxisLocation(1) = Ycg_;
       VSP_VLM().VehicleRotationAxisLocation(2) = Zcg_;  
       
       VSP_VLM().HeightAboveGround() = HeightAboveGround_;
       
       NumberOfAoAs_ = 1;
       
       AoAList_[1] = 0.;
       
       Zcg_ += HeightAboveGround_ - Zcg_;
          
       VSP_VLM().Xcg() = Xcg_;
       VSP_VLM().Ycg() = Ycg_;
       VSP_VLM().Zcg() = Zcg_;       

    }
    
    // Write out 2D FEM file
    
    if ( Write2DFEMFile_ ) VSP_VLM().Write2DFEMFile() = 1;
    
    // Write out Tecplot file
    
    if ( WriteTecplotFile_ ) VSP_VLM().WriteTecplotFile() = 1;
        
    // Save optimization data
    
    if ( OptimizationSolve_ ) VSP_VLM().OptimizationSolve() = 1;
            
    // Solve the adjoint problem
    
    if ( DoAdjointSolve_ ) VSP_VLM().DoAdjointSolve() = 1;
      
    // User choosing specific grid level to set as finest level
    
    if ( SolveOnMGLevel_ > 0 ) VSP_VLM().SolveOnMGLevel(SolveOnMGLevel_);
           
    // Set number of farfield wake nodes

    if ( NumberOfWakeNodes_ > 0 ) VSP_VLM().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);       
               
    // Force farfield distance for wake adaption
    
    if ( SetFarDist_ ) VSP_VLM().SetFarFieldDist(FarDist_);
                    
    // Load in the VSP degenerate geometry file
 
    VSP_VLM().ReadFile(FileName, sizeof(FileName)*sizeof(char));
    
    // Check adjoint component group settings
        
    if ( NumberOfAdjointComponentListSettings_ != 0 ) {
       
       if ( NumberOfAdjointComponentListSettings_ != VSP_VLM().VSPGeom().NumberOfComponentGroups() ) {
       
          printf("Number of adjoint component groups listed in .vspaero file does not match actual number of component groups defined in .groups file! \n");
          fflush(NULL);exit(1);
          
       }
       
       for ( i = 1 ; i <= VSP_VLM().VSPGeom().NumberOfComponentGroups() ; i++ ) {
    
          VSP_VLM().UserAdjointComponentList(i) = UserAdjointComponentList_[i];

       }
       
    }    
    
    // Geometry dump, no solver
    
    if ( DumpGeom_ ) VSP_VLM().DumpGeom() = 1;
    
    // Setup high lift file
    
    if ( SetupHighLiftFile_ ) VSP_VLM().CreateHighLiftFile() = 1;

    // Turn off spanwise loading calculations for Panel solves
    
    if ( NoPanelSpanWiseLoading_ )  VSP_VLM().PanelSpanWiseLoading() = 0;

    // We are doing an interrogation of a previous solution
    
    if ( RestartAndInterrogateSolution_ ) VSP_VLM().DoSolutionInterrogation() = 1;

    // Setup

    VSP_VLM().Setup();

    // Wake options

    if ( ImplicitWake_ > 0 ) VSP_VLM().ImplicitWake() = 1;
    
    if ( ImplicitWakeStartIteration_ > 0 ) VSP_VLM().ImplicitWakeStartIteration() = ImplicitWakeStartIteration_;

    // Inform solver of Control Surface Groups ( this information is used in VSP_SOLVER::WriteCaseHeader(FILE *fid) )

    VSP_VLM().SetControlSurfaceGroup( ControlSurfaceGroup_, NumberOfControlGroups_ );

    // Stability and control run
    
    if ( StabControlRun_ == 1 ) {

       FiniteDifference_StabilityAndControlSolve();
 
    }
    
    // Unsteady stability and control run (pqr)
    
    else if ( StabControlRun_ == 2 ||
              StabControlRun_ == 3 ||
              StabControlRun_ == 4 ) {
     
       UnsteadyStabilityAndControlSolve();
       
    }
    
    else if ( StabControlRun_ == 5 ) {
    
       CalculateAerodynamicCenter();
    
    }

    // Create noise files
    
    else if ( CreateNoiseFiles_ == 1 ) {
       
     // borked  Noise();
       
    }
    
    else if ( DoFiniteDiffTest ) {
       
       FiniteDiffTestSolve();
       
    }
    
    // Just a normal solve

    else {

       Solve();
       
       if ( DoAdjointSolve_ ) WriteOutAdjointStabilityDerivatives();

    }
    
    TotalTime = myclock() - TotalTime;
    
    printf("Total setup and solve time: %f seconds \n",TotalTime);

}

/*##############################################################################
#                                                                              #
#                              UsageHelp                                       #
#                                                                              #
##############################################################################*/

void PrintUsageHelp()
{
       printf("VSPAERO v.%d.%d.%d --- Compiled on: %s at %s PST \n", VER_MAJOR, VER_MINOR, VER_PATCH, __DATE__, __TIME__);
       printf("\n\n\n\n");

       printf("Usage: vspaero [options] <FileName>\n");
       printf("\n\n");
       printf("Options: \n");                  
       printf(" -omp <N>                           Use 'N' processes.\n");
       printf(" -stab                              Calculate stability derivatives.\n");
       printf("\n");                                                   
       printf(" -pstab                             Calculate unsteady roll  rate stability derivative analysis.\n");
       printf(" -qstab                             Calculate unsteady pitch rate stability derivative analysis.\n");
       printf(" -rstab                             Calculate unsteady yaw   rate stability derivative analysis.\n");
       printf("\n");                                                   
       printf(" -fs <M> END <A> END <B> END        Set/Override freestream Mach, Alpha, and Beta. note: M, A, and B are space delimited lists.\n");
       printf(" -save                              Save restart file.\n");
       printf(" -restart                           Restart analysis.\n");
       printf(" -geom                              Process and write geometry without solving.\n");
       printf(" -fem                               Load in FEM deformation file.\n");
       printf(" -write2dfem                        Write out 2D FEM load file.\n");
       printf(" -groundheight <H>                  Do ground effects analysis with cg set to <H> height above the ground. \n");
       printf(" -rotor <RPM>                       Do a rotor analysis, with specified rotor RPM. \n");
       printf(" -unsteady                          Run an unsteady analysis, assumes .groups file is setup! \n");
       printf("\n");                                                   
       printf("\n");                                                   
       printf("\n");                                                   
       printf(" -interrogate                       Reload an existing solution, and interrogate the data using survey points list. \n");
       printf(" -interrogate -unsteady             Reload an existing unsteady solution, and interrogate the data using survey points list. \n");
       printf("\n");                                                   
       printf("\n");
       printf("\n");
}

/*##############################################################################
#                                                                              #
#                              ParseInput                                      #
#                                                                              #
##############################################################################*/

void ParseInput(int argc, char *argv[])
{

    int i, iter;

    // Parse the input

    if ( argc < 2 ) {

       PrintUsageHelp();
 
       exit(1);

    }
    
    //NumberOfMachs_     = 0;
    //NumberOfAoAs_      = 0;
    //NumberOfBetas_     = 0;
    //NumberOfReCrefs_   = 0;    

    i = 1;

    while ( i <= argc - 2 ) {

       // Start up the graphics viewer after were all done

       if ( strcmp(argv[i],"-wtf") == 0 ) {

         printf("wtf back at you!  \n");   

       }

       else if ( strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0 ) {
        
          PrintUsageHelp();
 
          exit(1);
          
       }
       
       else if ( strcmp(argv[i],"-omp") == 0 ) {
        
          NumberOfThreads_ = atoi(argv[++i]);
          
       }

       else if ( strcmp(argv[i],"-stab") == 0 ) {
        
          StabControlRun_ = 1;
          
       }
       
       else if ( strcmp(argv[i],"-pstab") == 0 ) {
        
          StabControlRun_ = 2;
          
          VSP_VLM().TimeAccurate() = DoUnsteadyAnalysis_ = 1;
          VSP_VLM().TimeAnalysisType() = P_ANALYSIS;
          
          VSP_VLM().NumberOfTimeSteps()             = 128;
          VSP_VLM().Unsteady_AngleMax()             = 5.;
          VSP_VLM().Unsteady_HMax()                 = 0.;    
          VSP_VLM().CoreSizeFactor()                = 1.;    
          VSP_VLM().ForwardGMRESConvergenceFactor() = 1.;   
          VSP_VLM().WakeIterations()                = 25;
          
          NumberOfWakeNodes_ = 32;
          
          VSP_VLM().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);

       }      
       
       else if ( strcmp(argv[i],"-qstab") == 0 ) {
        
          StabControlRun_ = 3;
          
          VSP_VLM().TimeAccurate() = DoUnsteadyAnalysis_ = 1;
          VSP_VLM().TimeAnalysisType() = Q_ANALYSIS;
          
          VSP_VLM().NumberOfTimeSteps()             = 128;
          VSP_VLM().Unsteady_AngleMax()             = 5.;
          VSP_VLM().Unsteady_HMax()                 = 0.;    
          VSP_VLM().CoreSizeFactor()                = 1.;    
          VSP_VLM().ForwardGMRESConvergenceFactor() = 1.;
          VSP_VLM().WakeIterations()                = 25;
          
          NumberOfWakeNodes_ = 32;
          
          VSP_VLM().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);
                    
       }      
                     
       else if ( strcmp(argv[i],"-rstab") == 0 ) {
        
          StabControlRun_ = 4;
          
          VSP_VLM().TimeAccurate() = DoUnsteadyAnalysis_ = 1;
          VSP_VLM().TimeAnalysisType() = R_ANALYSIS;
          
          VSP_VLM().NumberOfTimeSteps()             = 128;
          VSP_VLM().Unsteady_AngleMax()             = 5.;
          VSP_VLM().Unsteady_HMax()                 = 0.;    
          VSP_VLM().CoreSizeFactor()                = 1.;    
          VSP_VLM().ForwardGMRESConvergenceFactor() = 1.;   
          VSP_VLM().WakeIterations()                = 25;
          
          NumberOfWakeNodes_ = 32;
          
          VSP_VLM().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);
                    
       }              

       else if ( strcmp(argv[i],"-acstab") == 0 ) {
        
          StabControlRun_ = 5;
          
       }
              
       else if ( strcmp(argv[i],"-unsteady") == 0 ) {

          VSP_VLM().TimeAccurate() = DoUnsteadyAnalysis_ = 1;

          VSP_VLM().TimeAnalysisType() = 0;

       }
       
       else if ( strcmp(argv[i],"-fs") == 0 ) {
        
          SetFreeStream_ = 1;
          
          // Read in Mach number list
          
          while ( strcmp(argv[i+1],"END") != 0 ) {
          
             MachList_[++NumberOfMachs_] = atof(argv[++i]);
             
             printf("NumberOfMachs_: %d ---> %s \n",NumberOfMachs_,argv[i]);
             
          }
          
          i++;
          
          // Read in AoA list
          
          while ( strcmp(argv[i+1],"END") != 0 ) {
          
             AoAList_[++NumberOfAoAs_] = atof(argv[++i]);
             
             printf("NumberOfAoAs_: %d ---> %s \n",NumberOfAoAs_,argv[i]);

          }
          
          i++;
          
          // Read in the Beta list                    
          
          while ( strcmp(argv[i+1],"END") != 0 ) {
          
             BetaList_[++NumberOfBetas_] = atof(argv[++i]);
             
             printf("NumberOfBetas_: %d ---> %s \n",NumberOfBetas_,argv[i]);

          }    
          
          i++;

       }

       else if ( strcmp(argv[i],"-restart") == 0 ) {
        
          DoRestartRun_ = 1;
          
       }    
       
       else if ( strcmp(argv[i],"-geom") == 0 ) {
        
          DumpGeom_ = 1;
          
       }           

       else if ( strcmp(argv[i],"-implicit") == 0 ) {

          ImplicitWake_ = 1;
          
       }         
       
       else if ( strcmp(argv[i],"-fem") == 0 ) {

          LoadFEMDeformation_ = 1;
          
       }
       
       else if ( strcmp(argv[i],"-groundheight") == 0 ) {
       
          DoGroundEffectsAnalysis_ = 1;
          
          HeightAboveGround_ = atof(argv[++i]);
          
       }   
       
       else if ( strcmp(argv[i],"-write2dfem") == 0 ) {
          
          Write2DFEMFile_ = 1;
          
       }

       else if ( strcmp(argv[i],"-writetecplot") == 0 ) {
          
          WriteTecplotFile_ = 1;
          
       }

       else if ( strcmp(argv[i],"-adjoint") == 0 ) {
          
          DoAdjointSolve_ = 1;
 
       }
    
       else if ( strcmp(argv[i],"-interrogate") == 0 ) {
                    
          RestartAndInterrogateSolution_ = 1;

       } 

       else if ( strcmp(argv[i],"-fdtest") == 0 ) {
                    
          DoFiniteDiffTest = 1;

       }          
       
       else if ( strcmp(argv[i],"-fileinputs") == 0 ) {
          
          VSPAERO_InputFileFormatOutput_ = 1;
    
       }

       else if ( strcmp(argv[i],"END") == 0 ) {

          // Do nothing... we assume this was the marker to the end of a list
          
       }                   
       
       // Unreconizable option

       else {

          printf("Unknown option argv[%d]: %s \n",i,argv[i]);
          
          PrintUsageHelp();

          exit(1);

       }

       i++;

    }

}

/*##############################################################################
#                                                                              #
#                                      LoadCaseFile                            #
#                                                                              #
##############################################################################*/

void LoadCaseFile(int ReadFlag)
{

    int i, j, k, NumberOfControlSurfaces, Done, Dir, Surface;
    double x,y,z, DumDouble, HingeVec[3], RotAngle, DeltaHeight;
    double Value, MassFlow, Velocity, DeltaCp;
    FILE *case_file;
    char file_name_w_ext[2000], DumChar[2000], DumChar2[2000], Comma[2000], *Next;
    QUAT Quat, InvQuat, Vec;

    // Delimiters
    
    snprintf(Comma,sizeof(Comma)*sizeof(char),",");


    // Open the case file

    if ( ReadFlag ) {
       
       snprintf(file_name_w_ext,sizeof(file_name_w_ext)*sizeof(char),"%s.vspaero",FileName);
   
       if ( (case_file = fopen(file_name_w_ext,"r")) == NULL ) {
   
          printf("Could not open the file: %s for input! \n",file_name_w_ext);
   
          exit(1);
   
       }
       
    }
    
    else {
              
       case_file = NULL;

    }
    
    Sref_                          = 1.0;
    Cref_                          = 1.0;  
    Bref_                          = 1.0; 
    Xcg_                           = 0.0;
    Ycg_                           = 0.0;
    Zcg_                           = 0.0;
    Mach_                          = 0.3;
    AoA_                           = 5.0;
    Beta_                          = 0.0;
    Vinf_                          = 100.;
    Vref_                          = 100.;
    Machref_                       = 0.3;
    Rho_                           = 0.002377;
    ReCref_                        = 10000000.;
    Clo2D_                         =  0.;    
    MaxTurningAngle_               = -1.;
    FarDist_                       = -1.;
    NumberOfWakeNodes_             = -1;
    WakeIterations_                = 3;
    ForwardGMRESConvergenceFactor_ = 1.;
    NonLinearConvergenceFactor_    = 1.;

    if ( SearchForFloatVariable(case_file, "Sref", Sref_) ) printf("Setting Sref to: %f \n",Sref_);
    if ( SearchForFloatVariable(case_file, "Cref", Cref_) ) printf("Setting Cref to: %f \n",Cref_);
    if ( SearchForFloatVariable(case_file, "Bref", Bref_) ) printf("Setting Bref to: %f \n",Bref_);
    if ( SearchForFloatVariable(case_file, "X_cg", Xcg_ ) ) printf("Setting X_cg to: %f \n",Xcg_);
    if ( SearchForFloatVariable(case_file, "Y_cg", Ycg_ ) ) printf("Setting Y_cg to: %f \n",Ycg_);
    if ( SearchForFloatVariable(case_file, "Z_cg", Zcg_ ) ) printf("Setting Z_cg to: %f \n",Zcg_);

    // Load in Mach list

    if ( !SearchForFloatVariableList(case_file, "Mach", MachList_, NumberOfMachs_) ) { if ( case_file != NULL ) { printf("Did not find an Mach list! \n");fflush(NULL); }; };
       
    if ( case_file != NULL ) { printf("Number of Machs: %d \n",NumberOfMachs_); };
    
    for ( i = 1 ; i <= NumberOfMachs_ ; i++ ) {
       
       if ( case_file != NULL ) { printf("Mach: %lf \n",MachList_[i]); };
       
    }
    
    Mach_ = MachList_[1];

    // Load in AoA list

    if ( !SearchForFloatVariableList(case_file, "AoA", AoAList_, NumberOfAoAs_) ) { if ( case_file != NULL ) printf("Did not find an AoA list! \n");fflush(NULL); };
       
    if ( case_file != NULL ) { printf("Number of AoAs: %d \n",NumberOfAoAs_); }
    
    for ( i = 1 ; i <= NumberOfAoAs_ ; i++ ) {
       
       if ( case_file != NULL ) printf("AoA: %lf \n",AoAList_[i]);
       
    }
    
    AoA_ = AoAList_[1];
 
    // Load in Beta list

    if ( !SearchForFloatVariableList(case_file, "Beta", BetaList_, NumberOfBetas_) ) { if ( case_file != NULL ) printf("Did not find an Beta list! \n");fflush(NULL); };
       
    if ( case_file != NULL ) { printf("Number of Betas: %d \n",NumberOfBetas_); };
    
    for ( i = 1 ; i <= NumberOfBetas_ ; i++ ) {
       
       if ( case_file != NULL ) printf("Beta: %lf \n",BetaList_[i]);
       
    }
    
    Beta_ = BetaList_[1];

    // Load in ReCref list

    if ( !SearchForFloatVariableList(case_file, "ReCref", ReCrefList_, NumberOfReCrefs_) ) { if ( case_file != NULL ) printf("Did not find an ReCref list! \n");fflush(NULL); };
       
    if ( case_file != NULL ) { printf("Number of ReCs: %d \n",NumberOfReCrefs_); };
    
    for ( i = 1 ; i <= NumberOfReCrefs_ ; i++ ) {
       
       if ( case_file != NULL )  printf("ReC: %lf \n",ReCrefList_[i]); 
       
    }
    
    ReCref_ = ReCrefList_[1];
                 
    // Search for Vinf
    
    if ( SearchForFloatVariable(case_file, "Vinf", Vinf_) ) { if ( case_file != NULL )  printf("Setting Vinf to: %f \n",Vinf_); };
    
    Vref_ = Vinf_;
 
    // Search for Vref and Machref
    
    if ( SearchForFloatVariable(case_file, "Vref", Vref_) ) {
       
       if ( case_file != NULL ) printf("Setting Vref to: %f \n",Vref_);
       
       if ( SearchForFloatVariable(case_file, "Mref", Machref_) ) {
          
          if ( case_file != NULL ) printf("Setting Mref to: %f \n",Machref_);
          
       }
       
       else if ( case_file != NULL ) {
 
          printf("Mref must be defined if Vref is defined \n");
          fflush(NULL);
          exit(1);
          
       }
       
    }
    
    // Search for Rho
    
    if ( SearchForFloatVariable(case_file, "Rho", Rho_) ) if ( case_file != NULL ) { printf("Setting Rho to: %f \n",Rho_); };

    // Search for stall model setting
    
    if ( SearchForIntegerVariable(case_file, "StallModel", StallModelIsOn) ) if ( case_file != NULL ) { printf("Setting StallModel to: %d \n",StallModelIsOn); };
    
    // Search for Clo2D
    
    if ( SearchForFloatVariable(case_file, "Clo2D", Clo2D_) ) if ( case_file != NULL ) {  printf("Setting Clo2D to: %f \n",Clo2D_); };
    
    // Search for Symmetry setting
    
    if ( SearchForIntegerVariable(case_file, "Symmetry", SymmetryFlag_) ) if ( case_file != NULL ) { printf("Setting Symmetry to: %d \n",SymmetryFlag_); };

    // Search for freeze multipole setting
    
    if ( SearchForIntegerVariable(case_file, "FreezeMultiPoleAtIteration", FreezeMultiPoleAtIteration_) ) if ( case_file != NULL ) { printf("Setting FreezeMultiPoleAtIteration to: %d \n",FreezeMultiPoleAtIteration_); };
         
    if ( FreezeMultiPoleAtIteration_ >= 0 ) VSP_VLM().FreezeMultiPoleAtIteration() = FreezeMultiPoleAtIteration_;

    // Search for freeze wake setting
    
    if ( SearchForIntegerVariable(case_file, "FreezeWakeAtIteration", FreezeWakeAtIteration_) ) if ( case_file != NULL ) { printf("Setting FreezeWakeAtIteration to: %d \n",FreezeWakeAtIteration_); };
         
    if ( FreezeWakeAtIteration_ >= 0 ) VSP_VLM().FreezeWakeAtIteration() = FreezeWakeAtIteration_;
    
    // Search for freeze wake root vortices setting
    
    if ( SearchForIntegerVariable(case_file, "FreezeWakeRootVortices", FreezeWakeRootVortices_) ) if ( case_file != NULL ) { printf("Setting FreezeWakeRootVortices to: %d \n",FreezeWakeRootVortices_); };
    
    if ( FreezeWakeRootVortices_ >= 0 ) VSP_VLM().FreezeWakeRootVortices() = FreezeWakeRootVortices_;

    // Search for implicit solve
    
    if ( SearchForIntegerVariable(case_file, "ImplicitWake", ImplicitWake_) ) if ( case_file != NULL ) { printf("Setting ImplicitWake to: %d \n",ImplicitWake_); }

    // Search for implicit solve
    
    if ( SearchForIntegerVariable(case_file, "ImplicitWakeStartIteration", ImplicitWakeStartIteration_) ) if ( case_file != NULL ) { printf("Setting ImplicitWakeStartIteration_ to: %d \n",ImplicitWake_); };


    // Search for FarDist
    
    FarDist_ = 0.;
    
    if ( SearchForFloatVariable(case_file, "FarDist", FarDist_) ) printf("Setting FarDist to: %f \n",FarDist_);

    if ( FarDist_ > 0. ) SetFarDist_ = 1;
    
    // Search for NumWakeNodes
    
    if ( SearchForIntegerVariable(case_file, "NumWakeNodes", NumberOfWakeNodes_) ) if ( case_file != NULL ) { printf("Setting NumWakeNodes to: %d \n",NumberOfWakeNodes_); };
    
    // Search for WakeIters
    
    if ( SearchForIntegerVariable(case_file, "WakeIters", WakeIterations_) ) if ( case_file != NULL ) { printf("Setting WakeIters to: %d \n",WakeIterations_); };
            
    // Search for wake relaxation factor
    
    WakeRelax_ = -1.;
    
    if ( SearchForFloatVariable(case_file, "WakeRelax", WakeRelax_) ) if ( case_file != NULL ) { printf("Setting WakeRelax to: %f \n",WakeRelax_); };
  
    if ( WakeRelax_ >= 0. ) VSP_VLM().WakeRelax() = WakeRelax_;
   
    // Search for Forward GMRES residual scale factor
    
    ForwardGMRESConvergenceFactor_ = 1.;
    
    if ( SearchForFloatVariable(case_file, "ForwardGMRESConvergenceFactor", ForwardGMRESConvergenceFactor_) ) if ( case_file != NULL ) { printf("Setting ForwardGMRESConvergenceFactor to: %e \n",ForwardGMRESConvergenceFactor_); };
              
    VSP_VLM().ForwardGMRESConvergenceFactor() = ForwardGMRESConvergenceFactor_;

    // Search for Adjoint GMRES residual scale factor
    
    AdjointGMRESConvergenceFactor_ = 1.;
    
    if ( SearchForFloatVariable(case_file, "AdjointGMRESConvergenceFactor", AdjointGMRESConvergenceFactor_) ) if ( case_file != NULL ) { printf("Setting AdjointGMRESConvergenceFactor_ to: %e \n",AdjointGMRESConvergenceFactor_); };
              
    VSP_VLM().AdjointGMRESConvergenceFactor() = AdjointGMRESConvergenceFactor_;
    
    // Search for Solver residual scale factor
    
    NonLinearConvergenceFactor_ = 1.;
    
    if ( SearchForFloatVariable(case_file, "NonLinearConvergenceFactor", NonLinearConvergenceFactor_) ) if ( case_file != NULL ) { printf("Setting NonLinearConvergenceFactor to: %e \n",NonLinearConvergenceFactor_); };
              
    VSP_VLM().NonLinearConvergenceFactor() = NonLinearConvergenceFactor_;
         
    // Search for coresize factor
    
    CoreSizeFactor_ = -1.;
    
    if ( SearchForFloatVariable(case_file, "CoreSizeFactor", CoreSizeFactor_) ) if ( case_file != NULL ) { printf("Setting CoreSizeFactor to: %f \n",CoreSizeFactor_); };
    
    if ( CoreSizeFactor_ > 0. ) VSP_VLM().CoreSizeFactor() = CoreSizeFactor_;

    // Search for Faraway factor
    
    FarAway_ = -1.;
    
    if ( SearchForFloatVariable(case_file, "FarAway", FarAway_) ) if ( case_file != NULL ) { printf("Setting multipole FarAway factor to: %f \n",FarAway_); };
    
    if ( FarAway_ > 0. ) VSP_VLM().FarAway() = FarAway_;
    
    if ( WakeIterations_ == 0 && DoUnsteadyAnalysis_ == 0 )VSP_VLM().GMRESTightConvergence() = 1;

    // Search for preconditioner update flag
    
    UpdateMatrixPreconditioner_ = 0;
    
    if ( SearchForIntegerVariable(case_file, "UpdateMatrixPreconditioner", UpdateMatrixPreconditioner_) ) if ( case_file != NULL ) { printf("Setting UpdateMatrixPreconditioner flag to: %d \n",UpdateMatrixPreconditioner_); };

    VSP_VLM().UpdateMatrixPreconditioner() = UpdateMatrixPreconditioner_;
        
    // Search for wake node preconditioner flag
    
    UseWakeNodeMatrixPreconditioner_ = 0;
    
    if ( SearchForIntegerVariable(case_file, "UseWakeNodeMatrixPreconditioner", UseWakeNodeMatrixPreconditioner_) ) if ( case_file != NULL ) { printf("Setting UseWakeNodeMatrixPreconditioner flag to: %d \n",UseWakeNodeMatrixPreconditioner_); };
    
    VSP_VLM().UseWakeNodeMatrixPreconditioner() = UseWakeNodeMatrixPreconditioner_;

    if ( SearchForFloatVariable(case_file, "MaxTurningAngle", MaxTurningAngle_) ) if ( case_file != NULL ) { printf("Setting MaxTurningAngle_ to: %d \n",MaxTurningAngle_); };
     
    if ( MaxTurningAngle_ <= 0. ) MaxTurningAngle_ = -1.;
    
    // Search for quad tree buffer levels
        
    if ( SearchForIntegerVariable(case_file, "QuadTreeBufferLevels", QuadTreeBufferLevels_) ) if ( case_file != NULL ) { printf("Setting QuadTreeBufferLevels_ flag to: %d \n",QuadTreeBufferLevels_); };
    
    VSP_VLM().QuadTreeBufferLevels() = QuadTreeBufferLevels_;
         
    // Search for adjoint force type
    
    AdjointSolutionForceType_ = 0;
    
    if ( SearchForIntegerVariable(case_file, "AdjointSolutionForceType", AdjointSolutionForceType_) ) if ( case_file != NULL ) { printf("Setting AdjointSolutionForceType to: %d \n",AdjointSolutionForceType_); };
    
    VSP_VLM().AdjointSolutionForceType() = AdjointSolutionForceType_;
         
    // Search fir adjoint force and moment case list

    NumberOfAdjointForceAndMomentCases_ = 0;
    
    if ( SearchForIntVariableList(case_file, "AdjointSolveForThisForceMomentCase", AdjointSolveForThisForceMomentCase_, NumberOfAdjointForceAndMomentCases_) ) if ( case_file != NULL ) { printf("Found AdjointSolveForThisForceMomentCase list... \n");fflush(NULL); };
       
    if ( case_file != NULL ) printf("Number of NumberOfAdjointForceAndMomentCases_: %d \n",NumberOfAdjointForceAndMomentCases_);
    
    if ( NumberOfAdjointForceAndMomentCases_ != 0 && NumberOfAdjointForceAndMomentCases_ != 6 && case_file != NULL ) {
       
       printf("Error! User must supply full list of adjoint cases to perform! \n");
       fflush(NULL);exit(1);
       
    }
    
    for ( i = 1 ; i <= NumberOfAdjointForceAndMomentCases_ ; i++ ) {
       
       if ( case_file != NULL ) { printf("AdjointSolveForThisForceMomentCase_[%d]: %d \n",i,AdjointSolveForThisForceMomentCase_[i]); };
       
    }
    
    if ( NumberOfAdjointForceAndMomentCases_ == 6 ) {
       
       for ( i = 1 ; i <= NumberOfAdjointForceAndMomentCases_ ; i++ ) {
          
          VSP_VLM().DoAdjointSolveForThisForceMomentCase(i) = AdjointSolveForThisForceMomentCase_[i];
          
       }
       
    }
    
    // Search for adjoint component settings
    
    NumberOfAdjointComponentListSettings_ = 0;
    
    if ( SearchForIntVariableList(case_file, "AdjointComponentList", UserAdjointComponentList_, NumberOfAdjointComponentListSettings_) ) if ( case_file != NULL ) { printf("Found UserAdjointComponentList list... \n");fflush(NULL); };
       
    if ( case_file != NULL ) { printf("Number of NumberOfAdjointComponentListSettings_: %d \n",NumberOfAdjointComponentListSettings_); };
           
    if ( NumberOfAdjointComponentListSettings_ != 0 ) {

       for ( i = 1 ; i <= NumberOfAdjointComponentListSettings_ ; i++ ) {
          
          if ( case_file != NULL ) printf("UserAdjointComponentList(%d): %d \n",i,UserAdjointComponentList_[i]);
          
       }
       
    }   
     
    // Search for FEM 2D file output flag
    
    Write2DFEMFile_ = 0;
    
    if ( SearchForIntegerVariable(case_file, "Write2DFEMFile", Write2DFEMFile_) ) if ( case_file != NULL ) { printf("Setting Write2DFEMFile to: %d \n",Write2DFEMFile_); };
    
    VSP_VLM().Write2DFEMFile() = Write2DFEMFile_;
    
    
    // Search for Tecplot file output flag
    
    WriteTecplotFile_ = 0;
    
    if ( SearchForIntegerVariable(case_file, "WriteTecplotFile", WriteTecplotFile_) ) if ( case_file != NULL ) {  printf("Setting WriteTecplotFile to: %d \n",WriteTecplotFile_); };
    
    VSP_VLM().WriteTecplotFile() = WriteTecplotFile_;    
    
    // We were just creating an input list of variables ... so quit
        
    if ( case_file == NULL ) { fflush(NULL); exit(1); };
     
    printf("Sref            = %lf \n",Sref_);
    printf("Cref            = %lf \n",Cref_);
    printf("Bref            = %lf \n",Bref_);
    printf("X_cg            = %lf \n",Xcg_);
    printf("Y_cg            = %lf \n",Ycg_);
    printf("Z_cg            = %lf \n",Zcg_);
    printf("Mach            = "); { for ( i = 1 ; i < NumberOfMachs_ ; i++ ) { printf("%f, ",MachList_[i]); }; printf("%f \n",MachList_[NumberOfMachs_]); };
    printf("AoA             = "); { for ( i = 1 ; i < NumberOfAoAs_  ; i++ ) { printf("%f, ", AoAList_[i]); }; printf("%f \n", AoAList_[NumberOfAoAs_ ]); };
    printf("Beta            = "); { for ( i = 1 ; i < NumberOfBetas_ ; i++ ) { printf("%f, ",BetaList_[i]); }; printf("%f \n",BetaList_[NumberOfBetas_]); };
    printf("Vinf            = %lf \n",Vinf_);
    printf("Vref            = %lf \n",Vref_);
    printf("Rho             = %lf \n",Rho_);
    printf("ReCref          = "); { for ( i = 1 ; i < NumberOfReCrefs_ ; i++ ) { printf("%f, ",ReCrefList_[i]); }; printf("%f \n",ReCrefList_[NumberOfReCrefs_]); };
    
    printf("StallModel      = %d  \n",StallModelIsOn);
    printf("Clo2D           = %lf \n",Clo2D_);    
    printf("MaxTurningAngle = %lf \n",MaxTurningAngle_);
    printf("Symmetry        = %d  \n",SymmetryFlag_);
    printf("FarDist         = %lf \n",FarDist_);
    printf("NumWakeNodes    = %d  \n",NumberOfWakeNodes_);
    printf("WakeIters       = %d  \n",WakeIterations_);

    VSP_VLM().Sref() = Sref_;
    VSP_VLM().Cref() = Cref_;
    VSP_VLM().Bref() = Bref_;

    VSP_VLM().Xcg() = Xcg_;
    VSP_VLM().Ycg() = Ycg_;
    VSP_VLM().Zcg() = Zcg_;
    
    VSP_VLM().Mach() = Mach_;
    VSP_VLM().AngleOfAttack() = AoA_ * TORAD;
    VSP_VLM().AngleOfBeta() = Beta_ * TORAD;
    
    VSP_VLM().Vinf() = Vinf_;
    
    VSP_VLM().Vref() = Vref_;
    
    VSP_VLM().Machref() = Machref_;
    
    VSP_VLM().Density() = Rho_;
    
    VSP_VLM().ReCref() = ReCref_;
    
    VSP_VLM().Clo2D() = Clo2D_;
    
    VSP_VLM().StallModelIsOn() = StallModelIsOn;
    
    VSP_VLM().MaxTurningAngle() = MaxTurningAngle_;
    
    VSP_VLM().WakeIterations() = WakeIterations_;
    
    VSP_VLM().RotationalRate_p() = 0.0;
    VSP_VLM().RotationalRate_q() = 0.0;
    VSP_VLM().RotationalRate_r() = 0.0;    

    VSP_VLM().DoSymmetryPlaneSolve() = SymmetryFlag_;
        
    // Load in the control surface data
    
    rewind(case_file);
    
    NumberOfControlGroups_ = 0;

    while ( fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfControlGroups") != NULL ) {
          
          sscanf(DumChar,"NumberOfControlGroups = %d \n",&NumberOfControlGroups_);

          printf( "NumberOfControlGroups = %d \n", NumberOfControlGroups_ );

          if ( NumberOfControlGroups_ < 0 ) {

              printf( "INVALID NumberOfControlGroups: %d\n", NumberOfControlGroups_ );

              exit( 1 );

          }

          ControlSurfaceGroup_ = new CONTROL_SURFACE_GROUP[NumberOfControlGroups_ + 1];
          
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) {
             
             // Control groupname
             
             fgets( ControlSurfaceGroup_[i].Name(), 200, case_file );
             
             ControlSurfaceGroup_[i].Name()[strcspn(ControlSurfaceGroup_[i].Name(), "\n")] = 0;
             
             printf("ControlSurfaceGroup_[%d].Name(): %s \n",i,ControlSurfaceGroup_[i].Name());
             
             // List of control surfaces in this group
             
             fgets(DumChar,300,case_file);
        
             // Determine the number of control surfaces in the group
         
             if ( strstr(DumChar,Comma) == NULL ) {
         
                NumberOfControlSurfaces = 1;
                
                ControlSurfaceGroup_[i].SizeList(NumberOfControlSurfaces);

                printf( "There is %d control surface in group: %d \n", NumberOfControlSurfaces, i );
                
                DumChar[strcspn(DumChar, "\n")] = 0;
                
                snprintf(ControlSurfaceGroup_[i].ControlSurface_Name(1),sizeof(ControlSurfaceGroup_[i].ControlSurface_Name(1))*sizeof(char),"%s",DumChar);
                
                printf("Control Surface(1): %s___ \n",ControlSurfaceGroup_[i].ControlSurface_Name(1));
                
                // Read in the control surface direction
                
                fgets(DumChar,2000,case_file);
                
                ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(1) = atof(DumChar);
                                                
                printf("Control surface(%d) direction: %f \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(1));
   
                //// Read in the control surface deflection
                //
                //fgets(DumChar,2000,case_file);
                //
                //ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle() = atof(DumChar);
                //
                //printf("Control surface(%d) deflection %f \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle());
                // 
             }
             
             else {
                
                // Save a copy of Dumchar
                
                snprintf(DumChar2,sizeof(DumChar2)*sizeof(char),"%s",DumChar);
            
                // Figure out how many control surfaces are in the list
                
                Next = strtok(DumChar,Comma);
         
                NumberOfControlSurfaces = 1;
   
                while ( Next != NULL ) {
                   
                    Next = strtok(NULL,Comma);
                    
                    if ( Next != NULL ) {
                       
                       NumberOfControlSurfaces++;
                       
                    }
                    
                }
                
                // Size the control surface list
                
                ControlSurfaceGroup_[i].SizeList(NumberOfControlSurfaces);
                
                printf("There are %d control surfaces in group: %d \n",NumberOfControlSurfaces,i);
         
                // Reparse the list to get the actual control surface names
                
                snprintf(DumChar,sizeof(DumChar)*sizeof(char),"%s",DumChar2);
                
                Next = strtok(DumChar,Comma);
  
                NumberOfControlSurfaces = 1;
                
                snprintf(ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces),sizeof(ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces))*sizeof(char),"%s",Next);
                
                ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(NumberOfControlSurfaces) = 1;
                
                printf("Control surface(%d): %s \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces));
   
                while ( Next != NULL ) {
                   
                    Next = strtok(NULL,",\n");
         
                    if ( Next != NULL ) {
                       
                       NumberOfControlSurfaces++;

                       snprintf(ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces),sizeof(ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces))*sizeof(char),"%s", Next );
                
                       printf("Control surface(%d): %s \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces));
   
                    }
                    
                }
                
                // Now read in the control surface directions
                
                fgets(DumChar,2000,case_file);
                
                Next = strtok(DumChar,Comma);
  
                NumberOfControlSurfaces = 1;
        
                ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(NumberOfControlSurfaces) = atof(Next);
                
                printf("Control surface(%d) direction: %f \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(NumberOfControlSurfaces));
   
                while ( Next != NULL ) {
                   
                    Next = strtok(NULL,Comma);
         
                    if ( Next != NULL ) {
                       
                       NumberOfControlSurfaces++;
                       
                       ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(NumberOfControlSurfaces) = atof(Next);
                
                       printf("Control surface(%d) direction %f \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(NumberOfControlSurfaces));
   
                    }
                    
                } 

             }

             // Finally, read in the control group deflection

             fscanf( case_file, "%lf\n", &(ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle()) );

             printf( "Control group deflection angle: %f \n", ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle() );

          }
          
       }
    
    }
          
    // Load in the rotor data
    
    rewind(case_file);
    
    NumberOfRotors_ = 0;
    
    Done = 0;
    
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfRotors") != NULL ) {
          
          sscanf(DumChar,"NumberOfRotors = %d \n",&NumberOfRotors_);
          
          printf("NumberOfRotors: %d \n",NumberOfRotors_);
          
          VSP_VLM().SetNumberOfRotors(NumberOfRotors_);
          
          for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
           
             fgets(DumChar,2000,case_file);
             fgets(DumChar,2000,case_file);
             
             printf("\nLoading data for rotor: %5d \n",i);
             
             VSP_VLM().RotorDisk(i).Load_STP_Data(case_file);
             
          }
          
          Done = 1;
          
       }
       
    }
    
    // Adjust rotors for ground effects calculation

    if ( DoGroundEffectsAnalysis_ ) {
           
       RotAngle = AoAList_[1] * TORAD;
       
       DeltaHeight = HeightAboveGround_ - Zcg_;
       
       HingeVec[0] = 0.;
       HingeVec[1] = 1.;
       HingeVec[2] = 0.;
           
       Quat.FormRotationQuat(HingeVec,RotAngle);
   
       InvQuat = Quat;
      
       InvQuat.FormInverse();   

       for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
          
          // Rotate rotors and shift in z
          
          x = VSP_VLM().RotorDisk(i).XYZ(0);
          y = VSP_VLM().RotorDisk(i).XYZ(1);
          z = VSP_VLM().RotorDisk(i).XYZ(2);

          VSP_VLM().RotorDisk(i).XYZ(0) = (x - Xcg_)*cos(RotAngle) - (z - Zcg_)*sin(RotAngle) + Xcg_;
          VSP_VLM().RotorDisk(i).XYZ(1) = y;
          VSP_VLM().RotorDisk(i).XYZ(2) = (x - Xcg_)*sin(RotAngle) + (z - Zcg_)*cos(RotAngle) + Zcg_ + DeltaHeight;    
     
          // Rotate rotors to align with free stream
          
          Vec(0) = VSP_VLM().RotorDisk(i).Normal(0);
          Vec(1) = VSP_VLM().RotorDisk(i).Normal(1);
          Vec(2) = VSP_VLM().RotorDisk(i).Normal(2);
       
          Vec = Quat * Vec * InvQuat;
       
          VSP_VLM().RotorDisk(i).Normal(0) = Vec(0);
          VSP_VLM().RotorDisk(i).Normal(1) = Vec(1); 
          VSP_VLM().RotorDisk(i).Normal(2) = Vec(2); 
          
       }
                    
    }
        
    // Load in the velocity survey data
    
    rewind(case_file);
    
    NumberofSurveyPoints_ = 0;
    
    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberofSurveyPoints") != NULL ) {

          sscanf(DumChar,"NumberofSurveyPoints = %d \n",&NumberofSurveyPoints_);
          
          printf("NumberofSurveyPoints: %d \n",NumberofSurveyPoints_);
          
          VSP_VLM().SetNumberOfSurveyPoints(NumberofSurveyPoints_);
          
          for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {
             
             fscanf(case_file,"%d %lf %lf %lf \n",&j,&x,&y,&z);
             
             printf("Survey Point: %10d: %10.5f %10.5f %10.5f \n",i,x,y,z);
             
             VSP_VLM().SurveyPointList(i).x() = x;
             VSP_VLM().SurveyPointList(i).y() = y;
             VSP_VLM().SurveyPointList(i).z() = z;
             
          }
       
       }
       
    }
    
    // Load in the quad tree survey planes
    
    rewind(case_file);
    
    NumberOfQuadTrees_ = 0;
    
    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfQuadTrees") != NULL ) {

          sscanf(DumChar,"NumberOfQuadTrees = %d \n",&NumberOfQuadTrees_);
          
          printf("NumberOfQuadTrees: %d \n",NumberOfQuadTrees_);
          
          VSP_VLM().SetNumberOfQuadTrees(NumberOfQuadTrees_);
          
          for ( i = 1 ; i <= NumberOfQuadTrees_ ; i++ ) {
             
             fscanf(case_file,"%d %d %lf \n",&j,&Dir,&Value);
             
             printf("QuadTree(%d): %10d: %10.5f \n",j,Dir,Value);
             
             VSP_VLM().QuadTreeDirection(i) = Dir;
             VSP_VLM().QuadTreeValue(i) = Value;
             
          }
       
       }
       
    }

    // Determine the number of inlets and nozzles
    
    rewind(case_file);
    
    NumberOfInlets_ = NumberOfNozzles_ = 0;

    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfInlets") != NULL ) {

          sscanf(DumChar,"NumberOfInlets = %d \n",&NumberOfInlets_);
          
          printf("NumberOfInlets: %d \n",NumberOfInlets_);
	  
       }
       
    }
    
    rewind(case_file);
	  
    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfNozzles") != NULL ) {

          sscanf(DumChar,"NumberOfNozzles = %d \n",&NumberOfNozzles_);
          
          printf("NumberOfNozzles: %d \n",NumberOfNozzles_);
	  
       }
       
    }
    
    VSP_VLM().SetNumberOfEngineFaces(NumberOfInlets_ + NumberOfNozzles_);
    
    // Read in inlet data
    
    rewind(case_file);
    	      
    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfInlets") != NULL ) {

          sscanf(DumChar,"NumberOfInlets = %d \n",&NumberOfInlets_);
          
          printf("NumberOfInlets: %d \n",NumberOfInlets_);
          
          for ( i = 1 ; i <= NumberOfInlets_ ; i++ ) {
             
             fscanf(case_file,"%d %d %lf \n",&j,&Surface,&MassFlow);

             printf("EngineFace(%d) with surface %10d and massflow of %10.5f is an inlet \n",j,Surface,-MassFlow);
                 
             VSP_VLM().EngineFace(i).SurfaceType() = INLET_FACE;
             VSP_VLM().EngineFace(i).SurfaceID() = Surface;
             VSP_VLM().EngineFace(i).MassFlow() = MassFlow;
             
          }
       
       }
       
    }
      
    // Read in nozzle data
    
    rewind(case_file);
    	      
    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfNozzles") != NULL ) {

          sscanf(DumChar,"NumberOfNozzles = %d \n",&NumberOfInlets_);
          
          printf("NumberOfNozzles: %d \n",NumberOfNozzles_);
          
          for ( i = 1 ; i <= NumberOfNozzles_ ; i++ ) {
	     
	          k = NumberOfInlets_ + i;
             
             fscanf(case_file,"%d %d %lf %lf \n",&j,&Surface,&Velocity,&DeltaCp);

	          fscanf(case_file,"%lf %lf %lf \n",&(VSP_VLM().EngineFace(k).NozzleXYZ(0)),&(VSP_VLM().EngineFace(k).NozzleXYZ(1)),&(VSP_VLM().EngineFace(k).NozzleXYZ(2)));
	          fscanf(case_file,"%lf %lf %lf \n",&(VSP_VLM().EngineFace(k).NozzleNormal(0)),&(VSP_VLM().EngineFace(k).NozzleNormal(1)),&(VSP_VLM().EngineFace(k).NozzleNormal(2)));	     
	          fscanf(case_file,"%lf \n",&(VSP_VLM().EngineFace(i).NozzleRadius()));
                      
             printf("EngineFace(%d) with surface %10d and velocity of %10.5f is a nozzle \n",j,Surface,Velocity);
	          printf("Nozzle xyz: %f %f %f \n",VSP_VLM().EngineFace(k).NozzleXYZ(0),VSP_VLM().EngineFace(k).NozzleXYZ(1),VSP_VLM().EngineFace(k).NozzleXYZ(2));
 	          printf("Nozzle normal: %f %f %f \n",VSP_VLM().EngineFace(k).NozzleNormal(0),VSP_VLM().EngineFace(k).NozzleNormal(1),VSP_VLM().EngineFace(k).NozzleNormal(2));
	          printf("Nozzle radius: %f \n",VSP_VLM().EngineFace(k).NozzleRadius());
         
             VSP_VLM().EngineFace(k).SurfaceType() = NOZZLE_FACE;                
             VSP_VLM().EngineFace(k).SurfaceID() = Surface;
             VSP_VLM().EngineFace(k).NozzleVelocity() = Velocity;
             VSP_VLM().EngineFace(k).NozzleDeltaCp() = DeltaCp;
             
          }
       
       }
       
    }
                
    // Look for number of output time steps for survey points
    
    rewind(case_file);
        
    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfSurveyTimeSteps") != NULL ) {

          sscanf(DumChar,"NumberOfSurveyTimeSteps = %d \n",&NumberOfSurveyTimeSteps_);
          
          printf("NumberOfSurveyTimeSteps: %d \n",NumberOfSurveyTimeSteps_);
          
          VSP_VLM().NumberOfSurveyTimeSteps() = NumberOfSurveyTimeSteps_;
          
          Done = 1;

       }
       
    }
        
    // Load in unsteady aero data
    
    rewind(case_file);
    
    Done = 0;

    if ( VSP_VLM().TimeAnalysisType() == 0 ) {

        // Search for the number of time steps
            
        if ( SearchForIntegerVariable(case_file, "NumberOfTimeSteps", NumberOfTimeSteps_) ) printf("Setting NumberOfTimeSteps to: %d \n",NumberOfTimeSteps_);
        
        VSP_VLM().NumberOfTimeSteps() = NumberOfTimeSteps_;
    
        // Search for the start of the time averaging time step
            
        if ( SearchForIntegerVariable(case_file, "StartAveragingTimeStep", StartAveragingTimeStep_) ) printf("Setting StartAveragingTimeStep to: %d \n",StartAveragingTimeStep_);
        
        VSP_VLM().StartAveragingTimeStep() = StartAveragingTimeStep_;
    
        // Search for time step
            
        if ( SearchForFloatVariable(case_file, "TimeStep", TimeStep_) ) printf("Setting TimeStep_ to: %f \n",TimeStep_);
        
        VSP_VLM().DeltaTime() = TimeStep_;

    }
        
    fclose(case_file);
    
}

/*##############################################################################
#                                                                              #
#                           ApplyControlDeflections                            #
#                                                                              #
##############################################################################*/

int SearchForIntegerVariable(FILE *File, const char *VariableName, int &Value)
{
   
    int Found;
    char DumChar1[2000], DumChar2[2000], DumChar3[2000], Space[2000], *Next;
    
    if ( File == NULL ) {
       
       printf("%s ... Integer \n",VariableName);
    
       return 0;
          
    }
        
    snprintf(Space,sizeof(Space)*sizeof(char)," ");
    
    // Search file from the begining
    
    Found = 0;
    
    rewind(File);
        
    while ( fgets(DumChar1,2000,File) != NULL && !Found ) {

       snprintf(DumChar2,sizeof(DumChar2)*sizeof(char),"%s",DumChar1);
   
       Next = strtok(DumChar1,Space); 

       if ( strcmp(Next,VariableName) == 0 ) {
          
          Found = 1;
 
          sscanf(DumChar2,"%s = %d \n",DumChar3,&Value);

       }
      
    }
        
    return Found; 

}

/*##############################################################################
#                                                                              #
#                           ApplyControlDeflections                            #
#                                                                              #
##############################################################################*/

int SearchForCharacterVariable(FILE *File, const char *VariableName, char *Variable)
{

    int Found;
    char DumChar1[2000], DumChar2[2000], DumChar3[2000], Space[2000], *Next;
    
    if ( File == NULL ) {
       
       printf("%s ... character string\n",VariableName);
    
       return 0;
          
    }
        
    snprintf(Space,sizeof(Space)*sizeof(char)," ");
    
    // Search file from the begining
    
    Found = 0;
    
    rewind(File);
        
    while ( fgets(DumChar1,2000,File) != NULL && !Found ) {

       snprintf(DumChar2,sizeof(DumChar2)*sizeof(char),"%s",DumChar1);
   
       Next = strtok(DumChar1,Space); 

       if ( strcmp(Next,VariableName) == 0 ) {
          
          Found = 1;
 
          sscanf(DumChar2,"%s = %s \n",DumChar3,Variable);

       }
      
    }
        
    return Found; 

}

/*##############################################################################
#                                                                              #
#                           ApplyControlDeflections                            #
#                                                                              #
##############################################################################*/

int SearchForFloatVariable(FILE *File, const char *VariableName, double &Value)
{

    int Found;
    char DumChar1[2000], DumChar2[2000], DumChar3[2000], Space[2000], *Next;
    
    if ( File == NULL ) {
       
       printf("%s ... Float \n",VariableName);
    
       return 0;
          
    }
        
    snprintf(Space,sizeof(Space)*sizeof(char)," ");
    
    // Search file from the begining
    
    Found = 0;
    
    rewind(File);
        
    while ( fgets(DumChar1,2000,File) != NULL && !Found ) {

       snprintf(DumChar2,sizeof(DumChar2)*sizeof(char),"%s",DumChar1);
   
       Next = strtok(DumChar1,Space); 

       if ( strcmp(Next,VariableName) == 0 ) {
          
          Found = 1;
 
          sscanf(DumChar2,"%s = %lf \n",DumChar3,&Value);

       }
      
    }
        
    return Found; 

}

/*##############################################################################
#                                                                              #
#                           SearchForFloatVariableList                         #
#                                                                              #
##############################################################################*/

int SearchForFloatVariableList(FILE *File, const char *VariableName, double *Value, int &NumberOfEntries)
{
   
    int Found;
    double DumDouble;
    char DumChar1[2000], DumChar2[2000], Space[2000], Comma[2000], *Next;

    if ( File == NULL ) {
       
       printf("%s ... Float list \n",VariableName);
    
       return 0;
          
    }
    
    // Delimiters

    snprintf(Space,sizeof(Space)*sizeof(char)," ");
   
    snprintf(Comma,sizeof(Comma)*sizeof(char),",");
    
    // Search file from the begining
    
    Found = 0;
    
    rewind(File);
        
    while ( fgets(DumChar2,2000,File) != NULL && !Found ) {

       snprintf(DumChar1,sizeof(DumChar1)*sizeof(char),"%s",DumChar2);

       Next = strtok(DumChar2,Space); 

       if ( strcmp(Next,VariableName) == 0 ) {
          
          Found = 1;
 
       }
      
    }

    // If we found it, then we need to parse the line

       if ( Found ) {

          // There's just one entry
          
          if ( strstr(DumChar1,Comma) == NULL ) {
      
             sscanf(DumChar1,"%s = %lf \n",DumChar2,&DumDouble);
             
             NumberOfEntries = 1;

             Value[1] = DumDouble; 

          }
          
          // There's a list
          
          else {
      
             Next = strtok(DumChar1,Comma);
             
             sscanf(DumChar1,"%s = %lf \n",DumChar2,&DumDouble);

             Value[1] = DumDouble;
             
             NumberOfEntries = 1;
  
             while ( Next != NULL ) {
                
                 Next = strtok(NULL,Comma);
                 
                 if ( Next != NULL ) {
                    
                    DumDouble = atof(Next);
   
                    Value[++NumberOfEntries] = DumDouble;
          
                 }
                 
             }
  
          }  
          
       }

    return Found; 

}

/*##############################################################################
#                                                                              #
#                           SearchForIntVariableList                           #
#                                                                              #
##############################################################################*/

int SearchForIntVariableList(FILE *File, const char *VariableName, int *Value, int &NumberOfEntries)
{
   
    int Found, DumInt;
    char DumChar1[2000], DumChar2[2000], Space[2000], Comma[2000], *Next;

    if ( File == NULL ) {
       
       printf("%s ... Integer list \n",VariableName);
    
       return 0;
          
    }
    
    // Delimiters

    snprintf(Space,sizeof(Space)*sizeof(char)," ");
   
    snprintf(Comma,sizeof(Comma)*sizeof(char),",");
    
    // Search file from the begining
    
    Found = 0;
    
    rewind(File);
        
    while ( fgets(DumChar2,2000,File) != NULL && !Found ) {

       snprintf(DumChar1,sizeof(DumChar1)*sizeof(char),"%s",DumChar2);

       Next = strtok(DumChar2,Space); 

       if ( strcmp(Next,VariableName) == 0 ) {
          
          Found = 1;
 
       }
      
    }

    // If we found it, then we need to parse the line

       if ( Found ) {

          // There's just one entry
          
          if ( strstr(DumChar1,Comma) == NULL ) {
      
             sscanf(DumChar1,"%s = %d \n",DumChar2,&DumInt);
             
             NumberOfEntries = 1;

             Value[1] = DumInt; 

          }
          
          // There's a list
          
          else {
      
             Next = strtok(DumChar1,Comma);
             
             sscanf(DumChar1,"%s = %d \n",DumChar2,&DumInt);

             Value[1] = DumInt;
             
             NumberOfEntries = 1;
  
             while ( Next != NULL ) {
                
                 Next = strtok(NULL,Comma);
                 
                 if ( Next != NULL ) {
                    
                    DumInt = atoi(Next);
   
                    Value[++NumberOfEntries] = DumInt;
          
                 }
                 
             }
  
          }  
          
       }

    return Found; 

}

/*##############################################################################
#                                                                              #
#                           ApplyControlDeflections                            #
#                                                                              #
##############################################################################*/

void ApplyControlDeflections()
{
    int i, j, k, Found;

    // Set control surface deflections
 
    Found = 0;
 
    for ( i = 1; i <= NumberOfControlGroups_; i++ ) {

       for ( j = 1 ; j <= ControlSurfaceGroup_[i].NumberOfControlSurfaces(); j++ ) {

          Found = 0;
          
          for ( k = 1 ; k <= VSP_VLM().VSPGeom().NumberOfControlSurfaces() ; k++ ) {

             if ( strstr( VSP_VLM().VSPGeom().ControlSurface(k).Name(), ControlSurfaceGroup_[i].ControlSurface_Name(j) ) != NULL ) {

                 Found = 1;
            
                 VSP_VLM().VSPGeom().ControlSurface(k).DeflectionAngle() = ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j) * ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle() * TORAD;

             }
 
          }
          
          // Print out error report
          
          if ( !Found ) {
             
              printf("Could not find control surface: %s in control surface group: %s \n",
                     ControlSurfaceGroup_[i].ControlSurface_Name(j),
                     ControlSurfaceGroup_[i].Name() );
    
              // print out names of all known surfaces
              
              printf( "Known control surfaces:\n" );
              
              for ( k = 1 ; k <= VSP_VLM().VSPGeom().NumberOfControlSurfaces() ; k++ ) {
                 
                 printf( "\t%20s\n", VSP_VLM().VSPGeom().ControlSurface(k).Name() );
              
              }
              
              fflush(NULL);exit(1);
         
          }
       
       }

    }

}

/*##############################################################################
#                                                                              #
#                                   Solve                                      #
#                                                                              #
##############################################################################*/

void Solve(void)
{

    int i, j, k, p, Case, NumCases, ****CaseList;
    double AR, E;
    char PolarFileName[2000];
    FILE *PolarFile;

    ApplyControlDeflections();
    
    NumCases = NumberOfBetas_ * NumberOfMachs_ * NumberOfAoAs_ * NumberOfReCrefs_;
    
    CaseList = new int***[NumberOfBetas_ + 1];
    
    for ( i = 1 ; i <= NumberOfBetas_ ; i++ ) {
       
       CaseList[i] = new int**[NumberOfMachs_ + 1];
       
       for ( j = 1 ; j <= NumberOfMachs_; j++ ) {
          
          CaseList[i][j] = new int*[NumberOfAoAs_ + 1];
             
          for ( k = 1 ; k <= NumberOfAoAs_ ; k++ ) {
             
             CaseList[i][j][k] = new int[NumberOfReCrefs_ + 1];
             
          }
          
       }
       
    }
    
    Case = 0;

    for ( i = 1 ; i <= NumberOfBetas_ ; i++ ) {
       
       for ( j = 1 ; j <= NumberOfMachs_; j++ ) {
             
          for ( k = 1 ; k <= NumberOfAoAs_ ; k++ ) {
             
             Case++;
             
             CaseList[i][j][k][1] = Case;
             
             // Set free stream conditions
             
             VSP_VLM().AngleOfBeta()   = BetaList_[i] * TORAD;
             VSP_VLM().Mach()          = MachList_[j];  
             VSP_VLM().AngleOfAttack() =  AoAList_[k] * TORAD;
     
             VSP_VLM().RotationalRate_p() = 0.;
             VSP_VLM().RotationalRate_q() = 0.;
             VSP_VLM().RotationalRate_r() = 0.;

             // Set a comment line
             
             snprintf(VSP_VLM().CaseString(),sizeof(VSP_VLM().CaseString())*sizeof(char),"Case: %-d ...",Case);
             
             if ( DoGroundEffectsAnalysis_ ) snprintf(VSP_VLM().CaseString(),sizeof(VSP_VLM().CaseString())*sizeof(char),"AoA: %7.3f ... H: %8.3f",-VSP_VLM().VehicleRotationAngleVector(1),HeightAboveGround_);

             // Solve this case
             
             if ( SaveRestartFile_ ) VSP_VLM().SaveRestartFile() = 1;
   
             if ( DoRestartRun_    ) VSP_VLM().DoRestart() = 1;
             
             if ( Case == 1 || Case < NumCases ) {
                
                if ( RestartAndInterrogateSolution_ ) {
                   
                   VSP_VLM().RestartAndInterrogateSolution(Case);
                   
                }
                
                else {
          
                   VSP_VLM().Solve(Case);
                   
                }
                
             }
             
             else {
               
                if ( RestartAndInterrogateSolution_ ) {
                   
                   VSP_VLM().RestartAndInterrogateSolution(-Case);
                   
                }
                                
                else {
                                   
                   VSP_VLM().Solve(-Case);
                   
                }
                
             }

             // Store aero coefficients
             
             CLForCase[Case] = VSP_VLM().CLi() + VSP_VLM().CLo(); 
             CDForCase[Case] = VSP_VLM().CDi() + VSP_VLM().CDo();       
             CSForCase[Case] = VSP_VLM().CSi() + VSP_VLM().CSo();        
             
             CDoForCase[Case] = VSP_VLM().CDo();     
             
             CDtForCase[Case] = VSP_VLM().CDiw();        
             
             CFxForCase[Case] = VSP_VLM().CFix() + VSP_VLM().CFox();
             CFyForCase[Case] = VSP_VLM().CFiy() + VSP_VLM().CFoy();       
             CFzForCase[Case] = VSP_VLM().CFiz() + VSP_VLM().CFoz();       
                 
             CMxForCase[Case] = VSP_VLM().CMix() + VSP_VLM().CMox();      
             CMyForCase[Case] = VSP_VLM().CMiy() + VSP_VLM().CMoy();      
             CMzForCase[Case] = VSP_VLM().CMiz() + VSP_VLM().CMoz();    
             
             CMlForCase[Case] = -CMxForCase[Case];    
             CMmForCase[Case] =  CMyForCase[Case];       
             CMnForCase[Case] = -CMzForCase[Case];        

             // Loop over any ReCref cases
             
             for ( p = 2 ; p <= NumberOfReCrefs_ ; p++ ) {
                
                Case++;
                
                CaseList[i][j][k][p] = Case;
                
                ReCref_ = ReCrefList_[p];
     
                VSP_VLM().ReCref() = ReCref_;
                
                VSP_VLM().ReCalculateForces();
                
                // Store aero coefficients
                
                CLForCase[Case] = VSP_VLM().CLi() + VSP_VLM().CLo(); 
                CDForCase[Case] = VSP_VLM().CDi() + VSP_VLM().CDo();       
                CSForCase[Case] = VSP_VLM().CSi() + VSP_VLM().CSo();        
                
                CDoForCase[Case] = VSP_VLM().CDo();     
                
                CDtForCase[Case] = VSP_VLM().CDiw();        
                
                CFxForCase[Case] = VSP_VLM().CFix() + VSP_VLM().CFox();
                CFyForCase[Case] = VSP_VLM().CFiy() + VSP_VLM().CFoy();       
                CFzForCase[Case] = VSP_VLM().CFiz() + VSP_VLM().CFoz();       
                    
                CMxForCase[Case] = VSP_VLM().CMix() + VSP_VLM().CMox();      
                CMyForCase[Case] = VSP_VLM().CMiy() + VSP_VLM().CMoy();      
                CMzForCase[Case] = VSP_VLM().CMiz() + VSP_VLM().CMoz();    
                
                CMlForCase[Case] = -CMxForCase[Case];    
                CMmForCase[Case] =  CMyForCase[Case];       
                CMnForCase[Case] = -CMzForCase[Case];     
                
             } 
             
             ReCref_ = ReCrefList_[1];
  
             VSP_VLM().ReCref() = ReCref_;             
                
             printf("\n");
      
          }
          
       }
       
    }

    // Write out final integrated force data
    
    snprintf(PolarFileName,sizeof(PolarFileName)*sizeof(char),"%s.polar",FileName);

    if ( (PolarFile = fopen(PolarFileName,"w")) == NULL ) {

       printf("Could not open the polar file output! \n");

       exit(1);

    }    

                     //1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456      
    fprintf(PolarFile,"      Beta             Mach             AoA             Re/1e6             CL              CDo              CDi             CDtot             CDt            CDtot_t             CS              L/D               E               CFx              CFy              CFz              CMx              CMy              CMz              CMl              CMm              CMn              FOpt \n");

    // Write out polars, note these are written out in a different order than they were calculated above - we group them by Re number
    
    for ( p = 1 ; p <= NumberOfReCrefs_ ; p++ ) {

       for ( i = 1 ; i <= NumberOfBetas_ ; i++ ) {
          
          for ( j = 1 ; j <= NumberOfMachs_; j++ ) {
                
             for ( k = 1 ; k <= NumberOfAoAs_ ; k++ ) {
 
                Case = CaseList[i][j][k][p];
                   
                AR = Bref_ * Bref_ / Sref_;
   
                E = ( CLForCase[Case] * CLForCase[Case] / ( PI * AR) ) / CDForCase[Case];
             
                fprintf(PolarFile,"%16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf\n",             
                        double(BetaList_[i]),
                        double(MachList_[j]),
                        double(AoAList_[k]),
                        double(ReCrefList_[p])/1.e6,
                        double(CLForCase[Case]),
                        double(CDoForCase[Case]),
                        double(CDForCase[Case] - CDoForCase[Case]),
                        double(CDForCase[Case]),
                        double(CDtForCase[Case]),
                        double(CDoForCase[Case] + CDtForCase[Case]),
                        double(CSForCase[Case]),            
                        double(CLForCase[Case]/(CDForCase[Case])),
                        double(E),
                        double(CFxForCase[Case]),
                        double(CFyForCase[Case]),
                        double(CFzForCase[Case]),
                        double(CMxForCase[Case]),
                        double(CMyForCase[Case]),
                        double(CMzForCase[Case]),
                        double(CMlForCase[Case]),       
                        double(CMmForCase[Case]),       
                        double(CMnForCase[Case]),
                        double(OptimizationFunctionForCase[Case]));
                           
             }
             
          }
          
       }       
       
    }
    
    fclose(PolarFile);

}

/*##############################################################################
#                                                                              #
#                 FiniteDifference_StabilityAndControlSolve                    #
#                                                                              #
##############################################################################*/

void FiniteDifference_StabilityAndControlSolve(void)
{

    int i, j, k, p, ic, jc, kc, Found, Case, Case0, Deriv, TotalCases, CaseTotal;
    char StabFileName[2000], VorviewFltFileName[2000];
    
    // Open the stability and control output file
    
    snprintf(StabFileName,sizeof(StabFileName)*sizeof(char),"%s.stab",FileName);

    if ( (StabFile = fopen(StabFileName,"w")) == NULL ) {

       printf("Could not open the stability and control file for output! \n");

       exit(1);

    }
    
    // Open the vorview flt
    
    snprintf(VorviewFltFileName,sizeof(VorviewFltFileName)*sizeof(char),"%s.flt",FileName);

    if ( (VorviewFlt = fopen(VorviewFltFileName,"w")) == NULL ) {

       printf("Could not open the vorview flt stability and control file for output! \n");

       exit(1);

    }
   
    VSP_VLM().ForwardGMRESConvergenceFactor() = ForwardGMRESConvergenceFactor_;
     
    TotalCases = ( NumStabCases_ + NumberOfControlGroups_) * NumberOfMachs_ * NumberOfAoAs_ * NumberOfBetas_;
    
    Case = CaseTotal = 0;
    
    for ( ic = 1 ; ic <= NumberOfBetas_ ; ic++ ) {
       
       for ( jc = 1 ; jc <= NumberOfMachs_; jc++ ) {
             
          for ( kc = 1 ; kc <= NumberOfAoAs_ ; kc++ ) {
             
             Case++;
                          
             // Set free stream conditions
             
             Beta_ = BetaList_[ic];
             Mach_ = MachList_[jc];  
             AoA_  =  AoAList_[kc];

             // Set Control surface group deflection to un-perturbed control surface deflections

             ApplyControlDeflections();
             
             // Perform stability and control calculation for this Mach, Alpha, Beta condition
      
             Delta_AoA_     = 0.01;
             Delta_Beta_    = 0.01;
             Delta_Mach_    = 0.1; if ( ABS(Mach_ + Delta_Mach_ - 1.) <= 0.01 ) Delta_Mach_ /= 2.;
             Delta_P_       = 0.01;
             Delta_Q_       = 0.01;
             Delta_R_       = 0.01;
             Delta_Control_ = 0.10;
         
             Case = 1;
         
             Case0 = Case;
             
             // Stability derivative cases
         
             for ( Deriv = 1 ; Deriv <= 7 ; Deriv++ ) {
         
                 Stab_MachList_[Case] = Mach_;
                  Stab_AoAList_[Case] = AoA_;
                 Stab_BetaList_[Case] = Beta_;
         
                RotationalRate_pList_[Case] = 0.;
                RotationalRate_qList_[Case] = 0.;
                RotationalRate_rList_[Case] = 0.;
         
                Case++;
         
             }
         
             // Perturb in alpha
                          
             Stab_AoAList_[Case0 + 1] = AoA_ + Delta_AoA_;
              
             // Perturb in Beta
                 
             Stab_BetaList_[Case0 + 2] = Beta_ + Delta_Beta_;
             
             // Perturb roll rate
         
             RotationalRate_pList_[Case0 + 3] = Delta_P_;
             RotationalRate_qList_[Case0 + 3] = 0.;
             RotationalRate_rList_[Case0 + 3] = 0.;    
         
             // Perturb pitch rate
         
             RotationalRate_pList_[Case0 + 4] = 0.;
             RotationalRate_qList_[Case0 + 4] = Delta_Q_;
             RotationalRate_rList_[Case0 + 4] = 0.;    
             
             // Perturb yaw rate
         
             RotationalRate_pList_[Case0 + 5] = 0.;
             RotationalRate_qList_[Case0 + 5] = 0.;
             RotationalRate_rList_[Case0 + 5] = Delta_R_;  
             
             // Perturb Mach number
         
             Stab_MachList_[Case0 + 6] = Mach_ + Delta_Mach_;
          
             printf("Calculating Stability Derivatives... \n"); 
         
             for ( Case = 1 ; Case <= NumStabCases_ ; Case++ ) {
                
                CaseTotal++;

                printf("Calculating stability derivative case: %d of %d \n",Case,NumStabCases_);
                
                // Set free stream conditions
                
                VSP_VLM().Mach()          = Stab_MachList_[Case];
                VSP_VLM().AngleOfAttack() =  Stab_AoAList_[Case] * TORAD;
                VSP_VLM().AngleOfBeta()   = Stab_BetaList_[Case] * TORAD;
         
                VSP_VLM().RotationalRate_p() = RotationalRate_pList_[Case];
                VSP_VLM().RotationalRate_q() = RotationalRate_qList_[Case];
                VSP_VLM().RotationalRate_r() = RotationalRate_rList_[Case];
                
                // Set a comment line

                if ( Case == 1 ) snprintf(VSP_VLM().CaseString(),sizeof(VSP_VLM().CaseString())*sizeof(char),"Base Aero         ");
                if ( Case == 2 ) snprintf(VSP_VLM().CaseString(),sizeof(VSP_VLM().CaseString())*sizeof(char),"Alpha      +%5.3lf",Delta_AoA_);
                if ( Case == 3 ) snprintf(VSP_VLM().CaseString(),sizeof(VSP_VLM().CaseString())*sizeof(char),"Beta       +%5.3lf",Delta_Beta_);
                if ( Case == 4 ) snprintf(VSP_VLM().CaseString(),sizeof(VSP_VLM().CaseString())*sizeof(char),"Roll Rate  +%5.3lf",Delta_P_);
                if ( Case == 5 ) snprintf(VSP_VLM().CaseString(),sizeof(VSP_VLM().CaseString())*sizeof(char),"Pitch Rate +%5.3lf",Delta_Q_);
                if ( Case == 6 ) snprintf(VSP_VLM().CaseString(),sizeof(VSP_VLM().CaseString())*sizeof(char),"Yaw Rate   +%5.3lf",Delta_R_);
                if ( Case == 7 ) snprintf(VSP_VLM().CaseString(),sizeof(VSP_VLM().CaseString())*sizeof(char),"Mach       +%5.3lf",Delta_Mach_);         
                
                // Solve this case
                
                VSP_VLM().SaveRestartFile() = VSP_VLM().DoRestart() = 0;
         
                if ( CaseTotal < TotalCases ) {
                   
                   VSP_VLM().Solve(CaseTotal);
                   
                }
                
                else {
                   
                   VSP_VLM().Solve(-CaseTotal);
                   
                }         
                   
                // Store aero coefficients
           
                CLForCase[Case] = VSP_VLM().CLi() + VSP_VLM().CLo(); 
                CDForCase[Case] = VSP_VLM().CDi() + VSP_VLM().CDo();       
                CSForCase[Case] = VSP_VLM().CSi() + VSP_VLM().CSo();        
                
                CDoForCase[Case] = VSP_VLM().CDo();     

                CDtForCase[Case] = VSP_VLM().CDiw();        
         
                CFxForCase[Case] = VSP_VLM().CFix() + VSP_VLM().CFox();
                CFyForCase[Case] = VSP_VLM().CFiy() + VSP_VLM().CFoy();       
                CFzForCase[Case] = VSP_VLM().CFiz() + VSP_VLM().CFoz();       
                                              
                CMxForCase[Case] = VSP_VLM().CMix() + VSP_VLM().CMox();      
                CMyForCase[Case] = VSP_VLM().CMiy() + VSP_VLM().CMoy();      
                CMzForCase[Case] = VSP_VLM().CMiz() + VSP_VLM().CMoz();    
                
                CMlForCase[Case] = -CMxForCase[Case];    
                CMmForCase[Case] =  CMyForCase[Case];       
                CMnForCase[Case] = -CMzForCase[Case];                  

                printf("\n");
         
             }
             
             Case--;
             
             // Now do the control derivatives
             
             printf("Calculating Control Derivatives... \n"); 
          
             for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) {
                
                CaseTotal++;
                
                printf("Calculating control derivative case: %d of %d \n",i,NumberOfControlGroups_);
                
                // Initialize to unperturbed free stream conditions
                
                VSP_VLM().Mach()          = Stab_MachList_[1];
                VSP_VLM().AngleOfAttack() =  Stab_AoAList_[1] * TORAD;
                VSP_VLM().AngleOfBeta()   = Stab_BetaList_[1] * TORAD;
             
                VSP_VLM().RotationalRate_p() = RotationalRate_pList_[1];
                VSP_VLM().RotationalRate_q() = RotationalRate_qList_[1];
                VSP_VLM().RotationalRate_r() = RotationalRate_rList_[1];
             
                // Perturb controls
             
                Case++;
                
                k = 1;
                
                for ( j = 1 ; j <= ControlSurfaceGroup_[i].NumberOfControlSurfaces() ; j++ ) {
                  
                   printf("Looking for: %s \n",ControlSurfaceGroup_[i].ControlSurface_Name(j));fflush(NULL);
                   
                   Found = 0;
             
                   p = 1;
                   
                   while ( p <= VSP_VLM().VSPGeom().NumberOfControlSurfaces() && !Found ) {
                      
                      printf("Checking: %s \n",VSP_VLM().VSPGeom().ControlSurface(p).Name());fflush(NULL);
           
                      if ( strstr(VSP_VLM().VSPGeom().ControlSurface(p).Name(), ControlSurfaceGroup_[i].ControlSurface_Name(j)) != NULL ) {
               
                         Found = 1;
                      
                         VSP_VLM().VSPGeom().ControlSurface(p).DeflectionAngle() = ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j) * (ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle() + Delta_Control_) * TORAD;

                      }
                      
                      p++;
                     
                   }
                     
                   if ( !Found ) {
                      
                      printf("Could not find control surface: %s in control surface group: %s \n",
                              ControlSurfaceGroup_[i].ControlSurface_Name(j),
                              ControlSurfaceGroup_[i].Name()); fflush(NULL);
                              
                      exit(1);
                      
                   }
                  
                }
                
                // Set a comment line
             
                snprintf(VSP_VLM().CaseString(),sizeof(VSP_VLM().CaseString())*sizeof(char),"Deflecting Control Group: %-d",i);
               
                // Now solve
               
                if ( CaseTotal < TotalCases ) {
                   
                   VSP_VLM().Solve(CaseTotal);
                   
                }
                
                else {
                   
                   VSP_VLM().Solve(-CaseTotal);
                   
                }         

                // Store aero coefficients
           
                CLForCase[Case] = VSP_VLM().CLi() + VSP_VLM().CLo(); 
                CDForCase[Case] = VSP_VLM().CDi() + VSP_VLM().CDo();       
                CSForCase[Case] = VSP_VLM().CSi() + VSP_VLM().CSo();        
                
                CDoForCase[Case] = VSP_VLM().CDo();     

                CDtForCase[Case] = VSP_VLM().CDiw();        
         
                CFxForCase[Case] = VSP_VLM().CFix() + VSP_VLM().CFox();
                CFyForCase[Case] = VSP_VLM().CFiy() + VSP_VLM().CFoy();       
                CFzForCase[Case] = VSP_VLM().CFiz() + VSP_VLM().CFoz();       
                                              
                CMxForCase[Case] = VSP_VLM().CMix() + VSP_VLM().CMox();      
                CMyForCase[Case] = VSP_VLM().CMiy() + VSP_VLM().CMoy();      
                CMzForCase[Case] = VSP_VLM().CMiz() + VSP_VLM().CMoz();    
                
                CMlForCase[Case] = -CMxForCase[Case];    
                CMmForCase[Case] =  CMyForCase[Case];       
                CMnForCase[Case] = -CMzForCase[Case];           

                // Reset Control surface group deflection to un-perturbed control surface deflections
             
                ApplyControlDeflections();
             
             }
             
             // Now calculate actual stability derivatives 
             
             CalculateStabilityDerivatives();
             
             // Write them out in the old vorview format... sigh
             
             WriteOutVorviewFLTFile();
             
          }
          
       }
       
    }
                   
    fclose(StabFile);
    fclose(VorviewFlt);
    
}

/*##############################################################################
#                                                                              #
#                           CalculateStabilityDerivatives                      #
#                                                                              #
##############################################################################*/

void CalculateStabilityDerivatives(void)
{

    int n;
    double Delta, SM, X_np;
    char CaseType[2000];
    char caseTypeFormatStr[] = "%-22s +%5.3lf %-9s";

    // Set free stream conditions

    VSP_VLM().Mach()          = Mach_;
    VSP_VLM().AngleOfAttack() = AoA_  * TORAD;
    VSP_VLM().AngleOfBeta()   = Beta_ * TORAD;

    VSP_VLM().RotationalRate_p() = 0.;
    VSP_VLM().RotationalRate_q() = 0.;
    VSP_VLM().RotationalRate_r() = 0.;
        
    // Write out generic header file
        
    VSP_VLM().WriteCaseHeader(StabFile);
    
    // Write out column labels
    
    fprintf(StabFile,"#\n");

    //                123456789012345678901234567890123456789 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012   
    fprintf(StabFile,"Case                   Delta    Units         CFx          CFy          CFz          CMx          CMy          CMz          CL           CD           CS           CMl          CMm          CMn\n");
       
    fprintf(StabFile,"#\n");

    for ( n = 1 ; n <= NumStabCases_ + NumberOfControlGroups_ ; n++ ) {
       
       // Stability derivative cases
                                     //12345678901234567890123456789
                                     
       if ( n == 1 ) snprintf(CaseType,sizeof(CaseType)*sizeof(char),caseTypeFormatStr,"Base_Aero", 0.0,        "n/a"); // note must print a "-" for the units column here or the parser in VSP has a hard time
       if ( n == 2 ) snprintf(CaseType,sizeof(CaseType)*sizeof(char),caseTypeFormatStr,"Alpha",     Delta_AoA_, "deg");
       if ( n == 3 ) snprintf(CaseType,sizeof(CaseType)*sizeof(char),caseTypeFormatStr,"Beta",      Delta_Beta_,"deg");
       if ( n == 4 ) snprintf(CaseType,sizeof(CaseType)*sizeof(char),caseTypeFormatStr,"Roll__Rate",Delta_P_,   "rad/Tunit");
       if ( n == 5 ) snprintf(CaseType,sizeof(CaseType)*sizeof(char),caseTypeFormatStr,"Pitch_Rate",Delta_Q_,   "rad/Tunit");
       if ( n == 6 ) snprintf(CaseType,sizeof(CaseType)*sizeof(char),caseTypeFormatStr,"Yaw___Rate",Delta_R_,   "rad/Tunit");
       if ( n == 7 ) snprintf(CaseType,sizeof(CaseType)*sizeof(char),caseTypeFormatStr,"Mach",      Delta_Mach_,"no_unit");

       if ( n  > 7 ) snprintf(CaseType,sizeof(CaseType)*sizeof(char),caseTypeFormatStr,ControlSurfaceGroup_[n - NumStabCases_].Name(), Delta_Control_, "deg" );
       
       fprintf(StabFile,"%-39s %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f \n",
               CaseType,
               
               CFxForCase[n],
               CFyForCase[n],    
               CFzForCase[n], 
                      
               CMxForCase[n],  
               CMyForCase[n],  
               CMzForCase[n],    
                      
               CLForCase[n],
               CDForCase[n],   
               CSForCase[n],
               
               CMlForCase[n],
               CMmForCase[n],   
               CMnForCase[n]);                 
 
       printf("\n");

    }
    
    fprintf(StabFile,"#\n");
    
    // Calculate the stability derivatives and write them out

    for ( n = 2 ; n <= NumStabCases_ ; n++ ) {
    
       if ( n == 2  ) Delta =  Delta_AoA_  * TORAD;            // wrt Alpha
       if ( n == 3  ) Delta =  Delta_Beta_ * TORAD;            // wrt Beta
       if ( n == 4  ) Delta =  Delta_P_ * Bref_ * 0.5 / Vinf_; // wrt roll rate
       if ( n == 5  ) Delta =  Delta_Q_ * Cref_ * 0.5 / Vinf_; // wrt pitch rate
       if ( n == 6  ) Delta =  Delta_R_ * Bref_ * 0.5 / Vinf_; // wrt yaw rate
       if ( n == 7  ) Delta =  Delta_Mach_;                    // wrt Mach number

       dCFx_wrt[n] = ( CFxForCase[n] - CFxForCase[1] )/Delta;
       dCFy_wrt[n] = ( CFyForCase[n] - CFyForCase[1] )/Delta;
       dCFz_wrt[n] = ( CFzForCase[n] - CFzForCase[1] )/Delta;
       
       dCMx_wrt[n] = ( CMxForCase[n] - CMxForCase[1] )/Delta;
       dCMy_wrt[n] = ( CMyForCase[n] - CMyForCase[1] )/Delta;
       dCMz_wrt[n] = ( CMzForCase[n] - CMzForCase[1] )/Delta;
        
       dCL_wrt[n]  = (  CLForCase[n] -  CLForCase[1] )/Delta;    
       dCD_wrt[n]  = (  CDForCase[n] -  CDForCase[1] )/Delta;    
       dCS_wrt[n]  = (  CSForCase[n] -  CSForCase[1] )/Delta;    
       
       dCMl_wrt[n] = ( CMlForCase[n] - CMlForCase[1] )/Delta;
       dCMm_wrt[n] = ( CMmForCase[n] - CMmForCase[1] )/Delta;
       dCMn_wrt[n] = ( CMnForCase[n] - CMnForCase[1] )/Delta;       

    }

    dCFx_wrt[8] = MachList_[1] * dCFx_wrt[7];
    dCFy_wrt[8] = MachList_[1] * dCFy_wrt[7];
    dCFz_wrt[8] = MachList_[1] * dCFz_wrt[7];

    dCMx_wrt[8] = MachList_[1] * dCMx_wrt[7];
    dCMy_wrt[8] = MachList_[1] * dCMy_wrt[7];
    dCMz_wrt[8] = MachList_[1] * dCMz_wrt[7];

    dCL_wrt[8]  = MachList_[1] * dCL_wrt[7];
    dCD_wrt[8]  = MachList_[1] * dCD_wrt[7];
    dCS_wrt[8]  = MachList_[1] * dCS_wrt[7];
    
    dCMl_wrt[8] = MachList_[1] * dCMl_wrt[7];
    dCMm_wrt[8] = MachList_[1] * dCMm_wrt[7];
    dCMn_wrt[8] = MachList_[1] * dCMn_wrt[7];    
    
    // Calculate the control derivatives and write them out

    for ( n = 8 ; n <= NumStabCases_ + NumberOfControlGroups_ ; n++ ) {
    
       Delta = Delta_Control_ * TORAD; // wrt control group deflection

       dCFx_wrt[n+1] = ( CFxForCase[n] - CFxForCase[1] )/Delta;
       dCFy_wrt[n+1] = ( CFyForCase[n] - CFyForCase[1] )/Delta;
       dCFz_wrt[n+1] = ( CFzForCase[n] - CFzForCase[1] )/Delta;
       
       dCMx_wrt[n+1] = ( CMxForCase[n] - CMxForCase[1] )/Delta;
       dCMy_wrt[n+1] = ( CMyForCase[n] - CMyForCase[1] )/Delta;
       dCMz_wrt[n+1] = ( CMzForCase[n] - CMzForCase[1] )/Delta;
        
       dCL_wrt[n+1]  = (  CLForCase[n] -  CLForCase[1] )/Delta;    
       dCD_wrt[n+1]  = (  CDForCase[n] -  CDForCase[1] )/Delta;    
       dCS_wrt[n+1]  = (  CSForCase[n] -  CSForCase[1] )/Delta;    
       
       dCMl_wrt[n+1] = ( CMlForCase[n] - CMlForCase[1] )/Delta;
       dCMm_wrt[n+1] = ( CMmForCase[n] - CMmForCase[1] )/Delta;
       dCMn_wrt[n+1] = ( CMnForCase[n] - CMnForCase[1] )/Delta;       

    }    

    fprintf(StabFile,"#\n");
    
    //                        123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012           
    fprintf(StabFile,"#             Base    Derivative:                                                                               "); for ( n = 8 ; n <= NumStabCases_ + NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"             ");     fprintf(StabFile,"\n");
    fprintf(StabFile,"#             Aero         wrt          wrt          wrt          wrt          wrt          wrt          wrt    "); for ( n = 8 ; n <= NumStabCases_ + NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      wrt    ");     fprintf(StabFile,"\n");
    fprintf(StabFile,"Coef          Total        Alpha        Beta          p            q            r           Mach         U      "); for ( n = 8 ; n <= NumStabCases_ + NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"  ConGrp_%-3d ",n-7); fprintf(StabFile,"\n");
    fprintf(StabFile,"#              -           per          per          per          per          per          per          per    "); for ( n = 8 ; n <= NumStabCases_ + NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      per    ");     fprintf(StabFile,"\n");
    fprintf(StabFile,"#              -           rad          rad          rad          rad          rad          M            u      "); for ( n = 8 ; n <= NumStabCases_ + NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      rad    ");     fprintf(StabFile,"\n");
   
    fprintf(StabFile,"#\n");
    
    fprintf(StabFile,"CFx    "); fprintf(StabFile,"%12.7f ",CFxForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCFx_wrt[n]); }; fprintf(StabFile,"\n");
    fprintf(StabFile,"CFy    "); fprintf(StabFile,"%12.7f ",CFyForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCFy_wrt[n]); }; fprintf(StabFile,"\n");
    fprintf(StabFile,"CFz    "); fprintf(StabFile,"%12.7f ",CFzForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCFz_wrt[n]); }; fprintf(StabFile,"\n");

    fprintf(StabFile,"CMx    "); fprintf(StabFile,"%12.7f ",CMxForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMx_wrt[n]); }; fprintf(StabFile,"\n");
    fprintf(StabFile,"CMy    "); fprintf(StabFile,"%12.7f ",CMyForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMy_wrt[n]); }; fprintf(StabFile,"\n");
    fprintf(StabFile,"CMz    "); fprintf(StabFile,"%12.7f ",CMzForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMz_wrt[n]); }; fprintf(StabFile,"\n");

    fprintf(StabFile,"CL     "); fprintf(StabFile,"%12.7f ",CLForCase[1]);  for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCL_wrt[n]); };  fprintf(StabFile,"\n");
    fprintf(StabFile,"CD     "); fprintf(StabFile,"%12.7f ",CDForCase[1]);  for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCD_wrt[n]); };  fprintf(StabFile,"\n");
    fprintf(StabFile,"CS     "); fprintf(StabFile,"%12.7f ",CSForCase[1]);  for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCS_wrt[n]); };  fprintf(StabFile,"\n");

    fprintf(StabFile,"CMl    "); fprintf(StabFile,"%12.7f ",CMlForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMl_wrt[n]); }; fprintf(StabFile,"\n");
    fprintf(StabFile,"CMm    "); fprintf(StabFile,"%12.7f ",CMmForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMm_wrt[n]); }; fprintf(StabFile,"\n");
    fprintf(StabFile,"CMn    "); fprintf(StabFile,"%12.7f ",CMnForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMn_wrt[n]); }; fprintf(StabFile,"\n");

    fprintf(StabFile,"#\n");
    fprintf(StabFile,"#\n");
    fprintf(StabFile,"#\n");

    // Calculate static margin and neutral point and write them out
    
    char headerFormatStr[] = "%-20s %12s %-20s\n";
    char dataFormatStr[] = "%-20s %12.7lf %-20s\n";

    fprintf( StabFile, headerFormatStr, "# Result", "Value   ", "  Units" );

    SM = -1.0 * dCMm_wrt[2] / dCL_wrt[2]; // -1 * CMm_alpha / CL_alpha
    X_np = SM * Cref_ + Xcg_;

    fprintf( StabFile, dataFormatStr, "SM", SM, "no_unit" );
    fprintf( StabFile, dataFormatStr, "X_np", X_np, "Lunit" );

    fprintf( StabFile, "#\n" );
    fprintf( StabFile, "#\n" );
    fprintf( StabFile, "#\n" );

}
   
/*##############################################################################
#                                                                              #
#                           WriteOutVorviewFLTFile                             #
#                                                                              #
##############################################################################*/

void WriteOutVorviewFLTFile(void)
{

    fprintf(VorviewFlt,"\n");
    fprintf(VorviewFlt," ##################\n");
    fprintf(VorviewFlt,"\n");

    fprintf(VorviewFlt," MACH_o:	      %15f \n",Mach_);
    fprintf(VorviewFlt," ALPHA_o:	   %15f \n",AoA_);
    fprintf(VorviewFlt," BETA_o:       %15f \n",Beta_);
    fprintf(VorviewFlt," U_o:	         %15f \n",Vinf_);

    // Base aero

    fprintf(VorviewFlt,"\n");    
    fprintf(VorviewFlt," CLo:          %15f \n",CLForCase[1]);
    fprintf(VorviewFlt," CDo:          %15f \n",CDForCase[1] + CDoForCase[1]);
    fprintf(VorviewFlt," CYo:          %15f \n",CFyForCase[1]);
    fprintf(VorviewFlt," Clo:          %15f \n",CMlForCase[1]);
    fprintf(VorviewFlt," Cmo:          %15f \n",CMmForCase[1]);
    fprintf(VorviewFlt," Cno:          %15f \n",CMnForCase[1]);
    fprintf(VorviewFlt,"\n");

    // CL derivatives
    
    fprintf(VorviewFlt," CL_alpha:     %15f \n",dCL_wrt[2]);
    fprintf(VorviewFlt," CL_beta:      %15f \n",dCL_wrt[3]);
    fprintf(VorviewFlt," CL_mach:      %15f \n",dCL_wrt[7]);
    fprintf(VorviewFlt," CL_p:         %15f \n",dCL_wrt[4]);
    fprintf(VorviewFlt," CL_q:         %15f \n",dCL_wrt[5]);
    fprintf(VorviewFlt," CL_r:         %15f \n",dCL_wrt[6]);
    fprintf(VorviewFlt," CL_u:         %15f \n",dCL_wrt[8]);
    fprintf(VorviewFlt," CL_alpha_2:   %15f \n",0.);
    fprintf(VorviewFlt," CL_alpha_dot: %15f \n",0.);

    // CD derivatives
    
    fprintf(VorviewFlt," CD_alpha:     %15f \n",dCD_wrt[2]);
    fprintf(VorviewFlt," CD_beta:      %15f \n",dCD_wrt[3]);
    fprintf(VorviewFlt," CD_mach:      %15f \n",dCD_wrt[7]);
    fprintf(VorviewFlt," CD_p:         %15f \n",dCD_wrt[4]);
    fprintf(VorviewFlt," CD_q:         %15f \n",dCD_wrt[5]);
    fprintf(VorviewFlt," CD_r:         %15f \n",dCD_wrt[6]);
    fprintf(VorviewFlt," CD_u:         %15f \n",dCD_wrt[8]);
    fprintf(VorviewFlt," CD_alpha_2:   %15f \n",0.);
    fprintf(VorviewFlt," CD_alpha_dot: %15f \n",0.);

    // CY derivatives
    
    fprintf(VorviewFlt," CY_alpha:     %15f \n",dCFy_wrt[2]);
    fprintf(VorviewFlt," CY_beta:      %15f \n",dCFy_wrt[3]);
    fprintf(VorviewFlt," CY_mach:      %15f \n",dCFy_wrt[7]);
    fprintf(VorviewFlt," CY_p:         %15f \n",dCFy_wrt[4]);
    fprintf(VorviewFlt," CY_q:         %15f \n",dCFy_wrt[5]);
    fprintf(VorviewFlt," CY_r:         %15f \n",dCFy_wrt[6]);
    fprintf(VorviewFlt," CY_u:         %15f \n",dCFy_wrt[8]);
    fprintf(VorviewFlt," CY_alpha_2:   %15f \n",0.);
    fprintf(VorviewFlt," CY_alpha_dot: %15f \n",0.);

    // Cl derivatives
    
    fprintf(VorviewFlt," Cl_alpha:     %15f \n",dCMl_wrt[2]);
    fprintf(VorviewFlt," Cl_beta:      %15f \n",dCMl_wrt[3]);
    fprintf(VorviewFlt," Cl_mach:      %15f \n",dCMl_wrt[7]);
    fprintf(VorviewFlt," Cl_p:         %15f \n",dCMl_wrt[4]);
    fprintf(VorviewFlt," Cl_q:         %15f \n",dCMl_wrt[5]);
    fprintf(VorviewFlt," Cl_r:         %15f \n",dCMl_wrt[6]);
    fprintf(VorviewFlt," Cl_u:         %15f \n",dCMl_wrt[8]);
    fprintf(VorviewFlt," Cl_alpha_2:   %15f \n",0.);
    fprintf(VorviewFlt," Cl_alpha_dot: %15f \n",0.);

    // Cm derivatives
    
    fprintf(VorviewFlt," Cm_alpha:     %15f \n",dCMm_wrt[2]);
    fprintf(VorviewFlt," Cm_beta:      %15f \n",dCMm_wrt[3]);
    fprintf(VorviewFlt," Cm_mach:      %15f \n",dCMm_wrt[7]);
    fprintf(VorviewFlt," Cm_p:         %15f \n",dCMm_wrt[4]);
    fprintf(VorviewFlt," Cm_q:         %15f \n",dCMm_wrt[5]);
    fprintf(VorviewFlt," Cm_r:         %15f \n",dCMm_wrt[6]);
    fprintf(VorviewFlt," Cm_u:         %15f \n",dCMm_wrt[8]);
    fprintf(VorviewFlt," Cm_alpha_2:   %15f \n",0.);
    fprintf(VorviewFlt," Cm_alpha_dot: %15f \n",0.);

    // Cn derivatives
    
    fprintf(VorviewFlt," Cn_alpha:     %15f \n",dCMn_wrt[2]);
    fprintf(VorviewFlt," Cn_beta:      %15f \n",dCMn_wrt[3]);
    fprintf(VorviewFlt," Cn_mach:      %15f \n",dCMn_wrt[7]);
    fprintf(VorviewFlt," Cn_p:         %15f \n",dCMn_wrt[4]);
    fprintf(VorviewFlt," Cn_q:         %15f \n",dCMn_wrt[5]);
    fprintf(VorviewFlt," Cn_r:         %15f \n",dCMn_wrt[6]);
    fprintf(VorviewFlt," Cn_u:         %15f \n",dCMn_wrt[8]);
    fprintf(VorviewFlt," Cn_alpha_2:   %15f \n",0.);
    fprintf(VorviewFlt," Cn_alpha_dot: %15f \n",0.);
 
    fprintf(VorviewFlt,"\n");
    fprintf(VorviewFlt,"\n");
 
}

/*##############################################################################
#                                                                              #
#                     WriteOutAdjointStabilityDerivatives                      #
#                                                                              #
##############################################################################*/

void WriteOutAdjointStabilityDerivatives(void)
{

    int i, j, p, n, Found, Case, NumberOfCases;
    double Delta, SM, X_np;
    char CaseType[2000];
    char caseTypeFormatStr[] = "%-22s +%5.3lf %-9s";
    char StabFileName[2000];

    // Calculate control group stability derivatives
    
   for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) {

       ControlSurfaceGroup_[i].pCFix_pDelta() = 0.;
       ControlSurfaceGroup_[i].pCFiy_pDelta() = 0.;
       ControlSurfaceGroup_[i].pCFiz_pDelta() = 0.;
                                         
       ControlSurfaceGroup_[i].pCMix_pDelta() = 0.;
       ControlSurfaceGroup_[i].pCMiy_pDelta() = 0.;
       ControlSurfaceGroup_[i].pCMiz_pDelta() = 0.;
                                            
       ControlSurfaceGroup_[i].pCFox_pDelta() = 0.;
       ControlSurfaceGroup_[i].pCFoy_pDelta() = 0.;
       ControlSurfaceGroup_[i].pCFoz_pDelta() = 0.;
                                             
       ControlSurfaceGroup_[i].pCMox_pDelta() = 0.;
       ControlSurfaceGroup_[i].pCMoy_pDelta() = 0.;
       ControlSurfaceGroup_[i].pCMoz_pDelta() = 0.;

       ControlSurfaceGroup_[i].pCLi_pDelta()  = 0.;
       ControlSurfaceGroup_[i].pCDi_pDelta()  = 0.;
       ControlSurfaceGroup_[i].pCSi_pDelta()  = 0.;
                                            
       ControlSurfaceGroup_[i].pCMli_pDelta() = 0.;
       ControlSurfaceGroup_[i].pCMmi_pDelta() = 0.; 
       ControlSurfaceGroup_[i].pCMni_pDelta() = 0.; 
                                           
       ControlSurfaceGroup_[i].pCLo_pDelta()  = 0.;
       ControlSurfaceGroup_[i].pCDo_pDelta()  = 0.;
       ControlSurfaceGroup_[i].pCSo_pDelta()  = 0.;
                                            
       ControlSurfaceGroup_[i].pCMlo_pDelta() = 0.;
       ControlSurfaceGroup_[i].pCMmo_pDelta() = 0.;
       ControlSurfaceGroup_[i].pCMno_pDelta() = 0.;
    
       for ( j = 1 ; j <= ControlSurfaceGroup_[i].NumberOfControlSurfaces() ; j++ ) {
         
          printf("Looking for: %s \n",ControlSurfaceGroup_[i].ControlSurface_Name(j));fflush(NULL);
          
          Found = 0;
    
          p = 1;
          
          while ( p <= VSP_VLM().VSPGeom().NumberOfControlSurfaces() && !Found ) {
             
             printf("Checking: %s \n",VSP_VLM().VSPGeom().ControlSurface(p).Name());fflush(NULL);
  
             if ( strstr(VSP_VLM().VSPGeom().ControlSurface(p).Name(), ControlSurfaceGroup_[i].ControlSurface_Name(j)) != NULL ) {
      
                Found = 1;
               
                ControlSurfaceGroup_[i].pCFix_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCFix_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCFiy_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCFiy_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCFiz_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCFiz_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                                                  
                ControlSurfaceGroup_[i].pCMix_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCMix_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMiy_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCMiy_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMiz_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCMiz_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                                                  
                ControlSurfaceGroup_[i].pCFox_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCFox_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCFoy_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCFoy_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCFoz_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCFoz_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                                                  
                ControlSurfaceGroup_[i].pCMox_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCMox_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMoy_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCMoy_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMoz_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCMoz_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 


                ControlSurfaceGroup_[i].pCLi_pDelta()  += VSP_VLM().VSPGeom().ControlSurface(p).pCLi_pDelta()  * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCDi_pDelta()  += VSP_VLM().VSPGeom().ControlSurface(p).pCDi_pDelta()  * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCSi_pDelta()  += VSP_VLM().VSPGeom().ControlSurface(p).pCSi_pDelta()  * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                                                  
                ControlSurfaceGroup_[i].pCMli_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCMli_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMmi_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCMmi_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMni_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCMni_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                                                  
                ControlSurfaceGroup_[i].pCLo_pDelta()  += VSP_VLM().VSPGeom().ControlSurface(p).pCLo_pDelta()  * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCDo_pDelta()  += VSP_VLM().VSPGeom().ControlSurface(p).pCDo_pDelta()  * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCSo_pDelta()  += VSP_VLM().VSPGeom().ControlSurface(p).pCSo_pDelta()  * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                                                  
                ControlSurfaceGroup_[i].pCMlo_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCMlo_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMmo_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCMmo_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMno_pDelta() += VSP_VLM().VSPGeom().ControlSurface(p).pCMno_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 

             }
             
             p++;
            
          }
            
          if ( !Found ) {
             
             printf("Could not find control surface: %s in control surface group: %s \n",
                     ControlSurfaceGroup_[i].ControlSurface_Name(j),
                     ControlSurfaceGroup_[i].Name()); fflush(NULL);
                     
             exit(1);
             
          }
         
       }

    }
             
                     
    printf("Writing out adjoint stability derivatives... \n");fflush(NULL);
    
    // Open the stability and control output file
    
    snprintf(StabFileName,sizeof(StabFileName)*sizeof(char),"%s.adjoint.stab",FileName);

    if ( (StabFile = fopen(StabFileName,"w")) == NULL ) {

       printf("Could not open the adjoint stability and control file for output! \n");

       exit(1);

    }

    // Write out generic header file
        
    VSP_VLM().WriteCaseHeader(StabFile);
    
    if ( VSP_VLM().AdjointSolutionForceType() == ADJOINT_INVISCID_FORCES             ) NumberOfCases = 1;
    if ( VSP_VLM().AdjointSolutionForceType() == ADJOINT_VISCOUS_FORCES              ) NumberOfCases = 1;
    if ( VSP_VLM().AdjointSolutionForceType() == ADJOINT_TOTAL_FORCES                ) NumberOfCases = 1;
    if ( VSP_VLM().AdjointSolutionForceType() == ADJOINT_INVISCID_AND_VISCOUS_FORCES ) NumberOfCases = 3;
       
    for ( Case = 1 ; Case <= NumberOfCases ; Case++ ) {
           
       // Write out column labels
       
       fprintf(StabFile,"#\n");

       // Total forces header
        
       if (   VSP_VLM().AdjointSolutionForceType() == ADJOINT_TOTAL_FORCES                               ||
            ( VSP_VLM().AdjointSolutionForceType() == ADJOINT_INVISCID_AND_VISCOUS_FORCES && Case == 1 )    ) {
   
                         // 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012          
          fprintf(StabFile,"#    Total        Total        Total        Total        Total        Total        Total        Total        Total        Total        Total        Total \n");
          fprintf(StabFile,"#     CFxt         CFyt         CFzt         CMxt         CMyt         CMzt         CLt          CDt          CSt          CMlt         CMmt         CMnt\n");
          fprintf(StabFile,"#\n");

          fprintf(StabFile,"# %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f \n",
                  VSP_VLM().CFix() + VSP_VLM().CFox(),
                  VSP_VLM().CFiy() + VSP_VLM().CFoy(),
                  VSP_VLM().CFiz() + VSP_VLM().CFoz(),
                         
                  VSP_VLM().CMix() + VSP_VLM().CMox(),
                  VSP_VLM().CMiy() + VSP_VLM().CMoy(),
                  VSP_VLM().CMiz() + VSP_VLM().CMoz(),
                         
                  VSP_VLM().CLi() + VSP_VLM().CLo(),
                  VSP_VLM().CDi() + VSP_VLM().CDo(),
                  VSP_VLM().CSi() + VSP_VLM().CSo(),
                  
                  VSP_VLM().CMli() + VSP_VLM().CMlo(),
                  VSP_VLM().CMmi() + VSP_VLM().CMmo(),
                  VSP_VLM().CMni() + VSP_VLM().CMno());     

          printf("\n");
         
          fprintf(StabFile,"#\n");


    
    
    
    
    



         
          fprintf(StabFile,"#\n");
          
          //                        123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012           
          fprintf(StabFile,"#             Base    Derivative:                                                                               "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"             ");     fprintf(StabFile,"\n");
          fprintf(StabFile,"#             Aero         wrt          wrt          wrt          wrt          wrt          wrt          wrt    "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      wrt    ");     fprintf(StabFile,"\n");
          fprintf(StabFile,"Coef          Total        Alpha        Beta          p            q            r           Mach         U      "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"  ConGrp_%-3d",n);   fprintf(StabFile,"\n");
          fprintf(StabFile,"#              -           per          per          per          per          per          per          per    "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      per    ");     fprintf(StabFile,"\n");
          fprintf(StabFile,"#              -           rad          rad          rad          rad          rad          M            u      "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      rad    ");     fprintf(StabFile,"\n");
         
          fprintf(StabFile,"#\n");
         
          fprintf(StabFile,"CFxt   "); PRINT_STAB_LINE( VSP_VLM().CFtx(),
                                                       VSP_VLM().pCFtx_pAlpha(),
                                                       VSP_VLM().pCFtx_pBeta(),
                                                       VSP_VLM().pCFtx_pP(),
                                                       VSP_VLM().pCFtx_pQ(),
                                                       VSP_VLM().pCFtx_pR(),
                                                       VSP_VLM().pCFtx_pMach(),
                                                       VSP_VLM().pCFtx_pMach()*VSP_VLM().Mach());
          
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFtx_pDelta()); }; fprintf(StabFile,"\n");
             
          fprintf(StabFile,"CFyt   "); PRINT_STAB_LINE( VSP_VLM().CFty(),
                                                       VSP_VLM().pCFty_pAlpha(),
                                                       VSP_VLM().pCFty_pBeta(),
                                                       VSP_VLM().pCFty_pP(),
                                                       VSP_VLM().pCFty_pQ(),
                                                       VSP_VLM().pCFty_pR(),
                                                       VSP_VLM().pCFty_pMach(),
                                                       VSP_VLM().pCFty_pMach()*VSP_VLM().Mach());
         

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) {fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFty_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CFzt   "); PRINT_STAB_LINE( VSP_VLM().CFtz(),
                                                       VSP_VLM().pCFtz_pAlpha(),
                                                       VSP_VLM().pCFtz_pBeta(),
                                                       VSP_VLM().pCFtz_pP(),
                                                       VSP_VLM().pCFtz_pQ(),
                                                       VSP_VLM().pCFtz_pR(),
                                                       VSP_VLM().pCFtz_pMach(),
                                                       VSP_VLM().pCFtz_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFtz_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMxt   "); PRINT_STAB_LINE( VSP_VLM().CMtx(),
                                                       VSP_VLM().pCMtx_pAlpha(),
                                                       VSP_VLM().pCMtx_pBeta(),
                                                       VSP_VLM().pCMtx_pP(),
                                                       VSP_VLM().pCMtx_pQ(),
                                                       VSP_VLM().pCMtx_pR(),
                                                       VSP_VLM().pCMtx_pMach(),
                                                       VSP_VLM().pCMtx_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMtx_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMyt   "); PRINT_STAB_LINE( VSP_VLM().CMty(),
                                                       VSP_VLM().pCMty_pAlpha(),
                                                       VSP_VLM().pCMty_pBeta(),
                                                       VSP_VLM().pCMty_pP(),
                                                       VSP_VLM().pCMty_pQ(),
                                                       VSP_VLM().pCMty_pR(),
                                                       VSP_VLM().pCMty_pMach(),
                                                       VSP_VLM().pCMty_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMty_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMzt   "); PRINT_STAB_LINE( VSP_VLM().CMtz(),
                                                       VSP_VLM().pCMtz_pAlpha(),
                                                       VSP_VLM().pCMtz_pBeta(),
                                                       VSP_VLM().pCMtz_pP(),
                                                       VSP_VLM().pCMtz_pQ(),
                                                       VSP_VLM().pCMtz_pR(),
                                                       VSP_VLM().pCMtz_pMach(),
                                                       VSP_VLM().pCMtz_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMtz_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CLt    "); PRINT_STAB_LINE( VSP_VLM().CLt(),
                                                       VSP_VLM().pCLt_pAlpha(),
                                                       VSP_VLM().pCLt_pBeta(),
                                                       VSP_VLM().pCLt_pP(),
                                                       VSP_VLM().pCLt_pQ(),
                                                       VSP_VLM().pCLt_pR(),
                                                       VSP_VLM().pCLt_pMach(),
                                                       VSP_VLM().pCLt_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCLt_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CDt    "); PRINT_STAB_LINE( VSP_VLM().CDt(),
                                                       VSP_VLM().pCDt_pAlpha(),
                                                       VSP_VLM().pCDt_pBeta(),
                                                       VSP_VLM().pCDt_pP(),
                                                       VSP_VLM().pCDt_pQ(),
                                                       VSP_VLM().pCDt_pR(),
                                                       VSP_VLM().pCDt_pMach(),
                                                       VSP_VLM().pCDt_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCDt_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CSt    "); PRINT_STAB_LINE( VSP_VLM().CSt(),
                                                       VSP_VLM().pCSt_pAlpha(),
                                                       VSP_VLM().pCSt_pBeta(),
                                                       VSP_VLM().pCSt_pP(),
                                                       VSP_VLM().pCSt_pQ(),
                                                       VSP_VLM().pCSt_pR(),
                                                       VSP_VLM().pCSt_pMach(),
                                                       VSP_VLM().pCSt_pMach()*VSP_VLM().Mach());
         

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCSt_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMLt   "); PRINT_STAB_LINE( VSP_VLM().CMlt(),
                                                       VSP_VLM().pCMlt_pAlpha(),
                                                       VSP_VLM().pCMlt_pBeta(),
                                                       VSP_VLM().pCMlt_pP(),
                                                       VSP_VLM().pCMlt_pQ(),
                                                       VSP_VLM().pCMlt_pR(),
                                                       VSP_VLM().pCMlt_pMach(),
                                                       VSP_VLM().pCMlt_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMlt_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMmt   "); PRINT_STAB_LINE( VSP_VLM().CMmt(),
                                                       VSP_VLM().pCMmt_pAlpha(),
                                                       VSP_VLM().pCMmt_pBeta(),
                                                       VSP_VLM().pCMmt_pP(),
                                                       VSP_VLM().pCMmt_pQ(),
                                                       VSP_VLM().pCMmt_pR(),
                                                       VSP_VLM().pCMmt_pMach(),
                                                       VSP_VLM().pCMmt_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMmt_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMnt   "); PRINT_STAB_LINE( VSP_VLM().CMnt(),
                                                       VSP_VLM().pCMnt_pAlpha(),
                                                       VSP_VLM().pCMnt_pBeta(),
                                                       VSP_VLM().pCMnt_pP(),
                                                       VSP_VLM().pCMnt_pQ(),
                                                       VSP_VLM().pCMnt_pR(),
                                                       VSP_VLM().pCMnt_pMach(),
                                                       VSP_VLM().pCMnt_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMnt_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"#\n");
          fprintf(StabFile,"#\n");
          fprintf(StabFile,"#\n");
                                
       }
   
       // Inviscid forces header
       
       if (   VSP_VLM().AdjointSolutionForceType() == ADJOINT_INVISCID_FORCES                            ||
            ( VSP_VLM().AdjointSolutionForceType() == ADJOINT_INVISCID_AND_VISCOUS_FORCES && Case == 2 )    ) {
   
                         // 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012          
          fprintf(StabFile,"#   Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid \n");
          fprintf(StabFile,"#     CFxi         CFyi         CFzi         CMxi         CMyi         CMzi         CLi          CDi          CSi          CMli         CMmi         CMni\n");
          fprintf(StabFile,"#\n");

          fprintf(StabFile,"# %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f \n",
                  VSP_VLM().CFix(),
                  VSP_VLM().CFiy(),
                  VSP_VLM().CFiz(),
                         
                  VSP_VLM().CMix(),
                  VSP_VLM().CMiy(),
                  VSP_VLM().CMiz(),
                         
                  VSP_VLM().CLi(),
                  VSP_VLM().CDi(),
                  VSP_VLM().CSi(),
                  
                  VSP_VLM().CMli(),
                  VSP_VLM().CMmi(),
                  VSP_VLM().CMni());     

          printf("\n");
         
          fprintf(StabFile,"#\n");
         
          fprintf(StabFile,"#\n");
          
          //                        123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012           
          fprintf(StabFile,"#             Base    Derivative:                                                                               "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"             ");     fprintf(StabFile,"\n");
          fprintf(StabFile,"#             Aero         wrt          wrt          wrt          wrt          wrt          wrt          wrt    "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      wrt    ");     fprintf(StabFile,"\n");
          fprintf(StabFile,"Coef          Inviscid     Alpha        Beta          p            q            r           Mach         U      "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"  ConGrp_%-3d",n);   fprintf(StabFile,"\n");
          fprintf(StabFile,"#              -           per          per          per          per          per          per          per    "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      per    ");     fprintf(StabFile,"\n");
          fprintf(StabFile,"#              -           rad          rad          rad          rad          rad          M            u      "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      rad    ");     fprintf(StabFile,"\n");
         
          fprintf(StabFile,"#\n");
         
          fprintf(StabFile,"CFxi   "); PRINT_STAB_LINE( VSP_VLM().CFix(),
                                                       VSP_VLM().pCFix_pAlpha(),
                                                       VSP_VLM().pCFix_pBeta(),
                                                       VSP_VLM().pCFix_pP(),
                                                       VSP_VLM().pCFix_pQ(),
                                                       VSP_VLM().pCFix_pR(),
                                                       VSP_VLM().pCFix_pMach(),
                                                       VSP_VLM().pCFix_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFix_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CFyi   "); PRINT_STAB_LINE( VSP_VLM().CFiy(),
                                                       VSP_VLM().pCFiy_pAlpha(),
                                                       VSP_VLM().pCFiy_pBeta(),
                                                       VSP_VLM().pCFiy_pP(),
                                                       VSP_VLM().pCFiy_pQ(),
                                                       VSP_VLM().pCFiy_pR(),
                                                       VSP_VLM().pCFiy_pMach(),
                                                       VSP_VLM().pCFiy_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFiy_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CFzi   "); PRINT_STAB_LINE( VSP_VLM().CFiz(),
                                                       VSP_VLM().pCFiz_pAlpha(),
                                                       VSP_VLM().pCFiz_pBeta(),
                                                       VSP_VLM().pCFiz_pP(),
                                                       VSP_VLM().pCFiz_pQ(),
                                                       VSP_VLM().pCFiz_pR(),
                                                       VSP_VLM().pCFiz_pMach(),
                                                       VSP_VLM().pCFiz_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFiz_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMxi   "); PRINT_STAB_LINE( VSP_VLM().CMix(),
                                                       VSP_VLM().pCMix_pAlpha(),
                                                       VSP_VLM().pCMix_pBeta(),
                                                       VSP_VLM().pCMix_pP(),
                                                       VSP_VLM().pCMix_pQ(),
                                                       VSP_VLM().pCMix_pR(),
                                                       VSP_VLM().pCMix_pMach(),
                                                       VSP_VLM().pCMix_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMix_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMyi   "); PRINT_STAB_LINE( VSP_VLM().CMiy(),
                                                       VSP_VLM().pCMiy_pAlpha(),
                                                       VSP_VLM().pCMiy_pBeta(),
                                                       VSP_VLM().pCMiy_pP(),
                                                       VSP_VLM().pCMiy_pQ(),
                                                       VSP_VLM().pCMiy_pR(),
                                                       VSP_VLM().pCMiy_pMach(),
                                                       VSP_VLM().pCMiy_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMiy_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMzi   "); PRINT_STAB_LINE( VSP_VLM().CMiz(),
                                                       VSP_VLM().pCMiz_pAlpha(),
                                                       VSP_VLM().pCMiz_pBeta(),
                                                       VSP_VLM().pCMiz_pP(),
                                                       VSP_VLM().pCMiz_pQ(),
                                                       VSP_VLM().pCMiz_pR(),
                                                       VSP_VLM().pCMiz_pMach(),
                                                       VSP_VLM().pCMiz_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMiz_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CLi    "); PRINT_STAB_LINE( VSP_VLM().CLi(),
                                                       VSP_VLM().pCLi_pAlpha(),
                                                       VSP_VLM().pCLi_pBeta(),
                                                       VSP_VLM().pCLi_pP(),
                                                       VSP_VLM().pCLi_pQ(),
                                                       VSP_VLM().pCLi_pR(),
                                                       VSP_VLM().pCLi_pMach(),
                                                       VSP_VLM().pCLi_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCLi_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CDi    "); PRINT_STAB_LINE( VSP_VLM().CDi(),
                                                       VSP_VLM().pCDi_pAlpha(),
                                                       VSP_VLM().pCDi_pBeta(),
                                                       VSP_VLM().pCDi_pP(),
                                                       VSP_VLM().pCDi_pQ(),
                                                       VSP_VLM().pCDi_pR(),
                                                       VSP_VLM().pCDi_pMach(),
                                                       VSP_VLM().pCDi_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCDi_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CSi    "); PRINT_STAB_LINE( VSP_VLM().CSi(),
                                                       VSP_VLM().pCSi_pAlpha(),
                                                       VSP_VLM().pCSi_pBeta(),
                                                       VSP_VLM().pCSi_pP(),
                                                       VSP_VLM().pCSi_pQ(),
                                                       VSP_VLM().pCSi_pR(),
                                                       VSP_VLM().pCSi_pMach(),
                                                       VSP_VLM().pCSi_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCSi_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMLi   "); PRINT_STAB_LINE( VSP_VLM().CMli(),
                                                       VSP_VLM().pCMli_pAlpha(),
                                                       VSP_VLM().pCMli_pBeta(),
                                                       VSP_VLM().pCMli_pP(),
                                                       VSP_VLM().pCMli_pQ(),
                                                       VSP_VLM().pCMli_pR(),
                                                       VSP_VLM().pCMli_pMach(),
                                                       VSP_VLM().pCMli_pMach()*VSP_VLM().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMli_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMmi   "); PRINT_STAB_LINE( VSP_VLM().CMmi(),
                                                       VSP_VLM().pCMmi_pAlpha(),
                                                       VSP_VLM().pCMmi_pBeta(),
                                                       VSP_VLM().pCMmi_pP(),
                                                       VSP_VLM().pCMmi_pQ(),
                                                       VSP_VLM().pCMmi_pR(),
                                                       VSP_VLM().pCMmi_pMach(),
                                                       VSP_VLM().pCMmi_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMmi_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMni   "); PRINT_STAB_LINE( VSP_VLM().CMni(),
                                                       VSP_VLM().pCMni_pAlpha(),
                                                       VSP_VLM().pCMni_pBeta(),
                                                       VSP_VLM().pCMni_pP(),
                                                       VSP_VLM().pCMni_pQ(),
                                                       VSP_VLM().pCMni_pR(),
                                                       VSP_VLM().pCMni_pMach(),
                                                       VSP_VLM().pCMni_pMach()*VSP_VLM().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMni_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"#\n");
          fprintf(StabFile,"#\n");
          fprintf(StabFile,"#\n");
                                      
       }
   
   
       // Inviscid forces header
       
       if (   VSP_VLM().AdjointSolutionForceType() == ADJOINT_VISCOUS_FORCES                             ||
            ( VSP_VLM().AdjointSolutionForceType() == ADJOINT_INVISCID_AND_VISCOUS_FORCES && Case == 3 )    ) {
   
                         // 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012          
          fprintf(StabFile,"#    Viscous      Viscous      Viscous      Viscous      Viscous      Viscous      Viscous      Viscous      Viscous      Viscous      Viscous      Viscous \n");
          fprintf(StabFile,"#     CFxo         CFyo         CFzo         CMxo         CMyo         CMzo         CLo          CDo          CSo          CMlo         CMmo         CMno\n");
          fprintf(StabFile,"#\n");

          fprintf(StabFile,"# %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f \n",
                  VSP_VLM().CFox(),
                  VSP_VLM().CFoy(),
                  VSP_VLM().CFoz(),
                  
                  VSP_VLM().CMox(),
                  VSP_VLM().CMoy(),
                  VSP_VLM().CMoz(),
                         
                  VSP_VLM().CLo(),
                  VSP_VLM().CDo(),
                  VSP_VLM().CSo(),
                  
                  VSP_VLM().CMlo(),
                  VSP_VLM().CMmo(),
                  VSP_VLM().CMno());     

          printf("\n");
         
          fprintf(StabFile,"#\n");
         
          fprintf(StabFile,"#\n");
          
          //                        123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012           
          fprintf(StabFile,"#             Base    Derivative:                                                                               "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"             ");     fprintf(StabFile,"\n");
          fprintf(StabFile,"#             Aero         wrt          wrt          wrt          wrt          wrt          wrt          wrt    "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      wrt    ");     fprintf(StabFile,"\n");
          fprintf(StabFile,"Coef          Viscous      Alpha        Beta          p            q            r           Mach         U      "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"  ConGrp_%-3d",n);   fprintf(StabFile,"\n");
          fprintf(StabFile,"#              -           per          per          per          per          per          per          per    "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      per    ");     fprintf(StabFile,"\n");
          fprintf(StabFile,"#              -           rad          rad          rad          rad          rad          M            u      "); for ( n = 1 ; n <= NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      rad    ");     fprintf(StabFile,"\n");
         
          fprintf(StabFile,"#\n");
         
          fprintf(StabFile,"CFxo   "); PRINT_STAB_LINE( VSP_VLM().CFox(),
                                                       VSP_VLM().pCFox_pAlpha(),
                                                       VSP_VLM().pCFox_pBeta(),
                                                       VSP_VLM().pCFox_pP(),
                                                       VSP_VLM().pCFox_pQ(),
                                                       VSP_VLM().pCFox_pR(),
                                                       VSP_VLM().pCFox_pMach(),
                                                       VSP_VLM().pCFox_pMach()*VSP_VLM().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFox_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CFyo   "); PRINT_STAB_LINE( VSP_VLM().CFoy(),
                                                       VSP_VLM().pCFoy_pAlpha(),
                                                       VSP_VLM().pCFoy_pBeta(),
                                                       VSP_VLM().pCFoy_pP(),
                                                       VSP_VLM().pCFoy_pQ(),
                                                       VSP_VLM().pCFoy_pR(),
                                                       VSP_VLM().pCFoy_pMach(),
                                                       VSP_VLM().pCFoy_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFoy_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CFzo   "); PRINT_STAB_LINE( VSP_VLM().CFoz(),
                                                       VSP_VLM().pCFoz_pAlpha(),
                                                       VSP_VLM().pCFoz_pBeta(),
                                                       VSP_VLM().pCFoz_pP(),
                                                       VSP_VLM().pCFoz_pQ(),
                                                       VSP_VLM().pCFoz_pR(),
                                                       VSP_VLM().pCFoz_pMach(),
                                                       VSP_VLM().pCFoz_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFoz_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMxo   "); PRINT_STAB_LINE( VSP_VLM().CMox(),
                                                       VSP_VLM().pCMox_pAlpha(),
                                                       VSP_VLM().pCMox_pBeta(),
                                                       VSP_VLM().pCMox_pP(),
                                                       VSP_VLM().pCMox_pQ(),
                                                       VSP_VLM().pCMox_pR(),
                                                       VSP_VLM().pCMox_pMach(),
                                                       VSP_VLM().pCMox_pMach()*VSP_VLM().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMox_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMyo   "); PRINT_STAB_LINE( VSP_VLM().CMoy(),
                                                       VSP_VLM().pCMoy_pAlpha(),
                                                       VSP_VLM().pCMoy_pBeta(),
                                                       VSP_VLM().pCMoy_pP(),
                                                       VSP_VLM().pCMoy_pQ(),
                                                       VSP_VLM().pCMoy_pR(),
                                                       VSP_VLM().pCMoy_pMach(),
                                                       VSP_VLM().pCMoy_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMoy_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMzo   "); PRINT_STAB_LINE( VSP_VLM().CMoz(),
                                                       VSP_VLM().pCMoz_pAlpha(),
                                                       VSP_VLM().pCMoz_pBeta(),
                                                       VSP_VLM().pCMoz_pP(),
                                                       VSP_VLM().pCMoz_pQ(),
                                                       VSP_VLM().pCMoz_pR(),
                                                       VSP_VLM().pCMoz_pMach(),
                                                       VSP_VLM().pCMoz_pMach()*VSP_VLM().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMoz_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CLo    "); PRINT_STAB_LINE( VSP_VLM().CLo(),
                                                       VSP_VLM().pCLo_pAlpha(),
                                                       VSP_VLM().pCLo_pBeta(),
                                                       VSP_VLM().pCLo_pP(),
                                                       VSP_VLM().pCLo_pQ(),
                                                       VSP_VLM().pCLo_pR(),
                                                       VSP_VLM().pCLo_pMach(),
                                                       VSP_VLM().pCLo_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCLo_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CDo    "); PRINT_STAB_LINE( VSP_VLM().CDo(),
                                                       VSP_VLM().pCDo_pAlpha(),
                                                       VSP_VLM().pCDo_pBeta(),
                                                       VSP_VLM().pCDo_pP(),
                                                       VSP_VLM().pCDo_pQ(),
                                                       VSP_VLM().pCDo_pR(),
                                                       VSP_VLM().pCDo_pMach(),
                                                       VSP_VLM().pCDo_pMach()*VSP_VLM().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCDo_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CSo    "); PRINT_STAB_LINE( VSP_VLM().CSo(),
                                                       VSP_VLM().pCSo_pAlpha(),
                                                       VSP_VLM().pCSo_pBeta(),
                                                       VSP_VLM().pCSo_pP(),
                                                       VSP_VLM().pCSo_pQ(),
                                                       VSP_VLM().pCSo_pR(),
                                                       VSP_VLM().pCSo_pMach(),
                                                       VSP_VLM().pCSo_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCSo_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMLo   "); PRINT_STAB_LINE( VSP_VLM().CMlo(),
                                                       VSP_VLM().pCMlo_pAlpha(),
                                                       VSP_VLM().pCMlo_pBeta(),
                                                       VSP_VLM().pCMlo_pP(),
                                                       VSP_VLM().pCMlo_pQ(),
                                                       VSP_VLM().pCMlo_pR(),
                                                       VSP_VLM().pCMlo_pMach(),
                                                       VSP_VLM().pCMlo_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMlo_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMmo   "); PRINT_STAB_LINE( VSP_VLM().CMmo(),
                                                       VSP_VLM().pCMmo_pAlpha(),
                                                       VSP_VLM().pCMmo_pBeta(),
                                                       VSP_VLM().pCMmo_pP(),
                                                       VSP_VLM().pCMmo_pQ(),
                                                       VSP_VLM().pCMmo_pR(),
                                                       VSP_VLM().pCMmo_pMach(),
                                                       VSP_VLM().pCMmo_pMach()*VSP_VLM().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMmo_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMno   "); PRINT_STAB_LINE( VSP_VLM().CMno(),
                                                       VSP_VLM().pCMno_pAlpha(),
                                                       VSP_VLM().pCMno_pBeta(),
                                                       VSP_VLM().pCMno_pP(),
                                                       VSP_VLM().pCMno_pQ(),
                                                       VSP_VLM().pCMno_pR(),
                                                       VSP_VLM().pCMno_pMach(),
                                                       VSP_VLM().pCMno_pMach()*VSP_VLM().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMno_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"#\n");
          fprintf(StabFile,"#\n");
          fprintf(StabFile,"#\n");
                                      
       }        

    }
    
    // Calculate static margin and neutral point and write them out
    
    char headerFormatStr[] = "%-20s %12s %-20s\n";
    char dataFormatStr[] = "%-20s %12.7lf %-20s\n";

    fprintf( StabFile, headerFormatStr, "# Result", "Value   ", "  Units" );

    SM = -1.0 * VSP_VLM().pCMmt_pAlpha() / VSP_VLM().pCLt_pAlpha(); // -1 * CMm_alpha / CL_alpha
    X_np = SM * Cref_ + Xcg_;

    fprintf( StabFile, dataFormatStr, "SM", SM, "no_unit" );
    fprintf( StabFile, dataFormatStr, "X_np", X_np, "Lunit" );

    fprintf( StabFile, "#\n" );
    fprintf( StabFile, "#\n" );
    fprintf( StabFile, "#\n" );
    
    fflush(NULL);
    
    fclose(StabFile);

}
 
/*##############################################################################
#                                                                              #
#                               PRINT_STAB_LINE                                #
#                                                                              #
##############################################################################*/

void PRINT_STAB_LINE(double F1,
                     double F2,
                     double F3,
                     double F4,
                     double F5,
                     double F6,
                     double F7,
                     double F8)
                     
{

    fprintf(StabFile,"%12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f ",
            F1, 
            F2,
            F3,
            F4,
            F5,
            F6,
            F7,
            F8);
   
}

/*##############################################################################
#                                                                              #
#                   UnsteadyStabilityAndControlSolve                           #
#                                                                              #
##############################################################################*/

void UnsteadyStabilityAndControlSolve(void)
{

    int i, j, k, p, pm, pm1, Case, NumCases;
    char StabFileName[2000];
    double  CL_damp,  CL_avg;
    double  CD_damp,  CD_avg;
    double  CS_damp,  CS_avg;   
    double CFx_damp, CFx_avg;
    double CFy_damp, CFy_avg;
    double CFz_damp, CFz_avg;
    double CMx_damp, CMx_avg;
    double CMy_damp, CMy_avg;
    double CMz_damp, CMz_avg;     
    double Fact, T, Theta;  

    if ( StabControlRun_ == 2 ) snprintf(StabFileName,sizeof(StabFileName)*sizeof(char),"%s.pstab",FileName); // Roll analysis
                                
    if ( StabControlRun_ == 3 ) snprintf(StabFileName,sizeof(StabFileName)*sizeof(char),"%s.qstab",FileName); // Pitch analysis
                                
    if ( StabControlRun_ == 4 ) snprintf(StabFileName,sizeof(StabFileName)*sizeof(char),"%s.rstab",FileName); // Yaw analysis

    if ( (StabFile = fopen(StabFileName,"w")) == NULL ) {

       printf("Could not open the stability and control file for output! \n");

       exit(1);

    }
    
    NumCases = NumberOfBetas_ * NumberOfMachs_ * NumberOfAoAs_;
       
    Case = 0;
    
    // Loop over all the cases
       
    for ( i = 1 ; i <= NumberOfBetas_ ; i++ ) {
       
       for ( j = 1 ; j <= NumberOfMachs_; j++ ) {
             
          for ( k = 1 ; k <= NumberOfAoAs_ ; k++ ) {
             
             Case++;
             
             // Set free stream conditions
             
             VSP_VLM().AngleOfBeta()   = BetaList_[i] * TORAD;
             VSP_VLM().Mach()          = MachList_[j];  
             VSP_VLM().AngleOfAttack() =  AoAList_[k] * TORAD;
      
             VSP_VLM().RotationalRate_p() = 0.;
             VSP_VLM().RotationalRate_q() = 0.;
             VSP_VLM().RotationalRate_r() = 0.;

             // Set Control surface group deflection to un-perturbed control surface deflections

             ApplyControlDeflections();
             
             // Write out generic header file
                 
             VSP_VLM().WriteCaseHeader(StabFile);

             // Unsteady solve
             
             if ( Case <= NumCases ) {
            
                VSP_VLM().Solve(Case);
                
             }
             
             else {
                
                VSP_VLM().Solve(-Case);
                
             }
                          
             // Calculate the damping derivatives

// Old method     
    
       //       CL_damp = (  VSP_VLM().CL_Unsteady(16) -  VSP_VLM().CL_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
       //       CD_damp = (  VSP_VLM().CD_Unsteady(16) -  VSP_VLM().CD_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
       //       CS_damp = (  VSP_VLM().CS_Unsteady(16) -  VSP_VLM().CS_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );   
       //      CFx_damp = ( VSP_VLM().CFx_Unsteady(16) - VSP_VLM().CFx_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
       //      CFy_damp = ( VSP_VLM().CFy_Unsteady(16) - VSP_VLM().CFy_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
       //      CFz_damp = ( VSP_VLM().CFz_Unsteady(16) - VSP_VLM().CFz_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
       //      CMx_damp = ( VSP_VLM().CMx_Unsteady(16) - VSP_VLM().CMx_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
       //      CMy_damp = ( VSP_VLM().CMy_Unsteady(16) - VSP_VLM().CMy_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
       //      CMz_damp = ( VSP_VLM().CMz_Unsteady(16) - VSP_VLM().CMz_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );        

              CL_avg = 0.;
              CD_avg = 0.;
              CS_avg = 0.;
             CFx_avg = 0.;
             CFy_avg = 0.;
             CFz_avg = 0.;
             CMx_avg = 0.;
             CMy_avg = 0.;
             CMz_avg = 0.;

             // Sum up from T = DT, to N*DT
             
             for ( p = VSP_VLM().NumberOfTimeSteps()/2 ; p <= VSP_VLM().NumberOfTimeSteps() ; p++ ) {
         
                 CL_avg += VSP_VLM().VSPGeom().ComponentGroupList(1).CLi(p);
                 CD_avg += VSP_VLM().VSPGeom().ComponentGroupList(1).CDi(p);
                 CS_avg += VSP_VLM().VSPGeom().ComponentGroupList(1).CSi(p);
                 
                CFx_avg += VSP_VLM().VSPGeom().ComponentGroupList(1).CFix(p);
                CFy_avg += VSP_VLM().VSPGeom().ComponentGroupList(1).CFiy(p);
                CFz_avg += VSP_VLM().VSPGeom().ComponentGroupList(1).CFiz(p);
                CMx_avg += VSP_VLM().VSPGeom().ComponentGroupList(1).CMix(p);
                CMy_avg += VSP_VLM().VSPGeom().ComponentGroupList(1).CMiy(p);
                CMz_avg += VSP_VLM().VSPGeom().ComponentGroupList(1).CMiz(p);
            
             }
             
             // Calculate average
             
              CL_avg /= (VSP_VLM().NumberOfTimeSteps()/2 + 1);
              CD_avg /= (VSP_VLM().NumberOfTimeSteps()/2 + 1);
              CS_avg /= (VSP_VLM().NumberOfTimeSteps()/2 + 1);
              
             CFx_avg /= (VSP_VLM().NumberOfTimeSteps()/2 + 1);
             CFy_avg /= (VSP_VLM().NumberOfTimeSteps()/2 + 1);
             CFz_avg /= (VSP_VLM().NumberOfTimeSteps()/2 + 1);
             CMx_avg /= (VSP_VLM().NumberOfTimeSteps()/2 + 1);
             CMy_avg /= (VSP_VLM().NumberOfTimeSteps()/2 + 1);
             CMz_avg /= (VSP_VLM().NumberOfTimeSteps()/2 + 1);
             
             printf("CMy_avg: %f \n",CMy_avg);
             
             // Integrate deltas
              
              CL_damp = 0.;
              CD_damp = 0.;
              CS_damp = 0.;             
             CFx_damp = 0.;             
             CFy_damp = 0.;             
             CFz_damp = 0.;             
             CMx_damp = 0.;             
             CMy_damp = 0.;             
             CMz_damp = 0.;             
             
             for ( p = VSP_VLM().NumberOfTimeSteps()/2 + 1 ; p <= VSP_VLM().NumberOfTimeSteps() ; p++ ) {
         
                 pm  = p;
                 pm1 = p - 1;
                 
                 if ( p == VSP_VLM().NumberOfTimeSteps() ) pm1 = VSP_VLM().NumberOfTimeSteps()/2;
                 
                 T = VSP_VLM().DeltaTime() * 0.5 * (double) (2*(p-1) + 1);
                 
                 Theta = VSP_VLM().Unsteady_AngleRate() * T;

                 CL_damp += 2.*( 0.5*(  VSP_VLM().VSPGeom().ComponentGroupList(1).CLi(pm1) +  VSP_VLM().VSPGeom().ComponentGroupList(1).CLi(pm) ) -  CL_avg ) * cos( Theta ) * VSP_VLM().DeltaTime();
                 CD_damp += 2.*( 0.5*(  VSP_VLM().VSPGeom().ComponentGroupList(1).CDi(pm1) +  VSP_VLM().VSPGeom().ComponentGroupList(1).CDi(pm) ) -  CD_avg ) * cos( Theta ) * VSP_VLM().DeltaTime();
                 CS_damp += 2.*( 0.5*(  VSP_VLM().VSPGeom().ComponentGroupList(1).CSi(pm1) +  VSP_VLM().VSPGeom().ComponentGroupList(1).CSi(pm) ) -  CS_avg ) * cos( Theta ) * VSP_VLM().DeltaTime();
                                                                                                    
                CFx_damp += 2.*( 0.5*(  VSP_VLM().VSPGeom().ComponentGroupList(1).CFix(pm1) + VSP_VLM().VSPGeom().ComponentGroupList(1).CFix(pm) ) - CFx_avg ) * cos( Theta ) * VSP_VLM().DeltaTime();
                CFy_damp += 2.*( 0.5*(  VSP_VLM().VSPGeom().ComponentGroupList(1).CFiy(pm1) + VSP_VLM().VSPGeom().ComponentGroupList(1).CFiy(pm) ) - CFy_avg ) * cos( Theta ) * VSP_VLM().DeltaTime();
                CFz_damp += 2.*( 0.5*(  VSP_VLM().VSPGeom().ComponentGroupList(1).CFiz(pm1) + VSP_VLM().VSPGeom().ComponentGroupList(1).CFiz(pm) ) - CFz_avg ) * cos( Theta ) * VSP_VLM().DeltaTime();
                CMx_damp += 2.*( 0.5*(  VSP_VLM().VSPGeom().ComponentGroupList(1).CMix(pm1) + VSP_VLM().VSPGeom().ComponentGroupList(1).CMix(pm) ) - CMx_avg ) * cos( Theta ) * VSP_VLM().DeltaTime();
                CMy_damp += 2.*( 0.5*(  VSP_VLM().VSPGeom().ComponentGroupList(1).CMiy(pm1) + VSP_VLM().VSPGeom().ComponentGroupList(1).CMiy(pm) ) - CMy_avg ) * cos( Theta ) * VSP_VLM().DeltaTime();
                CMz_damp += 2.*( 0.5*(  VSP_VLM().VSPGeom().ComponentGroupList(1).CMiz(pm1) + VSP_VLM().VSPGeom().ComponentGroupList(1).CMiz(pm) ) - CMz_avg ) * cos( Theta ) * VSP_VLM().DeltaTime();

             }
                  
             Fact = 0.5 * VSP_VLM().DeltaTime() * VSP_VLM().NumberOfTimeSteps() * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180.;
              
              CL_damp /= Fact;    
              CD_damp /= Fact;    
              CS_damp /= Fact;    
             CFx_damp /= Fact;    
             CFy_damp /= Fact;    
             CFz_damp /= Fact;    
             CMx_damp /= Fact;    
             CMy_damp /= Fact;    
             CMz_damp /= Fact;    

             if ( StabControlRun_ != 1 )
             {
                 fprintf(StabFile, "# Name \t\t Value \n");
             }
                 
             // Roll analysis
             
             if ( StabControlRun_ == 2 ) {
         
                fprintf(StabFile,"CFx_p \t\t %12.7f \n", CFx_damp);
                fprintf(StabFile,"CFy_p \t\t %12.7f \n", CFy_damp);
                fprintf(StabFile,"CFz_p \t\t %12.7f \n", CFz_damp);
            
                fprintf(StabFile,"CMx_p \t\t %12.7f \n", CMx_damp);
                fprintf(StabFile,"CMy_p \t\t %12.7f \n", CMy_damp);
                fprintf(StabFile,"CMz_p \t\t %12.7f \n", CMz_damp);
            
                fprintf(StabFile," CL_p \t\t %12.7f \n", CL_damp);
                fprintf(StabFile," CD_p \t\t %12.7f \n", CD_damp);
                fprintf(StabFile," CS_p \t\t %12.7f \n", CS_damp);
          
                fprintf(StabFile,"CMl_p \t\t %12.7f \n",-CMx_damp);
                fprintf(StabFile,"CMm_p \t\t %12.7f \n", CMy_damp);
                fprintf(StabFile,"CMn_p \t\t %12.7f \n",-CMz_damp);
                
             }
             
             // Pitch analysis
             
             if ( StabControlRun_ == 3 ) {
         
                fprintf(StabFile,"CFx_(q + alpha_dot) \t\t %12.7f \n", CFx_damp);
                fprintf(StabFile,"CFy_(q + alpha_dot) \t\t %12.7f \n", CFy_damp);
                fprintf(StabFile,"CFz_(q + alpha_dot) \t\t %12.7f \n", CFz_damp);
            
                fprintf(StabFile,"CMx_(q + alpha_dot) \t\t %12.7f \n", CMx_damp);
                fprintf(StabFile,"CMy_(q + alpha_dot) \t\t %12.7f \n", CMy_damp);
                fprintf(StabFile,"CMz_(q + alpha_dot) \t\t %12.7f \n", CMz_damp);
            
                fprintf(StabFile," CL_(q + alpha_dot) \t\t %12.7f \n", CL_damp);
                fprintf(StabFile," CD_(q + alpha_dot) \t\t %12.7f \n", CD_damp);
                fprintf(StabFile," CS_(q + alpha_dot) \t\t %12.7f \n", CS_damp);
          
                fprintf(StabFile,"CMl_(q + alpha_dot) \t\t %12.7f \n",-CMx_damp);
                fprintf(StabFile,"CMm_(q + alpha_dot) \t\t %12.7f \n", CMy_damp);
                fprintf(StabFile,"CMn_(q + alpha_dot) \t\t %12.7f \n",-CMz_damp);
                
             }
                         
             // Yaw analysis
             
             if ( StabControlRun_ == 4 ) {
         
                fprintf(StabFile,"CFx_(r - beta_dot) \t\t %12.7f \n", CFx_damp);
                fprintf(StabFile,"CFy_(r - beta_dot) \t\t %12.7f \n", CFy_damp);
                fprintf(StabFile,"CFz_(r - beta_dot) \t\t %12.7f \n", CFz_damp);
            
                fprintf(StabFile,"CMx_(r - beta_dot) \t\t %12.7f \n", CMx_damp);
                fprintf(StabFile,"CMy_(r - beta_dot) \t\t %12.7f \n", CMy_damp);
                fprintf(StabFile,"CMz_(r - beta_dot) \t\t %12.7f \n", CMz_damp);
            
                fprintf(StabFile," CL_(r - beta_dot) \t\t %12.7f \n", CL_damp);
                fprintf(StabFile," CD_(r - beta_dot) \t\t %12.7f \n", CD_damp);
                fprintf(StabFile," CS_(r - beta_dot) \t\t %12.7f \n", CS_damp);
          
                fprintf(StabFile,"CMl_(r - beta_dot) \t\t %12.7f \n",-CMx_damp);
                fprintf(StabFile,"CMm_(r - beta_dot) \t\t %12.7f \n", CMy_damp);
                fprintf(StabFile,"CMn_(r - beta_dot) \t\t %12.7f \n",-CMz_damp);
                
             }  
             
             fprintf(StabFile,"#\n");
             fprintf(StabFile,"#\n");
             fprintf(StabFile,"#\n");    
             
          }
          
       }
       
    }
    
    fclose(StabFile);
    
}    

/*##############################################################################
#                                                                              #
#                                   Noise                                      #
#                                                                              #
##############################################################################*/

void Noise(void)
{

 //  int NumCases;
 //
 //  ApplyControlDeflections();
 //  
 //  NumCases = NumberOfBetas_ * NumberOfMachs_ * NumberOfAoAs_ * NumberOfReCrefs_;
 //  
 //  if ( NumCases != 1 ) {
 //     
 //     printf("Don't know how to set up a noise case with more than one Mach-Alpha-Cref case! \n");
 //     exit(1);
 //     
 //  }
 //
 //  // Set free stream conditions
 //  
 //  VSP_VLM().AngleOfBeta()   = BetaList_[1] * TORAD;
 //  VSP_VLM().Mach()          = MachList_[1];  
 //  VSP_VLM().AngleOfAttack() =  AoAList_[1] * TORAD;
 //  
 //  VSP_VLM().RotationalRate_p() = 0.;
 //  VSP_VLM().RotationalRate_q() = 0.;
 //  VSP_VLM().RotationalRate_r() = 0.;
 //     
 //   VSP_VLM().WriteOutNoiseFiles(NumCases);

}

/*##############################################################################
#                                                                              #
#                           CalculateAerodynamicCenter                         #
#                                                                              #
##############################################################################*/

void CalculateAerodynamicCenter(void)
{

    int ic, jc, kc;
    double DeltaXcg;
    char StabFileName[2000];
    
    // Open the stability and control output file
    
    snprintf(StabFileName,sizeof(StabFileName)*sizeof(char),"%s.aerocenter.stab",FileName);

    if ( (StabFile = fopen(StabFileName,"w")) == NULL ) {

       printf("Could not open the stability and control file for output! \n");

       exit(1);

    }
     
    printf("Calculating Aerodynamic Center... \n"); 
     
    for ( ic = 1 ; ic <= NumberOfBetas_ ; ic++ ) {
       
       for ( jc = 1 ; jc <= NumberOfMachs_; jc++ ) {
             
          for ( kc = 1 ; kc <= NumberOfAoAs_ ; kc++ ) {
                  
             // Set free stream conditions
             
             Beta_ = BetaList_[ic];
             Mach_ = MachList_[jc];  
             AoA_  =  AoAList_[kc];
             
             // Solve the base case
             
             snprintf(VSP_VLM().CaseString(),sizeof(VSP_VLM().CaseString())*sizeof(char),"Base Aero         ");
                          
             VSP_VLM().Mach()          = Mach_;
             VSP_VLM().AngleOfAttack() =  AoA_ * TORAD;
             VSP_VLM().AngleOfBeta()   = Beta_ * TORAD;
      
             VSP_VLM().RotationalRate_p() = 0.;
             VSP_VLM().RotationalRate_q() = 0.;
             VSP_VLM().RotationalRate_r() = 0.;
             
             VSP_VLM().SaveRestartFile() = VSP_VLM().DoRestart() = 0;
             
             VSP_VLM().Solve(1);

             // Store aero coefficients
                
             CLForCase[1] = VSP_VLM().CLi() + VSP_VLM().CLo(); 
             CDForCase[1] = VSP_VLM().CDi() + VSP_VLM().CDo();       
             CSForCase[1] = VSP_VLM().CSi() + VSP_VLM().CSo();        
             
             CDoForCase[1] = VSP_VLM().CDo();     
         
             CDtForCase[1] = VSP_VLM().CDiw();                     
         
             CFxForCase[1] = VSP_VLM().CFix() + VSP_VLM().CFox();
             CFyForCase[1] = VSP_VLM().CFiy() + VSP_VLM().CFoy();       
             CFzForCase[1] = VSP_VLM().CFiz() + VSP_VLM().CFoz();       
                                                         
             CMxForCase[1] = VSP_VLM().CMix() + VSP_VLM().CMox();      
             CMyForCase[1] = VSP_VLM().CMiy() + VSP_VLM().CMoy();      
             CMzForCase[1] = VSP_VLM().CMiz() + VSP_VLM().CMoz();    
             
             CMlForCase[1] = -CMxForCase[1];    
             CMmForCase[1] =  CMyForCase[1];       
             CMnForCase[1] = -CMzForCase[1];     
                     
             // Solve the perturbed case
             
             snprintf(VSP_VLM().CaseString(),sizeof(VSP_VLM().CaseString())*sizeof(char),"Alpha      +%5.3lf",1.);
                   
             VSP_VLM().Mach()          = Mach_;
             VSP_VLM().AngleOfAttack() =  ( AoA_ + 1.) * TORAD;
             VSP_VLM().AngleOfBeta()   = Beta_ * TORAD;
      
             VSP_VLM().RotationalRate_p() = 0.;
             VSP_VLM().RotationalRate_q() = 0.;
             VSP_VLM().RotationalRate_r() = 0.;
                          
             VSP_VLM().SaveRestartFile() = VSP_VLM().DoRestart() = 0;
             
             VSP_VLM().Solve(-2);

             // Store aero coefficients
                
             CLForCase[2] = VSP_VLM().CLi() + VSP_VLM().CLo(); 
             CDForCase[2] = VSP_VLM().CDi() + VSP_VLM().CDo();       
             CSForCase[2] = VSP_VLM().CSi() + VSP_VLM().CSo();        
             
             CDoForCase[2] = VSP_VLM().CDo();     
         
             CDtForCase[2] = VSP_VLM().CDiw();                     
         
             CFxForCase[2] = VSP_VLM().CFix() + VSP_VLM().CFox();
             CFyForCase[2] = VSP_VLM().CFiy() + VSP_VLM().CFoy();       
             CFzForCase[2] = VSP_VLM().CFiz() + VSP_VLM().CFoz();       
                 
             CMxForCase[2] = VSP_VLM().CMix() + VSP_VLM().CMox();      
             CMyForCase[2] = VSP_VLM().CMiy() + VSP_VLM().CMoy();      
             CMzForCase[2] = VSP_VLM().CMiz() + VSP_VLM().CMoz();    
             
             CMlForCase[2] = -CMxForCase[2];    
             CMmForCase[2] =  CMyForCase[2];       
             CMnForCase[2] = -CMzForCase[2];     

             // Calculate aero center shift
             
             DeltaXcg = -( CMyForCase[2] - CMyForCase[1] ) / ( CFzForCase[2] - CFzForCase[1] ) * Cref_;
             
             VSP_VLM().WriteCaseHeader(StabFile);

             fprintf(StabFile,"Aerodynamic Center is at: (%12.7f, %12.7f, %12.7f) \n\n", 
             Xcg_ + DeltaXcg, Ycg_, Zcg_);

             printf("\n");
    
          }
          
       }
       
    }
                   
    fclose(StabFile);
    
}

/*##############################################################################
#                                                                              #
#                              FiniteDiffTestSolve                             #
#                                                                              #
##############################################################################*/

void FiniteDiffTestSolve(void)
{

    int i, j, Case, NumCases, LoadGroup;
    double Delta, *TempXYZ;
    char TestFileName[2000];
    FILE *FiniteDiffFile;

    // Open the output file for the test derivatives
    
    snprintf(TestFileName,sizeof(TestFileName)*sizeof(char),"%s.fd.gradient",FileName);

    if ( (FiniteDiffFile = fopen(TestFileName,"w")) == NULL ) {

       printf("Could not open the finite differences output file! \n");

       exit(1);

    }
    
    ApplyControlDeflections();
    
    NumCases = NumberOfBetas_ * NumberOfMachs_ * NumberOfAoAs_ * NumberOfReCrefs_;
    
    if ( NumCases > 1 ) {
       
       printf("Please choose a single Mach, AoA, Beta, and Re for complex diff test \n");
       fflush(NULL);
       exit(1);
       
    }

    // Set free stream conditions
    
    VSP_VLM().AngleOfBeta()   = BetaList_[1] * TORAD;
    VSP_VLM().Mach()          = MachList_[1];  
    VSP_VLM().AngleOfAttack() =  AoAList_[1] * TORAD;
    
    VSP_VLM().RotationalRate_p() = 0.;
    VSP_VLM().RotationalRate_q() = 0.;
    VSP_VLM().RotationalRate_r() = 0.;    
            
    // Loop over the grid nodes

    TempXYZ = new double[3*VSP_VLM().VSPGeom().Grid(0).NumberOfNodes() + 1];
    
    for ( j = 1 ; j <= VSP_VLM().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
    
       TempXYZ[3*j-2] = VSP_VLM().VSPGeom().Grid(0).NodeList(j).x();
       TempXYZ[3*j-1] = VSP_VLM().VSPGeom().Grid(0).NodeList(j).y();
       TempXYZ[3*j  ] = VSP_VLM().VSPGeom().Grid(0).NodeList(j).z();
       
    }
    
    double Fxmt, Fxpt, Fymt, Fypt, Fzmt, Fzpt;
    double Mxmt, Mxpt, Mymt, Mypt, Mzmt, Mzpt;

    double Fxmi, Fxpi, Fymi, Fypi, Fzmi, Fzpi;
    double Mxmi, Mxpi, Mymi, Mypi, Mzmi, Mzpi;
    
    double Fxmo, Fxpo, Fymo, Fypo, Fzmo, Fzpo;
    double Mxmo, Mxpo, Mymo, Mypo, Mzmo, Mzpo;
    
    double Fxmw, Fxpw, Fymw, Fypw, Fzmw, Fzpw;
            
    double dFidxyz[3][3], dMidxyz[3][3];
    double dFodxyz[3][3], dModxyz[3][3];
    double dFtdxyz[3][3], dMtdxyz[3][3];
    
    double dFwdxyz[3][3];
    
    Case = 0;
    
                            //1234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890 12345678901234567890  
    fprintf(FiniteDiffFile,"   Node             x                    y                    z                  pFxpX                pFxpY                pFxpZ                pFypX                pFypY                pFypZ                pFzpX                pFzpY                pFzpZ                pMxpX                pMxpY                pMxpZ                pMypX                pMypY               pMypZ                 pMzpX                pMzpY                pMzpZ               ");             
    fprintf(FiniteDiffFile,"pFxpX_i              pFxpY_i              pFxpZ_i              pFypX_i              pFypY_i              pFypZ_i              pFzpX_i              pFzpY_i              pFzpZ_i              pMxpX_i              pMxpY_i              pMxpZ_i              pMypX_i              pMyPyi              pMypZ_i               pMzpX_i              pMzpY_i              pMzpZ_i              ");             
    fprintf(FiniteDiffFile,"pFxpX_o              pFxpY_o              pFxpZ_o              pFypX_o              pFypY_o              pFypZ_o              pFzpX_o              pFzpY_o              pFzpZ_o              pMxpX_o              pMxpY_o              pMxpZ_o              pMypX_o              pMyPyi              pMypZ_o               pMzpX_o              pMzpY_o              pMzpZ_o              ");             
    fprintf(FiniteDiffFile,"pFxpX_w              pFxpY_w              pFxpZ_w              pFypX_w              pFypY_w              pFypZ_w              pFzpX_w              pFzpY_w              pFzpZ_w              \n");             
        

   
    // Do initial solve to set things up
    
  //  Case++;
      
  //  VSP_VLM().Solve(Case);    
    
    
    for ( j = 1 ; j <= VSP_VLM().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
    
       VSP_VLM().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
       VSP_VLM().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
       VSP_VLM().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
       
    }         
              
  //  VSP_VLM().UpdateMeshes();
              
    Delta = 1.e-6;    
    
    if ( DoUnsteadyAnalysis_ ) {
       
       Delta = 1.e-6;         
       
 //     VSP_VLM().FreezeMultiPoleAtIteration() = 0;


    }  
    
    LoadGroup = 1; // This needs to be set to either 0, 1, 2 ... to specify the group forces to use
                   // 0 ... is the total vehicle forces, 1, 2 ... N is the nth group forces/moments
                   
    for ( i = 1 ; i <= VSP_VLM().VSPGeom().Grid(0).NumberOfSurfaceNodes() ; i+=1 ) {
  // for ( i = 110 ; i <= 110 ; i+=1 ) {
 //  for ( i = 497 ; i <= 497 ; i+=1 ) {
  //    for ( i = 400 ; i <= VSP_VLM().VSPGeom().Grid(0).NumberOfSurfaceNodes() ; i+=1 ) {
    // for ( i = 19 ; i <= VSP_VLM().VSPGeom().Grid(0).NumberOfSurfaceNodes() ; i+=1 ) {

    // X
    
        printf("Working on node %d of %d \n",i,VSP_VLM().VSPGeom().Grid(0).NumberOfNodes());
        
        // + 
        
           Case++;
             
           VSP_VLM().VSPGeom().Grid(0).NodeList(i).x() += Delta;
           
           VSP_VLM().UpdateMeshes();
           
           VSP_VLM().Solve(Case);

           if ( !DoUnsteadyAnalysis_ ) {
             
              Fxpi = (                    VSP_VLM().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypi = (                    VSP_VLM().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpi = (                    VSP_VLM().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;

              Fxpw = (                    VSP_VLM().CFiwx() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypw = (                    VSP_VLM().CFiwy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpw = (                    VSP_VLM().CFiwz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                   
              Mxpi = (                    VSP_VLM().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypi = (                    VSP_VLM().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpi = (                    VSP_VLM().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
              Fxpo = ( VSP_VLM().CFox()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypo = ( VSP_VLM().CFoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpo = ( VSP_VLM().CFoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                                     
              Mxpo = ( VSP_VLM().CMox()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypo = ( VSP_VLM().CMoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpo = ( VSP_VLM().CMoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
              Fxpt = ( VSP_VLM().CFox() + VSP_VLM().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypt = ( VSP_VLM().CFoy() + VSP_VLM().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpt = ( VSP_VLM().CFoz() + VSP_VLM().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                      
              Mxpt = ( VSP_VLM().CMox() + VSP_VLM().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypt = ( VSP_VLM().CMoy() + VSP_VLM().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpt = ( VSP_VLM().CMoz() + VSP_VLM().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
           }
           
           else {

              Fxpi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                            
              Mxpi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
              Fxpo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                                                  
              Mxpo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                             
              Fxpt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                          
              Mxpt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
        
           }       
            
           for ( j = 1 ; j <= VSP_VLM().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
           
              VSP_VLM().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
              VSP_VLM().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
              VSP_VLM().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
              
           }         
              
        // - 
        
           Case++;
             
           VSP_VLM().VSPGeom().Grid(0).NodeList(i).x() -= Delta;
           
           VSP_VLM().UpdateMeshes();
           
           VSP_VLM().Solve(Case);
           
           if ( !DoUnsteadyAnalysis_ ) {

              Fxmi = (                    VSP_VLM().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymi = (                    VSP_VLM().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmi = (                    VSP_VLM().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                        
              Fxmw = (                    VSP_VLM().CFiwx() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymw = (                    VSP_VLM().CFiwy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmw = (                    VSP_VLM().CFiwz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                 
              Mxmi = (                    VSP_VLM().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymi = (                    VSP_VLM().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmi = (                    VSP_VLM().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                
              Fxmo = ( VSP_VLM().CFox()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymo = ( VSP_VLM().CFoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmo = ( VSP_VLM().CFoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                                   
              Mxmo = ( VSP_VLM().CMox()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymo = ( VSP_VLM().CMoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmo = ( VSP_VLM().CMoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
               
              Fxmt = ( VSP_VLM().CFox() + VSP_VLM().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymt = ( VSP_VLM().CFoy() + VSP_VLM().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmt = ( VSP_VLM().CFoz() + VSP_VLM().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                  
              Mxmt = ( VSP_VLM().CMox() + VSP_VLM().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymt = ( VSP_VLM().CMoy() + VSP_VLM().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmt = ( VSP_VLM().CMoz() + VSP_VLM().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                
           }
           
           else {

              Fxmi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                          
              Mxmi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                               
              Fxmo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                                               
              Mxmo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                               
              Fxmt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                          
              Mxmt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
      
           }    
              
           for ( j = 1 ; j <= VSP_VLM().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
           
              VSP_VLM().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
              VSP_VLM().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
              VSP_VLM().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
              
           }
          
           dFidxyz[0][0] = ( Fxpi - Fxmi )/(2.*Delta);
           dFidxyz[1][0] = ( Fypi - Fymi )/(2.*Delta);
           dFidxyz[2][0] = ( Fzpi - Fzmi )/(2.*Delta);

           dFwdxyz[0][0] = ( Fxpw - Fxmw )/(2.*Delta);
           dFwdxyz[1][0] = ( Fypw - Fymw )/(2.*Delta);
           dFwdxyz[2][0] = ( Fzpw - Fzmw )/(2.*Delta);
                                         
           dMidxyz[0][0] = ( Mxpi - Mxmi )/(2.*Delta);
           dMidxyz[1][0] = ( Mypi - Mymi )/(2.*Delta);
           dMidxyz[2][0] = ( Mzpi - Mzmi )/(2.*Delta);

           dFodxyz[0][0] = ( Fxpo - Fxmo )/(2.*Delta);
           dFodxyz[1][0] = ( Fypo - Fymo )/(2.*Delta);
           dFodxyz[2][0] = ( Fzpo - Fzmo )/(2.*Delta);
                     
           dModxyz[0][0] = ( Mxpo - Mxmo )/(2.*Delta);
           dModxyz[1][0] = ( Mypo - Mymo )/(2.*Delta);
           dModxyz[2][0] = ( Mzpo - Mzmo )/(2.*Delta);
           
           dFtdxyz[0][0] = ( Fxpt - Fxmt )/(2.*Delta);
           dFtdxyz[1][0] = ( Fypt - Fymt )/(2.*Delta);
           dFtdxyz[2][0] = ( Fzpt - Fzmt )/(2.*Delta);
                          
           dMtdxyz[0][0] = ( Mxpt - Mxmt )/(2.*Delta);
           dMtdxyz[1][0] = ( Mypt - Mymt )/(2.*Delta);
           dMtdxyz[2][0] = ( Mzpt - Mzmt )/(2.*Delta);
           
    // Y

        // Central differences
        
        if ( !SymmetryFlag_ || VSP_VLM().VSPGeom().Grid(0).NodeList(i).y() > 1.e-6 ) {
           
           // + 
           
              Case++;
                
              VSP_VLM().VSPGeom().Grid(0).NodeList(i).y() += Delta;
              
              VSP_VLM().UpdateMeshes();
              
              VSP_VLM().Solve(Case);
              
              if ( !DoUnsteadyAnalysis_ ) {
                 
                 Fxpi = (                    VSP_VLM().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypi = (                    VSP_VLM().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpi = (                    VSP_VLM().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                        
                 Fxpw = (                    VSP_VLM().CFiwx() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypw = (                    VSP_VLM().CFiwy() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpw = (                    VSP_VLM().CFiwz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                         
                 Mxpi = (                    VSP_VLM().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mypi = (                    VSP_VLM().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzpi = (                    VSP_VLM().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
                 Fxpo = ( VSP_VLM().CFox()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypo = ( VSP_VLM().CFoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpo = ( VSP_VLM().CFoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                                        
                 Mxpo = ( VSP_VLM().CMox()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mypo = ( VSP_VLM().CMoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzpo = ( VSP_VLM().CMoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
                 Fxpt = ( VSP_VLM().CFox() + VSP_VLM().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypt = ( VSP_VLM().CFoy() + VSP_VLM().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpt = ( VSP_VLM().CFoz() + VSP_VLM().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                         
                 Mxpt = ( VSP_VLM().CMox() + VSP_VLM().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mypt = ( VSP_VLM().CMoy() + VSP_VLM().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzpt = ( VSP_VLM().CMoz() + VSP_VLM().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
              }
              
              else {

                 Fxpi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                             
                 Mxpi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mypi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzpi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                     
                 Fxpo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                                                     
                 Mxpo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mypo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzpo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                  
                 Fxpt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                   
                 Mxpt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mypt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzpt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
              }      
            
              for ( j = 1 ; j <= VSP_VLM().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
              
                 VSP_VLM().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
                 VSP_VLM().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
                 VSP_VLM().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
                 
              }
                       
           // -
                       
              Case++;
                 
              VSP_VLM().VSPGeom().Grid(0).NodeList(i).y() -= Delta;
              
              VSP_VLM().UpdateMeshes();
              
              VSP_VLM().Solve(Case);
                
              if ( !DoUnsteadyAnalysis_ ) {
                 
                    Fxmi = (                    VSP_VLM().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fymi = (                    VSP_VLM().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fzmi = (                    VSP_VLM().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
              
                    Fxmw = (                    VSP_VLM().CFiwx() ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fymw = (                    VSP_VLM().CFiwy() ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fzmw = (                    VSP_VLM().CFiwz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                       
                    Mxmi = (                    VSP_VLM().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                    Mymi = (                    VSP_VLM().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                    Mzmi = (                    VSP_VLM().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                    
                    Fxmo = ( VSP_VLM().CFox()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fymo = ( VSP_VLM().CFoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fzmo = ( VSP_VLM().CFoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                                         
                    Mxmo = ( VSP_VLM().CMox()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                    Mymo = ( VSP_VLM().CMoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                    Mzmo = ( VSP_VLM().CMoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                    
                    Fxmt = ( VSP_VLM().CFox() + VSP_VLM().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fymt = ( VSP_VLM().CFoy() + VSP_VLM().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fzmt = ( VSP_VLM().CFoz() + VSP_VLM().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                      
                    Mxmt = ( VSP_VLM().CMox() + VSP_VLM().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                    Mymt = ( VSP_VLM().CMoy() + VSP_VLM().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                    Mzmt = ( VSP_VLM().CMoz() + VSP_VLM().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                    
              }
                 
              else {

                 Fxmi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fymi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzmi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                              
                 Mxmi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mymi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzmi = (                                                               VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                               
                 Fxmo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fymo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzmo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                               
                 Mxmo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mymo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzmo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                   
                 Fxmt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fymt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzmt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                
                 Mxmt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mymt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzmt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                  
              }    
                 
              for ( j = 1 ; j <= VSP_VLM().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
              
                 VSP_VLM().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
                 VSP_VLM().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
                 VSP_VLM().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
                 
              }         
              
              dFidxyz[0][1] = ( Fxpi - Fxmi )/(2.*Delta);
              dFidxyz[1][1] = ( Fypi - Fymi )/(2.*Delta);
              dFidxyz[2][1] = ( Fzpi - Fzmi )/(2.*Delta);

              dFwdxyz[0][1] = ( Fxpw - Fxmw )/(2.*Delta);
              dFwdxyz[1][1] = ( Fypw - Fymw )/(2.*Delta);
              dFwdxyz[2][1] = ( Fzpw - Fzmw )/(2.*Delta);
                                           
              dMidxyz[0][1] = ( Mxpi - Mxmi )/(2.*Delta);
              dMidxyz[1][1] = ( Mypi - Mymi )/(2.*Delta);
              dMidxyz[2][1] = ( Mzpi - Mzmi )/(2.*Delta);
                       
              dFodxyz[0][1] = ( Fxpo - Fxmo )/(2.*Delta);
              dFodxyz[1][1] = ( Fypo - Fymo )/(2.*Delta);
              dFodxyz[2][1] = ( Fzpo - Fzmo )/(2.*Delta);
                          
              dModxyz[0][1] = ( Mxpo - Mxmo )/(2.*Delta);
              dModxyz[1][1] = ( Mypo - Mymo )/(2.*Delta);
              dModxyz[2][1] = ( Mzpo - Mzmo )/(2.*Delta);
                       
              dFtdxyz[0][1] = ( Fxpt - Fxmt )/(2.*Delta);
              dFtdxyz[1][1] = ( Fypt - Fymt )/(2.*Delta);
              dFtdxyz[2][1] = ( Fzpt - Fzmt )/(2.*Delta);
                          
              dMtdxyz[0][1] = ( Mxpt - Mxmt )/(2.*Delta);
              dMtdxyz[1][1] = ( Mypt - Mymt )/(2.*Delta);
              dMtdxyz[2][1] = ( Mzpt - Mzmt )/(2.*Delta);
              
        }
        
        // Just zero gradients across symmetry plane
        
        else {

           dFidxyz[0][1] = 0.;
           dFidxyz[1][1] = 0.;
           dFidxyz[2][1] = 0.;
                     
           dMidxyz[0][1] = 0.;
           dMidxyz[1][1] = 0.;
           dMidxyz[2][1] = 0.;
                       
           dFidxyz[0][1] = 0.;
           dFidxyz[1][1] = 0.;
           dFidxyz[2][1] = 0.;
                       
           dMidxyz[0][1] = 0.;
           dMidxyz[1][1] = 0.;
           dMidxyz[2][1] = 0.;
                       
           dFtdxyz[0][1] = 0.;
           dFtdxyz[1][1] = 0.;
           dFtdxyz[2][1] = 0.;
                     
           dMtdxyz[0][1] = 0.;
           dMtdxyz[1][1] = 0.;
           dMtdxyz[2][1] = 0.;
        
        }           
           
        // Z
             
        // + 
        
           Case++;
             
           VSP_VLM().VSPGeom().Grid(0).NodeList(i).z() += Delta;
           
           VSP_VLM().UpdateMeshes();
           
           VSP_VLM().Solve(Case);
           
           if ( !DoUnsteadyAnalysis_ ) {
              
              Fxpi = (                    VSP_VLM().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypi = (                    VSP_VLM().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpi = (                    VSP_VLM().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;

              Fxpw = (                    VSP_VLM().CFiwx() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypw = (                    VSP_VLM().CFiwy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpw = (                    VSP_VLM().CFiwz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                    
              Mxpi = (                    VSP_VLM().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypi = (                    VSP_VLM().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpi = (                    VSP_VLM().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
              Fxpo = ( VSP_VLM().CFox()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypo = ( VSP_VLM().CFoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpo = ( VSP_VLM().CFoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                                     
              Mxpo = ( VSP_VLM().CMox()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypo = ( VSP_VLM().CMoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpo = ( VSP_VLM().CMoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
              Fxpt = ( VSP_VLM().CFox() + VSP_VLM().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypt = ( VSP_VLM().CFoy() + VSP_VLM().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpt = ( VSP_VLM().CFoz() + VSP_VLM().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                      
              Mxpt = ( VSP_VLM().CMox() + VSP_VLM().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypt = ( VSP_VLM().CMoy() + VSP_VLM().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpt = ( VSP_VLM().CMoz() + VSP_VLM().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
           }
           
           else {

              Fxpi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                             
              Mxpi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                     
              Fxpo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                                                
              Mxpo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                    
              Fxpt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                           
              Mxpt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
              
           }    

           for ( j = 1 ; j <= VSP_VLM().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
           
              VSP_VLM().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
              VSP_VLM().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
              VSP_VLM().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
              
           }    
                
        // - 
        
           Case++;
             
           VSP_VLM().VSPGeom().Grid(0).NodeList(i).z() -= Delta;
           
           VSP_VLM().UpdateMeshes();
           
           VSP_VLM().Solve(Case);
           
           if ( !DoUnsteadyAnalysis_ ) {
              
              Fxmi = (                    VSP_VLM().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymi = (                    VSP_VLM().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmi = (                    VSP_VLM().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;

              Fxmw = (                    VSP_VLM().CFiwx() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymw = (                    VSP_VLM().CFiwy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmw = (                    VSP_VLM().CFiwz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                  
              Mxmi = (                    VSP_VLM().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymi = (                    VSP_VLM().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmi = (                    VSP_VLM().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                
              Fxmo = ( VSP_VLM().CFox()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymo = ( VSP_VLM().CFoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmo = ( VSP_VLM().CFoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                                    
              Mxmo = ( VSP_VLM().CMox()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymo = ( VSP_VLM().CMoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmo = ( VSP_VLM().CMoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                
              Fxmt = ( VSP_VLM().CFox() + VSP_VLM().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymt = ( VSP_VLM().CFoy() + VSP_VLM().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmt = ( VSP_VLM().CFoz() + VSP_VLM().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                  
              Mxmt = ( VSP_VLM().CMox() + VSP_VLM().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymt = ( VSP_VLM().CMoy() + VSP_VLM().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmt = ( VSP_VLM().CMoz() + VSP_VLM().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
           }
           
           else {

              Fxmi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                           
              Mxmi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmi = (                                                                VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                     
              Fxmo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                                                 
              Mxmo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmo = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                    
              Fxmt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                           
              Mxmt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmt = ( VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg() + VSP_VLM().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
              
           }    
               
           //VSP_VLM().VSPGeom().Grid(0).NodeList(i).z() += Delta;

           for ( j = 1 ; j <= VSP_VLM().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
           
              VSP_VLM().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
              VSP_VLM().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
              VSP_VLM().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
              
           }         
                          
           dFidxyz[0][2] = ( Fxpi - Fxmi )/(2.*Delta);
           dFidxyz[1][2] = ( Fypi - Fymi )/(2.*Delta);
           dFidxyz[2][2] = ( Fzpi - Fzmi )/(2.*Delta);

           dFwdxyz[0][2] = ( Fxpw - Fxmw )/(2.*Delta);
           dFwdxyz[1][2] = ( Fypw - Fymw )/(2.*Delta);
           dFwdxyz[2][2] = ( Fzpw - Fzmw )/(2.*Delta);
                                    
           dMidxyz[0][2] = ( Mxpi - Mxmi )/(2.*Delta);
           dMidxyz[1][2] = ( Mypi - Mymi )/(2.*Delta);
           dMidxyz[2][2] = ( Mzpi - Mzmi )/(2.*Delta);
                     
           dFodxyz[0][2] = ( Fxpo - Fxmo )/(2.*Delta);
           dFodxyz[1][2] = ( Fypo - Fymo )/(2.*Delta);
           dFodxyz[2][2] = ( Fzpo - Fzmo )/(2.*Delta);
                          
           dModxyz[0][2] = ( Mxpo - Mxmo )/(2.*Delta);
           dModxyz[1][2] = ( Mypo - Mymo )/(2.*Delta);
           dModxyz[2][2] = ( Mzpo - Mzmo )/(2.*Delta);
                     
           dFtdxyz[0][2] = ( Fxpt - Fxmt )/(2.*Delta);
           dFtdxyz[1][2] = ( Fypt - Fymt )/(2.*Delta);
           dFtdxyz[2][2] = ( Fzpt - Fzmt )/(2.*Delta);
                      
           dMtdxyz[0][2] = ( Mxpt - Mxmt )/(2.*Delta);
           dMtdxyz[1][2] = ( Mypt - Mymt )/(2.*Delta);
           dMtdxyz[2][2] = ( Mzpt - Mzmt )/(2.*Delta);

        // Output results

        fprintf(FiniteDiffFile,"%10d %20.10e %20.10e %20.10e ",
         i,
         VSP_VLM().VSPGeom().Grid(0).NodeList(i).x(),
         VSP_VLM().VSPGeom().Grid(0).NodeList(i).y(),
         VSP_VLM().VSPGeom().Grid(0).NodeList(i).z());

        fprintf(FiniteDiffFile,"%20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e ",
         dFtdxyz[0][0],
         dFtdxyz[0][1],
         dFtdxyz[0][2],
         dFtdxyz[1][0],
         dFtdxyz[1][1],
         dFtdxyz[1][2],
         dFtdxyz[2][0],
         dFtdxyz[2][1],
         dFtdxyz[2][2],
          
         dMtdxyz[0][0],
         dMtdxyz[0][1],
         dMtdxyz[0][2],
         dMtdxyz[1][0],
         dMtdxyz[1][1],
         dMtdxyz[1][2],
         dMtdxyz[2][0],
         dMtdxyz[2][1],
         dMtdxyz[2][2]);

        fprintf(FiniteDiffFile,"%20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e ",
         dFidxyz[0][0],
         dFidxyz[0][1],
         dFidxyz[0][2],
         dFidxyz[1][0],
         dFidxyz[1][1],
         dFidxyz[1][2],
         dFidxyz[2][0],
         dFidxyz[2][1],
         dFidxyz[2][2],
          
         dMidxyz[0][0],
         dMidxyz[0][1],
         dMidxyz[0][2],
         dMidxyz[1][0],
         dMidxyz[1][1],
         dMidxyz[1][2],
         dMidxyz[2][0],
         dMidxyz[2][1],
         dMidxyz[2][2]);
         
        fprintf(FiniteDiffFile,"%20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e ",                  
         dFodxyz[0][0],
         dFodxyz[0][1],
         dFodxyz[0][2],
         dFodxyz[1][0],
         dFodxyz[1][1],
         dFodxyz[1][2],
         dFodxyz[2][0],
         dFodxyz[2][1],
         dFodxyz[2][2],
          
         dModxyz[0][0],
         dModxyz[0][1],
         dModxyz[0][2],
         dModxyz[1][0],
         dModxyz[1][1],
         dModxyz[1][2],
         dModxyz[2][0],
         dModxyz[2][1],
         dModxyz[2][2]);                 
         

        fprintf(FiniteDiffFile,"%20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e \n",                  
         dFwdxyz[0][0],
         dFwdxyz[0][1],
         dFwdxyz[0][2],
         dFwdxyz[1][0],
         dFwdxyz[1][1],
         dFwdxyz[1][2],
         dFwdxyz[2][0],
         dFwdxyz[2][1],
         dFwdxyz[2][2]);
                                    
    }
    
    delete [] TempXYZ;

}
