//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_BODY_H
#define VSP_BODY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "VSP_Grid.H"

#define VERTICAL   1
#define HORIZONTAL 2

// Definition of the VSP_BODY_H class

class VSP_BODY {

private:

    // Debug 
    
    int Verbose_;
    
    // Function data
    
    void init(void);
    
    // Wing geometry data
    
    char ComponentName_[2000];
    
    int SliceType_;    
    int NoseIsClosed_;
    int TailIsClosed_;
    
    int NumGeomI_;
    int NumGeomJ_;

    double *x_;
    double *y_;
    double *z_;
    
    double *u_;
    double *v_;
    
    // Number of nodes in I and J directions
    
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
    
    // Wake data
    
    int NumberOfKuttaNodes_;

    int *KuttaNode_;
    double *WakeTrailingEdgeX_;
    double *WakeTrailingEdgeY_;
    double *WakeTrailingEdgeZ_;
    
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
                
    // VSP Grid
    
    VSP_GRID Grid_;
    
    // Degenerate x-section tests
    
    void CheckForDegenerateXSections(void);

public:

    // Constructor, Destructor, Copy

    VSP_BODY(void);
   ~VSP_BODY(void);
    VSP_BODY(const VSP_BODY &VSP_Wing);

    // Copy function

    VSP_BODY& operator=(const VSP_BODY &VSP_Wing);
    
    // Access to the number of nodes in each direction
    
    int NumberOfSpanStations(void) { return NumPlateI_ - 1; };
    
    // Local chord 
    
    double LocalChord(int i) { return LocalChord_[i]; };
    
    // Access to kutta node list
    
    int NumberOfKuttaNodes(void) { return NumberOfKuttaNodes_; };
    int KuttaNode(int i) { return KuttaNode_[i]; };
    
    double WakeTrailingEdgeX(int i) { return WakeTrailingEdgeX_[i]; };
    double WakeTrailingEdgeY(int i) { return WakeTrailingEdgeY_[i]; };
    double WakeTrailingEdgeZ(int i) { return WakeTrailingEdgeZ_[i]; };

    // Create triangulated mesh
    
    void CreateMesh(int SurfaceID);
    
    VSP_GRID &Grid(void) { return Grid_; };
    
    // File I/O
    
    void ReadFile(char *Name, int Case, FILE *VSP_Degen_File);
   
    // Component name
    
    char* ComponentName(void) { return ComponentName_; };
    
};

#endif
