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

#include "pinocchioApi.h"
#include "debugging.h"
#include <fstream>
// #include <sys/time.h>

namespace Pinocchio {

std::ostream *Debugging::outStream = new std::ofstream();

PinocchioOutput autorig(const Skeleton &given, const Mesh &m)
{
  int i;
  PinocchioOutput out;

  Mesh newMesh = prepareMesh(m);

  if(newMesh.vertices.size() == 0)
    return out;

  TreeType *distanceField = constructDistanceField(newMesh);

  //discretization
  std::vector<Sphere> medialSurface = sampleMedialSurface(distanceField);

  std::vector<Sphere> spheres = packSpheres(medialSurface);

  PtGraph graph = connectSamples(distanceField, spheres);

  //discrete embedding
  std::vector<std::vector<int> > possibilities = computePossibilities(graph,
    spheres, given);

  //constraints can be set by respecifying possibilities for
  //skeleton joints:
  //to constrain joint i to sphere j, use: possiblities[i] =
  //std::vector<int>(1, j);

  std::vector<int> embeddingIndices = discreteEmbed(graph, spheres,
    given, possibilities);

  //failure
  if(embeddingIndices.size() == 0)
  {
    delete distanceField;
    return out;
  }

  std::vector<Vector3> discreteEmbedding = splitPaths(embeddingIndices,
    graph, given);

  //continuous refinement
  std::vector<Vector3> medialCenters(medialSurface.size());
  for(i = 0; i < (int)medialSurface.size(); ++i)
    medialCenters[i] = medialSurface[i].center;

  out.embedding = refineEmbedding(distanceField, medialCenters,
    discreteEmbedding, given);

  //attachment
  VisTester<TreeType> *tester = new VisTester<TreeType>(distanceField);

  out.attachment = new Attachment(newMesh, given, out.embedding, tester);

  //cleanup
  delete tester;
  delete distanceField;

  return out;
}

} // namespace Pinocchio
