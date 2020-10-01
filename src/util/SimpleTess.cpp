//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SimpleTess.cpp:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#include "SimpleTess.h"

SimpleTess::SimpleTess()
{
    m_FlipNormal = false;
}

SimpleTess::~SimpleTess()
{
}

void SimpleTess::Transform( const Matrix4d & mat )
{
    // Transform points
    for ( int i = 0; i < m_pnts.size(); i++ )
    {
        for ( int j = 0; j < m_pnts[i].size(); j++ )
        {
            mat.xformvec( m_pnts[i][j] );
        }
    }

    // Transform normal vectors
    for ( int i = 0; i < m_norms.size(); i++ )
    {
        for ( int j = 0; j < m_norms[i].size(); j++ )
        {
            mat.xformnormvec( m_norms[i][j] );
        }
    }
}

void SimpleTess::GetBoundingBox( BndBox &bb ) const
{
    bb.Reset();

    for ( int i = 0; i < m_pnts.size(); i++ )
    {
        for ( int j = 0; j < m_pnts[i].size(); j++ )
        {
            for ( int k = 0; k < m_pnts[i][j].size(); k++ )
            {
                bb.Update( m_pnts[i][j][k] );
            }
        }
    }
}
