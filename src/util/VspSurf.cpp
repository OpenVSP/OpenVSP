//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspSurf.cpp:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <set>

#include "VspSurf.h"
#include "StlHelper.h"

#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/surface/piecewise_body_of_revolution_creator.hpp"
#include "eli/geom/surface/piecewise_capped_surface_creator.hpp"
#include "eli/geom/intersect/minimum_distance_surface.hpp"

typedef piecewise_surface_type::index_type surface_index_type;
typedef piecewise_surface_type::point_type surface_point_type;
typedef piecewise_surface_type::rotation_matrix_type surface_rotation_matrix_type;
typedef piecewise_surface_type::bounding_box_type surface_bounding_box_type;
typedef piecewise_curve_type::point_type curve_point_type;

typedef eli::geom::curve::piecewise_linear_creator<double, 3, surface_tolerance_type> piecewise_linear_creator_type;
typedef eli::geom::surface::piecewise_general_skinning_surface_creator<double, 3, surface_tolerance_type> general_creator_type;
typedef eli::geom::surface::piecewise_capped_surface_creator<double, 3, surface_tolerance_type> capped_creator_type;

//===== Constructor  =====//
VspSurf::VspSurf()
{
    m_FlipNormal = false;
    m_SurfType = NORMAL_SURF;
}

//===== Destructor  =====//
VspSurf::~VspSurf()
{
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

    dist = eli::geom::intersect::minimum_distance( u, w, m_Surface, p, u0, w0 );

    return dist;
}

double VspSurf::FindNearest01( double &u, double &w, const vec3d &pt ) const
{
    double dist;
    int num_sectU, num_sectW;

    num_sectU = GetNumSectU();
    num_sectW = GetNumSectW();

    dist = FindNearest( u, w, pt );

    u = u / num_sectU;
    w = w / num_sectW;

    return dist;
}

double VspSurf::FindNearest01( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const
{
    double dist;
    int num_sectU, num_sectW;

    num_sectU = GetNumSectU();
    num_sectW = GetNumSectW();

    dist = FindNearest( u, w, pt, u0 * num_sectU, w0 * num_sectW );

    u = u / num_sectU;
    w = w / num_sectW;

    return dist;
}

void VspSurf::GetUConstCurve( VspCurve &c, const double &u ) const
{
    piecewise_curve_type pwc;
    m_Surface.get_uconst_curve(pwc, u);
    c.SetCurve(pwc);
}

void VspSurf::GetWConstCurve( VspCurve &c, const double &w ) const
{
    piecewise_curve_type pwc;
    m_Surface.get_vconst_curve(pwc, w);
    c.SetCurve(pwc);
}

//===== Compute a Relative Rotation Transformation Matrix from Component's
//      Coordinate System to a Surface Coordinate System ====//
Matrix4d VspSurf::CompRotCoordSys( const double &u, const double &w )
{
    Matrix4d retMat; // Return Matrix

    double tempMat[16];
    // Get du and norm, cross them to get the last orthonormal vector
    vec3d du = CompTanU01( u, w );
    du.normalize();
    vec3d norm = CompNorm01( u, w ); // use CompNorm01 since normals now face outward
    norm.normalize();
    vec3d dw = cross( norm, du );

    // Place axes in as cols of Rot mat
    retMat.getMat( tempMat );
    tempMat[0] = du.x();
    tempMat[4] = dw.x();
    tempMat[8]  = norm.x();
    tempMat[1] = du.y();
    tempMat[5] = dw.y();
    tempMat[9]  = norm.y();
    tempMat[2] = du.z();
    tempMat[6] = dw.z();
    tempMat[10] = norm.z();
    retMat.initMat( tempMat );
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

void VspSurf::CreateBodyRevolution( const VspCurve &input_crv )
{
    eli::geom::surface::create_body_of_revolution( m_Surface, input_crv.GetCurve(), 0, true );

    ResetFlipNormal();
    ResetUWSkip();
}

void VspSurf::SkinRibs( const vector<rib_data_type> &ribs, const vector < int > &degree, bool closed_flag )
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

    gc.create( m_Surface );
    ResetFlipNormal();
    ResetUWSkip();
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinRibs( const vector<rib_data_type> &ribs, bool closed_flag )
{
    surface_index_type nrib;
    nrib = ribs.size();
    vector< int > degree( nrib - 1, 0 );
    SkinRibs( ribs, degree, closed_flag );
}

void VspSurf::SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, const vector< int > &degree, bool closed_flag )
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
    SkinRibs( ribs, degree, closed_flag );
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
    return CompPnt( u * ( double )GetNumSectU(), v * ( double )GetNumSectW() );
}

