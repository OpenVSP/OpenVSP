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

#include "START_NAME_SPACE.H"

// Some common asserts

#define ASSERT_NULL(a) assert ( (a) != NULL )

#define TORAD (3.141592653589793/180.)

// Bounding box

struct bounding_box
{
    double x_min;
    double x_max;
    double y_min;
    double y_max;
    double z_min;
    double z_max;
    
    double Length_Squared;
};

typedef struct bounding_box BBOX;

/** Vector cross ... v3 = v1 x v2 **/

void vector_cross(double *vec1, double *vec2, double *cross);

/** Vector dot ... results = v1.v2 **/

double vector_dot(double *vec1, double *vec2);

/** Zero integer array of length size, the array is zeroed from 0 to size **/

void zero_int_array(int *array, int size);

/** Zero double array  of length size, the array is zeroed from 0 to size **/

void zero_double_array(double *array, int size);

/** Zero integer array of length size, the array is zeroed from 0 to size **/

void zero_array(int *array, int size);

/** Zero double array  of length size, the array is zeroed from 0 to size **/

void zero_array(double *array, int size);

/** Resize an integer array from size to new size and copy over the old data ... old array is deleted ... new array is returned **/

int* resize_int_array(int *array, int size, int new_size);

/** Resize an double array from size to new size and copy over the old data ... old array is deleted ... new array is returned **/

double* resize_double_array(double *array, int size, int new_size);

/** Calculate the insphere for a tetrahedra defined by nodes xyz... returns the center of the insphere and it's radius... if said sphere exists **/

void in_sphere(double xyz[4][3], double xyz_in[3], double &radius_in);

/** Calculate the circumsphere for a tetrahedra defined by nodes xyz... returns the center of the insphere and it's radius... if said sphere exists **/

void circ_sphere(double xyz[4][3], double xyz_out[3], double &radius_out);

/** Calculate outward facing normals on all four sides of a tetrahedra defined by nodes xyz ... **/

void get_normals(double xyz[4][3], double normals[4][3]);

/** Calculate the determinant of a 4x4 system **/

void determinant(double mat[4][4], double *det_matrix);

/** Find the intersection of a tri and a finite segment... if it exists **/

int tri_seg_int(double *p1, double *p2, double *p3, double *p4, double *p5,
                double *tt, double *uu, double *ww);

/** Find the intesection of a parallelogram and a finite segment if it exists 
 * we assume it's a plane and break it into two tris ... **/

int prl_seg_int(double *p1, double *p2, double *p3, double *p4, double *p5, double *tt,
                double *uu, double *ww);

/** compare two bounding boxes for overlap **/

int compare_boxes(BBOX &box1, BBOX &box2);

/** determine if a point ,xyz, is inside the bounding box **/

int inside_box(BBOX &box, double xyz[3]);

/** Estimate of the overlap between two bounding boxes **/

double calculate_box_overlap(BBOX &box1, BBOX &box2);

/** Estimate of the distance to a bounding box as a factor of the box size **/

double box_distance_ratio(BBOX &box, double xyz[3]);

/** Calculate the size of a bbox, squared... **/

void box_calculate_size(BBOX &box);

/** Determine if two planar lines intersect **/

int lines_intersect(double *u, double *v, double *p, double *q, double &t1, double &t2,
                    double &ds1, double &ds2 );

/** search a 1D array and find the 2 indices that boond the input search value **/

int asearch(double value, const double * array, int dim, double &sa, int &bound);

/** Determine if two planar lines intersect **/

int Intersect2DLines(double *u, double *v, double *p, double *q, double Epsilson);

/** Check if a point p is withing a 2D tri defined by points x and y **/

int CheckIfInsideTri(double *x, double *y, double *p, double Eps);

/** Determine if a point lies between 2 planes, defined by a point on each plane and their normals **/

int PointIsBetweenPlanes(double *Plane_1_Normal, double *Plane_1_Point,
                         double *Plane_2_Normal, double *Plane_2_Point,
                         double *Point);

/** Determine which side of a plane a point sits **/
                         
double PointIsOnRightSideOfPlane(double *PlaneNormal, double *PlanePoint, double *Point);

/** Find intesection of a point with a plane **/

int PlaneSegmentIntersection(double *PlaneNormal, double *PlanePoint, double *LinePoint_1, double *LinePoint_2, double &s);

// Some commonly used math operations and fixed constants

#ifndef PI
#define PI 3.141592
#endif

int ABS(int a);

double ABS(double a);

double SQR(double a);

int SGN(int a);

double SGN(double a);

int MIN(int a, int b);
int MAX(int a, int b);

double MIN(double a, double b);
double MAX(double a, double b);

int MIN3(int a, int b, int c);
int MAX3(int a, int b, int c);

double MIN3(double a, double b, double c);
double MAX3(double a, double b, double c);

#include "END_NAME_SPACE.H"

#include "matrix.H"

#endif
