//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef ADJOINT_GRADIENT_H
#define ADJOINT_GRADIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"

#include "START_NAME_SPACE.H"

// Definition of the FEM_NODE class

class ADJOINT_GRADIENT {

private:

    // Node based data
    
    int NumberOfDerivatives_;
    int NumberOfOptimizationFunctions_;
    int NumberOfTimeSteps_;

    VSPAERO_DOUBLE ***dF_dMesh_;
    VSPAERO_DOUBLE ***pF_pMesh_;         
    VSPAERO_DOUBLE ***pR_pMesh_;

public:

    // Constructor, Destructor, Copy

    ADJOINT_GRADIENT(void);
   ~ADJOINT_GRADIENT(void);
    ADJOINT_GRADIENT(const ADJOINT_GRADIENT &ADJOINT_GRADIENT);
    
    /** Size the list **/
    
    void SizeList(int NumberOfDerivatives, int NumberOfOptimizationFunctions_, int NumberOfTimeSteps);

    /** Number of derivatives in the list **/
    
    int NumberOfDerivatives(void) { return NumberOfDerivatives_; };
    
    /** Number of optimization functions in the list **/
    
    int NumberOfOptimizationFunctions(void) { return NumberOfOptimizationFunctions_; };
    
    /** Number of time steps in the list **/
    
    int NumberOfTimeSteps(void) { return NumberOfTimeSteps_; };
            
    /** i'th total derivative at t = Time, for optimization case = OptCase  **/

    VSPAERO_DOUBLE &dF_dMesh(int Time, int OptCase, int i) { return dF_dMesh_[Time][OptCase][i]; };

    /** i'th partial derivative of F wrt mesh at t = Time, for optimization case = OptCase  **/

    VSPAERO_DOUBLE &pF_pMesh(int Time, int OptCase, int i) { return pF_pMesh_[Time][OptCase][i]; };
    
    /** i'th partial derivative of R wrt mesh at t = Time, for optimization case = OptCase  **/

    VSPAERO_DOUBLE &pR_pMesh(int Time, int OptCase, int i) { return pR_pMesh_[Time][OptCase][i]; };    

    /** i'th total derivative at t = Time, for optimization case = OptCase  **/

    VSPAERO_DOUBLE &dF_dVariable(int Time, int OptCase, int i) { return dF_dMesh_[Time][OptCase][i]; };

    /** i'th partial derivative of F wrt mesh at t = Time, for optimization case = OptCase  **/

    VSPAERO_DOUBLE &pF_pVariable(int Time, int OptCase, int i) { return pF_pMesh_[Time][OptCase][i]; };
    
    /** i'th partial derivative of R wrt mesh at t = Time, for optimization case = OptCase  **/

    VSPAERO_DOUBLE &pR_pVariable(int Time, int OptCase, int i) { return pR_pMesh_[Time][OptCase][i]; };    
        
};

#include "END_NAME_SPACE.H"

#endif
