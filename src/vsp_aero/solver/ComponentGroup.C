//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "ComponentGroup.H"

/*##############################################################################
#                                                                              #
#                         ComponentGroup Constructor                           #
#                                                                              #
##############################################################################*/

COMPONENT_GROUP::COMPONENT_GROUP(void)
{

    // Just zero the quat

    NumberOfComponents_ = 0;
    
    GeometryIsFixed_ = 0;
    
    GeometryIsDynamic_ = 0;
    
    GeometryIsARotor_ = 0;
    
    GeometryHasWings_ = 0;
    
    GeometryHasBodies_ = 0;
    
    ComponentList_ = NULL;
    
    OVec_[0] = OVec_[1] = OVec_[2] = 0.;
    RVec_[0] = RVec_[1] = RVec_[2] = 0.;
    TVec_[0] = TVec_[1] = TVec_[2] = 0.;

    Velocity_[0] = Velocity_[1] = Velocity_[2] = 0.;
    
    Acceleration_[0] = Acceleration_[1] = Acceleration_[2] = 0.;
    
    Mass_ = 0.;
    
    Ixx_ = 0.;
    
    Iyy_ = 0.;
    
    Izz_ = 0.;
    
    Ixy_ = 0.;
    
    Ixz_ = 0.;
    
    Iyz_ = 0.;
        
    Omega_ = 0.;
    
    Angle_  = 0.;
    
    TimeStep_ = 0.;
    
    RotorDiameter_ = 0.;
    
    AngleMax_= 0.;
    
    Quat_(0) = Quat_(1) = Quat_(2) = 0.;
    
    InvQuat_(0) = InvQuat_(1) = InvQuat_(2) = 0.;
    
    WQuat_(0) = WQuat_(1) = WQuat_(2) = 0.;
    
    TotalRotationAngle_ = 0.;
    
    NumberOfTimeSamples_ = 0;
    
    StartAverageTime_ = 0.;
    
    Cx_[0]   = Cx_[1]   = 0.;
    Cy_[0]   = Cy_[1]   = 0.;
    Cz_[0]   = Cz_[1]   = 0.;
                        
    Cmx_[0]  = Cmx_[1]  = 0.;
    Cmy_[0]  = Cmy_[1]  = 0.;
    Cmz_[0]  = Cmz_[1]  = 0.;
                        
    CL_[0]   = CL_[1]   = 0.;
    CD_[0]   = CD_[1]   = 0.;
    CS_[0]   = CS_[1]   = 0.;
                        
    Cxo_[0]  = Cxo_[1]  = 0.;
    Cyo_[0]  = Cyo_[1]  = 0.;
    Czo_[0]  = Czo_[1]  = 0.;
                        
    Cmxo_[0] = Cmxo_[1] = 0.;
    Cmyo_[0] = Cmyo_[1] = 0.;
    Cmzo_[0] = Cmzo_[1] = 0.;
                        
    CLo_[0]  = CLo_[1]  = 0.;
    CDo_[0]  = CDo_[1]  = 0.;
    CSo_[0]  = CSo_[1]  = 0.; 
    
    Vref_ = 0.;
    Sref_ = 0.;
    Cref_ = 0.;
    Bref_ = 0.;

}

/*##############################################################################
#                                                                              #
#                        COMPONENT_GROUP Destructor                            #
#                                                                              #
##############################################################################*/

