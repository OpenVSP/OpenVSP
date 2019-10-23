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

#ifndef INDEXER_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
#define INDEXER_H_BFCF2002_4190_11E9_AA8F_EFB66606E782

#include "hashutils.h"
#include "vector.h"

namespace Pinocchio {

template<class Node, int Dim>
class DumbIndexer {
  public:
    typedef typename Node::Vec Vec;

    DumbIndexer() : root(NULL) {}

    void setRoot(Node *n) {
      root = n;
    }

    void preprocessIndex() {}

    Node *locate(const Vec &v) const {
      Node *out = root;
      while (out->getChild(0)) {
        Vec center = out->getChild(0)->getRect().getHi();
        out = out->getChild(BitComparator<Dim>::less(center, v));
      }
      return out;
    }

  private:
    Node *root;
};

extern PINOCCHIO_API unsigned int interLeaveLookupTable[32768];
extern PINOCCHIO_API unsigned int interLeave3LookupTable[1024];

inline unsigned int _lookup(const Vector2 &vec) {
  return interLeaveLookupTable[int(vec[0] * 32767.999)] + (interLeaveLookupTable[int(vec[1] * 32767.999)] << 1);
}


inline unsigned int _lookup(const Vector3 &vec) {
  return interLeave3LookupTable[int(vec[0] * 1023.999)] +
    (interLeave3LookupTable[int(vec[1] * 1023.999)] << 1) +
    (interLeave3LookupTable[int(vec[2] * 1023.999)] << 2);
}


template<class Node, int Dim>
class Indexer {
  public:
    typedef typename Node::Vec Vec;

    Indexer() : root(NULL) {}

    void setRoot(Node *n) {
      root = n;
    }

    void preprocessIndex() {}

    Node *locate(const Vec &v) const {
      Node *out = root;
      unsigned int idx = _lookup(v);
      static const int mask = (1 << Dim) - 1;
      while (out->getChild(idx & mask)) {
        out = out->getChild(idx & mask);
        idx = idx >> Dim;
      }
      return out;
    }
  private:
    Node *root;
};

template<class Node, int Dim>
class ArrayIndexer
{
  public:
    typedef typename Node::Vec Vec;

    ArrayIndexer() : root(NULL) {}

    void setRoot(Node *n) {
      root = n;
    }

    static const int bits = 16 - (16 % Dim);

    void preprocessIndex() {
      for (int i = 0; i < (1 << bits); ++i) {
        table[i] = root;
        int cur = i;
        int cnt = 0;
        static const int mask = (1 << Dim) - 1;
        while (table[i]->getChild(0) && cnt < (bits / Dim)) {
          ++cnt;
          table[i] = table[i]->getChild(cur & mask);
          cur = cur >> Dim;
        }
      }
    }

    Node *locate(const Vec &v) const {
      unsigned int idx = _lookup(v);

      Node *out = table[idx & ((1 << bits) - 1)];
      if (!out->getChild(0)) {
        return out;
      }

      idx = idx >> bits;
      static const int mask = (1 << Dim) - 1;
      do {
        out = out->getChild(idx & mask);
        idx = idx >> Dim;
      } while (out->getChild(idx & mask));

      return out;
    }
  private:
    Node *root;
    Node *table[(1 << bits)];
};

template<class Node, int Level>
class HashIndex
{
  public:
    HashIndex() { for (int i = 0; i < num; ++i) nodeMap[i] = make_pair(-1, (Node *)NULL); }

    void add(Node *node, unsigned int idx) {
      int idxx = (idx >> (Level - bits));
      //int idxx = idx % num;
      if (nodeMap[idxx].first == -1)
        nodeMap[idxx] = make_pair(idx, node);
    }

    Node *lookup(unsigned int idx) const
    {
      const std::pair<int, Node *> &p = nodeMap[(idx >> (Level - bits))];
      //const std::pair<int, Node *> &p = nodeMap[idx % num];
      return p.first == idx ? p.second : NULL;
    }

    static const int bits = 16;
    static const int num = (1 << bits);
    //static const int num = 75437;

  private:
    std::pair<int, Node *> nodeMap[num];
};

template<class Node, int Dim>
class HashIndexer
{
  public:
    typedef typename Node::Vec Vec;

    HashIndexer() : root(NULL) {}

    void setRoot(Node *n) {
      root = n;
    }

    static const int bits = 16;
    static const int hlev = 22;

    void preprocessIndex() {
      for (int i = 0; i < (1 << bits); ++i)
      {
        table[i] = root;
        int cur = i;
        int cnt = 0;
        while (table[i]->getChild(0) && cnt < (bits / 2)) {
          ++cnt;
          table[i] = table[i]->getChild(cur & 3);
          cur = cur >> 2;
        }
      }
      add(root, 0, 0);
    }

    Node *locate(const Vec &v) const {
      unsigned int idx = _lookup(v);

      Node *out = table[idx & ((1 << bits) - 1)];
      if (!out->getChild(0)) {
        return out;
      }

      Node *n = hNodes.lookup(idx & ((1 << hlev) - 1));
      if (!n) {
        idx = idx >> bits;
      } else {
        out = n;
        if (!out->getChild(0))
          return out;
        idx = idx >> hlev;
      }

      do {
        out = out->getChild(idx & 3);
        idx = idx >> 2;
      } while (out->getChild(idx & 3));

      return out;
    }
  private:
    unsigned int getIndex(Node *n) const {
      if (n == root) {
        return 0;
      }
      int level = getLevel(n);
      return (n->getChildIndex() << (2 * level - 2)) + getIndex(n->getParent());
    }
    int getLevel(Node *n) const {
      if (n == root) {
        return 0;
      }
      return 1 + getLevel(n->getParent());
    }

    void add(Node *cur, int level, unsigned int idx) {
      if (level == hlev) {
        hNodes.add(cur, idx);
        return;
      }
      if (cur->getChild(0) == NULL) {
        return;
      }
      for (int i = 0; i < 4; ++i) {
        add(cur->getChild(i), level + 2, idx + (i << level));
      }
    }

    Node *root;
    Node *table[(1 << bits)];
    HashIndex<Node, hlev> hNodes;
};

} // namespace Pinocchio

#endif // INDEXER_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
