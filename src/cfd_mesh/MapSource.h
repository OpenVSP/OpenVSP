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

#include <vector>
using namespace std;

using namespace nanoflann;

struct MapSource;
struct MSCloud;

typedef KDTreeSingleIndexAdaptor< L2_Simple_Adaptor< double, MSCloud > , MSCloud, 3 > MSTree;

typedef vector< pair< unsigned int, double > > MSTreeResults;

struct MapSource
{
    MapSource()
    {
        m_str = 0;
        m_dominated = false;
        m_maxvisited = -1;
        m_surfid = -1;
    };

    MapSource( const vec3d &pt, double str, int surfid )
    {
        m_pt = pt;
        m_str = str;
        m_dominated = false;
        m_maxvisited = -1;
        m_surfid = surfid;
    };

    vec3d m_pt;
    double m_str;
    bool m_dominated;
    int m_maxvisited;
    int m_surfid;
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

#endif
