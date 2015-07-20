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
// #include "FeaMeshMgr.h"
#include "Tritri.h"

//===== Check If New Intersection Point Is A Duplicate  =====//
int is_dup_int_pnt( int ni, vec3d int_pnt[2], vec3d& temp_pnt )
{
    if ( ni == 0 )
    {
        return( 0 );
    }

    double Dist_Tol = 1.0e-8;
//jrg 8/25/2010
//  double Dist_Tol = 0.0000000001;


    if ( dist_squared( int_pnt[0], temp_pnt ) < Dist_Tol )
    {
        return( 1 );
    }

    return( 0 );
}

//////////////////////////////////////////////////////////////////////

//===== Intersect Two Bezier Patches  =====//
void intersect( SurfPatch& bp1, SurfPatch& bp2 )
{
    if ( !Compare( *bp1.get_bbox(), *bp2.get_bbox() ) )
    {
        return;
    }

    SurfPatch bps[4];

    int pln_flag1 = bp1.test_planar( DEFAULT_PLANE_TOL );
    int pln_flag2 = bp2.test_planar( DEFAULT_PLANE_TOL );
    if ( pln_flag1 && pln_flag2 )
    {
        intersect_quads( bp1, bp2 );          // Plane - Plane Intersection
    }
    else if ( pln_flag1 )
    {
        bp2.split_patch( bps[0], bps[1], bps[2], bps[3] );    // Split Patch2 and Keep Subdividing
        intersect( bp1, bps[0] );
        intersect( bp1, bps[1] );
        intersect( bp1, bps[2] );
        intersect( bp1, bps[3] );
//      bp1.assemble_int_curves();
    }
    else if ( pln_flag2 )
    {
        bp1.split_patch( bps[0], bps[1], bps[2], bps[3] );    // Split Patch1 and Keep Subdividing
        intersect( bps[0], bp2 );
        intersect( bps[1], bp2 );
        intersect( bps[2], bp2 );
        intersect( bps[3], bp2 );

//      bp1.merge_int_curves(bps);
//      bp1.assemble_int_curves();
    }
    else if ( bp1.bnd_box.DiagDist() > bp2.bnd_box.DiagDist() )
    {
        bp1.split_patch( bps[0], bps[1], bps[2], bps[3] );    // Split Patch1 and Keep Subdividing
        intersect( bps[0], bp2 );
        intersect( bps[1], bp2 );
        intersect( bps[2], bp2 );
        intersect( bps[3], bp2 );

//      bp1.merge_int_curves(bps);
//      bp1.assemble_int_curves();
    }
    else
    {
        bp2.split_patch( bps[0], bps[1], bps[2], bps[3] );    // Split Patch2 and Keep Subdividing
        intersect( bp1, bps[0] );
        intersect( bp1, bps[1] );
        intersect( bp1, bps[2] );
        intersect( bp1, bps[3] );
//      bp1.assemble_int_curves();
    }
}

void intersect( SurfPatch& bp1, SurfPatch& bp2, int depth )
{
    int MAX_SUB = 3;
    if ( !Compare( *bp1.get_bbox(), *bp2.get_bbox() ) )
    {
        return;
    }

    SurfPatch bps1[4];
    SurfPatch bps2[4];

    //if ( bp1.bnd_box.diag_dist() < 0.1 )
    //  bp1.SetSubDepth( MAX_SUB + 1 );

    //if ( bp2.bnd_box.diag_dist() < 0.1 )
    //  bp2.SetSubDepth( MAX_SUB + 1 );

    if ( bp1.GetSubDepth() > MAX_SUB && bp2.GetSubDepth() > MAX_SUB )
    {
        intersect_quads( bp1, bp2 );          // Plane - Plane Intersection
    }
    else
    {
        if ( bp1.GetSubDepth() < bp2.GetSubDepth() )
        {
            bp1.split_patch( bps1[0], bps1[1], bps1[2], bps1[3] );      // Split Patch1 and Keep Subdividing
            for ( int i = 0 ; i < 4 ; i++ )
            {
                bps1[i].SetSubDepth( bp1.GetSubDepth() + 1 );
            }

            intersect( bps1[0], bp2, depth );
            intersect( bps1[1], bp2, depth );
            intersect( bps1[2], bp2, depth );
            intersect( bps1[3], bp2, depth );
        }
        else
        {
            bp2.split_patch( bps2[0], bps2[1], bps2[2], bps2[3] );      // Split Patch2 and Keep Subdividing
            for ( int i = 0 ; i < 4 ; i++ )
            {
                bps2[i].SetSubDepth( bp2.GetSubDepth() + 1 );
            }

            intersect( bp1, bps2[0], depth );
            intersect( bp1, bps2[1], depth );
            intersect( bp1, bps2[2], depth );
            intersect( bp1, bps2[3], depth );
        }
    }
}


