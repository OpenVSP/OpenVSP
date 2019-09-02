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

#ifndef ATTACHMENT_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
#define ATTACHMENT_H_BFCF2002_4190_11E9_AA8F_EFB66606E782

#include "pin_mesh.h"
#include "skeleton.h"
#include "transform.h"
#include "quatinterface.h"

namespace Pinocchio {

class VisibilityTester {
  public:
    virtual ~VisibilityTester() {}
    virtual bool canSee(const Vector3 &v1, const Vector3 &v2) const = 0;
};

template<class T>
class VisTester : public VisibilityTester {
  public:
    VisTester(const T *t) : tree(t) {}

    //faster when v2 is farther inside than v1
    virtual bool canSee(const Vector3 &v1, const Vector3 &v2) const {
      const double maxVal = 0.002;
      double atV2 = tree->locate(v2)->evaluate(v2);
      double left = (v2 - v1).length();
      double leftInc = left / 100.;
      Vector3 diff = (v2 - v1) / 100.;
      Vector3 cur = v1 + diff;
      while(left >= 0.) {
        double curDist = tree->locate(cur)->evaluate(cur);
        if(curDist > maxVal) {
          return false;
        }
        //if curDist and atV2 are so negative that distance won't reach above maxVal, return true
        if(curDist + atV2 + left <= maxVal) {
          return true;
        }
        cur += diff;
        left -= leftInc;
      }
      return true;
    }

  private:
    const T *tree;
};

//be sure to delete afterwards
template<class T>
VisibilityTester *makeVisibilityTester(const T *tree) {
  return new VisTester<T>(tree);
}


class AttachmentPrivate;

class PINOCCHIO_API Attachment {
  public:
    Attachment() : a(NULL) {}
    Attachment(const Attachment &);
    Attachment(const Mesh &mesh, const Skeleton &skeleton, const std::vector<Vector3> &match, const VisibilityTester *tester, double initialHeatWeight=1.);

    virtual ~Attachment();

    Mesh deform(const Mesh &mesh, const std::vector<Transform<> > &transforms) const;
    Mesh mixedBlend(const Mesh &mesh, const std::vector<Transform<> > &transforms) const;
    Mesh linearBlend(const Mesh &mesh, const std::vector<Transform<> > &transforms) const;
    Mesh dualQuaternion(const Mesh &mesh, const std::vector<Transform<> > &transforms) const;
    Vector<double, -1> getWeights(int i) const;
  private:
    AttachmentPrivate *a;
};

} // namespace Pinocchio

#endif // ATTACHMENT_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
