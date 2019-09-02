/*
Copyright (c) 2007 Ilya Baran

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "filter.h"
#include "../Pinocchio/deriv.h"

using namespace Pinocchio;

template<class Real>
Vectorn<Real> getFeet(const std::vector<Transform<Real> > &transforms, const std::vector<Vector3> &joints,
const std::vector<int> &prev) {
  int i;
  Vectorn<Real> out;
  Vectorn<Vector<Real, 3> > results(joints.size());

  results[0] = transforms[0] * Vector<Real, 3>(joints[0]);
  for (i = 1; i < (int)joints.size(); ++i) {
    results[i] = results[prev[i]] + transforms[i - 1].getRot() * Vector<Real, 3>(joints[i] - joints[prev[i]]);
  }

  out.push_back(results[7][0]);
  out.push_back(results[7][1]);
  out.push_back(results[7][2]);
  out.push_back(results[11][0]);
  out.push_back(results[11][1]);
  out.push_back(results[11][2]);
  out.push_back(results[2][0]);
  out.push_back(results[2][1]);
  out.push_back(results[2][2]);

  return out;
}


Vectorn<double> toVector(const std::vector<Transform<> > &transforms) {
  Vectorn<double> out(3 + 4 * transforms.size());
  out[0] = transforms[0].getTrans()[0];
  out[1] = transforms[0].getTrans()[1];
  out[2] = transforms[0].getTrans()[2];

  for (int i = 0; i < (int)transforms.size(); ++i) {
    out[3 + i * 4 + 0] = transforms[i].getRot()[0];
    out[3 + i * 4 + 1] = transforms[i].getRot()[1];
    out[3 + i * 4 + 2] = transforms[i].getRot()[2];
    out[3 + i * 4 + 3] = transforms[i].getRot()[3];
  }

  return out;
}


std::vector<Transform<> > fromVector(const Vectorn<double> &v) {
  std::vector<Transform<> > out;

  int i;
  Vector3 trans0(v[0], v[1], v[2]);
  for (i = 3; i + 3 < (int)v.size(); i += 4) {
    Quaternion<> rot;
    rot.set(v[i], Vector3(v[i + 1], v[i + 2], v[i + 3]));

    if (i == 3) {
      out.push_back(Transform<>(rot, 1., trans0));
    } else {
      out.push_back(Transform<>(rot));
    }
  }

  return out;
}


Vectorn<double> adjVector(const Vectorn<double> &v, const Vectorn<double> &dirs) {
  Vectorn<double> out = v;
  for (int i = 3; i + 3 < (int)v.size(); i += 4) {
    if (Vectorn<double>(v.begin() + i, v.begin() + i + 4) *
      Vectorn<double>(dirs.begin() + i, dirs.begin() + i + 4) < 0.) {
      for (int j = i; j < i + 4; ++j) {
        out[j] = -out[j];
      }
    }
  }
  return out;
}


Matrixn<double> MotionFilter::getJac(const std::vector<Transform<> > &transforms) const {
  typedef Deriv<double, -1> D;
  std::vector<Transform<D> > transD(transforms.size());
  int i, j;

  Vectorn<double> transVec = toVector(transforms);
  Vector<D, 3> trans0;
  for (i = 0; i < 3; ++i) {
    trans0[i] = D(transVec[i], i);
  }

  for (i = 0; i < (int)transforms.size(); ++i) {
    int curIdx = 3 + i * 4;

    Quaternion<D> rot;
    rot.set(D(transVec[curIdx], curIdx), Vector<D, 3>(D(transVec[curIdx + 1], curIdx + 1),
      D(transVec[curIdx + 2], curIdx + 2),
      D(transVec[curIdx + 3], curIdx + 3)));

    if (i == 0)
      transD[i] = Transform<D>(rot, 1., trans0);
    else
      transD[i] = Transform<D>(rot);
  }

  Vectorn<D> feet = getFeet(transD, joints, prev);
  Matrixn<double> out(feet.size(), 3 + 4 * transforms.size());
  for (i = 0; i < out.getRows(); ++i) for (j = 0; j < out.getCols(); ++j) {
    out[i][j] = feet[i].getDeriv(j);
  }

  return out;
}


void MotionFilter::step(const std::vector<Transform<> > &transforms, std::vector<Vector3> feet) {
  Vectorn<double> feetV;
  int i, j;
  for (i = 0; i < (int)feet.size(); ++i)
    for (j = 0; j < 3; ++j) {
      feetV.push_back(feet[i][j]);
    }

  if (curTransforms.size() == 0) {
    prevFeet = feetV;
    prevTrans = feet.back();
    curTransforms = transforms;
    addTranslation();
    return;
  }

  if (false) {
    curTransforms = fromVector(toVector(transforms));
    addTranslation();
    return;
  }

  //flips quaternions on incoming transforms to be more like current ones
  std::vector<Transform<> > transfs = fromVector(adjVector(toVector(transforms), toVector(curTransforms)));

  Matrixn<double> jac = getJac(curTransforms);

  //regular pseudoinverse
  #if 1
  Matrixn<double> regularizer = Matrixn<double>::identity(jac.getRows(), 1e-4);
  regularizer[6][6] = regularizer[7][7] = regularizer[8][8] = 1e-2;
  Matrixn<double> jacPI = ~jac * !(jac * ~jac + regularizer);
  //svd based pseudoinverse
  #else
  Matrixn<double> u, v;
  Vectorn<double> sigma;

  sigma = getSVD(~jac, u, v);
  double minSV = 1e-6;
  for (i = 0; i < (int)sigma.size(); ++i) {
    //?
    if (fabs(sigma[i]) < minSV) {
      sigma[i] = sigma[i] > 0 ? minSV : -minSV;
    }
    sigma[i] = 1. / sigma[i];
  }

  Matrixn<double> jacPI = u * (Matrixn<double>::identity(sigma) * ~v);
  #endif

  Vectorn<double> delta = feetV - prevFeet;
  if (delta.length() > .1)
    delta = delta.normalize() * .1;

  Vectorn<double> error = (feetV - getFeet(curTransforms, joints, prev)) * 0.5;

  delta = delta + error;

  Vectorn<double> tDelta = jacPI * delta;

  Matrixn<double> projMatrix = Matrixn<double>::identity(jac.getCols()) - jacPI * jac;

  Vectorn<double> transVector = toVector(transfs) - toVector(curTransforms);
  Vector3 newTrans = 0. * (transfs[0].getTrans() - prevTrans /*- Vector3(tDelta[0], tDelta[1], tDelta[2])*/);
  transVector[0] = newTrans[0];
  transVector[1] = newTrans[1];
  transVector[2] = newTrans[2];
  tDelta = tDelta + projMatrix * transVector * 0.8;

  curTransforms = fromVector(toVector(curTransforms) + tDelta);
  addTranslation();

  prevFeet = feetV;
  prevTrans = transfs[0].getTrans();
}


void MotionFilter::addTranslation() {
  int i;
  std::vector<Vector3> pts = joints;

  pts[0] = curTransforms[0] * pts[0];
  pts[1] = curTransforms[0] * pts[1];
  for (i = 1; i < (int)curTransforms.size(); ++i) {
    int pi = prev[i + 1];

    curTransforms[i] = Transform<>(pts[pi]) * Transform<>(curTransforms[i].getRot()) * Transform<>(-joints[pi]);

    pts[i + 1] = curTransforms[i] * joints[i + 1];
  }
}
