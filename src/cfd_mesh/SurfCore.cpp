//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "SurfCore.h"

SurfCore::SurfCore()
{
    m_NumU = m_NumW = 0;
}

SurfCore::~SurfCore()
{
}

void SurfCore::SetControlPnts( vector< vector < vec3d > > pnts )
{
    m_Pnts = pnts;

    m_NumU = m_Pnts.size();
    m_NumW = m_Pnts[0].size();
    m_MaxU = ( m_NumU - 1 ) / 3;
    m_MaxW = ( m_NumW - 1 ) / 3;
}

//===== Compute Point On Surf Given  U W (Between 0 1 ) =====//
vec3d SurfCore::CompPnt01( double u, double w )
{
    return CompPnt( u * m_MaxU, w * m_MaxW );
}


//===== Compute Tangent In U Direction   =====//
vec3d SurfCore::CompTanU01( double u01, double w01 )
{
    return CompTanU( u01 * m_MaxU, w01 * m_MaxW );
}

//===== Compute Tangent In W Direction   =====//
vec3d SurfCore::CompTanW01( double u01, double w01 )
{
    return CompTanW( u01 * m_MaxU, w01 * m_MaxW );
}

//===== Compute Second Derivative U,U   =====//
vec3d SurfCore::CompTanUU( double u, double w )
{
    return CompBez( u, w, &BlendDeriv2Funcs, &BlendFuncs );
}

//===== Compute Second Derivative W,W   =====//
vec3d SurfCore::CompTanWW( double u, double w )
{
    return CompBez( u, w, &BlendFuncs, &BlendDeriv2Funcs );
}

//===== Compute Second Derivative U,W   =====//
vec3d SurfCore::CompTanUW( double u, double w )
{
    return CompBez( u, w, &BlendDerivFuncs, &BlendDerivFuncs );
}

//===== Compute Tangent In U Direction   =====//
vec3d SurfCore::CompTanU( double u, double w )
{
    return CompBez( u, w, &BlendDerivFuncs, &BlendFuncs );
}

//===== Compute Tangent In W Direction   =====//
vec3d SurfCore::CompTanW( double u, double w )
{
    return CompBez( u, w, &BlendFuncs, &BlendDerivFuncs );
}

//===== Compute Point On Surf Given  U W =====//
vec3d SurfCore::CompPnt( double u, double w )
{
    return CompBez( u, w, &BlendFuncs, &BlendFuncs );
}



//===== Generic Bezier Surface Calculation  =====//
vec3d SurfCore::CompBez( double u, double w,
                     void ( *uBlendFun )( double u, double& F1, double& F2, double& F3, double& F4 ),
                     void ( *wBlendFun )( double u, double& F1, double& F2, double& F3, double& F4 ) )
{
    vec3d pnt;

    if ( m_NumU < 4 || m_NumW < 4 )
    {
        return pnt;
    }

    if ( u < 0.0 )
    {
        u = 0.0;
    }
    if ( w < 0.0 )
    {
        w = 0.0;
    }

    double F1u, F2u, F3u, F4u;
    double F1w, F2w, F3w, F4w;

    int trunc_u = ( int )u;
    int u_ind = trunc_u * 3;
    if ( u_ind >= m_NumU - 1 )
    {
        trunc_u = trunc_u - 1;
        u_ind = m_NumU - 4;
    }
    uBlendFun( u - ( double )trunc_u, F1u, F2u, F3u, F4u );

    int trunc_w = ( int )w;
    int w_ind = trunc_w * 3;
    if ( w_ind >= m_NumW - 1 )
    {
        trunc_w = trunc_w - 1;
        w_ind = m_NumW - 4;
    }
    wBlendFun( w - ( double )trunc_w, F1w, F2w, F3w, F4w );

    pnt =
        ( ( m_Pnts[u_ind][w_ind] * F1u     + m_Pnts[u_ind + 1][w_ind] * F2u +
            m_Pnts[u_ind + 2][w_ind] * F3u   + m_Pnts[u_ind + 3][w_ind] * F4u ) * F1w ) +
        ( ( m_Pnts[u_ind][w_ind + 1] * F1u   + m_Pnts[u_ind + 1][w_ind + 1] * F2u +
            m_Pnts[u_ind + 2][w_ind + 1] * F3u + m_Pnts[u_ind + 3][w_ind + 1] * F4u ) * F2w ) +
        ( ( m_Pnts[u_ind][w_ind + 2] * F1u  + m_Pnts[u_ind + 1][w_ind + 2] * F2u +
            m_Pnts[u_ind + 2][w_ind + 2] * F3u + m_Pnts[u_ind + 3][w_ind + 2] * F4u ) * F3w ) +
        ( ( m_Pnts[u_ind][w_ind + 3] * F1u  + m_Pnts[u_ind + 1][w_ind + 3] * F2u +
            m_Pnts[u_ind + 2][w_ind + 3] * F3u + m_Pnts[u_ind + 3][w_ind + 3] * F4u ) * F4w );

    return pnt;
}

