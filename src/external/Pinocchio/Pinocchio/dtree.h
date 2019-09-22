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

#ifndef DTREE_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
#define DTREE_H_BFCF2002_4190_11E9_AA8F_EFB66606E782

#include "rect.h"
#include "indexer.h"
#include <functional>

namespace Pinocchio {

template<class Data, int Dim>
class DNode : public Data {
  public:
    typedef DNode<Data, Dim> Self;
    typedef Vector<double, Dim> Vec;
    typedef Rect<double, Dim> MyRect;

    int countNodes() const {
      int nodes = 1;
      if (children[0] != NULL) {
        for (int i = 0; i < numChildren; ++i) {
          nodes += children[i]->countNodes();
        }
      }
      return nodes;
    }

    int maxLevel() const {
      if (children[0] == NULL) {
        return 0;
      }
      int ml = 0;
      for (int i = 0; i < numChildren; ++i) {
        ml = std::max(ml, children[i]->maxLevel());
      }
      return 1 + ml;
    }

    Self *getParent() const { return parent; }
    Self *getChild(int idx) const { return children[idx]; }
    const MyRect &getRect() const { return rect; }
    int getChildIndex() const { return childIndex; }

    static const int numChildren = 1 << Dim;

  private:
    DNode(MyRect r) : Data(this), parent(NULL), rect(r) {
      zeroChildren();
      Data::init();
    }

    DNode(Self *inParent, int inChildIndex) : Data(this), parent(inParent), childIndex(inChildIndex) {
      zeroChildren();
      rect = MyRect(inParent->rect.getCorner(childIndex)) | MyRect(inParent->rect.getCenter());
      Data::init();
    }

    ~DNode() {
      for (int i = 0; i < numChildren; ++i) {
        if (children[i]) {
          delete children[i];
        }
      }
    }

    void split() {
      for (int i = 0; i < numChildren; ++i) {
        children[i] = new Self(this, i);
      }
    }

    template<class D, int DI, template<typename N, int ID> class IDX> friend class DRootNode;

    void zeroChildren() { for (int i = 0; i < numChildren; ++i) children[i] = NULL; }

    //data
    Self *parent;
    Self *children[numChildren];
    int childIndex;

    MyRect rect;
};

template<class Data, int Dim, template<typename Node, int IDim> class Indexer = Pinocchio::DumbIndexer>
class DRootNode : public DNode<Data, Dim>, public Indexer<DNode<Data, Dim>, Dim>
{
  public:
    typedef DNode<Data, Dim> Node;
    typedef DRootNode<Data, Dim, Indexer> Self;
    typedef Indexer<Node, Dim> MyIndexer;
    typedef Vector<double, Dim> Vec;
    typedef Rect<double, Dim> MyRect;

    DRootNode(MyRect r = MyRect(Vec(), Vec().apply(std::bind(std::plus<double>(), std::placeholders::_1, 1.)))) : Node(r) {
      MyIndexer::setRoot(this);
    }

    void split(Node *node) {
      node->split();
    }
};

} // namespace Pinocchio

#endif // DTREE_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
