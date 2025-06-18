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
#include <iostream>

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

// OpenVSP headers

#ifdef VSPAERO_OPT
#include "APIDefines.h"
#include "APIErrorMgr.h"
#include "APIUpdateCountMgr.h"
#include "GuiInterface.h"
#include "Matrix4d.h"
#include "MessageMgr.h"
#include "Vec2d.h"
#include "Vec3d.h"
#include "VSP_Geom_API.h"
#endif

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
double CLMax2D_;
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

// Stability and control Mach, AoA, and Beta Lists

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

// KJ forces and moments

double CFixForCase[MAXRUNCASES];
double CFiyForCase[MAXRUNCASES];
double CFizForCase[MAXRUNCASES];
       
double CMixForCase[MAXRUNCASES];
double CMiyForCase[MAXRUNCASES];
double CMizForCase[MAXRUNCASES];
        
double CLiForCase[MAXRUNCASES];
double CDiForCase[MAXRUNCASES];
double CSiForCase[MAXRUNCASES];
        
double CMilForCase[MAXRUNCASES];
double CMimForCase[MAXRUNCASES];
double CMinForCase[MAXRUNCASES];

// Wake forces

double CFwxForCase[MAXRUNCASES];
double CFwyForCase[MAXRUNCASES];
double CFwzForCase[MAXRUNCASES];

double CLwForCase[MAXRUNCASES];
double CDwForCase[MAXRUNCASES];
double CSwForCase[MAXRUNCASES];

// Viscous forces and moments

double CFoxForCase[MAXRUNCASES];
double CFoyForCase[MAXRUNCASES];
double CFozForCase[MAXRUNCASES];
       
double CMoxForCase[MAXRUNCASES];
double CMoyForCase[MAXRUNCASES];
double CMozForCase[MAXRUNCASES];
        
double CLoForCase[MAXRUNCASES];
double CDoForCase[MAXRUNCASES];
double CSoForCase[MAXRUNCASES];
         
double CMolForCase[MAXRUNCASES];
double CMomForCase[MAXRUNCASES];
double CMonForCase[MAXRUNCASES];

// Total forces and moments

double CFtxForCase[MAXRUNCASES];
double CFtyForCase[MAXRUNCASES];
double CFtzForCase[MAXRUNCASES];
        
double CMtxForCase[MAXRUNCASES];
double CMtyForCase[MAXRUNCASES];
double CMtzForCase[MAXRUNCASES];
        
double CLtForCase[MAXRUNCASES];
double CDtForCase[MAXRUNCASES];
double CStForCase[MAXRUNCASES];
        
double CMtlForCase[MAXRUNCASES];
double CMtmForCase[MAXRUNCASES];
double CMtnForCase[MAXRUNCASES];

// Stall factor

double MinStallFactor[MAXRUNCASES];

// Optimization function for case

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
int DoFiniteDiffTest_                = 0;
int FiniteDiffTestStartNode_         = 1;
int FiniteDiffTestStepSize_          = 1;
int FiniteDiffTestLoadGroup_         = 0;
int StallModelIsOn_                   = 0;
int SolveOnMGLevel_                  = 0;
int UpdateMatrixPreconditioner_      = 0;
int UseWakeNodeMatrixPreconditioner_ = 0;
int FreezeMultiPoleAtIteration_      = 100000;
int FreezeWakeAtIteration_           = 100000;
int FreezeWakeRootVortices_          = 0;
int QuadTreeBufferLevels_            = 0;
int VSPAERO_InputFileFormatOutput_   = 0;
int RestartFromPreviousSolve_        = 0;


// Optimization variables

int OptimizationNumberOfIterations_      = 10;
int OptimizationNumber1DSearchSteps_     = 10;
int OptimizationUsingWakeForces_         =  0;
int OPtimizationUpdateGeometryGradients_ =  0;

double Optimization_CL_Weight_       = 0.;
double Optimization_CD_Weight_       = 0.;
double Optimization_CS_Weight_       = 0.;

double Optimization_CML_Weight_      = 0.;
double Optimization_CMM_Weight_      = 0.;
double Optimization_CMN_Weight_      = 0.;

double Optimization_CL_Required_     = 0.;
double Optimization_CD_Required_     = 0.;
double Optimization_CS_Required_     = 0.;

double Optimization_CML_Required_    = 0.;
double Optimization_CMM_Required_    = 0.;
double Optimization_CMN_Required_    = 0.;

double OptimizationGradientReduction_ = 0.001;

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

VSP_SOLVER VSPAERO_;
VSP_SOLVER &VSPAERO(void) { return VSPAERO_; };

// Optimization code

#ifdef VSPAERO_OPT

class PARAMETER_DATA {
   
public:
   
   double *ParameterValues;
   char **ParameterNames;

};

void VSPAERO_Optimize(void);

PARAMETER_DATA *ReadOpenVSPDesFile(char *FileName, int &NumberOfDesignVariables);

double *CreateVSPGeometry(char *FileName, int NumberOfDesignVariables, char **ParameterNames, double *ParameterValues);

void SaveVSPGeomFile(char *FileName, int NumberOfDesignVariables, char **ParameterNames, double *ParameterValues);

double **CalculateOpenVSPGeometryGradients(char *FileName, int NumberOfDesignVariables, PARAMETER_DATA *ParameterData);

void DeleteMeshGradients(int NumberOfDesignVariables, double **dMesh_dParameter);

double Normalize(double *Vector, int Length);

void CGState(double *Old, double *New, int Length);

#endif

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

#ifndef VSPAERO_OPT

    printf("VSPAERO v.%d.%d.%d --- Compiled on: %s at %s PST \n", VER_MAJOR, VER_MINOR, VER_PATCH, __DATE__, __TIME__);
    printf("\n\n\n\n");

#endif

#ifdef VSPAERO_OPT

    printf("VSPAERO_OPT v.%d.%d.%d --- Compiled on: %s at %s PST \n", VER_MAJOR, VER_MINOR, VER_PATCH, __DATE__, __TIME__);
    printf("\n\n\n\n");

#endif

    // Jump into optimization routines
    
#ifdef VSPAERO_OPT

    VSPAERO_Optimize();
    
#endif
    
    // Normal forward/adjoint solver
    
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
    
