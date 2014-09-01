//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "SurfCore.h"

#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/surface/piecewise_body_of_revolution_creator.hpp"
#include "eli/geom/surface/piecewise_capped_surface_creator.hpp"
#include "eli/geom/intersect/minimum_distance_surface.hpp"

typedef piecewise_surface_type::index_type surface_index_type;
typedef piecewise_surface_type::point_type surface_point_type;
typedef piecewise_surface_type::rotation_matrix_type surface_rotation_matrix_type;
typedef piecewise_surface_type::bounding_box_type surface_bounding_box_type;

typedef eli::geom::curve::piecewise_linear_creator<double, 3, surface_tolerance_type> piecewise_linear_creator_type;
typedef eli::geom::surface::piecewise_general_skinning_surface_creator<double, 3, surface_tolerance_type> general_creator_type;
typedef eli::geom::surface::piecewise_capped_surface_creator<double, 3, surface_tolerance_type> capped_creator_type;



SurfCore::SurfCore()
{
    m_NumU = m_NumW = 0;
}

SurfCore::~SurfCore()
{
}

void SurfCore::SetControlPnts( vector< vector < vec3d > > pnts )
{
    m_NumU = pnts.size();
    m_NumW = pnts[0].size();

    // Assume Cubic patches.
    int nupatch = ( m_NumU - 1 ) / 3;
    int nvpatch = ( m_NumW - 1 ) / 3;

    m_Surface.init_uv( nupatch, nvpatch ); // du = 1, dv = 1, u0 = 0, & v0 = 0 implied

    surface_patch_type::index_type ip, jp;
    for( ip = 0; ip < nupatch; ++ip )
    {
        for( jp = 0; jp < nvpatch; ++jp )
        {
            surface_patch_type::index_type icp, jcp;

            surface_patch_type patch;
            patch.resize( 3, 3 );

            for( icp = 0; icp <= 3; ++icp )
            {
                for( jcp = 0; jcp <= 3; ++jcp )
                {
                    vec3d p = pnts[ ip * 3 + icp ][ jp * 3 + jcp ];
                    surface_point_type cp( p.x(), p.y(), p.z() );
                    patch.set_control_point( cp, icp, jcp );
                }
            }
            m_Surface.set( patch, ip, jp );
        }
    }
}

vector< vector< vec3d > > SurfCore::GetControlPnts()
{
    vector< vector < vec3d > > ret;

    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;


    nupatch = m_Surface.number_u_patches();
    nvpatch = m_Surface.number_v_patches();

    int nupts = nupatch * 3 + 1;
    int nvpts = nvpatch * 3 + 1;

    ret.resize( nupts );
    for( int i = 0; i < nupts; ++i )
    {
        ret[i].resize( nvpts );
    }

    for( ip = 0; ip < nupatch; ++ip )
    {
        for( jp = 0; jp < nvpatch; ++jp )
        {
            surface_patch_type::index_type icp, jcp;

            surface_patch_type *patch = m_Surface.get_patch( ip, jp );

            for( icp = 0; icp <= 3; ++icp )
            {
                for( jcp = 0; jcp <= 3; ++jcp )
                {
                    surface_point_type cp;
                    cp = patch->get_control_point( icp, jcp );
                    ret[ ip * 3 + icp ][ jp * 3 + jcp ]  = vec3d( cp.x(), cp.y(), cp.z() );
                }
            }
        }
    }

    return ret;
}



//===== Compute Point On Surf Given  U W (Between 0 1 ) =====//
vec3d SurfCore::CompPnt01( double u, double w )
{
    return CompPnt( u * GetMaxU(), w * GetMaxW() );
}

//===== Compute Tangent In U Direction   =====//
vec3d SurfCore::CompTanU01( double u01, double w01 )
{
    return CompTanU( u01 * GetMaxU(), w01 * GetMaxW() );
}

//===== Compute Tangent In W Direction   =====//
vec3d SurfCore::CompTanW01( double u01, double w01 )
{
    return CompTanW( u01 * GetMaxU(), w01 * GetMaxW() );
}

