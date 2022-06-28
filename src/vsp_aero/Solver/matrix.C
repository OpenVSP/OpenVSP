//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "matrix.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                             MATRIX Constructor                               #
#                                                                              #
##############################################################################*/

MATRIX::MATRIX(void)
{

    // Just zero the size of the matrix

    row = 0;
    col = 0;

    coef = NULL;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Constructor                               #
#                                                                              #
##############################################################################*/

MATRIX::MATRIX(int size)
{

    // Allocate space for the matrix

    row = size;
    col = 1;

    coef = new VSPAERO_DOUBLE[row*col];

    // Initialize to zero

    (*this) = (VSPAERO_DOUBLE) 0.;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Constructor                               #
#                                                                              #
##############################################################################*/

MATRIX::MATRIX(int row_, int col_)
{

    // Allocate space for the matrix

    row = row_;
    col = col_;

    coef = new VSPAERO_DOUBLE[row*col];

    // Initialize to zero

    (*this) = (VSPAERO_DOUBLE) 0.;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Destructor                                #
#                                                                              #
##############################################################################*/

MATRIX::~MATRIX(void)
{

    // Free space for the matrix

    if ( coef != NULL ) delete [] coef;
    
    coef = NULL;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Copy                                      #
#                                                                              #
##############################################################################*/

MATRIX::MATRIX(const MATRIX &mat)
{

    int i;

    // Allocate space for the matrix

    row = mat.row;
    col = mat.col;

    coef = new VSPAERO_DOUBLE[row*col];

    // Copy contents of mat

    for ( i = 1 ; i <= row*col ; i++ ) {

       (*this)(i) = mat(i);

    }

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator =                                #
#                                                                              #
##############################################################################*/

MATRIX &MATRIX::operator=(const MATRIX &mat)
{

    int i;

#ifdef MATRIX_DEBUG

    if ( col != mat.col || row != mat.row ) {

       PRINTF("Error: Attempt to set equal two matrices of different size! \n");

       exit(1);

    }

#endif

    // Copy contents of mat

    for ( i = 1 ; i <= row*col ; i++ ) {

       (*this)(i) = mat(i);

    }

    return *this;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator =                                #
#                                                                              #
##############################################################################*/

MATRIX& MATRIX::operator=(VSPAERO_DOUBLE scalar)
{

    int i;

    // Set all ij to value of scalar

    for ( i = 1 ; i <= row*col ; i++ ) {

       (*this)(i) = scalar;

    }

    return *this;

}

/*##############################################################################
#                                                                              #
#                          MATRIX Operator +                                   #
#                                                                              #
##############################################################################*/

MATRIX operator+(const MATRIX &mat1, const MATRIX &mat2)
{

    int i;
    MATRIX A(mat1.row,mat1.col);

#ifdef MATRIX_DEBUG

    if ( A.col != mat2.col || A.row != mat2.row ) {

       PRINTF("Error: Attempt to add two matrices of different size! \n");

       exit(1);

    }

#endif

    // Copy contents of mat

    for ( i = 1 ; i <= A.row*A.col ; i++ ) {

       A(i) = mat1(i) + mat2(i);

    }

    return A;

}

/*##############################################################################
#                                                                              #
#                          MATRIX Operator +                                   #
#                                                                              #
##############################################################################*/

void MatPlusMat(const MATRIX &mat1, const MATRIX &mat2, MATRIX &A)
{

    int i;

    // Copy contents of mat

    for ( i = 1 ; i <= A.row*A.col ; i++ ) {

       A(i) = mat1(i) + mat2(i);

    }

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator -                                #
#                                                                              #
##############################################################################*/

MATRIX operator-(const MATRIX &mat1, const MATRIX &mat2)
{

    int i;
    MATRIX A(mat1.row,mat1.col);

#ifdef MATRIX_DEBUG

    if ( A.col != mat2.col || A.row != mat2.row ) {

       PRINTF("Error: Attempt to subtract two matrices of different size! \n");

       exit(1);

    }

#endif

    // Copy contents of mat

    for ( i = 1 ; i <= A.row*A.col ; i++ ) {

       A(i) = mat1(i) - mat2(i);

    }

    return A;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator -                                #
#                                                                              #
##############################################################################*/

void MatMinusMat(const MATRIX &mat1, const MATRIX &mat2, MATRIX &A)
{

    int i;

    // Copy contents of mat

    for ( i = 1 ; i <= A.row*A.col ; i++ ) {

       A(i) = mat1(i) - mat2(i);

    }

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator +                                #
#                                                                              #
##############################################################################*/

MATRIX operator+(const MATRIX &mat, VSPAERO_DOUBLE scalar)
{

    int i;
    MATRIX A(mat.row,mat.col);

    // Copy contents of mat

    for ( i = 1 ; i <= A.row*A.col ; i++ ) {

       A(i) = mat(i) + scalar;

    }

    return A;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator +                                #
#                                                                              #
##############################################################################*/

MATRIX operator+(VSPAERO_DOUBLE scalar, const MATRIX &mat)
{

    return (operator+(mat,scalar));

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator -                                #
#                                                                              #
##############################################################################*/

MATRIX operator-(const MATRIX &mat, VSPAERO_DOUBLE scalar)
{

    int i;
    MATRIX A(mat.row,mat.col);

    // Copy contents of mat

    for ( i = 1 ; i <= A.row*A.col ; i++ ) {

       A(i) = mat(i) - scalar;

    }

    return A;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator +                                #
#                                                                              #
##############################################################################*/

MATRIX operator-(VSPAERO_DOUBLE scalar, const MATRIX &mat)
{

    return (operator-(mat,scalar));

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator *                                #
#                                                                              #
##############################################################################*/

MATRIX operator*(const MATRIX &mat1, const MATRIX &mat2)
{

    int i, j, k;
    MATRIX A(mat1.row,mat2.col);

#ifdef MATRIX_DEBUG

    if ( mat1.col != mat2.row ) {

       PRINTF("Error: Attempt to multiply multiply matrices of wrong size! \n");

       exit(1);

    }

#endif

    for ( i = 1 ; i <= A.row ; i++ ) {

       for ( j = 1 ; j <= A.col ; j++ ) {

          A(i,j) = 0.;

          for ( k = 1 ; k <= mat1.col ; k++ ) {

             A(i,j) += mat1(i,k) * mat2(k,j);

          }

       }

    }

    return A;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator *                                #
#                                                                              #
##############################################################################*/

void MatTimesMat(const MATRIX &mat1, const MATRIX &mat2, MATRIX &A)
{

    int i, j, k;

    for ( i = 1 ; i <= A.row ; i++ ) {

       for ( j = 1 ; j <= A.col ; j++ ) {

          A(i,j) = 0.;

          for ( k = 1 ; k <= mat1.col ; k++ ) {

             A(i,j) += mat1(i,k) * mat2(k,j);

          }

       }

    }

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator *                                #
#                                                                              #
##############################################################################*/

MATRIX operator*(VSPAERO_DOUBLE scalar, const MATRIX &mat)
{

    int i;
    MATRIX A(mat.row,mat.col);

    for ( i = 1 ; i <= A.row*A.col ; i++ ) {

       A(i) = scalar*mat(i);

    }

    return A;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator *                                #
#                                                                              #
##############################################################################*/


MATRIX operator*(const MATRIX &mat, VSPAERO_DOUBLE scalar)
{

    return operator*(scalar,mat);

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator *                                #
#                                                                              #
##############################################################################*/


void MatTimesScalar(const MATRIX &mat, VSPAERO_DOUBLE scalar, MATRIX &A)
{

    int i;

    for ( i = 1 ; i <= A.row*A.col ; i++ ) {

       A(i) = scalar*mat(i);

    }

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator /                                #
#                                                                              #
##############################################################################*/

MATRIX operator/(const MATRIX &mat1, const MATRIX &mat2)
{

    int i, j;
    VSPAERO_DOUBLE *vec;
    MATRIX A(mat2), B(mat2.row,mat1.col);

#ifdef MATRIX_DEBUG

    if ( mat2.col != mat1.row ) {

       PRINTF("Error: Attempt to divide non-similar matrices! \n");

       exit(1);

    }

    if ( mat2.col != mat2.row ) {

       PRINTF("Error: Attempt to divide by non-square matrix! \n");

       exit(1);

    }

#endif

    // Find LU decompostion of mat2

    A.LU();

    // Now solve for mat1/mat2 one column at a time

    vec = new VSPAERO_DOUBLE[mat1.row + 1];

    for ( j = 1 ; j <= mat1.col ; j++ ) {

       for ( i = 1 ; i <= mat1.row ; i++ ) {

          vec[i] = mat1(i,j);

       }

       A.solve(vec);

       for ( i = 1 ; i <= mat1.row ; i++ ) {

          B(i,j) = vec[i];

       }

    }

    delete [] vec;

    return B;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator /                                #
#                                                                              #
##############################################################################*/

void MatDivMat(const MATRIX &mat1, const MATRIX &mat2, MATRIX &B)
{

    int i, j;
    VSPAERO_DOUBLE *vec;
    MATRIX A(mat2);

#ifdef MATRIX_DEBUG

    if ( mat2.col != mat1.row ) {

       PRINTF("Error: Attempt to divide non-similar matrices! \n");

       exit(1);

    }

    if ( mat2.col != mat2.row ) {

       PRINTF("Error: Attempt to divide by non-square matrix! \n");

       exit(1);

    }

#endif

    // Find LU decompostion of mat2

    A.LU();

    // Now solve for mat1/mat2 one column at a time

    vec = new VSPAERO_DOUBLE[mat1.row + 1];

    for ( j = 1 ; j <= mat1.col ; j++ ) {

       for ( i = 1 ; i <= mat1.row ; i++ ) {

          vec[i] = mat1(i,j);

       }

       A.solve(vec);

       for ( i = 1 ; i <= mat1.row ; i++ ) {

          B(i,j) = vec[i];

       }

    }

    delete [] vec;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator /                                #
#                                                                              #
##############################################################################*/

MATRIX operator/(VSPAERO_DOUBLE scalar, const MATRIX &mat)
{

    MATRIX A(mat.row,mat.col);

#ifdef MATRIX_DEBUG

    if ( mat.row != 1 ||  mat.col != 1 ) {

       PRINTF("Division of a scalar by a matrix only defined for 1x1 matrices!\n");

       exit(1);

    }

#endif

    A(1,1) = scalar;

    A(1,1) /= mat(1,1);

    return A;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator /                                #
#                                                                              #
##############################################################################*/

MATRIX operator/(const MATRIX &mat, VSPAERO_DOUBLE scalar)
{

    int i;
    VSPAERO_DOUBLE divisor;
    MATRIX A(mat.row,mat.col);

    divisor = ( (VSPAERO_DOUBLE) 1.)/scalar;

    for ( i = 1 ; i <= A.row*A.col ; i++ ) {

       A(i) = divisor*mat(i);

    }

    return A;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator /                                #
#                                                                              #
##############################################################################*/

void MatDivScalar(const MATRIX &mat, VSPAERO_DOUBLE scalar, MATRIX &A)
{

    int i;
    VSPAERO_DOUBLE divisor;

    divisor = ( (VSPAERO_DOUBLE) 1.)/scalar;

    for ( i = 1 ; i <= A.row*A.col ; i++ ) {

       A(i) = divisor*mat(i);

    }

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator %                                #
#                                                                              #
##############################################################################*/

MATRIX operator%(const MATRIX &mat1, const MATRIX &mat2)
{

    MATRIX A(mat2), B(mat2.row,mat1.col);

#ifdef MATRIX_DEBUG

    if ( mat1.col != mat2.row ) {

       PRINTF("Error: Attempt to post - divide non-similar matrices! \n");

       exit(1);

    }

    if ( mat2.col != mat2.row ) {

       PRINTF("Error: Attempt to post - divide by non-square matrix! \n");

       exit(1);

    }

#endif

    // Form mat1 * mat2_inverse

    B = mat1 * A.inverse();

    return B;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator *=                               #
#                                                                              #
##############################################################################*/

MATRIX& MATRIX::operator*=(const MATRIX &mat)
{

    (*this) = mat * (*this);

    return (*this);

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator /=                               #
#                                                                              #
##############################################################################*/

MATRIX& MATRIX::operator/=(const MATRIX &mat)
{

    (*this) = (*this) / mat;

    return (*this);

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator +=                               #
#                                                                              #
##############################################################################*/

MATRIX& MATRIX::operator+=(const MATRIX &mat)
{

    (*this) = (*this) + mat;

    return (*this);

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator -=                               #
#                                                                              #
##############################################################################*/

MATRIX& MATRIX::operator-=(const MATRIX &mat)
{

    (*this) = (*this) - mat;

    return (*this);

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator *=                               #
#                                                                              #
##############################################################################*/

MATRIX& MATRIX::operator*=(VSPAERO_DOUBLE scalar)
{

    (*this) = scalar * (*this);

    return (*this);

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator /=                               #
#                                                                              #
##############################################################################*/

MATRIX& MATRIX::operator/=(VSPAERO_DOUBLE scalar)
{

    (*this) = (*this) / scalar;

    return (*this);

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator +=                               #
#                                                                              #
##############################################################################*/

MATRIX& MATRIX::operator+=(VSPAERO_DOUBLE scalar)
{

    (*this) = (*this) + scalar;

    return (*this);

}

/*##############################################################################
#                                                                              #
#                             MATRIX Operator -=                               #
#                                                                              #
##############################################################################*/

MATRIX& MATRIX::operator-=(VSPAERO_DOUBLE scalar)
{

    (*this) = (*this) - scalar;

    return (*this);

}


/*##############################################################################
#                                                                              #
#                             MATRIX size                                      #
#                                                                              #
##############################################################################*/

void MATRIX::size(int size)
{

    // Allocate space for the matrix

    row = size;
    col = 1;

    coef = new VSPAERO_DOUBLE[row*col];

}

/*##############################################################################
#                                                                              #
#                             MATRIX size                                      #
#                                                                              #
##############################################################################*/

void MATRIX::size(int row_, int col_)
{

    // Allocate space for the matrix

    row = row_;
    col = col_;

    coef = new VSPAERO_DOUBLE[2*row*col];

}

/*##############################################################################
#                                                                              #
#                             MATRIX inverse                                   #
#                                                                              #
##############################################################################*/

MATRIX MATRIX::inverse(void)
{

    int i, j, neq;
    VSPAERO_DOUBLE *vec;
    MATRIX A_inv(*this), B(this->row,this->col);

#ifdef MATRIX_DEBUG

    if ( A_inv.row != A_inv.col ) {

       PRINTF("Inverse of non-square matrix not defined! \n");

       exit(1);

    }

#endif

    // Find LU decompostion of A

    A_inv.LU();

    // Now solve for inverse of A, one column at a time

    neq = A_inv.row;

    vec = new VSPAERO_DOUBLE[neq + 1];

    for ( j = 1 ; j <= neq ; j++ ) {

       for ( i = 1 ; i <= neq ; i++ ) {

          vec[i] = 0.;

       }

       vec[j] = 1.;

       A_inv.solve(vec);

       for ( i = 1 ; i <= neq ; i++ ) {

          B(i,j) = vec[i];

       }

    }

    delete [] vec;

    return B;

}

/*##############################################################################
#                                                                              #
#                             MATRIX inverse                                   #
#                                                                              #
##############################################################################*/

void MATRIX::inverse(MATRIX &A_inv, MATRIX &B)
{

    int i, j, neq;
    VSPAERO_DOUBLE *vec;

    A_inv = *this;

#ifdef MATRIX_DEBUG

    if ( A_inv.row != A_inv.col ) {

       PRINTF("Inverse of non-square matrix not defined! \n");

       exit(1);

    }

#endif

    // Find LU decompostion of A

    A_inv.LU();

    // Now solve for inverse of A, one column at a time

    neq = A_inv.row;

    vec = new VSPAERO_DOUBLE[neq + 1];

    for ( j = 1 ; j <= neq ; j++ ) {

       for ( i = 1 ; i <= neq ; i++ ) {

          vec[i] = 0.;

       }

       vec[j] = 1.;

       A_inv.solve(vec);

       for ( i = 1 ; i <= neq ; i++ ) {

          B(i,j) = vec[i];

       }

    }

    delete [] vec;

}

/*##############################################################################
#                                                                              #
#                             MATRIX transpose                                 #
#                                                                              #
##############################################################################*/

MATRIX MATRIX::transpose(void)
{

    int i, j;
    MATRIX B(this->col,this->row);

    // Form transpose of this matrix

    for ( i = 1 ; i <= this->row ; i++ ) {

       for ( j = 1 ; j <= this->col ; j++ ) {

          B(j,i) = (*this)(i,j);

       }

    }

    return B;

}

/*##############################################################################
#                                                                              #
#                             MATRIX transpose                                 #
#                                                                              #
##############################################################################*/

void MATRIX::transpose(MATRIX &B)
{

    int i, j;

    // Form transpose of this matrix

    for ( i = 1 ; i <= this->row ; i++ ) {

       for ( j = 1 ; j <= this->col ; j++ ) {

          B(j,i) = (*this)(i,j);

       }

    }

}

/*##############################################################################
#                                                                              #
#                             MATRIX LU                                        #
#                                                                              #
##############################################################################*/

void MATRIX::LU(void)
{

    int i, j, k, neq;

    // find LU decomposition of matrix

    neq = row;

    for ( k = 1 ; k <= neq - 1 ; k++ ) {

       for ( i = k + 1 ; i <= neq ; i++ ) {

          (*this)(i,k) /= (*this)(k,k);

       }

       for ( j = k + 1 ; j <= neq ; j++ ) {

          for ( i = k + 1 ; i <= neq ; i++ ) {

             (*this)(i,j) -= (*this)(i,k)*(*this)(k,j);

          }

       }

    }

}

/*##############################################################################
#                                                                              #
#                             MATRIX LU_pivot                                  #
#                                                                              #
##############################################################################*/

void MATRIX::LU_pivot(int *indx)
{

    int i, j, k, neq, imax;
    VSPAERO_DOUBLE big,dum,sum,d,tiny;
    VSPAERO_DOUBLE *ans;

    imax = 0;

    // find LU decomposition of matrix using Crout's algorithm - with pivots
    // From 'Numerical Recipes in C, 2nd edition' pp 46-47

    tiny = 1.0e-20;

    neq = row;

    ans = new VSPAERO_DOUBLE[neq + 1];

    d = 1.0;

    for (i = 1 ; i <= neq ; i++) {

        big = 0.0;

    for (j = 1 ; j <= neq ; j++ ) {

        if ( abs( (*this)(i,j) ) > big) {

          //  big = fabs( (*this)(i,j) );
            big = ABS( (*this)(i,j) );

        }

    }

    if (big == 0.0) {

        PRINTF("Singular matrix in LU_pivot! \n");

            exit(1);

    }

    ans[i] = ((VSPAERO_DOUBLE)1.0) / big;

    }

    for (j = 1 ; j <= neq ; j++ ) {

        for (i = 1 ; i < j ; i++ ) {

        sum = (*this)(i,j);

        for (k = 1 ; k < i ; k++) {

            sum -= (*this)(i,k) * (*this)(k,j);

        }

        (*this)(i,j) = sum;

    }

    big = 0.0;

    for (i = j ; i <= neq ; i++) {

        sum = (*this)(i,j);

        for (k = 1 ; k < j ; k++) {

            sum -= (*this)(i,k) * (*this)(k,j);

        }

        (*this)(i,j) = sum;

        // Some issue here as to whether the index should be i or j
        // in the ans[.] array.

        if ( (dum = ans[j]*abs( sum )) >= big ) {

            big = dum;

            imax = i;

        }

    }

    if ( j != imax ) {

        for (k = 1 ; k <= neq ; k++) {

            dum = (*this)(imax,k);

            (*this)(imax,k) = (*this)(j,k);

            (*this)(j,k) = dum;

        }

        d = -d;

        ans[imax] = ans[j];

    }

    indx[j] = imax;

    if ( (*this)(j,j) == 0.0 ) (*this)(j,j) = tiny;

    if (j != neq) {

        dum = ((VSPAERO_DOUBLE)1.0) / ( (*this)(j,j) );

        for (i = j+1 ; i <= neq ; i++) {

            (*this)(i,j) *= dum;

            //PRINTF("%f  ",(*this)(i,j));

        }
    //print("\n");
    }

    }

    delete [] ans;

}

/*##############################################################################
#                                                                              #
#                             MATRIX Solve                                     #
#                                                                              #
##############################################################################*/

void MATRIX::solve(VSPAERO_DOUBLE *vec)
{

    int i, j, neq;

    // Forward elimination

    neq = row;

    for ( i = 2 ; i <= neq ; i++ ) {

       for ( j = 1 ; j <= i - 1 ; j++ ) {

          vec[i] -= (*this)(i,j)*vec[j];

       }

    }

    // Backwards solve

    vec[neq] /= (*this)(neq,neq);

    for ( i = neq - 1 ; i >= 1 ; i-- ) {

       for ( j = i + 1 ; j <= neq ; j++ ) {

          vec[i] -= (*this)(i,j)*vec[j];

       }

       vec[i] /= (*this)(i,i);

    }

}

/*##############################################################################
#                                                                              #
#                             MATRIX Solve_vdk                                 #
#                                                                              #
##############################################################################*/

void MATRIX::solve_vdk(MATRIX &vec)
{

    int i, j, num, neq, ip, ii, *indx;
    VSPAERO_DOUBLE sum;
    MATRIX A_inv(*this);

    // Find LU decompostion of A

    neq = A_inv.row;

    indx = new int[neq +1];

    // initialize index array

    for (i = 1 ; i <= neq ; i++) {indx[i] = 0.0;}

    // Perform LU decomposition with pivots

    A_inv.LU_pivot(indx);

    //for (i = 1 ; i <= neq ; i++) {PRINTF("indx[%d] = %d\n",i,indx[i]);}

    //A_inv.print();

    (*this) = A_inv;

    // Forward elimination

    //indx[16] = 15;

    num = row;

    ii= 0;

    for ( i = 1 ; i <= num ; i++ ) {

       ip = indx[i];

       sum = vec(ip);

       vec(ip) = vec(i);

       //PRINTF("vec[%d] = %f -> indx[%d] = %d\n",ip,vec(ip),i,indx[i]);

       if (ii) {

           for ( j = ii ; j <= i - 1 ; j++ ) {

              sum -= (*this)(i,j)*vec(j);

           }

       } 
       
       else if ( sum > 0. ) {

            ii = i;

       }

       vec(i) = sum;

    }

    // Backwards solve

    for ( i = num ; i >= 1 ; i-- ) {

        sum = vec(i);

       for ( j = i + 1 ; j <= num ; j++ ) {

          sum -= (*this)(i,j)*vec(j);

       }

       vec(i) = sum / (*this)(i,i);

    }

    delete [] indx;

}

/*##############################################################################
#                                                                              #
#                             MATRIX gauss_solve                               #
#                                                                              #
##############################################################################*/

MATRIX MATRIX::gauss_solve(MATRIX &vec, int max_iters, VSPAERO_DOUBLE toler)
{

    int i, j, neq, iter;
    VSPAERO_DOUBLE res, delta, max_res;
    MATRIX x(this->row);

    // Perform Gauss-Seidel iteration to solve Ax = b

    neq = row;

    iter = 0;

    // Initial Guess

    for ( i = 1 ; i <= neq ; i++ ) {

        x(i) = 0.0;

    }

    max_res = 1.e6;

    while ( max_res >= toler && iter <= max_iters ) {

       max_res = 0.0;

       for ( i = 1 ; i <= neq ; i++ ) {

          res = vec(i);

          for ( j = 1 ; j <= neq ; j++ ) {

             res -=(*this)(i,j) * x(j);

          }

          delta = res/(*this)(i,i);

          x(i) += delta;

          if ( sqrt(res*res) > max_res ) max_res = sqrt(res*res);
     //     max_res = MAX(max_res,ABS(res));

       }

       iter++;

       PRINTF("Iteration %d, Max Res = %f for %d rows \r",iter,max_res,neq);

       fflush(NULL);

    }

    return x;
}

/*##############################################################################
#                                                                              #
#                             MATRIX diagonal                                  #
#                                                                              #
##############################################################################*/

void MATRIX::diagonal(VSPAERO_DOUBLE val)
{

    int i;

#ifdef MATRIX_DEBUG

    if ( row != col ) {

       PRINTF("Non-square diagonal matrices not defined! \n");

       exit(1);

    }

#endif

    // Set diagonal coefficients

    for ( i = 1 ; i <= row ; i++ ) {

       (*this)(i,i) = val;

    }

}

/*##############################################################################
#                                                                              #
#                             MATRIX print                                     #
#                                                                              #
##############################################################################*/

void MATRIX::print(char *name)
{

    int i, j;

    // Write out a title

    if ( strcmp(name,"") != 0 ) PRINTF("%s\n",name);

    // Write row and column numbers

    if ( col > 1 ) {

       PRINTF("          ");

       for ( j = 1 ; j <= col ; j++ ) {

          PRINTF("%10d   ",j);

       }

    }

    PRINTF("\n");

    // Print out the matrix contents

    for ( i = 1 ; i <= row ; i++ ) {

       PRINTF("%10d   ",i);

       for ( j = 1 ; j <= col ; j++ ) {

          PRINTF("%10.5f   ",(*this)(i,j));

       }

       PRINTF("\n");

    }

}

/*##############################################################################
#                                                                              #
#                             MATRIX print                                     #
#                                                                              #
##############################################################################*/

void MATRIX::print(void)
{

    // Just use normal print, but send a null string

    PRINTF("");

}

#include "END_NAME_SPACE.H"

