//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SurfPatch
//
//////////////////////////////////////////////////////////////////////

#include "SurfPatch.h"
#include "Surf.h"

//////////////////////////////////////////////////////////////////////

SurfPatch::SurfPatch()
{
    u_min = w_min = 0.0;
    u_max = w_max = 1.0;
    m_SurfPtr = NULL;
    sub_depth = 0;
    draw_flag = false;
}

SurfPatch::~SurfPatch()
{
}

//===== Compute Blending Functions  =====//
void SurfPatch::blend_funcs( double u, double& F1, double& F2, double& F3, double& F4 )
{
    double uu = u * u;
    double one_u = 1.0 - u;
    double one_u_sqr = one_u * one_u;

    F1 = one_u * one_u_sqr;
    F2 = 3.0 * u * one_u_sqr;
    F3 = 3.0 * uu * one_u;
    F4 = uu * u;
}

//==== Compute Bounding Box ====//
void SurfPatch::compute_bnd_box()
{
    bnd_box.Reset();
    for ( int iu = 0 ; iu < 4 ; iu++ )
    {
        for ( int iw = 0 ; iw < 4 ; iw++ )
        {
            bnd_box.Update( pnts[iu][iw] );
        }
    }

}

//===== Compute Point On Patch Given  U and W =====//
vec3d SurfPatch::comp_pnt( double u, double w )
{
    double F1u, F2u, F3u, F4u;
    double F1w, F2w, F3w, F4w;
    if ( u <= u_min )
    {
        F1u = 1.0;
        F2u = F3u = F4u = 0.0;
    }
    else if ( u >= u_max )
    {
        F1u = F2u = F3u = 0.0;
        F4u = 1.0;
    }
    else
    {
        blend_funcs( ( u - u_min ) / ( u_max - u_min ), F1u, F2u, F3u, F4u );
    }

    if ( w <= w_min )
    {
        F1w = 1.0;
        F2w = F3w = F4w = 0.0;
    }
    else if ( w >= w_max )
    {
        F1w = F2w = F3w = 0.0;
        F4w = 1.0;
    }
    else
    {
        blend_funcs( ( w - w_min ) / ( w_max - w_min ), F1w, F2w, F3w, F4w );
    }

    vec3d new_pnt =
        ( ( pnts[0][0] * F1u + pnts[1][0] * F2u + pnts[2][0] * F3u + pnts[3][0] * F4u ) * F1w ) +
        ( ( pnts[0][1] * F1u + pnts[1][1] * F2u + pnts[2][1] * F3u + pnts[3][1] * F4u ) * F2w ) +
        ( ( pnts[0][2] * F1u + pnts[1][2] * F2u + pnts[2][2] * F3u + pnts[3][2] * F4u ) * F3w ) +
        ( ( pnts[0][3] * F1u + pnts[1][3] * F2u + pnts[2][3] * F3u + pnts[3][3] * F4u ) * F4w );

    return( new_pnt );
}

