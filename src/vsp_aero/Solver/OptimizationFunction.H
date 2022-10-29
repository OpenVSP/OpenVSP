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

// CL, CD, CM

#define OPT_CL_TOTAL                            1
#define OPT_CD_TOTAL                            2
#define OPT_CS_TOTAL                            3

#define OPT_CL_INVISCID                         4
#define OPT_CD_INVISCID                         5
#define OPT_CS_INVISCID                         6

#define OPT_CL_VISCOUS                          7
#define OPT_CD_VISCOUS                          8
#define OPT_CS_VISCOUS                          9

// CX, CY, CZ
   
#define OPT_CX_TOTAL                           21
#define OPT_CY_TOTAL                           22
#define OPT_CZ_TOTAL                           23

#define OPT_CX_INVISCID                        24
#define OPT_CY_INVISCID                        25
#define OPT_CZ_INVISCID                        26

#define OPT_CX_VISCOUS                         27
#define OPT_CY_VISCOUS                         28
#define OPT_CZ_VISCOUS                         29
          
// CMX, CMY, CMZ
             
#define OPT_CMX_TOTAL                          31
#define OPT_CMY_TOTAL                          32
#define OPT_CMZ_TOTAL                          33
                                             
#define OPT_CMX_INVISCID                       34
#define OPT_CMY_INVISCID                       35
#define OPT_CMZ_INVISCID                       36
                                            
#define OPT_CMX_VISCOUS                        37
#define OPT_CMY_VISCOUS                        38
#define OPT_CMZ_VISCOUS                        39
      
// Wing loading
      
#define OPT_WING_CL_TOTAL                    2001                               
#define OPT_WING_CX_TOTAL                    2002
#define OPT_WING_CY_TOTAL                    2003
#define OPT_WING_CZ_TOTAL                    2004

#define OPT_WING_CL_INVISCID                 2011                               
#define OPT_WING_CX_INVISCID                 2012
#define OPT_WING_CY_INVISCID                 2013
#define OPT_WING_CZ_INVISCID                 2014

#define OPT_WING_CL_VISCOUS                  2021                               
#define OPT_WING_CX_VISCOUS                  2022
#define OPT_WING_CY_VISCOUS                  2023
#define OPT_WING_CZ_VISCOUS                  2024

// Rotor
                                      
#define OPT_ROTOR_CT_TOTAL                   3001
#define OPT_ROTOR_CP_TOTAL                   3002

#define OPT_ROTOR_CT_INVISCID                3011
#define OPT_ROTOR_CP_INVISCID                3012

#define OPT_ROTOR_CT_VISCOUS                 3021
#define OPT_ROTOR_CP_VISCOUS                 3022

// WRT free stream inputs

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

// WRT Control surfaces
                              
#define OPT_GRADIENT_WRT_CONTROL_SURFS         11
                
// Residual
                                           
#define OPT_RESIDUAL                        30000
                                             
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

     * OPT_CL_TOTAL                      
     * OPT_CD_TOTAL                      
     * OPT_CS_TOTAL                      
     *
     * OPT_CL_INVISCID                   
     * OPT_CD_INVISCID                   
     * OPT_CS_INVISCID                   
     *
     * OPT_CL_VISCOUS                    
     * OPT_CD_VISCOUS                    
     * OPT_CS_VISCOUS                    
     *
     * OPT_CX_TOTAL                      
     * OPT_CY_TOTAL                      
     * OPT_CZ_TOTAL                      
     *
     * OPT_CX_INVISCID                   
     * OPT_CY_INVISCID                   
     * OPT_CZ_INVISCID                   
     *
     * OPT_CX_VISCOUS                    
     * OPT_CY_VISCOUS                    
     * OPT_CZ_VISCOUS                    
     *             
     * OPT_CMX_TOTAL                     
     * OPT_CMY_TOTAL                     
     * OPT_CMZ_TOTAL                     
     *                                   
     * OPT_CMX_INVISCID                  
     * OPT_CMY_INVISCID                  
     * OPT_CMZ_INVISCID                  
     *                                   
     * OPT_CMX_VISCOUS                   
     * OPT_CMY_VISCOUS                   
     * OPT_CMZ_VISCOUS                   
     *      
     * OPT_WING_CL_TOTAL                                              
     * OPT_WING_CX_TOTAL                 
     * OPT_WING_CY_TOTAL                 
     * OPT_WING_CZ_TOTAL                 
     *
     * OPT_WING_CL_INVISCID                                           
     * OPT_WING_CX_INVISCID              
     * OPT_WING_CY_INVISCID              
     * OPT_WING_CZ_INVISCID              
     *
     * OPT_WING_CL_VISCOUS                                            
     * OPT_WING_CX_VISCOUS               
     * OPT_WING_CY_VISCOUS               
     * OPT_WING_CZ_VISCOUS               
     *
     * OPT_ROTOR_CT_TOTAL                
     * OPT_ROTOR_CP_TOTAL                
     *
     * OPT_ROTOR_CT_INVISCID             
     * OPT_ROTOR_CP_INVISCID             
     *
     * OPT_ROTOR_CT_VISCOUS              
     * OPT_ROTOR_CP_VISCOUS              
     *
     * OPT_GRADIENT_WRT_ALPHA            
     * OPT_GRADIENT_WRT_BETA             
     * OPT_GRADIENT_WRT_MACH             
     * OPT_GRADIENT_WRT_VINF             
     * OPT_GRADIENT_WRT_DENSITY          
     * OPT_GRADIENT_WRT_RECREF           
     * OPT_GRADIENT_WRT_P_RATE           
     * OPT_GRADIENT_WRT_Q_RATE           
     * OPT_GRADIENT_WRT_R_RATE           
     *
     * OPT_GRADIENT_WRT_CONTROL_SURFS    
     *
     * Residual
     *
     * OPT_RESIDUAL                      
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
