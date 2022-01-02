//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspSurf.cpp:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <cmath>
#include <algorithm>
#include <set>

#include "VspSurf.h"
#include "StlHelper.h"
#include "PntNodeMerge.h"
#include "Cluster.h"
#include "VspUtil.h"

#include "eli/geom/surface/piecewise_body_of_revolution_creator.hpp"
#include "eli/geom/surface/piecewise_multicap_surface_creator.hpp"
#include "eli/geom/intersect/minimum_distance_surface.hpp"
#include "eli/geom/intersect/distance_angle_surface.hpp"
#include "eli/geom/intersect/intersect_axis_surface.hpp"

typedef piecewise_surface_type::index_type surface_index_type;
typedef piecewise_surface_type::point_type surface_point_type;
typedef piecewise_surface_type::rotation_matrix_type surface_rotation_matrix_type;
typedef piecewise_surface_type::bounding_box_type surface_bounding_box_type;

typedef eli::geom::surface::piecewise_general_skinning_surface_creator<double, 3, surface_tolerance_type> general_creator_type;
typedef eli::geom::surface::piecewise_multicap_surface_creator<double, 3, surface_tolerance_type> multicap_creator_type;
typedef eli::geom::surface::piecewise_cubic_spline_skinning_surface_creator<double, 3, surface_tolerance_type> spline_creator_type;

typedef piecewise_curve_type::index_type curve_index_type;
typedef piecewise_curve_type::point_type curve_point_type;
typedef piecewise_curve_type::rotation_matrix_type curve_rotation_matrix_type;
typedef piecewise_curve_type::tolerance_type curve_tolerance_type;

typedef eli::geom::curve::piecewise_point_creator<double, 3, curve_tolerance_type> piecewise_point_creator;
typedef eli::geom::curve::piecewise_circle_creator<double, 3, curve_tolerance_type> piecewise_circle_creator;

//===== Constructor  =====//
VspSurf::VspSurf()
{
    m_FlipNormal = false;
    m_MagicVParm = false;
    m_HalfBOR = false;
    m_SurfType = vsp::NORMAL_SURF;
    m_SurfCfdType = vsp::CFD_NORMAL;
    m_SkinType = SKIN_NONE;

    m_CloneIndex = -1;

    SetClustering( 1.0, 1.0 );

    m_FoilSurf = NULL;
    m_LECluster = 1.0;
    m_TECluster = 1.0;
    m_SkinClosedFlag = 0;
}

//===== Destructor  =====//
VspSurf::~VspSurf()
{
}

void VspSurf::SetClustering( const double &le, const double &te )
{
    m_LECluster = le;
    m_TECluster = te;
}

void VspSurf::SetRootTipClustering( const vector < double > &root, const vector < double > &tip ) const
{
    m_RootCluster = root;
    m_TipCluster = tip;
}

int VspSurf::GetNumSectU() const
{
    return m_Surface.number_u_patches();
}

int VspSurf::GetNumSectW() const
{
    return m_Surface.number_v_patches();
}

double VspSurf::GetUMax() const
{
  return m_Surface.get_umax();
}

double VspSurf::GetWMax() const
{
  return m_Surface.get_vmax();
}

void VspSurf::ReverseUDirection()
{
    m_Surface.reverse_u();
}

void VspSurf::ReverseWDirection()
{
    m_Surface.reverse_v();
}

//==== Flip U/W Directions =====//
void VspSurf::SwapUWDirections()
{
    m_Surface.swap_uv();
}

//==== Transform Control Points =====//
void VspSurf::Transform( Matrix4d & mat )
{
    surface_rotation_matrix_type rmat;
    double *mmat( mat.data() );
    surface_point_type trans;

    rmat << mmat[0], mmat[4], mmat[8],
         mmat[1], mmat[5], mmat[9],
         mmat[2], mmat[6], mmat[10];
    trans << mmat[12], mmat[13], mmat[14];

    m_Surface.rotate( rmat );
    m_Surface.translate( trans );
}

void VspSurf::GetBoundingBox( BndBox &bb ) const
{
    surface_bounding_box_type bbx;
    vec3d v3min, v3max;

    m_Surface.get_bounding_box( bbx );
    v3min.set_xyz( bbx.get_min().x(), bbx.get_min().y(), bbx.get_min().z() );
    v3max.set_xyz( bbx.get_max().x(), bbx.get_max().y(), bbx.get_max().z() );
    bb.Reset();
    bb.Update( v3min );
    bb.Update( v3max );
}

void VspSurf::GetLimitedBoundingBox( BndBox &bb, const double &U0, const double &Uf, const double &W0, const double &Wf )
{
    surface_bounding_box_type bbx;
    vec3d v3min, v3max;

    m_Surface.get_bounding_box( bbx, U0, Uf, W0, Wf );
    v3min.set_xyz( bbx.get_min().x(), bbx.get_min().y(), bbx.get_min().z() );
    v3max.set_xyz( bbx.get_max().x(), bbx.get_max().y(), bbx.get_max().z() );
    bb.Reset();
    bb.Update( v3min );
    bb.Update( v3max );
}

bool VspSurf::IsClosedU() const
{
    return m_Surface.closed_u();
}

bool VspSurf::IsClosedW() const
{
    return m_Surface.closed_v();
}

double VspSurf::FindNearest( double &u, double &w, const vec3d &pt ) const
{
    double dist;
    surface_point_type p;
    p << pt.x(), pt.y(), pt.z();

    dist = eli::geom::intersect::minimum_distance( u, w, m_Surface, p );

    return dist;
}

