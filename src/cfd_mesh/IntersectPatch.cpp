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
#include "Tritri.h"

void intersect( SurfPatch& bp1, SurfPatch& bp2, int depth, SurfaceIntersectionSingleton *MeshMgr )
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
            SurfPatch bps1[4];
            bp1.split_patch( bps1[0], bps1[1], bps1[2], bps1[3] );      // Split Patch1 and Keep Subdividing
            for ( int i = 0 ; i < 4 ; i++ )
            {
                bps1[i].SetSubDepth( bp1.GetSubDepth() + 1 );
            }

            intersect( bps1[0], bp2, depth, MeshMgr );
            intersect( bps1[1], bp2, depth, MeshMgr );
            intersect( bps1[2], bp2, depth, MeshMgr );
            intersect( bps1[3], bp2, depth, MeshMgr );
        }
        else
        {
            SurfPatch bps2[4];
            bp2.split_patch( bps2[0], bps2[1], bps2[2], bps2[3] );      // Split Patch2 and Keep Subdividing
            for ( int i = 0 ; i < 4 ; i++ )
            {
                bps2[i].SetSubDepth( bp2.GetSubDepth() + 1 );
            }

            intersect( bp1, bps2[0], depth, MeshMgr );
            intersect( bp1, bps2[1], depth, MeshMgr );
            intersect( bp1, bps2[2], depth, MeshMgr );
            intersect( bp1, bps2[3], depth, MeshMgr );
        }
    }
}

void intersect_quads( SurfPatch& pa, SurfPatch& pb, SurfaceIntersectionSingleton *MeshMgr )
{
    int iflag;
    int coplanar;
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

    //==== Tri A1 and B1 ====//
    iflag = tri_tri_intersect_with_isectline( a0.v, a2.v, a3.v, b0.v, b2.v, b3.v, &coplanar, ip0.v, ip1.v );
    if ( iflag && !coplanar )
    {
        MeshMgr->AddIntersectionSeg( pa, pb, ip0, ip1 );
    }

    //==== Tri A1 and B2 ====//
    iflag = tri_tri_intersect_with_isectline( a0.v, a2.v, a3.v, b0.v, b1.v, b2.v, &coplanar, ip0.v, ip1.v );
    if ( iflag && !coplanar )
    {
        MeshMgr->AddIntersectionSeg( pa, pb, ip0, ip1 );
    }

    //==== Tri A2 and B1 ====//
    iflag = tri_tri_intersect_with_isectline( a0.v, a1.v, a2.v, b0.v, b2.v, b3.v, &coplanar, ip0.v, ip1.v );
    if ( iflag && !coplanar )
    {
        MeshMgr->AddIntersectionSeg( pa, pb,  ip0, ip1 );
    }

    //==== Tri A2 and B2 ====//
    iflag = tri_tri_intersect_with_isectline( a0.v, a1.v, a2.v, b0.v, b1.v, b2.v, &coplanar, ip0.v, ip1.v );
    if ( iflag && !coplanar )
    {
        MeshMgr->AddIntersectionSeg( pa, pb, ip0, ip1 );
    }
}