//===== Split Patch =====//
void SurfPatch::split_patch( SurfPatch& bp00, SurfPatch& bp10, SurfPatch& bp01, SurfPatch& bp11 )
{
    int iu, iw;
    vec3d tmp_hull[4][7];
    vec3d tmp0, tmp1;

    // === Hold u Constant and Insert in W Direction ===//
    for ( iu = 0 ; iu < 4 ; iu++ )
    {
        tmp_hull[iu][0] = pnts[iu][0];
        tmp_hull[iu][1] = ( pnts[iu][0] + pnts[iu][1] ) * 0.5;
        tmp0 = ( pnts[iu][1] + pnts[iu][2] ) * 0.5;
        tmp1 = ( pnts[iu][2] + pnts[iu][3] ) * 0.5;

        tmp_hull[iu][2] = ( tmp_hull[iu][1] + tmp0 ) * 0.5;
        tmp0 = ( tmp0 + tmp1 ) * 0.5;

        tmp_hull[iu][3] = ( tmp_hull[iu][2] + tmp0 ) * 0.5;
        tmp_hull[iu][4] = tmp0;
        tmp_hull[iu][5] = tmp1;
        tmp_hull[iu][6] = pnts[iu][3];
    }

    // === Hold W Constant and Insert in U Direction ===//
    for (  iw = 0 ; iw < 4 ; iw++ )
    {
        bp00.pnts[0][iw] =  tmp_hull[0][iw];
        bp00.pnts[1][iw] = ( tmp_hull[0][iw] + tmp_hull[1][iw] ) * 0.5;
        tmp0 = ( tmp_hull[1][iw] + tmp_hull[2][iw] ) * 0.5;
        tmp1 = ( tmp_hull[2][iw] + tmp_hull[3][iw] ) * 0.5;

        bp00.pnts[2][iw] = ( bp00.pnts[1][iw] + tmp0 ) * 0.5;
        tmp0 = ( tmp0 + tmp1 ) * 0.5;

        bp00.pnts[3][iw] = ( bp00.pnts[2][iw] + tmp0 ) * 0.5;
        bp10.pnts[0][iw] = bp00.pnts[3][iw];
        bp10.pnts[1][iw] = tmp0;
        bp10.pnts[2][iw] = tmp1;
        bp10.pnts[3][iw] = tmp_hull[3][iw];
    }

    for ( iw = 3 ; iw < 7 ; iw++ )
    {
        bp01.pnts[0][iw - 3] =  tmp_hull[0][iw];
        bp01.pnts[1][iw - 3] = ( tmp_hull[0][iw] + tmp_hull[1][iw] ) * 0.5;
        tmp0 = ( tmp_hull[1][iw] + tmp_hull[2][iw] ) * 0.5;
        tmp1 = ( tmp_hull[2][iw] + tmp_hull[3][iw] ) * 0.5;

        bp01.pnts[2][iw - 3] = ( bp01.pnts[1][iw - 3] + tmp0 ) * 0.5;
        tmp0 = ( tmp0 + tmp1 ) * 0.5;

        bp01.pnts[3][iw - 3] = ( bp01.pnts[2][iw - 3] + tmp0 ) * 0.5;
        bp11.pnts[0][iw - 3] = bp01.pnts[3][iw - 3];
        bp11.pnts[1][iw - 3] = tmp0;
        bp11.pnts[2][iw - 3] = tmp1;
        bp11.pnts[3][iw - 3] = tmp_hull[3][iw];
    }

    bp00.u_min = u_min;
    bp00.w_min = w_min;
    bp00.u_max = 0.5 * ( u_min + u_max );
    bp00.w_max = 0.5 * ( w_min + w_max );

    bp10.u_min = bp00.u_max;
    bp10.w_min = w_min;
    bp10.u_max = u_max;
    bp10.w_max = bp00.w_max;

    bp01.u_min = u_min;
    bp01.w_min = bp00.w_max;
    bp01.u_max = bp00.u_max;
    bp01.w_max = w_max;

    bp11.u_min = bp00.u_max;
    bp11.w_min = bp00.w_max;
    bp11.u_max = u_max;
    bp11.w_max = w_max;

    bp00.compute_bnd_box();
    bp10.compute_bnd_box();
    bp01.compute_bnd_box();
    bp11.compute_bnd_box();

    bp00.set_surf_ptr( m_SurfPtr );
    bp10.set_surf_ptr( m_SurfPtr );
    bp01.set_surf_ptr( m_SurfPtr );
    bp11.set_surf_ptr( m_SurfPtr );

}

