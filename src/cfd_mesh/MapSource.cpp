//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "MapSource.h"

#include <algorithm>
#include <cmath>
#include <limits>

// Code-Eli's headers turn floating point contraction off for the rest of each file that includes
// them, Surf.cpp among them.  Distances and limits here are rounded the same way, term by term,
// so a search gives the same values as a search through MSTree in Surf.cpp.
#if defined(__clang__)
#pragma STDC FP_CONTRACT OFF
#endif

// Sources per leaf.
static const int MS_LEAF_SIZE = 8;

// A source this close to the edge of the search radius, relative to its square, may or may not
// be found by a nanoflann radius search, whose running box distances round differently.
static const double MS_EDGE_TOL = 1.0e-10;

void MSBoundTree::Build( const vector< MapSource* > &sources )
{
    m_Sources = sources;
    m_TreeSources = sources;

    int n = ( int )m_TreeSources.size();

    m_Nodes.clear();
    // Leaves hold at least half of MS_LEAF_SIZE sources.
    m_Nodes.reserve( 2 * ( 2 * n / MS_LEAF_SIZE ) + 1 );

    if ( n > 0 )
    {
        m_Nodes.push_back( Node() );
        BuildNode( 0, 0, n );
    }

    m_Pts.resize( 3 * n );
    m_Surf.resize( n );
    for ( int k = 0; k < n; k++ )
    {
        m_Pts[ 3 * k ] = m_TreeSources[k]->m_pt.x();
        m_Pts[ 3 * k + 1 ] = m_TreeSources[k]->m_pt.y();
        m_Pts[ 3 * k + 2 ] = m_TreeSources[k]->m_pt.z();
        m_Surf[k] = m_TreeSources[k]->m_surfid;
    }

    int nnode = ( int )m_Nodes.size();
    m_Parent.assign( nnode, -1 );
    for ( int inode = 0; inode < nnode; inode++ )
    {
        if ( m_Nodes[inode].m_Child >= 0 )
        {
            m_Parent[ m_Nodes[inode].m_Child ] = inode;
            m_Parent[ m_Nodes[inode].m_Child + 1 ] = inode;
        }
    }
    m_NodeDirty.assign( nnode, false );

    m_SurfCount.clear();
    for ( int k = 0; k < n; k++ )
    {
        if ( m_Surf[k] >= 0 )
        {
            if ( m_Surf[k] >= ( int )m_SurfCount.size() )
            {
                m_SurfCount.resize( m_Surf[k] + 1, 0 );
            }
            m_SurfCount[ m_Surf[k] ]++;
        }
    }

    // The leaves that hold each surface's sources, surface by surface.
    int nsurf = ( int )m_SurfCount.size();
    vector< pair< int, int > > surfleaf;
    for ( int inode = 0; inode < nnode; inode++ )
    {
        const Node &node = m_Nodes[inode];
        if ( node.m_Child < 0 )
        {
            for ( int k = node.m_Begin; k < node.m_End; k++ )
            {
                if ( m_Surf[k] >= 0 )
                {
                    surfleaf.push_back( pair< int, int >( m_Surf[k], inode ) );
                }
            }
        }
    }
    std::sort( surfleaf.begin(), surfleaf.end() );
    surfleaf.erase( std::unique( surfleaf.begin(), surfleaf.end() ), surfleaf.end() );

    m_SurfLeaves.resize( surfleaf.size() );
    m_SurfLeafStart.assign( nsurf + 1, 0 );
    for ( int m = 0; m < ( int )surfleaf.size(); m++ )
    {
        m_SurfLeaves[m] = surfleaf[m].second;
        m_SurfLeafStart[ surfleaf[m].first + 1 ]++;
    }
    for ( int isurf = 0; isurf < nsurf; isurf++ )
    {
        m_SurfLeafStart[ isurf + 1 ] += m_SurfLeafStart[ isurf ];
    }

    UpdateStrengths();
}

void MSBoundTree::BuildNode( int inode, int begin, int end )
{
    Node node;
    for ( int d = 0; d < 3; d++ )
    {
        node.m_Lo[d] = std::numeric_limits< double >::max();
        node.m_Hi[d] = -std::numeric_limits< double >::max();
    }

    for ( int k = begin; k < end; k++ )
    {
        for ( int d = 0; d < 3; d++ )
        {
            double x = m_TreeSources[k]->m_pt.v[d];
            node.m_Lo[d] = std::min( node.m_Lo[d], x );
            node.m_Hi[d] = std::max( node.m_Hi[d], x );
        }
    }

    node.m_Begin = begin;
    node.m_End = end;
    node.m_Child = -1;
    node.m_Min = std::numeric_limits< double >::max();
    node.m_MinSurf = -1;
    node.m_MinOther = std::numeric_limits< double >::max();

    if ( end - begin <= MS_LEAF_SIZE )
    {
        m_Nodes[inode] = node;
        return;
    }

    int dim = 0;
    for ( int d = 1; d < 3; d++ )
    {
        if ( node.m_Hi[d] - node.m_Lo[d] > node.m_Hi[dim] - node.m_Lo[dim] )
        {
            dim = d;
        }
    }

    int mid = begin + ( end - begin ) / 2;
    std::nth_element( m_TreeSources.begin() + begin, m_TreeSources.begin() + mid, m_TreeSources.begin() + end,
                      [dim]( const MapSource* a, const MapSource* b )
                      {
                          return a->m_pt.v[dim] < b->m_pt.v[dim];
                      } );

    // The two children sit next to each other, so one index finds both.
    int c0 = ( int )m_Nodes.size();
    m_Nodes.push_back( Node() );
    m_Nodes.push_back( Node() );
    node.m_Child = c0;
    m_Nodes[inode] = node;

    BuildNode( c0, begin, mid );
    BuildNode( c0 + 1, mid, end );
}