//===== Compute Tangent In U Direction   =====//
vec3d VspSurf::CompTanU01( double u01, double v01 ) const
{
    return CompTanU( u01 * ( double )GetNumSectU(), v01 * ( double )GetNumSectW() );
}

//===== Compute Tangent In W Direction   =====//
vec3d VspSurf::CompTanW01( double u01, double v01 ) const
{
    return CompTanW( u01 * ( double )GetNumSectU(), v01 * ( double )GetNumSectW() );
}

//===== Compute Second Derivative U,U   =====//
vec3d VspSurf::CompTanUU01( double u01, double v01 ) const
{
    return CompTanUU( u01 * ( double )GetNumSectU(), v01 * ( double )GetNumSectW() );
}

//===== Compute Second Derivative W,W   =====//
vec3d VspSurf::CompTanWW01( double u01, double v01 ) const
{
    return CompTanWW( u01 * ( double )GetNumSectU(), v01 * ( double )GetNumSectW() );
}

//===== Compute Second Derivative U,W   =====//
vec3d VspSurf::CompTanUW01( double u01, double v01 ) const
{
    return CompTanUW( u01 * ( double )GetNumSectU(), v01 * ( double )GetNumSectW() );
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
    return CompNorm( u01 * ( double )GetNumSectU(), v01 * ( double )GetNumSectW() );
}

void VspSurf::ResetUWSkip()
{
    piecewise_surface_type::index_type ip, jp, nupatch, nwpatch;

    nupatch = m_Surface.number_u_patches();
    nwpatch = m_Surface.number_v_patches();

    m_USkip.resize( nupatch );
    m_WSkip.resize( nwpatch );

    for ( ip = 0; ip < nupatch; ip++ )
        m_USkip[ip] = false;

    for ( jp = 0; jp < nwpatch; jp++ )
        m_WSkip[jp] = false;
}

void VspSurf::FlagDuplicate( VspSurf *othersurf )
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

            surface_patch_type *patch = m_Surface.get_patch( ip, jp );
            surface_patch_type *otherpatch = othersurf->m_Surface.get_patch( ip, jp );

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
            othersurf->m_USkip[ip] = true;
        }
    }

    for( jp = 0; jp < nvpatch; ++jp )
    {
        if ( vmatchcnt[jp] == nupatch )
        {
            m_WSkip[jp] = true;
            othersurf->m_WSkip[jp] = true;
        }
    }
}

//==== Tesselate Surface ====//
void VspSurf::Tesselate( int num_u, int num_v, vector< vector< vec3d > > & pnts, vector< vector< vec3d > > & norms ) const
{
    std::vector< vector< vec3d > > uw_pnts;
    Tesselate( num_u, num_v, pnts, norms, uw_pnts );
}

void VspSurf::Tesselate( int num_u, int num_v, vector< vector< vec3d > > & pnts, vector< vector< vec3d > > & norms, vector< vector< vec3d > > & uw_pnts ) const
{
    vector<int> num_u_vec( GetNumSectU(), num_u );
    Tesselate( num_u_vec, num_v, pnts, norms, uw_pnts );
}

void VspSurf::Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms ) const
{
    std::vector< vector< vec3d > > uw_pnts;
    Tesselate( num_u, num_v, pnts, norms, uw_pnts );
}

