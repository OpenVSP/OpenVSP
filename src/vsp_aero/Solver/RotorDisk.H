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

    double Density_;
    
    double Vinf_[3];
    
    double RotorXYZ_[3];
    
    double RotorNormal_[3];
    
    double RotorRadius_;
    
    double RotorHubRadius_;
    
    double RotorRPM_;
    
    double Rotor_CT_;
    
    double Rotor_CP_;
    
    double VinfMag_;

    double Rotor_JRatio(void) { return ABS(VinfMag_) / ( 2. * ABS(RotorRPM_) * RotorRadius_ /60. ); };

    double RotorArea(void) { return PI*RotorRadius_*RotorRadius_; };
    
    double RotorThrustOverArea(void) { return RotorThrust() / RotorArea(); };
    
    double RotorRadiusXYZ_[NUM_ROTOR_NODES+1][3];
    
    double RotorDirectionVectorXYZ_[5][3];

    double RotorDirectionVectorNormal_[5][3];
    
public:

    // Constructor, Destructor, Copy

    ROTOR_DISK(void);
   ~ROTOR_DISK(void);
    ROTOR_DISK(const ROTOR_DISK &RotorDisk);
    ROTOR_DISK& operator=(const ROTOR_DISK &RotorDisk);

    // Component ID
    
    int &ComponentID(void) { return ComponentID_; };

    /** Free stream density **/
    
    double &Density(void) { return Density_; };
    
    /** Free stream velocity **/
    
    double &Vinf(int i) { return Vinf_[i]; };
    
    /** Propeller radius **/
    
    double &Radius(void) { return RotorRadius_; };
    
    /** Hub radius **/
    
    double &HubRadius(void) { return RotorHubRadius_; };
    
    /** Propeller RPM **/
    
    double &RPM(void) { return RotorRPM_; };

    /** Propeller thrust coefficient **/
    
    double &CT(void) { return Rotor_CT_; };

    /** Propeller power coefficient **/    
    
    double &CP(void) { return Rotor_CP_; };

    /** Center (origin) of propeller **/
            
    double &XYZ(int i) { return RotorXYZ_[i]; };
    
    /** Propeller normal vector... this is the direction about which the
     * RPM is defined... right hand rule... it is also the direction of
     * the thrust reported **/
     
    double &Normal(int i) { return RotorNormal_[i]; };
    
    /** Calculate velocity induced by rotor at coordinate (x,y,z) **/
    
    void Velocity(double xyz_p[3], double q[5]);
    
    /** Calculate the velocity potential function for rotor at coordinate (x,y,z) **/
    
    void VelocityPotential(double xyz_p[3], double q[5]);
    
    /** Intialize the rotor data **/
    
    void Initialize(void);
    
    /** Calculate the rotor geometry information... mostly used for the VIEWER app **/
    
    void CalculateRotorGeometry(void);
    
    /** Rotor radius xyz location... i'th azimuth value, and j = 0,1,2 for x,y,z ...
     * There are NUM_ROTOR_NODES azimuth values **/
    
    double RotorRadiusXYZ(int i, int j) { return RotorRadiusXYZ_[i][j]; };
    
    /** Rotor radius vector... pointing in the direction of rotation... i'th azimuth value, 
     * and j = 0,1,2 for x,y,z ... There are NUM_ROTOR_NODES azimuth values **/
         
    double RotorDirectionVectorXYZ(int i, int j) { return RotorDirectionVectorXYZ_[i][j]; };

    /** Rotor radius vector normals... points normal to the rotor direction...  used to draw the
     * arrow heads of the direction vector... i'th azimuth value, and j = 0,1,2 for x,y,z ... The
     **/
     
    double RotorDirectionVectorNormal(int i, int j) { return RotorDirectionVectorNormal_[i][j]; };
        
    /** Propeller thrust **/    

    double RotorThrust(void) { return Rotor_CT_ * Density_ * pow(ABS(RotorRPM_)/60.,2.) * pow(2.*RotorRadius_,4.); };
    
     /** Partial derivative of propeller thrust wrt CT **/    
   
    double pRotorThrust_pCT(void) { return Density_ * pow(ABS(RotorRPM_)/60.,2.) * pow(2.*RotorRadius_,4.); };
    
     /** Partial derivative of propeller thrust wrt Density **/    
    
    double pRotorThrust_pDensity(void) { return Rotor_CT_ * pow(ABS(RotorRPM_)/60.,2.) * pow(2.*RotorRadius_,4.); };
    
     /** Partial derivative of propeller thrust wrt RPM **/    
    
    double pRotorThrust_pRPM(void) { return Rotor_CT_ * Density_ * 2.*ABS(RotorRPM_)/3600. * pow(2.*RotorRadius_,4.); };
    
     /** Partial derivative of propeller thrust wrt Radius **/    
    
    double pRotorThrust_pRadius(void) { return Rotor_CT_ * Density_ * pow(ABS(RotorRPM_)/60.,2.) * 8. * pow(2.*RotorRadius_,3.); };
    
    /** Propeller power **/
    
    double RotorPower(void) { return Rotor_CP_ * Density_ * pow(ABS(RotorRPM_)/60.,3.) * pow(2.*RotorRadius_,5.); };
    
    /** Partial derivative of propeller power wrt CP **/
    
    double pRotorPower_pCP(void) { return Density_ * pow(ABS(RotorRPM_)/60.,3.) * pow(2.*RotorRadius_,5.); };
    
    /** Partial derivative of propeller power wrt Density **/
    
    double pRotorPower_pDensity(void) { return Rotor_CP_ * pow(ABS(RotorRPM_)/60.,3.) * pow(2.*RotorRadius_,5.); };

    /** Partial derivative of propeller power wrt RPM **/

    double pRotorPower_pRPM(void) { return Rotor_CP_ * Density_ * 3.*pow(ABS(RotorRPM_)/60.,2.)/60. * pow(2.*RotorRadius_,5.); };
    
    /** Partial derivative of propeller power wrt Radius **/
    
    double pRotorPower_pRadius(void) { return Rotor_CP_ * Density_ * pow(ABS(RotorRPM_)/60.,3.) * 10.*pow(2.*RotorRadius_,4.); };
    
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
    
    void UpdateGeometryLocation(double *TVec, double *OVec, QUAT &Quat, QUAT &InvQuat);

    
};

#include "END_NAME_SPACE.H"

#endif
