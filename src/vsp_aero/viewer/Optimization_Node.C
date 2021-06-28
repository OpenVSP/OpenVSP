//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "Optimization_Node.H"

/*##############################################################################
#                                                                              #
#                              OPT_NODE constructor                            #
#                                                                              #
##############################################################################*/

OPT_NODE::OPT_NODE(void)
{

    NumberOfOptNodes_ = 0;

    x_ = NULL;
    y_ = NULL;         
    z_ = NULL;
    
    dFdx_ = NULL;
    dFdy_ = NULL;
    dFdz_ = NULL; 
    
    Psi_ = NULL;

}

/*##############################################################################
#                                                                              #
#                                 OPT_NODE Copy                                #
#                                                                              #
##############################################################################*/

OPT_NODE::OPT_NODE(const OPT_NODE &OPTNode)
{

    // Just use operator = code
    
    *this = OPTNode;

}

/*##############################################################################
#                                                                              #
#                             OPT_NODE SizeList                                #
#                                                                              #
##############################################################################*/

void OPT_NODE::SizeList(int NumberOfOptNodes)
{

    NumberOfOptNodes_= NumberOfOptNodes;

    if ( x_      != NULL ) delete [] x_;         
    if ( y_      != NULL ) delete [] y_;       
    if ( z_      != NULL ) delete [] z_;        
              
    if ( dFdx_   != NULL ) delete [] dFdx_;  
    if ( dFdy_   != NULL ) delete [] dFdy_;  
    if ( dFdz_   != NULL ) delete [] dFdz_;   
     
    x_ = new double[NumberOfOptNodes_ + 1];
    y_ = new double[NumberOfOptNodes_ + 1];     
    z_ = new double[NumberOfOptNodes_ + 1];     
                
    dFdx_ = new double[NumberOfOptNodes_ + 1];  
    dFdy_ = new double[NumberOfOptNodes_ + 1];  
    dFdz_ = new double[NumberOfOptNodes_ + 1];   
    
    Psi_ = new double[NumberOfOptNodes_ + 1];

}

/*##############################################################################
#                                                                              #
#                            OPT_NODE operator=                                #
#                                                                              #
##############################################################################*/

OPT_NODE& OPT_NODE::operator=(const OPT_NODE &OPTNode)
{

    int i;
    
    NumberOfOptNodes_ = OPTNode.NumberOfOptNodes_;
    
    SizeList(NumberOfOptNodes_);
    
    for ( i = 1 ; i <= NumberOfOptNodes_ ; i ++ ) {
       
       x_[i]    = OPTNode.x_[i];                 
       y_[i]    = OPTNode.y_[i];               
       z_[i]    = OPTNode.z_[i];   
                          
       dFdx_[i] = OPTNode.dFdx_[i];    
       dFdy_[i] = OPTNode.dFdy_[i];   
       dFdz_[i] = OPTNode.dFdz_[i];    

       Psi_[i]  = OPTNode.Psi_[i];    

    }

    return *this;

}

/*##############################################################################
#                                                                              #
#                               OPT_NODE destructor                            #
#                                                                              #
##############################################################################*/

OPT_NODE::~OPT_NODE(void)
{

    NumberOfOptNodes_= 0;;

    if ( x_     != NULL ) delete [] x_;         
    if ( y_     != NULL ) delete [] y_;         
    if ( z_     != NULL ) delete [] z_;                 
    if ( dFdx_  != NULL ) delete [] dFdx_;  
    if ( dFdy_  != NULL ) delete [] dFdy_;  
    if ( dFdz_  != NULL ) delete [] dFdz_;   
    if ( Psi_   != NULL ) delete [] Psi_;   
 
}


