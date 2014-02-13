//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Bezier Curve Class  (Cubic)
//
//
//   J.R. Gloudemans - 7/20/94
//   Sterling Software
//
//******************************************************************************

#include "BezierCurve.h"


//===== Constructor  =====//
Bezier_curve::Bezier_curve()
{
    num_sections = 0;

}

//===== Destructor  =====//
Bezier_curve::~Bezier_curve()
{
}

//===== Compute Blending Functions  =====//
void Bezier_curve::blend_funcs( double u, double& F1, double& F2, double& F3, double& F4 )
{
    //==== Compute All Blending Functions ====//
    double uu = u * u;
    double one_u = 1.0 - u;
    double one_u_sqr = one_u * one_u;

    F1 = one_u * one_u_sqr;
    F2 = 3.0 * u * one_u_sqr;
    F3 = 3.0 * uu * one_u;
    F4 = uu * u;
}

//===== Compute Blending Function Derivatives  =====//
void Bezier_curve::der_blend_funcs( double u, double& F1, double& F2, double& F3, double& F4 )
{
    //==== Compute All Blending Functions ====//
    double three_uu = 3.0 * u * u;
    double six_u = 6.0 * u;

    F1 = -three_uu + six_u - 3.0;
    F2 = 3.0 * ( three_uu - 4.0 * u + 1.0 );
    F3 = -3.0 * three_uu + six_u;
    F4 = three_uu;
}

//===== Initialize Number of Sections  =====//
void Bezier_curve::init_num_sections( int num_sections_in )
{
    if ( num_sections == num_sections_in )      // Already correct size
    {
        return;
    }

    num_sections = num_sections_in;
//  pnts.init(num_sections*3 + 1);
    pnts.resize( num_sections * 3 + 1 );
}

//===== Compute Point  =====//
vec3d Bezier_curve::comp_pnt( int sec_num, double u )
{

    //==== Check For Valid Range ====//
    if ( sec_num < 0 )
    {
        return( pnts[0] );
    }

    else if ( sec_num > ( num_sections - 1 ) )
    {
        return( pnts[num_sections * 3] );
    }

    double F1, F2, F3, F4;

    blend_funcs( u, F1, F2, F3, F4 );

    vec3d new_pnt;

    int index = sec_num * 3;

    new_pnt = pnts[index] * F1   + pnts[index + 1] * F2 +
              pnts[index + 2] * F3 + pnts[index + 3] * F4;

    return( new_pnt );

}

//===== Compute Point  =====//
vec3d Bezier_curve::comp_pnt( double u )
{
    double usect = u * ( double )num_sections;

    int sect = ( int )usect;

    double remu = usect - ( double )sect;

    return comp_pnt( sect, remu );
}


//===== Compute Tangent  =====//
vec3d Bezier_curve::comp_tan( int sec_num, double u )
{
    double F1, F2, F3, F4;

    der_blend_funcs( u, F1, F2, F3, F4 );

    vec3d new_tan;

    int index = sec_num * 3;

    new_tan = pnts[index] * F1   + pnts[index + 1] * F2 +
              pnts[index + 2] * F3 + pnts[index + 3] * F4;

    return( new_tan );

}

//===== Compute Length  =====//
double Bezier_curve::get_length()
{
    double length = 0.0;

    vec3d last_pnt = comp_pnt( 0, 0.0 );

    for ( int i = 0 ; i < num_sections ; i++ )
    {
        for ( int j = 1 ; j < 11 ; j++ )
        {
            double u = ( double )j / 10.0;
            vec3d pnt =  comp_pnt( i, u );

            length += ( float ) dist( last_pnt, pnt );

            last_pnt = pnt;
        }
    }

    return length;
}

