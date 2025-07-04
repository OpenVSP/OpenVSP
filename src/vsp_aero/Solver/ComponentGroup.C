//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "ComponentGroup.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                         ComponentGroup Constructor                           #
#                                                                              #
##############################################################################*/

COMPONENT_GROUP::COMPONENT_GROUP(void)
{

    NumberOfComponents_ = 0;
   
    ComponentList_ = NULL;
    
    NumberOfLiftingSurfaces_ = 0;
    
    GeometryIsFixed_   = 0;
    GeometryIsDynamic_ = 0;
    GeometryIsARotor_  = 0;
    GeometryHasWings_  = 0;
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

    Omega_                    = 0.;
    RPM_                      = 0.;
    Angle_                    = 0.;
    DeltaTime_                 = 0.;
    CurrentTime_              = 0.;
    RotorDiameter_            = 0.;
    AngleMax_                 = 0.;
    TotalRotationAngle_       = 0.;
    StartDynamicAnalysisTime_ = 0.;
    DeltaFlatPlateDragArea_   = 0.;
    FlatPlateDragRefReNumber_ = 0.;

    // Integrated forces
    
    NumberOfTimeSamples_ = 0;
    
    // Integrated forces
    
    NumberOfTimeSamples_ = 0;
    
    // Invisicid forces and moments
    
    CLi_   = NULL;
    CDi_   = NULL;
    CSi_   = NULL;
           
    CFix_  = NULL;
    CFiy_  = NULL;
    CFiz_  = NULL;
           
    CMix_  = NULL;
    CMiy_  = NULL;
    CMiz_  = NULL;
           
    CTi_   = NULL;
    CQi_   = NULL;
    CPi_   = NULL;
    
    CTi_h_ = NULL;
    CQi_h_ = NULL;
    CPi_h_ = NULL;
             
    // Viscous forces and moments
   
    CLo_   = NULL;
    CSo_   = NULL;
    CDo_   = NULL;
           
    CFox_  = NULL;
    CFoy_  = NULL;
    CFoz_  = NULL;
           
    CMox_  = NULL;
    CMoy_  = NULL;
    CMoz_  = NULL;
           
    CTo_   = NULL;
    CQo_   = NULL;
    CPo_   = NULL;
    
    CTo_h_ = NULL;
    CQo_h_ = NULL;
    CPo_h_ = NULL;

    // Inviscid wake forces
    
    CLiw_ = NULL;
    CDiw_ = NULL;
    CSiw_ = NULL;
    
    CFiwx_ = NULL;
    CFiwy_ = NULL;
    CFiwz_ = NULL;    
    
    // Gradients wrt omega
        
    DCFxi_DOmega_ = NULL;
    DCFyi_DOmega_ = NULL;
    DCFzi_DOmega_ = NULL;
     
    DCMxi_DOmega_ = NULL;
    DCMyi_DOmega_ = NULL;
    DCMzi_DOmega_ = NULL;

    DCFxo_DOmega_ = NULL;
    DCFyo_DOmega_ = NULL;
    DCFzo_DOmega_ = NULL;
       
    DCMxo_DOmega_ = NULL;
    DCMyo_DOmega_ = NULL;
    DCMzo_DOmega_ = NULL;
    
    // Efficiencies
    
    FOM_  = NULL;
    EtaP_ = NULL;
        
    // Quats

    Quat_(0) = Quat_(1) = Quat_(2) = 0.;
    
    InvQuat_(0) = InvQuat_(1) = InvQuat_(2) = 0.;
    
    WQuat_(0) = WQuat_(1) = WQuat_(2) = 0.;
    
    pWQuat_pOmega_(0) = pWQuat_pOmega_(1) = pWQuat_pOmega_(2) = 0.;
    
    TotalQuat_(0) = TotalQuat_(1) = TotalQuat_(2) = TotalQuat_(3) = 0.;
        
    // Free stream conditions
    
    Density_ = 0.;
    Vref_    = 0.;
    Sref_    = 0.;
    Bref_    = 0.;  
    Cref_    = 0.;  


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

    Velocity_(1) = Velocity_(2) = Velocity_(3) = 0.;
                  
}

/*##############################################################################
#                                                                              #
#                        COMPONENT_GROUP Destructor                            #
#                                                                              #
##############################################################################*/

