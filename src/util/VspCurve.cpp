//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspCurve.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES
#include <cmath>

#include <cstdio>
#include <cfloat>
#include <limits>

#include "VspCurve.h"
#include "APIDefines.h"
#include "StlHelper.h"

#include "eli/geom/curve/length.hpp"
#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/intersect/specified_distance_curve.hpp"
#include "eli/geom/intersect/specified_thickness_curve.hpp"
#include "eli/geom/intersect/minimum_dimension_curve.hpp"
#include "eli/geom/intersect/one_d_curve_solver.hpp"

typedef piecewise_curve_type::index_type curve_index_type;
typedef piecewise_curve_type::point_type curve_point_type;
typedef piecewise_curve_type::rotation_matrix_type curve_rotation_matrix_type;
typedef piecewise_curve_type::bounding_box_type curve_bounding_box_type;
typedef piecewise_curve_type::tolerance_type curve_tolerance_type;

typedef eli::geom::curve::bezier<double, 1> oned_curve_segment_type;
typedef eli::geom::curve::piecewise<eli::geom::curve::bezier, double, 1> oned_piecewise_curve_type;
typedef oned_piecewise_curve_type::point_type oned_curve_point_type;


typedef eli::geom::curve::piecewise_point_creator<double, 3, curve_tolerance_type> piecewise_point_creator;
typedef eli::geom::curve::piecewise_cubic_spline_creator<double, 3, curve_tolerance_type> piecewise_cubic_spline_creator_type;
typedef eli::geom::curve::piecewise_linear_creator<double, 3, curve_tolerance_type> piecewise_linear_creator_type;
typedef eli::geom::curve::piecewise_binary_cubic_creator<double, 3, curve_tolerance_type> piecewise_binary_cubic_creator;
typedef eli::geom::curve::piecewise_binary_cubic_cylinder_projector<double, 3, curve_tolerance_type> piecewise_binary_cubic_cylinder_projector;
typedef eli::geom::curve::piecewise_binary_cubic_sphere_projector<double, 3, curve_tolerance_type> piecewise_binary_cubic_sphere_projector;
//=============================================================================//
//============================= VspCurve      =================================//
//=============================================================================//

//===== Constructor  =====//
VspCurve::VspCurve()
{
}

//===== Destructor  =====//
VspCurve::~VspCurve()
{
}

//==== Copy From Input Curve =====//
void VspCurve::Copy( const VspCurve & input_crv )
{
    m_Curve = input_crv.m_Curve;
}

//==== Split at Specified Parameter and Return Remaining Curve =====//
void VspCurve::Split( double u )
{
    m_Curve.split( u );
}

// cli integrand from Riegels.
// Riegels uses a slightly different transformation of x than Theory of Wing Sections
// and some other sources.
struct designcl_functor
{
    double operator()( const double &t )
    {
        vec3d p = m_crv->CompPnt( t );
        double x = p.x();
        double y = p.y();

        vec3d pprm = m_crv->CompTan( t );
        double dxdt = pprm.x();

        double phi = acos( 2.0 * x - 1.0 );
        double dphidx = -1.0 / sqrt( -x * ( x - 1.0 ) );

        double sinphi = sin( phi );

        return ( 4.0 * y / ( sinphi * sinphi ) ) * dphidx * dxdt ;
    }
    VspCurve *m_crv;
};

// Use thin airfoil theory to calculate cli for a camber line decomposed from an
// airfoil curve.
//
// The camber line produced by Decompose() is an approximation to the rigorous
// fluid dynamic definition.  Consequently, the design lift coefficient calculated
// from it will also be approximate.
//
// VspCurve camber, halfthick;
// m_OrigCurve.Decompose( camber, halfthick );
// double cli = camber.IntegrateCamberCli();
// printf( "cli %f\n", cli );

double VspCurve::IntegrateCamberCli()
{
    // Integral is singular at limits, skip them.
    double eps = 1e-6;
    double tmin = m_Curve.get_t0() + eps;
    double tmax = m_Curve.get_tmax() - eps;

    designcl_functor fun;
    fun.m_crv = this;

    eli::mutil::quad::simpson< double > quad;

    return quad( fun, tmin, tmax );
}

double VspCurve::Decompose( VspCurve &camber, VspCurve &halfthick )
{
    double ttol = 1e-6;
    double atol = 0.01; // Ignore value, not using corner_create
    int dmin = 2;
    int dmax = 12;

    piecewise_binary_cubic_creator pbcc;

    double tmin, tmax, tmid;
    tmin = m_Curve.get_parameter_min();
    tmax = m_Curve.get_parameter_max();
    tmid = ( tmin + tmax ) / 2.0;

    piecewise_curve_type low, up;

    m_Curve.split( low, up, tmid );

    // Setup copies base curve into creator.
    // tolerance, min adapt levels, max adapt levels
    pbcc.setup( low, ttol, atol, dmin, dmax );
    // Create makes new curve
    pbcc.create( low ); // Not using corner_create

    pbcc.setup( up, ttol, atol, dmin, dmax );
    pbcc.create( up ); // Not using corner_create

    up.reverse();

    // Shift up so parameters align.
    up.set_t0( low.get_t0() );

    MatchParameterSplits( low, up );

    piecewise_curve_type cam, deltaup;

    // cam = 0.5 * ( low + up );
    cam.scaledsum( 0.5, low, 0.5, up );

    // deltaup = up - cam;
    deltaup.scaledsum( 1.0, up, -1.0, cam );

    camber.SetCurve( cam );
    halfthick.SetCurve( deltaup );

    // Calculate maximum thickness of airfoil
    piecewise_curve_type deltasq;

    deltasq.square( deltaup );

    typedef piecewise_curve_type::onedpiecewisecurve onedpwc;
    onedpwc sumsq;

    sumsq = deltasq.sumcompcurve();

    // Negate to allow minimization instead of maximization.
    sumsq.scale( -1.0 );

    double utmax;
    double tocmax = 2.0 * sqrt( -1.0 * eli::geom::intersect::minimum_dimension( utmax, sumsq, 0 ) );

    return tocmax;
}

void VspCurve::Compose( const VspCurve &camber, const VspCurve &halfthick, const double &scale )
{
    m_Curve.clear();

    piecewise_curve_type cam, deltaup;
    cam = camber.GetCurve();
    deltaup = halfthick.GetCurve();

    // low = cam - scale * deltaup;
    m_Curve.scaledsum( 1.0, cam, -scale, deltaup );

    piecewise_curve_type up;
    // up  = cam + scale * deltaup;
    up.scaledsum( 1.0, cam, scale, deltaup );
    up.reverse();

    m_Curve.push_back( up );
    m_Curve.set_tmax( 4.0 );
}

// Shift curve parameter to start curve at u
// Assumes u is between umin, umax
void VspCurve::Spin( double u )
{
    double umin, umax;
    umax = m_Curve.get_parameter_max();
    umin = m_Curve.get_parameter_min();

    if ( u <= umin || u >= umax )
    {
        return;
    }

    piecewise_curve_type c1, c2;
    m_Curve.split( c1, c2, u );

    c1.set_t0( umin + umax - u );  // This is likely unneeded
    c2.set_t0( umin );

    c2.push_back( c1 );
    c2.set_tmax( umax ); // Should be redundant, but set in case of floating point error accumulation.
    m_Curve = c2;
}

// Shift curve parameter to start curve at u
// Negative u will shift the other way.
void VspCurve::Spin01( double u )
{
    // Force u positive by wrapping around.
    if ( u < 0 )
    {
        u = 1.0 + u;
    }

    Spin( u * m_Curve.get_tmax()  );
}

//==== Append Curve To Existing Curve ====//
void VspCurve::Append( const VspCurve & input_crv )
{
    curve_index_type i, nc( input_crv.GetNumSections() );

    for ( i = 0; i < nc; ++i )
    {
        piecewise_curve_type::error_code ec;
        curve_segment_type c;

        input_crv.GetCurveSegment( c, i );
        ec = m_Curve.push_back( c, input_crv.GetCurveDt( i ) );
        if ( ec != piecewise_curve_type::NO_ERRORS )
        {
            std::cerr << "Could not append curve." << std::endl;
        }
    }
}

bool VspCurve::IsClosed() const
{
    return m_Curve.closed();
}

