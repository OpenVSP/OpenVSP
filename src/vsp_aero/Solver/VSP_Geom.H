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

#include "START_NAME_SPACE.H"

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

    // Mesh the geometry
    
    void MeshGeom(void);
    
    // Try and figure out if this a panel or VLM mesh
    
    void DetermineModelType(void);

    // Update the coarse mesh based on the finer mesh update
    
    void UpdateCoarseMesh(VSP_GRID &FineGrid, VSP_GRID &CoarseGrid);
    
    // Bounding Box data
    
    void CreateComponentBBoxData(void);
            
public:

    // Constructor, Destructor, Copy

    VSP_GEOM(void);
   ~VSP_GEOM(void);
    VSP_GEOM(const VSP_GEOM &VSP_Geom);

    // Copy function

    VSP_GEOM& operator=(const VSP_GEOM &VSP_Geom);
    
    // Read in the VSP degenerate geometry file
    
    void ReadFile(char *FileName, int& ModelType, int& SurfaceType);
    
    /** Load in a FEM (beam method) FEM deformation file **/

    int &LoadDeformationFile(void) { return LoadDeformationFile_; };    
 
    /** Load in FEM (beam data) deformation data file **/
    
    void LoadFEMDeformationData(char *FileName);

    /** Number of Components **/
    
    int NumberOfComponents(void) { return NumberOfComponents_; };
    
    /** Number of degen wings **/
    
    int NumberOfDegenWings(void) { return NumberOfDegenWings_; };

    /** Number of degen bodies **/
    
    int NumberOfDegenBodies(void) { return NumberOfDegenBodies_; };

    /** Number of surfaces **/
    
    int NumberOfSurfaces(void) { return NumberOfSurfaces_; };
    
    /** Number of surface patches **/
        
    int NumberOfSurfacePatches(void) { return NumberOfSurfacePatches_; };
    
    /** Access to the VSP_SURFACE object list **/
    
    VSP_SURFACE &VSP_Surface(int i) { return VSP_Surface_[i]; };

    /** Bounding box for each component **/
    
    BBOX &BBoxForComponent(int i) { return BBoxForComponent_[i]; };
    
    /** Number of agglomeration levels **/
       
    int NumberOfGridLevels(void) { return NumberOfGridLevels_; };
    
    /** Access to the finest, user supplied grid **/
    
    VSP_GRID &Grid(void) { return *(Grid_[0]); };
    
    /** Access to the i'th level grid **/
    
    VSP_GRID &Grid(int i) { return *(Grid_[i]); };
    
    /** Number of rotors, aka actuator disks **/
    
    int NumberOfRotors(void) { return NumberOfRotors_; };
    
    /** Rotor disk list **/
    
    ROTOR_DISK &RotorDisk(int i) { return RotorDisk_[i]; };
    
    /** Flag that we are doing a symmetry plane analysis... **/
    
    int &DoSymmetryPlaneSolve(void) { return DoSymmetryPlaneSolve_; };
    
    /** Flag that we are doing a ground effects analysis **/
    
    int &DoGroundEffectsAnalysis(void) { return DoGroundEffectsAnalysis_; };
    
    /** Vehicle rotation angle vector... we rotate about this vector... use for ground effect modeling
     * ... as we rotate the vehicle for AoA, Beta...  **/
    
    VSPAERO_DOUBLE &VehicleRotationAngleVector(int i) { return VehicleRotationAngleVector_[i]; };
    
    /** Vehicle rotation origin ... used in ground effects solve ***/
    
    VSPAERO_DOUBLE &VehicleRotationAxisLocation(int i) { return VehicleRotationAxisLocation_[i]; };
    
    /** Height above the ground for ground effects model **/
    
    VSPAERO_DOUBLE &HeightAboveGround(void) { return HeightAboveGround_; };
    
    /** Update the mesh ... user has updated xyz points on the mesh, so we need to update
     * all the mesh dependent stuff... **/

    void UpdateMeshes(void);

};

#include "END_NAME_SPACE.H"

#endif
