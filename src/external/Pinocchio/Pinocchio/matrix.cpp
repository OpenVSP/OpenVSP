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

#include "matrix.h"
#include "debugging.h"

namespace Pinocchio {

namespace EigPrivate
{
  std::pair<int, double> findMaxAbs(const Vectorn<double> &vec, int maxElem)
  {
    std::pair<int, double> out = std::make_pair(0, fabs(vec[0]));
    for(int i = 1; i < maxElem; ++i)
      if(vec[i] > out.second)
        out = std::make_pair(i, fabs(vec[i]));
    return out;
  }

  void jacobi(int row, int col, Matrixn<double> &m, Matrixn<double> *vectors)
  {
    double num = m[row][row] - m[col][col];
    double dem = 2. * m[row][col];

    double theta = num / dem;
    double t;
    if(fabs(theta) > 1e20)
    {
      t = m[row][col] / num;
    }
    else
    {
      t = (theta > 0 ? 1. : -1.) / (fabs(theta) + sqrt(theta * theta + 1.));
    }

    double c = 1. / sqrt(t * t + 1.);
    double s = t * c;

    double tau = s / (1. + c);

    //rotation
    m[col][col] -= t * m[row][col];
    m[row][row] += t * m[row][col];

    int i;
    for(i = 0; i < col; ++i)
    {
      double oldCI = m[col][i];
      m[col][i] -= s * (m[row][i] + tau * m[col][i]);
      m[row][i] += s * (oldCI - tau * m[row][i]);
    }
    for(i = col + 1; i < row; ++i)
    {
      double oldIC = m[i][col];
      m[i][col] -= s * (m[row][i] + tau * m[i][col]);
      m[row][i] += s * (oldIC - tau * m[row][i]);
    }
    for(i = row + 1; i < m.getRows(); ++i)
    {
      double oldIC = m[i][col];
      m[i][col] -= s * (m[i][row] + tau * m[i][col]);
      m[i][row] += s * (oldIC - tau * m[i][row]);
    }

    if(vectors)
    {
      for(i = 0; i < m.getRows(); ++i)
      {
        double oldIC = (*vectors)[i][col];
        (*vectors)[i][col] -= s * ((*vectors)[i][row] + tau * (*vectors)[i][col]);
        (*vectors)[i][row] += s * (oldIC - tau * (*vectors)[i][row]);
      }
    }

    m[row][col] = 0;
  }
}


using namespace EigPrivate;

Vectorn<double> getEigensystem(Matrixn<double> m, Matrixn<double> *vectors)
{
  int i;
  assert(m.getRows() == m.getCols());
  int sz = m.getRows();
  assert(sz > 1);
  if(vectors)
    *vectors = Matrixn<double>::identity(sz);

  const double tol = 1e-12;
  int iter = 0;
  while(++iter < 50)
  {
    int j;
    double biggestSoFar = -1;
    for(i = 0; i < sz; ++i) for(j = 0; j < i; ++j)
    {
      biggestSoFar = std::max(biggestSoFar, fabs(m[i][j]));
      jacobi(i, j, m, vectors);
    }
    if(biggestSoFar < tol)
      break;
  }
  Debugging::out() << iter << std::endl;

  Vectorn<double> out(sz);
  for(i = 0; i < sz; ++i)
    out[i] = m[i][i];

  //now sort by decreasing eigenvalue
  std::vector<std::pair<double, int> > perm;
  for(i = 0; i < sz; ++i)
  {
    perm.push_back(std::make_pair(fabs(out[i]), i));
  }
  sort(perm.begin(), perm.end());
  std::reverse(perm.begin(), perm.end());
  Vectorn<double> oldOut = out;

  if(!vectors)
  {
    for(i = 0; i < sz; ++i)
      out[i] = oldOut[perm[i].second];
  }
  else
  {
    Matrixn<double> oldVectors = ~(*vectors);

    for(i = 0; i < sz; ++i)
    {
      out[i] = oldOut[perm[i].second];
      (*vectors)[i] = oldVectors[perm[i].second];
    }

    *vectors = ~(*vectors);
  }

  return out;
}

} // namespace Pinocchio
