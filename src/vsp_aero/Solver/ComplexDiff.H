//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef COMPLEX_DIFF_H
#define COMPLEX_DIFF_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <complex>
#include "VSPAERO_TYPES.H"

#include "START_NAME_SPACE.H"

// Some asserts

#define ASSERT_ROW(a) assert( (a) > 0 ) ; assert( (a) <= row )
#define ASSERT_COL(a) assert( (a) > 0 ) ; assert( (a) <= col )
#define ASSERT_ROW_COL(a) assert( (a) > 0 ) ; assert( (a) <= row*col )

class COMPLEX_DIFF: public std::complex<double> {

private:

   // std::complex<double>*  coef;

public:

// // Operators
// 
//     friend MATRIX operator+(const MATRIX &mat1, const MATRIX &mat2);
//     friend MATRIX operator-(const MATRIX &mat1, const MATRIX &mat2);
// 
//     friend MATRIX operator+(const MATRIX &mat, VSPAERO_DOUBLE scalar);
//     friend MATRIX operator+(VSPAERO_DOUBLE scalar, const MATRIX &mat);
// 
//     friend MATRIX operator-(const MATRIX &mat, VSPAERO_DOUBLE scalar);
//     friend MATRIX operator-(VSPAERO_DOUBLE scalar, const MATRIX &mat);
// 
//     friend MATRIX operator*(const MATRIX &mat1, const MATRIX &mat2);
//     friend MATRIX operator*(const MATRIX &mat, VSPAERO_DOUBLE scalar);
//     friend MATRIX operator*(VSPAERO_DOUBLE scalar, const MATRIX &mat);
// 
//     friend MATRIX operator/(const MATRIX &mat1, const MATRIX &mat2);
//     friend MATRIX operator/(const MATRIX &mat, VSPAERO_DOUBLE scalar);
//     friend MATRIX operator/(VSPAERO_DOUBLE scalar, const MATRIX &mat);
// 
//     friend MATRIX operator%(const MATRIX &mat1, const MATRIX &mat2);
// 
// // Functional operators, these are faster
// 
//     friend void  MatPlusMat(const MATRIX &mat1, const MATRIX &mat2, MATRIX &A);
//     friend void MatMinusMat(const MATRIX &mat1, const MATRIX &mat2, MATRIX &A);
//     friend void MatTimesMat(const MATRIX &mat1, const MATRIX &mat2, MATRIX &A);
//     friend void   MatDivMat(const MATRIX &mat1, const MATRIX &mat2, MATRIX &B);
// 
//     friend void MatTimesScalar(const MATRIX &mat, VSPAERO_DOUBLE scalar, MATRIX &A);
//     friend void   MatDivScalar(const MATRIX &mat, VSPAERO_DOUBLE scalar, MATRIX &A);
// 
//     MATRIX& operator*=(const MATRIX &mat);
//     MATRIX& operator/=(const MATRIX &mat);
//     MATRIX& operator+=(const MATRIX &mat);
//     MATRIX& operator-=(const MATRIX &mat);
// 
//     MATRIX& operator*=(VSPAERO_DOUBLE scalar);
//     MATRIX& operator/=(VSPAERO_DOUBLE scalar);
//     MATRIX& operator+=(VSPAERO_DOUBLE scalar);
//     MATRIX& operator-=(VSPAERO_DOUBLE scalar);



};

#include "END_NAME_SPACE.H"

#endif


