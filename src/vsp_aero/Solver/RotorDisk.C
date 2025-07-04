//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "RotorDisk.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                              ROTOR_DISK constructor                          #
#                                                                              #
##############################################################################*/

ROTOR_DISK::ROTOR_DISK(void)
{

    // Use init routine

    Initialize();
   
}

/*##############################################################################
#                                                                              #
#                              ROTOR_DISK Initialize                           #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::Initialize(void)
{

    // Use init routine

    ComponentID_ = 0;

    RotorXYZ_[0] = RotorXYZ_[1] = RotorXYZ_[2] = 0.;
    
    RotorNormal_[0] = RotorNormal_[1] = RotorNormal_[2] = 0.;
    
    RotorRadius_ = 1.;
    
    RotorHubRadius_ = 0.;

    Density_ = 0.;
    
    Vinf_[0] = 0.; Vinf_[1] = 0.; Vinf_[2] = 0.;

    RotorRadius_ = 0.;
    
    RotorRPM_ = 0.;

    Rotor_CT_ = 0.;
    
    Rotor_CP_ = 0.;
   
}

/*##############################################################################
#                                                                              #
#                               ROTOR_DISK destructor                          #
#                                                                              #
##############################################################################*/

ROTOR_DISK::~ROTOR_DISK(void)
{

    // Nothing to do..

}


/*##############################################################################
#                                                                              #
#                           ROTOR_DISK operator=                               #              
#                                                                              #
##############################################################################*/

ROTOR_DISK& ROTOR_DISK::operator=(const ROTOR_DISK &RotorDisk)
{

    // Copy stuff over...
    
    ComponentID_ = RotorDisk.ComponentID_;
    
    RotorXYZ_[0] = RotorDisk.RotorXYZ_[0];
    
    RotorXYZ_[1] = RotorDisk.RotorXYZ_[1];
    
    RotorXYZ_[2] = RotorDisk.RotorXYZ_[2];
    
    RotorNormal_[0] = RotorDisk.RotorNormal_[0];
    
    RotorNormal_[1] = RotorDisk.RotorNormal_[1];
    
    RotorNormal_[2] = RotorDisk.RotorNormal_[2];
    
    RotorRadius_ = RotorDisk.RotorRadius_;

    Density_ = RotorDisk.Density_;
    
    Vinf_[0] = RotorDisk.Vinf_[0];
    Vinf_[1] = RotorDisk.Vinf_[1];
    Vinf_[2] = RotorDisk.Vinf_[2];
    
    RotorRadius_ = RotorDisk.RotorRadius_;
    
    RotorRPM_= RotorDisk.RotorRPM_;

    Rotor_CT_= RotorDisk.Rotor_CT_;
    
    Rotor_CP_= RotorDisk.Rotor_CP_;
    
    return *this;
    
}

