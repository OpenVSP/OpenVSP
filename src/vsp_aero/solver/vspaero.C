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

#define VER_MAJOR 6
#define VER_MINOR 0
#define VER_PATCH 1

// Some globals...

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
double ClMax_;
double MaxTurningAngle_;
double FarDist_;
double TimeStep_;
double ReducedFrequency_;
double UnsteadyAngleMax_;
double UnsteadyHMax_;     
double HeightAboveGround_;
double BladeRPM_;

#define MAXRUNCASES 1000

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

double CMlForCase[MAXRUNCASES];
double CMmForCase[MAXRUNCASES];
double CMnForCase[MAXRUNCASES];

double CDoForCase[MAXRUNCASES];

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

int WakeIterations_          = 0;
int NumberOfRotors_          = 0;
int NumStabCases_            = 7;
int NumberOfThreads_         = 1;
int StabControlRun_          = 0;
int SetFreeStream_           = 0;
int SaveRestartFile_         = 0;
int DoRestartRun_            = 0;
int DoSymmetry_              = 0;
int SetFarDist_              = 0;
int Symmetry_                = 0;
int NumberOfWakeNodes_       = 0;
int DumpGeom_                = 0;
int NoWakeIteration_         = 0;
int NumberofSurveyPoints_    = 0;
int LoadFEMDeformation_      = 0;
int DoGroundEffectsAnalysis_ = 0;
int Write2DFEMFile_          = 0;
int DoUnsteadyAnalysis_      = 0;
int StartFromSteadyState_    = 0;
int UnsteadyAnalysisType_    = 0;
int NumberOfTimeSteps_       = 0;
int NumberOfTimeSamples_     = 0;
int RotorAnalysisRun_        = 0;
int CreateNoiseFiles_        = 0;

// Prototypes

int main(int argc, char **argv);
void PrintUsageHelp();
void ParseInput(int argc, char *argv[]);
void CreateInputFile(char *argv[], int argc, int &i);
void LoadCaseFile(void);
void ApplyControlDeflections(void);
void Solve(void);
void StabilityAndControlSolve(void);
void CalculateStabilityDerivatives(void);
void WriteOutVorviewFLTFile(void);
void UnsteadyStabilityAndControlSolve(void);
void RotorAnalysisSolve(void);
void Noise(void);

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

    double TotalTime;
    
    TotalTime = myclock();
    
    // Grab the file name
    
    FileName = argv[argc-1];

    // Parse the input

    ParseInput(argc,argv);

    // Output a header

    printf("VSPAERO v.%d.%d.%d --- Compiled on: %s at %s PST \n", VER_MAJOR, VER_MINOR, VER_PATCH, __DATE__, __TIME__);
    printf("\n\n\n\n");
    
#ifdef VSPAERO_OPENMP
    printf("Initializing OPENMP for %d threads \n",NumberOfThreads_);
   
    omp_set_num_threads(NumberOfThreads_);
    
    NumberOfThreads_ = omp_get_max_threads();

    printf("NumberOfThreads_: %d \n",NumberOfThreads_);
#else
    NumberOfThreads_ = 1;
    printf("Single threaded build.\n");
