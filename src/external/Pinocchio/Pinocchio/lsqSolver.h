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

#ifndef LSQSOLVER_H_69993332_462F_11E9_BC3D_13F24415EC5E
#define LSQSOLVER_H_69993332_462F_11E9_BC3D_13F24415EC5E

#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>

#include "mathutils.h"

namespace Pinocchio {

/**
 * Represents a factored spd matrix -- primary intended use is inside LSQSystem
 */
class LLTMatrix {
  public:
    virtual ~LLTMatrix() {}
    virtual bool solve(std::vector<double> &b) const = 0;
    virtual int size() const = 0;
};

/**
 * Represents a symmetric positive definite (spd) matrix --
 * primary intended use is inside LSQSystem (because it's symmetric, only the lower triangle
 * is stored)
 */
class SPDMatrix {
  public:
    SPDMatrix(const std::vector<std::vector<std::pair<int, double> > > &inM) : m(inM) {}
    LLTMatrix *factor() const;

  private:
    //computes a fill-reduction permutation
    std::vector<int> computePerm() const;

    //rows -- lower triangle
    std::vector<std::vector<std::pair<int, double> > > m;
};

/**
 * Sparse linear least squares solver -- with support for hard constraints
 * Intended usage:
 *    LSQSystem<V, C> s;
 *    [
 *      s.addConstraint(...); //boolean flag specifies whether soft or hard
 *    ]
 *    s.factor();
 *    [
 *      [
 *        s.setRhs(...);
 *      ]
 *      s.solve();
 *      [
 *        ... = s.getResult(...);
 *      ]
 *    ]
 * Where the stuff in brackets [] may be repeated multiple times
 */
template<class V, class C> class LSQSystem {
  public:
    LSQSystem() : factoredMatrix(NULL) {}
    ~LSQSystem() { if (factoredMatrix) delete factoredMatrix; }

    void addConstraint(bool hard, const std::map<V, double> &lhs, const C &id) {
      constraints[make_pair(id, -1)] = Constraint(hard, lhs);
    }

    void addConstraint(bool hard, double rhs, const std::map<V, double> &lhs) {
      constraints[make_pair(C(), (int)constraints.size())] = Constraint(hard, lhs, rhs);
    }

    void setRhs(const C &id, double rhs) {
      assert(constraints.count(make_pair(id, -1)));
      constraints[make_pair(id, -1)].rhs = rhs;
    }

    bool factor() {
      int i;
      typename std::map<std::pair<C, int>, Constraint>::iterator it;

      //This is a mess.  This function needs to assign indices to both variables and
      //constraints, row-reduce the hard-constraint matrix, and construct the soft constraint
      //matrix by substituting the reduced hard-constraints into the soft constraints.
      //All while keeping track of what's happening to the right hand side of the
      //system.
      //The result is somewhat difficult to follow.
      //One possible optimization (optimize factorization and especially solving in
      //the presence of large amounts of equality constraints and the like)
      //is to treat the constraints whose rhs will not change separately.
      //The way to make it prettier (should this become necessary) is to
      //make structures for the different types of indices (instead of using ints and pairs)
      //also put hard constraints in front rather than at the end.

      // Init
      varIds.clear();
      constraintMap.clear();
      substitutedHard.clear();
      rhsTransform.clear();
      softMatrix.clear();
      softNum = 0;

      // Assign indices to soft constraints
      for (it = constraints.begin(); it != constraints.end(); ++it) {
        if (!it->second.hard) {
          constraintMap[it->first] = softNum++;
        }
      }

      // Isolate hard constraints
      std::vector<std::map<V, double> > hardConstraints;
      std::vector<std::pair<C, int> > hardConstraintIds;
      int hardNum = 0;
      for (it = constraints.begin(); it != constraints.end(); ++it) {
        if (it->second.hard) {
          hardConstraints.push_back(it->second.lhs);
          hardConstraintIds.push_back(it->first);
          ++hardNum;
        }
      }

      //"substitutions[x] = (y, 3.), (z, 2.)" means "x = 3y+2z + c"
      std::map<V, std::map<V, double> > substitutions;
      // The "c" in the above equation as a lin.comb. of rhs's
      std::map<V, std::map<std::pair<C, int>, double> > substitutionsRhs;
      // Keeps track of which constraint a substitution came from
      std::map<V, int> substitutionConstraintIdx;
      std::vector<std::map<std::pair<C, int>, double> > hardRhs(hardConstraints.size());

      for (i = 0; i < (int)hardConstraints.size(); ++i) {
        hardRhs[i][hardConstraintIds[i]] = 1.;
      }

      while(hardConstraints.size()) {
        typename std::map<V, double>::iterator it;
        typename std::map<std::pair<C, int>, double>::iterator rit;
        // Find best variable and equation -- essentially pivoting
        V bestVar;
        int bestEq = -1;
        double bestVal = 0;
        for (i = 0; i < (int)hardConstraints.size(); ++i) {
          for (it = hardConstraints[i].begin(); it != hardConstraints[i].end(); ++it) {
            // Take the variable with the max absolute weight, but also heavily
            // Prefer variables with simple substitutions
            double val = fabs(it->second) / (double(hardConstraints[i].size()) - 0.9);
            if (val > bestVal) {
              bestVal = val;
              bestEq = i;
              bestVar = it->first;

              // An equality or hard assignment constraint is always good enough
              if (val > .5 && hardConstraints[i].size() <= 2) {
                // Break from the outer loop as well
                i = hardConstraints.size();
                break;
              }
            }
          }
        }

        if (bestVal < 1e-10) { // Near-singular matrix
          return false;
        }

        substitutionConstraintIdx[bestVar] = substitutions.size();
        substitutionsRhs[bestVar] = hardRhs[bestEq];
        constraintMap[hardConstraintIds[bestEq]] = softNum + substitutions.size();

        swap(hardConstraints[bestEq], hardConstraints.back());
        swap(hardConstraintIds[bestEq], hardConstraintIds.back());
        swap(hardRhs[bestEq], hardRhs.back());
        double factor = -1. / hardConstraints.back()[bestVar];
        //figure out the substitution
        std::map<V, double> &curSub = substitutions[bestVar];
        std::map<std::pair<C, int>, double> &curSubRhs = substitutionsRhs[bestVar];
        for (it = hardConstraints.back().begin(); it != hardConstraints.back().end(); ++it) {
          if (it->first != bestVar) {
            curSub[it->first] = it->second * factor;
          }
        }
        for (rit = curSubRhs.begin(); rit != curSubRhs.end(); ++rit)
          rit->second *= -factor;

        // Now substitute into the unprocessed hard constraints
        hardConstraints.pop_back();
        hardConstraintIds.pop_back();
        hardRhs.pop_back();
        for (i = 0; i < (int)hardConstraints.size(); ++i) {
          if (hardConstraints[i].count(bestVar) == 0) {
            continue;
          }
          double varWeight = hardConstraints[i][bestVar];
          hardConstraints[i].erase(bestVar);
          for (it = curSub.begin(); it != curSub.end(); ++it) {
            hardConstraints[i][it->first] += it->second * varWeight;
          }
          // And the rhs
          for (rit = curSubRhs.begin(); rit != curSubRhs.end(); ++rit) {
            hardRhs[i][rit->first] -= rit->second * varWeight;
          }
        }

        // Now substitute into the other substitutions
        typename std::map<V, std::map<V, double> >::iterator sit;
        for (sit = substitutions.begin(); sit != substitutions.end(); ++sit) {
          if (sit->second.count(bestVar) == 0) {
            continue;
          }
          double varWeight = sit->second[bestVar];
          sit->second.erase(bestVar);
          for (it = curSub.begin(); it != curSub.end(); ++it) {
            sit->second[it->first] += it->second * varWeight;
          }
          // And the rhs
          std::map<std::pair<C, int>, double> &srhs = substitutionsRhs[sit->first];
          for (rit = curSubRhs.begin(); rit != curSubRhs.end(); ++rit)
          {
            srhs[rit->first] += rit->second * varWeight;
          }
        }
      }

      // Now that we know which variables are determined by hard constraints, give indices to the rest maps variables to indices
      std::map<V, int> varMap;
      // Variables from soft constraints first
      for (it = constraints.begin(); it != constraints.end(); ++it) {
        if (it->second.hard) {
          continue;
        }
        typename std::map<V, double>::iterator it2;
        for (it2 = it->second.lhs.begin(); it2 != it->second.lhs.end(); ++it2) {
          if (varMap.count(it2->first) || substitutions.count(it2->first)) {
            continue;
          }
          varMap[it2->first] = varIds.size();
          varIds.push_back(it2->first);
        }
      }
      int softVars = varIds.size();
      //then the hard constraint variables
      varIds.resize(softVars + hardNum);
      typename std::map<V, std::map<V, double> >::iterator sit;
      for (sit = substitutions.begin(); sit != substitutions.end(); ++sit) {
        int idx = substitutionConstraintIdx[sit->first] + softVars;
        varMap[sit->first] = idx;
        varIds[idx] = sit->first;
      }

      //now compute substitutedHard -- the substitutions with respect to the indices
      substitutedHard.resize(substitutions.size());
      for (sit = substitutions.begin(); sit != substitutions.end(); ++sit) {
        typename std::map<V, double>::iterator it;
        int idx = substitutionConstraintIdx[sit->first];
        for (it = sit->second.begin(); it != sit->second.end(); ++it) {
          if (varMap.count(it->first) == 0) {
            // Variable is left free by both hard and soft constraints--bad system
            return false;
          }
          substitutedHard[idx].push_back(make_pair(varMap[it->first], it->second));
        }
      }

      //compute the softMatrix (and the rhs transform)
      //the rhsTransform matrix as a std::map
      std::vector<std::map<int, double> > rhsTransformMap(hardNum);
      softMatrix.resize(softNum);
      for (it = constraints.begin(); it != constraints.end(); ++it) {
        if (it->second.hard)
          continue;
        std::map<V, double> modLhs = it->second.lhs;
        typename std::map<V, double>::iterator it2, it3;
        int idx = constraintMap[it->first];
        for (it2 = it->second.lhs.begin(); it2 != it->second.lhs.end(); ++it2) {
          if (substitutions.count(it2->first) == 0) {
            continue;
          }
          double fac = it2->second;
          std::map<V, double> &curSub = substitutions[it2->first];
          for (it3 = curSub.begin(); it3 != curSub.end(); ++it3) {
            modLhs[it3->first] += fac * it3->second;
          }
          std::map<std::pair<C, int>, double> &curRhsSub = substitutionsRhs[it2->first];
          typename std::map<std::pair<C, int>, double>::iterator it4;
          for (it4 = curRhsSub.begin(); it4 != curRhsSub.end(); ++it4) {
            rhsTransformMap[constraintMap[it4->first] - softNum][idx] -= fac * it4->second;
          }
        }
        //write modLhs into the right form
        for (it2 = modLhs.begin(); it2 != modLhs.end(); ++it2) {
          if (substitutions.count(it2->first)) {
            continue;
          }
          softMatrix[idx].push_back(make_pair(varMap[it2->first], it2->second));
        }
        sort(softMatrix[idx].begin(), softMatrix[idx].end());
      }

      //add the rhs transforms for the hard constraints
      //and get the rhsTransform into the right form
      typename std::map<V, std::map<std::pair<C, int>, double> >::iterator rit;
      for (rit = substitutionsRhs.begin(); rit != substitutionsRhs.end(); ++rit) {
        typename std::map<std::pair<C, int>, double>::iterator it;
        int idx = substitutionConstraintIdx[rit->first] + softNum;
        for (it = rit->second.begin(); it != rit->second.end(); ++it) {
          rhsTransformMap[constraintMap[it->first] - softNum][idx] += it->second;
        }
      }

      for (i = 0; i < hardNum; ++i) {
        rhsTransform.push_back(std::vector<std::pair<int, double> >(rhsTransformMap[i].begin(),
          rhsTransformMap[i].end()));
      }

      //multiply the softMatrix by its transpose to get an SPDMatrix
      //the lower triangle of A^T * A
      std::vector<std::vector<std::pair<int, double> > > spdm;
      //the lower triangle of A^T * A as a std::map
      std::vector<std::map<int, double> > spdMap(softVars);
      for (i = 0; i < (int)softMatrix.size(); ++i) {
        for (int j = 0; j < (int)softMatrix[i].size(); ++j)
          for (int k = 0; k <= j; ++k) {
            spdMap[softMatrix[i][j].first][softMatrix[i][k].first] += softMatrix[i][j].second * softMatrix[i][k].second;
          }
      }

      for (i = 0; i < softVars; ++i) {
        spdm.push_back(std::vector<std::pair<int, double> >(spdMap[i].begin(), spdMap[i].end()));
      }

      //factor the SPDMatrix to get the LLTMatrix
      SPDMatrix spdMatrix(spdm);
      if (factoredMatrix) {
        delete factoredMatrix;
      }

      factoredMatrix = spdMatrix.factor();
      if (factoredMatrix->size() != softVars) {
        return false;
      }

      return true;
    }

    bool solve()
    {
      result.clear();
      typename std::map<std::pair<C, int>, Constraint>::const_iterator it;
      std::vector<double> rhs0, rhs1;

      //grab the rhs's of the constraints
      for (it = constraints.begin(); it != constraints.end(); ++it) {
        int idx = constraintMap[it->first];
        if ((int)rhs0.size() <= idx)
          rhs0.resize(idx + 1, 0.);
        rhs0[idx] = it->second.rhs;
      }

      rhs1 = rhs0;
      int i, j;
      for (i = softNum; i < (int)rhs1.size(); ++i)
        //for hard constraints, transform is absolute, not "additive"
        rhs1[i] = 0;
      //transform them (as per hard constraints substitution)
      for (i = 0; i < (int)rhsTransform.size(); ++i) {
        for (j = 0; j < (int)rhsTransform[i].size(); ++j) {
          rhs1[rhsTransform[i][j].first] += rhsTransform[i][j].second * rhs0[softNum + i];
        }
      }

      //multiply by A^T (as in (A^T A)^-1 x = A^T b )
      std::vector<double> rhs2(factoredMatrix->size(), 0);
      //i is row
      for (i = 0; i < (int)softMatrix.size(); ++i) {
        //softMatrix[i][j].first is column
        for (j = 0; j < (int)softMatrix[i].size(); ++j) {
          int col = softMatrix[i][j].first;
          //but the matrix is transposed :)
          rhs2[col] += softMatrix[i][j].second * rhs1[i];
        }
      }

      if (!factoredMatrix->solve(rhs2)) {
        return false;
      }

      for (i = 0; i < (int)rhs2.size(); ++i) {
        result[varIds[i]] = rhs2[i];
      }

      //now solve for the hard constraints
      int hardNum = (int)varIds.size() - (int)rhs2.size();
      for (i = 0; i < hardNum; ++i) {
        double cur = rhs1[softNum + i];
        for (j = 0; j < (int)substitutedHard[i].size(); ++j) {
          cur += substitutedHard[i][j].second * rhs2[substitutedHard[i][j].first];
        }
        result[varIds[i + rhs2.size()]] = cur;
      }

      return true;
    }

    double getResult(const V &var) const {
      assert(result.find(var) != result.end());
      return result.find(var)->second;
    }

  private:
    struct Constraint {
      Constraint() {}
      Constraint(bool inHard, const std::map<V, double> &inLhs, double inRhs = 0.)
        : hard(inHard), lhs(inLhs), rhs(inRhs) {}

      bool hard;
      std::map<V, double> lhs;
      double rhs;
    };

    std::map<std::pair<C, int>, Constraint> constraints;

    //set during solve
    std::map<V, double> result;

    //variables set during factor
    //number of soft constraints
    int softNum;
    //first the variables softly solved for, then the ones substituted
    std::vector<V> varIds;
    std::map<std::pair<C, int>, int> constraintMap;
    std::vector<std::vector<std::pair<int, double> > > substitutedHard;
    std::vector<std::vector<std::pair<int, double> > > rhsTransform;
    std::vector<std::vector<std::pair<int, double> > > softMatrix;
    LLTMatrix *factoredMatrix;
};

} // namespace Pinocchio

#endif // LSQSOLVER_H_69993332_462F_11E9_BC3D_13F24415EC5E
