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

#ifndef MODEL_H_A392C730_465A_11E9_B3CE_47F0D82200CA
#define MODEL_H_A392C730_465A_11E9_B3CE_47F0D82200CA

#include "DisplayMesh.h"

#include "../Pinocchio/pinocchioApi.h"

class AnimatedModel {
    protected:
        struct LineSegment {
            LineSegment() : thickness(1.) {}
            LineSegment(const Vector3 &inP1, const Vector3 &inP2,
                const Vector3 &inColor = Vector3(1, 1, 1), double inThickness = 1.)
                : p1(inP1), p2(inP2), color(inColor), thickness(inThickness) {}

            Vector3 p1, p2, color;
            double thickness;
        };

        void addMesh(DisplayMesh *inM) {
            meshes.push_back(inM);
        }

        void addLine(const LineSegment &l) {
            lines.push_back(l);
        }

        void clearLines() {
            lines.clear();
        }

    public:
        AnimatedModel() : flatShading(false) {}

        void loadObject(std::string obj_filename, std::string motion_filenamename);
        void drawModel();

    private:
        void drawMesh(const Pinocchio::Mesh &m, bool flatShading, Vector3 trans = Vector3());
        void drawFloor(bool flatShading);

        bool flatShading;

        Pinocchio::Transform<> transform;
        std::vector<DisplayMesh *> meshes;
        std::vector<LineSegment> lines;
};

#endif // MODEL_H_A392C730_465A_11E9_B3CE_47F0D82200CA
