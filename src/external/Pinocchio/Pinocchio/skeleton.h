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

#ifndef SKELETON_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
#define SKELETON_H_BFCF2002_4190_11E9_AA8F_EFB66606E782

#include <map>
#include "graphutils.h"

namespace Pinocchio {

class PINOCCHIO_API Skeleton {
  public:
    Skeleton() {}

    const PtGraph &fGraph() const { return fGraphV; }
    const std::vector<int> &fPrev() const { return fPrevV; }
    const std::vector<int> &fSym() const { return fSymV; }

    const PtGraph &cGraph() const { return cGraphV; }
    const std::vector<int> &cPrev() const { return cPrevV; }
    const std::vector<int> &cSym() const { return cSymV; }
    const std::vector<bool> &cFeet() const { return cFeetV; }
    const std::vector<bool> &cFat() const { return cFatV; }

    const std::vector<int> &cfMap() const { return cfMapV; }
    const std::vector<int> &fcMap() const { return fcMapV; }
    const std::vector<double> &fcFraction() const { return fcFractionV; }
    const std::vector<double> &cLength() const { return cLengthV; }

    int getJointForName(const std::string &name) const { if(jointNames.count(name)) return jointNames.find(name)->second; return -1; }

    void scale(double factor);

  public:
    void initCompressed();

    // Help for creation
    std::map<std::string, int> jointNames;
    void makeJoint(const std::string &name, const Vector3 &pos, const std::string &previous = std::string());
    void makeSymmetric(const std::string &name1, const std::string &name2);
    void setFoot(const std::string &name);
    void setFat(const std::string &name);

    void makeSymmetric(const int &name1, const int &name2);
    void setFoot(const int &name);
    void setFat(const int &name);

  private:
    PtGraph fGraphV; // Full
    std::vector<int> fPrevV; // Previous vertices
    std::vector<int> fSymV; // Symmetry

    PtGraph cGraphV; // Compressed (no degree 2 vertices)
    std::vector<int> cPrevV; // Previous vertices
    std::vector<int> cSymV; // Symmetry
    std::vector<bool> cFeetV; // Whether the vertex should be near the ground
    std::vector<bool> cFatV; // Whether the vertex should be in a large region

    std::vector<int> cfMapV; // Compressed to full std::map
    std::vector<int> fcMapV; // Full to compressed std::map, -1 when vertex is not in compressed
    std::vector<double> fcFractionV; // Maps full vertex number to ratio of its prev edge length to total length of
                                     //containing edge in the compressed graph
                                     //lengths of the compressed bones
    std::vector<double> cLengthV;
};

struct PINOCCHIO_API HumanSkeleton : public Skeleton {
    HumanSkeleton();
};

struct PINOCCHIO_API QuadSkeleton : public Skeleton {
    QuadSkeleton();
};

struct PINOCCHIO_API HorseSkeleton : public Skeleton {
    HorseSkeleton();
};

struct PINOCCHIO_API CentaurSkeleton : public Skeleton {
    CentaurSkeleton();
};

struct PINOCCHIO_API FileSkeleton : public Skeleton {
    FileSkeleton(const std::string &filename);
};

struct PINOCCHIO_API DataSkeleton : public Skeleton {
    DataSkeleton();

    void init( const std::vector<Vector3> &pts, const std::vector<int> &previd );
};

} // namespace Pinocchio

#endif // SKELETON_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