//===== Test If Patch Is Planar (within tol)  =====//
bool SurfPatch::test_planar( double tol )
{
    vec3d org = pnts[0][0];
    vec3d v1 = pnts[3][0] - pnts[0][0];
    vec3d v2 = pnts[0][3] - pnts[0][0];

    vec3d norm = cross( v1, v2 );

    norm.normalize();

    if ( dist_pnt_2_plane( org, norm, pnts[3][3] ) > tol )
    {
        return false;
    }
    if ( dist_pnt_2_plane( org, norm, pnts[1][1] ) > tol )
    {
        return false;
    }
    if ( dist_pnt_2_plane( org, norm, pnts[2][1] ) > tol )
    {
        return false;
    }
    if ( dist_pnt_2_plane( org, norm, pnts[1][2] ) > tol )
    {
        return false;
    }
    if ( dist_pnt_2_plane( org, norm, pnts[2][2] ) > tol )
    {
        return false;
    }

    if ( dist_pnt_2_line( pnts[0][0], pnts[3][0], pnts[1][0] ) > tol )
    {
        return false;
    }
    if ( dist_pnt_2_line( pnts[0][0], pnts[3][0], pnts[2][0] ) > tol )
    {
        return false;
    }
    if ( dist_pnt_2_line( pnts[0][0], pnts[0][3], pnts[0][1] ) > tol )
    {
        return false;
    }
    if ( dist_pnt_2_line( pnts[0][0], pnts[0][3], pnts[0][2] ) > tol )
    {
        return false;
    }
    if ( dist_pnt_2_line( pnts[3][3], pnts[0][3], pnts[1][3] ) > tol )
    {
        return false;
    }
    if ( dist_pnt_2_line( pnts[3][3], pnts[0][3], pnts[2][3] ) > tol )
    {
        return false;
    }
    if ( dist_pnt_2_line( pnts[3][3], pnts[3][0], pnts[3][1] ) > tol )
    {
        return false;
    }
    if ( dist_pnt_2_line( pnts[3][3], pnts[3][0], pnts[3][2] ) > tol )
    {
        return false;
    }

    return true;
}

//===== Find Closest UW On Patch to Given Point  =====//
void SurfPatch::find_closest_uw( vec3d& pnt_in, double guess_uw[2], double uw[2] )
{
    int Max_Iter  = 10;
    double UW_Tol = 1.0e-14;
    double norm_uw[2];
    double del_uw[2];
    vec3d guess_pnt;

    //==== Normalize uw Values Between 0 and 1 ====//
    norm_uw[0] = ( guess_uw[0] - u_min ) / ( u_max - u_min );
    norm_uw[1] = ( guess_uw[1] - w_min ) / ( w_max - w_min );

    //===== Loop Until U and W Stops Changing or Max Iterations is Hit  =====//
    int cnt = 0;
    int stop_flag = false;
    while ( !stop_flag )
    {
        guess_pnt = comp_pnt_01( norm_uw[0], norm_uw[1] );

        //==== Find Delta U and W Values ====//
        comp_delta_uw( pnt_in, guess_pnt, norm_uw, del_uw );

        norm_uw[0] += del_uw[0];
        norm_uw[1] -= del_uw[1];

        //==== Test To Stop Iteration ====//
        if ( ( fabs( del_uw[0] ) + fabs( del_uw[1] ) ) < UW_Tol )
        {
            stop_flag = true;
        }
        else if ( cnt > Max_Iter )
        {
            stop_flag = true;
        }
        else
        {
            cnt++;
        }
    }

    //==== Convert uw Values Back to Original Space ====//
    uw[0] = norm_uw[0] * ( u_max - u_min ) + u_min;
    uw[1] = norm_uw[1] * ( w_max - w_min ) + w_min;

    //==== Clamp Values At Bounds  ====//
    if      ( uw[0] < u_min )
    {
        uw[0] = u_min;
    }
    else if ( uw[0] > u_max )
    {
        uw[0] = u_max;
    }

    if      ( uw[1] < w_min )
    {
        uw[1] = w_min;
    }
    else if ( uw[1] > w_max )
    {
        uw[1] = w_max;
    }
}

//===== Find Closest UW On Patch to Given Point  =====//
void SurfPatch::find_closest_uw( vec3d& pnt_in, double uw[2] )
{
    vec2d guess_uw( 0.5 * ( u_max + u_min ), 0.5 * ( w_max + w_min ) );

    find_closest_uw( pnt_in, guess_uw.v, uw );
}

//===== Compute Point On Patch  =====//
vec3d SurfPatch::comp_pnt_01( double u, double w )
{
    double uu    = u * u;
    double one_u = 1.0 - u;
    double F1u   = one_u * one_u * one_u;
    double F2u   = 3.0 * one_u * one_u * u;
    double F3u   = 3.0 * one_u * uu;
    double F4u   = uu * u;

    double ww    = w * w;
    double one_w = 1.0 - w;
    double F1w   = one_w * one_w * one_w;
    double F2w   = 3.0 * one_w * one_w * w;
    double F3w   = 3.0 * one_w * ww;
    double F4w   = ww * w;

    //==== Compute Point On Surface ====//
    vec3d new_pnt =
        ( ( pnts[0][0] * F1u + pnts[1][0] * F2u + pnts[2][0] * F3u + pnts[3][0] * F4u ) * F1w ) +
        ( ( pnts[0][1] * F1u + pnts[1][1] * F2u + pnts[2][1] * F3u + pnts[3][1] * F4u ) * F2w ) +
        ( ( pnts[0][2] * F1u + pnts[1][2] * F2u + pnts[2][2] * F3u + pnts[3][2] * F4u ) * F3w ) +
        ( ( pnts[0][3] * F1u + pnts[1][3] * F2u + pnts[2][3] * F3u + pnts[3][3] * F4u ) * F4w );

    return( new_pnt );
}

