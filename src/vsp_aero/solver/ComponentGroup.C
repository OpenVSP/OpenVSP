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

    
    NumberOfComponents_ = 0;
   
    ComponentList_ = NULL;
    
    GeometryIsFixed_   = 0;
    GeometryIsDynamic_ = 0;
    GeometryIsARotor_  = 0;
    GeometryHasWings_  = 0;
    GeometryHasBodies_ = 0;
        
    OVec_[0] = OVec_[0] = OVec_[0] = 0.;
    RVec_[0] = RVec_[0] = RVec_[0] = 0.;
    TVec_[0] = TVec_[0] = TVec_[0] = 0.;
    
    UserInputVelocity_[0] = UserInputVelocity_[1] = UserInputVelocity_[2] = 0.;

    UserInputAcceleration_[0] = UserInputAcceleration_[1] = UserInputAcceleration_[2] = 0.;
    
    Mass_ = 0.;

    Ixx_ = 0.;
    Iyy_ = 0.;
    Izz_ = 0.;
    Ixy_ = 0.;
    Ixz_ = 0.;
    Iyz_ = 0.;

    Omega_                    = 0.;
    Angle_                    = 0.;
    TimeStep_                 = 0.;
    CurrentTime_              = 0.;
    RotorDiameter_            = 0.;
    AngleMax_                 = 0.;
    TotalRotationAngle_       = 0.;
    StartDynamicAnalysisTime_ = 0.;

    // Integrated forces
    
    NumberOfTimeSamples_ = 0;
    
    StartAveragingTimeStep_ = 0;
    
    StartAveragingTime_ = 0.;

    Cxo_[0] = Cxo_[1] = 0.;
    Cyo_[0] = Cyo_[1] = 0.;
    Czo_[0] = Czo_[1] = 0.;
    
    Cx_[0] = Cx_[1] = 0.;
    Cy_[0] = Cy_[1] = 0.;
    Cz_[0] = Cz_[1] = 0.;

    Cmxo_[0] = Cmxo_[1] = 0.;
    Cmyo_[0] = Cmyo_[1] = 0.;
    Cmzo_[0] = Cmzo_[1] = 0.;

    Cmx_[0] = Cmx_[1] = 0.;
    Cmy_[0] = Cmy_[1] = 0.;
    Cmz_[0] = Cmz_[1] = 0.;

    CL_[0] = CL_[1] = 0.;
    CD_[0] = CD_[1] = 0.;
    CS_[0] = CS_[1] = 0.;

    CLo_[0] = CLo_[1] = 0.;
    CDo_[0] = CDo_[1] = 0.;
    CSo_[0] = CSo_[1] = 0.;
    
    // Free stream conditions
    
    Density_ = 0.;
    Vref_    = 0.;
    Sref_    = 0.;
    Bref_    = 0.;  
    Cref_    = 0.;  
    
    // Spanwise forces and moments, for wings and rotors
    
    NumberOfLiftingSurfaces_ = 0;
    
    SpanLoadData_ = NULL;

    // Size and intialize matrices, vectors
    
                MassMatrix_.size(3,3);
             InertiaMatrix_.size(3,3);
      InertiaMatrixInverse_.size(3,3);       
            RotationMatrix_.size(3,3);
     RotationMatrixInverse_.size(3,3);      
                  OmegaDot_.size(3);
            LinearMomentum_.size(3);            
           AngularMomentum_.size(3);           
                  Velocity_.size(3);            
              Acceleration_.size(3);    
                  
}

/*##############################################################################
#                                                                              #
#                        COMPONENT_GROUP Destructor                            #
#                                                                              #
##############################################################################*/