void Bezier_curve::buildCurve( const vector< vec3d > & pVec, double tanStr, int closeFlag )
{
    if ( pVec.size() < 2 )
    {
        return;
    }

    int i;
    vec3d tan;

    //==== Allocate Space ====//
    init_num_sections( pVec.size() - 1  );      // Init New Curve

    //==== First Point ====//
    pnts[0] = pVec[0];

    if ( closeFlag )
    {
        tan = pVec[1] - pVec[pVec.size() - 2];
    }
    else
    {
        tan = pVec[1] - pVec[0];
    }

    tan.normalize();
    double mag = ( pVec[1] - pVec[0] ).mag();

    pnts[1] = pVec[0] + tan * ( mag * tanStr );

    for ( i = 1 ; i < ( int )pVec.size() - 1 ; i++ )
    {
        tan = pVec[i + 1] - pVec[i - 1];
        tan.normalize();
        mag = ( pVec[i] - pVec[i - 1] ).mag();

        pnts[( i - 1 ) * 3 + 2] = pVec[i] - tan * ( mag * tanStr );

        pnts[( i - 1 ) * 3 + 3] = pVec[i];

        mag = ( pVec[i + 1] - pVec[i] ).mag();
        pnts[( i - 1 ) * 3 + 4] = pVec[i] + tan * ( mag * tanStr );

    }
    //==== Last Point ====//
    if ( closeFlag )
    {
        tan = pVec[1] - pVec[pVec.size() - 2];
    }
    else
    {
        tan = pVec[pVec.size() - 1] - pVec[pVec.size() - 2];
    }

    tan.normalize();
    mag = ( pVec[pVec.size() - 1] - pVec[pVec.size() - 2] ).mag();

    int ind = ( pVec.size() - 2 ) * 3 + 2;
    pnts[ind] = pVec[pVec.size() - 1] - tan * ( mag * tanStr );

    pnts[ind + 1] = pVec[pVec.size() - 1] ;

}


