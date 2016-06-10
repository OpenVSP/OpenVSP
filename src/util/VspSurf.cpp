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
#include "PntNodeMerge.h"
#include "APIDefines.h"

#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/surface/piecewise_body_of_revolution_creator.hpp"
#include "eli/geom/surface/piecewise_multicap_surface_creator.hpp"
#include "eli/geom/intersect/minimum_distance_surface.hpp"

typedef piecewise_surface_type::index_type surface_index_type;
typedef piecewise_surface_type::point_type surface_point_type;
typedef piecewise_surface_type::rotation_matrix_type surface_rotation_matrix_type;
typedef piecewise_surface_type::bounding_box_type surface_bounding_box_type;
typedef piecewise_curve_type::point_type curve_point_type;

typedef eli::geom::curve::piecewise_linear_creator<double, 3, surface_tolerance_type> piecewise_linear_creator_type;
typedef eli::geom::surface::piecewise_general_skinning_surface_creator<double, 3, surface_tolerance_type> general_creator_type;
typedef eli::geom::surface::piecewise_multicap_surface_creator<double, 3, surface_tolerance_type> multicap_creator_type;
typedef eli::geom::surface::piecewise_cubic_spline_skinning_surface_creator<double, 3, surface_tolerance_type> spline_creator_type;

//===== Constructor  =====//
VspSurf::VspSurf()
{
    m_FlipNormal = false;
    m_MagicVParm = false;
    m_SurfType = vsp::NORMAL_SURF;
    m_SurfCfdType = vsp::CFD_NORMAL;

    SetClustering( 1.0, 1.0 );
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

void VspSurf::SetRootTipClustering( const vector < double > &root, const vector < double > &tip )
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

    if ( !setcond )
    {
        printf( "Failure in SkinRibs set_conditions\n" );
    }

    m_Surface.clear();
    bool creat = gc.create( m_Surface );

    if ( !creat )
    {
        printf( "Failure in SkinRibs create\n" );
    }

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

void VspSurf::SkinCubicSpline( const vector<rib_data_type> &ribs, const vector < int > &degree, bool closed_flag )
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

    m_Surface.clear();
    bool creat = sc.create( m_Surface );

    if ( !creat )
    {
        printf( "Failure in SkinCubicSpline create\n" );
    }

    ResetFlipNormal();
    ResetUWSkip();
}