COMPONENT_GROUP::~COMPONENT_GROUP(void)
{

    if ( ComponentList_ != NULL ) delete [] ComponentList_;
    
    if ( SpanLoadData_ != NULL ) delete [] SpanLoadData_;
    
    NumberOfComponents_ = 0;
    
    NumberOfLiftingSurfaces_ = 0;    
    
    GeometryIsFixed_ = 0;
    
    GeometryIsDynamic_ = 0;
    
    GeometryIsARotor_ = 0;
    
    GeometryHasWings_ = 0;
    
    GeometryHasBodies_ = 0;

    OVec_[0] = OVec_[1] = OVec_[2] = 0.;
    RVec_[0] = RVec_[1] = RVec_[2] = 0.;
    TVec_[0] = TVec_[1] = TVec_[2] = 0.;

    UserInputVelocity_[0] = UserInputVelocity_[1] = UserInputVelocity_[2] = 0.;
    
    UserInputAcceleration_[0] = UserInputAcceleration_[1] = UserInputAcceleration_[2] = 0.;
    
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
    
    StartAveragingTime_ = 0.;
    
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
    
    NumberOfLiftingSurfaces_ = ComponentGroup.NumberOfLiftingSurfaces_;
    
    SizeSpanLoadingList(NumberOfLiftingSurfaces_);
    
    for ( i = 1 ; i <= NumberOfLiftingSurfaces_ ; i++ ) {
       
       SpanLoadData_[i] = ComponentGroup.SpanLoadData_[i];
       
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
 
    UserInputVelocity_[0] = ComponentGroup.UserInputVelocity_[0];
    UserInputVelocity_[1] = ComponentGroup.UserInputVelocity_[1];
    UserInputVelocity_[2] = ComponentGroup.UserInputVelocity_[2];
    
    UserInputAcceleration_[0] = ComponentGroup.UserInputAcceleration_[0];
    UserInputAcceleration_[1] = ComponentGroup.UserInputAcceleration_[1];
    UserInputAcceleration_[2] = ComponentGroup.UserInputAcceleration_[2];
    
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
    
    StartAveragingTime_ = ComponentGroup.StartAveragingTime_;

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
        
                MassMatrix_ = ComponentGroup.MassMatrix_;
             InertiaMatrix_ = ComponentGroup.InertiaMatrix_;
      InertiaMatrixInverse_ = ComponentGroup.InertiaMatrixInverse_;    
            RotationMatrix_ = ComponentGroup.RotationMatrix_;
     RotationMatrixInverse_ = ComponentGroup.RotationMatrixInverse_;   
                  OmegaDot_ = ComponentGroup.OmegaDot_;
            LinearMomentum_ = ComponentGroup.LinearMomentum_;     
           AngularMomentum_ = ComponentGroup.AngularMomentum_;       
                  Velocity_ = ComponentGroup.Velocity_;        
              Acceleration_ = ComponentGroup.Acceleration_;       
      
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

void COMPONENT_GROUP::SizeSpanLoadingList(int NumberOfLiftingSurfaces)
{

    NumberOfLiftingSurfaces_ = NumberOfLiftingSurfaces;
    
    SpanLoadData_ = new SPAN_LOAD_ROTOR_DATA[NumberOfLiftingSurfaces + 1];

}           
            
/*##############################################################################
#                                                                              #
#                         COMPONENT_GROUP Update                               #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::Update(VSPAERO_DOUBLE TimeStep, VSPAERO_DOUBLE CurrentTime)
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
       
       PRINTF ("Unknown dynamic or rotor state \n");fflush(NULL);
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

    QUAT Omega;
    
    // Angular rate
   
    Omega(0) = Omega_ * RVec_[0];
    Omega(1) = Omega_ * RVec_[1];
    Omega(2) = Omega_ * RVec_[2];
    Omega(3) = 0.;

    // Quaternion for this rotation, and it's inverse
    
    Angle_ = Omega_ * TimeStep_;

    Quat_.FormRotationQuat(RVec_,Angle_);
   
    InvQuat_ = Quat_;
   
    InvQuat_.FormInverse();   
    
    // Update current total rotation angle

    if ( CurrentTime_ > 0. ) TotalRotationAngle_ += Angle_;
 
    // Quaternion rates

    WQuat_ = Omega * Quat_ * InvQuat_;
        
    // Translation vector

    TVec_[0] = UserInputVelocity_[0] * TimeStep_;
    TVec_[1] = UserInputVelocity_[1] * TimeStep_;
    TVec_[2] = UserInputVelocity_[2] * TimeStep_;

}

/*##############################################################################
#                                                                              #
#                    COMPONENT_GROUP UpdatePeriodicRates                       #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::UpdatePeriodicRates(void)
{

    VSPAERO_DOUBLE AngularRate, DeltaAngle;
    QUAT Omega;
    
    // Calculate angle

    Angle_ = AngleMax_*TORAD*sin(Omega_ * CurrentTime_);

    // Calculate change in angle
 
    DeltaAngle = AngleMax_*TORAD*sin(Omega_ * CurrentTime_ ) - AngleMax_*TORAD*sin(Omega_ * (CurrentTime_ - TimeStep_));

    // Calculate instantaneous angular rate
    
    AngularRate = AngleMax_*TORAD*cos(Omega_ * CurrentTime_)*Omega_;

    Omega(0) = AngularRate * RVec_[0];
    Omega(1) = AngularRate * RVec_[1];
    Omega(2) = AngularRate * RVec_[2];
    Omega(3) = 0.;
            
    // Quaternion for this rotation, and it's inverse

    Quat_.FormRotationQuat(RVec_,DeltaAngle);
    
    InvQuat_ = Quat_;
   
    InvQuat_.FormInverse();   
 
    // Quaternion rates

    WQuat_ = Omega * Quat_ * InvQuat_; 

    // Translation vector

    TVec_[0] = UserInputVelocity_[0] * TimeStep_;
    TVec_[1] = UserInputVelocity_[1] * TimeStep_;
    TVec_[2] = UserInputVelocity_[2] * TimeStep_;

}

/*##############################################################################
#                                                                              #
#                 COMPONENT_GROUP UpdateDynamicSystem                          #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::UpdateDynamicSystem(void)
{

    VSPAERO_DOUBLE Dot;
    QUAT Omega, DQuat, InvDQuat_, Half, dTime, Mass, Inertia;
    MATRIX Force(3), Moment(3), OmegaVec(3);

    dTime.Init(TimeStep_);
    
    Half.Init(0.5);

    // User specified accelerations
    
    Force(1) = Mass_ * UserInputAcceleration_[0];
    Force(2) = Mass_ * UserInputAcceleration_[1];
    Force(3) = Mass_ * UserInputAcceleration_[2];

    // Aerodynamic Forces
    
    Force(1) += Cx_[0] * 0.5 * Density_ * Sref_ * Vref_ * Vref_;
    Force(2) += Cy_[0] * 0.5 * Density_ * Sref_ * Vref_ * Vref_;
    Force(3) += Cz_[0] * 0.5 * Density_ * Sref_ * Vref_ * Vref_;

    if ( CurrentTime_ < StartDynamicAnalysisTime_ ) {
       
       Force(1) = 0.;
       Force(2) = 0.;
       Force(3) = 0.;

    }
       
    // Linear momentum
    
    LinearMomentum_(1) += TimeStep_ * Force(1);
    LinearMomentum_(2) += TimeStep_ * Force(2);
    LinearMomentum_(3) += TimeStep_ * Force(3);

    // Velocity
    
    Velocity_(1) = LinearMomentum_(1) / Mass_;
    Velocity_(2) = LinearMomentum_(2) / Mass_;
    Velocity_(3) = LinearMomentum_(3) / Mass_;

    // Translations
    
    TVec_[0] = Velocity_(1) * TimeStep_;
    TVec_[1] = Velocity_(2) * TimeStep_;
    TVec_[2] = Velocity_(3) * TimeStep_;

    // Moments

    Moment(1) = Cmx_[0] * 0.5 * Density_ * Bref_ * Sref_ * Vref_ * Vref_;
    Moment(2) = Cmy_[0] * 0.5 * Density_ * Cref_ * Sref_ * Vref_ * Vref_;
    Moment(3) = Cmz_[0] * 0.5 * Density_ * Bref_ * Sref_ * Vref_ * Vref_;

    if ( CurrentTime_ < StartDynamicAnalysisTime_ ) {
       
       Moment(1) = 0.;
       Moment(2) = 0.;
       Moment(3) = 0.;

    }
    
    // Angular momentum
    
    AngularMomentum_(1) += TimeStep_ * Moment(1);
    AngularMomentum_(2) += TimeStep_ * Moment(2);
    AngularMomentum_(3) += TimeStep_ * Moment(3);
    
    OmegaVec = InertiaMatrixInverse_ * AngularMomentum_;

    Omega(0) = OmegaVec(1);
    Omega(1) = OmegaVec(2);
    Omega(2) = OmegaVec(3);
    Omega(3) = 0.;
    
    // Update quaternion tracking overall rotation of geometry

    DQuat = dTime * Half * Omega * TotalQuat_;

    TotalQuat_ = TotalQuat_ + DQuat;

    // Calculate the quat for just the change over the current timestep

    Dot = sqrt( DQuat(0)*DQuat(0) + DQuat(1)*DQuat(1) + DQuat(2)*DQuat(2) + DQuat(3)*DQuat(3) );
        
    if ( Dot > 0. ) {
       
       RVec_[0] = DQuat(0)/Dot;
       RVec_[1] = DQuat(1)/Dot;
       RVec_[2] = DQuat(2)/Dot;

    }
    
    else {
       
       RVec_[0] = 1.;
       RVec_[1] = 1.;
       RVec_[2] = 1.;
       
       Dot = 0.;
       
    }
    
    Quat_.FormRotationQuat(RVec_, Dot);

    InvQuat_ = Quat_;

    InvQuat_.FormInverse();   

    // Quaternion rates, again for the current time step
    
    WQuat_ = Omega * DQuat * InvDQuat_; 
    
    // Update inertia matrix
    
    QuatToMatrix(TotalQuat_, RotationMatrix_, RotationMatrixInverse_);
    
    InertiaMatrixInverse_ = RotationMatrix_ * InertiaMatrixInverse_ * RotationMatrixInverse_;

}

/*##############################################################################
#                                                                              #
#                      COMPONENT_GROUP QuatToMatrix                            #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::QuatToMatrix(QUAT &Quat, MATRIX &Matrix, MATRIX &Inverse)
{
   
   VSPAERO_DOUBLE Qx, Qy, Qz, s;
   
   Qx = Quat(0);
   Qy = Quat(1);
   Qz = Quat(2);
    s = Quat(3);
    
   Matrix(1,1) = 1. - 2.*Qy*Qy - 2.*Qz*Qz;      Matrix(1,2) =      2.*Qx*Qy - 2.* s*Qz;      Matrix(1,3) =      2.*Qx*Qz + 2.* s*Qy;
   
   Matrix(2,1) =      2.*Qx*Qy + 2.* s*Qz;      Matrix(2,2) = 1. - 2.*Qx*Qx - 2.*Qz*Qz;      Matrix(2,3) =      2.*Qy*Qz - 2.* s*Qx;
                      
   Matrix(3,1) =      2.*Qx*Qz - 2.* s*Qy;      Matrix(3,2) =      2.*Qy*Qz + 2.* s*Qx;      Matrix(3,3) = 1. - 2.*Qx*Qx - 2.*Qy*Qy;
   
   Inverse = Matrix.transpose();
      
}


/*##############################################################################
#                                                                              #
#                      COMPONENT_GROUP WriteData                               #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::WriteData(FILE *File)
{

    int i;

    FPRINTF  (File,"GroupName = %s \n",GroupName_);
    
    FPRINTF  (File,"NumberOfComponents = %d \n",NumberOfComponents_);
 
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       FPRINTF  (File,"%d \n",ComponentList_[i]);
       
    }
    
    FPRINTF  (File,"GeometryIsFixed = %d \n",GeometryIsFixed_);
    
    FPRINTF  (File,"GeometryIsDynamic = %d \n",GeometryIsDynamic_);
    
    FPRINTF  (File,"GeometryIsARotor = %d \n",GeometryIsARotor_);
    
    FPRINTF  (File,"RotorDiameter = %f \n",RotorDiameter_);
    
    FPRINTF  (File,"OVec = %lf %lf %lf \n",OVec_[0], OVec_[1], OVec_[2]);
    
    FPRINTF  (File,"RVec = %lf %lf %lf \n",RVec_[0], RVec_[1], RVec_[2]);
    
    FPRINTF  (File,"Velocity = %lf %lf %lf \n",UserInputVelocity_[0], UserInputVelocity_[1], UserInputVelocity_[2]);

    FPRINTF  (File,"Acceleration = %lf %lf %lf \n",UserInputAcceleration_[0], UserInputAcceleration_[1], UserInputAcceleration_[2]);

    FPRINTF  (File,"Omega = %lf \n",Omega_);
    
    FPRINTF  (File,"Mass = %lf \n",Mass_);
    
    FPRINTF  (File,"Ixx = %lf \n",Ixx_);
                  
    FPRINTF  (File,"Iyy = %lf \n",Iyy_);
                  
    FPRINTF  (File,"Izz = %lf \n",Izz_);
                  
    FPRINTF  (File,"Ixy = %lf \n",Ixy_);
                  
    FPRINTF  (File,"Ixz = %lf \n",Ixz_);
                  
    FPRINTF  (File,"Iyz = %lf \n",Iyz_);

    FPRINTF  (File,"StartAnalysisTime = %lf \n",StartDynamicAnalysisTime_);


}

/*##############################################################################
#                                                                              #
#                      COMPONENT_GROUP LoadData                                #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::LoadData(FILE *File)
{

    int i;
    fpos_t LineLocation;
    char DumChar[2000];   
              
    // Parse the group data file              
              
    fscanf(File,"GroupName = %s \n",GroupName_);
    
    PRINTF ("GroupName_: %s \n",GroupName_);

    fscanf(File,"NumberOfComponents = %d \n",&NumberOfComponents_);
    
    PRINTF ("NumberOfComponents: %d \n",NumberOfComponents_);

    SizeList(NumberOfComponents_);
    
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       fscanf(File,"%d \n",&(ComponentList_[i]));

    }
    
    fscanf(File,"GeometryIsFixed = %d \n",&GeometryIsFixed_);
    
    PRINTF ("GeometryIsFixed: %d \n",GeometryIsFixed_);
    
    fscanf(File,"GeometryIsDynamic = %d \n",&GeometryIsDynamic_);
    
    PRINTF ("GeometryIsDynamic_: %d \n",GeometryIsDynamic_);
    
    fscanf(File,"GeometryIsARotor = %d \n",&GeometryIsARotor_);
    
    PRINTF ("GeometryIsARotor_: %d \n",GeometryIsARotor_);    
    
    fscanf(File,"RotorDiameter = %lf \n",&RotorDiameter_);

    PRINTF ("RotorDiameter_: %f \n",RotorDiameter_);

    fscanf(File,"OVec = %lf %lf %lf \n",&(OVec_[0]),&(OVec_[1]),&(OVec_[2]));

    PRINTF ("OVec: %f %f %f\n",(OVec_[0]),(OVec_[1]),(OVec_[2]));
    
    fscanf(File,"RVec = %lf %lf %lf \n",&(RVec_[0]),&(RVec_[1]),&(RVec_[2]));
    
    PRINTF ("RVec_: %f %f %f\n",(RVec_[0]),(RVec_[1]),(RVec_[2]));
    
    fscanf(File,"Velocity = %lf %lf %lf \n",&(UserInputVelocity_[0]),&(UserInputVelocity_[1]),&(UserInputVelocity_[2]));

    PRINTF ("Velocity: %f %f %f\n",(UserInputVelocity_[0]),(UserInputVelocity_[1]),(UserInputVelocity_[2]));

    fscanf(File,"Acceleration = %lf %lf %lf \n",&(UserInputAcceleration_[0]), &(UserInputAcceleration_[1]), &(UserInputAcceleration_[2]));

    fscanf(File,"Omega = %lf \n",&Omega_);
    
    PRINTF ("Omega: %f \n",Omega_);
    
    fscanf(File,"Mass = %lf \n",&Mass_);
    
    fscanf(File,"Ixx = %lf \n",&Ixx_);
                  
    fscanf(File,"Iyy = %lf \n",&Iyy_);
                  
    fscanf(File,"Izz = %lf \n",&Izz_);
                  
    fscanf(File,"Ixy = %lf \n",&Ixy_);
                  
    fscanf(File,"Ixz = %lf \n",&Ixz_);
                  
    fscanf(File,"Iyz = %lf \n",&Iyz_);
    
    // Version 6.x stuff...
    
    fgetpos(File,&LineLocation);
    
    if ( fgets(DumChar,2000,File) != NULL ) {
       
       if ( strncmp(DumChar,"StartAnalysisTime",17) == 0 ) {
          
          sscanf(DumChar,"StartAnalysisTime = %lf \n",&StartDynamicAnalysisTime_);
          
          PRINTF ("StartAnalysisTime: %f \n",StartDynamicAnalysisTime_);
          
       }
       
       else {
          
          fsetpos(File,&LineLocation);
          
       }
       
    }
    
    if ( GeometryIsFixed_ ) {
       
       Omega_ = 0.;
       
       OVec_[0] = OVec_[1] = OVec_[2] = 0.;
       
       RVec_[0] = RVec_[1] = RVec_[2] = 1.;
       
       Velocity_(1) = Velocity_(2) = Velocity_(3) = 0.;
       
    }

    // Initialize matrices, vectors

    MassMatrix_(1,1) = MassMatrix_(2,2) = MassMatrix_(3,3) = Mass_;
     
    InertiaMatrix_(1,1) =  Ixx_;  InertiaMatrix_(1,2) = -Ixy_;  InertiaMatrix_(1,3) = -Ixz_;
    InertiaMatrix_(2,1) = -Ixy_;  InertiaMatrix_(2,2) =  Iyy_;  InertiaMatrix_(2,3) = -Iyz_;
    InertiaMatrix_(3,1) = -Ixz_;  InertiaMatrix_(3,2) = -Iyz_;  InertiaMatrix_(3,3) =  Izz_;

    InertiaMatrixInverse_ = InertiaMatrix_.inverse();

           OmegaDot_(1) =        OmegaDot_(2) =        OmegaDot_(3) = 0.;
    AngularMomentum_(1) = AngularMomentum_(2) = AngularMomentum_(3) = 0.;
           Velocity_(1) =        Velocity_(2) =        Velocity_(3) = 0.;
       Acceleration_(1) =    Acceleration_(2) =    Acceleration_(3) = 0.;
          
    LinearMomentum_(1) = Mass_ * UserInputVelocity_[0];
    LinearMomentum_(2) = Mass_ * UserInputVelocity_[1];
    LinearMomentum_(3) = Mass_ * UserInputVelocity_[2];
    
    // Initialize total quat

    TotalQuat_(0) = 0.;
    TotalQuat_(1) = 0.;
    TotalQuat_(2) = 0.;
    TotalQuat_(3) = 1.;
   
}

/*##############################################################################
#                                                                              #
#                COMPONENT_GROUP ZeroAverageForcesAndMoments                   #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::ZeroAverageForcesAndMoments(void)
{

    int i;
    
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

    for ( i = 1 ; i <= NumberOfLiftingSurfaces_ ; i++ ) {
       
       SpanLoadData_[i].ZeroForcesAndMoments();
       
    }
    
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







