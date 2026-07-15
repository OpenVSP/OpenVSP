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

void SimpleFeatureTess::Transform( const Matrix4d & mat )
{
    // Transform points
    for ( int i = 0; i < m_ptline.size(); i++ )
    {
        mat.xformvec( m_ptline[i] );
    }
}

#include <type_traits>
// Guard the rule-of-zero cleanup: a user-declared destructor or copy operation would silently
// suppress the implicit move operations that vector<SimpleFeatureTess> relies on to avoid deep copies.
static_assert( std::is_nothrow_move_constructible< SimpleFeatureTess >::value, "SimpleFeatureTess must be nothrow move constructible" );
static_assert( std::is_nothrow_move_assignable< SimpleFeatureTess >::value, "SimpleFeatureTess must be nothrow move assignable" );
