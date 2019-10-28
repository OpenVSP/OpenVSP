//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef COMPONENT_GROUP_H
#define COMPONENT_GROUP_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.H"
#include "quat.H"
#include "matrix.H"
#include "WOPWOP.H"
#include "SpanLoadData.H"

#define STEADY_RATES   1
#define PERIODIC_RATES 2
#define FULL_DYNAMIC   3

class COMPONENT_GROUP {

private:

    char GroupName_[2000];
    
    int NumberOfComponents_;
    int *ComponentList_;
    
    int GeometryIsFixed_;
    int GeometryIsDynamic_;
    int GeometryIsARotor_;
    int GeometryHasWings_;
    int GeometryHasBodies_;
    
    WOPWOP WopWop_;
    
    double OVec_[3];
    double RVec_[3];
    double TVec_[3];
    
    double Velocity_[3];
    double Acceleration_[3];
    double Mass_;
    double Ixx_;
    double Iyy_;
    double Izz_;
    double Ixy_;
    double Ixz_;
    double Iyz_;
    double Omega_;
    double Angle_;
    double TimeStep_;
    double CurrentTime_;
    double RotorDiameter_;
    double AngleMax_;
    double TotalRotationAngle_;
    
    // Integrated forces
    
    int NumberOfTimeSamples_;
    
    double StartAverageTime_;

    double Cxo_[2];
    double Cyo_[2];
    double Czo_[2];
    
    double Cx_[2];
    double Cy_[2];
    double Cz_[2];

    double Cmxo_[2];
    double Cmyo_[2];
    double Cmzo_[2];

    double Cmx_[2];
    double Cmy_[2];
    double Cmz_[2];

    double CL_[2];
    double CD_[2];
    double CS_[2];

    double CLo_[2];
    double CDo_[2];
    double CSo_[2];
    
    // Free stream conditions
    
    double Density_;
    double Vref_;
    double Sref_;
    double Bref_;  
    double Cref_;  
    
    // Spanwise forces and moments, for wings and rotors
    
    int NumberOfSurfaces_;
    
    SPAN_LOAD_DATA *SpanLoadData_;
    
    MATRIX M_;
    MATRIX I_;
    MATRIX OmegaDot_;
    MATRIX XDot_;
    MATRIX VDot_;
    
    QUAT Quat_;
    QUAT InvQuat_;
    QUAT WQuat_;

    void UpdateSteadyRates(void);
    void UpdatePeriodicRates(void);
    void UpdateDynamicSystem(void);

public:

    COMPONENT_GROUP(void);
   ~COMPONENT_GROUP(void);
    COMPONENT_GROUP(const COMPONENT_GROUP &ComponentGroup);

    COMPONENT_GROUP& operator=(const COMPONENT_GROUP &ComponentGroup);
    
    char *GroupName(void) { return GroupName_; };

    int NumberOfComponents(void) { return NumberOfComponents_; };
    
    int &ComponentList(int i) { return ComponentList_[i]; };

    int &GeometryIsFixed(void) { return GeometryIsFixed_; };
    
    int &GeometryIsDynamic(void) { return GeometryIsDynamic_; };
    
    int &GeometryIsARotor(void) { return GeometryIsARotor_; };
    
    int &GeometryHasWings(void) { return GeometryHasWings_; };
    
    int &GeometryHasBodies(void) { return GeometryHasBodies_; };

    WOPWOP &WopWop(void) { return WopWop_; };
    
    double &RotorDiameter(void) { return RotorDiameter_; };
    
    double &AngleMax(void) { return AngleMax_; };
    
    // Integrated forces and moments
    
    double &Cxo(void) { return Cxo_[0]; };
    double &Cyo(void) { return Cyo_[0]; };
    double &Czo(void) { return Czo_[0]; };
        
    double &Cx(void) { return Cx_[0]; };
    double &Cy(void) { return Cy_[0]; };
    double &Cz(void) { return Cz_[0]; };

    double &Cmxo(void) { return Cmxo_[0]; };
    double &Cmyo(void) { return Cmyo_[0]; };
    double &Cmzo(void) { return Cmzo_[0]; };
    
    double &Cmx(void) { return Cmx_[0]; };
    double &Cmy(void) { return Cmy_[0]; };
    double &Cmz(void) { return Cmz_[0]; };

    double &CLo(void) { return CLo_[0]; };
    double &CDo(void) { return CDo_[0]; };
    double &CSo(void) { return CSo_[0]; };        

    double &CL(void) { return CL_[0]; };
    double &CD(void) { return CD_[0]; };
    double &CS(void) { return CS_[0]; };
        
    // Average integrated forces and moments
    
    double &CxoAvg(void) { return Cxo_[1]; };
    double &CyoAvg(void) { return Cyo_[1]; };
    double &CzoAvg(void) { return Czo_[1]; };
    
    double &CxAvg(void) { return Cx_[1]; };
    double &CyAvg(void) { return Cy_[1]; };
    double &CzAvg(void) { return Cz_[1]; };

    double &CmxoAvg(void) { return Cmxo_[1]; };
    double &CmyoAvg(void) { return Cmyo_[1]; };
    double &CmzoAvg(void) { return Cmzo_[1]; };
    
    double &CmxAvg(void) { return Cmx_[1]; };
    double &CmyAvg(void) { return Cmy_[1]; };
    double &CmzAvg(void) { return Cmz_[1]; };

    double &CLoAvg(void) { return CLo_[1]; };
    double &CDoAvg(void) { return CDo_[1]; };
    double &CSoAvg(void) { return CSo_[1]; };      

    double &CLAvg(void) { return CL_[1]; };
    double &CDAvg(void) { return CD_[1]; };
    double &CSAvg(void) { return CS_[1]; };
        
    double &StartAverageTime(void) { return StartAverageTime_; };
    
    void ZeroAverageForcesAndMoments(void);
    void UpdateAverageForcesAndMoments(void);
    void CalculateAverageForcesAndMoments(void);

    // Free stream conditions
    
    double Density(void) { return Density_; };
    double Vref(void) { return Vref_; };
    double Sref(void) { return Sref_; };
    double Bref(void) { return Bref_; };
    double Cref(void) { return Cref_; };    
        
    // Spanwise forces and moments, for wings and rotors
    
    void SizeSpanLoadingList(int NumberOfSurfaces);
    
    int NumberOfSurfaces(void) { return NumberOfSurfaces_; };
    
    SPAN_LOAD_DATA &SpanLoadData(int i) { return SpanLoadData_[i]; };
    
    // Geometry, orientation, rotation information
        
    double &OVec(int i) { return OVec_[i]; };
    double *OVec(void) { return OVec_; };

    double &RVec(int i) { return RVec_[i]; };
    double *RVec(void) { return RVec_; };

    double TVec(int i) { return TVec_[i]; };
    
    double &Velocity(int i ) { return Velocity_[i]; };
    double *Velocity(void) { return Velocity_; };    
        
    double &Omega(void) { return Omega_; };   
    
    double Period(void) { return ABS(2.*PI/Omega_); };
    
    double &TotalRotationAngle(void) { return TotalRotationAngle_; };
    
    double Angle(void) { return Angle_; }; 
       
    QUAT &Quat(void) { return Quat_; };
    QUAT &InvQuat(void) { return InvQuat_; };
    QUAT &WQuat(void) { return WQuat_; };
    
    void SizeList(int NumberOfComponents);

    void Update(double TimeStep, double CurrentTime);
    
    void WriteData(FILE *File);
    void LoadData(FILE *File);

};

#endif
