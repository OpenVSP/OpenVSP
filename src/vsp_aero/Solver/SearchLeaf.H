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

#include "START_NAME_SPACE.H"

class SURFACE_NODE {

public:

    int id;

    double xyz[3];

};


class TEST_NODE {

public:

    double xyz[3];
    double distance;
    
    int found;
    int id;

};


class SEARCH_LEAF {
   
public:

    SEARCH_LEAF(void);
   ~SEARCH_LEAF(void);
    SEARCH_LEAF(const SEARCH_LEAF &SearchLeaf);

    /** Sort direction... x, y, or z **/
    
    int sort_direction;
    
    /** Number of nodes in this leaf **/
    
    int number_of_nodes;
    
    /** Level in the overall hierarchy of the search tree **/
    
    int level;
    
    /** Split point in x,y, or z for this leaf **/
    
    double cut_off_value;
    
    /** Node list **/
    
    SURFACE_NODE *node;
    
    /** Left child **/
    
    SEARCH_LEAF *left;
    
    /** Right child **/
    
    SEARCH_LEAF *right;
    
};

#include "END_NAME_SPACE.H"

#endif
