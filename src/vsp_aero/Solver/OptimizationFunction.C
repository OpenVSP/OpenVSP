//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "OptimizationFunction.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                    OPTIMIZATION_FUNCTION Constructor                         #
#                                                                              #
##############################################################################*/

OPTIMIZATION_FUNCTION::OPTIMIZATION_FUNCTION(void)
{

    OptimizationFunction_ = OPT_CL;
    
    FunctionLength_ = 0;

    Wing_ = 1;

    Rotor_ = 1;
    
    Function_ = NULL;
    
    FunctionAverage_ = NULL;
    
    UserVector_ = NULL;
    
}

/*##############################################################################
#                                                                              #
#                     OPTIMIZATION_FUNCTION Destructor                         #
#                                                                              #
##############################################################################*/

OPTIMIZATION_FUNCTION::~OPTIMIZATION_FUNCTION(void)
{
       
    OptimizationFunction_ = 0;
    
    FunctionLength_ = 0;

    Wing_ = 0;

    Rotor_ = 0;
        
    if ( Function_  != NULL ) delete [] Function_;
    
    if ( FunctionAverage_  != NULL ) delete [] FunctionAverage_;
    
    if ( UserVector_ != NULL ) delete [] UserVector_;

}

/*##############################################################################
#                                                                              #
#                    OPTIMIZATION_FUNCTION Copy                                #
#                                                                              #
##############################################################################*/

OPTIMIZATION_FUNCTION::OPTIMIZATION_FUNCTION(const OPTIMIZATION_FUNCTION &Optfunction)
{

    // Use operator =

    (*this)= Optfunction;

}

/*##############################################################################
#                                                                              #
#                   OPTIMIZATION_FUNCTION Operator =                           #
#                                                                              #
##############################################################################*/

OPTIMIZATION_FUNCTION &OPTIMIZATION_FUNCTION::operator=(const OPTIMIZATION_FUNCTION &Optfunction)
{

    int i;
    
    OptimizationFunction_ = Optfunction.OptimizationFunction_;
    
    FunctionLength_ = Optfunction.FunctionLength_;

    Wing_ = Optfunction.Wing_;

    Rotor_ = Optfunction.Rotor_;
    
    if ( Function_ != NULL ) delete [] Function_;

    if ( FunctionAverage_ != NULL ) delete [] FunctionAverage_;
    
    if ( UserVector_ != NULL ) delete [] UserVector_;
    
    Function_ = new VSPAERO_DOUBLE[FunctionLength_ + 1];

    FunctionAverage_ = new VSPAERO_DOUBLE[FunctionLength_ + 1];
    
    UserVector_ = new double[FunctionLength_ + 1];
    
    for ( i = 1 ; i <= FunctionLength_ ; i++ ) {
    
       Function_[i] = Optfunction.Function_[i];
       
       FunctionAverage_[i] = Optfunction.FunctionAverage_[i];
       
       UserVector_[i] = Optfunction.UserVector_[i];
       
    }

    return *this;

}

/*##############################################################################
#                                                                              #
#                    OPTIMIZATION_FUNCTION SetFunctionLength                   #
#                                                                              #
##############################################################################*/

void OPTIMIZATION_FUNCTION::SetFunctionLength(int Length)
{

    int i;
    
    if ( Function_ != NULL ) delete [] Function_;

    if ( FunctionAverage_ != NULL ) delete [] FunctionAverage_;

    if ( UserVector_ != NULL ) delete [] UserVector_;

    FunctionLength_ = Length;
    
    Function_ = new VSPAERO_DOUBLE[FunctionLength_ + 1];

    FunctionAverage_ = new VSPAERO_DOUBLE[FunctionLength_ + 1];
    
    UserVector_ = new double[FunctionLength_ + 1];
    
    for ( i = 1 ; i <= FunctionLength_ ; i++ ) {
       
       Function_[i] = 0.;
       
       FunctionAverage_[i] = 0.;
       
       UserVector_[i] = 1.;
       
    }
    
}

#include "END_NAME_SPACE.H"
