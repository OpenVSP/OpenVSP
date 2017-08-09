//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspCurve.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <cmath>

#include "VspCurve.h"
#include "APIDefines.h"


#include "eli/geom/curve/length.hpp"
#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/intersect/specified_distance_curve.hpp"
#include "eli/geom/intersect/specified_thickness_curve.hpp"
#include "eli/geom/intersect/minimum_dimension_curve.hpp"

typedef piecewise_curve_type::index_type curve_index_type;
typedef piecewise_curve_type::point_type curve_point_type;
typedef piecewise_curve_type::rotation_matrix_type curve_rotation_matrix_type;
typedef piecewise_curve_type::bounding_box_type curve_bounding_box_type;
typedef piecewise_curve_type::tolerance_type curve_tolerance_type;


typedef eli::geom::curve::piecewise_cubic_spline_creator<double, 3, curve_tolerance_type> piecewise_cubic_spline_creator_type;
typedef eli::geom::curve::piecewise_linear_creator<double, 3, curve_tolerance_type> piecewise_linear_creator_type;
typedef eli::geom::curve::piecewise_binary_cubic_creator<double, 3, curve_tolerance_type> piecewise_binary_cubic_creator;

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
void VspCurve::Copy( VspCurve & input_crv )
{
    m_Curve = input_crv.m_Curve;
}

//==== Split at Specified Parameter and Return Remaining Curve =====//
void VspCurve::Split( double u )
{
    m_Curve.split( u );
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
void VspCurve::Append( VspCurve & input_crv )
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

void VspCurve::RoundJoint( double rad, int i )
{
    m_Curve.round( rad, i );
}

void VspCurve::RoundAllJoints( double rad )
{
    m_Curve.round( rad );
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
void VspCurve::InterpolateLinear( vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag )
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
void VspCurve::InterpolatePCHIP( vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag )
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
void VspCurve::InterpolateCSpline( vector< vec3d > & input_pnt_vec, const vector<double> &param, bool closed_flag )
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
void VspCurve::InterpolateCSpline( vector< vec3d > & input_pnt_vec, const vec3d &start_slope, const vec3d &end_slope, const vector<double> &param )
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

void VspCurve::ToBinaryCubic( bool wingtype )
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

        // Setup copies base curve into creator.
        // tolerance, min adapt levels, max adapt levels
        pbcc.setup( low, 1e-6, 0.01, 2, 15 );
        // Create makes new curve
        pbcc.corner_create( low );

        pbcc.setup( up, 1e-6, 0.01, 2, 15 );
        pbcc.corner_create( up );

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
        pbcc.setup( low, 1e-6, 0.01, 2, 15 );
        // Create makes new curve
        pbcc.corner_create( m_Curve );

        pbcc.setup( up, 1e-6, 0.01, 2, 15 );
        pbcc.corner_create( up );

        m_Curve.push_back( up );

        m_Curve.set_tmax( tmax );
    }

}

void VspCurve::SetCubicControlPoints( const vector< vec3d > & cntrl_pts, bool closed_flag )
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

            vec3d p = cntrl_pts[k];

            curve_point_type cp;
            cp << p.x(), p.y(), p.z();

            c.set_control_point( cp, j );
        }
        m_Curve.push_back( c );
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

void VspCurve::AppendCurveSegment( curve_segment_type &c )
{
    m_Curve.push_back( c, 1 );
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

double VspCurve::FindThickness( double &u1, double &u2, const vec3d &pt, const double &thick, const double &u10, const double &u20 ) const
{
    double dist;
    curve_point_type p;
    p << pt.x(), pt.y(), pt.z();

    dist = eli::geom::intersect::specified_thickness( u1, u2, m_Curve, p, thick, u10, u20 );

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
vec3d VspCurve::CompPnt( double u )
{
    vec3d rtn;
    curve_point_type v( m_Curve.f( u ) );

    rtn.set_xyz( v.x(), v.y(), v.z() );
    return rtn;
}

//===== Compute Tangent  =====//
vec3d VspCurve::CompTan( double u )
{
    vec3d rtn;
    curve_point_type v( m_Curve.fp( u ) );

    rtn.set_xyz( v.x(), v.y(), v.z() );
    return rtn;
}

//===== Compute Point U 0.0 -> 1.0 =====//
vec3d VspCurve::CompPnt01( double u )
{
    return CompPnt( u * m_Curve.get_tmax() );
}


//===== Compute Tan U 0.0 -> 1.0 =====//
vec3d VspCurve::CompTan01( double u )
{
    return CompTan( u * m_Curve.get_tmax() );
}

//===== Compute Length =====//
double VspCurve::CompLength( double tol )
{
    double len = 0.0;
    eli::geom::curve::length( len, m_Curve, tol );

    return len;
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
    int num_pnts_u = u.size();
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
void VspCurve::TessAdapt( vector< vec3d > & output, double tol, int Nlimit  )
{
    TessAdapt( m_Curve.get_parameter_min(), m_Curve.get_parameter_max(), output, tol, Nlimit );
}

void VspCurve::TessAdapt( double umin, double umax, std::vector< vec3d > & pnts, double tol, int Nlimit )
{
    vec3d pmin = CompPnt( umin );
    vec3d pmax = CompPnt( umax );

    TessAdapt( umin, umax, pmin, pmax, pnts, tol, Nlimit );

    pnts.push_back( pmax );
}

void VspCurve::TessAdapt( double umin, double umax, const vec3d & pmin, const vec3d & pmax, std::vector< vec3d > & pnts, double tol, int Nlimit, int Nadapt )
{
    double umid = ( umin + umax ) * 0.5;

    vec3d pmid = CompPnt( umid );

    double d = dist_pnt_2_line( pmin, pmax, pmid ) / dist( pmin, pmax );

    if ( ( d > tol && Nlimit > 0 ) || Nadapt < 3 )
    {
        TessAdapt( umin, umid, pmin, pmid, pnts, tol, Nlimit - 1, Nadapt + 1 );
        TessAdapt( umid, umax, pmid, pmax, pnts, tol, Nlimit - 1, Nadapt + 1 );
    }
    else
    {
        pnts.push_back( pmin );
        pnts.push_back( pmid );
    }
}


//===== Offset =====//
void VspCurve::Offset( vec3d offvec )
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

void VspCurve::Reflect( vec3d axis )
{
    curve_point_type a;

    a << axis.x(), axis.y(), axis.z();
    m_Curve.reflect( a );
}

void VspCurve::Reflect( vec3d axis, double d )
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
