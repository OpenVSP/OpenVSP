//
//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef ROTOR_DISK_H
#define ROTOR_DISK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "quat.H"

#include "START_NAME_SPACE.H"

#define NUM_ROTOR_NODES 30

// Definition of the ROTOR_DISK class

class ROTOR_DISK {

private:

    // Function data

    int ComponentID_;

    VSPAERO_DOUBLE Density_;
    VSPAERO_DOUBLE Vinf_[3];
    
    VSPAERO_DOUBLE RotorXYZ_[3];
    VSPAERO_DOUBLE RotorNormal_[3];
    
    VSPAERO_DOUBLE RotorRadius_;
    VSPAERO_DOUBLE RotorHubRadius_;
    VSPAERO_DOUBLE RotorRPM_;
    
    VSPAERO_DOUBLE Rotor_JRatio_;
    VSPAERO_DOUBLE Rotor_CT_;
    VSPAERO_DOUBLE Rotor_CP_;
    
    VSPAERO_DOUBLE VinfMag_;

    VSPAERO_DOUBLE Rotor_JRatio(void) { return ABS(VinfMag_) / ( 2. * ABS(RotorRPM_) * RotorRadius_ /60. ); };

    VSPAERO_DOUBLE RotorArea(void) { return PI*RotorRadius_*RotorRadius_; };
    
    VSPAERO_DOUBLE RotorThrustOverArea(void) { return RotorThrust() / RotorArea(); };
    
    VSPAERO_DOUBLE RotorRadiusXYZ_[NUM_ROTOR_NODES+1][3];
    
    VSPAERO_DOUBLE RotorDirectionVectorXYZ_[5][3];

    VSPAERO_DOUBLE RotorDirectionVectorNormal_[5][3];
    
public:

    // Constructor, Destructor, Copy

    ROTOR_DISK(void);
   ~ROTOR_DISK(void);
    ROTOR_DISK(const ROTOR_DISK &RotorDisk);
    ROTOR_DISK& operator=(const ROTOR_DISK &RotorDisk);

    // Component ID
    
    int &ComponentID(void) { return ComponentID_; };

    /** Free stream density **/
    
    VSPAERO_DOUBLE &Density(void) { return Density_; };
    
    /** Free stream velocity **/
    
    VSPAERO_DOUBLE &Vinf(int i) { return Vinf_[i]; };
    
    /** Propeller radius **/
    
    VSPAERO_DOUBLE &Radius(void) { return RotorRadius_; };
    
    /** Hub radius **/
    
    VSPAERO_DOUBLE &HubRadius(void) { return RotorHubRadius_; };
    
    /** Propeller RPM **/
    
    VSPAERO_DOUBLE &RPM(void) { return RotorRPM_; };

    /** Propeller thrust coefficient **/
    
    VSPAERO_DOUBLE &CT(void) { return Rotor_CT_; };

    /** Propeller power coefficient **/    
    
    VSPAERO_DOUBLE &CP(void) { return Rotor_CP_; };

    /** Center (origin) of propeller **/
            
    VSPAERO_DOUBLE &XYZ(int i) { return RotorXYZ_[i]; };
    
    /** Propeller normal vector... this is the direction about which the
     * RPM is defined... right hand rule... it is also the direction of
     * the thrust reported **/
     
    VSPAERO_DOUBLE &Normal(int i) { return RotorNormal_[i]; };
    
    /** Calculate velocity induced by rotor at coordinate (x,y,z) **/
    
    void Velocity(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[5]);
    
    /** Calculate the velocity potential function for rotor at coordinate (x,y,z) **/
    
    void VelocityPotential(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[5]);
    
    /** Intialize the rotor data **/
    
    void Initialize(void);
    
    /** Calculate the rotor geometry information... mostly used for the VIEWER app **/
    
    void CalculateRotorGeometry(void);
    
    /** Rotor radius xyz location... i'th azimuth value, and j = 0,1,2 for x,y,z ...
     * There are NUM_ROTOR_NODES azimuth values **/
    
    VSPAERO_DOUBLE RotorRadiusXYZ(int i, int j) { return RotorRadiusXYZ_[i][j]; };
    
    /** Rotor radius vector... pointing in the direction of rotation... i'th azimuth value, 
     * and j = 0,1,2 for x,y,z ... There are NUM_ROTOR_NODES azimuth values **/
         
    VSPAERO_DOUBLE RotorDirectionVectorXYZ(int i, int j) { return RotorDirectionVectorXYZ_[i][j]; };

    /** Rotor radius vector normals... points normal to the rotor direction...  used to draw the
     * arrow heads of the direction vector... i'th azimuth value, and j = 0,1,2 for x,y,z ... The
     **/
     
    VSPAERO_DOUBLE RotorDirectionVectorNormal(int i, int j) { return RotorDirectionVectorNormal_[i][j]; };
        
    /** Propeller thrust **/    

    VSPAERO_DOUBLE RotorThrust(void) { return Rotor_CT_ * Density_ * pow(ABS(RotorRPM_)/60.,2.) * pow(2.*RotorRadius_,4.); };
    
    /** Propeller power **/
    
    VSPAERO_DOUBLE RotorPower(void) { return Rotor_CP_ * Density_ * pow(ABS(RotorRPM_)/60.,3.) * pow(2.*RotorRadius_,5.); };
    
    /** Write out propeller data to file **/
    
    void Write_STP_Data(FILE *OutputFile);    
    
    /** Read in propeller data from file **/
    
    void Load_STP_Data(FILE *InputFile);
    
    /** Write propeller data out to binary file **/
    
    void Write_Binary_STP_Data(FILE *OutputFile); 
    
    /** Read in propeller data from binary file **/
    
    void Read_Binary_STP_Data(FILE *InputFile);    
    
    /** Skip over propeller data in binary file **/
    
    void Skip_Read_Binary_STP_Data(FILE *InputFile);
    
    /** Update geometry based on given translation and quaternion rotation **/
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat);

    
};

#include "END_NAME_SPACE.H"

#endif
