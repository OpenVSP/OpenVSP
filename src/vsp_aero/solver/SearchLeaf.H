//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef SEARCH_LEAF_H
#define SEARCH_LEAF_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.H"

class SURFACE_NODE {

public:

    int id;

    VSPAERO_DOUBLE xyz[3];

};


class TEST_NODE {

public:

    VSPAERO_DOUBLE xyz[3];
    VSPAERO_DOUBLE distance;
    
    int found;
    int id;

};


class SEARCH_LEAF {
   
public:

    SEARCH_LEAF(void);
   ~SEARCH_LEAF(void);
    SEARCH_LEAF(const SEARCH_LEAF &SearchLeaf);

    int sort_direction;
    int number_of_nodes;
    int level;
    
    VSPAERO_DOUBLE cut_off_value;
    
    SURFACE_NODE *node;
    
    SEARCH_LEAF *left;
    SEARCH_LEAF *right;
    
};


#endif
