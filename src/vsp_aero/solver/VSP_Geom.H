//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_GEOM_H
#define VSP_GEOM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "VSP_Surface.H"
#include "VSP_Agglom.H"
#include "RotorDisk.H"

#define   VLM_MODEL 1
#define PANEL_MODEL 2

#define SYM_X 1
#define SYM_Y 2
#define SYM_Z 3

// Definition of the VSP_GEOM_H class

class VSP_GEOM {

private:

    // Debug
    
    int Verbose_;
    
    // Function data
    
    void init(void);
    
    // Model type
    
    int ModelType_;
    
    // Surface type
    
    int SurfaceType_;
    
    // Symmetry flag
    
    int DoSymmetryPlaneSolve_;
    
    // Geometry data
    
    int NumberOfComponents_;
    
    // Wing Data
    
    int NumberOfDegenWings_;

    // Body Data
    
    int NumberOfDegenBodies_;
    
    // CART3D data
    
    int NumberOfCart3dSurfaces_;
    
    // Blade Element data
    
    int DoBladeElementAnalysis_;
    int NumberOfBladeElementSurfaces_;
    
    // Rotor Disk Data
    
    int NumberOfRotors_;
    ROTOR_DISK *RotorDisk_;

    // Surface Data
    
    int NumberOfSurfaces_;
    
    int NumberOfSurfacePatches_;
   
    VSP_SURFACE *VSP_Surface_;
    
    // Bounding Box data for each component
    
    BBOX *BBoxForComponent_;
        
    // Merged global mesh
    
    int NumberOfGridLevels_;
    VSP_GRID **Grid_;
    
    // Agglomeration routines
    
    void AgglomerateMeshes(void);

    // I/O
    
    void Read_CART3D_File(char *FileName);
    void Read_VSPGEOM_File(char *FileName);
    void Read_VSP_Degen_File(char *FileName);
    
    // FEM Analysis
    
    int LoadDeformationFile_;
    
    // Ground effects analysis
    
    int DoGroundEffectsAnalysis_;
    
    VSPAERO_DOUBLE VehicleRotationAngleVector_[3];
    
    VSPAERO_DOUBLE VehicleRotationAxisLocation_[3];
    
    VSPAERO_DOUBLE HeightAboveGround_;
    
    // Control surface routines
    
    void FindControlSurfaceVortexLoops(void);
    
    // Coarse grid information routines
    
    void OutputCoarseGridInfo(void);
    
    int CalculateNumberOfFineLoops(int Level, VSP_LOOP &Loop);

public:

    // Constructor, Destructor, Copy

    VSP_GEOM(void);
   ~VSP_GEOM(void);
    VSP_GEOM(const VSP_GEOM &VSP_Geom);

    // Copy function

    VSP_GEOM& operator=(const VSP_GEOM &VSP_Geom);
    
    // Read in the VSP degenerate geometry file
    
    void ReadFile(char *FileName, int& ModelType, int& SurfaceType);
    
    // FEM

    int &LoadDeformationFile(void) { return LoadDeformationFile_; };    
 
    void LoadFEMDeformationData(char *FileName);
    void FEMDeformGeometry(void);    
    
    // Force blade element analysis of rotors
    
    int &DoBladeElementAnalysis(void) { return DoBladeElementAnalysis_; };

    // Meshing
    
    void MeshGeom(void);
    
    void DetermineModelType(void);
    
    // Access to data
    
    int NumberOfComponents(void) { return NumberOfComponents_; };
    
    int NumberOfDegenWings(void) { return NumberOfDegenWings_; };

    int NumberOfDegenBodies(void) { return NumberOfDegenBodies_; };

    int NumberOfCart3dSurfaces(void) { return NumberOfCart3dSurfaces_; };
    
    int NumberOfBladeElementSurfaces(void) { return NumberOfBladeElementSurfaces_; };
 
    int NumberOfSurfaces(void) { return NumberOfSurfaces_; };
    
    VSP_SURFACE &VSP_Surface(int i) { return VSP_Surface_[i]; };
    
    // Bounding Box data
    
    void CreateComponentBBoxData(void);
    
    BBOX &BBoxForComponent(int i) { return BBoxForComponent_[i]; };
       
    int NumberOfGridLevels(void) { return NumberOfGridLevels_; };
    VSP_GRID &Grid(void) { return *(Grid_[0]); };
    VSP_GRID &Grid(int i) { return *(Grid_[i]); };
    
    int NumberOfRotors(void) { return NumberOfRotors_; };
    ROTOR_DISK &RotorDisk(int i) { return RotorDisk_[i]; };
    
    int &DoSymmetryPlaneSolve(void) { return DoSymmetryPlaneSolve_; };
    
    int &DoGroundEffectsAnalysis(void) { return DoGroundEffectsAnalysis_; };
    
    VSPAERO_DOUBLE &VehicleRotationAngleVector(int i) { return VehicleRotationAngleVector_[i]; };
    
    VSPAERO_DOUBLE &VehicleRotationAxisLocation(int i) { return VehicleRotationAxisLocation_[i]; };
    
    VSPAERO_DOUBLE &HeightAboveGround(void) { return HeightAboveGround_; };
    
    
};

#endif
