//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "Matrix4d.h"

Matrix4d::Matrix4d()
{
    loadIdentity();
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

void Matrix4d::translatev( const vec3d &v )
{
    translatef( v.x(), v.y(), v.z() );
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

void Matrix4d::matMult( const double* m )
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

void Matrix4d::postMult( const double* m )
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

void Matrix4d::matMult( const Matrix4d &m )
{
    double tmat[16];
    m.getMat( tmat );
    matMult( tmat );
}

void Matrix4d::postMult( const Matrix4d & m )
{
    double tmat[16];
    m.getMat( tmat );
    postMult( tmat );
}

void Matrix4d::getMat( double* m ) const
{
    memcpy( m, mat, 16 * sizeof( double ) );
}

void Matrix4d::initMat( const double* m )
{
    memcpy( mat, m, 16 * sizeof( double ) );
}

void Matrix4d::initMat( const Matrix4d & m )
{
    double tmat[16];
    m.getMat( tmat );
    initMat( tmat );
}

void Matrix4d::mult( const double in[4], double out[4] ) const
{
    out[0] = mat[0] * in[0] + mat[4] * in[1] + mat[8] * in[2] + mat[12] * in[3];
    out[1] = mat[1] * in[0] + mat[5] * in[1] + mat[9] * in[2] + mat[13] * in[3];
    out[2] = mat[2] * in[0] + mat[6] * in[1] + mat[10] * in[2] + mat[14] * in[3];
    out[3] = mat[3] * in[0] + mat[7] * in[1] + mat[11] * in[2] + mat[15] * in[3];
}

void Matrix4d::rotate( const double &angle, const vec3d & axis )
{
    vec3d a( axis );
    a.normalize();
    double c  = cos( angle );
    double s  = sin( angle );
    double x = a[0];
    double y = a[1];
    double z = a[2];
    double m  = 1.0 - c;

    double tmat[16];

    setIdentity( tmat );

    tmat[0]      = x * x * ( m ) + c;
    tmat[4]      = y * x * ( m ) + z * s;
    tmat[8]      = x * z * ( m ) - y * s;

    tmat[1]      = x * y * ( m ) - z * s;
    tmat[5]      = y * y * ( m ) + c;
    tmat[9]      = y * z * ( m ) + x * s;

    tmat[2]      = x * z * ( m ) + y * s;
    tmat[6]      = y * z * ( m ) - x * s;
    tmat[10]      = z * z * ( m ) + c;

    matMult( tmat );
}

void Matrix4d::rotatealongX( const vec3d & dir )
{
    vec3d dir1, dir2, dir3;
    dir1 = dir;
    dir1.normalize();
    dir2.v[ dir1.minor_comp() ] = 1.0;
    dir3 = cross( dir1, dir2 );
    dir3.normalize();

    dir2 = cross( dir3, dir1 );
    dir2.normalize();

    double tmat[16];
    setIdentity( tmat );
    for( int i = 0; i < 3; i++ )
    {
        tmat[ i * 4 ] = dir1.v[i];
        tmat[ i * 4 + 1 ] = dir2.v[i];
        tmat[ i * 4 + 2 ] = dir3.v[i];
    }

    matMult( tmat );
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

void Matrix4d::flipx()
{
    mat[0] *= -1.0;
}

vec3d Matrix4d::xform( const vec3d & in ) const
{
    vec3d out;
    out[0] = mat[0] * in[0] + mat[4] * in[1] + mat[8] * in[2] + mat[12];
    out[1] = mat[1] * in[0] + mat[5] * in[1] + mat[9] * in[2] + mat[13];
    out[2] = mat[2] * in[0] + mat[6] * in[1] + mat[10] * in[2] + mat[14];
    return out;
}

void Matrix4d::xformvec( std::vector < vec3d > & in ) const
{
    for ( int i = 0; i < in.size(); i++ )
    {
        in[i] = xform( in[i] );
    }
}

// Transform for normal vectors -- rotations only, no translations
vec3d Matrix4d::xformnorm( const vec3d & in ) const
{
    vec3d out;
    out[0] = mat[0] * in[0] + mat[4] * in[1] + mat[8] * in[2];
    out[1] = mat[1] * in[0] + mat[5] * in[1] + mat[9] * in[2];
    out[2] = mat[2] * in[0] + mat[6] * in[1] + mat[10] * in[2];
    return out;
}

void Matrix4d::xformnormvec( std::vector < vec3d > & in ) const
{
    for ( int i = 0; i < in.size(); i++ )
    {
        in[i] = xformnorm( in[i] );
    }
}

vec3d Matrix4d::getAngles() const
{
    vec3d angles;
    if ( std::abs( mat[8] ) != 1 )
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

vec3d Matrix4d::getTranslation() const
{
    vec3d out;
    out[0] = mat[12];
    out[1] = mat[13];
    out[2] = mat[14];
    return out;
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

void Matrix4d::buildXForm( const vec3d & pos, const vec3d & rot, const vec3d & cent_rot )
{
    Matrix4d tran_mat;
    tran_mat.translatef( pos.x(), pos.y(), pos.z() );

    Matrix4d rotate_mat;
    rotate_mat.rotateX( rot.x() );
    rotate_mat.rotateY( rot.y() );
    rotate_mat.rotateZ( rot.z() );

    Matrix4d cent_mat;
    cent_mat.translatef( -cent_rot.x(), -cent_rot.y(), -cent_rot.z() );

    Matrix4d inv_cent_mat;
    inv_cent_mat.translatef(  cent_rot.x(),  cent_rot.y(),  cent_rot.z() );

    postMult( cent_mat.data() );
    postMult( rotate_mat.data() );
    postMult( inv_cent_mat.data() );
    postMult( tran_mat.data() );
}

void Matrix4d::getBasis( vec3d &xdir, vec3d &ydir, vec3d &zdir )
{
    for( int i = 0; i < 3; i++ )
    {
        xdir.v[i] = mat[ i ];
        ydir.v[i] = mat[ i + 4 ];
        zdir.v[i] = mat[ i + 8 ];
    }
}

void Matrix4d::setBasis( const vec3d &xdir, const vec3d &ydir, const vec3d &zdir )
{
    for( int i = 0; i < 3; i++ )
    {
        mat[ i ] = xdir.v[i];
        mat[ i + 4 ] = ydir.v[i];
        mat[ i + 8 ] = zdir.v[i];
    }
}

void Matrix4d::toQuat( double &qw, double &qx, double &qy, double &qz, double &tx, double &ty, double &tz ) const
{
    // Copy out translations.
    tx = mat[12];
    ty = mat[13];
    tz = mat[14];

    float trace = mat[0] + mat[5] + mat[10]; // I removed + 1.0f; see discussion with Ethan
    if( trace > 0 )    // I changed M_EPSILON to 0
    {
        float s = 0.5f / sqrtf( trace + 1.0f );
        qw = 0.25f / s;
        qx = ( mat[6] - mat[9] ) * s;
        qy = ( mat[8] - mat[2] ) * s;
        qz = ( mat[1] - mat[4] ) * s;
    }
    else
    {
        if ( mat[0] > mat[5] && mat[0] > mat[10] )
        {
            float s = 2.0f * sqrtf( 1.0f + mat[0] - mat[5] - mat[10] );
            qw = ( mat[6] - mat[9] ) / s;
            qx = 0.25f * s;
            qy = ( mat[4] + mat[1] ) / s;
            qz = ( mat[8] + mat[2] ) / s;
        }
        else if ( mat[5] > mat[10])
        {
            float s = 2.0f * sqrtf( 1.0f + mat[5] - mat[0] - mat[10] );
            qw = ( mat[8] - mat[2] ) / s;
            qx = ( mat[4] + mat[1] ) / s;
            qy = 0.25f * s;
            qz = ( mat[9] + mat[6] ) / s;
        }
        else
        {
            float s = 2.0f * sqrtf( 1.0f + mat[10] - mat[0] - mat[5] );
            qw = ( mat[1] - mat[4] ) / s;
            qx = ( mat[8] + mat[2] ) / s;
            qy = ( mat[9] + mat[6] ) / s;
            qz = 0.25f * s;
        }
    }
}

/*
 [0][0]       [0][1]       [0][2]     [0][3]
 [1][0]       [1][1]       [1][2]     [1][3]
 [2][0]       [2][1]       [2][2]     [2][3]
 [3][0]       [3][1]       [3][2]     [3][3]

 mat[0]       mat[4]       mat[8]       mat[12]
 mat[1]       mat[5]       mat[9]       mat[13]
 mat[2]       mat[6]       mat[10]      mat[14]
 mat[3]       mat[7]       mat[11]      mat[15]
 */
