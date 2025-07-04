//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef BOUNDARY_CONDITION_DATA_H
#define BOUNDARY_CONDITION_DATA_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.H"
#include "quat.H"

#include "START_NAME_SPACE.H"

class BOUNDARY_CONDITION_DATA {

private:

    // Hight Lift Data
    
    int HasHightLiftSlat_;    
    
    double HighLiftSlatPercentage_;
    double HighLiftSlatMachDeploy_;
    
    
public:

    BOUNDARY_CONDITION_DATA(void);
   ~BOUNDARY_CONDITION_DATA(void);
    BOUNDARY_CONDITION_DATA(const BOUNDARY_CONDITION_DATA &BoundaryConditionData);

    BOUNDARY_CONDITION_DATA& operator=(const BOUNDARY_CONDITION_DATA &BoundaryConditionData);

    /** Indicate that the lifting surface has high lift data **/
    
    int &HasHightLiftSlat(void) { return HasHightLiftSlat_; };
    
    /** Percent of chord for the leading edge slat... acutally a fraction from 0 to 1. **/
    
    double &HighLiftSlatPercentage(void) { return HighLiftSlatPercentage_; };
    
    /** Mach number, below which, the slats are deployed **/
    
    double &HighLiftSlatMachDeploy(void) { return HighLiftSlatMachDeploy_; };
     
};

#include "END_NAME_SPACE.H"

#endif

