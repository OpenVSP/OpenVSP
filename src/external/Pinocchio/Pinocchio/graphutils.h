/*  This file is part of the Pinocchio automatic rigging library.
    Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef GRAPHUTILS_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
#define GRAPHUTILS_H_BFCF2002_4190_11E9_AA8F_EFB66606E782

#include <queue>
#include "vector.h"

namespace Pinocchio {

struct PtGraph {
  std::vector<Vector3> verts;
  std::vector<std::vector<int> > edges;

  bool integrityCheck() const;
};

class ShortestPather {
  public:
    ShortestPather(const PtGraph &g, int root);

    std::vector<int> pathFrom(int vtx) const {
      std::vector<int> out(1, vtx);
      while(prev[vtx] >= 0) {
        out.push_back(vtx = prev[vtx]);
      }
      return out;
    }
    double distFrom(int vtx) const { return dist[vtx]; }

  private:
    struct Inf {
      Inf(double inDist, int inNode, int inPrev) : dist(inDist), node(inNode), prev(inPrev) {}
      bool operator<(const Inf &inf) const { return dist > inf.dist; }
      double dist;
      int node, prev;
    };

    std::vector<int> prev;
    std::vector<double> dist;
};

class AllShortestPather {
  public:
    AllShortestPather() {}

    AllShortestPather(const PtGraph &g) {
      for (int i = 0; i < (int)g.verts.size(); ++i) {
        paths.push_back(ShortestPather(g, i));
      }
    }

    std::vector<int> path(int from, int to) const { return paths[to].pathFrom(from); }
    double dist(int from, int to) const { return paths[to].distFrom(from); }

  private:
    std::vector<ShortestPather> paths;
};

} // namespace Pinocchio

#endif // GRAPHUTILS_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
