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
#include <cfloat>

#include "VspSurf.h"
#include "StlHelper.h"
#include "PntNodeMerge.h"
#include "Cluster.h"
#include "VspUtil.h"

#include "eli/geom/surface/piecewise_body_of_revolution_creator.hpp"
#include "eli/geom/surface/piecewise_multicap_surface_creator.hpp"
#include "eli/geom/intersect/find_rst_surface.hpp"
#include "eli/geom/intersect/minimum_distance_surface.hpp"
#include "eli/geom/intersect/distance_angle_surface.hpp"
#include "eli/geom/intersect/intersect_axis_surface.hpp"
#include "eli/geom/intersect/intersect_segment_surface.hpp"

#include "eli/geom/curve/bezier.hpp"
#include "eli/geom/curve/piecewise.hpp"

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
    m_ThickSurf = true;
    m_PlateNum = -1;
    m_SkinType = SKIN_NONE;

    m_FeaOrientationType = vsp::FEA_ORIENT_OML_U;
    m_FeaOrientation = vec3d();
    m_FeaSymmIndex = -1;

    m_CloneIndex = -1;

    SetClustering( 1.0, 1.0 );

    m_FoilSurf = nullptr;
    m_LECluster = 1.0;
    m_TECluster = 1.0;
    m_SkinClosedFlag = 0;

    m_UMapMax = -1;

    m_Lmax = 0;

    m_PlanarUWAspect = -1;
}

//===== Destructor  =====//
VspSurf::~VspSurf()
{
}

