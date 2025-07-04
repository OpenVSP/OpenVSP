//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef GLOBAL_WAKE_LOOP_H
#define GLOBAL_WAKE_LOOP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "time.H"

#include "START_NAME_SPACE.H"

// Global loop class

class GLOBAL_WAKE_LOOP {

private:

    // Local Kutta edge for this vortex sheet
    
    int KuttaEdge_;
    
    // Global loop number
    
    int GlobalLoop_;
    
    // Global trailedge edge this wake loop extends from
    
    int GlobalTrailingEdge_;

    // Global Kutta stall equation number
    
    int KuttaStallEquationNumber_;
    
    // Global vortex stretching ratio equation number
    
    int VortexStretchingRatioEquationNumber_;

    // Stall factor
    
    double StallFactor_;

    // dStall factor for GMRES solve
    
    double dStallFactor_;
    
    // Vortex stretching ratio
    
    double VortexStretchingRatio_;
    
    // dVortex stretching ratio
    
    double dVortexStretchingRatio_;
    
    // Adjoint variable solution
    
    double Psi_;
     
public:

    // Constructor, Destructor, Copy

    GLOBAL_WAKE_LOOP(void);
   ~GLOBAL_WAKE_LOOP(void);
    GLOBAL_WAKE_LOOP(const GLOBAL_WAKE_LOOP &GlobalWakeLoop);
    GLOBAL_WAKE_LOOP& operator=(const GLOBAL_WAKE_LOOP &GlobalWakeLoop);
    
    // Local Kutta edge for this vortex sheet
    
    int &KuttaEdge(void) { return KuttaEdge_; };
        
    /** Global loop number of wake loop **/
    
    int &GlobalLoop(void) { return GlobalLoop_; };
    
    /** Global trailing edge number this wake loop comes from **/
    
    int &GlobalTrailingEdge(void) { return GlobalTrailingEdge_; };
    
    /** Global trailing stall equation number this wake loop is attached to **/
    
    int &KuttaStallEquationNumber(void) { return KuttaStallEquationNumber_; };      
    
    /** Global vortex stretching ratio equation number **/
    
    int &VortexStretchingRatioEquationNumber(void) { return VortexStretchingRatioEquationNumber_; };

    /** Stall factor **/
        
    double &StallFactor(void) { return StallFactor_; };

    /** dStall factor for GMRES solve **/
        
    double &dStallFactor(void) { return dStallFactor_; };

    // Vortex stretching ratio
    
    double &VortexStretchingRatio(void) { return VortexStretchingRatio_; };
    
    // dVortex stretching ratio
    
    double &dVortexStretchingRatio(void) { return dVortexStretchingRatio_; };

    /** Adjoint variable solution for vortex stretching ratio **/
    
    double &Psi(void) { return Psi_; };    
         
};

#include "END_NAME_SPACE.H"

#endif