//broken    if ( LoadFEMDeformation_ ) VSPAERO().LoadFEMDeformation() = 1;
    
    // Do ground effects analysis
    
    if ( DoGroundEffectsAnalysis_ ) {
       
       VSPAERO().DoGroundEffectsAnalysis() = 1;
       
       VSPAERO().VehicleRotationAngleVector(0) = 0.;
       VSPAERO().VehicleRotationAngleVector(1) = -AoAList_[1];
       VSPAERO().VehicleRotationAngleVector(2) = 0.;
       
       VSPAERO().VehicleRotationAxisLocation(0) = Xcg_;
       VSPAERO().VehicleRotationAxisLocation(1) = Ycg_;
       VSPAERO().VehicleRotationAxisLocation(2) = Zcg_;  
       
       VSPAERO().HeightAboveGround() = HeightAboveGround_;
       
       NumberOfAoAs_ = 1;
       
       AoAList_[1] = 0.;
       
       Zcg_ += HeightAboveGround_ - Zcg_;
          
       VSPAERO().Xcg() = Xcg_;
       VSPAERO().Ycg() = Ycg_;
       VSPAERO().Zcg() = Zcg_;       

    }
    
    // Write out 2D FEM file
    
    if ( Write2DFEMFile_ ) VSPAERO().Write2DFEMFile() = 1;
    
    // Write out Tecplot file
    
    if ( WriteTecplotFile_ ) VSPAERO().WriteTecplotFile() = 1;
        
    // Save optimization data
    
    if ( OptimizationSolve_ ) VSPAERO().OptimizationSolve() = 1;
            
    // Solve the adjoint problem
    
    if ( DoAdjointSolve_ ) VSPAERO().DoAdjointSolve() = 1;
      
    // User choosing specific grid level to set as finest level
    
    if ( SolveOnMGLevel_ > 0 ) VSPAERO().SolveOnMGLevel(SolveOnMGLevel_);
           
    // Set number of farfield wake nodes

    if ( NumberOfWakeNodes_ > 0 ) VSPAERO().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);       
               
    // Force farfield distance for wake adaption
    
    if ( SetFarDist_ ) VSPAERO().SetFarFieldDist(FarDist_);
                    
    // Load in the VSPgeom geometry file
 
    VSPAERO().ReadFile(FileName);
    
    // Check adjoint component group settings
        
    if ( NumberOfAdjointComponentListSettings_ != 0 ) {
       
       if ( NumberOfAdjointComponentListSettings_ != VSPAERO().VSPGeom().NumberOfComponentGroups() ) {
       
          printf("Number of adjoint component groups listed in .vspaero file does not match actual number of component groups defined in .groups file! \n");
          fflush(NULL);exit(1);
          
       }
       
       for ( i = 1 ; i <= VSPAERO().VSPGeom().NumberOfComponentGroups() ; i++ ) {
    
          VSPAERO().UserAdjointComponentList(i) = UserAdjointComponentList_[i];

       }
       
    }    
    
    // Geometry dump, no solver
    
    if ( DumpGeom_ ) VSPAERO().DumpGeom() = 1;
    
    // Setup high lift file
    
    if ( SetupHighLiftFile_ ) VSPAERO().CreateHighLiftFile() = 1;

    // Turn off spanwise loading calculations for Panel solves
    
    if ( NoPanelSpanWiseLoading_ )  VSPAERO().PanelSpanWiseLoading() = 0;

    // We are doing an interrogation of a previous solution
    
    if ( RestartAndInterrogateSolution_ ) VSPAERO().DoSolutionInterrogation() = 1;

    // Setup

    VSPAERO().Setup();

    // Wake options

    if ( ImplicitWake_ > 0 ) VSPAERO().ImplicitWake() = 1;
    
    if ( ImplicitWakeStartIteration_ > 0 ) VSPAERO().ImplicitWakeStartIteration() = ImplicitWakeStartIteration_;

    // Inform solver of Control Surface Groups ( this information is used in VSP_SOLVER::WriteCaseHeader(FILE *fid) )

    VSPAERO().SetControlSurfaceGroup( ControlSurfaceGroup_, NumberOfControlGroups_ );

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
    
    else if ( DoFiniteDiffTest_ ) {
       
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
          
          VSPAERO().TimeAccurate() = DoUnsteadyAnalysis_ = 1;
          VSPAERO().TimeAnalysisType() = P_ANALYSIS;
          
          VSPAERO().NumberOfTimeSteps()             = 128;
          VSPAERO().Unsteady_AngleMax()             = 5.;
          VSPAERO().Unsteady_HMax()                 = 0.;    
          VSPAERO().CoreSizeFactor()                = 1.;    
          VSPAERO().ForwardGMRESConvergenceFactor() = 1.;   
          VSPAERO().WakeIterations()                = 25;
          
          NumberOfWakeNodes_ = 32;
          
          VSPAERO().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);

       }      
       
       else if ( strcmp(argv[i],"-qstab") == 0 ) {
        
          StabControlRun_ = 3;
          
          VSPAERO().TimeAccurate() = DoUnsteadyAnalysis_ = 1;
          VSPAERO().TimeAnalysisType() = Q_ANALYSIS;
          
          VSPAERO().NumberOfTimeSteps()             = 128;
          VSPAERO().Unsteady_AngleMax()             = 5.;
          VSPAERO().Unsteady_HMax()                 = 0.;    
          VSPAERO().CoreSizeFactor()                = 1.;    
          VSPAERO().ForwardGMRESConvergenceFactor() = 1.;
          VSPAERO().WakeIterations()                = 25;
          
          NumberOfWakeNodes_ = 32;
          
          VSPAERO().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);
                    
       }      
                     
       else if ( strcmp(argv[i],"-rstab") == 0 ) {
        
          StabControlRun_ = 4;
          
          VSPAERO().TimeAccurate() = DoUnsteadyAnalysis_ = 1;
          VSPAERO().TimeAnalysisType() = R_ANALYSIS;
          
          VSPAERO().NumberOfTimeSteps()             = 128;
          VSPAERO().Unsteady_AngleMax()             = 5.;
          VSPAERO().Unsteady_HMax()                 = 0.;    
          VSPAERO().CoreSizeFactor()                = 1.;    
          VSPAERO().ForwardGMRESConvergenceFactor() = 1.;   
          VSPAERO().WakeIterations()                = 25;
          
          NumberOfWakeNodes_ = 32;
          
          VSPAERO().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);
                    
       }              

       else if ( strcmp(argv[i],"-acstab") == 0 ) {
        
          StabControlRun_ = 5;
          
       }
              
       else if ( strcmp(argv[i],"-unsteady") == 0 ) {

          VSPAERO().TimeAccurate() = DoUnsteadyAnalysis_ = 1;

          VSPAERO().TimeAnalysisType() = 0;

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
                    
          DoFiniteDiffTest_ = 1;
          
          FiniteDiffTestStartNode_ = atoi(argv[++i]);
          
          FiniteDiffTestStepSize_  = atoi(argv[++i]);
          
          FiniteDiffTestLoadGroup_ = atoi(argv[++i]);
          
          printf("Doing finite difference gradient test with start grid node: %d and grid node step size of: %d and reporting out load group: %d \n",
          FiniteDiffTestStartNode_,FiniteDiffTestStepSize_,FiniteDiffTestLoadGroup_);

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
    char file_name_w_ext[MAX_CHAR_SIZE], DumChar[MAX_CHAR_SIZE], DumChar2[MAX_CHAR_SIZE], Comma[MAX_CHAR_SIZE], *Next;
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
    Clo2D_                         = 0.;    
    CLMax2D_                       = 1.;
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
    
    if ( SearchForIntegerVariable(case_file, "StallModel", StallModelIsOn_) ) if ( case_file != NULL ) { printf("Setting StallModel to: %d \n",StallModelIsOn_); };
    
    // Search for Clo2D
    
    if ( SearchForFloatVariable(case_file, "Clo2D", Clo2D_) ) if ( case_file != NULL ) {  printf("Setting Clo2D to: %f \n",Clo2D_); };
    
    // Search for CLMax2D_
    
    if ( SearchForFloatVariable(case_file, "CLMax2D", CLMax2D_) ) if ( case_file != NULL ) {  printf("Setting CLMax2D to: %f \n",CLMax2D_); };
        
    // Search for Symmetry setting
    
    if ( SearchForIntegerVariable(case_file, "Symmetry", Symmetry_) ) if ( case_file != NULL ) { printf("Setting Symmetry to: %d \n",Symmetry_); };

    // Search for freeze multipole setting
    
    if ( SearchForIntegerVariable(case_file, "FreezeMultiPoleAtIteration", FreezeMultiPoleAtIteration_) ) if ( case_file != NULL ) { printf("Setting FreezeMultiPoleAtIteration to: %d \n",FreezeMultiPoleAtIteration_); };
         
    if ( FreezeMultiPoleAtIteration_ >= 0 ) VSPAERO().FreezeMultiPoleAtIteration() = FreezeMultiPoleAtIteration_;

    // Search for freeze wake setting
    
    if ( SearchForIntegerVariable(case_file, "FreezeWakeAtIteration", FreezeWakeAtIteration_) ) if ( case_file != NULL ) { printf("Setting FreezeWakeAtIteration to: %d \n",FreezeWakeAtIteration_); };
         
    if ( FreezeWakeAtIteration_ >= 0 ) VSPAERO().FreezeWakeAtIteration() = FreezeWakeAtIteration_;
    
    // Search for freeze wake root vortices setting
    
    if ( SearchForIntegerVariable(case_file, "FreezeWakeRootVortices", FreezeWakeRootVortices_) ) if ( case_file != NULL ) { printf("Setting FreezeWakeRootVortices to: %d \n",FreezeWakeRootVortices_); };
    
    if ( FreezeWakeRootVortices_ >= 0 ) VSPAERO().FreezeWakeRootVortices() = FreezeWakeRootVortices_;

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
  
    if ( WakeRelax_ >= 0. ) VSPAERO().WakeRelax() = WakeRelax_;
   
    // Search for Forward GMRES residual scale factor
    
    ForwardGMRESConvergenceFactor_ = 1.;
    
    if ( SearchForFloatVariable(case_file, "ForwardGMRESConvergenceFactor", ForwardGMRESConvergenceFactor_) ) if ( case_file != NULL ) { printf("Setting ForwardGMRESConvergenceFactor to: %e \n",ForwardGMRESConvergenceFactor_); };
              
    VSPAERO().ForwardGMRESConvergenceFactor() = ForwardGMRESConvergenceFactor_;

    // Search for Adjoint GMRES residual scale factor
    
    AdjointGMRESConvergenceFactor_ = 1.;
    
    if ( SearchForFloatVariable(case_file, "AdjointGMRESConvergenceFactor", AdjointGMRESConvergenceFactor_) ) if ( case_file != NULL ) { printf("Setting AdjointGMRESConvergenceFactor_ to: %e \n",AdjointGMRESConvergenceFactor_); };
              
    VSPAERO().AdjointGMRESConvergenceFactor() = AdjointGMRESConvergenceFactor_;
    
    // Search for Solver residual scale factor
    
    NonLinearConvergenceFactor_ = 1.;
    
    if ( SearchForFloatVariable(case_file, "NonLinearConvergenceFactor", NonLinearConvergenceFactor_) ) if ( case_file != NULL ) { printf("Setting NonLinearConvergenceFactor to: %e \n",NonLinearConvergenceFactor_); };
              
    VSPAERO().NonLinearConvergenceFactor() = NonLinearConvergenceFactor_;
         
    // Search for coresize factor
    
    CoreSizeFactor_ = -1.;
    
    if ( SearchForFloatVariable(case_file, "CoreSizeFactor", CoreSizeFactor_) ) if ( case_file != NULL ) { printf("Setting CoreSizeFactor to: %f \n",CoreSizeFactor_); };
    
    if ( CoreSizeFactor_ > 0. ) VSPAERO().CoreSizeFactor() = CoreSizeFactor_;

    // Search for Faraway factor
    
    FarAway_ = -1.;
    
    if ( SearchForFloatVariable(case_file, "FarAway", FarAway_) ) if ( case_file != NULL ) { printf("Setting multipole FarAway factor to: %f \n",FarAway_); };
    
    if ( FarAway_ > 0. ) VSPAERO().FarAway() = FarAway_;
    
    if ( WakeIterations_ == 0 && DoUnsteadyAnalysis_ == 0 )VSPAERO().GMRESTightConvergence() = 1;

    // Search for preconditioner update flag
    
    UpdateMatrixPreconditioner_ = 0;
    
    if ( SearchForIntegerVariable(case_file, "UpdateMatrixPreconditioner", UpdateMatrixPreconditioner_) ) if ( case_file != NULL ) { printf("Setting UpdateMatrixPreconditioner flag to: %d \n",UpdateMatrixPreconditioner_); };

    VSPAERO().UpdateMatrixPreconditioner() = UpdateMatrixPreconditioner_;
        
    // Search for wake node preconditioner flag
    
    UseWakeNodeMatrixPreconditioner_ = 0;
    
    if ( SearchForIntegerVariable(case_file, "UseWakeNodeMatrixPreconditioner", UseWakeNodeMatrixPreconditioner_) ) if ( case_file != NULL ) { printf("Setting UseWakeNodeMatrixPreconditioner flag to: %d \n",UseWakeNodeMatrixPreconditioner_); };
    
    VSPAERO().UseWakeNodeMatrixPreconditioner() = UseWakeNodeMatrixPreconditioner_;

    // Search for quad tree buffer levels
        
    if ( SearchForIntegerVariable(case_file, "QuadTreeBufferLevels", QuadTreeBufferLevels_) ) if ( case_file != NULL ) { printf("Setting QuadTreeBufferLevels_ flag to: %d \n",QuadTreeBufferLevels_); };
    
    VSPAERO().QuadTreeBufferLevels() = QuadTreeBufferLevels_;
    
    // Search for restart from previous solve option
    
    if ( SearchForIntegerVariable(case_file, "RestartFromPreviousSolve", RestartFromPreviousSolve_) ) if ( case_file != NULL ) { printf("Setting RestartFromPreviousSolve flag to: %d \n",RestartFromPreviousSolve_); };
            
    // Search for adjoint force type
    
    AdjointSolutionForceType_ = 0;
    
    if ( SearchForIntegerVariable(case_file, "AdjointSolutionForceType", AdjointSolutionForceType_) ) if ( case_file != NULL ) { printf("Setting AdjointSolutionForceType to: %d \n",AdjointSolutionForceType_); };
    
    VSPAERO().AdjointSolutionForceType() = AdjointSolutionForceType_;
         
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
          
          VSPAERO().DoAdjointSolveForThisForceMomentCase(i) = AdjointSolveForThisForceMomentCase_[i];
          
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
    
    VSPAERO().Write2DFEMFile() = Write2DFEMFile_;
        
    // Search for Tecplot file output flag
    
    WriteTecplotFile_ = 0;
    
    if ( SearchForIntegerVariable(case_file, "WriteTecplotFile", WriteTecplotFile_) ) if ( case_file != NULL ) {  printf("Setting WriteTecplotFile to: %d \n",WriteTecplotFile_); };
    
    VSPAERO().WriteTecplotFile() = WriteTecplotFile_;    
    
    // Check for zero wake iteration case... so just frozen wake

    if ( WakeIterations_ == 0 ) {
       
       FreezeWakeAtIteration_ = 0;
       
       ForwardGMRESConvergenceFactor_ = MIN(0.001, ForwardGMRESConvergenceFactor_);

       VSPAERO().ForwardGMRESConvergenceFactor() = ForwardGMRESConvergenceFactor_;
       
       VSPAERO().FreezeWakeAtIteration() = FreezeWakeAtIteration_;
    
       
    }
    
    // Optimization flags

    if ( SearchForIntegerVariable(case_file, "OptimizationNumberOfIterations", OptimizationNumberOfIterations_) ) if ( case_file != NULL ) { printf("Setting OptimizationNumberOfIterations to: %d \n",OptimizationNumberOfIterations_); };

    if ( SearchForIntegerVariable(case_file, "OptimizationNumber1DSearchSteps", OptimizationNumber1DSearchSteps_) ) if ( case_file != NULL ) { printf("Setting OptimizationNumber1DSearchSteps to: %d \n",OptimizationNumber1DSearchSteps_); };


    if ( SearchForIntegerVariable(case_file, "OptimizationUsingWakeForces", OptimizationUsingWakeForces_) ) if ( case_file != NULL ) { printf("Setting OptimizationUsingWakeForces flag to: %d \n",OptimizationUsingWakeForces_); };

    if ( SearchForIntegerVariable(case_file, "OPtimizationUpdateGeometryGradients", OPtimizationUpdateGeometryGradients_) ) if ( case_file != NULL ) { printf("Setting OPtimizationUpdateGeometryGradients flag to: %d \n",OPtimizationUpdateGeometryGradients_); };
            
    if ( SearchForFloatVariable(case_file, "OptimizationGradientReduction", OptimizationGradientReduction_) ) if ( case_file != NULL ) { printf("Setting OptimizationGradientReduction to: %f \n",OptimizationGradientReduction_); };

            
    if ( SearchForFloatVariable(case_file, "Optimization_CL_Weight", Optimization_CL_Weight_) ) if ( case_file != NULL ) { printf("Setting Optimization_CL_Weight to: %f \n",Optimization_CL_Weight_); };
    if ( SearchForFloatVariable(case_file, "Optimization_CD_Weight", Optimization_CD_Weight_) ) if ( case_file != NULL ) { printf("Setting Optimization_CD_Weight to: %f \n",Optimization_CD_Weight_); };
    if ( SearchForFloatVariable(case_file, "Optimization_CS_Weight", Optimization_CS_Weight_) ) if ( case_file != NULL ) { printf("Setting Optimization_CS_Weight to: %f \n",Optimization_CS_Weight_); };
   
    if ( SearchForFloatVariable(case_file, "Optimization_CML_Weight", Optimization_CML_Weight_) ) if ( case_file != NULL ) { printf("Setting Optimization_CML_Weight to: %f \n",Optimization_CML_Weight_); };
    if ( SearchForFloatVariable(case_file, "Optimization_CMM_Weight", Optimization_CMM_Weight_) ) if ( case_file != NULL ) { printf("Setting Optimization_CMM_Weight to: %f \n",Optimization_CMM_Weight_); };
    if ( SearchForFloatVariable(case_file, "Optimization_CMN_Weight", Optimization_CMN_Weight_) ) if ( case_file != NULL ) { printf("Setting Optimization_CMN_Weight to: %f \n",Optimization_CMN_Weight_); };


    if ( SearchForFloatVariable(case_file, "Optimization_CL_Required", Optimization_CL_Required_) ) if ( case_file != NULL ) { printf("Setting Optimization_CL_Required to: %f \n",Optimization_CL_Required_); };
    if ( SearchForFloatVariable(case_file, "Optimization_CD_Required", Optimization_CD_Required_) ) if ( case_file != NULL ) { printf("Setting Optimization_CD_Required to: %f \n",Optimization_CD_Required_); };
    if ( SearchForFloatVariable(case_file, "Optimization_CS_Required", Optimization_CS_Required_) ) if ( case_file != NULL ) { printf("Setting Optimization_CS_Required to: %f \n",Optimization_CS_Required_); };

    if ( SearchForFloatVariable(case_file, "Optimization_CML_Required", Optimization_CML_Required_) ) if ( case_file != NULL ) { printf("Setting Optimization_CML_Required to: %f \n",Optimization_CML_Required_); };
    if ( SearchForFloatVariable(case_file, "Optimization_CMM_Required", Optimization_CMM_Required_) ) if ( case_file != NULL ) { printf("Setting Optimization_CMM_Required to: %f \n",Optimization_CMM_Required_); };
    if ( SearchForFloatVariable(case_file, "Optimization_CMN_Required", Optimization_CMN_Required_) ) if ( case_file != NULL ) { printf("Setting Optimization_CMN_Required to: %f \n",Optimization_CMN_Required_); };

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
    
    printf("StallModel      = %d  \n",StallModelIsOn_);
    printf("Clo2D           = %lf \n",Clo2D_);    
    printf("CLMax2D_        = %lf \n",CLMax2D_);    
    
    printf("Symmetry        = %d  \n",Symmetry_);
    printf("FarDist         = %lf \n",FarDist_);
    printf("NumWakeNodes    = %d  \n",NumberOfWakeNodes_);
    printf("WakeIters       = %d  \n",WakeIterations_);

    VSPAERO().Sref() = Sref_;
    VSPAERO().Cref() = Cref_;
    VSPAERO().Bref() = Bref_;

    VSPAERO().Xcg() = Xcg_;
    VSPAERO().Ycg() = Ycg_;
    VSPAERO().Zcg() = Zcg_;
    
    VSPAERO().Mach() = Mach_;
    VSPAERO().AngleOfAttack() = AoA_ * TORAD;
    VSPAERO().AngleOfBeta() = Beta_ * TORAD;
    
    VSPAERO().Vinf() = Vinf_;
    
    VSPAERO().Vref() = Vref_;
    
    VSPAERO().Machref() = Machref_;
    
    VSPAERO().Density() = Rho_;
    
    VSPAERO().ReCref() = ReCref_;
    
    VSPAERO().Clo2D() = Clo2D_;
        
    VSPAERO().Clmax_2d() = CLMax2D_;
    
    VSPAERO().StallModelIsOn() = StallModelIsOn_;

    VSPAERO().WakeIterations() = WakeIterations_;
    
    VSPAERO().RotationalRate_p() = 0.0;
    VSPAERO().RotationalRate_q() = 0.0;
    VSPAERO().RotationalRate_r() = 0.0;    

    VSPAERO().DoSymmetryPlaneSolve() = Symmetry_;
        
    // Load in the control surface data
    
    rewind(case_file);
    
    NumberOfControlGroups_ = 0;

    while ( fgets(DumChar,MAX_CHAR_SIZE,case_file) != NULL ) {

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
                
                snprintf(ControlSurfaceGroup_[i].ControlSurface_Name(1),MAX_CHAR_SIZE*sizeof(char),"%s",DumChar);
                
                printf("Control Surface(1): %s___ \n",ControlSurfaceGroup_[i].ControlSurface_Name(1));
                
                // Read in the control surface direction
                
                fgets(DumChar,MAX_CHAR_SIZE,case_file);
                
                ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(1) = atof(DumChar);
                                                
                printf("Control surface(%d) direction: %f \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(1));
   
                //// Read in the control surface deflection
                //
                //fgets(DumChar,MAX_CHAR_SIZE,case_file);
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
                
                snprintf(ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces),MAX_CHAR_SIZE*sizeof(char),"%s",Next);
                
                ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(NumberOfControlSurfaces) = 1;
                
                printf("Control surface(%d): %s \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces));
   
                while ( Next != NULL ) {
                   
                    Next = strtok(NULL,",\n");
         
                    if ( Next != NULL ) {
                       
                       NumberOfControlSurfaces++;

                       snprintf(ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces),MAX_CHAR_SIZE*sizeof(char),"%s", Next );
                
                       printf("Control surface(%d): %s \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces));
   
                    }
                    
                }
                
                // Now read in the control surface directions
                
                fgets(DumChar,MAX_CHAR_SIZE,case_file);
                
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
    
    while ( !Done && fgets(DumChar,MAX_CHAR_SIZE,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfRotors") != NULL ) {
          
          sscanf(DumChar,"NumberOfRotors = %d \n",&NumberOfRotors_);
          
          printf("NumberOfRotors: %d \n",NumberOfRotors_);
          
          VSPAERO().SetNumberOfRotors(NumberOfRotors_);
          
          for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
           
             fgets(DumChar,MAX_CHAR_SIZE,case_file); printf("%s",DumChar);
             fgets(DumChar,MAX_CHAR_SIZE,case_file); printf("%s",DumChar);
             
             printf("\nLoading data for rotor: %5d \n",i);
             
             VSPAERO().RotorDisk(i).Load_STP_Data(case_file);
             
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
          
          x = VSPAERO().RotorDisk(i).XYZ(0);
          y = VSPAERO().RotorDisk(i).XYZ(1);
          z = VSPAERO().RotorDisk(i).XYZ(2);

          VSPAERO().RotorDisk(i).XYZ(0) = (x - Xcg_)*cos(RotAngle) - (z - Zcg_)*sin(RotAngle) + Xcg_;
          VSPAERO().RotorDisk(i).XYZ(1) = y;
          VSPAERO().RotorDisk(i).XYZ(2) = (x - Xcg_)*sin(RotAngle) + (z - Zcg_)*cos(RotAngle) + Zcg_ + DeltaHeight;    
     
          // Rotate rotors to align with free stream
          
          Vec(0) = VSPAERO().RotorDisk(i).Normal(0);
          Vec(1) = VSPAERO().RotorDisk(i).Normal(1);
          Vec(2) = VSPAERO().RotorDisk(i).Normal(2);
       
          Vec = Quat * Vec * InvQuat;
       
          VSPAERO().RotorDisk(i).Normal(0) = Vec(0);
          VSPAERO().RotorDisk(i).Normal(1) = Vec(1); 
          VSPAERO().RotorDisk(i).Normal(2) = Vec(2); 
          
       }
                    
    }
        
    // Load in the velocity survey data
    
    rewind(case_file);
    
    NumberofSurveyPoints_ = 0;
    
    Done = 0;
        
    while ( !Done && fgets(DumChar,MAX_CHAR_SIZE,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberofSurveyPoints") != NULL ) {

          sscanf(DumChar,"NumberofSurveyPoints = %d \n",&NumberofSurveyPoints_);
          
          printf("NumberofSurveyPoints: %d \n",NumberofSurveyPoints_);
          
          VSPAERO().SetNumberOfSurveyPoints(NumberofSurveyPoints_);
          
          for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {
             
             fscanf(case_file,"%d %lf %lf %lf \n",&j,&x,&y,&z);
             
             printf("Survey Point: %10d: %10.5f %10.5f %10.5f \n",i,x,y,z);
             
             VSPAERO().SurveyPointList(i).x() = x;
             VSPAERO().SurveyPointList(i).y() = y;
             VSPAERO().SurveyPointList(i).z() = z;
             
          }
       
       }
       
    }
    
    // Load in the quad tree survey planes
    
    rewind(case_file);
    
    NumberOfQuadTrees_ = 0;
    
    Done = 0;
        
    while ( !Done && fgets(DumChar,MAX_CHAR_SIZE,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfQuadTrees") != NULL ) {

          sscanf(DumChar,"NumberOfQuadTrees = %d \n",&NumberOfQuadTrees_);
          
          printf("NumberOfQuadTrees: %d \n",NumberOfQuadTrees_);
          
          VSPAERO().SetNumberOfQuadTrees(NumberOfQuadTrees_);
          
          for ( i = 1 ; i <= NumberOfQuadTrees_ ; i++ ) {
             
             fscanf(case_file,"%d %d %lf \n",&j,&Dir,&Value);
             
             printf("QuadTree(%d): %10d: %10.5f \n",j,Dir,Value);
             
             VSPAERO().QuadTreeDirection(i) = Dir;
             VSPAERO().QuadTreeValue(i) = Value;
             
          }
       
       }
       
    }

    // Determine the number of inlets and nozzles
    
    rewind(case_file);
    
    NumberOfInlets_ = NumberOfNozzles_ = 0;

    Done = 0;
        
    while ( !Done && fgets(DumChar,MAX_CHAR_SIZE,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfInlets") != NULL ) {

          sscanf(DumChar,"NumberOfInlets = %d \n",&NumberOfInlets_);
          
          printf("NumberOfInlets: %d \n",NumberOfInlets_);
	  
       }
       
    }
    
    rewind(case_file);
	  
    Done = 0;
        
    while ( !Done && fgets(DumChar,MAX_CHAR_SIZE,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfNozzles") != NULL ) {

          sscanf(DumChar,"NumberOfNozzles = %d \n",&NumberOfNozzles_);
          
          printf("NumberOfNozzles: %d \n",NumberOfNozzles_);
	  
       }
       
    }
    
    VSPAERO().SetNumberOfEngineFaces(NumberOfInlets_ + NumberOfNozzles_);
    
    // Read in inlet data
    
    rewind(case_file);
    	      
    Done = 0;
        
    while ( !Done && fgets(DumChar,MAX_CHAR_SIZE,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfInlets") != NULL ) {

          sscanf(DumChar,"NumberOfInlets = %d \n",&NumberOfInlets_);
          
          printf("NumberOfInlets: %d \n",NumberOfInlets_);
          
          for ( i = 1 ; i <= NumberOfInlets_ ; i++ ) {
             
             fscanf(case_file,"%d %d %lf \n",&j,&Surface,&MassFlow);

             printf("EngineFace(%d) with surface %10d and massflow of %10.5f is an inlet \n",j,Surface,-MassFlow);
                 
             VSPAERO().EngineFace(i).SurfaceType() = INLET_FACE;
             VSPAERO().EngineFace(i).SurfaceID() = Surface;
             VSPAERO().EngineFace(i).MassFlow() = MassFlow;
             
          }
       
       }
       
    }
      
    // Read in nozzle data
    
    rewind(case_file);
    	      
    Done = 0;
        
    while ( !Done && fgets(DumChar,MAX_CHAR_SIZE,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfNozzles") != NULL ) {

          sscanf(DumChar,"NumberOfNozzles = %d \n",&NumberOfInlets_);
          
          printf("NumberOfNozzles: %d \n",NumberOfNozzles_);
          
          for ( i = 1 ; i <= NumberOfNozzles_ ; i++ ) {
	     
	          k = NumberOfInlets_ + i;
             
             fscanf(case_file,"%d %d %lf %lf \n",&j,&Surface,&Velocity,&DeltaCp);

	          fscanf(case_file,"%lf %lf %lf \n",&(VSPAERO().EngineFace(k).NozzleXYZ(0)),&(VSPAERO().EngineFace(k).NozzleXYZ(1)),&(VSPAERO().EngineFace(k).NozzleXYZ(2)));
	          fscanf(case_file,"%lf %lf %lf \n",&(VSPAERO().EngineFace(k).NozzleNormal(0)),&(VSPAERO().EngineFace(k).NozzleNormal(1)),&(VSPAERO().EngineFace(k).NozzleNormal(2)));	     
	          fscanf(case_file,"%lf \n",&(VSPAERO().EngineFace(i).NozzleRadius()));
                      
             printf("EngineFace(%d) with surface %10d and velocity of %10.5f is a nozzle \n",j,Surface,Velocity);
	          printf("Nozzle xyz: %f %f %f \n",VSPAERO().EngineFace(k).NozzleXYZ(0),VSPAERO().EngineFace(k).NozzleXYZ(1),VSPAERO().EngineFace(k).NozzleXYZ(2));
 	          printf("Nozzle normal: %f %f %f \n",VSPAERO().EngineFace(k).NozzleNormal(0),VSPAERO().EngineFace(k).NozzleNormal(1),VSPAERO().EngineFace(k).NozzleNormal(2));
	          printf("Nozzle radius: %f \n",VSPAERO().EngineFace(k).NozzleRadius());
         
             VSPAERO().EngineFace(k).SurfaceType() = NOZZLE_FACE;                
             VSPAERO().EngineFace(k).SurfaceID() = Surface;
             VSPAERO().EngineFace(k).NozzleVelocity() = Velocity;
             VSPAERO().EngineFace(k).NozzleDeltaCp() = DeltaCp;
             
          }
       
       }
       
    }
                
    // Look for number of output time steps for survey points
    
    rewind(case_file);
        
    Done = 0;
        
    while ( !Done && fgets(DumChar,MAX_CHAR_SIZE,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfSurveyTimeSteps") != NULL ) {

          sscanf(DumChar,"NumberOfSurveyTimeSteps = %d \n",&NumberOfSurveyTimeSteps_);
          
          printf("NumberOfSurveyTimeSteps: %d \n",NumberOfSurveyTimeSteps_);
          
          VSPAERO().NumberOfSurveyTimeSteps() = NumberOfSurveyTimeSteps_;
          
          Done = 1;

       }
       
    }
        
    // Load in unsteady aero data
    
    rewind(case_file);
    
    Done = 0;

    if ( VSPAERO().TimeAnalysisType() == 0 ) {

        // Search for the number of time steps
            
        if ( SearchForIntegerVariable(case_file, "NumberOfTimeSteps", NumberOfTimeSteps_) ) printf("Setting NumberOfTimeSteps to: %d \n",NumberOfTimeSteps_);
        
        VSPAERO().NumberOfTimeSteps() = NumberOfTimeSteps_;
    
        // Search for the start of the time averaging time step
            
        if ( SearchForIntegerVariable(case_file, "StartAveragingTimeStep", StartAveragingTimeStep_) ) printf("Setting StartAveragingTimeStep to: %d \n",StartAveragingTimeStep_);
        
        VSPAERO().StartAveragingTimeStep() = StartAveragingTimeStep_;
    
        // Search for time step
            
        if ( SearchForFloatVariable(case_file, "TimeStep", TimeStep_) ) printf("Setting TimeStep_ to: %f \n",TimeStep_);
        
        VSPAERO().DeltaTime() = TimeStep_;

    }
        
    fclose(case_file);
    
}

/*##############################################################################
#                                                                              #
#                           SearchForIntegerVariable                           #
#                                                                              #
##############################################################################*/

int SearchForIntegerVariable(FILE *File, const char *VariableName, int &Value)
{
   
    int Found;
    char DumChar1[MAX_CHAR_SIZE], DumChar2[MAX_CHAR_SIZE], DumChar3[MAX_CHAR_SIZE], Space[MAX_CHAR_SIZE], *Next;
    
    if ( File == NULL ) {
       
       printf("%s ... Integer \n",VariableName);
    
       return 0;
          
    }
        
    snprintf(Space,sizeof(Space)*sizeof(char)," ");
    
    // Search file from the begining
    
    Found = 0;
    
    rewind(File);
        
    while ( fgets(DumChar1,MAX_CHAR_SIZE,File) != NULL && !Found ) {

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
#                           SearchForCharacterVariable                         #
#                                                                              #
##############################################################################*/

int SearchForCharacterVariable(FILE *File, const char *VariableName, char *Variable)
{

    int Found;
    char DumChar1[MAX_CHAR_SIZE], DumChar2[MAX_CHAR_SIZE], DumChar3[MAX_CHAR_SIZE], Space[MAX_CHAR_SIZE], *Next;
    
    if ( File == NULL ) {
       
       printf("%s ... character string\n",VariableName);
    
       return 0;
          
    }
        
    snprintf(Space,sizeof(Space)*sizeof(char)," ");
    
    // Search file from the begining
    
    Found = 0;
    
    rewind(File);
        
    while ( fgets(DumChar1,MAX_CHAR_SIZE,File) != NULL && !Found ) {

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
#                           SearchForFloatVariable                             #
#                                                                              #
##############################################################################*/

int SearchForFloatVariable(FILE *File, const char *VariableName, double &Value)
{

    int Found;
    char DumChar1[MAX_CHAR_SIZE], DumChar2[MAX_CHAR_SIZE], DumChar3[MAX_CHAR_SIZE], Space[MAX_CHAR_SIZE], *Next;
    
    if ( File == NULL ) {
       
       printf("%s ... Float \n",VariableName);
    
       return 0;
          
    }
        
    snprintf(Space,sizeof(Space)*sizeof(char)," ");
    
    // Search file from the begining
    
    Found = 0;
    
    rewind(File);
        
    while ( fgets(DumChar1,MAX_CHAR_SIZE,File) != NULL && !Found ) {

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
    char DumChar1[MAX_CHAR_SIZE], DumChar2[MAX_CHAR_SIZE], Space[MAX_CHAR_SIZE], Comma[MAX_CHAR_SIZE], *Next;

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
        
    while ( fgets(DumChar2,MAX_CHAR_SIZE,File) != NULL && !Found ) {

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
    char DumChar1[MAX_CHAR_SIZE], DumChar2[MAX_CHAR_SIZE], Space[MAX_CHAR_SIZE], Comma[MAX_CHAR_SIZE], *Next;

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
        
    while ( fgets(DumChar2,MAX_CHAR_SIZE,File) != NULL && !Found ) {

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
          
          for ( k = 1 ; k <= VSPAERO().VSPGeom().NumberOfControlSurfaces() ; k++ ) {

             if ( strstr( VSPAERO().VSPGeom().ControlSurface(k).Name(), ControlSurfaceGroup_[i].ControlSurface_Name(j) ) != NULL ) {

                 Found = 1;
            
                 VSPAERO().VSPGeom().ControlSurface(k).DeflectionAngle() = ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j) * ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle() * TORAD;

             }
 
          }
          
          // Print out error report
          
          if ( !Found ) {
             
              printf("Could not find control surface: %s in control surface group: %s \n",
                     ControlSurfaceGroup_[i].ControlSurface_Name(j),
                     ControlSurfaceGroup_[i].Name() );
    
              // print out names of all known surfaces
              
              printf( "Known control surfaces:\n" );
              
              for ( k = 1 ; k <= VSPAERO().VSPGeom().NumberOfControlSurfaces() ; k++ ) {
                 
                 printf( "\t%20s\n", VSPAERO().VSPGeom().ControlSurface(k).Name() );
              
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
    double AR, E, Ewake, LoD, LoDwake, CL, CLwake;
    char PolarFileName[MAX_CHAR_SIZE];
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
          
          VSPAERO().RestartFromPreviousSolve() = 0;
             
          for ( k = 1 ; k <= NumberOfAoAs_ ; k++ ) {
             
             Case++;
             
             CaseList[i][j][k][1] = Case;
             
             // Set free stream conditions
             
             VSPAERO().AngleOfBeta()   = BetaList_[i] * TORAD;
             VSPAERO().Mach()          = MachList_[j];  
             VSPAERO().AngleOfAttack() =  AoAList_[k] * TORAD;
     
             VSPAERO().RotationalRate_p() = 0.;
             VSPAERO().RotationalRate_q() = 0.;
             VSPAERO().RotationalRate_r() = 0.;

             // Set a comment line
             
             snprintf(VSPAERO().CaseString(),MAX_CHAR_SIZE*sizeof(char),"Case: %-d ...",Case);
             
             if ( DoGroundEffectsAnalysis_ ) snprintf(VSPAERO().CaseString(),MAX_CHAR_SIZE*sizeof(char),"AoA: %7.3f ... H: %8.3f",-VSPAERO().VehicleRotationAngleVector(1),HeightAboveGround_);

             // Solve this case
             
             if ( SaveRestartFile_ ) VSPAERO().SaveRestartFile() = 1;
   
             if ( DoRestartRun_    ) VSPAERO().DoRestart() = 1;
             
             if ( Case == 1 || Case < NumCases ) {
                
                if ( RestartAndInterrogateSolution_ ) {
                   
                   VSPAERO().RestartAndInterrogateSolution(Case);
                   
                }
                
                else {
          
                   VSPAERO().Solve(Case);
                   
                }
                
             }
             
             else {
               
                if ( RestartAndInterrogateSolution_ ) {
                   
                   VSPAERO().RestartAndInterrogateSolution(-Case);
                   
                }
                                
                else {
                                   
                   VSPAERO().Solve(-Case);
                   
                }
                
             }

             // KJ inviscid forces
             
             CLiForCase[Case]  =  VSPAERO().CLi(); 
             CDiForCase[Case]  =  VSPAERO().CDi();       
             CSiForCase[Case]  =  VSPAERO().CSi();        
                                  
             CFixForCase[Case] =  VSPAERO().CFix();
             CFiyForCase[Case] =  VSPAERO().CFiy();       
             CFizForCase[Case] =  VSPAERO().CFiz();       
                                  
             CMixForCase[Case] =  VSPAERO().CMix();      
             CMiyForCase[Case] =  VSPAERO().CMiy();      
             CMizForCase[Case] =  VSPAERO().CMiz();    
              
             CMilForCase[Case] = -VSPAERO().CMix();      
             CMimForCase[Case] =  VSPAERO().CMiy();      
             CMinForCase[Case] = -VSPAERO().CMiz();  

             // Wake inviscid forces
             
             CLwForCase[Case]  =  VSPAERO().CLiw(); 
             CDwForCase[Case]  =  VSPAERO().CDiw();       
             CSwForCase[Case]  =  VSPAERO().CSiw();        
                                             
             CFwxForCase[Case] =  VSPAERO().CFiwx();
             CFwyForCase[Case] =  VSPAERO().CFiwy();       
             CFwzForCase[Case] =  VSPAERO().CFiwz();       

             // Viscous forces and moments

             CLoForCase[Case]  =  VSPAERO().CLo(); 
             CDoForCase[Case]  =  VSPAERO().CDo();       
             CSoForCase[Case]  =  VSPAERO().CSo();        
                                  
             CFoxForCase[Case] =  VSPAERO().CFox();
             CFoyForCase[Case] =  VSPAERO().CFoy();       
             CFozForCase[Case] =  VSPAERO().CFoz();       
                                  
             CMoxForCase[Case] =  VSPAERO().CMox();      
             CMoyForCase[Case] =  VSPAERO().CMoy();      
             CMozForCase[Case] =  VSPAERO().CMoz();    
             
             CMolForCase[Case] = -VSPAERO().CMox();      
             CMomForCase[Case] =  VSPAERO().CMoy();      
             CMonForCase[Case] = -VSPAERO().CMoz();       
          
             // Total forces and moments, based on wake forces
             
             CLtForCase[Case]  =  VSPAERO().CLo() + VSPAERO().CLiw(); 
             CDtForCase[Case]  =  VSPAERO().CDo() + VSPAERO().CDiw();         
             CStForCase[Case]  =  VSPAERO().CSo() + VSPAERO().CSiw();         
                             
             CFtxForCase[Case] =  VSPAERO().CFox() + VSPAERO().CFiwx();
             CFtyForCase[Case] =  VSPAERO().CFoy() + VSPAERO().CFiwy();      
             CFtzForCase[Case] =  VSPAERO().CFoz() + VSPAERO().CFiwz();       
                              
             CMtxForCase[Case] =  VSPAERO().CMox() + VSPAERO().CMix();           
             CMtyForCase[Case] =  VSPAERO().CMoy() + VSPAERO().CMiy();         
             CMtzForCase[Case] =  VSPAERO().CMoz() + VSPAERO().CMiz(); 
             
             CMtlForCase[Case] = -VSPAERO().CMox() - VSPAERO().CMix();           
             CMtmForCase[Case] =  VSPAERO().CMoy() + VSPAERO().CMiy();           
             CMtnForCase[Case] = -VSPAERO().CMoz() - VSPAERO().CMiz();         
             
             // Stall factor
             
             MinStallFactor[Case] = VSPAERO().MinStallFactor();
                          
             // Loop over any ReCref cases
             
             for ( p = 2 ; p <= NumberOfReCrefs_ ; p++ ) {
                
                Case++;
                
                CaseList[i][j][k][p] = Case;
                
                ReCref_ = ReCrefList_[p];
     
                VSPAERO().ReCref() = ReCref_;
                
                VSPAERO().ReCalculateForces();
                
                // KJ inviscid forces
                
                CLiForCase[Case]  =  VSPAERO().CLi(); 
                CDiForCase[Case]  =  VSPAERO().CDi();       
                CSiForCase[Case]  =  VSPAERO().CSi();        
                                     
                CFixForCase[Case] =  VSPAERO().CFix();
                CFiyForCase[Case] =  VSPAERO().CFiy();       
                CFizForCase[Case] =  VSPAERO().CFiz();       
                                     
                CMixForCase[Case] =  VSPAERO().CMix();      
                CMiyForCase[Case] =  VSPAERO().CMiy();      
                CMizForCase[Case] =  VSPAERO().CMiz();    
                 
                CMilForCase[Case] = -VSPAERO().CMix();      
                CMimForCase[Case] =  VSPAERO().CMiy();      
                CMinForCase[Case] = -VSPAERO().CMiz();  
                
                // Wake inviscid forces
                
                CLwForCase[Case]  =  VSPAERO().CLiw(); 
                CDwForCase[Case]  =  VSPAERO().CDiw();       
                CSwForCase[Case]  =  VSPAERO().CSiw();        
                                              
                CFwxForCase[Case] =  VSPAERO().CFiwx();
                CFwyForCase[Case] =  VSPAERO().CFiwy();       
                CFwzForCase[Case] =  VSPAERO().CFiwz();       

                // Viscous forces
                
                CLoForCase[Case]  =  VSPAERO().CLo(); 
                CDoForCase[Case]  =  VSPAERO().CDo();       
                CSoForCase[Case]  =  VSPAERO().CSo();        
                                     
                CFoxForCase[Case] =  VSPAERO().CFox();
                CFoyForCase[Case] =  VSPAERO().CFoy();       
                CFozForCase[Case] =  VSPAERO().CFoz();       
                                     
                CMoxForCase[Case] =  VSPAERO().CMox();      
                CMoyForCase[Case] =  VSPAERO().CMoy();      
                CMozForCase[Case] =  VSPAERO().CMoz();    
                
                CMolForCase[Case] = -VSPAERO().CMox();      
                CMomForCase[Case] =  VSPAERO().CMoy();      
                CMonForCase[Case] = -VSPAERO().CMoz();     

                // Total forces and moments, based on wake forces
                
                CLtForCase[Case]  =  VSPAERO().CLo() + VSPAERO().CLiw(); 
                CDtForCase[Case]  =  VSPAERO().CDo() + VSPAERO().CDiw();         
                CStForCase[Case]  =  VSPAERO().CSo() + VSPAERO().CSiw();         
                                
                CFtxForCase[Case] =  VSPAERO().CFox() + VSPAERO().CFiwx();
                CFtyForCase[Case] =  VSPAERO().CFoy() + VSPAERO().CFiwy();      
                CFtzForCase[Case] =  VSPAERO().CFoz() + VSPAERO().CFiwz();       
                                 
                CMtxForCase[Case] =  VSPAERO().CMox() + VSPAERO().CMix();           
                CMtyForCase[Case] =  VSPAERO().CMoy() + VSPAERO().CMiy();         
                CMtzForCase[Case] =  VSPAERO().CMoz() + VSPAERO().CMiz(); 
                
                CMtlForCase[Case] = -VSPAERO().CMox() - VSPAERO().CMix();           
                CMtmForCase[Case] =  VSPAERO().CMoy() + VSPAERO().CMiy();           
                CMtnForCase[Case] = -VSPAERO().CMoz() - VSPAERO().CMiz();      
                
                // Stall factor
                
                MinStallFactor[Case] = VSPAERO().MinStallFactor();                
                             
             } 
             
             ReCref_ = ReCrefList_[1];
  
             VSPAERO().ReCref() = ReCref_;             
                
             printf("\n");
             
             if ( RestartFromPreviousSolve_ ) VSPAERO().RestartFromPreviousSolve() = 1;

          }
          
       }
       
    }

    // Write out final integrated force data
    
    snprintf(PolarFileName,sizeof(PolarFileName)*sizeof(char),"%s.polar",FileName);

    if ( (PolarFile = fopen(PolarFileName,"w")) == NULL ) {

       printf("Could not open the polar file output! \n");

       exit(1);

    }    
   
                     //1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456            
   
    fprintf(PolarFile,"                                                                    Surface Integration Forces and Moments -->                                                                                                                                                                                                                                                                                                                                                                                                                                                                   Wake Induced Forces -->  \n");                
    fprintf(PolarFile,"                                                                    Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Surf-Su Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake-Wake \n");
    fprintf(PolarFile,"      Beta             Mach             AoA             Re/1e6             CLo             CLi            CLtot              CDo              CDi             CDtot              CSo              CSi            CStot               L/D              E               CMox             CMoy             CMoz             CMix             CMiy             CMiz             CMxtot           CMytot           CMztot           CFox             CFoy             CFoz             CFix             CFiy             CFiz             CFxtot           CFytot           CFztot           CLwtot           CDwtot           CSwtot           CLiw             CDiw             CSiw             CFwxtot          CFwytot          CFwztot         CFiwx            CFiwy            CFiwz           LoDw              Ew            StallFactor \n");

    // Write out polars, note these are written out in a different order than they were calculated above - we group them by Re number
    
    for ( p = 1 ; p <= NumberOfReCrefs_ ; p++ ) {

       for ( i = 1 ; i <= NumberOfBetas_ ; i++ ) {
          
          for ( j = 1 ; j <= NumberOfMachs_; j++ ) {
                
             for ( k = 1 ; k <= NumberOfAoAs_ ; k++ ) {
 
                Case = CaseList[i][j][k][p];
                   
                AR = Bref_ * Bref_ / Sref_;
                
                // Surface integration 
                
                CL =  CLiForCase[Case] +  CLoForCase[Case];
   
                E = ( CL * CL / ( PI * AR) ) / CDiForCase[Case];
                
                LoD = (CLiForCase[Case] +  CLoForCase[Case])/(CDiForCase[Case] + CDoForCase[Case]);
             
                // Wake integration 
                
                CLwake =  CLiForCase[Case] +  CLoForCase[Case];
   
                Ewake = ( CLwake * CLwake / ( PI * AR) ) / CDwForCase[Case];
                
                LoDwake = (CLwForCase[Case] +  CLoForCase[Case])/(CDwForCase[Case] + CDoForCase[Case]);
                
                fprintf(PolarFile,"%16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf \n",
                        BetaList_[i],
                        MachList_[j],
                        AoAList_[k],
                        ReCrefList_[p]/1.e6,   
                        
                              
                        CLoForCase[Case],               
                        CLiForCase[Case],
                        CLiForCase[Case] + CLoForCase[Case],
                        CDoForCase[Case],
                        CDiForCase[Case],
                        CDiForCase[Case] + CDoForCase[Case],
                        CSoForCase[Case],
                        CSiForCase[Case],
                        CSiForCase[Case] + CSoForCase[Case],            
                        LoD,
                        E,     
                        CMoxForCase[Case],
                        CMoyForCase[Case],
                        CMozForCase[Case],                             
                        CMixForCase[Case],
                        CMiyForCase[Case],
                        CMizForCase[Case],
                        CMixForCase[Case] + CMoxForCase[Case],
                        CMiyForCase[Case] + CMoyForCase[Case],            
                        CMizForCase[Case] + CMozForCase[Case],       
                        CFoxForCase[Case],            
                        CFoyForCase[Case],            
                        CFozForCase[Case],  
                        CFixForCase[Case],            
                        CFiyForCase[Case],            
                        CFizForCase[Case],   
                                                     
                        CFixForCase[Case] + CFoxForCase[Case],            
                        CFiyForCase[Case] + CFoyForCase[Case],            
                        CFizForCase[Case] + CFozForCase[Case],                     
                
                        CLwForCase[Case] + CLoForCase[Case],
                        CDwForCase[Case] + CDoForCase[Case],
                        CSwForCase[Case] + CSoForCase[Case],
                
                        CLwForCase[Case],
                        CDwForCase[Case],
                        CSwForCase[Case],
                        
                        CFwxForCase[Case] + CFoxForCase[Case],            
                        CFwyForCase[Case] + CFoyForCase[Case],            
                        CFwzForCase[Case] + CFozForCase[Case],    
                                      
                        CFwxForCase[Case],
                        CFwyForCase[Case],
                        CFwzForCase[Case],
                        
                        LoDwake,
                        Ewake,
                        
                        VSPAERO().MinStallFactor());
               
                            
             
             
             
             
             
             
             // fprintf(PolarFile,"%16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf %16.12lf\n",             
             //         double(BetaList_[i]),
             //         double(MachList_[j]),
             //         double(AoAList_[k]),
             //         double(ReCrefList_[p])/1.e6,
             //         double(CLForCase[Case]),
             //         double(CDoForCase[Case]),
             //         double(CDForCase[Case] - CDoForCase[Case]),
             //         double(CDForCase[Case]),
             //         double(CDtForCase[Case]),
             //         double(CDoForCase[Case] + CDtForCase[Case]),
             //         double(CSForCase[Case]),            
             //         double(CLForCase[Case]/(CDForCase[Case])),
             //         double(E),
             //         double(CFxForCase[Case]),
             //         double(CFyForCase[Case]),
             //         double(CFzForCase[Case]),
             //         double(CMxForCase[Case]),
             //         double(CMyForCase[Case]),
             //         double(CMzForCase[Case]),
             //         double(CMlForCase[Case]),       
             //         double(CMmForCase[Case]),       
             //         double(CMnForCase[Case]),
             //         double(OptimizationFunctionForCase[Case]));
                           
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
    char StabFileName[MAX_CHAR_SIZE], VorviewFltFileName[MAX_CHAR_SIZE];
    
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
   
    VSPAERO().ForwardGMRESConvergenceFactor() = ForwardGMRESConvergenceFactor_;
     
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
                
                VSPAERO().Mach()          = Stab_MachList_[Case];
                VSPAERO().AngleOfAttack() =  Stab_AoAList_[Case] * TORAD;
                VSPAERO().AngleOfBeta()   = Stab_BetaList_[Case] * TORAD;
         
                VSPAERO().RotationalRate_p() = RotationalRate_pList_[Case];
                VSPAERO().RotationalRate_q() = RotationalRate_qList_[Case];
                VSPAERO().RotationalRate_r() = RotationalRate_rList_[Case];
                
                // Set a comment line

                if ( Case == 1 ) snprintf(VSPAERO().CaseString(),MAX_CHAR_SIZE*sizeof(char),"Base Aero         ");
                if ( Case == 2 ) snprintf(VSPAERO().CaseString(),MAX_CHAR_SIZE*sizeof(char),"Alpha      +%5.3lf",Delta_AoA_);
                if ( Case == 3 ) snprintf(VSPAERO().CaseString(),MAX_CHAR_SIZE*sizeof(char),"Beta       +%5.3lf",Delta_Beta_);
                if ( Case == 4 ) snprintf(VSPAERO().CaseString(),MAX_CHAR_SIZE*sizeof(char),"Roll Rate  +%5.3lf",Delta_P_);
                if ( Case == 5 ) snprintf(VSPAERO().CaseString(),MAX_CHAR_SIZE*sizeof(char),"Pitch Rate +%5.3lf",Delta_Q_);
                if ( Case == 6 ) snprintf(VSPAERO().CaseString(),MAX_CHAR_SIZE*sizeof(char),"Yaw Rate   +%5.3lf",Delta_R_);
                if ( Case == 7 ) snprintf(VSPAERO().CaseString(),MAX_CHAR_SIZE*sizeof(char),"Mach       +%5.3lf",Delta_Mach_);         
                
                // Solve this case
                
                VSPAERO().SaveRestartFile() = VSPAERO().DoRestart() = 0;
         
                if ( CaseTotal < TotalCases ) {
                   
                   VSPAERO().Solve(CaseTotal);
                   
                }
                
                else {
                   
                   VSPAERO().Solve(-CaseTotal);
                   
                }         
                   
                // KJ inviscid forces
                
                CLiForCase[Case]  =  VSPAERO().CLi(); 
                CDiForCase[Case]  =  VSPAERO().CDi();       
                CSiForCase[Case]  =  VSPAERO().CSi();        
                                     
                CFixForCase[Case] =  VSPAERO().CFix();
                CFiyForCase[Case] =  VSPAERO().CFiy();       
                CFizForCase[Case] =  VSPAERO().CFiz();       
                                     
                CMixForCase[Case] =  VSPAERO().CMix();      
                CMiyForCase[Case] =  VSPAERO().CMiy();      
                CMizForCase[Case] =  VSPAERO().CMiz();    
                 
                CMilForCase[Case] = -VSPAERO().CMix();      
                CMimForCase[Case] =  VSPAERO().CMiy();      
                CMinForCase[Case] = -VSPAERO().CMiz();  
                
                // Wake inviscid forces
                
                CLwForCase[Case]  =  VSPAERO().CLiw(); 
                CDwForCase[Case]  =  VSPAERO().CDiw();       
                CSwForCase[Case]  =  VSPAERO().CSiw();        
                                              
                CFwxForCase[Case] =  VSPAERO().CFiwx();
                CFwyForCase[Case] =  VSPAERO().CFiwy();       
                CFwzForCase[Case] =  VSPAERO().CFiwz();       

                // Viscous forces
                
                CLoForCase[Case]  =  VSPAERO().CLo(); 
                CDoForCase[Case]  =  VSPAERO().CDo();       
                CSoForCase[Case]  =  VSPAERO().CSo();        
                                     
                CFoxForCase[Case] =  VSPAERO().CFox();
                CFoyForCase[Case] =  VSPAERO().CFoy();       
                CFozForCase[Case] =  VSPAERO().CFoz();       
                                     
                CMoxForCase[Case] =  VSPAERO().CMox();      
                CMoyForCase[Case] =  VSPAERO().CMoy();      
                CMozForCase[Case] =  VSPAERO().CMoz();    
                
                CMolForCase[Case] = -VSPAERO().CMox();      
                CMomForCase[Case] =  VSPAERO().CMoy();      
                CMonForCase[Case] = -VSPAERO().CMoz();                  

                // Total forces and moments, based on wake forces
                
                CLtForCase[Case]  =  VSPAERO().CLo() + VSPAERO().CLiw(); 
                CDtForCase[Case]  =  VSPAERO().CDo() + VSPAERO().CDiw();         
                CStForCase[Case]  =  VSPAERO().CSo() + VSPAERO().CSiw();         
                                
                CFtxForCase[Case] =  VSPAERO().CFox() + VSPAERO().CFiwx();
                CFtyForCase[Case] =  VSPAERO().CFoy() + VSPAERO().CFiwy();      
                CFtzForCase[Case] =  VSPAERO().CFoz() + VSPAERO().CFiwz();       
                                 
                CMtxForCase[Case] =  VSPAERO().CMox() + VSPAERO().CMix();           
                CMtyForCase[Case] =  VSPAERO().CMoy() + VSPAERO().CMiy();         
                CMtzForCase[Case] =  VSPAERO().CMoz() + VSPAERO().CMiz(); 
                
                CMtlForCase[Case] = -VSPAERO().CMox() - VSPAERO().CMix();           
                CMtmForCase[Case] =  VSPAERO().CMoy() + VSPAERO().CMiy();           
                CMtnForCase[Case] = -VSPAERO().CMoz() - VSPAERO().CMiz();         
             
                printf("\n");
         
             }
             
             Case--;
             
             // Now do the control derivatives
             
             printf("Calculating Control Derivatives... \n"); 
          
             for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) {
                
                CaseTotal++;
                
                printf("Calculating control derivative case: %d of %d \n",i,NumberOfControlGroups_);
                
                // Initialize to unperturbed free stream conditions
                
                VSPAERO().Mach()          = Stab_MachList_[1];
                VSPAERO().AngleOfAttack() =  Stab_AoAList_[1] * TORAD;
                VSPAERO().AngleOfBeta()   = Stab_BetaList_[1] * TORAD;
             
                VSPAERO().RotationalRate_p() = RotationalRate_pList_[1];
                VSPAERO().RotationalRate_q() = RotationalRate_qList_[1];
                VSPAERO().RotationalRate_r() = RotationalRate_rList_[1];
             
                // Perturb controls
             
                Case++;
                
                k = 1;
                
                for ( j = 1 ; j <= ControlSurfaceGroup_[i].NumberOfControlSurfaces() ; j++ ) {
                  
                   printf("Looking for: %s \n",ControlSurfaceGroup_[i].ControlSurface_Name(j));fflush(NULL);
                   
                   Found = 0;
             
                   p = 1;
                   
                   while ( p <= VSPAERO().VSPGeom().NumberOfControlSurfaces() && !Found ) {
                      
                      printf("Checking: %s \n",VSPAERO().VSPGeom().ControlSurface(p).Name());fflush(NULL);
           
                      if ( strstr(VSPAERO().VSPGeom().ControlSurface(p).Name(), ControlSurfaceGroup_[i].ControlSurface_Name(j)) != NULL ) {
               
                         Found = 1;
                      
                         VSPAERO().VSPGeom().ControlSurface(p).DeflectionAngle() = ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j) * (ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle() + Delta_Control_) * TORAD;

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
             
                snprintf(VSPAERO().CaseString(),MAX_CHAR_SIZE*sizeof(char),"Deflecting Control Group: %-d",i);
               
                // Now solve
               
                if ( CaseTotal < TotalCases ) {
                   
                   VSPAERO().Solve(CaseTotal);
                   
                }
                
                else {
                   
                   VSPAERO().Solve(-CaseTotal);
                   
                }         

                // KJ inviscid forces
                
                CLiForCase[Case]  =  VSPAERO().CLi(); 
                CDiForCase[Case]  =  VSPAERO().CDi();       
                CSiForCase[Case]  =  VSPAERO().CSi();        
                                     
                CFixForCase[Case] =  VSPAERO().CFix();
                CFiyForCase[Case] =  VSPAERO().CFiy();       
                CFizForCase[Case] =  VSPAERO().CFiz();       
                                     
                CMixForCase[Case] =  VSPAERO().CMix();      
                CMiyForCase[Case] =  VSPAERO().CMiy();      
                CMizForCase[Case] =  VSPAERO().CMiz();    
                 
                CMilForCase[Case] = -VSPAERO().CMix();      
                CMimForCase[Case] =  VSPAERO().CMiy();      
                CMinForCase[Case] = -VSPAERO().CMiz();  
                
                // Wake inviscid forces
                
                CLwForCase[Case]  =  VSPAERO().CLiw(); 
                CDwForCase[Case]  =  VSPAERO().CDiw();       
                CSwForCase[Case]  =  VSPAERO().CSiw();        
                                              
                CFwxForCase[Case] =  VSPAERO().CFiwx();
                CFwyForCase[Case] =  VSPAERO().CFiwy();       
                CFwzForCase[Case] =  VSPAERO().CFiwz();       

                // Viscous forces
                
                CLoForCase[Case]  =  VSPAERO().CLo(); 
                CDoForCase[Case]  =  VSPAERO().CDo();       
                CSoForCase[Case]  =  VSPAERO().CSo();        
                                     
                CFoxForCase[Case] =  VSPAERO().CFox();
                CFoyForCase[Case] =  VSPAERO().CFoy();       
                CFozForCase[Case] =  VSPAERO().CFoz();       
                                     
                CMoxForCase[Case] =  VSPAERO().CMox();      
                CMoyForCase[Case] =  VSPAERO().CMoy();      
                CMozForCase[Case] =  VSPAERO().CMoz();    
                
                CMolForCase[Case] = -VSPAERO().CMox();      
                CMomForCase[Case] =  VSPAERO().CMoy();      
                CMonForCase[Case] = -VSPAERO().CMoz();     
                
                // Total forces and moments, based on wake forces
                
                CLtForCase[Case]  =  VSPAERO().CLo() + VSPAERO().CLiw(); 
                CDtForCase[Case]  =  VSPAERO().CDo() + VSPAERO().CDiw();         
                CStForCase[Case]  =  VSPAERO().CSo() + VSPAERO().CSiw();         
                                
                CFtxForCase[Case] =  VSPAERO().CFox() + VSPAERO().CFiwx();
                CFtyForCase[Case] =  VSPAERO().CFoy() + VSPAERO().CFiwy();      
                CFtzForCase[Case] =  VSPAERO().CFoz() + VSPAERO().CFiwz();       
                                 
                CMtxForCase[Case] =  VSPAERO().CMox() + VSPAERO().CMix();           
                CMtyForCase[Case] =  VSPAERO().CMoy() + VSPAERO().CMiy();         
                CMtzForCase[Case] =  VSPAERO().CMoz() + VSPAERO().CMiz(); 
                
                CMtlForCase[Case] = -VSPAERO().CMox() - VSPAERO().CMix();           
                CMtmForCase[Case] =  VSPAERO().CMoy() + VSPAERO().CMiy();           
                CMtnForCase[Case] = -VSPAERO().CMoz() - VSPAERO().CMiz();                               

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
    char CaseType[MAX_CHAR_SIZE];
    char caseTypeFormatStr[] = "%-22s +%5.3lf %-9s";

    // Set free stream conditions

    VSPAERO().Mach()          = Mach_;
    VSPAERO().AngleOfAttack() = AoA_  * TORAD;
    VSPAERO().AngleOfBeta()   = Beta_ * TORAD;

    VSPAERO().RotationalRate_p() = 0.;
    VSPAERO().RotationalRate_q() = 0.;
    VSPAERO().RotationalRate_r() = 0.;
        
    // Write out generic header file
        
    VSPAERO().WriteCaseHeader(StabFile);
    
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
               
               CFtxForCase[n],
               CFtyForCase[n],    
               CFtzForCase[n], 
                   
               CMtxForCase[n],  
               CMtyForCase[n],  
               CMtzForCase[n],    
                   
               CLtForCase[n],
               CDtForCase[n],   
               CStForCase[n],
               
               CMtlForCase[n],
               CMtmForCase[n],   
               CMtnForCase[n]);                 
 
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

       dCFx_wrt[n] = ( CFtxForCase[n] - CFtxForCase[1] )/Delta;
       dCFy_wrt[n] = ( CFtyForCase[n] - CFtyForCase[1] )/Delta;
       dCFz_wrt[n] = ( CFtzForCase[n] - CFtzForCase[1] )/Delta;
                         
       dCMx_wrt[n] = ( CMtxForCase[n] - CMtxForCase[1] )/Delta;
       dCMy_wrt[n] = ( CMtyForCase[n] - CMtyForCase[1] )/Delta;
       dCMz_wrt[n] = ( CMtzForCase[n] - CMtzForCase[1] )/Delta;
        
       dCL_wrt[n]  = (  CLtForCase[n] -  CLtForCase[1] )/Delta;    
       dCD_wrt[n]  = (  CDtForCase[n] -  CDtForCase[1] )/Delta;    
       dCS_wrt[n]  = (  CStForCase[n] -  CStForCase[1] )/Delta;    
       
       dCMl_wrt[n] = ( CMtlForCase[n] - CMtlForCase[1] )/Delta;
       dCMm_wrt[n] = ( CMtmForCase[n] - CMtmForCase[1] )/Delta;
       dCMn_wrt[n] = ( CMtnForCase[n] - CMtnForCase[1] )/Delta;       

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

       dCFx_wrt[n+1] = ( CFtxForCase[n] - CFtxForCase[1] )/Delta;
       dCFy_wrt[n+1] = ( CFtyForCase[n] - CFtyForCase[1] )/Delta;
       dCFz_wrt[n+1] = ( CFtzForCase[n] - CFtzForCase[1] )/Delta;
                          
       dCMx_wrt[n+1] = ( CMtxForCase[n] - CMtxForCase[1] )/Delta;
       dCMy_wrt[n+1] = ( CMtyForCase[n] - CMtyForCase[1] )/Delta;
       dCMz_wrt[n+1] = ( CMtzForCase[n] - CMtzForCase[1] )/Delta;
        
       dCL_wrt[n+1]  = (  CLtForCase[n] -  CLtForCase[1] )/Delta;    
       dCD_wrt[n+1]  = (  CDtForCase[n] -  CDtForCase[1] )/Delta;    
       dCS_wrt[n+1]  = (  CStForCase[n] -  CStForCase[1] )/Delta;    
       
       dCMl_wrt[n+1] = ( CMtlForCase[n] - CMtlForCase[1] )/Delta;
       dCMm_wrt[n+1] = ( CMtmForCase[n] - CMtmForCase[1] )/Delta;
       dCMn_wrt[n+1] = ( CMtnForCase[n] - CMtnForCase[1] )/Delta;       

    }    

    fprintf(StabFile,"#\n");
    
    //                        123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012           
    fprintf(StabFile,"#             Base    Derivative:                                                                               "); for ( n = 8 ; n <= NumStabCases_ + NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"             ");     fprintf(StabFile,"\n");
    fprintf(StabFile,"#             Aero         wrt          wrt          wrt          wrt          wrt          wrt          wrt    "); for ( n = 8 ; n <= NumStabCases_ + NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      wrt    ");     fprintf(StabFile,"\n");
    fprintf(StabFile,"Coef          Total        Alpha        Beta          p            q            r           Mach         U      "); for ( n = 8 ; n <= NumStabCases_ + NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"  ConGrp_%-3d ",n-7); fprintf(StabFile,"\n");
    fprintf(StabFile,"#              -           per          per          per          per          per          per          per    "); for ( n = 8 ; n <= NumStabCases_ + NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      per    ");     fprintf(StabFile,"\n");
    fprintf(StabFile,"#              -           rad          rad          rad          rad          rad          M            u      "); for ( n = 8 ; n <= NumStabCases_ + NumberOfControlGroups_ ; n++ ) fprintf(StabFile,"      rad    ");     fprintf(StabFile,"\n");
   
    fprintf(StabFile,"#\n");
    
    fprintf(StabFile,"CFx    "); fprintf(StabFile,"%12.7f ",CFtxForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCFx_wrt[n]); }; fprintf(StabFile,"\n");
    fprintf(StabFile,"CFy    "); fprintf(StabFile,"%12.7f ",CFtyForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCFy_wrt[n]); }; fprintf(StabFile,"\n");
    fprintf(StabFile,"CFz    "); fprintf(StabFile,"%12.7f ",CFtzForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCFz_wrt[n]); }; fprintf(StabFile,"\n");
                                                             
    fprintf(StabFile,"CMx    "); fprintf(StabFile,"%12.7f ",CMtxForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMx_wrt[n]); }; fprintf(StabFile,"\n");
    fprintf(StabFile,"CMy    "); fprintf(StabFile,"%12.7f ",CMtyForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMy_wrt[n]); }; fprintf(StabFile,"\n");
    fprintf(StabFile,"CMz    "); fprintf(StabFile,"%12.7f ",CMtzForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMz_wrt[n]); }; fprintf(StabFile,"\n");
                                                             
    fprintf(StabFile,"CL     "); fprintf(StabFile,"%12.7f ",CLtForCase[1]);  for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCL_wrt[n]); };  fprintf(StabFile,"\n");
    fprintf(StabFile,"CD     "); fprintf(StabFile,"%12.7f ",CDtForCase[1]);  for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCD_wrt[n]); };  fprintf(StabFile,"\n");
    fprintf(StabFile,"CS     "); fprintf(StabFile,"%12.7f ",CStForCase[1]);  for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCS_wrt[n]); };  fprintf(StabFile,"\n");
                                                            
    fprintf(StabFile,"CMl    "); fprintf(StabFile,"%12.7f ",CMtlForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMl_wrt[n]); }; fprintf(StabFile,"\n");
    fprintf(StabFile,"CMm    "); fprintf(StabFile,"%12.7f ",CMtmForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMm_wrt[n]); }; fprintf(StabFile,"\n");
    fprintf(StabFile,"CMn    "); fprintf(StabFile,"%12.7f ",CMtnForCase[1]); for ( n = 2 ; n <= NumStabCases_ + NumberOfControlGroups_ + 1 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMn_wrt[n]); }; fprintf(StabFile,"\n");

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
    fprintf(VorviewFlt," CLo:          %15f \n",CLtForCase[1]);
    fprintf(VorviewFlt," CDo:          %15f \n",CDtForCase[1] + CDoForCase[1]);
    fprintf(VorviewFlt," CYo:          %15f \n",CFtyForCase[1]);
    fprintf(VorviewFlt," Clo:          %15f \n",CMtlForCase[1]);
    fprintf(VorviewFlt," Cmo:          %15f \n",CMtmForCase[1]);
    fprintf(VorviewFlt," Cno:          %15f \n",CMtnForCase[1]);
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
    char CaseType[MAX_CHAR_SIZE];
    char caseTypeFormatStr[] = "%-22s +%5.3lf %-9s";
    char StabFileName[MAX_CHAR_SIZE];

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
          
          while ( p <= VSPAERO().VSPGeom().NumberOfControlSurfaces() && !Found ) {
             
             printf("Checking: %s \n",VSPAERO().VSPGeom().ControlSurface(p).Name());fflush(NULL);
  
             if ( strstr(VSPAERO().VSPGeom().ControlSurface(p).Name(), ControlSurfaceGroup_[i].ControlSurface_Name(j)) != NULL ) {
      
                Found = 1;
               
                ControlSurfaceGroup_[i].pCFix_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCFix_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCFiy_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCFiy_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCFiz_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCFiz_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                                                  
                ControlSurfaceGroup_[i].pCMix_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCMix_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMiy_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCMiy_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMiz_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCMiz_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                                                  
                ControlSurfaceGroup_[i].pCFox_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCFox_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCFoy_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCFoy_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCFoz_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCFoz_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                                                  
                ControlSurfaceGroup_[i].pCMox_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCMox_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMoy_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCMoy_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMoz_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCMoz_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 


                ControlSurfaceGroup_[i].pCLi_pDelta()  += VSPAERO().VSPGeom().ControlSurface(p).pCLi_pDelta()  * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCDi_pDelta()  += VSPAERO().VSPGeom().ControlSurface(p).pCDi_pDelta()  * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCSi_pDelta()  += VSPAERO().VSPGeom().ControlSurface(p).pCSi_pDelta()  * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                                                  
                ControlSurfaceGroup_[i].pCMli_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCMli_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMmi_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCMmi_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMni_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCMni_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                                                  
                ControlSurfaceGroup_[i].pCLo_pDelta()  += VSPAERO().VSPGeom().ControlSurface(p).pCLo_pDelta()  * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCDo_pDelta()  += VSPAERO().VSPGeom().ControlSurface(p).pCDo_pDelta()  * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCSo_pDelta()  += VSPAERO().VSPGeom().ControlSurface(p).pCSo_pDelta()  * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                                                  
                ControlSurfaceGroup_[i].pCMlo_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCMlo_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMmo_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCMmo_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 
                ControlSurfaceGroup_[i].pCMno_pDelta() += VSPAERO().VSPGeom().ControlSurface(p).pCMno_pDelta() * ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j); 

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
        
    VSPAERO().WriteCaseHeader(StabFile);
    
    if ( VSPAERO().AdjointSolutionForceType() == ADJOINT_INVISCID_FORCES             ) NumberOfCases = 1;
    if ( VSPAERO().AdjointSolutionForceType() == ADJOINT_VISCOUS_FORCES              ) NumberOfCases = 1;
    if ( VSPAERO().AdjointSolutionForceType() == ADJOINT_TOTAL_FORCES                ) NumberOfCases = 1;
    if ( VSPAERO().AdjointSolutionForceType() == ADJOINT_INVISCID_AND_VISCOUS_FORCES ) NumberOfCases = 3;
       
    for ( Case = 1 ; Case <= NumberOfCases ; Case++ ) {
           
       // Write out column labels
       
       fprintf(StabFile,"#\n");

       // Total forces header
        
       if (   VSPAERO().AdjointSolutionForceType() == ADJOINT_TOTAL_FORCES                               ||
            ( VSPAERO().AdjointSolutionForceType() == ADJOINT_INVISCID_AND_VISCOUS_FORCES && Case == 1 )    ) {
   
                         // 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012          
          fprintf(StabFile,"   Total        Total        Total        Total        Total        Total        Total        Total        Total        Total        Total        Total \n");    
          fprintf(StabFile,"    CFx          CFy          CFz          CMx          CMy          CMz          CL           CD           CS           CMl          CMm          CMn\n");
          fprintf(StabFile,"#\n");

          fprintf(StabFile,"%12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f \n",
                  VSPAERO().CFix() + VSPAERO().CFox(),
                  VSPAERO().CFiy() + VSPAERO().CFoy(),
                  VSPAERO().CFiz() + VSPAERO().CFoz(),
                         
                  VSPAERO().CMix() + VSPAERO().CMox(),
                  VSPAERO().CMiy() + VSPAERO().CMoy(),
                  VSPAERO().CMiz() + VSPAERO().CMoz(),
                         
                  VSPAERO().CLi() + VSPAERO().CLo(),
                  VSPAERO().CDi() + VSPAERO().CDo(),
                  VSPAERO().CSi() + VSPAERO().CSo(),
                  
                  VSPAERO().CMli() + VSPAERO().CMlo(),
                  VSPAERO().CMmi() + VSPAERO().CMmo(),
                  VSPAERO().CMni() + VSPAERO().CMno());     

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
         
          fprintf(StabFile,"CFx    "); PRINT_STAB_LINE( VSPAERO().CFtx(),
                                                       VSPAERO().pCFtx_pAlpha(),
                                                       VSPAERO().pCFtx_pBeta(),
                                                       VSPAERO().pCFtx_pP(),
                                                       VSPAERO().pCFtx_pQ(),
                                                       VSPAERO().pCFtx_pR(),
                                                       VSPAERO().pCFtx_pMach(),
                                                       VSPAERO().pCFtx_pMach()*VSPAERO().Mach());
          
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFtx_pDelta()); }; fprintf(StabFile,"\n");
             
          fprintf(StabFile,"CFy    "); PRINT_STAB_LINE( VSPAERO().CFty(),
                                                       VSPAERO().pCFty_pAlpha(),
                                                       VSPAERO().pCFty_pBeta(),
                                                       VSPAERO().pCFty_pP(),
                                                       VSPAERO().pCFty_pQ(),
                                                       VSPAERO().pCFty_pR(),
                                                       VSPAERO().pCFty_pMach(),
                                                       VSPAERO().pCFty_pMach()*VSPAERO().Mach());
         

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) {fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFty_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CFz    "); PRINT_STAB_LINE( VSPAERO().CFtz(),
                                                       VSPAERO().pCFtz_pAlpha(),
                                                       VSPAERO().pCFtz_pBeta(),
                                                       VSPAERO().pCFtz_pP(),
                                                       VSPAERO().pCFtz_pQ(),
                                                       VSPAERO().pCFtz_pR(),
                                                       VSPAERO().pCFtz_pMach(),
                                                       VSPAERO().pCFtz_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFtz_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMx    "); PRINT_STAB_LINE( VSPAERO().CMtx(),
                                                       VSPAERO().pCMtx_pAlpha(),
                                                       VSPAERO().pCMtx_pBeta(),
                                                       VSPAERO().pCMtx_pP(),
                                                       VSPAERO().pCMtx_pQ(),
                                                       VSPAERO().pCMtx_pR(),
                                                       VSPAERO().pCMtx_pMach(),
                                                       VSPAERO().pCMtx_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMtx_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMy    "); PRINT_STAB_LINE( VSPAERO().CMty(),
                                                       VSPAERO().pCMty_pAlpha(),
                                                       VSPAERO().pCMty_pBeta(),
                                                       VSPAERO().pCMty_pP(),
                                                       VSPAERO().pCMty_pQ(),
                                                       VSPAERO().pCMty_pR(),
                                                       VSPAERO().pCMty_pMach(),
                                                       VSPAERO().pCMty_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMty_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMz    "); PRINT_STAB_LINE( VSPAERO().CMtz(),
                                                       VSPAERO().pCMtz_pAlpha(),
                                                       VSPAERO().pCMtz_pBeta(),
                                                       VSPAERO().pCMtz_pP(),
                                                       VSPAERO().pCMtz_pQ(),
                                                       VSPAERO().pCMtz_pR(),
                                                       VSPAERO().pCMtz_pMach(),
                                                       VSPAERO().pCMtz_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMtz_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CL     "); PRINT_STAB_LINE( VSPAERO().CLt(),
                                                       VSPAERO().pCLt_pAlpha(),
                                                       VSPAERO().pCLt_pBeta(),
                                                       VSPAERO().pCLt_pP(),
                                                       VSPAERO().pCLt_pQ(),
                                                       VSPAERO().pCLt_pR(),
                                                       VSPAERO().pCLt_pMach(),
                                                       VSPAERO().pCLt_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCLt_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CD     "); PRINT_STAB_LINE( VSPAERO().CDt(),
                                                       VSPAERO().pCDt_pAlpha(),
                                                       VSPAERO().pCDt_pBeta(),
                                                       VSPAERO().pCDt_pP(),
                                                       VSPAERO().pCDt_pQ(),
                                                       VSPAERO().pCDt_pR(),
                                                       VSPAERO().pCDt_pMach(),
                                                       VSPAERO().pCDt_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCDt_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CS     "); PRINT_STAB_LINE( VSPAERO().CSt(),
                                                       VSPAERO().pCSt_pAlpha(),
                                                       VSPAERO().pCSt_pBeta(),
                                                       VSPAERO().pCSt_pP(),
                                                       VSPAERO().pCSt_pQ(),
                                                       VSPAERO().pCSt_pR(),
                                                       VSPAERO().pCSt_pMach(),
                                                       VSPAERO().pCSt_pMach()*VSPAERO().Mach());
         

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCSt_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CML    "); PRINT_STAB_LINE( VSPAERO().CMlt(),
                                                       VSPAERO().pCMlt_pAlpha(),
                                                       VSPAERO().pCMlt_pBeta(),
                                                       VSPAERO().pCMlt_pP(),
                                                       VSPAERO().pCMlt_pQ(),
                                                       VSPAERO().pCMlt_pR(),
                                                       VSPAERO().pCMlt_pMach(),
                                                       VSPAERO().pCMlt_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMlt_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMm    "); PRINT_STAB_LINE( VSPAERO().CMmt(),
                                                       VSPAERO().pCMmt_pAlpha(),
                                                       VSPAERO().pCMmt_pBeta(),
                                                       VSPAERO().pCMmt_pP(),
                                                       VSPAERO().pCMmt_pQ(),
                                                       VSPAERO().pCMmt_pR(),
                                                       VSPAERO().pCMmt_pMach(),
                                                       VSPAERO().pCMmt_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMmt_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMn    "); PRINT_STAB_LINE( VSPAERO().CMnt(),
                                                       VSPAERO().pCMnt_pAlpha(),
                                                       VSPAERO().pCMnt_pBeta(),
                                                       VSPAERO().pCMnt_pP(),
                                                       VSPAERO().pCMnt_pQ(),
                                                       VSPAERO().pCMnt_pR(),
                                                       VSPAERO().pCMnt_pMach(),
                                                       VSPAERO().pCMnt_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMnt_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"#\n");
          fprintf(StabFile,"#\n");
          fprintf(StabFile,"#\n");
                                
       }
   
       // Inviscid forces header
       
       if (   VSPAERO().AdjointSolutionForceType() == ADJOINT_INVISCID_FORCES                            ||
            ( VSPAERO().AdjointSolutionForceType() == ADJOINT_INVISCID_AND_VISCOUS_FORCES && Case == 2 )    ) {
   
                         // 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012          
          fprintf(StabFile,"  Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid     Inviscid \n");    
          fprintf(StabFile,"    CFx          CFy          CFz          CMx          CMy          CMz          CL           CD           CS           CMl          CMm          CMn\n");
          fprintf(StabFile,"#\n");

          fprintf(StabFile,"%12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f \n",
                  VSPAERO().CFix(),
                  VSPAERO().CFiy(),
                  VSPAERO().CFiz(),
                         
                  VSPAERO().CMix(),
                  VSPAERO().CMiy(),
                  VSPAERO().CMiz(),
                         
                  VSPAERO().CLi(),
                  VSPAERO().CDi(),
                  VSPAERO().CSi(),
                  
                  VSPAERO().CMli(),
                  VSPAERO().CMmi(),
                  VSPAERO().CMni());     

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
         
          fprintf(StabFile,"CFx    "); PRINT_STAB_LINE( VSPAERO().CFix(),
                                                       VSPAERO().pCFix_pAlpha(),
                                                       VSPAERO().pCFix_pBeta(),
                                                       VSPAERO().pCFix_pP(),
                                                       VSPAERO().pCFix_pQ(),
                                                       VSPAERO().pCFix_pR(),
                                                       VSPAERO().pCFix_pMach(),
                                                       VSPAERO().pCFix_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFix_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CFy    "); PRINT_STAB_LINE( VSPAERO().CFiy(),
                                                       VSPAERO().pCFiy_pAlpha(),
                                                       VSPAERO().pCFiy_pBeta(),
                                                       VSPAERO().pCFiy_pP(),
                                                       VSPAERO().pCFiy_pQ(),
                                                       VSPAERO().pCFiy_pR(),
                                                       VSPAERO().pCFiy_pMach(),
                                                       VSPAERO().pCFiy_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFiy_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CFz    "); PRINT_STAB_LINE( VSPAERO().CFiz(),
                                                       VSPAERO().pCFiz_pAlpha(),
                                                       VSPAERO().pCFiz_pBeta(),
                                                       VSPAERO().pCFiz_pP(),
                                                       VSPAERO().pCFiz_pQ(),
                                                       VSPAERO().pCFiz_pR(),
                                                       VSPAERO().pCFiz_pMach(),
                                                       VSPAERO().pCFiz_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFiz_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMx    "); PRINT_STAB_LINE( VSPAERO().CMix(),
                                                       VSPAERO().pCMix_pAlpha(),
                                                       VSPAERO().pCMix_pBeta(),
                                                       VSPAERO().pCMix_pP(),
                                                       VSPAERO().pCMix_pQ(),
                                                       VSPAERO().pCMix_pR(),
                                                       VSPAERO().pCMix_pMach(),
                                                       VSPAERO().pCMix_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMix_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMy    "); PRINT_STAB_LINE( VSPAERO().CMiy(),
                                                       VSPAERO().pCMiy_pAlpha(),
                                                       VSPAERO().pCMiy_pBeta(),
                                                       VSPAERO().pCMiy_pP(),
                                                       VSPAERO().pCMiy_pQ(),
                                                       VSPAERO().pCMiy_pR(),
                                                       VSPAERO().pCMiy_pMach(),
                                                       VSPAERO().pCMiy_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMiy_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMz    "); PRINT_STAB_LINE( VSPAERO().CMiz(),
                                                       VSPAERO().pCMiz_pAlpha(),
                                                       VSPAERO().pCMiz_pBeta(),
                                                       VSPAERO().pCMiz_pP(),
                                                       VSPAERO().pCMiz_pQ(),
                                                       VSPAERO().pCMiz_pR(),
                                                       VSPAERO().pCMiz_pMach(),
                                                       VSPAERO().pCMiz_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMiz_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CL     "); PRINT_STAB_LINE( VSPAERO().CLi(),
                                                       VSPAERO().pCLi_pAlpha(),
                                                       VSPAERO().pCLi_pBeta(),
                                                       VSPAERO().pCLi_pP(),
                                                       VSPAERO().pCLi_pQ(),
                                                       VSPAERO().pCLi_pR(),
                                                       VSPAERO().pCLi_pMach(),
                                                       VSPAERO().pCLi_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCLi_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CD     "); PRINT_STAB_LINE( VSPAERO().CDi(),
                                                       VSPAERO().pCDi_pAlpha(),
                                                       VSPAERO().pCDi_pBeta(),
                                                       VSPAERO().pCDi_pP(),
                                                       VSPAERO().pCDi_pQ(),
                                                       VSPAERO().pCDi_pR(),
                                                       VSPAERO().pCDi_pMach(),
                                                       VSPAERO().pCDi_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCDi_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CS     "); PRINT_STAB_LINE( VSPAERO().CSi(),
                                                       VSPAERO().pCSi_pAlpha(),
                                                       VSPAERO().pCSi_pBeta(),
                                                       VSPAERO().pCSi_pP(),
                                                       VSPAERO().pCSi_pQ(),
                                                       VSPAERO().pCSi_pR(),
                                                       VSPAERO().pCSi_pMach(),
                                                       VSPAERO().pCSi_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCSi_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CML    "); PRINT_STAB_LINE( VSPAERO().CMli(),
                                                       VSPAERO().pCMli_pAlpha(),
                                                       VSPAERO().pCMli_pBeta(),
                                                       VSPAERO().pCMli_pP(),
                                                       VSPAERO().pCMli_pQ(),
                                                       VSPAERO().pCMli_pR(),
                                                       VSPAERO().pCMli_pMach(),
                                                       VSPAERO().pCMli_pMach()*VSPAERO().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMli_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMm    "); PRINT_STAB_LINE( VSPAERO().CMmi(),
                                                       VSPAERO().pCMmi_pAlpha(),
                                                       VSPAERO().pCMmi_pBeta(),
                                                       VSPAERO().pCMmi_pP(),
                                                       VSPAERO().pCMmi_pQ(),
                                                       VSPAERO().pCMmi_pR(),
                                                       VSPAERO().pCMmi_pMach(),
                                                       VSPAERO().pCMmi_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMmi_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMn    "); PRINT_STAB_LINE( VSPAERO().CMni(),
                                                       VSPAERO().pCMni_pAlpha(),
                                                       VSPAERO().pCMni_pBeta(),
                                                       VSPAERO().pCMni_pP(),
                                                       VSPAERO().pCMni_pQ(),
                                                       VSPAERO().pCMni_pR(),
                                                       VSPAERO().pCMni_pMach(),
                                                       VSPAERO().pCMni_pMach()*VSPAERO().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMni_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"#\n");
          fprintf(StabFile,"#\n");
          fprintf(StabFile,"#\n");
                                      
       }
   
   
       // Inviscid forces header
       
       if (   VSPAERO().AdjointSolutionForceType() == ADJOINT_VISCOUS_FORCES                             ||
            ( VSPAERO().AdjointSolutionForceType() == ADJOINT_INVISCID_AND_VISCOUS_FORCES && Case == 3 )    ) {
   
                         // 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012          
          fprintf(StabFile,"   Viscous      Viscous      Viscous      Viscous      Viscous      Viscous      Viscous      Viscous      Viscous      Viscous      Viscous      Viscous \n");    
          fprintf(StabFile,"    CFx          CFy          CFz          CMx          CMy          CMz          CL           CD           CS           CMl          CMm          CMn\n");
          fprintf(StabFile,"#\n");

          fprintf(StabFile,"%12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f \n",
                  VSPAERO().CFox(),
                  VSPAERO().CFoy(),
                  VSPAERO().CFoz(),
                  
                  VSPAERO().CMox(),
                  VSPAERO().CMoy(),
                  VSPAERO().CMoz(),
                         
                  VSPAERO().CLo(),
                  VSPAERO().CDo(),
                  VSPAERO().CSo(),
                  
                  VSPAERO().CMlo(),
                  VSPAERO().CMmo(),
                  VSPAERO().CMno());     

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
         
          fprintf(StabFile,"CFx    "); PRINT_STAB_LINE( VSPAERO().CFox(),
                                                       VSPAERO().pCFox_pAlpha(),
                                                       VSPAERO().pCFox_pBeta(),
                                                       VSPAERO().pCFox_pP(),
                                                       VSPAERO().pCFox_pQ(),
                                                       VSPAERO().pCFox_pR(),
                                                       VSPAERO().pCFox_pMach(),
                                                       VSPAERO().pCFox_pMach()*VSPAERO().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFox_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CFy    "); PRINT_STAB_LINE( VSPAERO().CFoy(),
                                                       VSPAERO().pCFoy_pAlpha(),
                                                       VSPAERO().pCFoy_pBeta(),
                                                       VSPAERO().pCFoy_pP(),
                                                       VSPAERO().pCFoy_pQ(),
                                                       VSPAERO().pCFoy_pR(),
                                                       VSPAERO().pCFoy_pMach(),
                                                       VSPAERO().pCFoy_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFoy_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CFz    "); PRINT_STAB_LINE( VSPAERO().CFoz(),
                                                       VSPAERO().pCFoz_pAlpha(),
                                                       VSPAERO().pCFoz_pBeta(),
                                                       VSPAERO().pCFoz_pP(),
                                                       VSPAERO().pCFoz_pQ(),
                                                       VSPAERO().pCFoz_pR(),
                                                       VSPAERO().pCFoz_pMach(),
                                                       VSPAERO().pCFoz_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCFoz_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMx    "); PRINT_STAB_LINE( VSPAERO().CMox(),
                                                       VSPAERO().pCMox_pAlpha(),
                                                       VSPAERO().pCMox_pBeta(),
                                                       VSPAERO().pCMox_pP(),
                                                       VSPAERO().pCMox_pQ(),
                                                       VSPAERO().pCMox_pR(),
                                                       VSPAERO().pCMox_pMach(),
                                                       VSPAERO().pCMox_pMach()*VSPAERO().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMox_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMy    "); PRINT_STAB_LINE( VSPAERO().CMoy(),
                                                       VSPAERO().pCMoy_pAlpha(),
                                                       VSPAERO().pCMoy_pBeta(),
                                                       VSPAERO().pCMoy_pP(),
                                                       VSPAERO().pCMoy_pQ(),
                                                       VSPAERO().pCMoy_pR(),
                                                       VSPAERO().pCMoy_pMach(),
                                                       VSPAERO().pCMoy_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMoy_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMz    "); PRINT_STAB_LINE( VSPAERO().CMoz(),
                                                       VSPAERO().pCMoz_pAlpha(),
                                                       VSPAERO().pCMoz_pBeta(),
                                                       VSPAERO().pCMoz_pP(),
                                                       VSPAERO().pCMoz_pQ(),
                                                       VSPAERO().pCMoz_pR(),
                                                       VSPAERO().pCMoz_pMach(),
                                                       VSPAERO().pCMoz_pMach()*VSPAERO().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMoz_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CL     "); PRINT_STAB_LINE( VSPAERO().CLo(),
                                                       VSPAERO().pCLo_pAlpha(),
                                                       VSPAERO().pCLo_pBeta(),
                                                       VSPAERO().pCLo_pP(),
                                                       VSPAERO().pCLo_pQ(),
                                                       VSPAERO().pCLo_pR(),
                                                       VSPAERO().pCLo_pMach(),
                                                       VSPAERO().pCLo_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCLo_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CD     "); PRINT_STAB_LINE( VSPAERO().CDo(),
                                                       VSPAERO().pCDo_pAlpha(),
                                                       VSPAERO().pCDo_pBeta(),
                                                       VSPAERO().pCDo_pP(),
                                                       VSPAERO().pCDo_pQ(),
                                                       VSPAERO().pCDo_pR(),
                                                       VSPAERO().pCDo_pMach(),
                                                       VSPAERO().pCDo_pMach()*VSPAERO().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCDo_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CS     "); PRINT_STAB_LINE( VSPAERO().CSo(),
                                                       VSPAERO().pCSo_pAlpha(),
                                                       VSPAERO().pCSo_pBeta(),
                                                       VSPAERO().pCSo_pP(),
                                                       VSPAERO().pCSo_pQ(),
                                                       VSPAERO().pCSo_pR(),
                                                       VSPAERO().pCSo_pMach(),
                                                       VSPAERO().pCSo_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCSo_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CML    "); PRINT_STAB_LINE( VSPAERO().CMlo(),
                                                       VSPAERO().pCMlo_pAlpha(),
                                                       VSPAERO().pCMlo_pBeta(),
                                                       VSPAERO().pCMlo_pP(),
                                                       VSPAERO().pCMlo_pQ(),
                                                       VSPAERO().pCMlo_pR(),
                                                       VSPAERO().pCMlo_pMach(),
                                                       VSPAERO().pCMlo_pMach()*VSPAERO().Mach());
         
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMlo_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMm    "); PRINT_STAB_LINE( VSPAERO().CMmo(),
                                                       VSPAERO().pCMmo_pAlpha(),
                                                       VSPAERO().pCMmo_pBeta(),
                                                       VSPAERO().pCMmo_pP(),
                                                       VSPAERO().pCMmo_pQ(),
                                                       VSPAERO().pCMmo_pR(),
                                                       VSPAERO().pCMmo_pMach(),
                                                       VSPAERO().pCMmo_pMach()*VSPAERO().Mach());

          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) { fprintf(StabFile,"%12.7f ",ControlSurfaceGroup_[i].pCMmo_pDelta()); }; fprintf(StabFile,"\n");
         
          fprintf(StabFile,"CMn    "); PRINT_STAB_LINE( VSPAERO().CMno(),
                                                       VSPAERO().pCMno_pAlpha(),
                                                       VSPAERO().pCMno_pBeta(),
                                                       VSPAERO().pCMno_pP(),
                                                       VSPAERO().pCMno_pQ(),
                                                       VSPAERO().pCMno_pR(),
                                                       VSPAERO().pCMno_pMach(),
                                                       VSPAERO().pCMno_pMach()*VSPAERO().Mach());
         
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

    SM = -1.0 * VSPAERO().pCMmt_pAlpha() / VSPAERO().pCLt_pAlpha(); // -1 * CMm_alpha / CL_alpha
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
    char StabFileName[MAX_CHAR_SIZE];
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
             
             VSPAERO().AngleOfBeta()   = BetaList_[i] * TORAD;
             VSPAERO().Mach()          = MachList_[j];  
             VSPAERO().AngleOfAttack() =  AoAList_[k] * TORAD;
      
             VSPAERO().RotationalRate_p() = 0.;
             VSPAERO().RotationalRate_q() = 0.;
             VSPAERO().RotationalRate_r() = 0.;

             // Set Control surface group deflection to un-perturbed control surface deflections

             ApplyControlDeflections();
             
             // Write out generic header file
                 
             VSPAERO().WriteCaseHeader(StabFile);

             // Unsteady solve
             
             if ( Case <= NumCases ) {
            
                VSPAERO().Solve(Case);
                
             }
             
             else {
                
                VSPAERO().Solve(-Case);
                
             }
                          
             // Calculate the damping derivatives

