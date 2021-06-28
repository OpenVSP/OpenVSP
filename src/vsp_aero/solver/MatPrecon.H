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

class MATPRECON {

private:

    int NumberOfVortexLoops_;
    int *VortexLoopList_;
    
    MATRIX *A_;

    VSPAERO_DOUBLE *x_;

public:

    MATPRECON(void);
   ~MATPRECON(void);
    MATPRECON(const MATPRECON &MatPrecon);

    MATPRECON& operator=(const MATPRECON &MatPrecon);
    
    void Size(int NumberOfVortexLoops);
    
    int NumberOfVortexLoops(void) { return NumberOfVortexLoops_; };
    
    int &VortexLoopList(int i) { return VortexLoopList_[i]; };

    MATRIX &A(void) { return *A_; };

    VSPAERO_DOUBLE &x(int i) { return x_[i]; };
    
    void LU(void) { A_->LU(); };

    void Solve(void) { A_->solve(x_); };

};

#endif
