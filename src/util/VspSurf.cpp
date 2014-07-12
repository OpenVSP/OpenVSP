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
#include "eli/geom/intersect/minimum_distance_surface.hpp"

typedef piecewise_surface_type::index_type surface_index_type;
typedef piecewise_surface_type::point_type surface_point_type;
typedef piecewise_surface_type::rotation_matrix_type surface_rotation_matrix_type;
typedef piecewise_surface_type::bounding_box_type surface_bounding_box_type;
typedef piecewise_curve_type::point_type curve_point_type;

typedef eli::geom::curve::piecewise_linear_creator<double, 3, surface_tolerance_type> piecewise_linear_creator_type;
typedef eli::geom::surface::general_skinning_surface_creator<double, 3, surface_tolerance_type> general_creator_type;

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
}

void VspSurf::SkinCX( const vector< VspCurve > &input_crv_vec, bool closed_flag, const vector< int > &cx )
{
    general_creator_type gc;
    surface_index_type i, ncrv;

    ncrv = input_crv_vec.size();

    std::vector<rib_data_type> ribs( ncrv );
    std::vector<typename general_creator_type::index_type> max_degree( ncrv - 1, 0 );

    for( i = 0; i < ncrv; i++ )
    {
        ribs[i].set_f( input_crv_vec[i].GetCurve() );
    }

    for( i = 1; i < ncrv-1; i++ )
    {
        ribs[i].set_continuity( ( rib_data_type::connection_continuity ) cx[i] );
    }

    // create surface
    gc.set_conditions(ribs, max_degree, false);

    gc.set_u0( 0 );
    for( i = 0; i < ncrv - 1; i++ )
    {
        gc.set_segment_du( 1, i);
    }

    gc.create( m_Surface );
}