// Old method     
    
       //       CL_damp = (  VSPAERO().CL_Unsteady(16) -  VSPAERO().CL_Unsteady(32) )/( 2. * VSPAERO().ReducedFrequency() * VSPAERO().Unsteady_AngleMax() * PI / 180. );
       //       CD_damp = (  VSPAERO().CD_Unsteady(16) -  VSPAERO().CD_Unsteady(32) )/( 2. * VSPAERO().ReducedFrequency() * VSPAERO().Unsteady_AngleMax() * PI / 180. );
       //       CS_damp = (  VSPAERO().CS_Unsteady(16) -  VSPAERO().CS_Unsteady(32) )/( 2. * VSPAERO().ReducedFrequency() * VSPAERO().Unsteady_AngleMax() * PI / 180. );   
       //      CFx_damp = ( VSPAERO().CFx_Unsteady(16) - VSPAERO().CFx_Unsteady(32) )/( 2. * VSPAERO().ReducedFrequency() * VSPAERO().Unsteady_AngleMax() * PI / 180. );
       //      CFy_damp = ( VSPAERO().CFy_Unsteady(16) - VSPAERO().CFy_Unsteady(32) )/( 2. * VSPAERO().ReducedFrequency() * VSPAERO().Unsteady_AngleMax() * PI / 180. );
       //      CFz_damp = ( VSPAERO().CFz_Unsteady(16) - VSPAERO().CFz_Unsteady(32) )/( 2. * VSPAERO().ReducedFrequency() * VSPAERO().Unsteady_AngleMax() * PI / 180. );
       //      CMx_damp = ( VSPAERO().CMx_Unsteady(16) - VSPAERO().CMx_Unsteady(32) )/( 2. * VSPAERO().ReducedFrequency() * VSPAERO().Unsteady_AngleMax() * PI / 180. );
       //      CMy_damp = ( VSPAERO().CMy_Unsteady(16) - VSPAERO().CMy_Unsteady(32) )/( 2. * VSPAERO().ReducedFrequency() * VSPAERO().Unsteady_AngleMax() * PI / 180. );
       //      CMz_damp = ( VSPAERO().CMz_Unsteady(16) - VSPAERO().CMz_Unsteady(32) )/( 2. * VSPAERO().ReducedFrequency() * VSPAERO().Unsteady_AngleMax() * PI / 180. );        

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
             
             for ( p = VSPAERO().NumberOfTimeSteps()/2 ; p <= VSPAERO().NumberOfTimeSteps() ; p++ ) {
         
                 CL_avg += VSPAERO().VSPGeom().ComponentGroupList(1).CLi(p);
                 CD_avg += VSPAERO().VSPGeom().ComponentGroupList(1).CDi(p);
                 CS_avg += VSPAERO().VSPGeom().ComponentGroupList(1).CSi(p);
                 
                CFx_avg += VSPAERO().VSPGeom().ComponentGroupList(1).CFix(p);
                CFy_avg += VSPAERO().VSPGeom().ComponentGroupList(1).CFiy(p);
                CFz_avg += VSPAERO().VSPGeom().ComponentGroupList(1).CFiz(p);
                CMx_avg += VSPAERO().VSPGeom().ComponentGroupList(1).CMix(p);
                CMy_avg += VSPAERO().VSPGeom().ComponentGroupList(1).CMiy(p);
                CMz_avg += VSPAERO().VSPGeom().ComponentGroupList(1).CMiz(p);
            
             }
             
             // Calculate average
             
              CL_avg /= (VSPAERO().NumberOfTimeSteps()/2 + 1);
              CD_avg /= (VSPAERO().NumberOfTimeSteps()/2 + 1);
              CS_avg /= (VSPAERO().NumberOfTimeSteps()/2 + 1);
              
             CFx_avg /= (VSPAERO().NumberOfTimeSteps()/2 + 1);
             CFy_avg /= (VSPAERO().NumberOfTimeSteps()/2 + 1);
             CFz_avg /= (VSPAERO().NumberOfTimeSteps()/2 + 1);
             CMx_avg /= (VSPAERO().NumberOfTimeSteps()/2 + 1);
             CMy_avg /= (VSPAERO().NumberOfTimeSteps()/2 + 1);
             CMz_avg /= (VSPAERO().NumberOfTimeSteps()/2 + 1);
             
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
             
             for ( p = VSPAERO().NumberOfTimeSteps()/2 + 1 ; p <= VSPAERO().NumberOfTimeSteps() ; p++ ) {
         
                 pm  = p;
                 pm1 = p - 1;
                 
                 if ( p == VSPAERO().NumberOfTimeSteps() ) pm1 = VSPAERO().NumberOfTimeSteps()/2;
                 
                 T = VSPAERO().DeltaTime() * 0.5 * (double) (2*(p-1) + 1);
                 
                 Theta = VSPAERO().Unsteady_AngleRate() * T;

                 CL_damp += 2.*( 0.5*(  VSPAERO().VSPGeom().ComponentGroupList(1).CLi(pm1) +  VSPAERO().VSPGeom().ComponentGroupList(1).CLi(pm) ) -  CL_avg ) * cos( Theta ) * VSPAERO().DeltaTime();
                 CD_damp += 2.*( 0.5*(  VSPAERO().VSPGeom().ComponentGroupList(1).CDi(pm1) +  VSPAERO().VSPGeom().ComponentGroupList(1).CDi(pm) ) -  CD_avg ) * cos( Theta ) * VSPAERO().DeltaTime();
                 CS_damp += 2.*( 0.5*(  VSPAERO().VSPGeom().ComponentGroupList(1).CSi(pm1) +  VSPAERO().VSPGeom().ComponentGroupList(1).CSi(pm) ) -  CS_avg ) * cos( Theta ) * VSPAERO().DeltaTime();
                                                                                                    
                CFx_damp += 2.*( 0.5*(  VSPAERO().VSPGeom().ComponentGroupList(1).CFix(pm1) + VSPAERO().VSPGeom().ComponentGroupList(1).CFix(pm) ) - CFx_avg ) * cos( Theta ) * VSPAERO().DeltaTime();
                CFy_damp += 2.*( 0.5*(  VSPAERO().VSPGeom().ComponentGroupList(1).CFiy(pm1) + VSPAERO().VSPGeom().ComponentGroupList(1).CFiy(pm) ) - CFy_avg ) * cos( Theta ) * VSPAERO().DeltaTime();
                CFz_damp += 2.*( 0.5*(  VSPAERO().VSPGeom().ComponentGroupList(1).CFiz(pm1) + VSPAERO().VSPGeom().ComponentGroupList(1).CFiz(pm) ) - CFz_avg ) * cos( Theta ) * VSPAERO().DeltaTime();
                CMx_damp += 2.*( 0.5*(  VSPAERO().VSPGeom().ComponentGroupList(1).CMix(pm1) + VSPAERO().VSPGeom().ComponentGroupList(1).CMix(pm) ) - CMx_avg ) * cos( Theta ) * VSPAERO().DeltaTime();
                CMy_damp += 2.*( 0.5*(  VSPAERO().VSPGeom().ComponentGroupList(1).CMiy(pm1) + VSPAERO().VSPGeom().ComponentGroupList(1).CMiy(pm) ) - CMy_avg ) * cos( Theta ) * VSPAERO().DeltaTime();
                CMz_damp += 2.*( 0.5*(  VSPAERO().VSPGeom().ComponentGroupList(1).CMiz(pm1) + VSPAERO().VSPGeom().ComponentGroupList(1).CMiz(pm) ) - CMz_avg ) * cos( Theta ) * VSPAERO().DeltaTime();

             }
                  
             Fact = 0.5 * VSPAERO().DeltaTime() * VSPAERO().NumberOfTimeSteps() * VSPAERO().ReducedFrequency() * VSPAERO().Unsteady_AngleMax() * PI / 180.;
              
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
 //  VSPAERO().AngleOfBeta()   = BetaList_[1] * TORAD;
 //  VSPAERO().Mach()          = MachList_[1];  
 //  VSPAERO().AngleOfAttack() =  AoAList_[1] * TORAD;
 //  
 //  VSPAERO().RotationalRate_p() = 0.;
 //  VSPAERO().RotationalRate_q() = 0.;
 //  VSPAERO().RotationalRate_r() = 0.;
 //     
 //   VSPAERO().WriteOutNoiseFiles(NumCases);

}

