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
#include "utils.H"

#include "START_NAME_SPACE.H"

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
    
    VSPAERO_DOUBLE *x_;
    VSPAERO_DOUBLE *b_;
    VSPAERO_DOUBLE Gradient_[3];

public:

    // Constructor, Destructor, Copy

    GRADIENT(void);
   ~GRADIENT(void);
    GRADIENT(const GRADIENT &Gradient);
    GRADIENT& operator=(const GRADIENT &Gradient);
    
    /** Size the list... given number of Loops, equations, and degrees of freedom **/
    
    void SizeList(int NumberOfLoops, int NumberOfEquations, int NumberOfDegreesOfFreedom);

    /** Number of loops **/
    
    int NumberOfLoops(void) { return NumberOfLoops_; };
    
    /** Number of equations **/
    
    int NumberOfEquations(void) { return NumberOfEquations_; };
    
    /** Number of degrees of freedom **/
    
    int NumberOfDegreesOfFreedom(void) { return NumberOfDegreesOfFreedom_; };
    
    /** Loop list */
    
    int &LoopList(int i) { return LoopList_[i]; };
    
    /** Partial of function wrt x **/
 
    VSPAERO_DOUBLE &dv_dx(void) { return Gradient_[0]; };
    
    /** Partial of function wrt y **/
    
    VSPAERO_DOUBLE &dv_dy(void) { return Gradient_[1]; };

    /** Partial of function wrt z **/

    VSPAERO_DOUBLE &dv_dz(void) { return Gradient_[2]; };
    
    /** Gradient vector **/
    
    VSPAERO_DOUBLE *Vector(void) { return Gradient_; };
    
    /* Access to transposed matrix **/

    MATRIX &At(void) { return At_; };
    
    /** Access to matrix, note this may be the LU decomposition  **/
      
    MATRIX &AA(void) { return AA_; };
    
    /** Right hand side vector **/
    
    VSPAERO_DOUBLE &b(int i) { return b_[i]; };
    
    /** Form LU decomposition of A matrix **/
    
    void LU(void) { AA_.LU(); };
    
    /** Solve linear system for gradients **/
    
    void Solve(void);
    
};

#include "END_NAME_SPACE.H"

#endif
