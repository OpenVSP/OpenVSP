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
#include <math.h>
#include <assert.h>

// Some common asserts

#define ASSERT_NULL(a) assert ( (a) != NULL )


// Bounding box

struct bounding_box
{
    float x_min;
    float x_max;
    float y_min;
    float y_max;
    float z_min;
    float z_max;
};

typedef struct bounding_box BBOX;

// Some common utilities

void vector_cross(float *vec1, float *vec2, float *cross);
void vector_cross(double *vec1, double *vec2, double *cross);
void vector_cross(long double *vec1, long double *vec2, long double *cross);

float vector_dot(float *vec1, float *vec2);
double vector_dot(double *vec1, double *vec2);
long double vector_dot(long double *vec1, long double *vec2);

void zero_int_array(int *array, int size);
void zero_float_array(float *array, int size);

void zero_array(int *array, int size);
void zero_array(float *array, int size);
void zero_array(double *array, int size);
void zero_array(long double *array, int size);

int* resize_int_array(int *array, int size, int new_size);
float* resize_float_array(float *array, int size, int new_size);

void in_sphere(float xyz[4][3], float xyz_in[3], float &radius_in);
void circ_sphere(float xyz[4][3], float xyz_out[3], float &radius_out);
void get_normals(float xyz[4][3], float normals[4][3]);
void determinant(float mat[4][4], float *det_matrix);

int tri_seg_int(float *p1, float *p2, float *p3, float *p4, float *p5, float *tt,
                float *uu, float *ww);

int prl_seg_int(float *p1, float *p2, float *p3, float *p4, float *p5, float *tt,
                float *uu, float *ww);

int compare_boxes(BBOX box1, BBOX box2);

int lines_intersect(float *u, float *v, float *p, float *q, float &t1, float &t2,
                    float &ds1, float &ds2 );

int asearch(float value, const float * array, int dim, float &sa, int &bound);
float conesa(const float &mach, const float &cone_angle) ;

// Some commonly used math operations and fixed constants

#ifndef PI
#define PI 3.141592
#endif

#define ABS(a)   ( (a)<0 ? -(a) : (a) )

#define SQR(a) ( (a)*(a) )

#define SGN(a) ( (a)<0 ? (-1) : (1) )

#define MIN(a,b) ( (a)<=(b) ? (a) : (b) )
#define MAX(a,b) ( (a)>=(b) ? (a) : (b) )

#define MIN3(a,b,c) ( MIN( MIN( (a),(b) ) , (c) ) )
#define MAX3(a,b,c) ( MAX( MAX( (a),(b) ) , (c) ) )

#define MIN4(a,b,c,d) ( MIN( MIN( (a),(b) ) , MIN( (c),(d) ) ) )
#define MAX4(a,b,c,d) ( MAX( MAX( (a),(b) ) , MAX( (c),(d) ) ) )

#define MIN5(a,b,c,d,e) ( MIN( MIN( MIN((a),(b)) , MIN((c),(d)) ), (e) ) )
#define MAX5(a,b,c,d,e) ( MAX( MAX( MAX((a),(b)) , MAX((c),(d)) ), (e) ) )

#define MIN6(a,b,c,d,e,f) ( MIN( MIN( MIN( MIN((a),(b)) , MIN((c),(d)) ), (e) ), (f) ) )
#define MAX6(a,b,c,d,e,f) ( MAX( MAX( MAX( MAX((a),(b)) , MAX((c),(d)) ), (e) ), (f) ) )

#define PASS(a) ( (a)>0 ? 1 : 0 )

#define THREE(a) ( (a)*(a)*(a)             )

#define FOUR(a)  ( (a)*(a)*(a)*(a)         )

#define FIVE(a)  ( (a)*(a)*(a)*(a)*(a)     )

#define SIX(a)   ( (a)*(a)*(a)*(a)*(a)*(a) )

#define CUBE(a)  ( (a)*(a)*(a)             )

#define CONSTANT_PT 1
#define CONSTANT_PH 2
#define CONSTANT_PS 3
#define CONSTANT_HS 4

#endif
