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

void ROTOR_DISK::VelocityPotential(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[5])
{

    VSPAERO_DOUBLE Vh, z, r, vec[3], rvec[3], tvec[3], mag;
    VSPAERO_DOUBLE Velocity_X, Velocity_R, Velocity_T, Omega, PhiTotal;
    VSPAERO_DOUBLE Phi_D, Phi_D_z;
 
    vec[0] = xyz[0] - RotorXYZ_[0];
    vec[1] = xyz[1] - RotorXYZ_[1];
    vec[2] = xyz[2] - RotorXYZ_[2];
    
    // Axial distance
    
    z = vector_dot(vec,RotorNormal_);
    
    rvec[0] = xyz[0] - z*RotorNormal_[0]; 
    rvec[1] = xyz[1] - z*RotorNormal_[1]; 
    rvec[2] = xyz[2] - z*RotorNormal_[2]; 

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
    
 //   PRINTF("Velocity_X: %lf \n",Velocity_X);
 //   PRINTF("Velocity_R: %lf \n",Velocity_R);
  //  PRINTF("Velocity_T: %lf \n",Velocity_T);
    
    // Convert to xyz coordinates
    
    q[0] = Velocity_X*RotorNormal_[0] + Velocity_R * rvec[0] + Velocity_T * tvec[0];
    q[1] = Velocity_X*RotorNormal_[1] + Velocity_R * rvec[1] + Velocity_T * tvec[1];
    q[2] = Velocity_X*RotorNormal_[2] + Velocity_R * rvec[2] + Velocity_T * tvec[2];
    
    q[3] = PhiTotal;
    
  //  PRINTF("z: %lf ... Velocity_T * tvec[1]: %lf \n", xyz[2], Velocity_T * tvec[1]);

}