//===== Compute Second Derivative U,U   =====//
vec3d SurfCore::CompTanUU( double u, double w )
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_uu( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Second Derivative W,W   =====//
vec3d SurfCore::CompTanWW( double u, double w )
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_vv( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Second Derivative U,W   =====//
vec3d SurfCore::CompTanUW( double u, double w )
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_uv( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Tangent In U Direction   =====//
vec3d SurfCore::CompTanU( double u, double w )
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_u( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Tangent In W Direction   =====//
vec3d SurfCore::CompTanW( double u, double w )
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_v( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Point On Surf Given  U W =====//
vec3d SurfCore::CompPnt( double u, double w )
{
    vec3d rtn;

    double u0 = m_Surface.get_u0();
    double w0 = m_Surface.get_v0();

    double umx = m_Surface.get_umax();
    double wmx = m_Surface.get_vmax();

    if ( u < u0 )
        u = u0;

    if ( w < w0 )
        w = w0;

    if ( u > umx )
        u = umx;

    if ( w > wmx )
        w = wmx;

    surface_point_type p( m_Surface.f( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Surface Curvature Metrics Given  U W =====//
void SurfCore::CompCurvature( double u, double w, double& k1, double& k2, double& ka, double& kg )
{

    double tol = 1e-10;

    double bump = 1e-3;

    // First derivative vectors
    vec3d S_u = CompTanU( u, w );
    vec3d S_w = CompTanW( u, w );

    double E = dot( S_u, S_u );
    double G = dot( S_w, S_w );

    if( E < tol && G < tol )
    {
        double umid = GetMaxU() / 2.0;
        double wmid = GetMaxW() / 2.0;

        u = u + ( umid - u ) * bump;
        w = w + ( wmid - w ) * bump;

        S_u = CompTanU( u, w );
        S_w = CompTanW( u, w );

        E = dot( S_u, S_u );
        G = dot( S_w, S_w );
    }
    else if( E < tol ) // U direction degenerate
    {
        double wmid = GetMaxW() / 2.0;
        w = w + ( wmid - w ) * bump;

        S_u = CompTanU( u, w );
        S_w = CompTanW( u, w );

        E = dot( S_u, S_u );
        G = dot( S_w, S_w );
    }
    else if( G < tol ) // W direction degenerate
    {
        double umid = GetMaxU() / 2.0;
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
    if( fabs( kmax ) > fabs( kmin ) )
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

bool SurfCore::LessThanY( double val )
{
    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;
    nupatch = m_Surface.number_u_patches();
    nvpatch = m_Surface.number_v_patches();

    for( ip = 0; ip < nupatch; ++ip )
    {
        for( jp = 0; jp < nvpatch; ++jp )
        {
            surface_patch_type::index_type icp, jcp;
            surface_patch_type *patch = m_Surface.get_patch( ip, jp );

            for( icp = 0; icp <= patch->degree_u(); ++icp )
            {
                for( jcp = 0; jcp <= patch->degree_v(); ++jcp )
                {
                    surface_point_type cp;
                    cp = patch->get_control_point( icp, jcp );
                    if ( cp.y() > val )
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool SurfCore::OnYZeroPlane()
{
    vector< vector< vec3d > > pnts = GetControlPnts();

    double tol = 0.0000001;
    bool onPlaneFlag = true;
    for ( int i = 0 ; i < m_NumU ; i++ )                // Border Curve 1 w = 0
    {
        if ( fabs( pnts[i][0][1] ) > tol )
        {
            onPlaneFlag = false;
        }
    }
    if ( onPlaneFlag )
    {
        return onPlaneFlag;
    }

    onPlaneFlag = true;
    for ( int i = 0 ; i < m_NumU ; i++ )                // Border Curve 2 w = max
    {
        if ( fabs( pnts[i][m_NumW - 1][1] ) > tol )
        {
            onPlaneFlag = false;
        }
    }
    if ( onPlaneFlag )
    {
        return onPlaneFlag;
    }

    onPlaneFlag = true;
    for ( int i = 0 ; i < m_NumW ; i++ )                // Border Curve 3 u = 0
    {
        if ( fabs( pnts[0][i][1] ) > tol )
        {
            onPlaneFlag = false;
        }
    }
    if ( onPlaneFlag )
    {
        return onPlaneFlag;
    }

    onPlaneFlag = true;
    for ( int i = 0 ; i < m_NumW ; i++ )                // Border Curve 4 u = max
    {
        if ( fabs( pnts[m_NumU - 1][i][1] ) > tol )
        {
            onPlaneFlag = false;
        }
    }
    if ( onPlaneFlag )
    {
        return onPlaneFlag;
    }

    return false;
}

bool SurfCore::PlaneAtYZero()
{
    double tol = 0.000001;

    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;
    nupatch = m_Surface.number_u_patches();
    nvpatch = m_Surface.number_v_patches();

    for( ip = 0; ip < nupatch; ++ip )
    {
        for( jp = 0; jp < nvpatch; ++jp )
        {
            surface_patch_type::index_type icp, jcp;
            surface_patch_type *patch = m_Surface.get_patch( ip, jp );

            for( icp = 0; icp <= patch->degree_u(); ++icp )
            {
                for( jcp = 0; jcp <= patch->degree_v(); ++jcp )
                {
                    surface_point_type cp;
                    cp = patch->get_control_point( icp, jcp );
                    if ( fabs( cp.y() ) > tol )
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

void SurfCore::LoadBorderCurves( vector< vector <vec3d> > & borderCurves )
{
    vector< vector< vec3d > > pnts = GetControlPnts();

    vector< vec3d > borderPnts;

    borderPnts.clear();
    for ( int i = 0 ; i < m_NumU ; i++ )                // Border Curve w = 0
    {
        borderPnts.push_back( pnts[i][0] );
    }
    borderCurves.push_back( borderPnts );

    borderPnts.clear();
    for ( int i = 0 ; i < m_NumU ; i++ )                // Border Curve w = max
    {
        borderPnts.push_back( pnts[i][m_NumW - 1] );
    }
    borderCurves.push_back( borderPnts );

    borderPnts.clear();
    for ( int i = 0 ; i < m_NumW ; i++ )                // Border Curve u = 0
    {
        borderPnts.push_back( pnts[0][i] );
    }
    borderCurves.push_back( borderPnts );

    borderPnts.clear();
    for ( int i = 0 ; i < m_NumW ; i++ )                // Border Curve u = max
    {
        borderPnts.push_back( pnts[m_NumU - 1][i] );
    }
    borderCurves.push_back( borderPnts );
}

bool SurfCore::SurfMatch( SurfCore* otherSurf )
{
    vector< vector< vec3d > > pnts = GetControlPnts();

    double tol = 0.00000001;

    if ( otherSurf )
    {
        vector< vector< vec3d > > oPnts = otherSurf->GetControlPnts();
        int oNumU = oPnts.size();
        int oNumW = oPnts[0].size();

        if ( oNumU == m_NumU && oNumW == m_NumW ) // Possible match
        {
            bool match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ i ][ j ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }

            if ( match )
            {
                return true;
            }

            match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ m_NumU - 1 - i ][ j ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }


            if ( match )
            {
                return true;
            }

            match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ i ][ m_NumW - 1 - j ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }


            if ( match )
            {
                return true;
            }

            match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ m_NumU - 1 - i ][ m_NumW - 1 - j ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }

            if ( match )
            {
                return true;
            }
        }

        if ( oNumU == m_NumW && oNumW == m_NumU ) // Possible flipped match
        {
            bool match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ j ][ i ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }

            if ( match )
            {
                return true;
            }

            match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ m_NumW - 1 - j ][ i ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }

            if ( match )
            {
                return true;
            }

            match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ j ][ m_NumU - 1 - i ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }

            if ( match )
            {
                return true;
            }

            match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ m_NumW - 1 - j ][ m_NumU - 1 - i ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }

            if ( match )
            {
                return true;
            }
        }
    }
    return false;
}
