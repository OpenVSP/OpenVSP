//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "Gradient.H"

/*##############################################################################
#                                                                              #
#                              GRADIENT constructor                            #
#                                                                              #
##############################################################################*/

GRADIENT::GRADIENT(void)
{
    
    NumberOfLoops_ = 0;

    LoopList_ = NULL;
    
}

/*##############################################################################
#                                                                              #
#                                 GRADIENT Copy                                #
#                                                                              #
##############################################################################*/

GRADIENT::GRADIENT(const GRADIENT &Gradient)
{

    // Just use operator = code
    
    *this = Gradient;

}

/*##############################################################################
#                                                                              #
#                             GRADIENT SizeList                                #
#                                                                              #
##############################################################################*/

void GRADIENT::SizeList(int NumberOfLoops, int NumberOfEquations, int NumberOfDegreesOfFreedom)
{

    NumberOfLoops_ = NumberOfLoops;
    
    NumberOfEquations_ = NumberOfEquations;
    
    NumberOfDegreesOfFreedom_ = NumberOfDegreesOfFreedom;
    
    // Loop list
    
    LoopList_ = new int[NumberOfLoops_ + 1];
    
    zero_int_array(LoopList_, NumberOfLoops_);
    
    // Matrix

    AA_.size(NumberOfDegreesOfFreedom, NumberOfDegreesOfFreedom);
        
    At_.size(NumberOfDegreesOfFreedom, NumberOfEquations);
    
    x_ = new VSPAERO_DOUBLE[NumberOfDegreesOfFreedom + 1];
    
    zero_double_array(x_, NumberOfDegreesOfFreedom);
    
    b_ = new VSPAERO_DOUBLE[NumberOfEquations + 1];
    
    zero_double_array(b_, NumberOfEquations);
    
}

/*##############################################################################
#                                                                              #
#                            GRADIENT operator=                                #
#                                                                              #
##############################################################################*/

GRADIENT& GRADIENT::operator=(const GRADIENT &Gradient)
{

    int i;
    
    NumberOfLoops_ = Gradient.NumberOfLoops_;
    
    NumberOfEquations_ = Gradient.NumberOfEquations_;
    
    NumberOfDegreesOfFreedom_ = Gradient.NumberOfDegreesOfFreedom_;

    SizeList(NumberOfLoops_, NumberOfEquations_, NumberOfDegreesOfFreedom_);
    
    for ( i = 1 ; i <= NumberOfLoops_ ; i ++ ) {
       
       LoopList_[i] = Gradient.LoopList_[i];
       
    }

    for ( i = 1 ; i <= NumberOfEquations_ ; i ++ ) {
       
       b_[i] = Gradient.b_[i];
       
    }
    
    for ( i = 1 ; i <= NumberOfDegreesOfFreedom_ ; i ++ ) {
       
       x_[i] = Gradient.x_[i];
       
    }    
    
    Gradient_[0] = Gradient.Gradient_[0];
    Gradient_[1] = Gradient.Gradient_[1];
    Gradient_[2] = Gradient.Gradient_[2];

    AA_ = Gradient.AA_;
    At_ = Gradient.At_;
    
    return *this;

}

/*##############################################################################
#                                                                              #
#                               GRADIENT destructor                            #
#                                                                              #
##############################################################################*/

GRADIENT::~GRADIENT(void)
{

    NumberOfLoops_= 0;;

    if ( LoopList_ != NULL ) delete [] LoopList_;    
    if (        x_ != NULL ) delete [] x_;
    if (        b_ != NULL ) delete [] b_;

}

/*##############################################################################
#                                                                              #
#                               GRADIENT Solver                                #
#                                                                              #
##############################################################################*/

void GRADIENT::Solve(void)
{
 
    int i;
    MATRIX bb(NumberOfDegreesOfFreedom_), b(NumberOfEquations_);
    
    for ( i = 1 ; i <= NumberOfEquations_ ; i++ ) {
       
       b(i) = b_[i];
       
    }
    
    bb = At_ * b;
    
    for ( i = 1 ; i <= NumberOfDegreesOfFreedom_ ; i++ ) {
       
       x_[i] = bb(i);
       
    }
    
    AA_.solve(x_);
    
    Gradient_[0] = x_[1];
    Gradient_[1] = x_[2];
    Gradient_[2] = x_[3];

}



