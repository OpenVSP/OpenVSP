//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "VSPAERO_TYPES.H"

// Some asserts

#define ASSERT_ROW(a) assert( (a) > 0 ) ; assert( (a) <= row )
#define ASSERT_COL(a) assert( (a) > 0 ) ; assert( (a) <= col )
#define ASSERT_ROW_COL(a) assert( (a) > 0 ) ; assert( (a) <= row*col )

class MATRIX {

private:

    int row;
    int col;
    VSPAERO_DOUBLE*  coef;

public:

    MATRIX(void);
    MATRIX(int size);
    MATRIX(int row_, int col_);
   ~MATRIX(void);
    MATRIX(const MATRIX &mat);

    MATRIX& operator=(const MATRIX &mat);
    MATRIX& operator=(VSPAERO_DOUBLE scalar);

    VSPAERO_DOUBLE& operator()(int i) { return (coef[i-1]); };
    VSPAERO_DOUBLE& operator()(int i, int j) { return (coef[(i-1) + (j-1)*row]); };

    const VSPAERO_DOUBLE& operator()(int i) const { return (coef[i-1]); };
    const VSPAERO_DOUBLE& operator()(int i, int j) const { return (coef[(i-1) + (j-1)*row]); };

// Operators

    friend MATRIX operator+(const MATRIX &mat1, const MATRIX &mat2);
    friend MATRIX operator-(const MATRIX &mat1, const MATRIX &mat2);

    friend MATRIX operator+(const MATRIX &mat, VSPAERO_DOUBLE scalar);
    friend MATRIX operator+(VSPAERO_DOUBLE scalar, const MATRIX &mat);

    friend MATRIX operator-(const MATRIX &mat, VSPAERO_DOUBLE scalar);
    friend MATRIX operator-(VSPAERO_DOUBLE scalar, const MATRIX &mat);

    friend MATRIX operator*(const MATRIX &mat1, const MATRIX &mat2);
    friend MATRIX operator*(const MATRIX &mat, VSPAERO_DOUBLE scalar);
    friend MATRIX operator*(VSPAERO_DOUBLE scalar, const MATRIX &mat);

    friend MATRIX operator/(const MATRIX &mat1, const MATRIX &mat2);
    friend MATRIX operator/(const MATRIX &mat, VSPAERO_DOUBLE scalar);
    friend MATRIX operator/(VSPAERO_DOUBLE scalar, const MATRIX &mat);

    friend MATRIX operator%(const MATRIX &mat1, const MATRIX &mat2);

// Functional operators, these are faster

    friend void  MatPlusMat(const MATRIX &mat1, const MATRIX &mat2, MATRIX &A);
    friend void MatMinusMat(const MATRIX &mat1, const MATRIX &mat2, MATRIX &A);
    friend void MatTimesMat(const MATRIX &mat1, const MATRIX &mat2, MATRIX &A);
    friend void   MatDivMat(const MATRIX &mat1, const MATRIX &mat2, MATRIX &B);

    friend void MatTimesScalar(const MATRIX &mat, VSPAERO_DOUBLE scalar, MATRIX &A);
    friend void   MatDivScalar(const MATRIX &mat, VSPAERO_DOUBLE scalar, MATRIX &A);

    MATRIX& operator*=(const MATRIX &mat);
    MATRIX& operator/=(const MATRIX &mat);
    MATRIX& operator+=(const MATRIX &mat);
    MATRIX& operator-=(const MATRIX &mat);

    MATRIX& operator*=(VSPAERO_DOUBLE scalar);
    MATRIX& operator/=(VSPAERO_DOUBLE scalar);
    MATRIX& operator+=(VSPAERO_DOUBLE scalar);
    MATRIX& operator-=(VSPAERO_DOUBLE scalar);

    MATRIX inverse(void);
    MATRIX transpose(void);

    void inverse(MATRIX &A_inv, MATRIX &B);
    void transpose(MATRIX &B);

    void size(int size);
    void size(int row_, int col_);

    void LU(void);
    void LU_pivot(int *indx);
    void solve(VSPAERO_DOUBLE *vec);
    void solve_vdk(MATRIX &vec);
    void diagonal(VSPAERO_DOUBLE val);

    MATRIX gauss_solve(MATRIX &vec, int max_iters, VSPAERO_DOUBLE toler);

    void print(char *name);
    void print(void);

};

#endif