void Bezier_curve::buildCurve( const vector< vec3d > & pVec,
                               const vector< int > & pFlagVec, double tanStr, int closeFlag )
{
    if ( pVec.size() < 2 )
    {
        return;
    }
    if ( pVec.size() != pFlagVec.size() )
    {
        printf( "ERROR Bezier_curve::buildCurve mismatch size\n" );
        return;
    }

    int i;
    vec3d tan;

    //==== Load Bezier Curve ====//
    init_num_sections( pVec.size() - 1 );   // Init New Curve

    pnts[0] = pVec[0];

    if ( closeFlag )
    {
        tan = pVec[1] - pVec[pVec.size() - 2];
    }
    else
    {
        tan = pVec[1] - pVec[0];
    }

    if ( pFlagVec[0] == ZERO )
    {
        tan = vec3d( 0, 0, 0 );
    }
    else if ( pFlagVec[0] == SHARP )
    {
        tan = pVec[1] - pVec[0];
    }
    else if (  pFlagVec[0] == ZERO_X )
    {
        tan.set_x( 0.0 );
    }
    else if (  pFlagVec[0] == ZERO_Y )
    {
        tan.set_y( 0.0 );
    }
    else if (  pFlagVec[0] == ZERO_Z )
    {
        tan.set_z( 0.0 );
    }

    tan.normalize();
    double mag = ( pVec[1] - pVec[0] ).mag();

    pnts[1] = pVec[0] + tan * ( mag * tanStr );

    //==== Middle Points ====//
    for ( i = 1 ; i < ( int )pVec.size() - 1 ; i++ )
    {
        tan = pVec[i + 1] - pVec[i - 1];
        if ( pFlagVec[i] == ZERO )
        {
            tan = vec3d( 0, 0, 0 );
        }
        else if ( pFlagVec[i] == SHARP )
        {
            tan = pVec[i] - pVec[i - 1];
        }
        else if (  pFlagVec[i] == ZERO_X )
        {
            tan.set_x( 0.0 );
        }
        else if (  pFlagVec[i] == ZERO_Y )
        {
            tan.set_y( 0.0 );
        }
        else if (  pFlagVec[i] == ZERO_Z )
        {
            tan.set_z( 0.0 );
        }
        else if (  pFlagVec[i] == ONLY_BACK )
        {
            tan = pVec[i] - pVec[i - 1];
        }
        else if (  pFlagVec[i] == ONLY_FORWARD )
        {
            tan = pVec[i + 1] - pVec[i];
        }

        tan.normalize();
        mag = ( pVec[i] - pVec[i - 1] ).mag();

        pnts[( i - 1 ) * 3 + 2] = pVec[i] - tan * ( mag * tanStr );
        pnts[( i - 1 ) * 3 + 3] = pVec[i];

        tan = pVec[i + 1] - pVec[i - 1];
        if ( pFlagVec[i] == ZERO )
        {
            tan = vec3d( 0, 0, 0 );
        }
        else if ( pFlagVec[i] == SHARP )
        {
            tan = pVec[i + 1] - pVec[i];
        }
        else if (  pFlagVec[i] == ZERO_X )
        {
            tan.set_x( 0.0 );
        }
        else if (  pFlagVec[i] == ZERO_Y )
        {
            tan.set_y( 0.0 );
        }
        else if (  pFlagVec[i] == ZERO_Z )
        {
            tan.set_z( 0.0 );
        }
        else if (  pFlagVec[i] == ONLY_BACK )
        {
            tan = pVec[i] - pVec[i - 1];
        }
        else if (  pFlagVec[i] == ONLY_FORWARD )
        {
            tan = pVec[i + 1] - pVec[i];
        }

        tan.normalize();
        mag = ( pVec[i + 1] - pVec[i] ).mag();
        pnts[( i - 1 ) * 3 + 4] = pVec[i] + tan * ( mag * tanStr );
    }

    //==== Last Point ====//
    if ( closeFlag )
    {
        tan = pVec[1] - pVec[pVec.size() - 2];
    }
    else
    {
        tan = pVec[pVec.size() - 1] - pVec[pVec.size() - 2];
    }

    if ( pFlagVec[pVec.size() - 1] == ZERO )
    {
        tan = vec3d( 0, 0, 0 );
    }
    else if ( pFlagVec[pVec.size() - 1] == SHARP )
    {
        tan = pVec[pVec.size() - 1] - pVec[pVec.size() - 2];
    }
    else if (  pFlagVec[pVec.size() - 1] == ZERO_X )
    {
        tan.set_x( 0.0 );
    }
    else if (  pFlagVec[pVec.size() - 1] == ZERO_Y )
    {
        tan.set_y( 0.0 );
    }
    else if (  pFlagVec[pVec.size() - 1] == ZERO_Z )
    {
        tan.set_z( 0.0 );
    }

    tan.normalize();
    mag = ( pVec[pVec.size() - 1] - pVec[pVec.size() - 2] ).mag();

    int ind = ( pVec.size() - 2 ) * 3 + 2;

    pnts[ind] = pVec[pVec.size() - 1] - tan * ( mag * tanStr );
    pnts[ind + 1] = pVec[pVec.size() - 1] ;

}

