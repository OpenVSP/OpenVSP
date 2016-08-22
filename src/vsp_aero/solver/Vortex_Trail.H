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
#include "VSP_Edge.H"

// Definition of the VORTEX_TRAIL class

class VORTEX_TRAIL {

private:

    void init(void);
     
    int Verbose_;
 
    // Wing and edge, or trailing node this vortex belongs to
    
    int Wing_;
    
    int Node_;
    
    // Wake points list
    
    double Length_;
    
    double FarDist_;
    
    double *S_;
    
    int NumberOfNodes_;
    
    VSP_NODE TE_Node_;
    
    VSP_NODE *NodeList_;

    // List of trailing vortices
    
    int NumberOfLevels_;
    
    int *NumberOfSubVortices_;
    
    VSP_EDGE **VortexEdgeList_;

    VSP_EDGE *VortexEdgeList(int Level) { return VortexEdgeList_[Level]; };
  
    double **VortexEdgeVelocity_;
    
    double FreeStreamVelocity_[3];

    // Mach number
    
    static double Mach_;
    
    // Cut off distance
    
    double Sigma_;
    
    // Minimum Tolerance
    
    double Tolerance_;
    
    // Trailing edge vector
    
    double TEVec_[3];
    
    // Circulation strength
    
    int Evaluate_;

    double Gamma_;

    // Update gamma values for sub vortices
    
    void UpdateGamma(void);

    // Smooth out the trailing wake shape
    
    void Smooth(void);
    void SmoothVelocity(double *Velocity);
    void LimitVelocity(double q[3]);
    
    // Test stuff
    
    double EvaluatedLength_;
    
public:

    // Constructor, Destructor, Copy

    VORTEX_TRAIL(void);
   ~VORTEX_TRAIL(void);
    VORTEX_TRAIL(const VORTEX_TRAIL &Trailing_Vortex);

    // Copy function

    VORTEX_TRAIL& operator=(const VORTEX_TRAIL &Trailing_Vortex);
    
    // Accesss to Vortex edge data

    int &Wing(void) { return Wing_; };
    int &Node(void) { return Node_; };
 
    void Setup(int NumSubVortices, double FarDist, VSP_NODE &Node1, VSP_NODE &Node2);
                               
    void InducedVelocity(double xyz_p[3], double q[3]);
    
    void CalculateVelocityForSubVortex(VSP_EDGE &VortexEdge, double xyz_p[3], double q[3]);
 
    // Access to Mach number
    
    double &Mach(void) { return Mach_; };
    
    // Cut off distance
    
    double &Sigma(void) { return Sigma_; };
    
    // Trailing edge vector
    
    double &TEVec(int i) { return TEVec_[i]; };
    double *TEVec(void) { return TEVec_; };

    // Evaluation flag... for agglomeration multipole routine
    
    int &Evaluate(void) { return Evaluate_; };
    
    // Circulation strength
    
    double &Gamma(void) { return Gamma_; };
     
    // Trailing edge node
    
    VSP_NODE &TE_Node(void) { return TE_Node_; };
     
    // Vortex length
    
    double Length(void) { return Length_; };
    
    // Number of sub vortices

    int NumberOfSubVortices(int i) { return NumberOfSubVortices_[i]; };
    
    int NumberOfSubVortices(void) { return NumberOfSubVortices_[1]; };
    
    // Centroid of each sub vortex
    
    double *xyz_c(int i) { return VortexEdgeList_[1][i].xyz_c(); };
    
    // Velocity for each sub vortex
    
    double &U(int i) { return VortexEdgeVelocity_[i][0]; };
    double &V(int i) { return VortexEdgeVelocity_[i][1]; };
    double &W(int i) { return VortexEdgeVelocity_[i][2]; };
    
    double &Utmp(int i) { return VortexEdgeVelocity_[i][3]; };
    double &Vtmp(int i) { return VortexEdgeVelocity_[i][4]; };
    double &Wtmp(int i) { return VortexEdgeVelocity_[i][5]; };
        
    // Free stream velocity
    
    double &FreeStreamVelocity(int i) { return FreeStreamVelocity_[i];};
    
    double *VortexEdgeVelocity(int i) { return VortexEdgeVelocity_[i]; };
    
    void UpdateLocation(void);     
   
    void WriteToFile(FILE *adb_file);

};

#endif
