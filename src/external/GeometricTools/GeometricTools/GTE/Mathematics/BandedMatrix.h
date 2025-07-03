// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/LexicoArray2.h>
#include <algorithm>
#include <cstddef>
#include <cmath>
#include <vector>

namespace gte
{
    template <typename Real>
    class BandedMatrix
    {
    public:
        // Construction and destruction.
        BandedMatrix(int32_t size, int32_t numLBands, int32_t numUBands)
            :
            mSize(size),
            mDBand{},
            mLBands{},
            mUBands{},
            mZero((Real)0)
        {
            if (size > 0
                && 0 <= numLBands && numLBands < size
                && 0 <= numUBands && numUBands < size)
            {
                mDBand.resize(size);
                std::fill(mDBand.begin(), mDBand.end(), (Real)0);

                if (numLBands > 0)
                {
                    mLBands.resize(numLBands);
                    int32_t numElements = size - 1;
                    for (auto& band : mLBands)
                    {
                        band.resize(numElements--);
                        std::fill(band.begin(), band.end(), (Real)0);
                    }
                }

                if (numUBands > 0)
                {
                    mUBands.resize(numUBands);
                    int32_t numElements = size - 1;
                    for (auto& band : mUBands)
                    {
                        band.resize(numElements--);
                        std::fill(band.begin(), band.end(), (Real)0);
                    }
                }
            }
            else
            {
                // Invalid argument to BandedMatrix constructor.
                mSize = 0;
            }
        }

        ~BandedMatrix()
        {
        }

        // Member access.
        inline int32_t GetSize() const
        {
            return mSize;
        }

        inline std::vector<Real>& GetDBand()
        {
            return mDBand;
        }

        inline std::vector<Real> const& GetDBand() const
        {
            return mDBand;
        }

        inline std::vector<std::vector<Real>>& GetLBands()
        {
            return mLBands;
        }

        inline std::vector<std::vector<Real>> const& GetLBands() const
        {
            return mLBands;
        }

        inline std::vector<std::vector<Real>>& GetUBands()
        {
            return mUBands;
        }

        inline std::vector<std::vector<Real>> const& GetUBands() const
        {
            return mUBands;
        }

        Real& operator()(int32_t r, int32_t c)
        {
            if (0 <= r && r < mSize && 0 <= c && c < mSize)
            {
                int32_t band = c - r;
                if (band > 0)
                {
                    int32_t const numUBands = static_cast<int32_t>(mUBands.size());
                    if (--band < numUBands && r < mSize - 1 - band)
                    {
                        return mUBands[band][r];
                    }
                }
                else if (band < 0)
                {
                    band = -band;
                    int32_t const numLBands = static_cast<int32_t>(mLBands.size());
                    if (--band < numLBands && c < mSize - 1 - band)
                    {
                        return mLBands[band][c];
                    }
                }
                else
                {
                    return mDBand[r];
                }
            }
            // else invalid index


            // Set the value to zero in case someone unknowingly modified mZero on a
            // previous call to operator(int32_t,int32_t).
            mZero = (Real)0;
            return mZero;
        }

        Real const& operator()(int32_t r, int32_t c) const
        {
            if (0 <= r && r < mSize && 0 <= c && c < mSize)
            {
                int32_t band = c - r;
                if (band > 0)
                {
                    int32_t const numUBands = static_cast<int32_t>(mUBands.size());
                    if (--band < numUBands && r < mSize - 1 - band)
                    {
                        return mUBands[band][r];
                    }
                }
                else if (band < 0)
                {
                    band = -band;
                    int32_t const numLBands = static_cast<int32_t>(mLBands.size());
                    if (--band < numLBands && c < mSize - 1 - band)
                    {
                        return mLBands[band][c];
                    }
                }
                else
                {
                    return mDBand[r];
                }
            }
            // else invalid index


            // Set the value to zero in case someone unknowingly modified
            // mZero on a previous call to operator(int32_t,int32_t).
            mZero = (Real)0;
            return mZero;
        }

