//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// IntersectQuads
//
//////////////////////////////////////////////////////////////////////

#include "IntersectPatch.h"
#include "Surf.h"
#include "CfdMeshMgr.h"
#include "tri_tri_intersect.h"

#include "eli/geom/intersect/intersect_surface.hpp"

void intersect( const SurfPatch& bp1, const SurfPatch& bp2, SurfaceIntersectionSingleton *MeshMgr )
{
    int MAX_SUB = 12;
    int MIN_SUB = 3;

    if ( !Compare( *bp1.get_bbox(), *bp2.get_bbox() ) )
    {
        return;
    }

    // This test uses a relative tolerance.
    const double tol = 1.0e-3;

    bool planar1 = false;
    bool planar2 = false;

    // Don't even test planarity until MIN_SUB subdivisions
    if ( bp1.GetSubDepth() >= MIN_SUB )
    {
        planar1 = bp1.test_planar_rel( tol );
    }
    if ( bp2.GetSubDepth() >= MIN_SUB )
    {
        planar2 = bp2.test_planar_rel( tol );
    }

    if ( ( planar1 || bp1.GetSubDepth() > MAX_SUB ) &&
         ( planar2 || bp2.GetSubDepth() > MAX_SUB ) )
    {
        intersect_quads( bp1, bp2, MeshMgr );          // Plane - Plane Intersection
    }
    else
    {
        bool ref1 = true; // Arbitrary initialization

        if ( planar1 == planar2 )  // Both non-planar (both planar would have tripped earlier condition)
        {
            ref1 = bp1.GetSubDepth() < bp2.GetSubDepth();
        }
        else // One planar, one non-planar.
        {
            ref1 = !planar1; // Refine non-planar surface.
        }

        if ( ref1 )
        {
            int n = bp1.degree_u();
            int m = bp1.degree_v();
            int d = bp1.GetSubDepth() + 1;

            SurfPatch bps0( n, m, d );
            SurfPatch bps1( n, m, d );
            SurfPatch bps2( n, m, d );
            SurfPatch bps3( n, m, d );

            bp1.split_patch( bps0, bps1, bps2, bps3 );      // Split Patch1 and Keep Subdividing

            intersect( bps0, bp2, MeshMgr );
            intersect( bps1, bp2, MeshMgr );
            intersect( bps2, bp2, MeshMgr );
            intersect( bps3, bp2, MeshMgr );
        }
        else
        {
            int n = bp2.degree_u();
            int m = bp2.degree_v();
            int d = bp2.GetSubDepth() + 1;

            SurfPatch bps0( n, m, d );
            SurfPatch bps1( n, m, d );
            SurfPatch bps2( n, m, d );
            SurfPatch bps3( n, m, d );

            bp2.split_patch( bps0, bps1, bps2, bps3 );      // Split Patch2 and Keep Subdividing

            intersect( bp1, bps0, MeshMgr );
            intersect( bp1, bps1, MeshMgr );
            intersect( bp1, bps2, MeshMgr );
            intersect( bp1, bps3, MeshMgr );
        }
    }
}

