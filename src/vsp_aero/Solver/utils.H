//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////


#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include "VSPAERO_TYPES.H"

#include "START_NAME_SPACE.H"

// Some common asserts

#define ASSERT_NULL(a) assert ( (a) != NULL )

#define TORAD (3.141592653589793/180.)

// Bounding box

struct bounding_box
{
    VSPAERO_DOUBLE x_min;
    VSPAERO_DOUBLE x_max;
    VSPAERO_DOUBLE y_min;
    VSPAERO_DOUBLE y_max;
    VSPAERO_DOUBLE z_min;
    VSPAERO_DOUBLE z_max;
    
    VSPAERO_DOUBLE Length_Squared;
};

typedef struct bounding_box BBOX;

/** Vector cross ... v3 = v1 x v2 **/

void vector_cross(VSPAERO_DOUBLE *vec1, VSPAERO_DOUBLE *vec2, VSPAERO_DOUBLE *cross);

/** Vector dot ... results = v1.v2 **/

VSPAERO_DOUBLE vector_dot(VSPAERO_DOUBLE *vec1, VSPAERO_DOUBLE *vec2);

/** Zero integer array of length size, the array is zeroed from 0 to size **/

void zero_int_array(int *array, int size);

/** Zero VSPAERO_DOUBLE array  of length size, the array is zeroed from 0 to size **/

void zero_double_array(VSPAERO_DOUBLE *array, int size);

/** Zero integer array of length size, the array is zeroed from 0 to size **/

void zero_array(int *array, int size);

/** Zero VSPAERO_DOUBLE array  of length size, the array is zeroed from 0 to size **/

void zero_array(VSPAERO_DOUBLE *array, int size);

/** Resize an integer array from size to new size and copy over the old data ... old array is deleted ... new array is returned **/

int* resize_int_array(int *array, int size, int new_size);

/** Resize an VSPAERO_DOUBLE array from size to new size and copy over the old data ... old array is deleted ... new array is returned **/

VSPAERO_DOUBLE* resize_double_array(VSPAERO_DOUBLE *array, int size, int new_size);

/** Calculate the insphere for a tetrahedra defined by nodes xyz... returns the center of the insphere and it's radius... if said sphere exists **/

void in_sphere(VSPAERO_DOUBLE xyz[4][3], VSPAERO_DOUBLE xyz_in[3], VSPAERO_DOUBLE &radius_in);

/** Calculate the circumsphere for a tetrahedra defined by nodes xyz... returns the center of the insphere and it's radius... if said sphere exists **/

void circ_sphere(VSPAERO_DOUBLE xyz[4][3], VSPAERO_DOUBLE xyz_out[3], VSPAERO_DOUBLE &radius_out);

/** Calculate outward facing normals on all four sides of a tetrahedra defined by nodes xyz ... **/

void get_normals(VSPAERO_DOUBLE xyz[4][3], VSPAERO_DOUBLE normals[4][3]);

/** Calculate the determinant of a 4x4 system **/

void determinant(VSPAERO_DOUBLE mat[4][4], VSPAERO_DOUBLE *det_matrix);

/** Find the intersection of a tri and a finite segment... if it exists **/

int tri_seg_int(VSPAERO_DOUBLE *p1, VSPAERO_DOUBLE *p2, VSPAERO_DOUBLE *p3, VSPAERO_DOUBLE *p4, VSPAERO_DOUBLE *p5,
                VSPAERO_DOUBLE *tt, VSPAERO_DOUBLE *uu, VSPAERO_DOUBLE *ww);

/** Find the intesection of a parallelogram and a finite segment if it exists 
 * we assume it's a plane and break it into two tris ... **/

int prl_seg_int(VSPAERO_DOUBLE *p1, VSPAERO_DOUBLE *p2, VSPAERO_DOUBLE *p3, VSPAERO_DOUBLE *p4, VSPAERO_DOUBLE *p5, VSPAERO_DOUBLE *tt,
                VSPAERO_DOUBLE *uu, VSPAERO_DOUBLE *ww);

/** compare two bounding boxes for overlap **/