void Bezier_curve::buildCurve2( const vector< vec3d > & pVec,
                                const vector< int > & pFlagVec, double tanStr, int closeFlag )
{
    if ( pVec.size() < 2 )
    {
        return;
    }
    if ( pVec.size() != pFlagVec.size() )
    {
        printf( "ERROR Bezier_curve::buildCurve mismatch size\n" );
        return;
    }
    int i, ind;
    vec3d tan;
    double mag;

    //==== Load Bezier Curve ====//
    init_num_sections( pVec.size() - 1 );   // Init New Curve

    pnts[0] = pVec[0];

    setTan( tan, pVec, 0, pFlagVec[0], 1, closeFlag );
    mag = ( pVec[1] - pVec[0] ).mag();
    pnts[1] = pVec[0] + tan * ( mag * tanStr );

    //==== Middle Points ====//
    for ( i = 1 ; i < ( int )pVec.size() - 1 ; i++ )
    {
        double mag_minus = ( pVec[i] - pVec[i - 1] ).mag();
        double mag_plus  = ( pVec[i + 1] - pVec[i] ).mag();
        double skip_mag  = ( pVec[i + 1] - pVec[i - 1] ).mag();
        double sum_mag   = mag_minus + mag_plus;

        if ( skip_mag < sum_mag )
        {
            mag_minus = skip_mag * ( mag_minus / sum_mag );
            mag_plus  = skip_mag * ( mag_plus / sum_mag );
        }

        setTan( tan, pVec, i, pFlagVec[i], 0, closeFlag );
        pnts[( i - 1 ) * 3 + 2] = pVec[i] - tan * ( mag_minus * tanStr );
        pnts[( i - 1 ) * 3 + 3] = pVec[i];

        setTan( tan, pVec, i, pFlagVec[i], 1, closeFlag );
        pnts[( i - 1 ) * 3 + 4] = pVec[i] + tan * ( mag_plus * tanStr );
    }

    //==== Last Point ====//
    ind = ( int )pVec.size() - 1;
    setTan( tan, pVec, ind, pFlagVec[ind], 0, closeFlag );
    mag = ( pVec[pVec.size() - 1] - pVec[pVec.size() - 2] ).mag();

    ind = ( pVec.size() - 2 ) * 3 + 2;
    pnts[ind] = pVec[pVec.size() - 1] - tan * ( mag * tanStr );
    pnts[ind + 1] = pVec[pVec.size() - 1] ;

}

