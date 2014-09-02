//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "SurfCore.h"
#include "BezierCurve.h"

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
}

SurfCore::~SurfCore()
{
}

void SurfCore::SetControlPnts( vector< vector < vec3d > > pnts )
{
    int numU = pnts.size();
    int numW = pnts[0].size();

    // Assume Cubic patches.
    int nupatch = ( numU - 1 ) / 3;
    int nvpatch = ( numW - 1 ) / 3;

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

vector< vector< vec3d > > SurfCore::GetControlPnts() const
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

            const surface_patch_type *patch = m_Surface.get_patch( ip, jp );

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

void SurfCore::ExtractBorderControlPnts( const vec3d &uw0, const vec3d &uw1, vector< vec3d > & control_pnts )
{
    vector< vector< vec3d > > cpnts = GetControlPnts();
    int nu = cpnts.size();
    if ( nu < 4 )
    {
        return;
    }
    int nw = cpnts[0].size();
    if ( nw < 4 )
    {
        return;
    }

    double tol = 1.0e-12;

    if ( fabs( uw0.x() - uw1.x() ) < tol )
    {
        int ind = ( int )( uw0.x() + 0.5 ) * 3;

        if ( ind > nu - 1 )
        {
            ind = nu - 1;
        }

        for ( int w = 0 ; w < nw ; w++ )
        {
            control_pnts.push_back( cpnts[ind][w] );
        }
    }
    else if ( fabs( uw0.y() - uw1.y() ) < tol )
    {
        int ind = ( int )( uw0.y() + 0.5 ) * 3;

        if ( ind > nw - 1 )
        {
            ind = nw - 1;
        }

        for ( int u = 0 ; u < nu ; u++ )
        {
            control_pnts.push_back( cpnts[u][ind] );
        }
    }
}

//===== Compute Point On Surf Given  U W (Between 0 1 ) =====//
vec3d SurfCore::CompPnt01( double u, double w ) const
{
    return CompPnt( u * GetMaxU(), w * GetMaxW() );
}

//===== Compute Tangent In U Direction   =====//
vec3d SurfCore::CompTanU01( double u01, double w01 ) const
{
    return CompTanU( u01 * GetMaxU(), w01 * GetMaxW() );
}

//===== Compute Tangent In W Direction   =====//
vec3d SurfCore::CompTanW01( double u01, double w01 ) const
{
    return CompTanW( u01 * GetMaxU(), w01 * GetMaxW() );
}

//===== Compute Second Derivative U,U   =====//
vec3d SurfCore::CompTanUU( double u, double w ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_uu( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Second Derivative W,W   =====//
vec3d SurfCore::CompTanWW( double u, double w ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_vv( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Second Derivative U,W   =====//
vec3d SurfCore::CompTanUW( double u, double w ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_uv( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Tangent In U Direction   =====//
vec3d SurfCore::CompTanU( double u, double w ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_u( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Tangent In W Direction   =====//
vec3d SurfCore::CompTanW( double u, double w ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_v( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Point On Surf Given  U W =====//
vec3d SurfCore::CompPnt( double u, double w ) const
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
void SurfCore::CompCurvature( double u, double w, double& k1, double& k2, double& ka, double& kg ) const
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

bool SurfCore::LessThanY( double val ) const
{
    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;
    nupatch = m_Surface.number_u_patches();
    nvpatch = m_Surface.number_v_patches();

    for( ip = 0; ip < nupatch; ++ip )
    {
        for( jp = 0; jp < nvpatch; ++jp )
        {
            surface_patch_type::index_type icp, jcp;
            const surface_patch_type *patch = m_Surface.get_patch( ip, jp );

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

bool SurfCore::OnYZeroPlane() const
{
    vector< vector< vec3d > > pnts = GetControlPnts();
    int numU = pnts.size();
    int numW = pnts[0].size();

    double tol = 0.0000001;
    bool onPlaneFlag = true;
    for ( int i = 0 ; i < numU ; i++ )                // Border Curve 1 w = 0
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
    for ( int i = 0 ; i < numU ; i++ )                // Border Curve 2 w = max
    {
        if ( fabs( pnts[i][numW - 1][1] ) > tol )
        {
            onPlaneFlag = false;
        }
    }
    if ( onPlaneFlag )
    {
        return onPlaneFlag;
    }

    onPlaneFlag = true;
    for ( int i = 0 ; i < numW ; i++ )                // Border Curve 3 u = 0
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
    for ( int i = 0 ; i < numW ; i++ )                // Border Curve 4 u = max
    {
        if ( fabs( pnts[numU - 1][i][1] ) > tol )
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

bool SurfCore::PlaneAtYZero() const
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
            const surface_patch_type *patch = m_Surface.get_patch( ip, jp );

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

Bezier_curve SurfCore::GetBorderCurve( int iborder ) const
{
    piecewise_curve_type pwc;

    curve_segment_type s;
    s.resize( 3 );

    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;
    surface_patch_type::index_type icp, jcp;

    nupatch = m_Surface.number_u_patches();
    nvpatch = m_Surface.number_v_patches();

    if ( iborder == UMIN || iborder == UMAX )
    {
        if ( iborder == UMIN )
            ip = 0;
        else
            ip = nupatch - 1;


        pwc.set_t0( m_Surface.get_v0() );

        for( jp = 0; jp < nvpatch; ++jp )
        {
            const surface_patch_type *patch = m_Surface.get_patch( ip, jp );
            double dv = patch->get_vmax() - patch->get_vmin();

            s.resize( patch->degree_v() );

            if ( iborder == UMIN )
                icp = 0;
            else
                icp = patch->degree_u();

            for( jcp = 0; jcp <= patch->degree_v(); ++jcp )
            {
                surface_point_type cp;
                cp = patch->get_control_point( icp, jcp );
                s.set_control_point( cp, jcp );
            }
            pwc.push_back( s, dv );
        }
    }
    else if ( iborder == WMIN || iborder == WMAX )
    {
        if ( iborder == WMIN )
            jp = 0;
        else
            jp = nvpatch - 1;

        pwc.set_t0( m_Surface.get_u0() );

        for( ip = 0; ip < nupatch; ++ip )
        {
            const surface_patch_type *patch = m_Surface.get_patch( ip, jp );
            double du = patch->get_umax() - patch->get_umin();

            s.resize( patch->degree_u() );

            if ( iborder == WMIN )
                jcp = 0;
            else
                jcp = patch->degree_v();

            for( icp = 0; icp <= patch->degree_u(); ++icp )
            {
                surface_point_type cp;
                cp = patch->get_control_point( icp, jcp );
                s.set_control_point( cp, icp );
            }
            pwc.push_back( s, du );
        }
    }

    Bezier_curve crv( pwc );
    return crv;
}

void SurfCore::LoadBorderCurves( vector < Bezier_curve > & borderCurves ) const
{
	borderCurves.push_back( GetBorderCurve( WMIN ) );
	borderCurves.push_back( GetBorderCurve( WMAX ) );
	borderCurves.push_back( GetBorderCurve( UMIN ) );
	borderCurves.push_back( GetBorderCurve( UMAX ) );
}

bool SurfCore::SurfMatch( SurfCore* otherSurf ) const
{
    vector< vector< vec3d > > pnts = GetControlPnts();
    int numU = pnts.size();
    int numW = pnts[0].size();

    double tol = 0.00000001;

    if ( otherSurf )
    {
        vector< vector< vec3d > > oPnts = otherSurf->GetControlPnts();
        int oNumU = oPnts.size();
        int oNumW = oPnts[0].size();

        if ( oNumU == numU && oNumW == numW ) // Possible match
        {
            bool match = true;
            for ( int i = 0; i < numU; i++ )
            {
                for ( int j = 0; j < numW; j++ )
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
            for ( int i = 0; i < numU; i++ )
            {
                for ( int j = 0; j < numW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ numU - 1 - i ][ j ] ) > tol )
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
            for ( int i = 0; i < numU; i++ )
            {
                for ( int j = 0; j < numW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ i ][ numW - 1 - j ] ) > tol )
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
            for ( int i = 0; i < numU; i++ )
            {
                for ( int j = 0; j < numW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ numU - 1 - i ][ numW - 1 - j ] ) > tol )
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

        if ( oNumU == numW && oNumW == numU ) // Possible flipped match
        {
            bool match = true;
            for ( int i = 0; i < numU; i++ )
            {
                for ( int j = 0; j < numW; j++ )
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
            for ( int i = 0; i < numU; i++ )
            {
                for ( int j = 0; j < numW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ numW - 1 - j ][ i ] ) > tol )
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
            for ( int i = 0; i < numU; i++ )
            {
                for ( int j = 0; j < numW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ j ][ numU - 1 - i ] ) > tol )
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
            for ( int i = 0; i < numU; i++ )
            {
                for ( int j = 0; j < numW; j++ )
                {
                    if ( dist_squared( pnts[ i ][ j ], oPnts[ numW - 1 - j ][ numU - 1 - i ] ) > tol )
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

void SurfCore::WriteSurf( FILE* fp ) const
{
    vector< vector< vec3d > > pntVec = GetControlPnts();
    int numU = pntVec.size();
    int numW = pntVec[0].size();

    fprintf( fp, "%d		// Num Pnts U \n",    numU );
    fprintf( fp, "%d		// Num Pnts W \n",    numW );
    fprintf( fp, "%d		// Total Pnts (0,0),(0,1),(0,2)..(0,numW-1),(1,0)..(1,numW-1)..(numU-1,0)..(numU-1,numW-1)\n",    numU * numW );

    for ( int i = 0 ; i < numU ; i++ )
    {
        for ( int j = 0 ; j < numW ; j++ )
        {
            fprintf( fp, "%20.20lf, %20.20lf, %20.20lf \n",
                     pntVec[i][j].x(), pntVec[i][j].y(), pntVec[i][j].z() );
        }
    }
}