/*##############################################################################
#                                                                              #
#                              ROTOR_DISK Velocity                             #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::Velocity(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[5])
{

    VSPAERO_DOUBLE z, r, zp;
    VSPAERO_DOUBLE Term1, Term2, Vh, alpha, sinf, f, vec[3], rvec[3], tvec[3], mag;
    VSPAERO_DOUBLE Velocity_X, Velocity_R, Velocity_T, Omega, VxR0, Delta_Cp, Fact;
    VSPAERO_DOUBLE eta_mom, eta_prop, CT_h, CP_h, Sigma_Cd, Sigma_Cl, Vo, TotalVinfMag;
    VSPAERO_DOUBLE RotorWakeNormal[3];
    
    // Local free stream velocity normal to rotor
            
    VinfMag_ = vector_dot(Vinf_,RotorNormal_);
    
    TotalVinfMag = sqrt(vector_dot(Vinf_,Vinf_));
    
    // Rotor down wash
    
    Vh = -0.5*VinfMag_ + sqrt( pow(0.5*VinfMag_,2.) + RotorThrust()/(2.*Density_*RotorArea()) );
    
    // Calculate approximate direction of rotor down wash 
    // ... just a vector sum of the rotor wash + Vinf
    
    RotorWakeNormal[0] = Vh * RotorNormal_[0] + Vinf_[0];
    RotorWakeNormal[1] = Vh * RotorNormal_[1] + Vinf_[1];
    RotorWakeNormal[2] = Vh * RotorNormal_[2] + Vinf_[2];
    
    mag = sqrt(vector_dot(RotorWakeNormal,RotorWakeNormal));
    
    RotorWakeNormal[0] /= mag;
    RotorWakeNormal[1] /= mag;
    RotorWakeNormal[2] /= mag;

    // Local coordinate system wrt rotor

    vec[0] = xyz[0] - RotorXYZ_[0];
    vec[1] = xyz[1] - RotorXYZ_[1];
    vec[2] = xyz[2] - RotorXYZ_[2];
    
    // Axial distance
    
    z = vector_dot(vec,RotorNormal_);
    
    zp = vector_dot(vec,RotorWakeNormal);

    // Radial distance, based on sheared rotor wake

    rvec[0] = vec[0] - z*RotorWakeNormal[0]; 
    rvec[1] = vec[1] - z*RotorWakeNormal[1]; 
    rvec[2] = vec[2] - z*RotorWakeNormal[2]; 
    
    r = sqrt(vector_dot(rvec,rvec));

    r = MAX(r,1.e-9);
     
    // Radial direction

    rvec[0] = vec[0] - z*RotorNormal_[0]; 
    rvec[1] = vec[1] - z*RotorNormal_[1]; 
    rvec[2] = vec[2] - z*RotorNormal_[2]; 
    
    mag = sqrt(vector_dot(rvec,rvec));
    
    mag = MAX(mag,1.e-9);
    
    rvec[0] /= mag;
    rvec[1] /= mag;
    rvec[2] /= mag;

    // Angular velocity direction
    
    vector_cross(RotorNormal_, rvec, tvec);

    mag = sqrt(vector_dot(tvec,tvec));
    
    mag = MAX(mag,1.e-9);
    
    tvec[0] /= mag;
    tvec[1] /= mag;
    tvec[2] /= mag;  
        
    // Radial Velocity
    
 //   Vh = sqrt(RotorThrust()/(2.*Density_*RotorArea()));
    
    Vh = -0.5*VinfMag_ + sqrt( pow(0.5*VinfMag_,2.) + RotorThrust()/(2.*Density_*RotorArea()) );

// PRINTF("Vh: %lf ... Vh/VinfMag_: %lf  ...Thrust: %lf \n",Vh,Vh/VinfMag_,RotorThrust());
    
//    PRINTF("RotorThrust(): %lf \n",RotorThrust());
  //  PRINTF("Density: %lf \n",Density_);
   // PRINTF("RotorArea(): %lf \n",RotorArea());
    
  //  PRINTF("Vh: %lf \n",Vh);
  
    VxR0 = 0.;

    if ( r < RotorRadius_ ) VxR0 = Vh*sqrt(RotorRadius_*RotorRadius_ - r*r)/RotorRadius_;
    
  //  VxR0 *= sqrt(r/RotorRadius_);
    
    Fact = sqrt( pow(RotorRadius_*RotorRadius_ - r*r - z*z,2.) + pow(2.*RotorRadius_*z,2.) ) + RotorRadius_*RotorRadius_ - r*r - z*z; 
    
    alpha = 0.;
    
    if ( Fact >= 0. ) {
     
       alpha = sqrt( Fact/(2.*RotorRadius_*RotorRadius_) );
    
    }
                  
//    alpha = sqrt( ( sqrt( pow(RotorRadius_*RotorRadius_ - r*r - z*z,2.) + pow(2.*RotorRadius_*z,2.) ) 
//                  + RotorRadius_*RotorRadius_ - r*r - z*z 
//                  )/(2.*RotorRadius_*RotorRadius_) );
    
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
     
       Velocity_X  = 2.*VxR0 + Vh*( -alpha + z*f/RotorRadius_);
                     
    }
    
    else {
    
       Velocity_X = Vh*( alpha + z*f/RotorRadius_ );
       
    }

    // Angular velocity
    
    Omega = ABS(RotorRPM_) * 2. * PI / 60.;
 
    Velocity_T = 0.;
     
    // Page 43 in Johnson's book:
    
    CT_h = RotorThrust() / ( Density_ * RotorArea() * pow(Omega*RotorRadius_,2.) );
    
    CP_h = RotorPower() / ( Density_ * RotorArea() * pow(Omega*RotorRadius_,3.) );

    Vo = Vh/sqrt(1. + CT_h * log(0.5*CT_h) + 0.5*CT_h); 
    
    // Estimate local airfoil characteristics
    
    Sigma_Cl = 6. * CT_h;
    
    Sigma_Cd = 8.*( CP_h - 1.17 * pow(CT_h,1.5)/sqrt(2.));
    
    if ( r <= RotorRadius_ && z >= 0. ) {
     
       Velocity_T = 2. * ( VinfMag_ + Vo ) * Vo * Omega * r / ( pow(Omega*r,2.) + pow(VinfMag_+Vo,2.) );
       
       Velocity_T += 2. * Sigma_Cd / Sigma_Cl * Vo; // Page 45
       
       Velocity_T *= SGN(RotorRPM_);
       
    }
    
    // Delta-Cp
    
    Delta_Cp = 0.;

    if ( z >= 0. && r <= RotorRadius_ ) Delta_Cp = 2. * Density_ * ( VinfMag_ + VxR0 ) * VxR0;
    
    // Johnson
    
  //  if ( r <= RotorRadius_ ) Delta_Cp = 2. * Density_ * ( VinfMag_ + Vo) * Vo * pow(Omega * r,4.) / pow( pow(Omega*r,2.) + pow(VinfMag_+Vo,2.),2. );

    Delta_Cp /= (0.5*Density_*VinfMag_*VinfMag_);
    
    // Correct for propeller efficiency
    
    eta_mom = 2./(1. + sqrt(1. + Rotor_CT_));
    
    eta_prop = Rotor_JRatio() * Rotor_CT_ / Rotor_CP_;
    
    Delta_Cp *= eta_prop / eta_mom;

    // Convert to xyz coordinates
    
//    Velocity_X *= Omega * r / ( pow(Omega*r,2.) + pow(VinfMag_+Vh,2.) );
    
//    PRINTF("z, r/Ra, Vx/(2.*Vh): %lf %lf %lf \n",z, r/RotorRadius_,Velocity_X/(2.*Vh));

//if ( r <= RotorHubRadius_ ) Velocity_X = 0.;
 
    q[0] = Velocity_X*RotorNormal_[0] + Velocity_R * rvec[0] + Velocity_T * tvec[0];
    q[1] = Velocity_X*RotorNormal_[1] + Velocity_R * rvec[1] + Velocity_T * tvec[1];
    q[2] = Velocity_X*RotorNormal_[2] + Velocity_R * rvec[2] + Velocity_T * tvec[2];    
    q[3] = Delta_Cp;
    q[4] = 0.;
    if ( z >= 0. && r <= RotorRadius_ ) q[4] = Vh;

/*
    Vh = -0.5*VinfMag_ + sqrt( pow(0.5*VinfMag_,2.) + RotorThrust()/(2.*Density_*RotorArea()) );

PRINTF("RotorThrust: %lf \n",RotorThrust());
PRINTF("RotorPower: %lf \n",RotorPower()/550.);
PRINTF("RotorRPM_: %lf \n",RotorRPM_);
PRINTF("Rotor_CP_: %lf \n",Rotor_CP_);
PRINTF("Rotor_CT_: %lf \n",Rotor_CT_);
PRINTF("RotorArea(): %lf \n",RotorArea());
PRINTF("Density_: %lf \n",Density_);
PRINTF("VinfMag_: %lf \n",VinfMag_);
PRINTF("Vh: %lf \n",Vh);
*/

    if ( r <= RotorHubRadius_ ) q[0] = q[1] = q[2] = q[3] = q[4] = 0.; 
    
