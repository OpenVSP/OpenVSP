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

       PRINTF ("Could not open the PSUWopWop file: %s for output! \n",FileName);

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

       PRINTF ("Could not open the PSUWopWop file: %s for output! \n",FileName);

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

       PRINTF ("Could not open the PSUWopWop file: %s for output! \n",FileName);

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

       PRINTF ("Could not open the PSUWopWop file: %s for output! \n",FileName);

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

#include "END_NAME_SPACE.H"
