//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SurfPatch
//
//////////////////////////////////////////////////////////////////////

#include "SurfPatch.h"
#include "Surf.h"

#include "eli/geom/intersect/minimum_distance_surface.hpp"
typedef piecewise_surface_type::bounding_box_type surface_bounding_box_type;

//////////////////////////////////////////////////////////////////////

SurfPatch::SurfPatch()
{
    u_min = w_min = 0.0;
    u_max = w_max = 1.0;
    m_SurfPtr = NULL;
    sub_depth = 0;

    m_wasplanar = false;
    m_lastreltol = 12345.678;
}

SurfPatch::SurfPatch( int n, int m, int d ) : m_Patch( n, m )
{
    u_min = w_min = 0.0;
    u_max = w_max = 1.0;
    m_SurfPtr = NULL;
    sub_depth = d;

    m_wasplanar = false;
    m_lastreltol = 12345.678;
}

SurfPatch::~SurfPatch()
{
}

//==== Compute Bounding Box ====//
void SurfPatch::compute_bnd_box()
{
    surface_bounding_box_type bbx;

    m_Patch.get_bounding_box( bbx );

    vec3d v3min( bbx.get_min() );
    vec3d v3max( bbx.get_max() );

    bnd_box.Reset();
    bnd_box.Update( v3min );
    bnd_box.Update( v3max );
}

//===== Split Patch =====//
void SurfPatch::split_patch( SurfPatch& bp00, SurfPatch& bp10, SurfPatch& bp01, SurfPatch& bp11 ) const
{

    int n = m_Patch.degree_u();
    int m = m_Patch.degree_v();
    surface_patch_type pvlow( n, m );
    surface_patch_type pvhi( n, m );
    m_Patch.simple_split_v_half( pvlow, pvhi );

    surface_patch_type pulow, puhi;
    pvlow.simple_split_u_half( *(bp00.getPatch()), *(bp10.getPatch()) );

    pvhi.simple_split_u_half( *(bp01.getPatch()), *(bp11.getPatch()) );

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
bool SurfPatch::test_planar( double tol ) const
{
    return test_planar_rel( tol / bnd_box.DiagDist() );
}

//===== Test If Patch Is Planar (within relative tol)  =====//
bool SurfPatch::test_planar_rel( double reltol ) const
{
    if ( reltol == m_lastreltol )
    {
        return m_wasplanar;
    }

    surface_patch_type approx = m_Patch;
    approx.planar_approx();

    double dst = m_Patch.simple_eqp_distance_bound( approx );

    // These variables are mutable -- to allow this to still be a const method.
    // Set m_lastreltol after setting m_wasplanar as a defense against any future race
    // condition.
    m_wasplanar = dst < ( reltol * bnd_box.DiagDist() );
    m_lastreltol = reltol;

    return m_wasplanar;
}

//===== Find Closest UW On Patch to Given Point  =====//
void SurfPatch::find_closest_uw( const vec3d& pnt_in, double uw[2] ) const
{
    vec2d guess_uw( 0.5, 0.5 );

    surface_point_type p;
    p << pnt_in.x(), pnt_in.y(), pnt_in.z();

    double u, w;
    eli::geom::intersect::minimum_distance( u, w, m_Patch, p, guess_uw.x(), guess_uw.y() );

    uw[0] = u_min + u * ( u_max - u_min );
    uw[1] = w_min + w * ( w_max - w_min );
}

//===== Compute Point On Patch  =====//
vec3d SurfPatch::comp_pnt_01( double u, double w ) const
{
    surface_point_type p = m_Patch.f( u, w );
    vec3d new_pnt( p.x(), p.y(), p.z() );
    return new_pnt;
}

//===== Compute Tangent In U Direction   =====//
vec3d SurfPatch::comp_tan_u_01( double u, double w ) const
{
    surface_point_type p = m_Patch.f_u( u, w );
    vec3d new_pnt( p.x(), p.y(), p.z() );
    return new_pnt;
}

//===== Compute Tangent In W Direction   =====//
vec3d SurfPatch::comp_tan_w_01( double u, double w ) const
{
    surface_point_type p = m_Patch.f_v( u, w );
    vec3d new_pnt( p.x(), p.y(), p.z() );
    return new_pnt;
}

void SurfPatch::IntersectLineSeg( vec3d & p0, vec3d & p1, BndBox & line_box, vector< double > & t_vals ) const
{
    if ( !Compare( line_box, bnd_box ) )
    {
        return;
    }

    //==== Do Tri Seg intersection ====//
    if ( test_planar( 1.0e-5 ) )  // Uses a dimensional tolerance in test.
    {
        int n( m_Patch.degree_u() ), m( m_Patch.degree_v() );

        double r, s, t;
        vec3d OA1 = m_Patch.get_control_point( 0, 0 );
        vec3d A1 = m_Patch.get_control_point( n, m ) - OA1;
        vec3d B1 = m_Patch.get_control_point( n, 0 ) - OA1;
        vec3d C1 = m_Patch.get_control_point( 0, m ) - OA1;

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

    int n = degree_u();
    int m = degree_v();
    int d = GetSubDepth() + 1;

    SurfPatch bps0( n, m, d );
    SurfPatch bps1( n, m, d );
    SurfPatch bps2( n, m, d );
    SurfPatch bps3( n, m, d );

    split_patch( bps0, bps1, bps2, bps3 );

    bps0.IntersectLineSeg( p0, p1, line_box, t_vals );
    bps1.IntersectLineSeg( p0, p1, line_box, t_vals );
    bps2.IntersectLineSeg( p0, p1, line_box, t_vals );
    bps3.IntersectLineSeg( p0, p1, line_box, t_vals );
}

void SurfPatch::AddTVal( double t, vector< double > & t_vals ) const
{
    bool add_flag = true;
    for ( int i = 0 ; i < ( int )t_vals.size() ; i++ )
    {
        if ( std::abs( t - t_vals[i] ) < 1.0e-6 )
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


