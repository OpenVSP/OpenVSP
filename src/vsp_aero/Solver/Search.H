//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef SEARCH_H
#define SEARCH_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.H"
#include "SearchLeaf.H"
#include "VSP_Grid.H"

#include "START_NAME_SPACE.H"

// Definition of the SEARCH class

class SEARCH {

private:

    // Constructor, Destructor, Copy

    SEARCH_LEAF *root_;

    int leafs_;
 
    void create_tree_leafs(SEARCH_LEAF *root);
    
    int *merge_sort(SEARCH_LEAF *leaf);
    
    void merge_lists(int *list_1, int *list_2, int list_length, SEARCH_LEAF *leaf);
    
    int SearchTree_(SEARCH_LEAF *root, TEST_NODE &node);
    
    int SearchTreeNew_(SEARCH_LEAF *root, TEST_NODE &node);
    
    void search_list(SEARCH_LEAF *root, TEST_NODE &node);

    void test_node(SURFACE_NODE &SURFACE_NODE, TEST_NODE &TEST_NODE);

    double Tolerance_;
    
    double a_dist_;
  
public:

    SEARCH(void);
   ~SEARCH(void);
    SEARCH(const SEARCH &Search);
    
    /** Search the tree to find closest result to the TEST_NODE node **/

    int SearchTree(TEST_NODE &node) { return SearchTree_(root_, node); };

    /** Create a search tree given a surface mesh **/

    void CreateSearchTree(VSP_GRID &Grid);
    
    /** Set the tolerances on searches **/
    
    double &Tolerance(void) { return Tolerance_; };

};

#include "END_NAME_SPACE.H"

#endif