        // Factor the square banded matrix A into A = L*L^T, where L is a
        // lower-triangular matrix (L^T is an upper-triangular matrix).  This
        // is an LU decomposition that allows for stable inversion of A to
        // solve A*X = B.  The return value is 'true' iff the factorizing is
        // successful (L is invertible).  If successful, A contains the
        // Cholesky factorization: L in the lower-triangular part of A and
        // L^T in the upper-triangular part of A.
        bool CholeskyFactor()
        {
            if (mDBand.size() == 0 || mLBands.size() != mUBands.size())
            {
                // Invalid number of bands.
                return false;
            }

            int32_t const sizeM1 = mSize - 1;
            int32_t const numBands = static_cast<int32_t>(mLBands.size());

            int32_t k, kMax;
            for (int32_t i = 0; i < mSize; ++i)
            {
                int32_t jMin = i - numBands;
                if (jMin < 0)
                {
                    jMin = 0;
                }

                int32_t j;
                for (j = jMin; j < i; ++j)
                {
                    kMax = j + numBands;
                    if (kMax > sizeM1)
                    {
                        kMax = sizeM1;
                    }

                    for (k = i; k <= kMax; ++k)
                    {
                        operator()(k, i) -= operator()(i, j) * operator()(k, j);
                    }
                }

                kMax = j + numBands;
                if (kMax > sizeM1)
                {
                    kMax = sizeM1;
                }

                for (k = 0; k < i; ++k)
                {
                    operator()(k, i) = operator()(i, k);
                }

                Real diagonal = operator()(i, i);
                if (diagonal <= (Real)0)
                {
                    return false;
                }
                Real invSqrt = ((Real)1) / std::sqrt(diagonal);
                for (k = i; k <= kMax; ++k)
                {
                    operator()(k, i) *= invSqrt;
                }
            }

            return true;
        }

        // Solve the linear system A*X = B, where A is an NxN banded matrix
        // and B is an Nx1 vector.  The unknown X is also Nx1.  The input to
        // this function is B.  The output X is computed and stored in B.  The
        // return value is 'true' iff the system has a solution.  The matrix A
        // and the vector B are both modified by this function.  If
        // successful, A contains the Cholesky factorization: L in the
        // lower-triangular part of A and L^T in the upper-triangular part
        // of A.
        bool SolveSystem(Real* bVector)
        {
            return CholeskyFactor()
                && SolveLower(bVector)
                && SolveUpper(bVector);
        }

        // Solve the linear system A*X = B, where A is an NxN banded matrix
        // and B is an NxM matrix.  The unknown X is also NxM.  The input to
        // this function is B.  The output X is computed and stored in B.  The
        // return value is 'true' iff the system has a solution.  The matrix A
        // and the vector B are both modified by this function.  If
        // successful, A contains the Cholesky factorization: L in the
        // lower-triangular part of A and L^T in the upper-triangular part
        // of A.
        //
        // 'bMatrix' must have the storage order specified by the template
        // parameter.
        template <bool RowMajor>
        bool SolveSystem(Real* bMatrix, int32_t numBColumns)
        {
            return CholeskyFactor()
                && SolveLower<RowMajor>(bMatrix, numBColumns)
                && SolveUpper<RowMajor>(bMatrix, numBColumns);
        }

        // Compute the inverse of the banded matrix.  The return value is
        // 'true' when the matrix is invertible, in which case the 'inverse'
        // output is valid.  The return value is 'false' when the matrix is
        // not invertible, in which case 'inverse' is invalid and should not
        // be used.  The input matrix 'inverse' must be the same size as
        // 'this'.
        //
        // 'bMatrix' must have the storage order specified by the template
        // parameter.
        template <bool RowMajor>
        bool ComputeInverse(Real* inverse) const
        {
            LexicoArray2<RowMajor, Real> invA(mSize, mSize, inverse);

            BandedMatrix<Real> tmpA = *this;
            for (int32_t row = 0; row < mSize; ++row)
            {
                for (int32_t col = 0; col < mSize; ++col)
                {
                    if (row != col)
                    {
                        invA(row, col) = (Real)0;
                    }
                    else
                    {
                        invA(row, row) = (Real)1;
                    }
                }
            }

            // Forward elimination.
            for (int32_t row = 0; row < mSize; ++row)
            {
                // The pivot must be nonzero in order to proceed.
                Real diag = tmpA(row, row);
                if (diag == (Real)0)
                {
                    return false;
                }

                Real invDiag = ((Real)1) / diag;
                tmpA(row, row) = (Real)1;

                // Multiply the row to be consistent with diagonal term of 1.
                int32_t colMin = row + 1;
                int32_t colMax = colMin + static_cast<int32_t>(mUBands.size());
                if (colMax > mSize)
                {
                    colMax = mSize;
                }

                int32_t c;
                for (c = colMin; c < colMax; ++c)
                {
                    tmpA(row, c) *= invDiag;
                }
                for (c = 0; c <= row; ++c)
                {
                    invA(row, c) *= invDiag;
                }

                // Reduce the remaining rows.
                int32_t rowMin = row + 1;
                int32_t rowMax = rowMin + static_cast<int32_t>(mLBands.size());
                if (rowMax > mSize)
                {
                    rowMax = mSize;
                }

                for (int32_t r = rowMin; r < rowMax; ++r)
                {
                    Real mult = tmpA(r, row);
                    tmpA(r, row) = (Real)0;
                    for (c = colMin; c < colMax; ++c)
                    {
                        tmpA(r, c) -= mult * tmpA(row, c);
                    }
                    for (c = 0; c <= row; ++c)
                    {
                        invA(r, c) -= mult * invA(row, c);
                    }
                }
            }

            // Backward elimination.
            for (int32_t row = mSize - 1; row >= 1; --row)
            {
                int32_t rowMax = row - 1;
                int32_t rowMin = row - static_cast<int32_t>(mUBands.size());
                if (rowMin < 0)
                {
                    rowMin = 0;
                }

                for (int32_t r = rowMax; r >= rowMin; --r)
                {
                    Real mult = tmpA(r, row);
                    tmpA(r, row) = (Real)0;
                    for (int32_t c = 0; c < mSize; ++c)
                    {
                        invA(r, c) -= mult * invA(row, c);
                    }
                }
            }

            return true;
        }

