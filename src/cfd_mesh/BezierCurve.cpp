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
#include <algorithm>

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

void Bezier_curve::buildCurve( const vector< vec3d > & pVec, double tanStr, int closeFlag )
{
    if ( pVec.size() < 2 )
    {
        return;
    }

    int i;
    vec3d tan;

    //==== Allocate Space ====//
    num_sections = pVec.size() - 1;
    pnts.resize( num_sections * 3 + 1 );

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

void Bezier_curve::flipCurve()
{
    std::reverse( pnts.begin(), pnts.end() );
}

void Bezier_curve::put_pnts( const vector< vec3d > &pnts_in )
{
    int npts = pnts_in.size();
    num_sections = ( npts - 1 ) / 3;
    pnts = pnts_in;
}
