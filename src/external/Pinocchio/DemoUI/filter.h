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

#ifndef FILTER_H_EC263456_4190_11E9_9DFC_3FEA32820406
#define FILTER_H_EC263456_4190_11E9_9DFC_3FEA32820406

#include "../Pinocchio/matrix.h"
#include "../Pinocchio/vector.h"
#include "../Pinocchio/transform.h"

class MotionFilter
{
  public:
    MotionFilter(const std::vector<Vector3> &inJoints, const std::vector<int> inPrev)
      : joints(inJoints), prev(inPrev) {}

    void step(const std::vector<Pinocchio::Transform<> > &transforms, std::vector<Vector3> feet);

    const std::vector<Pinocchio::Transform<> > &getTransforms() const { return curTransforms; }

  private:
    Pinocchio::Matrixn<double> getJac(const std::vector<Pinocchio::Transform<> > &transforms) const;
    void addTranslation();

    std::vector<Vector3> joints;
    std::vector<int> prev;

    Vector3 prevTrans;
    Pinocchio::Vectorn<double> prevFeet;
    std::vector<Pinocchio::Transform<> > curTransforms;
};

#endif // FILTER_H_EC263456_4190_11E9_9DFC_3FEA32820406
