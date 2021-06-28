//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VORTEX_TRAIL_H
#define VORTEX_TRAIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "time.H"
#include "VSP_Edge.H"
#include "Search.H"

#define IMPLICIT_WAKE_GAMMAS 1
#define EXPLICIT_WAKE_GAMMAS 2
#define      ALL_WAKE_GAMMAS 3 

#define IMPULSE_ANALYSIS 1
#define HEAVE_ANALYSIS   2
#define P_ANALYSIS       3
#define Q_ANALYSIS       4
#define R_ANALYSIS       5

class SEARCH;

// Definition of the VORTEX_TRAIL class

class VORTEX_TRAIL {

private:

    void init(void);
     
    int Verbose_;
 
    // Wing, edge, andtrailing node this vortex belongs to
    
    int Wing_;
    
    int Edge_;
    
    int Node_;
    
    int ComponentID_;

    // Wake points list
    
    VSPAERO_DOUBLE Length_;
    
    VSPAERO_DOUBLE FarDist_;
    
    VSPAERO_DOUBLE *S_[2];
  
    int NumberOfNodes_;
    
    VSP_NODE TE_Node_;
    
    VSP_NODE *NodeList_;

    // List of trailing vortices
    
    int NumberOfLevels_;
    
    int *NumberOfSubVortices_;
    
    int TotalNumberOfSubVortices_;
    
    VSP_EDGE **VortexEdgeList_;

    VSP_EDGE *VortexEdgeList(int Level) { return VortexEdgeList_[Level]; };

    VSPAERO_DOUBLE ***VortexEdgeVelocity_;
    
    VSPAERO_DOUBLE FreeStreamVelocity_[3];
    
    VSPAERO_DOUBLE LocalVelocityAtTrailingEdge_[3];

    // Far away ratio
    
    static double FarAway_;
    
    // Distance between trailing wakes at trailing edge
    
    VSPAERO_DOUBLE Sigma_;
    
    // Core size
    
    VSPAERO_DOUBLE CoreSize_;
    
    // Minimum Tolerance
    
    VSPAERO_DOUBLE Tolerance_;
    
    // Trailing edge vector
    
    VSPAERO_DOUBLE TEVec_[3];

    // Location along span 
    
    VSPAERO_DOUBLE SoverB_;

    // Circulation strength
    
    int Evaluate_;
    int TimeAccurate_;
    int QuasiTimeAccurate_;
    int ConvectType_;
    int TimeAnalysisType_;
    int CurrentTimeStep_;
    int WakeDampingIsOn_;
    int IsARotor_;
    
    VSPAERO_DOUBLE TimeStep_;
    VSPAERO_DOUBLE Vinf_;
    VSPAERO_DOUBLE *Gamma_;
    VSPAERO_DOUBLE *GammaNew_;
    VSPAERO_DOUBLE *GammaSave_;

    VSPAERO_DOUBLE *WakeAge_;
        
    VSPAERO_DOUBLE *a_;
    VSPAERO_DOUBLE *b_;
    VSPAERO_DOUBLE *c_;
    VSPAERO_DOUBLE *d_;
    VSPAERO_DOUBLE *r_;
    
    VSPAERO_DOUBLE *dx_;
    VSPAERO_DOUBLE *dy_;
    VSPAERO_DOUBLE *dz_;

    // Blade analysis parameters
    
    int RotorAnalysis_;    
    VSPAERO_DOUBLE BladeRPM_;
    
    // Smooth out the trailing wake shape
    
    void Smooth(void);
    void SmoothWake(void);
    void SmoothVelocity(VSPAERO_DOUBLE *Velocity);
    void LimitVelocity(VSPAERO_DOUBLE q[3]);
    
    // Test stuff
    
    VSPAERO_DOUBLE EvaluatedLength_;
    
    // Ground effects
    
    int DoGroundEffectsAnalysis_;
    
    // Induced Velocity calculation
    
    void InducedVelocity_(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);
    
    // Search data structure
    
    int Searched_;
    
    VSPAERO_DOUBLE Distance_;
    
    SEARCH *Search_;
    
    void CreateSearchTree_(void);
    
    
public:

    // Constructor, Destructor, Copy

    VORTEX_TRAIL(void);
   ~VORTEX_TRAIL(void);
    VORTEX_TRAIL(const VORTEX_TRAIL &Trailing_Vortex);

    // Copy function

    VORTEX_TRAIL& operator=(const VORTEX_TRAIL &Trailing_Vortex);
    VORTEX_TRAIL& operator+=(const VORTEX_TRAIL &Trailing_Vortex);
    
    // Time accurate flag
    
    int &TimeAccurate(void) { return TimeAccurate_; };
    int &QuasiTimeAccurate(void) { return QuasiTimeAccurate_; };
    int &TimeAnalysisType(void ) { return TimeAnalysisType_; };
    
    // Accesss to Vortex edge data

    int &Wing(void) { return Wing_; };
    int &Edge(void) { return Edge_; };
    int &Node(void) { return Node_; };
    int &ComponentID(void) { return ComponentID_; };
 
