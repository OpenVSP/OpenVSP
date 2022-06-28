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
    
    double Rotor_JRatio_;
    double Rotor_CT_;
    double Rotor_CP_;
    
    double VinfMag_;

    double Rotor_JRatio(void) { return VinfMag_ / ( 2. * ABS(RotorRPM_) * RotorRadius_ /60. ); };

    double RotorThrust(void) { return Rotor_CT_ * Density_ * pow(ABS(RotorRPM_)/60.,2.) * pow(2.*RotorRadius_,4.); };
    
    double RotorPower(void) { return Rotor_CP_ * Density_ * pow(ABS(RotorRPM_)/60.,3.) * pow(2.*RotorRadius_,5.); };
    
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

    // Initialize rotor inputs
    
    double &Density(void) { return Density_; };
    double &Vinf(int i) { return Vinf_[i]; };
    
    double &Radius(void) { return RotorRadius_; };
    double &HubRadius(void) { return RotorHubRadius_; };
    double &RPM(void) { return RotorRPM_; };

    double &CT(void) { return Rotor_CT_; };
    double &CP(void) { return Rotor_CP_; };
        
    double &XYZ(int i) { return RotorXYZ_[i]; };
    double &Normal(int i) { return RotorNormal_[i]; };
    
    // Calculate velocity induced by rotor
    
    void Velocity(double xyz[3], double q[5]);
    void VelocityPotential(double xyz[3], double q[5]);
    
    // Initialize
    
    void Initialize(void);
    
    // Calculate rotor geometry
    
    void CalculateRotorGeometry(void);
    
    // Rotor tip geometry
    
    double RotorRadiusXYZ(int i, int j) { return RotorRadiusXYZ_[i][j]; };
    
    double RotorDirectionVectorXYZ(int i, int j) { return RotorDirectionVectorXYZ_[i][j]; };

    double RotorDirectionVectorNormal(int i, int j) { return RotorDirectionVectorNormal_[i][j]; };
    
    // File I/O
    
    void Write_STP_Data(FILE *InputFile);    
    void Load_STP_Data(FILE *InputFile);
    
    void Write_Binary_STP_Data(FILE *InputFile); 
    void Read_Binary_STP_Data(FILE *InputFile);    
    
};

#endif
