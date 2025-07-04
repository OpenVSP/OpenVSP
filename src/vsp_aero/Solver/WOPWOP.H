//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef WOPWOP_H
#define WOPWOP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"

#include "START_NAME_SPACE.H"

#define WOPWOP_CONSTANT  1
#define WOPWOP_PERIODIC  2
#define WOPWOP_APERIODIC 3

// Definition of the WOPWOP class

class WOPWOP {

private:

    // Rotor information
    
    int RotorID_;
    int NumberOfBlades_;
    int *SurfaceForBlade_;
    int NumberOfBladesSections_;

    FILE **RotorLoadingGeometryFile_;
    FILE **RotorLoadingFile_;
    FILE **RotorThicknessGeometryFile_;
    FILE *BPMFile_;
    
    // Wing information

    int WingID_;
    int NumberOfWingSurfaces_;
    int *SurfaceForWing_;
    int NumberOfWingSections_;
    
    FILE **WingLoadingGeometryFile_;
    FILE **WingLoadingFile_;
    FILE **WingThicknessGeometryFile_;
       
    // Body information
    
    int BodyID_;
    int NumberOfBodySurfaces_;
    int *SurfaceForBody_;
    int NumberOfBodySections_;
    
    FILE **BodyThicknessGeometryFile_;

    
    // PSU WOP WOP Coupling
    
    // Set psuwopwop periodicity type
    // WOPWOP_CONSTANT  1
    // WOPWOP_PERIODIC  2
    // WOPWOP_APERIODIC 3
     
    int &WopWopPeriodicity(void) { return WopWopPeriodicity_; };
    
    int &WopWopWriteOutADBFile(void) { return WopWopWriteOutADBFile_; };    
    
    double &WopWopUserStartTime(void) { return WopWopUserStartTime_; };
    double &WopWopUserFinishTime(void) { return WopWopUserFinishTime_; };
        
    int NoiseAnalysis_;
    int NoiseTimeRefinement_;
    int NoiseTime_;
    int NumberOfNoiseTimeSteps_;
    int SteadyStateNoise_;
    int NoiseInterpolation_;
    
    double NoiseTimeStep_;
    double NoiseTimeShift_;
    double CurrentNoiseTime_;
     
    // File IO
    
    FILE *OpenLoadingGeometryFile(int i, char *FileName, FILE **File);
    FILE *OpenLoadingFile(int i, char *FileName, FILE **File);
    FILE *OpenThicknessGeometryFile(int i, char *FileName, FILE **File);


    int WopWopEnglishUnits_;
    int WopWopFlyBy_;
    int WopWopPeriodicity_;
    int WopWopNumberOfRotors_;
    int WopWopNumberOfWings_;
    int WopWopNumberOfBodies_;
    int WopWopNumberOfTotalBlades_;
    int WopWopNumberOfTotalWingSurfaces_;
    int WopWopNumberOfTtoalBodySurfaces_;
    int WopWopNumberOfTimeSteps_;
    int WopWopComponentGroupStart_;
    int WopWopComponentGroupEnd_;
    int WopWopLongestPeriodGroup_;
    int WopWopWriteOutADBFile_;
    
    double WopWopDensityConversion_;
    double WopWopLengthConversion_;
    double WopWopAreaConversion_;
    double WopWopPressureConversion_;     
    double WopWopForceConversion_;
    double WopWopdTau_;
    double WopWopOmegaMin_;
    double WopWopOmegaMax_;
    double WopWopLongestPeriod_;
    double WopWopShortestPeriod_;
    double WopWopObserverTime_;
    double WopWopUserStartTime_;
    double WopWopUserFinishTime_;
    
    FILE *PSUWopWopNameListFile_;
    
    // Main driver routines
    
    void SetupPSUWopWopData(void);
    void WriteOutPSUWopWopFileHeadersForGroup(int c);
    void WriteOutPSUWopWopUnsteadyDataForGroup(int c);

    // Namelist files
    
    void WriteOutPSUWopWopCaseAndNameListFiles(void);
    void WriteOutPSUWopWopCaseAndNameListFilesForFlyBy(void);

    void WriteOutPSUWopWopCaseAndNameListFilesForFootPrint(void);

    // Compact Geometry
    
    void WriteOutPSUWopWopCompactGeometryHeaderForGroup(int c);
    void WriteOutPSUWopWopCompactGeometryDataForGroup(int c);
    
    // Compact Loading
    
    void WriteOutPSUWopWopCompactLoadingHeaderForGroup(int c);
    void WriteOutPSUWopWopCompactLoadingDataForGroup(int c);

    // Thickness Geometry

    void WriteOutPSUWopWopThicknessGeometryHeaderForGroup(int c);    
    void WriteOutPSUWopWopThicknessGeometryDataForGroup(int c); 
    
    // Full Geometry

    void WriteOutPSUWopWopLoadingGeometryHeaderForGroup(int c);        
    void WriteOutPSUWopWopLoadingGeometryDataForGroup(int c);   
    
    // Full Loading
    
    void WriteOutPSUWopWopLoadingHeaderForGroup(int c);        
    void WriteOutPSUWopWopLoadingDataForGroup(int c);    
    
