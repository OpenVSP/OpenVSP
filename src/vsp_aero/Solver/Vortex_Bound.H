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

#include "START_NAME_SPACE.H"

// Definition of the VORTEX_BOUND class

class VORTEX_BOUND {

private:

    void init(void);
     
    int Verbose_;

    int TimeAccurate_;
    
    int TimeAnalysisType_;
    
    int CurrentTimeStep_;

    VSPAERO_DOUBLE TimeStep_;
    
    // Starting vortex list
    
    int NumberOfStartingVortices_;
    
    VSP_EDGE *BoundVortexList_;
    
    VSP_EDGE &BoundVortexList(int i) { return BoundVortexList_[i]; };
    
    // Starting vortex gammas
    
    VSPAERO_DOUBLE *Gamma_;
 
    // Induced Velocity calculation
    
    VSPAERO_DOUBLE CoreSize_;

    void InducedVelocity_(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);
    
    void InducedVelocity_(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE CoreSize);
    
    // Initial span
    
    VSPAERO_DOUBLE InitialSpan_;

public:

    // Constructor, Destructor, Copy

    VORTEX_BOUND(void);
   ~VORTEX_BOUND(void);
    VORTEX_BOUND(const VORTEX_BOUND &Trailing_Vortex);

    // Copy function

    VORTEX_BOUND& operator=(const VORTEX_BOUND &VortexBound);
    VORTEX_BOUND& operator+=(const VORTEX_BOUND &VortexBound);
    
    /** Given 2 trailing vortex structures, set up bound vortex structure 
     * ... this structure contains the shed, bound, vortices, from the
     * trailing edge at previous time steps... for a steady solution
     * these bound vortices would have zero strength. For an unsteady
     * problem they contain the time history of the lift evolution 
     **/

    void Setup(VORTEX_TRAIL &Trail1, VORTEX_TRAIL &Trail2);
                    
    /** Update bound vortex locations as the wake is shed at each time step **/
    
    void UpdateGeometryLocation(VORTEX_TRAIL &Trail1, VORTEX_TRAIL &Trail2);

    /** Set the Mach number for the bound vortex list **/
    
    void SetMachNumber(VSPAERO_DOUBLE Mach);
    
    /** Update the bound vortex circulation strengths **/

    void UpdateGamma(void);
    
    /** Calculate the bound vortex induced velocities **/
    
    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3]);
    
    /** Calculate the bound vortex induced velocities ... assuming a finite core size model **/
    
    void InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE CorSize);
   
    /** Access to the bound vortex strengths at finest level **/
     
    VSPAERO_DOUBLE &Gamma(int i) { return Gamma_[i]; };
     
    /** Time accurate flag **/
    
    int &TimeAccurate(void) { return TimeAccurate_; };
    
    /** Time analysis type flag **/
    
    int &TimeAnalysisType(void) { return TimeAnalysisType_; };

    /** The current global time step, used in the core size evolution calculations **/
    
    int &CurrentTimeStep(void) { return CurrentTimeStep_; };
    
    /** Analysis time step **/
    
    VSPAERO_DOUBLE &TimeStep(void) { return TimeStep_; };

};

#include "END_NAME_SPACE.H"

#endif
