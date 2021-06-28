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

// Some common asserts

#define ASSERT_NULL(a) assert ( (a) != NULL )

#define TORAD (3.141592/180.)

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

// Some common utilities

void vector_cross(VSPAERO_DOUBLE *vec1, VSPAERO_DOUBLE *vec2, VSPAERO_DOUBLE *cross);

VSPAERO_DOUBLE vector_dot(VSPAERO_DOUBLE *vec1, VSPAERO_DOUBLE *vec2);

void zero_int_array(int *array, int size);
void zero_double_array(VSPAERO_DOUBLE *array, int size);

void zero_array(int *array, int size);
void zero_array(VSPAERO_DOUBLE *array, int size);

int* resize_int_array(int *array, int size, int new_size);
VSPAERO_DOUBLE* resize_double_array(VSPAERO_DOUBLE *array, int size, int new_size);

void in_sphere(VSPAERO_DOUBLE xyz[4][3], VSPAERO_DOUBLE xyz_in[3], VSPAERO_DOUBLE &radius_in);
void circ_sphere(VSPAERO_DOUBLE xyz[4][3], VSPAERO_DOUBLE xyz_out[3], VSPAERO_DOUBLE &radius_out);
void get_normals(VSPAERO_DOUBLE xyz[4][3], VSPAERO_DOUBLE normals[4][3]);
void determinant(VSPAERO_DOUBLE mat[4][4], VSPAERO_DOUBLE *det_matrix);

int tri_seg_int(VSPAERO_DOUBLE *p1, VSPAERO_DOUBLE *p2, VSPAERO_DOUBLE *p3, VSPAERO_DOUBLE *p4, VSPAERO_DOUBLE *p5,
                VSPAERO_DOUBLE *tt, VSPAERO_DOUBLE *uu, VSPAERO_DOUBLE *ww);

int prl_seg_int(VSPAERO_DOUBLE *p1, VSPAERO_DOUBLE *p2, VSPAERO_DOUBLE *p3, VSPAERO_DOUBLE *p4, VSPAERO_DOUBLE *p5, VSPAERO_DOUBLE *tt,
                VSPAERO_DOUBLE *uu, VSPAERO_DOUBLE *ww);

int compare_boxes(BBOX &box1, BBOX &box2);

int inside_box(BBOX &box, VSPAERO_DOUBLE xyz[3]);

VSPAERO_DOUBLE calculate_box_overlap(BBOX &box1, BBOX &box2);

VSPAERO_DOUBLE box_distance_ratio(BBOX &box, VSPAERO_DOUBLE xyz[3]);

void box_calculate_size(BBOX &box);

int lines_intersect(VSPAERO_DOUBLE *u, VSPAERO_DOUBLE *v, VSPAERO_DOUBLE *p, VSPAERO_DOUBLE *q, VSPAERO_DOUBLE &t1, VSPAERO_DOUBLE &t2,
                    VSPAERO_DOUBLE &ds1, VSPAERO_DOUBLE &ds2 );

int asearch(VSPAERO_DOUBLE value, const VSPAERO_DOUBLE * array, int dim, VSPAERO_DOUBLE &sa, int &bound);
VSPAERO_DOUBLE conesa(const VSPAERO_DOUBLE &mach, const VSPAERO_DOUBLE &cone_angle) ;

int Intersect2DLines(VSPAERO_DOUBLE *u, VSPAERO_DOUBLE *v, VSPAERO_DOUBLE *p, VSPAERO_DOUBLE *q, VSPAERO_DOUBLE Epsilson);
int CheckIfInsideTri(VSPAERO_DOUBLE *x, VSPAERO_DOUBLE *y, VSPAERO_DOUBLE *p, VSPAERO_DOUBLE Eps);

int Intersect2DLines(VSPAERO_DOUBLE *u, VSPAERO_DOUBLE *v, VSPAERO_DOUBLE *p, VSPAERO_DOUBLE *q, VSPAERO_DOUBLE Epsilson);
int CheckIfInsideTri(VSPAERO_DOUBLE *x, VSPAERO_DOUBLE *y, VSPAERO_DOUBLE *p, VSPAERO_DOUBLE Eps);

int PointIsBetweenPlanes(VSPAERO_DOUBLE *Plane_1_Normal, VSPAERO_DOUBLE *Plane_1_Point,
                         VSPAERO_DOUBLE *Plane_2_Normal, VSPAERO_DOUBLE *Plane_2_Point,
                         VSPAERO_DOUBLE *Point);
VSPAERO_DOUBLE PointIsOnRightSideOfPlane(VSPAERO_DOUBLE *PlaneNormal, VSPAERO_DOUBLE *PlanePoint, VSPAERO_DOUBLE *Point);
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

#include "matrix.H"

#endif
