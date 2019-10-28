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
 
    // Wing and edge, or trailing node this vortex belongs to
    
    int Wing_;
    
    int Node_;
    
    int ComponentID_;

    // Wake points list
    
    double Length_;
    
    double FarDist_;
    
    double *S_[2];
  
    int NumberOfNodes_;
    
    VSP_NODE TE_Node_;
    
    VSP_NODE *NodeList_;

    // List of trailing vortices
    
    int NumberOfLevels_;
    
    int *NumberOfSubVortices_;
    
    int TotalNumberOfSubVortices_;
    
    VSP_EDGE **VortexEdgeList_;

    VSP_EDGE *VortexEdgeList(int Level) { return VortexEdgeList_[Level]; };

    double ***VortexEdgeVelocity_;
    
    double FreeStreamVelocity_[3];
    
    double LocalVelocityAtTrailingEdge_[3];

    // Far away ratio
    
    static double FarAway_;
    
    // Distance between trailing wakes at trailing edge
    
    double Sigma_;
    
    // Core size
    
    double CoreSize_;
    
    // Minimum Tolerance
    
    double Tolerance_;
    
    // Trailing edge vector
    
    double TEVec_[3];

    // Location along span 
    
    double SoverB_;

    // Circulation strength
    
    int Evaluate_;
    int TimeAccurate_;
    int ConvectType_;
    int TimeAnalysisType_;
    int CurrentTimeStep_;
    
    double TimeStep_;
    double Vinf_;
    double *Gamma_;
    double *GammaNew_;
    double *GammaSave_;

    double *WakeAge_;
        
    double *a_;
    double *b_;
    double *c_;
    double *d_;
    double *r_;
    
    double *dx_;
    double *dy_;
    double *dz_;

    // Blade analysis parameters
    
    int RotorAnalysis_;    
    double BladeRPM_;
    
    // Smooth out the trailing wake shape
    
    void Smooth(void);
    void SmoothWake(void);
    void SmoothVelocity(double *Velocity);
    void LimitVelocity(double q[3]);
    
    // Test stuff
    
    double EvaluatedLength_;
    
    // Ground effects
    
    int DoGroundEffectsAnalysis_;
    
    // Induced Velocity calculation
    
    void InducedVelocity_(double xyz_p[3], double q[3]);
    
    // Search data structure
    
    int Searched_;
    
    double Distance_;
    
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
    int &TimeAnalysisType(void ) { return TimeAnalysisType_; };
    
    // Accesss to Vortex edge data

    int &Wing(void) { return Wing_; };
    int &Node(void) { return Node_; };
    int &ComponentID(void) { return ComponentID_; };
 
    void Setup(int NumSubVortices, double FarDist, VSP_NODE &Node1, VSP_NODE &Node2);
                               
    void InducedVelocity(double xyz_p[3], double q[3]);
    
    void InducedVelocity(double xyz_p[3], double q[3], double CorSize);

    void CalculateVelocityForSubVortex(VSP_EDGE &VortexEdge, double xyz_p[3], double q[3]);

    // Distance/2 between wakes at trailing edge
    
    double &Sigma(void) { return Sigma_; };

    // Core size at trailing edge
    
    double &CoreSize(void) { return CoreSize_; };
        
    // Trailing edge vector
    
    double &TEVec(int i) { return TEVec_[i]; };
    double *TEVec(void) { return TEVec_; };
    
    // Trailing edge location along span
    
    double &SoverB(void) { return SoverB_; };
    
    // Evaluation flag... for agglomeration multipole routine
    
    int &Evaluate(void) { return Evaluate_; };
    
    // Circulation strength

    double &Gamma(void) { return Gamma_[0]; };
    
    double &Gamma(int i) {  return Gamma_[i]; };
    
    // Update gamma values for sub vortices
    
    void UpdateGamma(void);    
     
    // Trailing edge node
    
    VSP_NODE &TE_Node(void) { return TE_Node_; };
     
    // Vortex length
    
    double Length(void) { return Length_; };
    
    // Number of sub vortices

    int NumberOfLevels(void) { return NumberOfLevels_; };
    
    int TotalNumberOfSubVortices(void) { return TotalNumberOfSubVortices_; };

    int NumberOfSubVortices(int i) { return NumberOfSubVortices_[i]; };
    
    int NumberOfSubVortices(void) { return NumberOfSubVortices_[1]; };
    
    // Access to each vortex edge on the finest level
    
    VSP_EDGE &VortexEdge(int i) { return VortexEdgeList_[1][i]; };
    
    VSP_EDGE &VortexEdge(int Level, int i) { return VortexEdgeList_[Level][i]; };
    
    // Centroid of each sub vortex
    
    double *xyz_c(int i) { return VortexEdgeList_[1][i].xyz_c(); };
    
    // Wake points list
    
    double S(int i) { return S_[0][i]; };

    double GammaScale(int i);
    
    // Velocity for each sub vortex
    
    double &U(int i) { return VortexEdgeVelocity_[1][i][0]; };
    double &V(int i) { return VortexEdgeVelocity_[1][i][1]; };
    double &W(int i) { return VortexEdgeVelocity_[1][i][2]; };

    double &U(int Level, int i) { return VortexEdgeVelocity_[Level][i][0]; };
    double &V(int Level, int i) { return VortexEdgeVelocity_[Level][i][1]; };
    double &W(int Level, int i) { return VortexEdgeVelocity_[Level][i][2]; };
    
    // Free stream velocity
    
    double &FreeStreamVelocity(int i) { return FreeStreamVelocity_[i];};
    
    double &LocalVelocityAtTrailingEdge(int i) { return LocalVelocityAtTrailingEdge_[i]; };
    
    double *VortexEdgeVelocity(int i) { return VortexEdgeVelocity_[1][i]; };
    
    void ZeroEdgeVelocities(void);
    
    void ProlongateEdgeVelocities(void);
    
    // Update the wake location
    
    double UpdateWakeLocation(void);     
    
    void ConvectWakeVorticity(int ConvectType);
    
    // Ground effects analysis flag
    
    int &DoGroundEffectsAnalysis(void) { return DoGroundEffectsAnalysis_; };

    // Bladed analysis flags
    
    int &RotorAnalysis(void) { return RotorAnalysis_; }; 
    double &BladeRPM(void) { return BladeRPM_; };
    
    // Unsteady analysis
    
    double &TimeStep(void) { return TimeStep_; };
    
    double &Vinf(void) { return Vinf_; };
 
    int &CurrentTimeStep(void) { return CurrentTimeStep_; };
    
    void CreateSearchTree(void) { CreateSearchTree_(); };

    void SaveVortexState(void);

    void WriteToFile(FILE *adb_file);
    void SkipReadInFile(FILE *adb_file);
    
    void UpdateGeometryLocation(double *TVec, double *OVec, QUAT &Quat, QUAT &InvQuat);

    int &Searched(void) { return Searched_; };
    
    double &Distance(void) { return Distance_; };

    SEARCH &Search(void) { return *Search_; };
    
    double &FarAwayRatio(void) { return FarAway_; };
            
};

#endif