void MSBoundTree::UpdateStrengths()
{
    int n = ( int )m_TreeSources.size();
    m_Str.resize( n );
    for ( int k = 0; k < n; k++ )
    {
        m_Str[k] = m_TreeSources[k]->m_str;
    }

    // Children always come after their parent.
    for ( int inode = ( int )m_Nodes.size() - 1; inode >= 0; inode-- )
    {
        UpdateNode( inode );
    }
}

void MSBoundTree::UpdateSurface( int isurf )
{
    if ( isurf < 0 || isurf + 1 >= ( int )m_SurfLeafStart.size() )
    {
        return;
    }

    m_Dirty.clear();

    for ( int m = m_SurfLeafStart[ isurf ]; m < m_SurfLeafStart[ isurf + 1 ]; m++ )
    {
        int inode = m_SurfLeaves[m];
        const Node &leaf = m_Nodes[inode];
        for ( int k = leaf.m_Begin; k < leaf.m_End; k++ )
        {
            m_Str[k] = m_TreeSources[k]->m_str;
        }

        // Mark the leaf and every node above it not yet marked.
        while ( inode >= 0 && !m_NodeDirty[inode] )
        {
            m_NodeDirty[inode] = true;
            m_Dirty.push_back( inode );
            inode = m_Parent[inode];
        }
    }

    // Children before parents.
    std::sort( m_Dirty.begin(), m_Dirty.end(), std::greater< int >() );

    for ( int m = 0; m < ( int )m_Dirty.size(); m++ )
    {
        UpdateNode( m_Dirty[m] );
        m_NodeDirty[ m_Dirty[m] ] = false;
    }
}

// The node's smallest strengths, from its sources or from its children's.
void MSBoundTree::UpdateNode( int inode )
{
    Node &node = m_Nodes[inode];

    double minstr = std::numeric_limits< double >::max();
    int minsurf = -1;
    double minother = std::numeric_limits< double >::max();

    if ( node.m_Child < 0 )
    {
        for ( int k = node.m_Begin; k < node.m_End; k++ )
        {
            if ( m_Str[k] < minstr )
            {
                minstr = m_Str[k];
                minsurf = m_Surf[k];
            }
        }
        for ( int k = node.m_Begin; k < node.m_End; k++ )
        {
            if ( m_Surf[k] != minsurf && m_Str[k] < minother )
            {
                minother = m_Str[k];
            }
        }
    }
    else
    {
        const Node &a = m_Nodes[ node.m_Child ];
        const Node &b = m_Nodes[ node.m_Child + 1 ];

        if ( b.m_Min < a.m_Min )
        {
            minstr = b.m_Min;
            minsurf = b.m_MinSurf;
        }
        else
        {
            minstr = a.m_Min;
            minsurf = a.m_MinSurf;
        }
        minother = std::min( MinExcluding( a, minsurf ), MinExcluding( b, minsurf ) );
    }

    node.m_Min = minstr;
    node.m_MinSurf = minsurf;
    node.m_MinOther = minother;
}

double MSBoundTree::MinExcluding( const Node &node, int skipsurf ) const
{
    if ( node.m_MinSurf != skipsurf )
    {
        return node.m_Min;
    }
    return node.m_MinOther;
}

bool MSBoundTree::MinStrength( int skipsurf, double &minstr ) const
{
    int nskip = 0;
    if ( skipsurf >= 0 && skipsurf < ( int )m_SurfCount.size() )
    {
        nskip = m_SurfCount[ skipsurf ];
    }

    if ( ( int )m_Surf.size() <= nskip )
    {
        return false;
    }

    minstr = MinExcluding( m_Nodes[0], skipsurf );
    return true;
}

// Squared distance from pt to the node's box, summed as the distance to a point is, so it is never
// more than the distance to any source in the box.
double MSBoundTree::MinDistSq( const Node &node, const double *pt ) const
{
    double result = 0.0;
    for ( int d = 0; d < 3; d++ )
    {
        double diff = 0.0;
        if ( pt[d] < node.m_Lo[d] )
        {
            diff = pt[d] - node.m_Lo[d];
        }
        else if ( pt[d] > node.m_Hi[d] )
        {
            diff = pt[d] - node.m_Hi[d];
        }
        result += diff * diff;
    }
    return result;
}

