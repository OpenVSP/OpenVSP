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

#ifndef POINTPROJECTOR_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
#define POINTPROJECTOR_H_BFCF2002_4190_11E9_AA8F_EFB66606E782

#include <set>
#include <algorithm>

#include "vector.h"
#include "rect.h"
#include "vecutils.h"
#include "debugging.h"

namespace Pinocchio {

struct Vec3Object {
  Vec3Object(const Vector3 &inV) : v(inV) {}

  Rect3 boundingRect() const { return Rect3(v); }
  double operator[](int i) const { return v[i]; }
  Vector3 project(const Vector3 &) const { return v; }

  Vector3 v;
};

struct Tri3Object {
  Tri3Object(const Vector3 &inV1, const Vector3 &inV2, const Vector3 &inV3) : v1(inV1), v2(inV2), v3(inV3) {}

  Rect3 boundingRect() const { return Rect3(v1) | Rect3(v2) | Rect3(v3); }
  //for comparison only, no need to divide by 3
  double operator[](int i) const {
    return v1[i] + v2[i] + v3[i];
  }
  Vector3 project(const Vector3 &v) const { return projToTri(v, v1, v2, v3); }

  Vector3 v1, v2, v3;
};

template<int Dim, class Obj>
class ObjectProjector
{
  public:
    typedef Vector<double, Dim> Vec;
    typedef Rect<double, Dim> Rec;

    ObjectProjector() {}
    ObjectProjector(const std::vector<Obj> &inObjs) : objs(inObjs) {
      int i, d;
      std::vector<int> orders[Dim];

      for(d = 0; d < Dim; ++d) {
        orders[d].reserve(objs.size());
        for(i = 0; i < (int)objs.size(); ++i) {
          orders[d].push_back(i);
        }
        std::sort(orders[d].begin(), orders[d].end(), DLess(d, objs));
      }

      rnodes.reserve((int)objs.size() * 2 - 1);
      initHelper(orders);
    }

    Vec project(const Vec &from) const {
      double minDistSq = 1e37;
      Vec closestSoFar;

      int sz = 1;
      static std::pair<double, int> todo[10000];
      todo[0] = std::make_pair(rnodes[0].rect.distSqTo(from), 0);

      while(sz > 0) {
        if (todo[--sz].first > minDistSq) {
          continue;
        }
        // The top element that was just popped
        int cur = todo[sz].second;

        int c1 = rnodes[cur].child1;
        int c2 = rnodes[cur].child2;

        // Not a leaf
        if (c1 >= 0) {
          double l1 = rnodes[c1].rect.distSqTo(from);
          if (l1 < minDistSq) {
            todo[sz++] = std::make_pair(l1, c1);
          }

          double l2 = rnodes[c2].rect.distSqTo(from);
          if (l2 < minDistSq) {
            todo[sz++] = std::make_pair(l2, c2);
          }

          if (sz >= 2 && todo[sz - 1].first > todo[sz - 2].first) {
            swap(todo[sz - 1], todo[sz - 2]);
          }

          // Getting close to our array limit
          if (sz > 9995) {
            Debugging::out() << "Large todo list, likely to fail" << std::endl;
          }
          continue;
        }

        //leaf -- consider the object
        Vec curPt = objs[c2].project(from);
        double distSq = (from - curPt).lengthsq();
        if (distSq <= minDistSq) {
          minDistSq = distSq;
          closestSoFar = curPt;
        }
      }

      return closestSoFar;
    };

    struct RNode {
      Rec rect;
      //if child1 is -1, child2 is the object index
      int child1, child2;
    };

    const std::vector<RNode> &getRNodes() const { return rnodes; }

  private:

    struct DL {
      bool operator()(const std::pair<double, int> &p1,
        const std::pair<double, int> &p2) const { return p1.first > p2.first; }
    };

    int initHelper(std::vector<int> orders[Dim], int curDim = 0) {
      int out = rnodes.size();
      rnodes.resize(out + 1);

      int num = orders[0].size();
      if (num == 1) {
        rnodes[out].rect = objs[orders[0][0]].boundingRect();
        rnodes[out].child1 = -1;
        rnodes[out].child2 = orders[0][0];
      } else {
        int i, d;
        std::vector<int> orders1[Dim], orders2[Dim];
        std::set<int> left;
        for(i = 0; i < num / 2; ++i)
          left.insert(orders[curDim][i]);

        for(d = 0; d < Dim; ++d) {
          orders1[d].reserve((num + 1) / 2);
          orders2[d].reserve((num + 1) / 2);
          for(i = 0; i < num; ++i) {
            if (left.count(orders[d][i])) {
              orders1[d].push_back(orders[d][i]);
            } else {
              orders2[d].push_back(orders[d][i]);
            }
          }
        }

        rnodes[out].child1 = initHelper(orders1, (curDim + 1) % Dim);
        rnodes[out].child2 = initHelper(orders2, (curDim + 1) % Dim);
        rnodes[out].rect = rnodes[rnodes[out].child1].rect | rnodes[rnodes[out].child2].rect;
      }
      return out;
    }

    class DLess {
      public:
        DLess(int inDim, const std::vector<Obj> &inObjs) : dim(inDim), objs(inObjs) {}
        bool operator()(int i1, int i2) { return objs[i1][dim] < objs[i2][dim]; }
      private:
        int dim;
        const std::vector<Obj> &objs;
    };

    std::vector<RNode> rnodes;
    std::vector<Obj> objs;
};

} // namespace Pinocchio

#endif // POINTPROJECTOR_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
