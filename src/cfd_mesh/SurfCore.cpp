//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "SurfCore.h"
#include "BezierCurve.h"

typedef piecewise_surface_type::bounding_box_type surface_bounding_box_type;

SurfCore::SurfCore()
{
}

SurfCore::~SurfCore()
{
}

void SurfCore::GetBorderCurve( const vec3d &uw0, const vec3d &uw1, Bezier_curve &crv ) const
{
    int iborder = -1;
    double tol = 1.0e-12;

    if ( std::abs( uw0.x() - uw1.x() ) < tol ) // U const, UMIN or UMAX
    {
        double umid = ( m_Surface.get_umax() + m_Surface.get_u0() ) / 2.0;

        if ( uw0.x() < umid )
            iborder = UMIN;
        else
            iborder = UMAX;
    }
    else if ( std::abs( uw0.y() - uw1.y() ) < tol )
    {
        double vmid = ( m_Surface.get_vmax() + m_Surface.get_v0() ) / 2.0;

        if ( uw0.y() < vmid )
            iborder = WMIN;
        else
            iborder = WMAX;
    }

    if ( iborder >= UMIN )
    crv = GetBorderCurve( iborder );
}

//===== Compute Point On Surf Given  U W (Between 0 1 ) =====//
vec3d SurfCore::CompPnt01( double u, double w ) const
{
    return CompPnt( GetMinU() + u * GetDU(), GetMinW() + w * GetDW() );
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

vec3d SurfCore::CompNorm( double u, double w ) const
{
    vec3d rtn;

    double umn = m_Surface.get_u0();
    double wmn = m_Surface.get_v0();

    double umx = m_Surface.get_umax();
    double wmx = m_Surface.get_vmax();

    double slop = 1e-3;
    if( u < (umn - slop) || w < (wmn - slop) || u > (umx + slop) || w > (wmx + slop) )
    {
        printf("BAD parameter in SurfCore::CompNorm! %f %f\n", u, w );
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

    surface_point_type p( m_Surface.normal( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Point On Surf Given  U W =====//
vec3d SurfCore::CompPnt( double u, double w ) const
{
    vec3d rtn;

    double umn = m_Surface.get_u0();
    double wmn = m_Surface.get_v0();

    double umx = m_Surface.get_umax();
    double wmx = m_Surface.get_vmax();

    double slop = 1e-3;
    if( u < (umn - slop) || w < (wmn - slop) || u > (umx + slop) || w > (wmx + slop) )
    {
        printf("BAD parameter in SurfCore::CompPnt! %f %f\n", u, w );
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

    surface_point_type p( m_Surface.f( u, w ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Surface Curvature Metrics Given  U W =====//
void SurfCore::CompCurvature( double u, double w, double& k1, double& k2, double& ka, double& kg ) const
{
    double umn = m_Surface.get_u0();
    double wmn = m_Surface.get_v0();

    double umx = m_Surface.get_umax();
    double wmx = m_Surface.get_vmax();

    double slop = 1e-3;
    if( u < (umn - slop) || w < (wmn - slop) || u > (umx + slop) || w > (wmx + slop) )
    {
        printf("BAD parameter in SurfCore::CompCurvature! %f %f\n", u, w );
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
        double umid = GetMidU();
        double wmid = GetMidW();

        u = u + ( umid - u ) * bump;
        w = w + ( wmid - w ) * bump;

        S_u = CompTanU( u, w );
        S_w = CompTanW( u, w );

        E = dot( S_u, S_u );
        G = dot( S_w, S_w );
    }
    else if( E < tol ) // U direction degenerate
    {
        double wmid = GetMidW();
        w = w + ( wmid - w ) * bump;

        S_u = CompTanU( u, w );
        S_w = CompTanW( u, w );

        E = dot( S_u, S_u );
        G = dot( S_w, S_w );
    }
    else if( G < tol ) // W direction degenerate
    {
        double umid = GetMidU();
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

int SurfCore::UWPointOnBorder( double u, double w, double tol ) const
{
    double umn = m_Surface.get_u0();
    double wmn = m_Surface.get_v0();

    double umx = m_Surface.get_umax();
    double wmx = m_Surface.get_vmax();

    int ret = NOBNDY;

    if ( std::abs( u - umn ) < tol )
        ret = UMIN;

    if ( std::abs( u - umx ) < tol )
        ret = UMAX;

    if ( std::abs( w - wmn ) < tol )
        ret = WMIN;

    if ( std::abs( w - wmx ) < tol )
        ret = WMAX;

    return ret;
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

bool SurfCore::PlaneAtYZero() const
{
    double tol = 1.0e-6;

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
                    if ( std::abs( cp.y() ) > tol )
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

    if ( iborder == UMIN || iborder == UMAX )
    {
        if ( iborder == UMIN )
            m_Surface.get_umin_bndy_curve( pwc );
        else
            m_Surface.get_umax_bndy_curve( pwc );
    }
    else if ( iborder == WMIN || iborder == WMAX )
    {
        if ( iborder == WMIN )
            m_Surface.get_vmin_bndy_curve( pwc );
        else
            m_Surface.get_vmax_bndy_curve( pwc );
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
    piecewise_surface_type osurf, swaposurf;

    osurf = otherSurf->m_Surface;
    if( MatchThisOrientation( osurf ) )
    {
        return true;
    }

    osurf = otherSurf->m_Surface;
    osurf.reverse_u();
    if( MatchThisOrientation( osurf ) )
    {
        return true;
    }

    osurf = otherSurf->m_Surface;
    osurf.reverse_v();
    if( MatchThisOrientation( osurf ) )
    {
        return true;
    }

    osurf = otherSurf->m_Surface;
    osurf.reverse_u();
    osurf.reverse_v();
    if( MatchThisOrientation( osurf ) )
    {
        return true;
    }

    swaposurf = otherSurf->m_Surface;
    swaposurf.swap_uv();

    osurf = swaposurf;
    if( MatchThisOrientation( osurf ) )
    {
        return true;
    }

    osurf = swaposurf;
    osurf.reverse_u();
    if( MatchThisOrientation( osurf ) )
    {
        return true;
    }

    osurf = swaposurf;
    osurf.reverse_v();
    if( MatchThisOrientation( osurf ) )
    {
        return true;
    }

    osurf = swaposurf;
    osurf.reverse_u();
    osurf.reverse_v();

    return MatchThisOrientation( osurf );
}

bool SurfCore::MatchThisOrientation( const piecewise_surface_type &osurf ) const
{
    int ip, jp, nupatch, nvpatch, onupatch, onvpatch;

    double tol = 1.0e-8;

    nupatch = m_Surface.number_u_patches();
    onupatch = osurf.number_u_patches();
    if ( nupatch != onupatch )
    {
        return false;
    }

    nvpatch = m_Surface.number_v_patches();
    onvpatch = osurf.number_v_patches();
    if ( nvpatch != onvpatch )
    {
        return false;
    }

    for ( ip = 0; ip < nupatch; ip++ )
    {
        const surface_patch_type *patch = m_Surface.get_patch( ip, 0 );
        const surface_patch_type *opatch = osurf.get_patch( ip, 0 );

        if ( patch->degree_u() != opatch->degree_u() )
        {
            return false;
        }
    }

    for ( jp = 0; jp < nvpatch; jp++ )
    {
        const surface_patch_type *patch = m_Surface.get_patch( 0, jp );
        const surface_patch_type *opatch = osurf.get_patch( 0, jp );

        if ( patch->degree_v() != opatch->degree_v() )
        {
            return false;
        }
    }

    for( ip = 0; ip < nupatch; ++ip )
    {
        for( jp = 0; jp < nvpatch; ++jp )
        {
            surface_patch_type::index_type icp, jcp;
            const surface_patch_type *patch = m_Surface.get_patch( ip, jp );
            const surface_patch_type *opatch = osurf.get_patch( 0, jp );

            for( icp = 0; icp <= patch->degree_u(); ++icp )
            {
                for( jcp = 0; jcp <= patch->degree_v(); ++jcp )
                {
                    vec3d cp, ocp;
                    cp = patch->get_control_point( icp, jcp );
                    ocp = opatch->get_control_point( icp, jcp );

                    if ( dist_squared( cp, ocp ) > tol )
                    {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

void SurfCore::WriteSurf( FILE* fp ) const
{
    vector< vector< vec3d > > pntVec;
    unsigned int numU, numW;

    int ip, jp, nupatch, nwpatch, ioffset, joffset;
    int icp, jcp, udeg = 0, wdeg = 0;

    nupatch = m_Surface.number_u_patches();
    nwpatch = m_Surface.number_v_patches();

    fprintf( fp, "%d        // Num U Patches, patch order:\n", nupatch );
    numU = 1;
    for ( ip = 0; ip < nupatch; ip++ )
    {
        const surface_patch_type *patch = m_Surface.get_patch( ip, 0 );
        udeg = patch->degree_u();

        numU += udeg;

        fprintf( fp, "%d ", udeg );
        if ( ip == nupatch - 1 )
            fprintf(fp, "\n" );
        else
            fprintf(fp, ", " );
    }

    fprintf( fp, "%d        // Num W Patches, patch order:\n", nwpatch );
    numW = 1;
    for ( jp = 0; jp < nwpatch; jp++ )
    {
        const surface_patch_type *patch = m_Surface.get_patch( 0, jp );
        wdeg = patch->degree_v();

        numW += wdeg;

        fprintf( fp, "%d ", wdeg );
        if ( jp == nwpatch - 1 )
            fprintf(fp, "\n" );
        else
            fprintf(fp, ", " );
    }

    pntVec.resize( numU );
    for ( icp = 0; icp < numU; icp++ )
    {
        pntVec[icp].resize( numW );
    }

    ioffset = 0;
    for( ip = 0; ip < nupatch; ++ip )
    {
        joffset = 0;
        for( jp = 0; jp < nwpatch; ++jp )
        {
            const surface_patch_type *patch = m_Surface.get_patch( ip, jp );
            udeg = patch->degree_u();
            wdeg = patch->degree_v();

            for( icp = 0; icp <= udeg; ++icp )
            {
                for( jcp = 0; jcp <= wdeg; ++jcp )
                {
                    surface_point_type cp;
                    cp = patch->get_control_point( icp, jcp );
                    pntVec[ ioffset + icp ][ joffset + jcp ]  = vec3d( cp.x(), cp.y(), cp.z() );
                }
            }
            joffset += wdeg;
        }
        ioffset += udeg;
    }

    fprintf( fp, "%u        // Num Pnts U \n",    numU );
    fprintf( fp, "%u        // Num Pnts W \n",    numW );
    fprintf( fp, "%u        // Total Pnts (0,0),(0,1),(0,2)..(0,numW-1),(1,0)..(1,numW-1)..(numU-1,0)..(numU-1,numW-1)\n",    numU * numW );

    for ( int i = 0 ; i < numU ; i++ )
    {
        for ( int j = 0 ; j < numW ; j++ )
        {
            fprintf( fp, "%20.20lf, %20.20lf, %20.20lf \n",
                     pntVec[i][j].x(), pntVec[i][j].y(), pntVec[i][j].z() );
        }
    }
}

void SurfCore::MakeWakeSurf( const piecewise_curve_type& lecrv, double endx, double angle, double start_stretch_x, double scale )
{
    Bezier_curve tecrv;
    tecrv.BuildWakeTECurve( lecrv, endx, angle, start_stretch_x, scale );

    piecewise_curve_type le = lecrv;
    piecewise_curve_type te = tecrv.GetCurve();

    int nuseg = le.number_segments();

    m_Surface.init_uv( nuseg, 1 );

    for ( int i = 0; i < nuseg; i++ )
    {
        curve_segment_type cle, cte;
        le.get( cle, i );
        te.get( cte, i );

        surface_patch_type patch;
        patch.resize( cle.degree(), 1 );

        for ( int j = 0; j <= cle.degree(); j++ )
        {
            patch.set_control_point( cle.get_control_point( j ), j, 0 );
            patch.set_control_point( cte.get_control_point( j ), j, 1 );
        }
        m_Surface.set( patch, i, 0 );
    }

    surface_point_type p = m_Surface.normal( 0.0, 0.0 );
    if ( p.z() < 0.0 )
    {
        m_Surface.reverse_u();
    }
}

void SurfCore::MakePlaneSurf( const threed_point_type &p0, const threed_point_type &p1, const threed_point_type &p2, const threed_point_type &p3 )
{
    surface_patch_type patch;
    patch.resize( 1, 1 );

    patch.set_control_point( p0, 0, 0 );
    patch.set_control_point( p1, 1, 0 );
    patch.set_control_point( p2, 0, 1 );
    patch.set_control_point( p3, 1, 1 );

    m_Surface.init_uv( 1, 1 );
    m_Surface.set( patch, 0, 0 );
}

void SurfCore::BuildPatches( Surf* srf ) const
{
    vector< SurfPatch* > patchVec = srf->GetPatchVec();

    for ( int i = 0 ; i < ( int )patchVec.size() ; i++ )
    {
        delete patchVec[i];
    }
    patchVec.clear();

    for ( int ip = 0; ip < m_Surface.number_u_patches(); ip++ )
    {
        for ( int jp = 0; jp < m_Surface.number_v_patches(); jp++ )
        {
            double umin = 0;
            double du = 0;
            double vmin = 0;
            double dv = 0;
            const surface_patch_type *epatch = m_Surface.get_patch( ip, jp, umin, du, vmin, dv );

            SurfPatch* patch = new SurfPatch();

            patch->setPatch( *epatch );

            patch->set_u_min_max( umin, umin + du );
            patch->set_w_min_max( vmin, vmin + dv );

            patch->set_surf_ptr( srf );
            patch->compute_bnd_box();

            patchVec.push_back( patch );
        }
    }

    surface_bounding_box_type bbox;
    m_Surface.get_bounding_box( bbox );
    srf->SetBBox( bbox.get_max(), bbox.get_min() );
    srf->SetPatchVec( patchVec );
}

double SurfCore::FindNearest( double &u, double &w, const vec3d &pt, double u0, double w0 ) const
{
    double dist;
    surface_point_type p;
    pt.get_pnt( p );

    double umn = m_Surface.get_u0();
    double wmn = m_Surface.get_v0();

    double umx = m_Surface.get_umax();
    double wmx = m_Surface.get_vmax();

    double slop = 1e-3;
    if( u0 < (umn - slop) || w0 < (wmn - slop) || u0 > (umx + slop) || w0 > (wmx + slop) )
    {
        printf("BAD parameter in SurfCore::FindNearest! %f %f\n", u0, w0 );
        assert(false);
    }

    if ( u0 < umn )
        u0 = umn;

    if ( w0 < wmn )
        w0 = wmn;

    if ( u0 > umx )
        u0 = umx;

    if ( w0 > wmx )
        w0 = wmx;

    dist = eli::geom::intersect::minimum_distance( u, w, m_Surface, p, u0, w0 );

    return dist;
}

double SurfCore::FindNearest( double &u, double &w, const vec3d &pt, double u0, double w0, double umn, double umx, double wmn, double wmx ) const
{
    double dist;
    surface_point_type p;
    pt.get_pnt( p );

    double slop = 1e-3;
    if( u0 < (umn - slop) || w0 < (wmn - slop) || u0 > (umx + slop) || w0 > (wmx + slop) )
    {
        printf("BAD parameter in SurfCore::FindNearest! %f %f\n", u0, w0 );
        assert(false);
    }

    if ( u0 < umn )
        u0 = umn;

    if ( w0 < wmn )
        w0 = wmn;

    if ( u0 > umx )
        u0 = umx;

    if ( w0 > wmx )
        w0 = wmx;

    dist = eli::geom::intersect::minimum_distance( u, w, m_Surface, p, u0, w0 );

    return dist;
}

double SurfCore::FindNearest( double &u, double &w, const vec3d &pt ) const
{
    double dist;
    surface_point_type p;
    pt.get_pnt( p );

    dist = eli::geom::intersect::minimum_distance( u, w, m_Surface, p );

    return dist;
}