    private:
        // The linear system is L*U*X = B, where A = L*U and U = L^T,  Reduce
        // this to U*X = L^{-1}*B.  The return value is 'true' iff the
        // operation is successful.
        bool SolveLower(Real* dataVector) const
        {
            int32_t const size = static_cast<int32_t>(mDBand.size());
            for (int32_t r = 0; r < size; ++r)
            {
                Real lowerRR = operator()(r, r);
                if (lowerRR > (Real)0)
                {
                    for (int32_t c = 0; c < r; ++c)
                    {
                        Real lowerRC = operator()(r, c);
                        dataVector[r] -= lowerRC * dataVector[c];
                    }
                    dataVector[r] /= lowerRR;
                }
                else
                {
                    return false;
                }
            }
            return true;
        }

        // The linear system is U*X = L^{-1}*B.  Reduce this to
        // X = U^{-1}*L^{-1}*B.  The return value is 'true' iff the operation
        // is successful.
        bool SolveUpper(Real* dataVector) const
        {
            int32_t const size = static_cast<int32_t>(mDBand.size());
            for (int32_t r = size - 1; r >= 0; --r)
            {
                Real upperRR = operator()(r, r);
                if (upperRR > (Real)0)
                {
                    for (int32_t c = r + 1; c < size; ++c)
                    {
                        Real upperRC = operator()(r, c);
                        dataVector[r] -= upperRC * dataVector[c];
                    }

                    dataVector[r] /= upperRR;
                }
                else
                {
                    return false;
                }
            }
            return true;
        }

        // The linear system is L*U*X = B, where A = L*U and U = L^T,  Reduce
        // this to U*X = L^{-1}*B.  The return value is 'true' iff the
        // operation is successful.  See the comments for
        // SolveSystem(Real*,int32_t) about the storage for dataMatrix.
        template <bool RowMajor>
        bool SolveLower(Real* dataMatrix, int32_t numColumns) const
        {
            LexicoArray2<RowMajor, Real> data(mSize, numColumns, dataMatrix);

            for (int32_t r = 0; r < mSize; ++r)
            {
                Real lowerRR = operator()(r, r);
                if (lowerRR > (Real)0)
                {
                    for (int32_t c = 0; c < r; ++c)
                    {
                        Real lowerRC = operator()(r, c);
                        for (int32_t bCol = 0; bCol < numColumns; ++bCol)
                        {
                            data(r, bCol) -= lowerRC * data(c, bCol);
                        }
                    }

                    Real inverse = ((Real)1) / lowerRR;
                    for (int32_t bCol = 0; bCol < numColumns; ++bCol)
                    {
                        data(r, bCol) *= inverse;
                    }
                }
                else
                {
                    return false;
                }
            }
            return true;
        }

        // The linear system is U*X = L^{-1}*B.  Reduce this to
        // X = U^{-1}*L^{-1}*B.  The return value is 'true' iff the operation
        // is successful.  See the comments for SolveSystem(Real*,int32_t) about
        // the storage for dataMatrix.
        template <bool RowMajor>
        bool SolveUpper(Real* dataMatrix, int32_t numColumns) const
        {
            LexicoArray2<RowMajor, Real> data(mSize, numColumns, dataMatrix);

            for (int32_t r = mSize - 1; r >= 0; --r)
            {
                Real upperRR = operator()(r, r);
                if (upperRR > (Real)0)
                {
                    for (int32_t c = r + 1; c < mSize; ++c)
                    {
                        Real upperRC = operator()(r, c);
                        for (int32_t bCol = 0; bCol < numColumns; ++bCol)
                        {
                            data(r, bCol) -= upperRC * data(c, bCol);
                        }
                    }

                    Real inverse = ((Real)1) / upperRR;
                    for (int32_t bCol = 0; bCol < numColumns; ++bCol)
                    {
                        data(r, bCol) *= inverse;
                    }
                }
                else
                {
                    return false;
                }
            }
            return true;
        }

        int32_t mSize;
        std::vector<Real> mDBand;
        std::vector<std::vector<Real>> mLBands, mUBands;

        // For return by operator()(int32_t,int32_t) for valid indices not in the
        // bands, in which case the matrix entries are zero,
        mutable Real mZero;
    };
}
