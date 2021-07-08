//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "Search.H"

/*##############################################################################
#                                                                              #
#                                  SEARCH constructor                          #
#                                                                              #
##############################################################################*/

SEARCH::SEARCH(void)
{

    root_ = NULL;
    
    Tolerance_ = 1.e9;

}

/*##############################################################################
#                                                                              #
#                                 SEARCH destructor                            #
#                                                                              #
##############################################################################*/

SEARCH::~SEARCH(void)
{

    if ( root_ != NULL ) delete root_;
    
    root_ = NULL;

}

/*##############################################################################
#                                                                              #
#                                   SEARCH Copy                                #
#                                                                              #
##############################################################################*/

SEARCH::SEARCH(const SEARCH &Search)
{
   
    PRINTF("Copy not implemented for SEARCH class! \n");
    exit(1);
     
}

/*##############################################################################

                        Function CreateSearchTree

Function Description:

The function creates a binary tree for searching a list of xyz points... 
originally setup for search CFD meshes... 

##############################################################################*/

void SEARCH::CreateSearchTree(VORTEX_TRAIL &Trail, int NumberOfNodes)
{

    int i;
    
    leafs_ = 0;

    // create and initialize the root level of the tree

    root_ = new SEARCH_LEAF;

    root_->sort_direction = 0;

    root_->number_of_nodes = NumberOfNodes;

    root_->node = new SURFACE_NODE[root_->number_of_nodes + 1];

    for ( i = 1 ; i <= root_->number_of_nodes ; i++ ) {

       root_->node[i].xyz[0] = Trail.VortexEdge(i).Xc();
       root_->node[i].xyz[1] = Trail.VortexEdge(i).Yc();
       root_->node[i].xyz[2] = Trail.VortexEdge(i).Zc();  
       
       root_->node[i].id = i;

    }

    // now create the rest of the tree - this is a recursive process 

    if ( root_->number_of_nodes > 8 ) create_tree_leafs(root_);

}

/*##############################################################################

                        Function CreateSearchTree

Function Description:

The function creates a binary tree for searching a list of xyz points... 
originally setup for search CFD meshes... 

##############################################################################*/

void SEARCH::CreateSearchTree(VSP_GRID &Grid)
{

    int i, Node1, Node2;
    
    leafs_ = 0;

    // create and initialize the root level of the tree

    root_ = new SEARCH_LEAF;

    root_->sort_direction = 0;

    root_->number_of_nodes = Grid.NumberOfEdges();;

    root_->node = new SURFACE_NODE[root_->number_of_nodes + 1];

    for ( i = 1 ; i <= Grid.NumberOfEdges() ; i++ ) {

       Node1 = Grid.EdgeList(i).Node1();
       Node2 = Grid.EdgeList(i).Node2();
       
       root_->node[i].xyz[0] = 0.5*( Grid.NodeList(Node1).x() + Grid.NodeList(Node2).x() );
       root_->node[i].xyz[1] = 0.5*( Grid.NodeList(Node1).y() + Grid.NodeList(Node2).y() );
       root_->node[i].xyz[2] = 0.5*( Grid.NodeList(Node1).z() + Grid.NodeList(Node2).z() );

       root_->node[i].id = i;

    }

    // now create the rest of the tree - this is a recursive process 

    if ( root_->number_of_nodes > 8 ) create_tree_leafs(root_);

}

/*##############################################################################

                        Function create_tree_leafs

Function Description:

The function creates a binary tree for the CFD grid. This is used to search
for the closest point to a given FEM grid point. This is used to transfer
the pressure from the CFD grid to the FEM grid.

Coded By: David J. Kinney
    Date: 12 - 30 - 1997

##############################################################################*/

