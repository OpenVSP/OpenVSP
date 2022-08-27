//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef MATPRECON_H
#define MATPRECON_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.H"
#include "matrix.H"

#include "START_NAME_SPACE.H"

class MATPRECON {

private:

    int NumberOfVortexLoops_;
    int *VortexLoopList_;
    int ThereIsTranspose_;
    
    MATRIX *A_;
    MATRIX *AT_;

    VSPAERO_DOUBLE *x_;

public:

    MATPRECON(void);
   ~MATPRECON(void);
    MATPRECON(const MATPRECON &MatPrecon);

    MATPRECON& operator=(const MATPRECON &MatPrecon);
    
    /** Size the list for the given number of vortex loops **/
    
    void Size(int NumberOfVortexLoops);
    
    /** Number of vortex loops for this matrix preconditioner **/
    
    int NumberOfVortexLoops(void) { return NumberOfVortexLoops_; };
    
    /** Vortex loop list **/
    
    int &VortexLoopList(int i) { return VortexLoopList_[i]; };
    
    /** The transpose has been calculated... used for the Adjoint solver **/
    
    int& ThereIsTranspose(void) { return ThereIsTranspose_; };
    
    /** Preconditioning matrix ... may be the LU decomposition **/

    MATRIX &A(void) { return *A_; };
    
    /** Transpose of preconditioning matrix... may be the LU decomposition **/
    
    MATRIX &AT(void) { return *AT_; };
    
    /** Right hand side vector... so we are solving v = (A^-1)x **/

    VSPAERO_DOUBLE &x(int i) { return x_[i]; };
    
    /** Form LU decomposition of A **/
    
    void LU(void) { A_->LU(); };
    
    /** Form LU decomposition of AT **/
    
    void LUT(void) { AT_->LU(); };
    
    /** Do matrix precondition of x **/

    void Solve(void) { A_->solve(x_); };
    
    /** Do matrix (tranpose) precondition of x**/
    
    void SolveT(void) { AT_->solve(x_); };

};

#include "END_NAME_SPACE.H"

#endif
