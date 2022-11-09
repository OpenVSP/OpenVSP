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

#include "START_NAME_SPACE.H"

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
    
    int Is2D_;
    
    int Level_;
    
    int SheetID_;
    
    int NumberOfLevels_;
    
    int NumberOfTrailingVortices_;
    
    int NumberOfStartingVortices_;
    
    int NumberOfSubVortices_;
    
    int TimeAccurate_;
    
    int OptimizationSolve_;
            
    int TimeAnalysisType_;
    
    int DoGroundEffectsAnalysis_;

    int CurrentTimeStep_;
    
    int IsARotor_;
    
    int FreezeWakeGeoemtry_;
    
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
    
    VSPAERO_DOUBLE CoreSizeFactor_;
    
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

    // Left and right trailing vortices for this vortex sheet **/
    
    void SetTrailingVortices(VORTEX_TRAIL &Trail1, VORTEX_TRAIL &Trail2) { VortexTrail1_ = &Trail1 ; VortexTrail2_ = &Trail2; };
    
    // Left trailing vortex 

    VORTEX_TRAIL& VortexTrail1(void) { return *VortexTrail1_; };
    
    // Right trailing vortex 
    
    VORTEX_TRAIL& VortexTrail2(void) { return *VortexTrail2_; };    
    
    // Set left and right vortex strengths for injection 
    
    void SetVortexGammas(VSPAERO_DOUBLE *Gamma1, VSPAERO_DOUBLE *Gamma2) { VortexTrailingGamma1_ = Gamma1 ; VortexTrailingGamma2_ = Gamma2; };

    // Vortex strength for trailing vortex 1 
    
    VSPAERO_DOUBLE &TrailingGamma1(int i) { return VortexTrailingGamma1_[i]; };
    
    // Vortex strength for trailing vortex 2 
    
    VSPAERO_DOUBLE &TrailingGamma2(int i) { return VortexTrailingGamma2_[i]; };

    // Set Child1, and Child2 for this vortex sheet 
    
    void SetupChildren(VORTEX_SHEET &Child1, VORTEX_SHEET &Child2) { Child1_ = &Child1 ; Child2_ = &Child2; ThereAreChildren_ = 2; };
    
    // Set Child1, no Child2 for this vortex sheet 
    
    void SetupChildren(VORTEX_SHEET &Child1)                       { Child1_ = &Child1 ; Child2_ = NULL;    ThereAreChildren_ = 1; };

    // This vortex sheet has children 
    
    int &ThereAreChildren(void) { return ThereAreChildren_; };
    
    // Child 1 for this vortex sheet 
    
    VORTEX_SHEET &Child1(void) { return *Child1_; };
    
    // Child 2 for this vortex sheet 
    
    VORTEX_SHEET &Child2(void) { return *Child2_; };

    // Setup this vortex sheet 
    
    void Setup(void);
    
    // Determine if evaluation point is faraway wrt this vortex sheet

    int FarAway(VSPAERO_DOUBLE xyz_p[3]);
    
    // Calculate distance from evaluation point to this vortex sheet
    
    VSPAERO_DOUBLE Distance(VSPAERO_DOUBLE xyz_p[3]);
         