void Bezier_curve::buildCurve( const vector< vec3d > & pVec, const vector< vec3d > & spineVec,
                               const vector< int > & pFlagVec, const vector< double > & tAngVec,
                               const vector< double > & tStr1Vec, const vector< double > & tStr2Vec, int closeFlag )
{
    if ( pVec.size() < 2 )
    {
        return;
    }
    if ( pVec.size() != pFlagVec.size() || pVec.size() != spineVec.size() ||
            pVec.size() != tStr1Vec.size() || pVec.size() != tStr2Vec.size() )
    {
        printf( "ERROR Bezier_curve::buildCurve mismatch size\n" );
        return;
    }

    int i;
    vec3d tan;
    vec3d ptan0;
    vec3d ptan1;

    //==== Load Bezier Curve ====//
    init_num_sections( pVec.size() - 1 );   // Init New Curve

    pnts[0] = pVec[0];

    if ( closeFlag )
    {
        tan = pVec[1] - pVec[pVec.size() - 2];
    }
    else
    {
        tan = pVec[1] - pVec[0];
    }

    if ( pFlagVec[0] == ZERO )
    {
        tan = vec3d( 0, 0, 0 );
    }
    else if ( pFlagVec[0] == SHARP )
    {
        tan = pVec[1] - pVec[0];
    }
    else if (  pFlagVec[0] == ZERO_X )
    {
        tan.set_x( 0.0 );
    }
    else if (  pFlagVec[0] == ZERO_Y )
    {
        tan.set_y( 0.0 );
    }
    else if (  pFlagVec[0] == ZERO_Z )
    {
        tan.set_z( 0.0 );
    }

    tan.normalize();
    double mag = ( pVec[1] - pVec[0] ).mag();

    pnts[1] = pVec[0] + tan * ( mag * tStr2Vec[0] );

    //==== Middle Points ====//
    for ( i = 1 ; i < ( int )pVec.size() - 1 ; i++ )
    {
        ptan0 = pVec[i - 1] + ( pVec[i - 1] - spineVec[i - 1] ) * -tAngVec[i];
        ptan1 = pVec[i + 1] + ( pVec[i + 1] - spineVec[i + 1] ) * tAngVec[i];

        tan = ptan1 - ptan0;
        if ( pFlagVec[i] == ZERO )
        {
            tan = vec3d( 0, 0, 0 );
        }
        else if ( pFlagVec[i] == SHARP )
        {
            tan = pVec[i] - pVec[i - 1];
        }
        else if (  pFlagVec[i] == ZERO_X )
        {
            tan.set_x( 0.0 );
        }
        else if (  pFlagVec[i] == ZERO_Y )
        {
            tan.set_y( 0.0 );
        }
        else if (  pFlagVec[i] == ZERO_Z )
        {
            tan.set_z( 0.0 );
        }
        else if (  pFlagVec[i] == ONLY_BACK )
        {
            tan = pVec[i] - pVec[i - 1];
        }
        else if (  pFlagVec[i] == ONLY_FORWARD )
        {
            tan = pVec[i + 1] - pVec[i];
        }

        tan.normalize();
        mag = ( pVec[i] - pVec[i - 1] ).mag();

        pnts[( i - 1 ) * 3 + 2] = pVec[i] - tan * ( mag * tStr1Vec[i] );
        pnts[( i - 1 ) * 3 + 3] = pVec[i];

        tan = ptan1 - ptan0;
        if ( pFlagVec[i] == ZERO )
        {
            tan = vec3d( 0, 0, 0 );
        }
        else if ( pFlagVec[i] == SHARP )
        {
            tan = pVec[i + 1] - pVec[i];
        }
        else if (  pFlagVec[i] == ZERO_X )
        {
            tan.set_x( 0.0 );
        }
        else if (  pFlagVec[i] == ZERO_Y )
        {
            tan.set_y( 0.0 );
        }
        else if (  pFlagVec[i] == ZERO_Z )
        {
            tan.set_z( 0.0 );
        }
        else if (  pFlagVec[i] == ONLY_BACK )
        {
            tan = pVec[i] - pVec[i - 1];
        }
        else if (  pFlagVec[i] == ONLY_FORWARD )
        {
            tan = pVec[i + 1] - pVec[i];
        }

        tan.normalize();
        mag = ( pVec[i + 1] - pVec[i] ).mag();
        pnts[( i - 1 ) * 3 + 4] = pVec[i] + tan * ( mag * tStr2Vec[i] );
    }

    //==== Last Point ====//
    if ( closeFlag )
    {
        tan = pVec[1] - pVec[pVec.size() - 2];
    }
    else
    {
        tan = pVec[pVec.size() - 1] - pVec[pVec.size() - 2];
    }

    if ( pFlagVec[pVec.size() - 1] == ZERO )
    {
        tan = vec3d( 0, 0, 0 );
    }
    else if ( pFlagVec[pVec.size() - 1] == SHARP )
    {
        tan = pVec[pVec.size() - 1] - pVec[pVec.size() - 2];
    }
    else if (  pFlagVec[pVec.size() - 1] == ZERO_X )
    {
        tan.set_x( 0.0 );
    }
    else if (  pFlagVec[pVec.size() - 1] == ZERO_Y )
    {
        tan.set_y( 0.0 );
    }
    else if (  pFlagVec[pVec.size() - 1] == ZERO_Z )
    {
        tan.set_z( 0.0 );
    }

    tan.normalize();
    mag = ( pVec[pVec.size() - 1] - pVec[pVec.size() - 2] ).mag();

    int ind = ( pVec.size() - 2 ) * 3 + 2;

    pnts[ind] = pVec[pVec.size() - 1] - tan * ( mag * tStr1Vec[pVec.size() - 1] );

    pnts[ind + 1] = pVec[pVec.size() - 1] ;

}

