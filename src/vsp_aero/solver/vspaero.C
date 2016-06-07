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
double ReCref_;
double ClMax_;
double MaxTurningAngle_;
double FarDist_;

double Set_AoA_;
double Set_Mach_;
double Set_Beta_;

#define MAXSTABCASES 10

// Stability case lists

double MachList_[MAXSTABCASES];
double AlphaList_[MAXSTABCASES];
double BetaList_[MAXSTABCASES];

double RotationalRate_pList_[MAXSTABCASES];
double RotationalRate_qList_[MAXSTABCASES];
double RotationalRate_rList_[MAXSTABCASES];

// Deltas for calculating derivaties

double Delta_AoA_;
double Delta_Beta_;
double Delta_Mach_;
double Delta_P_;
double Delta_Q_;
double Delta_R_;

// Raw stability data

double CFxForCase[MAXSTABCASES];
double CFyForCase[MAXSTABCASES];
double CFzForCase[MAXSTABCASES];

double CMxForCase[MAXSTABCASES];
double CMyForCase[MAXSTABCASES];
double CMzForCase[MAXSTABCASES];

double CLForCase[MAXSTABCASES];
double CDForCase[MAXSTABCASES];
double CSForCase[MAXSTABCASES];

double CMlForCase[MAXSTABCASES];
double CMmForCase[MAXSTABCASES];
double CMnForCase[MAXSTABCASES];

// Stability derivatives

double dCFx_wrt[MAXSTABCASES];
double dCFy_wrt[MAXSTABCASES];
double dCFz_wrt[MAXSTABCASES];

double dCMx_wrt[MAXSTABCASES];
double dCMy_wrt[MAXSTABCASES];
double dCMz_wrt[MAXSTABCASES];

double dCL_wrt[MAXSTABCASES];
double dCD_wrt[MAXSTABCASES];
double dCS_wrt[MAXSTABCASES];

double dCMl_wrt[MAXSTABCASES];
double dCMm_wrt[MAXSTABCASES];
double dCMn_wrt[MAXSTABCASES];

int NumStabCases_ = 7;

int WakeIterations_;
int NumberOfRotors_;
int NumberOfThreads_ = 1;
int StabControlRun_ = 0;
int SetFreeStream_ = 0;
int SaveRestartFile_ = 0;
int DoRestartRun_ = 0;
int DoSymmetry_ = 0;
int SetFarDist_ = 0;
int Symmetry_ = 0;
int NumberOfWakeNodes_ = 0;
int DumpGeom_ = 0;
int ForceAveragingIter_ = 0;
int NoWakeIteration_= 0;
int NumberofSurveyPoints_ = 0;

// Prototypes

int main(int argc, char **argv);
void ParseInput(int argc, char *argv[]);
void CreateInputFile(char *argv[], int argc, int &i);
void LoadCaseFile(void);
void StabilityAndControlSolve(void);
void CalculateStabilityDerivatives(void);

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

    // Grab the file name
    
    FileName = argv[argc-1];

    // Parse the input

    ParseInput(argc,argv);

    // Output a header

    printf("VSPAERO v.2.1 --- 3/10/2015 \n");
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
    
    // Load in the VSP degenerate geometry file
    
    VSP_VLM().ReadFile(FileName);
    
    // Command line arguments over-ride any inputs in the case file
    
    if ( SetFreeStream_ ) {
     
       Mach_ = Set_Mach_;
       AoA_  = Set_AoA_;
       Beta_ = Set_Beta_;
       
       VSP_VLM().Mach() = Mach_;
       VSP_VLM().AngleOfAttack() = AoA_ * TORAD;
       VSP_VLM().AngleOfBeta() = Beta_ * TORAD;       
       
    }
   
    // Solve
    
    VSP_VLM().Setup();
    
    // Symmetry options
    
    if ( DoSymmetry_ == SYM_X ) VSP_VLM().DoSymmetryPlaneSolve(SYM_X);
    if ( DoSymmetry_ == SYM_Y ) VSP_VLM().DoSymmetryPlaneSolve(SYM_Y);
    if ( DoSymmetry_ == SYM_Z ) VSP_VLM().DoSymmetryPlaneSolve(SYM_Z);
    
    // Force farfield distance for wake adaption
    
    if ( SetFarDist_ ) VSP_VLM().SetFarFieldDist(FarDist_);
    
    // Set number of farfield wake nodes
    
    if ( NumberOfWakeNodes_ > 0 ) VSP_VLM().SetNumberOfWakeTrailingNodes(NumberOfWakeNodes_);
     
    // Geometry dump, no solver
    
    if ( DumpGeom_ ) VSP_VLM().DumpGeom() = 1;
    
    // Force no wakes for some number of iterations
    
    if ( NoWakeIteration_ > 0 ) VSP_VLM().NoWakeIteration() = NoWakeIteration_;
    
    // Use force averaging
    
    if ( ForceAveragingIter_ > 0 ) {
       
       VSP_VLM().ForceType() = FORCE_AVERAGE;
       
       VSP_VLM().AveragingIteration() = ForceAveragingIter_;
       
    }
        
    if ( !StabControlRun_ ) {
 
       VSP_VLM().RotationalRate_p() = 0.;
       VSP_VLM().RotationalRate_q() = 0.;
       VSP_VLM().RotationalRate_r() = 0.;
       
       if ( SaveRestartFile_ ) VSP_VLM().SaveRestartFile() = 1;
   
       if ( DoRestartRun_    ) VSP_VLM().DoRestart() = 1;
               
       VSP_VLM().Solve();

       VSP_VLM().WriteOutAerothermalDatabaseFiles();
       
    }
    
    else {
     
       StabilityAndControlSolve();
       
    }

    printf("Done!\n");
}

