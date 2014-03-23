//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//


#include "Matrix.h"

Matrix4d::Matrix4d()
{
}

void Matrix4d::loadIdentity()
{
    setIdentity( mat );
}

void Matrix4d::setIdentity( double* m ) const
{
    for ( int i = 0 ; i < 4 ; i++ )
        for ( int j = 0 ; j < 4 ; j++ )
        {
            if ( i == j )
            {
                m[i * 4 + j] = 1.0;
            }
            else
            {
                m[i * 4 + j] = 0.0;
            }
        }
}

void Matrix4d::translatef( const double &x, const double &y, const double &z )
{
    double tmat[16];

    setIdentity( tmat );

    tmat[12] = x;
    tmat[13] = y;
    tmat[14] = z;

    matMult( tmat );

}

void Matrix4d::rotateX( const double &ang )
{
    double tmat[16];
    double rang = ang * ( double )PI / 180.0f;
    double ca = ( double )cos( rang );
    double sa = ( double )sin( rang );

    setIdentity( tmat );
    tmat[5] = ca;
    tmat[6] = sa;
    tmat[9] = -sa;
    tmat[10] = ca;

    matMult( tmat );
}

void Matrix4d::rotateY( const double &ang )
{
    double tmat[16];
    double rang = ang * ( double )PI / 180.0f;
    double ca = ( double )cos( rang );
    double sa = ( double )sin( rang );

    setIdentity( tmat );
    tmat[0] = ca;
    tmat[2] = -sa;
    tmat[8] = sa;
    tmat[10] = ca;

    matMult( tmat );
}

void Matrix4d::rotateZ( const double &ang )
{
    double tmat[16];
    double rang = ang * ( double )PI / 180.0f;
    double ca = ( double )cos( rang );
    double sa = ( double )sin( rang );

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
            res[j * 4 + i] = mat[i] * m[j * 4]     + mat[4 + i] * m[j * 4 + 1]
                             + mat[8 + i] * m[j * 4 + 2] + mat[12 + i] * m[j * 4 + 3];
        }

    memcpy( mat, res, 16 * sizeof( double ) );
}

void Matrix4d::postMult( double* m )
{
    double res[16];

    for ( int i = 0 ; i < 4 ; i++ )
        for ( int j = 0 ; j < 4 ; j++ )
        {
            res[j * 4 + i] = m[i] * mat[j * 4]     + m[4 + i] * mat[j * 4 + 1]
                             + m[8 + i] * mat[j * 4 + 2] + m[12 + i] * mat[j * 4 + 3];
        }

    memcpy( mat, res, 16 * sizeof( double ) );
}

void Matrix4d::getMat( double* m )
{
    memcpy( m, mat, 16 * sizeof( double ) );
}

void Matrix4d::initMat( double* m )
{
    memcpy( mat, m, 16 * sizeof( double ) );
}

void Matrix4d::mult( const double in[4], double out[4] ) const
{
    out[0] = mat[0] * in[0] + mat[4] * in[1] + mat[8] * in[2] + mat[12] * in[3];
    out[1] = mat[1] * in[0] + mat[5] * in[1] + mat[9] * in[2] + mat[13] * in[3];
    out[2] = mat[2] * in[0] + mat[6] * in[1] + mat[10] * in[2] + mat[14] * in[3];
    out[3] = mat[3] * in[0] + mat[7] * in[1] + mat[11] * in[2] + mat[15] * in[3];
}

void Matrix4d::rotate( const double angle, const vec3d & axis )
{
    vec3d a( axis );
    a.normalize();
    double c  = cos( angle );
    double s  = sin( angle );
    double x = a[0];
    double y = a[1];
    double z = a[2];
    double m  = 1.0 - c;

    loadIdentity();

    mat[0]      = x * x * ( m ) + c;
    mat[4]      = y * x * ( m ) + z * s;
    mat[8]      = x * z * ( m ) - y * s;

    mat[1]      = x * y * ( m ) - z * s;
    mat[5]      = y * y * ( m ) + c;
    mat[9]      = y * z * ( m ) + x * s;

    mat[2]      = x * z * ( m ) + y * s;
    mat[6]      = y * z * ( m ) - x * s;
    mat[10]      = z * z * ( m ) + c;
}

void Matrix4d::affineInverse()
{
    /*
    !!! For Affine Transformation Matrix holding only rotations and translations, not scale or shear !!!
    */


    double res[16];
    setIdentity( res );
    // Assuming the upper left 3x3 is a rotation matrix only and is orthonormal
    // Transpose of upper left 3x3
    res[0] = mat[0];
    res[4] = mat[1];
    res[8]  = mat[2];
    res[1] = mat[4];
    res[5] = mat[5];
    res[9]  = mat[6];
    res[2] = mat[8];
    res[6] = mat[9];
    res[10] = mat[10];

    // Negative of 3x3 transpose*translations_vector
    res[12] = -res[0] * mat[12] - res[4] * mat[13] - res[8] * mat[14];
    res[13] = -res[1] * mat[12] - res[5] * mat[13] - res[9] * mat[14];
    res[14] = -res[2] * mat[12] - res[6] * mat[13] - res[10] * mat[14];

    memcpy( mat, res, 16 * sizeof( double ) );
}

void Matrix4d::scale( const double &scale )
{
    mat[0] *= scale;
    mat[5] *= scale;
    mat[10] *= scale;
}

vec3d Matrix4d::xform( const vec3d & in ) const
{
    vec3d out;
    out[0] = mat[0] * in[0] + mat[4] * in[1] + mat[8] * in[2] + mat[12];
    out[1] = mat[1] * in[0] + mat[5] * in[1] + mat[9] * in[2] + mat[13];
    out[2] = mat[2] * in[0] + mat[6] * in[1] + mat[10] * in[2] + mat[14];
    return out;
}

vec3d Matrix4d::getAngles() const
{
    vec3d angles;
    if ( fabs( mat[8] ) != 1 )
    {
        angles.set_y( asin( mat[8] ) );
        double cos_y = cos( angles.y() );
        angles.set_x( -atan2( mat[9] / cos_y, mat[10] / cos_y ) );
        angles.set_z( -atan2( mat[4] / cos_y, mat[0] / cos_y ) );
    }
    else if ( mat[8] == 1 )
    {
        angles.set_y( PI / 2 );
        angles.set_x( atan2( mat[1], mat[5] ) );
        angles.set_z( 0 );
    }
    else if ( mat[8] == -1 )
    {
        angles.set_y( -PI / 2 );
        angles.set_x( atan2( mat[6], mat[2] ) );
        angles.set_z( 0 );
    }
    return angles * RAD_2_DEG;
}

void Matrix4d::loadXZRef()
{
    setIdentity( mat );
    mat[5] = -1;
}

void Matrix4d::loadXYRef()
{
    setIdentity( mat );
    mat[10] = -1;
}

void Matrix4d::loadYZRef()
{
    setIdentity( mat );
    mat[0] = -1;
}

