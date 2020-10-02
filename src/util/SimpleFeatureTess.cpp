//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SimpleFeatureTess.cpp:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#include "SimpleFeatureTess.h"

SimpleFeatureTess::SimpleFeatureTess()
{
    m_FlipNormal = false;
}

SimpleFeatureTess::~SimpleFeatureTess()
{
}

void SimpleFeatureTess::Transform( const Matrix4d & mat )
{
    // Transform points
    for ( int i = 0; i < m_ptline.size(); i++ )
    {
        mat.xformvec( m_ptline[i] );
    }
}