void VspSurf::Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts ) const
{
    if( m_Surface.number_u_patches() == 0 || m_Surface.number_v_patches() == 0 )
    {
        return;
    }

    surface_index_type i, j, nu, nv( num_v );
    double umin, umax, vmin, vmax;
    std::vector<double> u, v( nv );
    surface_point_type ptmp, ntmp;

    assert( num_u.size() == GetNumSectU() );
    assert( m_USkip.size() == GetNumSectU() );

    // calculate nu
    nu = 1;
    for ( int ii = 0; ii < GetNumSectU(); ++ii )
    {
        if ( !m_USkip[ii] )
        {
            nu += num_u[ii] - 1;
        }
    }

    // resize pnts and norms
    pnts.resize( nu );
    norms.resize( nu );
    uw_pnts.resize( nu );
    for ( i = 0; i < nu; ++i )
    {
        pnts[i].resize( nv );
        norms[i].resize( nv );
        uw_pnts[i].resize( nv );
    }

    // calculate the u and v parameterizations
    m_Surface.get_parameter_min( umin, vmin );
    m_Surface.get_parameter_max( umax, vmax );
    for ( j = 0; j < nv; ++j )
    {
        v[j] = vmin + ( vmax - vmin ) * static_cast<double>( j ) / ( nv - 1 );
    }

    u.resize( nu );
    double uumin( umin );
    size_t iusect;
    size_t iu = 0;
    for ( iusect = 0; iusect < (size_t)GetNumSectU(); ++iusect )
    {
        double du, dv;
        surface_patch_type surf;
        m_Surface.get( surf, du, dv, iusect, 0 );

        if ( !m_USkip[ iusect] )
        {
            for ( int isecttess = 0; isecttess < num_u[iusect] - 1; ++isecttess )
            {
                u[iu] = uumin + du * static_cast<double>( isecttess ) / ( num_u[iusect] - 1 );
                iu++;
            }
        }
        uumin += du;
    }
    u.back() = uumin;

    // calculate the coordinate and normal at each point
    for ( surface_index_type i = 0; i < nu; ++i )
    {
        for ( surface_index_type j = 0; j < nv; ++j )
        {
            ptmp = m_Surface.f( u[i], v[j] );
            ntmp = m_Surface.normal( u[i], v[j] );
            pnts[i][j].set_xyz( ptmp.x(), ptmp.y(), ptmp.z() );
            if ( m_FlipNormal )
            {
                ntmp = -ntmp;
            }
            norms[i][j].set_xyz( ntmp.x(), ntmp.y(), ntmp.z() );
            uw_pnts[i][j].set_xyz( u[i], v[j], 0.0 );
        }
    }
}

void VspSurf::TessUFeatureLine( int iu, int num_v, std::vector< vec3d > & pnts )
{
    double u = m_UFeature[ iu ];
    double vmin = m_Surface.get_v0();
    double vmax = m_Surface.get_vmax();
    TessLine( u, u, vmin, vmax, num_v, pnts);
}

void VspSurf::TessWFeatureLine( int iw, int num_u, std::vector< vec3d > & pnts )
{
    double v = m_WFeature[ iw ];
    double umin = m_Surface.get_u0();
    double umax = m_Surface.get_umax();
    TessLine( umin, umax, v, v, num_u, pnts);
}

void VspSurf::TessLine( double umin, double umax, double wmin, double wmax, int numpts, std::vector< vec3d > & pnts )
{
    pnts.resize( numpts );

    double du = (umax - umin)/(numpts - 1);
    double dw = (wmax - wmin)/(numpts - 1);

    double u = umin;
    double w = wmin;

    for ( int i = 0; i < numpts; i++ )
    {
        if ( i == numpts - 1 )
        {
            u = umax;
            w = wmax;
        }

        pnts[i] = CompPnt( u, w );

        u += du;
        w += dw;
    }
}