COMPONENT_GROUP::~COMPONENT_GROUP(void)
{

    if ( ComponentList_ != NULL ) delete [] ComponentList_;
    
    NumberOfComponents_ = 0;
    
    GeometryIsFixed_ = 0;
    
    GeometryIsDynamic_ = 0;
    
    GeometryIsARotor_ = 0;
    
    GeometryHasWings_ = 0;
    
    GeometryHasBodies_ = 0;

    OVec_[0] = OVec_[1] = OVec_[2] = 0.;
    RVec_[0] = RVec_[1] = RVec_[2] = 0.;
    TVec_[0] = TVec_[1] = TVec_[2] = 0.;

    Velocity_[0] = Velocity_[1] = Velocity_[2] = 0.;
    
    Acceleration_[0] = Acceleration_[1] = Acceleration_[2] = 0.;
    
    Mass_ = 0.;
    
    Ixx_ = 0.;
    
    Iyy_ = 0.;
    
    Izz_ = 0.;
    
    Ixy_ = 0.;
    
    Ixz_ = 0.;
    
    Iyz_ = 0.;
    
    Omega_ = 0.;
    
    Angle_  = 0.;
    
    TimeStep_ = 0.;
    
    RotorDiameter_ = 0.;
    
    Quat_(0) = Quat_(1) = Quat_(2) = 0.;
    
    InvQuat_(0) = InvQuat_(1) = InvQuat_(2) = 0.;
    
    WQuat_(0) = WQuat_(1) = WQuat_(2) = 0.;
    
    TotalRotationAngle_ = 0.;
    
    NumberOfTimeSamples_ = 0;
    
    StartAverageTime_ = 0.;
    
    Cx_[0]   = Cx_[1]   = 0.;
    Cy_[0]   = Cy_[1]   = 0.;
    Cz_[0]   = Cz_[1]   = 0.;
                        
    Cmx_[0]  = Cmx_[1]  = 0.;
    Cmy_[0]  = Cmy_[1]  = 0.;
    Cmz_[0]  = Cmz_[1]  = 0.;
                        
    CL_[0]   = CL_[1]   = 0.;
    CD_[0]   = CD_[1]   = 0.;
    CS_[0]   = CS_[1]   = 0.;
                        
    Cxo_[0]  = Cxo_[1]  = 0.;
    Cyo_[0]  = Cyo_[1]  = 0.;
    Czo_[0]  = Czo_[1]  = 0.;
                        
    Cmxo_[0] = Cmxo_[1] = 0.;
    Cmyo_[0] = Cmyo_[1] = 0.;
    Cmzo_[0] = Cmzo_[1] = 0.;
                        
    CLo_[0]  = CLo_[1]  = 0.;
    CDo_[0]  = CDo_[1]  = 0.;
    CSo_[0]  = CSo_[1]  = 0.; 

}

/*##############################################################################
#                                                                              #
#                           COMPONENT_GROUP Copy                               #
#                                                                              #
##############################################################################*/

COMPONENT_GROUP::COMPONENT_GROUP(const COMPONENT_GROUP &ComponentGroup)
{

    // Use operator =

    (*this)= ComponentGroup;

}

/*##############################################################################
#                                                                              #
#                      COMPONENT_GROUP Operator =                              #
#                                                                              #
##############################################################################*/

