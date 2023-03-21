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
#include "SpanLoadRotorData.H"

#include "START_NAME_SPACE.H"

#define STEADY_RATES   1
#define PERIODIC_RATES 2
#define FULL_DYNAMIC   3

#define INVISCID_FORCES 1
#define VISCOUS_FORCES  2

#define INSTANT_VALUE  0
#define AVG_VALUE      1

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
    
    VSPAERO_DOUBLE OVec_[3];
    VSPAERO_DOUBLE RVec_[3];
    VSPAERO_DOUBLE TVec_[3];
    
    VSPAERO_DOUBLE UserInputVelocity_[3];
    VSPAERO_DOUBLE UserInputAcceleration_[3];

    VSPAERO_DOUBLE Mass_;
    VSPAERO_DOUBLE Ixx_;
    VSPAERO_DOUBLE Iyy_;
    VSPAERO_DOUBLE Izz_;
    VSPAERO_DOUBLE Ixy_;
    VSPAERO_DOUBLE Ixz_;
    VSPAERO_DOUBLE Iyz_;
    VSPAERO_DOUBLE Omega_;
    VSPAERO_DOUBLE Angle_;
    VSPAERO_DOUBLE TimeStep_;
    VSPAERO_DOUBLE CurrentTime_;
    VSPAERO_DOUBLE RotorDiameter_;
    VSPAERO_DOUBLE AngleMax_;
    VSPAERO_DOUBLE TotalRotationAngle_;
    VSPAERO_DOUBLE StartDynamicAnalysisTime_;
    
    // Integrated forces
    
    int NumberOfTimeSamples_;
    
    int NumberOfIntegrationTimeSteps_;

    VSPAERO_DOUBLE StartAveragingTime_;

    VSPAERO_DOUBLE Cxo_[2];
    VSPAERO_DOUBLE Cyo_[2];
    VSPAERO_DOUBLE Czo_[2];
    
    VSPAERO_DOUBLE Cx_[2];
    VSPAERO_DOUBLE Cy_[2];
    VSPAERO_DOUBLE Cz_[2];

    VSPAERO_DOUBLE Cmxo_[2];
    VSPAERO_DOUBLE Cmyo_[2];
    VSPAERO_DOUBLE Cmzo_[2];

    VSPAERO_DOUBLE Cmx_[2];
    VSPAERO_DOUBLE Cmy_[2];
    VSPAERO_DOUBLE Cmz_[2];

    VSPAERO_DOUBLE CL_[2];
    VSPAERO_DOUBLE CD_[2];
    VSPAERO_DOUBLE CS_[2];

    VSPAERO_DOUBLE CLo_[2];
    VSPAERO_DOUBLE CDo_[2];
    VSPAERO_DOUBLE CSo_[2];

    VSPAERO_DOUBLE CTo_[2];
    VSPAERO_DOUBLE CQo_[2];
    VSPAERO_DOUBLE CPo_[2];

    VSPAERO_DOUBLE CT_[2];
    VSPAERO_DOUBLE CQ_[2];
    VSPAERO_DOUBLE CP_[2];
    VSPAERO_DOUBLE EtaP_[2];

    VSPAERO_DOUBLE CTo_h_[2];
    VSPAERO_DOUBLE CQo_h_[2];
    VSPAERO_DOUBLE CPo_h_[2];
        
    VSPAERO_DOUBLE CT_h_[2];
    VSPAERO_DOUBLE CQ_h_[2];
    VSPAERO_DOUBLE CP_h_[2];
    VSPAERO_DOUBLE FOM_[2];

    // Free stream conditions
    
    VSPAERO_DOUBLE Density_;
    VSPAERO_DOUBLE Vref_;
    VSPAERO_DOUBLE Sref_;
    VSPAERO_DOUBLE Bref_;  
    VSPAERO_DOUBLE Cref_;  
    
    // Dimensional forces
    
    VSPAERO_DOUBLE CalculateThrust(int DragComponent, int TimeType);
    VSPAERO_DOUBLE CalculateMoment(int DragComponent, int TimeType);
    VSPAERO_DOUBLE CalculatePower(int DragComponent, int TimeType);
    
    // Spanwise forces and moments, for wings and rotors
    
    int NumberOfLiftingSurfaces_;
    
    SPAN_LOAD_ROTOR_DATA *SpanLoadData_;
    
    MATRIX MassMatrix_;
    MATRIX InertiaMatrix_;
    MATRIX InertiaMatrixInverse_;
    MATRIX RotationMatrix_;
    MATRIX RotationMatrixInverse_;
    MATRIX OmegaDot_;
    MATRIX LinearMomentum_;
    MATRIX AngularMomentum_;
    MATRIX XDot_;
    MATRIX VDot_;
    MATRIX Velocity_;
    MATRIX Acceleration_;
        
    QUAT TotalQuat_; // Total quat ... full rotation over all time
    QUAT Quat_;      // Current quat, just rotation over current time step
    QUAT InvQuat_;   // Inverse of current quat
    QUAT WQuat_;     // Rate derivative of current quat, for velocities
    
    void UpdateSteadyRates(void);
    void UpdatePeriodicRates(void);
    void UpdateDynamicSystem(void);
    
    void QuatToMatrix(QUAT &Quat, MATRIX &Matrix, MATRIX &Inverse);
    