void intersect_quads( const SurfPatch& pa, const SurfPatch& pb, SurfaceIntersectionSingleton *MeshMgr )
{
    int iflag;
    int coplanar = 0; // Must be initialized to 0 before use in tri_tri_intersection_test_3d
    vec3d ip0, ip1;

    long an( pa.degree_u() ), am( pa.degree_v() );
    long bn( pb.degree_u() ), bm( pb.degree_v() );

    vec3d a0 = pa.m_Patch.get_control_point( 0, 0 );
    vec3d a1 = pa.m_Patch.get_control_point( an, 0 );
    vec3d a2 = pa.m_Patch.get_control_point( an, am );
    vec3d a3 = pa.m_Patch.get_control_point( 0, am );

    vec3d b0 = pb.m_Patch.get_control_point( 0, 0 );
    vec3d b1 = pb.m_Patch.get_control_point( bn, 0 );
    vec3d b2 = pb.m_Patch.get_control_point( bn, bm );
    vec3d b3 = pb.m_Patch.get_control_point( 0, bm );

    // Set very small values to 0. This is done to avoid + or - assignment to very small
    // numbers that have experienced floating point precision losses. While small, the sign
    // of these numbers can cause them to not be considered in the intersection algorithm. 
    // This issue mainly appears when the intersection point is on a patch edge and has an
    // X, Y, or Z coordinate of 0.
    double zero_tol = 1e-15;
    for ( size_t i = 0; i < 3; i++ )
    {
        if ( std::abs( a0[i] ) < zero_tol ) a0[i] = 0;
        if ( std::abs( a1[i] ) < zero_tol ) a1[i] = 0;
        if ( std::abs( a2[i] ) < zero_tol ) a2[i] = 0;
        if ( std::abs( a3[i] ) < zero_tol ) a3[i] = 0;
        if ( std::abs( b0[i] ) < zero_tol ) b0[i] = 0;
        if ( std::abs( b1[i] ) < zero_tol ) b1[i] = 0;
        if ( std::abs( b2[i] ) < zero_tol ) b2[i] = 0;
        if ( std::abs( b3[i] ) < zero_tol ) b3[i] = 0;
    }

    //==== Tri A1 and B1 ====//
    iflag = tri_tri_intersection_test_3d( a0.v, a2.v, a3.v, b0.v, b2.v, b3.v, &coplanar, ip0.v, ip1.v );
    if ( iflag && !coplanar )
    {
        MeshMgr->AddIntersectionSeg( pa, pb, ip0, ip1 );
    }

    //==== Tri A1 and B2 ====//
    coplanar = 0;
    iflag = tri_tri_intersection_test_3d( a0.v, a2.v, a3.v, b0.v, b1.v, b2.v, &coplanar, ip0.v, ip1.v );
    if ( iflag && !coplanar )
    {
        MeshMgr->AddIntersectionSeg( pa, pb, ip0, ip1 );
    }

    //==== Tri A2 and B1 ====//
    coplanar = 0;
    iflag = tri_tri_intersection_test_3d( a0.v, a1.v, a2.v, b0.v, b2.v, b3.v, &coplanar, ip0.v, ip1.v );
    if ( iflag && !coplanar )
    {
        MeshMgr->AddIntersectionSeg( pa, pb,  ip0, ip1 );
    }

    //==== Tri A2 and B2 ====//
    coplanar = 0;
    iflag = tri_tri_intersection_test_3d( a0.v, a1.v, a2.v, b0.v, b1.v, b2.v, &coplanar, ip0.v, ip1.v );
    if ( iflag && !coplanar )
    {
        MeshMgr->AddIntersectionSeg( pa, pb, ip0, ip1 );
    }
}

void refine_intersect_pt( const vec3d& pt, const SurfPatch &pA, double uwA[2], const SurfPatch &pB, double uwB[2] )
{

    if ( false )
    {
        vec2d guess_uw( 0.5, 0.5 );

        surface_point_type p;
        p << pt.x(), pt.y(), pt.z();

        double u01, w01;
        eli::geom::intersect::minimum_distance( u01, w01, pA.m_Patch, p, guess_uw.x(), guess_uw.y() );
        double u02, w02;
        eli::geom::intersect::minimum_distance( u02, w02, pB.m_Patch, p, guess_uw.x(), guess_uw.y() );

        double u1, w1, u2, w2, d;


        int ret = eli::geom::intersect::intersect( u1, w1, u2, w2, d, pA.m_Patch, pB.m_Patch, p, u01, w01, u02, w02 );

        uwA[0] = pA.u_min + u1 * ( pA.u_max - pA.u_min );
        uwA[1] = pA.w_min + w1 * ( pA.w_max - pA.w_min );

        uwB[0] = pB.u_min + u2 * ( pB.u_max - pB.u_min );
        uwB[1] = pB.w_min + w2 * ( pB.w_max - pB.w_min );

    }
    else
    {
        surface_point_type p;
        p << pt.x(), pt.y(), pt.z();

        vec2d uw0A = pA.m_SurfPtr->ClosestUW( pt, ( pA.u_max + pA.u_min ) * 0.5, ( pA.w_max + pA.w_min ) * 0.5 );
        vec2d uw0B = pB.m_SurfPtr->ClosestUW( pt, ( pB.u_max + pB.u_min ) * 0.5, ( pB.w_max + pB.w_min ) * 0.5 );


        double u1, w1, u2, w2, d;

        int ret = eli::geom::intersect::intersect( u1, w1, u2, w2, d, *( pA.m_SurfPtr->GetSurfCore()->GetSurf() ),
                                        *( pB.m_SurfPtr->GetSurfCore()->GetSurf() ),
                                        p, uw0A.x(), uw0A.y(), uw0B.x(), uw0B.y() );

        uwA[0] = u1;
        uwA[1] = w1;
        uwB[0] = u2;
        uwB[1] = w2;

    }
}

double refine_intersect_pt( const vec3d& pt, Surf *sA, vec2d &uwA, Surf *sB, vec2d &uwB )
{
    surface_point_type p;
    p << pt.x(), pt.y(), pt.z();

    double u1, w1, u2, w2, d;

    int ret = eli::geom::intersect::intersect( u1, w1, u2, w2, d, *( sA->GetSurfCore()->GetSurf() ),
                                     *( sB->GetSurfCore()->GetSurf() ),
                                     p, uwA.x(), uwA.y(), uwB.x(), uwB.y() );

    uwA[0] = u1;
    uwA[1] = w1;
    uwB[0] = u2;
    uwB[1] = w2;

    return d;
}