//////===== Intersect Two Close To Planar Quadrilaterals  =====//
////void intersect_quads( SurfPatch& bp1, SurfPatch& bp2)
////{
////    vec3d temp_pnt;
////    vec3d int_pnt[12];
////    double u1[12], w1[12], u2[12], w2[12];
////    double r, s, t;
////    r = s = t = 0.0;
////
////    //==== Form Triangle and Segment Vectors ====//
////    vec3d OA1 = bp1.pnts[0][0];
////    vec3d OB1 = bp1.pnts[3][3];
////    vec3d A1 = bp1.pnts[3][3] - bp1.pnts[0][0];
////    vec3d B1 = bp1.pnts[3][0] - bp1.pnts[0][0];
////    vec3d C1 = bp1.pnts[0][3] - bp1.pnts[0][0];
////    vec3d D1 = bp1.pnts[3][0] - bp1.pnts[3][3];
////    vec3d E1 = bp1.pnts[0][3] - bp1.pnts[3][3];
////    double del_u1 = bp1.u_max - bp1.u_min;
////    double del_w1 = bp1.w_max - bp1.w_min;
////
////    vec3d OA2 = bp2.pnts[0][0];
////    vec3d OB2 = bp2.pnts[3][3];
////    vec3d A2 = bp2.pnts[3][3] - bp2.pnts[0][0];
////    vec3d B2 = bp2.pnts[3][0] - bp2.pnts[0][0];
////    vec3d C2 = bp2.pnts[0][3] - bp2.pnts[0][0];
////    vec3d D2 = bp2.pnts[3][0] - bp2.pnts[3][3];
////    vec3d E2 = bp2.pnts[0][3] - bp2.pnts[3][3];
////    double del_u2 = bp2.u_max - bp2.u_min;
////    double del_w2 = bp2.w_max - bp2.w_min;
////
////    int ni = 0;
////
////    //==== Perform Intersections - First Triangle====//
////    if ( tri_seg_intersect(OA1, A1, B1, OA2, B2, r, s, t) )
////    {
////        u1[ni] = bp1.u_min + (r + s)*del_u1;
////        w1[ni] = bp1.w_min + r*del_w1;
////        u2[ni] = bp2.u_min + t*del_u2;
////        w2[ni] = bp2.w_min;
////        int_pnt[ni] = OA2 + B2*t;  ni++;
////    }
////    if ( tri_seg_intersect(OA1, A1, B1, OA2, C2, r, s, t) )
////    {
////        temp_pnt = OA2 + C2*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u1[ni] = bp1.u_min + (r + s)*del_u1;
////            w1[ni] = bp1.w_min + r*del_w1;
////            u2[ni] = bp2.u_min;
////            w2[ni] = bp2.w_min + t*del_w2;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA1, A1, B1, OB2, D2, r, s, t) )
////    {
////        temp_pnt = OB2 + D2*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u1[ni] = bp1.u_min + (r + s)*del_u1;
////            w1[ni] = bp1.w_min + r*del_w1;
////            u2[ni] = bp2.u_max;
////            w2[ni] = bp2.w_max - t*del_w2;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA1, A1, B1, OB2, E2, r, s, t) )
////    {
////        temp_pnt = OB2 + E2*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u1[ni] = bp1.u_min + (r + s)*del_u1;
////            w1[ni] = bp1.w_min + r*del_w1;
////            u2[ni] = bp2.u_max - t*del_u2;
////            w2[ni] = bp2.w_max;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////    //==== Perform Intersections - Second Triangle====//
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA1, A1, C1, OA2, B2, r, s, t) )
////    {
////        temp_pnt =  OA2 + B2*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u1[ni] = bp1.u_min + r*del_u1;
////            w1[ni] = bp1.w_min + (r + s)*del_w1;
////            u2[ni] = bp2.u_min + t*del_u2;
////            w2[ni] = bp2.w_min;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA1, A1, C1, OA2, C2, r, s, t) )
////    {
////        temp_pnt =  OA2 + C2*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u1[ni] = bp1.u_min + r*del_u1;
////            w1[ni] = bp1.w_min + (r + s)*del_w1;
////            u2[ni] = bp2.u_min;
////            w2[ni] = bp2.w_min + t*del_w2;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA1, A1, C1, OB2, D2, r, s, t) )
////    {
////        temp_pnt = OB2 + D2*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u1[ni] = bp1.u_min + r*del_u1;
////            w1[ni] = bp1.w_min + (r + s)*del_w1;
////            u2[ni] = bp2.u_max;
////            w2[ni] = bp2.w_max - t*del_w2;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA1, A1, C1, OB2, E2, r, s, t) )
////    {
////        temp_pnt = OB2 + E2*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u1[ni] = bp1.u_min + r*del_u1;
////            w1[ni] = bp1.w_min + (r + s)*del_w1;
////            u2[ni] = bp2.u_max - t*del_u2;
////            w2[ni] = bp2.w_max;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////
////    //==== Perform Intersections - Third Triangle====//
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA2, A2, B2, OA1, B1, r, s, t) )
////    {
////        temp_pnt = OA1 + B1*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u2[ni] = bp2.u_min + (r + s)*del_u2;
////            w2[ni] = bp2.w_min + r*del_w2;
////            u1[ni] = bp1.u_min + t*del_u1;
////            w1[ni] = bp1.w_min;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA2, A2, B2, OA1, C1, r, s, t) )
////    {
////        temp_pnt = OA1 + C1*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u2[ni] = bp2.u_min + (r + s)*del_u2;
////            w2[ni] = bp2.w_min + r*del_w2;
////            u1[ni] = bp1.u_min;
////            w1[ni] = bp1.w_min + t*del_w1;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA2, A2, B2, OB1, D1, r, s, t) )
////    {
////        temp_pnt = OB1 + D1*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u2[ni] = bp2.u_min + (r + s)*del_u2;
////            w2[ni] = bp2.w_min + r*del_w2;
////            u1[ni] = bp1.u_max;
////            w1[ni] = bp1.w_max - t*del_w1;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA2, A2, B2, OB1, E1, r, s, t) )
////    {
////        temp_pnt = OB1 + E1*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u2[ni] = bp2.u_min + (r + s)*del_u2;
////            w2[ni] = bp2.w_min + r*del_w2;
////            u1[ni] = bp1.u_max - t*del_u1;
////            w1[ni] = bp1.w_max;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////
////    //==== Perform Intersections - Fourth Triangle====//
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA2, A2, C2, OA1, B1, r, s, t) )
////    {
////        temp_pnt = OA1 + B1*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u2[ni] = bp2.u_min + r*del_u2;
////            w2[ni] = bp2.w_min + (r + s)*del_w2;
////            u1[ni] = bp1.u_min + t*del_u1;
////            w1[ni] = bp1.w_min;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA2, A2, C2, OA1, C1, r, s, t) )
////    {
////        temp_pnt = OA1 + C1*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u2[ni] = bp2.u_min + r*del_u2;
////            w2[ni] = bp2.w_min + (r + s)*del_w2;
////            u1[ni] = bp1.u_min;
////            w1[ni] = bp1.w_min + t*del_w1;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA2, A2, C2, OB1, D1, r, s, t) )
////    {
////        temp_pnt = OB1 + D1*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u2[ni] = bp2.u_min + r*del_u2;
////            w2[ni] = bp2.w_min + (r + s)*del_w2;
////            u1[ni] = bp1.u_max;
////            w1[ni] = bp1.w_max - t*del_w1;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////    if ( /*(ni < 2) &&*/  tri_seg_intersect(OA2, A2, C2, OB1, E1, r, s, t) )
////    {
////        temp_pnt = OB1 + E1*t;
////        if ( !is_dup_int_pnt(ni, int_pnt, temp_pnt) )
////        {
////            u2[ni] = bp2.u_min + r*del_u2;
////            w2[ni] = bp2.w_min + (r + s)*del_w2;
////            u1[ni] = bp1.u_max - t*del_u1;
////            w1[ni] = bp1.w_max;
////            int_pnt[ni] = temp_pnt; ni++;
////        }
////    }
////
////    if ( ni == 1 )
////        printf("SurfPatch: intersect_quads only one intersection point found\n");
////    if ( ni > 2 )
////        printf("SurfPatch: intersect_quads more than two intersection points found\n");
////
////    if ( ni > 1)
////    {
////        double real_uw[2];
////        double guess_uw[2];
////        Surf* s1 = bp1.get_surf_ptr();
////        Surf* s2 = bp2.get_surf_ptr();
////        CfdMeshMgr* smgr = s1->GetCfdMeshMgr();
////
////        for ( int iter = 0 ; iter < 1 ; iter++ )
////        {
////            guess_uw[0] = u1[0];  guess_uw[1] = w1[0];
////            bp1.find_closest_uw( int_pnt[0], guess_uw, real_uw );
////            u1[0] = real_uw[0];   w1[0] = real_uw[1];
////
////            guess_uw[0] = u1[1];  guess_uw[1] = w1[1];
////            bp1.find_closest_uw( int_pnt[1], guess_uw, real_uw );
////            u1[1] = real_uw[0];   w1[1] = real_uw[1];
////
////            guess_uw[0] = u2[0];  guess_uw[1] = w2[0];
////            bp2.find_closest_uw( int_pnt[0], guess_uw, real_uw );
////            u2[0] = real_uw[0];   w2[0] = real_uw[1];
////
////            guess_uw[0] = u2[1];  guess_uw[1] = w2[1];
////            bp2.find_closest_uw( int_pnt[1], guess_uw, real_uw );
////            u2[1] = real_uw[0];   w2[1] = real_uw[1];
////
////            int_pnt[0] = ( s1->CompPnt( u1[0], w1[0] ) + s2->CompPnt( u2[0], w2[0] ) )*0.5;
////            int_pnt[1] = ( s1->CompPnt( u1[1], w1[1] ) + s2->CompPnt( u2[1], w2[1] ) )*0.5;
////        }
////
//////vec3d pA = s1->CompPnt( u1[0], w1[0] );
//////vec3d pB = s2->CompPnt( u2[0], w2[0] );
//////double d = dist( pA, pB );
//////
//////vec3d pnA = pA;
//////vec3d pnB = pB;
//////for ( int iter = 0 ; iter < 100 ; iter++ )
//////{
//////vec3d pTarget = (pnA + pnB)*0.5;
//////vec2d uwA = s1->ClosestUW( pTarget, u1[0], w1[0], 0.000001, 0.000001, 0.0000000000001 );
//////vec2d uwB = s2->ClosestUW( pTarget, u2[0], w2[0], 0.000001, 0.000001, 0.0000000000001 );
//////pnA = s1->CompPnt( uwA[0], uwA[1] );
//////pnB = s2->CompPnt( uwB[0], uwB[1] );
//////}
//////double dn = dist( pnA, pnB );
//////
//////
//////if ( dn > 0.00005 )
//////{
//////  double l = dist( int_pnt[0], int_pnt[1] );
//////  int junk = 21;
//////}
////
////
////        assert( s1 && s2 && smgr );
////        vec2d uw10 = vec2d(u1[0],w1[0]);
////        vec2d uw11 = vec2d(u1[1],w1[1]);
////        vec2d uw20 = vec2d(u2[0],w2[0]);
////        vec2d uw21 = vec2d(u2[1],w2[1]);
////
////        smgr->AddISeg( s1, s2, uw10, uw11, uw20, uw21 );
////
////
////        //ISeg seg;
////        //seg.m_SurfA = s1;
////        //seg.m_SurfB = s2;
////
////        //seg.m_AUW[0] = vec2d( u1[0], w1[0] );
////        //seg.m_AUW[1] = vec2d( u1[1], w1[1] );
////        //seg.m_BUW[0] = vec2d( u2[0], w2[0] );
////        //seg.m_BUW[1] = vec2d( u2[1], w2[1] );
////
////        //smgr->AddISeg( seg );
////
////
////    }
////}

