#pragma once
#ifndef LIBIGL_PREDICATES
#define LIBIGL_PREDICATES

#ifdef __cplusplus
extern "C" {
#endif

void exactinit();

#ifdef LIBIGL_PREDICATES_USE_FLOAT
typedef float IGL_PREDICATES_REAL;
float orient2d(float pa[2], float pb[2], float pc[2]);
float orient3d(float pa[3], float pb[3], float pc[3], float pd[3]);
float incircle(float pa[2], float pb[2], float pc[2], float pd[2]);
float insphere(float pa[3], float pb[3], float pc[3], float pd[3], float pe[3]);
#else
typedef double IGL_PREDICATES_REAL;
double orient2d(double pa[2], double pb[2], double pc[2]);
double orient3d(double pa[3], double pb[3], double pc[3], double pd[3]);
double incircle(double pa[2], double pb[2], double pc[2], double pd[2]);
double insphere(double pa[3], double pb[3], double pc[3], double pd[3], double pe[3]);
#endif

#ifdef __cplusplus
}
#endif

#endif