//===== Compute Tangent In U Direction   =====//
vec3d SurfPatch::comp_tan_u_01( double u, double w )
{
    double uu    = u * u;
    double one_u = 1.0 - u;
    double F1u   = -3.0 * one_u * one_u;
    double F2u   = 3.0 - 12.0 * u + 9.0 * uu;
    double F3u   = 6.0 * u - 9.0 * uu;
    double F4u   = 3.0 * uu;

    double ww    = w * w;
    double one_w = 1.0 - w;
    double F1w   = one_w * one_w * one_w;
    double F2w   = 3.0 * one_w * one_w * w;
    double F3w   = 3.0 * one_w * ww;
    double F4w   = ww * w;

    //==== Compute Tangent In U Direction ====//
    vec3d new_pnt =
        ( ( pnts[0][0] * F1u + pnts[1][0] * F2u + pnts[2][0] * F3u + pnts[3][0] * F4u ) * F1w ) +
        ( ( pnts[0][1] * F1u + pnts[1][1] * F2u + pnts[2][1] * F3u + pnts[3][1] * F4u ) * F2w ) +
        ( ( pnts[0][2] * F1u + pnts[1][2] * F2u + pnts[2][2] * F3u + pnts[3][2] * F4u ) * F3w ) +
        ( ( pnts[0][3] * F1u + pnts[1][3] * F2u + pnts[2][3] * F3u + pnts[3][3] * F4u ) * F4w );

    return( new_pnt );

}

//===== Compute Tangent In W Direction   =====//
vec3d SurfPatch::comp_tan_w_01( double u, double w )
{
    double uu    = u * u;
    double one_u = 1.0 - u;
    double F1u   = one_u * one_u * one_u;
    double F2u   = 3.0 * one_u * one_u * u;
    double F3u   = 3.0 * one_u * uu;
    double F4u   = uu * u;

    double ww    = w * w;
    double one_w = 1.0 - w;
    double F1w   = -3.0 * one_w * one_w;
    double F2w   = 3.0 - 12.0 * w + 9.0 * ww;
    double F3w   = 6.0 * w - 9.0 * ww;
    double F4w   = 3.0 * ww;

    //==== Compute Point On Surface ====//
    vec3d new_pnt =
        ( ( pnts[0][0] * F1u + pnts[1][0] * F2u + pnts[2][0] * F3u + pnts[3][0] * F4u ) * F1w ) +
        ( ( pnts[0][1] * F1u + pnts[1][1] * F2u + pnts[2][1] * F3u + pnts[3][1] * F4u ) * F2w ) +
        ( ( pnts[0][2] * F1u + pnts[1][2] * F2u + pnts[2][2] * F3u + pnts[3][2] * F4u ) * F3w ) +
        ( ( pnts[0][3] * F1u + pnts[1][3] * F2u + pnts[2][3] * F3u + pnts[3][3] * F4u ) * F4w );

    return( new_pnt );

}

//===== Compute Point On Patch  =====//
void SurfPatch::comp_delta_uw( vec3d& pnt_in, vec3d& guess_pnt, double norm_uw[2], double delta_uw[2] )
{
    vec3d tan_u = comp_tan_u_01( norm_uw[0], norm_uw[1] );
    vec3d tan_w = comp_tan_w_01( norm_uw[0], norm_uw[1] );

    vec3d dist_vec = guess_pnt - pnt_in;

//static double max_mag = 0.0;
//if ( dist_vec.mag() > max_mag ) max_mag = dist_vec.mag();
//printf( "DistVec Mag = %f Mx = %f\n", dist_vec.mag(), max_mag );

    vec3d A = cross( tan_w, dist_vec );
    vec3d B = cross( tan_u, dist_vec );
    vec3d norm = cross( tan_u, tan_w );

    double N = dot( norm, norm );

    if( fabs( N ) > DBL_EPSILON )
    {
        delta_uw[0] = dot( A, norm ) / N;
        delta_uw[1] = dot( B, norm ) / N;
    }
    else
    {
        delta_uw[0] = 0.0;
        delta_uw[1] = 0.0;
    }
}

