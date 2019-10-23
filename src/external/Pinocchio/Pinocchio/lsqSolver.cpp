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

#include "lsqSolver.h"

#include <queue>
#include <set>
#include <iostream>
#include <unordered_set>

#include "hashutils.h"
#include "debugging.h"

//TAUCS
#ifdef TAUCS

#include <complex>
#include <cmath>
extern "C"
{
  #include "taucs.h"
}

namespace Pinocchio {

std::vector<int> SPDMatrix::computePerm() const
{
  //do nothing
  return std::vector<int>();
}


class TaucsLLTMatrix : public LLTMatrix
{
  public:
    TaucsLLTMatrix() : m(NULL), factorization(NULL), sz(0) {}

    ~TaucsLLTMatrix()
    {
      taucs_ccs_free(m);
      taucs_linsolve(NULL, &factorization, 0, NULL, NULL, NULL, NULL);
    }

    bool solve(std::vector<double> &b) const
    {
      TaucsLLTMatrix *cthis = const_cast<TaucsLLTMatrix *>(this);
      std::vector<double> x(b.size());
      char* solve [] = {"taucs.factor=false", NULL};
      int rc = taucs_linsolve(m, &(cthis->factorization), 1, &(*x.begin()), &(*b.begin()), solve, NULL);
      if(rc != TAUCS_SUCCESS)
      {
        Debugging::out() << "Taucs error solving: rc = " << rc << std::endl;
        assert(false);
      }
      b = x;
      return true;
    }

    int size() const { return sz; }

  private:
    taucs_ccs_matrix *m;
    void *factorization;
    int sz;
    friend class SPDMatrix;
};

LLTMatrix *SPDMatrix::factor() const
{
  //taucs_logfile("stdout");
  int i, j;
  TaucsLLTMatrix *out = new TaucsLLTMatrix();
  int sz = out->sz = m.size();
  int nz = 0;
  Debugging::out() << "Size = " << sz << std::endl;

  for(i = 0; i < sz; ++i)
    nz += m[i].size();

  out->m = taucs_ccs_create(sz, sz, nz, TAUCS_DOUBLE | TAUCS_SYMMETRIC | TAUCS_LOWER);

  //transposed values
  std::vector<std::vector<std::pair<int, double> > > mt(sz);
  for(i = 0; i < sz; ++i)
  {
    for(j = 0; j < (int)m[i].size(); ++j)
    {
      mt[m[i][j].first].push_back(std::make_pair(i, m[i][j].second));
    }
  }

  out->m->colptr[0] = 0;
  for(i = 0; i < sz; ++i)
  {
    int cur = out->m->colptr[i];
    out->m->colptr[i + 1] = cur + mt[i].size();
    for(j = 0; j < (int)mt[i].size(); ++j)
    {
      if(mt[i][j].first < i)
        Debugging::out() << "Err!" << std::endl;
      out->m->rowind[cur + j] = mt[i][j].first;
      out->m->values.d[cur + j] = mt[i][j].second;
    }
  }

  //char* mf[]   = {"taucs.factor.LLT=true", "taucs.factor.mf=true", NULL};
  //char* ll[]   = {"taucs.factor.LLT=true", "taucs.factor.ll=true", NULL};
  char* def[]   = {"taucs.factor.LLT=true", NULL};

  int rc;
  rc = taucs_linsolve(out->m, &(out->factorization), 0, NULL, NULL, def, NULL);

  if(rc != TAUCS_SUCCESS)
  {
    Debugging::out() << "Taucs error factoring: rc = " << rc << std::endl;
    assert(false);
  }
  else Debugging::out() << "Factored!" << std::endl;

  return out;
}

} // namespace Pinocchio

#else

namespace Pinocchio {

class MyLLTMatrix : public LLTMatrix
{
  public:
    //solves it in place
    bool solve(std::vector<double> &b) const;
    int size() const { return m.size(); }

  private:
    void initMt();
    //off-diagonal values stored by rows
    std::vector<std::vector<std::pair<int, double> > > m;
    //off-diagonal values transposed stored by rows
    std::vector<std::vector<std::pair<int, double> > > mt;
    //values on diagonal
    std::vector<double> diag;
    //permutation
    std::vector<int> perm;

