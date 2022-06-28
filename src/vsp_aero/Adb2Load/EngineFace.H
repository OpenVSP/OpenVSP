//
//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef ENGINE_FACE_H
#define ENGINE_FACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "quat.H"

#define VSPAERO_DOUBLE double
#define FWRITE fwrite
#define FREAD fread

// Definition of the ENGINE_FACE class

#define INLET_FACE  1
#define NOZZLE_FACE 2

#define NUM_NOZZLE_NODES 15

class ENGINE_FACE {

private:

    // Nozzle or Inlet data
    
    int SurfaceType_;
    
    int SurfaceID_;

    VSPAERO_DOUBLE MassFlow_;
    VSPAERO_DOUBLE Density_;
    VSPAERO_DOUBLE Area_;
    
    VSPAERO_DOUBLE NozzleVelocity_;
    VSPAERO_DOUBLE NozzleDeltaCp_;
    VSPAERO_DOUBLE NozzleXYZ_[3];
    VSPAERO_DOUBLE NozzleNormal_[3];
    
    VSPAERO_DOUBLE NozzleRadius_;

    VSPAERO_DOUBLE NozzleRadiusXYZ_[NUM_NOZZLE_NODES+1][3];
    VSPAERO_DOUBLE NozzleRadiusNormal_[NUM_NOZZLE_NODES+1][3];
     
public:

    // Constructor, Destructor, Copy

    ENGINE_FACE(void);
   ~ENGINE_FACE(void);
    ENGINE_FACE(const ENGINE_FACE &EngineFace);
    ENGINE_FACE& operator=(const ENGINE_FACE &EngineFace);
    
    int &SurfaceType(void) { return SurfaceType_; };
    int &SurfaceID(void) { return SurfaceID_; };

    VSPAERO_DOUBLE &NozzleRadius(void) { return NozzleRadius_; };
    VSPAERO_DOUBLE &NozzleXYZ(int i) { return NozzleXYZ_[i]; };
    VSPAERO_DOUBLE &NozzleNormal(int i) { return NozzleNormal_[i]; };
    VSPAERO_DOUBLE &NozzleVelocity(void) { return NozzleVelocity_; };
    VSPAERO_DOUBLE &NozzleDeltaCp(void) { return NozzleDeltaCp_; };
            
    VSPAERO_DOUBLE &MassFlow(void) { return MassFlow_; };
    VSPAERO_DOUBLE &Density(void) { return Density_; };
    VSPAERO_DOUBLE &Area(void) { return Area_; };

    void Write_Binary_STP_Data(FILE *InputFile); 
    void Read_Binary_STP_Data(FILE *InputFile);    
    void Skip_Read_Binary_STP_Data(FILE *InputFile); 
    
    VSPAERO_DOUBLE NormalVelocity(void);

    void Velocity(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[5]);

    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);

    void CalculateNozzleGeometry(void);

    VSPAERO_DOUBLE NozzleRadiusXYZ(int i, int j) { return NozzleRadiusXYZ_[i][j]; };
    VSPAERO_DOUBLE NozzleRadiusNormal(int i, int j) { return NozzleRadiusNormal_[i][j]; };
    
};

#endif