COMPONENT_GROUP::~COMPONENT_GROUP(void)
{

    if ( ComponentList_ != NULL ) delete [] ComponentList_;
        
    ComponentList_ = NULL;

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

    Velocity_(1) = Velocity_(2) = Velocity_(3) = 0.;
    
    UserInputAcceleration_[0] = UserInputAcceleration_[1] = UserInputAcceleration_[2] = 0.;
    
    Mass_ = 0.;
    
    Ixx_ = 0.;
    
    Iyy_ = 0.;
    
    Izz_ = 0.;
    
    Ixy_ = 0.;
    
    Ixz_ = 0.;
    
    Iyz_ = 0.;
    
    Omega_ = 0.;
    
    RPM_   = 0.;
    
    Angle_  = 0.;
    
    DeltaTime_ = 0.;
    
    RotorDiameter_ = 0.;
    
    Quat_(0) = Quat_(1) = Quat_(2) = 0.;
    
    InvQuat_(0) = InvQuat_(1) = InvQuat_(2) = 0.;
    
    WQuat_(0) = WQuat_(1) = WQuat_(2) = 0.;
    
    pWQuat_pOmega_(0) = pWQuat_pOmega_(1) = pWQuat_pOmega_(2) = 0.;
    
    TotalRotationAngle_ = 0.;
    
    // Integrated forces
    
    NumberOfTimeSamples_ = 0;
    
    // Delta flat plate drag area
    
    DeltaFlatPlateDragArea_ = 0.;
    
    // Flat plate drag reference Reynold's number
    
    FlatPlateDragRefReNumber_ = 0.;
    
    // Invisicid forces and moments
    
    if ( CLi_   != NULL ) delete [] CLi_;
    if ( CDi_   != NULL ) delete [] CDi_;
    if ( CSi_   != NULL ) delete [] CSi_;
  
    if ( CFix_  != NULL ) delete [] CFix_;
    if ( CFiy_  != NULL ) delete [] CFiy_;
    if ( CFiz_  != NULL ) delete [] CFiz_; 
                                        
    if ( CMix_  != NULL ) delete [] CMix_;
    if ( CMiy_  != NULL ) delete [] CMiy_;
    if ( CMiz_  != NULL ) delete [] CMiz_;
  
    if ( CTi_   != NULL ) delete [] CTi_;
    if ( CQi_   != NULL ) delete [] CQi_;
    if ( CPi_   != NULL ) delete [] CPi_;
 
    if ( CTi_h_ != NULL ) delete [] CTi_h_;
    if ( CQi_h_ != NULL ) delete [] CQi_h_;
    if ( CPi_h_ != NULL ) delete [] CPi_h_;
             
    // Viscous forces and moments
   
    if ( CLo_   != NULL ) delete [] CLo_;
    if ( CSo_   != NULL ) delete [] CSo_;
    if ( CDo_   != NULL ) delete [] CDo_;
    
    if ( CFox_  != NULL ) delete [] CFox_;
    if ( CFoy_  != NULL ) delete [] CFoy_;
    if ( CFoz_  != NULL ) delete [] CFoz_;
                                       
    if ( CMox_  != NULL ) delete [] CMox_;
    if ( CMoy_  != NULL ) delete [] CMoy_;
    if ( CMoz_  != NULL ) delete [] CMoz_;
                                    
    if ( CTo_   != NULL ) delete [] CTo_;
    if ( CQo_   != NULL ) delete [] CQo_;
    if ( CPo_   != NULL ) delete [] CPo_;
                                    
    if ( CTo_h_ != NULL ) delete [] CTo_h_;
    if ( CQo_h_ != NULL ) delete [] CQo_h_;
    if ( CPo_h_ != NULL ) delete [] CPo_h_;

    // Inviscid wake forces
    
    if ( CLiw_ != NULL ) delete [] CLiw_;
    if ( CDiw_ != NULL ) delete [] CDiw_;
    if ( CSiw_ != NULL ) delete [] CSiw_;

    if ( CFiwx_ != NULL ) delete [] CFiwx_;
    if ( CFiwy_ != NULL ) delete [] CFiwy_;
    if ( CFiwz_ != NULL ) delete [] CFiwz_;   
    
    // Invsicid force gradients wrt inputs
    
    if ( DCFxi_DOmega_ != NULL ) delete [] DCFxi_DOmega_;
    if ( DCFyi_DOmega_ != NULL ) delete [] DCFyi_DOmega_;
    if ( DCFzi_DOmega_ != NULL ) delete [] DCFzi_DOmega_;
                                                      
    if ( DCMxi_DOmega_ != NULL ) delete [] DCMxi_DOmega_;
    if ( DCMyi_DOmega_ != NULL ) delete [] DCMyi_DOmega_;
    if ( DCMzi_DOmega_ != NULL ) delete [] DCMzi_DOmega_;
                                                       
    // Viscous force gradients wrt inputs              
                                                       
    if ( DCFxo_DOmega_ != NULL ) delete [] DCFxo_DOmega_;
    if ( DCFyo_DOmega_ != NULL ) delete [] DCFyo_DOmega_;
    if ( DCFzo_DOmega_ != NULL ) delete [] DCFzo_DOmega_;
                                                     
    if ( DCMxo_DOmega_ != NULL ) delete [] DCMxo_DOmega_;
    if ( DCMyo_DOmega_ != NULL ) delete [] DCMyo_DOmega_;
    if ( DCMzo_DOmega_ != NULL ) delete [] DCMzo_DOmega_;
    
    // Efficiencies
    
    if ( FOM_  != NULL ) delete [] FOM_;
    if ( EtaP_ != NULL ) delete [] EtaP_;
        
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
    
    DeltaFlatPlateDragArea_ = ComponentGroup.DeltaFlatPlateDragArea_;
    
    FlatPlateDragRefReNumber_ = ComponentGroup.FlatPlateDragRefReNumber_;
        
    Omega_ = ComponentGroup.Omega_;
    
    RPM_ = 60. * Omega_ / (2. * PI);
    
    Angle_ = ComponentGroup.Angle_;
    
    DeltaTime_ = ComponentGroup.DeltaTime_;
    
    RotorDiameter_ = ComponentGroup.RotorDiameter_;
    
    AngleMax_ = ComponentGroup.AngleMax_;
    
    Quat_ = ComponentGroup.Quat_;
    
    InvQuat_ = ComponentGroup.InvQuat_;
    
    WQuat_ = ComponentGroup.WQuat_;
    
    pWQuat_pOmega_ = ComponentGroup.pWQuat_pOmega_;

    TotalRotationAngle_ = ComponentGroup.TotalRotationAngle_;
    
    NumberOfTimeSamples_ = ComponentGroup.NumberOfTimeSamples_;
    
    SizeForceAndMomentsTables(NumberOfTimeSamples_);
    
    for ( i = 0 ; i <= NumberOfTimeSamples_ ; i++ ) {
    
       // Invisicid forces and moments
       
       CLi_[i]   = ComponentGroup.CLi_[i];
       CDi_[i]   = ComponentGroup.CDi_[i];
       CSi_[i]   = ComponentGroup.CSi_[i];
                  
       CFix_[i]  = ComponentGroup.CFix_[i];
       CFiy_[i]  = ComponentGroup.CFiy_[i];
       CFiz_[i]  = ComponentGroup.CFiz_[i];
              
       CMix_[i]  = ComponentGroup.CMix_[i];
       CMiy_[i]  = ComponentGroup.CMiy_[i];
       CMiz_[i]  = ComponentGroup.CMiz_[i];
                 
       CTi_[i]   = ComponentGroup.CTi_[i];
       CQi_[i]   = ComponentGroup.CQi_[i];
       CPi_[i]   = ComponentGroup.CPi_[i];
                
       CTi_h_[i] = ComponentGroup.CTi_h_[i];
       CQi_h_[i] = ComponentGroup.CQi_h_[i];
       CPi_h_[i] = ComponentGroup.CPi_h_[i];
                
       // Viscous forces and moments
       
       CLo_[i]   = ComponentGroup.CLo_[i];
       CSo_[i]   = ComponentGroup.CSo_[i];
       CDo_[i]   = ComponentGroup.CDo_[i];
                 
       CFox_[i]  = ComponentGroup.CFox_[i];
       CFoy_[i]  = ComponentGroup.CFoy_[i];
       CFoz_[i]  = ComponentGroup.CFoz_[i];
                
       CMox_[i]  = ComponentGroup.CMox_[i];
       CMoy_[i]  = ComponentGroup.CMoy_[i];
       CMoz_[i]  = ComponentGroup.CMoz_[i];
                
       CTo_[i]   = ComponentGroup.CTo_[i];
       CQo_[i]   = ComponentGroup.CQo_[i];
       CPo_[i]   = ComponentGroup.CPo_[i];
                  
       CTo_h_[i] = ComponentGroup.CTo_h_[i];
       CQo_h_[i] = ComponentGroup.CQo_h_[i];
       CPo_h_[i] = ComponentGroup.CPo_h_[i];
       
       // Inviscid wake forces
       
       CLiw_[i] = ComponentGroup.CLiw_[i];
       CDiw_[i] = ComponentGroup.CDiw_[i];
       CSiw_[i] = ComponentGroup.CSiw_[i];
   
       CFiwx_[i] = ComponentGroup.CFiwx_[i];
       CFiwy_[i] = ComponentGroup.CFiwy_[i];
       CFiwz_[i] = ComponentGroup.CFiwz_[i];   
          
       // Efficiencies
       
       FOM_[i]   = ComponentGroup.FOM_[i];
       EtaP_[i]  = ComponentGroup.EtaP_[i];
       
       // Gradients

       DCFxi_DOmega_[i] = ComponentGroup.DCFxi_DOmega_[i];
       DCFyi_DOmega_[i] = ComponentGroup.DCFyi_DOmega_[i];
       DCFzi_DOmega_[i] = ComponentGroup.DCFzi_DOmega_[i];
                                                     
       DCMxi_DOmega_[i] = ComponentGroup.DCMxi_DOmega_[i];
       DCMyi_DOmega_[i] = ComponentGroup.DCMyi_DOmega_[i];
       DCMzi_DOmega_[i] = ComponentGroup.DCMzi_DOmega_[i];
                                                     
       DCFxo_DOmega_[i] = ComponentGroup.DCFxo_DOmega_[i];
       DCFyo_DOmega_[i] = ComponentGroup.DCFyo_DOmega_[i];
       DCFzo_DOmega_[i] = ComponentGroup.DCFzo_DOmega_[i];
                                                     
       DCMxo_DOmega_[i] = ComponentGroup.DCMxo_DOmega_[i];
       DCMyo_DOmega_[i] = ComponentGroup.DCMyo_DOmega_[i];
       DCMzo_DOmega_[i] = ComponentGroup.DCMzo_DOmega_[i];
             
    } 
        
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
#                           COMPONENT_GROUP Update                             #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::Update(void)
{

    if ( GeometryIsDynamic_ == STEADY_RATES || GeometryIsFixed_ || GeometryIsARotor_ > 0 ) {

       double Angle;
       
       // This routine is called to get back the rotation rates
       
       QUAT Omega;
       
       // Angular rate

       Omega(0) = Omega_ * RVec_[0];
       Omega(1) = Omega_ * RVec_[1];
       Omega(2) = Omega_ * RVec_[2];
       Omega(3) = 0.;
       
       // Quaternion for this rotation, and it's inverse
       
       Angle = 0.;
       
       Quat_.FormRotationQuat(RVec_,Angle);
       
       InvQuat_ = Quat_;
       
       InvQuat_.FormInverse();   
       
       // Quaternion rates
       
       WQuat_ = Omega * Quat_ * InvQuat_;
           
       // Partial of WQuat wrt omega
       
       Omega(0) = RVec_[0];
       Omega(1) = RVec_[1];
       Omega(2) = RVec_[2];
       Omega(3) = 0.;
       
       pWQuat_pOmega_ = Omega * Quat_ * InvQuat_;
               
       // Translation vector
       
       TVec_[0] = 0.;
       TVec_[1] = 0.;
       TVec_[2] = 0.;
       
       // Update the total quat
       
       TotalQuat_ = Quat_ * TotalQuat_;
       
    }

}
            
/*##############################################################################
#                                                                              #
#                         COMPONENT_GROUP Update                               #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::Update(double DeltaTime, double CurrentTime)
{

    // Time step and Current time
            
    DeltaTime_ = DeltaTime;
    
    CurrentTime_ = CurrentTime;
    
    // Steady rates or rotor analysis
       
    if ( GeometryIsDynamic_ == STEADY_RATES || GeometryIsFixed_ || GeometryIsARotor_ > 0 ) {
       
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
       
       printf ("Unknown dynamic or rotor state \n");fflush(NULL);
       printf("GeometryIsFixed_:   %d \n",GeometryIsFixed_);
       printf("GeometryIsDynamic_: %d \n",GeometryIsDynamic_);
       printf("GeometryIsARotor_:  %d \n",GeometryIsARotor_);
       
       exit(1);
       
    }

}

/*##############################################################################
#                                                                              #
#                    COMPONENT_GROUP ZeroOutTotals                             #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::ZeroOutTotals(void)
{
   
    TotalRotationAngle_ = 0.;

    TotalQuat_(0) = 0.;
    TotalQuat_(1) = 0.;
    TotalQuat_(2) = 0.;
    TotalQuat_(3) = 1.;
       
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
    
    Angle_ = Omega_ * DeltaTime_;

    Quat_.FormRotationQuat(RVec_,Angle_);
   
    InvQuat_ = Quat_;
   
    InvQuat_.FormInverse();   
    
    // Update current total rotation angle

    if ( CurrentTime_ > 0. ) TotalRotationAngle_ += Angle_;

    // Quaternion rates

    WQuat_ = Omega * Quat_ * InvQuat_;
        
    // Partial of WQuat wrt omega
    
    Omega(0) = RVec_[0];
    Omega(1) = RVec_[1];
    Omega(2) = RVec_[2];
    Omega(3) = 0.;

    pWQuat_pOmega_ = Omega * Quat_ * InvQuat_;
            
    // Translation vector

    TVec_[0] = UserInputVelocity_[0] * DeltaTime_;
    TVec_[1] = UserInputVelocity_[1] * DeltaTime_;
    TVec_[2] = UserInputVelocity_[2] * DeltaTime_;
    
    // Update the total quat

    if ( CurrentTime_ > 0. ) TotalQuat_ = Quat_ * TotalQuat_;

}

/*##############################################################################
#                                                                              #
#                    COMPONENT_GROUP UpdateQuaternions                         #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::UpdateQuaternions(double DeltaTime)
{

    QUAT Omega;
    
    // Angular rate
   
    Omega(0) = Omega_ * RVec_[0];
    Omega(1) = Omega_ * RVec_[1];
    Omega(2) = Omega_ * RVec_[2];
    Omega(3) = 0.;

    // Quaternion for this rotation, and it's inverse

    Angle_ = Omega_ * DeltaTime;

    Quat_.FormRotationQuat(RVec_,Angle_);
   
    InvQuat_ = Quat_;
   
    InvQuat_.FormInverse();   
  
    // Quaternion rates

    WQuat_ = Omega * Quat_ * InvQuat_;
    
    // Partial of WQuat wrt omega
    
    Omega(0) = RVec_[0];
    Omega(1) = RVec_[1];
    Omega(2) = RVec_[2];
    Omega(3) = 0.;

    pWQuat_pOmega_ = Omega * Quat_ * InvQuat_;
      
}

/*##############################################################################
#                                                                              #
#                    COMPONENT_GROUP UpdatePeriodicRates                       #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::UpdatePeriodicRates(void)
{

    double AngularRate, DeltaAngle;
    QUAT Omega;
    
    // Calculate angle

    Angle_ = AngleMax_*TORAD*sin(Omega_ * CurrentTime_);

    // Calculate change in angle
 
    DeltaAngle = AngleMax_*TORAD*sin(Omega_ * CurrentTime_ ) - AngleMax_*TORAD*sin(Omega_ * (CurrentTime_ - DeltaTime_));

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

    // Partial of WQuat wrt omega

    Omega(0) = RVec_[0];
    Omega(1) = RVec_[1];
    Omega(2) = RVec_[2];
    Omega(3) = 0.;

    pWQuat_pOmega_ = Omega * Quat_ * InvQuat_;
    
    // Translation vector

    TVec_[0] = UserInputVelocity_[0] * DeltaTime_;
    TVec_[1] = UserInputVelocity_[1] * DeltaTime_;
    TVec_[2] = UserInputVelocity_[2] * DeltaTime_;

    // Update the total quat
    
    TotalQuat_ = Quat_ * TotalQuat_;
    
}

/*##############################################################################
#                                                                              #
#                 COMPONENT_GROUP UpdateDynamicSystem                          #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::UpdateDynamicSystem(void)
{

    double Dot;
    QUAT Omega, DQuat, InvDQuat_, Half, dTime, Mass, Inertia;
    MATRIX Force(3), Moment(3), OmegaVec(3);

    dTime.Init(DeltaTime_);
    
    Half.Init(0.5);

    // User specified accelerations
    
    Force(1) = Mass_ * UserInputAcceleration_[0];
    Force(2) = Mass_ * UserInputAcceleration_[1];
    Force(3) = Mass_ * UserInputAcceleration_[2];

    // Aerodynamic Forces
    
    Force(1) += (CFox_[1] + CFix_[1]) * 0.5 * Density_ * Sref_ * Vref_ * Vref_;
    Force(2) += (CFoy_[1] + CFiy_[1]) * 0.5 * Density_ * Sref_ * Vref_ * Vref_;
    Force(3) += (CFoz_[1] + CFiz_[1]) * 0.5 * Density_ * Sref_ * Vref_ * Vref_;

    if ( CurrentTime_ < StartDynamicAnalysisTime_ ) {
       
       Force(1) = 0.;
       Force(2) = 0.;
       Force(3) = 0.;

    }
       
    // Linear momentum
    
    LinearMomentum_(1) += DeltaTime_ * Force(1);
    LinearMomentum_(2) += DeltaTime_ * Force(2);
    LinearMomentum_(3) += DeltaTime_ * Force(3);

    // Velocity
    
    Velocity_(1) = LinearMomentum_(1) / Mass_;
    Velocity_(2) = LinearMomentum_(2) / Mass_;
    Velocity_(3) = LinearMomentum_(3) / Mass_;

    // Translations
    
    TVec_[0] = Velocity_(1) * DeltaTime_;
    TVec_[1] = Velocity_(2) * DeltaTime_;
    TVec_[2] = Velocity_(3) * DeltaTime_;

    // Moments

    Moment(1) = ( CMix_[1] + CMox_[1] ) * 0.5 * Density_ * Bref_ * Sref_ * Vref_ * Vref_;
    Moment(2) = ( CMiy_[1] + CMoy_[1] ) * 0.5 * Density_ * Cref_ * Sref_ * Vref_ * Vref_;
    Moment(3) = ( CMiz_[1] + CMoz_[1] ) * 0.5 * Density_ * Bref_ * Sref_ * Vref_ * Vref_;

    if ( CurrentTime_ < StartDynamicAnalysisTime_ ) {
       
       Moment(1) = 0.;
       Moment(2) = 0.;
       Moment(3) = 0.;

    }
    
    // Angular momentum
    
    AngularMomentum_(1) += DeltaTime_ * Moment(1);
    AngularMomentum_(2) += DeltaTime_ * Moment(2);
    AngularMomentum_(3) += DeltaTime_ * Moment(3);
    
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
    
    // Partial of WQuat wrt omega
    
    Omega(0) = RVec_[0];
    Omega(1) = RVec_[1];
    Omega(2) = RVec_[2];
    Omega(3) = 0.;

    pWQuat_pOmega_ = Omega * Quat_ * InvQuat_;
        
    // Update inertia matrix
    
    QuatToMatrix(TotalQuat_, RotationMatrix_, RotationMatrixInverse_);
    
    InertiaMatrixInverse_ = RotationMatrix_ * InertiaMatrixInverse_ * RotationMatrixInverse_;

    // Update the total quat
    
    TotalQuat_ = Quat_ * TotalQuat_;
    
}

/*##############################################################################
#                                                                              #
#                      COMPONENT_GROUP QuatToMatrix                            #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::QuatToMatrix(QUAT &Quat, MATRIX &Matrix, MATRIX &Inverse)
{
   
   double Qx, Qy, Qz, s;
   
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

    fprintf  (File,"GroupName = %s \n",GroupName_);
    
    fprintf  (File,"NumberOfComponents = %d \n",NumberOfComponents_);
 
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       fprintf  (File,"%d \n",ComponentList_[i]);
       
    }
    
    fprintf  (File,"GeometryIsFixed = %d \n",GeometryIsFixed_);
    
    fprintf  (File,"GeometryIsDynamic = %d \n",GeometryIsDynamic_);
    
    fprintf  (File,"GeometryIsARotor = %d \n",GeometryIsARotor_);
    
    fprintf  (File,"RotorDiameter = %f \n",RotorDiameter_);
    
    fprintf  (File,"OVec = %lf %lf %lf \n",OVec_[0], OVec_[1], OVec_[2]);
    
    fprintf  (File,"RVec = %lf %lf %lf \n",RVec_[0], RVec_[1], RVec_[2]);
    
    fprintf  (File,"Velocity = %lf %lf %lf \n",UserInputVelocity_[0], UserInputVelocity_[1], UserInputVelocity_[2]);

    fprintf  (File,"Acceleration = %lf %lf %lf \n",UserInputAcceleration_[0], UserInputAcceleration_[1], UserInputAcceleration_[2]);

    fprintf  (File,"Omega = %lf \n",Omega_);
    
    fprintf  (File,"Mass = %lf \n",Mass_);
    
    fprintf  (File,"Ixx = %lf \n",Ixx_);
                  
    fprintf  (File,"Iyy = %lf \n",Iyy_);
                  
    fprintf  (File,"Izz = %lf \n",Izz_);
                  
    fprintf  (File,"Ixy = %lf \n",Ixy_);
                  
    fprintf  (File,"Ixz = %lf \n",Ixz_);
                  
    fprintf  (File,"Iyz = %lf \n",Iyz_);

    fprintf  (File,"StartAnalysisTime = %lf \n",StartDynamicAnalysisTime_);


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
    char DumChar[MAX_CHAR_SIZE];   
              
    // Parse the group data file              
              
    fscanf(File,"GroupName = %1999s \n",GroupName_);
    
    printf ("GroupName_: %s \n",GroupName_);

    fscanf(File,"NumberOfComponents = %d \n",&NumberOfComponents_);
    
    printf ("NumberOfComponents: %d \n",NumberOfComponents_);

    SizeList(NumberOfComponents_);
    
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       fscanf(File,"%d \n",&(ComponentList_[i]));

    }
    
    fscanf(File,"GeometryIsFixed = %d \n",&GeometryIsFixed_);
    
    printf ("GeometryIsFixed: %d \n",GeometryIsFixed_);
    
    fscanf(File,"GeometryIsDynamic = %d \n",&GeometryIsDynamic_);
    
    printf ("GeometryIsDynamic_: %d \n",GeometryIsDynamic_);
    
    fscanf(File,"GeometryIsARotor = %d \n",&GeometryIsARotor_);
    
    printf ("GeometryIsARotor_: %d \n",GeometryIsARotor_);    
    
    fscanf(File,"RotorDiameter = %lf \n",&RotorDiameter_);

    printf ("RotorDiameter_: %f \n",RotorDiameter_);

    fscanf(File,"OVec = %lf %lf %lf \n",&(OVec_[0]),&(OVec_[1]),&(OVec_[2]));

    printf ("OVec: %f %f %f\n",(OVec_[0]),(OVec_[1]),(OVec_[2]));
    
    fscanf(File,"RVec = %lf %lf %lf \n",&(RVec_[0]),&(RVec_[1]),&(RVec_[2]));
    
    printf ("RVec_: %f %f %f\n",(RVec_[0]),(RVec_[1]),(RVec_[2]));
    
    fscanf(File,"Velocity = %lf %lf %lf \n",&(UserInputVelocity_[0]),&(UserInputVelocity_[1]),&(UserInputVelocity_[2]));

    printf ("Velocity: %f %f %f\n",(UserInputVelocity_[0]),(UserInputVelocity_[1]),(UserInputVelocity_[2]));

    fscanf(File,"Acceleration = %lf %lf %lf \n",&(UserInputAcceleration_[0]), &(UserInputAcceleration_[1]), &(UserInputAcceleration_[2]));

    fscanf(File,"Omega = %lf \n",&Omega_);
    
    printf ("Omega: %f \n",Omega_);
    
    fscanf(File,"Mass = %lf \n",&Mass_);
    
    fscanf(File,"Ixx = %lf \n",&Ixx_);
                  
    fscanf(File,"Iyy = %lf \n",&Iyy_);
                  
    fscanf(File,"Izz = %lf \n",&Izz_);
                  
    fscanf(File,"Ixy = %lf \n",&Ixy_);
                  
    fscanf(File,"Ixz = %lf \n",&Ixz_);
                  
    fscanf(File,"Iyz = %lf \n",&Iyz_);
    
    // Version 6.x stuff...
    
    fgetpos(File,&LineLocation);
    
    if ( fgets(DumChar,MAX_CHAR_SIZE,File) != NULL ) {
       
       if ( strncmp(DumChar,"StartAnalysisTime",17) == 0 ) {
          
          sscanf(DumChar,"StartAnalysisTime = %lf \n",&StartDynamicAnalysisTime_);
          
          printf ("StartAnalysisTime: %f \n",StartDynamicAnalysisTime_);
          
       }
       
       else {
          
          fsetpos(File,&LineLocation);
          
       }
       
    }
    
    // Version 7.x stuff
    
    fgetpos(File,&LineLocation);
    
    if ( fgets(DumChar,MAX_CHAR_SIZE,File) != NULL ) {
       
       if ( strncmp(DumChar,"DeltaFlatPlateDragArea",22) == 0 ) {
          
          sscanf(DumChar,"DeltaFlatPlateDragArea = %lf \n",&DeltaFlatPlateDragArea_);
          
          printf ("DeltaFlatPlateDragArea: %f \n",DeltaFlatPlateDragArea_);
          
       }
       
       else {
          
          fsetpos(File,&LineLocation);
          
       }
       
    }    

    fgetpos(File,&LineLocation);
    
    if ( fgets(DumChar,MAX_CHAR_SIZE,File) != NULL ) {
       
       if ( strncmp(DumChar,"FlatPlateDragRefReNumber",24) == 0 ) {
          
          sscanf(DumChar,"FlatPlateDragRefReNumber = %lf \n",&FlatPlateDragRefReNumber_);
          
          printf ("FlatPlateDragRefReNumber: %f \n",FlatPlateDragRefReNumber_);
          
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
    
    // Calculate blade rpm
    
    RPM_ = 60. * Omega_ / (2. * PI);
   
}

/*##############################################################################
#                                                                              #
#                 COMPONENT_GROUP ZeroForcesAndMoments                         #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::ZeroForcesAndMoments(void)
{

    int i;
    
    for ( i = 0 ; i <= NumberOfTimeSamples_ + 2 ; i++ ) {
    
       // Invisicid forces and moments
       
       CLi_[i]   = 0.;
       CDi_[i]   = 0.;
       CSi_[i]   = 0.;
                 
       CFix_[i]  = 0.;
       CFiy_[i]  = 0.;
       CFiz_[i]  = 0.;
                 
       CMix_[i]  = 0.;
       CMiy_[i]  = 0.;
       CMiz_[i]  = 0.;
                 
       CTi_[i]   = 0.;
       CQi_[i]   = 0.;
       CPi_[i]   = 0.;
       
       CTi_h_[i] = 0.;
       CQi_h_[i] = 0.;
       CPi_h_[i] = 0.;
                
       // Viscous forces and moments
       
       CLo_[i]   = 0.;
       CSo_[i]   = 0.;
       CDo_[i]   = 0.;
                 
       CFox_[i]  = 0.;
       CFoy_[i]  = 0.;
       CFoz_[i]  = 0.;
                 
       CMox_[i]  = 0.;
       CMoy_[i]  = 0.;
       CMoz_[i]  = 0.;
                 
       CTo_[i]   = 0.;
       CQo_[i]   = 0.;
       CPo_[i]   = 0.;
       
       CTo_h_[i] = 0.;
       CQo_h_[i] = 0.;
       CPo_h_[i] = 0.;
       
       // Inviscid wake forces
       
       CLiw_[i] = 0.;
       CDiw_[i] = 0.;
       CSiw_[i] = 0.;
              
       CFiwx_[i] = 0.;
       CFiwy_[i] = 0.;
       CFiwz_[i] = 0.;  
              
       // Efficiencies
       
       FOM_[i]   = 0.;
       EtaP_[i]  = 0.;
         
    }
 
}

/*##############################################################################
#                                                                              #
#            COMPONENT_GROUP CalculateAverageForcesAndMoments                  #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::CalculateAverageForcesAndMoments(void)
{

    CalculateAverageForcesAndMoments_(1);

}

/*##############################################################################
#                                                                              #
#            COMPONENT_GROUP CalculateAverageForcesAndMoments                  #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::CalculateAverageForcesAndMoments(int StartSample)
{

printf("StartSample: %d \n",StartSample);

    CalculateAverageForcesAndMoments_(StartSample);

}

/*##############################################################################
#                                                                              #
#            COMPONENT_GROUP CalculateAverageForcesAndMoments                  #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::CalculateAverageForcesAndMoments_(int StartSample)
{

    int i, NumberOfCases;
    
    NumberOfCases = NumberOfTimeSamples_ - StartSample + 1;
    
    for ( i = StartSample + 1 ; i <= NumberOfTimeSamples_ + 1 ; i++ ) {
    
       // Invisicid forces and moments
       
         CLi_[0] +=   CLi_[i] / NumberOfCases;
         CDi_[0] +=   CDi_[i] / NumberOfCases;
         CSi_[0] +=   CSi_[i] / NumberOfCases;
                          
                             
        CFix_[0] +=  CFix_[i] / NumberOfCases;
        CFiy_[0] +=  CFiy_[i] / NumberOfCases;
        CFiz_[0] +=  CFiz_[i] / NumberOfCases;
                            
        CMix_[0] +=  CMix_[i] / NumberOfCases;
        CMiy_[0] +=  CMiy_[i] / NumberOfCases;
        CMiz_[0] +=  CMiz_[i] / NumberOfCases;
                             
         CTi_[0] +=   CTi_[i] / NumberOfCases;
         CQi_[0] +=   CQi_[i] / NumberOfCases;
         CPi_[0] +=   CPi_[i] / NumberOfCases;
                             
       CTi_h_[0] += CTi_h_[i] / NumberOfCases;
       CQi_h_[0] += CQi_h_[i] / NumberOfCases;
       CPi_h_[0] += CPi_h_[i] / NumberOfCases;
                
       // Viscous forces and moments
       
         CLo_[0] +=   CLo_[i] / NumberOfCases;
         CSo_[0] +=   CSo_[i] / NumberOfCases;
         CDo_[0] +=   CDo_[i] / NumberOfCases;
                               
        CFox_[0] +=  CFox_[i] / NumberOfCases;
        CFoy_[0] +=  CFoy_[i] / NumberOfCases;
        CFoz_[0] +=  CFoz_[i] / NumberOfCases;
                            
        CMox_[0] +=  CMox_[i] / NumberOfCases;
        CMoy_[0] +=  CMoy_[i] / NumberOfCases;
        CMoz_[0] +=  CMoz_[i] / NumberOfCases;
                             
         CTo_[0] +=   CTo_[i] / NumberOfCases;
         CQo_[0] +=   CQo_[i] / NumberOfCases;
         CPo_[0] +=   CPo_[i] / NumberOfCases;
                               
       CTo_h_[0] += CTo_h_[i] / NumberOfCases;
       CQo_h_[0] += CQo_h_[i] / NumberOfCases;
       CPo_h_[0] += CPo_h_[i] / NumberOfCases;

       // Inviscid wake forces
       
        CLiw_[0] += CLiw_[i] / NumberOfCases; 
        CDiw_[0] += CDiw_[i] / NumberOfCases;
        CSiw_[0] += CSiw_[i] / NumberOfCases;
          
        CFiwx_[0] += CFiwx_[i] / NumberOfCases;
        CFiwy_[0] += CFiwy_[i] / NumberOfCases;
        CFiwz_[0] += CFiwz_[i] / NumberOfCases;
                                      
       // Efficiencies         
                               
         FOM_[0] +=   FOM_[i] / NumberOfCases;
        EtaP_[0] +=  EtaP_[i] / NumberOfCases;
         
    }
 
}

/*##############################################################################
#                                                                              #
#                COMPONENT_GROUP SizeForceAndMomentsTables                     #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::SizeForceAndMomentsTables(void)
{

    NumberOfTimeSamples_ = 1;

    SizeForceAndMomentsTables_();
    
    ZeroForcesAndMoments();
        
}

/*##############################################################################
#                                                                              #
#            COMPONENT_GROUP SizeForceAndMomentsTables                         #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::SizeForceAndMomentsTables(int NumberOfTimeSamples)
{

    NumberOfTimeSamples_ = NumberOfTimeSamples;

    SizeForceAndMomentsTables_();
    
    ZeroForcesAndMoments();
        
}

/*##############################################################################
#                                                                              #
#            COMPONENT_GROUP SizeForceAndMomentsTables_                        #
#                                                                              #
##############################################################################*/

void COMPONENT_GROUP::SizeForceAndMomentsTables_(void)
{

    // Invisicid forces and moments
    
    CLi_   = new double[NumberOfTimeSamples_ + 3];
    CDi_   = new double[NumberOfTimeSamples_ + 3];
    CSi_   = new double[NumberOfTimeSamples_ + 3];
                                             
    CFix_  = new double[NumberOfTimeSamples_ + 3];
    CFiy_  = new double[NumberOfTimeSamples_ + 3];
    CFiz_  = new double[NumberOfTimeSamples_ + 3];
                                             
    CMix_  = new double[NumberOfTimeSamples_ + 3];
    CMiy_  = new double[NumberOfTimeSamples_ + 3];
    CMiz_  = new double[NumberOfTimeSamples_ + 3];
                                             
    CTi_   = new double[NumberOfTimeSamples_ + 3];
    CQi_   = new double[NumberOfTimeSamples_ + 3];
    CPi_   = new double[NumberOfTimeSamples_ + 3];
                                             
    CTi_h_ = new double[NumberOfTimeSamples_ + 3];
    CQi_h_ = new double[NumberOfTimeSamples_ + 3];
    CPi_h_ = new double[NumberOfTimeSamples_ + 3];
                                              
    // Viscous forces and moments             
                                              
    CLo_   = new double[NumberOfTimeSamples_ + 3];
    CSo_   = new double[NumberOfTimeSamples_ + 3];
    CDo_   = new double[NumberOfTimeSamples_ + 3];
                                             
    CFox_  = new double[NumberOfTimeSamples_ + 3];
    CFoy_  = new double[NumberOfTimeSamples_ + 3];
    CFoz_  = new double[NumberOfTimeSamples_ + 3];
                                              
    CMox_  = new double[NumberOfTimeSamples_ + 3];
    CMoy_  = new double[NumberOfTimeSamples_ + 3];
    CMoz_  = new double[NumberOfTimeSamples_ + 3];
                                              
    CTo_   = new double[NumberOfTimeSamples_ + 3];
    CQo_   = new double[NumberOfTimeSamples_ + 3];
    CPo_   = new double[NumberOfTimeSamples_ + 3];
                                              
    CTo_h_ = new double[NumberOfTimeSamples_ + 3];
    CQo_h_ = new double[NumberOfTimeSamples_ + 3];
    CPo_h_ = new double[NumberOfTimeSamples_ + 3];

    // Inviscid wake forces
    
    CLiw_  = new double[NumberOfTimeSamples_ + 3];
    CDiw_  = new double[NumberOfTimeSamples_ + 3];
    CSiw_  = new double[NumberOfTimeSamples_ + 3];
           
    CFiwx_ = new double[NumberOfTimeSamples_ + 3];
    CFiwy_ = new double[NumberOfTimeSamples_ + 3];
    CFiwz_ = new double[NumberOfTimeSamples_ + 3];
    
    // Gradients wrt omega
        
    DCFxi_DOmega_ = new double[NumberOfTimeSamples_ + 3];
    DCFyi_DOmega_ = new double[NumberOfTimeSamples_ + 3];
    DCFzi_DOmega_ = new double[NumberOfTimeSamples_ + 3];
                                                     
    DCMxi_DOmega_ = new double[NumberOfTimeSamples_ + 3];
    DCMyi_DOmega_ = new double[NumberOfTimeSamples_ + 3];
    DCMzi_DOmega_ = new double[NumberOfTimeSamples_ + 3];
                                                     
    DCFxo_DOmega_ = new double[NumberOfTimeSamples_ + 3];
    DCFyo_DOmega_ = new double[NumberOfTimeSamples_ + 3];
    DCFzo_DOmega_ = new double[NumberOfTimeSamples_ + 3];
                                                     
    DCMxo_DOmega_ = new double[NumberOfTimeSamples_ + 3];
    DCMyo_DOmega_ = new double[NumberOfTimeSamples_ + 3];
    DCMzo_DOmega_ = new double[NumberOfTimeSamples_ + 3];
    
    // Efficiencies
    
    FOM_  = new double[NumberOfTimeSamples_ + 3];
    EtaP_ = new double[NumberOfTimeSamples_ + 3];
       
}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER CalculateRotorCoefficients                       #
#                                                                              #
##############################################################################*/

double COMPONENT_GROUP::CalculateThrust(int DragComponent, int TimeType)
{

    double Thrust, RPM, n;

    RPM = 60. * Omega_ / (2. * PI);

    // Revs per second
    
    n = RPM / 60.;
    
    Thrust = 0.;

    if ( DragComponent == VISCOUS_FORCES  ) Thrust = CTo_[TimeType] * Density_ * n * n * pow(RotorDiameter_, 4.);
    if ( DragComponent == INVISCID_FORCES ) Thrust = CTi_[TimeType] * Density_ * n * n * pow(RotorDiameter_, 4.);
    
    return Thrust;

}   
   
/*##############################################################################
#                                                                              #
#                  VSP_SOLVER CalculateRotorCoefficients                       #
#                                                                              #
##############################################################################*/

double COMPONENT_GROUP::CalculateMoment(int DragComponent, int TimeType)
{
   
    double Moment, RPM, n;

    RPM = 60. * Omega_ / (2. * PI);

    // Revs per second
    
    n = RPM / 60.;
        
    Moment = 0.;
    
    if ( DragComponent == VISCOUS_FORCES  ) Moment = CQo_[TimeType] * Density_ * n * n * pow(RotorDiameter_, 5.);
    if ( DragComponent == INVISCID_FORCES ) Moment = CQi_[TimeType] * Density_ * n * n * pow(RotorDiameter_, 5.);
    
    return Moment;

}  

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER CalculateRotorCoefficients                       #
#                                                                              #
##############################################################################*/

double COMPONENT_GROUP::CalculatePower(int DragComponent, int TimeType)
{
         
    double Moment, Power, RPM, n;

    RPM = 60. * Omega_ / (2. * PI);

    // Revs per second
    
    n = RPM / 60.;
    
    Power = Moment = 0.;
        
    if ( DragComponent == VISCOUS_FORCES  ) Moment = CQo_[TimeType] * Density_ * n * n * pow(RotorDiameter_, 5.);
    if ( DragComponent == INVISCID_FORCES ) Moment = CQi_[TimeType] * Density_ * n * n * pow(RotorDiameter_, 5.);
    
    Power = Moment * Omega_;
    
    return Power;
    
}         

#include "END_NAME_SPACE.H"