int compare_boxes(BBOX &box1, BBOX &box2);

/** determine if a point ,xyz, is inside the bounding box **/

int inside_box(BBOX &box, VSPAERO_DOUBLE xyz[3]);

/** Estimate of the overlap between two bounding boxes **/

VSPAERO_DOUBLE calculate_box_overlap(BBOX &box1, BBOX &box2);

/** Estimate of the distance to a bounding box as a factor of the box size **/

VSPAERO_DOUBLE box_distance_ratio(BBOX &box, VSPAERO_DOUBLE xyz[3]);

/** Calculate the size of a bbox, squared... **/

void box_calculate_size(BBOX &box);

/** Determine if two planar lines intersect **/

int lines_intersect(VSPAERO_DOUBLE *u, VSPAERO_DOUBLE *v, VSPAERO_DOUBLE *p, VSPAERO_DOUBLE *q, VSPAERO_DOUBLE &t1, VSPAERO_DOUBLE &t2,
                    VSPAERO_DOUBLE &ds1, VSPAERO_DOUBLE &ds2 );

/** search a 1D array and find the 2 indices that boond the input search value **/

int asearch(VSPAERO_DOUBLE value, const VSPAERO_DOUBLE * array, int dim, VSPAERO_DOUBLE &sa, int &bound);

/** Determine if two planar lines intersect **/

int Intersect2DLines(VSPAERO_DOUBLE *u, VSPAERO_DOUBLE *v, VSPAERO_DOUBLE *p, VSPAERO_DOUBLE *q, VSPAERO_DOUBLE Epsilson);

/** Check if a point p is withing a 2D tri defined by points x and y **/

int CheckIfInsideTri(VSPAERO_DOUBLE *x, VSPAERO_DOUBLE *y, VSPAERO_DOUBLE *p, VSPAERO_DOUBLE Eps);

/** Determine if a point lies between 2 planes, defined by a point on each plane and their normals **/

int PointIsBetweenPlanes(VSPAERO_DOUBLE *Plane_1_Normal, VSPAERO_DOUBLE *Plane_1_Point,
                         VSPAERO_DOUBLE *Plane_2_Normal, VSPAERO_DOUBLE *Plane_2_Point,
                         VSPAERO_DOUBLE *Point);

/** Determine which side of a plane a point sits **/
                         
VSPAERO_DOUBLE PointIsOnRightSideOfPlane(VSPAERO_DOUBLE *PlaneNormal, VSPAERO_DOUBLE *PlanePoint, VSPAERO_DOUBLE *Point);

/** Find intesection of a point with a plane **/

int PlaneSegmentIntersection(VSPAERO_DOUBLE *PlaneNormal, VSPAERO_DOUBLE *PlanePoint, VSPAERO_DOUBLE *LinePoint_1, VSPAERO_DOUBLE *LinePoint_2, VSPAERO_DOUBLE &s);

// Some commonly used math operations and fixed constants

#ifndef PI
#define PI 3.141592
#endif

int ABS(int a);

VSPAERO_DOUBLE ABS(VSPAERO_DOUBLE a);

VSPAERO_DOUBLE SQR(VSPAERO_DOUBLE a);

int SGN(int a);

VSPAERO_DOUBLE SGN(VSPAERO_DOUBLE a);

int MIN(int a, int b);
int MAX(int a, int b);

VSPAERO_DOUBLE MIN(VSPAERO_DOUBLE a, VSPAERO_DOUBLE b);
VSPAERO_DOUBLE MAX(VSPAERO_DOUBLE a, VSPAERO_DOUBLE b);

int MIN3(int a, int b, int c);
int MAX3(int a, int b, int c);

VSPAERO_DOUBLE MIN3(VSPAERO_DOUBLE a, VSPAERO_DOUBLE b, VSPAERO_DOUBLE c);
VSPAERO_DOUBLE MAX3(VSPAERO_DOUBLE a, VSPAERO_DOUBLE b, VSPAERO_DOUBLE c);

#include "END_NAME_SPACE.H"

#include "matrix.H"

#endif
