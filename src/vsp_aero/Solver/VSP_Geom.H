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
#include "Vortex_Trail.H"
#include "Vortex_Sheet.H"
#include "VSP_Agglom.H"
#include "RotorDisk.H"
#include "BoundaryConditionData.H"
#include "ComponentGroup.H"
#include "ControlSurface.H"
#include "CharSizes.H"

#include "START_NAME_SPACE.H"

#define   VLM_MODEL 1
#define PANEL_MODEL 2

#define CART3D_SURFACE       3
#define VSPGEOM_SURFACE      4
#define THIN_SURFACE        10
#define THICK_SURFACE       20

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
    
    // User specified mglevel to perform solve on
    
    int SolveOnMGLevel_;
    
    // Max number of grid levels
    
    int MaxNumberOfGridLevels_;
    
    // Model type
    
    int ModelType_;
    
    // Surface type
    
    int SurfaceType_;
    
    // Symmetry flag
    
    int DoSymmetryPlaneSolve_;

    // Component name
    
    char ComponentName_[MAX_CHAR_SIZE];
    
    // Number of surfaces
    
    int NumberOfSurfaces_;    

    // Component ID for surface
    
    int *ComponentIDForSurface_;
    
    // OpenVSP Component ID for surface
    
    int *OpenVSP_ComponentIDForSurface_;
    
    // VSP Surface ID for Surface //
    
    int *VSPSurfaceIDForSurface_;
    
    // Number of components
    
    int NumberOfComponents_;
    
    // Component ID for component
    
    int *ComponentIDForComponent_;

    // Is component fixed or moving
        
    int *GeometryComponentIsFixed_;    

    // ID # for the group the user has put each component into (from .groups file)
    
    int *GeometryGroupID_;
        
    // Surface name
    
    char **SurfaceNameList_;
    
    // Surface GID 
    
    char **SurfaceGIDList_;

    // Surface is thick surface flag
    
    int *SurfaceIsThick_;
    
    // Surface thick/plate flag
    
    int *SurfaceDegenType_;
    
    // Component is thick surface flag
    
    int *ComponentIsThick_;

    // Number of kutta nodes provided by vspgeom file
        
    int NumberOfKuttaNodes_;
    
    // Kutta node list provided by vpsgeom file
        
    int *KuttaNodeList_;

    // Vortex Sheets
    
    int NumberOfVortexSheets_;
    
    VORTEX_SHEET *VortexSheet_;
    
    // Number of wake residual equations
    
    int NumberOfWakeResidualEquations_;
    
    // Number of stall residual equations
    
    int NumberOfStallResidualEquations_;
    
    // Number of vortex stretching ratio equations
    
    int NumberOfVortexStretchingRatioEquations_;
    
    // There are rotors...
    
    int ThereAreRotors_;
    
    // Solver flags
    
    int TimeAccurate_;
    
    int TimeAnalysisType_;
    
    int DoVortexStretching_;
    
    int OptimizationSolve_;
    
    int FlowIs2D_;
    
    int NumberOfWakeTrailingNodes_;

    double CoreSizeFactor_;
    
    double Vinf_;
    
    double WakeRelax_;
      
    int SetFarFieldDist_;
    
    double FarFieldDist_;
            
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
    
    // Tri mesh provided by user
    
    VSP_GRID *GridF_;
    
    // Flag that input mesh is made up of mixed polygons
    
    int InputMeshIsMixedPolys_;
    
    // Coarse wake mesh
    
    VSP_GRID *GridWC_;
    
    // Fine Wake mesh

    VSP_GRID *GridWF_;
    
    // Node to tri list
        
    int *NumberOfTrisForNode_;                    
    
    int **NodeToTriList_;
    
    // Incident kutta edges on a node list
    
    int *IncidentKuttaEdgesOnNode_;

    // Control surface information
    
    int NumberOfControlSurfaces_;
    
    CONTROL_SURFACE *ControlSurface_;
    
    // A user defined groups file was read in
    
    int UserSuppliedGroupsFile_;
    
    // Number of component groups
    
    int NumberOfComponentGroups_;
    
    // All components are fixed relative to each other
    
    int AllComponentsAreFixed_;
    
    // There is relative motion between components
    
    int ThereIsRelativeComponentMotion_;
    
    // Component group data
    
    COMPONENT_GROUP *ComponentGroupList_;
            
    // I/O
    
    void Read_CART3D_File(char *FileName);
    
    void Read_VSPGEOM_File(char *FileName);
    
    void ReadCart3DDataFromFile(char *Name, FILE *CART3D_File, FILE *TKEY_File);
    
    void ReadVSPGeomDataFromFile(char *Name, FILE *VSPGeom_File, FILE *VKEY_File);

    void ReadControlSurfaceInformation(char *FileName);
    
    void ReadGroupFile(char *FileName);
    
    void AssociateWakesWithComponentGroups(void);
        
    void RotateGeometry_About_Y_Axis(void);

    void FindSharpEdges(int NumberOfSharpNodes, int *SharpNodeList);
        
    int SurfaceAtNodeIsConvex(int Node);
        
    // FEM Analysis
    
    int LoadDeformationFile_;
    
    // Ground effects analysis
    
    int DoGroundEffectsAnalysis_;
    
    double VehicleRotationAngleVector_[3];
    
    double VehicleRotationAxisLocation_[3];
    
    double HeightAboveGround_;

    // Coarse grid information routines
    
    void OutputCoarseGridInfo(void);
    
    // Calculate the number of fine grid loops
    
    int CalculateNumberOfFineLoops(int Level, VSP_LOOP &Loop); 

    // Mesh the geometry
    
    void MeshGeom(void);
    
    // Write out cart3d tri formatted file for the finest mesh... typicall used
    // for writing out a whacked mesh with the whacky tris marked as surface 999
    
    void WriteOutTriMesh(void);
    
    // Sanitize meshes with thin surfaces.. this basically generates unique edges for 
    // every thin surfaces. So any thin-thick, or thin-thin intersections will have
    // shared nodes but unique edges for each surface. This guarantees that all edges
    // only point to 2 loops... keeping the topology sane.
    
    void SanitizeThinMeshes(void);
    
    // Mark all edges and nodes on intersections. This is a bit more complicted when
    // we have thin-thick, or thin-thin intersections.
    
    void MarkSurfaceSurfaceIntersections(void);    
        
    // Try and figure out if this a panel or VLM mesh
    
    void DetermineModelType(void);

    // Update the coarse mesh based on the finer mesh update
    
    void UpdateCoarseMesh(VSP_GRID &FineGrid, VSP_GRID &CoarseGrid);
    
    // Bounding Box data
    
    void CreateComponentBBoxData(void);
    
    // Create wake data structures
    
    void CreateVortexSheets(void);
    
    // Create wake grids
    
    void CreateWakeGrids(void);

    // Find and store the kutta edges in the vortex sheet data structures
    
    void StoreWakeKuttaEdges(void);
    
    // Restrict wake equation data to the coarse grid
    
    void RestrictWakeEquations(int Level);
    
    // Merge 2 grids into 1
    
    VSP_GRID *MergeGrids(VSP_GRID *Grid1, VSP_GRID *Grid2, int CoarseGridLoopOffSet);

    // Initialize
    
    void init(void);    

    // Update the surface and possible the meshes 

    void UpdateMeshes(int UpdateWakes);

    // Update the vortex core sizes
    
    void UpdateVortexCoreSize(void);
    
