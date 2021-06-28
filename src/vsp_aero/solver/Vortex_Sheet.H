//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VORTEX_SHEET_H
#define VORTEX_SHEET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "Vortex_Trail.H"
#include "Vortex_Bound.H"
#include "Interaction.H"
#include "time.H"

class COMMON_VORTEX_SHEET {

private: 

    int Sheet_i_;
    int Sheet_j_;
    
    int TEVortex_i_;
    int TEVortex_j_;

public: 

    int &Sheet_i(void) { return Sheet_i_; };
    int &Sheet_j(void) { return Sheet_j_; };

    int &TEVortex_i(void) { return TEVortex_i_; };
    int &TEVortex_j(void) { return TEVortex_j_; };
 
};

// Definition of the VORTEX_SHEET class

class VORTEX_SHEET {

private:

    void init(void);

    int Verbose_;
    
    // Initial trailing vortex list
    
    int WingSurface_;
    
    int IsPeriodic_;
    
    int Level_;
    
    int SheetID_;
    
    int NumberOfLevels_;
    
    int NumberOfTrailingVortices_;
    
    int NumberOfStartingVortices_;
    
    int NumberOfSubVortices_;
    
    int TimeAccurate_;
        
    int QuasiTimeAccurate_;
    
    int TimeAnalysisType_;
    
    int DoGroundEffectsAnalysis_;

    int CurrentTimeStep_;
    
    int IsARotor_;
    
    VSPAERO_DOUBLE Vinf_;
    
    VSPAERO_DOUBLE FreeStreamVelocity_[3];
    
    VSPAERO_DOUBLE TimeStep_;

    VSPAERO_DOUBLE **TrailingGamma_;
    
    VSPAERO_DOUBLE **StartingGamma_;
        
    VSPAERO_DOUBLE *VortexTrailingGamma1_;
    
    VSPAERO_DOUBLE *VortexTrailingGamma2_;
    
    VSPAERO_DOUBLE ***TrailingGammaListForLevel_;

    VSPAERO_DOUBLE &StartingGamma(int i) { return StartingGamma_[0][i]; };

    VSPAERO_DOUBLE CoreSize_;
    
    VSPAERO_DOUBLE Distance_;
    
    static double FarAway_;

    VORTEX_TRAIL **TrailingVortexList_;
    
    // Agglomerated trailing vortex list
    
    int NumberOfAgglomeratedTrailingVortices_;
    
    VORTEX_TRAIL **AgglomeratedTrailingVortexList_;
    
    VORTEX_TRAIL *AgglomeratedTrailingVortexList(int i) { return AgglomeratedTrailingVortexList_[i]; };

    // Trailing vortex lists for each sub level
    
    int *NumberOfTrailingVorticesForLevel_;

    VORTEX_TRAIL ***TrailingVortexListForLevel_;
    
    // Trailing vortex sheet list for each sub level
    
    int *NumberOfVortexSheetsForLevel_;
    
    int TotalNumberOfVortexSheets_;

    VORTEX_SHEET **VortexSheetListForLevel_;
    
    // Left and right trailing vortex
    
    VORTEX_TRAIL *VortexTrail1_;
    VORTEX_TRAIL *VortexTrail2_;

    // Children
    
    int ThereAreChildren_;
    
    VORTEX_SHEET *Child1_;
    VORTEX_SHEET *Child2_;
    
    // Startup vortices for this vortex sheet    
            
    VORTEX_BOUND BoundVortex_;
    
    VORTEX_BOUND &BoundVortex(void) { return BoundVortex_; };

    // Set up the vortex sheets

    void SetupPlanarVortexSheets(void);
      
    void SetupCircularVortexSheets(void);  
    
    // Velocity calculations
    
    VSPAERO_DOUBLE Span_;
    
    void CreateTrailingVortexInteractionList(VORTEX_SHEET &VortexSheet, VSPAERO_DOUBLE xyz_p[3]);

    void CreateVortexSheetInteractionList(VORTEX_SHEET &VortexSheet, VSPAERO_DOUBLE xyz_p[3]);

    void StartingVorticesInducedVelocity(VORTEX_SHEET &VortexSheet, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE dq[3]);
    
    void StartingVorticesInducedVelocity(VORTEX_SHEET &VortexSheet, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE dq[3], VSPAERO_DOUBLE CoreSize);
    
    void UpdateGeometryLocation(void);

    int Evaluate_;

    // TE Bounding Box
    
    BBOX TEBox_;
    
    // Common trailing edge node list
    
    int NumberOfCommonTE_;
    
    int *NumberOfCommonNodesForTE_;
       
    COMMON_VORTEX_SHEET *CommonTEList_;   
     
public:

    // Constructor, Destructor, Copy

    VORTEX_SHEET(void);
   ~VORTEX_SHEET(void);
    VORTEX_SHEET(const VORTEX_SHEET &Vortex_Sheet);

    // Copy function

    VORTEX_SHEET& operator=(const VORTEX_SHEET &Vortex_Sheet);
    VORTEX_SHEET& operator+=(const VORTEX_SHEET &VortexSheet);
    
    // Set up the vortex sheet

    void SetupVortexSheets(void);
    
    // Wing Surface ID
    
    int &WingSurface(void) { return WingSurface_; };
        
    // Trailing vortices
    
    int NumberOfSubVortices(void) { return NumberOfSubVortices_; };
    
    int NumberOfStartingVortices(void) { return NumberOfStartingVortices_; };
    
    void SizeTrailingVortexList(int NumberOfTrailingVortices);
    
