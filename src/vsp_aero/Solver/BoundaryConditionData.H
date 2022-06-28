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
    
    VSPAERO_DOUBLE HighLiftSlatPercentage_;
    VSPAERO_DOUBLE HighLiftSlatMachDeploy_;
    
    
public:

    BOUNDARY_CONDITION_DATA(void);
   ~BOUNDARY_CONDITION_DATA(void);
    BOUNDARY_CONDITION_DATA(const BOUNDARY_CONDITION_DATA &BoundaryConditionData);

    BOUNDARY_CONDITION_DATA& operator=(const BOUNDARY_CONDITION_DATA &BoundaryConditionData);

    void Size(int NumberOfSurfaces);

    int &HasHightLiftSlat(void) { return HasHightLiftSlat_; };
    
    VSPAERO_DOUBLE &HighLiftSlatPercentage(void) { return HighLiftSlatPercentage_; };
    VSPAERO_DOUBLE &HighLiftSlatMachDeploy(void) { return HighLiftSlatMachDeploy_; };
     
};

#include "END_NAME_SPACE.H"

#endif