public:

    // Constructor, Destructor, Copy

    VORTEX_SHEET(void);
   ~VORTEX_SHEET(void);
    VORTEX_SHEET(const VORTEX_SHEET &Vortex_Sheet);

    // Copy function

    VORTEX_SHEET& operator=(const VORTEX_SHEET &Vortex_Sheet);
    VORTEX_SHEET& operator+=(const VORTEX_SHEET &VortexSheet);
    
    /** Set up the vortex sheet **/

    void SetupVortexSheets(void);
    
    /** Wing Surface ID for this vortex sheet **/
    
    int &WingSurface(void) { return WingSurface_; };
        
    /** Number of Subvortices for each trailing vortex **/
    
    int NumberOfSubVortices(void) { return NumberOfSubVortices_; };
    
    /** Number of starting vortices that have been shed down stream from the trailing edge **/
    
    int NumberOfStartingVortices(void) { return NumberOfStartingVortices_; };
    
    /** Size the trailing vortex list data structures **/
    
    void SizeTrailingVortexList(int NumberOfTrailingVortices);
    
    /** Number of trailing vortices **/
    
    int NumberOfTrailingVortices(void) { return NumberOfTrailingVortices_; };
    
    /** Number of vortex sheets across all the levels after agglomeration **/
    
    int TotalNumberOfVortexSheets(void) { return TotalNumberOfVortexSheets_; };
    
    /** Access to the i'th trailing vortex **/
    
    VORTEX_TRAIL& TrailingVortex(int i) { return *(TrailingVortexList_[i]); };


    /** Agglomeration level for this vortex sheet **/
    
    int &Level(void) { return Level_; };
    
    /** Evaluation flag... for agglomeration multipole routine **/
    
    int &Evaluate(void) { return Evaluate_; };    
    
    /** Global sheet ID **/
    
    int &SheetID(void) { return SheetID_; };
    
    /** Distance to evaluation point **/
    
    VSPAERO_DOUBLE &Distance(void) { return Distance_; };
    
    /** Span of this vortex sheet **/
    
    VSPAERO_DOUBLE Span(void) { return Span_; };
    
    /** Time accurate analysis flag **/
    
    int &TimeAccurate(void) { return TimeAccurate_; };
    
    /** Optimization flag... this will turn off some of the unsteady wake and essentially force a steady state solution **/
    
    int &OptimizationSolve(void) { return OptimizationSolve_; };
    
    /** Time accurate analysis type flag **/
        
    int &TimeAnalysisType(void ) { return TimeAnalysisType_; };

    /** Current time step for a time accurate analysis **/
    
    int &CurrentTimeStep(void) { return CurrentTimeStep_; };
    
    /** This vortex sheet is attached to a rotor **/
    
    int &IsARotor(void) { return IsARotor_; };
    
    /** Freestream velocity magnitude **/
    
    VSPAERO_DOUBLE &Vinf(void) { return Vinf_; };
    
    /** Free stream velocity vector **/
    
    VSPAERO_DOUBLE &FreeStreamVelocity(int i) { return FreeStreamVelocity_[i]; };
    
    /** Time step for time accurate analysis **/
    
    VSPAERO_DOUBLE &TimeStep(void) { return TimeStep_; };

    /** Vortex sheet is periodoic... ala a nacelle **/
    
    int &IsPeriodic(void) { return IsPeriodic_; };
    
    /** Vortex sheet is 2D... this is mostly for testing purposed **/
    
    int &Is2D(void) { return Is2D_; };
    
    /** Save the current vortex state **/
    
    void SaveVortexState(void);
    
    /** Update the vortex strengths **/
    
    void UpdateVortexStrengths(int UpdateType);
    
    /** Update how far the wake has been evolved in a time accurate analysis **/
    
    void UpdateConvectedDistance(void);
    
    /** Create a vortex sheet interaction list for point xyz **/
    
    VORTEX_SHEET_ENTRY *CreateInteractionSheetList(VSPAERO_DOUBLE xyz_p[3], int &NumberOfEvaluatedSheets);
    
    /** Calculate the induced velocity at point xyz **/
    
    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);
    
    /** Calculate the induced velocity at point xyz that is on trailing wake start at xyz_te... we are trying not
     * to do self trailing vortex evaluations **/
    
    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE xyz_te[3]);
    
    /** Calculate the induced trailing edge induced velocity ... this basically uses Munk's theorem to 
     * shift the incoming point to the x location of the start of each trailing vortex and then calculates
     * the induced velocity ... this is similar to doing a Trefftz plane analysis for the induced drag **/
     
    void InducedKuttaVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);    

    /** Calculate the induced velocity at point xyz from the vortex sheet list previously determined for this point **/
    
    void InducedVelocity(int NumberOfSheets, VORTEX_SHEET_ENTRY *SheetList, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);

    /** Calculate the induced velocity at point xyz from the vortex sheet list previously determined for this point ... but check for self induced
     * cases as we are on a trailing vortex **/
     
    void InducedVelocity(int NumberOfSheets, VORTEX_SHEET_ENTRY *SheetList, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE xyz_te[3], VSPAERO_DOUBLE q[3]);
 
    /** Calculate the induced trailing edge induced velocity ... this basically uses Munk's theorem to 
     * shift the incoming point to the x location of the start of each trailing vortex and then calculates
     * the induced velocity ... this is similar to doing a Trefftz plane analysis for the induced drag ...
     * in this case we are using a predetermine set of vortex sheets **/
     
    void InducedKuttaVelocity(int NumberOfSheets, VORTEX_SHEET_ENTRY *SheetList, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);
    
    /** Values for starting gammas (circulation) **/
   
    VSPAERO_DOUBLE StartingGamma(int i, int j) { return StartingGamma_[i][j]; };
    
    /** Flag to do ground effect analysis **/
 
    int &DoGroundEffectsAnalysis(void) { return DoGroundEffectsAnalysis_; };
    
    /** Update the wake location **/
    
    VSPAERO_DOUBLE UpdateWakeLocation(void);
    
    /** Update the geometry based on an input translation vector and rotation quaternion, this also shift the wake back in time **/
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat, int *ComponentInThisGroup);
    
    /** Update the trailing edge geometry based on an input translation vector and rotation quaternion **/
    
    void UpdateTrailingEdgeGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat, int *ComponentInThisGroup);
        
    /** Set the trailing and bound vortices Mach number **/
    
    void SetMachNumber(VSPAERO_DOUBLE Mach);

    /** Move down vortex sheet, and trailing edge structures to note that anything in the passed in component group is part of a rotor **/
     
    void UpdateRotorFlags(int *ComponentInThisGroup);
  
    /** Zero velocities evaluated for each edge making up the trailing vortices... these are the velocities used to adapt the location of
     * the wakes **/
    
    void ZeroEdgeVelocities(void);

    /** Prolongate the calculated induced velocites from the coarse to finer meshes as part of the multipole process **/
    
    void ProlongateEdgeVelocities(void);
    
    /** Finite core size for the trailing vortices **/
    
    VSPAERO_DOUBLE CoreSize(void) { return CoreSize_; };
    
    /** User adjustment of core size model **/
    
    VSPAERO_DOUBLE &CoreSizeFactor(void) { return CoreSizeFactor_; };
    
    /** Ratio used to determine if a point is far enough away from a vortex sheet to be accurately evaluated... or if we need
     * to move to a finer mesh **/
    
    double &FarAwayRatio(void) { return FarAway_; };

    /** Wake bounding box **/
    
    BBOX &TEBox(void) { return TEBox_; };    

    /** Common trailing edge node list **/
    
    void SizeCommonTEList(int NumberOfCommonTE);
    
    /** Number of common trailing edge nodes **/

    int NumberOfCommonTE(void) { return NumberOfCommonTE_; };
    
    /** Number of common nodes that share this xyz location **/
    
    int &NumberOfCommonNodesForTE(int i) { return NumberOfCommonNodesForTE_[i]; };
    
    /** Common vortex sheet list **/
   
    COMMON_VORTEX_SHEET &CommonTEList(int i) { return CommonTEList_[i]; };      
    
    /** Turn on Wake damping **/
    
    void TurnWakeDampingOn(void);
    
    /** Turn off wake damping **/
    
    void TurnWakeDampingOff(void);
    
};

#include "END_NAME_SPACE.H"

#endif
