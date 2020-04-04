//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Vsp1DCurve.h:
//
//////////////////////////////////////////////////////////////////////


#include <cstdio>

#include "Vsp1DCurve.h"


#include "eli/geom/curve/length.hpp"
#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/intersect/specified_distance_curve.hpp"
#include "eli/geom/intersect/specified_thickness_curve.hpp"

typedef oned_piecewise_curve_type::index_type oned_curve_index_type;
typedef oned_piecewise_curve_type::point_type oned_curve_point_type;
typedef oned_piecewise_curve_type::tolerance_type oned_curve_tolerance_type;
typedef oned_piecewise_curve_type::bounding_box_type oned_curve_bounding_box_type;


typedef eli::geom::curve::piecewise_cubic_spline_creator<double, 1, oned_curve_tolerance_type> oned_piecewise_cubic_spline_creator_type;
typedef eli::geom::curve::piecewise_linear_creator<double, 1, oned_curve_tolerance_type> oned_piecewise_linear_creator_type;
typedef eli::geom::curve::piecewise_binary_cubic_creator<double, 1, oned_curve_tolerance_type> oned_piecewise_binary_cubic_creator;

//=============================================================================//
//============================= VspCurve      =================================//
//=============================================================================//

//===== Constructor  =====//
Vsp1DCurve::Vsp1DCurve()
{
}

//===== Destructor  =====//
Vsp1DCurve::~Vsp1DCurve()
{
}

//==== Copy From Input Curve =====//
void Vsp1DCurve::Copy( Vsp1DCurve & input_crv )
{
    m_Curve = input_crv.m_Curve;
}

//==== Split at Specified Parameter and Return Remaining Curve =====//
void Vsp1DCurve::Split( double u )
{
    m_Curve.split( u );
}

//==== Append Curve To Existing Curve ====//
void Vsp1DCurve::Append( Vsp1DCurve & input_crv )
{
    oned_curve_index_type i, nc( input_crv.GetNumSections() );

    for ( i = 0; i < nc; ++i )
    {
        oned_piecewise_curve_type::error_code ec;
        oned_curve_segment_type c;

        input_crv.GetCurveSegment( c, i );
        ec = m_Curve.push_back( c, input_crv.GetCurveDt( i ) );
        if ( ec != oned_piecewise_curve_type::NO_ERRORS )
        {
            std::cerr << "Could not append curve." << std::endl;
        }
    }
}

bool Vsp1DCurve::IsClosed() const
{
    return m_Curve.closed();
}

//===== Interpolate Creates piecewise linear curves ===//
void Vsp1DCurve::InterpolateLinear( vector< double > & input_pnt_vec, const vector<double> &param, bool closed_flag )
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
    vector<oned_curve_point_type> pts( input_pnt_vec.size() );
    for ( size_t i = 0; i < pts.size(); ++i )
    {
        pts[i] << input_pnt_vec[i];
    }

    if ( closed_flag )
    {
        pts.push_back( pts[0] );
    }

    int nseg( pts.size() - 1 );
    oned_piecewise_linear_creator_type plc( nseg );

    // set the delta t for each curve segment
    plc.set_t0( param[0] );
    for ( oned_curve_index_type i = 0; i < plc.get_number_segments(); ++i )
    {
        plc.set_segment_dt( param[i + 1] - param[i], i );
    }

    // set the polygon corners
    for ( oned_curve_index_type i = 0; i < static_cast<oned_curve_index_type>( pts.size() ); ++i )
    {
        plc.set_corner( pts[i], i );
    }

    if ( !plc.create( m_Curve ) )
    {
        std::cerr << "Failed to create linear curve. " << __LINE__ << std::endl;
    }
}

