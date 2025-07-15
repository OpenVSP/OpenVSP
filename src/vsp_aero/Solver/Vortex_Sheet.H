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
#include "GlobalWakeLoop.H"
#include "VSP_Grid.H"
#include "time.H"

#include "START_NAME_SPACE.H"

// Definition of the VORTEX_SHEET class

class VORTEX_SHEET {

private:

    void init(void);
    
    int Verbose_;
    
    // Wing surface this wake is attached to
    
    int WingSurface_;
    
    // VSPAERO Component ID
    
    int ComponentID_;
    
    // Wake is periodic ... for instance on a nacelle
    
    int IsPeriodic_;
    
    // Wake should be treated as 2D
    
    int Is2D_;

    // Optimization flag
    
    int OptimizationSolve_;

    // Time accurate flag

    int TimeAccurate_;
    
    // Time analysis type flag
 
    int TimeAnalysisType_;

    // Current global time
    
    int Time_;
    
    // Integration time step
    
    double DeltaTime_;
    
    // Wake is shed from a rotor
    
    int IsARotor_;
    
    // Turn on vortex stretching
    
    int DoVortexStretching_;    
    
    // Free stream velocity magnitude
    
    double Vinf_;
    
    // Free stream velocity vector
    
    double FreeStreamVelocity_[3];

    // Core size
    
    double CoreWidth_;
    
    // User specified core size factor
    
    double CoreSizeFactor_;

    // Number of trailing vortices

    int NumberOfTrailingVortices_;
    
    // Trailing vortex list
    
    VORTEX_TRAIL *TrailingVortexList_;
    
    // Number of kutta edges
    
    int NumberOfKuttaEdges_;

    // Number of nodes on each trailing vortex
    
    int NumberOfWakeTrailingNodes_;

    // Number of wake loops
    
    int NumberOfWakeLoops_;
    
    // Wake loop list
    
    GLOBAL_WAKE_LOOP *WakeLoopList_;
    
    // Set up the vortex sheets

    void SetupPlanarVortexSheets(void);
      
    void SetupCircularVortexSheets(void);  
 
    // Update wake location
    
    void UpdateGeometryLocation(void);

    // Size the global wake loop list data structure 
    
    void SizeWakeLoopList(void);

public:

    // Constructor, Destructor, Copy

    VORTEX_SHEET(void);
   ~VORTEX_SHEET(void);
    VORTEX_SHEET(const VORTEX_SHEET &Vortex_Sheet);

    // Copy function

    VORTEX_SHEET& operator=(const VORTEX_SHEET &Vortex_Sheet);
    
    /** Set up the vortex sheet **/

    void SetupVortexSheets(void);
     
    /** Size the trailing vortx list **/
    
    void SizeTrailingVortexList(int NumberOfTrailingVortices);

    /** Wing Surface ID for this vortex sheet **/
    
    int &WingSurface(void) { return WingSurface_; };
    
    /** VSPAERO Component ID **/
    
    int &ComponentID(void) { return ComponentID_; };    

    /** Number of trailing vortices **/
    
    int NumberOfTrailingVortices(void) { return NumberOfTrailingVortices_; };

    /** Access to the i'th trailing vortex **/
    
    VORTEX_TRAIL& TrailingVortex(int i) { return TrailingVortexList_[i]; };
    
    /** Number of kutta edges **/
    
    int NumberOfKuttaEdges(void) { return NumberOfKuttaEdges_; };
    
    /** Number of nodes on each trailing vortex **/
    
    int &NumberOfWakeTrailingNodes(void) { return NumberOfWakeTrailingNodes_; };

    /** Number of wake loops **/
    
    int &NumberOfWakeLoops(void) { return NumberOfWakeLoops_; };
    
    // Wake loop list
    
    GLOBAL_WAKE_LOOP &WakeLoopList(int i) { return WakeLoopList_[i]; };
    
    /** Time accurate analysis flag **/
    
    int &TimeAccurate(void) { return TimeAccurate_; };

    /** Time accurate analysis type flag **/
        
    int &TimeAnalysisType(void ) { return TimeAnalysisType_; };

    /** Current time step for a time accurate analysis **/
    
    int &Time(void) { return Time_; };
        
    /**  Integration time step for time accurate analysis **/
    
    double &DeltaTime(void) { return DeltaTime_; };
        
    /** This vortex sheet is attached to a rotor **/
    
    int &IsARotor(void) { return IsARotor_; };
    
    /** Turn on vortex stretching model **/
    
    int &DoVortexStretching(void) { return DoVortexStretching_; };
    
    /** Freestream velocity magnitude **/
    
    double &Vinf(void) { return Vinf_; };
    
    /** Free stream velocity vector **/
    
    double &FreeStreamVelocity(int i) { return FreeStreamVelocity_[i]; };

    /** Vortex sheet is periodoic... ala a nacelle **/
    
    int &IsPeriodic(void) { return IsPeriodic_; };
    
    /** Vortex sheet is 2D... this is mostly for testing purposed **/
    
    int &Is2D(void) { return Is2D_; };

    /** Update how far the wake has been evolved in a time accurate analysis **/
    
    void UpdateConvectedDistance(void);
 
    /** Update the wake location **/
    
    void UpdateWakeLocation(double WakeRelax, double &ResMax, double &ResL2);

    /** Update the unsteady wake location **/
    
    void StoreWakeLocation(void);
        
    /** Update the unsteady wake location **/
    
    void UpdateUnsteadyWakeLocation(double WakeRelax, double &ResMax, double &ResL2);
        
    /** Update geometry of bound vortices after trailing wakes have been updated externally... **/
        
    void Update(void);
    
    /** Move down vortex sheet, and trailing edge structures to note that anything in the passed in component group is part of a rotor **/
     
    void UpdateRotorFlags(int *ComponentInThisGroup);
      
    /** Finite core size for the trailing vortices **/
    
    double CoreWidth(void) { return CoreWidth_; };
    
    /** User adjustment of core size model **/
    
    double &CoreSizeFactor(void) { return CoreSizeFactor_; };
    
    /** Turn on Wake damping **/
    
    void TurnWakeDampingOn(void);
    
    /** Turn off wake damping **/
    
    void TurnWakeDampingOff(void);
    
    /** Calculate wake equations residuals **/

    void CalculateWakeResidual(double *Residual);

    /** Wake equations matrix multiply **/
    
    void WakeMatrixMultiply(double *VecIn, double *VecOut);

    /** Wake equations forward preconditioning **/
    
    void DoWakeForwardMatrixPrecondition(double *VecIn);

    /** Wake mesh perturbation equations transpose matrix multiply **/

    void WakeTranposeMatrixMultiplyMeshPerturbations(double *VecIn, double *VecOut);
           
};

#include "END_NAME_SPACE.H"

#endif
