// Header for Eminton-Lord zero-lift wave drag code from Sriram
// Rallabhandi
//
// wavedragEL.h
//
// Header written by Michael Waddington
//
//////////////////////////////////////////////////////////////////////

#ifndef WAVEDRAGEL_H
#define WAVEDRAGEL_H

#define _USE_MATH_DEFINES
#include <math.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"{
#endif


void matinv( double ***a, int prts, double *rhs, double **soln );

double emlord( int prts, double *naxs, double *sarea, double *r );

#ifdef __cplusplus
}
#endif

#endif // WAVEDRAGEL_H
