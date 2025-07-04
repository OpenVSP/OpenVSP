//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "WOPWOP.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                            WOPWOP constructor                                #
#                                                                              #
##############################################################################*/

WOPWOP::WOPWOP(void)
{

    // Rotor
    
    NumberOfBlades_ = 0;
    
    SurfaceForBlade_ = NULL;
    
    NumberOfBladesSections_ = 0;
    
    RotorID_ = 0;
    
    // Wing
    
    NumberOfWingSurfaces_ = 0;

    SurfaceForWing_ = NULL;
    
    WingID_ = 0;
    
    NumberOfWingSections_ = 0;
    
    // Body
    
    NumberOfBodySurfaces_ = 0;
    
    SurfaceForBody_ = NULL;
    
    BodyID_ = 0;
    
    NumberOfBodySections_ = 0;
    
    RotorLoadingGeometryFile_   = NULL;
    RotorLoadingFile_           = NULL;
    RotorThicknessGeometryFile_ = NULL;
    BPMFile_               = NULL;

    WingLoadingGeometryFile_   = NULL;
    WingLoadingFile_           = NULL;
    WingThicknessGeometryFile_ = NULL;

    BodyThicknessGeometryFile_ = NULL;
  
}

/*##############################################################################
#                                                                              #
#                                WOPWOP Copy                                   #
#                                                                              #
##############################################################################*/

WOPWOP::WOPWOP(const WOPWOP &WopWopRotor)
{

    // Just use operator = code
    
    *this = WopWopRotor;

}

/*##############################################################################
#                                                                              #
#                       WOPWOP SizeBladeList                                   #
#                                                                              #
##############################################################################*/

void WOPWOP::SizeBladeList(int NumberOfBlades)
{

    int i;
    
    NumberOfBlades_ = NumberOfBlades;
    
    SurfaceForBlade_ = new int[NumberOfBlades_ + 1];
        
    RotorLoadingGeometryFile_   = new FILE*[NumberOfBlades_ + 1];
    RotorLoadingFile_           = new FILE*[NumberOfBlades_ + 1];
    RotorThicknessGeometryFile_ = new FILE*[NumberOfBlades_ + 1];    
    
    for ( i = 1 ; i <= NumberOfBlades_ ; i++ ) {
       
       SurfaceForBlade_[i] = 0;
       
       RotorLoadingGeometryFile_[i]   = NULL;
       RotorLoadingFile_[i]           = NULL;
       RotorThicknessGeometryFile_[i] = NULL;
       
    }
    
    BPMFile_ = NULL;
    
}

/*##############################################################################
#                                                                              #
#                       WOPWOP SizeWingSurfaceList                             #
#                                                                              #
##############################################################################*/

void WOPWOP::SizeWingSurfaceList(int NumberOfSurfaces)
{

    int i;
    
    NumberOfWingSurfaces_ = NumberOfSurfaces;
    
    SurfaceForWing_ = new int[NumberOfWingSurfaces_ + 1];
        
    WingLoadingGeometryFile_   = new FILE*[NumberOfWingSurfaces_ + 1];
    WingLoadingFile_           = new FILE*[NumberOfWingSurfaces_ + 1];
    WingThicknessGeometryFile_ = new FILE*[NumberOfWingSurfaces_ + 1];    
    
    for ( i = 1 ; i <= NumberOfWingSurfaces_ ; i++ ) {
       
       SurfaceForWing_[i] = 0;
       
       WingLoadingGeometryFile_[i]   = NULL;
       WingLoadingFile_[i]           = NULL;
       WingThicknessGeometryFile_[i] = NULL;
       
    }
    
    BPMFile_ = NULL;
    
}

/*##############################################################################
#                                                                              #
#                       WOPWOP SizeBodySurfaceList                             #
#                                                                              #
##############################################################################*/

void WOPWOP::SizeBodySurfaceList(int NumberOfSurfaces)
{

    int i;
    
    NumberOfBodySurfaces_ = NumberOfSurfaces;
    
    SurfaceForBody_ = new int[NumberOfBodySurfaces_ + 1];
        
    BodyThicknessGeometryFile_ = new FILE*[NumberOfBodySurfaces_ + 1];    
    
    for ( i = 1 ; i <= NumberOfBodySurfaces_ ; i++ ) {
       
       SurfaceForBody_[i] = 0;
       
       BodyThicknessGeometryFile_[i] = NULL;
       
    }
    
    BPMFile_ = NULL;
    
}


/*##############################################################################
#                                                                              #
#                      WOPWOP OpenLoadingGeometryFile                          #
#                                                                              #
##############################################################################*/

FILE *WOPWOP::OpenLoadingGeometryFile(int i, char *FileName, FILE **File)
{

    // Open file
    
    if ( (File[i] = fopen(FileName, "wb")) == NULL ) {

       printf ("Could not open the PSUWopWop file: %s for output! \n",FileName);

       exit(1);

    }    
          
    return File[i];
          
}

/*##############################################################################
#                                                                              #
#                          WOPWOP OpenLoadingFile                              #
#                                                                              #
##############################################################################*/

FILE *WOPWOP::OpenLoadingFile(int i, char *FileName, FILE **File)
{

    // Open file
    
    if ( (File[i] = fopen(FileName, "wb")) == NULL ) {

       printf ("Could not open the PSUWopWop file: %s for output! \n",FileName);

       exit(1);

    }    
     
    return File[i];
          
}

/*##############################################################################
#                                                                              #
#                    WOPWOP OpenThicknessGeometryFile                          #
#                                                                              #
##############################################################################*/

FILE *WOPWOP::OpenThicknessGeometryFile(int i, char *FileName, FILE **File)
{

    // Open file
    
    if ( (File[i] = fopen(FileName, "wb")) == NULL ) {

       printf ("Could not open the PSUWopWop file: %s for output! \n",FileName);

       exit(1);

    }    
          
    return File[i];
          
}

/*##############################################################################
#                                                                              #
#                            WOPWOP OpenBPMFile                                #
#                                                                              #
##############################################################################*/

FILE *WOPWOP::OpenBPMFile(char *FileName)
{

    // Open file
    
    if ( (BPMFile_ = fopen(FileName, "wb")) == NULL ) {

       printf ("Could not open the PSUWopWop file: %s for output! \n",FileName);

       exit(1);

    }   
    
    return BPMFile_;
          
}

/*##############################################################################
#                                                                              #
#                         WOPWOP operator=                                     #
#                                                                              #
##############################################################################*/

WOPWOP& WOPWOP::operator=(const WOPWOP &WopWopRotor)
{

    int i;
    
    // Rotor
    
    NumberOfBlades_ = WopWopRotor.NumberOfBlades_;
    
    SizeBladeList(NumberOfBlades_);
    
    for ( i = 1 ; i <= NumberOfBlades_ ; i++ ) {

       SurfaceForBlade_[i] = WopWopRotor.SurfaceForBlade_[i];

    }
    
    NumberOfBladesSections_ = WopWopRotor.NumberOfBladesSections_;
    
    RotorID_ = WopWopRotor.RotorID_;
 
    // Wing

    NumberOfWingSurfaces_ = WopWopRotor.NumberOfWingSurfaces_;
    
    SizeWingSurfaceList(NumberOfWingSurfaces_);
    
    for ( i = 1 ; i <= NumberOfWingSurfaces_ ; i++ ) {

       SurfaceForWing_[i] = WopWopRotor.SurfaceForWing_[i];

    }
    
    NumberOfWingSections_ = WopWopRotor.NumberOfWingSections_;
    
    WingID_ = WopWopRotor.WingID_;    
    
    // Body

    NumberOfBodySurfaces_ = WopWopRotor.NumberOfBodySurfaces_;
    
    SizeBodySurfaceList(NumberOfBodySurfaces_);
    
    for ( i = 1 ; i <= NumberOfBodySurfaces_ ; i++ ) {

       SurfaceForBody_[i] = WopWopRotor.SurfaceForBody_[i];

    }
    
    NumberOfBodySections_ = WopWopRotor.NumberOfBodySections_;
    
    BodyID_ = WopWopRotor.BodyID_;        
           
    return *this;

}

/*##############################################################################
#                                                                              #
#                              WOPWOP destructor                               #
#                                                                              #
##############################################################################*/

WOPWOP::~WOPWOP(void)
{

    if ( SurfaceForBlade_ != NULL ) delete [] SurfaceForBlade_;
    if ( SurfaceForWing_  != NULL ) delete [] SurfaceForWing_;
    if ( SurfaceForBody_  != NULL ) delete [] SurfaceForBody_;
    
    if ( RotorLoadingGeometryFile_   != NULL ) delete [] RotorLoadingGeometryFile_;
    if ( RotorLoadingFile_           != NULL ) delete [] RotorLoadingFile_;
    if ( RotorThicknessGeometryFile_ != NULL ) delete [] RotorThicknessGeometryFile_;
    if ( BPMFile_                    != NULL ) delete [] BPMFile_;

    if ( WingLoadingGeometryFile_   != NULL ) delete [] WingLoadingGeometryFile_;
    if ( WingLoadingFile_           != NULL ) delete [] WingLoadingFile_;
    if ( WingThicknessGeometryFile_ != NULL ) delete [] WingThicknessGeometryFile_;

    if ( BodyThicknessGeometryFile_ != NULL ) delete [] BodyThicknessGeometryFile_;

    // Rotor
    
    NumberOfBlades_ = 0;
    
    SurfaceForBlade_ = NULL;
    
    NumberOfBladesSections_ = 0;
    
    RotorID_ = 0;
    
    // Wing
    
    NumberOfWingSurfaces_ = 0;

    SurfaceForWing_ = NULL;
    
    WingID_ = 0;
    
    NumberOfWingSections_ = 0;
    
    // Body
    
    NumberOfBodySurfaces_ = 0;
    
    SurfaceForBody_ = NULL;
    
    BodyID_ = 0;
    
    NumberOfBodySections_ = 0;
    
    RotorLoadingGeometryFile_   = NULL;
    RotorLoadingFile_           = NULL;
    RotorThicknessGeometryFile_ = NULL;
    BPMFile_                    = NULL;

    WingLoadingGeometryFile_   = NULL;
    WingLoadingFile_           = NULL;
    WingThicknessGeometryFile_ = NULL;

    BodyThicknessGeometryFile_ = NULL;
    
}

/*##############################################################################
#                                                                              #
#                              WOPWOP CloseFiles                               #
#                                                                              #
##############################################################################*/