//===== Compute Blending Functions  =====//
void SurfCore::BlendFuncs( double u, double& F1, double& F2, double& F3, double& F4 )
{
    //==== Compute All Blending Functions ====//
    double uu = u * u;
    double one_u = 1.0 - u;
    double one_u_sqr = one_u * one_u;

    F1 = one_u * one_u_sqr;
    F2 = 3.0 * u * one_u_sqr;
    F3 = 3.0 * uu * one_u;
    F4 = uu * u;
}

void SurfCore::BlendDerivFuncs( double u, double& F1, double& F2, double& F3, double& F4 )
{
    double uu    = u * u;
    double one_u = 1.0 - u;
    F1 = -3.0 * one_u * one_u;
    F2 = 3.0 - 12.0 * u + 9.0 * uu;
    F3 = 6.0 * u - 9.0 * uu;
    F4 = 3.0 * uu;
}

void SurfCore::BlendDeriv2Funcs( double u, double& F1, double& F2, double& F3, double& F4 )
{
    F1 = 6.0 - 6.0 * u;
    F2 = -12.0 + 18.0 * u;
    F3 = 6.0 - 18.0 * u;
    F4 = 6.0 * u;
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
        double umid = m_MaxU / 2.0;
        double wmid = m_MaxW / 2.0;

        u = u + ( umid - u ) * bump;
        w = w + ( wmid - w ) * bump;

        S_u = CompTanU( u, w );
        S_w = CompTanW( u, w );

        E = dot( S_u, S_u );
        G = dot( S_w, S_w );
    }
    else if( E < tol ) // U direction degenerate
    {
        double wmid = m_MaxW / 2.0;
        w = w + ( wmid - w ) * bump;

        S_u = CompTanU( u, w );
        S_w = CompTanW( u, w );

        E = dot( S_u, S_u );
        G = dot( S_w, S_w );
    }
    else if( G < tol ) // W direction degenerate
    {
        double umid = m_MaxU / 2.0;
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
    for ( int i = 0 ; i < m_NumU ; i++ )
        for ( int j = 0 ; j < m_NumW ; j++ )
        {
            if ( m_Pnts[i][j][1] > val )
            {
                return false;
            }
        }
    return true;


}

bool SurfCore::OnYZeroPlane()
{
    double tol = 0.0000001;
    bool onPlaneFlag = true;
    for ( int i = 0 ; i < m_NumU ; i++ )                // Border Curve 1 w = 0
    {
        if ( fabs( m_Pnts[i][0][1] ) > tol )
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
        if ( fabs( m_Pnts[i][m_NumW - 1][1] ) > tol )
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
        if ( fabs( m_Pnts[0][i][1] ) > tol )
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
        if ( fabs( m_Pnts[m_NumU - 1][i][1] ) > tol )
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
    for ( int i = 0 ; i < m_NumU ; i++ )
        for ( int j = 0 ; j < m_NumW ; j++ )
        {
            double yval = m_Pnts[i][j][1];
            if ( fabs( yval ) > tol )
            {
                return false;
            }
        }
    return true;
}

void SurfCore::LoadBorderCurves( vector< vector <vec3d> > & borderCurves )
{
    vector< vec3d > borderPnts;

    borderPnts.clear();
    for ( int i = 0 ; i < m_NumU ; i++ )                // Border Curve w = 0
    {
        borderPnts.push_back( m_Pnts[i][0] );
    }
    borderCurves.push_back( borderPnts );

    borderPnts.clear();
    for ( int i = 0 ; i < m_NumU ; i++ )                // Border Curve w = max
    {
        borderPnts.push_back( m_Pnts[i][m_NumW - 1] );
    }
    borderCurves.push_back( borderPnts );

    borderPnts.clear();
    for ( int i = 0 ; i < m_NumW ; i++ )                // Border Curve u = 0
    {
        borderPnts.push_back( m_Pnts[0][i] );
    }
    borderCurves.push_back( borderPnts );

    borderPnts.clear();
    for ( int i = 0 ; i < m_NumW ; i++ )                // Border Curve u = max
    {
        borderPnts.push_back( m_Pnts[m_NumU - 1][i] );
    }
    borderCurves.push_back( borderPnts );
}

bool SurfCore::SurfMatch( SurfCore* otherSurf )
{
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
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ i ][ j ] ) > tol )
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
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ m_NumU - 1 - i ][ j ] ) > tol )
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
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ i ][ m_NumW - 1 - j ] ) > tol )
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
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ m_NumU - 1 - i ][ m_NumW - 1 - j ] ) > tol )
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
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ j ][ i ] ) > tol )
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
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ m_NumW - 1 - j ][ i ] ) > tol )
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
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ j ][ m_NumU - 1 - i ] ) > tol )
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
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ m_NumW - 1 - j ][ m_NumU - 1 - i ] ) > tol )
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

