//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "EngineFace.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                              ENGINE_FACE constructor                         #
#                                                                              #
##############################################################################*/

ENGINE_FACE::ENGINE_FACE(void)
{

    NozzleXYZ_[0] = NozzleXYZ_[1] = NozzleXYZ_[2] = 0.;
    
    NozzleNormal_[0] = NozzleNormal_[1] = NozzleNormal_[2] = 0.;
    
    NozzleVelocity_ = 0.;
    
    NozzleDeltaCp_ = 0.;
    
    NozzleRadius_ = 1.;
    
    SurfaceType_ = 0;
    
    SurfaceID_ = 0;

    MassFlow_ = 0.;
    
    Density_ = 0.;
    
    Area_ = 0.;
      
}

/*##############################################################################
#                                                                              #
#                              ENGINE_FACE destructor                          #
#                                                                              #
##############################################################################*/

ENGINE_FACE::~ENGINE_FACE(void)
{

    NozzleXYZ_[0] = NozzleXYZ_[1] = NozzleXYZ_[2] = 0.;
    
    NozzleNormal_[0] = NozzleNormal_[1] = NozzleNormal_[2] = 0.;
    
    NozzleVelocity_ = 0.;
    
    NozzleDeltaCp_ = 0.;
    
    NozzleRadius_ = 1.;
    
    SurfaceType_ = 0;
    
    SurfaceID_ = 0;

    MassFlow_ = 0.;
    
    Density_ = 0.;
    
    Area_ = 0.;
    
}

/*##############################################################################
#                                                                              #
#                          ENGINE_FACE operator=                               #              
#                                                                              #
##############################################################################*/

ENGINE_FACE& ENGINE_FACE::operator=(const ENGINE_FACE &EngineFace)
{

    // Copy stuff over...
    
    NozzleXYZ_[0] = EngineFace.NozzleXYZ_[0];
    NozzleXYZ_[1] = EngineFace.NozzleXYZ_[1];
    NozzleXYZ_[2] = EngineFace.NozzleXYZ_[2];

    NozzleNormal_[0] = EngineFace.NozzleNormal_[0];
    NozzleNormal_[1] = EngineFace.NozzleNormal_[1];
    NozzleNormal_[2] = EngineFace.NozzleNormal_[2];
    
    NozzleVelocity_ = EngineFace.NozzleVelocity_;
    
    NozzleDeltaCp_ = EngineFace.NozzleDeltaCp_;
    
    NozzleRadius_ = EngineFace.NozzleRadius_;
    
    SurfaceID_ = EngineFace.SurfaceID_;

    SurfaceID_ = EngineFace.SurfaceID_;

    MassFlow_  = EngineFace.MassFlow_;
    Density_   = EngineFace.Density_;
    Area_      = EngineFace.Area_;
    
    return *this;
    
}

/*##############################################################################
#                                                                              #
#                          ENGINE_FACE operator=                               #              
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE ENGINE_FACE::NormalVelocity(void)
{

    if ( SurfaceType_ == INLET_FACE ) {
       
       return -MassFlow_ / ( Density_ * Area_ );
    
    }
    
    else {
       
       return NozzleVelocity_;
       
    }
   
}

/*##############################################################################
#                                                                              #
#                          ENGINE_FACE operator=                               #              
#                                                                              #
##############################################################################*/

void ENGINE_FACE::Velocity(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[5])
{

    VSPAERO_DOUBLE vec[3], Dot;

    q[0] = q[1] = q[2] = q[3] = q[4] = 0.;

    if ( SurfaceType_ == NOZZLE_FACE ) {

       // Vector from center of engine face
       
       vec[0] = xyz[0] - NozzleXYZ_[0];
       vec[1] = xyz[1] - NozzleXYZ_[1];
       vec[2] = xyz[2] - NozzleXYZ_[2];
       
       Dot = vector_dot(vec,NozzleNormal_);
       
       if ( Dot > 0. ) {
              
	  vec[0] -= Dot * NozzleNormal_[0];
	  vec[1] -= Dot * NozzleNormal_[1];
	  vec[2] -= Dot * NozzleNormal_[2];
   
	  Dot = sqrt(vector_dot(vec,vec));
	  
	  if ( Dot <= NozzleRadius_ ) {
	     
	     q[0] = NozzleVelocity_ * NozzleNormal_[0];
	     q[1] = NozzleVelocity_ * NozzleNormal_[1];
	     q[2] = NozzleVelocity_ * NozzleNormal_[2];
	     q[3] = NozzleDeltaCp_;
	     q[4] = NozzleVelocity_;
	     	     
	  }
	  	  
       }
	 
    }
    
}

