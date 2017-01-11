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
#include "Util.h"

#include "eli/geom/curve/length.hpp"

typedef piecewise_curve_type::index_type curve_index_type;
typedef piecewise_curve_type::tolerance_type curve_tolerance_type;
typedef piecewise_curve_type::bounding_box_type curve_bounding_box_type;

//===== Constructor  =====//
Bezier_curve::Bezier_curve()
{
}


Bezier_curve::Bezier_curve( const piecewise_curve_type &crv )
{
    m_Curve = crv;
}

//===== Destructor  =====//
Bezier_curve::~Bezier_curve()
{
}

//===== Compute Point  =====//
vec3d Bezier_curve::CompPnt01( double u ) const
{
    vec3d rtn;

    // Code-Eli does not handle out-of-bounds very well.
    if ( u > 1.0 )
        u = 1.0;

    curve_point_type v( m_Curve.f( u * m_Curve.number_segments() ) );
    rtn.set_xyz( v.x(), v.y(), v.z() );

    return rtn;
}

void Bezier_curve::BuildCurve( const vector< vec3d > & pVec, double tanStr )
{
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
    PutControlPoints( pnts );
}

void Bezier_curve::FlipCurve()
{
    m_Curve.reverse();
}

void Bezier_curve::BuildWakeTECurve( const Bezier_curve &lecrv, double endx, double angle )
{
    m_Curve = lecrv.m_Curve;

    int nsect = m_Curve.number_segments();

    for ( int i = 0; i < nsect; i++ )
    {
        curve_segment_type c;
        m_Curve.get( c, i );

        for ( int j = 0; j <= c.degree(); j++ )
        {
            curve_point_type cp = c.get_control_point( j );
            curve_point_type newpt = ComputeWakeTrailEdgePnt( cp, endx, angle );
            c.set_control_point( newpt, j );
        }
        m_Curve.replace( c, i );
    }
}

curve_point_type Bezier_curve::ComputeWakeTrailEdgePnt( const curve_point_type &pnt, double endx, double angle )
{
    curve_point_type wkpnt;
    double z = pnt.z() + ( endx - pnt.x() ) * tan( DEG2RAD( angle ) );
    wkpnt << endx, pnt.y(), z;
    return wkpnt;
}

void Bezier_curve::PutControlPoints( const vector< vec3d > &pnts_in )
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

vec3d Bezier_curve::FirstPnt() const  // Could be implemented with comp_pnt, but should be faster/more accurate.
{
    curve_segment_type c;
    m_Curve.get( c, 0 );
    curve_point_type cp = c.get_control_point( 0 );
    vec3d p( cp.x(), cp.y(), cp.z() );
    return p;
}

vec3d Bezier_curve::LastPnt() const
{
    curve_segment_type c;
    m_Curve.get( c, m_Curve.number_segments() - 1 );
    curve_point_type cp = c.get_control_point( c.degree() );
    vec3d p( cp.x(), cp.y(), cp.z() );
    return p;
}

void Bezier_curve::UWCurveToXYZCurve( const Surf *srf )
{
    int nsect = m_Curve.number_segments();

    piecewise_curve_type newcurve;
    newcurve.set_t0( m_Curve.get_t0() );

    for ( int i = 0; i < nsect; i++ )
    {
        curve_segment_type c;
        double dt;
        m_Curve.get( c, dt, i );

        for ( int j = 0; j <= c.degree(); j++ )
        {
            curve_point_type cp = c.get_control_point( j );
            vec3d newpt = srf->CompPnt( cp.x(), cp.y() );
            cp << newpt.x(), newpt.y(), newpt.z();
            c.set_control_point( cp, j );
        }
        newcurve.push_back( c, dt );
    }
    m_Curve = newcurve;
}

void Bezier_curve::XYZCurveToUWCurve( const Surf *srf )
{
    int nsect = m_Curve.number_segments();

    piecewise_curve_type newcurve;
    newcurve.set_t0( m_Curve.get_t0() );

    for ( int i = 0; i < nsect; i++ )
    {
        curve_segment_type c;
        double dt;
        m_Curve.get( c, dt, i );

        for ( int j = 0; j <= c.degree(); j++ )
        {
            curve_point_type cp = c.get_control_point( j );
            vec3d p = vec3d( cp.x(), cp.y(), cp.z() );
            vec2d uw = srf->ClosestUW( p, srf->GetSurfCore()->GetMidU(), srf->GetSurfCore()->GetMidW() );
            cp << uw.x(), uw.y(), 0.0;
            c.set_control_point( cp, j );
        }
        newcurve.push_back( c, dt );
    }
    m_Curve = newcurve;
}

int Bezier_curve::CountMatch( const Bezier_curve &ocrv, double tol ) const
{
    int num_match = 0;

    int nsect = m_Curve.number_segments();

    if ( nsect != ocrv.m_Curve.number_segments() )
    {
        return -1;
    }

    for ( int i = 0; i < nsect; i++ )
    {
        curve_segment_type cA, cB;
        m_Curve.get( cA, i );
        ocrv.m_Curve.get( cB, i );

        if ( cA.degree() != cB.degree() )
        {
            return -1;
        }

        for ( int j = 0; j <= cA.degree(); j++ )
        {
            curve_point_type cpA = cA.get_control_point( j );
            curve_point_type cpB = cB.get_control_point( j );

            if ( eli::geom::point::distance( cpA, cpB ) < tol )
            {
                num_match++;
            }
        }
    }
    return num_match;
}

bool Bezier_curve::Match( const Bezier_curve &ocrv, double tol ) const
{
    if ( MatchFwd( ocrv, tol ) )
        return true;

    if ( MatchBkwd( ocrv, tol ) )
        return true;

    return false;
}

bool Bezier_curve::MatchFwd( const Bezier_curve &ocrv, double tol ) const
{
    int nsect = m_Curve.number_segments();

    if ( nsect != ocrv.m_Curve.number_segments() )
        return false;

    for ( int i = 0; i < nsect; i++ )
    {
        curve_segment_type cA, cB;
        m_Curve.get( cA, i );
        ocrv.m_Curve.get( cB, i );

        if ( cA.degree() != cB.degree() )
        {
            return false;
        }

        for ( int j = 0; j <= cA.degree(); j++ )
        {
            curve_point_type cpA = cA.get_control_point( j );
            curve_point_type cpB = cB.get_control_point( j );

            if ( eli::geom::point::distance( cpA, cpB ) > tol )
            {
                return false;
            }
        }
    }

    return true;
}

bool Bezier_curve::MatchBkwd( const Bezier_curve &ocrv, double tol ) const
{
    Bezier_curve revcrv = ocrv;
    revcrv.FlipCurve();
    return MatchFwd( revcrv, tol );
}

bool Bezier_curve::SingleLinear()
{
    int nsect = m_Curve.number_segments();

    if ( nsect != 1 )
    {
        return false;
    }

    for ( int i = 0; i < nsect; i++ )
    {
        curve_segment_type cA;
        m_Curve.get( cA, i );

        if ( cA.degree() != 1 )
        {
            return false;
        }
    }
    return true;
}

void Bezier_curve::GetBBox( BndBox &box )
{
    box.Reset();

    curve_bounding_box_type bbox;
    m_Curve.get_bounding_box( bbox );

    box.Update( bbox.get_max() );
    box.Update( bbox.get_min() );
}
