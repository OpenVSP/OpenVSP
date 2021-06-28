//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_SURFACE_H
#define VSP_SURFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "VSP_Grid.H"
#include "VSP_Agglom.H"
#include "FEM_Node.H"
#include "ControlSurface.H"

#define VERTICAL   1
#define HORIZONTAL 2

#define LEFTSIDE  1
#define RIGHTSIDE 2

#define DEGEN_WING_SURFACE   1
#define DEGEN_BODY_SURFACE   2
#define CART3D_SURFACE       3
#define VSPGEOM_SURFACE      4
#define DEGEN_SURFACE        5

// Simple kutta node structure

class KUTTA_NODE {

public:

   int IsKuttaNode;
   int Node;
   int Edge1;
   int Edge2;

};

// Definition of the VSP_SURFACE_H class

class VSP_SURFACE {

private:

    // Debug 
    
    int Verbose_;
    
    // Lifting surface flag
    
    int IsLiftingSurface_;    
    
    // Number of surface patches
    
    int NumberOfSurfacePatches_;
    
    // Function data
    
    void init(void);
    
    // Geometry data
    
    char ComponentName_[2000];
    
    int SurfaceType_;
    
    int ComponentID_;
    
    int Case_;
    
    // Degenerate mesh flags
    
    int NoseIsClosed_;
    int TailIsClosed_;
    
    // Number of nodes in I and J directions
    
    int NumGeomI_;
    int NumGeomJ_;
    
    // XYZ coordinates

    VSPAERO_DOUBLE *x_;
    VSPAERO_DOUBLE *y_;
    VSPAERO_DOUBLE *z_;
    
    // Normals, for wire frame geometry
    
    VSPAERO_DOUBLE *Nx_;
    VSPAERO_DOUBLE *Ny_;
    VSPAERO_DOUBLE *Nz_;
    
    // Area
    
    VSPAERO_DOUBLE *Area_;
    
    // UV coordinates
    
    VSPAERO_DOUBLE *u_;
    VSPAERO_DOUBLE *v_;
    
    // Number of nodes in I and J directions for flat plate model
    
    int NumPlateI_;
    int NumPlateJ_;
    
    // XYZ coordinates
    
    VSPAERO_DOUBLE *x_plate_;
    VSPAERO_DOUBLE *y_plate_;
    VSPAERO_DOUBLE *z_plate_;
    
    // UV coordinates
    
    VSPAERO_DOUBLE *u_plate_;
    VSPAERO_DOUBLE *v_plate_;
   
    // Surface normals.. these define the camber line normal
    
    VSPAERO_DOUBLE *Nx_Camber_;
    VSPAERO_DOUBLE *Ny_Camber_;
    VSPAERO_DOUBLE *Nz_Camber_;
    
    // Flat plate normal 
    
    VSPAERO_DOUBLE *Nx_FlatPlateNormal_;
    VSPAERO_DOUBLE *Ny_FlatPlateNormal_;
    VSPAERO_DOUBLE *Nz_FlatPlateNormal_;
    
    // Camber 
    
    VSPAERO_DOUBLE *Camber_;
    
    // Control surface subsurface bouding boxes
    
    int MaxNumberOfControlSurfaces_;
    int NumberOfControlSurfaces_;
    CONTROL_SURFACE *ControlSurface_;
    
    void LocateHingeLine(VSPAERO_DOUBLE *up, VSPAERO_DOUBLE *wp, int *HingeNode);

    // Initialize
    
    void SizeGeometryLists(int NumI, int NumJ);
    void SizeFlatPlateLists(int NumI, int NumJ);
        
    // Access to wire frame surface data - xyz, u, v, etc...
    
    VSPAERO_DOUBLE &x(int i) { return x_[i]; };
    VSPAERO_DOUBLE &y(int i) { return y_[i]; };
    VSPAERO_DOUBLE &z(int i) { return z_[i]; };
    VSPAERO_DOUBLE &u(int i) { return u_[i]; };
    VSPAERO_DOUBLE &v(int i) { return v_[i]; };

    VSPAERO_DOUBLE &x(int i, int j) { return x_[(i-1)*NumGeomJ_ + j]; };
    VSPAERO_DOUBLE &y(int i, int j) { return y_[(i-1)*NumGeomJ_ + j]; };
    VSPAERO_DOUBLE &z(int i, int j) { return z_[(i-1)*NumGeomJ_ + j]; };

    VSPAERO_DOUBLE &Nx(int i, int j) { return Nx_[(i-1)*NumGeomJ_ + j]; };
    VSPAERO_DOUBLE &Ny(int i, int j) { return Ny_[(i-1)*NumGeomJ_ + j]; };
    VSPAERO_DOUBLE &Nz(int i, int j) { return Nz_[(i-1)*NumGeomJ_ + j]; };
        