double VspSurf::FindNearest( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const
{
    double dist;
    surface_point_type p;
    p << pt.x(), pt.y(), pt.z();

    double u0in = u0;
    double w0in = w0;

    u0in = clamp( u0in, 0.0, GetUMax() );
    w0in = clamp( w0in, 0.0, GetWMax() );

    dist = eli::geom::intersect::minimum_distance( u, w, m_Surface, p, u0in, w0in );

    return dist;
}

double VspSurf::FindNearest01( double &u, double &w, const vec3d &pt ) const
{
    double dist;

    dist = FindNearest( u, w, pt );

    u = u / GetUMax();
    w = w / GetWMax();

    return dist;
}

double VspSurf::FindNearest01( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const
{
    double dist;

    dist = FindNearest( u, w, pt, u0 * GetUMax(), w0 * GetWMax() );

    u = u / GetUMax();
    w = w / GetWMax();

    return dist;
}

void VspSurf::FindDistanceAngle( double &u, double &w, const vec3d &pt, const vec3d &dir, const double &d, const double &theta, const double &u0, const double &w0 ) const
{
    surface_point_type p, dr;
    p << pt.x(), pt.y(), pt.z();
    dr << dir.x(), dir.y(), dir.z();

    double dot = cos( theta );

    double wllim, wulim;
    if ( w0 < 2.0 )
    {
        wllim = 0.0 + TMAGIC;
        wulim = 2.0 - TMAGIC;
    }
    else
    {
        wllim = 2.0 + TMAGIC;
        wulim = 4.0 - TMAGIC;
    }

    double wguess = w0;
    if ( wguess <= wllim )
    {
        wguess = wllim + 1e-6;
    }
    else if ( wguess >= wulim )
    {
        wguess = wulim - 1e-6;
    }

    double umin = m_Surface.get_u0();
    double umax = m_Surface.get_umax();
    double uguess = u0;
    if ( uguess <= umin )
    {
        uguess = umin + 1e-6;
    }
    else if ( uguess >= umax )
    {
        uguess = umax - 1e-6;
    }

    int retval;
    eli::geom::intersect::distance_angle( u, w, m_Surface, p, dr, d*d, dot, uguess, wguess, wllim, wulim, retval );
}

void VspSurf::GuessDistanceAngle( double &du, double &dw, const vec3d &udir, const vec3d & wdir, const double &d, const double &theta ) const
{
    if ( udir.mag() < 1e-6 )
    {
        dw = d / wdir.mag();
        du = 0;
    }
    else
    {
        double k = dot( wdir, udir ) / dot( udir, udir );
        vec3d wproju = udir * k;
        vec3d ndir = wdir - wproju;

        double dn = sin ( theta ) / ndir.mag();
        dw = d * dn;

        du = d * ( cos( theta ) / udir.mag() - k * dn );
    }
}

double VspSurf::ProjectPt( const vec3d &inpt, const int &idir, double &u_out, double &w_out, vec3d &outpt ) const
{
    surface_point_type p, p0;
    p0 << inpt.x(), inpt.y(), inpt.z();

    double idist = eli::geom::intersect::intersect( u_out, w_out, p, m_Surface, p0, idir );

    outpt = p;
    return idist;
}

double VspSurf::ProjectPt( const vec3d &inpt, const int &idir, const double &u0, const double &w0, double &u_out, double &w_out, vec3d &outpt ) const
{
    surface_point_type p, p0;
    p0 << inpt.x(), inpt.y(), inpt.z();

    double idist = eli::geom::intersect::intersect( u_out, w_out, p, m_Surface, p0, idir, u0, w0 );

    outpt = p;
    return idist;
}

double VspSurf::ProjectPt01( const vec3d &inpt, const int &idir, double &u_out, double &w_out, vec3d &outpt) const
{
    double idist = ProjectPt( inpt, idir, u_out, w_out, outpt );

    u_out = u_out / GetUMax();
    w_out = w_out / GetWMax();

    return idist;
}

double VspSurf::ProjectPt01( const vec3d &inpt, const int &idir, const double &u0, const double &w0, double &u_out, double &w_out, vec3d & outpt ) const
{
    double idist = ProjectPt( inpt, idir, u0 * GetUMax(), w0 * GetWMax(), u_out, w_out, outpt );

    u_out = u_out / GetUMax();
    w_out = w_out / GetWMax();

    return idist;
}

void VspSurf::GetUConstCurve( VspCurve &c, const double &u ) const
{
    piecewise_curve_type pwc;

    double uin = u;
    uin = clamp( uin, 0.0, GetUMax() );

    m_Surface.get_uconst_curve(pwc, uin);
    c.SetCurve(pwc);
}

void VspSurf::GetWConstCurve( VspCurve &c, const double &w ) const
{
    piecewise_curve_type pwc;

    double win = w;
    win = clamp( win, 0.0, GetWMax() );

    m_Surface.get_vconst_curve(pwc, win);
    c.SetCurve(pwc);
}

void VspSurf::GetU01ConstCurve( VspCurve &c, const double &u01 ) const
{
    GetUConstCurve( c, u01 * GetUMax() );
}

void VspSurf::GetW01ConstCurve( VspCurve &c, const double &w01 ) const
{
    GetWConstCurve( c, w01 * GetWMax() );
}

//===== Compute a Relative Rotation Transformation Matrix from Component's
//      Coordinate System to a Surface Coordinate System ====//
Matrix4d VspSurf::CompRotCoordSys( const double &u, const double &w )
{
    Matrix4d retMat; // Return Matrix

    // Get du and norm, cross them to get the last orthonormal vector
    vec3d du = CompTanU01( u, w );
    du.normalize();
    vec3d norm = CompNorm01( u, w ); // use CompNorm01 since normals now face outward
    norm.normalize();

    if ( m_MagicVParm ) // Surfs with magic parameter treatment (wings) have added chances to degenerate
    {
        double tmagic01 = TMAGIC / GetWMax();

        if ( du.mag() < 1e-6 ) // Zero direction vector
        {
            if ( w <= tmagic01 ) // Near TE lower
            {
                du = CompTanU01( u, tmagic01 + 1e-6 );
                du.normalize();
            }

            if ( w >= ( 0.5 - tmagic01 ) && w <= ( 0.5 + tmagic01 ) ) // Near leading edge
            {
                du = CompTanU01( u, 0.5 + tmagic01 + 1e-6 );
                du.normalize();
            }

            if ( w >= ( 1.0 - tmagic01 ) ) // Near TE upper
            {
                du = CompTanU01( u, 1.0 - ( tmagic01 + 1e-6 ) );
                du.normalize();
            }
        }

        if ( norm.mag() < 1e-6 ) // Zero normal vector
        {
            if ( w <= tmagic01 ) // Near TE lower
            {
                norm = CompNorm01( u, tmagic01 + 1e-6 );
                norm.normalize();
            }

            if ( w >= ( 0.5 - tmagic01 ) && w <= ( 0.5 + tmagic01 ) ) // Near leading edge
            {
                norm = CompNorm01( u, 0.5 + tmagic01 + 1e-6 );
                norm.normalize();
            }

            if ( w >= ( 1.0 - tmagic01 ) ) // Near TE upper
            {
                norm = CompNorm01( u, 1.0 - ( tmagic01 + 1e-6 ) );
                norm.normalize();
            }
        }
    }

    vec3d dw = cross( norm, du );

    // Place axes in as cols of Rot mat
    retMat.setBasis( du, dw, norm );
    return retMat;
}

//===== Compute a Relative Translation Transformation Matrix from Component's
//      Coordinate System to a Surface Coordinate System ====//
Matrix4d VspSurf::CompTransCoordSys( const double &u, const double &w )
{
    Matrix4d retMat; // Return Matrix

    // Get x,y,z location of u,w coordinate and place in translation matrix
    vec3d cartCoords = CompPnt01( u, w );
    retMat.translatef( cartCoords.x(), cartCoords.y(), cartCoords.z() );
    return retMat;
}

void VspSurf::CreateBodyRevolution( const VspCurve &input_crv, bool match_uparm )
{
    eli::geom::surface::create_body_of_revolution( m_Surface, input_crv.GetCurve(), 0, true, match_uparm );

    ResetFlipNormal();
    ResetUSkip();

    //==== Store Skinning Data ====//
    m_SkinType = SKIN_BODY_REV;
    m_BodyRevCurve = input_crv;
}

void VspSurf::CreateDisk( double dia, int ix, int iy )
{
    vector< VspCurve > crv_vec(2);

    piecewise_curve_type c;
    curve_point_type origin;
    origin << 0, 0, 0;

    // create point with 4 segments
    piecewise_point_creator ppc( 4 );

    // set point, make sure have 4 sections that go from 0 to 4
    ppc.set_point( origin );
    ppc.set_t0( 0 );
    ppc.set_segment_dt( 1, 0 );
    ppc.set_segment_dt( 1, 1 );
    ppc.set_segment_dt( 1, 2 );
    ppc.set_segment_dt( 1, 3 );

    if ( !ppc.create( c ) )
    {
        std::cerr << "Failed to create point for disk. " << __LINE__ << std::endl;
    }
    else
    {
        crv_vec[0].SetCurve( c );
    }

    piecewise_circle_creator pcc( 4 );
    curve_point_type start, xdir, ydir;

    xdir = origin;
    ydir = origin;

    xdir( ix ) = 1.0;
    ydir( iy ) = 1.0;

    // set circle params, make sure that entire curve goes from 0 to 4
    pcc.set_origin( origin );
    pcc.set_radius( dia / 2.0 );
    pcc.set_xy_directions( xdir, ydir );
    pcc.set_t0( 0 );
    pcc.set_segment_dt( 1, 0 );
    pcc.set_segment_dt( 1, 1 );
    pcc.set_segment_dt( 1, 2 );
    pcc.set_segment_dt( 1, 3 );

    if ( !pcc.create( c ) )
    {
        std::cerr << "Failed to create circle for disk. " << __LINE__ << std::endl;
    }
    else
    {
        c.reverse();
        crv_vec[1].SetCurve( c );
    }

    SkinC0( crv_vec, false );
}

void VspSurf::SkinRibs( const vector<rib_data_type> &ribs, const vector < int > &degree, const vector < double > & param, bool closed_flag )
{
    general_creator_type gc;
    surface_index_type nrib, i;

    nrib = ribs.size();

    std::vector<typename general_creator_type::index_type> max_degree( nrib - 1, 0 );

    assert( degree.size() == nrib - 1 );
    for( i = 0; i < nrib - 1; i++ )
    {
        max_degree[i] = degree[i];
    }

    // create surface
    bool setcond = gc.set_conditions(ribs, max_degree, closed_flag);
    assert( setcond );

    if ( !setcond )
    {
        printf( "Failure in SkinRibs set_conditions\n" );
    }

    // set the delta u for each surface segment
    gc.set_u0( param[0] );
    for ( i = 0; i < gc.get_number_u_segments(); ++i )
    {
        gc.set_segment_du( param[i + 1] - param[i], i );
    }

    m_Surface.clear();
    bool creat = gc.create( m_Surface );

    if ( !creat )
    {
        printf( "Failure in SkinRibs create\n" );
    }

    ResetFlipNormal();
    ResetUSkip();

    //==== Store Skinning Data ====//
    m_SkinType = SKIN_RIBS;
    m_SkinRibVec = ribs;
    m_SkinDegreeVec = degree;
    m_SkinParmVec = param;
    m_SkinClosedFlag = closed_flag;

}

void VspSurf::SkinRibs( const vector<rib_data_type> &ribs, const vector < double > & param, bool closed_flag )
{
    surface_index_type nrib;
    nrib = ribs.size();
    vector< int > degree( nrib - 1, 0 );
    SkinRibs( ribs, degree, param, closed_flag );
}

void VspSurf::SkinRibs( const vector<rib_data_type> &ribs, const vector < int > &degree, bool closed_flag )
{
    surface_index_type nrib;
    nrib = ribs.size();
    vector< double > param( nrib );
    for ( int i = 0; i < nrib; i++ )
    {
        param[i] = 1.0 * i;
    }
    SkinRibs( ribs, degree, param, closed_flag );
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinRibs( const vector<rib_data_type> &ribs, bool closed_flag )
{
    surface_index_type nrib;
    nrib = ribs.size();
    vector< int > degree( nrib - 1, 0 );
    SkinRibs( ribs, degree, closed_flag );
}

void VspSurf::SkinCubicSpline( const vector<rib_data_type> &ribs, const vector<double> &param, const vector <double> &tdisc, const vector < int > &degree, bool closed_flag )
{
    spline_creator_type sc;
    surface_index_type nrib, i;

    nrib = ribs.size();

    std::vector<typename spline_creator_type::index_type> max_degree( nrib - 1, 0 );

    assert( degree.size() == nrib - 1 );
    for( i = 0; i < nrib - 1; i++ )
    {
        max_degree[i] = degree[i];
    }

    // create surface
    bool setcond = sc.set_conditions(ribs, max_degree, closed_flag);
    assert( setcond );

    if ( !setcond )
    {
        printf( "Failure in SkinCubicSpline set_conditions\n" );
    }

    // set the delta u for each surface segment
    sc.set_u0( param[0] );
    for ( i = 0; i < sc.get_number_u_segments(); ++i )
    {
        sc.set_segment_du( param[i + 1] - param[i], i );
    }

    sc.set_tdisc( tdisc );

    m_Surface.clear();
    bool creat = sc.create( m_Surface );

    if ( !creat )
    {
        printf( "Failure in SkinCubicSpline create\n" );
    }

    ResetFlipNormal();
    ResetUSkip();
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinCubicSpline( const vector<rib_data_type> &ribs, const vector<double> &param, const vector <double> &tdisc, bool closed_flag )
{
    surface_index_type nrib;
    nrib = ribs.size();
    vector< int > degree( nrib - 1, 0 );
    SkinCubicSpline( ribs, param, tdisc, degree, closed_flag );
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinCubicSpline( const vector<rib_data_type> &ribs, const vector<double> &param, bool closed_flag )
{
    surface_index_type nrib;
    nrib = ribs.size();
    vector< int > degree( nrib - 1, 0 );
    vector < double > tdisc(2);
    tdisc[0] = param[0];
    tdisc[1] = param.back();

    SkinCubicSpline( ribs, param, tdisc, degree, closed_flag );
}

void VspSurf::SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, const vector < int > &degree, const vector<double> &param, bool closed_flag )
{
    general_creator_type gc;
    surface_index_type i, ncrv;

    ncrv = input_crv_vec.size();

    std::vector<rib_data_type> ribs( ncrv );

    for( i = 0; i < ncrv; i++ )
    {
        ribs[i].set_f( input_crv_vec[i].GetCurve() );
    }

    for( i = 1; i < ncrv-1; i++ )
    {
        ribs[i].set_continuity( ( rib_data_type::connection_continuity ) cx[i] );
    }

    // create surface
    SkinRibs( ribs, degree, param, closed_flag );
}

void VspSurf::SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, const vector< int > &degree, bool closed_flag )
{
    surface_index_type ncrv;
    ncrv = input_crv_vec.size();
    vector< double > param( ncrv );
    for ( int i = 0; i < ncrv; i++ )
    {
        param[i] = 1.0 * i;
    }

    SkinCX( input_crv_vec, cx, degree, param, closed_flag );
}

void VspSurf::SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, const vector< double > &param, bool closed_flag )
{
    surface_index_type ncrv;

    ncrv = input_crv_vec.size();
    vector< int > degree( ncrv - 1, 0 );

    SkinCX( input_crv_vec, cx, degree, param, closed_flag );
}

void VspSurf::SkinCX( const vector< VspCurve > &input_crv_vec, int cx, const vector< double > &param, bool closed_flag )
{
    surface_index_type ncrv;

    ncrv = input_crv_vec.size();

    vector < int > cxv( ncrv, cx );
    cxv[ 0 ] = 0;
    cxv[ ncrv - 1 ] = 0;

    vector< int > degree( ncrv - 1, 0 );

    SkinCX( input_crv_vec, cxv, degree, param, closed_flag );
}

void VspSurf::SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, bool closed_flag )
{
    surface_index_type ncrv;

    ncrv = input_crv_vec.size();
    vector< int > degree( ncrv - 1, 0 );
    SkinCX( input_crv_vec, cx, degree, closed_flag );
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinCX( const vector< VspCurve > &input_crv_vec, int cx, bool closed_flag  )
{
    surface_index_type ncrv;

    ncrv = input_crv_vec.size();

    vector < int > cxv( ncrv, cx );
    cxv[ 0 ] = 0;
    cxv[ ncrv - 1 ] = 0;

    SkinCX( input_crv_vec, cxv, closed_flag );
}

void VspSurf::SkinC0( const vector< VspCurve > &input_crv_vec, const vector<double> &param, bool closed_flag )
{
    SkinCX( input_crv_vec, rib_data_type::C0, param, closed_flag );
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinC0( const vector< VspCurve > &input_crv_vec, bool closed_flag )
{
    SkinCX( input_crv_vec, rib_data_type::C0, closed_flag );
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinC1( const vector< VspCurve > &input_crv_vec, bool closed_flag )
{
    SkinCX( input_crv_vec, rib_data_type::C1, closed_flag );
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinC2( const vector< VspCurve > &input_crv_vec, bool closed_flag )
{
    SkinCX( input_crv_vec, rib_data_type::C2, closed_flag );
}

//===== Compute Point On Surf Given  U V (Between 0 1 ) =====//
vec3d VspSurf::CompPnt01( double u, double v ) const
{
    return CompPnt( u * GetUMax(), v * GetWMax() );
}

//===== Compute Tangent In U Direction   =====//
vec3d VspSurf::CompTanU01( double u01, double v01 ) const
{
    return CompTanU( u01 * GetUMax(), v01 * GetWMax() ) * GetUMax();
}

//===== Compute Tangent In W Direction   =====//
vec3d VspSurf::CompTanW01( double u01, double v01 ) const
{
    return CompTanW( u01 * GetUMax(), v01 * GetWMax() ) * GetWMax();
}

//===== Compute Second Derivative U,U   =====//
vec3d VspSurf::CompTanUU01( double u01, double v01 ) const
{
    return CompTanUU( u01 * GetUMax(), v01 * GetWMax() ) * ( GetUMax() * GetUMax() );
}

//===== Compute Second Derivative W,W   =====//
vec3d VspSurf::CompTanWW01( double u01, double v01 ) const
{
    return CompTanWW( u01 * GetUMax(), v01 * GetWMax() ) * ( GetWMax() * GetWMax() );
}

//===== Compute Second Derivative U,W   =====//
vec3d VspSurf::CompTanUW01( double u01, double v01 ) const
{
    return CompTanUW( u01 * GetUMax(), v01 * GetWMax() ) * ( GetUMax() * GetWMax() );
}

//===== Compute Second Derivative U,U   =====//
vec3d VspSurf::CompTanUU( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_uu( u, v ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Second Derivative W,W   =====//
vec3d VspSurf::CompTanWW( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_vv( u, v ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Second Derivative U,W   =====//
vec3d VspSurf::CompTanUW( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_uv( u, v ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Tangent In U Direction   =====//
vec3d VspSurf::CompTanU( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_u( u, v ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Tangent In W Direction   =====//
vec3d VspSurf::CompTanW( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_v( u, v ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Point On Surf Given  U W =====//
vec3d VspSurf::CompPnt( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f( u, v ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Normal  =====//
vec3d VspSurf::CompNorm( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.normal( u, v ) );

    if ( m_FlipNormal )
    {
        p = -p;
    }

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Normal  0->1.0  =====//
vec3d VspSurf::CompNorm01( double u01, double v01 ) const
{
    return CompNorm( u01 * GetUMax(), v01 * GetWMax() );
}

//===== Compute Surface Curvature Metrics Given  U W =====//
void VspSurf::CompCurvature( double u, double w, double& k1, double& k2, double& ka, double& kg ) const
{
    double umn = m_Surface.get_u0();
    double wmn = m_Surface.get_v0();

    double umx = m_Surface.get_umax();
    double wmx = m_Surface.get_vmax();

    double slop = 1e-3;
    if( u < (umn - slop) || w < (wmn - slop) || u > (umx + slop) || w > (wmx + slop) )
    {
        printf("BAD parameter in VspSurf::CompCurvature! %f %f\n", u, w );
        assert(false);
    }

    if ( u < umn )
        u = umn;

    if ( w < wmn )
        w = wmn;

    if ( u > umx )
        u = umx;

    if ( w > wmx )
        w = wmx;

    double tol = 1e-10;

    double bump = 1e-3;

    // First derivative vectors
    vec3d S_u = CompTanU( u, w );
    vec3d S_w = CompTanW( u, w );

    double E = dot( S_u, S_u );
    double G = dot( S_w, S_w );

    if( E < tol && G < tol )
    {
        double umid = ( umn + umx ) / 2.0;
        double wmid = ( wmn + wmx ) / 2.0;

        u = u + ( umid - u ) * bump;
        w = w + ( wmid - w ) * bump;

        S_u = CompTanU( u, w );
        S_w = CompTanW( u, w );

        E = dot( S_u, S_u );
        G = dot( S_w, S_w );
    }
    else if( E < tol ) // U direction degenerate
    {
        double wmid = ( wmn + wmx ) / 2.0;
        w = w + ( wmid - w ) * bump;

        S_u = CompTanU( u, w );
        S_w = CompTanW( u, w );

        E = dot( S_u, S_u );
        G = dot( S_w, S_w );
    }
    else if( G < tol ) // W direction degenerate
    {
        double umid = ( umn + umx ) / 2.0;
        u = u + ( umid - u ) * bump;

        S_u = CompTanU( u, w );
        S_w = CompTanW( u, w );

        E = dot( S_u, S_u );
        G = dot( S_w, S_w );
    }

    // Second derivative vectors
    vec3d S_uu = CompTanUU( u, w );
    vec3d S_uw = CompTanUW( u, w );
    vec3d S_ww = CompTanWW( u, w );

    // Unit normal vector
    vec3d Q = cross( S_u, S_w );
    Q.normalize();

    double F = dot( S_u, S_w );

    double L = dot( S_uu, Q );
    double M = dot( S_uw, Q );
    double N = dot( S_ww, Q );

    // Mean curvature
    ka = ( E * N + G * L - 2.0 * F * M ) / ( 2.0 * ( E * G - F * F ) );

    // Gaussian curvature
    kg = ( L * N - M * M ) / ( E * G - F * F );

    double b = sqrt( ka * ka - kg );

    // Principal curvatures
    double kmax = ka + b;
    double kmin = ka - b;

    // Ensure k1 has largest magnitude
    if( std::abs( kmax ) > std::abs( kmin ) )
    {
        k1 = kmax;
        k2 = kmin;
    }
    else
    {
        k1 = kmin;
        k2 = kmax;
    }
}

//===== Compute Normal  0->1.0  =====//
void VspSurf::CompCurvature01( double u01, double v01, double& k1, double& k2, double& ka, double& kg ) const
{
    CompCurvature( u01 * GetUMax(), v01 * GetWMax(), k1, k2, ka, kg );
}

void VspSurf::ResetUSkip() const
{
    piecewise_surface_type::index_type ip, nupatch, nwpatch;

    nupatch = m_Surface.number_u_patches();
    nwpatch = m_Surface.number_v_patches();

    m_RootCluster.resize( nupatch );
    m_TipCluster.resize( nupatch );

    m_USkip.resize( nupatch );

    for ( ip = 0; ip < nupatch; ip++ )
    {
        m_USkip[ip] = false;
        m_RootCluster[ip] = 1.0;
        m_TipCluster[ip] = 1.0;
    }
}

void VspSurf::SetUSkipFirst( bool f )
{
    if( !m_USkip.empty() )
    {
        m_USkip.front() = f;
    }
}

void VspSurf::SetUSkipLast( bool f )
{
    if( !m_USkip.empty() )
    {
        m_USkip.back() = f;
    }
}

void VspSurf::FlagDuplicate( const VspSurf &othersurf ) const
{
    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;

    nupatch = m_Surface.number_u_patches();
    nvpatch = m_Surface.number_v_patches();

    vector < int > umatchcnt( nupatch, 0 );
    vector < int > vmatchcnt( nvpatch, 0 );

    double tol = 0.00000001;

    for( ip = 0; ip < nupatch; ++ip )
    {
        for( jp = 0; jp < nvpatch; ++jp )
        {

            const surface_patch_type * patch = m_Surface.get_patch( ip, jp );
            const surface_patch_type * otherpatch = othersurf.m_Surface.get_patch( ip, jp );

            if ( patch->abouteq( *otherpatch, tol ) )
            {
                umatchcnt[ip]++;
                vmatchcnt[jp]++;
            }
            else
            {
                umatchcnt[ip]--;
                vmatchcnt[jp]--;
            }
        }
    }

    for( ip = 0; ip < nupatch; ++ip )
    {
        if ( umatchcnt[ip] == nvpatch )
        {
            m_USkip[ip] = true;
            othersurf.m_USkip[ip] = true;
        }
    }

}

void VspSurf::MakeUTess( const vector<int> &num_u, vector<double> &u, const std::vector<int> & umerge ) const
{
    if ( umerge.size() != 0 )
    {
        const unsigned int nusect = num_u.size();

        // Build merged version of m_USkip as uskip.
        vector <bool> uskip( nusect, false );
        int iusect = 0;
        for ( int i = 0; i < nusect; i++ )
        {
            uskip[i] = m_USkip[iusect];
            for (int j = 0; j < umerge[i]; j++)
            {
                iusect++;
            }
        }

        if ( nusect != umerge.size() )
        {
            printf( "Error.  num_u does not match umerge.\n" );
        }
        assert( uskip.size() == nusect );

        unsigned int nu = 1;
        for ( int i = 0; i < nusect; i++ )
        {
            if ( !uskip[i] )
            {
                nu += num_u[i] - 1;
            }
        }

        iusect = 0;
        double ustart = m_Surface.get_u0();
        double uend = ustart;

        u.resize( nu );
        int iu = 0;

        for ( int i = 0; i < nusect; i++ )
        {
            for ( int j = 0; j < umerge[i]; j++ )
            {
                double du, dv;
                surface_patch_type surf;
                m_Surface.get( surf, du, dv, iusect, 0 );

                uend += du;
                iusect++;
            }

            double du = uend - ustart;

            if ( !uskip[i] )
            {
                for ( int isecttess = 0; isecttess < num_u[i] - 1; ++isecttess )
                {
                    u[iu] = ustart + du * Cluster(static_cast<double>( isecttess ) / (num_u[i] - 1), m_RootCluster[i], m_TipCluster[i]);
                    iu++;
                }
            }

            if ( !( i == nusect - 1 && uskip[i] ) )
            {
                ustart = uend;
            }

        }
        u.back() = ustart;


    }
    else
    {
        surface_index_type nu;
        double umin;

        const int nusect = num_u.size();

        assert( m_USkip.size() == nusect );

        // calculate nu
        nu = 1;
        for ( int ii = 0; ii < nusect; ++ii )
        {
            if ( !m_USkip[ii] )
            {
                nu += num_u[ii] - 1;
            }
        }

        // calculate the u and v parameterizations
        umin = m_Surface.get_u0();

        u.resize( nu );
        double uumin( umin );
        size_t iusect;
        size_t iu = 0;
        for ( iusect = 0; iusect < (size_t)nusect; ++iusect )
        {
            double du, dv;
            surface_patch_type surf;
            m_Surface.get( surf, du, dv, iusect, 0 );

            if ( !m_USkip[ iusect] )
            {
                for ( int isecttess = 0; isecttess < num_u[iusect] - 1; ++isecttess )
                {
                    u[iu] = uumin + du * Cluster( static_cast<double>( isecttess ) / ( num_u[iusect] - 1 ), m_RootCluster[iusect], m_TipCluster[iusect] );
                    iu++;
                }
            }
            if ( !( iusect == nusect - 1 && m_USkip[ iusect ] ) )
            {
                uumin += du;
            }
        }
        u.back() = uumin;
    }
}

void VspSurf::MakeVTess( int num_v, std::vector<double> &vtess, const int &n_cap, bool degen ) const
{
    double vmin, vmax, vabsmin, vabsmax, vle, vlelow, vleup;
    surface_index_type nv( num_v );

    vmin = m_Surface.get_v0();
    vmax = m_Surface.get_vmax();

    vabsmin = vmin;
    vabsmax = vmax;

    double tol = 1e-8;

    // This case applies to wings, propellers, and flowthrough bodies of revolution
    if ( IsMagicVParm() && !IsHalfBOR() ) // V uses 'Magic' values for things like blunted TE.
    {
        vmin += TMAGIC;
        vmax -= TMAGIC;

        vle = ( vmin + vmax ) * 0.5;
        vlelow = vle - TMAGIC;
        vleup = vle + TMAGIC;

        vtess.resize(nv);
        int jle = ( nv - 1 ) / 2;
        int j = 0;
        if ( degen )
        {
            vtess[j] = vabsmin;
            j++;
        }
        for ( ; j < jle; ++j )
        {
            vtess[j] = vmin + ( vlelow - vmin ) * Cluster( 2.0 * static_cast<double>( j ) / ( nv - 1 ), m_TECluster, m_LECluster );
        }
        if ( degen )
        {
            vtess[j] = vle;
            j++;
        }
        for ( ; j < nv; ++j )
        {
            vtess[j] = vleup + ( vmax - vleup ) * ( 1.0 - Cluster( 1.0 - 2.0 * static_cast<double>( j - jle ) / ( nv - 1 ), m_TECluster, m_LECluster ));
        }
        if ( degen )
        {
            vtess[ nv - 1 ] = vabsmax;
        }

        if ( degen ) // DegenGeom, don't tessellate blunt TE or LE.
        {
            return;
        }

        piecewise_curve_type c1, c2;
        m_Surface.get_vconst_curve( c1, vabsmin );
        m_Surface.get_vconst_curve( c2, vmin );

        // Note: piecewise_curve_type::abouteq test is based on distance squared.
        if ( !c1.abouteq( c2, tol * tol ) ) // V Min edge is not repeated.
        {
            for ( j = 0; j < n_cap; j++ )
            {
                vtess.push_back( vabsmin + TMAGIC * j / (n_cap -1) );
            }
        }

        m_Surface.get_vconst_curve( c1, vabsmax );
        m_Surface.get_vconst_curve( c2, vmax );

        if ( !c1.abouteq( c2, tol * tol ) ) // V Max edge is not repeated.
        {
            for ( j = 0; j < n_cap; j++ )
            {
                vtess.push_back( vmax + TMAGIC * j / (n_cap -1) );
            }
        }

        m_Surface.get_vconst_curve( c1, vlelow );
        m_Surface.get_vconst_curve( c2, vleup );

        if ( !c1.abouteq( c2, tol * tol ) ) // Leading edge is not repeated.
        {
            for ( j = 0; j < n_cap * 2 - 1; j++ )
            {
                vtess.push_back( vlelow + TMAGIC * j / (n_cap -1) );
            }
        }

        // Sort parameters
        std::sort( vtess.begin(), vtess.end() );

        // Remove duplicate parameters
        vector < double >::iterator sit;
        sit=std::unique( vtess.begin(), vtess.end() );
        vtess.resize( distance( vtess.begin(), sit ) );
    }
    else if ( IsMagicVParm() && IsHalfBOR() ) // This case applies to upper/lower bodies of revolution
    {
        vmin += TMAGIC;
        vmax -= TMAGIC;

        vtess.resize(nv);
        int j = 0;
        if ( degen )
        {
            vtess[j] = vabsmin;
            j++;
        }
        for ( ; j < nv; ++j )
        {
            vtess[j] = vmin + ( vmax - vmin ) * Cluster( static_cast<double>( j ) / ( nv - 1 ), m_TECluster, m_LECluster );
        }
        if ( degen )
        {
            vtess[ nv - 1 ] = vabsmax;
        }

        if ( degen ) // DegenGeom, don't tessellate blunt TE or LE.
        {
            return;
        }

        piecewise_curve_type c1, c2;
        m_Surface.get_vconst_curve( c1, vabsmin );
        m_Surface.get_vconst_curve( c2, vmin );

        // Note: piecewise_curve_type::abouteq test is based on distance squared.
        if ( !c1.abouteq( c2, tol * tol ) ) // V Min edge is not repeated.
        {
            for ( j = 0; j < n_cap; j++ )
            {
                vtess.push_back( vabsmin + TMAGIC * j / (n_cap -1) );
            }
        }

        m_Surface.get_vconst_curve( c1, vabsmax );
        m_Surface.get_vconst_curve( c2, vmax );

        if ( !c1.abouteq( c2, tol * tol ) ) // V Max edge is not repeated.
        {
            for ( j = 0; j < n_cap; j++ )
            {
                vtess.push_back( vmax + TMAGIC * j / (n_cap -1) );
            }
        }

        // Sort parameters
        std::sort( vtess.begin(), vtess.end() );

        // Remove duplicate parameters
        vector < double >::iterator sit;
        sit=std::unique( vtess.begin(), vtess.end() );
        vtess.resize( distance( vtess.begin(), sit ) );
    }
    else // Magic values not employed on this surface.
    {
        vtess.resize(nv);
        for ( int j = 0; j < nv; ++j )
        {
            vtess[j] = vmin + ( vmax - vmin ) * static_cast<double>( j ) / ( nv - 1 );
        }
    }
}

//==== Tesselate Surface ====//
void VspSurf::TesselateTEforWake( vector< vector< vec3d > > & pnts ) const
{
    std::vector< vector< vec3d > > norms;
    std::vector< vector< vec3d > > uw_pnts;

    vector<double> u;
    m_Surface.get_pmap_u( u );

    vector<double> v(1);
    v[0] = 0.0;

    Tesselate( u, v, pnts, norms, uw_pnts );
}
void VspSurf::GetWakeTECurve( piecewise_curve_type & curve ) const
{
    m_Surface.get_vconst_curve( curve, 0 );
}

// FeaDome::UpdateDrawObjs
// Geom::UpdateTesselate
void VspSurf::Tesselate( int num_u, int num_v, vector< vector< vec3d > > & pnts, vector< vector< vec3d > > & norms, vector< vector< vec3d > > & uw_pnts, const int &n_cap, bool degen ) const
{
    vector<int> num_u_vec( GetNumSectU(), num_u );
    Tesselate( num_u_vec, num_v, pnts, norms, uw_pnts, n_cap, degen );
}

// Geom::UpdateSplitTesselate
void VspSurf::SplitTesselate( int num_u, int num_v, vector< vector< vector< vec3d > > > & pnts, vector< vector< vector< vec3d > > > & norms, const int &n_cap ) const
{
    vector<int> num_u_vec( GetNumSectU(), num_u );
    SplitTesselate( num_u_vec, num_v, pnts, norms, n_cap );
}

// VspSurf::Tesselate
// FuselageGeom::UpdateTesselate
// PropGeom::UpdateTesselate
// StackGeom::UpdateTesselate
// WingGeom::UpdateTesselate
void VspSurf::Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts, const int &n_cap, bool degen, const std::vector<int> & umerge ) const
{
    if( m_Surface.number_u_patches() == 0 || m_Surface.number_v_patches() == 0 )
    {
        return;
    }

    std::vector<double> u, v;

    MakeVTess( num_v, v, n_cap, degen );
    MakeUTess( num_u, u, umerge );

    Tesselate( u, v, pnts, norms, uw_pnts );
}

// VspSurf::SplitTesselate
// FuselageGeom::UpdateSplitTesselate
// PropGeom::UpdateSplitTesselate
// StackGeom::UpdateSplitTesselate
// WingGeom::UpdateSplitTesselate
void VspSurf::SplitTesselate( const vector<int> &num_u, int num_v, std::vector< vector< vector< vec3d > > > & pnts,  std::vector< vector< vector< vec3d > > > & norms, const int &n_cap, const std::vector<int> & umerge ) const
{
    if( m_Surface.number_u_patches() == 0 || m_Surface.number_v_patches() == 0 )
    {
        return;
    }

    std::vector<double> u, v;

    MakeVTess( num_v, v, n_cap, false );
    MakeUTess( num_u, u, umerge );

    SplitTesselate( m_UFeature, m_WFeature, u, v, pnts, norms );
}

// VspSurf::SplitTesselate
// VspSurf::Tesselate
// VspSurf::TesselateTEforWake
// Low level routine that actually calls Code-Eli to evaluate grid of points.
// No smarts about what U/V tess to work on, just evaluates what it is told.
// Also called by SplitTess below.
void VspSurf::Tesselate( const vector<double> &u, const vector<double> &v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts ) const
{
    if ( u.size() == 0 || v.size() == 0 )
    {
        printf( "ERROR: Empty U or V vectors \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return;
    }

    unsigned int nu = (unsigned int)u.size();
    unsigned int nv = (unsigned int)v.size();

    vector < vector < surface_point_type > > ptmat, nmat;

    m_Surface.f_pt_normal_grid( u, v, ptmat, nmat);

    // resize pnts and norms
    pnts.resize( nu );
    norms.resize( nu );
    uw_pnts.resize( nu );
    for ( surface_index_type i = 0; i < nu; ++i )
    {
        pnts[i].resize( nv );
        norms[i].resize( nv );
        uw_pnts[i].resize( nv );

        for ( surface_index_type j = 0; j < nv; j++ )
        {
            pnts[i][j] = ptmat[i][j];

            vec3d norm = nmat[i][j];
            if ( norm.mag() < 1e-6 ) // Zero normal vector
            {
                double tmax = GetWMax();
                double thalf = 0.5 * GetWMax();
                if ( v[j] <= TMAGIC ) // Near TE lower
                {
                    norm = CompNorm( u[i], TMAGIC + 1e-6 );
                }
                else if ( v[j] <= thalf && v[j] >= ( thalf - TMAGIC ) ) // Near leading edge
                {
                    norm = CompNorm( u[i], thalf - ( TMAGIC + 1e-6 ) );
                }
                else if ( v[j] >= thalf && v[j] <= ( thalf + TMAGIC ) ) // Near leading edge
                {
                    norm = CompNorm( u[i], thalf + TMAGIC + 1e-6 );
                }
                else if ( v[j] >= ( tmax - TMAGIC ) ) // Near TE upper
                {
                    norm = CompNorm( u[i], tmax - ( TMAGIC + 1e-6 ) );
                }
                norm.normalize();
            }

            if ( m_FlipNormal )
            {
                norms[i][j] = -1.0 * norm;
            }
            else
            {
                norms[i][j] = norm;
            }
            uw_pnts[i][j].set_xyz( u[i], v[j], 0.0 );
        }
    }
}

// VspSurf::SplitTesselate
// Low level SplitTesselate
// Does not figure out what U/V tess to work on, just does what it is told
// Splits at split lines, returning 3D vectors of results for patches.
void VspSurf::SplitTesselate( const vector<double> &usplit, const vector<double> &vsplit, const vector<double> &u, const vector<double> &v, std::vector< vector< vector< vec3d > > > & pnts,  std::vector< vector< vector< vec3d > > > & norms ) const
{
    if ( usplit.size() == 0 || vsplit.size() == 0 )
    {
        printf( "ERROR: Empty U or V split \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return;
    }
    vector < int > iusplit;
    iusplit.resize( usplit.size() );

    for ( int i = 0; i < usplit.size(); i++ )
    {
        double d = std::numeric_limits < double > ::max();

        int j = 0;
        if ( i > 0 )
        {
            j = iusplit[ i - 1 ];
        }
        for ( ; j < u.size(); j++ )
        {
            double dnew = std::abs( u[j] - usplit[i] );
            if ( dnew < d )
            {
                d = dnew;
                iusplit[i] = j;
            }
        }
    }

    vector < int > ivsplit;
    ivsplit.resize( vsplit.size() );

    for ( int i = 0; i < vsplit.size(); i++ )
    {
        double d = std::numeric_limits < double > ::max();

        int j = 0;
        if ( i > 0 )
        {
            j = ivsplit[ i - 1 ];
        }
        for ( ; j < v.size(); j++ )
        {
            double dnew = std::abs( v[j] - vsplit[i] );
            if ( dnew < d )
            {
                d = dnew;
                ivsplit[i] = j;
            }
        }
    }

    unsigned int nu = usplit.size() - 1;
    unsigned int nv = vsplit.size() - 1;

    unsigned int n = nu * nv;

    pnts.resize( n );
    norms.resize( n );

    vector< vector< vec3d > > uw_pnts;

    int k = 0;
    for ( int i = 0; i < nu; i++ )
    {
        vector < double > usubs( u.begin() + iusplit[i], u.begin() + iusplit[i+1] + 1 );

        for ( int j = 0; j < nv; j++ )
        {
            vector < double > vsubs( v.begin() + ivsplit[j], v.begin() + ivsplit[j+1] + 1 );

            Tesselate( usubs, vsubs, pnts[k], norms[k], uw_pnts );
            k++;
        }
    }
}

void VspSurf::TessUFeatureLine( int iu, std::vector< vec3d > & pnts, double tol ) const
{
    double u = m_UFeature[ iu ];

    double vmin, vmax;
    vec3d pmin, pmax;

    // Use WFeature points as starting point for adaptation for U lines because file-type
    // XSecs can be made of a large number of segments and using them would be inefficient.
    vmin = m_WFeature[0];
    pmin = CompPnt( u, vmin );

    pnts.clear();
    for ( int i = 0; i < m_WFeature.size() - 1; i++ )
    {
        vmax = m_WFeature[i+1];
        pmax = CompPnt( u, vmax );

        TessAdaptLine( u, u, vmin, vmax, pmin, pmax, pnts, tol, 10 );

        vmin = vmax;
        pmin = pmax;
    }
    pnts.push_back( pmax );
}

void VspSurf::TessWFeatureLine( int iw, std::vector< vec3d > & pnts, double tol ) const
{
    double v = m_WFeature[ iw ];

    vector < double > upts;
    m_Surface.get_pmap_u( upts );

    double umin, umax;
    vec3d pmin, pmax;

    // Use segment boundary points as starting point for adaptation for W lines because
    // not all segment boundaries are features and this helps resolve behavior along a body.
    umin = upts[0];
    pmin = CompPnt( umin, v );

    pnts.clear();
    for ( int i = 0; i < upts.size() - 1; i++ )
    {
        umax = upts[i+1];
        pmax = CompPnt( umax, v );

        TessAdaptLine( umin, umax, v, v, pmin, pmax, pnts, tol, 10 );

        umin = umax;
        pmin = pmax;
    }
    pnts.push_back( pmax );
}

void VspSurf::TessAdaptLine( double umin, double umax, double wmin, double wmax, std::vector< vec3d > & pnts, double tol, int Nlimit ) const
{
    vec3d pmin = CompPnt( umin, wmin );
    vec3d pmax = CompPnt( umax, wmax );

    TessAdaptLine( umin, umax, wmin, wmax, pmin, pmax, pnts, tol, Nlimit );

    pnts.push_back( pmax );
}

void VspSurf::TessAdaptLine( double umin, double umax, double wmin, double wmax, const vec3d & pmin, const vec3d & pmax, std::vector< vec3d > & pnts, double tol, int Nlimit, int Nadapt ) const
{
    double umid = ( umin + umax ) * 0.5;
    double wmid = ( wmin + wmax ) * 0.5;

    vec3d pmid = CompPnt( umid, wmid );

    double d = dist_pnt_2_line( pmin, pmax, pmid ) / dist( pmin, pmax );

    if ( ( d > tol && Nlimit > 0 ) || Nadapt < 2 )
    {
        TessAdaptLine( umin, umid, wmin, wmid, pmin, pmid, pnts, tol, Nlimit - 1, Nadapt + 1 );
        TessAdaptLine( umid, umax, wmid, wmax, pmid, pmax, pnts, tol, Nlimit - 1, Nadapt + 1 );
    }
    else
    {
        pnts.push_back( pmin );
        pnts.push_back( pmid );
    }
}

void VspSurf::BuildFeatureLines( bool force_xsec_flag)
{
    if ( m_Surface.number_u_patches() > 0 && m_Surface.number_v_patches() > 0 )
    {
        // Detect feature lines, clear()'s both vectors as first step. Any derivative vectors with
        // angle larger than acos(1-0.01)~8.1 deg will be considered a feature edge.
        m_Surface.find_interior_feature_edges( m_UFeature, m_WFeature, 0.01 );

        // Add start/end curves.
        double umin = m_Surface.get_u0();
        double umax = m_Surface.get_umax();
        double urng = umax - umin;
        m_UFeature.push_back( umin );
        m_UFeature.push_back( umax );

        if ( GetSurfType() == vsp::WING_SURF || force_xsec_flag )
        {
            // Force all patch boundaries in u direction.
            for ( double u = umin; u <= umax; u++ )
            {
                m_UFeature.push_back( u );
            }
        }

        // Add start/mid/end curves.
        double vmin = m_Surface.get_v0();
        double vmax = m_Surface.get_vmax();
        double vrng = vmax - vmin;
        double vmid = vmin + vrng * 0.5;

        m_WFeature.push_back( vmin );
        m_WFeature.push_back( vmid );
        m_WFeature.push_back( vmax );

        // If fuse-type, add .25 and .75 curves.
        if ( GetSurfType() != vsp::WING_SURF && GetSurfType() != vsp::PROP_SURF )
        {
            m_WFeature.push_back( vmin + 0.25 * vrng );
            m_WFeature.push_back( vmin + 0.75 * vrng );
        }
        else
        {
            if ( IsHalfBOR() )
            {
                m_WFeature.push_back(vmin + TMAGIC);
                m_WFeature.push_back(vmax - TMAGIC);
            }
            else
            {
                m_WFeature.push_back(vmin + TMAGIC);
                m_WFeature.push_back(vmid - TMAGIC);
                m_WFeature.push_back(vmid + TMAGIC);
                m_WFeature.push_back(vmax - TMAGIC);
            }
        }

        // Sort feature parameters
        std::sort( m_UFeature.begin(), m_UFeature.end() );
        std::sort( m_WFeature.begin(), m_WFeature.end() );

        // Remove duplicate feature parameters
        vector < double >::iterator sit;
        sit=std::unique( m_UFeature.begin(), m_UFeature.end() );
        m_UFeature.resize( distance( m_UFeature.begin(), sit ) );
        sit=std::unique( m_WFeature.begin(), m_WFeature.end() );
        m_WFeature.resize( distance( m_WFeature.begin(), sit ) );

        if ( m_UFeature.size() < 3 )
        {
            m_UFeature.push_back( umin + 0.5 * urng );

            // Sort feature parameters
            std::sort( m_UFeature.begin(), m_UFeature.end() );

            // Remove duplicate feature parameters
            sit=std::unique( m_UFeature.begin(), m_UFeature.end() );
            m_UFeature.resize( distance( m_UFeature.begin(), sit ) );
        }
    }
    else
    {
        m_UFeature.resize( 0 );
        m_WFeature.resize( 0 );
    }
}

bool VspSurf::CapUMin(int CapType, double len, double str, double offset, bool swflag)
{
    if (CapType == vsp::NO_END_CAP)
    {
        ResetUSkip();
        return false;
    }
    multicap_creator_type cc;
    bool rtn_flag;

    int captype = multicap_creator_type::FLAT;

    switch( CapType ){
      case vsp::FLAT_END_CAP:
        captype = multicap_creator_type::FLAT;
        break;
      case vsp::ROUND_END_CAP:
        captype = multicap_creator_type::ROUND;
        break;
      case vsp::EDGE_END_CAP:
        captype = multicap_creator_type::EDGE;
        break;
      case vsp::SHARP_END_CAP:
        captype = multicap_creator_type::SHARP;
        break;
    }

    rtn_flag = cc.set_conditions(m_Surface, captype, 1.0, multicap_creator_type::CAP_UMIN, len, offset, str, swflag );

    if (!rtn_flag)
    {
      ResetUSkip();
      return false;
    }

    rtn_flag = cc.create(m_Surface);

    if (!rtn_flag)
    {
      ResetUSkip();
      return false;
    }

    m_Surface.set_u0( 0.0 );
    ResetUSkip();
    return true;
}

bool VspSurf::CapUMax(int CapType, double len, double str, double offset, bool swflag)
{
    if (CapType == vsp::NO_END_CAP)
    {
      ResetUSkip();
      return false;
    }
    multicap_creator_type cc;
    bool rtn_flag;

    int captype = multicap_creator_type::FLAT;

    switch( CapType ){
      case vsp::FLAT_END_CAP:
        captype = multicap_creator_type::FLAT;
        break;
      case vsp::ROUND_END_CAP:
        captype = multicap_creator_type::ROUND;
        break;
      case vsp::EDGE_END_CAP:
        captype = multicap_creator_type::EDGE;
        break;
      case vsp::SHARP_END_CAP:
        captype = multicap_creator_type::SHARP;
        break;
    }

    rtn_flag = cc.set_conditions(m_Surface, captype, 1.0, multicap_creator_type::CAP_UMAX, len, offset, str, swflag );

    if (!rtn_flag)
    {
      ResetUSkip();
      return false;
    }

    rtn_flag = cc.create(m_Surface);

    if (!rtn_flag)
    {
      ResetUSkip();
      return false;
    }
    ResetUSkip();
    return true;
}

bool VspSurf::CapWMin(int CapType)
{
    if (CapType == vsp::NO_END_CAP)
      return false;

    std::cout << "Am Capping WMin on this one!" << std::endl;
    return false;
}

bool VspSurf::CapWMax(int CapType)
{
    if (CapType == vsp::NO_END_CAP)
      return false;

    std::cout << "Am Capping WMax on this one!" << std::endl;
    return false;
}

void VspSurf::SplitSurfs( vector< piecewise_surface_type > &surfvec, const vector < double > &usuppress, const vector < double > &wsuppress ) const
{
    vector < double > usplits = m_UFeature;
    vector < double > wsplits = m_WFeature;

    vector_remove_vector( usplits, usuppress );
    vector_remove_vector( wsplits, wsuppress );

    SplitSurfsU( surfvec, usplits );
    SplitSurfsW( surfvec, wsplits );
}

void VspSurf::SplitSurfs( vector< piecewise_surface_type > &surfvec ) const
{
    SplitSurfsU( surfvec, m_UFeature );
    SplitSurfsW( surfvec, m_WFeature );
}

void SplitSurfsU( vector< piecewise_surface_type > &surfvec, const vector < double > &USplit )
{
    for ( int i = 0; i < USplit.size(); ++i )
    {
        vector < piecewise_surface_type > splitsurfvec;
        for ( int j = 0; j < surfvec.size(); j++ )
        {
            piecewise_surface_type s, s1, s2;

            s = surfvec[j];

            if ( s.get_u0() < USplit[i] && s.get_umax() > USplit[i] )
            {
                s.split_u( s1, s2, USplit[i] );

                if ( s1.number_u_patches() > 0 && s1.number_v_patches() > 0 )
                {
                    splitsurfvec.push_back( s1 );
                }
                if ( s2.number_u_patches() > 0 && s2.number_v_patches() > 0 )
                {
                    splitsurfvec.push_back( s2 );
                }
            }
            else
            {
                splitsurfvec.push_back( s );
            }
        }
        surfvec = splitsurfvec;
    }
}

void SplitSurfsW( vector< piecewise_surface_type > &surfvec, const vector < double > &WSplit )
{
    for ( int i = 0; i < WSplit.size(); ++i )
    {
        vector < piecewise_surface_type > splitsurfvec;
        for ( int j = 0; j < surfvec.size(); j++ )
        {
            piecewise_surface_type s, s1, s2;

            s = surfvec[j];

            if ( s.get_v0() < WSplit[i] && s.get_vmax() > WSplit[i] )
            {
                s.split_v( s1, s2, WSplit[i] );

                if ( s1.number_u_patches() > 0 && s1.number_v_patches() > 0 )
                {
                    splitsurfvec.push_back( s1 );
                }
                if ( s2.number_u_patches() > 0 && s2.number_v_patches() > 0 )
                {
                    splitsurfvec.push_back( s2 );
                }
            }
            else
            {
                splitsurfvec.push_back( s );
            }
        }
        surfvec = splitsurfvec;
    }
}

// Check for degenerate patches by looking for coincident corners and edges.
// This will return false for normal watertight surfaces, so it should only be
// used to test patches from split surfaces.
bool VspSurf::CheckValidPatch( const piecewise_surface_type &surf )
{
    if ( surf.number_u_patches() == 0 || surf.number_v_patches() == 0 )
    {
        // Empty surface.
        return false;
    }

    double umin, vmin, umax, vmax;
    surf.get_parameter_min( umin, vmin );
    surf.get_parameter_max( umax, vmax );

    surface_point_type p1( surf.f( umin, vmin ) );
    surface_point_type p2( surf.f( umax, vmin ) );
    surface_point_type p3( surf.f( umax, vmax ) );
    surface_point_type p4( surf.f( umin, vmax ) );

    double d1 = ( p2 - p1 ).norm();
    double d2 = ( p3 - p2 ).norm();
    double d3 = ( p4 - p3 ).norm();
    double d4 = ( p1 - p4 ).norm();

    double tol = 1.0e-8;

    if ( ( d1 < tol && d2 < tol ) || ( d2 < tol && d3 < tol ) || ( d3 < tol && d4 < tol ) || ( d4 < tol && d1 < tol ) )
    {
        // Degenerate surface, skip it.
        // Two consecutive edges have collapsed, i.e. three corners are coincident.
        return false;
    }

    piecewise_curve_type c1, c2;
    surf.get_umin_bndy_curve( c1 );
    surf.get_umax_bndy_curve( c2 );

    // Note: piecewise_curve_type::abouteq test is based on distance squared.
    if ( c1.abouteq( c2, tol * tol ) )
    {
        // Degenerate surface, skip it.
        // Opposite edges are equal.
        return false;
    }

    surf.get_vmin_bndy_curve( c1 );
    surf.get_vmax_bndy_curve( c2 );

    if ( c1.abouteq( c2, tol * tol ) )
    {
        // Degenerate surface, skip it.
        // Opposite edges are equal.
        return false;
    }

    // Passed all tests, valid surface.
    return true;
}

void VspSurf::FetchXFerSurf( const std::string &geom_id, int surf_ind, int comp_ind, vector< XferSurf > &xfersurfs, const vector < double > &usuppress, const vector < double > &wsuppress ) const
{
    vector < piecewise_surface_type > surfvec;
    surfvec.push_back( m_Surface );
    SplitSurfs( surfvec, usuppress, wsuppress );

    int num_sections = surfvec.size();

    for ( int isect = 0; isect < num_sections; isect++ )
    {
        piecewise_surface_type surf = surfvec[isect];

        if ( !CheckValidPatch( surf ) )
        {
            continue;
        }

        // Made it through all checks, keep surface.
        XferSurf xsurf;
        xsurf.m_FlipNormal = m_FlipNormal;
        xsurf.m_Surface = surfvec[isect];
        xsurf.m_GeomID = geom_id;
        xsurf.m_SurfIndx = surf_ind;
        xsurf.m_SurfType = GetSurfType();
        xsurf.m_SurfCfdType = m_SurfCfdType;
        xsurf.m_CompIndx = comp_ind;
        xfersurfs.push_back( xsurf );
    }
}

void VspSurf::ToSTEP_BSpline_Quilt( STEPutil * step, vector<SdaiB_spline_surface_with_knots *> &surfs, const string& label, bool splitsurf, bool mergepts, bool tocubic, double tol, bool trimte, const vector < double > &USplit, const vector < double > &WSplit ) const
{
    vector < piecewise_surface_type > surfvec = PrepCADSurfs( splitsurf, tocubic, tol, trimte, USplit, WSplit );

    //==== Compute Tol ====//
    BndBox bbox;
    GetBoundingBox( bbox );
    double merge_tol = bbox.DiagDist() * 1.0e-10;

    if ( merge_tol < 1.0e-10 )
    {
        merge_tol = 1.0e-10;
    }

    for ( int isurf = 0; isurf < surfvec.size(); isurf++ )
    {
        piecewise_surface_type s = surfvec[isurf];

        if( !m_FlipNormal )
        {
            s.reverse_v();
        }

        // Don't export degenerate split patches
        if ( splitsurf && !CheckValidPatch( s ) )
        {
            continue;
        }

        SdaiSurface* surf = step->MakeSurf( s, label, mergepts, merge_tol );
        SdaiB_spline_surface_with_knots* nurbs = dynamic_cast<SdaiB_spline_surface_with_knots*>( surf );

        surfs.push_back( nurbs );
    }
}

void VspSurf::ToIGES( IGESutil* iges, bool splitsurf, bool tocubic, double tol, bool trimTE, const vector < double > &USplit, const vector < double > &WSplit, const string &labelprefix, bool labelSplitNo, const string &delim ) const
{
    vector < piecewise_surface_type > surfvec = PrepCADSurfs( splitsurf, tocubic, tol, trimTE, USplit, WSplit );

    for ( int is = 0; is < surfvec.size(); is++ )
    {
        piecewise_surface_type s = surfvec[is];

        if( !m_FlipNormal )
        {
            s.reverse_v();
        }

        // Don't export degenerate split patches
        if ( splitsurf && !CheckValidPatch( s ) )
        {
            continue;
        }

        string label = labelprefix;

        if ( labelSplitNo )
        {
            if ( label.size() > 0 )
            {
                label.append( delim );
            }
            label.append( to_string( is ) );
        }

        iges->MakeSurf( s, label );
    }
}

vector < piecewise_surface_type > VspSurf::PrepCADSurfs( bool splitsurf, bool tocubic, double tol, bool trimTE, const vector < double >& USplit, const vector < double >& WSplit ) const
{
    // Make copy for local changes.
    piecewise_surface_type s( m_Surface );

    if ( trimTE && m_MagicVParm )
    {
        piecewise_surface_type s1, s2;
        s.split_v( s1, s2, s.get_v0() + TMAGIC );
        s2.split_v( s, s1, s.get_vmax() - TMAGIC );
    }

    if ( tocubic )
    {
        // Only demote, do not promote
        piecewise_surface_type::index_type minu, minv;
        piecewise_surface_type::index_type maxu, maxv;
        s.degree_u( minu, maxu );
        s.degree_v( minv, maxv );

        if ( maxv > 3 )
        {
            s.to_cubic_u( tol );
        }
        if ( maxv > 3 )
        {
            s.to_cubic_v( tol );
        }
    }

    vector < piecewise_surface_type > surfvec;
    surfvec.push_back( s );
    if ( splitsurf )
    {
        SplitSurfs( surfvec );
    }

    SplitSurfsU( surfvec, USplit );
    SplitSurfsW( surfvec, WSplit );

    return surfvec;
}

void VspSurf::Offset( const vec3d &offvec )
{
    threed_point_type p;
    p << offvec.x(), offvec.y(), offvec.z();

    m_Surface.translate( p );
}

void VspSurf::OffsetX( double x )
{
    vec3d off( x, 0, 0 );
    Offset( off );
}

void VspSurf::OffsetY( double y )
{
    vec3d off( 0, y, 0 );
    Offset( off );
}

void VspSurf::OffsetZ( double z )
{
    vec3d off( 0, 0, z );
    Offset( off );
}

void VspSurf::Scale( double s )
{
    m_Surface.scale( s );
}

void VspSurf::ScaleX( double s )
{
    m_Surface.scale_x( s );
}

void VspSurf::ScaleY( double s )
{
    m_Surface.scale_y( s );
}

void VspSurf::ScaleZ( double s )
{
    m_Surface.scale_z( s );
}

void VspSurf::MakePlaneSurf( const vec3d &ptA, const vec3d &ptB, const vec3d &ptC, const vec3d &ptD )
{
    // This function is used to construct a plane, using the four inputs as corner points
    // If the inputs are not truly planar, the function will work, but the surface will be 
    // constructed via bi-linear interpolation

    surface_patch_type patch;
    patch.resize( 1, 1 );

    threed_point_type pt0, pt1, pt2, pt3;

    ptA.get_pnt( pt0 );
    ptB.get_pnt( pt1 );
    ptC.get_pnt( pt2 );
    ptD.get_pnt( pt3 );

    patch.set_control_point( pt0, 0, 0 );
    patch.set_control_point( pt1, 1, 0 );
    patch.set_control_point( pt2, 0, 1 );
    patch.set_control_point( pt3, 1, 1 );

    m_Surface.init_uv( 1, 1 );
    m_Surface.set( patch, 0, 0 );
}
