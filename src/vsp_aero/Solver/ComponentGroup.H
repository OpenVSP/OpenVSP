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
#include "CharSizes.H"

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

    char GroupName_[MAX_CHAR_SIZE];
    
    int NumberOfComponents_;
    int *ComponentList_;
    
    int GeometryIsFixed_;
    int GeometryIsDynamic_;
    int GeometryIsARotor_;
    int GeometryHasWings_;
    int GeometryHasBodies_;

    double OVec_[3];
    double RVec_[3];
    double TVec_[3];
    
    double UserInputVelocity_[3];
    double UserInputAcceleration_[3];

    double Mass_;
    double Ixx_;
    double Iyy_;
    double Izz_;
    double Ixy_;
    double Ixz_;
    double Iyz_;
    double Omega_;
    double RPM_;
    double Angle_;
    double DeltaTime_;
    double CurrentTime_;
    double RotorDiameter_;
    double AngleMax_;
    double TotalRotationAngle_;
    double StartDynamicAnalysisTime_;
    double DeltaFlatPlateDragArea_;
    double FlatPlateDragRefReNumber_;
    
    // Integrated forces
    
    int NumberOfTimeSamples_;
    
    // Invisicid forces and moments
    
    double *CLi_;
    double *CDi_;
    double *CSi_;
          
    double *CFix_;
    double *CFiy_;
    double *CFiz_;
          
    double *CMix_;
    double *CMiy_;
    double *CMiz_;
          
    double *CTi_;
    double *CQi_;
    double *CPi_;
          
    double *CTi_h_;
    double *CQi_h_;
    double *CPi_h_;
             
    // Viscous forces and moments
   
    double *CLo_;
    double *CSo_;
    double *CDo_;
           
    double *CFox_;
    double *CFoy_;
    double *CFoz_;
          
    double *CMox_;
    double *CMoy_;
    double *CMoz_;
          
    double *CTo_;
    double *CQo_;
    double *CPo_;
          
    double *CTo_h_;
    double *CQo_h_;
    double *CPo_h_;
    
    // Inviscid wake forces
    
    double *CLiw_;
    double *CDiw_;
    double *CSiw_;
           
    double *CFiwx_;
    double *CFiwy_;
    double *CFiwz_;    

    // Efficiencies
    
    double *FOM_;
    double *EtaP_;

    // Free stream conditions
    
    double Density_;
    double Vref_;
    double Sref_;
    double Bref_;  
    double Cref_;  
    
    // Invsicid force gradients wrt inputs
    
    double *DCFxi_DOmega_;
    double *DCFyi_DOmega_;
    double *DCFzi_DOmega_;
          
    double *DCMxi_DOmega_;
    double *DCMyi_DOmega_;
    double *DCMzi_DOmega_;

    // Viscous force gradients wrt inputs
    
    double *DCFxo_DOmega_;
    double *DCFyo_DOmega_;
    double *DCFzo_DOmega_;
          
    double *DCMxo_DOmega_;
    double *DCMyo_DOmega_;
    double *DCMzo_DOmega_;

    // Dimensional forces
    
    double CalculateThrust(int DragComponent, int TimeType);
    double CalculateMoment(int DragComponent, int TimeType);
    double CalculatePower(int DragComponent, int TimeType);
    
    // Spanwise forces and moments, for wings and rotors
    
    int NumberOfLiftingSurfaces_;

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
        
    QUAT TotalQuat_;     // Total quat ... full rotation over all time
    QUAT Quat_;          // Current quat, just rotation over current time step
    QUAT InvQuat_;       // Inverse of current quat
    QUAT WQuat_;         // Rate derivative of current quat, for velocities
    QUAT pWQuat_pOmega_; // Partial of WQuat wrt Omega
    
    void UpdateSteadyRates(void);
    void UpdatePeriodicRates(void);
    void UpdateDynamicSystem(void);
    
    void QuatToMatrix(QUAT &Quat, MATRIX &Matrix, MATRIX &Inverse);
    
    // Size time lists
    
    void SizeForceAndMomentsTables_(void);
    
    // Do the time averaged calculation of the forces and moments
    
    void CalculateAverageForcesAndMoments_(int StartSample);
        
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
  
    /** Group is a rotor with some periodic motion about an axis 
        1 --> Rotor is to use the quasi 1D rotor model for wake down wash in unsteady flows
        2 --> Rotor is to use full unsteady wake analysis in unsteady flows **/
    
    int &GeometryIsARotor(void) { return GeometryIsARotor_; };
    
    /** Group contains 1 or more wings **/
     
    int &GeometryHasWings(void) { return GeometryHasWings_; };
    
    /** Groups contains 1 or more bodies **/
    
    int &GeometryHasBodies(void) { return GeometryHasBodies_; };

    /** Diameter of rotor, could be zero if no rotor exists **/
    
    double &RotorDiameter(void) { return RotorDiameter_; };
    
    /** Maximum angle for dynamic, periodic, motions **/
    
    double &AngleMax(void) { return AngleMax_; };

    // Time averaged invisicid forces and moments
    
    double &CLi_avg(void)   { return   CLi_[0]; };
    double &CDi_avg(void)   { return   CDi_[0]; };
    double &CSi_avg(void)   { return   CSi_[0]; };
                                
    double &CFix_avg(void)  { return  CFix_[0]; };
    double &CFiy_avg(void)  { return  CFiy_[0]; };
    double &CFiz_avg(void)  { return  CFiz_[0]; };
                               
    double &CMix_avg(void)  { return  CMix_[0]; };
    double &CMiy_avg(void)  { return  CMiy_[0]; };
    double &CMiz_avg(void)  { return  CMiz_[0]; };
                    
    double &CTi_avg(void)   { return   CTi_[0]; };
    double &CQi_avg(void)   { return   CQi_[0]; }; 
    double &CPi_avg(void)   { return   CPi_[0]; };
         
    double &CTi_h_avg(void) { return CTi_h_[0]; };
    double &CQi_h_avg(void) { return CQi_h_[0]; };
    double &CPi_h_avg(void) { return CPi_h_[0]; };
             
    // Time averaged viscous forces and moments
   
    double &CLo_avg(void)   { return   CLo_[0]; };
    double &CDo_avg(void)   { return   CDo_[0]; };
    double &CSo_avg(void)   { return   CSo_[0]; };
                                  
    double &CFox_avg(void)  { return  CFox_[0]; };
    double &CFoy_avg(void)  { return  CFoy_[0]; };
    double &CFoz_avg(void)  { return  CFoz_[0]; };
                                
    double &CMox_avg(void)  { return  CMox_[0]; };
    double &CMoy_avg(void)  { return  CMoy_[0]; };
    double &CMoz_avg(void)  { return  CMoz_[0]; };
                
    double &CTo_avg(void)   { return   CTo_[0]; };
    double &CQo_avg(void)   { return   CQo_[0]; }; 
    double &CPo_avg(void)   { return   CPo_[0]; };
           
    double &CTo_h_avg(void) { return CTo_h_[0]; };
    double &CQo_h_avg(void) { return CQo_h_[0]; };
    double &CPo_h_avg(void) { return CPo_h_[0]; };
    
    // Time averaged inviscid wake forces
    
    double &CLiw_avg(void) { return CLiw_[0]; };
    double &CDiw_avg(void) { return CDiw_[0]; };
    double &CSiw_avg(void) { return CSiw_[0]; };
                               
    double &CFiwx_avg(void) { return CFiwx_[0]; };
    double &CFiwy_avg(void) { return CFiwy_[0]; };
    double &CFiwz_avg(void) { return CFiwz_[0]; };

    /** Time averaged Propeller efficiency **/
         
    double &EtaP_avg(void) { return EtaP_[0]; };

    /** Time averaged Helicopter rotor figure of merit **/

    double &FOM_avg(void) { return FOM_[0]; };  
    
    // Steady state or current time step invisicid forces and moments
    
    double &CLi(void)   { return   CLi_[1]; };
    double &CDi(void)   { return   CDi_[1]; };
    double &CSi(void)   { return   CSi_[1]; };
                                   
    double &CFix(void)  { return  CFix_[1]; };
    double &CFiy(void)  { return  CFiy_[1]; };
    double &CFiz(void)  { return  CFiz_[1]; };
                                     
    double &CMix(void)  { return  CMix_[1]; };
    double &CMiy(void)  { return  CMiy_[1]; };
    double &CMiz(void)  { return  CMiz_[1]; };
                                  
    double &CTi(void)   { return   CTi_[1]; };
    double &CQi(void)   { return   CQi_[1]; }; 
    double &CPi(void)   { return   CPi_[1]; };
                                   
    double &CTi_h(void) { return CTi_h_[1]; };
    double &CQi_h(void) { return CQi_h_[1]; };
    double &CPi_h(void) { return CPi_h_[1]; };
             
    // Steady state or current time step viscous forces and moments
   
    double &CLo(void)   { return   CLo_[1]; };
    double &CDo(void)   { return   CDo_[1]; };
    double &CSo(void)   { return   CSo_[1]; };
                                
    double &CFox(void)  { return  CFox_[1]; };
    double &CFoy(void)  { return  CFoy_[1]; };
    double &CFoz(void)  { return  CFoz_[1]; };
                                    
    double &CMox(void)  { return  CMox_[1]; };
    double &CMoy(void)  { return  CMoy_[1]; };
    double &CMoz(void)  { return  CMoz_[1]; };
                                  
    double &CTo(void)   { return   CTo_[1]; };
    double &CQo(void)   { return   CQo_[1]; }; 
    double &CPo(void)   { return   CPo_[1]; };
                                  
    double &CTo_h(void) { return CTo_h_[1]; };
    double &CQo_h(void) { return CQo_h_[1]; };
    double &CPo_h(void) { return CPo_h_[1]; };

    // Steady state or current time step inviscid wake forces
    
    double &CLiw(void) { return CLiw_[1]; };
    double &CDiw(void) { return CDiw_[1]; };
    double &CSiw(void) { return CSiw_[1]; };
                                    
    double &CFiwx(void) { return CFiwx_[1]; };
    double &CFiwy(void) { return CFiwy_[1]; };
    double &CFiwz(void) { return CFiwz_[1]; };
    
    /** Steady state or current time step Propeller efficiency **/
         
    double &EtaP(void) { return EtaP_[1]; };

    /** Steady state or current time step Helicopter rotor figure of merit **/

    double &FOM(void) { return FOM_[1]; };  
    
    // Unsteady invisicid forces and moments
    
    double &CLi(int i)   { return   CLi_[i+1]; };
    double &CDi(int i)   { return   CDi_[i+1]; };
    double &CSi(int i)   { return   CSi_[i+1]; };
                                          
    double &CFix(int i)  { return  CFix_[i+1]; };
    double &CFiy(int i)  { return  CFiy_[i+1]; };
    double &CFiz(int i)  { return  CFiz_[i+1]; };
                                       
    double &CMix(int i)  { return  CMix_[i+1]; };
    double &CMiy(int i)  { return  CMiy_[i+1]; };
    double &CMiz(int i)  { return  CMiz_[i+1]; };
                                          
    double &CTi(int i)   { return   CTi_[i+1]; };
    double &CQi(int i)   { return   CQi_[i+1]; }; 
    double &CPi(int i)   { return   CPi_[i+1]; };
                                         
    double &CTi_h(int i) { return CTi_h_[i+1]; };
    double &CQi_h(int i) { return CQi_h_[i+1]; };
    double &CPi_h(int i) { return CPi_h_[i+1]; };
                                         
    // Unsteady viscous forces and moments+
                                          
    double &CLo(int i)   { return   CLo_[i+1]; };
    double &CDo(int i)   { return   CDo_[i+1]; };
    double &CSo(int i)   { return   CSo_[i+1]; };
                                          
    double &CFox(int i)  { return  CFox_[i+1]; };
    double &CFoy(int i)  { return  CFoy_[i+1]; };
    double &CFoz(int i)  { return  CFoz_[i+1]; };
                                          
    double &CMox(int i)  { return  CMox_[i+1]; };
    double &CMoy(int i)  { return  CMoy_[i+1]; };
    double &CMoz(int i)  { return  CMoz_[i+1]; };
                                         
    double &CTo(int i)   { return   CTo_[i+1]; };
    double &CQo(int i)   { return   CQo_[i+1]; }; 
    double &CPo(int i)   { return   CPo_[i+1]; };
                                        
    double &CTo_h(int i) { return CTo_h_[i+1]; };
    double &CQo_h(int i) { return CQo_h_[i+1]; };
    double &CPo_h(int i) { return CPo_h_[i+1]; };

    // Unsteady inviscid wake forces
    
    double &CLiw(int i) { return CLiw_[i+1]; };
    double &CDiw(int i) { return CDiw_[i+1]; };
    double &CSiw(int i) { return CSiw_[i+1]; };
                                    
    double &CFiwx(int i) { return CFiwx_[i+1]; };
    double &CFiwy(int i) { return CFiwy_[i+1]; };
    double &CFiwz(int i) { return CFiwz_[i+1]; };
    
    /** Unsteady Propeller efficiency **/
         
    double &EtaP(int i) { return EtaP_[i+1]; };

    /** Unsteady Helicopter rotor figure of merit **/

    double &FOM(int i) { return FOM_[i+1]; };  

    // Steady Invsicid force gradients wrt inputs
    
    double &DCFxi_DOmega(void) { return DCFxi_DOmega_[1]; };
    double &DCFyi_DOmega(void) { return DCFyi_DOmega_[1]; };
    double &DCFzi_DOmega(void) { return DCFzi_DOmega_[1]; };
                                                   
    double &DCMxi_DOmega(void) { return DCMxi_DOmega_[1]; };
    double &DCMyi_DOmega(void) { return DCMyi_DOmega_[1]; };
    double &DCMzi_DOmega(void) { return DCMzi_DOmega_[1]; };
                                                      
    // Steady Viscous force gradients wrt inputs           
                                                      
    double &DCFxo_DOmega(void) { return DCFxo_DOmega_[1]; };
    double &DCFyo_DOmega(void) { return DCFyo_DOmega_[1]; };
    double &DCFzo_DOmega(void) { return DCFzo_DOmega_[1]; };
                                                     
    double &DCMxo_DOmega(void) { return DCMxo_DOmega_[1]; };
    double &DCMyo_DOmega(void) { return DCMyo_DOmega_[1]; };
    double &DCMzo_DOmega(void) { return DCMzo_DOmega_[1]; };

    // Unsteady Invsicid force gradients wrt inputs
    
    double &DCFxi_DOmega(int i) { return DCFxi_DOmega_[i+1]; };
    double &DCFyi_DOmega(int i) { return DCFyi_DOmega_[i+1]; };
    double &DCFzi_DOmega(int i) { return DCFzi_DOmega_[i+1]; };
                                                        
    double &DCMxi_DOmega(int i) { return DCMxi_DOmega_[i+1]; };
    double &DCMyi_DOmega(int i) { return DCMyi_DOmega_[i+1]; };
    double &DCMzi_DOmega(int i) { return DCMzi_DOmega_[i+1]; };
                                                        
    // Unsteady Viscous force gradients wrt inputs       
                                                        
    double &DCFxo_DOmega(int i) { return DCFxo_DOmega_[i+1]; };
    double &DCFyo_DOmega(int i) { return DCFyo_DOmega_[i+1]; };
    double &DCFzo_DOmega(int i) { return DCFzo_DOmega_[i+1]; };
                                                       
    double &DCMxo_DOmega(int i) { return DCMxo_DOmega_[i+1]; };
    double &DCMyo_DOmega(int i) { return DCMyo_DOmega_[i+1]; };
    double &DCMzo_DOmega(int i) { return DCMzo_DOmega_[i+1]; };
        
 //  /** Calculate time averaged rotor thrust **/ 
 //
 //  double Thrusto_avg(void) { return CalculateThrust(VISCOUS_FORCES,  0); };        
 //  double Thrusti_avg(void) { return CalculateThrust(INVISCID_FORCES, 0); };  
 //  
 //  /** Steady state or current time step rotor thrust **/ 
 //  
 //  double Thrusto(void) { return CalculateThrust(VISCOUS_FORCES,  1); };  
 //  double Thrusti(void) { return CalculateThrust(INVISCID_FORCES, 1); };  
 //  
 //  /** Unsteady rotor thrust **/ 
 //  
 //  double Thrusto(int i) { return CalculateThrust(VISCOUS_FORCES,  i+1); };  
 //  double Thrusti(int i) { return CalculateThrust(INVISCID_FORCES, i+1); };  
 //  
 //  /** Calculate time averaged rotor moment **/ 
 //  
 //  double Momento_avg(void) { return CalculateMoment(VISCOUS_FORCES,  0); };  
 //  double Momenti_avg(void) { return CalculateMoment(INVISCID_FORCES, 0); };  
 //      
 //  /** Steady state or current time step rotor moment **/ 
 //      
 //  double Momento(void) { return CalculateMoment(VISCOUS_FORCES,  1); };  
 //  double Momenti(void) { return CalculateMoment(INVISCID_FORCES, 1); };  
 //
 //  /** Unsteady rotor moment **/ 
 //
 //  double Momento(int i) { return CalculateMoment(VISCOUS_FORCES,  i+1); };  
 //  double Momenti(int i) { return CalculateMoment(INVISCID_FORCES, i+1); };  
 //
 //  /** Calculate time averaged rotor power **/
 //
 //  double Powero_avg(void) { return CalculatePower(VISCOUS_FORCES,  0); };  
 //  double Poweri_avg(void) { return CalculatePower(INVISCID_FORCES, 0); };  
 //   
 //  /** Steady state or current time step rotor power **/ 
 //   
 //  double Powero(void) { return CalculatePower(VISCOUS_FORCES,  1); };  
 //  double Poweri(void) { return CalculatePower(INVISCID_FORCES, 1); };      
 //
 //  /** Unsteady rotor power **/ 
 //
 //  double Powero(int i) { return CalculatePower(VISCOUS_FORCES,  i+1); };  
 //  double Poweri(int i) { return CalculatePower(INVISCID_FORCES, i+1); };      

    /** Zero out the forces and moments **/
    
    void ZeroForcesAndMoments(void);

    /** Do the time averaged calculation of the forces and moments **/
    
    void CalculateAverageForcesAndMoments(void);
    
    /** Do the time averaged calculation of the forces and moments ... starting the time = StartSample **/
    
    void CalculateAverageForcesAndMoments(int StartSample);

    /** Free stream density **/
    
    double &Density(void) { return Density_; };
    
    /** Free stream velocity **/
    
    double &Vref(void) { return Vref_; };
    
    /** Reference Area **/
    
    double &Sref(void) { return Sref_; };
    
    /** Reference span **/
    
    double &Bref(void) { return Bref_; };
    
    /** Refererence chord **/
    
    double &Cref(void) { return Cref_; };    

    /** Access the number of lifting surfaces in this component group **/
    
    int NumberOfLiftingSurfaces(void) { return NumberOfLiftingSurfaces_; };

    /** Rotation origin, also used for moment calculations **/
        
    double &OVec(int i) { return OVec_[i]; };
    
    /** Rotation origin, also used for moment calculations **/

    double *OVec(void) { return OVec_; };
    
    /** Axis of rotation **/

    double &RVec(int i) { return RVec_[i]; };
    
    /** Axis of rotation **/
    
    double *RVec(void) { return RVec_; };

    /** Translation vector to be applied in transformations **/

    double TVec(int i) { return TVec_[i]; };

    /** Velocity of group **/
        
    double &Velocity(int i ) { return Velocity_(i+1); };
    
    /** Angular velocity of group... typically used to model a rotor **/
     
    double &Omega(void) { return Omega_; };   
    
    /** RPM of group... typically used to model a rotor **/
   
    double RPM(void) { return RPM_; };   
    
    /** Period, based on user supplied Omega **/
    
    double Period(void) { return ABS(2.*PI/Omega_); };
    
    /** Total rotation angle that has occurred thus far in an unsteady calculation **/
    
    double &TotalRotationAngle(void) { return TotalRotationAngle_; };
    
    /** Start time of dynamic analysis **/
    
    double StartDynamicAnalysisTime(void) { return StartDynamicAnalysisTime_; }; 
    
    /** Additional flat plate drag area for this group **/
    
    double DeltaFlatPlateDragArea(void) { return DeltaFlatPlateDragArea_; };
    
    /** Flat plate drag reference reynolds number **/
    
    double FlatPlateDragRefReNumber(void) { return FlatPlateDragRefReNumber_; };
    
    /** Current delta rotation angle, typically timestep_ * omega_ ... note this USUALLY computed by the class, not set... **/
        
    double &Angle(void) { return Angle_; }; 
    
    /** Access to quaternion calculated based on user inputs, to be applied to geometry in this group **/
       
    QUAT &Quat(void) { return Quat_; };
    
    /** Access to inverse quaternion calculated based on user inputs, to be applied to geometry in this group **/
    
    QUAT &InvQuat(void) { return InvQuat_; };
    
    /** Access to quaternion derivative, calculated based on user inputs, to be applied to geometry in this group **/
    
    QUAT &WQuat(void) { return WQuat_; };
    
    /** Access to partial of quaternion derivative wrt omega, calculated based on user inputs, to be applied to geometry in this group **/

    QUAT &pWQuat_pOmega(void) { return pWQuat_pOmega_; };

    /** Total quaternion transformation applied up to this point in the unsteady calculation for this group **/
    
    QUAT &TotalQuat(void) { return TotalQuat_; };
    
    /** Size the list for the number of components **/
    
    void SizeList(int NumberOfComponents);
    
    /** Set the number of time samples to store, defaults to a single value for steady cases **/
    
    void SizeForceAndMomentsTables(void);

    /** Set the number of time samples to store **/

    void SizeForceAndMomentsTables(int NumberOfTimeSamples);

    /** Zero out total angles, quats ... **/
    
    void ZeroOutTotals(void);
    
    /** Update the state of the group **/

    void Update(void);
    
    /** Update the state of the group at current time, with current time step **/
    
    void Update(double TimeStep, double CurrentTime);
    
    /** Update the quaternions **/
    
    void UpdateQuaternions(double TimeStep);
    
    /** Write out file containing group information **/
    
    void WriteData(FILE *File);
    
    /** Read in data for this group from a file... typically this is reading in from the .groups file **/
    
    void LoadData(FILE *File);

};

#include "END_NAME_SPACE.H"

#endif