COMPONENT_GROUP &COMPONENT_GROUP::operator=(const COMPONENT_GROUP &ComponentGroup)
{

    int i;

    NumberOfComponents_ = ComponentGroup.NumberOfComponents_;
    
    SizeList(NumberOfComponents_);
    
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       ComponentList_[i] = ComponentGroup.ComponentList_[i];
       
    }
    
    GeometryIsFixed_ = ComponentGroup.GeometryIsFixed_;
    
    GeometryIsARotor_ = ComponentGroup.GeometryIsARotor_;
    
    GeometryHasWings_ = ComponentGroup.GeometryHasWings_;
    
    GeometryHasBodies_ = ComponentGroup.GeometryHasBodies_;
    
    GeometryIsDynamic_ = 0;

    OVec_[0] = ComponentGroup.OVec_[0];
    OVec_[1] = ComponentGroup.OVec_[1];
    OVec_[2] = ComponentGroup.OVec_[2];

    RVec_[0] = ComponentGroup.RVec_[0];
    RVec_[1] = ComponentGroup.RVec_[1];
    RVec_[2] = ComponentGroup.RVec_[2];

    TVec_[0] = ComponentGroup.TVec_[0];
    TVec_[1] = ComponentGroup.TVec_[1];
    TVec_[2] = ComponentGroup.TVec_[2];
 
    Velocity_[0] = ComponentGroup.Velocity_[0];
    Velocity_[1] = ComponentGroup.Velocity_[1];
    Velocity_[2] = ComponentGroup.Velocity_[2];
    
    Acceleration_[0] = ComponentGroup.Acceleration_[0];
    Acceleration_[1] = ComponentGroup.Acceleration_[1];
    Acceleration_[2] = ComponentGroup.Acceleration_[2];
    
    Mass_ = ComponentGroup.Mass_;
    
    Ixx_ = ComponentGroup.Ixx_;
                          
    Iyy_ = ComponentGroup.Iyy_;
                          
    Izz_ = ComponentGroup.Izz_;
                          
    Ixy_ = ComponentGroup.Ixy_;
                          
    Ixz_ = ComponentGroup.Ixz_;
                          
    Iyz_ = ComponentGroup.Iyz_;
        
    Omega_ = ComponentGroup.Omega_;
    
    Angle_ = ComponentGroup.Angle_;
    
    TimeStep_ = ComponentGroup.TimeStep_;
    
    RotorDiameter_ = ComponentGroup.RotorDiameter_;
    
    AngleMax_ = ComponentGroup.AngleMax_;
    
    Quat_ = ComponentGroup.Quat_;
    
    InvQuat_ = ComponentGroup.InvQuat_;
    
    WQuat_ = ComponentGroup.WQuat_;
    
    TotalRotationAngle_ = ComponentGroup.TotalRotationAngle_;
    
    NumberOfTimeSamples_ = ComponentGroup.NumberOfTimeSamples_;
    
    StartAverageTime_ = ComponentGroup.StartAverageTime_;

    Cx_[0]   = ComponentGroup.Cx_[0];
    Cy_[0]   = ComponentGroup.Cy_[0];
    Cz_[0]   = ComponentGroup.Cz_[0];
          
    Cmx_[0]  = ComponentGroup.Cmx_[0];
    Cmy_[0]  = ComponentGroup.Cmy_[0];
    Cmz_[0]  = ComponentGroup.Cmz_[0];
          
    CL_[0]   = ComponentGroup.CL_[0];
    CD_[0]   = ComponentGroup.CD_[0];
    CS_[0]   = ComponentGroup.CS_[0];
          
    Cxo_[0]  = ComponentGroup.Cxo_[0];
    Cyo_[0]  = ComponentGroup.Cyo_[0];
    Czo_[0]  = ComponentGroup.Czo_[0];
          
    Cmxo_[0] = ComponentGroup.Cmxo_[0];
    Cmyo_[0] = ComponentGroup.Cmyo_[0];
    Cmzo_[0] = ComponentGroup.Cmzo_[0];
          
    CLo_[0]  = ComponentGroup.CLo_[0];
    CDo_[0]  = ComponentGroup.CDo_[0];
    CSo_[0]  = ComponentGroup.CSo_[0];
    
    Cx_[1]   = ComponentGroup.Cx_[1];
    Cy_[1]   = ComponentGroup.Cy_[1];
    Cz_[1]   = ComponentGroup.Cz_[1];
          
    Cmx_[1]  = ComponentGroup.Cmx_[1];
    Cmy_[1]  = ComponentGroup.Cmy_[1];
    Cmz_[1]  = ComponentGroup.Cmz_[1];
          
    CL_[1]   = ComponentGroup.CL_[1];
    CD_[1]   = ComponentGroup.CD_[1];
    CS_[1]   = ComponentGroup.CS_[1];
          
    Cxo_[1]  = ComponentGroup.Cxo_[1];
    Cyo_[1]  = ComponentGroup.Cyo_[1];
    Czo_[1]  = ComponentGroup.Czo_[1];
          
    Cmxo_[1] = ComponentGroup.Cmxo_[1];
    Cmyo_[1] = ComponentGroup.Cmyo_[1];
    Cmzo_[1] = ComponentGroup.Cmzo_[1];
          
    CLo_[1]  = ComponentGroup.CLo_[1];
    CDo_[1]  = ComponentGroup.CDo_[1];
    CSo_[1]  = ComponentGroup.CSo_[1];
        
    return *this;

}

