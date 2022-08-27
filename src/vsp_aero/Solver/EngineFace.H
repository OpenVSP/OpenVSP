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

#include "START_NAME_SPACE.H"

// Definition of the ENGINE_FACE class

#define INLET_FACE  1
#define NOZZLE_FACE 2

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
      
public:

    // Constructor, Destructor, Copy

    ENGINE_FACE(void);
   ~ENGINE_FACE(void);
    ENGINE_FACE(const ENGINE_FACE &EngineFace);
    ENGINE_FACE& operator=(const ENGINE_FACE &EngineFace);
    
    /** Surface type ... INLET or NOZZLE **/
    
    int &SurfaceType(void) { return SurfaceType_; };
    
    /** Surface ID ... pointing back to the surface mesh Surface IDs **/
    
    int &SurfaceID(void) { return SurfaceID_; };

    /** Nozzle radius **/
    
    VSPAERO_DOUBLE &NozzleRadius(void) { return NozzleRadius_; };
    
    /** Nozzle xyz origin location **/
    
    VSPAERO_DOUBLE &NozzleXYZ(int i) { return NozzleXYZ_[i]; };
    
    /** Nozzle normal vector **/
    
    VSPAERO_DOUBLE &NozzleNormal(int i) { return NozzleNormal_[i]; };
    
    /** Nozzle exit velocity **/
    
    VSPAERO_DOUBLE &NozzleVelocity(void) { return NozzleVelocity_; };
    
    /** Nozzle exit delta Cp in the exit jet **/
    
    VSPAERO_DOUBLE &NozzleDeltaCp(void) { return NozzleDeltaCp_; };
    
    /** Inlet mass flow **/
            
    VSPAERO_DOUBLE &MassFlow(void) { return MassFlow_; };
    
    /** Inlet density **/
    
    VSPAERO_DOUBLE &Density(void) { return Density_; };
    
    /** Inlet area **/
    
    VSPAERO_DOUBLE &Area(void) { return Area_; };

    /** Write state of this nozzle / inlet to file **/
    
    void Write_Binary_STP_Data(FILE *OutputFile); 
    
    /** Read state for this nozzle from file **/
    
    void Read_Binary_STP_Data(FILE *InputFile);    
    
    /** Skip over engine data in this file **/
    
    void Skip_Read_Binary_STP_Data(FILE *InputFile); 
    
    /** Normal velocity for this engine face **/
    
    VSPAERO_DOUBLE NormalVelocity(void);
    
    /** Velocity, (u,v,w,DeltaCp,VelocityMagnitude) at location (x,y,z) **/
    
    void Velocity(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[5]);
    
    /** Update the location, orientation of the engine data based on a translation and quaternion rotation **/

    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);
          
};

#include "END_NAME_SPACE.H"

#endif