//==== Interpolate A Set Of Points =====//
void VspSurf::SkinCubicSpline( const vector<rib_data_type> &ribs, bool closed_flag )
{
    surface_index_type nrib;
    nrib = ribs.size();
    vector< int > degree( nrib - 1, 0 );
    SkinCubicSpline( ribs, degree, closed_flag );
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

void VspSurf::ResetUWSkip()
{
    piecewise_surface_type::index_type ip, jp, nupatch, nwpatch;

    nupatch = m_Surface.number_u_patches();
    nwpatch = m_Surface.number_v_patches();

    m_RootCluster.resize( nupatch );
    m_TipCluster.resize( nupatch );

    m_USkip.resize( nupatch );
    m_WSkip.resize( nwpatch );

    for ( ip = 0; ip < nupatch; ip++ )
    {
        m_USkip[ip] = false;
        m_RootCluster[ip] = 1.0;
        m_TipCluster[ip] = 1.0;
    }

    for ( jp = 0; jp < nwpatch; jp++ )
        m_WSkip[jp] = false;
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

void VspSurf::SetWSkipFirst( bool f )
{
    if( !m_WSkip.empty() )
    {
        m_WSkip.front() = f;
    }
}

void VspSurf::SetWSkipLast( bool f )
{
    if( !m_WSkip.empty() )
    {
        m_WSkip.back() = f;
    }
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

double VspSurf::Cluster( const double &t, const double &a, const double &b ) const
{
    double mt = 1.0 - t;
    return mt * mt * t * a + mt * t * t * ( 3.0 - b ) + t * t * t;
}

void VspSurf::MakeUTess( const vector<int> &num_u, vector<double> &u, const std::vector<int> & umerge ) const
{
    if ( umerge.size() != 0 )
    {
        if ( num_u.size() != umerge.size() )
        {
            printf( "Error.  num_u does not match umerge.\n" );
        }

        const int nusect = GetNumSectU();

        int sum = 0;
        int nu = 1;
        for ( int i = 0; i < umerge.size(); i++ )
        {
            sum += umerge[i];
            nu += num_u[i] - 1;
        }

        if ( nusect != sum )
        {
            printf( "Error.  umerge does not match nsect.\n" );
        }

        int iusect = 0;
        double ustart = m_Surface.get_u0();

        u.resize( nu );
        int iu = 0;

        for ( int i = 0; i < umerge.size(); i++ )
        {
            double uend = ustart;
            for ( int j = 0; j < umerge[i]; j++ )
            {
                double du, dv;
                surface_patch_type surf;
                m_Surface.get( surf, du, dv, iusect, 0 );

                uend += du;
                iusect++;
            }


            double du = uend - ustart;

            for ( int isecttess = 0; isecttess < num_u[i] - 1; ++isecttess )
            {
                u[iu] = ustart + du * Cluster( static_cast<double>( isecttess ) / ( num_u[i] - 1 ), m_RootCluster[i], m_TipCluster[i] );
                iu++;
            }
            ustart = uend;


        }
        u.back() = ustart;


    }
    else
    {
        surface_index_type i, j, nu;
        double umin, umax;

        const int nusect = GetNumSectU();

        assert( num_u.size() == nusect );
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
        umax = m_Surface.get_umax();

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
    double vmin, vmax, vabsmin, vle, vlelow, vleup;
    surface_index_type nv( num_v );

    vmin = m_Surface.get_v0();
    vmax = m_Surface.get_vmax();

    vabsmin = vmin;

    double tol = 1e-6;

    if ( IsMagicVParm() ) // V uses 'Magic' values for things like blunted TE.
    {
        vmin += TMAGIC;
        vmax -= TMAGIC;

        vle = ( vmin + vmax ) * 0.5;
        vlelow = vle - TMAGIC;
        vleup = vle + TMAGIC;

        vtess.resize(nv);
        int jle = ( nv - 1 ) / 2;
        int j = 0;
        for ( ; j < jle; ++j )
        {
            vtess[j] = vmin + ( vlelow - vmin ) * Cluster( 2.0 * static_cast<double>( j ) / ( nv - 1 ), m_TECluster, m_LECluster );
        }
        for ( ; j < nv; ++j )
        {
            vtess[j] = vleup + ( vmax - vleup ) * (Cluster( 2.0 * static_cast<double>( j - jle ) / ( nv - 1 ), m_LECluster, m_TECluster ));
        }

        if ( degen ) // DegenGeom, don't tessellate blunt TE or LE.
        {
            return;
        }

        piecewise_curve_type c1, c2;
        m_Surface.get_vconst_curve( c1, vmin );
        m_Surface.get_vconst_curve( c2, vmin + TMAGIC );

        if ( !c1.abouteq( c2, tol ) ) // V Min edge is not repeated.
        {
            for ( int j = 0; j < n_cap; j++ )
            {
                vtess.push_back( vabsmin + TMAGIC * j / (n_cap -1) );
            }
        }

        m_Surface.get_vconst_curve( c1, vmax );
        m_Surface.get_vconst_curve( c2, vmax - TMAGIC );

        if ( !c1.abouteq( c2, tol ) ) // V Max edge is not repeated.
        {
            for ( int j = 0; j < n_cap; j++ )
            {
                vtess.push_back( vmax + TMAGIC * j / (n_cap -1) );
            }
        }

        m_Surface.get_vconst_curve( c1, vlelow );
        m_Surface.get_vconst_curve( c2, vleup );

        if ( !c1.abouteq( c2, tol ) ) // Leading edge is not repeated.
        {
            for ( int j = 0; j < n_cap * 2 - 1; j++ )
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

void VspSurf::Tesselate( int num_u, int num_v, vector< vector< vec3d > > & pnts, vector< vector< vec3d > > & norms, vector< vector< vec3d > > & uw_pnts, const int &n_cap, bool degen ) const
{
    vector<int> num_u_vec( GetNumSectU(), num_u );
    Tesselate( num_u_vec, num_v, pnts, norms, uw_pnts, n_cap, degen );
}

void VspSurf::SplitTesselate( int num_u, int num_v, vector< vector< vector< vec3d > > > & pnts, vector< vector< vector< vec3d > > > & norms, const int &n_cap ) const
{
    vector<int> num_u_vec( GetNumSectU(), num_u );
    SplitTesselate( num_u_vec, num_v, pnts, norms, n_cap );
}

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

void VspSurf::Tesselate( const vector<double> &u, const vector<double> &v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts ) const
{
    int nu = u.size();
    int nv = v.size();

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
            if ( m_FlipNormal )
            {
                norms[i][j] = -nmat[i][j];
            }
            else
            {
                norms[i][j] = nmat[i][j];
            }
            uw_pnts[i][j].set_xyz( u[i], v[j], 0.0 );
        }
    }
}

void VspSurf::SplitTesselate( const vector<double> &usplit, const vector<double> &vsplit, const vector<double> &u, const vector<double> &v, std::vector< vector< vector< vec3d > > > & pnts,  std::vector< vector< vector< vec3d > > > & norms ) const
{
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
            double dnew = fabs( u[j] - usplit[i] );
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
            double dnew = fabs( v[j] - vsplit[i] );
            if ( dnew < d )
            {
                d = dnew;
                ivsplit[i] = j;
            }
        }
    }

    int nu = usplit.size() - 1;
    int nv = vsplit.size() - 1;

    int n = nu * nv;

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

void VspSurf::TessUFeatureLine( int iu, std::vector< vec3d > & pnts, double tol )
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

void VspSurf::TessWFeatureLine( int iw, std::vector< vec3d > & pnts, double tol )
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

void VspSurf::TessAdaptLine( double umin, double umax, double wmin, double wmax, std::vector< vec3d > & pnts, double tol, int Nlimit )
{
    vec3d pmin = CompPnt( umin, wmin );
    vec3d pmax = CompPnt( umax, wmax );

    TessAdaptLine( umin, umax, wmin, wmax, pmin, pmax, pnts, tol, Nlimit );

    pnts.push_back( pmax );
}

void VspSurf::TessAdaptLine( double umin, double umax, double wmin, double wmax, const vec3d & pmin, const vec3d & pmax, std::vector< vec3d > & pnts, double tol, int Nlimit, int Nadapt )
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

        if ( GetSurfType() == vsp::WING_SURF )
        {
            // Force all patch boundaries in u direction.
            vector < double > pmap;
            m_Surface.get_pmap_u( pmap );
            m_UFeature.insert( m_UFeature.end(), pmap.begin(), pmap.end() );
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
            m_WFeature.push_back( vmin + TMAGIC );
            m_WFeature.push_back( vmid - TMAGIC );
            m_WFeature.push_back( vmid + TMAGIC );
            m_WFeature.push_back( vmax - TMAGIC );
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
        ResetUWSkip();
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
      ResetUWSkip();
      return false;
    }

    rtn_flag = cc.create(m_Surface);

    if (!rtn_flag)
    {
      ResetUWSkip();
      return false;
    }

    m_Surface.set_u0( 0.0 );
    ResetUWSkip();
    return true;
}

bool VspSurf::CapUMax(int CapType, double len, double str, double offset, bool swflag)
{
    if (CapType == vsp::NO_END_CAP)
    {
      ResetUWSkip();
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
      ResetUWSkip();
      return false;
    }

    rtn_flag = cc.create(m_Surface);

    if (!rtn_flag)
    {
      ResetUWSkip();
      return false;
    }
    ResetUWSkip();
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

void VspSurf::SplitSurfs( const piecewise_surface_type &basesurf, vector< piecewise_surface_type > &surfvec )
{
    surfvec.push_back( basesurf );

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

    if ( c1.abouteq( c2, tol ) )
    {
        // Degenerate surface, skip it.
        // Opposite edges are equal.
        return false;
    }

    surf.get_vmin_bndy_curve( c1 );
    surf.get_vmax_bndy_curve( c2 );

    if ( c1.abouteq( c2, tol ) )
    {
        // Degenerate surface, skip it.
        // Opposite edges are equal.
        return false;
    }

    // Passed all tests, valid surface.
    return true;
}

void VspSurf::FetchXFerSurf( const std::string &geom_id, int surf_ind, int comp_ind, vector< XferSurf > &xfersurfs )
{
    vector < piecewise_surface_type > surfvec;
    SplitSurfs( m_Surface, surfvec );

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
        xsurf.m_SurfType = m_SurfType;
        xsurf.m_SurfCfdType = m_SurfCfdType;
        xsurf.m_CompIndx = comp_ind;
        xfersurfs.push_back( xsurf );
    }
}

void VspSurf::ExtractCPts( piecewise_surface_type &s, vector< vector< int > > &ptindxs, vector< vec3d > &allPntVec,
                  piecewise_surface_type::index_type &maxu, piecewise_surface_type::index_type &maxv,
                  piecewise_surface_type::index_type &nupatch, piecewise_surface_type::index_type &nvpatch,
                  piecewise_surface_type::index_type &nupts, piecewise_surface_type::index_type &nvpts )
{
    piecewise_surface_type::index_type minu, minv;
    piecewise_surface_type::index_type ip, jp;

    nupatch = s.number_u_patches();
    nvpatch = s.number_v_patches();

    s.degree_u( minu, maxu );
    s.degree_v( minv, maxv );

    nupts = nupatch * maxu + 1;
    nvpts = nvpatch * maxv + 1;

    ptindxs.resize( nupts );
    for( int i = 0; i < nupts; ++i )
    {
        ptindxs[i].resize( nvpts );
    }

    allPntVec.clear();

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
                    surface_patch_type::point_type p = patch->get_control_point( icp, jcp );
                    ptindxs[ ip * maxu + icp ][ jp * maxv + jcp ] = allPntVec.size();
                    allPntVec.push_back( vec3d( p[0], p[1], p[2] ) );
                }
        }
    }
}