    int NumberOfTrailingVortices(void) { return NumberOfTrailingVortices_; };
    
    int TotalNumberOfVortexSheets(void) { return TotalNumberOfVortexSheets_; };
    
    VORTEX_TRAIL& TrailingVortex(int i) { return *(TrailingVortexList_[i]); };

    // Left and right trailing vortices
    
    void SetTrailingVortices(VORTEX_TRAIL &Trail1, VORTEX_TRAIL &Trail2) { VortexTrail1_ = &Trail1 ; VortexTrail2_ = &Trail2; };

    VORTEX_TRAIL& VortexTrail1(void) { return *VortexTrail1_; };
    VORTEX_TRAIL& VortexTrail2(void) { return *VortexTrail2_; };    
    
    // Set left and right vortex strengths for injection
    
    void SetVortexGammas(VSPAERO_DOUBLE *Gamma1, VSPAERO_DOUBLE *Gamma2) { VortexTrailingGamma1_ = Gamma1 ; VortexTrailingGamma2_ = Gamma2; };

    VSPAERO_DOUBLE &TrailingGamma1(int i) { return VortexTrailingGamma1_[i]; };
    VSPAERO_DOUBLE &TrailingGamma2(int i) { return VortexTrailingGamma2_[i]; };

    // Vortex sheet children
    
    void SetupChildren(VORTEX_SHEET &Child1, VORTEX_SHEET &Child2) { Child1_ = &Child1 ; Child2_ = &Child2; ThereAreChildren_ = 2; };
    void SetupChildren(VORTEX_SHEET &Child1)                       { Child1_ = &Child1 ; Child2_ = NULL;    ThereAreChildren_ = 1; };

    int &ThereAreChildren(void) { return ThereAreChildren_; };
    
    VORTEX_SHEET &Child1(void) { return *Child1_; };
    VORTEX_SHEET &Child2(void) { return *Child2_; };

    // Level for this vortex sheet
    
    int &Level(void) { return Level_; };
    
    // Evaluation flag... for agglomeration multipole routine
    
    int &Evaluate(void) { return Evaluate_; };    
    
    // Sheed ID
    
    int &SheetID(void) { return SheetID_; };
    
    // Distance to evaluation point
    
    VSPAERO_DOUBLE &Distance(void) { return Distance_; };
    
    // Span
    
    VSPAERO_DOUBLE Span(void) { return Span_; };
    
    // Time accurate data
    
    int &TimeAccurate(void) { return TimeAccurate_; };
    
    int &QuasiTimeAccurate(void) { return QuasiTimeAccurate_; };
    
    int &TimeAnalysisType(void ) { return TimeAnalysisType_; };

    int &CurrentTimeStep(void) { return CurrentTimeStep_; };
    
    int &IsARotor(void) { return IsARotor_; };
    
    VSPAERO_DOUBLE &Vinf(void) { return Vinf_; };
    
    VSPAERO_DOUBLE &FreeStreamVelocity(int i) { return FreeStreamVelocity_[i]; };
    
    VSPAERO_DOUBLE &TimeStep(void) { return TimeStep_; };

    // Induced velocity from this vortex sheet
    
    void Setup(void);
    
    int &IsPeriodic(void) { return IsPeriodic_; };

    int FarAway(VSPAERO_DOUBLE xyz_p[3]);
    
    VSPAERO_DOUBLE Distance(VSPAERO_DOUBLE xyz_p[3]);
        
    void SaveVortexState(void);
    
    void UpdateVortexStrengths(int UpdateType);
    
    void UpdateConvectedDistance(void);
    
    VORTEX_SHEET_ENTRY *CreateInteractionSheetList(VSPAERO_DOUBLE xyz_p[3], int &NumberOfEvaluatedSheets);
    
    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);
    
    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE xyz_te[3]);
    
    void InducedKuttaVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);    

    void InducedVelocity(int NumberOfSheets, VORTEX_SHEET_ENTRY *SheetList, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);

    void InducedVelocity(int NumberOfSheets, VORTEX_SHEET_ENTRY *SheetList, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE xyz_te[3], VSPAERO_DOUBLE q[3]);
 
    void InducedKuttaVelocity(int NumberOfSheets, VORTEX_SHEET_ENTRY *SheetList, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);
   
    VSPAERO_DOUBLE StartingGamma(int i, int j) { return StartingGamma_[i][j]; };
 
    int &DoGroundEffectsAnalysis(void) { return DoGroundEffectsAnalysis_; };
    
    VSPAERO_DOUBLE UpdateWakeLocation(void);
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat, int *ComponentInThisGroup);

    void UpdateRotorFlags(int *ComponentInThisGroup);
  
    void ZeroEdgeVelocities(void);

    void ProlongateEdgeVelocities(void);
    
    VSPAERO_DOUBLE CoreSize(void) { return CoreSize_; };
    
    double &FarAwayRatio(void) { return FarAway_; };

    // Wake Bounding Box
    
    BBOX &TEBox(void) { return TEBox_; };    

    // Common trailing edge node list
    
    void SizeCommonTEList(int NumberOfCommonTE);

    int NumberOfCommonTE(void) { return NumberOfCommonTE_; };
    
    int &NumberOfCommonNodesForTE(int i) { return NumberOfCommonNodesForTE_[i]; };
       
    COMMON_VORTEX_SHEET &CommonTEList(int i) { return CommonTEList_[i]; };      
    
    // Wake Damping
    
    void TurnWakeDampingOn(void);
    void TurnWakeDampingOff(void);
    
};

#endif
