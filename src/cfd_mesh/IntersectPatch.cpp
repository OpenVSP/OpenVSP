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

void intersect( SurfPatch& bp1, SurfPatch& bp2, int depth, CfdMeshMgrSingleton *MeshMgr )
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
        intersect_quads( bp1, bp2, MeshMgr );          // Plane - Plane Intersection
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

            intersect( bps1[0], bp2, depth, MeshMgr );
            intersect( bps1[1], bp2, depth, MeshMgr );
            intersect( bps1[2], bp2, depth, MeshMgr );
            intersect( bps1[3], bp2, depth, MeshMgr );
        }
        else
        {
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

void intersect_quads( SurfPatch& pa, SurfPatch& pb, CfdMeshMgrSingleton *MeshMgr )
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

