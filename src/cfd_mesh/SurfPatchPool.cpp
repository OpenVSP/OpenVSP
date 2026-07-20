//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// SurfPatchPool.cpp
//////////////////////////////////////////////////////////////////////

#include "SurfPatchPool.h"

SurfPatchPool::SurfPatchPool()
{
    m_top = 0;

    // Pre-create patches for a fairly deep recursion so the arena does not have to grow in the
    // middle of meshing.  intersect() caps each patch at MAX_SUB = 12 subdivisions; with the two
    // patches alternating that is roughly two dozen levels, four sub-patches per level.  The
    // headroom is cheap -- an unused patch is an empty degree-0 grid until it is first acquired --
    // and if a recursion ever runs deeper the deque simply grows (addresses stay valid).
    const int estimate_levels = 30;
    for ( int i = 0; i < 4 * estimate_levels; i++ )
    {
        m_pool.emplace_back();
    }
}

SurfPatchPool & SurfPatchPool::instance()
{
    static thread_local SurfPatchPool pool;
    return pool;
}

SurfPatch & SurfPatchPool::at( int i )
{
    while ( ( int )m_pool.size() <= i )
    {
        m_pool.emplace_back();
    }
    return m_pool[ i ];
}

int SurfPatchPool::acquire4( int n, int m, int d, SurfPatch *&p0, SurfPatch *&p1, SurfPatch *&p2, SurfPatch *&p3 )
{
    int marker = m_top;

    p0 = &at( m_top );
    p1 = &at( m_top + 1 );
    p2 = &at( m_top + 2 );
    p3 = &at( m_top + 3 );

    m_top += 4;

    p0->PrepareForReuse( n, m, d );
    p1->PrepareForReuse( n, m, d );
    p2->PrepareForReuse( n, m, d );
    p3->PrepareForReuse( n, m, d );

    return marker;
}
