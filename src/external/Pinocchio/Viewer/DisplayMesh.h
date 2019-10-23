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

#ifndef DISPLAYMESH_H_A392C6AE_465A_11E9_B3CD_87B0B67BD348
#define DISPLAYMESH_H_A392C6AE_465A_11E9_B3CD_87B0B67BD348

#include "../Pinocchio/pin_mesh.h"

class DisplayMesh {
    public:
        virtual ~DisplayMesh() {}

        virtual const Pinocchio::Mesh &getMesh(int &framenum) = 0;
        virtual std::vector<Vector3> getSkel() const { return std::vector<Vector3>(); }
};

class StaticDisplayMesh : public DisplayMesh {
    public:
        StaticDisplayMesh(const Pinocchio::Mesh &inM) : m(inM) {}

        virtual const Pinocchio::Mesh &getMesh(int &framenum) { return m; }
    private:

        Pinocchio::Mesh m;
};
#endif // DISPLAYMESH_H_A392C6AE_465A_11E9_B3CD_87B0B67BD348