void VspSurf::BuildFeatureLines()
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

        // Add start/mid/end curves.
        double vmin = m_Surface.get_v0();
        double vmax = m_Surface.get_vmax();
        double vrng = vmax - vmin;

        m_WFeature.push_back( vmin );
        m_WFeature.push_back( vmin + 0.5 * vrng );
        m_WFeature.push_back( vmax );

        // If fuse-type, add .25 and .75 curves.
        if ( GetSurfType() != VspSurf::WING_SURF )
        {
            m_WFeature.push_back( vmin + 0.25 * vrng );
            m_WFeature.push_back( vmin + 0.75 * vrng );
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

bool VspSurf::CapUMin(int CapType)
{
    if (CapType == NO_END_CAP)
      return false;

    capped_creator_type cc;
    bool rtn_flag;

    rtn_flag = cc.set_conditions(m_Surface, 1.0, capped_creator_type::CAP_UMIN);
    if (!rtn_flag)
    {
      assert(false);
      return false;
    }

    rtn_flag = cc.create(m_Surface);
    if (!rtn_flag)
    {
      assert(false);
      return false;
    }

    m_Surface.set_u0( 0.0 );
    ResetUWSkip();
    return true;
}

bool VspSurf::CapUMax(int CapType)
{
    if (CapType == NO_END_CAP)
      return false;

    capped_creator_type cc;
    bool rtn_flag;

    rtn_flag = cc.set_conditions(m_Surface, 1.0, capped_creator_type::CAP_UMAX);
    if (!rtn_flag)
    {
      assert(false);
      return false;
    }

    rtn_flag = cc.create(m_Surface);
    if (!rtn_flag)
    {
      assert(false);
      return false;
    }
    ResetUWSkip();
    return true;
}

bool VspSurf::CapWMin(int CapType)
{
    if (CapType == NO_END_CAP)
      return false;

    std::cout << "Am Capping WMin on this one!" << std::endl;
    return false;
}

bool VspSurf::CapWMax(int CapType)
{
    if (CapType == NO_END_CAP)
      return false;

    std::cout << "Am Capping WMax on this one!" << std::endl;
    return false;
}

void VspSurf::SplitSurfs( vector< piecewise_surface_type > &surfvec )
{
	piecewise_surface_type s(m_Surface);
	SplitSurfs( surfvec, s );
}

void VspSurf::SplitSurfs( vector< piecewise_surface_type > &surfvec, piecewise_surface_type initsurf )
{
    surfvec.clear();
    surfvec.push_back( initsurf );

    for ( int i = 0; i < m_UFeature.size(); ++i )
    {
        vector < piecewise_surface_type > splitsurfvec;
        for ( int j = 0; j < surfvec.size(); j++ )
        {
            piecewise_surface_type s, s1, s2;

            s = surfvec[j];

            if ( s.get_u0() < m_UFeature[i] && s.get_umax() > m_UFeature[i] )
            {
                s.split_u( s1, s2, m_UFeature[i] );
                splitsurfvec.push_back( s1 );
                splitsurfvec.push_back( s2 );
            }
            else
            {
                splitsurfvec.push_back( s );
            }
        }
        surfvec = splitsurfvec;
    }


    for ( int i = 0; i < m_WFeature.size(); ++i )
    {
        vector < piecewise_surface_type > splitsurfvec;
        for ( int j = 0; j < surfvec.size(); j++ )
        {
            piecewise_surface_type s, s1, s2;

            s = surfvec[j];

            if ( s.get_v0() < m_WFeature[i] && s.get_vmax() > m_WFeature[i] )
            {
                s.split_v( s1, s2, m_WFeature[i] );
                splitsurfvec.push_back( s1 );
                splitsurfvec.push_back( s2 );
            }
            else
            {
                splitsurfvec.push_back( s );
            }
        }
        surfvec = splitsurfvec;
    }
}

void VspSurf::WriteBezFile( FILE* file_id, const std::string &geom_id, int surf_ind, vector< XferSurf > &xfersurfs )
{
    // Make copy for local changes.

    piecewise_surface_type scubic( m_Surface );
    piecewise_surface_type::data_type ttol = 1e-6;
    scubic.to_cubic( ttol );

    vector < piecewise_surface_type > surfvec;
    SplitSurfs( surfvec, scubic );

    int num_sections = surfvec.size();

    for ( int isect = 0; isect < num_sections; isect++ )
    {
        XferSurf xsurf;
        xsurf.m_FlipNormal = m_FlipNormal;
        xsurf.m_Surface = surfvec[isect];;
        xsurf.m_GeomID = geom_id;
        xsurf.m_SurfIndx = surf_ind;
        xfersurfs.push_back( xsurf );
    }
}

int VspSurf::ClosestPatchEnd( const vector<double> & patch_endings, double end_val ) const
{
    // patch_endings should be a sorted vector
    int ind = ClosestElement( patch_endings, end_val );

    return ind * 3;
}

void VspSurf::ToSTEP_BSpline_Quilt( STEPutil * step, vector<SdaiB_spline_surface_with_knots *> &surfs )
{
    // Make copy for local changes.
    piecewise_surface_type s( m_Surface );

    if( m_FlipNormal )
    {
        s.reverse_v();
    }

    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;
    piecewise_surface_type::index_type minu, maxu, minv, maxv;

    nupatch = s.number_u_patches();
    nvpatch = s.number_v_patches();

    s.degree_u( minu, maxu );
    s.degree_v( minv, maxv );

    SdaiB_spline_surface_with_knots *surf = ( SdaiB_spline_surface_with_knots* ) step->registry->ObjCreate( "B_SPLINE_SURFACE_WITH_KNOTS" );
    step->instance_list->Append( ( SDAI_Application_instance * ) surf, completeSE );
    surf->u_degree_( maxu );
    surf->v_degree_( maxv );
    surf->name_( "''" );

    if( s.closed_u() )
    {
        surf->u_closed_( SDAI_LOGICAL( LTrue ) );
    }
    else
    {
        surf->u_closed_( SDAI_LOGICAL( LFalse ) );
    }

    if( s.closed_v() )
    {
        surf->v_closed_( SDAI_LOGICAL( LTrue ) );
    }
    else
    {
        surf->v_closed_( SDAI_LOGICAL( LFalse ) );
    }

    surf->self_intersect_( SDAI_LOGICAL( LFalse ) );
    surf->surface_form_( B_spline_surface_form__unspecified );

    int nupts = nupatch * maxu + 1;
    int nvpts = nvpatch * maxv + 1;

    vector< vector< surface_patch_type::point_type> > pts;
    pts.resize( nupts );
    for( int i = 0; i < nupts; ++i )
    {
        pts[i].resize( nvpts );
    }

    for( ip = 0; ip < nupatch; ++ip )
    {
        for( jp = 0; jp < nvpatch; ++jp )
        {
            surface_patch_type::index_type icp, jcp;

            surface_patch_type *patch = s.get_patch( ip, jp );

            patch->promote_u_to( maxu );
            patch->promote_v_to( maxv );

            for( icp = 0; icp <= maxu; ++icp )
                for( jcp = 0; jcp <= maxv; ++jcp )
                {
                    pts[ ip * maxu + icp ][ jp * maxv + jcp ] = patch->get_control_point( icp, jcp );
                }
        }
    }


    for( int i = 0; i < nupts; ++i )
    {
        std::ostringstream ss;
        ss << "(";
        for( int j = nvpts - 1; j >= 0; --j )
        {
            surface_patch_type::point_type p = pts[i][j];

            SdaiCartesian_point *pt = step->MakePoint( p.x(), p.y(), p.z() );
            ss << "#" << pt->GetFileId();

            if( j > 0 )
            {
                ss << ", ";
            }
        }
        ss << ")";
        surf->control_points_list_()->AddNode( new GenericAggrNode( ss.str().c_str() ) );
    }

    surf->u_multiplicities_()->AddNode( new IntNode( maxu + 1 ) );
    surf->u_knots_()->AddNode( new RealNode( 0.0 ) );
    for( ip = 1; ip < nupatch; ++ip )
    {
        surf->u_multiplicities_()->AddNode( new IntNode( maxu ) );
        surf->u_knots_()->AddNode( new RealNode( ip ) );
    }
    surf->u_multiplicities_()->AddNode( new IntNode( maxu + 1 ) );
    surf->u_knots_()->AddNode( new RealNode( nupatch ) );


    surf->v_multiplicities_()->AddNode( new IntNode( maxv + 1 ) );
    surf->v_knots_()->AddNode( new RealNode( 0.0 ) );
    for( jp = 1; jp < nvpatch; ++jp )
    {
        surf->v_multiplicities_()->AddNode( new IntNode( maxv ) );
        surf->v_knots_()->AddNode( new RealNode( jp ) );
    }
    surf->v_multiplicities_()->AddNode( new IntNode( maxv + 1 ) );
    surf->v_knots_()->AddNode( new RealNode( nvpatch ) );

    surf->knot_spec_( Knot_type__piecewise_bezier_knots );

    surfs.push_back( surf );
}

void VspSurf::ToSTEP_Bez_Patches( STEPutil * step, vector<SdaiBezier_surface *> &surfs )
{
    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;

    nupatch = m_Surface.number_u_patches();
    nvpatch = m_Surface.number_v_patches();

    for( ip = 0; ip < nupatch; ++ip )
    {
        for( jp = 0; jp < nvpatch; ++jp )
        {
            surface_patch_type::index_type icp, jcp, nu, nv;

            surface_patch_type *patch = m_Surface.get_patch_unordered( ip, jp );

            nu = patch->degree_u();
            nv = patch->degree_v();

            SdaiBezier_surface *surf = ( SdaiBezier_surface* ) step->registry->ObjCreate( "BEZIER_SURFACE" );
            step->instance_list->Append( ( SDAI_Application_instance * ) surf, completeSE );
            surf->u_degree_( nu );
            surf->v_degree_( nv );
            surf->name_( "''" );
            surf->u_closed_( SDAI_LOGICAL( LFalse ) ); // patch->closed_u();
            surf->v_closed_( SDAI_LOGICAL( LFalse ) ); // patch->closed_v();
            surf->self_intersect_( SDAI_LOGICAL( LFalse ) );
            surf->surface_form_( B_spline_surface_form__unspecified );

            for( jcp = 0; jcp <= nv; ++jcp )
            {
                std::ostringstream ss;
                ss << "(";
                for( icp = 0; icp <= nu; ++icp )
                {

                    surface_patch_type::point_type p = patch->get_control_point( icp, jcp );

                    SdaiCartesian_point *pt = step->MakePoint( p.x(), p.y(), p.z() );
                    ss << "#" << pt->GetFileId();

                    if( icp < nu )
                    {
                        ss << ", ";
                    }
                }
                ss << ")";
                surf->control_points_list_()->AddNode( new GenericAggrNode( ss.str().c_str() ) );
            }
            surfs.push_back( surf );
        }
    }
}
