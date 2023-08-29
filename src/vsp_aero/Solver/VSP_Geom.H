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
#include "VSP_Agglom.H"
#include "RotorDisk.H"
#include "BoundaryConditionData.H"
#include "ControlSurface.H"

#include "START_NAME_SPACE.H"

#define   VLM_MODEL 1
#define PANEL_MODEL 2

#define CART3D_SURFACE       3
#define VSPGEOM_SURFACE      4
#define THIN_SURFACE        10
#define THICK_SURFACE       20

#define SYM_X 1
#define SYM_Y 2
#define SYM_Z 3

// Simple kutta node structure

class KUTTA_NODE {

public:

   int IsKuttaNode;
   int Node;
   int Edge1;
   int Edge2;

};

// Definition of the VSP_GEOM_H class

class VSP_GEOM {

private:

    // Debug
    
    int Verbose_;
    
    // Function data
    
    void init(void);
    
    // Max number of grid levels
    
    int MaxNumberOfGridLevels_;
    
    // Model type
    
    int ModelType_;
    
    // Surface type
    
    int SurfaceType_;
    
    // Symmetry flag
    
    int DoSymmetryPlaneSolve_;

    // Component name
    
    char ComponentName_[2000];
    
    // Number of surface patches
    
    int NumberOfSurfaces_;    

    // Component ID for surface patch
    
    int *ComponentIDForSurface_;
    
    // Number of components
    
    int NumberOfComponents_;
    
    // Component ID for component
    
    int *ComponentIDForComponent_;
    
    // Surface patch name
    
    char **SurfaceNameList_;

    // Surface is thick surface flag
    
    int *SurfaceIsThick_;
    
    // Component is thick surface flag
    
    int *ComponentIsThick_;

    // Number of kutta nodes provided by vspgeom file
        
    int NumberOfKuttaNodes_;
    
    // Kutta node list provided by vpsgeom file
        
    int *KuttaNodeList_;
    
    // Number of thin surfaces
    
    int NumberOfThinSurfaces_;
    
    // Number of thick surfaces
    
    int NumberOfThickSurfaces_;
    
    // Number of thin components
    
    int NumberOfThinComponents_;
    
    // Number of thick components
    
    int NumberOfThickComponents_;  
    
    // Boundary condition data
    
    BOUNDARY_CONDITION_DATA *BoundaryConditionForSurface_;
            
    // Rotor Disk Data
    
    int NumberOfRotors_;
    
    ROTOR_DISK *RotorDisk_;

    // Bounding Box data for each component
    
    BBOX *BBoxForComponent_;
        
    // Merged global mesh
    
    int NumberOfGridLevels_;
    
    VSP_GRID **Grid_;
    
    // Possible simplified mesh provided by user
    
    VSP_GRID *GridC_;
    
    // Possible fine, original mesh provided by user
    
    VSP_GRID *GridF_;
    
    int InputMeshIsMixedPolys_;
    
    // Node to tri list
        
    int *NumberOfTrisForNode_;                    
    
    int **NodeToTriList_;

    // Control surface information
    
    int NumberOfControlSurfaces_;
    
    CONTROL_SURFACE *ControlSurface_;
        
    // I/O
    
    void Read_CART3D_File(char *FileName);
    
    void Read_VSPGEOM_File(char *FileName);
    
    void ReadCart3DDataFromFile(char *Name, FILE *CART3D_File, FILE *TKEY_File);
    
    void ReadVSPGeomDataFromFile(char *Name, FILE *VSPGeom_File, FILE *VKEY_File);

    void ReadControlSurfaceInformation(char *FileName);
    
    void RotateGeometry_About_Y_Axis(void);

    void FindSharpEdges(int NumberOfSharpNodes, int *SharpNodeList);
        
    int SurfaceAtNodeIsConvex(int Node);
        
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
    
    // Write out cart3d tri formatted file for the finest mesh... typicall used
    // for writing out a whacked mesh with the whacky tris marked as surface 999
    
    void WriteOutTriMesh(void);
    
    // Sanitize meshes with thin surfaces
    
    void SanitizeThinMeshes(void);
        
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

    /** Component name **/
    
    char* ComponentName(void) { return ComponentName_; };
    
    /** Surface type **/
    
    int SurfaceType(void) { return SurfaceType_; };
    
    /** Number of surface patches **/

    int NumberOfSurfaces(void) { return NumberOfSurfaces_; };    

    /** Component ID for surfac patch i **/
    
    int ComponentIDForSurface(int i) { return ComponentIDForSurface_[i]; };
        
    /** Number of components **/
    
    int NumberOfComponents(void) { return NumberOfComponents_; };
    
    /** Component ID for component **/
    
    int ComponentIDForSurfaceComponent(int i) { return ComponentIDForComponent_[i]; };
   
    /** Name for surface patch i **/
    
    char *SurfaceNameList(int i) { return SurfaceNameList_[i]; };
    
    /** Surface is thick surface flag **/
    
    int SurfaceIsThick(int i) { return SurfaceIsThick_[i]; };
    
    /** Component is thick surface flag **/
    
    int ComponentIsThick(int i) { return ComponentIsThick_[i]; };
    
    /** Number of thin surfaces **/
    
    int NumberOfThinSurfaces(void) { return NumberOfThinSurfaces_; };
    
    /** Number of thick surfaces **/
    
    int NumberOfThickSurfaces(void) { return NumberOfThickSurfaces_; };
    
    /** Number of thin components **/
    
    int NumberOfThinComponents(void) { return NumberOfThinComponents_; };
    
    /** Number of thick components **/
    
    int NumberOfThickComponents(void) { return NumberOfThickComponents_; };    

    /** Bounding box for each component **/
    
    BBOX &BBoxForComponent(int i) { return BBoxForComponent_[i]; };
    
    /** Number of agglomeration levels **/
       
    int NumberOfGridLevels(void) { return NumberOfGridLevels_; };
        
    /** Boundary condition data for surface i **/
    
    BOUNDARY_CONDITION_DATA &BoundaryConditionForSurface(int i) { return BoundaryConditionForSurface_[i]; };
          
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

    /** Number of control surfaces **/
    
    int NumberOfControlSurfaces(void) { return NumberOfControlSurfaces_; };
    
    /** Control surface data **/
    
    CONTROL_SURFACE &ControlSurface(int i) { return ControlSurface_[i]; };
            
    /** Update the mesh ... user has updated xyz points on the mesh, so we need to update
     * all the mesh dependent stuff... **/

    void UpdateMeshes(void);

};

#include "END_NAME_SPACE.H"

#endif
