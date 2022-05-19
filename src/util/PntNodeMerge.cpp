//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Merge Near Pnts
//
//   J.R. Gloudemans 8/28/12
//
//******************************************************************************

#include "PntNodeMerge.h"


void PntNodeCloud::AddPntNodes( const vector< vec3d > & pnts )
{
    m_PntNodes.reserve( m_PntNodes.size() + pnts.size() );
    for ( size_t i = 0 ; i < pnts.size() ; i++ )
    {
        m_PntNodes.emplace_back( PntNode( pnts[i] ) );
    }
}

void PntNodeCloud::ReserveMorePntNodes( int n )
{
    m_PntNodes.reserve( m_PntNodes.size() + n );
}

void PntNodeCloud::AddPntNode( const vec3d & pnt )
{
    m_PntNodes.emplace_back( PntNode( pnt ) );
}

bool PntNodeCloud::UsedNode( int i )
{
    assert ( i >= 0 && i < ( int )m_PntNodes.size() );

    return i == m_PntNodes[i].m_Index;
}

int PntNodeCloud::GetNodeUsedIndex( int i )
{
    assert ( i >= 0 && i < ( int )m_PntNodes.size() );

    int ind = m_PntNodes[i].m_Index;

    return m_PntNodes[ind].m_UsedIndex;

}

int PntNodeCloud::GetNodeBaseIndex( int i )
{
    assert ( i >= 0 && i < ( int )m_PntNodes.size() );

    return m_PntNodes[i].m_Index;
}

void IndexPntNodes( PntNodeCloud & cloud, double tol )
{
    PNTree index( 3, cloud, KDTreeSingleIndexAdaptorParams( 10 )  );
    index.buildIndex();

    //==== Find Close Point Groups ====//
    int cnt = 0;
    for ( size_t i = 0 ; i < cloud.m_PntNodes.size() ; i++ )
    {
        if ( cloud.m_PntNodes[i].m_Index == -1 )
        {
            std::vector < std::pair < unsigned int, double > > ret_matches;

            nanoflann::SearchParams params;
            index.radiusSearch( &cloud.m_PntNodes[i].m_Pnt[0], tol, ret_matches, params );

            for ( size_t j = 0 ; j < ret_matches.size() ; j++ )
            {
                unsigned int m_ind = ret_matches[j].first;
                cloud.m_PntNodes[ m_ind ].m_Index = i;
            }
            cloud.m_PntNodes[i].m_UsedIndex = cnt;
            cnt++;
        }
    }
    cloud.m_NumUsedPts = cnt;
}



