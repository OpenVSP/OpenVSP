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

PntNodeCloud::PntNodeCloud()
{
    m_NumUsedPts = 0;
    m_index = nullptr;
}

PntNodeCloud::~PntNodeCloud()
{
    Cleanup();
}

void PntNodeCloud::Cleanup()
{
    m_PntNodes.clear();
    if ( m_index )
    {
        delete m_index;
        m_index = nullptr;
    }
}

void PntNodeCloud::AddPntNodes( const vector< vec3d > & pnts )
{
    m_PntNodes.reserve( m_PntNodes.size() + pnts.size() );
    for ( size_t i = 0 ; i < pnts.size() ; i++ )
    {
        m_PntNodes.emplace_back( PntNode( pnts[i] ) );
    }
}

void PntNodeCloud::ReserveMorePntNodes( long long int n )
{
    m_PntNodes.reserve( m_PntNodes.size() + n );
}

void PntNodeCloud::AddPntNode( const vec3d & pnt )
{
    m_PntNodes.emplace_back( PntNode( pnt ) );
}

bool PntNodeCloud::UsedNode( long long int i )
{
    assert ( i >= 0 && i < ( int )m_PntNodes.size() );

    return i == m_PntNodes[i].m_Index;
}

long long int PntNodeCloud::GetNodeUsedIndex( long long int i )
{
    assert ( i >= 0 && i < ( long long int )m_PntNodes.size() );

    long long int ind = m_PntNodes[i].m_Index;

    return m_PntNodes[ind].m_UsedIndex;

}

long long int PntNodeCloud::GetNodeBaseIndex( long long int i )
{
    assert ( i >= 0 && i < ( long long int )m_PntNodes.size() );

    return m_PntNodes[i].m_Index;
}

vector < long long int > PntNodeCloud::GetMatches( long long int i )
{
    assert ( i >= 0 && i < ( long long int )m_PntNodes.size() );

    long long int ind = m_PntNodes[i].m_Index;

    return m_PntNodes[ ind ].m_Matches;
}

long long int PntNodeCloud::LookupPntUsed( const vec3d & pnt )
{
    long long int num_results = 1;
    vector < unsigned int > ret_index( num_results );
    vector < double > out_dist_sqr( num_results );

    num_results = m_index->knnSearch( &pnt[0], num_results, &ret_index[0], &out_dist_sqr[0] );

    // In case of less points in the tree than requested:
    ret_index.resize( num_results );
    out_dist_sqr.resize( num_results );

    if ( ret_index.size() >= 1 )
    {
        return GetNodeUsedIndex( ret_index[0] );
    }

    printf( "Can't find point in LookupPntUsed\n" );
    return -1;
}

void PntNodeCloud::LookupPntBase( const vec3d & pnt, int num_results, vector < long long int > & results_vec )
{
    results_vec.clear();
    vector < unsigned int > ret_index( num_results );
    vector < double > out_dist_sqr( num_results );

    num_results = m_index->knnSearch( &pnt[0], num_results, &ret_index[0], &out_dist_sqr[0] );

    // In case of less points in the tree than requested:
    ret_index.resize( num_results );
    out_dist_sqr.resize( num_results );

    if ( ret_index.size() >= 1 )
    {
        results_vec.resize( ret_index.size() );
        for ( size_t i = 0; i < ret_index.size(); i++ )
        {
            results_vec[i] = GetNodeBaseIndex( ret_index[i] );
        }
    }
    else
    {
        printf( "Can't find point in LookupPntBase\n" );
    }
}

long long int PntNodeCloud::LookupPntBase( const vec3d & pnt )
{
    long long int num_results = 1;
    vector < unsigned int > ret_index( num_results );
    vector < double > out_dist_sqr( num_results );

    num_results = m_index->knnSearch( &pnt[0], num_results, &ret_index[0], &out_dist_sqr[0] );

    // In case of less points in the tree than requested:
    ret_index.resize( num_results );
    out_dist_sqr.resize( num_results );

    if ( ret_index.size() >= 1 )
    {
        return GetNodeBaseIndex( ret_index[0] );
    }

    printf( "Can't find point in LookupPntBase\n" );
    return -1;
}

void IndexPntNodes( PntNodeCloud & cloud, double tol )
{
    cloud.m_index = new PNTree( 3, cloud, KDTreeSingleIndexAdaptorParams( 10 )  );
    cloud.m_index->buildIndex();

    //==== Find Close Point Groups ====//
    int cnt = 0;
    for ( size_t i = 0 ; i < cloud.m_PntNodes.size() ; i++ )
    {
        if ( cloud.m_PntNodes[i].m_Index == -1 )
        {
            std::vector < std::pair < unsigned int, double > > ret_matches;

            nanoflann::SearchParams params;
            cloud.m_index->radiusSearch( &cloud.m_PntNodes[i].m_Pnt[0], tol, ret_matches, params );

            for ( size_t j = 0 ; j < ret_matches.size() ; j++ )
            {
                unsigned int m_ind = ret_matches[j].first;
                cloud.m_PntNodes[ m_ind ].m_Index = i;
                cloud.m_PntNodes[ i ].m_Matches.push_back( m_ind );
            }
            cloud.m_PntNodes[i].m_UsedIndex = cnt;
            cnt++;
        }
    }
    cloud.m_NumUsedPts = cnt;
}



