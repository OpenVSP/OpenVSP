//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef OPTIMIZATION_FUNCTION_H
#define OPTIMIZATION_FUNCTION_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.H"
#include "quat.H"

#include "START_NAME_SPACE.H"

#define OPT_CL        1
#define OPT_CD        2
#define OPT_CS        3
                     
#define OPT_CMx       4
#define OPT_CMy       5
#define OPT_CMz       6
                     
#define OPT_CD_CL_CM  8

#define OPT_ROTOR_CT  9
#define OPT_ROTOR_CP  10

#define OPT_WING_LOAD 11

#define OPT_RESIDUAL  12

#define OPT_WING_CX 13
#define OPT_WING_CY 14
#define OPT_WING_CZ 15

#define OPT_ROTOR_RPM 20

class OPTIMIZATION_FUNCTION {

private:

    int OptimizationFunction_;
    int FunctionLength_;
    
    int Wing_;
    int Rotor_;
    
    VSPAERO_DOUBLE *Function_;
    VSPAERO_DOUBLE *FunctionAverage_;
    double *UserVector_;
    
public:

    OPTIMIZATION_FUNCTION(void);
   ~OPTIMIZATION_FUNCTION(void);
    OPTIMIZATION_FUNCTION(const OPTIMIZATION_FUNCTION &Optfunction);

    OPTIMIZATION_FUNCTION& operator=(const OPTIMIZATION_FUNCTION &Optfunction);
    
    void SetOptimizationFunction(int Function) { OptimizationFunction_ = Function; };
    void SetFunctionLength(int Length);
    void SetWing(int Wing) { Wing_ = Wing; };
    void SetRotor(int Rotor) { Rotor_ = Rotor; };

    int OptimizationFunction(void) { return OptimizationFunction_; };
    int Wing(void) { return Wing_; };
    int Rotor(void) { return Rotor_; };
    
    int FunctionLength(void) { return FunctionLength_; };
    
    VSPAERO_DOUBLE &Function(void) { return Function_[1]; };    
    VSPAERO_DOUBLE &Function(int i) { return Function_[i]; };

    VSPAERO_DOUBLE &FunctionAverage(void) { return FunctionAverage_[1]; };    
    VSPAERO_DOUBLE &FunctionAverage(int i) { return FunctionAverage_[i]; };
         
    double &UserVector(int i) { return UserVector_[i]; };     
         
};

#include "END_NAME_SPACE.H"

#endif