#endif
                    
    // Load in the case file

    LoadCaseFile();
        
    // Read in FEM deformation file
    
    if ( LoadFEMDeformation_ ) VSP_VLM().LoadFEMDeformation() = 1;
    
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
            
    // Load in the VSP degenerate geometry file
    
    VSP_VLM().ReadFile(FileName);
     
    // Geometry dump, no solver
    
    if ( DumpGeom_ ) VSP_VLM().DumpGeom() = 1;
    
    // Rotor Analysis... could be either steady state, or unsteady analysis
    
    if ( RotorAnalysisRun_ ) VSP_VLM().RotorAnalysis() = 1;
               
    // Force farfield distance for wake adaption
    
    if ( SetFarDist_ ) VSP_VLM().SetFarFieldDist(FarDist_);
    
    // Solve
    
    VSP_VLM().Setup();

    // Set number of farfield wake nodes
    
    if ( NumberOfWakeNodes_ > 0 ) VSP_VLM().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);

    // Force no wakes for some number of iterations
    
    if ( NoWakeIteration_ > 0 ) VSP_VLM().NoWakeIteration() = NoWakeIteration_;

    // Inform solver of Control Surface Groups ( this information is used in VSP_SOLVER::WriteCaseHeader(FILE *fid) )

    VSP_VLM().SetControlSurfaceGroup( ControlSurfaceGroup_, NumberOfControlGroups_ );

    // Stability and control run
    
    if ( StabControlRun_ == 1 ) {

       StabilityAndControlSolve();
 
    }
    
    // Unsteady stability and control run (pqr)
    
    else if ( StabControlRun_ == 2 ||
              StabControlRun_ == 3 ||
              StabControlRun_ == 4 ) {
     
       UnsteadyStabilityAndControlSolve();
       
    }
    
    // Steady state rotor analysis
    
    else if ( RotorAnalysisRun_ && !DoUnsteadyAnalysis_ ) {

       RotorAnalysisSolve();
       
    }
    
    // Create noise files
    
    else if ( CreateNoiseFiles_ == 1 ) {
       
       Noise();
       
    }
    
    else {

       Solve();
       
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
       printf(" -omp <N>           Use N processes.\n");
       printf(" -fs <M> END <A> END <B> END     Set/Override freestream Mach, Alpha, and Beta. note: M, A, and B are space delimited lists.\n");
       printf(" -setup             Write template *.vspaero file, can specify parameters below:\n");
       printf("     -sref  <S>        Reference area S.\n");
       printf("     -bref  <b>        Reference span b.\n");
       printf("     -cref  <c>        Reference chord c.\n");
       printf("     -cg  <X Y Z>      Moment reference point.\n");
       printf("     -mach <M> END     Freestream Mach number.  M is a space delimited list of mach values.\n");
       printf("     -aoa  <A> END     Angle of attack.  A is a space delimited list of aoa values.\n");
       printf("     -beta <B> END     Sideslip angle.  B is a space delimited list of beta values.\n");
       printf("     -wakeiters <N>    Number of wake iterations to calculate.\n");
       printf("     -symx             Symetry flag - vehicle is symetric at x=0.\n");
       printf("     -symy             Symetry flag - vehicle is symetric at y=0 (this is the most common).\n");
       printf("     -symz             Symetry flag - vehicle is symetric at z=0.\n");
       printf(" -save              Save restart file.\n");
       printf(" -restart           Restart analysis.\n");
       printf(" -geom              Process and write geometry without solving.\n");
       printf(" -dokt              Turn on the 2nd order Karman-Tsien Mach number correction. \n");
       printf(" -groundheight <H>  Do ground effects analysis with cg set to <H> height above the ground. \n");
       printf(" -fem               Load in FEM deformation file.\n");
       printf(" -write2dfem        Write out 2D FEM load file.\n");
       printf(" -stab              Calculate stability derivatives.\n");
       printf(" -pstab             Calculate unsteady rolling stability derivatives.\n");
       printf(" -qstab             Calculate unsteady pitching stability derivatives.\n");
       printf(" -rstab             Calculate unsteady yawing stability derivatives.\n");
       printf(" -nowake <N>        No wake for first N iterations.\n");
       printf(" -rotor <RPM>       Do a rotating frame rotor analysis, with specified rotor RPM.\n");
       printf(" -hoverramp <V1>    Decay freestream velocity from V1 to Vinf.\n");
       printf(" -unsteady          Do unsteady analysis.\n");
       printf(" -fromsteadystate   Start unsteady analysis from steady solution.\n");
       printf(" -noise             Do calculations for and write PSU-WOPWOP file.\n");
       printf("     -steady           Do steady state noise calcs.\n");
       printf("     -english          Assume geometry and VSPAERO inputs in english (ft lbf slug s) units, will convert to SI (m N kg s) for PSU-WOPWOP.\n");
       printf("     -flyby            Set up fly by noise analysis for PSU-WOPWOP.\n");
       printf("     -footprint        Set up footprint noise analysis for PSU-WOPWOP.\n");
       printf(" -jacobi            Use Jacobi matrix preconditioner for GMRES solve (not recommended).\n");
       printf(" -ssor              Use SSOR matrix preconditioner for GMRES solve (not recommended).\n");
       printf("\n");
       printf("EXAMPLES:\n");
       printf("Example: Creating a setup file for testModel with mach and alpha sweep matrix\n");
       printf("vspaero -setup -sref 10 -bref 10 -cref 1 -cg 0.1 0 0.1 -mach 0 0.1 0.3 END -aoa 1 5.5 10 END -beta 0 END testModel_DegenGeom\n");
       printf("\n");
       printf("Example: Solve testModel with 4 threads\n");
       printf("vspaero -omp 4 testModel_DegenGeom\n");
       printf("\n");
       printf("Example: Solve testModel with 4 threads and freestream overrides\n");
       printf("vspaero -omp 4 -fs 0.05 0.15 0.35 END 1 5.5 10 END 0 END testModel_DegenGeom\n");
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
    
    NumberOfMachs_     = 0;
    NumberOfAoAs_      = 0;
    NumberOfBetas_     = 0;
    NumberOfReCrefs_   = 0;    

    i = 1;

    while ( i <= argc - 2 ) {

       // Start up the graphics viewer after were all done

       if ( strcmp(argv[i],"-wtf") == 0 ) {

         printf("wtf back at you!  \n");   

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
          VSP_VLM().TimeAnalysisType()    = P_ANALYSIS;
          VSP_VLM().NumberOfTimeSteps()   = 32;
          VSP_VLM().Unsteady_AngleMax()   = 1.;
          VSP_VLM().Unsteady_HMax()       = 0.;    
          NumberOfWakeNodes_ = 64;
          VSP_VLM().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);

       }      
       
       else if ( strcmp(argv[i],"-qstab") == 0 ) {
        
          StabControlRun_ = 3;
          
          VSP_VLM().TimeAccurate() = DoUnsteadyAnalysis_ = 1;
          VSP_VLM().TimeAnalysisType()    = Q_ANALYSIS;
          VSP_VLM().NumberOfTimeSteps()   = 32;
          VSP_VLM().Unsteady_AngleMax()   = 1.;
          VSP_VLM().Unsteady_HMax()       = 0.;    
          NumberOfWakeNodes_ = 64;
          VSP_VLM().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);
                    
       }      
                     
       else if ( strcmp(argv[i],"-rstab") == 0 ) {
        
          StabControlRun_ = 4;
          
          VSP_VLM().TimeAccurate() = DoUnsteadyAnalysis_ = 1;
          VSP_VLM().TimeAnalysisType()    = R_ANALYSIS;
          VSP_VLM().NumberOfTimeSteps()   = 32;
          VSP_VLM().Unsteady_AngleMax()   = 1.;
          VSP_VLM().Unsteady_HMax()       = 0.;    
          NumberOfWakeNodes_ = 64;
          VSP_VLM().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);
                    
       }       
       
       else if ( strcmp(argv[i],"-unsteady") == 0 ) {

          VSP_VLM().TimeAccurate() = DoUnsteadyAnalysis_ = 1;

          VSP_VLM().TimeAnalysisType() = 0;

       }
       
       else if ( strcmp(argv[i],"-fromsteadystate") == 0 ) {

          VSP_VLM().StartFromSteadyState() = StartFromSteadyState_ = 1;

       }       
       
       else if ( strcmp(argv[i],"-rotor") == 0 ) {
       
          RotorAnalysisRun_ = 1;
          
          BladeRPM_ = atof(argv[++i]);
          
          VSP_VLM().BladeRPM() = BladeRPM_;
 
       }

       else if ( strcmp(argv[i],"-dokt") == 0 ) {
          
          VSP_VLM().KarmanTsienCorrection() = 1;
          
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
     
       else if ( strcmp(argv[i],"-save") == 0 ) {
        
          SaveRestartFile_ = 1;
          
       }     
       
       else if ( strcmp(argv[i],"-restart") == 0 ) {
        
          DoRestartRun_ = 1;
          
       }    
       
       else if ( strcmp(argv[i],"-geom") == 0 ) {
        
          DumpGeom_ = 1;
          
       }           

       else if ( strcmp(argv[i],"-setup") == 0 ) {

          printf("Creating vspaero input file... \n");fflush(NULL);
          
          CreateInputFile(argv,argc,i);
          
          exit(1);
          
       }                
       
       else if ( strcmp(argv[i],"-nowake") == 0 ) {

          NoWakeIteration_ = atoi(argv[++i]);
          
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

       else if ( strcmp(argv[i],"-jacobi") == 0 ) {
          
          VSP_VLM().Preconditioner() = JACOBI;
          
       }       
       
       else if ( strcmp(argv[i],"-ssor") == 0 ) {
          
          VSP_VLM().Preconditioner() = SSOR;
          
       }
       
       else if ( strcmp(argv[i],"-hoverramp") == 0 ) {
          
          VSP_VLM().DoHoverRampFreeStream() = atoi(argv[++i]);
          
          VSP_VLM().HoverRampFreeStreamVelocity() = atof(argv[++i]);
          
       }

       else if ( strcmp(argv[i],"-noise") == 0 ) {
          
          CreateNoiseFiles_ = 1;
          
          VSP_VLM().NoiseAnalysis() = 1;
          
          VSP_VLM().TimeAccurate() = DoUnsteadyAnalysis_ = 1;

          VSP_VLM().TimeAnalysisType() = 0;
          
          // Iterate... order of items on command line is uncertain
          
          for ( iter = 1 ; iter <= 4 ; iter++ ) {
             
             // Check if we are doing a steady state calculation
             
             if ( strcmp(argv[i+1],"-steady") == 0 ) {
                
                printf("Doing steady state noise calculation \n");
                
                VSP_VLM().SteadyStateNoise() = 1;
                
                i++;
                
             }
             
             // Check if we are to use english units
   
             if ( strcmp(argv[i+1],"-english") == 0 ) {
                
                printf("Geometry and inputs are in english units... will convert to SI for noise analysis. \n");
                
                VSP_VLM().WopWopEnglishUnits() = 1;
                
                i++;
                
             }
             
             // Check if we are to do fly by
   
             if ( strcmp(argv[i+1],"-flyby") == 0 ) {
                
                printf("Doing fly by noise analysis.");
                
                VSP_VLM().DoWopWopFlyBy();;
                
                i++;
                
             }             

             // Check if we are to do foot print
   
             if ( strcmp(argv[i+1],"-footprint") == 0 ) {
                
                printf("Doing foot print noise analysis. \n");
                
                VSP_VLM().DoWopWopFootPrint();
                
                i++;
                
             }    
                          
          }
                    
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
#                                   CreateInputFile                            #
#                                                                              #
##############################################################################*/

void CreateInputFile(char *argv[], int argc, int &i)
{

    int k, p, k2, p2, NumberOfControlGroups, Group, NumControls;
    int NumberOfUsedRotors, *RotorIsUsed;
    FILE *case_file;
    char file_name_w_ext[2000], SymmetryFlag[80];
    
    // Defaults
    
    Sref_              = 1.0;
    Cref_              = 1.0;  
    Bref_              = 1.0; 
    Xcg_               = 0.0;
    Ycg_               = 0.0;
    Zcg_               = 0.0;
    Mach_              = 0.3;
    AoA_               = 5.0;
    Beta_              = 0.0;
    Vinf_              = 100.;
    Vref_              = 100.;
    Machref_           = 0.3;    
    Rho_               = 0.002377;
    ReCref_            = 10000000.;
    ClMax_             = -1.;
    MaxTurningAngle_   = -1.;
    FarDist_           = -1.;
    NumberOfWakeNodes_ = -1;
    WakeIterations_    = 5;
    NumberOfRotors_    = 0;
    
    NumberOfMachs_     = 1;
    NumberOfAoAs_      = 1;
    NumberOfBetas_     = 1;    
    
    MachList_[1] = Mach_;
    AoAList_[1] = AoA_;
    BetaList_[1] = Beta_;

    sprintf(SymmetryFlag,"NO");
        
    // Read in the degen geometry file
    
    VSP_VLM().ReadFile(FileName);

    // Open the case file

    sprintf(file_name_w_ext,"%s.vspaero",FileName);

    if ( (case_file = fopen(file_name_w_ext,"w")) == NULL ) {

       printf("Could not open the file: %s for writing! \n",file_name_w_ext);

       exit(1);

    }
    
    // Check for any user supplied values on the command line
    
    while ( i <= argc - 2 ) {
    
       if ( strcmp(argv[i],"-sref") == 0 ) {
          
          Sref_ = atof(argv[++i]);
          
       }
       
       else if ( strcmp(argv[i],"-cref") == 0 ) {
          
          Cref_ = atof(argv[++i]);
          
       }
       
       else if ( strcmp(argv[i],"-bref") == 0 ) {
          
          Bref_ = atof(argv[++i]);
          
       }                     

       else if ( strcmp(argv[i],"-cg") == 0 ) {
          
          Xcg_ = atof(argv[++i]);
          
          Ycg_ = atof(argv[++i]);
          
          Zcg_ = atof(argv[++i]);
          
       }     
       
       else if ( strcmp(argv[i],"-mach") == 0 ) {
          
          NumberOfMachs_ = 0;
          
          while ( strcmp(argv[i+1],"END") != 0 ) {
          
             MachList_[++NumberOfMachs_] = atof(argv[++i]);
          
          }
          
       }     
       
       else if ( strcmp(argv[i],"-aoa") == 0 ) {
          
          NumberOfAoAs_ = 0;
          
          while ( strcmp(argv[i+1],"END") != 0 ) {
          
             AoAList_[++NumberOfAoAs_] = atof(argv[++i]);
             
          }

       }     
       
       else if ( strcmp(argv[i],"-beta") == 0 ) {
          
          NumberOfBetas_ = 0;    
          
          while ( strcmp(argv[i+1],"END") != 0 ) {
          
             BetaList_[++NumberOfBetas_] = atof(argv[++i]);
             
          }          
          
       }    
       
       else if ( strcmp(argv[i],"-symx") == 0 ) {
          
          sprintf(SymmetryFlag,"X");
          
          DoSymmetry_ = SYM_X;
          
       }         
         
       else if ( strcmp(argv[i],"-symy") == 0 ) {
          
          sprintf(SymmetryFlag,"Y");
          
          DoSymmetry_ = SYM_Y;
          
       }  
       
       else if ( strcmp(argv[i],"-symz") == 0 ) {
          
          sprintf(SymmetryFlag,"Z");
          
          DoSymmetry_ = SYM_Z;
          
       }  
                     
       else if ( strcmp(argv[i],"-wakeiters") == 0 ) {

          WakeIterations_ = atoi(argv[++i]);

       }
       i++; 
       
    }
                                
    fprintf(case_file,"Sref = %lf \n",Sref_);
    fprintf(case_file,"Cref = %lf \n",Cref_);
    fprintf(case_file,"Bref = %lf \n",Bref_);
    fprintf(case_file,"X_cg = %lf \n",Xcg_);
    fprintf(case_file,"Y_cg = %lf \n",Ycg_);
    fprintf(case_file,"Z_cg = %lf \n",Zcg_);
    
    fprintf(case_file,"Mach = "); for ( i = 1 ; i < NumberOfMachs_ ; i++ ) { fprintf(case_file,"%lf, ",MachList_[i]); }; fprintf(case_file,"%lf \n",MachList_[NumberOfMachs_]);
    fprintf(case_file,"AoA = ");  for ( i = 1 ; i < NumberOfAoAs_  ; i++ ) { fprintf(case_file,"%lf, ", AoAList_[i]); }; fprintf(case_file,"%lf \n", AoAList_[NumberOfAoAs_]);
    fprintf(case_file,"Beta = "); for ( i = 1 ; i < NumberOfBetas_ ; i++ ) { fprintf(case_file,"%lf, ",BetaList_[i]); }; fprintf(case_file,"%lf \n",BetaList_[NumberOfBetas_]);
    
    fprintf(case_file,"Vinf = %lf \n",Vinf_);
    fprintf(case_file,"Vref = %lf \n",Vref_);
    fprintf(case_file,"Machref = %lf \n",Machref_);
    fprintf(case_file,"Rho = %lf \n",Rho_);
    fprintf(case_file,"ReCref = %lf \n",ReCref_);
    fprintf(case_file,"ClMax = %lf \n",ClMax_);
    fprintf(case_file,"MaxTurningAngle = %lf \n",MaxTurningAngle_);
    fprintf(case_file,"Symmetry = %s \n",SymmetryFlag);
    fprintf(case_file,"FarDist = %lf \n",FarDist_);
    fprintf(case_file,"NumWakeNodes = %d \n",NumberOfWakeNodes_);
    fprintf(case_file,"WakeIters = %d \n",WakeIterations_);
    
    printf("VSP_VLM().VSPGeom().NumberOfRotors(): %d \n",VSP_VLM().VSPGeom().NumberOfRotors());
    
    if ( VSP_VLM().VSPGeom().NumberOfRotors() != 0 ) NumberOfRotors_ = VSP_VLM().VSPGeom().NumberOfRotors();
 
    NumberOfUsedRotors = 0;
       
    // Adjust rotors for symmetry
    
    if ( NumberOfRotors_ > 0 ) {
       
       RotorIsUsed = new int[NumberOfRotors_ + 1];
       
       zero_int_array(RotorIsUsed, NumberOfRotors_);
    
       if ( DoSymmetry_ == SYM_X ) {
          
          for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
             
             if ( VSP_VLM().VSPGeom().RotorDisk(i).XYZ(0) >= 0. ) { NumberOfUsedRotors++ ; RotorIsUsed[i] = 1; };
             
          }
          
       }
       
       if ( DoSymmetry_ == SYM_Y ) {
                 
          for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
             
             if ( VSP_VLM().VSPGeom().RotorDisk(i).XYZ(1) >= 0. ) { NumberOfUsedRotors++ ; RotorIsUsed[i] = 1; };
             
          }
          
       }
       
       if ( DoSymmetry_ == SYM_Z ) {
          
          for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
             
             if ( VSP_VLM().VSPGeom().RotorDisk(i).XYZ(2) >= 0. ) { NumberOfUsedRotors++ ; RotorIsUsed[i] = 1; };
             
          }
          
       }       
       
       if ( DoSymmetry_ == 0 ) {
          
          for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
             
             NumberOfUsedRotors++ ; RotorIsUsed[i] = 1;;
             
          }
          
       }       
       
       fprintf(case_file,"NumberOfRotors = %d \n",NumberOfUsedRotors);
       
       for ( i = 1 ;  i <= NumberOfRotors_ ; i++ ) {
          
          if ( RotorIsUsed[i] ) {
             
             fprintf(case_file,"PropElement_%-d\n",i);
             fprintf(case_file,"%d\n",i);
             
             VSP_VLM().VSPGeom().RotorDisk(i).Write_STP_Data(case_file);
             
          }
          
       }
        
       delete [] RotorIsUsed;
       
    }
    
    else {
       
       fprintf(case_file,"NumberOfRotors = %d \n",NumberOfUsedRotors);
       
    }
    
    // Control surface groups
    
    NumberOfControlGroups = 0;
    
    for ( k = 1 ; k <= VSP_VLM().VSPGeom().NumberOfSurfaces() ; k++ ) {
            
      for ( p = 1 ; p <= VSP_VLM().VSPGeom().VSP_Surface(k).NumberOfControlSurfaces() ; p++ ) {
         
         NumberOfControlGroups++;
         
      }
      
    }

    for ( k = 1 ; k <= VSP_VLM().VSPGeom().NumberOfSurfaces() ; k++ ) {
            
       for ( p = 1 ; p <= VSP_VLM().VSPGeom().VSP_Surface(k).NumberOfControlSurfaces() ; p++ ) {
          
          for ( k2 = k ; k2 <= VSP_VLM().VSPGeom().NumberOfSurfaces() ; k2++ ) {
            
             for ( p2 = p ; p2 <= VSP_VLM().VSPGeom().VSP_Surface(k2).NumberOfControlSurfaces() ; p2++ ) {
                
                if ( (p != p2) || ( k != k2 ) ) {
            
                   if ( strcmp(VSP_VLM().VSPGeom().VSP_Surface(k ).ControlSurface(p ).Name(),
                               VSP_VLM().VSPGeom().VSP_Surface(k2).ControlSurface(p2).Name() ) == 0 ) {

                       printf( "\nERROR - duplicate control surface name: %s\n", VSP_VLM().VSPGeom().VSP_Surface( k ).ControlSurface( p ).Name() );
                       printf( "\tSurface 1 [VSP_Surface].[ControlSurface]: %s.%s\n", VSP_VLM().VSPGeom().VSP_Surface( k ).ComponentName(), VSP_VLM().VSPGeom().VSP_Surface( k ).ControlSurface( p ).Name() );
                       printf( "\tSurface 2 [VSP_Surface].[ControlSurface]: %s.%s\n", VSP_VLM().VSPGeom().VSP_Surface( k2 ).ComponentName(), VSP_VLM().VSPGeom().VSP_Surface( k2 ).ControlSurface( p2 ).Name() );

                       NumberOfControlGroups--; // this ensures unique names for all sub-surfaces deflecting a control surface requires a unique name
                      
                   }
                               
                }
                
             }
             
          }
          
       }
       
    }

    fprintf(case_file,"NumberOfControlGroups = %d \n",NumberOfControlGroups);

    // Group control surfaces and write successful groupings to string buffer

    char tempStrBuf[2000];
    char controlGroupStrBuf[10000];
    strcpy( controlGroupStrBuf, "\0" );
   
    // Control surface groups
   
    NumberOfControlGroups = 0;
   
    Group = 0;
   
    for ( k = 1 ; k <= VSP_VLM().VSPGeom().NumberOfSurfaces() ; k++ ) {
        
       for ( p = 1 ; p <= VSP_VLM().VSPGeom().VSP_Surface(k).NumberOfControlSurfaces() ; p++ ) {
   
          if ( VSP_VLM().VSPGeom().VSP_Surface(k).ControlSurface(p).ControlGroup() == 0 ) {
      
             Group++;
      
             NumControls = 1;
   
             sprintf( tempStrBuf,"ControlGroup_%d\n",Group);
             strcat( controlGroupStrBuf, tempStrBuf );
   
             sprintf( tempStrBuf,"%s",VSP_VLM().VSPGeom().VSP_Surface(k).ControlSurface(p).Name());
             strcat( controlGroupStrBuf, tempStrBuf );
         
             for ( k2 = k ; k2 <= VSP_VLM().VSPGeom().NumberOfSurfaces() ; k2++ ) {
           
                for ( p2 = p ; p2 <= VSP_VLM().VSPGeom().VSP_Surface(k2).NumberOfControlSurfaces() ; p2++ ) {
               
                   if ( (p != p2) || ( k != k2 ) ) {
   
                       // Compare parent component name then compare surface short name for grouping
                       //   Note: the Name field gets filled with a unique name in OpenVSP versions 
                       //   where the geomID is appended to the name.  This allows for fewer naming 
                       //   restrictions on the user
                       
                       if ( strcmp( VSP_VLM().VSPGeom().VSP_Surface( k  ).ComponentName(),
                                    VSP_VLM().VSPGeom().VSP_Surface( k2 ).ComponentName() ) == 0 ){
   
                           if ( strcmp( VSP_VLM().VSPGeom().VSP_Surface( k  ).ControlSurface( p ).ShortName(),
                                        VSP_VLM().VSPGeom().VSP_Surface( k2 ).ControlSurface( p2 ).ShortName() ) == 0 ){
   
                             sprintf( tempStrBuf,",%s", VSP_VLM().VSPGeom().VSP_Surface( k2 ).ControlSurface( p2 ).Name() );
                             strcat( controlGroupStrBuf, tempStrBuf );
                     
                             NumControls++;
                     
                             VSP_VLM().VSPGeom().VSP_Surface(k ).ControlSurface(p ).ControlGroup() = Group;
                             VSP_VLM().VSPGeom().VSP_Surface(k2).ControlSurface(p2).ControlGroup() = Group;
                     
                          }
   
                       }
   
                   }
               
                }
            
             }
         
             strcat( controlGroupStrBuf, "\n" );
         
             for ( i = 1 ; i < NumControls ; i++ ) {
                
                strcat( controlGroupStrBuf,"1., ");
                
             }
                
             strcat( controlGroupStrBuf,"1. \n");
         
             strcat( controlGroupStrBuf, "10. \n" );
         
          }
      
       }
   
    }
    
    NumberOfControlGroups = Group;
   
    // Final writing of control surface grouping to the file if successful at grouping

    fprintf( case_file, "%s", controlGroupStrBuf );

    fclose(case_file);       
 
}