//===== Interpolate Creates PCHIP ====//
void Vsp1DCurve::InterpolatePCHIP( vector< double > & input_pnt_vec, const vector<double> &param, bool closed_flag )
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
    vector<oned_curve_point_type> pts( input_pnt_vec.size() );
    for ( size_t i = 0; i < pts.size(); ++i )
    {
        pts[i] << input_pnt_vec[i];
    }

    // create creator for known number of segments
    int nseg( pts.size() - 1 );
    if ( closed_flag )
    {
        ++nseg;
    }
    oned_piecewise_cubic_spline_creator_type pcsc( nseg );

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
void Vsp1DCurve::InterpolateCSpline( vector< double > & input_pnt_vec, const vector<double> &param, bool closed_flag )
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
    vector<oned_curve_point_type> pts( input_pnt_vec.size() );
    for ( size_t i = 0; i < pts.size(); ++i )
    {
        pts[i] << input_pnt_vec[i];
    }

    // create creator for known number of segments
    int nseg( pts.size() - 1 );
    if ( closed_flag )
    {
        ++nseg;
    }
    oned_piecewise_cubic_spline_creator_type pcsc( nseg );

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
void Vsp1DCurve::InterpolateCSpline( vector< double > & input_pnt_vec, const double &start_slope, const double &end_slope, const vector<double> &param )
{
    // do some checking of vector lengths
    if ( param.size() != input_pnt_vec.size() )
    {
        std::cerr << "Invalid number of points and parameters in curve interpolation " << __LINE__ << std::endl;
        assert( false );
        return;
    }

    // copy points over to new type
    vector<oned_curve_point_type> pts( input_pnt_vec.size() );
    oned_curve_point_type sslope, eslope;

    for ( size_t i = 0; i < pts.size(); ++i )
    {
        pts[i] << input_pnt_vec[i];
    }
    sslope << start_slope;
    eslope << end_slope;

    // create creator for known number of segments
    oned_piecewise_cubic_spline_creator_type pcsc( pts.size() - 1 );

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

void Vsp1DCurve::BinCubicTMap( vector < double > &tmap, vector < double > &tdisc )
{
    oned_piecewise_binary_cubic_creator pbcc;

    // Setup copies base curve into creator.
    // tolerance, min adapt levels, max adapt levels
    pbcc.setup( m_Curve, 1e-2, 0.01, 0, 7 );

    // Pick off discontinuities because we want to return them as well
    // as use them in corner_create
    m_Curve.find_discontinuities( 0.01, tdisc );
    tdisc.push_back( m_Curve.get_tmax() );

    // Create makes new curve
    oned_piecewise_curve_type c;
    pbcc.corner_create( c, tdisc );
    // Get parameter map
    c.get_pmap( tmap );
}

void Vsp1DCurve::GetTMap( vector < double > &tmap, vector < double > &tdisc )
{
    // Pick off discontinuities because we want to return them as well
    // as use them in corner_create
    m_Curve.find_discontinuities( 0.01, tdisc );
    tdisc.push_back( m_Curve.get_tmax() );

    // Get parameter map
    m_Curve.get_pmap( tmap );
}

void Vsp1DCurve::ToBinaryCubic( )
{
    oned_piecewise_binary_cubic_creator pbcc;

    // Setup copies base curve into creator.
    // ttol -- tolerance on gap between curve midpoints
    // atol -- angle tolerance used to detect corners before adaptation.  Smooth if:  abs(1-cos(theta)) <= atol
    // dmin -- minimum number of divisions of curve
    // dmax -- maximum number of divisions of curve
    pbcc.setup( m_Curve, 5e-2, 0.01, 0, 4 );
    // Create makes new curve
    pbcc.corner_create( m_Curve );
}

void Vsp1DCurve::ToCubic()
{
    m_Curve.to_cubic( 0.01 );
}

void Vsp1DCurve::SetCubicControlPoints( const vector< double > & cntrl_pts, bool closed_flag )
{
    int ncp = cntrl_pts.size();
    int nseg = ( ncp - 1 ) / 3;

    m_Curve.clear();
    m_Curve.set_t0( 0.0 );

    for ( int i = 0; i < nseg; i++ )
    {
        oned_curve_segment_type c;
        c.resize( 3 );

        for ( int j = 0; j < 4; j++ )
        {
            int k = i * 3 + j;

            double p = cntrl_pts[k];

            oned_curve_point_type cp;
            cp << p;

            c.set_control_point( cp, j );
        }
        m_Curve.push_back( c );
    }
}

void Vsp1DCurve::SetCubicControlPoints( const vector< double > & cntrl_pts, const vector< double > & param, bool closed_flag )
{
    int ncp = cntrl_pts.size();
    int nseg = ( ncp - 1 ) / 3;

    assert( ncp == param.size() );

    m_Curve.clear();
    m_Curve.set_t0( param[0] );

    for ( int i = 0; i < nseg; i++ )
    {
        oned_curve_segment_type c;
        c.resize( 3 );

        for ( int j = 0; j < 4; j++ )
        {
            int k = i * 3 + j;

            double p = cntrl_pts[k];

            oned_curve_point_type cp;
            cp << p;

            c.set_control_point( cp, j );
        }
        m_Curve.push_back( c, param[ 3 * ( i + 1 ) ] - param[ 3 * i ] );
    }
}

void Vsp1DCurve::GetCubicControlPoints( vector< double > & cntrl_pts, vector< double > & param )
{
    unsigned int nseg = m_Curve.number_segments();

    unsigned int ncp = nseg * 3  + 1;

    cntrl_pts.clear();
    param.clear();

    cntrl_pts.reserve( ncp );
    param.reserve( ncp );

    vector < double > pmap;
    m_Curve.get_pmap( pmap );

    for ( int i = 0; i < nseg; i++ )
    {
        oned_curve_segment_type c;

        m_Curve.get( c, i );

        for ( int j = 0; j < c.degree(); j++ )
        {
            oned_curve_point_type p;
            p = c.get_control_point( j );

            cntrl_pts.push_back( p[0] );
        }

        double tstart = pmap[ i ];
        double tend = pmap[ i + 1 ];
        double dt = tend - tstart;

        param.push_back( tstart );
        param.push_back( tstart + dt / 3.0 );
        param.push_back( tend - dt / 3.0 );

        if ( i == nseg - 1 )
        {
            oned_curve_point_type p;
            p = c.get_control_point( 3 );

            cntrl_pts.push_back( p[0] );
            param.push_back( tend );
        }
    }
}

//===== Get Number Of Sections =====//
int Vsp1DCurve::GetNumSections() const
{
    return m_Curve.number_segments();
}

const oned_piecewise_curve_type & Vsp1DCurve::GetCurve() const
{
    return m_Curve;
}

void Vsp1DCurve::SetCurve( const oned_piecewise_curve_type &c )
{
    m_Curve = c;
}

void Vsp1DCurve::GetCurveSegment( oned_curve_segment_type &c, int i ) const
{
    if ( i < GetNumSections() )
    {
        m_Curve.get( c, i );
    }
}

double Vsp1DCurve::GetCurveDt( int i ) const
{
    double dt( -1 );

    if ( i < GetNumSections() )
    {
        oned_curve_segment_type c;

        m_Curve.get( c, dt, i );
    }

    return dt;
}

double Vsp1DCurve::FindNearest( double &u, const double &pt ) const
{
    double dist;
    oned_curve_point_type p( pt );

    dist = eli::geom::intersect::minimum_distance( u, m_Curve, p );

    return dist;
}

double Vsp1DCurve::FindNearest( double &u, const double &pt, const double &u0 ) const
{
    double dist;
    oned_curve_point_type p( pt );

    dist = eli::geom::intersect::minimum_distance( u, m_Curve, p, u0 );

    return dist;
}

double Vsp1DCurve::FindNearest01( double &u, const double &pt ) const
{
    double dist;

    dist = FindNearest( u, pt );

    u = u / m_Curve.get_tmax();

    return dist;
}

double Vsp1DCurve::FindNearest01( double &u, const double &pt, const double &u0 ) const
{
    double dist;

    dist = FindNearest( u, pt, u0 * m_Curve.get_tmax() );

    u = u / m_Curve.get_tmax();

    return dist;
}

void Vsp1DCurve::AppendCurveSegment( oned_curve_segment_type &c )
{
    m_Curve.push_back( c, 1 );
}

//===== Compute Point  =====//
double Vsp1DCurve::CompPnt( double u )
{
    oned_curve_point_type v( m_Curve.f( u ) );

    return v.x();
}

//===== Compute Tangent  =====//
double Vsp1DCurve::CompTan( double u )
{
    oned_curve_point_type v( m_Curve.fp( u ) );

    return v.x();
}

//===== Compute Point U 0.0 -> 1.0 =====//
double Vsp1DCurve::CompPnt01( double u )
{
    return CompPnt( u * m_Curve.get_tmax() );
}


//===== Compute Tan U 0.0 -> 1.0 =====//
double Vsp1DCurve::CompTan01( double u )
{
    return CompTan( u * m_Curve.get_tmax() );
}

//===== Tesselate =====//
void Vsp1DCurve::TesselateNoCorner( int num_pnts_u, double umin, double umax, vector< double > & output, vector< double > &uout )
{
    oned_curve_index_type i;
    oned_curve_point_type p;
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

void Vsp1DCurve::Tesselate( const vector< double > &u, vector< double > & output )
{
    unsigned int num_pnts_u = u.size();
    oned_curve_index_type i;
    oned_curve_point_type p;

    output.resize( num_pnts_u );

    for ( i = 0; i < num_pnts_u; ++i )
    {
        p = m_Curve.f( u[i] );
        output[i] = p.x();
    }
}

//===== Tesselate =====//
void Vsp1DCurve::TessAdapt( vector< double > & output, vector< double > & uout, double tol, int Nlimit  )
{
    TessAdapt( m_Curve.get_parameter_min(), m_Curve.get_parameter_max(), output, uout, tol, Nlimit );
}

void Vsp1DCurve::TessAdapt( double umin, double umax, std::vector< double > & pnts, vector< double > & uout, double tol, int Nlimit )
{
    double pmin = CompPnt( umin );
    double pmax = CompPnt( umax );

    TessAdapt( umin, umax, pmin, pmax, pnts, uout, tol, Nlimit );

    pnts.push_back( pmax );
    uout.push_back( umax );
}

void Vsp1DCurve::TessAdapt( double umin, double umax, const double & pmin, const double & pmax, std::vector< double > & pnts, vector< double > & uout, double tol, int Nlimit, int Nadapt )
{
    double umid = ( umin + umax ) * 0.5;

    double pmid = CompPnt( umid );

    double d = std::abs( pmid - ( pmax + pmin ) / 2.0 );

    if ( ( d > tol && Nlimit > 0 ) || Nadapt < 3 )
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

void Vsp1DCurve::Scale( double s )
{
    m_Curve.scale( s );
}

void Vsp1DCurve::Reverse()
{
    m_Curve.reverse();
}

void Vsp1DCurve::product( Vsp1DCurve c1, Vsp1DCurve c2 )
{
    vector < double > pmap;
    c1.m_Curve.get_pmap( pmap );

    vector < double > pmap2;
    c2.m_Curve.get_pmap( pmap2 );

    pmap.insert( pmap.end(), pmap2.begin(), pmap2.end() );
    std::sort( pmap.begin(), pmap.end() );
    auto pmit = std::unique( pmap.begin(), pmap.end() );
    pmap.erase( pmit, pmap.end() );

    for( int i = 0; i < pmap.size(); i++ )
    {
        c1.m_Curve.split( pmap[i] );
        c2.m_Curve.split( pmap[i] );
    }

    m_Curve.product1d( c1.m_Curve, c2.m_Curve );
}

bool Vsp1DCurve::IsEqual( const Vsp1DCurve & crv )
{
    int ns0 = m_Curve.number_segments();
    int ns1 = crv.m_Curve.number_segments();

    if ( ns0 != ns1 )
        return false;

    // get control points and print
    int i, pp;

    for ( pp=0 ; pp < ns0 ; ++pp )
    {
      oned_curve_segment_type bez0;
      oned_curve_segment_type bez1;

      m_Curve.get(bez0, pp);
      crv.m_Curve.get(bez1, pp);

      if ( bez0.degree() != bez1.degree() )
          return false;

      for (i=0; i<=bez0.degree(); ++i)
      {
          oned_curve_point_type cp0 = bez0.get_control_point(i);
          oned_curve_point_type cp1 = bez1.get_control_point(i);
          double v0 = cp0.x();
          double v1 = cp1.x();

          if ( std::abs( v0 - v1 ) > 1.0e-12 )
              return false;
      }
    }

    return true;

}

void Vsp1DCurve::GetBoundingBox( double &ymin, double &ymax ) const
{
    oned_curve_bounding_box_type bbx;

    m_Curve.get_bounding_box( bbx );
    ymin = bbx.get_min().x();
    ymax = bbx.get_max().x();
}

struct crv_functor
{
    double operator()( const double &r )
    {
        return m_crv->CompPnt( r );
    }
    Vsp1DCurve *m_crv;
};

struct crv_r_functor
{
    double operator()( const double &r )
    {
        return m_crv->CompPnt( r ) * r;
    }
    Vsp1DCurve *m_crv;
};

struct crv_rsq_functor
{
    double operator()( const double &r )
    {
        return m_crv->CompPnt( r ) * r * r;
    }
    Vsp1DCurve *m_crv;
};

struct crv_rcub_functor
{
    double operator()( const double &r )
    {
        return m_crv->CompPnt( r ) * r * r * r;
    }
    Vsp1DCurve *m_crv;
};

double Vsp1DCurve::IntegrateCrv()
{
    return IntegrateCrv( 0 );
}

double Vsp1DCurve::IntegrateCrv( double r0 )
{
    double rmin = m_Curve.get_t0();
    if ( r0 < rmin )
    {
        r0 = rmin;
    }

    crv_functor fun;
    fun.m_crv = this;

    eli::mutil::quad::simpson< double > quad;

    return quad( fun, r0, 1.0 );
}

// Calculate the integral of a given curve weighted by radius.
// The parameter itself is fraction of the radius.
double Vsp1DCurve::IntegrateCrv_r()
{
    return IntegrateCrv_r( 0 );
}

double Vsp1DCurve::IntegrateCrv_r( double r0 )
{
    double rmin = m_Curve.get_t0();
    if ( r0 < rmin )
    {
        r0 = rmin;
    }

    crv_r_functor fun;
    fun.m_crv = this;

    eli::mutil::quad::simpson< double > quad;

    return quad( fun, r0, 1.0 );
}

// Calculate the integral of a given curve weighted by radius squared.
// The parameter itself is fraction of the radius.
double Vsp1DCurve::IntegrateCrv_rsq()
{
    return IntegrateCrv_rsq( 0 );
}

double Vsp1DCurve::IntegrateCrv_rsq( double r0 )
{
    double rmin = m_Curve.get_t0();
    if ( r0 < rmin )
    {
        r0 = rmin;
    }

    crv_rsq_functor fun;
    fun.m_crv = this;

    eli::mutil::quad::simpson< double > quad;

    return quad( fun, r0, 1.0 );
}

// Calculate the integral of a given curve weighted by radius cubed.
// The parameter itself is fraction of the radius.
double Vsp1DCurve::IntegrateCrv_rcub()
{
    return IntegrateCrv_rcub( 0 );
}

double Vsp1DCurve::IntegrateCrv_rcub( double r0 )
{
    double rmin = m_Curve.get_t0();
    if ( r0 < rmin )
    {
        r0 = rmin;
    }

    crv_rcub_functor fun;
    fun.m_crv = this;

    eli::mutil::quad::simpson< double > quad;

    return quad( fun, r0, 1.0 );
}
