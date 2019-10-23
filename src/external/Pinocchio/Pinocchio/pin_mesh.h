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

#ifndef MESH_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
#define MESH_H_BFCF2002_4190_11E9_AA8F_EFB66606E782

#include <vector>
#include <string>

#include "vector.h"
#include "rect.h"

namespace Pinocchio {

struct MeshVertex {
  MeshVertex() : edge(-1), origVertID(-999999999) {}

  Vector3 pos;
  Vector3 normal;
  int edge; // An edge such that edge->prev->vertex is this
  int origVertID;
};

struct MeshEdge {
  MeshEdge() : vertex(-1), prev(-1), twin(-1) {}

  int vertex; // The vertex the edge points to--the start vertex is prev->vertex
  int prev; // CCW, next is prev->prev
  int twin;
};

class PINOCCHIO_API Mesh {
  public:
    Mesh() : scale(1.) {}
    Mesh(const std::string &file, int algo=Mesh::LBS, float weight=1.);

    bool integrityCheck() const;
    bool isConnected() const; // Returns true if the mesh consists of a single connected component
    void computeVertexNormals();
    void normalizeBoundingBox();
    void computeTopology();
    void writeObj(const std::string &filename) const;
    void fixDupFaces();

  private:
    void readObj(std::istream &strm);
    void readOff(std::istream &strm);
    void readPly(std::istream &strm);
    void readGts(std::istream &strm);
    void readStl(std::istream &strm);
    void sortEdges(); // Sort edges so that triplets forming faces are adjacent

  public:
    std::vector<MeshVertex> vertices;
    std::vector<MeshEdge> edges; // Half-Edges, really

    Vector3 toAdd;
    double scale;
    float blendWeight;
    int algo;
    static int LBS;
    static int DQS;
    static int MIX;
};

} // namespace Pinocchio

#endif // MESH_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