#if 0
void octave_print( int figno, const piecewise_curve_type &pc )
{
    curve_index_type i, pp, ns;
    double tmin, tmax;

    ns = pc.number_segments();
    pc.get_parameter_min( tmin );
    pc.get_parameter_max( tmax );

    std::cout << "figure(" << figno << ");" << std::endl;

    // get control points and print
    std::cout << "cp_x=[";
    for ( pp = 0; pp < ns; ++pp )
    {
        curve_segment_type bez;
        pc.get( bez, pp );
        for ( i = 0; i <= bez.degree(); ++i )
        {
            std::cout << bez.get_control_point( i ).x();
            if ( i < bez.degree() )
            {
                std::cout << ", ";
            }
            else if ( pp < ns - 1 )
            {
                std::cout << "; ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "];" << std::endl;

    std::cout << "cp_y=[";
    for ( pp = 0; pp < ns; ++pp )
    {
        curve_segment_type bez;
        pc.get( bez, pp );
        for ( i = 0; i <= bez.degree(); ++i )
        {
            std::cout << bez.get_control_point( i ).y();
            if ( i < bez.degree() )
            {
                std::cout << ", ";
            }
            else if ( pp < ns - 1 )
            {
                std::cout << "; ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "];" << std::endl;

    std::cout << "cp_z=[";
    for ( pp = 0; pp < ns; ++pp )
    {
        curve_segment_type bez;
        pc.get( bez, pp );
        for ( i = 0; i <= bez.degree(); ++i )
        {
            std::cout << bez.get_control_point( i ).z();
            if ( i < bez.degree() )
            {
                std::cout << ", ";
            }
            else if ( pp < ns - 1 )
            {
                std::cout << "; ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "];" << std::endl;

    // initialize the t parameters
    std::vector<double> t( 129 );
    for ( i = 0; i < static_cast<curve_index_type>( t.size() ); ++i )
    {
        t[i] = tmin + ( tmax - tmin ) * static_cast<double>( i ) / ( t.size() - 1 );
    }

    // set the surface points
    std::cout << "surf_x=[";
    for ( i = 0; i < static_cast<curve_index_type>( t.size() ); ++i )
    {
        std::cout << pc.f( t[i] ).x();
        if ( i < static_cast<curve_index_type>( t.size() - 1 ) )
        {
            std::cout << ", ";
        }
    }
    std::cout << "];" << std::endl;

    std::cout << "surf_y=[";
    for ( i = 0; i < static_cast<curve_index_type>( t.size() ); ++i )
    {
        std::cout << pc.f( t[i] ).y();
        if ( i < static_cast<curve_index_type>( t.size() - 1 ) )
        {
            std::cout << ", ";
        }
    }
    std::cout << "];" << std::endl;

    std::cout << "surf_z=[";
    for ( i = 0; i < static_cast<curve_index_type>( t.size() ); ++i )
    {
        std::cout << pc.f( t[i] ).z();
        if ( i < static_cast<curve_index_type>( t.size() - 1 ) )
        {
            std::cout << ", ";
        }
    }
    std::cout << "];" << std::endl;

    std::cout << "setenv('GNUTERM', 'x11');" << std::endl;
    std::cout << "plot3(surf_x, surf_y, surf_z, '-k');" << std::endl;
    std::cout << "hold on;" << std::endl;
    std::cout << "plot3(cp_x', cp_y', cp_z', '-ok', 'MarkerFaceColor', [0 0 0]);" << std::endl;
    std::cout << "hold off;" << std::endl;
}
#endif

bool VspCurve::RoundJoint( double rad, int i )
{
    return m_Curve.round( rad, i );
}

bool VspCurve::RoundJoint( double rad, double u )
{
    vector < double > umap;
    m_Curve.get_pmap( umap );

    int irnd = vector_find_val( umap, u );
    if ( irnd > 0 )
    {
        return RoundJoint( rad, irnd );
    }
    return false;
}

bool VspCurve::RoundJoints( const vector < double > &r_vec, const vector < int > &ivec )
{
    bool retval = true; // Assume all success
    for ( int i = r_vec.size() - 1; i >= 0; i-- ) // Loop backwards to preserve indexing.
    {
        bool ret = RoundJoint( r_vec[ i ], ivec[ i ] );

        if ( !ret ) // Detect any failure.
        {
            retval = false;
        }
    }
    return retval;
}

bool VspCurve::RoundJoints( double rad, const vector < double > &uvec )
{
    vector < double > umap;

    bool retval = true; // Assume all success
    bool ret;

    for ( int i = 0; i < uvec.size(); i++ )
    {
        // Allowing shifting of the curve start means some rounding can wrap-around the start/end point, thereby making
        // it impossible to guarantee that the first parts of umap go unchanged.  Further complexity is not worthwhile,
        // it is best to get get_pmap every time.
        m_Curve.get_pmap( umap );
        int irnd = vector_find_val( umap, uvec[i], 1e-8 );
        if ( irnd >= 0 )
        {
            ret = RoundJoint( rad, irnd );
        }
        else
        {
            ret = false;
        }

        if ( !ret ) // Detect any failure.
        {
            retval = false;
        }
    }
    return retval;
}

void VspCurve::Modify( int type, bool le, double len, double off, double str )
{
    int elitype = 0;

    double tmid = m_Curve.get_t0();

    if ( le )
    {
        tmid = ( m_Curve.get_t0() + m_Curve.get_tmax() ) * 0.5;
    }

    switch ( type )
    {
    case vsp::ROUND_END_CAP:
        elitype = piecewise_curve_type::ROUND;
        break;
    case vsp::EDGE_END_CAP:
        elitype = piecewise_curve_type::EDGE;
        break;
    case vsp::SHARP_END_CAP:
        elitype = piecewise_curve_type::SHARP;
        break;
    case vsp::FLAT_END_CAP:
    default:
        // Do nothing, already flat.
        return; //elitype = piecewise_curve_type::FLAT;
        break;
    }

    m_Curve.modify( elitype, tmid, TMAGIC, len, off, str );
}

//===== Interpolate Creates piecewise linear curves ===//
void VspCurve::InterpolateLinear(const vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag )
{
    // do some checking of vector lengths
    if ( closed_flag )
    {
        if ( param.size() != ( input_pnt_vec.size() + 1 ) )
        {
            std::cerr << "Invalid number of points and parameters in curve interpolation " << __LINE__ << std::endl;
            assert( false );
            return;
        }
    }
    else
    {
        if ( param.size() != input_pnt_vec.size() )
        {
            std::cerr << "Invalid number of points and parameters in curve interpolation " << __LINE__ << std::endl;
            assert( false );
            return;
        }
    }

    // copy points over to new type
    vector<curve_point_type> pts( input_pnt_vec.size() );
    for ( size_t i = 0; i < pts.size(); ++i )
    {
        pts[i] << input_pnt_vec[i].x(), input_pnt_vec[i].y(), input_pnt_vec[i].z();
    }

    if ( closed_flag )
    {
        pts.push_back( pts[0] );
    }

    int nseg( pts.size() - 1 );
    piecewise_linear_creator_type plc( nseg );

    // set the delta t for each curve segment
    plc.set_t0( param[0] );
    for ( curve_index_type i = 0; i < plc.get_number_segments(); ++i )
    {
        plc.set_segment_dt( param[i + 1] - param[i], i );
    }

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

//===== Interpolate Creates PCHIP ====//
void VspCurve::InterpolatePCHIP( const vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag )
{
    // do some checking of vector lengths
    if ( closed_flag )
    {
        if ( param.size() != ( input_pnt_vec.size() + 1 ) )
        {
            std::cerr << "Invalid number of points and parameters in curve interpolation " << __LINE__ << std::endl;
            assert( false );
            return;
        }
    }
    else
    {
        if ( param.size() != input_pnt_vec.size() )
        {
            std::cerr << "Invalid number of points and parameters in curve interpolation " << __LINE__ << std::endl;
            assert( false );
            return;
        }
    }

    // copy points over to new type
    vector<curve_point_type> pts( input_pnt_vec.size() );
    for ( size_t i = 0; i < pts.size(); ++i )
    {
        pts[i] << input_pnt_vec[i].x(), input_pnt_vec[i].y(), input_pnt_vec[i].z();
    }

    // create creator for known number of segments
    int nseg( pts.size() - 1 );
    if ( closed_flag )
    {
        ++nseg;
    }
    piecewise_cubic_spline_creator_type pcsc( nseg );

    // set the delta t for each curve segment
    pcsc.set_t0( param[0] );
    for ( size_t i = 0; i < ( param.size() - 1 ); ++i )
    {
        pcsc.set_segment_dt( param[i + 1] - param[i], i );
    }

    if ( closed_flag )
    {
        pcsc.set_chip( pts.begin(), eli::geom::general::C1 );
    }
    else
    {
        pcsc.set_chip( pts.begin(), eli::geom::general::NOT_CONNECTED );
    }

    if ( !pcsc.create( m_Curve ) )
    {
        std::cerr << "Failed to create PCHIP. " << __LINE__ << std::endl;
    }
}

//===== Interpolate Creates cubic spline with either not-a-knot ends or closed ends  ====//
void VspCurve::InterpolateCSpline( const vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag )
{
    // do some checking of vector lengths
    if ( closed_flag )
    {
        if ( param.size() != ( input_pnt_vec.size() + 1 ) )
        {
            std::cerr << "Invalid number of points and parameters in curve interpolation " << __LINE__ << std::endl;
            assert( false );
            return;
        }
    }
    else
    {
        if ( param.size() != input_pnt_vec.size() )
        {
            std::cerr << "Invalid number of points and parameters in curve interpolation " << __LINE__ << std::endl;
            assert( false );
            return;
        }
    }

    // copy points over to new type
    vector<curve_point_type> pts( input_pnt_vec.size() );
    for ( size_t i = 0; i < pts.size(); ++i )
    {
        pts[i] << input_pnt_vec[i].x(), input_pnt_vec[i].y(), input_pnt_vec[i].z();
    }

    // create creator for known number of segments
    int nseg( pts.size() - 1 );
    if ( closed_flag )
    {
        ++nseg;
    }
    piecewise_cubic_spline_creator_type pcsc( nseg );

    // set the delta t for each curve segment
    pcsc.set_t0( param[0] );
    for ( size_t i = 0; i < ( param.size() - 1 ); ++i )
    {
        pcsc.set_segment_dt( param[i + 1] - param[i], i );
    }

    if ( closed_flag )
    {
        pcsc.set_closed_cubic_spline( pts.begin() );
    }
    else
    {
        pcsc.set_cubic_spline( pts.begin() );
    }

    if ( !pcsc.create( m_Curve ) )
    {
        std::cerr << "Failed to create CSpline. " << __LINE__ << std::endl;
    }
}

//===== Interpolate Creates cubic spline with set end slopes ====//
void VspCurve::InterpolateCSpline( const vector< vec3d > & input_pnt_vec, const vec3d &start_slope, const vec3d &end_slope, const vector<double> &param )
{
    // do some checking of vector lengths
    if ( param.size() != input_pnt_vec.size() )
    {
        std::cerr << "Invalid number of points and parameters in curve interpolation " << __LINE__ << std::endl;
        assert( false );
        return;
    }

    // copy points over to new type
    vector<curve_point_type> pts( input_pnt_vec.size() );
    curve_point_type sslope, eslope;

    for ( size_t i = 0; i < pts.size(); ++i )
    {
        pts[i] << input_pnt_vec[i].x(), input_pnt_vec[i].y(), input_pnt_vec[i].z();
    }
    sslope << start_slope.x(), start_slope.y(), start_slope.z();
    eslope << end_slope.x(), end_slope.y(), end_slope.z();

    // create creator for known number of segments
    piecewise_cubic_spline_creator_type pcsc( pts.size() - 1 );

    // set the delta t for each curve segment
    pcsc.set_t0( param[0] );
    for ( size_t i = 0; i < ( param.size() - 1 ); ++i )
    {
        pcsc.set_segment_dt( param[i + 1] - param[i], i );
    }

    pcsc.set_clamped_cubic_spline( pts.begin(), sslope, eslope );

    if ( !pcsc.create( m_Curve ) )
    {
        std::cerr << "Failed to create CSpline. " << __LINE__ << std::endl;
    }
}

// ttol -- tolerance on gap between curve midpoints
// atol -- angle tolerance used to detect corners before adaptation.  Smooth if:  abs(1-cos(theta)) <= atol
// dmin -- minimum number of divisions of curve
// dmax -- maximum number of divisions of curve
void VspCurve::ToBinaryCubic( bool wingtype, double ttol, double atol, int dmin, int dmax, bool flapflag, double flapT, double flapDT )
{
    piecewise_binary_cubic_creator pbcc;

    double tmin, tmax, tmid;
    tmin = m_Curve.get_parameter_min();
    tmax = m_Curve.get_parameter_max();
    tmid = ( tmin + tmax ) / 2.0;

    if ( wingtype )
    {
        piecewise_curve_type crv, telow, teup, le, low, up, rest;

        m_Curve.split( telow, crv, tmin + TMAGIC );
        crv.split( low, rest, tmid - TMAGIC );
        crv = rest;
        crv.split( le, rest, tmid + TMAGIC );
        crv = rest;
        crv.split( up, teup, tmax - TMAGIC );

        if ( !flapflag )
        {
            // Setup copies base curve into creator.
            // tolerance, min adapt levels, max adapt levels
            pbcc.setup( low, ttol, atol, dmin, dmax );
            // Create makes new curve
            pbcc.corner_create( low );

            pbcc.setup( up, ttol, atol, dmin, dmax );
            pbcc.corner_create( up );
        }
        else
        {
            double tslow = tmin + TMAGIC + 2.0 * flapT;
            double tsup = tmax - TMAGIC - 2.0 * flapT;

            piecewise_curve_type clowrear, clowflap, clowfwd, cupfwd, cupflap, cuprear;
            low.split( clowrear, rest, tslow );
            rest.split( clowflap, clowfwd, tslow + 2.0 * flapDT );

            up.split( cupfwd, rest, tsup - 2.0 * flapDT );
            rest.split( cupflap, cuprear, tsup );

            pbcc.setup( clowrear, ttol, atol, dmin, dmax );
            pbcc.corner_create( clowrear );

            pbcc.setup( clowfwd, ttol, atol, dmin, dmax );
            pbcc.corner_create( clowfwd );

            low = clowrear;
            low.push_back( clowflap );
            low.push_back( clowfwd );
            low.set_tmax( tmid - TMAGIC );

            pbcc.setup( cupfwd, ttol, atol, dmin, dmax );
            pbcc.corner_create( cupfwd );

            pbcc.setup( cuprear, ttol, atol, dmin, dmax );
            pbcc.corner_create( cuprear );

            up = cupfwd;
            up.push_back( cupflap );
            up.push_back( cuprear );
            up.set_tmax( tmax - TMAGIC );
        }

        m_Curve = telow;
        m_Curve.push_back( low );
        m_Curve.push_back( le );
        m_Curve.push_back( up );
        m_Curve.push_back( teup );

        m_Curve.set_tmax( tmax );
    }
    else
    {
        piecewise_curve_type low, up;

        m_Curve.split( low, up, tmid );

        // Setup copies base curve into creator.
        // tolerance, min adapt levels, max adapt levels
        pbcc.setup( low, ttol, atol, dmin, dmax );
        // Create makes new curve
        pbcc.corner_create( m_Curve );

        pbcc.setup( up, ttol, atol, dmin, dmax );
        pbcc.corner_create( up );

        m_Curve.push_back( up );

        m_Curve.set_tmax( tmax );
    }

}

void VspCurve::SetCubicControlPoints( const vector< vec3d > & cntrl_pts )
{
    int ncp = cntrl_pts.size();
    int nseg = ( ncp - 1 ) / 3;

    m_Curve.clear();
    m_Curve.set_t0( 0.0 );

    for ( int i = 0; i < nseg; i++ )
    {
        curve_segment_type c;
        c.resize( 3 );

        for ( int j = 0; j < 4; j++ )
        {
            int k = i * 3 + j;

            const vec3d& p = cntrl_pts[k];

            curve_point_type cp;
            cp << p.x(), p.y(), p.z();

            c.set_control_point( cp, j );
        }
        m_Curve.push_back( c );
    }
}

void VspCurve::SetCubicControlPoints( const vector< vec3d >& cntrl_pts, const vector< double >& param )
{
    int ncp = cntrl_pts.size();
    int nseg = ( ncp - 1 ) / 3;

    assert( ncp == param.size() );

    m_Curve.clear();
    m_Curve.set_t0( param[0] );

    for ( int i = 0; i < nseg; i++ )
    {
        curve_segment_type c;
        c.resize( 3 );

        for ( int j = 0; j < 4; j++ )
        {
            int k = i * 3 + j;

            const vec3d& p = cntrl_pts[k];

            curve_point_type cp;
            cp << p.x(), p.y(), p.z();

            c.set_control_point( cp, j );
        }
        m_Curve.push_back( c, param[3 * ( i + 1 )] - param[3 * i] );
    }
}

void VspCurve::GetCubicControlPoints( vector< vec3d >& cntrl_pts, vector< double >& param )
{
    int nseg = m_Curve.number_segments();

    int ncp = nseg * 3 + 1;

    cntrl_pts.clear();
    param.clear();

    cntrl_pts.reserve( ncp );
    param.reserve( ncp );

    vector < double > pmap;
    m_Curve.get_pmap( pmap );

    for ( size_t i = 0; i < nseg; i++ )
    {
        curve_segment_type c;

        m_Curve.get( c, i );

        for ( size_t j = 0; j < c.degree(); j++ )
        {
            curve_point_type p;
            p = c.get_control_point( j );

            cntrl_pts.push_back( p );
        }

        double tstart = pmap[i];
        double tend = pmap[i + 1];
        double dt = tend - tstart;

        param.push_back( tstart );
        param.push_back( tstart + dt / 3.0 );
        param.push_back( tend - dt / 3.0 );

        if ( i == nseg - 1 )
        {
            curve_point_type p;
            p = c.get_control_point( 3 );

            cntrl_pts.push_back( p );
            param.push_back( tend );
        }
    }
}

void VspCurve::GetLinearControlPoints( vector< vec3d >& cntrl_pts, vector< double >& param )
{
    int nseg = m_Curve.number_segments();

    int ncp = nseg + 1;

    cntrl_pts.clear();
    param.clear();

    cntrl_pts.reserve( ncp );
    param.reserve( ncp );

    m_Curve.get_pmap( param );

    for ( size_t i = 0; i < nseg; i++ )
    {
        curve_segment_type c;

        m_Curve.get( c, i );

        curve_point_type p;
        p = c.get_control_point( 0 );

        cntrl_pts.push_back( p );


        if ( i == nseg - 1 )
        {
            p = c.get_control_point( 1 );

            cntrl_pts.push_back( p );
        }
    }
}

//===== Interpolate ====//
//void VspCurve::Interpolate( vector< vec3d > & ip_vec, vector< VspPntData > & data_vec, bool closed_flag )
//{
//  std::cerr << "Need to implement " << "VspCurve "<< __LINE__<< std::endl;
//
//  double small_num = 1.0e-10;
//  int ip_size = ip_vec.size();
//
//  if ( ip_size < 2 ) return;
//
//  if ( ip_size == 2 )
//  {
//      LinearInterpolate( ip_vec );
//      return;
//  }
//
//  //==== Check Data Vec Size ====//
//  if ( data_vec.size() != ip_size )
//      data_vec.resize( ip_size );
//
//  //==== Resize Control Points ====//
//  m_ControlPnts.resize( 3*(ip_size-1)+1 );
//
//  //==== Compute Tangent Direction Base On Flags =====//
//  ComputeTanDir( ip_vec, data_vec, closed_flag );
//
//  //==== First Point ====//
//  m_ControlPnts[0] = ip_vec[0];
//
//  double mag = (ip_vec[1] - ip_vec[0]).mag();
//  if ( closed_flag )
//  {
//      double mag_minus = ( ip_vec[ip_size-1] - ip_vec[ip_size-2] ).mag();
//      double skip_mag  = ( ip_vec[1] - ip_vec[ip_size-2]).mag();
//      mag = skip_mag*(mag/(mag_minus + mag  + small_num ));
//  }
//
//  m_ControlPnts[1] = ip_vec[0] + data_vec[0].m_Tan2*(mag*m_Tension);
//
//  //==== Middle Points ====//
//  for ( int i = 1 ; i < (int)ip_vec.size()-1 ; i++ )
//  {
//      double mag_minus = ( ip_vec[i]   - ip_vec[i-1] ).mag();
//      double mag_plus  = ( ip_vec[i+1] - ip_vec[i]).mag();
//      double skip_mag  = ( ip_vec[i+1] - ip_vec[i-1]).mag();
//      double mag_sum   = mag_minus + mag_plus + small_num;
//      mag_minus = skip_mag*(mag_minus/mag_sum);
//      mag_plus  = skip_mag*(mag_plus/mag_sum);
//
//      m_ControlPnts[(i-1)*3+2] = ip_vec[i] + data_vec[i].m_Tan1*(mag_minus*m_Tension);
//      m_ControlPnts[(i-1)*3+3] = ip_vec[i];
//      m_ControlPnts[(i-1)*3+4] = ip_vec[i] + data_vec[i].m_Tan2*(mag_plus*m_Tension);
//
//      if ( data_vec[i].m_Type == VspPntData::SHARP )
//      {
//          vec3d dir = m_ControlPnts[(i-1)*3+1] - ip_vec[i];
//          m_ControlPnts[(i-1)*3+2] = ip_vec[i] + dir*m_Tension;
//      }
//      if ( data_vec[i-1].m_Type == VspPntData::SHARP )
//      {
//          vec3d dir = m_ControlPnts[(i-1)*3+2] - ip_vec[i-1];
//          m_ControlPnts[(i-1)*3+1] = ip_vec[i-1] + dir*m_Tension;
//      }
//  }
//
//  //==== Last Point ====//
//  mag = ( ip_vec[ip_size-1] - ip_vec[ip_size-2] ).mag() + 1.0e-10;
//
//  if ( closed_flag )
//  {
//      double mag_plus = ( ip_vec[1] - ip_vec[0] ).mag();
//      double skip_mag  = ( ip_vec[1] - ip_vec[ip_size-2]).mag();
//      mag = skip_mag*(mag/(mag_plus + mag + small_num ));
//  }
//
//  int c_size = m_ControlPnts.size();
//  m_ControlPnts[c_size-2] = ip_vec[ip_size-1] + data_vec[ip_size-1].m_Tan1*(mag*m_Tension);
//  m_ControlPnts[c_size-1] = ip_vec[ip_size-1];
//}

//===== Compute Tangent Directions Using Flags ====//
//void VspCurve::ComputeTanDir( vector< vec3d > & pnt_vec, vector< VspPntData > & data_vec, bool closed_flag )
//{
//  int size = (int)pnt_vec.size();
//  vec3d tan, tan1, tan2;
//  for ( int i = 0 ; i < size ; i++ )
//  {
//      if ( i == 0 || i == size-1 )
//      {
//          if ( closed_flag )  tan = pnt_vec[1] - pnt_vec[size-2];
//          else if ( i == 0 )  tan = pnt_vec[1] - pnt_vec[0];
//          else                tan = pnt_vec[size-1] - pnt_vec[size-2];
//      }
//      else
//      {
//          tan = pnt_vec[i+1] - pnt_vec[i-1];
//      }
//
//      if ( data_vec[i].m_Type == VspPntData::ZERO )               tan = vec3d(0,0,0);
//      else if ( data_vec[i].m_Type == VspPntData::ZERO_X )            tan.set_x(0.0);
//      else if ( data_vec[i].m_Type == VspPntData::ZERO_Y )            tan.set_y(0.0);
//      else if ( data_vec[i].m_Type == VspPntData::ZERO_Z )            tan.set_z(0.0);
//      else if ( data_vec[i].m_Type == VspPntData::ONLY_BACK && i > 0 )
//          tan = pnt_vec[i] - pnt_vec[i-1];
//      else if ( data_vec[i].m_Type == VspPntData::ONLY_FORWARD && i < size-1 )
//          tan = pnt_vec[i+1] - pnt_vec[i];
//      else if ( data_vec[i].m_Type == VspPntData::PREDICT  )
//      {
//          tan.normalize();
//          vec3d predict_dir = tan;
//          if ( i == 0 )
//              predict_dir = pnt_vec[2] - pnt_vec[0];
//          else if ( i == size-1 )
//              predict_dir = pnt_vec[size-1] - pnt_vec[size-3];
//          predict_dir.normalize();
//          tan = tan*2.0 - predict_dir;
//      }
//      tan1 =  tan*-1.0;
//      tan2 =  tan;
//
//      if ( data_vec[i].m_Type == VspPntData::SHARP && i > 0 && i < size-1 )
//      {
//          tan1 = pnt_vec[i-1] - pnt_vec[i];
//          tan2 = pnt_vec[i+1] - pnt_vec[i];
//      }
//
//      tan1.normalize();
//      tan2.normalize();
//
//      if ( !data_vec[i].m_UseTan1 )
//          data_vec[i].m_Tan1 = tan1;
//
//      if ( !data_vec[i].m_UseTan2 )
//          data_vec[i].m_Tan2 = tan2;
//  }
//}

//===== Get Number Of Sections =====//
int VspCurve::GetNumSections() const
{
    return m_Curve.number_segments();
}

const piecewise_curve_type & VspCurve::GetCurve() const
{
    return m_Curve;
}

void VspCurve::SetCurve( const piecewise_curve_type &c )
{
    m_Curve = c;
}

void VspCurve::InterpolateEqArcLenPCHIP( const piecewise_curve_type &c )
{
    const unsigned int npts = 201; // Must be odd to hit LE point.

    double t0 = c.get_parameter_min();
    double tmax = c.get_parameter_max();
    double t = t0;
    double dt = ( tmax - t0 ) / ( npts - 1 );
    int ile = ( npts - 1 ) / 2;

    vector< vec3d > pnts( npts );
    vector< double > arclen( npts );

    pnts[0] = c.f( t );
    arclen[0] = 0.0;
    for ( int i = 1 ; i < npts ; i++ )
    {
        if ( i == ile )
        {
            t = 2.0; // Ensure LE point precision.
        }
        else if ( i == ( npts - 1 ) )
        {
            t = 4.0;  // Ensure end point precision.
        }
        else
        {
            t = dt * i; // All other points.
        }

        pnts[i] = c.f( t );

        double ds = dist( pnts[i], pnts[i-1] );
        if ( ds < 1e-8 )
        {
            ds = 1.0/npts;
        }
        arclen[i] = arclen[i-1] + ds;
    }

    double lenlower = arclen[ile];
    double lenupper = arclen[npts-1] - lenlower;

    double lowerscale = 2.0/lenlower;
    int i;
    for ( i = 1; i < ile; i++ )
    {
        arclen[i] = arclen[i] * lowerscale;
    }
    arclen[ile] = 2.0;
    i++;

    double upperscale = 2.0/lenupper;
    for ( ; i < npts - 1; i++ )
    {
        arclen[i] = 2.0 + ( arclen[i] - lenlower) * upperscale;
    }
    arclen[npts-1] = 4.0;

    InterpolatePCHIP( pnts, arclen, false );
}

void VspCurve::GetCurveSegment( curve_segment_type &c, int i ) const
{
    if ( i < GetNumSections() )
    {
        m_Curve.get( c, i );
    }
}

double VspCurve::GetCurveDt( int i ) const
{
    double dt( -1 );

    if ( i < GetNumSections() )
    {
        curve_segment_type c;

        m_Curve.get( c, dt, i );
    }

    return dt;
}

void VspCurve::AppendCurveSegment( const curve_segment_type &c )
{
    m_Curve.push_back( c, 1 );
}

void VspCurve::MakePoint()
{
    piecewise_curve_type c;
    curve_point_type pt;
    pt << 0, 0, 0;

    // create point with 4 segments
    piecewise_point_creator ppc( 4 );

    // set point, make sure have 4 sections that go from 0 to 4
    ppc.set_point( pt );
    ppc.set_t0( 0 );
    ppc.set_segment_dt( 1, 0 );
    ppc.set_segment_dt( 1, 1 );
    ppc.set_segment_dt( 1, 2 );
    ppc.set_segment_dt( 1, 3 );
    if ( !ppc.create( c ) )
    {
        std::cerr << "Failed to create point curve. " << __LINE__ << std::endl;
    }
    else
    {
        SetCurve( c );
    }
}

double VspCurve::FindDistant( double &u, const vec3d &pt, const double &d, const double &u0 ) const
{
    double dist;
    curve_point_type p;
    p << pt.x(), pt.y(), pt.z();

    dist = eli::geom::intersect::specified_distance( u, m_Curve, p, d, u0 );

    return dist;
}

double VspCurve::FindDistant( double &u, const vec3d &pt, const double &d ) const
{
    double dist;
    curve_point_type p;
    p << pt.x(), pt.y(), pt.z();

    dist = eli::geom::intersect::specified_distance( u, m_Curve, p, d );

    return dist;
}

double VspCurve::FindDistant( double &u, const vec3d &pt, const double &d, const double &umin, const double &umax ) const
{
    double dist;
    curve_point_type p;
    p << pt.x(), pt.y(), pt.z();

    dist = eli::geom::intersect::specified_distance( u, m_Curve, p, d, umin, umax );

    return dist;
}

double VspCurve::FindThickness( double &u1, double &u2, const vec3d &pt, const double &thick, const double &u10, const double &u20,
                                const double &u1min, const double &u1max, const double &u2min, const double &u2max ) const
{
    double dist;
    curve_point_type p;
    p << pt.x(), pt.y(), pt.z();

    dist = eli::geom::intersect::specified_thickness( u1, u2, m_Curve, p, thick, u10, u20,
                                                 u1min, u1max, u2min, u2max );

    return dist;
}

double VspCurve::FindNearest( double &u, const vec3d &pt ) const
{
    double dist;
    curve_point_type p;
    p << pt.x(), pt.y(), pt.z();

    dist = eli::geom::intersect::minimum_distance( u, m_Curve, p );

    return dist;
}

double VspCurve::FindNearest( double &u, const vec3d &pt, const double &u0 ) const
{
    double dist;
    curve_point_type p;
    p << pt.x(), pt.y(), pt.z();

    dist = eli::geom::intersect::minimum_distance( u, m_Curve, p, u0 );

    return dist;
}

double VspCurve::FindNearest01( double &u, const vec3d &pt ) const
{
    double dist;

    dist = FindNearest( u, pt );

    u = u / m_Curve.get_tmax();

    return dist;
}

double VspCurve::FindNearest01( double &u, const vec3d &pt, const double &u0 ) const
{
    double dist;

    dist = FindNearest( u, pt, u0 * m_Curve.get_tmax() );

    u = u / m_Curve.get_tmax();

    return dist;
}

double VspCurve::FindMinimumDimension( double &u, const int &idim, const double &u0 ) const
{
    double r = eli::geom::intersect::minimum_dimension( u, m_Curve, idim, u0 );

    return r;
}

double VspCurve::FindMinimumDimension( double &u, const int &idim ) const
{
    double r = eli::geom::intersect::minimum_dimension( u, m_Curve, idim );

    return r;
}

//===== Compute Point  =====//
vec3d VspCurve::CompPnt( double u ) const
{
    vec3d rtn;
    curve_point_type v( m_Curve.f( u ) );

    rtn.set_xyz( v.x(), v.y(), v.z() );
    return rtn;
}

//===== Compute Tangent  =====//
vec3d VspCurve::CompTan( double u ) const
{
    vec3d rtn;
    curve_point_type v( m_Curve.fp( u ) );

    rtn.set_xyz( v.x(), v.y(), v.z() );
    return rtn;
}

vec3d VspCurve::CompTan( double u, int sideflag ) const
{
    curve_point_type v;

    if ( sideflag == BEFORE )
    {
        piecewise_curve_type c1, c2;

        if ( u > m_Curve.get_parameter_min() )
        {
            m_Curve.split( c1, c2, u );
            v = c1.fp( u );
        }
        else
        {
            v = m_Curve.fp( u );
        }
    }
    else
    {
        piecewise_curve_type c1, c2;

        if ( u < m_Curve.get_parameter_max() )
        {
            m_Curve.split( c1, c2, u );
            v = c2.fp( u );
        }
        else
        {
            v = m_Curve.fp( u );
        }
    }

    vec3d rtn;
    rtn.set_xyz( v.x(), v.y(), v.z() );
    return rtn;
}

vec3d VspCurve::CompNorm( double u) const
{
    vec3d rtn;
    curve_point_type v( m_Curve.fpp( u ) );

    rtn.set_xyz( v.x(), v.y(), v.z() );
    return rtn;
}

vec3d VspCurve::CompNorm( double u, int sideflag ) const
{
    curve_point_type v;

    if ( sideflag == BEFORE )
    {
        piecewise_curve_type c1, c2;

        if ( u > m_Curve.get_parameter_min() )
        {
            m_Curve.split( c1, c2, u );
            v = c1.fpp( u );
        }
        else
        {
            v = m_Curve.fpp( u );
        }
    }
    else
    {
        piecewise_curve_type c1, c2;

        if ( u < m_Curve.get_parameter_max() )
        {
            m_Curve.split( c1, c2, u );
            v = c2.fpp( u );
        }
        else
        {
            v = m_Curve.fpp( u );
        }
    }

    vec3d rtn;
    rtn.set_xyz( v.x(), v.y(), v.z() );
    return rtn;
}

double VspCurve::CompCurve( double u ) const
{
    curve_point_type fp( m_Curve.fp( u ) );
    curve_point_type fpp( m_Curve.fpp( u ) );

    double fpn = fp.norm();
    double denom = fpn * fpn * fpn;
    return ( fp.cross( fpp ) ).norm() / denom;
}

double VspCurve::CompCurve( double u, int sideflag ) const
{
    curve_point_type fp, fpp;

    if ( sideflag == BEFORE )
    {
        piecewise_curve_type c1, c2;

        if ( u > m_Curve.get_parameter_min() )
        {
            m_Curve.split( c1, c2, u );
            fp = c1.fp( u );
            fpp = c1.fpp( u );
        }
        else
        {
            fp = m_Curve.fp( u );
            fpp = m_Curve.fpp( u );
        }
    }
    else
    {
        piecewise_curve_type c1, c2;

        if ( u < m_Curve.get_parameter_max() )
        {
            m_Curve.split( c1, c2, u );
            fp = c2.fp( u );
            fpp = c2.fpp( u );
        }
        else
        {
            fp = m_Curve.fp( u );
            fpp = m_Curve.fpp( u );
        }
    }

    double fpn = fp.norm();
    double denom = fpn * fpn * fpn;
    return ( fp.cross( fpp ) ).norm() / denom;
}

//===== Compute Point U 0.0 -> 1.0 =====//
vec3d VspCurve::CompPnt01( double u ) const
{
    return CompPnt( u * m_Curve.get_tmax() );
}


//===== Compute Tan U 0.0 -> 1.0 =====//
vec3d VspCurve::CompTan01( double u ) const
{
    return CompTan( u * m_Curve.get_tmax() );
}

//===== Compute Length =====//
double VspCurve::CompLength( double tol ) const
{
    double len = 0.0;
    eli::geom::curve::length( len, m_Curve, tol );

    return len;
}

double VspCurve::CompArea( int idir, int jdir ) const
{
    return m_Curve.area( idir, jdir );
}

//===== Tesselate =====//
void VspCurve::TesselateNoCorner( int num_pnts_u, double umin, double umax, vector< vec3d > & output, vector< double > &uout )
{
    curve_index_type i;
    curve_point_type p;
    double delta;

    delta = ( umax - umin ) / ( num_pnts_u - 1 );

    uout.resize( num_pnts_u );
    for ( i = 0; i < num_pnts_u; ++i )
    {
        double u = umin + delta * i;
        uout[i] = u;
    }

    Tesselate( uout, output );
}

void VspCurve::Tesselate( const vector< double > &u, vector< vec3d > & output )
{
    unsigned int num_pnts_u = u.size();
    curve_index_type i;
    curve_point_type p;

    output.resize( num_pnts_u );

    for ( i = 0; i < num_pnts_u; ++i )
    {
        p = m_Curve.f( u[i] );
        output[i].set_xyz( p.x(), p.y(), p.z() );
    }
}

//===== Tesselate =====//
void VspCurve::TessAdapt( vector< vec3d > & pnts, double tol, int Nlimit )
{
    vector< double > uout;
    TessAdapt( pnts, uout, tol, Nlimit );
}

void VspCurve::TessAdapt( vector< vec3d > & pnts, vector< double > & uout, double tol, int Nlimit )
{
    TessAdapt( m_Curve.get_parameter_min(), m_Curve.get_parameter_max(), pnts, uout, tol, Nlimit );
}

void VspCurve::TessCornerAdapt( vector< vec3d > & pnts, vector< double > & uout, double tol, double atol, int Nlimit )
{
    vector < double > tdisc;
    m_Curve.find_discontinuities( atol, tdisc );

    // Make sure we always have integer parameter values and final value.
    // most curves are parameterized [0,4] with 1,2,3 being key points.
    for ( double t = m_Curve.get_t0(); t <= m_Curve.get_tmax(); t++ )
    {
        tdisc.push_back( t );
    }
    tdisc.push_back( m_Curve.get_tmax() );

    // Make sure no points are duplicated.
    std::sort( tdisc.begin(), tdisc.end() );
    auto last = std::unique( tdisc.begin(), tdisc.end() );
    tdisc.erase( last, tdisc.end() );

    TessBreaks( tdisc, pnts, uout, tol, Nlimit );
}

void VspCurve::TessSegAdapt( vector< vec3d > & pnts, vector< double > & uout, double tol, int Nlimit )
{
    vector < double > umap;
    m_Curve.get_pmap( umap );
    TessBreaks( umap, pnts, uout, tol, Nlimit );
}

void VspCurve::TessBreaks( const vector < double > & ubreak, vector< vec3d > & pnts, vector< double > & uout, double tol, int Nlimit )
{
    int nseg = ubreak.size() - 1;

    if ( nseg > 0 )
    {
        double usegstart = ubreak[0];
        vec3d psegstart = CompPnt( usegstart );

        for ( int i = 0; i < nseg; i++ )
        {
            double usegend = ubreak[ i + 1 ];
            vec3d psegend = CompPnt( usegend );
            TessAdapt( usegstart, usegend, psegstart, psegend, pnts, uout, tol, Nlimit );

            usegstart = usegend;
            psegstart = psegend;
        }

        pnts.push_back( psegstart );
        uout.push_back( usegstart );
    }
}

void VspCurve::TessAdapt( double umin, double umax, std::vector< vec3d > & pnts, vector< double > & uout, double tol, int Nlimit )
{
    vec3d pmin = CompPnt( umin );
    vec3d pmax = CompPnt( umax );

    TessAdapt( umin, umax, pmin, pmax, pnts, uout, tol, Nlimit );

    pnts.push_back( pmax );
    uout.push_back( umax );
}

void VspCurve::TessAdapt( double umin, double umax, const vec3d & pmin, const vec3d & pmax, std::vector< vec3d > & pnts, vector< double > & uout, double tol, int Nlimit, int Nadapt )
{
    double umid = ( umin + umax ) * 0.5;

    vec3d pmid = CompPnt( umid );

    double len = dist( pmin, pmax );
    double d = dist_pnt_2_line( pmin, pmax, pmid ) / len;

    if ( ( len > DBL_EPSILON && d > tol && Nlimit > 0 ) || Nadapt < 3 )
    {
        TessAdapt( umin, umid, pmin, pmid, pnts, uout, tol, Nlimit - 1, Nadapt + 1 );
        TessAdapt( umid, umax, pmid, pmax, pnts, uout, tol, Nlimit - 1, Nadapt + 1 );
    }
    else
    {
        pnts.push_back( pmin );
        pnts.push_back( pmid );

        uout.push_back( umin );
        uout.push_back( umid );
    }
}

curve_point_type setX( curve_point_type & cp, void* data )
{
    curve_point_type newcp;
    double *x = (double*) data;
    newcp << *x, cp.y(), cp.z();
    return newcp;
}

void VspCurve::AssignX( double x )
{
    m_Curve.transmute( setX, &x );
}

void VspCurve::FindMinMaxX( double & minx, double & maxx )
{
    vector < curve_point_type > cp_vec;
    m_Curve.get_control_point_vec( cp_vec );

    maxx = -std::numeric_limits< double >::max();
    minx = std::numeric_limits< double >::max();
    for ( int i = 0; i < cp_vec.size(); i++ )
    {
        if ( cp_vec[i].x() > maxx )
        {
            maxx = cp_vec[i].x();
        }

        if ( cp_vec[i].x() < minx )
        {
            minx = cp_vec[i].x();
        }
    }
}

//===== Offset =====//
void VspCurve::Offset( const vec3d &offvec )
{
    curve_point_type tr;
    tr << offvec.x(), offvec.y(), offvec.z();

    m_Curve.translate( tr );
}


//===== Offset X =====//
void VspCurve::OffsetX( double x )
{
    vec3d offvec( x, 0, 0 );
    Offset( offvec );
}

//===== Offset Y =====//
void VspCurve::OffsetY( double y )
{
    vec3d offvec( 0, y, 0 );
    Offset( offvec );
}

//===== Offset Z =====//
void VspCurve::OffsetZ( double z )
{
    vec3d offvec( 0, 0, z );
    Offset( offvec );
}

void VspCurve::ProjectOntoCylinder( double r, bool wingtype, double ttol, double atol, int dmin, int dmax )
{
    piecewise_binary_cubic_cylinder_projector pbccp;

    double tmin, tmax, tmid;
    tmin = m_Curve.get_parameter_min();
    tmax = m_Curve.get_parameter_max();
    tmid = ( tmin + tmax ) / 2.0;

    if ( wingtype )
    {
        piecewise_curve_type crv, telow, teup, le, low, up, rest;

        m_Curve.split( telow, crv, tmin + TMAGIC );
        crv.split( low, rest, tmid - TMAGIC );
        crv = rest;
        crv.split( le, rest, tmid + TMAGIC );
        crv = rest;
        crv.split( up, teup, tmax - TMAGIC );

        pbccp.setup( telow, r, ttol, atol, dmin, dmax );
        pbccp.corner_create( telow );

        pbccp.setup( low, r, ttol, atol, dmin, dmax );
        pbccp.corner_create( low );

        pbccp.setup( le, r, ttol, atol, dmin, dmax );
        pbccp.corner_create( le );

        pbccp.setup( up, r, ttol, atol, dmin, dmax );
        pbccp.corner_create( up );

        pbccp.setup( teup, r, ttol, atol, dmin, dmax );
        pbccp.corner_create( teup );

        m_Curve = telow;
        m_Curve.push_back( low );
        m_Curve.push_back( le );
        m_Curve.push_back( up );
        m_Curve.push_back( teup );

        m_Curve.set_tmax( tmax );
    }
    else
    {
        piecewise_curve_type low, up;

        m_Curve.split( low, up, tmid );

        // Setup copies base curve into creator.
        // tolerance, min adapt levels, max adapt levels
        pbccp.setup( low, r, ttol, atol, dmin, dmax );
        // Create makes new curve
        pbccp.corner_create( m_Curve );

        pbccp.setup( up, r, ttol, atol, dmin, dmax );
        pbccp.corner_create( up );

        m_Curve.push_back( up );

        m_Curve.set_tmax( tmax );
    }
}

void VspCurve::EvaluateOnSphere( bool wingtype, double ttol, double atol, int dmin, int dmax )
{
    piecewise_binary_cubic_sphere_projector pbccp;

    double tmin, tmax, tmid;
    tmin = m_Curve.get_parameter_min();
    tmax = m_Curve.get_parameter_max();
    tmid = ( tmin + tmax ) / 2.0;

    if ( wingtype )
    {
        piecewise_curve_type crv, telow, teup, le, low, up, rest;

        m_Curve.split( telow, crv, tmin + TMAGIC );
        crv.split( low, rest, tmid - TMAGIC );
        crv = rest;
        crv.split( le, rest, tmid + TMAGIC );
        crv = rest;
        crv.split( up, teup, tmax - TMAGIC );

        pbccp.setup( telow, ttol, atol, dmin, dmax );
        pbccp.corner_create( telow );

        pbccp.setup( low, ttol, atol, dmin, dmax );
        pbccp.corner_create( low );

        pbccp.setup( le, ttol, atol, dmin, dmax );
        pbccp.corner_create( le );

        pbccp.setup( up, ttol, atol, dmin, dmax );
        pbccp.corner_create( up );

        pbccp.setup( teup, ttol, atol, dmin, dmax );
        pbccp.corner_create( teup );

        m_Curve = telow;
        m_Curve.push_back( low );
        m_Curve.push_back( le );
        m_Curve.push_back( up );
        m_Curve.push_back( teup );

        m_Curve.set_tmax( tmax );
    }
    else
    {
        piecewise_curve_type low, up;

        m_Curve.split( low, up, tmid );

        // Setup copies base curve into creator.
        // tolerance, min adapt levels, max adapt levels
        pbccp.setup( low, ttol, atol, dmin, dmax );
        // Create makes new curve
        pbccp.corner_create( m_Curve );

        pbccp.setup( up, ttol, atol, dmin, dmax );
        pbccp.corner_create( up );

        m_Curve.push_back( up );

        m_Curve.set_tmax( tmax );
    }
}

//===== Rotate About X-Axis  =====//
void VspCurve::RotateX( double ang )
{
    double cosang = cos( ang );
    double sinang = sin( ang );

    curve_rotation_matrix_type rot;
    rot <<  1, 0,      0,
        0, cosang, sinang,
        0, -sinang, cosang;

    m_Curve.rotate( rot );
}

//===== Rotate About Y-Axis  =====//
void VspCurve::RotateY( double ang )
{
    double cosang = cos( ang );
    double sinang = sin( ang );

    curve_rotation_matrix_type rot;
    rot <<  cosang, 0, -sinang,
        0,      1, 0,
        sinang, 0, cosang;
    m_Curve.rotate( rot );
}
//===== Rotate About Z-Axis  =====//
void VspCurve::RotateZ( double ang )
{
    double cosang = cos( ang );
    double sinang = sin( ang );

    curve_rotation_matrix_type rot;
    rot <<  cosang, sinang, 0,
        -sinang, cosang, 0,
        0,      0,      1;
    m_Curve.rotate( rot );
}

//==== Transform Control Points =====//
void VspCurve::Transform( Matrix4d & mat )
{
    curve_rotation_matrix_type rmat;
    double *mmat( mat.data() );
    curve_point_type trans;

    rmat << mmat[0], mmat[4], mmat[8],
         mmat[1], mmat[5], mmat[9],
         mmat[2], mmat[6], mmat[10];
    trans << mmat[12], mmat[13], mmat[14];

    m_Curve.rotate( rmat );
    m_Curve.translate( trans );
}

void VspCurve::Scale( double s )
{
    m_Curve.scale( s );
}

void VspCurve::ScaleX( double s )
{
    m_Curve.scale_x( s );
}

void VspCurve::ScaleY( double s )
{
    m_Curve.scale_y( s );
}

void VspCurve::ScaleZ( double s )
{
    m_Curve.scale_z( s );
}

void VspCurve::ZeroI( int i )
{
    if ( i == 0 )
    {
        m_Curve.scale_x( 0.0 );
    }
    else if ( i == 1 )
    {
        m_Curve.scale_y( 0.0 );
    }
    else
    {
        m_Curve.scale_z( 0.0 );
    }
}

void VspCurve::ReflectXY()
{
    m_Curve.reflect_xy();
}

void VspCurve::ReflectXZ()
{
    m_Curve.reflect_xz();
}

void VspCurve::ReflectYZ()
{
    m_Curve.reflect_yz();
}

void VspCurve::Reflect( const vec3d &axis )
{
    curve_point_type a;

    a << axis.x(), axis.y(), axis.z();
    m_Curve.reflect( a );
}

void VspCurve::Reflect( const vec3d &axis, double d )
{
    curve_point_type a;

    a << axis.x(), axis.y(), axis.z();
    m_Curve.reflect( a, d );
}

void VspCurve::Reverse()
{
    m_Curve.reverse();
}


bool VspCurve::IsEqual( const VspCurve & crv )
{
    int ns0 = m_Curve.number_segments();
    int ns1 = crv.m_Curve.number_segments();

    if ( ns0 != ns1 )
        return false;

    // get control points and print
    int i, pp;

    for ( pp=0 ; pp < ns0 ; ++pp )
    {
      curve_segment_type bez0;
      curve_segment_type bez1;

      m_Curve.get(bez0, pp);
      crv.m_Curve.get(bez1, pp);

      if ( bez0.degree() != bez1.degree() )
          return false;

      for (i=0; i<=bez0.degree(); ++i)
      {
          curve_point_type cp0 = bez0.get_control_point(i);
          curve_point_type cp1 = bez1.get_control_point(i);
          vec3d v0( cp0.x(), cp0.y(), cp0.z() );
          vec3d v1( cp1.x(), cp1.y(), cp1.z() );

          if ( dist( v0, v1 ) > 1.0e-12 )
              return false;
      }
    }

    return true;

}

void VspCurve::GetBoundingBox( BndBox &bb ) const
{
    curve_bounding_box_type bbx;
    vec3d v3min, v3max;

    m_Curve.get_bounding_box( bbx );
    v3min.set_xyz( bbx.get_min().x(), bbx.get_min().y(), bbx.get_min().z() );
    v3max.set_xyz( bbx.get_max().x(), bbx.get_max().y(), bbx.get_max().z() );
    bb.Reset();
    bb.Update( v3min );
    bb.Update( v3max );
}

// This routine estimates the thickness of an airfoil from the curves directly.
// It constructs the equiparameteric distance squared curve.  It then maximizes that
// curve.
double VspCurve::CalculateThick( double &loc ) const
{
    piecewise_curve_type crv , c1, c2, c3, c4;
    crv = m_Curve;

    double tmid = ( crv.get_parameter_max() + crv.get_parameter_min() ) / 2.0;

    crv.split( c1, c2, tmid );  // Split at LE
    c2.reverse();
    c2.set_t0( c1.get_t0() );

    MatchParameterSplits( c1, c2 );

    c1.scale( -1.0 );
    c3.sum( c1, c2 );

    c4.square( c3 );

    typedef piecewise_curve_type::onedpiecewisecurve onedpwc;
    onedpwc sumsq;

    sumsq = c4.sumcompcurve();

    // Negate to allow minimization instead of maximization.
    sumsq.scale( -1.0 );

    double utmax;
    double tmax = sqrt( -1.0 * eli::geom::intersect::minimum_dimension( utmax, sumsq, 0 ) );

    loc = ( c2.f( utmax ).x() - c1.f( utmax ).x() ) * 0.5;

    return tmax;
}

void VspCurve::MatchThick( const double & ttarget )
{
    piecewise_curve_type crv , c1, c2, c3, c4, clow, cup, cmid;
    crv = m_Curve;

    double tmid = ( crv.get_parameter_max() + crv.get_parameter_min() ) / 2.0;

    crv.split( c1, c2, tmid );  // Split at LE
    c2.reverse();
    c2.set_t0( c1.get_t0() );

    MatchParameterSplits( c1, c2 );

    cmid.sum( c1, c2 );
    cmid.scale( 0.5 );

    c1.scale( -1.0 );
    c3.sum( c1, c2 );

    c4.square( c3 );

    typedef piecewise_curve_type::onedpiecewisecurve onedpwc;
    onedpwc sumsq;

    typedef onedpwc::point_type onedpt;
    onedpt p;

    sumsq = c4.sumcompcurve();

    // Negate to allow minimization instead of maximization.
    sumsq.scale( -1.0 );

    double utmax;
    double tmax = sqrt( -1.0 * eli::geom::intersect::minimum_dimension( utmax, sumsq, 0 ) );

    double sf = ( ttarget / tmax ) * 0.5; // half because we need to add half thickness up and down.

    c3.scale( sf ); // Scale delta by scale factor.

    cup.sum( cmid, c3 ); // Construct upper surface.
    c3.scale( -1.0 );
    clow.sum( cmid, c3 );  // Construct lower surface.

    cup.reverse();
    clow.push_back( cup );  // Append top/bottom curves into one again.

    m_Curve = clow; // Set thickness scaled curve back to m_Curve.
}

// Find the angle between two points on a curve.
// First point: u1, considering curve dir1 = BEFORE or AFTER the point
// Second point: u2, considering curve dir2 = BEFORE or AFTER the point
// flipflag determines whether the second curve's sign is changed before calculating angle.
double VspCurve::Angle( const double & u1, const int &dir1, const double & u2, const int &dir2, const bool & flipflag ) const
{
    vec3d tan1, tan2;

    tan1 = CompTan( u1, dir1 );
    tan2 = CompTan( u2, dir2 );

    if ( flipflag )
    {
        tan2 = tan2 * -1.0;
    }

    return angle( tan1, tan2 );
}

vector < BezierSegment > VspCurve::GetBezierSegments()
{
    vector < BezierSegment > seg_vec;

    double t0 = 0.0;
    double tmax = m_Curve.get_tmax(); // Get tmax for entire curve so that the sum of dt of all bezier segments = 1.0

    int num_sections = m_Curve.number_segments();

    for ( int i = 0; i < num_sections; i++ )
    {
        BezierSegment seg;
        curve_segment_type c;
        double dt;

        m_Curve.get( c, dt, i );

        seg.order = c.degree();
        seg.t0 = t0;
        seg.tmax = t0 + ( dt / tmax );
        t0 = seg.tmax;

        for ( int j = 0; j < c.degree() + 1; j++ )
        {
            curve_point_type cp = c.get_control_point( j );

            seg.control_pnt_vec.emplace_back( vec3d( cp ) );
        }

        seg_vec.push_back( seg );
    }

    return seg_vec;
}

void VspCurve::CreateRoundedRectangle( double w, double h, double k, double sk, double vsk, const double & r1, const double & r2, const double & r3, const double & r4, bool keycorner )
{
    vector<vec3d> pt;
    vector<double> u;

    bool round_curve( true );

    double wt = 2.0 * k * w;
    double wb = 2.0 * ( 1 - k ) * w;
    double wt2 = 0.5 * wt;
    double wb2 = 0.5 * wb;
    double w2 = 0.5 * w;
    double w_off = sk * w2;
    double h2 = 0.5 * h;
    double h_off = vsk * h2;

    // catch special cases of degenerate cases
    if ( ( w2 == 0 ) || ( h2 == 0 ) )
    {
        pt.resize( 4 );
        u.resize( 5 );

        // set the segment points
        pt[0].set_xyz(  w,   0, 0 );
        pt[1].set_xyz( w2, -h2, 0 );
        pt[2].set_xyz(  0,   0, 0 );
        pt[3].set_xyz( w2,  h2, 0 );

        // set the corresponding parameters
        u[0] = 0;
        u[1] = 1;
        u[2] = 2;
        u[3] = 3;
        u[4] = 4;

        round_curve = false;
    }
    // create rectangle
    else
    {
        pt.resize( 8 );
        u.resize( 9 );

        // set the segment points
        pt[0].set_xyz( w,                 h_off,        0 );
        pt[1].set_xyz( w2 + wb2 - w_off, -h2 + h_off,   0 );
        pt[2].set_xyz( w2 - w_off,       -h2,           0 );
        pt[3].set_xyz( w2 - wb2 - w_off, -h2 - h_off,   0 );
        pt[4].set_xyz( 0,                -h_off,        0 );
        pt[5].set_xyz( w2 - wt2 + w_off,  h2 - h_off,   0 );
        pt[6].set_xyz( w2 + w_off,        h2,           0 );
        pt[7].set_xyz( w2 + wt2 + w_off,  h2 + h_off,   0 );

        // set the corresponding parameters
        u[0] = 0;
        u[2] = 1;
        u[4] = 2;
        u[6] = 3;
        u[8] = 4;

        if ( keycorner )
        {
            u[1] = 0.5;
            u[3] = 1.5;
            u[5] = 2.5;
            u[7] = 3.5;
        }
        else
        {
            double d1 = dist( pt[0], pt[1] );
            double d2 = dist( pt[1], pt[2] );
            double du = d1 / ( d1 + d2 );
            if ( du < 0.001 ) du = 0.001;
            if ( du > 0.999 ) du = 0.999;
            u[1] = du;

            d1 = dist( pt[2], pt[3] );
            d2 = dist( pt[3], pt[4] );
            du = d1 / ( d1 + d2 );
            if ( du < 0.001 ) du = 0.001;
            if ( du > 0.999 ) du = 0.999;
            u[3] = 1 + du;

            d1 = dist( pt[4], pt[5] );
            d2 = dist( pt[5], pt[6] );
            du = d1 / ( d1 + d2 );
            if ( du < 0.001 ) du = 0.001;
            if ( du > 0.999 ) du = 0.999;
            u[5] = 2 + du;

            d1 = dist( pt[6], pt[7] );
            d2 = dist( pt[7], pt[0] );
            du = d1 / ( d1 + d2 );
            if ( du < 0.001 ) du = 0.001;
            if ( du > 0.999 ) du = 0.999;
            u[7] = 3 + du;
        }
    }

    // build the polygon
    InterpolateLinear( pt, u, true );

    // round all joints if needed
    if ( round_curve )
    {
        vector < double > r_vec{ r1, r2, r3, r4 };
        // Iterate backwards to avoid adjusting node indices for added corners.
        for ( int r = r_vec.size() - 1; r >= 0; r-- )
        {
            if ( r_vec[r] > 1e-12 )
            {
                bool rtn = RoundJoint( r_vec[r], ( 2 * r ) + 1 );
            }
        }
    }
}

void VspCurve::ToCubic( double tol )
{
    m_Curve.to_cubic( tol );
}

void VspCurve::CreateTire( double Do, double W, double Ds, double Ws, double Drim, double Wrim, double Hflange, int mode )
{
    double beta = 0.5 * M_PI;
    double k = eli::constants::math < double >::cubic_bezier_circle_const() * tan( beta * 0.25 );

    m_Curve.clear();
    m_Curve.set_t0( 0.0 );

    double dt = 1.0;
    curve_segment_type clin, carc;
    curve_point_type pt;

    clin.resize( 1 ); // Linear
    carc.resize( 3 ); // Cubic

    double t;
    oned_curve_segment_type c1d;
    oned_curve_point_type x1d;
    curve_segment_type c1, c2;


    if ( mode >= vsp::TIRE_BALLOON )
    {


        double r = W / 2.0;

        if ( mode == vsp::TIRE_BALLOON ||
             mode == vsp::TIRE_BALLOON_WHEEL )
        {
            // Bottom of balloon tire
            pt << 0, 0, Do / 2.0 - 2 * r;
            carc.set_control_point( pt, 0 );
            pt << 0, k * r, Do / 2.0 - 2 * r;
            carc.set_control_point( pt, 1 );
            pt << 0, r, Do / 2.0 - 2 * r + k * r;
            carc.set_control_point( pt, 2 );
            pt << 0, r, Do / 2.0 - r;
            carc.set_control_point( pt, 3 );

            if ( mode == vsp::TIRE_BALLOON_WHEEL )
            {
                // Find intersection between wheel and balloon tire bottom
                x1d << -Drim / 2.0;
                c1d = carc.singledimensioncurve( 2 );
                c1d.translate( x1d );
                eli::geom::intersect::find_zero( t, c1d, 0.5 );

                // Split bottom at intersection point
                carc.split( c1, c2, t );

                // Wheel
                pt = c2.get_control_point( 0 );
                pt( 2 ) = 0;

                clin.set_control_point( pt, 0 );
                clin.set_control_point( c2.get_control_point( 0 ), 1 );
                m_Curve.push_back( clin, dt );

                m_Curve.push_back( c2, dt );
            }
            else
            {
                // Use complete balloon tire bottom
                m_Curve.push_back( carc, dt );
            }
        }
        else // mode == vsp::TIRE_BALLOON_FAIR_WHEEL
        {
            // Wheel
            pt << 0, r, 0;
            clin.set_control_point( pt, 0 );
            pt << 0, r, Do / 2.0 - r;
            clin.set_control_point( pt, 1 );
            m_Curve.push_back( clin, dt );
        }



        // Top of balloon tire
        pt << 0, r, Do / 2.0 - r;
        carc.set_control_point( pt, 0 );
        pt << 0, r, Do / 2.0 - r + r * k;
        carc.set_control_point( pt, 1 );
        pt << 0, k * r, Do / 2.0;
        carc.set_control_point( pt, 2 );
        pt << 0, 0, Do / 2.0;
        carc.set_control_point( pt, 3 );
        m_Curve.push_back( carc, dt );

    }
    else
    {
        // Tire height
        double H = 0.5 * ( Do - Drim );

        // Flange radius
        double Rflange = 0.5 * Hflange;
        // Flange width
        double B = 1.3 * Rflange;
        // Width of wheel to outside of flanges
        double Wflange = Wrim + 2.0 * B;

        // Shoulder origin
        double Hs = Ds - Do;
        double rs = 0.5 * Ws;
        double xs0 = rs - sqrt( 0.25 * ( -Hs * Hs - 2 * Ws * Hs ) );
        double ys0 = 0.5 * ( Do - Ws );

        double xs = 0.5 * Ws;
        double ys = 0.5 * Ds;

        // Cheek origin
        double yc0 = 0.5 * Drim + 0.5 * H;
        double dy = 0.5 * Ds - yc0;
        double xc0 = ( 0.25 * ( Ws * Ws - W * W ) + dy * dy ) / ( Ws - W );
        double rc = 0.5 * W - xc0;

        // Solve for mutually tangent point between flange and flank
        // flange center point
        double x1 = 0.5 * Wrim + Rflange;
        double y1 = 0.5 * Drim + Rflange;

        // Deltas to tire width point -- cheek/flank intersection
        double dx1 = 0.5 * W - x1;
        double dy1 = yc0 - y1;

        // Flank radius
        double rflank = ( dx1 * dx1 + dy1 * dy1 - Rflange * Rflange ) / ( 2.0 * ( Rflange + dx1 ) );

        // Flank origin
        double yf0 = yc0;
        double xf0 = 0.5 * W - rflank;

        // Triangle scale fraction.
        double f = rflank / ( Rflange + rflank );

        // Point of tangency.
        double xt = xf0 + f * ( x1 - xf0 );
        double yt = yf0 + f * ( y1 - yf0 );


        if ( mode == vsp::TIRE_TRA ||
             mode == vsp::TIRE_FAIR_FLANGE )
        {
            // Wheel
            pt << 0, Wflange / 2.0, 0;
            clin.set_control_point( pt, 0 );
            pt << 0, Wflange / 2.0, Drim / 2.0 + Hflange;
            clin.set_control_point( pt, 1 );
            m_Curve.push_back( clin, dt );

            if ( mode == vsp::TIRE_FAIR_FLANGE )
            {
                // Faired over flange and flank
                pt << 0, Wflange / 2.0, Drim / 2.0 + Hflange;
                clin.set_control_point( pt, 0 );
                pt << 0, W / 2.0, yf0;
                clin.set_control_point( pt, 1 );
                m_Curve.push_back( clin, dt ); // Was 3 * dt
            }
            else // vsp::TIRE_TRA
            {
                // Flange flat
                pt << 0, Wflange / 2.0, Drim / 2.0 + Hflange;
                clin.set_control_point( pt, 0 );
                pt << 0, Wflange / 2.0 - 0.3 * Rflange, Drim / 2.0 + Hflange;
                clin.set_control_point( pt, 1 );
                m_Curve.push_back( clin, dt );


                // Flange face
                pt << 0, x1, y1 + Rflange;
                carc.set_control_point( pt, 0 );
                pt << 0, x1 - k * Rflange, y1 + Rflange;
                carc.set_control_point( pt, 1 );
                pt << 0, x1 - Rflange, y1 + k * Rflange;
                carc.set_control_point( pt, 2 );
                pt << 0, x1 - Rflange, y1;
                carc.set_control_point( pt, 3 );

                x1d << -xt;
                c1d = carc.singledimensioncurve( 1 );
                c1d.translate( x1d );
                eli::geom::intersect::find_zero( t, c1d, 0.5 );

                carc.split( c1, c2, t );

                // Force tangent point.
                pt << 0, xt, yt;
                c1.set_control_point( pt, 3 );
                m_Curve.push_back( c1, dt );


                // Flank
                pt << 0, xf0, yf0 - rflank;
                carc.set_control_point( pt, 0 );
                pt << 0, xf0 + k * rflank, yf0 - rflank;
                carc.set_control_point( pt, 1 );
                pt << 0, W / 2, yf0 - k * rflank;
                carc.set_control_point( pt, 2 );
                pt << 0, W / 2, yf0;
                carc.set_control_point( pt, 3 );

                x1d << -xt;
                c1d = carc.singledimensioncurve( 1 );
                c1d.translate( x1d );
                eli::geom::intersect::find_zero( t, c1d, 0.5 );

                carc.split( c1, c2, t );

                // Force tangent point.
                pt << 0, xt, yt;
                c2.set_control_point( pt, 0 );
                m_Curve.push_back( c2, dt );
            }
        }
        else // TIRE_FAIR_WHEEL
        {
            // Wheel
            pt << 0, W / 2.0, 0;
            clin.set_control_point( pt, 0 );
            pt << 0, W / 2.0, yf0;
            clin.set_control_point( pt, 1 );
            m_Curve.push_back( clin, dt ); // was 4 * dt
        }

        // Cheek
        pt << 0, W / 2.0, yc0;
        carc.set_control_point( pt, 0 );
        pt << 0, W / 2.0, yc0 + k * rc;
        carc.set_control_point( pt, 1 );
        pt << 0, xc0 + k * rc, yc0 + rc;
        carc.set_control_point( pt, 2 );
        pt << 0, xc0, yc0 + rc;
        carc.set_control_point( pt, 3 );

        x1d << -rs;
        c1d = carc.singledimensioncurve( 1 );
        c1d.translate( x1d );
        eli::geom::intersect::find_zero( t, c1d, 0.5 );

        carc.split( c1, c2, t );

        // Force shoulder point.
        pt << 0, xs, ys;
        c1.set_control_point( pt, 3 );
        m_Curve.push_back( c1, dt );

        // Shoulder
        pt << 0, xs0 + rs, ys0;
        carc.set_control_point( pt, 0 );
        pt << 0, xs0 + rs, ys0 + k * rs;
        carc.set_control_point( pt, 1 );
        pt << 0, xs0 + k * rs, Do / 2.0;
        carc.set_control_point( pt, 2 );
        pt << 0, xs0, Do / 2.0;
        carc.set_control_point( pt, 3 );

        x1d << -rs;
        c1d = carc.singledimensioncurve( 1 );
        c1d.translate( x1d );
        eli::geom::intersect::find_zero( t, c1d, 0.5 );

        carc.split( c1, c2, t );

        // Force shoulder point.
        pt << 0, xs, ys;
        c2.set_control_point( pt, 0 );
        m_Curve.push_back( c2, dt );

        // Flat
        pt << 0, xs0, Do / 2.0;
        clin.set_control_point( pt, 0 );
        pt << 0, 0, Do / 2.0;
        clin.set_control_point( pt, 1 );
        m_Curve.push_back( clin, dt );
    }


    // First quadrant has been created.  Now, copy, reflect, and merge to complete curve.

    // m_Curve.reflect_xy();
    piecewise_curve_type pc1 = m_Curve;
    pc1.reflect_xz();
    pc1.reverse();
    m_Curve.push_back( pc1 );
}

void VspCurve::CreateAC25773( int side )
{
    vector < vec3d > ctrl_pnts;
    vector < double > t_vec;

    ctrl_pnts.resize( 15 );
    ctrl_pnts[0].set_xyz( 20.0, 0.0, 0.0 );
    ctrl_pnts[1].set_xyz( 20.0, -10.0, 0.0 );
    ctrl_pnts[2].set_xyz( 10.0, -17.0, 0.0 );
    ctrl_pnts[3].set_xyz( -30.0, -17.0, 0.0 );
    ctrl_pnts[4].set_xyz( -60.0, -24.5, 0.0 ); // interpolated
    ctrl_pnts[5].set_xyz( -70.0, -27.0, 0.0 );
    ctrl_pnts[6].set_xyz( -95.0, -27.0, 0.0 );
    ctrl_pnts[7].set_xyz( -120.0, -15.0, 0.0 );
    ctrl_pnts[8].set_xyz( -120.0, 0.0, 0.0 );
    ctrl_pnts[9].set_xyz( -120.0, 15.0, 0.0 );
    ctrl_pnts[10].set_xyz( -80.0, 35.0, 0.0 );
    ctrl_pnts[11].set_xyz( -60.0, 35.0, 0.0 );
    ctrl_pnts[12].set_xyz( -40.0, 35.0, 0.0 );
    ctrl_pnts[13].set_xyz( 20.0, 15.0, 0.0 );
    ctrl_pnts[14].set_xyz( 20.0, 0.0, 0.0 );

    t_vec.resize( 15 );
    t_vec[0] = 0;
    t_vec[1] = 0.25;
    t_vec[2] = 0.5;
    t_vec[3] = 0.75;
    t_vec[4] = 1.0;
    t_vec[5] = 1.25;
    t_vec[6] = 1.5;
    t_vec[7] = 1.75;
    t_vec[8] = 2.0;
    t_vec[9] = 2.25;
    t_vec[10] = 2.6;
    t_vec[11] = 3.0;
    t_vec[12] = 3.4;
    t_vec[13] = 3.75;
    t_vec[14] = 4.0;

    InterpolateLinear( ctrl_pnts, t_vec, false );

    int flip = 1;
    if ( side == vsp::XSEC_RIGHT_SIDE )
    {
        flip = -1;
    }

    ScaleX( flip );

    if ( flip != 1 )
    {
        Reverse();
    }
}

void MatchParameterSplits( piecewise_curve_type &c1, piecewise_curve_type &c2 )
{
    vector < double > pmap;
    c1.get_pmap( pmap );

    vector < double > pmap2;
    c2.get_pmap( pmap2 );

    pmap.insert( pmap.end(), pmap2.begin(), pmap2.end() );
    std::sort( pmap.begin(), pmap.end() );
    auto pmit = std::unique( pmap.begin(), pmap.end() );
    pmap.erase( pmit, pmap.end() );

    for( int i = 0; i < pmap.size(); i++ )
    {
        c1.split( pmap[i] );
        c2.split( pmap[i] );
    }
}
