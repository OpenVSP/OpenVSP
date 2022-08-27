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

#define OPT_CL                                  1
#define OPT_CD                                  2
#define OPT_CS                                  3
   
#define OPT_CX                                  4
#define OPT_CY                                  5
#define OPT_CZ                                  6
                                               
#define OPT_CMX                                 7
#define OPT_CMY                                 8
#define OPT_CMZ                                 9
                                                
#define OPT_CD_CL_CM                           10
                                                
#define OPT_ROTOR_CT                           11
#define OPT_ROTOR_CP                           12
                                            
#define OPT_RESIDUAL                         1000
                                             
#define OPT_WING_LOAD                        2000                               
#define OPT_WING_CX                          2001
#define OPT_WING_CY                          2002
#define OPT_WING_CZ                          2003

#define OPT_GRADIENT_NUMBER_OF_INPUTS           9

#define OPT_GRADIENT_WRT_ALPHA                  1
#define OPT_GRADIENT_WRT_BETA                   2
#define OPT_GRADIENT_WRT_MACH                   3
#define OPT_GRADIENT_WRT_VINF                   4
#define OPT_GRADIENT_WRT_DENSITY                5
#define OPT_GRADIENT_WRT_RECREF                 6
#define OPT_GRADIENT_WRT_P_RATE                 7
#define OPT_GRADIENT_WRT_Q_RATE                 8
#define OPT_GRADIENT_WRT_R_RATE                 9

#define OPT_GRADIENT_WRT_CONTROL_SURFS         11

class OPTIMIZATION_FUNCTION {

private:

    int OptimizationFunction_;
    int FunctionLength_;
    int NumberOfTimeSteps_;
    int VectorLength_;
    
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
    
    /** Set the optimization function 
     * 
     * OPT_CL                         
     * OPT_CD                         
     * OPT_CS                         
     *                                
     * OPT_CMx                        
     * OPT_CMy                        
     * OPT_CMz                        
     *                                
     * OPT_CD_CL_CM                   
     *                                
     * OPT_ROTOR_CT                   
     * OPT_ROTOR_CP                   
     *                                
     * OPT_RESIDUAL                   
     *                                
     * OPT_WING_LOAD                                                
     * OPT_WING_CX                    
     * OPT_WING_CY                    
     * OPT_WING_CZ                    
     * 
     * OPT_GRADIENT_WRT_INPUTS        
     * OPT_GRADIENT_WRT_ROTOR_RPM     
     * OPT_GRADIENT_WRT_ALPHA         
     * OPT_GRADIENT_WRT_BETA          
     * OPT_GRADIENT_WRT_MACH          
     * OPT_GRADIENT_WRT_VINF          
     * OPT_GRADIENT_WRT_DENSITY       
     * OPT_GRADIENT_WRT_RECREF        
     * OPT_GRADIENT_WRT_P_RATE        
     * OPT_GRADIENT_WRT_Q_RATE        
     * OPT_GRADIENT_WRT_R_RATE        
     * OPT_GRADIENT_WRT_CONTROL_SURFS    
     * 
     **/
     
    void SetOptimizationFunction(int Function) { OptimizationFunction_ = Function; };
    
    /** Set the function length... ie, is it a scalar or a vector objective function **/
    
    void SetFunctionLength(int Length, int NumberOfTimeSteps);
    
    /** Set wing ID... for wing related objective functions. Zero for non wings **/
    
    void SetWing(int Wing) { Wing_ = Wing; };
    
    /** Set rotor ID... for wing related objective functions. Zero for non rotors **/
    
    void SetRotor(int Rotor) { Rotor_ = Rotor; };
    
    /** Return the optimization function **/

    int OptimizationFunction(void) { return OptimizationFunction_; };
    
    /** Return the wing ID **/
    
    int Wing(void) { return Wing_; };
    
    /** Return the rotor ID **/
    
    int Rotor(void) { return Rotor_; };
    
    /** Return the objective function length **/
    
    int FunctionLength(void) { return FunctionLength_; };
    
    /** Return the number of time steps used in time averaging **/
    
    int NumberOfTimeSteps(void) { return NumberOfTimeSteps_; };
    
    /** Return the total vector length ... this is NumberOfTimeSteps_ x FunctionLength_ **/
    
    int VectorLength(void) { return VectorLength_; };
    
    /** Return value of the single scalar function **/
    
    VSPAERO_DOUBLE &Function(void) { return Function_[1]; };    
    
    /** Return value of the i'th scalar function ... this may be a mix of time and space **/
    
    VSPAERO_DOUBLE &Function(int i) { return Function_[i]; };
    
    /** Return average value of the single scalar function for an unsteady analysis **/

    VSPAERO_DOUBLE &FunctionAverage(void) { return FunctionAverage_[1]; };    
    
    /** Return average value of the i'th scalar function  **/
    
    VSPAERO_DOUBLE &FunctionAverage(int i) { return FunctionAverage_[i]; };
         
    double &UserVector(int i) { return UserVector_[i]; };     
         
};

#include "END_NAME_SPACE.H"

#endif