public:

    COMPONENT_GROUP(void);
   ~COMPONENT_GROUP(void);
    COMPONENT_GROUP(const COMPONENT_GROUP &ComponentGroup);

    COMPONENT_GROUP& operator=(const COMPONENT_GROUP &ComponentGroup);
    
    /** Group name **/
    
    char *GroupName(void) { return GroupName_; };

    /** Number of components in this group **/
    
    int NumberOfComponents(void) { return NumberOfComponents_; };
    
    /** ith component in this group **/
    
    int &ComponentList(int i) { return ComponentList_[i]; };

    /** Group is fixed (rigid) in space and time **/
    
    int &GeometryIsFixed(void) { return GeometryIsFixed_; };
    
    /** Group is dynamic ... ie, under force and moments due to aero, etc **/
    
    int &GeometryIsDynamic(void) { return GeometryIsDynamic_; };
  
    /** Group is a rotor with some periodic motion about an axis **/
    
    int &GeometryIsARotor(void) { return GeometryIsARotor_; };
    
    /** Group contains 1 or more wings **/
     
    int &GeometryHasWings(void) { return GeometryHasWings_; };
    
    /** Groups contains 1 or more bodies **/
    
    int &GeometryHasBodies(void) { return GeometryHasBodies_; };

    /** WopWop flag **/
    
    WOPWOP &WopWop(void) { return WopWop_; };
    
    /** Diameter of rotor, could be zero if no rotor exists **/
    
    VSPAERO_DOUBLE &RotorDiameter(void) { return RotorDiameter_; };
    
    /** Maximum angle for dynamic, periodic, motions **/
    
    VSPAERO_DOUBLE &AngleMax(void) { return AngleMax_; };
    
    /** Integrated viscous force coefficients in x **/
    
    VSPAERO_DOUBLE &Cxo(void) { return Cxo_[0]; };
    
    /** Integrated viscous force coefficients in y **/
    
    VSPAERO_DOUBLE &Cyo(void) { return Cyo_[0]; };

    /** Integrated viscous force coefficients in z **/

    VSPAERO_DOUBLE &Czo(void) { return Czo_[0]; };
    
    /** Integrated inviscid force coefficients in x **/
            
    VSPAERO_DOUBLE &Cx(void) { return Cx_[0]; };
    
    /** Integrated inviscid force coefficients in y **/
    
    VSPAERO_DOUBLE &Cy(void) { return Cy_[0]; };

    /** Integrated inviscid force coefficients in z **/

    VSPAERO_DOUBLE &Cz(void) { return Cz_[0]; };

    /** Integrated viscous moment coefficients in x **/
     
    VSPAERO_DOUBLE &Cmxo(void) { return Cmxo_[0]; };
    
    /** Integrated viscous moment coefficients in y **/
    
    VSPAERO_DOUBLE &Cmyo(void) { return Cmyo_[0]; };

    /** Integrated viscous moment coefficients in z **/

    VSPAERO_DOUBLE &Cmzo(void) { return Cmzo_[0]; };

    /** Integrated inviscid moment coefficients in x **/
         
    VSPAERO_DOUBLE &Cmx(void) { return Cmx_[0]; };
    
    /** Integrated inviscid moment coefficients in y **/
    
    VSPAERO_DOUBLE &Cmy(void) { return Cmy_[0]; };

    /** Integrated inviscid moment coefficients in z **/

    VSPAERO_DOUBLE &Cmz(void) { return Cmz_[0]; };

    /** Viscous lift coefficient **/
     
    VSPAERO_DOUBLE &CLo(void) { return CLo_[0]; };
    
    /** Viscous drag coefficient **/
    
    VSPAERO_DOUBLE &CDo(void) { return CDo_[0]; };

    /** Viscous side force coefficient **/

    VSPAERO_DOUBLE &CSo(void) { return CSo_[0]; };        

    /** Inviscid lift coefficient **/
     
    VSPAERO_DOUBLE &CL(void) { return CL_[0]; };
    
    /** Inviscid drag coefficient **/
    
    VSPAERO_DOUBLE &CD(void) { return CD_[0]; };

    /** Inviscid side force coefficient **/

    VSPAERO_DOUBLE &CS(void) { return CS_[0]; };

    /** Propeller viscous thrust coefficient **/
     
    VSPAERO_DOUBLE &CTo(void) { return CTo_[0]; };
    
    /** Propeller viscous torque coefficient **/
    
    VSPAERO_DOUBLE &CQo(void) { return CQo_[0]; };
    
    /** Propeller viscous power coefficient **/
    
    VSPAERO_DOUBLE &CPo(void) { return CPo_[0]; };

    /** Propeller inviscid thrust coefficient **/
     
    VSPAERO_DOUBLE &CT(void) { return CT_[0]; };

    /** Propeller inviscid torque coefficient **/
    
    VSPAERO_DOUBLE &CQ(void) { return CQ_[0]; };

    /** Propeller inviscid power coefficient **/

    VSPAERO_DOUBLE &CP(void) { return CP_[0]; };
    

    /** Propeller efficiency **/
         
    VSPAERO_DOUBLE &EtaP(void) { return EtaP_[0]; };

    /** Helicopter rotor viscous thrust coefficient **/
     
    VSPAERO_DOUBLE &CTo_h(void) { return CTo_h_[0]; };
    
    /** Helicopter rotor viscous torque coefficient **/
    
    VSPAERO_DOUBLE &CQo_h(void) { return CQo_h_[0]; };

    /** Helicopter rotor viscous power coefficient **/

    VSPAERO_DOUBLE &CPo_h(void) { return CPo_h_[0]; };


    /** Helicopter rotor inviscid thrust coefficient **/
     
    VSPAERO_DOUBLE &CT_h(void) { return CT_h_[0]; };
    
    /** Helicopter rotor inviscid torque coefficient **/
    
    VSPAERO_DOUBLE &CQ_h(void) { return CQ_h_[0]; };

    /** Helicopter rotor inviscid power coefficient **/

    VSPAERO_DOUBLE &CP_h(void) { return CP_h_[0]; };

    /** Helicopter rotor figure of merit **/

    VSPAERO_DOUBLE &FOM(void) { return FOM_[0]; };  
   
    /** Time averaged viscous force coefficient in x **/
    
    VSPAERO_DOUBLE &CxoAvg(void) { return Cxo_[1]; };
    
    /** Time averaged viscous force coefficient in y **/
    
    VSPAERO_DOUBLE &CyoAvg(void) { return Cyo_[1]; };

    /** Time averaged viscous force coefficient in z **/

    VSPAERO_DOUBLE &CzoAvg(void) { return Czo_[1]; };

    /** Time averaged inviscid force coefficient in x **/
         
    VSPAERO_DOUBLE &CxAvg(void) { return Cx_[1]; };
    
    /** Time averaged inviscid force coefficient in y **/
    
    VSPAERO_DOUBLE &CyAvg(void) { return Cy_[1]; };

    /** Time averaged inviscid force coefficient in z **/

    VSPAERO_DOUBLE &CzAvg(void) { return Cz_[1]; };


    /** Time averaged viscous moment coefficient in x **/
     
    VSPAERO_DOUBLE &CmxoAvg(void) { return Cmxo_[1]; };
    
    /** Time averaged viscous moment coefficient in y **/
    
    VSPAERO_DOUBLE &CmyoAvg(void) { return Cmyo_[1]; };

    /** Time averaged viscous moment coefficient in z **/

    VSPAERO_DOUBLE &CmzoAvg(void) { return Cmzo_[1]; };

    /** Time averaged inviscid moment coefficient in x **/
         
    VSPAERO_DOUBLE &CmxAvg(void) { return Cmx_[1]; };
    
    /** Time averaged inviscid moment coefficient in y **/
    
    VSPAERO_DOUBLE &CmyAvg(void) { return Cmy_[1]; };

    /** Time averaged inviscid moment coefficient in z **/

    VSPAERO_DOUBLE &CmzAvg(void) { return Cmz_[1]; };

    /** Time averaged viscous lift coefficient **/
         
    VSPAERO_DOUBLE &CLoAvg(void) { return CLo_[1]; };
    
    /** Time averaged viscous drag coefficient **/
    
    VSPAERO_DOUBLE &CDoAvg(void) { return CDo_[1]; };

    /** Time averaged viscous side force coefficient **/

    VSPAERO_DOUBLE &CSoAvg(void) { return CSo_[1]; };      
 
    /** Time averaged inviscid lift coefficient **/
     
    VSPAERO_DOUBLE &CLAvg(void) { return CL_[1]; };
    
    /** Time averaged inviscid drag coefficient **/
    
    VSPAERO_DOUBLE &CDAvg(void) { return CD_[1]; };

    /** Time averaged inviscid side force coefficient **/

    VSPAERO_DOUBLE &CSAvg(void) { return CS_[1]; };

    /** Time averaged propeller viscous thrust coefficient **/
     
    VSPAERO_DOUBLE &CToAvg(void) { return CTo_[1]; };
    
    /** Time averaged propeller viscous torque coefficient **/
    
    VSPAERO_DOUBLE &CQoAvg(void) { return CQo_[1]; };

    /** Time averaged propeller viscous power coefficient **/

    VSPAERO_DOUBLE &CPoAvg(void) { return CPo_[1]; };

    /** Time averaged propeller inviscid thrust coefficient **/
          
    VSPAERO_DOUBLE &CTAvg(void) { return CT_[1]; };
    
    /** Time averaged propeller torque thrust coefficient **/
    
    VSPAERO_DOUBLE &CQAvg(void) { return CQ_[1]; };

    /** Time averaged propeller power thrust coefficient **/

    VSPAERO_DOUBLE &CPAvg(void) { return CP_[1]; };
    
    /** Time averaged propeller power efficiency **/
    
    VSPAERO_DOUBLE &EtaPAvg(void) { return EtaP_[1]; };

    /** Time averaged helicopter rotor viscous thrust coefficient **/
     
    VSPAERO_DOUBLE &CTo_hAvg(void) { return CTo_h_[1]; };
    
    /** Time averaged helicopter rotor viscous torque coefficient **/
    
    VSPAERO_DOUBLE &CQo_hAvg(void) { return CQo_h_[1]; };

    /** Time averaged helicopter rotor viscous power coefficient **/

    VSPAERO_DOUBLE &CPo_hAvg(void) { return CPo_h_[1]; };

    /** Time averaged helicopter rotor inviscid thrust coefficient **/
             
    VSPAERO_DOUBLE &CT_hAvg(void) { return CT_h_[1]; };
    
    /** Time averaged helicopter rotor inviscid torque coefficient **/
    
    VSPAERO_DOUBLE &CQ_hAvg(void) { return CQ_h_[1]; };

    /** Time averaged helicopter rotor inviscid power coefficient **/

    VSPAERO_DOUBLE &CP_hAvg(void) { return CP_h_[1]; };
    
    /** Time averaged helicopter rotor figure of merit **/
         
    VSPAERO_DOUBLE &FOMAvg(void) { return FOM_[1]; };  

    /** Time to begin averaging in an unsteady analysis **/
                
    VSPAERO_DOUBLE &StartAveragingTime(void) { return StartAveragingTime_; };
    
    /** Number of integration time steps for averaging process **/
    
    int &NumberOfIntegrationTimeSteps(void) { return NumberOfIntegrationTimeSteps_; };
    
    /** Zero out the forces and moments **/
    
    void ZeroAverageForcesAndMoments(void);
    
    /** Update the forces and moments **/
    
    void UpdateAverageForcesAndMoments(void);
    
    /** Do the time averaged calculation of the forces and moments **/
    
    void CalculateAverageForcesAndMoments(void);

    /** Free stream density **/
    
    VSPAERO_DOUBLE &Density(void) { return Density_; };
    
    /** Free stream velocity **/
    
    VSPAERO_DOUBLE &Vref(void) { return Vref_; };
    
    /** Reference Area **/
    
    VSPAERO_DOUBLE &Sref(void) { return Sref_; };
    
    /** Reference span **/
    
    VSPAERO_DOUBLE &Bref(void) { return Bref_; };
    
    /** Refererence chord **/
    
    VSPAERO_DOUBLE &Cref(void) { return Cref_; };    
        
    /** Size the spanwise loading arrays for **/
    
    void SizeSpanLoadingList(int NumberOfLiftingSurfaces);
    
    /** Access the number of lifting surfaces in this component group **/
    
    int NumberOfLiftingSurfaces(void) { return NumberOfLiftingSurfaces_; };
    
    /** Access to the span loading data objects **/
    
    SPAN_LOAD_ROTOR_DATA &SpanLoadData(int i) { return SpanLoadData_[i]; };
    
    /** Rotation origin, also used for moment calculations **/
        
    VSPAERO_DOUBLE &OVec(int i) { return OVec_[i]; };
    
    /** Rotation origin, also used for moment calculations **/

    VSPAERO_DOUBLE *OVec(void) { return OVec_; };
    
    /** Axis of rotation **/

    VSPAERO_DOUBLE &RVec(int i) { return RVec_[i]; };
    
    /** Axis of rotation **/
    
    VSPAERO_DOUBLE *RVec(void) { return RVec_; };

    /** Translation vector to be applied in transformations **/

    VSPAERO_DOUBLE TVec(int i) { return TVec_[i]; };

    /** Velocity of group **/
        
    VSPAERO_DOUBLE &Velocity(int i ) { return Velocity_(i+1); };
    
    /** Angular velocity of group... typicall used to model a rotor **/
     
    VSPAERO_DOUBLE &Omega(void) { return Omega_; };   
    
    /** Period, based on user supplied Omega **/
    
    VSPAERO_DOUBLE Period(void) { return ABS(2.*PI/Omega_); };
    
    /** Total rotation angle that has occurred thus far in an unsteady calculation **/
    
    VSPAERO_DOUBLE &TotalRotationAngle(void) { return TotalRotationAngle_; };
    
    /** Current rotation angle, typically timestep_ * omega_ **/
        
    VSPAERO_DOUBLE Angle(void) { return Angle_; }; 
    
    /** Access to quaternion calculated based on user inputs, to be applied to geometry in this group **/
       
    QUAT &Quat(void) { return Quat_; };
    
    /** Access to inverse quaternion calculated based on user inputs, to be applied to geometry in this group **/
    
    QUAT &InvQuat(void) { return InvQuat_; };
    
    /** Access to quaternion derivative, calculated based on user inputs, to be applied to geometry in this group **/
    
    QUAT &WQuat(void) { return WQuat_; };
    
    /** Total quaternion transformation applied up to this point in the unsteady calculation for this group **/
    
    QUAT &TotalQuat(void) { return TotalQuat_; };
    
    /** Size the list for the number of components **/
    
    void SizeList(int NumberOfComponents);

    /** Zero out total angles, quats ... **/
    
    void ZeroOutTotals(void);
    
    /** Update the state of the group **/

    void Update(void);
    
    /** Update the state of the group at current time, with current time step **/
    
    void Update(VSPAERO_DOUBLE TimeStep, VSPAERO_DOUBLE CurrentTime);
    
    /** Update the quaternions **/
    
    void UpdateQuaternions(VSPAERO_DOUBLE TimeStep);
    
    /** Write out file containing group information **/
    
    void WriteData(FILE *File);
    
    /** Read in data for this group from a file... typically this is reading in from the .groups file **/
    
    void LoadData(FILE *File);

    /** Viscous thrust **/ 
     
    VSPAERO_DOUBLE Thrusto(void) { return CalculateThrust(VISCOUS_FORCES, INSTANT_VALUE); };  
    
    /** Invsicid thrust **/ 
    
    VSPAERO_DOUBLE Thrusti(void) { return CalculateThrust(INVISCID_FORCES, INSTANT_VALUE); };  
    
    /** Total thrust **/ 
    
    VSPAERO_DOUBLE  Thrust(void) { return ( Thrusti() + Thrusto() ); };

    /** Time averaged viscous thrust **/
           
    VSPAERO_DOUBLE Thrusto_avg(void) { return CalculateThrust(VISCOUS_FORCES, AVG_VALUE); };        
    
    /** Time averaged inviscid thrust **/
    
    VSPAERO_DOUBLE Thrusti_avg(void) { return CalculateThrust(INVISCID_FORCES, AVG_VALUE); };  

    /** Time averaged total thrust **/

    VSPAERO_DOUBLE  Thrust_avg(void) { return ( Thrusti_avg() + Thrusto_avg() ); };

    /** Viscous moment **/
    
    VSPAERO_DOUBLE Momento(void) { return CalculateMoment(VISCOUS_FORCES, INSTANT_VALUE); };  
    
    /** Invsicid moment **/
    
    VSPAERO_DOUBLE Momenti(void) { return CalculateMoment(INVISCID_FORCES, INSTANT_VALUE); };  

    /** Total moment **/

    VSPAERO_DOUBLE  Moment(void) { return ( Momenti() + Momento() ); };

    /**  Time averaged viscous moment **/
          
    VSPAERO_DOUBLE Momento_avg(void) { return CalculateMoment(VISCOUS_FORCES, AVG_VALUE); };  
    
    /**  Time averaged invsiscid moment **/
    
    VSPAERO_DOUBLE Momenti_avg(void) { return CalculateMoment(INVISCID_FORCES, AVG_VALUE); };  

    /**  Time averaged total moment **/

    VSPAERO_DOUBLE  Moment_avg(void) { return ( Momenti_avg() + Momento_avg() ); };

    /**  Viscous power **/
     
    VSPAERO_DOUBLE Powero(void) { return CalculatePower(VISCOUS_FORCES, INSTANT_VALUE); };  
    
    /**  Inviscid power **/
    
    VSPAERO_DOUBLE Poweri(void) { return CalculatePower(INVISCID_FORCES, INSTANT_VALUE); };      

    /**  Total power **/

    VSPAERO_DOUBLE  Power(void) { return ( Poweri() + Powero() ); };
  
    /**  Time averaged viscous power **/
     
    VSPAERO_DOUBLE Powero_avg(void) { return CalculatePower(VISCOUS_FORCES, AVG_VALUE); };  
    
    /**  Time averaged invsicid power **/
    
    VSPAERO_DOUBLE Poweri_avg(void) { return CalculatePower(INVISCID_FORCES, AVG_VALUE); };  

    /**  Time averaged total power **/

    VSPAERO_DOUBLE  Power_avg(void) { return ( Poweri_avg() + Powero_avg() ); };
    
};

#include "END_NAME_SPACE.H"

#endif