    VSPAERO_DOUBLE &u(int i, int j) { return u_[(i-1)*NumGeomJ_ + j]; };
    VSPAERO_DOUBLE &v(int i, int j) { return v_[(i-1)*NumGeomJ_ + j]; };
    
    VSPAERO_DOUBLE &Area(int i, int j) { return Area_[(i-1)*NumGeomJ_ + j]; };

    // Access to flat plate representation

    VSPAERO_DOUBLE &x_plate(int i) { return x_plate_[i]; };
    VSPAERO_DOUBLE &y_plate(int i) { return y_plate_[i]; };
    VSPAERO_DOUBLE &z_plate(int i) { return z_plate_[i]; };

    VSPAERO_DOUBLE &Nx_Camber(int i) { return Nx_Camber_[i]; };
    VSPAERO_DOUBLE &Ny_Camber(int i) { return Ny_Camber_[i]; };
    VSPAERO_DOUBLE &Nz_Camber(int i) { return Nz_Camber_[i]; };
    
    VSPAERO_DOUBLE &u_plate(int i) { return u_plate_[i]; };
    VSPAERO_DOUBLE &v_plate(int i) { return v_plate_[i]; };
    
    VSPAERO_DOUBLE &Camber(int i) { return Camber_[i]; };
         
    VSPAERO_DOUBLE &x_plate(int i, int j) { return x_plate_[(i-1)*NumPlateJ_ + j]; };
    VSPAERO_DOUBLE &y_plate(int i, int j) { return y_plate_[(i-1)*NumPlateJ_ + j]; };
    VSPAERO_DOUBLE &z_plate(int i, int j) { return z_plate_[(i-1)*NumPlateJ_ + j]; };

    VSPAERO_DOUBLE &Nx_Camber(int i, int j) { return Nx_Camber_[(i-1)*NumPlateJ_ + j]; };
    VSPAERO_DOUBLE &Ny_Camber(int i, int j) { return Ny_Camber_[(i-1)*NumPlateJ_ + j]; };
    VSPAERO_DOUBLE &Nz_Camber(int i, int j) { return Nz_Camber_[(i-1)*NumPlateJ_ + j]; };
    
    VSPAERO_DOUBLE &u_plate(int i, int j) { return u_plate_[(i-1)*NumPlateJ_ + j]; };
    VSPAERO_DOUBLE &v_plate(int i, int j) { return v_plate_[(i-1)*NumPlateJ_ + j]; };
    
    VSPAERO_DOUBLE &Camber(int i, int j) { return Camber_[(i-1)*NumPlateJ_ + j]; };

    // Original leading and trailing edge locations
    
    VSPAERO_DOUBLE *xLE_;
    VSPAERO_DOUBLE *yLE_;
    VSPAERO_DOUBLE *zLE_;

    VSPAERO_DOUBLE *xTE_;
    VSPAERO_DOUBLE *yTE_;
    VSPAERO_DOUBLE *zTE_;
    
    // Deformed Leading and trailing edges
    
    VSPAERO_DOUBLE *xLE_Def_;
    VSPAERO_DOUBLE *yLE_Def_;
    VSPAERO_DOUBLE *zLE_Def_;

    VSPAERO_DOUBLE *xTE_Def_;
    VSPAERO_DOUBLE *yTE_Def_;
    VSPAERO_DOUBLE *zTE_Def_;   
    
    // Arc length along wing quarter chord, from root to top
    
    VSPAERO_DOUBLE *s_;
    
    // Deformed Arc length along wing quarter chord, from root to top
    
    VSPAERO_DOUBLE *s_Def_;    
        
    // Local chord
    
    VSPAERO_DOUBLE *LocalChord_;
    
    // Root LE, TE, and Quarter chord
    
    VSPAERO_DOUBLE Root_LE_[3];
    VSPAERO_DOUBLE Root_TE_[3];
    VSPAERO_DOUBLE Root_QC_[3];
    
    // Tip LE, TE, and Quarter chord
    
    VSPAERO_DOUBLE Tip_LE_[3];
    VSPAERO_DOUBLE Tip_TE_[3];
    VSPAERO_DOUBLE Tip_QC_[3];    

    // Average chord
    
    VSPAERO_DOUBLE AverageChord_;

    // Thickness to chord
        
    VSPAERO_DOUBLE *ThicknessToChord_;
    
    // Location of maximum thickness to chord
    
    VSPAERO_DOUBLE *LocationOfMaxThickness_;
    
    // Radius to chord ratio
    
    VSPAERO_DOUBLE *RadiusToChord_;

    // Wetted area
    
    VSPAERO_DOUBLE WettedArea_;
    
    // Zero lift drag
    
    VSPAERO_DOUBLE CDo_;
                
