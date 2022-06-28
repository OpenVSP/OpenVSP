#include "search.H"

int leafs = 0;

/*##############################################################################

                        Function create_cfd_tree

Function Description:

The function creates a binary tree for the CFD grid. This is used to search
for the closest point to a given FEM grid point. This is used to transfer
the pressure from the CFD grid to the FEM grid.

Coded By: David J. Kinney
    Date: 12 - 30 - 1997

##############################################################################*/

LEAF *create_cfd_tree(INTERP_MESH *Mesh)
{

    int   i, j, k, p, ii, jj, num_nodes, iblank[4], sum, tri[2][3];
    int node1, node2, node3;
    LEAF *root;

    /* create and initialize the root level of the tree */

    root = ( LEAF * ) calloc( 1, sizeof(LEAF) );

    root->sort_direction = 0;

    root->number_of_nodes = Mesh->number_of_tris;

    root->node = (SNODE *) calloc( root->number_of_nodes + 1, sizeof(SNODE));

    num_nodes = 0;

    printf("Inserting %d triangles into tree... \n",Mesh->number_of_tris);

    for ( i = 1 ; i <= Mesh->number_of_tris ; i++ ) {

	    node1 = Mesh->TriList[i].node1;
	    node2 = Mesh->TriList[i].node2;
	    node3 = Mesh->TriList[i].node3;

       // Node 1

       root->node[i].node[0].node = node1;

       root->node[i].node[0].xyz[0] = Mesh->NodeList[node1].x;
       root->node[i].node[0].xyz[1] = Mesh->NodeList[node1].y;
       root->node[i].node[0].xyz[2] = Mesh->NodeList[node1].z;

       root->node[i].node[0].Variable[ 0] = Mesh->NodeList[node1].Cp;
       root->node[i].node[0].Variable[ 1] = Mesh->NodeList[node1].Cp_Unsteady;
       root->node[i].node[0].Variable[ 2] = Mesh->NodeList[node1].Gamma;

       // Node 2

       root->node[i].node[1].node = node2;

       root->node[i].node[1].xyz[0] = Mesh->NodeList[node2].x;
       root->node[i].node[1].xyz[1] = Mesh->NodeList[node2].y;
       root->node[i].node[1].xyz[2] = Mesh->NodeList[node2].z;

       root->node[i].node[1].Variable[ 0] = Mesh->NodeList[node2].Cp;
       root->node[i].node[1].Variable[ 1] = Mesh->NodeList[node2].Cp_Unsteady;
       root->node[i].node[1].Variable[ 2] = Mesh->NodeList[node2].Gamma;

       // Node 3

       root->node[i].node[2].node = node3;

       root->node[i].node[2].xyz[0] = Mesh->NodeList[node3].x;
       root->node[i].node[2].xyz[1] = Mesh->NodeList[node3].y;
       root->node[i].node[2].xyz[2] = Mesh->NodeList[node3].z;

       root->node[i].node[2].Variable[ 0] = Mesh->NodeList[node3].Cp;
       root->node[i].node[2].Variable[ 1] = Mesh->NodeList[node3].Cp_Unsteady;
       root->node[i].node[2].Variable[ 2] = Mesh->NodeList[node3].Gamma;

       // Centroid

       root->node[i].xyz[0] = Mesh->TriList[i].x;
       root->node[i].xyz[1] = Mesh->TriList[i].y;
       root->node[i].xyz[2] = Mesh->TriList[i].z;

       // Normal and area

       root->node[i].normal[0] = Mesh->TriList[i].nx;
       root->node[i].normal[1] = Mesh->TriList[i].ny;
       root->node[i].normal[2] = Mesh->TriList[i].nz;

       root->node[i].area = Mesh->TriList[i].area;

    }

    /* now create the rest of the tree - this is a recursive process */

    create_tree_leafs(root);

    printf("Created %d branches in binary tree \n",leafs);

    return(root);

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

void create_tree_leafs(LEAF *root)
{

    int   i, j, p, num_nodes, *perm, dir, icut, StopRecursion;
    LEAF  *left_leaf, *right_leaf;
    SNODE *temp_node;

    leafs++;

    if ( fmod(leafs,50) == 0 ) {

       printf("Created %d branches in binary tree \r",leafs);

    }

    /* sort direction */

    dir = root->sort_direction;


    /* left leaf */

    left_leaf = ( LEAF * ) calloc( 1, sizeof(LEAF) );

    left_leaf->left_leaf = NULL;

    left_leaf->right_leaf = NULL;

    left_leaf->sort_direction = dir + 1;

    if ( left_leaf->sort_direction > 2 ) left_leaf->sort_direction = 0;

    root->left_leaf = left_leaf;


    /* right leaf */

    right_leaf = ( LEAF * ) calloc( 1, sizeof(LEAF) );

    right_leaf->left_leaf = NULL;

    right_leaf->right_leaf = NULL;

    right_leaf->sort_direction = dir + 1;

    if ( right_leaf->sort_direction > 2 ) right_leaf->sort_direction = 0;

    root->right_leaf = right_leaf;


    /* sort the root nodes in increasing x, y, or z direction */

    perm = merge_sort(root);

    temp_node = (SNODE *) calloc(root->number_of_nodes + 1, sizeof(SNODE));

    for ( i = 1 ; i <= root->number_of_nodes ; i++ ) {

       temp_node[perm[i]] = root->node[i];

    }


    /* find dividing point in list for left and right leaves */

    icut = root->number_of_nodes/2;

    while ( ( temp_node[icut].xyz[dir] == temp_node[icut+1].xyz[dir] ) &&
            icut < root->number_of_nodes ) {

       icut++;

    }

    if ( icut == root->number_of_nodes ) {

       while ( ( temp_node[icut].xyz[dir] == temp_node[icut+1].xyz[dir] ) &&
               icut > 1 ) {

          icut--;

       }

    }

    StopRecursion = 0;

    if ( icut == 1 ) {

       StopRecursion = 1;

       icut = root->number_of_nodes;

    }

    root->cut_off_value = temp_node[icut].xyz[dir];


    /* pack the left leaf */

    left_leaf->number_of_nodes = icut;

    left_leaf->node = (SNODE *) calloc( left_leaf->number_of_nodes + 1, sizeof(SNODE));

    for ( i = 1 ; i <= icut ; i++ ) {

       left_leaf->node[i] = temp_node[i];

    }


    /* pack the right leaf */

    right_leaf->number_of_nodes = root->number_of_nodes - icut;

    right_leaf->node = (SNODE *) calloc( right_leaf->number_of_nodes + 1, sizeof(SNODE));

    for ( i = 1 ; i <= right_leaf->number_of_nodes ; i++ ) {

       right_leaf->node[i] = temp_node[icut + i];

    }


    /* free up some space */

    free(perm);

    free(temp_node);

    free(root->node);


    /* continue down left and right paths */

    if ( left_leaf->number_of_nodes > 25 ) create_tree_leafs(left_leaf);

    if ( right_leaf->number_of_nodes > 25 ) create_tree_leafs(right_leaf);

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

int *merge_sort(LEAF *root)
{
    int *list_1, *list_2, list_length, i;

    /* Space for lists needed by merge-sort algorithm */

    list_1 = (int *) calloc(root->number_of_nodes + 1, sizeof(int));

    list_2 = (int *) calloc(root->number_of_nodes + 1, sizeof(int));

    /* check that memory allocation was succesfull */

    if ( list_1 == NULL || list_2 == NULL ) {

       printf("Memory allocation failed in merge_sort! \n");

       exit(1);

    }

    /* intialize the lists */

    for ( i = 1 ; i <= root->number_of_nodes ; i++ ) {

        list_1[i] = i;

        list_2[i] = 0;

    }

    /* start with root->number_of_nodes lists of length 1 */

    list_length = 1;

    /* keep merging longer and longer lists (multiple of 2) until done */

    while ( list_length < root->number_of_nodes/2 ) {

          merge_lists(list_1,list_2,list_length,root);

          list_length = list_length * 2;

          merge_lists(list_2,list_1,list_length,root);

          list_length = list_length * 2;

    }

    /* one more pass if # of pts is odd or a multiple of 2 */

    if ( list_length < root->number_of_nodes ) {

       merge_lists(list_1,list_2,list_length,root);

       for ( i = 1 ; i <= root->number_of_nodes ; i++ ) {

          list_1[list_2[i]] = i;

       }

       free(list_2);

       return(list_1);

    }

    else {

       for ( i = 1 ; i <= root->number_of_nodes ; i++ ) {

          list_2[list_1[i]] = i;

       }

       free(list_1);

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

void merge_lists(int *list_1, int *list_2, int list_length, LEAF *root)
{

    int   list_1_front, list_1_end;
    int   list_2_front, list_2_end;
    int   new_list_front, i;
    float x_1, x_2;

    /* front position of new permutation vector */

    new_list_front = 1;

    /* front position of list_1 */

    list_1_front = 1;

    /* front position of list_2 */

    list_2_front = list_length + 1;

    /* merge lists while there still are elements in the lists */

    while ( list_1_front <= root->number_of_nodes ) {

       /* position for end of list 1 */

       list_1_end = list_1_front + list_length;

       /* if list 1 goes off end, set to end + 1 to stop next pass */

       if ( list_1_end > root->number_of_nodes ) {

          list_1_end = root->number_of_nodes + 1;

       }

       /* otherwise, set end position of list 2 */

       else {

          list_2_end = list_2_front + list_length;

          /* if list 2 goes off end, set to end + 1 */

          if ( list_2_end > root->number_of_nodes ) {

              list_2_end = root->number_of_nodes + 1;

          }

          /* merge lists while both are not empty */

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

       /* one and only one list may have some leftover stuff */

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

       /* set new positions for fronts of list 1 and 2 */

       list_1_front = list_2_front;

       list_2_front = list_2_front + list_length;

    }

}

/*##############################################################################

                        Function search_tree

Function Description:

The function searches through the binary tree for the closest point to
the point stored in the node structure.

Coded By: David J. Kinney
    Date: 12 - 30 - 1997

##############################################################################*/

int search_tree(LEAF *root, TNODE *node, double Tolerance)
{

    float ds;
    int   i;

    /* Don't search a NULL list */

    if (root == NULL ) return(0);


    /* either search list, or continue down the tree */

    if ( root->left_leaf == NULL && root->right_leaf == NULL ) {

       search_list(root,node);

    }

    else if ( node->xyz[root->sort_direction] <= root->cut_off_value ) {

       search_tree(root->left_leaf,node,Tolerance);

       ds = SQR(node->xyz[root->sort_direction] - root->cut_off_value);

       if ( ( node->found == 1 && ds <= node->normal_distance*node->search_radius ) ||
            ( node->found == 2 && ds <= node->distance       *node->search_radius ) ) {

          search_tree(root->right_leaf,node,Tolerance);

       }

       else if ( node->found == 0 ) {

          ds = SQR(node->xyz[root->sort_direction] - root->cut_off_value);

          if ( ds <= Tolerance ) {
   
             search_tree(root->right_leaf,node,Tolerance);
   
          }
             
       }
             
    }

    else {

       search_tree(root->right_leaf,node,Tolerance);

       ds = SQR(node->xyz[root->sort_direction] - root->cut_off_value);

       if ( ( node->found == 1 && ds <= node->normal_distance*node->search_radius ) ||
            ( node->found == 2 && ds <= node->distance       *node->search_radius ) ) {

          search_tree(root->left_leaf,node,Tolerance);

       }

       else if ( node->found == 0 ) {

          ds = SQR(node->xyz[root->sort_direction] - root->cut_off_value);

          if ( ds <= Tolerance ) {
   
             search_tree(root->left_leaf,node,Tolerance);
   
          }
             
       }
       
    }

    return(1);

}

/*##############################################################################

                        Function search_list

Function Description:

The function searches the final list of nodes held by the last leaft.

Coded By: David J. Kinney
    Date: 12 - 30 - 1997

##############################################################################*/

void search_list(LEAF *root, TNODE *node)
{

    int i;

    /* search the list */

    for ( i = 1 ; i <= root->number_of_nodes ; i++ ) {

       test_node(&(root->node[i]),node);

    }

}

/*##############################################################################

                        Function test_node

Function Description:

The function test a given node to see if it fits the user defined criteria.

Coded By: David J. Kinney
    Date: 1 - 5 - 1998

##############################################################################*/

void test_node(SNODE *snode, TNODE *tnode)
{

    float dot, angle, a_dist, distance, volume, vec1[3], vec2[3], vec3[3];
    float xyz[3], interp[MAX_VARIABLES], area[3], InterpWeight[3];
    int   i, j, ifound, stencil, InterpNode[3];

    /* check normals */

    vec1[0] = snode->normal[0];
    vec1[1] = snode->normal[1];
    vec1[2] = snode->normal[2];

    vec2[0] = tnode->normal[0];
    vec2[1] = tnode->normal[1];
    vec2[2] = tnode->normal[2];

    dot = vector_dot(vec1,vec2);

    angle = acos(dot) * 180. / PI;

    if ( ( dot > 0. && angle <= 25. ) || tnode->ignore_normals ) {
  //  if ( dot > 0. || tnode->ignore_normals ) {

       /* get absolute distance */

       a_dist = SQR(snode->xyz[0] - tnode->xyz[0])
              + SQR(snode->xyz[1] - tnode->xyz[1])
              + SQR(snode->xyz[2] - tnode->xyz[2]);

       /* check interpolation stencil */

       stencil = test_stencil(snode,tnode,area);

       distance = interpolate(snode,tnode,area,xyz,interp,InterpWeight,InterpNode);

       /* determine distance criteria */

       if ( stencil == 1 ) {

          if ( distance <= tnode->normal_distance ) {

             tnode->normal_distance = distance;

             tnode->found = 1;

             tnode->xyz_best[0] = xyz[0];
             tnode->xyz_best[1] = xyz[1];
             tnode->xyz_best[2] = xyz[2];

             tnode->InterpNode[0] = InterpNode[0];
             tnode->InterpNode[1] = InterpNode[1];
             tnode->InterpNode[2] = InterpNode[2];

             tnode->InterpWeight[0] = InterpWeight[0];
             tnode->InterpWeight[1] = InterpWeight[1];
             tnode->InterpWeight[2] = InterpWeight[2];
             
             tnode->DonorArea = snode->area;

             for ( j = 0 ; j < MAX_VARIABLES ; j++ ) {

                tnode->Variable[j] = interp[j];

             }

          }

       }

       else {

          if ( a_dist <= tnode->distance && tnode->found != 1 ) {

             tnode->distance = a_dist;

             tnode->found = 2;

             tnode->xyz_best[0] = xyz[0];
             tnode->xyz_best[1] = xyz[1];
             tnode->xyz_best[2] = xyz[2];

             tnode->InterpNode[0] = InterpNode[0];
             tnode->InterpNode[1] = InterpNode[1];
             tnode->InterpNode[2] = InterpNode[2];

             tnode->InterpWeight[0] = InterpWeight[0];
             tnode->InterpWeight[1] = InterpWeight[1];
             tnode->InterpWeight[2] = InterpWeight[2];
             
             tnode->DonorArea = snode->area;

             for ( j = 0 ; j < MAX_VARIABLES ; j++ ) {

                tnode->Variable[j] = interp[j];

			    }

          }

       }

    }

}

/*##############################################################################

                        Function test_stencil

Function Description:

The function checks the interpolate stencil to see if we are actually
extrapolating.

Coded By: David J. Kinney
    Date: 1 - 6 - 1998

##############################################################################*/

int test_stencil(SNODE *snode, TNODE *tnode, float *area)
{

    float vec1[3], vec2[3], tot_area, ratio_1, ratio_2;
    float normal1[3], normal2[3], normal3[3], normal4[3];
    int   check_sign;

    /* triangle 1 */

    vec1[0] = snode->node[1].xyz[0] - snode->node[0].xyz[0];
    vec1[1] = snode->node[1].xyz[1] - snode->node[0].xyz[1];
    vec1[2] = snode->node[1].xyz[2] - snode->node[0].xyz[2];

    vec2[0] = tnode->xyz[0] - snode->node[0].xyz[0];
    vec2[1] = tnode->xyz[1] - snode->node[0].xyz[1];
    vec2[2] = tnode->xyz[2] - snode->node[0].xyz[2];

    vector_cross(vec1,vec2,normal1);

    area[2] = vector_dot(normal1,snode->normal);

    /* triangle 2 */

    vec1[0] = snode->node[2].xyz[0] - snode->node[1].xyz[0];
    vec1[1] = snode->node[2].xyz[1] - snode->node[1].xyz[1];
    vec1[2] = snode->node[2].xyz[2] - snode->node[1].xyz[2];

    vec2[0] = tnode->xyz[0] - snode->node[1].xyz[0];
    vec2[1] = tnode->xyz[1] - snode->node[1].xyz[1];
    vec2[2] = tnode->xyz[2] - snode->node[1].xyz[2];

    vector_cross(vec1,vec2,normal2);

    area[0] = vector_dot(normal2,snode->normal);

    /* triangle 3 */

    vec1[0] = snode->node[0].xyz[0] - snode->node[2].xyz[0];
    vec1[1] = snode->node[0].xyz[1] - snode->node[2].xyz[1];
    vec1[2] = snode->node[0].xyz[2] - snode->node[2].xyz[2];

    vec2[0] = tnode->xyz[0] - snode->node[2].xyz[0];
    vec2[1] = tnode->xyz[1] - snode->node[2].xyz[1];
    vec2[2] = tnode->xyz[2] - snode->node[2].xyz[2];

    vector_cross(vec1,vec2,normal3);

    area[1] = vector_dot(normal3,snode->normal);

    /* area of complete triangle */

    vec1[0] = snode->node[1].xyz[0] - snode->node[0].xyz[0];
    vec1[1] = snode->node[1].xyz[1] - snode->node[0].xyz[1];
    vec1[2] = snode->node[1].xyz[2] - snode->node[0].xyz[2];

    vec2[0] = snode->node[2].xyz[0] - snode->node[0].xyz[0];
    vec2[1] = snode->node[2].xyz[1] - snode->node[0].xyz[1];
    vec2[2] = snode->node[2].xyz[2] - snode->node[0].xyz[2];

    vector_cross(vec1,vec2,normal4);

    tot_area = vector_dot(normal4,snode->normal);

    /* check interpolation stencil */

    check_sign = 0;

    if ( area[0] >= 0. ) check_sign++;
    if ( area[1] >= 0. ) check_sign++;
    if ( area[2] >= 0. ) check_sign++;

    ratio_1 = ( ABS(area[0]) + ABS(area[1]) + ABS(area[2]) )/ABS(tot_area);

    ratio_2 = 0.5*ABS( ABS(area[0]) + ABS(area[1]) + ABS(area[2]) - ABS(tot_area) )/tnode->area;

    if ( check_sign == 0 || check_sign == 3 ) return(1);

    if ( ratio_1 < 2.0 && ratio_2 < 4. ) return(1);

    return(0);

}

/*##############################################################################

                        Function interpolate

Function Description:

The function interpolates unknown from grid 1 to grid 2.

Coded By: David J. Kinney
    Date: 1 - 6 - 1998

##############################################################################*/

float interpolate(SNODE *snode, TNODE *tnode, float *area, float *xyz, float *interp, float *InterpWeight, int *InterpNode)
{

    int i;
    float p1, p2, p3, total_area, distance, MinV, MaxV;

    /* get total area */

    total_area = area[0] + area[1] + area[2];

    /* interpolate  xyz coordinates */

    p1 = area[0]*snode->node[0].xyz[0];
    p2 = area[1]*snode->node[1].xyz[0];
    p3 = area[2]*snode->node[2].xyz[0];

    xyz[0] = (p1 + p2 + p3)/total_area;

    p1 = area[0]*snode->node[0].xyz[1];
    p2 = area[1]*snode->node[1].xyz[1];
    p3 = area[2]*snode->node[2].xyz[1];

    xyz[1] = (p1 + p2 + p3)/total_area;

    p1 = area[0]*snode->node[0].xyz[2];
    p2 = area[1]*snode->node[1].xyz[2];
    p3 = area[2]*snode->node[2].xyz[2];

    xyz[2] = (p1 + p2 + p3)/total_area;

    distance = SQR(tnode->xyz[0] - xyz[0])
             + SQR(tnode->xyz[1] - xyz[1])
             + SQR(tnode->xyz[2] - xyz[2]);

    /* save the weights and nodes used to interpolate */

    InterpNode[0] = snode->node[0].node;
    InterpNode[1] = snode->node[1].node;
    InterpNode[2] = snode->node[2].node;

    InterpWeight[0] = area[0]/total_area;
    InterpWeight[1] = area[1]/total_area;
    InterpWeight[2] = area[2]/total_area;

    /* interpolate unknowns */

    for ( i = 0 ; i < MAX_VARIABLES ; i++ ) {

       p1 = area[0]*snode->node[0].Variable[i];
       p2 = area[1]*snode->node[1].Variable[i];
       p3 = area[2]*snode->node[2].Variable[i];

       interp[i] = (p1 + p2 + p3)/total_area;

       // Do not introduce any new min/max's

       MinV = MIN3(snode->node[0].Variable[i],snode->node[1].Variable[i],snode->node[2].Variable[i]);
       MaxV = MAX3(snode->node[0].Variable[i],snode->node[1].Variable[i],snode->node[2].Variable[i]);

       if ( interp[i] < MinV ) interp[i] = MinV;
       if ( interp[i] > MaxV ) interp[i] = MaxV;

    }

    return(distance);

}