void Bezier_curve::buildCurve( const vector< vec3d > & pVec, vec3d & axis, const vector< double > & tAngVec,
                               const vector< double > & tStr1Vec, const vector< double > & tStr2Vec )
{
    if ( pVec.size() < 2 )
    {
        return;
    }
    if ( pVec.size() != tAngVec.size()  ||
            pVec.size() != tStr1Vec.size() || pVec.size() != tStr2Vec.size() )
    {
        printf( "ERROR Bezier_curve::buildCurve mismatch size\n" );
        return;
    }

    int i;
    vec3d tan;
    vec3d ptan0;
    vec3d ptan1;

    //==== Load Bezier Curve ====//
    init_num_sections( pVec.size() - 1 );   // Init New Curve

    //==== First Point/Tan ====//
    pnts[0] = pVec[0];
    double pmag = ( pVec[1] - pVec[0] ).mag();

    vec3d xaxis( 1, 0, 0 );
    tan = RotateArbAxis( xaxis, tAngVec[0], axis );
    tan.normalize();

    pnts[1] = pVec[0] + tan * ( pmag * tStr2Vec[0] );

    //==== Middle Points ====//
    for ( i = 1 ; i < ( int )pVec.size() - 1 ; i++ )
    {
        tan = RotateArbAxis( xaxis, tAngVec[i], axis );
        tan.normalize();

        pmag = ( pVec[i] - pVec[i - 1] ).mag();

        pnts[( i - 1 ) * 3 + 2] = pVec[i] - tan * ( pmag * tStr1Vec[i] );
        pnts[( i - 1 ) * 3 + 3] = pVec[i];

        pmag = ( pVec[i + 1] - pVec[i] ).mag();
        pnts[( i - 1 ) * 3 + 4] = pVec[i] + tan * ( pmag * tStr2Vec[i] );
    }

    //==== Last Point ====//
    tan = RotateArbAxis( xaxis, tAngVec[pVec.size() - 1], axis );
    tan.normalize();

    pmag = ( pVec[pVec.size() - 1] - pVec[pVec.size() - 2] ).mag();

    int ind = ( pVec.size() - 2 ) * 3 + 2;
    pnts[ind] = pVec[pVec.size() - 1] - tan * ( pmag * tStr1Vec[pVec.size() - 1] );
    pnts[ind + 1] = pVec[pVec.size() - 1] ;

}
void Bezier_curve::setTan( vec3d & tan, const vector< vec3d > & pVec, int ind, int flag, int forward, int close )
{
    if ( ind == 0 )
    {
        if ( close )
        {
            tan = pVec[1] - pVec[pVec.size() - 2];
        }
        else
        {
            tan = pVec[1] - pVec[0];
        }

        if ( flag == PREDICT )
        {
            vec3d t10 = tan;
            vec3d t20 = pVec[2] - pVec[0];

            t10.normalize();
            t20.normalize();
            vec3d del = t10 - t20;
            tan = t10 + del;
        }
    }
    else if ( ind == pVec.size() - 1 )
    {
        if ( close )
        {
            tan = pVec[1] - pVec[pVec.size() - 2];
        }
        else
        {
            tan = pVec[pVec.size() - 1] - pVec[pVec.size() - 2];
        }

        if ( flag == PREDICT )
        {
            vec3d t10 = tan;
            vec3d t20 = pVec[pVec.size() - 1] - pVec[pVec.size() - 3];

            t10.normalize();
            t20.normalize();
            vec3d del = t10 - t20;
            tan = t10 + del;
        }
    }
    else
    {
        tan = pVec[ind + 1] - pVec[ind - 1];
    }

    if ( flag == ZERO )
    {
        tan = vec3d( 0, 0, 0 );
    }
    else if ( flag == SHARP )
    {
        if ( forward )
        {
            tan = pVec[ind + 1] - pVec[ind];
        }
        else
        {
            tan = pVec[ind] - pVec[ind - 1];
        }
    }
    else if (  flag == ZERO_X )
    {
        tan.set_x( 0.0 );
    }
    else if (  flag == ZERO_Y )
    {
        tan.set_y( 0.0 );
    }
    else if (  flag == ZERO_Z )
    {
        tan.set_z( 0.0 );
    }
    else if (  flag == ONLY_BACK )
    {
        tan = pVec[ind] - pVec[ind - 1];
    }
    else if (  flag == ONLY_FORWARD )
    {
        tan = pVec[ind + 1] - pVec[ind];
    }

    tan.normalize();

}
