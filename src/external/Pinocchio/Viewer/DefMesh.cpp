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

#include "DefMesh.h"
#include "Motion.h"
#include "MotionFilter.h"

#include "../Pinocchio/intersector.h"

using namespace Pinocchio;

std::vector<Transform<> > DefMesh::computeTransforms() const {
    std::vector<Transform<> > out;
    int i;

    if(motion) {
        std::vector<Transform<> > ts;
        ts = motion->get();

        double legRatio = getLegRatio();
        Vector3 trans = ts[0].getTrans() * legRatio;

        for(int times = 0; times < 2; ++times) {

            if(times == 1)
                trans += (out[0] * match[0] - out[1] * match[2]);

            out.clear();
            std::vector<Vector3> tm;
            tm.push_back(match[0]);
            ts[0] = ts[0].linearComponent();
            for(i = 0; i < (int)ts.size(); ++i)
            {
                int prevV = origSkel.fPrev()[i + 1];
                out.push_back(Transform<>(tm[prevV]) * ts[i] * Transform<>(-match[prevV]));
                tm.push_back(out.back() * match[i + 1]);
            }

            for(i = 0; i < (int)out.size(); ++i)
                out[i] = Transform<>(trans + Vector3(0.5, 0, 0.5)) * out[i];
        }

        return out;
    }

    out.push_back(Transform<>(Vector3(0.5, 0, 0.5)));

    for(i = 1; i < (int)origSkel.fPrev().size(); ++i) {
        int prevV = origSkel.fPrev()[i];
        Transform<> cur = out[prevV];
        cur = cur * Transform<>(match[prevV]) * Transform<>(transforms[i - 1]) * Transform<>(-match[prevV]);

        out.push_back(cur);
    }

    out.erase(out.begin());
    return out;
}


bool reallyDeform = true;

void DefMesh::updateMesh(int &framenum) const {
    std::vector<Transform<> > t = computeTransforms();

    if(motion) {
        if(footOffsets.empty()) {
            Intersector s(origMesh, Vector3(0, 1, 0));

            std::vector<Vector3> sects;
            double offs;

            sects = s.intersect(match[7]);
            offs = 0;
            for(int i = 0; i < (int)sects.size(); ++i)
                offs = std::max(offs, match[7][1] - sects[i][1]);
            const_cast<std::vector<double> *>(&footOffsets)->push_back(offs);

            sects = s.intersect(match[11]);
            offs = 0;
            for(int i = 0; i < (int)sects.size(); ++i)
                offs = std::max(offs, match[11][1] - sects[i][1]);
            const_cast<std::vector<double> *>(&footOffsets)->push_back(offs);
        }

        std::vector<Vector3> pose = motion->getPose(framenum);
        std::vector<Vector3> refPose = motion->getRefPose();
        std::vector<Vector3> feet;

        double legRatio = getLegRatio();
        feet.push_back(pose[15] * legRatio);
        feet.push_back(pose[7] * legRatio);

        double widthDiff = 0.3 * ((refPose[8][0] - refPose[4][0]) * legRatio - (match[7][0] - match[11][0]));
        Vector3 offs1 = t[6].getRot() * Vector3(-widthDiff, 0, 0);
        Vector3 offs2 = t[10].getRot() * Vector3(widthDiff, 0, 0);
        offs1[1] = offs2[1] = 0;
        offs1 += /*t[6].getRot() */ Vector3(0, footOffsets[0], 0);
        offs2 += /*t[10].getRot() */ Vector3(0, footOffsets[1], 0);
        feet[0] += offs1;
        feet[1] += offs2;

        Vector3 pelvisVec = (refPose[0] - 0.5 * (refPose[4] + refPose[8])) * legRatio;
        Vector3 mpelvisVec = (match[2] - 0.5 * (match[7] + match[11]));
        mpelvisVec += Vector3(0, std::min(footOffsets[0], footOffsets[1]), 0);
        Vector3 v(0, 1, 0);
        feet.push_back(pose[0] * legRatio + v * (v * (mpelvisVec - pelvisVec)));
//feet.back()[1] = 0.;

#if 0
        Debugging::clear();
        for(int i = 0; i < (int)feet.size(); ++i)
            Debugging::drawCircle(feet[i], 0.01, QPen(Qt::blue));

        Debugging::drawLine(feet[0], feet[0] - offs1, QPen(Qt::red));
        Debugging::drawLine(feet[1], feet[1] - offs2, QPen(Qt::red));
#endif

        filter.step(t, feet);
        if(reallyDeform)
            curMesh = attachment.deform(origMesh, filter.getTransforms());

#if 0
        static int period = 1;
        if(--period == 0) {
            period = 3;
            if(rand() % 40 == 0)
                Debugging::clear();
            PtGraph skelGraph = origSkel.fGraph();
            skelGraph.verts = getSkel();
            Debugging::drawGraph(skelGraph, QPen(Qt::red, 5));
        }
#endif
    }
    else
        curMesh = attachment.deform(origMesh, t);
}


std::vector<Vector3> DefMesh::getSkel() const {
    std::vector<Vector3> out = match;

    std::vector<Transform<> > t;
    if(motion)
        t = filter.getTransforms();
    else
        t = computeTransforms();

    for(int i = 0; i < (int)out.size(); ++i) {
        out[i] = t[std::max(0, i - 1)] * out[i];
    }

    return out;
}


double DefMesh::getLegRatio() const {
    double lengthRatio = fabs(match[7][1] - match[2][1]) / motion->getLegLength();
    double widthRatio = fabs(match[7][0] - match[11][0]) / motion->getLegWidth();

    return lengthRatio;
    return std::max(lengthRatio, std::min(lengthRatio * 1.4, sqrt(lengthRatio * widthRatio)));
}
