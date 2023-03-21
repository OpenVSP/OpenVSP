//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//
// Created by Rob McDonald on 3/20/23.
//

#ifndef TWODNN_H
#define TWODNN_H

#include <vector>

#include "Vec2d.h"

#include "nanoflann.hpp"

using namespace std;
using namespace nanoflann;

struct TwoDNode;
struct TwoDNodeCloud;

typedef KDTreeSingleIndexAdaptor< L2_Simple_Adaptor< double, TwoDNodeCloud > , TwoDNodeCloud, 2 > TwoDTree;

struct TwoDNode
{
    TwoDNode( vec2d uv, double iu, double iv )
    {
        m_UV = uv;
        m_iU = iu;
        m_iV = iv;
    }

    vec2d m_UV;
    int m_iU;
    int m_iV;
};

// The data source fed into the KD-tree library must adhere to an interface.  The following
// struct implements that interface for the pnt kd-tree.

struct TwoDNodeCloud
{
    TwoDNodeCloud();
    ~TwoDNodeCloud();

    void Cleanup();

    // Underlying storage a vector.
    vector< TwoDNode > m_PntNodes;
    TwoDTree *m_index;

    long long int m_NumUsedPts;

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const
    {
        return m_PntNodes.size();
    }

    // Returns the dim'th component of the idx'th point in the class:
    inline double kdtree_get_pt( const size_t idx, int dim ) const
    {
        return m_PntNodes[idx].m_UV.v[dim];
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox( BBOX &bb ) const
    {
        return false;
    }

    void AddPntNodes( const vector< vector< vec2d > > & pnts );

    long long int LookupPnt( const vec2d & pnt );

    void BuildIndex();
};

#endif //TWODNN_H