void SurfPatch::IntersectLineSeg( vec3d & p0, vec3d & p1, BndBox & line_box, vector< double > & t_vals )
{
    if ( !Compare( line_box, bnd_box ) )
    {
        return;
    }

    //==== Do Tri Seg intersection ====//
    if ( test_planar( DEFAULT_PLANE_TOL ) )
    {
        double r, s, t;
        vec3d OA1 = pnts[0][0];
        vec3d A1 = pnts[3][3] - pnts[0][0];
        vec3d B1 = pnts[3][0] - pnts[0][0];
        vec3d C1 = pnts[0][3] - pnts[0][0];
        vec3d p1p0 = p1 - p0;

        if ( tri_seg_intersect( OA1, A1, B1, p0, p1p0, r, s, t ) )
        {
            AddTVal( t, t_vals );
        }
        if ( tri_seg_intersect( OA1, C1, A1, p0, p1p0, r, s, t ) )
        {
            AddTVal( t, t_vals );
        }
        return;
    }

    SurfPatch bps[4];
    split_patch( bps[0], bps[1], bps[2], bps[3] );      // Split Patch2 and Keep Subdividing

    bps[0].IntersectLineSeg( p0, p1, line_box, t_vals );
    bps[1].IntersectLineSeg( p0, p1, line_box, t_vals );
    bps[2].IntersectLineSeg( p0, p1, line_box, t_vals );
    bps[3].IntersectLineSeg( p0, p1, line_box, t_vals );
}

void SurfPatch::AddTVal( double t, vector< double > & t_vals )
{
    bool add_flag = true;
    for ( int i = 0 ; i < ( int )t_vals.size() ; i++ )
    {
        if ( fabs( t - t_vals[i] ) < 1.0e-6 )
        {
            add_flag = false;
        }
    }

    if ( add_flag )
    {
        t_vals.push_back( t );
    }
}

/*
void SurfPatch::Draw()
{
//if ( !draw_flag )
//  return;

    glLineWidth( 1.0 );
    glColor3ub( 0, 255, 0 );

    glBegin( GL_LINE_LOOP );
        glVertex3dv( pnts[0][0].data() );
        glVertex3dv( pnts[3][0].data() );
        glVertex3dv( pnts[3][3].data() );
        glVertex3dv( pnts[0][3].data() );
    glEnd();


    //glBegin( GL_LINE_LOOP );
    //  glVertex3dv( bnd_box.get_pnt(0).data() );
    //  glVertex3dv( bnd_box.get_pnt(1).data() );
    //  glVertex3dv( bnd_box.get_pnt(3).data() );
    //  glVertex3dv( bnd_box.get_pnt(2).data() );
    //glEnd();

    //glBegin( GL_LINE_LOOP );
    //  glVertex3dv( bnd_box.get_pnt(4).data() );
    //  glVertex3dv( bnd_box.get_pnt(5).data() );
    //  glVertex3dv( bnd_box.get_pnt(7).data() );
    //  glVertex3dv( bnd_box.get_pnt(6).data() );
    //glEnd();

    //glBegin( GL_LINES );
    //  glVertex3dv( bnd_box.get_pnt(0).data() );
    //  glVertex3dv( bnd_box.get_pnt(4).data() );
    //  glVertex3dv( bnd_box.get_pnt(1).data() );
    //  glVertex3dv( bnd_box.get_pnt(5).data() );
    //  glVertex3dv( bnd_box.get_pnt(3).data() );
    //  glVertex3dv( bnd_box.get_pnt(7).data() );
    //  glVertex3dv( bnd_box.get_pnt(2).data() );
    //  glVertex3dv( bnd_box.get_pnt(6).data() );
    //glEnd();

}
*/


