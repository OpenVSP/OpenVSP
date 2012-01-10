//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//


#include "matrix.h"

Matrix::Matrix()
{
}

void Matrix::loadIdentity()
{
  setIdentity( mat );
}

void Matrix::setIdentity( float* m )
{
  for ( int i = 0 ; i < 4 ; i++ )
    for ( int j = 0 ; j < 4 ; j++ )
      {
        if ( i == j )
          m[i*4 + j] = 1.0;
        else
          m[i*4 + j] = 0.0;
      }
}

void Matrix::translatef( float x, float y, float z)
{
  float tmat[16];

  setIdentity( tmat );

  tmat[12] = x;
  tmat[13] = y;
  tmat[14] = z;

  matMult( tmat );

}

void Matrix::rotateX( float ang )
{
  float tmat[16];
  float rang = ang*(float)PI/180.0f; 
  float ca = (float)cos(rang);
  float sa = (float)sin(rang);

  setIdentity( tmat );
  tmat[5] = ca;
  tmat[6] = sa;
  tmat[9] = -sa;
  tmat[10] = ca;

  matMult( tmat );
}

void Matrix::rotateY( float ang )
{
  float tmat[16];
  float rang = ang*(float)PI/180.0f; 
  float ca = (float)cos(rang);
  float sa = (float)sin(rang);

  setIdentity( tmat );
  tmat[0] = ca;
  tmat[2] = -sa;
  tmat[8] = sa;
  tmat[10] = ca;

  matMult( tmat );
}

void Matrix::rotateZ( float ang )
{
  float tmat[16];
  float rang = ang*(float)PI/180.0f; 
  float ca = (float)cos(rang);
  float sa = (float)sin(rang);

  setIdentity( tmat );
  tmat[0] = ca;
  tmat[1] = sa;
  tmat[4] = -sa;
  tmat[5] = ca;

  matMult( tmat );
}

void Matrix::matMult( float* m )
{
  float res[16];

  for ( int i = 0 ; i < 4 ; i++ )
    for ( int j = 0 ; j < 4 ; j++ )
    {
      res[j*4 + i] = mat[i]*m[j*4]     + mat[4+i]*m[j*4+1] 
                   + mat[8+i]*m[j*4+2] + mat[12+i]*m[j*4+3];
    }

  memcpy( mat, res, 16*sizeof(float) );
}

void Matrix::postMult( float* m )
{
  float res[16];

  for ( int i = 0 ; i < 4 ; i++ )
    for ( int j = 0 ; j < 4 ; j++ )
    {
      res[j*4 + i] = m[i]*mat[j*4]     + m[4+i]*mat[j*4+1] 
                   + m[8+i]*mat[j*4+2] + m[12+i]*mat[j*4+3];
    }

  memcpy( mat, res, 16*sizeof(float) );
}

void Matrix::getMat( float* m )
{
  memcpy( m, mat, 16*sizeof(float) );

}


void Matrix::initMat( float* m )
{
  memcpy( mat, m, 16*sizeof(float) );
}

void Matrix::mult( float in[4], float out[4] )
{
	out[0] = mat[0] * in[0] + mat[4] * in[1] + mat[8] * in[2] + mat[12] * in[3];
	out[1] = mat[1] * in[0] + mat[5] * in[1] + mat[9] * in[2] + mat[13] * in[3];
	out[2] = mat[2] * in[0] + mat[6] * in[1] + mat[10] * in[2] + mat[14] * in[3];
	out[3] = mat[3] * in[0] + mat[7] * in[1] + mat[11] * in[2] + mat[15] * in[3];
}
/////////////////////////////////////////////////////////////////////////

#include "matrix.h"

Matrix4d::Matrix4d()
{
}

void Matrix4d::loadIdentity()
{
  setIdentity( mat );
}

void Matrix4d::setIdentity( double* m )
{
  for ( int i = 0 ; i < 4 ; i++ )
    for ( int j = 0 ; j < 4 ; j++ )
      {
        if ( i == j )
          m[i*4 + j] = 1.0;
        else
          m[i*4 + j] = 0.0;
      }
}

void Matrix4d::translatef( double x, double y, double z)
{
  double tmat[16];

  setIdentity( tmat );

  tmat[12] = x;
  tmat[13] = y;
  tmat[14] = z;

  matMult( tmat );

}

