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

#ifndef PINOCCHIOAPI_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
#define PINOCCHIOAPI_H_BFCF2002_4190_11E9_AA8F_EFB66606E782

#include "pin_mesh.h"
#include "quaddisttree.h"
#include "attachment.h"

namespace Pinocchio {

struct PinocchioOutput {
  PinocchioOutput() : attachment(NULL) {}

  std::vector<Vector3> embedding;
  //user responsible for deletion
  Attachment *attachment;
};

//calls the other functions and does the whole rigging process
//see the implementation of this function to find out how to use the individual functions
PinocchioOutput PINOCCHIO_API autorig(const Skeleton &given, const Mesh &m);

//============================================individual steps=====================================

//fits mesh inside unit cube, makes sure there's exactly one connected component
Mesh PINOCCHIO_API prepareMesh(const Mesh &m);

//our distance field octree type
typedef DRootNode<DistData<3>, 3, ArrayIndexer> TreeType;
static const double defaultTreeTol = 0.003;

//constructs a distance field on an octree--user responsible for deleting output
TreeType PINOCCHIO_API *constructDistanceField(const Mesh &m, double tol = defaultTreeTol);

struct Sphere {
  Sphere() : radius(0.) {}
  Sphere(const Vector3 &inC, double inR) : center(inC), radius(inR) {}

  Vector3 center;
  double radius;
};

//samples the distance field to find spheres on the medial surface
//output is sorted by radius in decreasing order
std::vector<Sphere> PINOCCHIO_API sampleMedialSurface(TreeType *distanceField, double tol = defaultTreeTol);

//takes sorted medial surface samples and sparsifies the std::vector
std::vector<Sphere> PINOCCHIO_API packSpheres(const std::vector<Sphere> &samples, int maxSpheres = 1000);

//constructs graph on packed sphere centers
PtGraph PINOCCHIO_API connectSamples(TreeType *distanceField, const std::vector<Sphere> &spheres);

//finds which joints can be embedded into which sphere centers
std::vector<std::vector<int> > PINOCCHIO_API computePossibilities(const PtGraph &graph, const std::vector<Sphere> &spheres,
const Skeleton &skeleton);

//finds discrete embedding
std::vector<int> PINOCCHIO_API discreteEmbed(const PtGraph &graph, const std::vector<Sphere> &spheres,
const Skeleton &skeleton, const std::vector<std::vector<int> > &possibilities);

//reinserts joints for unreduced skeleton
std::vector<Vector3> PINOCCHIO_API splitPaths(const std::vector<int> &discreteEmbedding, const PtGraph &graph,
const Skeleton &skeleton);

//refines embedding
std::vector<Vector3> PINOCCHIO_API refineEmbedding(TreeType *distanceField, const std::vector<Vector3> &medialSurface,
const std::vector<Vector3> &initialEmbedding, const Skeleton &skeleton);

//to compute the attachment, create a new Attachment object

} // namespace Pinocchio

#endif // PINOCCHIOAPI_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
