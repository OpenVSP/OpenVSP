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

#include "eli/geom/curve/length.hpp"
#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/intersect/minimum_distance_curve.hpp"

typedef piecewise_curve_type::index_type curve_index_type;
typedef piecewise_curve_type::point_type curve_point_type;
typedef piecewise_curve_type::rotation_matrix_type curve_rotation_matrix_type;
typedef piecewise_curve_type::bounding_box_type curve_bounding_box_type;
typedef piecewise_curve_type::tolerance_type curve_tolerance_type;

typedef eli::geom::curve::piecewise_cubic_spline_creator<double, 3, curve_tolerance_type> piecewise_cubic_spline_creator_type;
typedef eli::geom::curve::piecewise_linear_creator<double, 3, curve_tolerance_type> piecewise_linear_creator_type;


//===== Constructor  =====//
Bezier_curve::Bezier_curve()
{
}

//===== Destructor  =====//
Bezier_curve::~Bezier_curve()
{
}

//===== Compute Point  =====//
vec3d Bezier_curve::comp_pnt( double u )
{
    vec3d rtn;

    // Code-Eli does not handle out-of-bounds very well.
    if ( u > 1.0 )
        u = 1.0;

    curve_point_type v( m_Curve.f( u * m_Curve.number_segments() ) );
    rtn.set_xyz( v.x(), v.y(), v.z() );

    return rtn;
}

void Bezier_curve::buildCurve( const vector< vec3d > & pVec )
{
    double tanStr = 0.3;
    int closeFlag = 0;
    if ( pVec.size() < 2 )
    {
        return;
    }

    int i;
    vec3d tan;

    //==== Allocate Space ====//
    int num_sections = pVec.size() - 1;

    vector < vec3d > pnts;

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

    pnts[ind + 1] = pVec[pVec.size() - 1];

    // Assign control points to Code-Eli curve.
    put_pnts( pnts );

}

void Bezier_curve::flipCurve()
{
    m_Curve.reverse();
}

void Bezier_curve::put_pnts( const vector< vec3d > &pnts_in )
{
    int npts = pnts_in.size();
    int num_sections = ( npts - 1 ) / 3;

    m_Curve.clear();
    m_Curve.set_t0( 0.0 );

    for ( int i = 0; i < num_sections; i++ )
    {
        curve_segment_type c;
        c.resize( 3 );

        for ( int j = 0; j < 4; j++ )
        {
            vec3d p = pnts_in[ j + ( i * 3 ) ];
            curve_point_type cp( p.x(), p.y(), p.z() );
            c.set_control_point( cp, j );
        }
        m_Curve.push_back( c );
    }
}

void Bezier_curve::get_pnts( vector< vec3d > &pnts_out )
{
    int nsect = m_Curve.number_segments();
    pnts_out.resize( nsect * 3 + 1 );

    for ( int i = 0; i < nsect; i++ )
    {
        curve_segment_type c;
        m_Curve.get( c, i );

        for ( int j = 0; j < 4; j++ )
        {
            curve_point_type cp = c.get_control_point( j );
            vec3d p( cp.x(), cp.y(), cp.z() );
            pnts_out[ j + ( i * 3 ) ] = p;
        }
    }
}

vec3d Bezier_curve::first_pnt()  // Could be implemented with comp_pnt, but should be faster/more accurate.
{
    curve_segment_type c;
    m_Curve.get( c, 0 );
    curve_point_type cp = c.get_control_point( 0 );
    vec3d p( cp.x(), cp.y(), cp.z() );
    return p;
}

vec3d Bezier_curve::last_pnt()
{
    curve_segment_type c;
    m_Curve.get( c, m_Curve.number_segments() - 1 );
    curve_point_type cp = c.get_control_point( c.degree() );
    vec3d p( cp.x(), cp.y(), cp.z() );
    return p;
}
