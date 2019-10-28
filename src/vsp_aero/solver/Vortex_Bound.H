//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VORTEX_BOUND_H
#define VORTEX_BOUND_H
#include "VSPAERO_OMP.H"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "time.H"
#include "VSP_Edge.H"
#include "Vortex_Trail.H"

// Definition of the VORTEX_BOUND class

class VORTEX_BOUND {

private:

    void init(void);
     
    int Verbose_;

    int TimeAccurate_;
    
    int TimeAnalysisType_;
    
    int CurrentTimeStep_;

    double TimeStep_;
    
    // Starting vortex list
    
    int NumberOfStartingVortices_;
    
    VSP_EDGE *BoundVortexList_;
    
    VSP_EDGE &BoundVortexList(int i) { return BoundVortexList_[i]; };
    
    // Starting vortex gammas
    
    double *Gamma_;
 
    // Induced Velocity calculation
    
    double CoreSize_;

    void InducedVelocity_(double xyz_p[3], double q[3]);
    
    void InducedVelocity_(double xyz_p[3], double q[3], double CoreSize);

public:

    // Constructor, Destructor, Copy

    VORTEX_BOUND(void);
   ~VORTEX_BOUND(void);
    VORTEX_BOUND(const VORTEX_BOUND &Trailing_Vortex);

    // Copy function

    VORTEX_BOUND& operator=(const VORTEX_BOUND &VortexBound);
    VORTEX_BOUND& operator+=(const VORTEX_BOUND &VortexBound);
    
    // Setup bound vortex data structure

    void Setup(VORTEX_TRAIL &Trail1, VORTEX_TRAIL &Trail2);
                    
    // Update bound vortex locations
    
    void UpdateGeometryLocation(VORTEX_TRAIL &Trail1, VORTEX_TRAIL &Trail2);

    // Induced velocities

    void UpdateGamma(void);
    
    void InducedVelocity(double xyz_p[3], double q[3]);
    
    void InducedVelocity(double xyz_p[3], double q[3], double CorSize);
   
    // Bound vortex strengths at finest level
     
    double &Gamma(int i) { return Gamma_[i]; };
     
    // Solution parameters
    
    int &TimeAccurate(void) { return TimeAccurate_; };
    
    int &TimeAnalysisType(void) { return TimeAnalysisType_; };

    int &CurrentTimeStep(void) { return CurrentTimeStep_; };
    
    double &TimeStep(void) { return TimeStep_; };

};

#endif