/*##############################################################################
#                                                                              #
#                                      LoadCaseFile                            #
#                                                                              #
##############################################################################*/

void LoadCaseFile(void)
{

    int i, j, NumberOfControlSurfaces, Done;
    double x,y,z, DumDouble, HingeVec[3], RotAngle, DeltaHeight;
    FILE *case_file;
    char file_name_w_ext[2000], DumChar[2000], DumChar2[2000], Comma[2000], *Next;
    char SymmetryFlag[2000];
    QUAT Quat, InvQuat, Vec;

    // Delimiters
    
    sprintf(Comma,",");

    // Open the case file

    sprintf(file_name_w_ext,"%s.vspaero",FileName);

    if ( (case_file = fopen(file_name_w_ext,"r")) == NULL ) {

       printf("Could not open the file: %s for input! \n",file_name_w_ext);

       exit(1);

    }

    Sref_              = 1.0;
    Cref_              = 1.0;  
    Bref_              = 1.0; 
    Xcg_               = 0.0;
    Ycg_               = 0.0;
    Zcg_               = 0.0;
    Mach_              = 0.3;
    AoA_               = 5.0;
    Beta_              = 0.0;
    Vinf_              = 100.;
    Vref_              = 100.;
    Machref_           = 0.3;
    Rho_               = 0.002377;
    ReCref_            = 10000000.;
    ClMax_             = -1.;
    MaxTurningAngle_   = -1.;
    FarDist_           = -1.;
    NumberOfWakeNodes_ = -1;
    WakeIterations_    = 5;

    fscanf(case_file,"Sref = %lf \n",&Sref_);
    fscanf(case_file,"Cref = %lf \n",&Cref_);
    fscanf(case_file,"Bref = %lf \n",&Bref_);
    fscanf(case_file,"X_cg = %lf \n",&Xcg_);
    fscanf(case_file,"Y_cg = %lf \n",&Ycg_);
    fscanf(case_file,"Z_cg = %lf \n",&Zcg_);

    // Load in Mach list
    
    fgets(DumChar,2000,case_file);

    if ( strstr(DumChar,Comma) == NULL ) {

       sscanf(DumChar,"Mach = %lf \n",&DumDouble);
       
       if ( !SetFreeStream_ ) {
          
          NumberOfMachs_ = 1;
       
          MachList_[1] = Mach_ = DumDouble;  printf("Mach: %lf \n",MachList_[NumberOfMachs_]);
          
       }
       
       else {
          
          printf("Mach list overridden by command line -fs inputs \n");
          
       }
       
    }
    
    else {
       
       Next = strtok(DumChar,Comma);

       sscanf(DumChar,"Mach = %lf \n",&DumDouble);
       
       if ( !SetFreeStream_ ) {
          
          NumberOfMachs_ = 1;
       
          MachList_[1] = Mach_ = DumDouble;  printf("Mach: %lf \n",MachList_[NumberOfMachs_]);
          
       }
       
       else {
          
          printf("Mach list overridden by command line -fs inputs \n");
          
       }          
       
       while ( Next != NULL ) {
          
           Next = strtok(NULL,Comma);
           
           if ( Next != NULL ) {
              
              DumDouble = atof(Next);
              
              if ( !SetFreeStream_ ) {
                 
                 MachList_[++NumberOfMachs_] = DumDouble ;  printf("Mach: %lf \n",MachList_[NumberOfMachs_]);
             
              }
             
              else {
                
                 printf("Mach list overridden by command line -fs inputs \n");
                
              }   
                    
           }
           
       }
       
       Mach_ = MachList_[1];
       
    }
    
    // Load in AoA list
    
    fgets(DumChar,2000,case_file);

    if ( strstr(DumChar,Comma) == NULL ) {

       sscanf(DumChar,"AoA = %lf \n",&DumDouble);
       
       if ( !SetFreeStream_ ) {
          
          NumberOfAoAs_ = 1;
       
          AoAList_[1] = AoA_ = DumDouble;  printf("AoA: %lf \n",AoAList_[NumberOfAoAs_]);

       }
       
       else {
          
          printf("Alpha list overridden by command line -fs inputs \n");
          
       }   
              
    }
    
    else {

       Next = strtok(DumChar,Comma);
       
       sscanf(DumChar,"AoA = %lf \n",&DumDouble);
       
       if ( !SetFreeStream_ ) {
          
          NumberOfAoAs_ = 1;
       
          AoAList_[1] = AoA_ = DumDouble;  printf("AoA: %lf \n",AoAList_[NumberOfAoAs_]);

       }
       
       else {
          
          printf("Alpha list overridden by command line -fs inputs \n");
          
       }   
              
       while ( Next != NULL ) {
          
           Next = strtok(NULL,Comma);
           
           if ( Next != NULL ) {
              
              DumDouble = atof(Next);
              
              if ( !SetFreeStream_ ) {              
              
                 AoAList_[++NumberOfAoAs_] = DumDouble;
              
              }
             
              else {
                
                 printf("Alpha list overridden by command line -fs inputs \n");
                
              }   
                            
           }
           
       }
       
       AoA_ = AoAList_[1];
       
    }    
    
    // Load in Beta list
    
    fgets(DumChar,2000,case_file);

    if ( strstr(DumChar,Comma) == NULL ) {

       sscanf(DumChar,"Beta = %lf \n",&DumDouble);
       
       if ( !SetFreeStream_ ) {       
          
          NumberOfBetas_ = 1;
       
          BetaList_[1] = DumDouble;
          
       }

       else {
          
          printf("Beta list overridden by command line -fs inputs \n");
          
       }   
              
    }
    
    else {

       Next = strtok(DumChar,Comma);
       
       sscanf(DumChar,"Beta = %lf \n",&DumDouble);
       
       if ( !SetFreeStream_ ) {
          
          NumberOfBetas_ = 1;
                 
          BetaList_[1] = Beta_ = DumDouble;  printf("Beta: %lf \n",BetaList_[NumberOfBetas_]);
          
       }
          
       else {
          
          printf("Beta list overridden by command line -fs inputs \n");
          
       }             
       
       while ( Next != NULL ) {
          
           Next = strtok(NULL,Comma);
           
           if ( Next != NULL ) {
              
              DumDouble = atof(Next);
              
              if ( !SetFreeStream_ ) {                       
              
                 BetaList_[++NumberOfBetas_] = DumDouble;
                 
              }
              
              else {
                
                 printf("Beta list overridden by command line -fs inputs \n");
                
              }                 
              
           }
           
       }
       
       Beta_ = BetaList_[1];
       
    }    

    fscanf(case_file,"Vinf = %lf \n",&Vinf_);
    
    // Look for Vref
    
    fgets(DumChar,2000,case_file);
    
    if ( strstr(DumChar,"Vref") != NULL ) {
       
       sscanf(DumChar,"Vref = %lf \n",&Vref_);

       // Look for Machref
       
       fgets(DumChar,2000,case_file);
       
       if ( strstr(DumChar,"Machref") != NULL ) {
          
          sscanf(DumChar,"Machref = %lf \n",&Machref_);
          
          fscanf(case_file,"Rho = %lf \n",&Rho_);
          
       }
       
       else {
          
          printf("Machref must be defined if Vref is defined \n");
          fflush(NULL);
          exit(1);
          
       }
    
    }
    
    else {
       
       Vref_ = Vinf_;
       
       Machref_ = -1.;
       
       sscanf(DumChar,"Rho = %lf \n",&Rho_);
       
    }

    // Load in ReCref list
    
    fgets(DumChar,2000,case_file);

    if ( strstr(DumChar,Comma) == NULL ) {


       sscanf(DumChar,"ReCref = %lf \n",&DumDouble);

       NumberOfReCrefs_ = 1;
    
       ReCrefList_[1] = ReCref_ = DumDouble;  printf("ReCref_: %lf \n",ReCrefList_[NumberOfReCrefs_]);
           
    }
    
    else {

       Next = strtok(DumChar,Comma);
       
       sscanf(DumChar,"ReCref = %lf \n",&DumDouble);
    
       NumberOfReCrefs_ = 1;
    
       ReCrefList_[1] = ReCref_ = DumDouble;  printf("ReCref_: %lf \n",ReCrefList_[NumberOfReCrefs_]);
              
       while ( Next != NULL ) {
          
           Next = strtok(NULL,Comma);
           
           if ( Next != NULL ) {
              
              DumDouble = atof(Next);
    
              ReCrefList_[++NumberOfReCrefs_] = DumDouble;  printf("ReCref_: %lf \n",ReCrefList_[NumberOfReCrefs_]);
       
           }
           
       }
       
       ReCref_ = ReCrefList_[1];
       
    }      
    
    fscanf(case_file,"ClMax = %lf \n",&ClMax_);
    fscanf(case_file,"MaxTurningAngle = %lf \n",&MaxTurningAngle_);
    fscanf(case_file,"Symmetry = %s \n",SymmetryFlag);
    fscanf(case_file,"FarDist = %lf \n",&FarDist_);
    fscanf(case_file,"NumWakeNodes = %d \n",&NumberOfWakeNodes_);
    fscanf(case_file,"WakeIters = %d \n",&WakeIterations_);
    
    if ( FarDist_ > 0. ) SetFarDist_ = 1;
    
    if ( WakeIterations_ <= 3 ) WakeIterations_ = 3;
    
    if ( ClMax_ <= 0. ) ClMax_ = -1.;
    
    if ( MaxTurningAngle_ <= 0. ) MaxTurningAngle_ = -1.;

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
    
    printf("ClMax           = %lf \n",ClMax_);
    printf("MaxTurningAngle = %lf \n",MaxTurningAngle_);
    printf("Symmetry        = %s  \n",SymmetryFlag);
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
    
    VSP_VLM().ClMax() = ClMax_;
    
    VSP_VLM().MaxTurningAngle() = MaxTurningAngle_;
    
    VSP_VLM().WakeIterations() = WakeIterations_;
    
    VSP_VLM().RotationalRate_p() = 0.0;
    VSP_VLM().RotationalRate_q() = 0.0;
    VSP_VLM().RotationalRate_r() = 0.0;    
    
    // Symmetry options
    
    if ( strcmp(SymmetryFlag,"X") == 0 ) DoSymmetry_ = SYM_X;
    if ( strcmp(SymmetryFlag,"Y") == 0 ) DoSymmetry_ = SYM_Y;
    if ( strcmp(SymmetryFlag,"Z") == 0 ) DoSymmetry_ = SYM_Z;
        
    if ( DoSymmetry_ == SYM_X ) VSP_VLM().DoSymmetryPlaneSolve(SYM_X);
    if ( DoSymmetry_ == SYM_Y ) VSP_VLM().DoSymmetryPlaneSolve(SYM_Y);
    if ( DoSymmetry_ == SYM_Z ) VSP_VLM().DoSymmetryPlaneSolve(SYM_Z);
    
    // Load in the control surface data
    
    rewind(case_file);
    
    NumberOfControlGroups_ = 0;

    while ( fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfControlGroups") != NULL ) {
          
          sscanf(DumChar,"NumberOfControlGroups = %d \n",&NumberOfControlGroups_);

          printf( "NumberOfControlGroups = %d \n", NumberOfControlGroups_ );

          if (NumberOfControlGroups_ < 0) {

              printf( "INVALID NumberOfControlGroups: %d\n", NumberOfControlGroups_ );

              exit( 1 );

          }

          ControlSurfaceGroup_ = new CONTROL_SURFACE_GROUP[NumberOfControlGroups_ + 1];
          
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) {
             
             // Control groupname
             
             fgets( ControlSurfaceGroup_[i].Name(), 200, case_file );
             
             printf("ControlSurfaceGroup_[%d].Name(): %s \n",i,ControlSurfaceGroup_[i].Name());
             
             // List of control surfaces in this group
             
             fgets(DumChar,300,case_file);
        
             // Determine the number of control surfaces in the group
         
             if ( strstr(DumChar,Comma) == NULL ) {
         
                NumberOfControlSurfaces = 1;
                
                ControlSurfaceGroup_[i].SizeList(NumberOfControlSurfaces);

                printf( "There is %d control surface in group: %d \n", NumberOfControlSurfaces, i );
                
                DumChar[strcspn(DumChar, "\n")] = 0;
                
                sprintf(ControlSurfaceGroup_[i].ControlSurface_Name(1),"%s",DumChar);
                
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
                
                sprintf(DumChar2,"%s",DumChar);
            
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
                
                sprintf(DumChar,"%s",DumChar2);
                
                Next = strtok(DumChar,Comma);
  
                NumberOfControlSurfaces = 1;
                
                sprintf(ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces),"%s",Next);
                
                ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(NumberOfControlSurfaces) = 1;
                
                printf("Control surface(%d): %s___ \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces));
   
                while ( Next != NULL ) {
                   
                    Next = strtok(NULL,",\n");
         
                    if ( Next != NULL ) {
                       
                       NumberOfControlSurfaces++;

                       sprintf(ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces),"%s", Next );
                
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
    
    // Load in unsteady aero data
    
    rewind(case_file);
    
    Done = 0;

    if ( VSP_VLM().TimeAnalysisType() == 0 ) {

       while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {
   
          if ( strstr(DumChar,"TimeStep") != NULL ) {
   
             printf("Unsteady analysis data: \n");
                 
             sscanf(DumChar,"TimeStep = %lf \n",&TimeStep_);
             
             printf("TimeStep: %f \n",TimeStep_);
                                                  
             fgets(DumChar,2000,case_file);
             
             if ( strstr(DumChar, "NumberOfTimeSteps") != NULL) {
                
                 sscanf(DumChar,"NumberOfTimeSteps = %d \n",&NumberOfTimeSteps_);
                 
                 printf("NumberOfTimeSteps: %d \n",NumberOfTimeSteps_);
                 
                 Done = 1;
                 
             }
   
             VSP_VLM().TimeStep()          = TimeStep_;
             VSP_VLM().NumberOfTimeSteps() = NumberOfTimeSteps_;
   
          }
                  
       }
       
    }
        
    fclose(case_file);
    
}

/*##############################################################################
#                                                                              #
#                           ApplyControlDeflections                            #
#                                                                              #
##############################################################################*/

void ApplyControlDeflections()
{
    int i, j, k, p, Found;

    // Set control surface deflections

    for ( i = 1; i <= NumberOfControlGroups_; i++ )
    {

        for ( j = 1; j <= ControlSurfaceGroup_[i].NumberOfControlSurfaces(); j++ )
        {

            Found = 0;
            
            k = 1;

            while ( k <= VSP_VLM().VSPGeom().NumberOfSurfaces() && !Found )
            {

                for ( p = 1; p <= VSP_VLM().VSPGeom().VSP_Surface( k ).NumberOfControlSurfaces(); p++ )
                {

                    if ( strcmp( ControlSurfaceGroup_[i].ControlSurface_Name( j ), VSP_VLM().VSPGeom().VSP_Surface( k ).ControlSurface( p ).Name() ) == 0 )
                    {

                        Found = 1;

                        VSP_VLM().VSPGeom().VSP_Surface( k ).ControlSurface( p ).DeflectionAngle() = ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection( j ) * ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle() * TORAD;

                    }

                }

                k++;

            }

            // Print out error report
            if ( !Found )
            {
                printf( "Could not find control surface: %s in control surface group: %s \n",
                    ControlSurfaceGroup_[i].ControlSurface_Name( j ),
                    ControlSurfaceGroup_[i].Name() );

                // print out names of all known surfaces
                printf( "Known control surfaces:\n" );
                for ( k = 1; k <= VSP_VLM().VSPGeom().NumberOfSurfaces(); k++ )
                {
                    for ( p = 1; p <= VSP_VLM().VSPGeom().VSP_Surface( k ).NumberOfControlSurfaces(); p++ )
                    {
                        printf( "\t%20s\n", VSP_VLM().VSPGeom().VSP_Surface( k ).ControlSurface( p ).Name() );
                    }
                }
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
             
             sprintf(VSP_VLM().CaseString(),"Case: %-d ...",Case);
             
             if ( DoGroundEffectsAnalysis_ ) sprintf(VSP_VLM().CaseString(),"AoA: %7.3f ... H: %8.3f",-VSP_VLM().VehicleRotationAngleVector(1),HeightAboveGround_);

             // Solve this case
             
             if ( SaveRestartFile_ ) VSP_VLM().SaveRestartFile() = 1;
   
             if ( DoRestartRun_    ) VSP_VLM().DoRestart() = 1;

             if ( Case <= NumCases ) {
                
                VSP_VLM().Solve(Case);
                
             }
             
             else {
                
                VSP_VLM().Solve(-Case);
                
             }
       
             // Store aero coefficients
        
             CLForCase[Case] = VSP_VLM().CL(); 
             CDForCase[Case] = VSP_VLM().CD();        
             CSForCase[Case] = VSP_VLM().CS();        
      
             CFxForCase[Case] = VSP_VLM().CFx();
             CFyForCase[Case] = VSP_VLM().CFy();       
             CFzForCase[Case] = VSP_VLM().CFz();       
                 
             CMxForCase[Case] = VSP_VLM().CMx();       
             CMyForCase[Case] = VSP_VLM().CMy();       
             CMzForCase[Case] = VSP_VLM().CMz();     
             
             CMlForCase[Case] = -VSP_VLM().CMx();       
             CMmForCase[Case] =  VSP_VLM().CMy();       
             CMnForCase[Case] = -VSP_VLM().CMz();     

             CDoForCase[Case] = VSP_VLM().CDo();     
             
             // Loop over any ReCref cases
             
             for ( p = 2 ; p <= NumberOfReCrefs_ ; p++ ) {
                
                Case++;
                
                CaseList[i][j][k][p] = Case;
                
                ReCref_ = ReCrefList_[p];
     
                VSP_VLM().ReCref() = ReCref_;
                
                VSP_VLM().ReCalculateForces();
                
                CLForCase[Case] = VSP_VLM().CL(); 
                CDForCase[Case] = VSP_VLM().CD();        
                CSForCase[Case] = VSP_VLM().CS();        
         
                CFxForCase[Case] = VSP_VLM().CFx();
                CFyForCase[Case] = VSP_VLM().CFy();       
                CFzForCase[Case] = VSP_VLM().CFz();       
                    
                CMxForCase[Case] = VSP_VLM().CMx();       
                CMyForCase[Case] = VSP_VLM().CMy();       
                CMzForCase[Case] = VSP_VLM().CMz();     
                
                CMlForCase[Case] = -VSP_VLM().CMx();       
                CMmForCase[Case] =  VSP_VLM().CMy();       
                CMnForCase[Case] = -VSP_VLM().CMz();     
   
                CDoForCase[Case] = VSP_VLM().CDo();   
        
             } 
             
             ReCref_ = ReCrefList_[1];
  
             VSP_VLM().ReCref() = ReCref_;             
                
             printf("\n");
      
          }
          
       }
       
    }

    // Write out final integrated force data
    
    sprintf(PolarFileName,"%s.polar",FileName);

    if ( (PolarFile = fopen(PolarFileName,"w")) == NULL ) {

       printf("Could not open the polar file output! \n");

       exit(1);

    }    

                     //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789   
    fprintf(PolarFile,"  Beta      Mach       AoA      Re/1e6     CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       CMl       CMm       CMn \n");

    // Write out polars, not these are written out in a different order than they were calculated above - we group them by Re number
    
    for ( p = 1 ; p <= NumberOfReCrefs_ ; p++ ) {

       for ( i = 1 ; i <= NumberOfBetas_ ; i++ ) {
          
          for ( j = 1 ; j <= NumberOfMachs_; j++ ) {
                
             for ( k = 1 ; k <= NumberOfAoAs_ ; k++ ) {
 
                Case = CaseList[i][j][k][p];
                   
                AR = Bref_ * Bref_ / Sref_;
   
                E = ( CLForCase[Case] *CLForCase[Case] / ( PI * AR) ) / CDForCase[Case];
                
                fprintf(PolarFile,"%9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",             
                        BetaList_[i],
                        MachList_[j],
                        AoAList_[k],
                        ReCrefList_[p]/1.e6,
                        CLForCase[Case],
                        CDoForCase[Case],
                        CDForCase[Case],
                        CDoForCase[Case] + CDForCase[Case],
                        CSForCase[Case],            
                        CLForCase[Case]/(CDoForCase[Case] + CDForCase[Case]),
                        E,
                        CFxForCase[Case],
                        CFyForCase[Case],
                        CFzForCase[Case],
                        CMxForCase[Case],
                        CMyForCase[Case],
                        CMzForCase[Case],
                        CMlForCase[Case],       
                        CMmForCase[Case],       
                        CMnForCase[Case]);
                           
             }
             
          }
          
       }       
       
    }
    
    fclose(PolarFile);

}

/*##############################################################################
#                                                                              #
#                           StabilityAndControlSolve                           #
#                                                                              #
##############################################################################*/

void StabilityAndControlSolve(void)
{

    int i, j, k, p, ic, jc, kc, Found, Case, Case0, Deriv, TotalCases, CaseTotal;
    char StabFileName[2000], VorviewFltFileName[2000];
    
    // Open the stability and control output file
    
    sprintf(StabFileName,"%s.stab",FileName);

    if ( (StabFile = fopen(StabFileName,"w")) == NULL ) {

       printf("Could not open the stability and control file for output! \n");

       exit(1);

    }
    
    // Open the vorview flt
    
    sprintf(VorviewFltFileName,"%s.flt",FileName);

    if ( (VorviewFlt = fopen(VorviewFltFileName,"w")) == NULL ) {

       printf("Could not open the vorview flt stability and control file for output! \n");

       exit(1);

    }
     
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
      
             Delta_AoA_     = 1.0;
             Delta_Beta_    = 1.0;
             Delta_Mach_    = 0.1; if ( ABS(Mach_ + Delta_Mach_ - 1.) <= 0.01 ) Delta_Mach_ /= 2.;
             Delta_P_       = 1.0;
             Delta_Q_       = 1.0;
             Delta_R_       = 1.0;
             Delta_Control_ = 1.0;
         
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

                if ( Case == 1 ) sprintf(VSP_VLM().CaseString(),"Base Aero         ");
                if ( Case == 2 ) sprintf(VSP_VLM().CaseString(),"Alpha      +%5.3lf",Delta_AoA_);
                if ( Case == 3 ) sprintf(VSP_VLM().CaseString(),"Beta       +%5.3lf",Delta_Beta_);
                if ( Case == 4 ) sprintf(VSP_VLM().CaseString(),"Roll Rate  +%5.3lf",Delta_P_);
                if ( Case == 5 ) sprintf(VSP_VLM().CaseString(),"Pitch Rate +%5.3lf",Delta_Q_);
                if ( Case == 6 ) sprintf(VSP_VLM().CaseString(),"Yaw Rate   +%5.3lf",Delta_R_);
                if ( Case == 7 ) sprintf(VSP_VLM().CaseString(),"Mach       +%5.3lf",Delta_Mach_);         
                
                // Solve this case
                
                VSP_VLM().SaveRestartFile() = VSP_VLM().DoRestart() = 0;
         
                if ( CaseTotal < TotalCases ) {
                   
                   VSP_VLM().Solve(CaseTotal);
                   
                }
                
                else {
                   
                   VSP_VLM().Solve(-CaseTotal);
                   
                }         
                   
                // Store aero coefficients
           
                CLForCase[Case] = VSP_VLM().CL(); 
                CDForCase[Case] = VSP_VLM().CD();        
                CSForCase[Case] = VSP_VLM().CS();        
                
                CDoForCase[Case] = VSP_VLM().CDo();     
         
                CFxForCase[Case] = VSP_VLM().CFx();
                CFyForCase[Case] = VSP_VLM().CFy();       
                CFzForCase[Case] = VSP_VLM().CFz();       
                    
                CMxForCase[Case] = VSP_VLM().CMx();       
                CMyForCase[Case] = VSP_VLM().CMy();       
                CMzForCase[Case] = VSP_VLM().CMz();     
                
                CMlForCase[Case] = -VSP_VLM().CMx();       
                CMmForCase[Case] =  VSP_VLM().CMy();       
                CMnForCase[Case] = -VSP_VLM().CMz();                     
         
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
                  
                   Found = 0;
         
                   while ( k <= VSP_VLM().VSPGeom().NumberOfSurfaces() && !Found ) {
                     
                      for ( p = 1 ; p <= VSP_VLM().VSPGeom().VSP_Surface(k).NumberOfControlSurfaces() ; p++ ) {
              
                         if ( strcmp(ControlSurfaceGroup_[i].ControlSurface_Name(j), VSP_VLM().VSPGeom().VSP_Surface(k).ControlSurface(p).Name()) == 0 ) {
                  
                            Found = 1;
                           
                            VSP_VLM().VSPGeom().VSP_Surface(k).ControlSurface(p).DeflectionAngle() = ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(j) * (ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle() + Delta_Control_) * TORAD;
           
                         }
                        
                      }
                     
                      k++;
                     
                   }
                  
                   if ( !Found ) {
                      
                      printf("Could not find control surface: %s in control surface group: %s \n",
                              ControlSurfaceGroup_[i].ControlSurface_Name(j),
                              ControlSurfaceGroup_[i].Name()); fflush(NULL);
                              
                      exit(1);
                      
                   }
                  
                }
                
                // Set a comment line

                sprintf(VSP_VLM().CaseString(),"Deflecting Control Group: %-d",i);
               
                // Now solve
               
                if ( CaseTotal < TotalCases ) {
                   
                   VSP_VLM().Solve(CaseTotal);
                   
                }
                
                else {
                   
                   VSP_VLM().Solve(-CaseTotal);
                   
                }         
                   
                // Store aero coefficients
           
                CLForCase[Case] = VSP_VLM().CL(); 
                CDForCase[Case] = VSP_VLM().CD();        
                CSForCase[Case] = VSP_VLM().CS();        
                
                CDoForCase[Case] = VSP_VLM().CDo();
         
                CFxForCase[Case] = VSP_VLM().CFx();
                CFyForCase[Case] = VSP_VLM().CFy();       
                CFzForCase[Case] = VSP_VLM().CFz();       
                    
                CMxForCase[Case] = VSP_VLM().CMx();       
                CMyForCase[Case] = VSP_VLM().CMy();       
                CMzForCase[Case] = VSP_VLM().CMz();    
                
                CMlForCase[Case] = -VSP_VLM().CMx();       
                CMmForCase[Case] =  VSP_VLM().CMy();       
                CMnForCase[Case] = -VSP_VLM().CMz();                        
         
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
                                     
       if ( n == 1 ) sprintf(CaseType,caseTypeFormatStr,"Base_Aero", 0.0,        "n/a"); // note must print a "-" for the units column here or the parser in VSP has a hard time
       if ( n == 2 ) sprintf(CaseType,caseTypeFormatStr,"Alpha",     Delta_AoA_, "deg");
       if ( n == 3 ) sprintf(CaseType,caseTypeFormatStr,"Beta",      Delta_Beta_,"deg");
       if ( n == 4 ) sprintf(CaseType,caseTypeFormatStr,"Roll__Rate",Delta_P_,   "rad/Tunit");
       if ( n == 5 ) sprintf(CaseType,caseTypeFormatStr,"Pitch_Rate",Delta_Q_,   "rad/Tunit");
       if ( n == 6 ) sprintf(CaseType,caseTypeFormatStr,"Yaw___Rate",Delta_R_,   "rad/Tunit");
       if ( n == 7 ) sprintf(CaseType,caseTypeFormatStr,"Mach",      Delta_Mach_,"no_unit");

       if ( n  > 7 ) sprintf(CaseType,caseTypeFormatStr,ControlSurfaceGroup_[n - NumStabCases_].Name(), Delta_Control_, "deg" );
       
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
#                   UnsteadyStabilityAndControlSolve                           #
#                                                                              #
##############################################################################*/

void UnsteadyStabilityAndControlSolve(void)
{

    int i, j, k, p, pm1, Case, NumCases;
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
      
    // Open the stability and control output file
    
    sprintf(StabFileName,"%s.stab",FileName);

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
             
             VSP_VLM().AngleOfBetaZero()   = BetaList_[i] * TORAD;
             VSP_VLM().Mach()              = MachList_[j];  
             VSP_VLM().AngleOfAttackZero() =  AoAList_[k] * TORAD;
      
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
/*    
              CL_damp = (  VSP_VLM().CL_Unsteady(16) -  VSP_VLM().CL_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
              CD_damp = (  VSP_VLM().CD_Unsteady(16) -  VSP_VLM().CD_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
              CS_damp = (  VSP_VLM().CS_Unsteady(16) -  VSP_VLM().CS_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );   
             CFx_damp = ( VSP_VLM().CFx_Unsteady(16) - VSP_VLM().CFx_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
             CFy_damp = ( VSP_VLM().CFy_Unsteady(16) - VSP_VLM().CFy_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
             CFz_damp = ( VSP_VLM().CFz_Unsteady(16) - VSP_VLM().CFz_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
             CMx_damp = ( VSP_VLM().CMx_Unsteady(16) - VSP_VLM().CMx_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
             CMy_damp = ( VSP_VLM().CMy_Unsteady(16) - VSP_VLM().CMy_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );
             CMz_damp = ( VSP_VLM().CMz_Unsteady(16) - VSP_VLM().CMz_Unsteady(32) )/( 2. * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180. );        
*/
              CL_avg = 0.;
              CD_avg = 0.;
              CS_avg = 0.;
             CFx_avg = 0.;
             CFy_avg = 0.;
             CFz_avg = 0.;
             CMx_avg = 0.;
             CMy_avg = 0.;
             CMz_avg = 0.;

              // First and last point are assumed to be the same
              
              p = VSP_VLM().NumberOfTimeSteps();
              
              CL_avg =  VSP_VLM().CL_Unsteady(p);
              CD_avg =  VSP_VLM().CD_Unsteady(p);
              CS_avg =  VSP_VLM().CS_Unsteady(p);
             CFx_avg = VSP_VLM().CFx_Unsteady(p);
             CFy_avg = VSP_VLM().CFy_Unsteady(p);
             CFz_avg = VSP_VLM().CFz_Unsteady(p);
             CMx_avg = VSP_VLM().CMx_Unsteady(p);
             CMy_avg = VSP_VLM().CMy_Unsteady(p);
             CMz_avg = VSP_VLM().CMz_Unsteady(p);    
             
             // Sum up from T = DT, to N*DT
             
             for ( p = 1 ; p <= VSP_VLM().NumberOfTimeSteps() ; p++ ) {
         
                 CL_avg +=  VSP_VLM().CL_Unsteady(p);
                 CD_avg +=  VSP_VLM().CD_Unsteady(p);
                 CS_avg +=  VSP_VLM().CS_Unsteady(p);
                CFx_avg += VSP_VLM().CFx_Unsteady(p);
                CFy_avg += VSP_VLM().CFy_Unsteady(p);
                CFz_avg += VSP_VLM().CFz_Unsteady(p);
                CMx_avg += VSP_VLM().CMx_Unsteady(p);
                CMy_avg += VSP_VLM().CMy_Unsteady(p);
                CMz_avg += VSP_VLM().CMz_Unsteady(p);
                
             }
             
             // Calculate average
             
              CL_avg /= (VSP_VLM().NumberOfTimeSteps() + 1);
              CD_avg /= (VSP_VLM().NumberOfTimeSteps() + 1);
              CS_avg /= (VSP_VLM().NumberOfTimeSteps() + 1);
             CFx_avg /= (VSP_VLM().NumberOfTimeSteps() + 1);
             CFy_avg /= (VSP_VLM().NumberOfTimeSteps() + 1);
             CFz_avg /= (VSP_VLM().NumberOfTimeSteps() + 1);
             CMx_avg /= (VSP_VLM().NumberOfTimeSteps() + 1);
             CMy_avg /= (VSP_VLM().NumberOfTimeSteps() + 1);
             CMz_avg /= (VSP_VLM().NumberOfTimeSteps() + 1);
             
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
             
             for ( p = 1 ; p <= VSP_VLM().NumberOfTimeSteps() ; p++ ) {
         
                 pm1 = p - 1;
                 
                 if ( p == 1 ) pm1 = VSP_VLM().NumberOfTimeSteps();
                 
                 T = VSP_VLM().TimeStep() * 0.5 * (double) (2*p + 1);
                 
                 Theta = VSP_VLM().Unsteady_AngleRate() * T;

                 CL_damp += 2.*( 0.5*(  VSP_VLM().CL_Unsteady(pm1) +  VSP_VLM().CL_Unsteady(p) ) -  CL_avg ) * cos( Theta ) * VSP_VLM().TimeStep();
                 CD_damp += 2.*( 0.5*(  VSP_VLM().CD_Unsteady(pm1) +  VSP_VLM().CD_Unsteady(p) ) -  CD_avg ) * cos( Theta ) * VSP_VLM().TimeStep();
                 CS_damp += 2.*( 0.5*(  VSP_VLM().CS_Unsteady(pm1) +  VSP_VLM().CS_Unsteady(p) ) -  CS_avg ) * cos( Theta ) * VSP_VLM().TimeStep();
                CFx_damp += 2.*( 0.5*( VSP_VLM().CFx_Unsteady(pm1) + VSP_VLM().CFx_Unsteady(p) ) - CFx_avg ) * cos( Theta ) * VSP_VLM().TimeStep();
                CFy_damp += 2.*( 0.5*( VSP_VLM().CFy_Unsteady(pm1) + VSP_VLM().CFy_Unsteady(p) ) - CFy_avg ) * cos( Theta ) * VSP_VLM().TimeStep();
                CFz_damp += 2.*( 0.5*( VSP_VLM().CFz_Unsteady(pm1) + VSP_VLM().CFz_Unsteady(p) ) - CFz_avg ) * cos( Theta ) * VSP_VLM().TimeStep();
                CMx_damp += 2.*( 0.5*( VSP_VLM().CMx_Unsteady(pm1) + VSP_VLM().CMx_Unsteady(p) ) - CMx_avg ) * cos( Theta ) * VSP_VLM().TimeStep();
                CMy_damp += 2.*( 0.5*( VSP_VLM().CMy_Unsteady(pm1) + VSP_VLM().CMy_Unsteady(p) ) - CMy_avg ) * cos( Theta ) * VSP_VLM().TimeStep();
                CMz_damp += 2.*( 0.5*( VSP_VLM().CMz_Unsteady(pm1) + VSP_VLM().CMz_Unsteady(p) ) - CMz_avg ) * cos( Theta ) * VSP_VLM().TimeStep();

             }    
                  
              Fact = VSP_VLM().TimeStep() * VSP_VLM().NumberOfTimeSteps() * VSP_VLM().ReducedFrequency() * VSP_VLM().Unsteady_AngleMax() * PI / 180.;
              
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
#                               RotorAnalysisSolve                             #
#                                                                              #
##############################################################################*/

void RotorAnalysisSolve(void)
{

    int i, j, k, Case, NumCases;

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
             VSP_VLM().AngleOfAttack() = 0.;

             VSP_VLM().RotationalRate_p() = BladeRPM_*2.*PI/60.;
             VSP_VLM().RotationalRate_q() = 0.;
             VSP_VLM().RotationalRate_r() = 0.;
             
             sprintf(VSP_VLM().CaseString(),"RPM: %f",BladeRPM_);
             
             // Set Control surface group deflection to un-perturbed control surface deflections

             ApplyControlDeflections();

             // Solve
             
             if ( Case <= NumCases ) {
                
                VSP_VLM().Solve(Case);
                
             }
             
             else {
                
                VSP_VLM().Solve(-Case);
                
             }
 
          }
          
       }
       
    }
    
}    

/*##############################################################################
#                                                                              #
#                                   Noise                                      #
#                                                                              #
##############################################################################*/

void Noise(void)
{

    int NumCases;

    ApplyControlDeflections();
    
    NumCases = NumberOfBetas_ * NumberOfMachs_ * NumberOfAoAs_ * NumberOfReCrefs_;
    
    if ( NumCases != 1 ) {
       
       printf("Don't know how to set up a noise case with more than one Mach-Alpha-Cref case! \n");
       exit(1);
       
    }

    // Set free stream conditions
    
    VSP_VLM().AngleOfBeta()   = BetaList_[1] * TORAD;
    VSP_VLM().Mach()          = MachList_[1];  
    VSP_VLM().AngleOfAttack() =  AoAList_[1] * TORAD;
    
    VSP_VLM().RotationalRate_p() = 0.;
    VSP_VLM().RotationalRate_q() = 0.;
    VSP_VLM().RotationalRate_r() = 0.;
       
    VSP_VLM().WriteOutNoiseFiles(NumCases);

}