public:

    // Constructor, Destructor, Copy

    VSP_GEOM(void);
   ~VSP_GEOM(void);
    VSP_GEOM(const VSP_GEOM &VSP_Geom);

    // Copy function

    VSP_GEOM& operator=(const VSP_GEOM &VSP_Geom);
    
    /** Set the number of trailing wake nodes... this must be done before reading in the mesh file **/
    
    void SetNumberOfWakeTrailingNodes(int NumberOfNodes) { NumberOfWakeTrailingNodes_ = NumberOfNodes; };

    /** Number of trailing wake nodes... this must be done before reading in the mesh file **/
    
    int NumberOfWakeTrailingNodes(void) { return NumberOfWakeTrailingNodes_; };
      
    /** Set the grid solve level **/
    
    int &SolveOnMGLevel(void) { return SolveOnMGLevel_; };
    
    /** Read in the VSPGEOM or CART3D geometry file **/
    
    void ReadFile(char *FileName);

    /** Component name **/
    
    char* ComponentName(void) { return ComponentName_; };

    /** Model type **/
    
    int ModelType(void) { return ModelType_; };
        
    /** Surface type **/
    
    int SurfaceType(void) { return SurfaceType_; };
    
    /** Number of surface patches **/

    int NumberOfSurfaces(void) { return NumberOfSurfaces_; };    

    /** Component ID for surface patch i **/
    
    int ComponentIDForSurface(int i) { return ComponentIDForSurface_[i]; };
    
    /** VSP Component ID for surface patch i... it's 1 less than my list...  **/
    
    int VSPComponentIDForSurface(int i) { return ComponentIDForSurface_[i] - 1; };
            
    /** VSP Surface ID for Surface **/
    
    int VSPSurfaceIDForSurface(int i) { return VSPSurfaceIDForSurface_[i]; };
 
    /** Number of components **/
    
    int NumberOfComponents(void) { return NumberOfComponents_; };
    
    /** Component ID for component **/
    
    int ComponentIDForComponent(int i) { return ComponentIDForComponent_[i]; };

    /**  Is component fixed or moving **/
        
    int &GeometryComponentIsFixed(int i) { return GeometryComponentIsFixed_[i]; };    

    /**  ID # for the group the user has put each component into (from .groups file) **/
    
    int &GeometryGroupID(int i) { return GeometryGroupID_[i]; };
       
    /** Name for surface i **/
    
    char *SurfaceNameList(int i) { return SurfaceNameList_[i]; };
    
    /** GID for surface i **/
    
    char *SurfaceGIDList(int i) { return SurfaceGIDList_[i]; };
    
    /** Surface is thick surface flag **/
    
    int SurfaceIsThick(int i) { return SurfaceIsThick_[i]; };
    
    /** Surface Degen typ **/
    
    int VSPSurfaceDegenType(int i) { return SurfaceDegenType_[i]; };
    
    /** Component is thick surface flag **/
    
    int ComponentIsThick(int i) { return ComponentIsThick_[i]; };
    
    /** Number of kutta nodes provided by vspgeom file **/
        
    int NumberOfKuttaNodes(void) { return NumberOfKuttaNodes_; };
    
    /** Kutta node list provided by vpsgeom file **/
        
    int KuttaNodeList(int i) { return KuttaNodeList_[i]; };

    /** Number of vortex sheets **/
    
    int NumberOfVortexSheets(void) { return NumberOfVortexSheets_; };
    
    /** Vortex sheet data **/
    
    VORTEX_SHEET &VortexSheet(int i) { return VortexSheet_[i]; };
    
    /** Number of wake residual equations **/
    
    int NumberOfWakeResidualEquations(void) { return NumberOfWakeResidualEquations_; };
        
    /** Number of stall residual equations **/
    
    int NumberOfStallResidualEquations(void) { return NumberOfStallResidualEquations_; };
    
    /** Number of vortex stretching ratio equations **/
    
    int NumberOfVortexStretchingRatioEquations(void) { return NumberOfVortexStretchingRatioEquations_; };
            
    /** Number of thin surfaces **/
    
    int NumberOfThinSurfaces(void) { return NumberOfThinSurfaces_; };
    
    /** Number of thick surfaces **/
    
    int NumberOfThickSurfaces(void) { return NumberOfThickSurfaces_; };
    
    /** Number of thin components **/
    
    int NumberOfThinComponents(void) { return NumberOfThinComponents_; };
    
    /** Number of thick components **/
    
    int NumberOfThickComponents(void) { return NumberOfThickComponents_; };    

    /** There are rotors in the model **/
    
    int ThereAreRotors(void) { return ThereAreRotors_; };
    
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
    
    double &VehicleRotationAngleVector(int i) { return VehicleRotationAngleVector_[i]; };
    
    /** Vehicle rotation origin ... used in ground effects solve ***/
    
    double &VehicleRotationAxisLocation(int i) { return VehicleRotationAxisLocation_[i]; };
    
    /** Height above the ground for ground effects model **/
    
    double &HeightAboveGround(void) { return HeightAboveGround_; };

    /** Number of control surfaces **/
    
    int NumberOfControlSurfaces(void) { return NumberOfControlSurfaces_; };
    
    /** Control surface data **/
    
    CONTROL_SURFACE &ControlSurface(int i) { return ControlSurface_[i]; };
    
    /** Size the component group list **/
    
    void SetNumberOfComponentGroupsTo(int NumberOfComponentGroups);
    
    /** Number of component groups **/
    
    int NumberOfComponentGroups(void) { return NumberOfComponentGroups_; };
    
    /** All components are fixed relative to each other **/
    
    int AllComponentsAreFixed(void) { return AllComponentsAreFixed_; };
    
    /** There is relative motion between components **/
    
    int ThereIsRelativeComponentMotion(void) { return ThereIsRelativeComponentMotion_; };
    
    /** Component group data **/
    
    COMPONENT_GROUP &ComponentGroupList(int i) { return ComponentGroupList_[i]; };
        
    /** Set far field dist **/
    
    int &SetFarFieldDist(void) { return SetFarFieldDist_; };
    
    /** Far field distance **/
    
    double &FarFieldDist(void) { return FarFieldDist_; };
    
    /** Mark and setup and wakes coming off rotors **/
    
    void MarkAndSetupRotorWakes(void);
        
    /** Update the wake mesh **/

    void InitializeWakeGrid(double Vinf, 
                            double FreeStreamVelocity_[3], 
                            double WakeAngle[3], 
                            double Xmax, 
                            int TimeAccurate);
                                               
    /** Update the surface and wake meshes ... user has updated xyz points on the mesh, so we need to update
     * all the mesh dependent stuff... Note... this forces xyz matching at trailing edge nodes with the
     * corresponding wake points **/

    void UpdateMeshes(void);

    /** Update the surface mesh ... user has updated xyz points on the mesh, so we need to update
     * all the mesh dependent stuff... Note... the wake mesh IS NOT UPDATED with this call, the trailing 
     * edge nodes on the surface and the corresponding nodes on the wake will not be coincident if you change
     * them on the surface! **/
     
    void UpdateSurfacesMeshes(void);
    
    /** Deflect the control surfaces and update the mesh **/
 
    void DeflectControlSurfaces(void);
    
    /** Un-Deflect the control surfaces and update the mesh **/
 
    void UnDeflectControlSurfaces(void);    
    
    /** Freeze wake grids **/
    
    void FreezeWakeGrids(void);
    
    /** Freeze wake root vortices **/
        
    void FreezeWakeRootVortices(int Level);
    
};

#include "END_NAME_SPACE.H"

#endif
