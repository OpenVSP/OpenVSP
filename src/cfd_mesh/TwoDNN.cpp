//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//
// Created by Rob McDonald on 3/20/23.
//

#include "TwoDNN.h"

TwoDNodeCloud::TwoDNodeCloud()
{
    m_index = nullptr;
    m_NumUsedPts = 0;
}

TwoDNodeCloud::~TwoDNodeCloud()
{
    Cleanup();
}

void TwoDNodeCloud::Cleanup()
{
    m_PntNodes.clear();
    if ( m_index )
    {
        delete m_index;
        m_index = nullptr;
    }
}

void TwoDNodeCloud::AddPntNodes( const vector< vector< vec2d > > & pnts )
{
    size_t i, j;

    if( pnts.size() > 0 )
    {
        m_PntNodes.reserve( m_PntNodes.size() + pnts.size() * pnts[0].size() );

        i = 0;
        for ( j = 1; j < pnts[i].size()-1; j++ )
        {
            m_PntNodes.emplace_back( TwoDNode( pnts[ i ][ j ], i, j ) );
        }

        i = pnts.size()-1;
        for ( j = 1; j < pnts[i].size()-1; j++ )
        {
            m_PntNodes.emplace_back( TwoDNode( pnts[ i ][ j ], i, j ) );
        }

        j = 0;
        for ( i = 0; i < pnts.size(); i++ )
        {
            m_PntNodes.emplace_back( TwoDNode( pnts[ i ][ j ], i, j ) );
        }

        j = pnts.size() - 1;
        for ( i = 0; i < pnts.size(); i++ )
        {
            m_PntNodes.emplace_back( TwoDNode( pnts[ i ][ j ], i, j ) );
        }

        for ( i = 1; i < pnts.size()-1; i++ )
        {
            for ( j = 1; j < pnts[i].size()-1; j++ )
            {
                m_PntNodes.emplace_back( TwoDNode( pnts[ i ][ j ], i, j ) );
            }
        }

    }
}

long long int TwoDNodeCloud::LookupPnt( const vec2d & pnt )
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
        return ret_index[0];
    }

    printf( "Can't find point in LookupPnt\n" );
    return -1;
}

void TwoDNodeCloud::BuildIndex()
{
    m_index = new TwoDTree( 2, *this, KDTreeSingleIndexAdaptorParams( 10 )  );
    m_index->buildIndex();
}
