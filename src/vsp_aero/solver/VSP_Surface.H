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

    double *x_;
    double *y_;
    double *z_;
    
    // Normals, for wire frame geometry
    
    double *Nx_;
    double *Ny_;
    double *Nz_;
    
    // Area
    
    double *Area_;
    
    // UV coordinates
    
    double *u_;
    double *v_;
    
    // Number of nodes in I and J directions for flat plate model
    
    int NumPlateI_;
    int NumPlateJ_;
    
    // XYZ coordinates
    
    double *x_plate_;
    double *y_plate_;
    double *z_plate_;
    
    // UV coordinates
    
    double *u_plate_;
    double *v_plate_;
   
    // Surface normals.. these define the camber line normal
    
    double *Nx_Camber_;
    double *Ny_Camber_;
    double *Nz_Camber_;
    
    // Flat plate normal 
    
    double *Nx_FlatPlateNormal_;
    double *Ny_FlatPlateNormal_;
    double *Nz_FlatPlateNormal_;
    
    // Camber 
    
    double *Camber_;
    
    // Control surface subsurface bouding boxes
    
    int MaxNumberOfControlSurfaces_;
    int NumberOfControlSurfaces_;
    CONTROL_SURFACE *ControlSurface_;
    
    void LocateHingeLine(double *up, double *wp, int *HingeNode);

    // Initialize
    
    void SizeGeometryLists(int NumI, int NumJ);
    void SizeFlatPlateLists(int NumI, int NumJ);
        
    // Access to wire frame surface data - xyz, u, v, etc...
    
    double &x(int i) { return x_[i]; };
    double &y(int i) { return y_[i]; };
    double &z(int i) { return z_[i]; };
    double &u(int i) { return u_[i]; };
    double &v(int i) { return v_[i]; };

    double &x(int i, int j) { return x_[(i-1)*NumGeomJ_ + j]; };
    double &y(int i, int j) { return y_[(i-1)*NumGeomJ_ + j]; };
    double &z(int i, int j) { return z_[(i-1)*NumGeomJ_ + j]; };

    double &Nx(int i, int j) { return Nx_[(i-1)*NumGeomJ_ + j]; };
    double &Ny(int i, int j) { return Ny_[(i-1)*NumGeomJ_ + j]; };
    double &Nz(int i, int j) { return Nz_[(i-1)*NumGeomJ_ + j]; };
        
    double &u(int i, int j) { return u_[(i-1)*NumGeomJ_ + j]; };
    double &v(int i, int j) { return v_[(i-1)*NumGeomJ_ + j]; };
    
    double &Area(int i, int j) { return Area_[(i-1)*NumGeomJ_ + j]; };
    
    // Access to flat plate representation

    double &x_plate(int i) { return x_plate_[i]; };
    double &y_plate(int i) { return y_plate_[i]; };
    double &z_plate(int i) { return z_plate_[i]; };

    double &Nx_Camber(int i) { return Nx_Camber_[i]; };
    double &Ny_Camber(int i) { return Ny_Camber_[i]; };
    double &Nz_Camber(int i) { return Nz_Camber_[i]; };
    
    double &u_plate(int i) { return u_plate_[i]; };
    double &v_plate(int i) { return v_plate_[i]; };
    
    double &Camber(int i) { return Camber_[i]; };
         
    double &x_plate(int i, int j) { return x_plate_[(i-1)*NumPlateJ_ + j]; };
    double &y_plate(int i, int j) { return y_plate_[(i-1)*NumPlateJ_ + j]; };
    double &z_plate(int i, int j) { return z_plate_[(i-1)*NumPlateJ_ + j]; };

    double &Nx_Camber(int i, int j) { return Nx_Camber_[(i-1)*NumPlateJ_ + j]; };
    double &Ny_Camber(int i, int j) { return Ny_Camber_[(i-1)*NumPlateJ_ + j]; };
    double &Nz_Camber(int i, int j) { return Nz_Camber_[(i-1)*NumPlateJ_ + j]; };
    
    double &u_plate(int i, int j) { return u_plate_[(i-1)*NumPlateJ_ + j]; };
    double &v_plate(int i, int j) { return v_plate_[(i-1)*NumPlateJ_ + j]; };
    
    double &Camber(int i, int j) { return Camber_[(i-1)*NumPlateJ_ + j]; };

    // Original leading and trailing edge locations
    
    double *xLE_;
    double *yLE_;
    double *zLE_;

    double *xTE_;
    double *yTE_;
    double *zTE_;
    
    // Deformed Leading and trailing edges
    
    double *xLE_Def_;
    double *yLE_Def_;
    double *zLE_Def_;

    double *xTE_Def_;
    double *yTE_Def_;
    double *zTE_Def_;   
    
    // Arc length along wing quarter chord, from root to top
    
    double *s_;
    
    // Deformed Arc length along wing quarter chord, from root to top
    
    double *s_Def_;    
        
    // Local chord
    
    double *LocalChord_;
    
    // Root LE, TE, and Quarter chord
    
    double Root_LE_[3];
    double Root_TE_[3];
    double Root_QC_[3];
    
    // Tip LE, TE, and Quarter chord
    
    double Tip_LE_[3];
    double Tip_TE_[3];
    double Tip_QC_[3];    

    // Average chord
    
    double AverageChord_;

    // Thickness to chord
        
    double *ThicknessToChord_;
    
    // Location of maximum thickness to chord
    
    double *LocationOfMaxThickness_;
    
    // Radius to chord ratio
    
    double *RadiusToChord_;

    // Wetted area
    
    double WettedArea_;
    
    // Zero lift drag
    
    double CDo_;
                
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
    
    void FindSharpEdges(void);

    int SurfaceAtNodeIsConvex(int Node);
    
    int **NodeToTriList_;
    int  *NumberOfTrisForNode_;
            
    // FEM data

    int LoadFEMDeformation_;
    FEM_NODE FemData_;
    FEM_NODE &FemData(void) { return FemData_; };   
    
    // Control surface routines
    
    void Interpolate_XYZ_From_UV(double u, double v, double *xyz); 
    
    // Ground effects data
    
    int DoGroundEffectsAnalysis_;
    double GroundEffectsRotationAngle_;
    double GroundEffectsCGLocation_[3];
    double GroundEffectsHeightAboveGround_;
    
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
        
    double Surface_x(int i) { return x_[i]; };
    double Surface_y(int i) { return y_[i]; };
    double Surface_z(int i) { return z_[i]; };
    double Surface_u(int i) { return u_[i]; };
    double Surface_v(int i) { return v_[i]; };

    double Surface_x(int i, int j) { return x_[(i-1)*NumGeomJ_ + j]; };
    double Surface_y(int i, int j) { return y_[(i-1)*NumGeomJ_ + j]; };
    double Surface_z(int i, int j) { return z_[(i-1)*NumGeomJ_ + j]; };

    double Surface_Nx(int i, int j) { return Nx_[(i-1)*NumGeomJ_ + j]; };
    double Surface_Ny(int i, int j) { return Ny_[(i-1)*NumGeomJ_ + j]; };
    double Surface_Nz(int i, int j) { return Nz_[(i-1)*NumGeomJ_ + j]; };
        
    double Surface_u(int i, int j) { return u_[(i-1)*NumGeomJ_ + j]; };
    double Surface_v(int i, int j) { return v_[(i-1)*NumGeomJ_ + j]; };
    
    double Surface_Area(int i, int j) { return Area_[(i-1)*NumGeomJ_ + j]; };
    
    // Access to the number of nodes in each direction
    
    int NumberOfSpanStations(void) { return NumPlateI_ - 1; };

    // Leading and trailing edges
    
    double xLE(int i) { return xLE_[i]; };
    double yLE(int i) { return yLE_[i]; };
    double zLE(int i) { return zLE_[i]; };

    double xTE(int i) { return xTE_[i]; };
    double yTE(int i) { return yTE_[i]; };
    double zTE(int i) { return zTE_[i]; };
    
    // Deformed leading and trailing edges
    
    double xLE_Def(int i) { return xLE_Def_[i]; };
    double yLE_Def(int i) { return yLE_Def_[i]; };
    double zLE_Def(int i) { return zLE_Def_[i]; };

    double xTE_Def(int i) { return xTE_Def_[i]; };
    double yTE_Def(int i) { return yTE_Def_[i]; };
    double zTE_Def(int i) { return zTE_Def_[i]; };    
    
    // Flat plate normal
    
    double NxQC(int i) { return Nx_FlatPlateNormal_[i]; };
    double NyQC(int i) { return Ny_FlatPlateNormal_[i]; };
    double NzQC(int i) { return Nz_FlatPlateNormal_[i]; };
 
    // Arc length along wing, from root to top
    
    double s(int i) { return s_[i]; };
    
    // Deformed Arc length along wing, from root to top
    
    double s_Def(int i) { return s_Def_[i]; };
        
    // Local chord 
    
    double LocalChord(int i) { return LocalChord_[i]; };
    
    // Average chord
    
    double AverageChord(void) { return AverageChord_; };
    
    // Root LE, TE, and Quarter chord
    
    double Root_LE(int i ) { return Root_LE_[i]; };
    double Root_TE(int i ) { return Root_TE_[i]; };
    double Root_QC(int i ) { return Root_QC_[i]; };
    
    // Tip LE, TE, and Quarter chord
    
    double Tip_LE(int i ) { return Tip_LE_[i]; };
    double Tip_TE(int i ) { return Tip_TE_[i]; };
    double Tip_QC(int i ) { return Tip_QC_[i]; };  

    // Wetted area
    
    double WettedArea(void) { return WettedArea_; };
    
    // Zero lift drag
    
    double &CDo(void) { return CDo_; };

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
    
    void FEMUnDeformGeometry(double xyz_p[3]);
    
    // Ground effects analysis
    
    int &DoGroundEffectsAnalysis(void) { return DoGroundEffectsAnalysis_; };
    double &GroundEffectsRotationAngle(void) { return GroundEffectsRotationAngle_; };
    double &GroundEffectsCGLocation(int i) { return GroundEffectsCGLocation_[i]; };
    double &GroundEffectsHeightAboveGround(void) { return GroundEffectsHeightAboveGround_; };

    // Control surfaces
    
    int NumberOfControlSurfaces(void) { return NumberOfControlSurfaces_; };
    
    CONTROL_SURFACE &ControlSurface(int i) { return ControlSurface_[i]; };
    
    // Update geometry location
    
    void UpdateGeometryLocation(double *TVec, double *OVec, QUAT &Quat, QUAT &InvQuat);

};

#endif