//==== Interpolate A Set Of Points =====//
void VspSurf::SkinCX( const vector< VspCurve > &input_crv_vec, bool closed_flag, int cx  )
{
    surface_index_type ncrv;

    ncrv = input_crv_vec.size();

    vector < int > cxv( ncrv, cx );
    cxv[ 0 ] = 0;
    cxv[ ncrv - 1 ] = 0;

    SkinCX( input_crv_vec, closed_flag, cxv );
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinC0( const vector< VspCurve > &input_crv_vec, bool closed_flag )
{
    SkinCX( input_crv_vec, closed_flag, rib_data_type::C0 );
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinRibs( const vector<rib_data_type> &ribs )
{
    general_creator_type gc;
    surface_index_type nrib;

    nrib = ribs.size();

    std::vector<typename general_creator_type::index_type> max_degree( nrib - 1, 0 );

    // create surface
    bool setcond = gc.set_conditions(ribs, max_degree, false);
    assert( setcond );

    gc.create( m_Surface );
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinC1( const vector< VspCurve > &input_crv_vec, bool closed_flag )
{
    SkinCX( input_crv_vec, closed_flag, rib_data_type::C1 );
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinC2( const vector< VspCurve > &input_crv_vec, bool closed_flag )
{
    SkinCX( input_crv_vec, closed_flag, rib_data_type::C2 );
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

//==== Tesselate Surface ====//
void VspSurf::Tesselate( int num_u, int num_v, vector< vector< vec3d > > & pnts, vector< vector< vec3d > > & norms ) const
{
    std::vector< vector< vec3d > > uw_pnts;
    Tesselate( num_u, num_v, pnts, norms, uw_pnts );
}

void VspSurf::Tesselate( int num_u, int num_v, vector< vector< vec3d > > & pnts, vector< vector< vec3d > > & norms, vector< vector< vec3d > > & uw_pnts ) const
{
    if( m_Surface.number_u_patches() == 0 || m_Surface.number_v_patches() == 0 )
    {
        return;
    }

    surface_index_type i, j, nu( num_u ), nv( num_v );
    double umin, umax, vmin, vmax;
    std::vector<double> u( nu ), v( nv );
    surface_point_type ptmp, ntmp;

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
    for ( i = 0; i < nu; ++i )
    {
        u[i] = umin + ( umax - umin ) * static_cast<double>( i ) / ( nu - 1 );
    }
    for ( j = 0; j < nv; ++j )
    {
        v[j] = vmin + ( vmax - vmin ) * static_cast<double>( j ) / ( nv - 1 );
    }

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

void VspSurf::Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms ) const
{
    std::vector< vector< vec3d > > uw_pnts;
    Tesselate( num_u, num_v, pnts, norms, uw_pnts );
}

void VspSurf::Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts ) const
{
    surface_index_type i, j, nu, nv( num_v );
    double umin, umax, vmin, vmax;
    std::vector<double> u, v( nv );
    surface_point_type ptmp, ntmp;

    // calculate nu
    nu = 1;
    for ( int ii = 0; ii < GetNumSectU(); ++ii )
    {
        nu += num_u[ii] - 1;
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
    size_t ii;
    for ( i = 0, ii = 0; ii < (size_t)GetNumSectU(); ++ii )
    {
        double du, dv;

        surface_patch_type surf;
        m_Surface.get( surf, du, dv, ii, 0 );
        for ( int iii = 0; iii < num_u[ii] - 1; ++iii, ++i )
        {
            u[i] = uumin + du * static_cast<double>( iii ) / ( num_u[ii] - 1 );
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

struct joint_pred
{
    double ref_val;
    double tol;
    bool operator()( double a ) const
    {
        return ( std::abs( a - ref_val ) < tol );
    }
};

void VspSurf::WriteBezFile( FILE* file_id, const std::string &geom_id, int surf_ind )
{
    // Make copy for local changes.
    piecewise_surface_type s( m_Surface );

    piecewise_surface_type::data_type ttol = 1e-6;
    s.to_cubic( ttol );

    vector<double> u_pmap;
    vector<double> w_pmap;
    s.get_pmap_uv( u_pmap, w_pmap );

    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;

    nupatch = s.number_u_patches();
    nvpatch = s.number_v_patches();

    int nupts = nupatch * 3 + 1;
    int nvpts = nvpatch * 3 + 1;

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

            for( icp = 0; icp <= 3; ++icp )
                for( jcp = 0; jcp <= 3; ++jcp )
                {
                    pts[ ip * 3 + icp ][ jp * 3 + jcp ] = patch->get_control_point( icp, jcp );
                }
        }
    }

    vector<int> split_u, split_w;
    split_u.push_back( 0 );
    split_w.push_back( 0 );

    split_w.push_back( ClosetPatchEnd( w_pmap, 0.25 * w_pmap[w_pmap.size() - 1] ) );
    split_w.push_back( ClosetPatchEnd( w_pmap, 0.5 * w_pmap[w_pmap.size() - 1] ) );
    split_w.push_back( ClosetPatchEnd( w_pmap, 0.75 * w_pmap[w_pmap.size() - 1] ) );

    split_u.push_back( nupts - 1 );
    split_w.push_back( nvpts - 1 );

    int num_sections = ( split_u.size() - 1 ) * ( split_w.size() - 1 );

    fprintf( file_id, "%s Component\n", geom_id.c_str() );
    fprintf( file_id, "%d  Num_Sections\n", num_sections );
    fprintf( file_id, "%d Flip_Normal\n", m_FlipNormal );

    for ( int iu = 0 ; iu < ( int )split_u.size() - 1 ; iu++ )
    {
        for ( int iw = 0 ; iw < ( int )split_w.size() - 1 ; iw++ )
        {
            //==== Write Section ====//
            int num_u = split_u[iu + 1] - split_u[iu] + 1;
            int num_w = split_w[iw + 1] - split_w[iw] + 1;
            int num_u_map = split_u[iu + 1] / 3 - split_u[iu] / 3 + 1;
            int num_w_map = split_w[iw + 1] / 3 - split_w[iw] / 3 + 1;
            fprintf ( file_id, "%d %d  NumU, NumW\n", num_u, num_w );
            fprintf( file_id, "%d %d NumU_Map, NumW_Map\n", num_u_map, num_w_map );
            fprintf( file_id, "%d VspSurf_Index\n", surf_ind );
            //==== Write U,W Mapping ====//
            for ( int umi = split_u[iu] / 3; umi <= split_u[iu + 1] / 3; umi++ )
            {
                fprintf( file_id, "%20.20lf\n", u_pmap[umi] );
            }

            for ( int wmi = split_w[iw] / 3; wmi <= split_w[iw + 1] / 3; wmi++ )
            {
                fprintf( file_id, "%20.20lf\n", w_pmap[wmi] );
            }

            for ( int i = split_u[iu] ; i <= split_u[iu + 1] ; i++ )
                for ( int j = split_w[iw] ; j <= split_w[iw + 1] ; j++ )
                {
                    surface_patch_type::point_type p = pts[i][j];

                    fprintf( file_id, "%20.20lf %20.20lf %20.20lf\n", p.x(), p.y(), p.z() );
                }
        }
    }
}

int VspSurf::ClosetPatchEnd( const vector<double> & patch_endings, double end_val ) const
{
    // patch_endings should be a sorted vector
    int ind = ClosestElement( patch_endings, end_val );

    return ind * 3;

}