void intersect_quads( SurfPatch& pa, SurfPatch& pb )
{
    int iflag;
    int coplanar;
    vec3d ip0, ip1;

    vec3d a0 = pa.pnts[0][0];
    vec3d a1 = pa.pnts[3][0];
    vec3d a2 = pa.pnts[3][3];
    vec3d a3 = pa.pnts[0][3];
    vec3d b0 = pb.pnts[0][0];
    vec3d b1 = pb.pnts[3][0];
    vec3d b2 = pb.pnts[3][3];
    vec3d b3 = pb.pnts[0][3];

    //==== Tri A1 and B1 ====//
    iflag = tri_tri_intersect_with_isectline( a0.v, a2.v, a3.v, b0.v, b2.v, b3.v, &coplanar, ip0.v, ip1.v );
    if ( iflag && !coplanar )
    {
        CfdMeshMgr.AddIntersectionSeg( pa, pb, ip0, ip1 );
    }

    //==== Tri A1 and B2 ====//
    iflag = tri_tri_intersect_with_isectline( a0.v, a2.v, a3.v, b0.v, b1.v, b2.v, &coplanar, ip0.v, ip1.v );
    if ( iflag && !coplanar )
    {
        CfdMeshMgr.AddIntersectionSeg( pa, pb, ip0, ip1 );
    }

    //==== Tri A2 and B1 ====//
    iflag = tri_tri_intersect_with_isectline( a0.v, a1.v, a2.v, b0.v, b2.v, b3.v, &coplanar, ip0.v, ip1.v );
    if ( iflag && !coplanar )
    {
        CfdMeshMgr.AddIntersectionSeg( pa, pb,  ip0, ip1 );
    }

    //==== Tri A2 and B2 ====//
    iflag = tri_tri_intersect_with_isectline( a0.v, a1.v, a2.v, b0.v, b1.v, b2.v, &coplanar, ip0.v, ip1.v );
    if ( iflag && !coplanar )
    {
        CfdMeshMgr.AddIntersectionSeg( pa, pb, ip0, ip1 );
    }
}