/*##############################################################################
#                                                                              #
#                         COMPONENT_GROUP SizeList                             #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::SizeList(int NumberOfComponents)
{

    if ( ComponentList_ != NULL ) delete [] ComponentList_;

    NumberOfComponents_ = NumberOfComponents;
    
    ComponentList_ = new int[NumberOfComponents_ + 1];
    
    zero_int_array(ComponentList_, NumberOfComponents_);

}
  
 
/*##############################################################################
#                                                                              #
#                     COMPONENT_GROUP SizeSpanLoadingList                      #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::SizeSpanLoadingList(int NumberOfSurfaces)
{

    NumberOfSurfaces_ = NumberOfSurfaces;
    
    SpanLoadData_ = new SPAN_LOAD_DATA[NumberOfSurfaces + 1];

}           
            
/*##############################################################################
#                                                                              #
#                         COMPONENT_GROUP Update                               #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::Update(double TimeStep, double CurrentTime)
{

            
    // Time step and Current time
            
    TimeStep_ = TimeStep;
    
    CurrentTime_ = CurrentTime;
    
    // Steady rates or rotor analysis
       
    if ( GeometryIsDynamic_ == STEADY_RATES || GeometryIsFixed_ || GeometryIsARotor_ ) {
       
       UpdateSteadyRates();
       
    }
    
    // Periodic rates
    
    else if ( GeometryIsDynamic_ == PERIODIC_RATES ) {
       
       UpdatePeriodicRates();
       
    }
    
    else if ( GeometryIsDynamic_ == FULL_DYNAMIC ) {
      
       UpdateDynamicSystem();
       
    }
    
    else {
       
       printf("Unknown dynamic or rotor state \n");fflush(NULL);
       exit(1);
       
    }

}

/*##############################################################################
#                                                                              #
#                    COMPONENT_GROUP UpdateSteadyRates                         #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::UpdateSteadyRates(void)
{

    QUAT Omega, DQuatDt;

    // Angular rate
   
    Omega(0) = Omega_ * RVec_[0];
    Omega(1) = Omega_ * RVec_[1];
    Omega(2) = Omega_ * RVec_[2];

    // Quaternion for this rotation, and it's inverse
    
    Angle_ = Omega_ * TimeStep_;

    Quat_.FormRotationQuat(RVec_,Angle_);
    
    InvQuat_ = Quat_;
   
    InvQuat_.FormInverse();   
    
    // Update current total rotation angle

    if ( CurrentTime_ > 0. ) TotalRotationAngle_ += Angle_;
 
    // Quaternion rates
    
    DQuatDt = Omega * Quat_;
    
    WQuat_ = DQuatDt * InvQuat_; // There is a factor of 1/2 missing here, but it gets multiplied away later
    
    // Translation vector

    TVec_[0] = Velocity_[0] * TimeStep_;
    TVec_[1] = Velocity_[1] * TimeStep_;
    TVec_[2] = Velocity_[2] * TimeStep_;

}

/*##############################################################################
#                                                                              #
#                    COMPONENT_GROUP UpdatePeriodicRates                       #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::UpdatePeriodicRates(void)
{

    double AngularRate, DeltaAngle;
    QUAT Omega, DQuatDt;
    
    // Calculate angle

    Angle_ = AngleMax_*TORAD*sin(Omega_ * CurrentTime_);

    // Calculate change in angle
 
    DeltaAngle = AngleMax_*TORAD*sin(Omega_ * CurrentTime_ ) - AngleMax_*TORAD*sin(Omega_ * (CurrentTime_ - TimeStep_));

    // Calculate instantaneous angular rate
    
    AngularRate = AngleMax_*TORAD*cos(Omega_ * CurrentTime_)*Omega_;
   
   // AngularRate = DeltaAngle/TimeStep_;

    Omega(0) = AngularRate * RVec_[0];
    Omega(1) = AngularRate * RVec_[1];
    Omega(2) = AngularRate * RVec_[2];
            
    // Quaternion for this rotation, and it's inverse

    Quat_.FormRotationQuat(RVec_,DeltaAngle);
    
    InvQuat_ = Quat_;
   
    InvQuat_.FormInverse();   
 
    // Quaternion rates
    
    DQuatDt = Omega * Quat_;
    
    WQuat_ = DQuatDt * InvQuat_; // There is a factor of 1/2 missing here, but it gets multiplied away later
    
    // Translation vector

    TVec_[0] = Velocity_[0] * TimeStep_;
    TVec_[1] = Velocity_[1] * TimeStep_;
    TVec_[2] = Velocity_[2] * TimeStep_;

}

/*##############################################################################
#                                                                              #
#                 COMPONENT_GROUP UpdateDynamicSystem                          #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::UpdateDynamicSystem(void)
{

    double Dot;
    QUAT Omega, DQuatDt;
    MATRIX Force(3), Moment(3);

    Mass_ = 2000./32.2; //slugs
    
    Ixx_ =  20.*Mass_;
    Iyy_ =  80.*Mass_;
    Izz_ =  80.*Mass_;
    Ixy_ =   0.*Mass_;
    Ixz_ =   0.*Mass_;
    Iyz_ =   0.*Mass_;
    
    // Calculate forces and moments
    
    Density_ = 0.0021;
    Vref_ = 540.;
    Sref_ = 823.210124;
    Cref_ = 8.434377;
    Bref_ = 85.291011;

    Force(1) = Cx_[0] * 0.5 * Density_ * Sref_ * Vref_ * Vref_;
    Force(2) = Cy_[0] * 0.5 * Density_ * Sref_ * Vref_ * Vref_;
    Force(3) = Cz_[0] * 0.5 * Density_ * Sref_ * Vref_ * Vref_;
    
    Force(3) -= Mass_ * 32.2;
        
    Moment(1) = Cmx_[0] * 0.5 * Density_ * Bref_ * Sref_ * Vref_ * Vref_;
    Moment(2) = Cmy_[0] * 0.5 * Density_ * Cref_ * Sref_ * Vref_ * Vref_;
    Moment(3) = Cmz_[0] * 0.5 * Density_ * Bref_ * Sref_ * Vref_ * Vref_;

if ( CurrentTime_ < 64.*TimeStep_ ) {

Force(1) = 0.;
Force(2) = 0.;
Force(3) = 0.;

Moment(1) = 0.;
Moment(2) = 0.;
Moment(3) = 0.;

}

Moment.print("Moment");

    // Mass and Inertia matrices

    I_(1,1) =  Ixx_; I_(1,2) = -Ixy_; I_(1,3) =  Ixz_;
    I_(2,1) = -Ixy_; I_(2,2) =  Iyy_; I_(2,3) = -Iyz_;
    I_(3,1) = -Ixz_; I_(3,2) = -Ixy_; I_(3,3) =  Izz_;
I_.print("I_");      
    M_(1,1) = Mass_; M_(1,2) =    0.; M_(1,3) =    0.;
    M_(2,1) =    0.; M_(2,2) = Mass_; M_(2,3) =    0.;
    M_(3,1) =    0.; M_(3,2) =    0.; M_(3,3) = Mass_;
M_.print("M_");  
    //
    //
    //
    //
    //
        
    // Need to do transformation from body to inertial axes of I_
    
    //
    //
    //
    //
    //
    
    // Calculate angular acceleration

    OmegaDot_ = Moment / I_;

OmegaDot_.print("OmegaDot_");
    
    // Update angular rates
    
    RVec_[0] *= Omega_;
    RVec_[1] *= Omega_;
    RVec_[2] *= Omega_;
    
    RVec_[0] += OmegaDot_(1) * TimeStep_;
    RVec_[1] += OmegaDot_(2) * TimeStep_;
    RVec_[2] += OmegaDot_(3) * TimeStep_;

    Dot = sqrt(vector_dot(RVec_,RVec_));
    
    if ( Dot > 0. ) {
       
       RVec_[0] /= Dot;
       RVec_[1] /= Dot;
       RVec_[2] /= Dot;
    
    }
    
    else {
       
       RVec_[0] = 1.;
       RVec_[1] = 0.;
       RVec_[2] = 0.;
              
    }
    
    Omega_ = sqrt(OmegaDot_(1)*OmegaDot_(1) + OmegaDot_(2)*OmegaDot_(2) + OmegaDot_(3)*OmegaDot_(3))*TimeStep_;
    
    // Calculate linear accelerations
    
Force.print("Force");
  
    VDot_ = Force / M_;

VDot_.print("VDot_");
    
    // Update velocity
    
    Velocity_[0] += VDot_(1) * TimeStep_;
    Velocity_[1] += VDot_(2) * TimeStep_;
    Velocity_[2] += VDot_(3) * TimeStep_;

printf("Velocity: %f %f %f \n",Velocity_[0], Velocity_[1], Velocity_[2]);      
    // Update translation vector
    
    TVec_[0] = Velocity_[0] * TimeStep_;
    TVec_[1] = Velocity_[1] * TimeStep_;
    TVec_[2] = Velocity_[2] * TimeStep_;

printf("TVec_: %f %f %f \n",TVec_[0], TVec_[1], TVec_[2]);      

    // Current angle change over this time step

    Angle_ = Omega_;

printf("Angle_: %f \n",Angle_);

    // Quaternion for this rotation, and it's inverse
printf("RVec_: %f %f %f \n",RVec_[0],RVec_[1],RVec_[2]);
printf("OVec_: %f %f %f \n",OVec_[0],OVec_[1],OVec_[2]);

    Quat_.FormRotationQuat(RVec_,Angle_);
    
    InvQuat_ = Quat_;
   
    InvQuat_.FormInverse();   
 
    // Quaternion rates
    
    DQuatDt = Omega * Quat_;
    
    WQuat_ = DQuatDt * InvQuat_; // There is a factor of 1/2 missing here, but it gets multiplied away later

}

/*##############################################################################
#                                                                              #
#                      COMPONENT_GROUP WriteData                               #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::WriteData(FILE *File)
{

    int i;

    fprintf(File,"GroupName = %s \n",GroupName_);
    
    fprintf(File,"NumberOfComponents = %d \n",NumberOfComponents_);
 
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       fprintf(File,"%d \n",ComponentList_[i]);
       
    }
    
    fprintf(File,"GeometryIsFixed = %d \n",GeometryIsFixed_);
    
    fprintf(File,"GeometryIsDynamic = %d \n",GeometryIsDynamic_);
    
    fprintf(File,"GeometryIsARotor = %d \n",GeometryIsARotor_);
    
    fprintf(File,"RotorDiameter = %f \n",RotorDiameter_);
    
    fprintf(File,"OVec = %lf %lf %lf \n",OVec_[0], OVec_[1], OVec_[2]);
    
    fprintf(File,"RVec = %lf %lf %lf \n",RVec_[0], RVec_[1], RVec_[2]);
    
    fprintf(File,"Velocity = %lf %lf %lf \n",Velocity_[0], Velocity_[1], Velocity_[2]);

    fprintf(File,"Acceleration = %lf %lf %lf \n",Acceleration_[0], Acceleration_[1], Acceleration_[2]);

    fprintf(File,"Omega = %lf \n",Omega_);
    
    fprintf(File,"Mass = %lf \n",Mass_);
    
    fprintf(File,"Ixx = %lf \n",Ixx_);
                  
    fprintf(File,"Iyy = %lf \n",Iyy_);
                  
    fprintf(File,"Izz = %lf \n",Izz_);
                  
    fprintf(File,"Ixy = %lf \n",Ixy_);
                  
    fprintf(File,"Ixz = %lf \n",Ixz_);
                  
    fprintf(File,"Iyz = %lf \n",Iyz_);

}

/*##############################################################################
#                                                                              #
#                      COMPONENT_GROUP LoadData                                #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::LoadData(FILE *File)
{

    int i;
    
    fscanf(File,"GroupName = %s \n",GroupName_);
    
    printf("GroupName_: %s \n",GroupName_);

    fscanf(File,"NumberOfComponents = %d \n",&NumberOfComponents_);
    
    printf("NumberOfComponents: %d \n",NumberOfComponents_);

    SizeList(NumberOfComponents_);
    
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       fscanf(File,"%d \n",&(ComponentList_[i]));

    }
    
    fscanf(File,"GeometryIsFixed = %d \n",&GeometryIsFixed_);
    
    printf("GeometryIsFixed: %d \n",GeometryIsFixed_);
    
    fscanf(File,"GeometryIsDynamic = %d \n",&GeometryIsDynamic_);
    
    printf("GeometryIsDynamic_: %d \n",GeometryIsDynamic_);
    
    fscanf(File,"GeometryIsARotor = %d \n",&GeometryIsARotor_);
    
    printf("GeometryIsARotor_: %d \n",GeometryIsARotor_);    
    
    fscanf(File,"RotorDiameter = %lf \n",&RotorDiameter_);

    printf("RotorDiameter_: %f \n",RotorDiameter_);

    fscanf(File,"OVec = %lf %lf %lf \n",&(OVec_[0]),&(OVec_[1]),&(OVec_[2]));

    printf("OVec: %f %f %f\n",(OVec_[0]),(OVec_[1]),(OVec_[2]));
    
    fscanf(File,"RVec = %lf %lf %lf \n",&(RVec_[0]),&(RVec_[1]),&(RVec_[2]));
    
    printf("RVec_: %f %f %f\n",(RVec_[0]),(RVec_[1]),(RVec_[2]));
    
    fscanf(File,"Velocity = %lf %lf %lf \n",&(Velocity_[0]),&(Velocity_[1]),&(Velocity_[2]));

    printf("Velocity: %f %f %f\n",(Velocity_[0]),(Velocity_[1]),(Velocity_[2]));

    fscanf(File,"Acceleration = %lf %lf %lf \n",&(Acceleration_[0]), &(Acceleration_[1]), &(Acceleration_[2]));

    fscanf(File,"Omega = %lf \n",&Omega_);
    
    printf("Omega: %f \n",Omega_);
    
    fscanf(File,"Mass = %lf \n",&Mass_);
    
    fscanf(File,"Ixx = %lf \n",&Ixx_);
                  
    fscanf(File,"Iyy = %lf \n",&Iyy_);
                  
    fscanf(File,"Izz = %lf \n",&Izz_);
                  
    fscanf(File,"Ixy = %lf \n",&Ixy_);
                  
    fscanf(File,"Ixz = %lf \n",&Ixz_);
                  
    fscanf(File,"Iyz = %lf \n",&Iyz_);
    
    if ( GeometryIsFixed_ ) {
       
       Omega_ = 0.;
       
       OVec_[0] = OVec_[1] = OVec_[2] = 0.;
       
       RVec_[0] = RVec_[1] = RVec_[2] = 1.;
       
       Velocity_[0] = Velocity_[1] = Velocity_[2] = 0.;
       
    }
    
    if ( GeometryIsDynamic_ ) {
       
              M_.size(3,3);
              I_.size(3,3);
       OmegaDot_.size(3);
           XDot_.size(3);
           VDot_.size(3);
       
       M_(1,1) = M_(2,2) = M_(3,3) = Mass_;
       
       I_(1,1) = Ixx_;  I_(1,2) = Ixy_;  I_(1,3) = Ixz_;
       I_(1,1) = Ixy_;  I_(1,2) = Iyy_;  I_(1,3) = Iyz_;
       I_(1,1) = Ixz_;  I_(1,2) = Iyz_;  I_(1,3) = Izz_;
       
    }

}

/*##############################################################################
#                                                                              #
#                COMPONENT_GROUP ZeroAverageForcesAndMoments                   #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::ZeroAverageForcesAndMoments(void)
{

    NumberOfTimeSamples_ = 0;
    
    Cx_[0]   = Cx_[1]   = 0.;
    Cy_[0]   = Cy_[1]   = 0.;
    Cz_[0]   = Cz_[1]   = 0.;
                        
    Cmx_[0]  = Cmx_[1]  = 0.;
    Cmy_[0]  = Cmy_[1]  = 0.;
    Cmz_[0]  = Cmz_[1]  = 0.;
                        
    CL_[0]   = CL_[1]   = 0.;
    CD_[0]   = CD_[1]   = 0.;
    CS_[0]   = CS_[1]   = 0.;
                        
    Cxo_[0]  = Cxo_[1]  = 0.;
    Cyo_[0]  = Cyo_[1]  = 0.;
    Czo_[0]  = Czo_[1]  = 0.;
                        
    Cmxo_[0] = Cmxo_[1] = 0.;
    Cmyo_[0] = Cmyo_[1] = 0.;
    Cmzo_[0] = Cmzo_[1] = 0.;
                        
    CLo_[0]  = CLo_[1]  = 0.;
    CDo_[0]  = CDo_[1]  = 0.;
    CSo_[0]  = CSo_[1]  = 0.; 

}

/*##############################################################################
#                                                                              #
#            COMPONENT_GROUP UpdateAverageForcesAndMoments                     #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::UpdateAverageForcesAndMoments(void)
{

    NumberOfTimeSamples_++;
    
    Cx_[1]   += Cx_[0];
    Cy_[1]   += Cy_[0];
    Cz_[1]   += Cz_[0];
          
    Cmx_[1]  += Cmx_[0];
    Cmy_[1]  += Cmy_[0];
    Cmz_[1]  += Cmz_[0];
          
    CL_[1]   += CL_[0];
    CD_[1]   += CD_[0];
    CS_[1]   += CS_[0];
           
    Cxo_[1]  += Cxo_[0];
    Cyo_[1]  += Cyo_[0];
    Czo_[1]  += Czo_[0];
          
    Cmxo_[1] += Cmxo_[0];
    Cmyo_[1] += Cmyo_[0];
    Cmzo_[1] += Cmzo_[0];
           
    CLo_[1]  += CLo_[0];
    CDo_[1]  += CDo_[0];
    CSo_[1]  += CSo_[0]; 

}

/*##############################################################################
#                                                                              #
#          COMPONENT_GROUP CalculateAverageForcesAndMoments                    #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::CalculateAverageForcesAndMoments(void)
{

    Cx_[1]   /= NumberOfTimeSamples_;
    Cy_[1]   /= NumberOfTimeSamples_;
    Cz_[1]   /= NumberOfTimeSamples_;
          
    Cmx_[1]  /= NumberOfTimeSamples_;
    Cmy_[1]  /= NumberOfTimeSamples_;
    Cmz_[1]  /= NumberOfTimeSamples_;
          
    CL_[1]   /= NumberOfTimeSamples_;
    CD_[1]   /= NumberOfTimeSamples_;
    CS_[1]   /= NumberOfTimeSamples_;
           
    Cxo_[1]  /= NumberOfTimeSamples_;
    Cyo_[1]  /= NumberOfTimeSamples_;
    Czo_[1]  /= NumberOfTimeSamples_;
          
    Cmxo_[1] /= NumberOfTimeSamples_;
    Cmyo_[1] /= NumberOfTimeSamples_;
    Cmzo_[1] /= NumberOfTimeSamples_;
           
    CLo_[1]  /= NumberOfTimeSamples_;
    CDo_[1]  /= NumberOfTimeSamples_;
    CSo_[1]  /= NumberOfTimeSamples_; 
    
}