    // VSP Grid
    
    int MaxNumberOfGridLevels_;
    int NumberOfGridLevels_;
        
    VSP_GRID **Grid_;
    
    // Degenerate x, span station tests
    
    void CheckForDegenerateXSections(void);    
    
    void CheckForDegenerateSpanSections(void);
    
    // Create triangulated mesh
    
    void CreateWingTriMesh(int SurfaceID);
    
    void CreateBodyTriMesh(int SurfaceID);
                                        
    // Create upwind edge data
    
    void CreateUpwindEdgeData(int SurfaceID);
    
    // Do RCM Sort
    
    void DoRCMSort(void);
    
    // Find sharp trailing edges
    
    void FindSharpEdges(int NumberOfSharpNodes, int *SharpNodeList);

    int SurfaceAtNodeIsConvex(int Node);
    
    int **NodeToTriList_;
    int  *NumberOfTrisForNode_;
    
    int **NodeToEdgeList_;
    int  *NumberOfEdgesForNode_;
            
    // FEM data

    int LoadFEMDeformation_;
    FEM_NODE FemData_;
    FEM_NODE &FemData(void) { return FemData_; };   
    
    // Control surface routines
    
    void Interpolate_XYZ_From_UV(VSPAERO_DOUBLE u, VSPAERO_DOUBLE v, VSPAERO_DOUBLE *xyz); 
    
    // Ground effects data
    
    int DoGroundEffectsAnalysis_;
    VSPAERO_DOUBLE GroundEffectsRotationAngle_;
    VSPAERO_DOUBLE GroundEffectsCGLocation_[3];
    VSPAERO_DOUBLE GroundEffectsHeightAboveGround_;
    
    void RotateGeometry_About_Y_Axis(void);    
    
public:

    // Constructor, Destructor, Copy

    VSP_SURFACE(void);
   ~VSP_SURFACE(void);
    VSP_SURFACE(const VSP_SURFACE &VSP_Surface);

    // Copy function

    VSP_SURFACE& operator=(const VSP_SURFACE &VSP_Surface);
    
    // Access to wire frame surface data - xyz, u, v, etc...
    
    int Surface_NumI(void) { return NumGeomI_; };
    int Surface_NumJ(void) { return NumGeomJ_; };
        
    VSPAERO_DOUBLE Surface_x(int i) { return x_[i]; };
    VSPAERO_DOUBLE Surface_y(int i) { return y_[i]; };
    VSPAERO_DOUBLE Surface_z(int i) { return z_[i]; };
    VSPAERO_DOUBLE Surface_u(int i) { return u_[i]; };
    VSPAERO_DOUBLE Surface_v(int i) { return v_[i]; };

    VSPAERO_DOUBLE Surface_x(int i, int j) { return x_[(i-1)*NumGeomJ_ + j]; };
    VSPAERO_DOUBLE Surface_y(int i, int j) { return y_[(i-1)*NumGeomJ_ + j]; };
    VSPAERO_DOUBLE Surface_z(int i, int j) { return z_[(i-1)*NumGeomJ_ + j]; };

    VSPAERO_DOUBLE Surface_Nx(int i, int j) { return Nx_[(i-1)*NumGeomJ_ + j]; };
    VSPAERO_DOUBLE Surface_Ny(int i, int j) { return Ny_[(i-1)*NumGeomJ_ + j]; };
    VSPAERO_DOUBLE Surface_Nz(int i, int j) { return Nz_[(i-1)*NumGeomJ_ + j]; };
        
    VSPAERO_DOUBLE Surface_u(int i, int j) { return u_[(i-1)*NumGeomJ_ + j]; };
    VSPAERO_DOUBLE Surface_v(int i, int j) { return v_[(i-1)*NumGeomJ_ + j]; };
    
    VSPAERO_DOUBLE Surface_Area(int i, int j) { return Area_[(i-1)*NumGeomJ_ + j]; };
    
    // Access to the number of nodes in each direction
    
    int NumberOfSpanStations(void) { return NumPlateI_ - 1; };

    // Leading and trailing edges
    
    VSPAERO_DOUBLE xLE(int i) { return xLE_[i]; };
    VSPAERO_DOUBLE yLE(int i) { return yLE_[i]; };
    VSPAERO_DOUBLE zLE(int i) { return zLE_[i]; };

    VSPAERO_DOUBLE xTE(int i) { return xTE_[i]; };
    VSPAERO_DOUBLE yTE(int i) { return yTE_[i]; };
    VSPAERO_DOUBLE zTE(int i) { return zTE_[i]; };
    
    // Deformed leading and trailing edges
    
