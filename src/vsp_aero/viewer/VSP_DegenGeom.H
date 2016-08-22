//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_DEGEN_GEOM_H
#define VSP_DEGEN_GEOM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "VSP_Wing.H"
#include "VSP_Body.H"

#define WING_SURFACE   1
#define BODY_SURFACE   2  
#define CART3D_SURFACE 3

// Definition of the VSP_DEGEN_GEOM_H class

class VSP_DEGEN_GEOM {

private:

    // Debug
    
    int Verbose_;
    
    // Function data
    
    void init(void);
    
    // Geometry data
    
    int NumberOfComponents_;
    
    // Wing Data
    
    int NumberOfWings_;
    
    VSP_WING *VSP_Wing_;
    
    // Body Data
    
    int NumberOfBodies_;
    
    VSP_BODY *VSP_Body_;

public:

    // Constructor, Destructor, Copy

    VSP_DEGEN_GEOM(void);
   ~VSP_DEGEN_GEOM(void);
    VSP_DEGEN_GEOM(const VSP_DEGEN_GEOM &VSP_DegenGeom);

    // Copy function

    VSP_DEGEN_GEOM& operator=(const VSP_DEGEN_GEOM &VSP_DegenGeom);
    
    // Read in the VSP degenerate geometry file
    
    void ReadFile(char *FileName);
    
    // Meshing
    
    void MeshGeom(void);
    
    void WriteMeshFile(char *FileName);
    
    // Access to data
    
    int NumberOfComponents(void) { return NumberOfComponents_; };
    
    int NumberOfWings(void) { return NumberOfWings_; };
    
    VSP_WING &VSP_Wing(int i) { return VSP_Wing_[i]; };

    int NumberOfBodies(void) { return NumberOfBodies_; };
    
    VSP_BODY &VSP_Body(int i) { return VSP_Body_[i]; };

};

#endif
