//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "Search.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                             SEARCH_LEAF constructor                          #
#                                                                              #
##############################################################################*/

SEARCH_LEAF::SEARCH_LEAF(void)
{

    sort_direction = 0;
    
    number_of_nodes = 0;
    
    level = 0;
    
    cut_off_value = 0.;
    
    node = NULL;
    
    left = NULL;
    
    right = NULL;    
    
}

/*##############################################################################
#                                                                              #
#                            SEARCH_LEAF destructor                            #
#                                                                              #
##############################################################################*/

SEARCH_LEAF::~SEARCH_LEAF(void)
{

    if (  node != NULL ) delete [] node;
    
    if (  left != NULL ) delete  left;
  
    if ( right != NULL ) delete right;
    
    node = NULL;
    
    left = NULL;
    
    right = NULL;        

}

/*##############################################################################
#                                                                              #
#                              SEARCH_LEAF Copy                                #
#                                                                              #
##############################################################################*/

SEARCH_LEAF::SEARCH_LEAF(const SEARCH_LEAF &SearchLeaf)
{
   
    PRINTF("Copy not implemented for SEARCH_LEAF class! \n");
    exit(1);
     
}

#include "END_NAME_SPACE.H"
