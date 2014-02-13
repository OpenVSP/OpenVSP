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
using namespace std;
using namespace nanoflann;

struct PntNode;
struct PntNodeCloud;

typedef KDTreeSingleIndexAdaptor< L2_Simple_Adaptor< double, PntNodeCloud > , PntNodeCloud, 3 > PNTree;
typedef vector< pair< size_t, double > > PNTreeResults;

struct PntNode
{
    PntNode()
    {
        m_Index = -1;
        m_UsedIndex = -1;
    }
    PntNode( vec3d & p )
    {
        m_Index = -1;
        m_UsedIndex = -1;
        m_Pnt = p;
    }
    vec3d m_Pnt;
    int m_Index;
    int m_UsedIndex;
};

// The data source fed into the KD-tree library must adhere to an interface.  The following
// struct implements that interface for the pnt kd-tree.

struct PntNodeCloud
{
    // Underlying storage a vector.
    vector< PntNode > m_PntNodes;

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const
    {
        return m_PntNodes.size();
    }

    // Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
    inline double kdtree_distance( const double *p1, const size_t idx_p2, size_t size ) const
    {
        const double d0 = p1[0] - m_PntNodes[idx_p2].m_Pnt.x();
        const double d1 = p1[1] - m_PntNodes[idx_p2].m_Pnt.y();
        const double d2 = p1[2] - m_PntNodes[idx_p2].m_Pnt.z();
        return d0 * d0 + d1 * d1 + d2 * d2;
    }

    // Returns the dim'th component of the idx'th point in the class:
    // Since this is inlined and the "dim" argument is typically an immediate value, the
    //  "if/else's" are actually solved at compile time.
    inline double kdtree_get_pt( const size_t idx, int dim ) const
    {
        if ( dim == 0 )
        {
            return m_PntNodes[idx].m_Pnt.x();
        }
        else if ( dim == 1 )
        {
            return m_PntNodes[idx].m_Pnt.y();
        }
        else
        {
            return m_PntNodes[idx].m_Pnt.z();
        }
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox( BBOX &bb ) const
    {
        return false;
    }

    void AddPntNodes( vector< vec3d > & pnts );
    bool UsedNode( int i );
    int GetNodeUsedIndex( int i );

};

void IndexPntNodes( PntNodeCloud & cloud, double tol );

#endif
