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

#ifndef QUADDISTTREE_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
#define QUADDISTTREE_H_BFCF2002_4190_11E9_AA8F_EFB66606E782

#include "hashutils.h"
#include "dtree.h"
#include "multilinear.h"
#include "intersector.h"
#include "pointprojector.h"
#include <numeric>
#include <map>
#include <unordered_map>

namespace Pinocchio {

template<int Dim>
class DistFunction : public Multilinear<double, Dim>
{
  typedef Multilinear<double, Dim> super;
  typedef Rect<double, Dim> MyRect;
  public:
    template<class Eval> void initFunc(const Eval &eval, const MyRect &rect)
    {
      for(int i = 0; i < (1 << Dim); ++i)
      {
        this->setValue(i, eval(rect.getCorner(i)));
      }
      return;
    }

  private:
};

template<int Dim>
class DistData : public DistFunction<Dim>
{
  public:
    typedef DistFunction<Dim> super;
    typedef DNode<DistData<Dim>, Dim> NodeType;

    DistData(NodeType *inNode) : node(inNode) {}

    void init() { }

    template<class Eval, template<typename Node, int IDim> class Indexer>
      void fullSplit(const Eval &eval, double tol, DRootNode<DistData<Dim>, Dim, Indexer> *rootNode, int level = 0, bool cropOutside = false)
    {
      int i;
      const Rect<double, Dim> &rect = node->getRect();
      node->initFunc(eval, rect);

      bool nextCropOutside = cropOutside;
      if(cropOutside && level > 0)
      {
        double center = eval(rect.getCenter());
        double len = rect.getSize().length() * 0.5;
        if(center > len)
          return;
        if(center < -len)
          nextCropOutside = false;
      }

      if(level == (32 / Dim))
        return;
      bool doSplit = false;
      if(node->getParent() == NULL)
        doSplit = true;
      if(!doSplit)
      {
        int idx[Dim + 1];
        for(i = 0; i < Dim + 1; ++i)
          idx[i] = 0;
        Vector<double, Dim> center = rect.getCenter();
        while(idx[Dim] == 0)
        {
          Vector<double, Dim> cur;
          bool anyMid = false;
          for(i = 0; i < Dim; ++i)
          {
            switch(idx[i])
            {
              case 0: cur[i] = rect.getLo()[i]; break;
              case 1: cur[i] = rect.getHi()[i]; break;
              case 2: cur[i] = center[i]; anyMid = true; break;
            }
          }
          if(anyMid && fabs(evaluate(cur) - eval(cur)) > tol)
          {
            doSplit = true;
            break;
          }
          for(i = 0; i < Dim + 1; ++i)
          {
            if(idx[i] != 2)
            {
              idx[i] += 1;
              for(--i; i >= 0; --i)
                idx[i] = 0;
              break;
            }
          }
        }
      }
      if(!doSplit)
        return;
      rootNode->split(node);
      for(i = 0; i < NodeType::numChildren; ++i)
      {
        eval.setRect(Rect<double, Dim>(rect.getCorner(i)) | Rect<double, Dim>(rect.getCenter()));
        node->getChild(i)->fullSplit(eval, tol, rootNode, level + 1, nextCropOutside);
      }
    }

    template<class Real> Real evaluate(const Vector<Real, Dim> &v)
    {
      if(node->getChild(0) == NULL)
      {
        return super::evaluate((v - node->getRect().getLo()).apply(std::divides<Real>(),
          node->getRect().getSize()));
      }
      Vector<Real, Dim> center = node->getRect().getCenter();
      int idx = 0;
      for(int i = 0; i < Dim; ++i)
        if(v[i] > center[i])
          idx += (1 << i);
      return node->getChild(idx)->evaluate(v);
    }

