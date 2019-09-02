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

#ifndef MOTION_H_CCB0E12A_4190_11E9_BF66_EB91561F8FE5
#define MOTION_H_CCB0E12A_4190_11E9_BF66_EB91561F8FE5

#include "../Pinocchio/transform.h"
#include "shared.h"

class Motion
{
  public:
    Motion(const std::string &filename);

    bool empty() const { return data.empty(); }
    std::vector<Pinocchio::Transform<> > get() const;
    std::vector<Vector3> getPose(int &framenum) const;
    std::vector<Vector3> getRefPose() const { return refPose; }
    double getLegLength() const { return legLength; }
    double getLegWidth() const { return legWidth; }

    const std::vector<std::vector<Pinocchio::Transform<> > > &getData() const { return data; }
    void setFixedFrame(int inFrame) { fixedFrame = inFrame < 0 ? -1 : (int)(inFrame % data.size()); }
  private:
    int getFrameIdx() const;
    void readH(std::istream &strm);
    std::vector<std::vector<Pinocchio::Transform<> > > data;
    std::vector<std::vector<Vector3> > poses;
    std::vector<Vector3> refPose;
    double legLength;
    double legWidth;
    int fixedFrame;
};

// Time functions
long getT();

#endif // MOTION_H_CCB0E12A_4190_11E9_BF66_EB91561F8FE5
