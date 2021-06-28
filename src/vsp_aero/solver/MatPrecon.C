//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "MatPrecon.H"

/*##############################################################################
#                                                                              #
#                               MATPRECON Constructor                          #
#                                                                              #
##############################################################################*/

MATPRECON::MATPRECON(void)
{

    NumberOfVortexLoops_ = 0;
    
    A_ = NULL;
    
    x_ = NULL;
    
    VortexLoopList_ = NULL;

}

/*##############################################################################
#                                                                              #
#                               MATPRECON Destructor                           #
#                                                                              #
##############################################################################*/

MATPRECON::~MATPRECON(void)
{

    // Just zero the matrix

    if ( A_ != NULL ) delete A_;
    
    if ( x_ != NULL ) delete [] x_;
    
    if ( VortexLoopList_ != NULL ) delete [] VortexLoopList_;
  
}

/*##############################################################################
#                                                                              #
#                               MATPRECON Copy                                 #
#                                                                              #
##############################################################################*/

MATPRECON::MATPRECON(const MATPRECON &MatPrecon)
{

    // Use operator = 
    
    *this = MatPrecon;    

}

/*##############################################################################
#                                                                              #
#                               MATPRECON Operator =                           #
#                                                                              #
##############################################################################*/

MATPRECON &MATPRECON::operator=(const MATPRECON &MatPrecon)
{

    int i;

    NumberOfVortexLoops_ = MatPrecon.NumberOfVortexLoops_;
    
    A_ = MatPrecon.A_;

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       x_[i] = MatPrecon.x_[i];
       
       VortexLoopList_[i] = MatPrecon.VortexLoopList_[i];

    }

    return *this;

}

/*##############################################################################
#                                                                              #
#                               MATPRECON Size                                 #
#                                                                              #
##############################################################################*/

void MATPRECON::Size(int NumberOfVortexLoops)
{

    NumberOfVortexLoops_ = NumberOfVortexLoops;
    
    A_ = new MATRIX;
    
    A_->size(NumberOfVortexLoops_,NumberOfVortexLoops_);
    
    x_ = new VSPAERO_DOUBLE[NumberOfVortexLoops_ + 1];
    
    VortexLoopList_ = new int[NumberOfVortexLoops_ + 1];

}