void SEARCH::create_tree_leafs(SEARCH_LEAF *root)
{

    int i, *perm, dir, icut;
    VSPAERO_DOUBLE Xmin, Ymin, Xmax, Ymax, Zmin, Zmax, MaxLength, Length[3];
    SEARCH_LEAF  *left, *right;
    SURFACE_NODE *temp_node;

    leafs_++;
    
    // Initialize leafs
    
    root->left = left = NULL;
    
    root->right = right = NULL;

    // sort the root nodes in increasing x, y, or z direction 

    perm = merge_sort(root);

    temp_node = new SURFACE_NODE[root->number_of_nodes + 1];

    for ( i = 1 ; i <= root->number_of_nodes ; i++ ) {

       temp_node[perm[i]] = root->node[i];

    }

    // find dividing point in list for left and right leaves 

    icut = root->number_of_nodes/2;

    while ( ( temp_node[icut].xyz[root->sort_direction] == temp_node[icut+1].xyz[root->sort_direction] ) &&
            icut < root->number_of_nodes - 1 ) {

       icut++;

    }

    if ( icut == root->number_of_nodes ) {

       while ( ( temp_node[icut].xyz[root->sort_direction] == temp_node[icut+1].xyz[root->sort_direction] ) &&
               icut > 1 ) {

          icut--;

       }

    }

    // find min/max 
    
    Xmin = Ymin = Zmin = 1.e9;
    
    Xmax = Ymax = Zmax = -1.e9;
    
    for ( i = 1 ; i <= root->number_of_nodes ; i++ ) {

       Xmin = MIN(root->node[i].xyz[0],Xmin);
       Xmax = MAX(root->node[i].xyz[0],Xmax);

       Ymin = MIN(root->node[i].xyz[1],Ymin);
       Ymax = MAX(root->node[i].xyz[1],Ymax);
       
       Zmin = MIN(root->node[i].xyz[2],Zmin);
       Zmax = MAX(root->node[i].xyz[2],Zmax);
       
    }
    
    Length[0] = Xmax - Xmin;
    Length[1] = Ymax - Ymin;
    Length[2] = Zmax - Zmin;
    
    // Sort in the max length direction on next pass
    
    MaxLength = Length[0]; dir = 0;
  
    if ( Length[1] > MaxLength ) { MaxLength = Length[1] ; dir = 1; };
    if ( Length[2] > MaxLength ) { MaxLength = Length[2] ; dir = 2; };
  
    if ( icut > 1 && icut < root->number_of_nodes ) {
       
       root->cut_off_value = temp_node[icut].xyz[root->sort_direction];
       
       // left leaf 
       
       left = new SEARCH_LEAF;

       left->sort_direction = dir;

       root->left = left;
   
       left->number_of_nodes = icut;
   
       left->node = new SURFACE_NODE[left->number_of_nodes + 1];
   
       for ( i = 1 ; i <= left->number_of_nodes ; i++ ) {
   
          left->node[i] = temp_node[i];
   
       }

       // right leaf 

       right = new SEARCH_LEAF;

       right->sort_direction = dir;

       root->right = right;
   
       right->number_of_nodes = root->number_of_nodes - icut;
   
       right->node = new SURFACE_NODE[right->number_of_nodes + 1];
   
       for ( i = 1 ; i <= right->number_of_nodes ; i++ ) {
   
          right->node[i] = temp_node[icut + i];
   
       }
              
    }

    // free up some space 
    
    delete [] perm;
    
    delete [] temp_node;
    
    if ( left != NULL || right != NULL ) {
       
       delete [] root->node;
    
       root->node = NULL;
       
    }

    // continue down left and right paths 

    if ( left != NULL && left->number_of_nodes > 8 ) create_tree_leafs(left);

    if ( right != NULL && right->number_of_nodes > 8 ) create_tree_leafs(right);

}

/*##############################################################################

                        Function merge_sort

Function Description:

The function re-orders the grid points using Merge-Sort Algorithm.  This is
a very efficient algorithm, taking N*logN (base 2) operations to finish the
sorting process.  The algorithm works by merging already ordered lists into
a new single list.  We start by taking N points and making up N lists of
length 1 (these lists are therefore already sorted).  We then make up N/2
lists of length 2 that are sorted, then N/4 lists of length 4, etc. and so
on until we have a list of length N that is sorted.  If N is odd there is
some extra logic (one more pass of the list sort step).

Coded By: David J. Kinney
    Date: 11 - 2 - 1994

##############################################################################*/

int *SEARCH::merge_sort(SEARCH_LEAF *root)
{
    int *list_1, *list_2, list_length, i;

    // Space for lists needed by merge-sort algorithm 

    list_1 = new int[root->number_of_nodes + 1];

    list_2 = new int[root->number_of_nodes + 1];

    // check that memory allocation was succesfull 

    if ( list_1 == NULL || list_2 == NULL ) {

       PRINTF("Memory allocation failed in merge_sort! \n");

       exit(1);

    }

    // intialize the lists 

    for ( i = 1 ; i <= root->number_of_nodes ; i++ ) {

        list_1[i] = i;

        list_2[i] = 0;

    }

    // start with root->number_of_nodes lists of length 1 

    list_length = 1;

    // keep merging longer and longer lists (multiple of 2) until done 

    while ( list_length < root->number_of_nodes/2 ) {

          merge_lists(list_1,list_2,list_length,root);

          list_length = list_length * 2;

          merge_lists(list_2,list_1,list_length,root);

          list_length = list_length * 2;

    }

    // one more pass if # of pts is odd or a multiple of 2 

    if ( list_length < root->number_of_nodes ) {

       merge_lists(list_1,list_2,list_length,root);

       for ( i = 1 ; i <= root->number_of_nodes ; i++ ) {

          list_1[list_2[i]] = i;

       }

       delete [] list_2;

       return(list_1);

    }

    else {

       for ( i = 1 ; i <= root->number_of_nodes ; i++ ) {

          list_2[list_1[i]] = i;

       }

       delete [] list_1;

       return(list_2);

    }

}