    friend class SPDMatrix;
};

std::vector<int> SPDMatrix::computePerm() const
{
  int i, j;

  std::vector<int> out;
  int sz = m.size();

  //No permutation
  #if 0
  out.resize(sz);
  for(i = 0; i < sz; ++i)
    out[i] = i;
  return out;
  random_shuffle(out.begin(), out.end());
  return out;
  #endif

  //initialize
  std::set<std::pair<int, int> > neighborSize;
  std::vector<std::unordered_set<int> > neighbors(sz);
  for(i = 0; i < sz; ++i)
  {
    for(j = 0; j < (int)m[i].size() - 1; ++j)
    {
      neighbors[i].insert(m[i][j].first);
      neighbors[m[i][j].first].insert(i);
    }
  }
  for(i = 0; i < sz; ++i)
    neighborSize.insert(std::make_pair(neighbors[i].size(), i));

  //iterate
  while(!neighborSize.empty())
  {
    //remove the neighbor of minimum degree
    int cur = (neighborSize.begin())->second;
    neighborSize.erase(neighborSize.begin());

    out.push_back(cur);

    //collect the neighbors of eliminated vertex
    std::vector<int> nb(neighbors[cur].begin(), neighbors[cur].end());
    //erase them from the neighborSize std::set because their neighborhood sizes are about
    //to change
    for(i = 0; i < (int)nb.size(); ++i)
      neighborSize.erase(std::make_pair(neighbors[nb[i]].size(), nb[i]));
    //erase the eliminated vertex from their neighbor lists
    for(i = 0; i < (int)nb.size(); ++i)
      neighbors[nb[i]].erase(neighbors[nb[i]].find(cur));
    //make the neighbors all adjacent
    for(i = 0; i < (int)nb.size(); ++i) for(j = 0; j < i; ++j)
    {
      if(neighbors[nb[i]].count(nb[j]) == 0)
      {
        neighbors[nb[i]].insert(nb[j]);
        neighbors[nb[j]].insert(nb[i]);
      }
    }
    //and put them back into the neighborSize std::set
    for(i = 0; i < (int)nb.size(); ++i)
      neighborSize.insert(std::make_pair(neighbors[nb[i]].size(), nb[i]));
  }

  std::vector<int> oout = out;
  //invert the permutation
  for(i = 0; i < sz; ++i)
    out[oout[i]] = i;

  return out;
}


LLTMatrix *SPDMatrix::factor() const
{
  int i, j, k;
  MyLLTMatrix *outP = new MyLLTMatrix();
  MyLLTMatrix &out = *outP;
  int sz = m.size();
  out.m.resize(sz);
  out.diag.resize(sz);

  Debugging::out() << "Factoring size = " << sz << std::endl;

  out.perm = computePerm();

  Debugging::out() << "Perm computed" << std::endl;

  //permute matrix according to the permuation
  std::vector<std::vector<std::pair<int, double> > > pm(sz);
  for(i = 0; i < sz; ++i)
  {
    for(j = 0; j < (int)m[i].size(); ++j)
    {
      int ni = out.perm[i], nidx = out.perm[m[i][j].first];
      if(ni >= nidx)
        pm[ni].push_back(std::make_pair(nidx, m[i][j].second));
      else
        pm[nidx].push_back(std::make_pair(ni, m[i][j].second));
    }
  }
  for(i = 0; i < sz; ++i)
    sort(pm[i].begin(), pm[i].end());

  //prepare for decomposition
  std::vector<std::vector<std::pair<int, double> > > cols(sz);
  //inverses of out.diag
  std::vector<double> dinv(sz);

  std::vector<bool> added(sz, false);

  //Sparse cholesky decomposition
  //current row
  for(i = 0; i < sz; ++i)
  {
    std::vector<int> columnsAdded;
    columnsAdded.reserve(2 * pm[i].size());

    //compute columnsAdded (nonzero indices of factor in current row)
    int inCA = 0;
    for(j = 0; j < (int)pm[i].size() - 1; ++j)
    {
      added[pm[i][j].first] = true;
      columnsAdded.push_back(pm[i][j].first);
    }
    while(inCA < (int)columnsAdded.size())
    {
      int idx = columnsAdded[inCA];
      ++inCA;
      for(k = 0; k < (int)cols[idx].size(); ++k)
      {
        int curCol = cols[idx][k].first;
        if(!added[curCol])
        {
          added[curCol] = true;
          columnsAdded.push_back(curCol);
        }
      }
    }
    sort(columnsAdded.begin(), columnsAdded.end());

    //add the columns and clear added
    for(j = 0; j < (int)columnsAdded.size(); ++j)
    {
      added[columnsAdded[j]] = false;
      cols[columnsAdded[j]].push_back(std::make_pair(i, 0.));
    }

    //initialize it with m's entries
    for(j = 0; j < (int)pm[i].size() - 1; ++j)
    {
      int curCol = pm[i][j].first;
      cols[curCol].back().second = pm[i][j].second * dinv[curCol];
    }
    //current column
    for(j = 0; j < (int)columnsAdded.size(); ++j)
    {
      int idx = columnsAdded[j];
      int csz = cols[idx].size() - 1;
      //index in column above current row -- inner loop
      for(k = 0; k < csz; ++k)
      {
        //index into current row
        int tidx = cols[idx][k].first;
        double prod = cols[idx][k].second * cols[idx].back().second * dinv[tidx];
        cols[tidx].back().second -= prod;
      }
    }
    //now diagonal
    out.diag[i] = pm[i].back().second;
    for(j = 0; j < (int)columnsAdded.size(); ++j)
    {
      double val = cols[columnsAdded[j]].back().second;
      out.diag[i] -= SQR(val);
      //also add rows to output
      out.m[i].push_back(std::make_pair(columnsAdded[j], val));
    }
    //not positive definite
    if(out.diag[i] <= 0.)
    {
      assert(false && "Not positive definite matrix (or ill-conditioned)");
      delete outP;
      return new MyLLTMatrix();
    }
    out.diag[i] = sqrt(out.diag[i]);
    dinv[i] = 1. / out.diag[i];
  }

  out.initMt();

  /* Error check
  double totErr = 0;
  for(i = 0; i < m.size(); ++i) {
      for(j = 0; j < m[i].size(); ++j) {
          int q = m[i][j].first;
          double total = -m[i][j].second;
          out.m[i].push_back(std::make_pair(i, out.diag[i]));
          if(i != q) out.m[q].push_back(std::make_pair(q, out.diag[q]));
          for(k = 0; k < out.m[i].size(); ++k) {
              for(int z = 0; z < out.m[q].size(); ++z) {
                  if(out.m[i][k].first != out.m[q][z].first)
                      continue;
                  total += out.m[i][k].second * out.m[q][z].second;
              }
          }
          out.m[i].pop_back();
          if(i != q) out.m[q].pop_back();

          totErr += fabs(total);
      }
  }
  */

  return outP;
}


//compute the transposed entries (by rows)
void MyLLTMatrix::initMt()
{
  int i, j;

  mt.clear();
  mt.resize(m.size());

  for(i = 0; i < (int)m.size(); ++i)
  {
    for(j = 0; j < (int)m[i].size(); ++j)
    {
      mt[m[i][j].first].push_back(std::make_pair(i, m[i][j].second));
    }
  }
}


bool MyLLTMatrix::solve(std::vector<double> &b) const
{
  int i, j;

  if(b.size() != m.size())
    return false;

  std::vector<double> bp(b.size());
  //permute
  for(i = 0; i < (int)b.size(); ++i)
    bp[perm[i]] = b[i];

  //solve L (L^T x) = b for (L^T x)
  for(i = 0; i < (int)bp.size(); ++i)
  {
    for(j = 0; j < (int)m[i].size(); ++j)
      bp[i] -= bp[m[i][j].first] * m[i][j].second;
    bp[i] /= diag[i];
  }

  //solve L^T x = b for x
  for(i = bp.size() - 1; i >= 0; --i)
  {
    for(j = 0; j < (int)mt[i].size(); ++j)
      bp[i] -= bp[mt[i][j].first] * mt[i][j].second;
    bp[i] /= diag[i];
  }

  //unpermute
  for(i = 0; i < (int)b.size(); ++i)
    b[i] = bp[perm[i]];

  return true;
}

} // namespace Pinocchio

#endif