void VspSurf::ToSTEP_BSpline_Quilt( STEPutil * step, vector<SdaiB_spline_surface_with_knots *> &surfs, bool splitsurf, bool mergepts, bool tocubic, double tol )
{
    // Make copy for local changes.
    piecewise_surface_type s( m_Surface );

    if( !m_FlipNormal )
    {
        s.reverse_v();
    }

    if ( tocubic )
    {
        s.to_cubic_u( tol );
        s.to_cubic_v( tol );
    }

    vector < piecewise_surface_type > surfvec;
    if ( splitsurf )
    {
        SplitSurfs( s, surfvec );
    }
    else
    {
        surfvec.push_back( s );
    }

    for ( int isurf = 0; isurf < surfvec.size(); isurf++ )
    {
        s = surfvec[isurf];

        // Don't export degenerate split patches
        if ( splitsurf && !CheckValidPatch( s ) )
        {
            continue;
        }

        piecewise_surface_type::index_type ip, jp;
        piecewise_surface_type::index_type nupatch, nvpatch;
        piecewise_surface_type::index_type maxu, maxv;
        piecewise_surface_type::index_type nupts, nvpts;

        vector< vector< int > > ptindxs;
        vector< vec3d > allPntVec;

        ExtractCPts( s, ptindxs, allPntVec, maxu, maxv, nupatch, nvpatch, nupts, nvpts );

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

        PntNodeCloud pnCloud;
        vector < SdaiCartesian_point* > usedPts;

        if ( mergepts )
        {
            //==== Build Map ====//
            pnCloud.AddPntNodes( allPntVec );

            //==== Use NanoFlann to Find Close Points and Group ====//
            IndexPntNodes( pnCloud, 1e-6 );

            //==== Load Used Points ====//
            for ( int i = 0 ; i < ( int )allPntVec.size() ; i++ )
            {
                if ( pnCloud.UsedNode( i ) )
                {
                    vec3d p = allPntVec[i];
                    SdaiCartesian_point *pt = step->MakePoint( p.x(), p.y(), p.z() );
                    usedPts.push_back( pt );
                }
            }
        }
        else
        {
            for ( int i = 0 ; i < ( int )allPntVec.size() ; i++ )
            {
                vec3d p = allPntVec[i];
                SdaiCartesian_point *pt = step->MakePoint( p.x(), p.y(), p.z() );
                usedPts.push_back( pt );
            }
        }

        for( int i = 0; i < nupts; ++i )
        {
            std::ostringstream ss;
            ss << "(";
            for( int j = 0; j < nvpts; j++ )
            {
                int pindx = ptindxs[i][j];

                SdaiCartesian_point *pt;

                if ( mergepts )
                {
                    pt = usedPts[ pnCloud.GetNodeUsedIndex( pindx ) ];
                }
                else
                {
                    pt = usedPts[ pindx ];
                }
                ss << "#" << pt->GetFileId();

                if( j < nvpts - 1 )
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

void VspSurf::ToIGES( DLL_IGES &model, bool splitsurf, bool tocubic, double tol )
{
    // Make copy for local changes.
    piecewise_surface_type s( m_Surface );

    if( !m_FlipNormal )
    {
        s.reverse_v();
    }

    if ( tocubic )
    {
        s.to_cubic_u( tol );
        s.to_cubic_v( tol );
    }

    vector < piecewise_surface_type > surfvec;
    if ( splitsurf )
    {
        SplitSurfs( s, surfvec );
    }
    else
    {
        surfvec.push_back( s );
    }

    for ( int is = 0; is < surfvec.size(); is++ )
    {
        s = surfvec[is];

        // Don't export degenerate split patches
        if ( splitsurf && !CheckValidPatch( s ) )
        {
            continue;
        }

        piecewise_surface_type::index_type ip, jp;
        piecewise_surface_type::index_type nupatch, nvpatch;
        piecewise_surface_type::index_type maxu, maxv;
        piecewise_surface_type::index_type nupts, nvpts;

        vector< vector< int > > ptindxs;
        vector< vec3d > allPntVec;

        ExtractCPts( s, ptindxs, allPntVec, maxu, maxv, nupatch, nvpatch, nupts, nvpts );

        vector < double > coeff( nupts * nvpts * 3 );

        int icoeff = 0;
        for( int j = 0; j < nvpts; j++ )
        {
            for( int i = 0; i < nupts; ++i )
            {
                int pindx = ptindxs[i][j];
                vec3d pt = allPntVec[ pindx ];

                for( int k = 0; k < 3; k++ )
                {
                    coeff[icoeff] = pt.v[k];
                    icoeff++;
                }
            }
        }

        vector < double > knotu;
        IGESKnots( maxu, nupatch, knotu );

        vector < double > knotv;
        IGESKnots( maxv, nvpatch, knotv );

        DLL_IGES_ENTITY_128 isurf( model, true );

        if( !isurf.SetNURBSData( nupts, nvpts, maxu + 1, maxv + 1,
             knotu.data(), knotv.data(), coeff.data(),
             false, false, false,
             0.0, 1.0*nupatch, 0.0, 1.0*nvpatch ) )
        {
            model.DelEntity( &isurf );
        }
    }
}

void VspSurf::IGESKnots( int deg, int npatch, vector< double > &knot )
{
    int i, j;

    knot.clear();

    for( i = 0; i <= deg; i++ )
    {
        knot.push_back( 0.0 );
    }
    for( i = 1; i <= npatch; ++i )
    {
        for( j = 0; j < deg; j++ )
        {
            knot.push_back( 1.0 * i );
        }
    }
    for( i = 0; i <= deg; i++ )
    {
        knot.push_back( 1.0 * npatch );
    }
}
