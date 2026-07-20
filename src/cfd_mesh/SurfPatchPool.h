//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// SurfPatchPool.h
//
// Per-thread arena of reusable SurfPatch objects for the recursive patch subdivision in
// intersect() (IntersectPatch.cpp) and SurfPatch::IntersectLineSeg().  Both routines subdivide
// a patch into four sub-patches at every recursion node and recurse depth-first.  Constructing
// four fresh SurfPatch per node heap-allocates four Bezier control-point grids, which dominates
// the memory churn during surface intersection and interior-tri removal.
//
// Because the recursion is depth-first, patches are needed and finished in strict LIFO order.
// This pool hands out blocks of four sub-patches from a growable arena and reclaims them as the
// recursion unwinds, so the underlying grids are reused: the heap allocation happens only during
// warm-up and is amortized away for the rest of the (per-thread) mesh run.  All of the pooling
// and index bookkeeping lives here so the recursion sites change minimally.
//////////////////////////////////////////////////////////////////////

#if !defined(SURF_SURFPATCHPOOL__INCLUDED_)
#define SURF_SURFPATCHPOOL__INCLUDED_

#include "SurfPatch.h"

#include <deque>

class SurfPatchPool
{
public:
    SurfPatchPool();

    // The pool belonging to the calling thread.  Intersection may run on worker threads, so the
    // pool is thread-local; each thread gets its own independent arena.
    static SurfPatchPool & instance();

    // Hand out four sub-patches, each sized to degree (n,m) and stamped with recursion depth d.
    // Returns a marker to hand back to release() once this block -- and everything acquired above
    // it -- is finished.
    int acquire4( int n, int m, int d, SurfPatch *&p0, SurfPatch *&p1, SurfPatch *&p2, SurfPatch *&p3 );

    // Release the arena back to a marker previously returned by acquire4().  Calls must be
    // strictly LIFO, which the depth-first recursion guarantees.
    void release( int marker )
    {
        m_top = marker;
    }

    // Two reusable scratch patches for the intermediate half-splits inside SurfPatch::split_patch.
    // These are not part of the arena: split_patch is not reentrant, so one fixed pair suffices
    // and it never overlaps a live arena block.
    surface_patch_type & splitScratchLo()
    {
        return m_splitLo;
    }
    surface_patch_type & splitScratchHi()
    {
        return m_splitHi;
    }

private:
    // Return patch i, growing the arena (without invalidating existing addresses) if needed.
    SurfPatch & at( int i );

    std::deque< SurfPatch > m_pool;   // deque: growth never invalidates pointers to elements
    int m_top;                        // high-water cursor: index of the next free patch

    surface_patch_type m_splitLo;
    surface_patch_type m_splitHi;
};

#endif // SURF_SURFPATCHPOOL__INCLUDED_