/*##############################################################################

                        Function merge_lists

Function Description:

The function is the heart of the merge-sort algorithm. Two list (perhaps of
different size) are given.  Each list is ordered in either increasing x,
y, or z -> depending on the value of root->sort_direction. The two lists are
then merged such that the final list is also ordered.

Coded By: David J. Kinney
    Date: 11 - 2 - 1994

##############################################################################*/

void SEARCH::merge_lists(int *list_1, int *list_2, int list_length, SEARCH_LEAF *root)
{

    int   list_1_front, list_1_end;
    int   list_2_front, list_2_end;
    int   new_list_front, i;
    VSPAERO_DOUBLE x_1, x_2;

    // front position of new permutation vector 

    new_list_front = 1;

    // front position of list_1 

    list_1_front = 1;

    // front position of list_2 

    list_2_front = list_length + 1;

    // merge lists while there still are elements in the lists 

    while ( list_1_front <= root->number_of_nodes ) {

       // position for end of list 1 

       list_1_end = list_1_front + list_length;

       // if list 1 goes off end, set to end + 1 to stop next pass 

       if ( list_1_end > root->number_of_nodes ) {

          list_1_end = root->number_of_nodes + 1;

       }

       // otherwise, set end position of list 2 

       else {

          list_2_end = list_2_front + list_length;

          // if list 2 goes off end, set to end + 1 

          if ( list_2_end > root->number_of_nodes ) {

              list_2_end = root->number_of_nodes + 1;

          }

          // merge lists while both are not empty 

          while ( list_1_front != list_1_end &&
                  list_2_front != list_2_end ) {

             x_1 = root->node[list_1[list_1_front]].xyz[root->sort_direction];

             x_2 = root->node[list_1[list_2_front]].xyz[root->sort_direction];

             if ( x_1 <= x_2 ) {

                list_2[new_list_front] = list_1[list_1_front];

                list_1_front++;

                new_list_front++;

             }

             else {

                list_2[new_list_front] = list_1[list_2_front];

                list_2_front++;

                new_list_front++;

             }

          }

       }

       // one and only one list may have some leftover stuff 

       if ( list_1_front < list_1_end ) {

          for ( i = list_1_front ; i < list_1_end ; i++ ) {

             list_2[new_list_front] = list_1[i];

             new_list_front++;

          }

          list_1_front = list_1_end;

       }

       else {

          for ( i = list_2_front ; i < list_2_end ; i++ ) {

             list_2[new_list_front] = list_1[i];

             new_list_front++;

          }

          list_2_front = list_2_end;

       }

       // set new positions for fronts of list 1 and 2 

       list_1_front = list_2_front;

       list_2_front = list_2_front + list_length;

    }

}

/*##############################################################################

                        Function SearchTree

Function Description:

The function searches through the binary tree for the closest point to
the point stored in the node structure.

Coded By: David J. Kinney
    Date: 12 - 30 - 1997

##############################################################################*/

int SEARCH::SearchTree_(SEARCH_LEAF *root, TEST_NODE &node)
{

    VSPAERO_DOUBLE ds;

    // Don't search a NULL list 

    if ( root == NULL ) return(0);

    // either search list, or continue down the tree 

    if ( root->left == NULL && root->right == NULL ) {

       search_list(root,node);

    }

    else if ( node.xyz[root->sort_direction] <= root->cut_off_value ) {

       SearchTree_(root->left,node);

       ds = SQR(node.xyz[root->sort_direction] - root->cut_off_value);

       if ( ds <= node.distance || node.xyz[root->sort_direction] == root->cut_off_value ) {

          SearchTree_(root->right,node);

       }

    }

    else {

       SearchTree_(root->right,node);

       ds = SQR(node.xyz[root->sort_direction] - root->cut_off_value);

       if ( ds <= node.distance || node.xyz[root->sort_direction] == root->cut_off_value ) {

          SearchTree_(root->left,node);

       }

    }

    return(1);

}

/*##############################################################################

                        Function search_list

Function Description:

The function searches the final list of nodes held by the last SEARCH_LEAFt.

Coded By: David J. Kinney
    Date: 12 - 30 - 1997

##############################################################################*/

void SEARCH::search_list(SEARCH_LEAF *root, TEST_NODE &node)
{

    int i;

    // search the list 

    for ( i = 1 ; i <= root->number_of_nodes ; i++ ) {

       test_node(root->node[i],node);

    }

}

/*##############################################################################

                        Function test_node

Function Description:

The function test a given node to see if it fits the user defined criteria.

Coded By: David J. Kinney
    Date: 1 - 5 - 1998

##############################################################################*/

void SEARCH::test_node(SURFACE_NODE &snode, TEST_NODE &tnode)
{

    VSPAERO_DOUBLE a_dist;

    // get absolute distance 

    a_dist = SQR(snode.xyz[0] - tnode.xyz[0])
           + SQR(snode.xyz[1] - tnode.xyz[1])
           + SQR(snode.xyz[2] - tnode.xyz[2]);

    if ( a_dist <= tnode.distance ) {

       tnode.distance = a_dist;
       
       tnode.id = snode.id;

       tnode.found = 1;

    }

}
