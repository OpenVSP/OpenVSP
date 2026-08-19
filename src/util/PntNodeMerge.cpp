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
#include "VspSurf.h"

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




bool AnyPntWithinTol( PntNodeCloud & cloud, const vec3d & pnt, double tol )
{
    if ( !cloud.m_index )
    {
        return false;
    }

    vector < pair < unsigned int, double > > matches;
    nanoflann::SearchParams params;

    // L2_Simple_Adaptor works in squared distance, so the radius handed to radiusSearch is squared.
    vec3d p = pnt;
    return cloud.m_index->radiusSearch( &p[0], tol * tol, matches, params ) > 0;
}

vector < vec3d > FilterPntsByMembership( const vector < vec3d > & pnts, const vector < vec3d > & other_pnts,
                                         double tol, bool keep_matched )
{
    vector < vec3d > out;

    if ( other_pnts.empty() )
    {
        // Nothing to match against: everything matches nothing, so keeping the matched ones keeps
        // none and dropping them keeps all.
        if ( !keep_matched )
        {
            out = pnts;
        }
        return out;
    }

    PntNodeCloud cloud;
    cloud.AddPntNodes( other_pnts );
    IndexPntNodes( cloud, tol * tol );

    out.reserve( pnts.size() );
    for ( int i = 0; i < ( int )pnts.size(); i++ )
    {
        if ( AnyPntWithinTol( cloud, pnts[i], tol ) == keep_matched )
        {
            out.push_back( pnts[i] );
        }
    }

    return out;
}

vector < vec3d > UniquePnts( const vector < vec3d > & pnts, double tol )
{
    vector < vec3d > out;

    if ( pnts.empty() )
    {
        return out;
    }

    // The tree groups every point with the others within tol and names one of each group; those
    // are the ones kept.  O( n log n ) rather than comparing every pair.
    PntNodeCloud cloud;
    cloud.AddPntNodes( pnts );
    IndexPntNodes( cloud, tol * tol );

    out.reserve( pnts.size() );
    for ( int i = 0; i < ( int )pnts.size(); i++ )
    {
        if ( cloud.UsedNode( i ) )
        {
            out.push_back( pnts[i] );
        }
    }

    return out;
}

vector < vec3d > FilterPntsInBBox( const vector < vec3d > & pnts, const vec3d & min_pnt, const vec3d & max_pnt,
                                   bool keep_inside )
{
    vector < vec3d > out;
    out.reserve( pnts.size() );

    for ( int i = 0; i < ( int )pnts.size(); i++ )
    {
        const vec3d &p = pnts[i];

        bool inside = p.x() >= min_pnt.x() && p.x() <= max_pnt.x() &&
                      p.y() >= min_pnt.y() && p.y() <= max_pnt.y() &&
                      p.z() >= min_pnt.z() && p.z() <= max_pnt.z();

        if ( inside == keep_inside )
        {
            out.push_back( p );
        }
    }

    return out;
}

vector < vec3d > FilterPntsInRange( const vector < vec3d > & pnts, int dir_index, double low, double high,
                                    bool keep_inside )
{
    vector < vec3d > out;
    out.reserve( pnts.size() );

    for ( int i = 0; i < ( int )pnts.size(); i++ )
    {
        double v = pnts[i].v[dir_index];

        bool inside = v >= low && v <= high;

        if ( inside == keep_inside )
        {
            out.push_back( pnts[i] );
        }
    }

    return out;
}

vector < vec3d > FilterPntsByValue( const vector < vec3d > & pnts, int dir_index, double val, bool keep_above )
{
    vector < vec3d > out;
    out.reserve( pnts.size() );

    for ( int i = 0; i < ( int )pnts.size(); i++ )
    {
        bool above = pnts[i].v[dir_index] > val;

        if ( above == keep_above )
        {
            out.push_back( pnts[i] );
        }
    }

    return out;
}

vector < vec3d > FilterPntsNearPnt( const vector < vec3d > & pnts, const vec3d & center, double radius,
                                    bool keep_near )
{
    vector < vec3d > out;
    out.reserve( pnts.size() );

    double r2 = radius * radius;

    for ( int i = 0; i < ( int )pnts.size(); i++ )
    {
        bool near = dist_squared( pnts[i], center ) <= r2;

        if ( near == keep_near )
        {
            out.push_back( pnts[i] );
        }
    }

    return out;
}

vector < vec3d > FilterPntsNearSurf( const vector < vec3d > & pnts, const VspSurf * surf, double tol,
                                     bool keep_near )
{
    vector < vec3d > out;

    if ( !surf )
    {
        return out;
    }

    out.reserve( pnts.size() );

    for ( int i = 0; i < ( int )pnts.size(); i++ )
    {
        double u, w;
        double d = surf->FindNearest01( u, w, pnts[i] );

        if ( ( d <= tol ) == keep_near )
        {
            out.push_back( pnts[i] );
        }
    }

    return out;
}

vector < vec3d > UnionPnts( const vector < vec3d > & pnts_a, const vector < vec3d > & pnts_b, double tol )
{
    vector < vec3d > both;

    both.reserve( pnts_a.size() + pnts_b.size() );
    both.insert( both.end(), pnts_a.begin(), pnts_a.end() );
    both.insert( both.end(), pnts_b.begin(), pnts_b.end() );

    return UniquePnts( both, tol );
}