//    PRINTF("x,r,t: %lf %lf %lf \n",Velocity_X, Velocity_R, Velocity_T);

  //  PRINTF("z: %lf ... Velocity_T * tvec[1]: %lf \n", xyz[2], Velocity_T * tvec[1]);
  

}

/*##############################################################################
#                                                                              #
#                              ROTOR_DISK VelX                                 #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::CalculateRotorGeometry(void)
{
 
    int i;
    VSPAERO_DOUBLE DTheta, Theta, s[3], t[3], r[3], mag, XYZo[3], XYZp[3];
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

void ROTOR_DISK::Write_STP_Data(FILE *InputFile)
{

    // Write out STP file data

    FPRINTF(InputFile,"%lf %lf %lf \n",RotorXYZ_[0], RotorXYZ_[1], RotorXYZ_[2]);
    
    FPRINTF(InputFile,"%lf %lf %lf \n",RotorNormal_[0], RotorNormal_[1], RotorNormal_[2]);
    
    FPRINTF(InputFile,"%lf \n",RotorRadius_);

    FPRINTF(InputFile,"%lf \n",RotorHubRadius_);
    
    FPRINTF(InputFile,"%lf \n",RotorRPM_);
  
    FPRINTF(InputFile,"%lf \n",Rotor_CT_);
    
    FPRINTF(InputFile,"%lf \n",Rotor_CP_);
   
}

/*##############################################################################
#                                                                              #
#                    ROTOR_DISK Write_Binary_STP_Data                          #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::Write_Binary_STP_Data(FILE *InputFile)
{
 
    int i_size, c_size, d_size;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    d_size = sizeof(double);
    
    // Write out STP file data

    FWRITE(&(RotorXYZ_[0]), d_size, 1, InputFile); 
    FWRITE(&(RotorXYZ_[1]), d_size, 1, InputFile); 
    FWRITE(&(RotorXYZ_[2]), d_size, 1, InputFile); 
    
    FWRITE(&(RotorNormal_[0]), d_size, 1, InputFile); 
    FWRITE(&(RotorNormal_[1]), d_size, 1, InputFile); 
    FWRITE(&(RotorNormal_[2]), d_size, 1, InputFile); 
    
    FWRITE(&(RotorRadius_), d_size, 1, InputFile); 
    
    FWRITE(&(RotorHubRadius_), d_size, 1, InputFile); 
     
    FWRITE(&(RotorRPM_), d_size, 1, InputFile); 
      
    FWRITE(&(Rotor_CT_), d_size, 1, InputFile); 
    
    FWRITE(&(Rotor_CP_), d_size, 1, InputFile); 

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

    PRINTF("RotorXYZ:       %10.5lf %10.5lf %10.5lf \n",RotorXYZ_[0], RotorXYZ_[1], RotorXYZ_[2]);
    
    PRINTF("RotorNormal:    %10.5lf %10.5lf %10.5lf \n",RotorNormal_[0], RotorNormal_[1], RotorNormal_[2]);
    
    PRINTF("RotorRadius:    %10.5f \n",RotorRadius_);
    
    PRINTF("RotorHubRadius: %10.5lf \n",RotorHubRadius_);
    
    PRINTF("RotorRPM:       %10.5lf \n",RotorRPM_);
    
    PRINTF("Rotor_CT:       %10.5lf \n",Rotor_CT_);
    
    PRINTF("Rotor_CP:       %10.5lf \n",Rotor_CP_);

      
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

    FREAD(&(RotorXYZ_[0]), d_size, 1, InputFile); 
    FREAD(&(RotorXYZ_[1]), d_size, 1, InputFile); 
    FREAD(&(RotorXYZ_[2]), d_size, 1, InputFile); 
    
    FREAD(&(RotorNormal_[0]), d_size, 1, InputFile); 
    FREAD(&(RotorNormal_[1]), d_size, 1, InputFile); 
    FREAD(&(RotorNormal_[2]), d_size, 1, InputFile); 
    
    FREAD(&(RotorRadius_), d_size, 1, InputFile); 

    FREAD(&(RotorHubRadius_), d_size, 1, InputFile); 
     
    FREAD(&(RotorRPM_), d_size, 1, InputFile); 
      
    FREAD(&(Rotor_CT_), d_size, 1, InputFile); 
    
    FREAD(&(Rotor_CP_), d_size, 1, InputFile); 

}

/*##############################################################################
#                                                                              #
#                    ROTOR_DISK Skip_Read_Binary_STP_Data                      #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::Skip_Read_Binary_STP_Data(FILE *InputFile)
{
 
    int i_size, c_size, d_size;
    
    VSPAERO_DOUBLE DumDouble;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    d_size = sizeof(double);
    
    // Write out STP file data

    FREAD(&DumDouble, d_size, 1, InputFile); 
    FREAD(&DumDouble, d_size, 1, InputFile); 
    FREAD(&DumDouble, d_size, 1, InputFile); 
    
    FREAD(&DumDouble, d_size, 1, InputFile); 
    FREAD(&DumDouble, d_size, 1, InputFile); 
    FREAD(&DumDouble, d_size, 1, InputFile); 
    
    FREAD(&DumDouble, d_size, 1, InputFile); 

    FREAD(&DumDouble, d_size, 1, InputFile); 
     
    FREAD(&DumDouble, d_size, 1, InputFile); 
      
    FREAD(&DumDouble, d_size, 1, InputFile); 
    
    FREAD(&DumDouble, d_size, 1, InputFile); 

}

/*##############################################################################
#                                                                              #
#                     ROTOR_DISK UpdateGeometryLocation                        #
#                                                                              #
##############################################################################*/

void ROTOR_DISK::UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat)
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