/*##############################################################################
#                                                                              #
#                     ENGINE_FACE UpdateGeometryLocation                       #
#                                                                              #
##############################################################################*/

void ENGINE_FACE::UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat)
{

    QUAT Vec;
    
    // Update location
    
    Vec(0) = NozzleXYZ_[0] - OVec[0];
    Vec(1) = NozzleXYZ_[1] - OVec[1];
    Vec(2) = NozzleXYZ_[2] - OVec[2];

    Vec = Quat * Vec * InvQuat;

    NozzleXYZ_[0] = Vec(0) + OVec[0] + TVec[0];
    NozzleXYZ_[1] = Vec(1) + OVec[1] + TVec[1];
    NozzleXYZ_[2] = Vec(2) + OVec[2] + TVec[2];    
    
    // Update normal

    Vec(0) = NozzleNormal_[0];
    Vec(1) = NozzleNormal_[1];
    Vec(2) = NozzleNormal_[2];
    
    Vec = Quat * Vec * InvQuat;

    NozzleNormal_[0] = Vec(0);
    NozzleNormal_[1] = Vec(1);
    NozzleNormal_[2] = Vec(2);    

}

/*##############################################################################
#                                                                              #
#                    ENGINE_FACE Write_Binary_STP_Data                         #
#                                                                              #
##############################################################################*/

void ENGINE_FACE::Write_Binary_STP_Data(FILE *InputFile)
{
 
    int i_size, c_size, d_size;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    d_size = sizeof(double);
    
    // Write out STP file data

    FWRITE(&(NozzleXYZ_[0]), d_size, 1, InputFile); 
    FWRITE(&(NozzleXYZ_[1]), d_size, 1, InputFile); 
    FWRITE(&(NozzleXYZ_[2]), d_size, 1, InputFile); 
    
    FWRITE(&(NozzleNormal_[0]), d_size, 1, InputFile); 
    FWRITE(&(NozzleNormal_[1]), d_size, 1, InputFile); 
    FWRITE(&(NozzleNormal_[2]), d_size, 1, InputFile); 
    
    FWRITE(&(NozzleRadius_), d_size, 1, InputFile); 

}

/*##############################################################################
#                                                                              #
#                    ENGINE_FACE Read_Binary_STP_Data                          #
#                                                                              #
##############################################################################*/

void ENGINE_FACE::Read_Binary_STP_Data(FILE *InputFile)
{
 
    int i_size, c_size, d_size;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    d_size = sizeof(double);
    
    // Write out STP file data

    FREAD(&(NozzleXYZ_[0]), d_size, 1, InputFile); 
    FREAD(&(NozzleXYZ_[1]), d_size, 1, InputFile); 
    FREAD(&(NozzleXYZ_[2]), d_size, 1, InputFile); 
    
    FREAD(&(NozzleNormal_[0]), d_size, 1, InputFile); 
    FREAD(&(NozzleNormal_[1]), d_size, 1, InputFile); 
    FREAD(&(NozzleNormal_[2]), d_size, 1, InputFile); 
    
    FREAD(&(NozzleRadius_), d_size, 1, InputFile); 

}

/*##############################################################################
#                                                                              #
#                    ENGINE_FACE Skip_Read_Binary_STP_Data                     #
#                                                                              #
##############################################################################*/

void ENGINE_FACE::Skip_Read_Binary_STP_Data(FILE *InputFile)
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

}

#include "END_NAME_SPACE.H"








