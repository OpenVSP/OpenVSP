//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef CONTROL_SURFACE_GROUP_H
#define CONTROL_SURFACE_GROUP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"

#include "START_NAME_SPACE.H"

// Definition of the CONTROL_SURFACE_GROUP class

class CONTROL_SURFACE_GROUP {

private:

    // Control group name
    
    char Name_[2000];
    
    // List of control surfaces
    
    int NumberOfControlSurfaces_;
    
    char **ControlSurface_Name_;
    
    // Control surface deflection sign
    
    VSPAERO_DOUBLE *ControlSurface_DeflectionDirection_;
    
    // Control surface deflection
    
    VSPAERO_DOUBLE ControlSurface_DeflectionAngle_;
    
public:

    // Constructor, Destructor, Copy

    CONTROL_SURFACE_GROUP(void);
   ~CONTROL_SURFACE_GROUP(void);
    CONTROL_SURFACE_GROUP(const CONTROL_SURFACE_GROUP &ControlSurfaceGroup);
    CONTROL_SURFACE_GROUP& operator=(const CONTROL_SURFACE_GROUP &ControlSurfaceGroup);
    
    // Name
    
    char *Name(void) { return Name_; };
    
    // Size the list
    
    void SizeList(int NumberOfControlSurfaces);
    
    // Number of control surface
    
    int NumberOfControlSurfaces(void) { return NumberOfControlSurfaces_; };
    
    // Control surface name
    
    char *ControlSurface_Name(int i) { return ControlSurface_Name_[i]; };
    
    // Control surface deflection sign
    
    VSPAERO_DOUBLE &ControlSurface_DeflectionDirection(int i) { return ControlSurface_DeflectionDirection_[i]; };
    
    // Control surface deflection angle
    
    VSPAERO_DOUBLE &ControlSurface_DeflectionAngle(void) { return ControlSurface_DeflectionAngle_; };
    
};

#include "END_NAME_SPACE.H"

#endif
