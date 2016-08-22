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
#include "time.H"

// Definition of the VORTEX_SHEET class

class VORTEX_SHEET {

private:

    void init(void);

    int Verbose_;
    
    // Initial trailing vortex list
    
    int IsPeriodic_;
    
    int Level_;
    
    int NumberOfLevels_;
    
    int NumberOfTrailingVortices_;

    double *Gamma_;
    
    VORTEX_TRAIL *TrailingVortexList_;
    
    // Agglomerated trailing vortex list
    
    int NumberOfAgglomeratedTrailingVortices_;
    
    VORTEX_TRAIL **AgglomeratedTrailingVortexList_;
    
    VORTEX_TRAIL *AgglomeratedTrailingVortexList(int i) { return AgglomeratedTrailingVortexList_[i]; };

    // Trailing vortex lists for each sub level
    
    int *NumberOfTrailingVorticesForLevel_;
    
    int **TrailingVortexEvaluatedAtLevel_;
    
    double **TrailingVortexGammaForLevel_;
    
    VORTEX_TRAIL ***TrailingVortexListForLevel_;
    
    // Trailing vortex sheet list for each sub level
    
    int *NumberOfVortexSheetsForLevel_;
    
    VORTEX_SHEET **VortexSheetListForLevel_;
    
    // Left and right trailing vortex
    
    VORTEX_TRAIL *VortexTrail1_;
    VORTEX_TRAIL *VortexTrail2_;

    // Children
    
    int ThereAreChildren_;
    
    VORTEX_SHEET *Child1_;
    VORTEX_SHEET *Child2_;

    // Set up the vortex sheets

    void SetupPlanarVortexSheets(void);
      
    void SetupCircularVortexSheets(void);  
    
    // Velocity calculations
    
    double Span_;
    double MidSpan_[3];
    
    void CreateTrailingVortexInteractionList(VORTEX_SHEET &VortexSheet, double xyz_p[3]);
    
    void InjectCirculation(void);

public:

    // Constructor, Destructor, Copy

    VORTEX_SHEET(void);
   ~VORTEX_SHEET(void);
    VORTEX_SHEET(const VORTEX_SHEET &Vortex_Sheet);

    // Copy function

    VORTEX_SHEET& operator=(const VORTEX_SHEET &Vortex_Sheet);
    
    // Set up the vortex sheet

    void SetupVortexSheets(void);
        
    // Trailing vortices
    
    void SizeTrailingVortexList(int NumberOfTrailingVortices);
    
    int NumberOfTrailingVortices(void) { return NumberOfTrailingVortices_; };
    
    VORTEX_TRAIL& TrailingVortexEdge(int i) { return TrailingVortexList_[i]; };

    // Left and right trailing vortices
    
    void SetTrailingVortices(VORTEX_TRAIL &Trail1, VORTEX_TRAIL &Trail2) { VortexTrail1_ = &Trail1 ; VortexTrail2_ = &Trail2; };

    VORTEX_TRAIL& VortexTrail1(void) { return *VortexTrail1_; };
    VORTEX_TRAIL& VortexTrail2(void) { return *VortexTrail2_; };    

    // Vortex sheet children
    
    void SetupChildren(VORTEX_SHEET &Child1, VORTEX_SHEET &Child2) { Child1_ = &Child1 ; Child2_ = &Child2; ThereAreChildren_ = 2; };
    void SetupChildren(VORTEX_SHEET &Child1)                       { Child1_ = &Child1 ;                    ThereAreChildren_ = 1; };

    int &ThereAreChildren(void) { return ThereAreChildren_; };
    
    VORTEX_SHEET &Child1(void) { return *Child1_; };
    VORTEX_SHEET &Child2(void) { return *Child2_; };
    
    // Level for this vortex sheet
    
    int &Level(void) { return Level_; };
    
    // Induced velocity from this vortex sheet
    
    void Setup(void);
    
    int &IsPeriodic(void) { return IsPeriodic_; };

    int FarAway(double xyz_p[3]);
    
    void UpdateVortexStrengths(void);
    
    void InducedVelocity(double xyz_p[3], double q[3]);
    
    void InducedVelocity(double xyz_p[3], double q[3], double xyz_te[3]);
    
    void InducedKuttaVelocity(double xyz_p[3], double q[3]);    

};

#endif