/*##############################################################################
#                                                                              #
#                       ROTOR_DISK VelocityPotenial                            #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::VelocityPotential(double xyz_p[3], double q[5])
{

    double Vh, z, r, vec[3], rvec[3], tvec[3], mag;
    double Velocity_X, Velocity_R, Velocity_T, Omega, PhiTotal;
    double Phi_D, Phi_D_z;
 
    vec[0] = xyz_p[0] - RotorXYZ_[0];
    vec[1] = xyz_p[1] - RotorXYZ_[1];
    vec[2] = xyz_p[2] - RotorXYZ_[2];
    
    // Axial distance
    
    z = vector_dot(vec,RotorNormal_);
    
    rvec[0] = xyz_p[0] - z*RotorNormal_[0]; 
    rvec[1] = xyz_p[1] - z*RotorNormal_[1]; 
    rvec[2] = xyz_p[2] - z*RotorNormal_[2]; 

    // Radial distance
    
    r = sqrt(vector_dot(rvec,rvec));
    
    if ( r > 0. ) {
    
       rvec[0] /= r;
       rvec[1] /= r;
       rvec[2] /= r;
       
    }

    // Angular velocity direction
    
    vector_cross(RotorNormal_, rvec, tvec);

    mag = sqrt(vector_dot(tvec,tvec));
    
    if ( mag > 0. ) {
    
       tvec[0] /= mag;
       tvec[1] /= mag;
       tvec[2] /= mag;  
       
    }

    // Hover velocity
    
    Vh = sqrt(RotorThrust()/(2.*Density_*RotorArea()));
    
    // Decay function
    
    Phi_D = z + RotorRadius_*exp(-r/RotorRadius_);
    
    Phi_D_z = 1. - exp(r/RotorRadius_);
        
    // Axial velocity
    
    Velocity_X = Phi_D_z * 2. * Vh;

    PhiTotal = Phi_D * 2. * Vh;

    // Radial Velocity
    
    Velocity_R = 0.;
    
    // Angular velocity
    
    Omega = RotorRPM_ * 2. * PI / 60.;
    
    Velocity_T = 0.;

    // Convert to xyz coordinates
    
    q[0] = Velocity_X*RotorNormal_[0] + Velocity_R * rvec[0] + Velocity_T * tvec[0];
    q[1] = Velocity_X*RotorNormal_[1] + Velocity_R * rvec[1] + Velocity_T * tvec[1];
    q[2] = Velocity_X*RotorNormal_[2] + Velocity_R * rvec[2] + Velocity_T * tvec[2];
    
    q[3] = PhiTotal;
    
  //  printf("z: %lf ... Velocity_T * tvec[1]: %lf \n", xyz[2], Velocity_T * tvec[1]);

}

// pV_pxyz
// pV_pCT
// pV_pCP
// pV_pRadius
// pV_pVinf
// pV_pNormal


/*##############################################################################
#                                                                              #
#                              ROTOR_DISK Velocity                             #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::Velocity(double xyz_p[3], double q[5])
{

    double z, r, zp;
    double Term1, Term2, Vh, alpha, sinf, f, vec[3], rvec[3], tvec[3], mag;
    double Velocity_X, Velocity_R, Velocity_T, Omega, VxR0, Delta_Cp, Fact;
    double eta_mom, eta_prop, CT_h, CP_h, Sigma_Cd, Sigma_Cl, Vo;
    double RotorWakeNormal[3];
    
    double pVinfMag_pVinf[3];
    double pVinfMag_pRotorNormal[3];
    
    double pVh_pVinfMag;
    double pVh_pRotorThrust;
    double pVh_pDensity;
    double pVh_pRotorArea;
    
    double pRotorWakeNormal0_pVh;
    double pRotorWakeNormal1_pVh;
    double pRotorWakeNormal2_pVh;    
    double pRotorWakeNormal0_pRotorNormal[3];
    double pRotorWakeNormal1_pRotorNormal[3];
    double pRotorWakeNormal2_pRotorNormal[3];
    double pRotorWakeNormal0_pVinf[3];
    double pRotorWakeNormal1_pVinf[3];
    double pRotorWakeNormal2_pVinf[3];
    
    double dmag_dRotorWakeNormal[3];
    
    double pvec0_pxyz_p[3];
    double pvec1_pxyz_p[3];
    double pvec2_pxyz_p[3];
    
    double pvec0_pRotorXYZ[3];
    double pvec1_pRotorXYZ[3];
    double pvec2_pRotorXYZ[3];
    
    double pz_pvec[3];
    double pz_pRotorNormal[3];

    double pzp_pvec[3];
    double pzp_pRotorWakeNormal[3];

    double prvec0_pvec[3];
    double prvec1_pvec[3];
    double prvec2_pvec[3];

    double prvec0_pz;
    double prvec1_pz;
    double prvec2_pz;
    
    double prvec0_pRotorWakeNormal[3];
    double prvec1_pRotorWakeNormal[3];
    double prvec2_pRotorWakeNormal[3];
    
    double pr_pvec[3];
    
    double pmag_prvec[3];
    
    // Local free stream velocity normal to rotor
            
    VinfMag_ = vector_dot(Vinf_,RotorNormal_);
    
    pVinfMag_pVinf[0] = RotorNormal_[0];
    pVinfMag_pVinf[1] = RotorNormal_[1];
    pVinfMag_pVinf[2] = RotorNormal_[2];

    pVinfMag_pRotorNormal[0] = Vinf_[0];
    pVinfMag_pRotorNormal[1] = Vinf_[1];
    pVinfMag_pRotorNormal[2] = Vinf_[2];

    // Rotor down wash
    
    Vh = -0.5*VinfMag_ + sqrt( pow(0.5*VinfMag_,2.) + RotorThrust()/(2.*Density_*RotorArea()) );
    
    pVh_pVinfMag = -0.5 + 0.5 / sqrt( pow(0.5*VinfMag_,2.) + RotorThrust()/(2.*Density_*RotorArea()) ) * VinfMag_;
    
    pVh_pRotorThrust =    0.5 / sqrt( pow(0.5*VinfMag_,2.) + RotorThrust()/(2.*Density_*RotorArea()) ) / ( 2.*Density_*RotorArea() );
    
    pVh_pDensity =        0.5 / sqrt( pow(0.5*VinfMag_,2.) + RotorThrust()/(2.*Density_*RotorArea()) ) / ( -2.*RotorThrust()*RotorArea()/pow(2.*Density_*RotorArea(),2) );

    pVh_pRotorArea =      0.5 / sqrt( pow(0.5*VinfMag_,2.) + RotorThrust()/(2.*Density_*RotorArea()) ) / ( -2.*RotorThrust()*Density_   /pow(2.*Density_*RotorArea(),2) );
    
    // Calculate approximate direction of rotor down wash 
    // ... just a vector sum of the rotor wash + Vinf
    
    RotorWakeNormal[0] = Vh * RotorNormal_[0] + Vinf_[0];
    RotorWakeNormal[1] = Vh * RotorNormal_[1] + Vinf_[1];
    RotorWakeNormal[2] = Vh * RotorNormal_[2] + Vinf_[2];
   
    pRotorWakeNormal0_pVh = RotorNormal_[0];
    pRotorWakeNormal1_pVh = RotorNormal_[1];
    pRotorWakeNormal2_pVh = RotorNormal_[2];
    
    pRotorWakeNormal0_pRotorNormal[0] = Vh;
    pRotorWakeNormal0_pRotorNormal[1] = 0.;
    pRotorWakeNormal0_pRotorNormal[2] = 0.;

    pRotorWakeNormal1_pRotorNormal[0] = 0.;
    pRotorWakeNormal1_pRotorNormal[1] = Vh;
    pRotorWakeNormal1_pRotorNormal[2] = 0.;

    pRotorWakeNormal2_pRotorNormal[0] = 0.;
    pRotorWakeNormal2_pRotorNormal[1] = 0.;
    pRotorWakeNormal2_pRotorNormal[2] = Vh;
            
    pRotorWakeNormal0_pVinf[0] = 1.;
    pRotorWakeNormal0_pVinf[1] = 0.;
    pRotorWakeNormal0_pVinf[2] = 0.;

    pRotorWakeNormal1_pVinf[0] = 0.;
    pRotorWakeNormal1_pVinf[1] = 1.;
    pRotorWakeNormal1_pVinf[2] = 0.;
    
    pRotorWakeNormal2_pVinf[0] = 0.;
    pRotorWakeNormal2_pVinf[1] = 0.;
    pRotorWakeNormal2_pVinf[2] = 1.;
    
    mag = sqrt(vector_dot(RotorWakeNormal,RotorWakeNormal));
    
    dmag_dRotorWakeNormal[0] = RotorWakeNormal[0] / mag;
    dmag_dRotorWakeNormal[1] = RotorWakeNormal[1] / mag;
    dmag_dRotorWakeNormal[2] = RotorWakeNormal[2] / mag;
                                                                            
    RotorWakeNormal[0] /= mag;
    RotorWakeNormal[1] /= mag;
    RotorWakeNormal[2] /= mag;
  
    // Scale the above derivatives
    
    pRotorWakeNormal0_pVh = pRotorWakeNormal0_pVh/mag - pRotorWakeNormal0_pVh * dmag_dRotorWakeNormal[0] / ( mag * mag );
    pRotorWakeNormal1_pVh = pRotorWakeNormal1_pVh/mag - pRotorWakeNormal1_pVh * dmag_dRotorWakeNormal[1] / ( mag * mag );
    pRotorWakeNormal2_pVh = pRotorWakeNormal2_pVh/mag - pRotorWakeNormal2_pVh * dmag_dRotorWakeNormal[2] / ( mag * mag );
    
    pRotorWakeNormal0_pRotorNormal[0] = pRotorWakeNormal0_pRotorNormal[0]/mag - RotorWakeNormal[0] * dmag_dRotorWakeNormal[0] / (mag * mag);
    pRotorWakeNormal0_pRotorNormal[1] = pRotorWakeNormal0_pRotorNormal[1]/mag - RotorWakeNormal[0] * dmag_dRotorWakeNormal[0] / (mag * mag); 
    pRotorWakeNormal0_pRotorNormal[2] = pRotorWakeNormal0_pRotorNormal[2]/mag - RotorWakeNormal[0] * dmag_dRotorWakeNormal[0] / (mag * mag); 

    pRotorWakeNormal1_pRotorNormal[0] = pRotorWakeNormal1_pRotorNormal[0]/mag - RotorWakeNormal[1] * dmag_dRotorWakeNormal[1] / (mag * mag);
    pRotorWakeNormal1_pRotorNormal[1] = pRotorWakeNormal1_pRotorNormal[1]/mag - RotorWakeNormal[1] * dmag_dRotorWakeNormal[1] / (mag * mag); 
    pRotorWakeNormal1_pRotorNormal[2] = pRotorWakeNormal1_pRotorNormal[2]/mag - RotorWakeNormal[1] * dmag_dRotorWakeNormal[1] / (mag * mag); 
    
    pRotorWakeNormal2_pRotorNormal[0] = pRotorWakeNormal2_pRotorNormal[0]/mag - RotorWakeNormal[2] * dmag_dRotorWakeNormal[2] / (mag * mag);
    pRotorWakeNormal2_pRotorNormal[1] = pRotorWakeNormal2_pRotorNormal[1]/mag - RotorWakeNormal[2] * dmag_dRotorWakeNormal[2] / (mag * mag); 
    pRotorWakeNormal2_pRotorNormal[2] = pRotorWakeNormal2_pRotorNormal[2]/mag - RotorWakeNormal[2] * dmag_dRotorWakeNormal[2] / (mag * mag); 
    
    pRotorWakeNormal0_pVinf[0] = pRotorWakeNormal0_pVinf[0]/mag;
    pRotorWakeNormal0_pVinf[1] = pRotorWakeNormal0_pVinf[1]/mag;
    pRotorWakeNormal0_pVinf[2] = pRotorWakeNormal0_pVinf[2]/mag;

    pRotorWakeNormal1_pVinf[0] = pRotorWakeNormal1_pVinf[0]/mag;
    pRotorWakeNormal1_pVinf[1] = pRotorWakeNormal1_pVinf[1]/mag;
    pRotorWakeNormal1_pVinf[2] = pRotorWakeNormal1_pVinf[2]/mag;
    
    pRotorWakeNormal2_pVinf[0] = pRotorWakeNormal2_pVinf[0]/mag;
    pRotorWakeNormal2_pVinf[1] = pRotorWakeNormal2_pVinf[1]/mag;
    pRotorWakeNormal2_pVinf[2] = pRotorWakeNormal2_pVinf[2]/mag;
                    
    // Local coordinate system wrt rotor

    vec[0] = xyz_p[0] - RotorXYZ_[0];
    vec[1] = xyz_p[1] - RotorXYZ_[1];
    vec[2] = xyz_p[2] - RotorXYZ_[2];
    
    pvec0_pxyz_p[0] = 1.;
    pvec0_pxyz_p[1] = 0.;
    pvec0_pxyz_p[2] = 0.;

    pvec1_pxyz_p[0] = 0.;
    pvec1_pxyz_p[1] = 1.;
    pvec1_pxyz_p[2] = 0.;

    pvec2_pxyz_p[0] = 0.;
    pvec2_pxyz_p[1] = 0.;
    pvec2_pxyz_p[2] = 1.;

    pvec0_pRotorXYZ[0] = -1.;
    pvec0_pRotorXYZ[1] =  0.;
    pvec0_pRotorXYZ[2] =  0.;

    pvec1_pRotorXYZ[0] =  0.;
    pvec1_pRotorXYZ[1] = -1.;
    pvec1_pRotorXYZ[2] =  0.;

    pvec2_pRotorXYZ[0] =  0.;
    pvec2_pRotorXYZ[1] =  0.;
    pvec2_pRotorXYZ[2] = -1.;
                
    // Axial distance
    
    z = vector_dot(vec,RotorNormal_);
    
    pz_pvec[0] = RotorNormal_[0];
    pz_pvec[1] = RotorNormal_[1];
    pz_pvec[2] = RotorNormal_[2];

    pz_pRotorNormal[0] = vec[0];
    pz_pRotorNormal[1] = vec[1];
    pz_pRotorNormal[2] = vec[2];
        
    zp = vector_dot(vec,RotorWakeNormal);
    
    pzp_pvec[0] = RotorWakeNormal[0];
    pzp_pvec[1] = RotorWakeNormal[1];
    pzp_pvec[2] = RotorWakeNormal[2];

    pzp_pRotorWakeNormal[0] = vec[0];
    pzp_pRotorWakeNormal[1] = vec[1];
    pzp_pRotorWakeNormal[2] = vec[2];
        
    // Radial distance, based on sheared rotor wake

    rvec[0] = vec[0] - zp*RotorWakeNormal[0]; 
    rvec[1] = vec[1] - zp*RotorWakeNormal[1]; 
    rvec[2] = vec[2] - zp*RotorWakeNormal[2]; 
        
    prvec0_pvec[0] = 1.;
    prvec0_pvec[1] = 0.;
    prvec0_pvec[2] = 0.;

    prvec1_pvec[0] = 0.;
    prvec1_pvec[1] = 1.;
    prvec1_pvec[2] = 0.;
    
    prvec2_pvec[0] = 0.;
    prvec2_pvec[1] = 0.;
    prvec2_pvec[2] = 1.;   
    
    prvec0_pz = RotorWakeNormal[0]; 
    prvec1_pz = RotorWakeNormal[1]; 
    prvec2_pz = RotorWakeNormal[2]; 

    prvec0_pRotorWakeNormal[0] = -zp;
    prvec0_pRotorWakeNormal[1] =  0.;
    prvec0_pRotorWakeNormal[2] =  0.;
           
    prvec1_pRotorWakeNormal[0] =  0.; 
    prvec1_pRotorWakeNormal[1] =  -zp;
    prvec1_pRotorWakeNormal[2] =  0.;
           
    prvec2_pRotorWakeNormal[0] =  0.; 
    prvec2_pRotorWakeNormal[1] =  0.;
    prvec2_pRotorWakeNormal[2] = -zp;
    
    r = sqrt(vector_dot(rvec,rvec));
    
    r = MAX(r,1.e-9);
    
    pr_pvec[0] = rvec[0] / r;
    pr_pvec[1] = rvec[1] / r;
    pr_pvec[2] = rvec[2] / r;
     
 // // Radial direction
 //
 // rvec[0] = vec[0] - z*RotorNormal_[0]; 
 // rvec[1] = vec[1] - z*RotorNormal_[1]; 
 // rvec[2] = vec[2] - z*RotorNormal_[2]; 
 // 
 // 
 // 
 // 
 // mag = sqrt(vector_dot(rvec,rvec));
 //
 // mag = MAX(mag,1.e-9);
 // 
 // pmag_prvec[0] = rvec[0]/mag;
 // pmag_prvec[1] = rvec[1]/mag;
 // pmag_prvec[2] = rvec[2]/mag;
 // 
 // rvec[0] /= mag;
 // rvec[1] /= mag;
 // rvec[2] /= mag;
 //
 //
 // prve

    // Angular velocity direction
    
    vector_cross(RotorNormal_, rvec, tvec);

    mag = sqrt(vector_dot(tvec,tvec));
    
    mag = MAX(mag,1.e-9);
    
    tvec[0] /= mag;
    tvec[1] /= mag;
    tvec[2] /= mag;  
        
    // Radial Velocity

    VxR0 = 0.;

    if ( r < RotorRadius_ ) VxR0 = Vh*sqrt(RotorRadius_*RotorRadius_ - r*r)/RotorRadius_;

    Fact = sqrt( pow(RotorRadius_*RotorRadius_ - r*r - z*z,2.) + pow(2.*RotorRadius_*z,2.) ) + RotorRadius_*RotorRadius_ - r*r - z*z; 
    
    alpha = 0.;
    
    if ( Fact >= 0. ) {
     
       alpha = sqrt( Fact/(2.*RotorRadius_*RotorRadius_) );
    
    }
  
    sinf = 2.*RotorRadius_ / (sqrt(z*z + pow(RotorRadius_ + r,2.)) + sqrt(z*z + pow(RotorRadius_-r,2.)));
    
    sinf = MIN(MAX(-1.,sinf),1.);
    
    f = asin(sinf);
   
    Term1 = 0.;
    
    if ( r > 0. && alpha > 0. ) Term1 = ABS(z)*(1./alpha - alpha)/(2.*r);

    Term2 = r*f/(2.*RotorRadius_);
    
    Velocity_R = Vh * ( Term1 - Term2 );
    
    if ( z == 0. && r > RotorRadius_ ) {
       
       Velocity_R = 0.5 * Vh * ( sqrt(1.-pow(RotorRadius_/r,2.)) - r/RotorRadius_*asin(RotorRadius_/r) );
       
    }       

    // Axial velocity
   
    if ( z >= 0. ) {
     
       Velocity_X = 2.*VxR0 + Vh*( -alpha + z*f/RotorRadius_);
                     
    }
    
    else {
    
       Velocity_X = Vh*( alpha + z*f/RotorRadius_ );
       
    }

    // Angular velocity
    
    Omega = ABS(RotorRPM_) * 2. * PI / 60.;
      
    // Page 43 in Johnson's book:
    
    CT_h = RotorThrust() / ( Density_ * RotorArea() * pow(Omega*RotorRadius_,2.) );
    
    CP_h = RotorPower() / ( Density_ * RotorArea() * pow(Omega*RotorRadius_,3.) );

    Vo = Vh/sqrt(1. + CT_h * log(0.5*CT_h) + 0.5*CT_h); 
    
    // Estimate local airfoil characteristics
    
    Sigma_Cl = 6. * CT_h;
    
    Sigma_Cd = 8.*( CP_h - 1.17 * pow(CT_h,1.5)/sqrt(2.));
 
    // Calculate the tangential velocity
    
    Velocity_T = 0.;
   
    if ( r <= RotorRadius_ && z >= 0. ) {
     
       Velocity_T = 2. * ( VinfMag_ + Vo ) * Vo * Omega * r / ( pow(Omega*r,2.) + pow(VinfMag_+Vo,2.) );
       
       Velocity_T += 2. * Sigma_Cd / Sigma_Cl * Vo; // Page 45 in Johnson's book
       
       Velocity_T *= SGN(RotorRPM_);
       
    }
    
    // Delta-Cp
    
    Delta_Cp = 0.;

    if ( z >= 0. && r <= RotorRadius_ ) Delta_Cp = 2. * Density_ * ( VinfMag_ + VxR0 ) * VxR0;

    Delta_Cp /= (0.5*Density_*VinfMag_*VinfMag_);
    
    // Correct for propeller efficiency
    
    eta_mom = 2./(1. + sqrt(1. + Rotor_CT_));
    
    eta_prop = Rotor_JRatio() * Rotor_CT_ / Rotor_CP_;
    
    Delta_Cp *= eta_prop / eta_mom;

    // Convert to xyz coordinates

    q[0] = Velocity_X*RotorNormal_[0] + Velocity_R * rvec[0] + Velocity_T * tvec[0];
    q[1] = Velocity_X*RotorNormal_[1] + Velocity_R * rvec[1] + Velocity_T * tvec[1];
    q[2] = Velocity_X*RotorNormal_[2] + Velocity_R * rvec[2] + Velocity_T * tvec[2];    
    q[3] = Delta_Cp;
    q[4] = 0.;
//    if ( z >= 0. && r <= RotorRadius_ ) q[4] = Vh;

}

/*##############################################################################
#                                                                              #
#                              ROTOR_DISK VelX                                 #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::CalculateRotorGeometry(void)
{
 
    int i;
    double DTheta, Theta, s[3], t[3], r[3], mag, XYZo[3], XYZp[3];
    QUAT Quat, InvQuat, Vec1, Vec2, Vec3;
    
    // Random vector
    
    t[0] = 1.;
    t[1] = 2.;
    t[2] = 3.;
    
    // Cross into rotor normal to get a radius vector
    
    vector_cross(RotorNormal_, t, r);
    
    mag = sqrt(vector_dot(r,r));
    
    r[0] /= mag;
    r[1] /= mag;
    r[2] /= mag;
    
    // Scale it by the rotor radius
    
    r[0] *= RotorRadius_;
    r[1] *= RotorRadius_;
    r[2] *= RotorRadius_;
    
    // Now rotate this vector around the center of the rotor

    DTheta = 2.*PI / ( NUM_ROTOR_NODES - 1 );
    
    for ( i = 1 ; i <= NUM_ROTOR_NODES ; i++ ) {

       // Build quaternion about this hinge vector
     
       Theta = (i-1)*DTheta;
       
       Quat.FormRotationQuat(RotorNormal_,Theta);
 
       InvQuat = Quat;

       InvQuat.FormInverse();

       // Rotate about the normal vector

       Vec1(0) = r[0];
       Vec1(1) = r[1];
       Vec1(2) = r[2];

       Vec1 = Quat * Vec1 * InvQuat;
       
       // Shift by center of rotor
       
       RotorRadiusXYZ_[i][0] = Vec1(0) + RotorXYZ_[0];
       RotorRadiusXYZ_[i][1] = Vec1(1) + RotorXYZ_[1];
       RotorRadiusXYZ_[i][2] = Vec1(2) + RotorXYZ_[2];
       
    }

    // Vector along direction of rotor travel
    
    if ( RotorRPM_ >= 0. ) {
     
       s[0] = RotorRadiusXYZ_[2][0] - RotorRadiusXYZ_[1][0];
       s[1] = RotorRadiusXYZ_[2][1] - RotorRadiusXYZ_[1][1];
       s[2] = RotorRadiusXYZ_[2][2] - RotorRadiusXYZ_[1][2];
       
       XYZo[0] = RotorRadiusXYZ_[1][0];
       XYZo[1] = RotorRadiusXYZ_[1][1];
       XYZo[2] = RotorRadiusXYZ_[1][2];

       XYZp[0] = RotorRadiusXYZ_[2][0];
       XYZp[1] = RotorRadiusXYZ_[2][1];
       XYZp[2] = RotorRadiusXYZ_[2][2];
       
    }
    
    else {
     
       s[0] = RotorRadiusXYZ_[1][0] - RotorRadiusXYZ_[2][0];
       s[1] = RotorRadiusXYZ_[1][1] - RotorRadiusXYZ_[2][1];
       s[2] = RotorRadiusXYZ_[1][2] - RotorRadiusXYZ_[2][2];
       
       XYZo[0] = RotorRadiusXYZ_[2][0];
       XYZo[1] = RotorRadiusXYZ_[2][1];
       XYZo[2] = RotorRadiusXYZ_[2][2];       

       XYZp[0] = RotorRadiusXYZ_[1][0];
       XYZp[1] = RotorRadiusXYZ_[1][1];
       XYZp[2] = RotorRadiusXYZ_[1][2];
       
    }

    // Random vector
    
    t[0] = 1.;
    t[1] = 2.;
    t[2] = 3.;
    
    // Cross into rotor normal to get a radius vector
    
    vector_cross(RotorNormal_, t, r);

    mag = sqrt(vector_dot(r,r));
    
    r[0] /= mag;
    r[1] /= mag;
    r[2] /= mag;
    
    // Scale it by the rotor radius
    
    r[0] *= 0.050*RotorRadius_;
    r[1] *= 0.050*RotorRadius_;
    r[2] *= 0.050*RotorRadius_;
    
    DTheta = 2.*PI / ( 3 );
    
    for ( i = 1 ; i <= 3 ; i++ ) {

       // Build quaternion about this hinge vector
     
       Theta = (i-1)*DTheta;
       
       Quat.FormRotationQuat(s,Theta);
 
       InvQuat = Quat;

       InvQuat.FormInverse();

       // Rotate about the normal vector

       Vec1(0) = r[0];
       Vec1(1) = r[1];
       Vec1(2) = r[2];

       Vec1 = Quat * Vec1 * InvQuat;
       
       // Shift by center of rotor
       
       RotorDirectionVectorXYZ_[i][0] = Vec1(0) + XYZo[0];
       RotorDirectionVectorXYZ_[i][1] = Vec1(1) + XYZo[1];
       RotorDirectionVectorXYZ_[i][2] = Vec1(2) + XYZo[2];
       
    }
 
    RotorDirectionVectorXYZ_[4][0] = XYZp[0];
    RotorDirectionVectorXYZ_[4][1] = XYZp[1];
    RotorDirectionVectorXYZ_[4][2] = XYZp[2];
    
    // Calculate normals for each side of the arrow head
    
    // Tri 1
    
       s[0] = RotorDirectionVectorXYZ_[2][0] - RotorDirectionVectorXYZ_[1][0];
       s[1] = RotorDirectionVectorXYZ_[2][1] - RotorDirectionVectorXYZ_[1][1];
       s[2] = RotorDirectionVectorXYZ_[2][2] - RotorDirectionVectorXYZ_[1][2];
       
       t[0] = RotorDirectionVectorXYZ_[3][0] - RotorDirectionVectorXYZ_[1][0];
       t[1] = RotorDirectionVectorXYZ_[3][1] - RotorDirectionVectorXYZ_[1][1];
       t[2] = RotorDirectionVectorXYZ_[3][2] - RotorDirectionVectorXYZ_[1][2];
       
       vector_cross(s,t,r);
       
       mag = sqrt(vector_dot(r,r));
    
       r[0] /= mag;
       r[1] /= mag;
       r[2] /= mag;
       
       RotorDirectionVectorNormal_[1][0] = -r[0];
       RotorDirectionVectorNormal_[1][1] = -r[1];
       RotorDirectionVectorNormal_[1][2] = -r[2];
       
    // Tri 2
    
       s[0] = RotorDirectionVectorXYZ_[4][0] - RotorDirectionVectorXYZ_[2][0];
       s[1] = RotorDirectionVectorXYZ_[4][1] - RotorDirectionVectorXYZ_[2][1];
       s[2] = RotorDirectionVectorXYZ_[4][2] - RotorDirectionVectorXYZ_[2][2];
       
       t[0] = RotorDirectionVectorXYZ_[3][0] - RotorDirectionVectorXYZ_[2][0];
       t[1] = RotorDirectionVectorXYZ_[3][1] - RotorDirectionVectorXYZ_[2][1];
       t[2] = RotorDirectionVectorXYZ_[3][2] - RotorDirectionVectorXYZ_[2][2];
       
       vector_cross(s,t,r);
       
       mag = sqrt(vector_dot(r,r));
    
       r[0] /= mag;
       r[1] /= mag;
       r[2] /= mag;
       
       RotorDirectionVectorNormal_[2][0] = -r[0];
       RotorDirectionVectorNormal_[2][1] = -r[1];
       RotorDirectionVectorNormal_[2][2] = -r[2];       
 
    // Tri 3
    
       s[0] = RotorDirectionVectorXYZ_[4][0] - RotorDirectionVectorXYZ_[1][0];
       s[1] = RotorDirectionVectorXYZ_[4][1] - RotorDirectionVectorXYZ_[1][1];
       s[2] = RotorDirectionVectorXYZ_[4][2] - RotorDirectionVectorXYZ_[1][2];
       
       t[0] = RotorDirectionVectorXYZ_[2][0] - RotorDirectionVectorXYZ_[1][0];
       t[1] = RotorDirectionVectorXYZ_[2][1] - RotorDirectionVectorXYZ_[1][1];
       t[2] = RotorDirectionVectorXYZ_[2][2] - RotorDirectionVectorXYZ_[1][2];
       
       vector_cross(s,t,r);
       
       mag = sqrt(vector_dot(r,r));
    
       r[0] /= mag;
       r[1] /= mag;
       r[2] /= mag;
       
       RotorDirectionVectorNormal_[3][0] = -r[0];
       RotorDirectionVectorNormal_[3][1] = -r[1];
       RotorDirectionVectorNormal_[3][2] = -r[2];              

    // Tri 4
    
       s[0] = RotorDirectionVectorXYZ_[3][0] - RotorDirectionVectorXYZ_[1][0];
       s[1] = RotorDirectionVectorXYZ_[3][1] - RotorDirectionVectorXYZ_[1][1];
       s[2] = RotorDirectionVectorXYZ_[3][2] - RotorDirectionVectorXYZ_[1][2];
       
       t[0] = RotorDirectionVectorXYZ_[4][0] - RotorDirectionVectorXYZ_[1][0];
       t[1] = RotorDirectionVectorXYZ_[4][1] - RotorDirectionVectorXYZ_[1][1];
       t[2] = RotorDirectionVectorXYZ_[4][2] - RotorDirectionVectorXYZ_[1][2];
       
       vector_cross(s,t,r);
       
       mag = sqrt(vector_dot(r,r));
    
       r[0] /= mag;
       r[1] /= mag;
       r[2] /= mag;
       
       RotorDirectionVectorNormal_[4][0] = -r[0];
       RotorDirectionVectorNormal_[4][1] = -r[1];
       RotorDirectionVectorNormal_[4][2] = -r[2];    
       
}
 
/*##############################################################################
#                                                                              #
#                              ROTOR_DISK Write_STP_Data                       #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::Write_STP_Data(FILE *OutputFile)
{

    // Write out STP file data

    fprintf(OutputFile,"%lf %lf %lf \n",RotorXYZ_[0], RotorXYZ_[1], RotorXYZ_[2]);
    
    fprintf(OutputFile,"%lf %lf %lf \n",RotorNormal_[0], RotorNormal_[1], RotorNormal_[2]);
    
    fprintf(OutputFile,"%lf \n",RotorRadius_);

    fprintf(OutputFile,"%lf \n",RotorHubRadius_);
    
    fprintf(OutputFile,"%lf \n",RotorRPM_);
  
    fprintf(OutputFile,"%lf \n",Rotor_CT_);
    
    fprintf(OutputFile,"%lf \n",Rotor_CP_);
   
}

/*##############################################################################
#                                                                              #
#                    ROTOR_DISK Write_Binary_STP_Data                          #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::Write_Binary_STP_Data(FILE *OutputFile)
{
 
    int i_size, c_size, d_size;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    d_size = sizeof(double);
    
    // Write out STP file data

    fwrite(&(RotorXYZ_[0]), d_size, 1, OutputFile); 
    fwrite(&(RotorXYZ_[1]), d_size, 1, OutputFile); 
    fwrite(&(RotorXYZ_[2]), d_size, 1, OutputFile); 
    
    fwrite(&(RotorNormal_[0]), d_size, 1, OutputFile); 
    fwrite(&(RotorNormal_[1]), d_size, 1, OutputFile); 
    fwrite(&(RotorNormal_[2]), d_size, 1, OutputFile); 
    
    fwrite(&(RotorRadius_), d_size, 1, OutputFile); 
    
    fwrite(&(RotorHubRadius_), d_size, 1, OutputFile); 
     
    fwrite(&(RotorRPM_), d_size, 1, OutputFile); 
      
    fwrite(&(Rotor_CT_), d_size, 1, OutputFile); 
    
    fwrite(&(Rotor_CP_), d_size, 1, OutputFile); 

}

/*##############################################################################
#                                                                              #
#                     ROTOR_DISK Load_STP_Data                                 #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::Load_STP_Data(FILE *InputFile)
{

    // Load in STP file data

    fscanf(InputFile,"%lf %lf %lf \n",&(RotorXYZ_[0]), &(RotorXYZ_[1]), &(RotorXYZ_[2]));
    
    fscanf(InputFile,"%lf %lf %lf \n",&(RotorNormal_[0]), &(RotorNormal_[1]), &(RotorNormal_[2]));
    
    fscanf(InputFile,"%lf \n",&RotorRadius_);

    fscanf(InputFile,"%lf \n",&RotorHubRadius_);
    
    fscanf(InputFile,"%lf \n",&RotorRPM_);
    
    fscanf(InputFile,"%lf \n",&Rotor_CT_);
    
    fscanf(InputFile,"%lf \n",&Rotor_CP_);
    
    // Echo inputs

    printf("RotorXYZ:       %10.5lf %10.5lf %10.5lf \n",RotorXYZ_[0], RotorXYZ_[1], RotorXYZ_[2]);
    
    printf("RotorNormal:    %10.5lf %10.5lf %10.5lf \n",RotorNormal_[0], RotorNormal_[1], RotorNormal_[2]);
    
    printf("RotorRadius:    %10.5f \n",RotorRadius_);
    
    printf("RotorHubRadius: %10.5lf \n",RotorHubRadius_);
    
    printf("RotorRPM:       %10.5lf \n",RotorRPM_);
    
    printf("Rotor_CT:       %10.5lf \n",Rotor_CT_);
    
    printf("Rotor_CP:       %10.5lf \n",Rotor_CP_);

      
}

/*##############################################################################
#                                                                              #
#                    ROTOR_DISK Read_Binary_STP_Data                           #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::Read_Binary_STP_Data(FILE *InputFile)
{
 
    int i_size, c_size, d_size;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    d_size = sizeof(double);
    
    // Write out STP file data

    fread(&(RotorXYZ_[0]), d_size, 1, InputFile); 
    fread(&(RotorXYZ_[1]), d_size, 1, InputFile); 
    fread(&(RotorXYZ_[2]), d_size, 1, InputFile); 
    
    fread(&(RotorNormal_[0]), d_size, 1, InputFile); 
    fread(&(RotorNormal_[1]), d_size, 1, InputFile); 
    fread(&(RotorNormal_[2]), d_size, 1, InputFile); 
    
    fread(&(RotorRadius_), d_size, 1, InputFile); 

    fread(&(RotorHubRadius_), d_size, 1, InputFile); 
     
    fread(&(RotorRPM_), d_size, 1, InputFile); 
      
    fread(&(Rotor_CT_), d_size, 1, InputFile); 
    
    fread(&(Rotor_CP_), d_size, 1, InputFile); 

}

/*##############################################################################
#                                                                              #
#                    ROTOR_DISK Skip_Read_Binary_STP_Data                      #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::Skip_Read_Binary_STP_Data(FILE *InputFile)
{
 
    int i_size, c_size, d_size;
    
    double DumDouble;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    d_size = sizeof(double);
    
    // Write out STP file data

    fread(&DumDouble, d_size, 1, InputFile); 
    fread(&DumDouble, d_size, 1, InputFile); 
    fread(&DumDouble, d_size, 1, InputFile); 
    
    fread(&DumDouble, d_size, 1, InputFile); 
    fread(&DumDouble, d_size, 1, InputFile); 
    fread(&DumDouble, d_size, 1, InputFile); 
    
    fread(&DumDouble, d_size, 1, InputFile); 

    fread(&DumDouble, d_size, 1, InputFile); 
     
    fread(&DumDouble, d_size, 1, InputFile); 
      
    fread(&DumDouble, d_size, 1, InputFile); 
    
    fread(&DumDouble, d_size, 1, InputFile); 

}

/*##############################################################################
#                                                                              #
#                     ROTOR_DISK UpdateGeometryLocation                        #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::UpdateGeometryLocation(double *TVec, double *OVec, QUAT &Quat, QUAT &InvQuat)
{

    QUAT Vec;
    
    // Update location
    
    Vec(0) = RotorXYZ_[0] - OVec[0];
    Vec(1) = RotorXYZ_[1] - OVec[1];
    Vec(2) = RotorXYZ_[2] - OVec[2];

    Vec = Quat * Vec * InvQuat;

    RotorXYZ_[0] = Vec(0) + OVec[0] + TVec[0];
    RotorXYZ_[1] = Vec(1) + OVec[1] + TVec[1];
    RotorXYZ_[2] = Vec(2) + OVec[2] + TVec[2];    
    
    // Update normal

    Vec(0) = RotorNormal_[0];
    Vec(1) = RotorNormal_[1];
    Vec(2) = RotorNormal_[2];
    
    Vec = Quat * Vec * InvQuat;

    RotorNormal_[0] = Vec(0);
    RotorNormal_[1] = Vec(1);
    RotorNormal_[2] = Vec(2);    

}

#include "END_NAME_SPACE.H"


