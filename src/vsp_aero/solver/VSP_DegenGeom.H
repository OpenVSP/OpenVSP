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
#include "VSP_Surface.H"
#include "VSP_Agglom.H"
#include "RotorDisk.H"

#define WING_SURFACE 1
#define BODY_SURFACE 2  

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

    // Body Data
    
    int NumberOfBodies_;

    // Rotor Disk Data

    int NumberOfRotors_;
    ROTOR_DISK *RotorDisk_;

    // Surface Data
    
    int NumberOfSurfaces_;
    
    VSP_SURFACE *VSP_Surface_;
    
    // Merged global mesh
    
    int NumberOfGridLevels_;
    VSP_GRID **Grid_;
    
    // Agglomeration routines
    
    void AgglomerateMeshes(void);

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

    // Access to data
    
    int NumberOfComponents(void) { return NumberOfComponents_; };
    
    int NumberOfWings(void) { return NumberOfWings_; };

    int NumberOfBodies(void) { return NumberOfBodies_; };
 
    int NumberOfSurfaces(void) { return NumberOfSurfaces_; };
    
    VSP_SURFACE &VSP_Surface(int i) { return VSP_Surface_[i]; };
    
    int NumberOfGridLevels(void) { return NumberOfGridLevels_; };
    VSP_GRID &Grid(void) { return *(Grid_[0]); };
    VSP_GRID &Grid(int i) { return *(Grid_[i]); };

    int NumberOfRotors(void) { return NumberOfRotors_; };
    ROTOR_DISK &RotorDisk(int i) { return RotorDisk_[i]; };

 
};

#endif