    VSPAERO_DOUBLE xLE_Def(int i) { return xLE_Def_[i]; };
    VSPAERO_DOUBLE yLE_Def(int i) { return yLE_Def_[i]; };
    VSPAERO_DOUBLE zLE_Def(int i) { return zLE_Def_[i]; };

    VSPAERO_DOUBLE xTE_Def(int i) { return xTE_Def_[i]; };
    VSPAERO_DOUBLE yTE_Def(int i) { return yTE_Def_[i]; };
    VSPAERO_DOUBLE zTE_Def(int i) { return zTE_Def_[i]; };    
 
    // Flat plate normal
    
    VSPAERO_DOUBLE NxQC(int i) { return Nx_FlatPlateNormal_[i]; };
    VSPAERO_DOUBLE NyQC(int i) { return Ny_FlatPlateNormal_[i]; };
    VSPAERO_DOUBLE NzQC(int i) { return Nz_FlatPlateNormal_[i]; };
 
    // Arc length along wing, from root to top
    
    VSPAERO_DOUBLE s(int i) { return s_[i]; };
    
    // Deformed Arc length along wing, from root to top
    
    VSPAERO_DOUBLE s_Def(int i) { return s_Def_[i]; };
        
    // Local chord 
    
    VSPAERO_DOUBLE LocalChord(int i) { return LocalChord_[i]; };
    
    // Average chord
    
    VSPAERO_DOUBLE AverageChord(void) { return AverageChord_; };
    
    // Root LE, TE, and Quarter chord
    
    VSPAERO_DOUBLE Root_LE(int i ) { return Root_LE_[i]; };
    VSPAERO_DOUBLE Root_TE(int i ) { return Root_TE_[i]; };
    VSPAERO_DOUBLE Root_QC(int i ) { return Root_QC_[i]; };
    
    // Tip LE, TE, and Quarter chord
    
    VSPAERO_DOUBLE Tip_LE(int i ) { return Tip_LE_[i]; };
    VSPAERO_DOUBLE Tip_TE(int i ) { return Tip_TE_[i]; };
    VSPAERO_DOUBLE Tip_QC(int i ) { return Tip_QC_[i]; };  

    // Wetted area
    
    VSPAERO_DOUBLE WettedArea(void) { return WettedArea_; };
    
    // Zero lift drag
    
    VSPAERO_DOUBLE &CDo(void) { return CDo_; };

    // Create triangulated mesh
    
    void CreateMesh(int SurfaceID);
    void AgglomerateMesh(void);
    
    VSP_GRID &Grid(void) { return *(Grid_[0]); };
    VSP_GRID &Grid(int i) { return *(Grid_[MIN(i,NumberOfGridLevels_-1)]); };
    VSP_GRID &FineGrid(void) { return *(Grid_[0]); };
    
    int NumberOfGridLevels(void) { return NumberOfGridLevels_; };
     
    // File I/O
    
    void GetComponentBBox(FILE *VSP_Degen_File, BBOX &WingBox);
    
    void ReadCart3DDataFromFile(char *Name, FILE *CART3D_File);
    void ReadVSPGeomDataFromFile(char *Name, FILE *VSPGeom_File);

    void ReadWingDataFromFile(char *Name, FILE *VSP_Degen_File);
    void ReadBodyDataFromFile(char *Name, int Case, FILE *VSP_Degen_File);
    
    // FEM
        
    void LoadFEMDeformationData(char *FileName);
    void FEMDeformGeometry(void);        

    // Component name
    
    char* ComponentName(void) { return ComponentName_; };
    
    // Surface type
    
    int SurfaceType(void) { return SurfaceType_; };
    
    // Internal component ID
    
    int &ComponentID(void) { return ComponentID_; };
    
    // Case number... DegenGeoms are split 4 ways, keep track of which one we are
    
    int &Case(void) { return Case_; };
    
    // Number of surface patches
    
    int NumberOfSurfacePatches(void) { return NumberOfSurfacePatches_; };
    
    // FEM data
    
    void FEMUnDeformGeometry(VSPAERO_DOUBLE xyz_p[3]);
    
    // Ground effects analysis
    
    int &DoGroundEffectsAnalysis(void) { return DoGroundEffectsAnalysis_; };
    VSPAERO_DOUBLE &GroundEffectsRotationAngle(void) { return GroundEffectsRotationAngle_; };
    VSPAERO_DOUBLE &GroundEffectsCGLocation(int i) { return GroundEffectsCGLocation_[i]; };
    VSPAERO_DOUBLE &GroundEffectsHeightAboveGround(void) { return GroundEffectsHeightAboveGround_; };

    // Control surfaces
    
    int NumberOfControlSurfaces(void) { return NumberOfControlSurfaces_; };
    
    CONTROL_SURFACE &ControlSurface(int i) { return ControlSurface_[i]; };
    
    // Update geometry location
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);

};

#endif