void VspSurf::CopyNonSurfaceData( const VspSurf & s )
{
    m_FlipNormal = s.m_FlipNormal;
    m_MagicVParm = s.m_MagicVParm;
    m_HalfBOR = s.m_HalfBOR;
    m_SurfType = s.m_SurfType;
    m_SurfCfdType = s.m_SurfCfdType;

    m_FeaOrientationType = s.m_FeaOrientationType;
    m_FeaOrientation = s.m_FeaOrientation;
    m_FeaSymmIndex = s.m_FeaSymmIndex;

    m_ThickSurf = s.m_ThickSurf;
    m_PlateNum = s.m_PlateNum;
    // Main surface, do not copy.
    // m_Surface

    m_UFeature = s.m_UFeature;
    m_WFeature = s.m_WFeature;

    m_UMapping = s.m_UMapping;
    m_UMapMax = s.m_UMapMax;

    m_Lmax = s.m_Lmax;
    m_LCurve = s.m_LCurve;

    m_LECluster = s.m_LECluster;
    m_TECluster = s.m_TECluster;

    //==== Store Skinning Inputs =====//
    m_SkinType = s.m_SkinType;
    m_BodyRevCurve = s.m_BodyRevCurve;
    m_SkinRibVec = s.m_SkinRibVec;
    m_SkinDegreeVec = s.m_SkinDegreeVec;
    m_SkinParmVec = s.m_SkinParmVec;
    m_SkinClosedFlag = s.m_SkinClosedFlag;

    m_CloneIndex = s.m_CloneIndex;
    m_CloneMat = s.m_CloneMat;

    // Pointer, be careful.
    m_FoilSurf = s.m_FoilSurf;

    m_PlanarUWAspect = s.m_PlanarUWAspect;

    // private:
    m_USkip = s.m_USkip;

    m_RootCluster = s.m_RootCluster;
    m_TipCluster = s.m_TipCluster;
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

double VspSurf::GetRootCluster( const int &index ) const
{
    if ( index >= 0 && index < m_RootCluster.size() )
    {
        return m_RootCluster[ index ];
    }
    return 1.0;
}

double VspSurf::GetTipCluster( const int &index ) const
{
    if ( index >= 0 && index < m_TipCluster.size() )
    {
        return m_TipCluster[ index ];
    }
    return 1.0;
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

    m_FeaOrientation = mat.xformnorm( m_FeaOrientation );
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

double VspSurf::FindRST( const vec3d & pt, const double &r0, const double &s0, const double &t0, double &r, double &s, double &t ) const
{
    surface_point_type p0;
    p0 << pt.x(), pt.y(), pt.z();
    surface_index_type ret;
    double dist = eli::geom::intersect::find_rst( r, s, t, m_Surface, p0, r0, s0, t0, ret );
    return dist;
}

double VspSurf::FindRST( const vec3d & pt, double &r, double &s, double &t ) const
{
    surface_point_type p0;
    p0 << pt.x(), pt.y(), pt.z();
    surface_index_type ret;
    double dist = eli::geom::intersect::find_rst( r, s, t, m_Surface, p0, ret );
    return dist;
}

void VspSurf::FindRST( const vector < vec3d > & pt, vector < double > &r, vector < double > &s, vector < double > &t, vector < double > &d  ) const
{
    int n = pt.size();
    vector <surface_point_type> pts( n );
    vector <surface_index_type> ret(n);

    for ( int i = 0; i < n; i++ )
    {
        pts[i] << pt[i].x(), pt[i].y(), pt[i].z();
    }
    eli::geom::intersect::find_rst( r, s, t, d, m_Surface, pts, ret );
}

void VspSurf::ConvertRSTtoLMN( const double &r, const double &s, const double &t, double &l_out, double &m_out, double &n_out ) const
{
    l_out = m_LCurve.CompPnt( r );
    Vsp1DCurve mcurve;
    BuildMCurve( r, mcurve );
    m_out = mcurve.CompPnt( s );
    n_out = t;
}

void VspSurf::ConvertRtoL( const double &r, double &l_out ) const
{
    l_out = m_LCurve.CompPnt( r );
}

void VspSurf::ConvertLMNtoRST( const double &l, const double &m, const double &n, double &r_out, double &s_out, double &t_out ) const
{
    r_out = m_LCurve.Invert( l );
    Vsp1DCurve mcurve;
    BuildMCurve( r_out, mcurve );
    s_out = mcurve.Invert( m );
    t_out = n;
}

void VspSurf::ConvertLtoR( const double &l, double &r_out) const
{
    r_out = m_LCurve.Invert( l );
}

double VspSurf::ProjectPt( const vec3d &inpt, const int &idir, double &u_out, double &w_out ) const
{
    surface_point_type p, p0;
    p0 << inpt.x(), inpt.y(), inpt.z();

    double idist = eli::geom::intersect::intersect( u_out, w_out, p, m_Surface, p0, idir );

    return idist;
}

double VspSurf::ProjectPt( const vec3d &inpt, const int &idir, const double &u0, const double &w0, double &u_out, double &w_out ) const
{
    surface_point_type p, p0;
    p0 << inpt.x(), inpt.y(), inpt.z();

    double idist = eli::geom::intersect::intersect( u_out, w_out, p, m_Surface, p0, idir, u0, w0 );

    return idist;
}

double VspSurf::ProjectPt01( const vec3d &inpt, const int &idir, double &u_out, double &w_out) const
{
    double idist = ProjectPt( inpt, idir, u_out, w_out );

    u_out = u_out / GetUMax();
    w_out = w_out / GetWMax();

    return idist;
}

double VspSurf::ProjectPt01( const vec3d &inpt, const int &idir, const double &u0, const double &w0, double &u_out, double &w_out ) const
{
    double idist = ProjectPt( inpt, idir, u0 * GetUMax(), w0 * GetWMax(), u_out, w_out );

    u_out = u_out / GetUMax();
    w_out = w_out / GetWMax();

    return idist;
}

bool VspSurf::IsInside( const vec3d &pt ) const
{
    surface_point_type p;
    p << pt.x(), pt.y(), pt.z();

    return eli::geom::intersect::inside( m_Surface, p );
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
Matrix4d VspSurf::CompRotCoordSys( double u, double w ) const
{
    double tol = 1e-10;

    Matrix4d retMat; // Return Matrix

    double uprm = m_UMapping.Invert( u * m_UMapMax ) / GetUMax();

    uprm = clamp( uprm, 0.0 + tol, 1.0 - tol );
    w = clamp( w, 0.0 + tol, 1.0 - tol );

    // Get du and norm, cross them to get the last orthonormal vector
    vec3d du = CompTanU01( uprm, w );
    du.normalize();
    vec3d norm = CompNorm01( uprm, w ); // use CompNorm01 since normals now face outward
    norm.normalize();

    if ( m_MagicVParm ) // Surfs with magic parameter treatment (wings) have added chances to degenerate
    {
        double tmagic01 = TMAGIC / GetWMax();

        if ( du.mag() < tol ) // Zero direction vector
        {
            if ( w <= tmagic01 ) // Near TE lower
            {
                du = CompTanU01( uprm, tmagic01 + tol );
                du.normalize();
            }

            if ( w >= ( 0.5 - tmagic01 ) && w <= ( 0.5 + tmagic01 ) ) // Near leading edge
            {
                du = CompTanU01( uprm, 0.5 + tmagic01 + tol );
                du.normalize();
            }

            if ( w >= ( 1.0 - tmagic01 ) ) // Near TE upper
            {
                du = CompTanU01( uprm, 1.0 - ( tmagic01 + tol ) );
                du.normalize();
            }
        }

        if ( norm.mag() < tol ) // Zero normal vector
        {
            if ( w <= tmagic01 ) // Near TE lower
            {
                norm = CompNorm01( uprm, tmagic01 + tol );
                norm.normalize();
            }

            if ( w >= ( 0.5 - tmagic01 ) && w <= ( 0.5 + tmagic01 ) ) // Near leading edge
            {
                norm = CompNorm01( uprm, 0.5 + tmagic01 + tol );
                norm.normalize();
            }

            if ( w >= ( 1.0 - tmagic01 ) ) // Near TE upper
            {
                norm = CompNorm01( uprm, 1.0 - ( tmagic01 + tol ) );
                norm.normalize();
            }
        }
    }

    vec3d dw = cross( norm, du );
    dw.normalize();

    // Place axes in as cols of Rot mat
    retMat.setBasis( du, dw, norm );
    return retMat;
}

//===== Compute a Relative Translation Transformation Matrix from Component's
//      Coordinate System to a Surface Coordinate System ====//
Matrix4d VspSurf::CompTransCoordSys( const double &u, const double &w ) const
{
    Matrix4d retMat; // Return Matrix

    double uprm = m_UMapping.Invert( u * m_UMapMax ) / GetUMax();

    // Get x,y,z location of u,w coordinate and place in translation matrix
    vec3d cartCoords = CompPnt01( uprm, w );
    retMat.translatef( cartCoords.x(), cartCoords.y(), cartCoords.z() );
    return retMat;
}

Matrix4d VspSurf::CompRotCoordSysRST( double r, double s, double t ) const
{
    double tol = 1e-10;

    Matrix4d retMat; // Return Matrix

    double rprm = m_UMapping.Invert( r * m_UMapMax ) / GetUMax();

    rprm = clamp( rprm, 0.0 + tol, 1.0 - tol );
    s = clamp( s, 0.0 + tol, 1.0 - tol );
    t = clamp( t, 0.0 + tol, 1.0 - tol );

    vec3d dr = CompTanR( rprm, s, t );
    vec3d ds = CompTanS( rprm, s, t );
    vec3d dt = CompTanT( rprm, s, t );

    dr.normalize();
    ds.normalize();
    dt.normalize();

    vec3d norm = cross( dr, ds );
    if ( norm.mag() < tol )
    {
        norm = dt;
    }
    norm.normalize();

    vec3d d2 = cross( norm, dr );
    if ( d2.mag() < tol )
    {
        d2 = ds;
    }
    d2.normalize();

    // Place axes in as cols of Rot mat
    retMat.setBasis( dr, d2, norm );
    return retMat;
}

Matrix4d VspSurf::CompTransCoordSysRST( const double &r, const double &s, const double &t ) const
{
    Matrix4d retMat; // Return Matrix

    double rprm = m_UMapping.Invert( r * m_UMapMax ) / GetUMax();

    // Get x,y,z location of r, s, t coordinate and place in translation matrix
    vec3d cartCoords = CompPntRST( rprm, s, t );
    retMat.translatef( cartCoords.x(), cartCoords.y(), cartCoords.z() );
    return retMat;
}

Matrix4d VspSurf::CompRotCoordSysLMN( const double &l, const double &m, const double &n ) const
{
    double r, s, t;
    ConvertLMNtoRST( l, m, n, r, s, t );
    return CompRotCoordSysRST( r, s, t );
}

Matrix4d VspSurf::CompTransCoordSysLMN( const double &l, const double &m, const double &n ) const
{
    double r, s, t;
    ConvertLMNtoRST( l, m, n, r, s, t );
    return CompTransCoordSysRST( r, s, t );
}

void VspSurf::CreateBodyRevolution( const VspCurve &input_crv, bool match_uparm, int iaxis )
{
    eli::geom::surface::create_body_of_revolution( m_Surface, input_crv.GetCurve(), iaxis, true, match_uparm );

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
        return;
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
        return;
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

vec3d VspSurf::CompPntRST( double r, double s, double t ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.fRST( r, s, t ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

vec3d VspSurf::CompTanR( double r, double s, double t ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_R( r, s, t ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

vec3d VspSurf::CompTanS( double r, double s, double t ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_S( r, s, t ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

vec3d VspSurf::CompTanT( double r, double s, double t ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_T( r, s, t ) );

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

vec3d VspSurf::CompAveNorm( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.average_normal( u, v ) );

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

vec3d VspSurf::CompAveNorm01( double u01, double v01 ) const
{
    return CompAveNorm( u01 * GetUMax(), v01 * GetWMax() );
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

void VspSurf::SetUSkipFirst( int nskip, bool f )
{
    if( !m_USkip.empty() )
    {
        for ( int iskip = 0; iskip < m_USkip.size() && iskip < nskip; iskip++ )
        {
            m_USkip[ iskip ] = f;
        }
    }
}

void VspSurf::SetUSkipLast( int nskip, bool f )
{
    if( !m_USkip.empty() )
    {
        int n = m_USkip.size();
        for ( int iskip = 0; iskip < n && iskip < nskip; iskip++ )
        {
            m_USkip[ n - iskip - 1 ] = f;
        }
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

void VspSurf::MakeUTess( vector < double > &u, const vector < int > &num_u, const std::vector < int > &umerge, const int &n_cap, const int &n_default ) const
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
                double du;
                du = m_Surface.get_du( iusect );

                uend += du;
                iusect++;
            }

            double du = uend - ustart;

            if ( !uskip[i] )
            {
                for ( int isecttess = 0; isecttess < num_u[i] - 1; ++isecttess )
                {
                    u[iu] = ustart + du * Cluster(static_cast<double>( isecttess ) / (num_u[i] - 1), GetRootCluster( i ), GetTipCluster( i ) );
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
        const int nusect = m_Surface.number_u_patches();

        vector < double > umap;
        m_UMapping.GetTMap( umap );

        vector < int > num_uu( nusect );
        vector < double > rootc( nusect );
        vector < double > tipc( nusect );
        vector < bool > skip( nusect );

        // Count nu and build up tess parameters, translating from specification to modified
        // parameter space in the case of a GeomEngine.
        int nu = 1;
        for ( int isect = 0; isect < nusect; ++isect )
        {
            double ustart = m_UMapping.GetSegFirstPoint( isect );
            double uend  = m_UMapping.GetSegLastPoint( isect );

            // Determine original section index from mapping.
            // I believe the operations performed on the mapping make it impossible for floor to give a wrong result
            // in this situation.
            int iorig = floor( ustart );

            if ( iorig == -1 )        // Cap added by GeomEngine::UpdateEngine();
            {
                num_uu[isect] = n_cap;
                rootc[isect] = 1.0;
                tipc[isect] = 1.0;
                nu += num_uu[isect] - 1;
                skip[isect] = false;
            }
            else if ( iorig == -2 )   // Extension added by GeomEngine::UpdateEngine()
            {
                num_uu[isect] = n_default;
                rootc[isect] = 1.0;
                tipc[isect] = 1.0;
                nu += num_uu[isect] - 1;
                skip[isect] = false;
            }
            else                      // Maps to an original segment.
            {
                skip[isect] = m_USkip[iorig];
                if ( !m_USkip[iorig] )
                {
                    int nuu = num_u[iorig];
                    double rc = GetRootCluster( iorig );
                    double tc = GetTipCluster( iorig );

                    double fstart = ustart - iorig;
                    double fend = uend - iorig;

                    double tol = 1e-6;
                    if ( fstart > tol || fend < ( 1.0 - tol ) )
                    {
                        // Find where points will lie for fully tessellated section.
                        vector < double > fvec( nuu );
                        for ( int ifv = 0; ifv < nuu; ifv++ )
                        {
                            fvec[ ifv ] = Cluster( static_cast<double>( ifv ) / ( nuu - 1 ), rc, tc );
                        }

                        int istart = vector_find_nearest( fvec, fstart );
                        int iend = vector_find_nearest( fvec, fend );

                        if ( istart > 0 )
                        {
                            if ( istart < nuu - 1 )
                            {
                                rc = 2.0 / ( ( fvec[ istart + 1 ] - fvec[ istart - 1 ] ) * ( nuu - 1 ) );
                            }
                            else
                            {
                                rc = 1.0 / ( ( fvec[ istart ] - fvec[ istart - 1 ] ) * ( nuu - 1 ) );
                            }
                        }

                        if ( iend < ( nuu - 1 ))
                        {
                            if ( iend > 0 )
                            {
                                tc = 2.0 / ( ( fvec[ iend + 1 ] - fvec[ iend - 1 ] ) * ( nuu - 1 ) );
                            }
                            else
                            {
                                tc = 1.0 / ( ( fvec[ iend + 1 ] - fvec[ iend ] ) * ( nuu - 1 ) );
                            }
                        }

                        if ( istart > 0 || iend < ( nuu - 1 ) )
                        {
                            nuu = iend - istart + 1;
                        }
                    }

                    nu += nuu - 1;

                    num_uu[isect] = nuu;
                    rootc[isect] = rc;
                    tipc[isect] = tc;
                }
            }
        }

        // Construct u tessellation.
        u.resize( nu );
        double uumin( m_Surface.get_u0() );
        int iu = 0;
        for ( int isect = 0; isect < nusect; ++isect )
        {
            double du = m_Surface.get_du( isect );

            if ( !skip[ isect ] )
            {
                for ( int isecttess = 0; isecttess < num_uu[ isect ] - 1; ++isecttess )
                {
                    u[iu] = uumin + du * Cluster( static_cast<double>( isecttess ) / ( num_uu[ isect ] - 1 ), rootc[ isect ], tipc[ isect ] );
                    iu++;
                }
            }
            if ( !( isect == nusect - 1 && skip[ isect ] ) )
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
void VspSurf::Tesselate( int num_u, int num_v, vector< vector< vec3d > > & pnts, vector< vector< vec3d > > & norms, vector< vector< vec3d > > & uw_pnts, const int &n_cap, const int &n_default, bool degen ) const
{
    vector<int> num_u_vec( GetNumSectU(), num_u );
    Tesselate( num_u_vec, num_v, pnts, norms, uw_pnts, n_cap, n_default, degen );
}

// Geom::UpdateSplitTesselate
void VspSurf::SplitTesselate( int num_u, int num_v, vector< vector< vector< vec3d > > > & pnts, vector< vector< vector< vec3d > > > & norms, const int &n_cap, const int &n_default ) const
{
    vector<int> num_u_vec( GetNumSectU(), num_u );
    SplitTesselate( num_u_vec, num_v, pnts, norms, n_cap, n_default );
}

// VspSurf::Tesselate
// FuselageGeom::UpdateTesselate
// PropGeom::UpdateTesselate
// StackGeom::UpdateTesselate
// WingGeom::UpdateTesselate
void VspSurf::Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts, const int &n_cap, const int &n_default, bool degen, const std::vector<int> & umerge ) const
{
    if( m_Surface.number_u_patches() == 0 || m_Surface.number_v_patches() == 0 )
    {
        return;
    }

    std::vector<double> u, v;

    MakeVTess( num_v, v, n_cap, degen );
    MakeUTess( u, num_u, umerge, n_cap, n_default );

    Tesselate( u, v, pnts, norms, uw_pnts );
}

// VspSurf::SplitTesselate
// FuselageGeom::UpdateSplitTesselate
// PropGeom::UpdateSplitTesselate
// StackGeom::UpdateSplitTesselate
// WingGeom::UpdateSplitTesselate
void VspSurf::SplitTesselate( const vector<int> &num_u, int num_v, std::vector< vector< vector< vec3d > > > & pnts,  std::vector< vector< vector< vec3d > > > & norms, const int &n_cap, const int &n_default, const std::vector<int> & umerge ) const
{
    if( m_Surface.number_u_patches() == 0 || m_Surface.number_v_patches() == 0 )
    {
        return;
    }

    std::vector<double> u, v;

    MakeVTess( num_v, v, n_cap, false );
    MakeUTess( u, num_u, umerge, n_cap, n_default );

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

void VspSurf::TessULine( double u, std::vector< vec3d > & pnts, double tol ) const
{
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

void VspSurf::TessUFeatureLine( int iu, std::vector< vec3d > & pnts, double tol ) const
{
    double u = m_UFeature[ iu ];

    TessULine( u, pnts, tol );
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

    double len = dist( pmin, pmax );
    double d = dist_pnt_2_line( pmin, pmax, pmid ) / len;

    if ( ( len > DBL_EPSILON && d > tol && Nlimit > 0 ) || Nadapt < 2 )
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

void VspSurf::BuildSimpleFeatureLines()
{
    m_Surface.get_pmap_uv( m_UFeature, m_WFeature );
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

        if ( GetSurfType() == vsp::WING_SURF || force_xsec_flag )
        {
            // Forget result of find_interior_feature_edges above.
            m_UFeature.clear();

            // Force all patch boundaries in u direction.
            // umin and umax are close to integers, but might not be exactly.  Furthermore, this code does not
            // guarantee that umax is added.
            for ( double u = umin; u <= umax; u++ )
            {
                m_UFeature.push_back( u );
            }
        }

        // Make sure start/end are always included.
        m_UFeature.push_back( umin );
        m_UFeature.push_back( umax );

        // Add start/mid/end curves.
        double vmin = m_Surface.get_v0();
        double vmax = m_Surface.get_vmax();
        double vrng = vmax - vmin;
        double vmid = vmin + vrng * 0.5;

        m_WFeature.push_back( vmin );
        m_WFeature.push_back( vmid );
        m_WFeature.push_back( vmax );

        // If fuse-type, add .25 and .75 curves.
        if ( GetSurfType() != vsp::WING_SURF )
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

bool VspSurf::CapUMin(int CapType, double len, double str, double offset, const vec3d &pt, bool swflag)
{
    return CapUHandler( multicap_creator_type::CAP_UMIN, CapType, len, str, offset, pt, swflag );
}

bool VspSurf::CapUMax(int CapType, double len, double str, double offset, const vec3d &pt, bool swflag)
{
    return CapUHandler( multicap_creator_type::CAP_UMAX, CapType, len, str, offset, pt, swflag );
}

bool VspSurf::CapUHandler(int whichCap, int CapType, double len, double str, double offset, const vec3d &pt, bool swflag)
{
    if (CapType == vsp::NO_END_CAP)
    {
      ResetUSkip();
      return false;
    }
    multicap_creator_type cc;
    bool rtn_flag;

    int captype = multicap_creator_type::FLAT;

    bool extle = false;
    bool extte = false;

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
      case vsp::POINT_END_CAP:
        captype = multicap_creator_type::POINT;
        break;
      case vsp::ROUND_EXT_END_CAP_NONE:
        captype = multicap_creator_type::ROUND_EXT;
        break;
      case vsp::ROUND_EXT_END_CAP_LE:
        captype = multicap_creator_type::ROUND_EXT;
        extle = true;
        break;
      case vsp::ROUND_EXT_END_CAP_TE:
        captype = multicap_creator_type::ROUND_EXT;
        extte = true;
        break;
      case vsp::ROUND_EXT_END_CAP_BOTH:
        captype = multicap_creator_type::ROUND_EXT;
        extle = true;
        extte = true;
        break;
    }

    surface_point_type p;
    p << pt.x(), pt.y(), pt.z();

    if ( CapType >= vsp::ROUND_EXT_END_CAP_NONE )
    {
        piecewise_curve_type b, e, d, dsq;

        cc.set_ext_conditions( extle, extte );

        rtn_flag = cc.set_conditions( m_Surface, multicap_creator_type::FLAT, 1.0, (typename multicap_creator_type::edge_cap_identifier) whichCap, len, 0, str, p, swflag );
        rtn_flag = cc.create_curve( b );

        rtn_flag = cc.set_conditions( m_Surface, multicap_creator_type::ROUND, 1.0, (typename multicap_creator_type::edge_cap_identifier) whichCap, len, 0, str, p, swflag );
        rtn_flag = cc.create_curve( e );

        b.scale( -1.0 );
        d.sum( b, e );
        dsq.square( d );

        typedef piecewise_curve_type::onedpiecewisecurve onedpwc;
        typedef onedpwc::point_type onedpt;
        onedpwc sumsq;

        sumsq = dsq.sumcompcurve();

        // Negate to allow minimization instead of maximization.
        sumsq.scale( -1.0 );

        onedpwc sweepcurve;
        cc.create_sweepfactor_sq_curve( sweepcurve );
        cc.dirty_prep(); // Needed because we split the surface and need to re-load it later.


        onedpwc objfun;
        objfun.product( sumsq, sweepcurve );

        double utmax;
        double hmax = sqrt( -1.0 * eli::geom::intersect::minimum_dimension( utmax, objfun, 0 ) );

        double xloc, wlow, wup;

        wlow = utmax;
        wup = GetWMax() - ( utmax - 0.0 );

        SplitW( wup );
        SplitW( wlow );

        typedef onedpwc::curve_type onedcurve_type;
        onedcurve_type c;

        onedpt onedp;
        objfun.get( c, 1 );
        onedp = c.get_control_point( 0 );
        double h_start = sqrt( -onedp.x() );

        objfun.get( c, objfun.number_segments() - 2 );
        onedp = c.get_control_point( c.degree() );
        double h_end = sqrt( -onedp.x() );

        // Need to pass wlow (which we do) and then search for i_hmax internally after surface has been split,
        // flipped, and matched top to bottom.  This matching is likely throwing off the patch count.
        cc.set_h_vals( wlow, hmax, h_start, h_end );
    }

    rtn_flag = cc.set_conditions(m_Surface, captype, 1.0, (typename multicap_creator_type::edge_cap_identifier) whichCap, len, offset, str, p, swflag );

    if ( whichCap == multicap_creator_type::CAP_UMAX )
    {
        m_UMapping.CapMax();
    }
    else
    {
        m_UMapping.CapMin();
    }

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

    if ( whichCap == multicap_creator_type::CAP_UMIN )
    {
        m_Surface.set_u0( 0.0 );
    }

    ResetUSkip();
    return true;
}

void VspSurf::RollU( const double &u )
{
    m_Surface.split_u( u );

    vector < double > pmap;
    m_Surface.get_pmap_u( pmap );
    int iu = vector_find_val( pmap, u );

    m_Surface.roll_u( iu );

    m_UMapping.Roll( u );
}

void VspSurf::RollW( const double &w )
{
    m_Surface.split_v( w );

    vector < double > pmap;
    m_Surface.get_pmap_v( pmap );
    int iw = vector_find_val( pmap, w );

    m_Surface.roll_v( iw );
}

void VspSurf::SplitU( const double &u )
{
    m_Surface.split_u( u );
}

void VspSurf::SplitW( const double &w )
{
    m_Surface.split_v( w );
}

void VspSurf::JoinU( const VspSurf & sa, const VspSurf & sb )
{
    m_Surface.join_u( sa.m_Surface, sb.m_Surface );

    m_UMapping.Join( sa.m_UMapping, sb.m_UMapping );

    m_UMapMax = max( sa.m_UMapMax, sb.m_UMapMax );
}

void VspSurf::JoinW( const VspSurf & sa, const VspSurf & sb )
{
    m_Surface.join_v( sa.m_Surface, sb.m_Surface );
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

void VspSurf::TrimU( double u, bool before )
{
    piecewise_surface_type s1, s2;

    m_Surface.split_u( s1, s2, u );

    if ( before )
    {
        m_Surface = s1;
    }
    else
    {
        s2.set_u0( 0.0 );
        m_Surface = s2;
    }

    m_UMapping.Trim( u, before );
}

void VspSurf::TrimV( double v, bool before )
{
    piecewise_surface_type s1, s2;

    m_Surface.split_v( s1, s2, v );

    if ( before )
    {
        m_Surface = s1;
    }
    else
    {
        s2.set_v0( 0.0 );
        m_Surface = s2;
    }
}

void VspSurf::TrimClosedV( double vstart, double vend )
{
    piecewise_surface_type s;

    if ( vstart != vend )
    {
        if ( m_Surface.trim_v( vstart, vend, s ) ) // Success.
        {
            m_Surface = s;
        }
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

void VspSurf::FetchXFerSurf( const std::string &geom_id, const std::string &name, int surf_ind, int comp_ind, int part_surf_num, vector< XferSurf > &xfersurfs, const vector < double > &usuppress, const vector < double > &wsuppress ) const
{
    vector < piecewise_surface_type > surfvec;
    surfvec.push_back( m_Surface );
    SplitSurfs( surfvec, usuppress, wsuppress );

    int num_sections = surfvec.size();

    int ivalid = 0;
    for ( int isect = 0; isect < num_sections; isect++ )
    {
        const piecewise_surface_type & surf = surfvec[isect];

        if ( !CheckValidPatch( surf ) )
        {
            continue;
        }

        // Made it through all checks, keep surface.
        XferSurf xsurf;
        xsurf.m_FlipNormal = m_FlipNormal;
        xsurf.m_Surface = surf;
        xsurf.m_GeomID = geom_id;
        xsurf.m_Name = name;
        xsurf.m_SplitNum = ivalid;
        xsurf.m_SurfIndx = surf_ind;
        xsurf.m_SurfType = GetSurfType();
        xsurf.m_SurfCfdType = m_SurfCfdType;
        xsurf.m_FeaOrientationType = m_FeaOrientationType;
        xsurf.m_FeaOrientation = m_FeaOrientation;
        xsurf.m_ThickSurf = m_ThickSurf;
        xsurf.m_PlateNum = m_PlateNum;
        xsurf.m_PlanarUWAspect = m_PlanarUWAspect;
        xsurf.m_CompIndx = comp_ind;
        xsurf.m_FeaPartSurfNum = part_surf_num;
        xsurf.m_FeaSymmIndex = m_FeaSymmIndex;
        xfersurfs.push_back( xsurf );
        ivalid++;
    }
}

void VspSurf::ToSTEP_BSpline_Quilt( STEPutil *step, vector < SdaiB_spline_surface_with_knots * > &surfs,
                                    const string &labelprefix, bool splitsurf, bool mergepts, bool tocubic, double tol,
                                    bool trimte, bool mergeLETE, const vector < double > &USplit,
                                    const vector < double > &WSplit, bool labelSplitNo, bool labelAirfoilPart,
                                    const string &delim ) const
{
    vector < piecewise_surface_type > surfvec = PrepCADSurfs( splitsurf, tocubic, tol, trimte, mergeLETE, USplit, WSplit );

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

        string label = labelprefix;

        if ( labelSplitNo )
        {
            if ( label.size() > 0 )
            {
                label.append( delim );
            }
            label.append( to_string( isurf ) );
        }

        if ( splitsurf && GetSurfType() == vsp::WING_SURF && labelAirfoilPart )
        {
            // Overall surface parameter limits.
            double vmax = m_Surface.get_vmax();
            double vmid = 0.5 * vmax;

            // Center parameter of this patch.
            double vave = 0.5 * ( s.get_v0() + s.get_vmax() );

            string af_label;
            if ( mergeLETE )
            {
                if ( vave < vmid - TMAGIC )
                {
                    af_label = "lower";
                }
                else if ( vave < vmid + TMAGIC )
                {
                    af_label = "le";
                }
                else if ( vave < vmax - TMAGIC )
                {
                    af_label = "upper";
                }
                else
                {
                    af_label = "te";
                }
            }
            else
            {
                if ( vave < TMAGIC )
                {
                    af_label = "telower";
                }
                else if ( vave < vmid - TMAGIC )
                {
                    af_label = "lower";
                }
                else if ( vave < vmid )
                {
                    af_label = "lelower";
                }
                else if ( vave < vmid + TMAGIC )
                {
                    af_label = "leupper";
                }
                else if ( vave < vmax - TMAGIC )
                {
                    af_label = "upper";
                }
                else
                {
                    af_label = "teupper";
                }
            }

            if ( label.size() > 0 )
            {
                label.append( delim );
            }
            label.append( af_label );
        }

        SdaiSurface* surf = step->MakeSurf( s, label, mergepts, merge_tol );
        SdaiB_spline_surface_with_knots* nurbs = dynamic_cast<SdaiB_spline_surface_with_knots*>( surf );

        surfs.push_back( nurbs );
    }
}

void VspSurf::ToIGES( IGESutil *iges, bool splitsurf, bool tocubic, double tol, bool trimTE, bool mergeLETE,
                      const vector < double > &USplit, const vector < double > &WSplit, const string &labelprefix,
                      bool labelSplitNo, bool labelAirfoilPart, const string &delim ) const
{
    vector < piecewise_surface_type > surfvec = PrepCADSurfs( splitsurf, tocubic, tol, trimTE, mergeLETE, USplit, WSplit );

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

        if ( splitsurf && GetSurfType() == vsp::WING_SURF && labelAirfoilPart )
        {
            // Overall surface parameter limits.
            double vmax = m_Surface.get_vmax();
            double vmid = 0.5 * vmax;

            // Center parameter of this patch.
            double vave = 0.5 * ( s.get_v0() + s.get_vmax() );

            string af_label;
            if ( mergeLETE )
            {
                if ( vave < vmid - TMAGIC )
                {
                    af_label = "lower";
                }
                else if ( vave < vmid + TMAGIC )
                {
                    af_label = "le";
                }
                else if ( vave < vmax - TMAGIC )
                {
                    af_label = "upper";
                }
                else
                {
                    af_label = "te";
                }
            }
            else
            {
                if ( vave < TMAGIC )
                {
                    af_label = "telower";
                }
                else if ( vave < vmid - TMAGIC )
                {
                    af_label = "lower";
                }
                else if ( vave < vmid )
                {
                    af_label = "lelower";
                }
                else if ( vave < vmid + TMAGIC )
                {
                    af_label = "leupper";
                }
                else if ( vave < vmax - TMAGIC )
                {
                    af_label = "upper";
                }
                else
                {
                    af_label = "teupper";
                }
            }

            if ( label.size() > 0 )
            {
                label.append( delim );
            }
            label.append( af_label );
        }

        iges->MakeSurf( s, label );
    }
}

vector < piecewise_surface_type >
VspSurf::PrepCADSurfs( bool splitsurf, bool tocubic, double tol, bool trimTE, bool mergeLETE,
                       const vector < double > &USplit, const vector < double > &WSplit ) const
{
    // Make copy for local changes.
    piecewise_surface_type s( m_Surface );

    vector < double > usuppress;
    vector < double > wsuppress;

    if ( GetSurfType() == vsp::WING_SURF )
    {
        if ( trimTE )
        {
            piecewise_surface_type s1, s2;
            s.split_v( s1, s2, s.get_v0() + TMAGIC );
            s2.split_v( s, s1, s.get_vmax() - TMAGIC );
        }
        else if ( mergeLETE )
        {
            // V parameter of lower TE corner.
            double v = s.get_v0() + TMAGIC;

            // Equivalent to RollV(v)
            s.split_v( v);

            vector < double > pmap;
            s.get_pmap_v( pmap );
            int iv = vector_find_val( pmap, v );

            s.roll_v( iv );
            // End of equivalent code.

            // roll_v() sets v0 = 0.0.  Re-set v0 back to original value.
            s.set_v0( v );

            // Suppress splitting along these feature lines.
            wsuppress.push_back( 0.0 ); // TE center line from bottom (should not exist, v0 = TMAGIC now)
            wsuppress.push_back( 2.0 ); // LE center line
            wsuppress.push_back( 4.0 ); // TE center line from top
        }
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
        SplitSurfs( surfvec, usuppress, wsuppress );
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

void VspSurf::MakePlaneSurf( const vec3d &ptA, const vec3d &ptB, const vec3d &ptC, const vec3d &ptD, double expand )
{
    // This function is used to construct a plane, using the four inputs as corner points
    // If the inputs are not truly planar, the function will work, but the surface will be
    // constructed via bi-linear interpolation

    vec3d cen = 0.25 * ( ptA + ptB + ptC + ptD );
    vec3d uA = ptA - cen;
    vec3d uB = ptB - cen;
    vec3d uC = ptC - cen;
    vec3d uD = ptD - cen;

    vec3d pA = cen + expand * uA;
    vec3d pB = cen + expand * uB;
    vec3d pC = cen + expand * uC;
    vec3d pD = cen + expand * uD;

    surface_patch_type patch;
    patch.resize( 1, 1 );

    threed_point_type pt0, pt1, pt2, pt3;

    pA.get_pnt( pt0 );
    pB.get_pnt( pt1 );
    pC.get_pnt( pt2 );
    pD.get_pnt( pt3 );

    patch.set_control_point( pt0, 0, 0 );
    patch.set_control_point( pt1, 1, 0 );
    patch.set_control_point( pt2, 0, 1 );
    patch.set_control_point( pt3, 1, 1 );

    double du = dist( pA, pB ) + dist( pC, pD );
    double dv = dist( pA, pC ) + dist( pB, pD );

    m_PlanarUWAspect = du / dv;

    m_Surface.init_uv( 1, 1 );
    m_Surface.set( patch, 0, 0 );
}

void VspSurf::MakePolyPlaneSurf( const vector < vec3d > &up_pts, const vector < vec3d > &low_pts )
{
    int npts = up_pts.size();
    assert( npts == low_pts.size() );

    m_Surface.init_uv( npts - 1, 1 );

    for ( int i = 0; i < npts - 1; i++ )
    {
        surface_patch_type patch;
        patch.resize( 1, 1 );

        threed_point_type pt0, pt1, pt2, pt3;

        up_pts[ i ].get_pnt( pt0 );
        up_pts[ i + 1 ].get_pnt( pt1 );
        low_pts[ i ].get_pnt( pt2 );
        low_pts[ i + 1 ].get_pnt( pt3 );

        patch.set_control_point( pt0, 0, 0 );
        patch.set_control_point( pt1, 1, 0 );
        patch.set_control_point( pt2, 0, 1 );
        patch.set_control_point( pt3, 1, 1 );

        // patch.octave_print( i + 1 );

        m_Surface.set( patch, i, 0 );
    }

    // m_Surface.octave_print( npts );
}

void VspSurf::DegenCamberSurf( const VspSurf & parent )
{
    piecewise_surface_type s1, s2;
    double umin, vmin, umax, vmax, vmid;

    parent.m_Surface.get_parameter_min( umin, vmin );
    parent.m_Surface.get_parameter_max( umax, vmax );
    vmid = ( vmin + vmax ) * 0.5;

    parent.m_Surface.split_v( s1, s2, vmid );

    s2.reverse_v();
    s2.set_v0( vmin );

    m_Surface.sum( s1, s2);
    m_Surface.scale( 0.5 );

    m_ThickSurf = false;
    SetSurfCfdType( vsp::CFD_TRANSPARENT );
    FlipNormal();
}

void VspSurf::DegenPlanarSurf( const VspSurf & parent, int vhflag )
{
    vector < VspCurve > crvs( 2 );
    vector < double > param( 2 );

    double umin, vmin, umax, vmax, dv;

    parent.m_Surface.get_parameter_min( umin, vmin );
    parent.m_Surface.get_parameter_max( umax, vmax );
    dv = vmax - vmin;

    double vstart, vend;
    if ( vhflag ) // vertical.
    {
        vstart = vmin + 0.25 * dv;
        vend = vmin + 0.75 * dv;
    }
    else // horizontal.
    {
        vstart = vmin;
        vend = vmin + 0.5 * dv;
    }

    parent.GetWConstCurve( crvs[0], vstart );
    parent.GetWConstCurve( crvs[1], vend );

    param[ 0 ] = vstart;
    param[ 1 ] = vend;

    SkinC0( crvs, param, false );
    SwapUWDirections();
    FlipNormal();
    m_ThickSurf = false;
    SetSurfCfdType( vsp::CFD_TRANSPARENT );
}

void VspSurf::InitUMapping()
{
    m_UMapMax = GetUMax();

    vector < double > umap;
    m_Surface.get_pmap_u( umap );

    m_UMapping.InterpolateLinear( umap, umap, false );
}

void VspSurf::InitUMapping( double val )
{
    int n = GetNumSectU() + 1;

    vector < double > uvals( n, val );
    vector < double > tvals( n );
    for ( int i = 0; i < n; i++ )
    {
        tvals[i] = 1.0 * i;
    }

    m_UMapping.InterpolateLinear( uvals, tvals, false );
}

void VspSurf::PrintUMapping() const
{
    m_UMapping.GetCurve().print();
}

void VspSurf::ParmReport()
{
    m_Surface.parameter_report();
}

double VspSurf::InvertUMapping( double u ) const
{
    if ( m_UMapMax >= 0 )
    {
        return m_UMapping.Invert( u );
    }
    return u;
}

double VspSurf::EvalUMapping( double u ) const
{
    if ( m_UMapMax >= 0 )
    {
        return m_UMapping.CompPnt( u );
    }
    return u;
}

void VspSurf::BuildLCurve()
{
    double umin, vmin, umax, vmax, dv;
    double vlow, vup;

    m_Surface.get_parameter_min( umin, vmin );
    m_Surface.get_parameter_max( umax, vmax );
    dv = vmax - vmin;

    vlow = vmin + 0.25 * dv;
    vup = vmin + 0.75 * dv;

    piecewise_curve_type c, clow, cup;

    m_Surface.get_vconst_curve( clow, vlow );
    m_Surface.get_vconst_curve( cup, vup );

    c.sum( clow, cup );
    c.scale( 0.5 );

    VspCurve spine;
    spine.SetCurve( c );

    vector< vec3d > x;
    vector< double > u;
    spine.TessSegAdapt( x, u, 1e-2, 10 );

    if ( x.size() > 0 )
    {
        vector < double > s;
        s.resize( x.size(), 0.0 );
        for ( int i = 1; i < x.size(); i++ )
        {
            vec3d dx = x[i] - x[i-1];
            s[i] = s[i-1] + dx.mag();
        }

        m_Lmax = s[ x.size() - 1 ];

        for ( int i = 0; i < x.size(); i++ )
        {
            s[i] /= m_Lmax;
            u[i] /= umax;
        }

        m_LCurve.InterpolateLinear( s, u, false );
    }
    else
    {
        m_Lmax = 0;
        m_LCurve = Vsp1DCurve();
    }
}

void VspSurf::BuildMCurve( const double &r, Vsp1DCurve &mcurve ) const
{
    if ( m_Surface.number_u_patches() > 0 && m_Surface.number_v_patches() > 0 )
    {
        double umin, vmin, umax, vmax;
        double vmid;

        m_Surface.get_parameter_min( umin, vmin );
        m_Surface.get_parameter_max( umax, vmax );
        vmid = 0.5 * ( vmin + vmax );

        double u = r * umax;

        piecewise_curve_type cut, clow, cup, cspine;
        m_Surface.get_uconst_curve( cut, u );

        cut.split( clow, cup, vmid );
        cup.reverse();

        cspine.sum( clow, cup );
        cspine.scale( 0.5 );

        VspCurve spine;
        spine.SetCurve( cspine );

        vector< vec3d > x;
        vector< double > s;
        spine.TessAdapt( x, s, 1e-2, 10 );

        vector < double > m;
        m.resize( x.size(), 0.0 );
        for ( int i = 1; i < x.size(); i++ )
        {
            vec3d dx = x[i] - x[i-1];
            m[i] = m[ i - 1 ] + dx.mag();
        }

        double mmax = m[ x.size() - 1 ];
        double vvmax = s[ x.size() - 1 ];

        for ( int i = 0; i < x.size(); i++ )
        {
            m[i] /= mmax;
            s[i] /= vvmax;
        }

        mcurve.InterpolateLinear( m, s, false );
    }
}