    template<class Real> Real integrate(Rect<Real, Dim> r)
    {
      r &= Rect<Real, Dim>(node->getRect());
      if(r.isEmpty())
        return Real();
      if(node->getChild(0) == NULL)
      {
        Vector<Real, Dim> corner = node->getRect().getLo(), size = node->getRect().getSize();
        Rect<Real, Dim> adjRect((r.getLo() - corner).apply(std::divides<Real>(), size),
          (r.getHi() - corner).apply(std::divides<Real>(), size));
        return Real(node->getRect().getContent()) * super::integrate(adjRect);
      }
      Real out = Real();
      for(int i = 0; i < NodeType::numChildren; ++i)
        out += node->getChild(i)->integrate(r);
      return out;
    }

  private:
    NodeType *node;
};

typedef DistData<3>::NodeType OctTreeNode;
typedef DRootNode<DistData<3>, 3> OctTreeRoot;

template<class RootNode = OctTreeRoot> class OctTreeMaker
{
  public:
    static RootNode *make(const ObjectProjector<3, Tri3Object> &proj, const Mesh &m, double tol)
    {
      DistObjEval eval(proj, m);
      RootNode *out = new RootNode();

      out->fullSplit(eval, tol, out, 0, true);
      out->preprocessIndex();

      return out;
    }

    static RootNode *make(const ObjectProjector<3, Vec3Object> &proj, double tol, const RootNode *dTree = NULL)
    {
      PointObjDistEval eval(proj, dTree);
      RootNode *out = new RootNode();

      out->fullSplit(eval, tol, out);
      out->preprocessIndex();

      return out;
    }

  private:
    class DistObjEval
    {
      public:
        DistObjEval(const ObjectProjector<3, Tri3Object> &inProj, const Mesh &m) : proj(inProj), mint(m, Vector3(1, 0, 0))
        {
          level = 0;
          rects[0] = Rect3(Vector3(), Vector3(1.));
          inside[0] = 0;
        }

        double operator()(const Vector3 &vec) const
        {
          unsigned int cur = ROUND(vec[0] * 1023.) + 1024 * (ROUND(vec[1] * 1023.) + 1024 * ROUND(vec[2] * 1023.));
          unsigned int sz = cache.size();
          double &d = cache[cur];
          if(sz == cache.size())
            return d;
          return d = compute(vec);
        }

        void setRect(const Rect3 &r) const
        {
          while(!(rects[level].contains(r.getCenter()))) --level;

          inside[level + 1] = inside[level];
          ++level;
          if(!inside[level])
          {
            double d = (*this)(r.getCenter());
            double diag2 = r.getSize().length() * 0.5;

            if(d >= diag2)
              inside[level] = 1;
            else if(d <= -diag2)
              inside[level] = -1;
          }

          rects[level] = r;
        }

      private:
        double compute(const Vector3 &vec) const
        {
          int i, ins = inside[level];
          if(!ins)
          {
            ins = 1;
            std::vector<Vector3> isecs = mint.intersect(vec);
            for(i = 0; i < (int)isecs.size(); ++i)
            {
              if(isecs[i][0] > vec[0])
                ins = -ins;
            }
          }

          return (vec - proj.project(vec)).length() * ins;
        }

        mutable std::map<unsigned int, double> cache;
        const ObjectProjector<3, Tri3Object> &proj;
        Intersector mint;
        mutable Rect3 rects[11];
        mutable int inside[11];
        //essentially index of last rect
        mutable int level;
    };

    class PointObjDistEval
    {
      public:
        PointObjDistEval(const ObjectProjector<3, Vec3Object> &inProj, const RootNode *inDTree) : proj(inProj), dTree(inDTree) {}

        double operator()(const Vector3 &vec) const
        {
          unsigned int cur = ROUND(vec[0] * 1023.) + 1024 * (ROUND(vec[1] * 1023.) + 1024 * ROUND(vec[2] * 1023.));
          unsigned int sz = cache.size();
          double &d = cache[cur];
          if(sz == cache.size())
            return d;
          return d = (vec - proj.project(vec)).length();
        }

        void setRect(const Rect3 &r) const { }

      private:
        mutable std::unordered_map<unsigned int, double> cache;
        const ObjectProjector<3, Vec3Object> &proj;
        const RootNode *dTree;
    };
};

} // namespace Pinocchio

#endif // QUADDISTTREE_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