void WOPWOP::CloseFiles(void)
{

    int i, NumFiles;

    NumFiles = MAX(NumberOfBlades_, NumberOfWingSurfaces_);
    
    for ( i = 1 ; i <= NumberOfBlades_ ; i++ ) {
       
       if ( RotorLoadingGeometryFile_[i]   != NULL ) fclose(RotorLoadingGeometryFile_[i]);
       if ( RotorLoadingFile_[i]           != NULL ) fclose(RotorLoadingFile_[i]);
       if ( RotorThicknessGeometryFile_[i] != NULL ) fclose(RotorThicknessGeometryFile_[i]);
       
    }

    for ( i = 1 ; i <= NumberOfWingSurfaces_ ; i++ ) {
       
       if ( WingLoadingGeometryFile_[i]   != NULL ) fclose(WingLoadingGeometryFile_[i]);
       if ( WingLoadingFile_[i]           != NULL ) fclose(WingLoadingFile_[i]);
       if ( WingThicknessGeometryFile_[i] != NULL ) fclose(WingThicknessGeometryFile_[i]);
       
    }
    
    for ( i = 1 ; i <= NumberOfBodySurfaces_ ; i++ ) {
       
       if ( BodyThicknessGeometryFile_[i] != NULL ) fclose(BodyThicknessGeometryFile_[i]);
       
    }
        
    if ( BPMFile_ != NULL ) fclose(BPMFile_);
    
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
   
    zero_double_array(Gamma_[0], VSPGeom().Grid(MGLevel_).NumberOfLoops()); Gamma_[0][0] = 0.;   
    zero_double_array(Gamma_[1], VSPGeom().Grid(MGLevel_).NumberOfLoops()); Gamma_[1][0] = 0.;
    zero_double_array(Gamma_[2], VSPGeom().Grid(MGLevel_).NumberOfLoops()); Gamma_[2][0] = 0.;
    zero_double_array(Delta_,    VSPGeom().Grid(MGLevel_).NumberOfLoops());    Delta_[0] = 0.;
        
    CurrentTime_ = 0.;
       
    // Keep track of unsteady forces and moments
    
    if ( TimeAccurate_ ) {
       
       if (  CLi_Unsteady_ != NULL ) delete []  CLi_Unsteady_;    
       if (  CDi_Unsteady_ != NULL ) delete []  CDi_Unsteady_;      
       if (  CSi_Unsteady_ != NULL ) delete []  CSi_Unsteady_;       
       if ( CFix_Unsteady_ != NULL ) delete [] CFix_Unsteady_;      
       if ( CFiy_Unsteady_ != NULL ) delete [] CFiy_Unsteady_;       
       if ( CFiz_Unsteady_ != NULL ) delete [] CFiz_Unsteady_;       
       if ( CMix_Unsteady_ != NULL ) delete [] CMix_Unsteady_;       
       if ( CMiy_Unsteady_ != NULL ) delete [] CMiy_Unsteady_;       
       if ( CMiz_Unsteady_ != NULL ) delete [] CMiz_Unsteady_;       
             
        CLi_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
        CDi_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
        CSi_Unsteady_ = new double[NumberOfTimeSteps_ + 1];    
       CFix_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CFiy_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CFiz_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMix_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMiy_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMiz_Unsteady_ = new double[NumberOfTimeSteps_ + 1];       
 
    }

    // Update geometry location... really just the surface velocities
             
    CurrentTime_ = CurrentNoiseTime_ = 0.;
             
    Time_ = 0;
             
    if ( TimeAccurate_ && !StartFromSteadyState_ ) UpdateGeometryLocation(GEOMETRY_UPDATE_DO_STARTUP);
                 
    // Initialize free stream
    
    InitializeFreeStream();

    // Create interaction list for fixed components

    CreateSurfaceVorticesInteractionList(0);

    // Initialize the wake trailing vortices

    InitializeTrailingVortices();
    
    ZeroVortexState();

    // Calculate the right hand side
    
    CalculateRightHandSide();

    for ( i = 1 ; i <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; i++ ) {

        VSPGeom().Grid(MGLevel_).LoopList(i).Gamma() = Gamma(i) = 0.;
 
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

                       //1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456           
    fprintf(StatusFile_,"      Time             Mach             AoA              Beta              CLo             CLi            CLtot              CDo              CDi             CDtot              CDt            CDtot_t           CSo              CSi            CStot               L/D              E               CFox             CFoy             CFoz             CFix             CFiy             CFiz             CFxtot           CFytot           CFztot           CMox             CMoy             CMoz             CMix             CMiy             CMiz             CMxtot           CMytot           CMztot          T/QS \n");
 
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

                            //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890    
       fprintf(GroupFile_[c],"Force Coefficients for group %d --> %s \n\n",c,ComponentGroupList_[c].GroupName()); 
       fprintf(GroupFile_[c],"   Time        Cx         Cy         Cz        Cxo        Cyo        Czo        Cxi        Cyi        Czi        Cmx        Cmy        Cmz        Cmxo       Cmyo       Cmzo       Cmxi       Cmyi       Cmzi        CL         CD         CS        CLo        CDo        CLi        CDi \n");
                          
       // Create rotor file
       
       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          k++;
          
          sprintf(RotorFileName,"%s.noise.rotor.%d",FileName_,k);
    
          if ( (RotorFile_[k] = fopen(RotorFileName, "w")) == NULL ) {
      
             printf("Could not open the %s rotor coefficient file! \n",RotorFileName);
      
             exit(1);
      
          }
 
                               //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890
          fprintf(RotorFile_[k],"                                                                                                                                        ------------ Propeller Coefficients _----------       -------- Rotor Coefficients _-------\n\n");                        
          fprintf(RotorFile_[k]," Time       Diameter     RPM       Thrust    Thrusto    Thrusti     Power      Powero     Poweri     Moment     Momento    Momenti      J          CT         CQ         CP        EtaP       CT_H       CQ_H       CP_H       FOM        Angle \n"); 
       
       }

    }     
  
    // Open the input adb file
    
    if ( Case == 0 || Case == 1 ) {

       sprintf(ADBFileName,"%s.adb",FileName_);
       
       if ( (InputADBFile_ = fopen(ADBFileName, "rb")) == NULL ) {
   
          printf("Could not open the aero data base file for binary input! \n");
   
          exit(1);
   
       }

       // Read in the header
       
       ReadInAerothermalDatabaseHeader();

    }
    
    // Open the output adb file
    
    if ( Case == 0 || Case == 1 ) {

       sprintf(ADBFileName,"%s.noise.adb",FileName_);
       
       if ( (ADBFile_ = fopen(ADBFileName, "wb")) == NULL ) {
   
          printf("Could not open the aero data base file for binary output! \n");
   
          exit(1);
   
       }

       WriteOutAerothermalDatabaseHeader();

    }

    // Open the case file
    
    sprintf(ADBFileName,"%s.noise.adb.cases",FileName_);
    
    if ( (ADBCaseListFile_ = fopen(ADBFileName, "w")) == NULL ) {

       printf("Could not open the aero data base case list file for output! \n");

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
                 
       UpdateVortexEdgeStrengths(MGLevel_, ALL_WAKE_GAMMAS);
          
       // Initialize the free stream conditions
                  
       InitializeFreeStream();
    
       // Calculate forces

       CalculateForces();
   
       // Output status

       OutputStatusFile(0);

       // Write out any rotor coefficients
       
       OutputForcesAndMomentsForGroup(0);
       
       // Write out geometry and current solution

       sprintf(CaseString_,"Time: %-f ...",CurrentNoiseTime_);

       WriteOutAerothermalDatabaseGeometry();

       InterpolateSolutionFromGrid(MGLevel_);
       
       WriteOutAerothermalDatabaseSolution();
                     
       // Write out PSU WopWop Files for unsteady analysis

       if ( Time_ == NumberOfTimeSteps_ - 1 ) { for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) WriteOutPSUWopWopUnsteadyDataForGroup(c); };

       // Update geometry location and interaction lists for moving geoemtries
       
       UpdateGeometryLocation(GEOMETRY_UPDATE_DO_ALL);     
         
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
    double Time_0, Time_1, Time_2, Epsilon, EvaluationTime, Period;
    char StatusFileName[2000], ADBFileName[2000];
    char GroupFileName[2000], RotorFileName[2000];
   
    // Zero out solution
   
    zero_double_array(Gamma_[0], VSPGeom().Grid(MGLevel_).NumberOfLoops()); Gamma_[0][0] = 0.;   
    zero_double_array(Gamma_[1], VSPGeom().Grid(MGLevel_).NumberOfLoops()); Gamma_[1][0] = 0.;
    zero_double_array(Gamma_[2], VSPGeom().Grid(MGLevel_).NumberOfLoops()); Gamma_[2][0] = 0.;
    zero_double_array(Delta_,    VSPGeom().Grid(MGLevel_).NumberOfLoops());    Delta_[0] = 0.;
        
    CurrentTime_ = 0.;
       
    // Keep track of unsteady forces and moments
    
    if ( TimeAccurate_ ) {
       
       if (  CLi_Unsteady_ != NULL ) delete []  CLi_Unsteady_;    
       if (  CDi_Unsteady_ != NULL ) delete []  CDi_Unsteady_;      
       if (  CSi_Unsteady_ != NULL ) delete []  CSi_Unsteady_;       
       if ( CFix_Unsteady_ != NULL ) delete [] CFix_Unsteady_;      
       if ( CFiy_Unsteady_ != NULL ) delete [] CFiy_Unsteady_;       
       if ( CFiz_Unsteady_ != NULL ) delete [] CFiz_Unsteady_;       
       if ( CMix_Unsteady_ != NULL ) delete [] CMix_Unsteady_;       
       if ( CMiy_Unsteady_ != NULL ) delete [] CMiy_Unsteady_;       
       if ( CMiz_Unsteady_ != NULL ) delete [] CMiz_Unsteady_;       
             
        CLi_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
        CDi_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
        CSi_Unsteady_ = new double[NumberOfTimeSteps_ + 1];    
       CFix_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CFiy_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CFiz_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMix_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMiy_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMiz_Unsteady_ = new double[NumberOfTimeSteps_ + 1];       
 
    }

    // Update geometry location... really just the surface velocities
             
    CurrentTime_ = CurrentNoiseTime_ = 0.;
    
    Time_ = 0;
             
    if ( TimeAccurate_ && !StartFromSteadyState_ ) UpdateGeometryLocation(GEOMETRY_UPDATE_DO_STARTUP);
                   
    // Initialize free stream
    
    InitializeFreeStream();

    // Initialize the wake trailing vortices

    InitializeTrailingVortices();
    
    ZeroVortexState();

    // Create interaction list for fixed components

    CreateSurfaceVorticesInteractionList(0);
  
    // Calculate the right hand side
    
    CalculateRightHandSide();

    for ( i = 1 ; i <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; i++ ) {

        VSPGeom().Grid(MGLevel_).LoopList(i).Gamma() = Gamma(i) = 0.;
 
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

                       //1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456 1234567890123456           
    fprintf(StatusFile_,"      Time             Mach             AoA              Beta              CLo             CLi            CLtot              CDo              CDi             CDtot              CDt            CDtot_t           CSo              CSi            CStot               L/D              E               CFox             CFoy             CFoz             CFix             CFiy             CFiz             CFxtot           CFytot           CFztot           CMox             CMoy             CMoz             CMix             CMiy             CMiz             CMxtot           CMytot           CMztot          T/QS \n");

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

                               //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890
          fprintf(RotorFile_[k],"                                                                                                                                        ------------ Propeller Coefficients _----------       -------- Rotor Coefficients _-------\n\n");                        
          fprintf(RotorFile_[k]," Time       Diameter     RPM       Thrust    Thrusto    Thrusti     Power      Powero     Poweri     Moment     Momento    Momenti      J          CT         CQ         CP        EtaP       CT_H       CQ_H       CP_H       FOM        Angle \n");
                             
       }

    }     
  
    // Open the input adb file

    sprintf(ADBFileName,"%s.adb",FileName_);
    
    if ( (InputADBFile_ = fopen(ADBFileName, "rb")) == NULL ) {
    
       printf("Could not open the aero data base file for binary input! \n");
    
       exit(1);
    
    }

    // Open the output adb file

    WopWopWriteOutADBFile_ = 1;    

    if ( WopWopWriteOutADBFile_ ) {

       sprintf(ADBFileName,"%s.noise.adb",FileName_);
       
       if ( (ADBFile_ = fopen(ADBFileName, "wb")) == NULL ) {
   
          printf("Could not open the aero data base file for binary output! \n");
   
          exit(1);
   
       }

       WriteOutAerothermalDatabaseHeader();

    }

    // Open the case file
    
    sprintf(ADBFileName,"%s.noise.adb.cases",FileName_);
    
    if ( (ADBCaseListFile_ = fopen(ADBFileName, "w")) == NULL ) {

       printf("Could not open the aero data base case list file for output! \n");

       exit(1);

    }       
    
    // Set up PSU-WopWop data
           
    SetupPSUWopWopData();
   
    // Loop over all the component groups and write out the PSU-WopWop data files

    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       for ( i = 1 ; i <= NumberOfComponentGroups_ ; i++ ) {
          
          ComponentGroupList_[i].ZeroAverageForcesAndMoments();
          
       }
              
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
   
       if ( Verbose_ ) { printf("Stepping through solution and writing out sound files... \n\n");fflush(NULL); };
   
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
  
             UpdateVortexEdgeStrengths(MGLevel_, ALL_WAKE_GAMMAS);
   
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
   
          sprintf(CaseString_,"Time: %-f ...",CurrentNoiseTime_);
   
          if ( WopWopWriteOutADBFile_ && c == 1 ) WriteOutAerothermalDatabaseGeometry();
   
          InterpolateSolutionFromGrid(MGLevel_);
          
          if ( WopWopWriteOutADBFile_ && c == 1 ) WriteOutAerothermalDatabaseSolution();
                        
          // Write out PSU WopWop Files for unsteady analysis

          WriteOutPSUWopWopUnsteadyDataForGroup(c);
  
          // Update geometry location and interaction lists for moving geoemtries
          
          UpdateGeometryLocation(GEOMETRY_UPDATE_DO_ALL);    
             
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
#                       VSP_SOLVER SetupPSUWopWopData                          #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::SetupPSUWopWopData(void)
{

// broken    int c, i, j, k, NumberOfBlades, NumberOfWingSurfaces, NumberOfBodySurfaces;
// broken    int LastComponentID, *ComponentInThisGroup, PowerOfTwo;
// broken    
// broken    // Unit conversions
// broken
// broken    if ( WopWopEnglishUnits_ ) {
// broken       
// broken       WopWopDensityConversion_  = 515.379;
// broken       
// broken       WopWopLengthConversion_   = 0.3048;
// broken       
// broken       WopWopAreaConversion_     = WopWopLengthConversion_ * WopWopLengthConversion_;
// broken       
// broken       WopWopPressureConversion_ = 47.880172;
// broken       
// broken       WopWopForceConversion_    = 4.4482216;
// broken       
// broken    }
// broken    
// broken    else {
// broken       
// broken       WopWopDensityConversion_  = 1.;
// broken       
// broken       WopWopLengthConversion_   = 1.;
// broken       
// broken       WopWopAreaConversion_     = 1.;
// broken       
// broken       WopWopPressureConversion_ = 1.;     
// broken       
// broken       WopWopForceConversion_    = 1.;
// broken       
// broken    }  
// broken    
// broken    // Time steps
// broken    
// broken    if ( !TimeAccurate_ ) {
// broken
// broken       WopWopdTau_ = (1./(BladeRPM_/60.))/128.;
// broken       
// broken       if ( !WopWopFlyBy_ ) WopWopdTau_ /= 10.;
// broken
// broken    }
// broken    
// broken    else {
// broken       
// broken       WopWopdTau_ = NoiseTimeStep_;
// broken       
// broken    }    
// broken    
// broken    // Determine the number of rotors and blades per rotor
// broken
// broken    ComponentInThisGroup = new int[VSPGeom().NumberOfComponents() + 1];
// broken   
// broken    WopWopNumberOfRotors_ = WopWopNumberOfWings_ = WopWopNumberOfBodies_ = WopWopNumberOfTotalBlades_ = WopWopNumberOfTotalWingSurfaces_ = WopWopNumberOfTtoalBodySurfaces_ = 0;
// broken   
// broken    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
// broken
// broken       // Rotor
// broken       
// broken       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
// broken          
// broken          WopWopNumberOfRotors_++;
// broken          
// broken          zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());
// broken
// broken          for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
// broken          
// broken             ComponentInThisGroup[ComponentGroupList_[c].ComponentList(j)] = 1;
// broken             
// broken          }
// broken                 
// broken          // Determine number of blades
// broken                    
// broken          NumberOfBlades = 0;
// broken       
// broken          for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
// broken          
// broken             if ( ComponentInThisGroup[VSPGeom().ComponentIDForSurface(i)] ) NumberOfBlades++;
// broken             
// broken          }
// broken
// broken          ComponentGroupList_[c].WopWop().SizeBladeList(NumberOfBlades);
// broken          
// broken          ComponentGroupList_[c].WopWop().RotorID() = WopWopNumberOfRotors_;
// broken
// broken          k = 0;
// broken          
// broken          for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
// broken          
// broken             if ( ComponentInThisGroup[VSPGeom().ComponentIDForSurface(i)] ) {
// broken                
// broken                ComponentGroupList_[c].WopWop().SurfaceForBlade(++k) = i;
// broken             
// broken              ComponentGroupList_[c].WopWop().NumberOfBladesSections() = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
// broken         
// broken             }
// broken             
// broken          }
// broken          
// broken          WopWopNumberOfTotalBlades_ += NumberOfBlades;
// broken          
// broken       }
// broken       
// broken    }
// broken    
// broken    // Check for possible wing surfaces
// broken
// broken    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
// broken       
// broken       if ( !ComponentGroupList_[c].GeometryIsARotor() ) {
// broken          
// broken          zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());
// broken
// broken          for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
// broken          
// broken             ComponentInThisGroup[ComponentGroupList_[c].ComponentList(j)] = 1;
// broken             
// broken          }
// broken                 
// broken          // Determine number of blades
// broken                    
// broken          NumberOfWingSurfaces = 0;
// broken       
// broken          for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
// broken          
// broken             if ( ComponentInThisGroup[VSPGeom().ComponentIDForSurface(i)] && VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) NumberOfWingSurfaces++;
// broken             
// broken          }
// broken          
// broken          if ( NumberOfWingSurfaces > 0 ) {
// broken             
// broken             printf("Found: %d wing surfaces in group: %d \n",NumberOfWingSurfaces, c);
// broken             
// broken             WopWopNumberOfWings_++;
// broken
// broken             ComponentGroupList_[c].WopWop().SizeWingSurfaceList(NumberOfWingSurfaces);
// broken             
// broken             ComponentGroupList_[c].WopWop().WingID() = WopWopNumberOfWings_;
// broken             
// broken             ComponentGroupList_[c].GeometryHasWings() = 1;
// broken   
// broken             k = 0;
// broken             
// broken             for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
// broken             
// broken                if ( ComponentInThisGroup[VSPGeom().ComponentIDForSurface(i)] && VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
// broken                   
// broken                   ComponentGroupList_[c].WopWop().SurfaceForWing(++k) = i;
// broken                
// broken                   ComponentGroupList_[c].WopWop().NumberOfWingSections() = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
// broken            
// broken                }
// broken                
// broken             }
// broken             
// broken             WopWopNumberOfTotalWingSurfaces_ += NumberOfWingSurfaces;
// broken             
// broken          }          
// broken          
// broken       }
// broken       
// broken    }
// broken
// broken    // Check for possible body surfaces
// broken
// broken    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
// broken       
// broken       if ( !ComponentGroupList_[c].GeometryIsARotor() ) {
// broken          
// broken          zero_int_array(ComponentInThisGroup, VSPGeom().NumberOfComponents());
// broken
// broken          for ( j = 1 ; j <= ComponentGroupList_[c].NumberOfComponents() ; j++ ) {
// broken          
// broken             ComponentInThisGroup[ComponentGroupList_[c].ComponentList(j)] = 1;
// broken             
// broken          }
// broken                 
// broken          // Determine number of blades
// broken                    
// broken          NumberOfBodySurfaces = 0;
// broken       
// broken          LastComponentID = -999;
// broken          
// broken          for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
// broken          
// broken             if ( ComponentInThisGroup[VSPGeom().ComponentIDForSurface(i)] && VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
// broken                
// broken                if ( VSPGeom().ComponentIDForSurface(i) != LastComponentID ) {
// broken                   
// broken                   NumberOfBodySurfaces++;
// broken                   
// broken                   LastComponentID = VSPGeom().ComponentIDForSurface(i);
// broken                   
// broken                }
// broken                
// broken             }
// broken             
// broken          }
// broken          
// broken          if ( NumberOfBodySurfaces > 0 ) {
// broken             
// broken             printf("Found: %d body surfaces in group: %d \n",NumberOfBodySurfaces, c);
// broken             
// broken             WopWopNumberOfBodies_++;
// broken
// broken             ComponentGroupList_[c].WopWop().SizeBodySurfaceList(NumberOfBodySurfaces);
// broken             
// broken             ComponentGroupList_[c].WopWop().BodyID() = WopWopNumberOfBodies_;
// broken             
// broken             ComponentGroupList_[c].GeometryHasBodies() = 1;
// broken   
// broken             LastComponentID = -999;
// broken    
// broken             k = 0;
// broken             
// broken             for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
// broken             
// broken                if ( ComponentInThisGroup[VSPGeom().ComponentIDForSurface(i)] && VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
// broken                   
// broken                   if ( VSPGeom().ComponentIDForSurface(i) != LastComponentID ) {
// broken                      
// broken                      LastComponentID = VSPGeom().ComponentIDForSurface(i);
// broken                   
// broken                      ComponentGroupList_[c].WopWop().SurfaceForBody(++k) = i;
// broken                
// broken                     ComponentGroupList_[c].WopWop().NumberOfBodySections() = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
// broken                      
// broken                   }
// broken            
// broken                }
// broken                
// broken             }
// broken             
// broken             WopWopNumberOfTtoalBodySurfaces_ += NumberOfWingSurfaces;
// broken             
// broken          }          
// broken          
// broken       }
// broken       
// broken    }
// broken    
// broken    delete [] ComponentInThisGroup;
// broken    
// broken    // Determine the slowest and fastest rotor
// broken    
// broken    WopWopOmegaMin_ =  1.e9;
// broken    WopWopOmegaMax_ = -1.e9;
// broken    
// broken    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
// broken
// broken       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
// broken          
// broken          if ( ABS(ComponentGroupList_[c].Omega()) < WopWopOmegaMin_ ) {
// broken          
// broken             WopWopLongestPeriodGroup_ = c;
// broken          
// broken             WopWopOmegaMin_ = ABS(ComponentGroupList_[c].Omega());
// broken             
// broken          }
// broken
// broken          if ( ABS(ComponentGroupList_[c].Omega()) > WopWopOmegaMax_ ) {
// broken
// broken             WopWopOmegaMax_ = ABS(ComponentGroupList_[c].Omega());
// broken             
// broken          }
// broken                    
// broken       }
// broken       
// broken   }
// broken   
// broken   if ( TimeStep_ < 0. ) {
// broken
// broken      printf("wtf!... Time Step is still less than zero! \n");fflush(NULL);exit(1);
// broken      
// broken   }   
// broken   
// broken   // 2 degrees per step
// broken   
// broken   NumberOfNoiseTimeSteps_ = 181;
// broken
// broken   // Calculate start point of periodic noise analysis, this is
// broken   // driven by the longest period motion
// broken   
// broken   WopWopLongestPeriod_ = 2.*PI/WopWopOmegaMin_;
// broken   
// broken   NoiseTimeStep_ = WopWopLongestPeriod_ / (NumberOfNoiseTimeSteps_ - 1);
// broken   
// broken   NoiseTimeShift_ = NumberOfTimeSteps_ * TimeStep_ - WopWopLongestPeriod_- TimeStep_;
// broken
// broken   // Calculate observer time, make it a power of 2
// broken
// broken   WopWopObserverTime_ = 5.*WopWopLongestPeriod_;
// broken
// broken   WopWopNumberOfTimeSteps_ = float( 4. * ( WopWopObserverTime_ / NoiseTimeStep_ + 1.) );
// broken   
// broken   PowerOfTwo = 1;
// broken   
// broken   while ( 2*PowerOfTwo < WopWopNumberOfTimeSteps_ ) PowerOfTwo *= 2;
// broken
// broken   WopWopNumberOfTimeSteps_ = PowerOfTwo;
// broken
// broken   printf("PSU-WopWop Setup: \n");
// broken   printf("Total Solution time:      %f \n",NumberOfTimeSteps_ * TimeStep_);
// broken   printf("WopWopOmegaMin:           %f \n",WopWopOmegaMin_);
// broken   printf("Longest Period:           %f \n",WopWopLongestPeriod_);
// broken   printf("NumberOfNoiseTimeSteps_:  %d \n",NumberOfNoiseTimeSteps_);
// broken   printf("NoiseTimeShift_:          %f \n",NoiseTimeShift_);
// broken   printf("WopWopObserverTime_:      %f \n",WopWopObserverTime_);
// broken   printf("WopWopNumberOfTimeSteps_: %d \n",WopWopNumberOfTimeSteps_);
// broken   printf("\n");

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

       printf("Could not open the PSUWopWop Case File output! \n");

       exit(1);

    }    
    
    // Baseline case, with all rotors

    fprintf(WopWopCaseFile,"! \n");
    fprintf(WopWopCaseFile,"!Remove the comment delimiter from those cases below you wish to run, beyond the default all up case \n");
    fprintf(WopWopCaseFile,"! \n");
    
    sprintf(NameListFile,"%s.nam",FileName_);
    
    fprintf(WopWopCaseFile,"&caseName\n");
    fprintf(WopWopCaseFile,"   globalFolderName = \'./\' \n");
    fprintf(WopWopCaseFile,"   caseNameFile = \'%s\' \n",NameListFile);
    fprintf(WopWopCaseFile,"/ \n");        
    
    // Case with no rotors

    sprintf(NameListFile,"%s.NoRotors.nam",FileName_);
    
    fprintf(WopWopCaseFile,"!&caseName\n");
    fprintf(WopWopCaseFile,"!   globalFolderName = \'./\' \n");
    fprintf(WopWopCaseFile,"!   caseNameFile = \'%s\' \n",NameListFile);
    fprintf(WopWopCaseFile,"!/ \n");       
    
    // Cases with a single rotor included 
        
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          i = ComponentGroupList_[c].WopWop().RotorID();
          
          sprintf(NameListFile,"%s.Rotor.%d.nam",FileName_,i);
          
          fprintf(WopWopCaseFile,"!&caseName\n");
          fprintf(WopWopCaseFile,"!   globalFolderName = \'./\' \n");
          fprintf(WopWopCaseFile,"!   caseNameFile = \'%s\' \n",NameListFile);
          fprintf(WopWopCaseFile,"!/ \n");      
          
       }
       
    }        
                    
    fclose(WopWopCaseFile);
    
    // Create multiple psuwopwop cases, body/wings alone, bodies/wings + single rotors, and full up case
    
    for ( Case = -1 ; Case <= WopWopNumberOfRotors_ ; Case++ ) {

       if ( Case == -1 ) sprintf(NameListFile,"%s.NoRotors.nam",FileName_);
       if ( Case ==  0 ) sprintf(NameListFile,"%s.nam",FileName_);
       if ( Case >   0 ) sprintf(NameListFile,"%s.Rotor.%d.nam",FileName_,Case);
       
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
       
       if ( Case == -1 ) {
          
          sprintf(pressureFileName,       "pressure.NoRotors");
          sprintf(SPLFileName,            "spl.NoRotors");
          sprintf(OASPLFileName,          "OASPL.NoRotors.");
          sprintf(phaseFileName,          "phase.NoRotors.");
          sprintf(complexPressureFileName,"complexPressure.NoRotors");
          sprintf(audioFileName,          "sigma.NoRotors");
          
       }
     
       if ( Case == 0 ) {
          
          sprintf(pressureFileName,       "pressure.AllRotors");
          sprintf(SPLFileName,            "spl.AllRotors");
          sprintf(OASPLFileName,          "OASPL.AllRotors.");
          sprintf(phaseFileName,          "phase.AllRotors.");
          sprintf(complexPressureFileName,"complexPressure.AllRotors");
          sprintf(audioFileName,          "sigma.AllRotors");
          
       }
    
       if ( Case > 0 ) {
          
          sprintf(pressureFileName,       "pressure.Rotor.%d",Case);
          sprintf(SPLFileName,            "spl.Rotor.%d",Case);
          sprintf(OASPLFileName,          "OASPL.Rotor.%d.",Case);
          sprintf(phaseFileName,          "phase.Rotor.%d.",Case);
          sprintf(complexPressureFileName,"complexPressure.Rotor.%d",Case);
          sprintf(audioFileName,          "sigma.Rotor.%d",Case);
          
       }

       fprintf(PSUWopWopNameListFile_,"   pressureFileName        = %s \n",pressureFileName);
       fprintf(PSUWopWopNameListFile_,"   SPLFileName             = %s \n",SPLFileName);           
       fprintf(PSUWopWopNameListFile_,"   OASPLFileName           = %s \n",OASPLFileName);         
       fprintf(PSUWopWopNameListFile_,"   phaseFileName           = %s \n",phaseFileName);         
       fprintf(PSUWopWopNameListFile_,"   complexPressureFileName = %s \n",complexPressureFileName);
       fprintf(PSUWopWopNameListFile_,"   audioFileName           = %s \n",audioFileName);         
          
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
       
       // Just wings/bodies
       
       if ( Case == -1 ) {
          
          fprintf(PSUWopWopNameListFile_,"   nbContainer  = %d \n",                    0 + WopWopNumberOfWings_ + WopWopNumberOfBodies_);
       
       }
       
       // All rotors + wings/bodies
       
       else if ( Case == 0 ) {
          
          fprintf(PSUWopWopNameListFile_,"   nbContainer  = %d \n",WopWopNumberOfRotors_ + WopWopNumberOfWings_ + WopWopNumberOfBodies_);
          
       }
       
       // Wings/bodies + 1 of the rotors
       
       else {
          
          fprintf(PSUWopWopNameListFile_,"   nbContainer  = %d \n",                    1 + WopWopNumberOfWings_ + WopWopNumberOfBodies_);
          
       }
       
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
       
       if ( Case >= 0 ) {
          
          for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
      
             if ( ComponentGroupList_[c].GeometryIsARotor() ) {
                
                i = ComponentGroupList_[c].WopWop().RotorID();
                
                if ( Case == 0 || Case == i ) {
      
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

       printf("Could not open the PSUWopWop Case File output! \n");

       exit(1);

    }    
    
    // Baseline case, with all rotors

    fprintf(WopWopCaseFile,"! \n");
    fprintf(WopWopCaseFile,"!Remove the comment delimiter from those cases below you wish to run, beyond the default all up case \n");
    fprintf(WopWopCaseFile,"! \n");
   
    sprintf(NameListFile,"%s.nam",FileName_);
    
    fprintf(WopWopCaseFile,"&caseName\n");
    fprintf(WopWopCaseFile,"   globalFolderName = \'./\' \n");
    fprintf(WopWopCaseFile,"   caseNameFile = \'%s\' \n",NameListFile);
    fprintf(WopWopCaseFile,"/ \n");        
    
    // Case with no rotors

    sprintf(NameListFile,"%s.NoRotors.nam",FileName_);
    
    fprintf(WopWopCaseFile,"!&caseName\n");
    fprintf(WopWopCaseFile,"!   globalFolderName = \'./\' \n");
    fprintf(WopWopCaseFile,"!   caseNameFile = \'%s\' \n",NameListFile);
    fprintf(WopWopCaseFile,"!/ \n");       
    
    // Cases with a single rotor included 
        
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {

       if ( ComponentGroupList_[c].GeometryIsARotor() ) {
          
          i = ComponentGroupList_[c].WopWop().RotorID();
          
          sprintf(NameListFile,"%s.Rotor.%d.nam",FileName_,i);
          
          fprintf(WopWopCaseFile,"!&caseName\n");
          fprintf(WopWopCaseFile,"!   globalFolderName = \'./\' \n");
          fprintf(WopWopCaseFile,"!   caseNameFile = \'%s\' \n",NameListFile);
          fprintf(WopWopCaseFile,"!/ \n");      
          
       }
       
    }          
                    
    fclose(WopWopCaseFile);

    // Create multiple psuwopwop cases, body/wings alone, bodies/wings + single rotors, and full up case
    
    for ( Case = -1 ; Case <= WopWopNumberOfRotors_ ; Case++ ) {

       if ( Case == -1 ) sprintf(NameListFile,"%s.NoRotors.nam",FileName_);
       if ( Case ==  0 ) sprintf(NameListFile,"%s.nam",FileName_);
       if ( Case >   0 ) sprintf(NameListFile,"%s.Rotor.%d.nam",FileName_,Case);

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

       if ( Case == -1 ) {
          
          sprintf(pressureFileName,       "pressure.NoRotors");
          sprintf(SPLFileName,            "spl.NoRotors");
          sprintf(OASPLFileName,          "OASPL.NoRotors.");
          sprintf(phaseFileName,          "phase.NoRotors.");
          sprintf(complexPressureFileName,"complexPressure.NoRotors");
          sprintf(audioFileName,          "sigma.NoRotors");
          
       }
     
       if ( Case == 0 ) {
          
          sprintf(pressureFileName,       "pressure.AllRotors");
          sprintf(SPLFileName,            "spl.AllRotors");
          sprintf(OASPLFileName,          "OASPL.AllRotors.");
          sprintf(phaseFileName,          "phase.AllRotors.");
          sprintf(complexPressureFileName,"complexPressure.AllRotors");
          sprintf(audioFileName,          "sigma.AllRotors");
          
       }
    
       if ( Case > 0 ) {
          
          sprintf(pressureFileName,       "pressure.Rotor.%d",Case);
          sprintf(SPLFileName,            "spl.Rotor.%d",Case);
          sprintf(OASPLFileName,          "OASPL.Rotor.%d.",Case);
          sprintf(phaseFileName,          "phase.Rotor.%d.",Case);
          sprintf(complexPressureFileName,"complexPressure.Rotor.%d",Case);
          sprintf(audioFileName,          "sigma.Rotor.%d",Case);
          
       }

       fprintf(PSUWopWopNameListFile_,"   pressureFileName        = %s \n",pressureFileName);
       fprintf(PSUWopWopNameListFile_,"   SPLFileName             = %s \n",SPLFileName);           
       fprintf(PSUWopWopNameListFile_,"   OASPLFileName           = %s \n",OASPLFileName);         
       fprintf(PSUWopWopNameListFile_,"   phaseFileName           = %s \n",phaseFileName);         
       fprintf(PSUWopWopNameListFile_,"   complexPressureFileName = %s \n",complexPressureFileName);
       fprintf(PSUWopWopNameListFile_,"   audioFileName           = %s \n",audioFileName);         
           
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
       
       if ( Case >= 0 ) {
          
          for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
      
             if ( ComponentGroupList_[c].GeometryIsARotor() ) {
                
                i = ComponentGroupList_[c].WopWop().RotorID();
                
                if ( Case == 0 || Case == i ) {

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
      
          fwrite(HeaderName, c_size, 32, WopFile); // Units
         
          // Comment line
          
          sprintf(DumChar,"ThicknessFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          fwrite(DumChar, c_size, 1024, WopFile); // Comment lLine

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
              
          fwrite(HeaderName, c_size, 32, WopFile);
          
//broken          NumberI = VSPGeom().VSP_Surface(k).Surface_NumI();
//broken          NumberJ = VSPGeom().VSP_Surface(k).Surface_NumJ();

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
   
                Period = float (ComponentGroupList_[c].Period());
          
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
      
          fwrite(HeaderName, c_size, 32, WopFile); // Units
         
          // Comment line
          
          sprintf(DumChar,"ThicknessFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          fwrite(DumChar, c_size, 1024, WopFile); // Comment lLine

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
              
          fwrite(HeaderName, c_size, 32, WopFile);
          
//broken          NumberI = VSPGeom().VSP_Surface(k).Surface_NumI();
//broken          NumberJ = VSPGeom().VSP_Surface(k).Surface_NumJ();

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
   
                Period = float(WopWopLongestPeriod_);
          
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
      
          fwrite(HeaderName, c_size, 32, WopFile); // Units
         
          // Comment line
          
          sprintf(DumChar,"ThicknessFile\n");
      
          Length = strlen(DumChar);
          memset(&DumChar[Length], ' ', 1024 - Length);
              
          fwrite(DumChar, c_size, 1024, WopFile); // Comment lLine

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
              
          fwrite(HeaderName, c_size, 32, WopFile);
          
//broken          NumberI = VSPGeom().VSP_Surface(k).Surface_NumI();
//broken          NumberJ = VSPGeom().VSP_Surface(k).Surface_NumJ();

          if ( !SteadyStateNoise_ ) {
               
             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
   
                Period = float (WopWopLongestPeriod_);
          
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
    
//broken   int i, j, k, m, n;
//broken   int i_size, c_size, f_size;
//broken   double Translation[3];
//broken   float DumFloat, x, y, z, Time;
//broken   FILE *WopFile;
//broken
//broken   // Sizeof int and float
//broken
//broken   i_size = sizeof(int);
//broken   c_size = sizeof(char);
//broken   f_size = sizeof(float);
//broken   
//broken   // Vehicle translation vector
//broken   
//broken   Translation[0] = Translation[1] = Translation[2] = 0.;
//broken   
//broken   if ( !SteadyStateNoise_ && WopWopPeriodicity_ == WOPWOP_APERIODIC ) {
//broken          
//broken      Translation[0] = -FreeStreamVelocity_[0] * CurrentNoiseTime_;
//broken      Translation[1] = -FreeStreamVelocity_[1] * CurrentNoiseTime_;
//broken      Translation[2] = -FreeStreamVelocity_[2] * CurrentNoiseTime_;
//broken      
//broken   }
//broken
//broken   // Loop over rotors and blades
//broken
//broken   if ( ComponentGroupList_[c].GeometryIsARotor() ) {
//broken      
//broken      i = ComponentGroupList_[c].WopWop().RotorID();
//broken   
//broken      for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {
//broken
//broken         // Thickness Geometry file
//broken         
//broken         WopFile = ComponentGroupList_[c].WopWop().ThicknessGeometryFileForBlade(j);
//broken         
//broken         // Surface for this blade
//broken         
//broken         k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);
//broken
//broken         // Current time
//broken     
//broken         Time = float (CurrentNoiseTime_);
//broken      
//broken         if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);             
//broken                         
//broken         // X node values
//broken   
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken
//broken                x = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_x(m,n) + Translation[0] ) );
//broken                
//broken                fwrite(&(x), f_size, 1, WopFile);
//broken           
//broken             }
//broken             
//broken          }
//broken          
//broken          // Y node values
//broken          
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken  
//broken                y = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_y(m,n) + Translation[1] ) );
//broken     
//broken                fwrite(&(y), f_size, 1, WopFile);
//broken                
//broken             }
//broken             
//broken          }
//broken
//broken          // z node values
//broken          
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken    
//broken                z = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_z(m,n) + Translation[2] ) );
//broken      
//broken                fwrite(&(z), f_size, 1, WopFile);                
//broken                
//broken             }
//broken             
//broken          } 
//broken          
//broken          // X Normal values
//broken           
//broken         for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken            
//broken            for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken         
//broken               DumFloat = float ( -VSPGeom().VSP_Surface(k).Surface_Nx(m,n) );
//broken       
//broken               fwrite(&(DumFloat), f_size, 1, WopFile);
//broken               
//broken            }
//broken            
//broken         } 
//broken          
//broken          // Y Normal values
//broken              
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken          
//broken                DumFloat = float ( -VSPGeom().VSP_Surface(k).Surface_Ny(m,n) );
//broken           
//broken                fwrite(&(DumFloat), f_size, 1, WopFile);
//broken                
//broken             }
//broken             
//broken          }  
//broken          
//broken          // Z Normal values
//broken              
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken          
//broken                DumFloat = float ( -VSPGeom().VSP_Surface(k).Surface_Nz(m,n) );
//broken                
//broken                fwrite(&(DumFloat), f_size, 1, WopFile);
//broken                
//broken             }
//broken             
//broken          }   
//broken          
//broken       }
//broken      
//broken   } 
//broken
//broken    // Loop over wings and surfaces
//broken
//broken    if ( ComponentGroupList_[c].GeometryHasWings() ) {
//broken       
//broken       i = ComponentGroupList_[c].WopWop().WingID();
//broken    
//broken       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {
//broken
//broken          // Thickness Geometry file
//broken          
//broken          WopFile = ComponentGroupList_[c].WopWop().ThicknessGeometryFileForWingSurface(j);
//broken          
//broken          // Surface for this blade
//broken          
//broken          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);
//broken
//broken          // Current time
//broken      
//broken          Time = float (CurrentNoiseTime_);
//broken       
//broken          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);             
//broken                          
//broken          // X node values
//broken    
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken                
//broken                x = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_x(m,n) + Translation[0] ) );
//broken
//broken                fwrite(&(x), f_size, 1, WopFile);
//broken           
//broken             }
//broken             
//broken          }
//broken          
//broken          // Y node values
//broken          
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken
//broken                y = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_y(m,n) + Translation[1] ) );
//broken     
//broken                fwrite(&(y), f_size, 1, WopFile);
//broken                
//broken             }
//broken             
//broken          }
//broken
//broken          // z node values
//broken          
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken   
//broken                z = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_z(m,n) + Translation[2] ) );
//broken      
//broken                fwrite(&(z), f_size, 1, WopFile);                
//broken                
//broken             }
//broken             
//broken          } 
//broken          
//broken          // X Normal values
//broken           
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken          
//broken                DumFloat = float ( -VSPGeom().VSP_Surface(k).Surface_Nx(m,n) );
//broken        
//broken                fwrite(&(DumFloat), f_size, 1, WopFile);
//broken                
//broken             }
//broken             
//broken          } 
//broken          
//broken          // Y Normal values
//broken              
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken          
//broken                DumFloat = float ( -VSPGeom().VSP_Surface(k).Surface_Ny(m,n) );
//broken           
//broken                fwrite(&(DumFloat), f_size, 1, WopFile);
//broken                
//broken             }
//broken             
//broken          }  
//broken          
//broken          // Z Normal values
//broken              
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken          
//broken                DumFloat = float ( -VSPGeom().VSP_Surface(k).Surface_Nz(m,n) );
//broken                
//broken                fwrite(&(DumFloat), f_size, 1, WopFile);
//broken                
//broken             }
//broken             
//broken          }   
//broken          
//broken       }
//broken       
//broken    } 
//broken
//broken    // Loop over body surfaces
//broken
//broken    if ( ComponentGroupList_[c].GeometryHasBodies() ) {
//broken       
//broken       i = ComponentGroupList_[c].WopWop().BodyID();
//broken    
//broken       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBodySurfaces() ; j++ ) {
//broken
//broken          // Thickness Geometry file
//broken          
//broken          WopFile = ComponentGroupList_[c].WopWop().ThicknessGeometryFileForBodySurface(j);
//broken          
//broken          // Surface for this blade
//broken          
//broken          k = ComponentGroupList_[c].WopWop().SurfaceForBody(j);
//broken
//broken          // Current time
//broken      
//broken          Time = float (CurrentNoiseTime_);
//broken       
//broken          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);             
//broken                          
//broken          // X node values
//broken    
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken
//broken                x = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_x(m,n) + Translation[0] ) );
//broken                
//broken                fwrite(&(x), f_size, 1, WopFile);
//broken           
//broken             }
//broken             
//broken          }
//broken          
//broken          // Y node values
//broken          
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken   
//broken                y = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_y(m,n) + Translation[1] ) );
//broken     
//broken                fwrite(&(y), f_size, 1, WopFile);
//broken                
//broken             }
//broken             
//broken          }
//broken
//broken          // z node values
//broken          
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken  
//broken                z = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).Surface_z(m,n) + Translation[2] ) );
//broken      
//broken                fwrite(&(z), f_size, 1, WopFile);                
//broken                
//broken             }
//broken             
//broken          } 
//broken          
//broken          // X Normal values
//broken           
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken          
//broken                DumFloat = float ( -VSPGeom().VSP_Surface(k).Surface_Nx(m,n) );
//broken        
//broken                fwrite(&(DumFloat), f_size, 1, WopFile);
//broken                
//broken             }
//broken             
//broken          } 
//broken          
//broken          // Y Normal values
//broken              
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken          
//broken                DumFloat = float ( -VSPGeom().VSP_Surface(k).Surface_Ny(m,n) );
//broken           
//broken                fwrite(&(DumFloat), f_size, 1, WopFile);
//broken                
//broken             }
//broken             
//broken          }  
//broken          
//broken          // Z Normal values
//broken              
//broken          for ( n = 1 ; n <= VSPGeom().VSP_Surface(k).Surface_NumJ() ; n++ ) {
//broken             
//broken             for ( m = 1 ; m <= VSPGeom().VSP_Surface(k).Surface_NumI() ; m++ ) {
//broken          
//broken                DumFloat = float ( -VSPGeom().VSP_Surface(k).Surface_Nz(m,n) );
//broken                
//broken                fwrite(&(DumFloat), f_size, 1, WopFile);
//broken                
//broken             }
//broken             
//broken          }   
//broken          
//broken       }
//broken       
//broken    } 
  
}

/*##############################################################################
#                                                                              #
#             VSP_SOLVER WriteOutPSUWopWopCompactGeometryHeader                #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCompactGeometryHeaderForGroup(int c)
{

//broken    char HeaderName[32], DumChar[2000];
//broken    char PatchGeometryName[2000];
//broken    int i, j, k, i_size, c_size, f_size, DumInt;
//broken    int NumberI, NumberJ, Length;
//broken    float Period;
//broken    FILE *WopFile;
//broken
//broken    // Sizeof int and float
//broken
//broken    i_size = sizeof(int);
//broken    c_size = sizeof(char);
//broken    f_size = sizeof(float);
//broken
//broken    // Loop over rotors and blades
//broken
//broken    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
//broken       
//broken       i = ComponentGroupList_[c].WopWop().RotorID();
//broken    
//broken       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ;  j++ ) {
//broken          
//broken          sprintf(PatchGeometryName,"%s.PSUWopWop.Loading.Geometry.Rotor.%d.Blade.%d.dat",FileName_,i,j);
//broken
//broken          // Thickness Geometry file
//broken          
//broken          WopFile = ComponentGroupList_[c].WopWop().OpenLoadingGeometryFileForBlade(j,PatchGeometryName);
//broken
//broken          // Geometry File Header
//broken          
//broken          DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
//broken          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
//broken          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
//broken          
//broken          // Force units... this is just a comment
//broken          
//broken          sprintf(HeaderName,"Pa\n");
//broken          
//broken          Length = strlen(HeaderName);
//broken          memset(&HeaderName[Length], ' ', 32 - Length);
//broken      
//broken          fwrite(HeaderName, c_size, 32, WopFile); // Units
//broken         
//broken          // Comment line
//broken          
//broken          sprintf(DumChar,"CompactGeometryFile\n");
//broken      
//broken          Length = strlen(DumChar);
//broken          memset(&DumChar[Length], ' ', 1024 - Length);
//broken              
//broken          fwrite(DumChar, c_size, 1024, WopFile); // Comment lLine
//broken
//broken          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
//broken          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones
//broken          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid
//broken
//broken          if ( SteadyStateNoise_ ) {
//broken             
//broken             DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Constant data
//broken
//broken          }
//broken          
//broken          else {
//broken             
//broken             fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic
//broken
//broken
//broken          }
//broken                       
//broken          DumInt =  1;        fwrite(&(DumInt), i_size, 1, WopFile); // 1 - node based normals
//broken          DumInt =  1;        fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
//broken          DumInt =  0;        fwrite(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
//broken          DumInt =  0;        fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
//broken          
//broken          // Write out surface inforamation for this blade
//broken          
//broken          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);
//broken          
//broken          sprintf(HeaderName,"Blade_%d_Rotor_%d",i,j);
//broken         
//broken          Length = strlen(HeaderName);
//broken          memset(&HeaderName[Length], ' ', 32 - Length);
//broken              
//broken          fwrite(HeaderName, c_size, 32, WopFile);
//broken
//broken          if ( !SteadyStateNoise_ ) {
//broken               
//broken             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
//broken
//broken                Period = float ( ComponentGroupList_[c].Period() );
//broken                
//broken                fwrite(&(Period), f_size, 1, WopFile); // Period
//broken                
//broken                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
//broken                
//broken             }
//broken             
//broken             else {
//broken                
//broken                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
//broken
//broken             }
//broken             
//broken          }
//broken                     
//broken          NumberI = 1;
//broken          NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
//broken                                 
//broken          fwrite(&(NumberI),            i_size, 1, WopFile); // Number in I
//broken          fwrite(&(NumberJ),            i_size, 1, WopFile); // Number in J
//broken
//broken       }
//broken       
//broken    }
//broken
//broken    // Loop over wings and surfaces
//broken 
//broken    if ( ComponentGroupList_[c].GeometryHasWings() ) {
//broken       
//broken       i = ComponentGroupList_[c].WopWop().WingID();
//broken    
//broken       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ;  j++ ) {
//broken          
//broken          sprintf(PatchGeometryName,"%s.PSUWopWop.Loading.Geometry.Wing.%d.Surface.%d.dat",FileName_,i,j);
//broken
//broken          // Thickness Geometry 
//broken
//broken          WopFile = ComponentGroupList_[c].WopWop().OpenLoadingGeometryFileForWingSurface(j,PatchGeometryName);
//broken
//broken          // Geometry File Header
//broken          
//broken          DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
//broken          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
//broken          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
//broken          
//broken          // Force units... this is just a comment
//broken          
//broken          sprintf(HeaderName,"Pa\n");
//broken          
//broken          Length = strlen(HeaderName);
//broken          memset(&HeaderName[Length], ' ', 32 - Length);
//broken      
//broken          fwrite(HeaderName, c_size, 32, WopFile); // Units
//broken         
//broken          // Comment line
//broken          
//broken          sprintf(DumChar,"CompactGeometryFile\n");
//broken      
//broken          Length = strlen(DumChar);
//broken          memset(&DumChar[Length], ' ', 1024 - Length);
//broken              
//broken          fwrite(DumChar, c_size, 1024, WopFile); // Comment lLine
//broken
//broken          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
//broken          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones
//broken          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid
//broken
//broken          if ( SteadyStateNoise_ ) {
//broken             
//broken             DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Constant data
//broken
//broken          }
//broken          
//broken          else {
//broken             
//broken             fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic
//broken
//broken
//broken          }
//broken                       
//broken          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - node based normals
//broken          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
//broken          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
//broken          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
//broken          
//broken          // Write out surface inforamation for this blade
//broken          
//broken          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);
//broken          
//broken          sprintf(HeaderName,"Blade_%d_Rotor_%d",i,j);
//broken         
//broken          Length = strlen(HeaderName);
//broken          memset(&HeaderName[Length], ' ', 32 - Length);
//broken              
//broken          fwrite(HeaderName, c_size, 32, WopFile);
//broken
//broken          if ( !SteadyStateNoise_ ) {
//broken               
//broken             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
//broken
//broken                Period = float ( WopWopLongestPeriod_ );
//broken                
//broken                fwrite(&(Period), f_size, 1, WopFile); // Period
//broken                
//broken                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
//broken                
//broken             }
//broken             
//broken             else {
//broken                
//broken                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
//broken
//broken             }
//broken             
//broken          }
//broken                     
//broken          NumberI = 1;
//broken          NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
//broken                                 
//broken          fwrite(&(NumberI),            i_size, 1, WopFile); // Number in I
//broken          fwrite(&(NumberJ),            i_size, 1, WopFile); // Number in J
//broken
//broken       }
//broken       
//broken    }


/*##############################################################################
#                                                                              #
#              VSP_SOLVER WriteOutPSUWopWopCompactGeometryData                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCompactGeometryDataForGroup(int c)
{
    
//broken    int i, j, k, m;
//broken    int i_size, c_size, f_size, NumberOfSpanStations;
//broken    float x, y, z, Time, Translation[3];
//broken    FILE *WopFile;
//broken
//broken    // Sizeof int and float
//broken
//broken    i_size = sizeof(int);
//broken    c_size = sizeof(char);
//broken    f_size = sizeof(float);
//broken
//broken    // Vehicle translation vector
//broken    
//broken    Translation[0] = Translation[1] = Translation[2] = 0.;
//broken    
//broken    if ( !SteadyStateNoise_ && WopWopPeriodicity_ == WOPWOP_APERIODIC ) {
//broken           
//broken       Translation[0] = float ( -FreeStreamVelocity_[0] * CurrentNoiseTime_ );
//broken       Translation[1] = float ( -FreeStreamVelocity_[1] * CurrentNoiseTime_ );
//broken       Translation[2] = float ( -FreeStreamVelocity_[2] * CurrentNoiseTime_ );
//broken       
//broken    }
//broken    
//broken    // Loop over rotors and blades
//broken
//broken    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
//broken       
//broken       i = ComponentGroupList_[c].WopWop().RotorID();
//broken    
//broken       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {
//broken
//broken          // Thickness Geometry file
//broken          
//broken          WopFile = ComponentGroupList_[c].WopWop().LoadingGeometryFileForBlade(j);
//broken
//broken          // Write out surface information for this blade
//broken          
//broken          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);
//broken 
//broken          NumberOfSpanStations = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
//broken     
//broken          // Current time
//broken      
//broken          Time = float ( CurrentNoiseTime_ );
//broken        
//broken          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);         
//broken
//broken          // X node values
//broken
//broken          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
//broken
//broken             x = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).xTE(m) + 0.75*(VSPGeom().VSP_Surface(k).xLE(m) - VSPGeom().VSP_Surface(k).xTE(m)) + Translation[0] ) );
//broken
//broken             fwrite(&(x), f_size, 1, WopFile);
//broken        
//broken          }
//broken
//broken          // Y node values
//broken          
//broken          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
//broken
//broken             y = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).yTE(m) + 0.75*(VSPGeom().VSP_Surface(k).yLE(m) - VSPGeom().VSP_Surface(k).yTE(m)) + Translation[1] ) );
//broken
//broken             fwrite(&(y), f_size, 1, WopFile);
//broken                     
//broken          }
//broken
//broken          // z node values
//broken          
//broken          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
//broken
//broken             z = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).zTE(m) + 0.75*(VSPGeom().VSP_Surface(k).zLE(m) - VSPGeom().VSP_Surface(k).zTE(m)) + Translation[2] ) );
//broken
//broken             fwrite(&(z), f_size, 1, WopFile);
//broken        
//broken          }
//broken
//broken          // X Normal values
//broken           
//broken          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
//broken
//broken             x = float ( VSPGeom().VSP_Surface(k).NxQC(m) );
//broken
//broken             fwrite(&(x), f_size, 1, WopFile);
//broken        
//broken          }
//broken          
//broken          // Y Normal values
//broken              
//broken          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
//broken
//broken             y = float ( VSPGeom().VSP_Surface(k).NyQC(m) );
//broken
//broken             fwrite(&(y), f_size, 1, WopFile);
//broken        
//broken          }
//broken          
//broken          // Z Normal values
//broken              
//broken          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
//broken
//broken             z = float ( VSPGeom().VSP_Surface(k).NzQC(m) );
//broken
//broken             fwrite(&(z), f_size, 1, WopFile);
//broken        
//broken          }
//broken          
//broken       }
//broken       
//broken    }  
//broken
//broken    // Loop over wings and surfaces
//broken  
//broken    if ( ComponentGroupList_[c].GeometryHasWings() ) {
//broken       
//broken       i = ComponentGroupList_[c].WopWop().WingID();
//broken    
//broken       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {
//broken
//broken          // Thickness Geometry file
//broken          
//broken          WopFile = ComponentGroupList_[c].WopWop().LoadingGeometryFileForWingSurface(j);
//broken
//broken          // Write out surface information for this blade
//broken          
//broken          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);
//broken     
//broken          NumberOfSpanStations = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
//broken     
//broken          // Current time
//broken      
//broken          Time = float ( CurrentNoiseTime_ );
//broken        
//broken          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);         
//broken
//broken          // X node values
//broken
//broken          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
//broken
//broken             x = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).xTE(m) + 0.75*(VSPGeom().VSP_Surface(k).xLE(m) - VSPGeom().VSP_Surface(k).xTE(m)) + Translation[0] ) );
//broken
//broken             fwrite(&(x), f_size, 1, WopFile);
//broken        
//broken          }
//broken
//broken          // Y node values
//broken          
//broken          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
//broken
//broken             y = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).yTE(m) + 0.75*(VSPGeom().VSP_Surface(k).yLE(m) - VSPGeom().VSP_Surface(k).yTE(m)) + Translation[1] ) );
//broken
//broken             fwrite(&(y), f_size, 1, WopFile);
//broken                     
//broken          }
//broken
//broken          // z node values
//broken          
//broken          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
//broken
//broken             z = float ( WopWopLengthConversion_ * ( VSPGeom().VSP_Surface(k).zTE(m) + 0.75*(VSPGeom().VSP_Surface(k).zLE(m) - VSPGeom().VSP_Surface(k).zTE(m)) + Translation[2] ) );
//broken
//broken             fwrite(&(z), f_size, 1, WopFile);
//broken        
//broken          }
//broken
//broken          // X Normal values
//broken           
//broken          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
//broken
//broken             x = float ( VSPGeom().VSP_Surface(k).NxQC(m) );
//broken
//broken             fwrite(&(x), f_size, 1, WopFile);
//broken        
//broken          }
//broken          
//broken          // Y Normal values
//broken              
//broken          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
//broken
//broken             y = float ( VSPGeom().VSP_Surface(k).NyQC(m) );
//broken
//broken             fwrite(&(y), f_size, 1, WopFile);
//broken        
//broken          }
//broken          
//broken          // Z Normal values
//broken              
//broken          for ( m = 1 ; m <= NumberOfSpanStations ; m++ ) {
//broken
//broken             z = float ( VSPGeom().VSP_Surface(k).NzQC(m) );
//broken
//broken             fwrite(&(z), f_size, 1, WopFile);
//broken        
//broken          }
//broken          
//broken       }
//broken       
//broken    }  
 
}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutPSUWopWopCompactLoadingHeader              #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCompactLoadingHeaderForGroup(int c)
{

//broken    char HeaderName[32], DumChar[2000];
//broken    char WopWopFileName[2000];
//broken    int i, j, k, i_size, c_size, f_size, DumInt;
//broken    int NumberI, NumberJ, Length;
//broken    float Period;
//broken    FILE *WopFile;
//broken       
//broken    // Sizeof int and float
//broken
//broken    i_size = sizeof(int);
//broken    c_size = sizeof(char);
//broken    f_size = sizeof(float);
//broken
//broken    // Loop over rotors and blades
//broken
//broken    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
//broken       
//broken       i = ComponentGroupList_[c].WopWop().RotorID();
//broken    
//broken       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ; j++ ) {
//broken
//broken          sprintf(WopWopFileName, "%s.PSUWopWop.Loading.Rotor.%d.Blade.%d.dat",FileName_,i,j);
//broken
//broken          // Thickness Geometry file
//broken          
//broken          WopFile = ComponentGroupList_[c].WopWop().OpenLoadingFileForBlade(j, WopWopFileName);
//broken
//broken          // Write out surface information for this blade
//broken          
//broken          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);
//broken
//broken          // Load File Header
//broken          
//broken          DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
//broken          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
//broken          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
//broken      
//broken          fwrite(DumChar, c_size, 1024, WopFile); // Comment Line   
//broken              
//broken          DumInt = 2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 indicates this is a loading file
//broken          DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones
//broken          DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid
//broken          
//broken          if ( SteadyStateNoise_ ) {
//broken             
//broken             DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // Constant data
//broken             
//broken          }
//broken          
//broken          else {
//broken             
//broken             fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic
//broken             
//broken          }
//broken                
//broken          DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - node centered normals
//broken          DumInt =   2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - data is loading vector
//broken          
//broken          if ( SteadyStateNoise_ ) {
//broken             
//broken             DumInt = 2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - Rotating frame
//broken        
//broken          }           
//broken                      
//broken          else {      
//broken                      
//broken             DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - Ground reference fixed frame
//broken             
//broken          }
//broken                    
//broken          DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
//broken          DumInt =   0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
//broken          DumInt =   0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
//broken          DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones with data
//broken          DumInt =  -1; fwrite(&(DumInt), i_size, 1, WopFile); //negative since compact loading
//broken
//broken          sprintf(HeaderName,"Blade_%d_Rotor_%d",i,j);
//broken          
//broken          Length = strlen(HeaderName);
//broken          memset(&HeaderName[Length], ' ', 32 - Length);
//broken                  
//broken          fwrite(HeaderName, c_size, 32, WopFile); // Patch name
//broken   
//broken          if ( !SteadyStateNoise_ ) {
//broken               
//broken             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
//broken
//broken                ComponentGroupList_[c].Period();
//broken                
//broken                fwrite(&(Period), f_size, 1, WopFile); // Period
//broken                
//broken                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
//broken                
//broken             }
//broken             
//broken             else {
//broken                
//broken                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
//broken
//broken             }
//broken             
//broken          }
//broken
//broken          NumberI = 1;
//broken          NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
//broken         
//broken          fwrite(&(NumberI), i_size, 1, WopFile); // NI
//broken          fwrite(&(NumberJ), i_size, 1, WopFile); // NJ = NumberOfSpanStations
//broken
//broken       }
//broken       
//broken    }
//broken
//broken    // Loop over wings and surfaces
//broken
//broken    if ( ComponentGroupList_[c].GeometryHasWings() ) {
//broken       
//broken       i = ComponentGroupList_[c].WopWop().WingID();
//broken    
//broken       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ; j++ ) {
//broken
//broken          sprintf(WopWopFileName, "%s.PSUWopWop.Loading.Wing.%d.Surface.%d.dat",FileName_,i,j);
//broken
//broken          // Thickness Geometry file
//broken          
//broken          WopFile = ComponentGroupList_[c].WopWop().OpenLoadingFileForWingSurface(j, WopWopFileName);
//broken
//broken          // Write out surface information for this blade
//broken          
//broken          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);
//broken
//broken          // Load File Header
//broken          
//broken          DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
//broken          DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
//broken          DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
//broken      
//broken          fwrite(DumChar, c_size, 1024, WopFile); // Comment Line   
//broken              
//broken          DumInt = 2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 indicates this is a loading file
//broken          DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones
//broken          DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is an structured grid
//broken          
//broken          if ( SteadyStateNoise_ ) {
//broken             
//broken             DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // Constant data
//broken             
//broken          }
//broken          
//broken          else {
//broken             
//broken             fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic
//broken             
//broken          }
//broken                
//broken          DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - node centered normals
//broken          DumInt =   2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - data is loading vector
//broken          
//broken          if ( SteadyStateNoise_ ) {
//broken             
//broken             DumInt = 2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - Rotating frame
//broken        
//broken          }           
//broken                      
//broken          else {      
//broken                      
//broken             DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - Ground reference fixed frame
//broken             
//broken          }
//broken                    
//broken          DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
//broken          DumInt =   0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
//broken          DumInt =   0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
//broken          DumInt =   1; fwrite(&(DumInt), i_size, 1, WopFile); // Number of zones with data
//broken          DumInt =  -1; fwrite(&(DumInt), i_size, 1, WopFile); //negative since compact loading
//broken
//broken          sprintf(HeaderName,"Wing_%d_Surface_%d",i,j);
//broken          
//broken          Length = strlen(HeaderName);
//broken          memset(&HeaderName[Length], ' ', 32 - Length);
//broken                  
//broken          fwrite(HeaderName, c_size, 32, WopFile); // Patch name
//broken   
//broken          if ( !SteadyStateNoise_ ) {
//broken               
//broken             if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
//broken
//broken                Period = float( WopWopLongestPeriod_ );
//broken                
//broken                fwrite(&(Period), f_size, 1, WopFile); // Period
//broken                
//broken                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
//broken                
//broken             }
//broken             
//broken             else {
//broken                
//broken                fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
//broken
//broken             }
//broken             
//broken          }
//broken
//broken          NumberI = 1;
//broken          NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
//broken         
//broken          fwrite(&(NumberI), i_size, 1, WopFile); // NI
//broken          fwrite(&(NumberJ), i_size, 1, WopFile); // NJ = NumberOfSpanStations
//broken
//broken       }
//broken       
//broken    }

}

/*##############################################################################
#                                                                              #
#           VSP_SOLVER WriteOutPSUWopWopCompactLoadingData                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopCompactLoadingDataForGroup(int c)
{
    
//broken    int i, j, k, m, NumberOfStations;
//broken    int i_size, c_size, f_size;
//broken    float DumFloat, DynP, Time;
//broken    FILE *WopFile;
//broken
//broken    // Sizeof int and float
//broken
//broken    i_size = sizeof(int);
//broken    c_size = sizeof(char);
//broken    f_size = sizeof(float);
//broken    
//broken    // Forces
//broken    
//broken    DynP = float( 0.5 * Density_ * Vinf_ * Vinf_ * WopWopPressureConversion_ );
//broken
//broken    // Loop over rotors and blades
//broken
//broken    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
//broken       
//broken       i = ComponentGroupList_[c].WopWop().RotorID();
//broken     
//broken       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfBlades() ;  j++ ) {
//broken  
//broken          // Loading file
//broken          
//broken          WopFile = ComponentGroupList_[c].WopWop().LoadingFileForBlade(j);
//broken
//broken          // Write out surface inforamation for this blade
//broken          
//broken          k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);
//broken          
//broken          // Current time
//broken      
//broken          Time = float( CurrentNoiseTime_ );
//broken     
//broken          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);
//broken
//broken          NumberOfStations = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
//broken              
//broken          // X force per length
//broken              
//broken          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
//broken             
//broken             DumFloat = float( DynP * SpanLoadData(k).Span_Cx(m) * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );
//broken
//broken             fwrite(&(DumFloat), f_size, 1, WopFile);
//broken    
//broken          }
//broken          
//broken          // Y force per length
//broken              
//broken          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
//broken             
//broken             DumFloat = float( DynP * SpanLoadData(k).Span_Cy(m) * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );
//broken
//broken             fwrite(&(DumFloat), f_size, 1, WopFile);
//broken         
//broken          }
//broken          
//broken          // Z force per length
//broken              
//broken          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
//broken             
//broken             DumFloat = float( DynP * SpanLoadData(k).Span_Cz(m) * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );
//broken
//broken             fwrite(&(DumFloat), f_size, 1, WopFile);
//broken    
//broken          }                    
//broken 
//broken       }
//broken                 
//broken    }
//broken
//broken    // Loop over wings and surfaces
//broken
//broken    if ( ComponentGroupList_[c].GeometryHasWings() ) {
//broken       
//broken       i = ComponentGroupList_[c].WopWop().WingID();
//broken     
//broken       for ( j = 1 ; j <= ComponentGroupList_[c].WopWop().NumberOfWingSurfaces() ;  j++ ) {
//broken  
//broken          // Loading file
//broken          
//broken          WopFile = ComponentGroupList_[c].WopWop().LoadingFileForWingSurface(j);
//broken
//broken          // Write out surface inforamation for this blade
//broken          
//broken          k = ComponentGroupList_[c].WopWop().SurfaceForWing(j);
//broken          
//broken          // Current time
//broken      
//broken          Time = float( CurrentNoiseTime_ );
//broken     
//broken          if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);
//broken
//broken          NumberOfStations = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
//broken              
//broken          // X force per length
//broken              
//broken          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
//broken             
//broken             DumFloat = float( DynP * SpanLoadData(k).Span_Cx(m) * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );
//broken
//broken             fwrite(&(DumFloat), f_size, 1, WopFile);
//broken    
//broken          }
//broken          
//broken          // Y force per length
//broken              
//broken          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
//broken             
//broken             DumFloat = float( DynP * SpanLoadData(k).Span_Cy(m) * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );
//broken
//broken             fwrite(&(DumFloat), f_size, 1, WopFile);
//broken         
//broken          }
//broken          
//broken          // Z force per length
//broken              
//broken          for ( m = 1 ; m <= NumberOfStations ; m++ ) {
//broken             
//broken             DumFloat = float( DynP * SpanLoadData(k).Span_Cz(m) * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );
//broken
//broken             fwrite(&(DumFloat), f_size, 1, WopFile);
//broken    
//broken          }                    
//broken      
//broken       }
//broken                 
//broken    }

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutPSUWopWopPeggNamelist                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopPeggNamelist(void)
{

//broken    int i, k, i_size, c_size, f_size;
//broken    double x1, y1, z1, x2, y2, z2;
//broken    double BladeArea, BladeRadius, Thrust, BladeCL, RotationSpeed;
//broken       
//broken    // Sizeof int and float
//broken
//broken    i_size = sizeof(int);
//broken    c_size = sizeof(char);
//broken    f_size = sizeof(float);
//broken            
//broken    // Calculate total blade area
//broken        
//broken    BladeArea = 0.;
//broken
//broken    for ( k = 1 ; k <= VSPGeom().NumberOfSurfaces() ; k++ ) {
//broken         
//broken       // Wing   
//broken            
//broken       if ( VSPGeom().VSP_Surface(k).SurfaceType() == DEGEN_WING_SURFACE ) {
//broken      
//broken          for ( i = 1 ; i < VSPGeom().VSP_Surface(k).NumberOfSpanStations() ; i++ ) {
//broken
//broken             BladeArea += SpanLoadData(k).Span_Area(i);
//broken
//broken          }
//broken          
//broken       }
//broken       
//broken    }
//broken
//broken    // Calculate blade radius
//broken    
//broken    BladeRadius = 0.;
//broken    
//broken    for ( k = 1 ; k <= VSPGeom().NumberOfSurfaces() ; k++ ) {
//broken         
//broken       // Wing   
//broken            
//broken       if ( VSPGeom().VSP_Surface(k).SurfaceType() == DEGEN_WING_SURFACE ) {
//broken          
//broken          i = 1;
//broken
//broken          x1 = VSPGeom().VSP_Surface(k).xTE(i) + 0.75*(VSPGeom().VSP_Surface(k).xLE(i) - VSPGeom().VSP_Surface(k).xTE(i));
//broken          y1 = VSPGeom().VSP_Surface(k).yTE(i) + 0.75*(VSPGeom().VSP_Surface(k).yLE(i) - VSPGeom().VSP_Surface(k).yTE(i));
//broken          z1 = VSPGeom().VSP_Surface(k).zTE(i) + 0.75*(VSPGeom().VSP_Surface(k).zLE(i) - VSPGeom().VSP_Surface(k).zTE(i));
//broken          
//broken          i = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
//broken
//broken          x2 = VSPGeom().VSP_Surface(k).xTE(i) + 0.75*(VSPGeom().VSP_Surface(k).xLE(i) - VSPGeom().VSP_Surface(k).xTE(i));
//broken          y2 = VSPGeom().VSP_Surface(k).yTE(i) + 0.75*(VSPGeom().VSP_Surface(k).yLE(i) - VSPGeom().VSP_Surface(k).yTE(i));
//broken          z2 = VSPGeom().VSP_Surface(k).zTE(i) + 0.75*(VSPGeom().VSP_Surface(k).zLE(i) - VSPGeom().VSP_Surface(k).zTE(i));
//broken          
//broken          BladeRadius = sqrt( SQR(x2-x1) + SQR(y2-y1) + SQR(z2-z1) );
//broken
//broken       }
//broken       
//broken    }
//broken    
//broken    // Calculate averge lift coefficient
//broken  
//broken    RotationSpeed = BladeRPM_ * 2. * PI / 60.;
//broken
//broken    Thrust = -0.5*Density_*Sref_*SQR(Vinf_)*CFix_[0];
//broken    
//broken    BladeCL = 6.*Thrust/ (Density_ * BladeArea * SQR(RotationSpeed));
//broken
//broken    // PeggIn namelist
//broken        
//broken    fprintf(PSUWopWopNameListFile_,"      &PeggIn\n");
//broken
//broken    fprintf(PSUWopWopNameListFile_,"         TotalBladeAreaFlag = \'USERVALUE\' \n");
//broken    fprintf(PSUWopWopNameListFile_,"         TotalBladeArea     = %lf \n",BladeArea*WopWopAreaConversion_);
//broken                                                               
//broken    fprintf(PSUWopWopNameListFile_,"         BladeRadiusFlag    = \'USERVALUE\' \n");
//broken    fprintf(PSUWopWopNameListFile_,"         BladeRadius        = %lf \n",BladeRadius*WopWopLengthConversion_);
//broken                                                               
//broken    fprintf(PSUWopWopNameListFile_,"         RotSpeedFlag       = \'USERVALUE\' \n");
//broken    fprintf(PSUWopWopNameListFile_,"         RotSpeed           = %lf \n",RotationSpeed);
//broken                                                               
//broken    fprintf(PSUWopWopNameListFile_,"         CLBarFlag          = \'USERVALUE\' \n");
//broken    fprintf(PSUWopWopNameListFile_,"         CLBar              = %lf \n",BladeCL);
//broken                                                               
//broken    fprintf(PSUWopWopNameListFile_,"         TotalThrustFlag    = \'USERVALUE\' \n");
//broken    fprintf(PSUWopWopNameListFile_,"         TotalThrust        = %lf \n",Thrust*WopWopForceConversion_);
//broken                                                               
//broken    fprintf(PSUWopWopNameListFile_,"         HubAxisFlag        = \'USERVALUE\' \n");
//broken    fprintf(PSUWopWopNameListFile_,"         HubAxis            = %f, %f, %f \n",-1., 0., 0.);
//broken
//broken    fprintf(PSUWopWopNameListFile_,"      / \n");        

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutPSUWopWopBPMHeader                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopBPMHeaderForGroup(int c)
{

//broken    char WopWopFileName[2000];
//broken    int i, k, i_size, c_size, f_size, DumInt, NumberJ;
//broken    FILE *WopFile;
//broken       
//broken    // Sizeof int and float
//broken
//broken    i_size = sizeof(int);
//broken    c_size = sizeof(char);
//broken    f_size = sizeof(float);
//broken
//broken    // Loop over rotors and blades
//broken
//broken    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
//broken       
//broken       i = ComponentGroupList_[c].WopWop().RotorID();
//broken
//broken       sprintf(WopWopFileName,"%s.PSUWopWop.BPM.Rotor.%d.dat",FileName_,i);
//broken
//broken       // BPM file
//broken       
//broken       WopFile = ComponentGroupList_[c].WopWop().OpenBPMFile(WopWopFileName);
//broken
//broken       // Write out surface inforamation for this blade
//broken       
//broken       k = ComponentGroupList_[c].WopWop().SurfaceForBlade(1);
//broken    
//broken       // BPM File Header
//broken       
//broken       NumberJ = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
//broken    
//broken       DumInt = 42; fwrite(&(DumInt ), i_size, 1, WopFile); // Magic Number
//broken                    fwrite(&(NumberJ), i_size, 1, WopFile); // Number of sections definining the blade
//broken       DumInt =  0; fwrite(&(DumInt ), i_size, 1, WopFile); // Non-uniform blade sections
//broken       DumInt =  1; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section chord data provided
//broken       DumInt =  0; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section length data not provided
//broken       DumInt =  1; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section TE thickness provided
//broken       DumInt =  1; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section TE flow angle provided
//broken       DumInt =  1; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section effective AoA provided
//broken       DumInt =  1; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section tip lift curve slope provided
//broken       DumInt =  1; fwrite(&(DumInt ), i_size, 1, WopFile); // Blade section free stream velocity provided
//broken   
//broken       if ( !TimeAccurate_ ) {
//broken          
//broken          DumInt = 1; fwrite(&(DumInt), i_size, 1, WopFile); // constant data
//broken                   
//broken       }           
//broken                   
//broken       else {      
//broken                   
//broken          DumInt = 3; fwrite(&(DumInt), i_size, 1, WopFile); // aperiodic data
//broken          
//broken       }
//broken
//broken    }

}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER WriteOutPSUWopWopBPMData                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopBPMDataForGroup(int c)
{

//broken    int i, j, k, m, i_size, c_size, f_size;
//broken    int NumberOfStations;
//broken    float DumFloat; 
//broken    FILE *WopFile;
//broken       
//broken    // Sizeof int and float
//broken
//broken    i_size = sizeof(int);
//broken    c_size = sizeof(char);
//broken    f_size = sizeof(float);
//broken          
//broken    // Loop over rotors
//broken
//broken    if ( ComponentGroupList_[c].GeometryIsARotor() ) {
//broken       
//broken       i = ComponentGroupList_[c].WopWop().RotorID();
//broken    
//broken       // WopWop and BPM assume all the blades are the same
//broken       
//broken       j = 1;
//broken
//broken       // BPM file
//broken       
//broken       WopFile = ComponentGroupList_[c].WopWop().BPMFile();
//broken
//broken       // Write out surface inforamation for this blade
//broken       
//broken       k = ComponentGroupList_[c].WopWop().SurfaceForBlade(j);
//broken   
//broken       NumberOfStations = VSPGeom().VSP_Surface(k).NumberOfSpanStations();
//broken
//broken       for ( m = 1 ; m <= NumberOfStations ; m++ ) {
//broken
//broken          // Chord
//broken          
//broken          DumFloat = float( VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );
//broken
//broken          fwrite(&(DumFloat), f_size, 1, WopFile);
//broken   
//broken          // Span
//broken          
//broken          DumFloat = float( SpanLoadData(k).Span_Area(m) / VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );
//broken
//broken// djk... let psu-wopwop calculate this
//broken          
//broken     //     fwrite(&(DumFloat), f_size, 1, WopFile);
//broken          
//broken          // Blade section TE thickness
//broken          
//broken          DumFloat = float( 0.0005 * VSPGeom().VSP_Surface(k).LocalChord(m) * WopWopLengthConversion_ );
//broken                
//broken          fwrite(&(DumFloat), f_size, 1, WopFile);
//broken          
//broken          // Blade section TE flow angle, radians
//broken          
//broken          DumFloat = 0.244;
//broken            
//broken          fwrite(&(DumFloat), f_size, 1, WopFile);             
//broken   
//broken       }
//broken    
//broken       for ( m = 1 ; m <= NumberOfStations ; m++ ) {
//broken          
//broken          // Blade section effective AoA
//broken          
//broken          DumFloat = 0.0;
//broken           
//broken          fwrite(&(DumFloat), f_size, 1, WopFile);        
//broken   
//broken          // Blade section tip lift curve slope
//broken          
//broken          DumFloat = 1.0;
//broken                      
//broken          fwrite(&(DumFloat), f_size, 1, WopFile); 
//broken          
//broken          // Blade section free stream speed
//broken          
//broken          DumFloat = float( Vinf_ * WopWopLengthConversion_ );
//broken            
//broken          fwrite(&(DumFloat), f_size, 1, WopFile);     
//broken
//broken       }
//broken
//broken    }

}

/*##############################################################################
#                                                                              #
#             VSP_SOLVER WriteOutPSUSWopWopLoadingGeometryHeader               #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutPSUWopWopLoadingGeometryHeaderForGroup(int c)
{

//broken    char HeaderName[32], DumChar[2000];
//broken    char PatchGeometryName[2000];
//broken    int i, j, Node, Level, i_size, c_size, f_size, DumInt;
//broken    int NumberOfNodes, NumberOfLoops, Length;
//broken    float Period;
//broken    FILE *WopFile;
//broken
//broken    // Sizeof int and float
//broken
//broken    i_size = sizeof(int);
//broken    c_size = sizeof(char);
//broken    f_size = sizeof(float);
//broken    
//broken    sprintf(PatchGeometryName, "%s.PSUWopWop.Loading.Geometry.dat",FileName_);
//broken    
//broken    // Geometry file
//broken    
//broken    if ( (WopFile = fopen(PatchGeometryName, "wb")) == NULL ) {
//broken
//broken       printf("Could not open the PSUWopWop Case File output! \n");
//broken
//broken       exit(1);
//broken
//broken    }    
//broken    
//broken    // Geometry File Header
//broken    
//broken    DumInt = 42; fwrite(&(DumInt), i_size, 1, WopFile); // Magic Number
//broken    DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // Version number
//broken    DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // Version number, line 2
//broken  
//broken    // Force units... this is just a comment
//broken    
//broken    sprintf(HeaderName,"Pascals\n");
//broken    
//broken    Length = strlen(HeaderName);
//broken    memset(&HeaderName[Length], ' ', 32 - Length);
//broken
//broken    fwrite(HeaderName, c_size, 32, WopFile); // Units
//broken   
//broken    // Comment line
//broken    
//broken    sprintf(DumChar,"VSPAERO Created PSUWopWop Loading Geometry File");
//broken
//broken    Length = strlen(DumChar);
//broken    memset(&DumChar[Length], ' ', 1024 - Length);
//broken        
//broken    fwrite(DumChar, c_size, 1024, WopFile); // Comment lLine
//broken        
//broken    DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 indicates this is a geometry file
//broken    DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1, single, zone for now
//broken    DumInt =  2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 indicates this is an unstructured grid
//broken    
//broken    if ( SteadyStateNoise_ ) {
//broken    
//broken       DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - Constant data
//broken       
//broken    }
//broken    
//broken    else {
//broken       
//broken       fwrite(&(WopWopPeriodicity_), i_size, 1, WopFile); // Define input type... constant, periodic, or aperiodic
//broken       
//broken    }       
//broken       
//broken    DumInt =  2; fwrite(&(DumInt), i_size, 1, WopFile); // 2 - faced centered normals
//broken    DumInt =  1; fwrite(&(DumInt), i_size, 1, WopFile); // 1 - single precision data
//broken    DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // 0 - no iblank data
//broken    DumInt =  0; fwrite(&(DumInt), i_size, 1, WopFile); // 0, reserved 
//broken    
//broken    sprintf(HeaderName,"Zone_1_Geom_Uns");
//broken    
//broken    Length = strlen(HeaderName);
//broken    memset(&HeaderName[Length], ' ', 32 - Length);
//broken        
//broken    fwrite(HeaderName, c_size, 32, WopFile);
//broken    
//broken    Level = 0;
//broken    
//broken    NumberOfNodes = VSPGeom().Grid(Level).NumberOfNodes();
//broken    NumberOfLoops = VSPGeom().Grid(Level).NumberOfLoops();
//broken    
//broken    if ( !SteadyStateNoise_ ) {
//broken         
//broken       if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {
//broken
//broken          Period = float( WopWopLongestPeriod_ );
//broken          
//broken          fwrite(&(Period), f_size, 1, WopFile); // Period
//broken          
//broken          fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
//broken          
//broken       }
//broken       
//broken       else {
//broken          
//broken          fwrite(&(NumberOfNoiseTimeSteps_), i_size, 1, WopFile); // Number of time steps    
//broken
//broken       }
//broken       
//broken    }
//broken    
//broken    fwrite(&(NumberOfNodes),      i_size, 1, WopFile); // Number of nodes
//broken    fwrite(&(NumberOfLoops),      i_size, 1, WopFile); // Number of loops
//broken    
//broken    // Write out loop connnectivity
//broken
//broken    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {
//broken       
//broken       DumInt = VSPGeom().Grid(Level).LoopList(i).NumberOfNodes();
//broken  
//broken       fwrite(&(DumInt), i_size, 1, WopFile);
//broken       
//broken       for ( j = 1 ; j <= VSPGeom().Grid(Level).LoopList(i).NumberOfNodes() ; j++ ) {
//broken          
//broken          Node = VSPGeom().Grid(Level).LoopList(i).Node(j);
//broken
//broken          fwrite(&Node, i_size, 1, WopFile);
//broken          
//broken       }
//broken       
//broken    }

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
    
    Time = float( CurrentNoiseTime_ );
 
    if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);

    Level = 0;
       
    // X node values
       
    for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

       x = float( WopWopLengthConversion_ * ( VSPGeom().Grid(Level).NodeList(j).x() + Translation[0] ) );
       
       fwrite(&(x), f_size, 1, WopFile);

    }
    
    // Y node values
    
    for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

       y = float( WopWopLengthConversion_ * ( VSPGeom().Grid(Level).NodeList(j).y() + Translation[1] ) );

       fwrite(&(y), f_size, 1, WopFile);
  
    }
    
    // Z node values
    
    for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

       z = float( WopWopLengthConversion_ * ( VSPGeom().Grid(Level).NodeList(j).z() + Translation[2] ) );

       fwrite(&(z), f_size, 1, WopFile);         
  
    }        

    // X Normal values
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       DumFloat = float( WopWopAreaConversion_ * ( VSPGeom().Grid(Level).LoopList(i).Nx() * VSPGeom().Grid(Level).LoopList(i).Area()) );

       fwrite(&DumFloat, f_size, 1, WopFile);

    }    
    
    // Y Normal values
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       DumFloat = float( WopWopAreaConversion_ * ( VSPGeom().Grid(Level).LoopList(i).Ny() * VSPGeom().Grid(Level).LoopList(i).Area()) );

       fwrite(&DumFloat, f_size, 1, WopFile);

    }    
    
    // Z Normal values
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       DumFloat = float( WopWopAreaConversion_ * ( VSPGeom().Grid(Level).LoopList(i).Nz() * VSPGeom().Grid(Level).LoopList(i).Area()) );

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

    fwrite(DumChar, c_size, 1024, WopFile); // Comment lLine
        
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
            
    fwrite(HeaderName, c_size, 32, WopFile); // Patch name
 
    Level = 0;
    
    NumberOfLoops = VSPGeom().Grid(Level).NumberOfLoops();
     
    if ( !SteadyStateNoise_ ) {
         
       if ( WopWopPeriodicity_ == WOPWOP_PERIODIC ) {

          Period = float( WopWopLongestPeriod_ );
          
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

    Time = float( CurrentNoiseTime_ );

    if ( !SteadyStateNoise_ ) fwrite(&Time, f_size, 1, WopFile);

    // Forces
    
    DynP = float( 0.5 * Density_ * Vinf_ * Vinf_ * WopWopPressureConversion_ );

    Level = 0;

    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       Fx = float( -DynP * VSPGeom().Grid(Level).LoopList(i).dCp() * VSPGeom().Grid(Level).LoopList(i).Nx() );

       fwrite(&Fx, f_size, 1, WopFile);
     
    }  
     
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       Fy = float( -DynP * VSPGeom().Grid(Level).LoopList(i).dCp() * VSPGeom().Grid(Level).LoopList(i).Ny() );

       fwrite(&Fy, f_size, 1, WopFile);
    
    }   
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {

       Fz = float( -DynP * VSPGeom().Grid(Level).LoopList(i).dCp() * VSPGeom().Grid(Level).LoopList(i).Nz() );

       fwrite(&Fz, f_size, 1, WopFile);
    
    }    

}


#include "END_NAME_SPACE.H"
