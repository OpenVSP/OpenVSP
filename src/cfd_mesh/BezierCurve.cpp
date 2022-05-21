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
#include "VspUtil.h"

#include "eli/geom/curve/length.hpp"

typedef piecewise_curve_type::index_type curve_index_type;
typedef piecewise_curve_type::tolerance_type curve_tolerance_type;
typedef piecewise_curve_type::bounding_box_type curve_bounding_box_type;
typedef piecewise_curve_type::point_type curve_point_type;

typedef eli::geom::curve::piecewise_linear_creator<double, 3, curve_tolerance_type> piecewise_linear_creator_type;

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

void Bezier_curve::FlipCurve()
{
    m_Curve.reverse();
}

void Bezier_curve::BuildWakeTECurve( const piecewise_curve_type& lecrv, double endx, double angle, double start_stretch_x, double scale )
{
    m_Curve.clear();

    double factor = scale - 1.0;
    int nsect = lecrv.number_segments();

    for ( int i = 0; i < nsect; i++ )
    {
        curve_segment_type c;
        lecrv.get( c, i );

        for ( int j = 0; j <= c.degree(); j++ )
        {
            curve_point_type le = c.get_control_point( j );
            curve_point_type te = ComputeWakeTrailEdgePnt( le, endx, angle );
            double numer = te.x() - start_stretch_x;
            double fract = numer / ( endx - start_stretch_x );
            double xx = start_stretch_x + numer * ( 1.0 + factor * fract * fract );
            curve_point_type newpt = ComputeWakeTrailEdgePnt( te, xx, angle );
            c.set_control_point( newpt, j );
        }

        m_Curve.push_back( c );
    }
}

curve_point_type Bezier_curve::ComputeWakeTrailEdgePnt( const curve_point_type &pnt, double endx, double angle )
{
    curve_point_type wkpnt;
    double z = pnt.z() + ( endx - pnt.x() ) * tan( DEG2RAD( angle ) );
    wkpnt << endx, pnt.y(), z;
    return wkpnt;
}

void Bezier_curve::GetControlPoints( vector< vec3d > &pnts_out )
{
    pnts_out.clear();

    int num_sections = m_Curve.number_segments();

    for ( int i = 0; i < num_sections; i++ )
    {
        curve_segment_type c;

        m_Curve.get( c, i );

        for ( int j = 0; j < c.degree() + 1; j++ )
        {
            curve_point_type cp = c.get_control_point( j );
            pnts_out.emplace_back( vec3d( cp ) );
        }
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

//===== Tesselate =====//
void Bezier_curve::TessAdaptXYZ( const Surf& srf, vector< vec3d >& output, double tol, int Nlimit )
{
    vector< double > uvec; // Ignore returned values
    TessAdaptXYZ( srf, m_Curve.get_parameter_min(), m_Curve.get_parameter_max(), output, tol, Nlimit, uvec );
}

void Bezier_curve::TessAdaptXYZ( const Surf &srf, vector< vec3d > & output, double tol, int Nlimit, vector< double >& uvec )
{
    TessAdaptXYZ( srf, m_Curve.get_parameter_min(), m_Curve.get_parameter_max(), output, tol, Nlimit, uvec );
}

void Bezier_curve::TessAdaptXYZ( const Surf &srf, double umin, double umax, std::vector< vec3d > & pnts, double tol, int Nlimit, std::vector< double >& uvec )
{
    vec3d uwmin = m_Curve.f( umin );
    vec3d uwmax = m_Curve.f( umax );
    vec3d pmin = srf.CompPnt( uwmin.x(), uwmin.y() );
    vec3d pmax = srf.CompPnt( uwmax.x(), uwmax.y() );

    TessAdaptXYZ( srf, umin, umax, pmin, pmax, pnts, tol, Nlimit, uvec );

    pnts.push_back( pmax );
    uvec.push_back( umax );
}

void Bezier_curve::TessAdaptXYZ( const Surf &srf, double umin, double umax, const vec3d & pmin, const vec3d & pmax, std::vector< vec3d > & pnts, double tol, int Nlimit, std::vector< double >& uvec, int Nadapt )
{
    double umid = ( umin + umax ) * 0.5;

    vec3d uwmid = m_Curve.f( umid );
    vec3d pmid = srf.CompPnt( uwmid.x(), uwmid.y() );

    double d = dist_pnt_2_line( pmin, pmax, pmid ) / dist( pmin, pmax );

    if ( ( d > tol && Nlimit > 0 ) || Nadapt < 3 )
    {
        TessAdaptXYZ( srf, umin, umid, pmin, pmid, pnts, tol, Nlimit - 1, uvec, Nadapt + 1 );
        TessAdaptXYZ( srf, umid, umax, pmid, pmax, pnts, tol, Nlimit - 1, uvec, Nadapt + 1 );
    }
    else
    {
        pnts.push_back( pmin );
        pnts.push_back( pmid );
        uvec.push_back( umin );
        uvec.push_back( umid );
    }
}

//===== Interpolate Creates piecewise linear curves ===//
void Bezier_curve::InterpolateLinear( const vector< vec3d > & input_pnt_vec )
{
    // copy points over to new type
    vector<curve_point_type> pts( input_pnt_vec.size() );
    for ( size_t i = 0; i < pts.size(); ++i )
    {
        pts[i] << input_pnt_vec[i].x(), input_pnt_vec[i].y(), input_pnt_vec[i].z();
    }

    int nseg( pts.size() - 1 );
    piecewise_linear_creator_type plc( nseg );

    // set the polygon corners
    for ( curve_index_type i = 0; i < static_cast<curve_index_type>( pts.size() ); ++i )
    {
        plc.set_corner( pts[i], i );
    }

    if ( !plc.create( m_Curve ) )
    {
        std::cerr << "Failed to create linear curve. " << __LINE__ << std::endl;
    }
}

void Bezier_curve::PromoteTo( int deg )
{
    m_Curve.degree_promote_to( deg );
}
