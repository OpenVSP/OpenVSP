#ifndef SEARCH_H
#define SEARCH_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp.H"

#define MAX_VARIABLES 30

/*##############################################################################

                        Triangle node data structure

##############################################################################*/

struct TRIANGLE_NODE {
	 int   node;
    float xyz[3];
    float Variable[MAX_VARIABLES];
};
typedef struct TRIANGLE_NODE TRI_NODE;

/*##############################################################################

                        Surface node data structure

##############################################################################*/

struct SURFACE_NODE {
    float    xyz[3];
    float    normal[3];
    float    area;
    TRI_NODE node[3];
    BBOX     box;
};
typedef struct SURFACE_NODE SNODE;

/*##############################################################################

                        Test node data structure

##############################################################################*/

struct TEST_NODE {
    BBOX  box;
    float xyz[3];
    float normal[3];
    float area;
    float xyz_best[3];
    float distance;
    float normal_distance;
    float Variable[MAX_VARIABLES];
    float InterpWeight[3];
    int   found;
    int   ignore_normals;
    int   search_radius;
    int   InterpNode[3];
    float DonorArea;
};
typedef struct TEST_NODE TNODE;

/*##############################################################################

                        Binary tree leaf data structure

##############################################################################*/

struct LEAF_STRUCTURE {
    int                     sort_direction;
    int                     number_of_nodes;
    int                     level;
    float                   cut_off_value;
    SNODE                   *node;
    struct LEAF_STRUCTURE   *left_leaf;
    struct LEAF_STRUCTURE   *right_leaf;
};
typedef struct LEAF_STRUCTURE LEAF;

/*##############################################################################

                        Function Prototypes

##############################################################################*/

LEAF *create_cfd_tree(INTERP_MESH *Mesh);

void create_tree_leafs(LEAF *root);

int *merge_sort(LEAF *leaf);

void merge_lists(int *list_1, int *list_2, int list_length, LEAF *leaf);

int search_tree(LEAF *root, TNODE *node, double Tolerance);

void search_list(LEAF *root, TNODE *node);

void test_node(SNODE *snode, TNODE *tnode);

int test_stencil(SNODE *snode, TNODE *tnode, float *area);

float interpolate(SNODE *snode, TNODE *tnode, float *area, float *xyz, float *interp, float *InterpWeight, int *InterpNode);

#endif