    // Pegg and BPM data

    void WriteOutPSUWopWopPeggNamelist(void);
    void WriteOutPSUWopWopBPMHeaderForGroup(int c);    
    void WriteOutPSUWopWopBPMDataForGroup(int c);
           
public:

    // Constructor, Destructor, Copy

    WOPWOP(void);
   ~WOPWOP(void);
    WOPWOP(const WOPWOP &WopWopRotor);
    WOPWOP& operator=(const WOPWOP &WopWopRotor);

    // Size the lists
    
    void SizeBladeList(int NumberOfBlades);
    void SizeWingSurfaceList(int NumberOfSurfaces);
    void SizeBodySurfaceList(int NumberOfSurfaces);
    
    // Rotor Info
    
    int &RotorID(void) { return RotorID_; };
    int NumberOfBlades(void) { return NumberOfBlades_; };
    int &SurfaceForBlade(int i) { return SurfaceForBlade_[i]; };
    int &NumberOfBladesSections(void) { return NumberOfBladesSections_; };    
    
    // Wing Info
    
    int &WingID(void) { return WingID_; };
    int &NumberOfWingSurfaces(void) { return NumberOfWingSurfaces_; };
    int &SurfaceForWing(int i) { return SurfaceForWing_[i]; };
    int &NumberOfWingSections(void) { return NumberOfWingSections_; };    
    
    // Body Info
    
    int &BodyID(void) { return BodyID_; };
    int &NumberOfBodySurfaces(void) { return NumberOfBodySurfaces_; };
    int &SurfaceForBody(int i) { return SurfaceForBody_[i]; };
    int &NumberOfBodySections(void) { return NumberOfBodySections_; };

    // Rotor files
    
    FILE *OpenLoadingGeometryFileForBlade(int i, char *FileName) { return OpenLoadingGeometryFile(i, FileName, RotorLoadingGeometryFile_); };
    FILE *OpenLoadingFileForBlade(int i, char *FileName) { return OpenLoadingFile(i, FileName, RotorLoadingFile_); };
    FILE *OpenThicknessGeometryFileForBlade(int i, char *FileName) { return OpenThicknessGeometryFile(i, FileName, RotorThicknessGeometryFile_); };
    FILE *OpenBPMFile(char *FileName);

    FILE *LoadingGeometryFileForBlade(int i)   { return RotorLoadingGeometryFile_[i]; };
    FILE *LoadingFileForBlade(int i)           { return RotorLoadingFile_[i]; };
    FILE *ThicknessGeometryFileForBlade(int i) { return RotorThicknessGeometryFile_[i]; };
    FILE *BPMFile(void)                        { return BPMFile_; };  
    
    // Wing files
    
    FILE *OpenLoadingGeometryFileForWingSurface(int i, char *FileName) { return OpenLoadingGeometryFile(i, FileName, WingLoadingGeometryFile_); };
    FILE *OpenLoadingFileForWingSurface(int i, char *FileName) { return OpenLoadingFile(i, FileName, WingLoadingFile_); };
    FILE *OpenThicknessGeometryFileForWingSurface(int i, char *FileName) { return OpenThicknessGeometryFile(i, FileName, WingThicknessGeometryFile_); };

    FILE *LoadingGeometryFileForWingSurface(int i)   { return WingLoadingGeometryFile_[i]; };
    FILE *LoadingFileForWingSurface(int i)           { return WingLoadingFile_[i]; };
    FILE *ThicknessGeometryFileForWingSurface(int i) { return WingThicknessGeometryFile_[i]; };

    // Body files
    
    FILE *OpenThicknessGeometryFileForBodySurface(int i, char *FileName) { return OpenThicknessGeometryFile(i, FileName, BodyThicknessGeometryFile_); };

    FILE *ThicknessGeometryFileForBodySurface(int i) { return BodyThicknessGeometryFile_[i]; };
        
    void CloseFiles(void);






   /** Write out the noise files for psuwopwop **/
    
    void WriteOutNoiseFiles(int Case);  
    
    /** Write out time accurate noise files for psuwopwop **/
    
    void WriteOutTimeAccurateNoiseFiles(int Case);  
    
    /** Write out steady state noise files for psuwopwop **/
    
    void WriteOutSteadyStateNoiseFiles(int Case);  

   /** Do a noise analysis... this is a post processing step on an existing solution **/
    
    int &NoiseAnalysis(void) { return NoiseAnalysis_; };
    
    /** Create files for an assumed steady state analysis... ie, not time accurate **/
    
    int &SteadyStateNoise(void) { return SteadyStateNoise_; };    
   
    /** Input geometry and flight conditions are in english units...
     * 
     * ... so we will convert to SI for output to psuwopwop
     * 
     **/

    int &WopWopEnglishUnits(void) { return WopWopEnglishUnits_; };   

    /** Do a psuwopwop flyby analysis **/

    void DoWopWopFlyBy(void) { WopWopFlyBy_ = 1; };
    
    /** Do a psuwopwop foot print analysis **/
    
    void DoWopWopFootPrint(void) { WopWopFlyBy_ = 0; };
    
         
};

#include "END_NAME_SPACE.H"

#endif
