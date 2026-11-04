//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Map Source and KD-Tree
//
//   Rob McDonald 4/4/12
//
//******************************************************************************

#ifndef MAPSOURCE_H
#define MAPSOURCE_H

#include "Vec3d.h"
#include "GridDensity.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include "nanoflann.hpp"

#include <map>
#include <vector>
using namespace std;

using namespace nanoflann;

struct MapSource;
struct MSCloud;

typedef KDTreeSingleIndexAdaptor< L2_Simple_Adaptor< double, MSCloud > , MSCloud, 3 > MSTree;

// The reason given a strength limited by growth from a source with the given reason.
inline int GrowLimitReason( int reason )
{
    if ( reason < vsp::MIN_GROW_LIMIT )
    {
        return reason + vsp::GROW_LIMIT_INCREMENT;
    }
    return reason;
}

struct MapSource
{
    MapSource()
    {
        m_str = 0;
        m_dominated = false;
        m_maxvisited = -1;
        m_surfid = -1;
        m_reason = -1;
    };

    MapSource( const vec3d &pt, double str, int surfid, int reason )
    {
        m_pt = pt;
        m_str = str;
        m_dominated = false;
        m_maxvisited = -1;
        m_surfid = surfid;
        m_reason = reason;
    };

    vec3d m_pt;
    double m_str;
    bool m_dominated;
    int m_maxvisited;
    int m_surfid;
    int m_reason;
};


// The data source fed into the KD-tree library must adhere to an interface.  The following
// struct implements that interface for the edge source kd-tree.

struct MSCloud
{
    // Underlying storage a vector.
    vector< MapSource* > sources;

    double str;
    double deltaT;
    double grm1;

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const
    {
        return sources.size();
    }

    // Returns the dim'th component of the idx'th point in the class:
    inline double kdtree_get_pt( const size_t idx, int dim ) const
    {
        return sources[idx]->m_pt.v[dim];
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox( BBOX &bb ) const
    {
        return false;
    }
};

// A k-d tree over every map source for rigorous growth limiting.  Each node also holds the
// smallest strength among its sources, the surface that strength is from, and the smallest
// strength from any other surface.  A search for the smallest strength grown out to a point can
// then pass over every node that cannot beat the best found so far, and can leave out the sources
// of the surface being limited without a tree of its own.

class MSBoundTree
{
public:

    // Built on up to nthread threads.
    void Build( const vector< MapSource* > &sources, int nthread );

    // Takes up the current strengths of the sources.
    void UpdateStrengths();

    // Takes up the current strengths of one surface's sources.
    void UpdateSurface( int isurf );

    // The smallest strength of a source not on surface skipsurf.  False when there is none.
    bool MinStrength( int skipsurf, double &minstr ) const;

    // Lowers t to the smallest str + grm1 * r over the sources not on skipsurf within r2max of pt,
    // with the reason and squared distance of a source that gives it.
    void LimitStrength( const vec3d &pt, double r2max, int skipsurf, double grm1, double &t, int &reason, double &r2 ) const;

private:

    struct Node
    {
        double m_Lo[3];
        double m_Hi[3];
        int m_Child;        // First of two children, or -1 for a leaf.
        int m_Begin;
        int m_End;
        double m_Min;
        int m_MinSurf;
        double m_MinOther;  // Smallest strength from a surface other than m_MinSurf.
    };

    struct Search
    {
        double m_Pt[3];
        double m_R2Max;
        int m_SkipSurf;
        double m_Grm1;
        double m_T;
        int m_Reason;
        double m_R2;
    };

    int CountDescendants( int nsrc );
    int Descendants( int nsrc ) const;
    void BuildNode( int inode, int begin, int end, int ifirst, int nsplit );
    void UpdateNode( int inode );
    double MinDistSq( const Node &node, const double *pt ) const;
    double MinExcluding( const Node &node, int skipsurf ) const;
    bool Prune( const Node &node, double md2, const Search &srch ) const;
    void SearchNode( int inode, Search &srch ) const;

    vector< Node > m_Nodes;
    vector< MapSource* > m_TreeSources; // In tree order.
    vector< int > m_TreeIndex;          // Order given, in tree order, while building.
    vector< double > m_GivenPts;        // xyz in the order given, while building.
    map< int, int > m_Descendants;      // Nodes below a node of so many sources.
    vector< double > m_Pts;             // xyz in tree order.
    vector< double > m_Str;             // In tree order.
    vector< int > m_Surf;               // In tree order.
    vector< int > m_SurfCount;          // Sources on each surface.
    vector< int > m_Parent;
    vector< int > m_SurfLeaves;         // Leaves holding each surface's sources,
    vector< int > m_SurfLeafStart;      // from m_SurfLeafStart[isurf] up to m_SurfLeafStart[isurf + 1].
    vector< bool > m_NodeDirty;
    vector< int > m_Dirty;
};

#endif
