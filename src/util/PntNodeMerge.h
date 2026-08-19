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

#ifndef PNTNODEMERGE_H
#define PNTNODEMERGE_H

#include "Vec3d.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include "nanoflann.hpp"

#include <vector>
using std::vector;
using std::pair;
using namespace nanoflann;

struct PntNode;
struct PntNodeCloud;

typedef KDTreeSingleIndexAdaptor< L2_Simple_Adaptor< double, PntNodeCloud > , PntNodeCloud, 3 > PNTree;
typedef vector < pair< unsigned int, double > > PNTreeResults;

struct PntNode
{
    PntNode()
    {
        m_Index = -1;
        m_UsedIndex = -1;
    }
    PntNode( const vec3d & p )
    {
        m_Index = -1;
        m_UsedIndex = -1;
        m_Pnt = p;
    }
    vec3d m_Pnt;
    long long int m_Index;
    long long int m_UsedIndex;
    vector < long long int > m_Matches;
};

// The data source fed into the KD-tree library must adhere to an interface.  The following
// struct implements that interface for the pnt kd-tree.

struct PntNodeCloud
{
    PntNodeCloud();
    ~PntNodeCloud();

    void Cleanup();

    // Underlying storage a vector.
    vector< PntNode > m_PntNodes;
    PNTree *m_index;

    long long int m_NumUsedPts;

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const
    {
        return m_PntNodes.size();
    }

    // Returns the dim'th component of the idx'th point in the class:
    inline double kdtree_get_pt( const size_t idx, int dim ) const
    {
        return m_PntNodes[idx].m_Pnt.v[dim];
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox( BBOX &bb ) const
    {
        return false;
    }

    void AddPntNodes( const vector< vec3d > & pnts );
    void ReserveMorePntNodes( long long int n );
    void AddPntNode( const vec3d & pnt );
    bool UsedNode( long long int i );
    long long int GetNodeUsedIndex( long long int i );
    long long int GetNodeBaseIndex( long long int i );
    vector < long long int > GetMatches( long long int i );

    long long int LookupPntUsed( const vec3d & pnt );
    void LookupPntBase( const vec3d & pnt, int num_results, vector < long long int > & results_vec );
    long long int LookupPntBase( const vec3d & pnt );

};

void IndexPntNodes( PntNodeCloud & cloud, double tol );

class VspSurf;

//==== Point set operations ====//
//
// The filters below carry the loops that pick points out of a set.  Each takes a flag saying which
// side of the test to keep, so a filter and its inverse are one piece of code.
//
// Coordinate tests walk the set once and want no tree.  Membership tests build one, because
// comparing two sets point by point is the thing worth avoiding.

// Points inside, or outside, an axis aligned box.
vector < vec3d > FilterPntsInBBox( const vector < vec3d > & pnts, const vec3d & min_pnt, const vec3d & max_pnt,
                                   bool keep_inside );

// Points whose coordinate along dir_index lies between low and high, or those that do not.
vector < vec3d > FilterPntsInRange( const vector < vec3d > & pnts, int dir_index, double low, double high,
                                    bool keep_inside );

// Points whose coordinate along dir_index is above val, or at or below it.
vector < vec3d > FilterPntsByValue( const vector < vec3d > & pnts, int dir_index, double val, bool keep_above );

// Points within radius of a single point, or beyond it.  One query against many, so walking the
// set beats building a tree over it.
vector < vec3d > FilterPntsNearPnt( const vector < vec3d > & pnts, const vec3d & center, double radius,
                                    bool keep_near );

// Points within tol of a surface, or beyond it.  Costs a surface projection per point rather than a
// comparison, so cut the set down with a cheaper filter first where that is possible.
vector < vec3d > FilterPntsNearSurf( const vector < vec3d > & pnts, const VspSurf * surf, double tol,
                                     bool keep_near );

//==== Point set operations built on the tree ====//
//
// These take and return plain vectors of points; the cloud they build to answer with is made and
// thrown away inside.  Callers -- the API among them -- never see a PntNodeCloud.

// Is there a point of an indexed cloud within tol of the query point?
bool AnyPntWithinTol( PntNodeCloud & cloud, const vec3d & pnt, double tol );

// Keep the points of pnts that have a point of other_pnts within tol of them, or the ones that do
// not, according to keep_matched.  O( n log m ) rather than comparing every pair.
vector < vec3d > FilterPntsByMembership( const vector < vec3d > & pnts, const vector < vec3d > & other_pnts,
                                         double tol, bool keep_matched );

// One point out of every group that falls within tol of one another.
vector < vec3d > UniquePnts( const vector < vec3d > & pnts, double tol );

// Both sets together, with one point out of every group that falls within tol of one another.
vector < vec3d > UnionPnts( const vector < vec3d > & pnts_a, const vector < vec3d > & pnts_b, double tol );

#endif