    void Setup(int NumSubVortices, VSPAERO_DOUBLE FarDist, VSP_NODE &Node1, VSP_NODE &Node2);
                       
    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);

    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE CorSize);

    void CalculateVelocityForSubVortex(VSP_EDGE &VortexEdge, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);

    // Distance/2 between wakes at trailing edge
    
    VSPAERO_DOUBLE &Sigma(void) { return Sigma_; };

    // Core size at trailing edge
    
    VSPAERO_DOUBLE &CoreSize(void) { return CoreSize_; };
        
    // Trailing edge vector
    
    VSPAERO_DOUBLE &TEVec(int i) { return TEVec_[i]; };
    VSPAERO_DOUBLE *TEVec(void) { return TEVec_; };
    
    // Trailing edge location along span
    
    VSPAERO_DOUBLE &SoverB(void) { return SoverB_; };
    
    // Evaluation flag... for agglomeration multipole routine
    
    int &Evaluate(void) { return Evaluate_; };
    
    // Circulation strength

    VSPAERO_DOUBLE &Gamma(void) { return Gamma_[0]; };
    
    VSPAERO_DOUBLE &Gamma(int i) {  return Gamma_[i]; };
    
    // Update gamma values for sub vortices
    
    void UpdateGamma(void);    
     
    // Trailing edge node
    
    VSP_NODE &TE_Node(void) { return TE_Node_; };
     
    // Vortex length
    
    VSPAERO_DOUBLE Length(void) { return Length_; };
    
    // Number of sub vortices

    int NumberOfLevels(void) { return NumberOfLevels_; };
    
    int TotalNumberOfSubVortices(void) { return TotalNumberOfSubVortices_; };

    int NumberOfSubVortices(int i) { return NumberOfSubVortices_[i]; };
    
    int NumberOfSubVortices(void) { return NumberOfSubVortices_[1]; };
    
    // Access to each vortex edge on the finest level
    
    VSP_EDGE &VortexEdge(int i) { return VortexEdgeList_[1][i]; };
    
    VSP_EDGE &VortexEdge(int Level, int i) { return VortexEdgeList_[Level][i]; };
    
    // Centroid of each sub vortex
    
    VSPAERO_DOUBLE *xyz_c(int i) { return VortexEdgeList_[1][i].xyz_c(); };
    
    // Wake points list
    
    VSPAERO_DOUBLE S(int i) { return S_[0][i]; };

    VSPAERO_DOUBLE GammaScale(int i);
    
    // Velocity for each sub vortex
    
    VSPAERO_DOUBLE &U(int i) { return VortexEdgeVelocity_[1][i][0]; };
    VSPAERO_DOUBLE &V(int i) { return VortexEdgeVelocity_[1][i][1]; };
    VSPAERO_DOUBLE &W(int i) { return VortexEdgeVelocity_[1][i][2]; };

    VSPAERO_DOUBLE &U(int Level, int i) { return VortexEdgeVelocity_[Level][i][0]; };
    VSPAERO_DOUBLE &V(int Level, int i) { return VortexEdgeVelocity_[Level][i][1]; };
    VSPAERO_DOUBLE &W(int Level, int i) { return VortexEdgeVelocity_[Level][i][2]; };
    
    // Free stream velocity
    
    VSPAERO_DOUBLE &FreeStreamVelocity(int i) { return FreeStreamVelocity_[i];};
    
    VSPAERO_DOUBLE &LocalVelocityAtTrailingEdge(int i) { return LocalVelocityAtTrailingEdge_[i]; };
    
    VSPAERO_DOUBLE *VortexEdgeVelocity(int i) { return VortexEdgeVelocity_[1][i]; };
    
    void ZeroEdgeVelocities(void);
    
    void ProlongateEdgeVelocities(void);
    
    // Update the wake location
    
    VSPAERO_DOUBLE UpdateWakeLocation(void);     
    
    void ConvectWakeVorticity(int ConvectType);
    
    // Ground effects analysis flag
    
    int &DoGroundEffectsAnalysis(void) { return DoGroundEffectsAnalysis_; };

    // Bladed analysis flags
    
    int &RotorAnalysis(void) { return RotorAnalysis_; }; 
    
    VSPAERO_DOUBLE &BladeRPM(void) { return BladeRPM_; };
    
    // Unsteady analysis
    
    VSPAERO_DOUBLE &TimeStep(void) { return TimeStep_; };
    
    VSPAERO_DOUBLE &Vinf(void) { return Vinf_; };
 
    int &CurrentTimeStep(void) { return CurrentTimeStep_; };
    
    int &WakeDampingIsOn(void) { return WakeDampingIsOn_; };
    
    int &IsARotor(void) { return IsARotor_; };
    
    void CreateSearchTree(void) { CreateSearchTree_(); };

    void SaveVortexState(void);

    void WriteToFile(FILE *adb_file);
    void ReadInFile(FILE *adb_file);
    void SkipReadInFile(FILE *adb_file);
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);

    int &Searched(void) { return Searched_; };
    
    VSPAERO_DOUBLE &Distance(void) { return Distance_; };

    SEARCH &Search(void) { return *Search_; };
    
    double &FarAwayRatio(void) { return FarAway_; };
            
};

#endif