// A node is passed over when every source in it is out of reach, or none can lower t.  Once a
// source has lowered t, a node that could only equal it is still searched for a tie.
bool MSBoundTree::Prune( const Node &node, double md2, const Search &srch ) const
{
    if ( !( md2 < srch.m_R2Max ) )
    {
        return true;
    }

    double bound = MinExcluding( node, srch.m_SkipSurf ) + srch.m_Grm1 * sqrt( md2 );

    if ( srch.m_Found )
    {
        return bound > srch.m_T;
    }
    return bound >= srch.m_T;
}

void MSBoundTree::SearchNode( int inode, Search &srch ) const
{
    const Node &node = m_Nodes[inode];

    if ( node.m_Child < 0 )
    {
        for ( int k = node.m_Begin; k < node.m_End; k++ )
        {
            if ( m_Surf[k] == srch.m_SkipSurf )
            {
                continue;
            }

            const double *p = &m_Pts[ 3 * k ];
            double result = 0.0;
            for ( int d = 0; d < 3; d++ )
            {
                const double diff = srch.m_Pt[d] - p[d];
                result += diff * diff;
            }

            if ( result < srch.m_R2Max )
            {
                double r = sqrt( result );
                double ts = m_Str[k] + srch.m_Grm1 * r;

                if ( ts < srch.m_T )
                {
                    srch.m_T = ts;
                    srch.m_Reason = m_TreeSources[k]->m_reason;
                    srch.m_Found = true;
                    srch.m_Tie = false;
                    srch.m_Inside = result < srch.m_R2Inside;
                }
                else if ( srch.m_Found && ts == srch.m_T )
                {
                    if ( GrowLimitReason( m_TreeSources[k]->m_reason ) != GrowLimitReason( srch.m_Reason ) )
                    {
                        srch.m_Tie = true;
                    }
                    if ( result < srch.m_R2Inside )
                    {
                        srch.m_Inside = true;
                    }
                }
            }
        }
        return;
    }

    int ca = node.m_Child;
    int cb = node.m_Child + 1;
    double mda = MinDistSq( m_Nodes[ca], srch.m_Pt );
    double mdb = MinDistSq( m_Nodes[cb], srch.m_Pt );

    if ( mdb < mda )
    {
        std::swap( ca, cb );
        std::swap( mda, mdb );
    }

    if ( !Prune( m_Nodes[ca], mda, srch ) )
    {
        SearchNode( ca, srch );
    }
    if ( !Prune( m_Nodes[cb], mdb, srch ) )
    {
        SearchNode( cb, srch );
    }
}

bool MSBoundTree::LimitStrength( const vec3d &pt, double r2max, int skipsurf, double grm1, double &t, int &reason ) const
{
    if ( m_Nodes.empty() )
    {
        return true;
    }

    Search srch;
    for ( int d = 0; d < 3; d++ )
    {
        srch.m_Pt[d] = pt.v[d];
    }
    srch.m_R2Max = r2max;
    srch.m_R2Inside = r2max * ( 1.0 - MS_EDGE_TOL );
    srch.m_Inside = false;
    srch.m_SkipSurf = skipsurf;
    srch.m_Grm1 = grm1;
    srch.m_T = t;
    srch.m_Reason = reason;
    srch.m_Found = false;
    srch.m_Tie = false;

    const Node &root = m_Nodes[0];
    double md2 = MinDistSq( root, srch.m_Pt );
    if ( !Prune( root, md2, srch ) )
    {
        SearchNode( 0, srch );
    }

    t = srch.m_T;
    reason = srch.m_Reason;

    if ( srch.m_Tie )
    {
        return false;
    }
    if ( srch.m_Found && !srch.m_Inside )
    {
        return false;
    }
    return true;
}

void MSBoundTree::BuildCloud( int skipsurf, MSCloud &cloud ) const
{
    cloud.sources.clear();
    cloud.sources.reserve( m_Sources.size() );
    for ( int k = 0; k < ( int )m_Sources.size(); k++ )
    {
        if ( m_Sources[k]->m_surfid != skipsurf )
        {
            cloud.sources.push_back( m_Sources[k] );
        }
    }
}

MSSurfTree::MSSurfTree( const MSBoundTree &ms_tree, int skipsurf, int nthread )
{
    ms_tree.BuildCloud( skipsurf, m_Cloud );

    if ( nthread < 1 )
    {
        nthread = 1;
    }

    // The constructor builds the index.
    m_Tree = new MSTree( 3, m_Cloud, KDTreeSingleIndexAdaptorParams( 10, KDTreeSingleIndexAdaptorFlags::None, nthread ) );
}

MSSurfTree::~MSSurfTree()
{
    delete m_Tree;
}