/*##############################################################################
#                                                                              #
#                           CalculateAerodynamicCenter                         #
#                                                                              #
##############################################################################*/

void CalculateAerodynamicCenter(void)
{

    int ic, jc, kc, Case;
    double DeltaXcg;
    char StabFileName[MAX_CHAR_SIZE];
    
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
             
             snprintf(VSPAERO().CaseString(),MAX_CHAR_SIZE*sizeof(char),"Base Aero         ");
                          
             VSPAERO().Mach()          = Mach_;
             VSPAERO().AngleOfAttack() =  AoA_ * TORAD;
             VSPAERO().AngleOfBeta()   = Beta_ * TORAD;
      
             VSPAERO().RotationalRate_p() = 0.;
             VSPAERO().RotationalRate_q() = 0.;
             VSPAERO().RotationalRate_r() = 0.;
             
             VSPAERO().SaveRestartFile() = VSPAERO().DoRestart() = 0;
             
             VSPAERO().Solve(1);

             Case = 1;

             // KJ inviscid forces
                          
             CLiForCase[Case]  =  VSPAERO().CLi(); 
             CDiForCase[Case]  =  VSPAERO().CDi();       
             CSiForCase[Case]  =  VSPAERO().CSi();        
                                  
             CFixForCase[Case] =  VSPAERO().CFix();
             CFiyForCase[Case] =  VSPAERO().CFiy();       
             CFizForCase[Case] =  VSPAERO().CFiz();       
                                  
             CMixForCase[Case] =  VSPAERO().CMix();      
             CMiyForCase[Case] =  VSPAERO().CMiy();      
             CMizForCase[Case] =  VSPAERO().CMiz();    
              
             CMilForCase[Case] = -VSPAERO().CMix();      
             CMimForCase[Case] =  VSPAERO().CMiy();      
             CMinForCase[Case] = -VSPAERO().CMiz();  

             // Wake inviscid forces
             
             CLwForCase[Case]  =  VSPAERO().CLiw(); 
             CDwForCase[Case]  =  VSPAERO().CDiw();       
             CSwForCase[Case]  =  VSPAERO().CSiw();        
                                             
             CFwxForCase[Case] =  VSPAERO().CFiwx();
             CFwyForCase[Case] =  VSPAERO().CFiwy();       
             CFwzForCase[Case] =  VSPAERO().CFiwz();       

             // Viscous forces and moments

             CLoForCase[Case]  =  VSPAERO().CLo(); 
             CDoForCase[Case]  =  VSPAERO().CDo();       
             CSoForCase[Case]  =  VSPAERO().CSo();        
                                  
             CFoxForCase[Case] =  VSPAERO().CFox();
             CFoyForCase[Case] =  VSPAERO().CFoy();       
             CFozForCase[Case] =  VSPAERO().CFoz();       
                                  
             CMoxForCase[Case] =  VSPAERO().CMox();      
             CMoyForCase[Case] =  VSPAERO().CMoy();      
             CMozForCase[Case] =  VSPAERO().CMoz();    
             
             CMolForCase[Case] = -VSPAERO().CMox();      
             CMomForCase[Case] =  VSPAERO().CMoy();      
             CMonForCase[Case] = -VSPAERO().CMoz();       
          
             // Total forces and moments, based on wake forces
             
             CLtForCase[Case]  =  VSPAERO().CLo() + VSPAERO().CLiw(); 
             CDtForCase[Case]  =  VSPAERO().CDo() + VSPAERO().CDiw();         
             CStForCase[Case]  =  VSPAERO().CSo() + VSPAERO().CSiw();         
                             
             CFtxForCase[Case] =  VSPAERO().CFox() + VSPAERO().CFiwx();
             CFtyForCase[Case] =  VSPAERO().CFoy() + VSPAERO().CFiwy();      
             CFtzForCase[Case] =  VSPAERO().CFoz() + VSPAERO().CFiwz();       
                              
             CMtxForCase[Case] =  VSPAERO().CMox() + VSPAERO().CMix();           
             CMtyForCase[Case] =  VSPAERO().CMoy() + VSPAERO().CMiy();         
             CMtzForCase[Case] =  VSPAERO().CMoz() + VSPAERO().CMiz(); 
             
             CMtlForCase[Case] = -VSPAERO().CMox() - VSPAERO().CMix();           
             CMtmForCase[Case] =  VSPAERO().CMoy() + VSPAERO().CMiy();           
             CMtnForCase[Case] = -VSPAERO().CMoz() - VSPAERO().CMiz();     
                     
             // Solve the perturbed case
             
             snprintf(VSPAERO().CaseString(),MAX_CHAR_SIZE*sizeof(char),"Alpha      +%5.3lf",1.);
                   
             VSPAERO().Mach()          = Mach_;
             VSPAERO().AngleOfAttack() =  ( AoA_ + 1.) * TORAD;
             VSPAERO().AngleOfBeta()   = Beta_ * TORAD;
      
             VSPAERO().RotationalRate_p() = 0.;
             VSPAERO().RotationalRate_q() = 0.;
             VSPAERO().RotationalRate_r() = 0.;
                          
             VSPAERO().SaveRestartFile() = VSPAERO().DoRestart() = 0;
             
             VSPAERO().Solve(-2);

             Case = 2;

             // KJ inviscid forces
                          
             CLiForCase[Case]  =  VSPAERO().CLi(); 
             CDiForCase[Case]  =  VSPAERO().CDi();       
             CSiForCase[Case]  =  VSPAERO().CSi();        
                                  
             CFixForCase[Case] =  VSPAERO().CFix();
             CFiyForCase[Case] =  VSPAERO().CFiy();       
             CFizForCase[Case] =  VSPAERO().CFiz();       
                                  
             CMixForCase[Case] =  VSPAERO().CMix();      
             CMiyForCase[Case] =  VSPAERO().CMiy();      
             CMizForCase[Case] =  VSPAERO().CMiz();    
              
             CMilForCase[Case] = -VSPAERO().CMix();      
             CMimForCase[Case] =  VSPAERO().CMiy();      
             CMinForCase[Case] = -VSPAERO().CMiz();  

             // Wake inviscid forces
             
             CLwForCase[Case]  =  VSPAERO().CLiw(); 
             CDwForCase[Case]  =  VSPAERO().CDiw();       
             CSwForCase[Case]  =  VSPAERO().CSiw();        
                                             
             CFwxForCase[Case] =  VSPAERO().CFiwx();
             CFwyForCase[Case] =  VSPAERO().CFiwy();       
             CFwzForCase[Case] =  VSPAERO().CFiwz();       

             // Viscous forces and moments

             CLoForCase[Case]  =  VSPAERO().CLo(); 
             CDoForCase[Case]  =  VSPAERO().CDo();       
             CSoForCase[Case]  =  VSPAERO().CSo();        
                                  
             CFoxForCase[Case] =  VSPAERO().CFox();
             CFoyForCase[Case] =  VSPAERO().CFoy();       
             CFozForCase[Case] =  VSPAERO().CFoz();       
                                  
             CMoxForCase[Case] =  VSPAERO().CMox();      
             CMoyForCase[Case] =  VSPAERO().CMoy();      
             CMozForCase[Case] =  VSPAERO().CMoz();    
             
             CMolForCase[Case] = -VSPAERO().CMox();      
             CMomForCase[Case] =  VSPAERO().CMoy();      
             CMonForCase[Case] = -VSPAERO().CMoz();       
          
             // Total forces and moments, based on wake forces
             
             CLtForCase[Case]  =  VSPAERO().CLo() + VSPAERO().CLiw(); 
             CDtForCase[Case]  =  VSPAERO().CDo() + VSPAERO().CDiw();         
             CStForCase[Case]  =  VSPAERO().CSo() + VSPAERO().CSiw();         
                             
             CFtxForCase[Case] =  VSPAERO().CFox() + VSPAERO().CFiwx();
             CFtyForCase[Case] =  VSPAERO().CFoy() + VSPAERO().CFiwy();      
             CFtzForCase[Case] =  VSPAERO().CFoz() + VSPAERO().CFiwz();       
                              
             CMtxForCase[Case] =  VSPAERO().CMox() + VSPAERO().CMix();           
             CMtyForCase[Case] =  VSPAERO().CMoy() + VSPAERO().CMiy();         
             CMtzForCase[Case] =  VSPAERO().CMoz() + VSPAERO().CMiz(); 
             
             CMtlForCase[Case] = -VSPAERO().CMox() - VSPAERO().CMix();           
             CMtmForCase[Case] =  VSPAERO().CMoy() + VSPAERO().CMiy();           
             CMtnForCase[Case] = -VSPAERO().CMoz() - VSPAERO().CMiz();     

             // Calculate aero center shift
             
             DeltaXcg = -( CMtyForCase[2] - CMtyForCase[1] ) / ( CFtzForCase[2] - CFtzForCase[1] ) * Cref_;
             
             VSPAERO().WriteCaseHeader(StabFile);

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
    char TestFileName[MAX_CHAR_SIZE];
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
    
    VSPAERO().AngleOfBeta()   = BetaList_[1] * TORAD;
    VSPAERO().Mach()          = MachList_[1];  
    VSPAERO().AngleOfAttack() =  AoAList_[1] * TORAD;
    
    VSPAERO().RotationalRate_p() = 0.;
    VSPAERO().RotationalRate_q() = 0.;
    VSPAERO().RotationalRate_r() = 0.;    
            
    // Loop over the grid nodes

    TempXYZ = new double[3*VSPAERO().VSPGeom().Grid(0).NumberOfNodes() + 1];
    
    for ( j = 1 ; j <= VSPAERO().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
    
       TempXYZ[3*j-2] = VSPAERO().VSPGeom().Grid(0).NodeList(j).x();
       TempXYZ[3*j-1] = VSPAERO().VSPGeom().Grid(0).NodeList(j).y();
       TempXYZ[3*j  ] = VSPAERO().VSPGeom().Grid(0).NodeList(j).z();
       
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
    fprintf(FiniteDiffFile,"     1              2                    3                    4                    5                    6                    7                    8                    9                   10                   11                   12                   13                   14                   15                   16                   17                   18                  19                    20                   21                   22                  23                   24                   25                   26                   27                   28                   29                   30                   31                   32                   33                   34                   35                   36                  37                    28                   39                   40                   41                   42                   43                   44                   45                   46                   47                   48                   49                   50                   51                   52                   53                   54                  55                    56                   57                   58                   59                   60                   61                   62                   63                   64                   64                   66                   67                   68                   69                   70                   71                   72                   73                   74                   75                   76 \n");
    fprintf(FiniteDiffFile,"   Node             x                    y                    z                  pFxpX                pFxpY                pFxpZ                pFypX                pFypY                pFypZ                pFzpX                pFzpY                pFzpZ                pMxpX                pMxpY                pMxpZ                pMypX                pMypY               pMypZ                 pMzpX                pMzpY                pMzpZ               ");             
    fprintf(FiniteDiffFile,"pFxpX_i              pFxpY_i              pFxpZ_i              pFypX_i              pFypY_i              pFypZ_i              pFzpX_i              pFzpY_i              pFzpZ_i              pMxpX_i              pMxpY_i              pMxpZ_i              pMypX_i              pMyPyi              pMypZ_i               pMzpX_i              pMzpY_i              pMzpZ_i              ");             
    fprintf(FiniteDiffFile,"pFxpX_o              pFxpY_o              pFxpZ_o              pFypX_o              pFypY_o              pFypZ_o              pFzpX_o              pFzpY_o              pFzpZ_o              pMxpX_o              pMxpY_o              pMxpZ_o              pMypX_o              pMyPyi              pMypZ_o               pMzpX_o              pMzpY_o              pMzpZ_o              ");             
    fprintf(FiniteDiffFile,"pFxpX_w              pFxpY_w              pFxpZ_w              pFypX_w              pFypY_w              pFypZ_w              pFzpX_w              pFzpY_w              pFzpZ_w              pFxpX_wt             pFxpY_wt             pFxpZ_wt             pFypX_wt             pFypY_wt             pFypZ_wt             pFzpX_wt             pFzpY_wt             pFzpZ_wt             \n");             
        

   
    // Do initial solve to set things up
    
  //  Case++;
      
  //  VSPAERO().Solve(Case);    
    
    
    for ( j = 1 ; j <= VSPAERO().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
    
       VSPAERO().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
       VSPAERO().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
       VSPAERO().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
       
    }         
              
  //  VSPAERO().UpdateMeshes();
              
    Delta = 1.e-6;    
    
    if ( DoUnsteadyAnalysis_ ) {
       
       Delta = 1.e-6;         
       
 //     VSPAERO().FreezeMultiPoleAtIteration() = 0;


    }  
    
    // This needs to be set to either 0, 1, 2 ... to specify the group forces to use
    // 0 ... is the total vehicle forces, 1, 2 ... N is the nth group forces/moments
     
    LoadGroup = FiniteDiffTestLoadGroup_; 
                  
    for ( i = FiniteDiffTestStartNode_ ; i <= VSPAERO().VSPGeom().Grid(0).NumberOfSurfaceNodes() ; i += FiniteDiffTestStepSize_ ) {

    // X
    
        printf("Working on node %d of %d \n",i,VSPAERO().VSPGeom().Grid(0).NumberOfNodes());
        
        // + 
        
           Case++;
             
           VSPAERO().VSPGeom().Grid(0).NodeList(i).x() += Delta;
           
           VSPAERO().UpdateMeshes();
           
           VSPAERO().Solve(Case);
           
           if ( RestartFromPreviousSolve_ ) VSPAERO().RestartFromPreviousSolve() = RestartFromPreviousSolve_;         
           
           if ( !DoUnsteadyAnalysis_ ) {
             
              Fxpi = (                    VSPAERO().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypi = (                    VSPAERO().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpi = (                    VSPAERO().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;

              Fxpw = (                    VSPAERO().CFiwx() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypw = (                    VSPAERO().CFiwy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpw = (                    VSPAERO().CFiwz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                   
              Mxpi = (                    VSPAERO().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypi = (                    VSPAERO().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpi = (                    VSPAERO().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
              Fxpo = ( VSPAERO().CFox()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypo = ( VSPAERO().CFoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpo = ( VSPAERO().CFoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                                     
              Mxpo = ( VSPAERO().CMox()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypo = ( VSPAERO().CMoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpo = ( VSPAERO().CMoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
              Fxpt = ( VSPAERO().CFox() + VSPAERO().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypt = ( VSPAERO().CFoy() + VSPAERO().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpt = ( VSPAERO().CFoz() + VSPAERO().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                      
              Mxpt = ( VSPAERO().CMox() + VSPAERO().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypt = ( VSPAERO().CMoy() + VSPAERO().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpt = ( VSPAERO().CMoz() + VSPAERO().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
           }
           
           else {

              Fxpi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                            
              Mxpi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
              Fxpo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                                                  
              Mxpo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                             
              Fxpt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                          
              Mxpt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
        
           }       
            
           for ( j = 1 ; j <= VSPAERO().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
           
              VSPAERO().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
              VSPAERO().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
              VSPAERO().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
              
           }         
              
        // - 
        
           Case++;
             
           VSPAERO().VSPGeom().Grid(0).NodeList(i).x() -= Delta;
           
           VSPAERO().UpdateMeshes();
           
           VSPAERO().Solve(Case);
           
           if ( !DoUnsteadyAnalysis_ ) {

              Fxmi = (                    VSPAERO().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymi = (                    VSPAERO().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmi = (                    VSPAERO().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                        
              Fxmw = (                    VSPAERO().CFiwx() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymw = (                    VSPAERO().CFiwy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmw = (                    VSPAERO().CFiwz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                 
              Mxmi = (                    VSPAERO().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymi = (                    VSPAERO().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmi = (                    VSPAERO().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                
              Fxmo = ( VSPAERO().CFox()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymo = ( VSPAERO().CFoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmo = ( VSPAERO().CFoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                                   
              Mxmo = ( VSPAERO().CMox()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymo = ( VSPAERO().CMoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmo = ( VSPAERO().CMoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
               
              Fxmt = ( VSPAERO().CFox() + VSPAERO().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymt = ( VSPAERO().CFoy() + VSPAERO().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmt = ( VSPAERO().CFoz() + VSPAERO().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                  
              Mxmt = ( VSPAERO().CMox() + VSPAERO().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymt = ( VSPAERO().CMoy() + VSPAERO().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmt = ( VSPAERO().CMoz() + VSPAERO().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                
           }
           
           else {

              Fxmi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                          
              Mxmi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                               
              Fxmo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                                               
              Mxmo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                               
              Fxmt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                          
              Mxmt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
      
           }    
              
           for ( j = 1 ; j <= VSPAERO().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
           
              VSPAERO().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
              VSPAERO().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
              VSPAERO().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
              
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
        
        if ( !Symmetry_ || VSPAERO().VSPGeom().Grid(0).NodeList(i).y() > 1.e-6 ) {
           
           // + 
           
              Case++;
                
              VSPAERO().VSPGeom().Grid(0).NodeList(i).y() += Delta;
              
              VSPAERO().UpdateMeshes();
              
              VSPAERO().Solve(Case);
              
              if ( !DoUnsteadyAnalysis_ ) {
                 
                 Fxpi = (                    VSPAERO().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypi = (                    VSPAERO().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpi = (                    VSPAERO().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                        
                 Fxpw = (                    VSPAERO().CFiwx() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypw = (                    VSPAERO().CFiwy() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpw = (                    VSPAERO().CFiwz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                         
                 Mxpi = (                    VSPAERO().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mypi = (                    VSPAERO().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzpi = (                    VSPAERO().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
                 Fxpo = ( VSPAERO().CFox()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypo = ( VSPAERO().CFoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpo = ( VSPAERO().CFoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                                        
                 Mxpo = ( VSPAERO().CMox()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mypo = ( VSPAERO().CMoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzpo = ( VSPAERO().CMoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
                 Fxpt = ( VSPAERO().CFox() + VSPAERO().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypt = ( VSPAERO().CFoy() + VSPAERO().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpt = ( VSPAERO().CFoz() + VSPAERO().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                         
                 Mxpt = ( VSPAERO().CMox() + VSPAERO().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mypt = ( VSPAERO().CMoy() + VSPAERO().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzpt = ( VSPAERO().CMoz() + VSPAERO().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
              }
              
              else {

                 Fxpi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                             
                 Mxpi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mypi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzpi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                     
                 Fxpo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                                                     
                 Mxpo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mypo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzpo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                  
                 Fxpt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fypt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzpt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                   
                 Mxpt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mypt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzpt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
              }      
            
              for ( j = 1 ; j <= VSPAERO().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
              
                 VSPAERO().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
                 VSPAERO().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
                 VSPAERO().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
                 
              }
                       
           // -
                       
              Case++;
                 
              VSPAERO().VSPGeom().Grid(0).NodeList(i).y() -= Delta;
              
              VSPAERO().UpdateMeshes();
              
              VSPAERO().Solve(Case);
                
              if ( !DoUnsteadyAnalysis_ ) {
                 
                    Fxmi = (                    VSPAERO().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fymi = (                    VSPAERO().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fzmi = (                    VSPAERO().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
              
                    Fxmw = (                    VSPAERO().CFiwx() ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fymw = (                    VSPAERO().CFiwy() ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fzmw = (                    VSPAERO().CFiwz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                       
                    Mxmi = (                    VSPAERO().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                    Mymi = (                    VSPAERO().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                    Mzmi = (                    VSPAERO().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                    
                    Fxmo = ( VSPAERO().CFox()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fymo = ( VSPAERO().CFoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fzmo = ( VSPAERO().CFoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                                         
                    Mxmo = ( VSPAERO().CMox()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                    Mymo = ( VSPAERO().CMoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                    Mzmo = ( VSPAERO().CMoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                    
                    Fxmt = ( VSPAERO().CFox() + VSPAERO().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fymt = ( VSPAERO().CFoy() + VSPAERO().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
                    Fzmt = ( VSPAERO().CFoz() + VSPAERO().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                      
                    Mxmt = ( VSPAERO().CMox() + VSPAERO().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                    Mymt = ( VSPAERO().CMoy() + VSPAERO().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                    Mzmt = ( VSPAERO().CMoz() + VSPAERO().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                    
              }
                 
              else {

                 Fxmi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fymi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzmi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                              
                 Mxmi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mymi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzmi = (                                                               VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                               
                 Fxmo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fymo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzmo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                               
                 Mxmo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mymo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzmo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg()                                                               ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                   
                 Fxmt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fymt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                 Fzmt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                
                 Mxmt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 Mymt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
                 Mzmt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                  
              }    
                 
              for ( j = 1 ; j <= VSPAERO().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
              
                 VSPAERO().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
                 VSPAERO().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
                 VSPAERO().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
                 
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
             
           VSPAERO().VSPGeom().Grid(0).NodeList(i).z() += Delta;
           
           VSPAERO().UpdateMeshes();
           
           VSPAERO().Solve(Case);
           
           if ( !DoUnsteadyAnalysis_ ) {
              
              Fxpi = (                    VSPAERO().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypi = (                    VSPAERO().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpi = (                    VSPAERO().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;

              Fxpw = (                    VSPAERO().CFiwx() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypw = (                    VSPAERO().CFiwy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpw = (                    VSPAERO().CFiwz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                    
              Mxpi = (                    VSPAERO().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypi = (                    VSPAERO().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpi = (                    VSPAERO().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
              Fxpo = ( VSPAERO().CFox()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypo = ( VSPAERO().CFoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpo = ( VSPAERO().CFoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                                     
              Mxpo = ( VSPAERO().CMox()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypo = ( VSPAERO().CMoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpo = ( VSPAERO().CMoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
              Fxpt = ( VSPAERO().CFox() + VSPAERO().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypt = ( VSPAERO().CFoy() + VSPAERO().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpt = ( VSPAERO().CFoz() + VSPAERO().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                      
              Mxpt = ( VSPAERO().CMox() + VSPAERO().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypt = ( VSPAERO().CMoy() + VSPAERO().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpt = ( VSPAERO().CMoz() + VSPAERO().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
           }
           
           else {

              Fxpi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                             
              Mxpi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                     
              Fxpo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                                                
              Mxpo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                    
              Fxpt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fypt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzpt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                           
              Mxpt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mypt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzpt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
              
           }    

           for ( j = 1 ; j <= VSPAERO().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
           
              VSPAERO().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
              VSPAERO().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
              VSPAERO().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
              
           }    
                
        // - 
        
           Case++;
             
           VSPAERO().VSPGeom().Grid(0).NodeList(i).z() -= Delta;
           
           VSPAERO().UpdateMeshes();
           
           VSPAERO().Solve(Case);
           
           if ( !DoUnsteadyAnalysis_ ) {
              
              Fxmi = (                    VSPAERO().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymi = (                    VSPAERO().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmi = (                    VSPAERO().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;

              Fxmw = (                    VSPAERO().CFiwx() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymw = (                    VSPAERO().CFiwy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmw = (                    VSPAERO().CFiwz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                  
              Mxmi = (                    VSPAERO().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymi = (                    VSPAERO().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmi = (                    VSPAERO().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                
              Fxmo = ( VSPAERO().CFox()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymo = ( VSPAERO().CFoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmo = ( VSPAERO().CFoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_;
                                    
              Mxmo = ( VSPAERO().CMox()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymo = ( VSPAERO().CMoy()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmo = ( VSPAERO().CMoz()                    ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                
              Fxmt = ( VSPAERO().CFox() + VSPAERO().CFix() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymt = ( VSPAERO().CFoy() + VSPAERO().CFiy() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmt = ( VSPAERO().CFoz() + VSPAERO().CFiz() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                  
              Mxmt = ( VSPAERO().CMox() + VSPAERO().CMix() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymt = ( VSPAERO().CMoy() + VSPAERO().CMiy() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmt = ( VSPAERO().CMoz() + VSPAERO().CMiz() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              
           }
           
           else {

              Fxmi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                           
              Mxmi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmi = (                                                                VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                                     
              Fxmo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                                                                                                                 
              Mxmo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmo = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg()                                                                ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                                                                                                    
              Fxmt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFox_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fymt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoy_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
              Fzmt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFoz_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CFiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_;
                                                           
              Mxmt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMox_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMix_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
              Mymt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoy_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiy_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Cref_;
              Mzmt = ( VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMoz_avg() + VSPAERO().VSPGeom().ComponentGroupList(LoadGroup).CMiz_avg() ) * 0.5 * Sref_ * Vref_ * Vref_ * Bref_;
                 
              
           }    
               
           //VSPAERO().VSPGeom().Grid(0).NodeList(i).z() += Delta;

           for ( j = 1 ; j <= VSPAERO().VSPGeom().Grid(0).NumberOfNodes() ; j++ ) {
           
              VSPAERO().VSPGeom().Grid(0).NodeList(j).x() = TempXYZ[3*j-2];
              VSPAERO().VSPGeom().Grid(0).NodeList(j).y() = TempXYZ[3*j-1];
              VSPAERO().VSPGeom().Grid(0).NodeList(j).z() = TempXYZ[3*j  ];
              
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
         VSPAERO().VSPGeom().Grid(0).NodeList(i).x(),
         VSPAERO().VSPGeom().Grid(0).NodeList(i).y(),
         VSPAERO().VSPGeom().Grid(0).NodeList(i).z());

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
         
        fprintf(FiniteDiffFile,"%20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e ",                  
         dFwdxyz[0][0],
         dFwdxyz[0][1],
         dFwdxyz[0][2],
         dFwdxyz[1][0],
         dFwdxyz[1][1],
         dFwdxyz[1][2],
         dFwdxyz[2][0],
         dFwdxyz[2][1],
         dFwdxyz[2][2]);

        fprintf(FiniteDiffFile,"%20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e %20.10e \n",                  
         dFwdxyz[0][0] + dFodxyz[0][0],
         dFwdxyz[0][1] + dFodxyz[0][1],
         dFwdxyz[0][2] + dFodxyz[0][2],
         dFwdxyz[1][0] + dFodxyz[1][0],
         dFwdxyz[1][1] + dFodxyz[1][1],
         dFwdxyz[1][2] + dFodxyz[1][2],
         dFwdxyz[2][0] + dFodxyz[2][0],
         dFwdxyz[2][1] + dFodxyz[2][1],
         dFwdxyz[2][2] + dFodxyz[2][2]);                                    
    }
    
    delete [] TempXYZ;

}

#ifdef VSPAERO_OPT

/*##############################################################################
#                                                                              #
#                            Optimize                                          #
#                                                                              #
##############################################################################*/

void VSPAERO_Optimize(void)
{
   
    int Case, NumberOfThreads;
    int i, j, Iter, NumberOfParameterValues, NumberOfNodes, k, Done, Optimum;

    double CL, CD, CS, CML, CMM, CMN;
    double CLo, CDo, CSo, CMLo, CMMo, CMNo;
    double *NodeXYZ, *dFdMesh[3], *dF_dParameter, *Gradient, *GradientOld;
    double F, F1, Fnew, Delta, Delta1, FReduction, DeltaReduction, StepSize;
    double Lambda_1, Lambda_2, Lambda_3, Lambda_4, Lambda_5, Lambda_6, **dMesh_dParameter;
    double *ParameterValues, *NewParameterValues, Time0, TotalTime, *MeshNodes;

    char HistoryFileName[MAX_CHAR_SIZE], CommandLine[MAX_CHAR_SIZE], OptimizationSetupFileName[MAX_CHAR_SIZE], **ParameterNames;
    char OpenVSP_FileName[MAX_CHAR_SIZE], OpenVSP_VSPGeomFileName[MAX_CHAR_SIZE], NewFileName[MAX_CHAR_SIZE];

    FILE *HistoryFile, *OptimizationSetupFile;
    
    int NumberOfForwardSolves, NumberOfAdjointSolves, NumberOfGeometryUpdates;
    double Time, ForwardSolveTime, AdjointSolveTime, GeometryUpdateTime;

    // OpenVSP parameter data
            
    PARAMETER_DATA *ParameterData;

    // OPENMP stuff
                             
#ifdef VSPAERO_OPENMP
   
    printf("Initializing OPENMP for %d threads \n",NumberOfThreads_);
   
    omp_set_num_threads(NumberOfThreads_);
    
    NumberOfThreads = omp_get_max_threads();

    printf("NumberOfThreads_: %d \n",NumberOfThreads);
    
#else

    NumberOfThreads = 1;

    printf("Single threaded build.\n");

#endif

    // Write out 2D FEM file
    
    if ( Write2DFEMFile_ ) VSPAERO().Write2DFEMFile() = 1;
    
    // Write out Tecplot file
    
    if ( WriteTecplotFile_ ) VSPAERO().WriteTecplotFile() = 1;
        
    // Save optimization data
    
    if ( OptimizationSolve_ ) VSPAERO().OptimizationSolve() = 1;
        
    // Set number of farfield wake nodes

    if ( NumberOfWakeNodes_ > 0 ) VSPAERO().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);       
               
    // Force farfield distance for wake adaption
    
    if ( SetFarDist_ ) VSPAERO().SetFarFieldDist(FarDist_);

    // Zero out statistics
    
    NumberOfForwardSolves = NumberOfAdjointSolves = NumberOfGeometryUpdates = 0;
    
    ForwardSolveTime = AdjointSolveTime = GeometryUpdateTime = 0.;

    printf("Running wing optimization... \n");fflush(NULL);

    // Read in the OpenVSP geometry
    
    snprintf(OpenVSP_FileName,sizeof(OpenVSP_FileName)*sizeof(char),"%s.vsp3",FileName);

    vsp::ReadVSPFile( OpenVSP_FileName );
    
    // Initialize the starting geometry
    
    Time = Time0 = myclock();
    
    vsp::SetAnalysisInputDefaults( "VSPAEROComputeGeometry" );
    
    vsp::SetAnalysisInputDefaults( "VSPAEROSweep" ); // We use the user define thick/thin sets in the VSPAERO setup gui
    
   // vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "GeomSet", {vsp::SET_FIRST_USER + 0}, 0);      // Thick geometry -- "Panel"... we assume it's set 0
   // 
   // vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "ThinGeomSet", {vsp::SET_FIRST_USER + 1}, 0);  // Thin geometry -- "VLM"... we assume it's set 1
    
    int m_SymFlagVec = 0;
    
    vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "Symmetry", {m_SymFlagVec}, 0);

    vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "CullFracFlag", {1}, 0);
    
    vsp::SetDoubleAnalysisInput("VSPAEROComputeGeometry", "CullFrac", {0.1}, 0);

    // Create the initial .vspgeom file for analysis
    
    string compgeom_resid = vsp::ExecAnalysis( "VSPAEROComputeGeometry" );
    
  //  vsp::SetIntAnalysisInput( "VSPAEROSweep", "StopBeforeRun", {1}, 0 );
  //  
  //  vsp::ExecAnalysis( "VSPAEROSweep" );

    // Also save a copy
    
    snprintf(NewFileName,sizeof(NewFileName)*sizeof(char),"%s.Initial.vspgeom",FileName);
    
    vsp::ExportFile( NewFileName, vsp::SET_ALL, vsp::EXPORT_VSPGEOM );

    GeometryUpdateTime += myclock() - Time;
                            
    // Initialize VSPAERO solver settings

    VSPAERO().Sref() = Sref_;

    VSPAERO().Cref() = Cref_;

    VSPAERO().Bref() = Bref_;
    
    VSPAERO().Xcg() = Xcg_;

    VSPAERO().Ycg() = Ycg_;

    VSPAERO().Zcg() = Zcg_;
    
    VSPAERO().Mach() = Mach_;
    
    VSPAERO().AngleOfAttack() = AoA_ * TORAD;

    VSPAERO().AngleOfBeta() = Beta_ * TORAD;
    
    VSPAERO().Vinf() = Vinf_;
    
    VSPAERO().Vref() = Vref_;
    
    VSPAERO().Machref() = Machref_;
    
    VSPAERO().Density() = Rho_;
    
    VSPAERO().ReCref() = ReCref_;
    
    VSPAERO().RotationalRate_p() = 0.0;
    
    VSPAERO().RotationalRate_q() = 0.0;
    
    VSPAERO().RotationalRate_r() = 0.0;    

    VSPAERO().DoSymmetryPlaneSolve() = Symmetry_;
        
    VSPAERO().StallModelIsOn() = StallModelIsOn_;
    
    VSPAERO().WakeIterations() = WakeIterations_;
    
    VSPAERO().ForwardGMRESConvergenceFactor() = ForwardGMRESConvergenceFactor_;
    
    VSPAERO().AdjointGMRESConvergenceFactor() = AdjointGMRESConvergenceFactor_;
    
    VSPAERO().NonLinearConvergenceFactor() = NonLinearConvergenceFactor_;

    VSPAERO().WakeRelax() = WakeRelax_;
    
    VSPAERO().ImplicitWake() = ImplicitWake_;
    
    VSPAERO().DoAdjointSolve() = 1; // This needs to be set before Setup() is called... 
    
    if ( !OptimizationUsingWakeForces_ ) {
       
       VSPAERO().AdjointSolutionForceType() = ADJOINT_TOTAL_FORCES; // Only total forces gradients

    }
    
    else {
       
       VSPAERO().AdjointSolutionForceType() = ADJOINT_TOTAL_FORCES_USING_WAKE_FORCES; // Only total forces gradients, but based on Trefftz forces
          
    }
      
    VSPAERO().ReadFile(FileName);

    VSPAERO().Setup();

    // Load in the optimization parameter value list

    ParameterData = ReadOpenVSPDesFile(FileName,NumberOfParameterValues);
    
    dF_dParameter = new double[NumberOfParameterValues + 1];
    
    ParameterValues = new double[NumberOfParameterValues + 1];
    
    NewParameterValues = new double[NumberOfParameterValues + 1];

    Gradient = new double[NumberOfParameterValues + 1];
    
    GradientOld = new double[NumberOfParameterValues + 1];

    for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {

       GradientOld[j] = Gradient[j] = dF_dParameter[j] = 0.;
       
       ParameterValues[j] = ParameterData->ParameterValues[j];

    }
    
    // Calculate partials of mesh wrt parameters

    Time = myclock();
        
    dMesh_dParameter = CalculateOpenVSPGeometryGradients(FileName,NumberOfParameterValues,ParameterData);
    
    GeometryUpdateTime += myclock() - Time;
    
    NumberOfGeometryUpdates += 2*NumberOfParameterValues + 1;

    printf("Finished calculate mesh gradients... \n");fflush(NULL);

    // Allocate some space for the derivatives 
    
    dFdMesh[0] = new double[VSPAERO().VSPGeom().Grid(0).NumberOfSurfaceNodes() + 1];
    dFdMesh[1] = new double[VSPAERO().VSPGeom().Grid(0).NumberOfSurfaceNodes() + 1];
    dFdMesh[2] = new double[VSPAERO().VSPGeom().Grid(0).NumberOfSurfaceNodes() + 1];
    
    // Open the history file

    snprintf(HistoryFileName,sizeof(HistoryFileName)*sizeof(char),"%s.opt.history",FileName);

    if ( (HistoryFile = fopen(HistoryFileName, "w")) == NULL ) {
    
       printf("Could not open the optimization history output file! \n");
    
       exit(1);
    
    }
    
    // Clean up any old opt adb files
    
    snprintf(CommandLine,sizeof(CommandLine)*sizeof(char),"rm %s.opt.*.adb",FileName);
    
    system(CommandLine);

    // Header
     
                        //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 
    fprintf(HistoryFile,"    Iter        CL         CD         CS        CML        CMM        CMN        L/D         F      FReduction   GradF    GradReduct   1DSteps   StepSize     Time   \n");

    k = 0;

    Delta = StepSize = 0.;

    Lambda_1 = 0.;
    Lambda_2 = 0.;
    Lambda_3 = 0.;
             
    Lambda_4 = 0.;
    Lambda_5 = 0.;
    Lambda_6 = 0.;
         
    TotalTime = myclock() - Time0;
    
    Case = 0;
    
    Optimum = 0;
    
    Iter = 1;
    
    while ( Iter <= OptimizationNumberOfIterations_ && !Optimum ) {

       // Solve the forward problem and the adjoint
       
       printf("Running VSPAERO forward and adjoint solvers... \n");fflush(NULL);
       
       Case++;

       VSPAERO().DoAdjointSolve() = 1;
       
       Time = myclock();
       
       VSPAERO().Solve(Case);

       AdjointSolveTime += myclock() - Time;
       
       NumberOfForwardSolves++;
       
       NumberOfAdjointSolves += 6;
       
       // Save the .adb file for later viewing
       
       snprintf(CommandLine,sizeof(CommandLine)*sizeof(char),"cp %s.adb %s.opt.%d.adb",FileName,FileName,Iter);
       
       system(CommandLine);
       
       if ( Iter == 1 ) {
          
          // Save the .adb file for later viewing
          
          snprintf(CommandLine,sizeof(CommandLine)*sizeof(char),"cp %s.adb %s.opt.adb",FileName,FileName);
          
          system(CommandLine);
          
          // Copy over the .cases file for viewer
          
          snprintf(CommandLine,sizeof(CommandLine)*sizeof(char),"cp %s.adb.cases %s.opt.adb.cases",FileName,FileName);
          
          system(CommandLine);          
       
       }          
       
       // Grab total Cl, CD, and CM ... this included both invisicid and viscous contributions
       
       if ( !OptimizationUsingWakeForces_   ) {
          
          CL = VSPAERO().CLi() + VSPAERO().CLo();
          CD = VSPAERO().CDi() + VSPAERO().CDo();
          CS = VSPAERO().CSi() + VSPAERO().CSo();
          
          CML = VSPAERO().CMix() + VSPAERO().CMox();
          CMM = VSPAERO().CMiy() + VSPAERO().CMoy();
          CMN = VSPAERO().CMiz() + VSPAERO().CMoy();
          
       }
       
       else if ( OptimizationUsingWakeForces_ ) {
          
          CL = VSPAERO().CLiw() + VSPAERO().CLo();
          CD = VSPAERO().CDiw() + VSPAERO().CDo();
          CS = VSPAERO().CSiw() + VSPAERO().CSo();
          
          CML = VSPAERO().CMix() + VSPAERO().CMox();
          CMM = VSPAERO().CMiy() + VSPAERO().CMoy();
          CMN = VSPAERO().CMiz() + VSPAERO().CMoy();

       }
 
       else {
          
          printf("Unknown VSPAERO solver type! \n");
          fflush(NULL);exit(1);
          
       }

       CLo = CL;
       CDo = CD;
       CSo = CS;
       
       CMLo = CML;
       CMMo = CMM;
       CMMo = CMN;
       
       // Weights
   
       if ( Iter == 1 ) {
          
         Lambda_1 = Optimization_CL_Weight_ / pow(CL-Optimization_CL_Required_,2.);
         Lambda_2 = Optimization_CD_Weight_ / pow(CD-Optimization_CD_Required_,2.);
         Lambda_3 = Optimization_CS_Weight_ / pow(CS-Optimization_CS_Required_,2.);
         
         Lambda_4 = Optimization_CML_Weight_ / pow(CML-Optimization_CML_Required_,2.);
         Lambda_5 = Optimization_CMM_Weight_ / pow(CMM-Optimization_CMM_Required_,2.);
         Lambda_6 = Optimization_CMN_Weight_ / pow(CMN-Optimization_CMN_Required_,2.);
                  
       }
       
       Lambda_1 = MIN(Lambda_1,10000.);
       Lambda_2 = MIN(Lambda_2,10000.);
       Lambda_3 = MIN(Lambda_3,10000.);
       Lambda_4 = MIN(Lambda_4,10000.);
       Lambda_5 = MIN(Lambda_5,10000.);
       Lambda_6 = MIN(Lambda_6,10000.);
         
       // Calculate the final objective function and it's gradients
       // Here we have f = L1*(CL - CLreq)^2 + L2*CD^2 + L3*CM^2

       F = Lambda_1 * pow(CL  - Optimization_CL_Required_,2.)
         + Lambda_2 * pow(CD  - Optimization_CD_Required_,2.) 
         + Lambda_3 * pow(CS  - Optimization_CS_Required_,2.) 
         
         + Lambda_4 * pow(CML - Optimization_CML_Required_,2.)
         + Lambda_5 * pow(CMM - Optimization_CMM_Required_,2.)
         + Lambda_6 * pow(CMN - Optimization_CMN_Required_,2.);
         
       if ( Iter == 1 ) F1 = F;
    
       for ( i = 1 ; i <= VSPAERO().VSPGeom().Grid(1).NumberOfSurfaceNodes() ; i++ ) {
       
          dFdMesh[0][i] = 2.*Lambda_1*(CL  - Optimization_CL_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCLt_DX() 
                        + 2.*Lambda_2*(CD  - Optimization_CD_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCDt_DX() 
                        + 2.*Lambda_3*(CS  - Optimization_CS_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCSt_DX() 
                        
                        + 2.*Lambda_4*(CML - Optimization_CML_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCMmt_DX()
                        + 2.*Lambda_5*(CMM - Optimization_CMM_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCMmt_DX()
                        + 2.*Lambda_6*(CMN - Optimization_CMN_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCMmt_DX();
                
          dFdMesh[1][i] = 2.*Lambda_1*(CL  - Optimization_CL_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCLt_DY() 
                        + 2.*Lambda_2*(CD  - Optimization_CD_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCDt_DY() 
                        + 2.*Lambda_3*(CS  - Optimization_CS_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCSt_DY() 
                        
                        + 2.*Lambda_4*(CML - Optimization_CML_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCMmt_DY()
                        + 2.*Lambda_5*(CMM - Optimization_CMM_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCMmt_DY()
                        + 2.*Lambda_6*(CMN - Optimization_CMN_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCMmt_DY();
                        
          dFdMesh[2][i] = 2.*Lambda_1*(CL  - Optimization_CL_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCLt_DZ() 
                        + 2.*Lambda_2*(CD  - Optimization_CD_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCDt_DZ() 
                        + 2.*Lambda_3*(CS  - Optimization_CS_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCSt_DZ() 
                        
                        + 2.*Lambda_4*(CML - Optimization_CML_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCMmt_DZ()
                        + 2.*Lambda_5*(CMM - Optimization_CMM_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCMmt_DZ()
                        + 2.*Lambda_6*(CMN - Optimization_CMN_Required_)*VSPAERO().VSPGeom().Grid(1).NodeList(i).DCMmt_DZ();

       }

       printf("Doing chain rule... \n");fflush(NULL);

       // Chain rule... calculate derivatives wrt parameters
       
       for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
          
          dF_dParameter[j] = 0;
       
          for ( i = 1 ; i <= VSPAERO().VSPGeom().Grid(1).NumberOfSurfaceNodes() ; i++ ) {
             
             dF_dParameter[j] +=   dFdMesh[0][i] * dMesh_dParameter[j][3*i-2]
                                 + dFdMesh[1][i] * dMesh_dParameter[j][3*i-1]
                                 + dFdMesh[2][i] * dMesh_dParameter[j][3*i  ];
                        
          }
          
       }  

       // Store old and new gradients
       
       for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
       
          GradientOld[j] = Gradient[j];
          
          Gradient[j] = dF_dParameter[j];
          
       }
                    
       // Conjugate gradient adjustment of the gradient...
       
       if ( Iter > 1 ) CGState(GradientOld,Gradient,NumberOfParameterValues);
         
       // Calculate magnitude of the gradient and normalize it
       
       Delta = Normalize(Gradient, NumberOfParameterValues);
       
       if ( Iter == 1 ) Delta1 = Delta;

       StepSize = 0.25*Delta;       
       
       if ( StepSize > 0.25 ) StepSize = 0.25;

       if ( Iter == 1 ) {
          
          FReduction = 0.;
          
          DeltaReduction = 0.;
          
          fprintf(HistoryFile,"%10d %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10d %10.5f %10.5f \n",0,CL,CD,CS,CML,CMM,CMN,CL/CD,F,FReduction,Delta,DeltaReduction,k,StepSize,TotalTime);

       }
       
       if ( Iter != OptimizationNumberOfIterations_ ) {
          
          // A really bad 1D search...
          
          k = 0;
          
          Done = 0;
   
          while ( !Done && k < OptimizationNumber1DSearchSteps_ ) {
             
             printf("\n\n\n\n\n\n Doing 1D search iteration %d the current Step Size: %f \n\n\n\n\n\n ",k,StepSize);
          
             for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
             
                ParameterValues[j] -= StepSize * Gradient[j];
                
               // printf("ParameterValues[%d]: %f \n",j,ParameterValues[j]);fflush(NULL);
                
             }

             Time = myclock();
             
             NodeXYZ = CreateVSPGeometry(FileName,NumberOfParameterValues,ParameterData->ParameterNames, ParameterValues);
             
             GeometryUpdateTime += myclock() - Time;

             NumberOfGeometryUpdates++;

             // Update mesh
             
             for ( j = 1 ; j <= VSPAERO().VSPGeom().Grid(0).NumberOfSurfaceNodes() ; j++ ) {

                VSPAERO().VSPGeom().Grid(0).NodeList(j).x() = NodeXYZ[3*j-2];
                VSPAERO().VSPGeom().Grid(0).NodeList(j).y() = NodeXYZ[3*j-1];
                VSPAERO().VSPGeom().Grid(0).NodeList(j).z() = NodeXYZ[3*j  ];
      
             }
                         
             Time = myclock();
                          
             VSPAERO().VSPGeom().UpdateMeshes();
             
             GeometryUpdateTime += myclock() - Time;

             delete [] NodeXYZ;
             
             // Do a forward solve only to evaluate the functional
             
             Case++;
             
             Time = myclock();
             
             VSPAERO().DoAdjointSolve() = 0;
                          
             VSPAERO().Solve(Case);
             
             ForwardSolveTime += myclock() - Time;
                          
             NumberOfForwardSolves++;
            
             CLo = CL;
             CDo = CD;
             CSo = CS;
             
             CMLo = CML;
             CMMo = CMM;
             CMNo = CMN;     
                          
             if ( !OptimizationUsingWakeForces_   ) {
                
                CL = VSPAERO().CLi() + VSPAERO().CLo();
                CD = VSPAERO().CDi() + VSPAERO().CDo();
                CS = VSPAERO().CSi() + VSPAERO().CSo();
                
                CML = VSPAERO().CMix() + VSPAERO().CMox();
                CMM = VSPAERO().CMiy() + VSPAERO().CMoy();
                CMN = VSPAERO().CMiz() + VSPAERO().CMoy();
                
             }
             
             else if ( OptimizationUsingWakeForces_ ) {
                
                CL = VSPAERO().CLiw() + VSPAERO().CLo();
                CD = VSPAERO().CDiw() + VSPAERO().CDo();
                CS = VSPAERO().CSiw() + VSPAERO().CSo();
                
                CML = VSPAERO().CMix() + VSPAERO().CMox();
                CMM = VSPAERO().CMiy() + VSPAERO().CMoy();
                CMN = VSPAERO().CMiz() + VSPAERO().CMoy();
             
             }
        
             Fnew = Lambda_1 * pow(CL  - Optimization_CL_Required_,2.)
                  + Lambda_2 * pow(CD  - Optimization_CD_Required_,2.) 
                  + Lambda_3 * pow(CS  - Optimization_CS_Required_,2.) 
                  
                  + Lambda_4 * pow(CML - Optimization_CML_Required_,2.)
                  + Lambda_5 * pow(CMM - Optimization_CMM_Required_,2.)
                  + Lambda_6 * pow(CMN - Optimization_CMN_Required_,2.);
         
             printf("1D search... current F: %f ... previous F: %f \n",Fnew,F); fflush(NULL);
           
             // Just keep going until function has increased... 
             
             if ( Fnew > F ) {
       
                printf("Stopping 1D search and backing up a step... \n");                
                
                // Back up 
                
                if ( k > 0 ) {

                   for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
                   
                      ParameterValues[j] += StepSize * Gradient[j];
                 
                   }
    
                   Time = myclock();
                   
                   NodeXYZ = CreateVSPGeometry(FileName,NumberOfParameterValues,ParameterData->ParameterNames, ParameterValues);
                  
                   GeometryUpdateTime += myclock() - Time;
                  
                   NumberOfGeometryUpdates++;
                  
                   // Update mesh
                   
                   for ( j = 1 ; j <= VSPAERO().VSPGeom().Grid(0).NumberOfSurfaceNodes() ; j++ ) {
                   
                      VSPAERO().VSPGeom().Grid(0).NodeList(j).x() = NodeXYZ[3*j-2];
                      VSPAERO().VSPGeom().Grid(0).NodeList(j).y() = NodeXYZ[3*j-1];
                      VSPAERO().VSPGeom().Grid(0).NodeList(j).z() = NodeXYZ[3*j  ];
                   
                   }
                                
                   // Update the solver and adjoint meshes
                   
                   VSPAERO().VSPGeom().UpdateMeshes();
                   
                   delete [] NodeXYZ;
                   
                   CL = CLo;
                   CD = CDo;
                   CS = CSo;
                   
                   CML = CMLo;
                   CMM = CMMo;
                   CMM = CMNo;      
                       
                }
                   
                Done = 1; 
                
                if ( k == 0 ) F = Fnew;
                
             }
             
             else {
                
                F = Fnew;
                
             }                             
             
             if ( !Done ) {
                
                k++;
             
                if ( !Done ) StepSize *= 1.618;
         
             }
             
          }
             
       }

       TotalTime = myclock() - Time0;

       FReduction = log10(F/F1);

       DeltaReduction = log10(Delta/Delta1);
       
       fprintf(HistoryFile,"%10d %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10d %10.5f %10.5f \n",Iter,CL,CD,CS,CML,CMM,CMN,CL/CD,F,FReduction,Delta,DeltaReduction,k,StepSize,TotalTime);
       
       // Update the mesh gradients
       
       if ( OPtimizationUpdateGeometryGradients_ ) {
          
          dMesh_dParameter = CalculateOpenVSPGeometryGradients(FileName,NumberOfParameterValues,ParameterData);
          
       }
       
       if ( DeltaReduction <= log10(OptimizationGradientReduction_) ) Optimum = 1;
       
       Iter++;

    }
    
    // Save the final state
    
    SaveVSPGeomFile(FileName, NumberOfParameterValues,ParameterData->ParameterNames, ParameterValues);
    
    // Output some solver stats...

    printf("\n\n");
    printf("NumberOfForwardSolves:   %d \n",NumberOfForwardSolves);
    printf("NumberOfAdjointSolves:   %d \n",NumberOfAdjointSolves);
    printf("NumberOfGeometryUpdates: %d \n",NumberOfGeometryUpdates);
    printf("\n\n");
    printf("Forward  Solve  Time %f seconds \n",ForwardSolveTime);
    printf("Adoint   Solve  Time %f seconds \n",AdjointSolveTime);
    printf("Geometry Update Time %f seconds \n",GeometryUpdateTime);
    printf("Total Optimization time: %f seconds \n",TotalTime);
        
    fprintf(HistoryFile,"\n\n");
    fprintf(HistoryFile,"NumberOfForwardSolves:   %d \n",NumberOfForwardSolves);
    fprintf(HistoryFile,"NumberOfAdjointSolves:   %d \n",NumberOfAdjointSolves);
    fprintf(HistoryFile,"NumberOfGeometryUpdates: %d \n",NumberOfGeometryUpdates);
    fprintf(HistoryFile,"\n\n");
    fprintf(HistoryFile,"Forward  Solve  Time %f seconds \n",ForwardSolveTime);
    fprintf(HistoryFile,"Adoint   Solve  Time %f seconds \n",AdjointSolveTime);
    fprintf(HistoryFile,"Geometry Update Time %f seconds \n",GeometryUpdateTime);
    fprintf(HistoryFile,"Total Optimization time: %f seconds \n",TotalTime);
       
    fclose(HistoryFile);
    
    // Free up memory
    
    DeleteMeshGradients(NumberOfParameterValues,dMesh_dParameter);
    
    // Exit
    
    exit(0);
    
}

/*##############################################################################
#                                                                              #
#                            ReadOpenVSPDesFile                                #
#                                                                              #
##############################################################################*/

PARAMETER_DATA *ReadOpenVSPDesFile(char *FileName, int &NumberOfDesignVariables)
{
    
    int i;
    double *ParameterValues;
    char DesignFileName[MAX_CHAR_SIZE], Variable[MAX_CHAR_SIZE];
    FILE *DesignFile;
    PARAMETER_DATA *ParameterData;
    
    ParameterData = new PARAMETER_DATA;
    
    // Open the OpenVSP des file

    snprintf(DesignFileName,sizeof(DesignFileName)*sizeof(char),"%s.des",FileName);
    
    printf("Opening: %s \n",DesignFileName);fflush(NULL);

    if ( (DesignFile = fopen(DesignFileName, "r")) == NULL ) {
    
       printf("Could not open the OpenVSP des file! \n");
    
       exit(1);
    
    }
    
    // Parse the des file, replace parameters with new values    
    
    fscanf(DesignFile,"%d \n",&NumberOfDesignVariables);

    ParameterData->ParameterValues = new double[NumberOfDesignVariables + 1];
    
    ParameterData->ParameterNames = new char*[NumberOfDesignVariables + 1];

    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {

       ParameterData->ParameterNames[i] = new char[11];

       fscanf(DesignFile,"%s%lf\n",(Variable),&( ParameterData->ParameterValues[i]));
              
       strncpy(ParameterData->ParameterNames[i],Variable,11);

       printf("%s --> %f \n",Variable,ParameterData->ParameterValues[i]);fflush(NULL);

    }
    
    fclose(DesignFile);
    
    return ParameterData;
 
}

/*##############################################################################
#                                                                              #
#                            CreateVSPGeometry                                 #
#                                                                              #
##############################################################################*/

double *CreateVSPGeometry(char *FileName, int NumberOfDesignVariables, char **ParameterNames, double *ParameterValues)
{
    
    int i, j, Node, NumVars;
    int Surface;
    int surf_indx;
    int degen_type;
    int *DidThisNode;    
    double Value, *MeshNodesXYZ;
    char DesignFileName[MAX_CHAR_SIZE], Variable[MAX_CHAR_SIZE], CommandLine[MAX_CHAR_SIZE];
    FILE *DesignFile, *OptDesFile;
    
    // Update the VSP geometry

    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {

       double temp = vsp::SetParmVal( ParameterNames[i], ParameterValues[i] );
    
    }

    vsp::Update();
 
    // Now parse the new geometry and create a new list of xyz node data
    
    vector<double> uvec, wvec;
     
    uvec.resize( 1 );
    
    wvec.resize( 1 );
     
    MeshNodesXYZ = new double[3*VSPAERO().VSPGeom().Grid(1).NumberOfNodes() + 1];

    DidThisNode = new int[VSPAERO().VSPGeom().Grid(1).NumberOfNodes() + 1];
    
    zero_int_array(DidThisNode, VSPAERO().VSPGeom().Grid(1).NumberOfNodes());
    
    // Loop over loops and calculate the new xyz locations of every node... yes, this means
    // we do a lot of the nodes more than once... oh well.

    for ( i = 1 ; i <= VSPAERO().VSPGeom().Grid(1).NumberOfSurfaceLoops() ; i++ ) {
       
       Surface = VSPAERO().VSPGeom().Grid(1).LoopList(i).SurfaceID();
       
       surf_indx = VSPAERO().VSPGeom().VSPSurfaceIDForSurface(Surface);
       
       degen_type = VSPAERO().VSPGeom().VSPSurfaceDegenType(Surface);
       
       string geom_id = VSPAERO().VSPGeom().SurfaceGIDList(Surface);

       for ( j = 1 ; j <= VSPAERO().VSPGeom().Grid(1).LoopList(i).NumberOfNodes() ; j++ ) {
          
          Node = VSPAERO().VSPGeom().Grid(1).LoopList(i).Node(j);
          
          if ( !DidThisNode[Node] ) {
          
             uvec[0] = VSPAERO().VSPGeom().Grid(1).LoopList(i).U_Node(j);
             wvec[0] = VSPAERO().VSPGeom().Grid(1).LoopList(i).V_Node(j);
             
             vector< vec3d > ptvec = vsp::CompVecDegenPnt01( geom_id, surf_indx, degen_type, uvec, wvec);
             
             MeshNodesXYZ[3*Node - 2] = ptvec[0].x();
             MeshNodesXYZ[3*Node - 1] = ptvec[0].y();
             MeshNodesXYZ[3*Node    ] = ptvec[0].z();
             
           //  printf("Node: %d --> x,y,z: %f, %f ... %f, %f .. %f, %f \n",
           //  Node,
           //  VSPAERO().VSPGeom().Grid(1).NodeList(Node).x(), ptvec[0].x(),
           //  VSPAERO().VSPGeom().Grid(1).NodeList(Node).y(), ptvec[0].y(),
           //  VSPAERO().VSPGeom().Grid(1).NodeList(Node).z(), ptvec[0].z());
               
             DidThisNode[Node] = 1;
             
          }
          
       }
      
    }
    
    delete [] DidThisNode;
    
    return MeshNodesXYZ;

}

/*##############################################################################
#                                                                              #
#                            SaveVSPGeomFile                                   #
#                                                                              #
##############################################################################*/

void SaveVSPGeomFile(char *FileName, int NumberOfDesignVariables, char **ParameterNames, double *ParameterValues)
{
    
    int i, NumVars;
    double Value;
    char DesignFileName[MAX_CHAR_SIZE], Variable[MAX_CHAR_SIZE];
    FILE *DesignFile, *OptDesFile;
    
    // Update the VSP geometry

    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {

       double temp = vsp::SetParmVal( ParameterNames[i], ParameterValues[i] );
    
    }

    vsp::Update();
    
    // Save the current geometry out as a vspgeom file
    
    vsp::SetAnalysisInputDefaults( "VSPAEROComputeGeometry" );

    vsp::SetAnalysisInputDefaults( "VSPAEROSweep" ); // We use the user define thick/thin sets in the VSPAERO setup gui

  // vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "GeomSet", {vsp::SET_FIRST_USER + 0}, 0);      // Thick geometry -- "Panel"... we assume it's set 0
  // 
  // vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "ThinGeomSet", {vsp::SET_FIRST_USER + 1}, 0);  // Thin geometry -- "VLM"... we assume it's set 1
    
    int m_SymFlagVec = 0;
    
    vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "Symmetry", {m_SymFlagVec}, 0);

    vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "CullFracFlag", {1}, 0);
    
    vsp::SetDoubleAnalysisInput("VSPAEROComputeGeometry", "CullFrac", {0.1}, 0);

    string compgeom_resid = vsp::ExecAnalysis( "VSPAEROComputeGeometry" );

    // Open des file
    
    snprintf(DesignFileName,sizeof(DesignFileName)*sizeof(char),"%s.des",FileName);
    
    //printf("Opening: %s \n",DesignFileName);fflush(NULL);

    if ( (DesignFile = fopen(DesignFileName, "r")) == NULL ) {
    
       printf("Could not open the OpenVSP Opt des file! \n");
    
       exit(1);
    
    }
    
    // Open Opt des file
    
    snprintf(DesignFileName,sizeof(DesignFileName)*sizeof(char),"%s.Opt.Final.des",FileName);
    
    //printf("Opening: %s \n",DesignFileName);fflush(NULL);

    if ( (OptDesFile = fopen(DesignFileName, "w")) == NULL ) {
    
       printf("Could not open the OpenVSP Opt des file! \n");
    
       exit(1);
    
    }
    
    // Parse the des file, replace parameters with new values    
    
    fscanf(DesignFile,"%d \n",&NumVars);
    
    if ( NumberOfDesignVariables != NumVars ) {
       
       printf("Number of design variables does not match OpenVSP des file! \n");
       fflush(NULL);exit(1);
       
    }
    
    fprintf(OptDesFile,"%d\n",NumVars);
    
    fflush(NULL);
    
    for ( i = 1 ; i <= NumVars ; i++ ) {

       fscanf(DesignFile,"%s%lf\n",Variable,&Value);

       fprintf(OptDesFile,"%s %20.10e\n",Variable,ParameterValues[i]);
       
    }
    
    fclose(DesignFile);
    
    fclose(OptDesFile);       
    
}

/*##############################################################################
#                                                                              #
#                    CalculateOpenVSPGeometryGradients                         #
#                                                                              #
##############################################################################*/

double **CalculateOpenVSPGeometryGradients(char *FileName, int NumberOfDesignVariables, PARAMETER_DATA *ParameterData)
{
    
    int i, j, NumberOfMeshNodes;
    double **dMesh_dParameter, *NewParameterValues, Delta, *MeshMinus;
    
    printf("Calculating OpenVSP mesh gradients ... \n");fflush(NULL);
    
    // Create space for the mesh gradients
    
    NewParameterValues = new double[NumberOfDesignVariables + 1];
    
    dMesh_dParameter = new double*[NumberOfDesignVariables + 1];

    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {

       NewParameterValues[i] = ParameterData->ParameterValues[i];
       
    }
    
    // Create the baseline geometry

    dMesh_dParameter[0] = CreateVSPGeometry(FileName,NumberOfDesignVariables,ParameterData->ParameterNames,ParameterData->ParameterValues);

    // Loop over parameters and calculate mesh gradients using finite differences
    
    Delta = 0.01;
    
    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {
    
       printf("Working on parameter: %d out of %d \r",i,NumberOfDesignVariables);fflush(NULL);
         
       // + Perturbation
       
       NewParameterValues[i] = ParameterData->ParameterValues[i] + Delta;
       
       dMesh_dParameter[i] = CreateVSPGeometry(FileName,NumberOfDesignVariables,ParameterData->ParameterNames,NewParameterValues);

       // - Perturbation
    
       NewParameterValues[i] = ParameterData->ParameterValues[i] - Delta;
       
       MeshMinus = CreateVSPGeometry(FileName,NumberOfDesignVariables,ParameterData->ParameterNames,NewParameterValues);

       // Calculate derivative using central differences
       
       for ( j = 1 ; j <= 3*VSPAERO().VSPGeom().Grid(1).NumberOfSurfaceNodes() ; j++ ) {

          dMesh_dParameter[i][j] = ( dMesh_dParameter[i][j] - MeshMinus[j] )/(2.*Delta);
       
       }
       
       delete [] MeshMinus;
       
       NewParameterValues[i] = ParameterData->ParameterValues[i];

    }

    return dMesh_dParameter;
 
}

/*##############################################################################
#                                                                              #
#                    CalculateOpenVSPGeometryGradients                         #
#                                                                              #
##############################################################################*/

void DeleteMeshGradients(int NumberOfDesignVariables, double **dMesh_dParameter)
{
    
    int i;
    
    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {

       delete [] dMesh_dParameter[i];
       
    }
    
    delete [] dMesh_dParameter;
    
}
    
/*##############################################################################
#                                                                              #
#                                   Normalize                                  #
#                                                                              #
##############################################################################*/

double Normalize(double *Vector, int Length)
{
   
   int i;
   double Mag;
   
   Mag = 0.;
   
   for ( i = 1 ; i <= Length ; i++ ) {
      
      Mag += Vector[i]*Vector[i];
      
   }
   
   Mag = sqrt(Mag);
   
   for ( i = 1 ; i <= Length ; i++ ) {
      
      Vector[i] /= Mag;
      
   }
   
   return Mag;
   
}

/*##############################################################################
#                                                                              #
#                                   CGState                                    #
#                                                                              #
##############################################################################*/

void CGState(double *Old, double *New, int Length)
{
   
   int i;
   double Dot;
   
   Dot = 0.;
   
   for ( i = 1 ; i <= Length ; i++ ) {
      
      Dot += Old[i]*New[i];
      
   }   

   for ( i = 1 ; i <= Length ; i++ ) {
      
      New[i] -= Dot * Old[i];
      
   }      

}

#endif