void Matrix4d::rotateX( double ang )
{
  double tmat[16];
  double rang = ang*(double)PI/180.0f; 
  double ca = (double)cos(rang);
  double sa = (double)sin(rang);

  setIdentity( tmat );
  tmat[5] = ca;
  tmat[6] = sa;
  tmat[9] = -sa;
  tmat[10] = ca;

  matMult( tmat );
}

void Matrix4d::rotateY( double ang )
{
  double tmat[16];
  double rang = ang*(double)PI/180.0f; 
  double ca = (double)cos(rang);
  double sa = (double)sin(rang);

  setIdentity( tmat );
  tmat[0] = ca;
  tmat[2] = -sa;
  tmat[8] = sa;
  tmat[10] = ca;

  matMult( tmat );
}

void Matrix4d::rotateZ( double ang )
{
  double tmat[16];
  double rang = ang*(double)PI/180.0f; 
  double ca = (double)cos(rang);
  double sa = (double)sin(rang);

  setIdentity( tmat );
  tmat[0] = ca;
  tmat[1] = sa;
  tmat[4] = -sa;
  tmat[5] = ca;

  matMult( tmat );
}

void Matrix4d::matMult( double* m )
{
  double res[16];

  for ( int i = 0 ; i < 4 ; i++ )
    for ( int j = 0 ; j < 4 ; j++ )
    {
      res[j*4 + i] = mat[i]*m[j*4]     + mat[4+i]*m[j*4+1] 
                   + mat[8+i]*m[j*4+2] + mat[12+i]*m[j*4+3];
    }

  memcpy( mat, res, 16*sizeof(double) );
}

void Matrix4d::postMult( double* m )
{
  double res[16];

  for ( int i = 0 ; i < 4 ; i++ )
    for ( int j = 0 ; j < 4 ; j++ )
    {
      res[j*4 + i] = m[i]*mat[j*4]     + m[4+i]*mat[j*4+1] 
                   + m[8+i]*mat[j*4+2] + m[12+i]*mat[j*4+3];
    }

  memcpy( mat, res, 16*sizeof(double) );
}

void Matrix4d::getMat( double* m )
{
  memcpy( m, mat, 16*sizeof(double) );

}



void Matrix4d::initMat( double* m )
{
  memcpy( mat, m, 16*sizeof(double) );
}

void Matrix4d::mult( double in[4], double out[4] )
{
	out[0] = mat[0] * in[0] + mat[4] * in[1] + mat[8] * in[2] + mat[12] * in[3];
	out[1] = mat[1] * in[0] + mat[5] * in[1] + mat[9] * in[2] + mat[13] * in[3];
	out[2] = mat[2] * in[0] + mat[6] * in[1] + mat[10] * in[2] + mat[14] * in[3];
	out[3] = mat[3] * in[0] + mat[7] * in[1] + mat[11] * in[2] + mat[15] * in[3];
}

void Matrix4d::rotate( double angle, vec3d & axis )
{
       vec3d a(axis);
       a.normalize();
       double c  = cos(angle);
       double s  = sin(angle);
       double x = a[0];
       double y = a[1];
       double z = a[2];
       double m  = 1.0 - c;

       loadIdentity();

       mat[0]      = x*x*(m)+c;
       mat[4]      = y*x*(m)+z*s;
       mat[8]      = x*z*(m)-y*s;

       mat[1]      = x*y*(m)-z*s;
       mat[5]      = y*y*(m)+c;
       mat[9]      = y*z*(m)+x*s;

       mat[2]      = x*z*(m)+y*s;
       mat[6]      = y*z*(m)-x*s;
       mat[10]      = z*z*(m)+c;

       //mat[0][0]      = x*x*(m)+c;
       //mat[0][1]      = y*x*(m)+z*s;
       //mat[0][2]      = x*z*(m)-y*s;

       //mat[1][0]      = x*y*(m)-z*s;
       //mat[1][1]      = y*y*(m)+c;
       //mat[1][2]      = y*z*(m)+x*s;

       //mat[2][0]      = x*z*(m)+y*s;
       //mat[2][1]      = y*z*(m)-x*s;
       //mat[2][2]      = z*z*(m)+c;
}

vec3d Matrix4d::xform( vec3d & in )
{
	vec3d out;
	out[0] = mat[0] * in[0] + mat[4] * in[1] + mat[8] * in[2] + mat[12];
	out[1] = mat[1] * in[0] + mat[5] * in[1] + mat[9] * in[2] + mat[13];
	out[2] = mat[2] * in[0] + mat[6] * in[1] + mat[10] * in[2] + mat[14];
	return out;
}

