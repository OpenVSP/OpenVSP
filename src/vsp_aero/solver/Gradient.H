//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef GRADIENT_H
#define GRADIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "matrix.H"

// Definition of the GRADIENT class

class GRADIENT {

private:

    // Node based data
    
    int NumberOfLoops_;
    
    int NumberOfEquations_;

    int NumberOfDegreesOfFreedom_;
    
    int *LoopList_;
    
    MATRIX AA_;
    MATRIX At_;
    
    double *x_;
    double *b_;
    double Gradient_[3];

public:

    // Constructor, Destructor, Copy

    GRADIENT(void);
   ~GRADIENT(void);
    GRADIENT(const GRADIENT &Gradient);
    GRADIENT& operator=(const GRADIENT &Gradient);
    
    // Size the list
    
    void SizeList(int NumberOfLoops, int NumberOfEquations, int NumberOfDegreesOfFreedom);

    // Accesss to data
    
    int NumberOfLoops(void) { return NumberOfLoops_; };
    
    int NumberOfEquations(void) { return NumberOfEquations_; };
    
    int NumberOfDegreesOfFreedom(void) { return NumberOfDegreesOfFreedom_; };
    
    int &LoopList(int i) { return LoopList_[i]; };
 
    double &dv_dx(void) { return Gradient_[0]; };
    double &dv_dy(void) { return Gradient_[1]; };
    double &dv_dz(void) { return Gradient_[2]; };
    
    double *Vector(void) { return Gradient_; };

    MATRIX &At(void) { return At_; };
    MATRIX &AA(void) { return AA_; };
    
    double &b(int i) { return b_[i]; };
    
    void LU(void) { AA_.LU(); };
    void Solve(void);
    
};

#endif