/*##############################################################################
#                                                                              #
#                              ParseInput                                      #
#                                                                              #
##############################################################################*/

void ParseInput(int argc, char *argv[])
{

    int i;

    // Parse the input

    if ( argc < 2 ) {

       printf("VSPAERO v.2.1 --- 2/11/2015 \n");
       printf("\n\n\n\n");

       printf("Usage: vspaero -(see below) FileName\n");
       printf("\n\n");
       printf("Additional options: \n");
       printf(" -omp N          Use 'N' processes.\n");
       printf(" -stab           Calculate stability derivatives.\n");
       printf(" -fs M A B       Set freestream Mach, Alpha, and Beta.\n");
       printf(" -save           Save restart file.\n");
       printf(" -restart        Restart analysis.\n");
       printf(" -geom           Process and write geometry without solving.\n");
       printf(" -avg N          Force averaging startign at wake iteration N\n");
       printf(" -nowake N       No wake for first N iterations.\n");
       printf(" -setup          Write template *.vspaero file, can specify parameters below:\n");
       printf("    -sref  S     Reference area.\n");
       printf("    -bref  b     Reference span.\n");
       printf("    -cref  c     Reference chord.\n");
       printf("    -cg  X Y Z   Moment reference point.\n");
       printf("    -mach  M     Freestream Mach number.\n");
       printf("    -aoa   A     Angle of attack.\n");
       printf("    -beta  B     Sideslip angle.\n");

       exit(1);

    }

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
       
       else if ( strcmp(argv[i],"-fs") == 0 ) {
        
          SetFreeStream_ = 1;
          
          Set_Mach_ = atof(argv[++i]);
          Set_AoA_  = atof(argv[++i]);
          Set_Beta_ = atof(argv[++i]);
          
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

       else if ( strcmp(argv[i],"-avg") == 0 ) {

          ForceAveragingIter_ = atoi(argv[++i]);
          
       }     
       
       else if ( strcmp(argv[i],"-setup") == 0 ) {

          printf("Creating vspaero input file... \n");fflush(NULL);
          
          CreateInputFile(argv,argc,i);
          
          exit(1);
          
       }                
       
       else if ( strcmp(argv[i],"-nowake") == 0 ) {

          NoWakeIteration_ = atoi(argv[++i]);
          
       }           
       
       // Unreconizable option

       else {

          printf("argv[i]: %s \n",argv[i]);
          
          printf("VSPAERO v.2.1 --- 2/11/2015 \n");
          printf("\n\n\n\n");

          printf("Usage: vspaero -(see below) FileName\n");
          printf("\n\n");
          printf("Additional options: \n");
          printf(" -omp N          Use 'N' processes.\n");
          printf(" -stab           Calculate stability derivatives.\n");
          printf(" -fs M A B       Set freestream Mach, Alpha, and Beta.\n");
          printf(" -save           Save restart file.\n");
          printf(" -restart        Restart analysis.\n");
          printf(" -geom           Process and write geometry without solving.\n");
          printf(" -avg N          Force averaging startign at wake iteration N\n");
          printf(" -nowake N       No wake for first N iterations.\n");
          printf(" -setup          Write template *.vspaero file, can specify parameters below:\n");
          printf("    -sref  S     Reference area.\n");
          printf("    -bref  b     Reference span.\n");
          printf("    -cref  c     Reference chord.\n");
          printf("    -cg  X Y Z   Moment reference point.\n");
          printf("    -mach  M     Freestream Mach number.\n");
          printf("    -aoa   A     Angle of attack.\n");
          printf("    -beta  B     Sideslip angle.\n");

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

    FILE *case_file;
    char file_name_w_ext[2000];

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
    Rho_               = 0.002377;
    ReCref_            = 10000000.;
    ClMax_             = -1.;
    MaxTurningAngle_   = -1.;
    FarDist_           = -1.;
    NumberOfWakeNodes_ = -1;
    WakeIterations_    = 5;
    NumberOfRotors_    = 0.;

    // Read in the degen geometry file

    VSP_VLM().ReadFile(FileName);

    // Open the case file

    sprintf(file_name_w_ext,"%s.vspaero",FileName);

    if ( (case_file = fopen(file_name_w_ext,"w")) == NULL ) {

       printf("Could not open the file: %s for input! \n",file_name_w_ext);

       exit(1);

    }
    
    // Check for any user supplied values on the command line
    
    while ( i <= argc - 2 ) {
    
       if ( strcmp(argv[i],"-sref") == 0 ) {
          
          Sref_ = atof(argv[++i]);
          
       }
       
       else if ( strcmp(argv[i],"-sref") == 0 ) {
          
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
          
          Mach_ = atof(argv[++i]);
          
       }     
       
       else if ( strcmp(argv[i],"-aoa") == 0 ) {
          
          AoA_ = atof(argv[++i]);
          
       }     
       
       else if ( strcmp(argv[i],"-beta") == 0 ) {
          
          Beta_ = atof(argv[++i]);
          
       }    
       
       i++; 
       
    }
                                
    fprintf(case_file,"Sref = %lf \n",Sref_);
    fprintf(case_file,"Cref = %lf \n",Cref_);
    fprintf(case_file,"Bref = %lf \n",Bref_);
    fprintf(case_file,"X_cg = %lf \n",Xcg_);
    fprintf(case_file,"Y_cg = %lf \n",Ycg_);
    fprintf(case_file,"Z_cg = %lf \n",Zcg_);
    fprintf(case_file,"Mach = %lf \n",Mach_);
    fprintf(case_file,"AoA = %lf \n",AoA_);
    fprintf(case_file,"Beta = %lf \n",Beta_);
    fprintf(case_file,"Vinf = %lf \n",Vinf_);
    fprintf(case_file,"Rho = %lf \n",Rho_);
    fprintf(case_file,"ReCref = %lf \n",ReCref_);
    fprintf(case_file,"ClMax = %lf \n",ClMax_);
    fprintf(case_file,"MaxTurningAngle = %lf \n",MaxTurningAngle_);
    fprintf(case_file,"Symmetry = No \n");
    fprintf(case_file,"FarDist = %lf \n",FarDist_);
    fprintf(case_file,"NumWakeNodes = %d \n",NumberOfWakeNodes_);
    fprintf(case_file,"WakeIters = %d \n",WakeIterations_);

    printf("VSP_VLM().VSPGeom().NumberOfRotors(): %d \n",VSP_VLM().VSPGeom().NumberOfRotors());

    if ( VSP_VLM().VSPGeom().NumberOfRotors() != 0 ) NumberOfRotors_ = VSP_VLM().VSPGeom().NumberOfRotors();

    fprintf(case_file,"NumberOfRotors = %d \n",NumberOfRotors_);

    for ( i = 1 ;  i <= NumberOfRotors_ ; i++ ) {

       fprintf(case_file,"PropElement_%-d\n",i);
       fprintf(case_file,"%d\n",i);

       VSP_VLM().VSPGeom().RotorDisk(i).Write_STP_Data(case_file);

    }

    fclose(case_file);
 
}

/*##############################################################################
#                                                                              #
#                                      LoadCaseFile                            #
#                                                                              #
##############################################################################*/

void LoadCaseFile(void)
{

    int i, j;
    float x,y,z;
    FILE *case_file;
    char file_name_w_ext[2000], DumChar[200];
    char SymmetryFlag[80];

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
    fscanf(case_file,"Mach = %lf \n",&Mach_);
    fscanf(case_file,"AoA = %lf \n",&AoA_);
    fscanf(case_file,"Beta = %lf \n",&Beta_);
    fscanf(case_file,"Vinf = %lf \n",&Vinf_);
    fscanf(case_file,"Rho = %lf \n",&Rho_);
    fscanf(case_file,"ReCref = %lf \n",&ReCref_);
    fscanf(case_file,"ClMax = %lf \n",&ClMax_);
    fscanf(case_file,"MaxTurningAngle = %lf \n",&MaxTurningAngle_);
    fscanf(case_file,"Symmetry = %s \n",SymmetryFlag);
    fscanf(case_file,"FarDist = %lf \n",&FarDist_);
    fscanf(case_file,"NumWakeNodes = %d \n",&NumberOfWakeNodes_);
    fscanf(case_file,"WakeIters = %d \n",&WakeIterations_);
    
    if ( FarDist_ > 0. ) SetFarDist_ = 1;
    
    if ( WakeIterations_ <= 0 ) WakeIterations_ = 1;

    if ( ClMax_ <= 0. ) ClMax_ = -1.;

    if ( MaxTurningAngle_ <= 0. ) MaxTurningAngle_ = -1.;

    printf("Sref            = %lf \n",Sref_);
    printf("Cref            = %lf \n",Cref_);
    printf("Bref            = %lf \n",Bref_);
    printf("X_cg            = %lf \n",Xcg_);
    printf("Y_cg            = %lf \n",Ycg_);
    printf("Z_cg            = %lf \n",Zcg_);
    printf("Mach            = %lf \n",Mach_);
    printf("AoA             = %lf \n",AoA_);
    printf("Beta            = %lf \n",Beta_);
    printf("Vinf            = %lf \n",Vinf_);
    printf("Rho             = %lf \n",Rho_);
    printf("ReCref          = %lf \n",ReCref_);
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
    
    VSP_VLM().Density() = Rho_;
    
    VSP_VLM().ReCref() = ReCref_;
    
    VSP_VLM().ClMax() = ClMax_;
    
    VSP_VLM().MaxTurningAngle() = MaxTurningAngle_;

    VSP_VLM().WakeIterations() = WakeIterations_;
    
    VSP_VLM().RotationalRate_p() = 0.0;
    VSP_VLM().RotationalRate_q() = 0.0;
    VSP_VLM().RotationalRate_r() = 0.0;    
    
    if ( strcmp(SymmetryFlag,"X") == 0 ) DoSymmetry_ = SYM_X;
    if ( strcmp(SymmetryFlag,"Y") == 0 ) DoSymmetry_ = SYM_Y;
    if ( strcmp(SymmetryFlag,"Z") == 0 ) DoSymmetry_ = SYM_Z;
        
    // Load in the rotor data
    
    NumberOfRotors_ = 0;
    
    fscanf(case_file,"NumberOfRotors = %d \n",&NumberOfRotors_);
    
    printf("NumberOfRotors: %d \n",NumberOfRotors_);
    
    VSP_VLM().SetNumberOfRotors(NumberOfRotors_);
    
    for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
     
       fgets(DumChar,200,case_file);
       fgets(DumChar,200,case_file);
       
       printf("\nLoading data for rotor: %5d \n",i);
       
       VSP_VLM().RotorDisk(i).Load_STP_Data(case_file);
       
    }
    
    // Load in the velocity survey data

    fscanf(case_file,"NumberofSurveyPoints = %d \n",&NumberofSurveyPoints_);

    printf("NumberofSurveyPoints: %d \n",NumberofSurveyPoints_);

    VSP_VLM().SetNumberOfSurveyPoints(NumberofSurveyPoints_);

    for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {

       fscanf(case_file,"%d %f %f %f \n",&j,&x,&y,&z);

       printf("Survey Point: %10d: %10.5f %10.5f %10.5f \n",i,x,y,z);

       VSP_VLM().SurveyPointList(i).x() = x;
       VSP_VLM().SurveyPointList(i).y() = y;
       VSP_VLM().SurveyPointList(i).z() = z;

    }

    fclose(case_file);

}

/*##############################################################################
#                                                                              #
#                           StabilityAndControlSolve                           #
#                                                                              #
##############################################################################*/

void StabilityAndControlSolve(void)
{

    int Case, Case0, Deriv;

    Delta_AoA_  = 0.100;		//units: deg
    Delta_Beta_ = 0.100;		//units: deg
    Delta_Mach_ = 0.100;		//units: deg
    Delta_P_    = 0.001;	    //units: rad/sec
    Delta_Q_    = 0.001;	    //units: rad/sec
    Delta_R_    = 0.001;	    //units: rad/sec

    Case = 1;

    Case0 = Case;

    for ( Deriv = 1 ; Deriv <= 7 ; Deriv++ ) {

        MachList_[Case] = Mach_;
       AlphaList_[Case] = AoA_;
        BetaList_[Case] = Beta_;

       RotationalRate_pList_[Case] = 0.;
       RotationalRate_qList_[Case] = 0.;
       RotationalRate_rList_[Case] = 0.;

       Case++;

    }

    // Perturb in alpha
                 
    AlphaList_[Case0 + 1] = AoA_ + Delta_AoA_;
     
    // Perturb in Beta
        
    BetaList_[Case0 + 2] = Beta_ + Delta_Beta_;
    
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

    MachList_[Case0 + 6] = Mach_ + Delta_Mach_;
 
    printf("Doing stability and control run... \n"); 

    for ( Case = 1 ; Case <= NumStabCases_ ; Case++ ) {

       printf("Calculating stability and control case: %d of %d \n",Case,NumStabCases_);
       
       // Set free stream conditions
       
       VSP_VLM().Mach()          = MachList_[Case];
       VSP_VLM().AngleOfAttack() = AlphaList_[Case] * TORAD;
       VSP_VLM().AngleOfBeta()   = BetaList_[Case] * TORAD;

       VSP_VLM().RotationalRate_p() = RotationalRate_pList_[Case];
       VSP_VLM().RotationalRate_q() = RotationalRate_qList_[Case];
       VSP_VLM().RotationalRate_r() = RotationalRate_rList_[Case];

       // Solve this case
       
       VSP_VLM().SaveRestartFile() = VSP_VLM().DoRestart() = 0;
      
       if ( Case == 1 ) VSP_VLM().SaveRestartFile() = 1;

       if ( Case > 1 ) VSP_VLM().DoRestart() = 1;
       
       if ( Case < NumStabCases_ ) {
          
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

       printf("\n");

    }

    // Now calculate actual stability derivatives 
    
    CalculateStabilityDerivatives();

}

/*##############################################################################
#                                                                              #
#                           CalculateStabilityDerivatives                      #
#                                                                              #
##############################################################################*/

void CalculateStabilityDerivatives(void)
{

    int n, Case, Case0;
    double Delta;
    char StabFileName[2000],CaseType[2000];
    FILE *StabFile;

    sprintf(StabFileName,"%s.stab",FileName);

    if ( (StabFile = fopen(StabFileName,"w")) == NULL ) {

       printf("Could not open the stability and control file for output! \n");

       exit(1);

    }
    
    // Write out raw data to file
    
    fprintf(StabFile,"Sref_: %12.4lf \n",Sref_);
    fprintf(StabFile,"Cref_: %12.4lf \n",Cref_);
    fprintf(StabFile,"Bref_: %12.4lf \n",Bref_);
    fprintf(StabFile,"Xcg_:  %12.4lf \n",Xcg_);
    fprintf(StabFile,"Ycg_:  %12.4lf \n",Ycg_);
    fprintf(StabFile,"Zcg_:  %12.4lf \n",Zcg_);
    fprintf(StabFile,"AoA_:  %12.4lf \n",AoA_);
    fprintf(StabFile,"Beta_: %12.4lf \n",Beta_);
    fprintf(StabFile,"Mach_: %12.4lf \n",Mach_);
    fprintf(StabFile,"Rho_:  %12.4lf \n",Rho_);
    fprintf(StabFile,"Vinf_: %12.4lf \n",Vinf_);
    fprintf(StabFile,"#\n");

    //                1234567890123 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012    
    fprintf(StabFile,"     Case           CFx          CFy          CFz          CMx          CMy          CMz          CL           CD           CS     \n");
       
    fprintf(StabFile,"#\n");

    for ( Case = 1 ; Case <= NumStabCases_ ; Case++ ) {
                                        //1234567890123
       if ( Case == 1 ) sprintf(CaseType,"Base Aero    ");
       if ( Case == 2 ) sprintf(CaseType,"Alpha + %5.3lf",Delta_AoA_);
       if ( Case == 3 ) sprintf(CaseType,"Beta  + %5.3lf",Delta_Beta_);
       if ( Case == 4 ) sprintf(CaseType,"Roll  + %5.3lf",Delta_P_);
       if ( Case == 5 ) sprintf(CaseType,"Pitch + %5.3lf",Delta_Q_);
       if ( Case == 6 ) sprintf(CaseType,"Yaw   + %5.3lf",Delta_R_);
       if ( Case == 7 ) sprintf(CaseType,"Mach  + %5.3lf",Delta_Mach_);

       fprintf(StabFile,"%13s %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f %12.7f \n",
               CaseType,
               
               CFxForCase[Case],
               CFyForCase[Case],    
               CFzForCase[Case], 
                      
               CMxForCase[Case],  
               CMyForCase[Case],  
               CMzForCase[Case],    
                      
               CLForCase[Case],
               CDForCase[Case],   
               CSForCase[Case]);  
 
       printf("\n");

    }
    
    // Now calculate stability derivatives and write them out
    
    Case = 0;

    while ( Case < NumStabCases_ ) {
     
       // Skip first case in a set... it's the baseline
              
       Case++;

       Case0 = Case;

       for ( n = 2 ; n <= 7 ; n++ ) {
       
          if ( n == 2  ) Delta =  Delta_AoA_  * TORAD;            // wrt Alpha
          if ( n == 3  ) Delta =  Delta_Beta_ * TORAD;            // wrt Beta
          if ( n == 4  ) Delta =  Delta_P_ * Bref_ * 0.5 / Vinf_; // wrt roll rate
          if ( n == 5  ) Delta =  Delta_Q_ * Cref_ * 0.5 / Vinf_; // wrt pitch rate
          if ( n == 6  ) Delta =  Delta_R_ * Bref_ * 0.5 / Vinf_; // wrt yaw rate
          if ( n == 7  ) Delta =  Delta_Mach_;                    // wrt Mach number
           
          Case++;
           
          dCFx_wrt[n] = ( CFxForCase[Case] - CFxForCase[Case0] )/Delta;
          dCFy_wrt[n] = ( CFyForCase[Case] - CFyForCase[Case0] )/Delta;
          dCFz_wrt[n] = ( CFzForCase[Case] - CFzForCase[Case0] )/Delta;
          
          dCMx_wrt[n] = ( CMxForCase[Case] - CMxForCase[Case0] )/Delta;
          dCMy_wrt[n] = ( CMyForCase[Case] - CMyForCase[Case0] )/Delta;
          dCMz_wrt[n] = ( CMzForCase[Case] - CMzForCase[Case0] )/Delta;
           
          dCL_wrt[n]  = (  CLForCase[Case] -  CLForCase[Case0] )/Delta;    
          dCD_wrt[n]  = (  CDForCase[Case] -  CDForCase[Case0] )/Delta;    
          dCS_wrt[n]  = (  CSForCase[Case] -  CSForCase[Case0] )/Delta;    

       }

       dCFx_wrt[8] = MachList_[Case0] * dCFx_wrt[7];
       dCFy_wrt[8] = MachList_[Case0] * dCFy_wrt[7];
       dCFz_wrt[8] = MachList_[Case0] * dCFz_wrt[7];

       dCMx_wrt[8] = MachList_[Case0] * dCMx_wrt[7];
       dCMy_wrt[8] = MachList_[Case0] * dCMy_wrt[7];
       dCMz_wrt[8] = MachList_[Case0] * dCMz_wrt[7];

       dCL_wrt[8]  = MachList_[Case0] * dCL_wrt[7];
       dCD_wrt[8]  = MachList_[Case0] * dCD_wrt[7];
       dCS_wrt[8]  = MachList_[Case0] * dCS_wrt[7];
      
	   /*
	   //DUPLICATE to information at the top of this file
       fprintf(StabFile,"Mach:    %12.8f \n",MachList_[Case0]);
       fprintf(StabFile,"Alpha:   %12.8f \n",AlphaList_[Case0]);
       fprintf(StabFile,"Density: %12.8f \n",Rho_);
       fprintf(StabFile,"Uo:      %12.8f \n",Vinf_);
       */

       fprintf(StabFile,"#\n");
       
       //                        123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012 123456789012           
       fprintf(StabFile,"#             Base    Derivative:                                                                              \n");
       fprintf(StabFile,"#             Aero         wrt          wrt          wrt          wrt          wrt          wrt          wrt  \n");
       fprintf(StabFile,"#             Coef         Alpha        Beta          p            q            r           Mach         U    \n");
       fprintf(StabFile,"#                          per          per          per          per          per          per          per  \n");
       fprintf(StabFile,"#                          rad          rad          rad          rad          rad          rad          rad  \n");
      
       fprintf(StabFile,"#\n");
       
       fprintf(StabFile,"CFx    "); fprintf(StabFile,"%12.7f ",CFxForCase[Case0]); for ( n = 2 ; n <= 8 ; n++ ) { fprintf(StabFile,"%12.7f ",dCFx_wrt[n]); }; fprintf(StabFile,"\n");
       fprintf(StabFile,"CFy    "); fprintf(StabFile,"%12.7f ",CFyForCase[Case0]); for ( n = 2 ; n <= 8 ; n++ ) { fprintf(StabFile,"%12.7f ",dCFy_wrt[n]); }; fprintf(StabFile,"\n");
       fprintf(StabFile,"CFz    "); fprintf(StabFile,"%12.7f ",CFzForCase[Case0]); for ( n = 2 ; n <= 8 ; n++ ) { fprintf(StabFile,"%12.7f ",dCFz_wrt[n]); }; fprintf(StabFile,"\n");

       fprintf(StabFile,"CMx    "); fprintf(StabFile,"%12.7f ",CMxForCase[Case0]); for ( n = 2 ; n <= 8 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMx_wrt[n]); }; fprintf(StabFile,"\n");
       fprintf(StabFile,"CMy    "); fprintf(StabFile,"%12.7f ",CMyForCase[Case0]); for ( n = 2 ; n <= 8 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMy_wrt[n]); }; fprintf(StabFile,"\n");
       fprintf(StabFile,"CMz    "); fprintf(StabFile,"%12.7f ",CMzForCase[Case0]); for ( n = 2 ; n <= 8 ; n++ ) { fprintf(StabFile,"%12.7f ",dCMz_wrt[n]); }; fprintf(StabFile,"\n");

       fprintf(StabFile,"CL     "); fprintf(StabFile,"%12.7f ",CLForCase[Case0]);  for ( n = 2 ; n <= 8 ; n++ ) { fprintf(StabFile,"%12.7f ",dCL_wrt[n]); };  fprintf(StabFile,"\n");
       fprintf(StabFile,"CD     "); fprintf(StabFile,"%12.7f ",CDForCase[Case0]);  for ( n = 2 ; n <= 8 ; n++ ) { fprintf(StabFile,"%12.7f ",dCD_wrt[n]); };  fprintf(StabFile,"\n");
       fprintf(StabFile,"CS     "); fprintf(StabFile,"%12.7f ",CSForCase[Case0]);  for ( n = 2 ; n <= 8 ; n++ ) { fprintf(StabFile,"%12.7f ",dCS_wrt[n]); };  fprintf(StabFile,"\n");
                                                                                                                                                                                         
       fprintf(StabFile,"#\n");
              
    }
    
    fclose(StabFile);

}
    

