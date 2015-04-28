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

#define VERTICAL   1
#define HORIZONTAL 2

#define LEFTSIDE  1
#define RIGHTSIDE 2

#define WING_SURFACE 1
#define BODY_SURFACE 2

// Definition of the VSP_SURFACE_H class

class VSP_SURFACE {

private:

    // Debug 
    
    int Verbose_;
    
    // Lifting surface flag
    
    int IsLiftingSurface_;
    
    // Function data
    
    void init(void);
    
    // Geometry data
    
    char ComponentName_[2000];
    
    int SurfaceType_;
    
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
    
    // Surface normals
    
    double *Nx_plate_;
    double *Ny_plate_;
    double *Nz_plate_;

    // Initialize
    
    void SizeGeometryLists(int NumI, int NumJ);
    void SizeFlatPlateLists(int NumI, int NumJ);
        
    // Access to xyz
    
    double &x(int i) { return x_[i]; };
    double &y(int i) { return y_[i]; };
    double &z(int i) { return z_[i]; };
    double &u(int i) { return u_[i]; };
    double &v(int i) { return v_[i]; };

    double &x(int i, int j) { return x_[(i-1)*NumGeomJ_ + j]; };
    double &y(int i, int j) { return y_[(i-1)*NumGeomJ_ + j]; };
    double &z(int i, int j) { return z_[(i-1)*NumGeomJ_ + j]; };
    
    double &u(int i, int j) { return u_[(i-1)*NumGeomJ_ + j]; };
    double &v(int i, int j) { return v_[(i-1)*NumGeomJ_ + j]; };
    
    // Access to flat plate representation

    double &x_plate(int i) { return x_plate_[i]; };
    double &y_plate(int i) { return y_plate_[i]; };
    double &z_plate(int i) { return z_plate_[i]; };

    double &Nx_plate(int i) { return Nx_plate_[i]; };
    double &Ny_plate(int i) { return Ny_plate_[i]; };
    double &Nz_plate(int i) { return Nz_plate_[i]; };

    double &x_plate(int i, int j) { return x_plate_[(i-1)*NumPlateJ_ + j]; };
    double &y_plate(int i, int j) { return y_plate_[(i-1)*NumPlateJ_ + j]; };
    double &z_plate(int i, int j) { return z_plate_[(i-1)*NumPlateJ_ + j]; };

    double &Nx_plate(int i, int j) { return Nx_plate_[(i-1)*NumPlateJ_ + j]; };
    double &Ny_plate(int i, int j) { return Ny_plate_[(i-1)*NumPlateJ_ + j]; };
    double &Nz_plate(int i, int j) { return Nz_plate_[(i-1)*NumPlateJ_ + j]; };
    
    // Local chord
    
    double *LocalChord_;
    
    // Average chord
    
    double AverageChord_;
    
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

public:

    // Constructor, Destructor, Copy

    VSP_SURFACE(void);
   ~VSP_SURFACE(void);
    VSP_SURFACE(const VSP_SURFACE &VSP_Surface);

    // Copy function

    VSP_SURFACE& operator=(const VSP_SURFACE &VSP_Surface);
    
    // Access to the number of nodes in each direction
    
    int NumberOfSpanStations(void) { return NumPlateI_ - 1; };
    
    // Local chord 
    
    double LocalChord(int i) { return LocalChord_[i]; };
    
    // Average chord
    
    double AverageChord(void) { return AverageChord_; };
    
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
    
    void ReadWingDataFromFile(char *Name, FILE *VSP_Degen_File);
    void ReadBodyDataFromFile(char *Name, int Case, FILE *VSP_Degen_File);

    // Component name
    
    char* ComponentName(void) { return ComponentName_; };
    
    // Surface type
    
    int SurfaceType(void) { return SurfaceType_; };
    
};

#endif
