//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

/* Header for
 *
 * Triangle/triangle intersection test routine,
 * by Tomas Moller, 1997.
 * See article "A Fast Triangle-Triangle Intersection Test",
 * Journal of Graphics Tools, 2(2), 1997
 * updated: 2001-06-20 (added line of intersection)
 *
 * jgt.akpeters.com/papers/Moller97/
 *
 * and
 *
 * intersect_triangle
 * by Tomas Moller and Ben Trumbore 1997.
 * See article "Fast, Minimum Storage Ray-Triangle Intersection",
 * Journal of Graphics Tools, 2(1), 1997
 *
 * jgt.akpeters.com/papers/MollerTrumbore97/
 *
 * Header created by J.R. Gloudemans
 *
 */
#ifndef TRIINT_H
#define TRIINT_H

#include <math.h>

int coplanar_tri_tri(double N[3],double V0[3],double V1[3],double V2[3],
                     double U0[3],double U1[3],double U2[3]);

int tri_tri_intersect(double V0[3],double V1[3],double V2[3],
                      double U0[3],double U1[3],double U2[3]);

int NoDivTriTriIsect(double V0[3],double V1[3],double V2[3],
                     double U0[3],double U1[3],double U2[3]);

void isect2(double VTX0[3],double VTX1[3],double VTX2[3],double VV0,double VV1,double VV2,
	    double D0,double D1,double D2,double *isect0,double *isect1,
		double isectpoint0[3],double isectpoint1[3]);

int tri_tri_intersect_with_isectline(double V0[3],double V1[3],double V2[3],
				     double U0[3],double U1[3],double U2[3],int *coplanar,
				     double isectpt1[3],double isectpt2[3]);

int intersect_triangle(double orig[3], double dir[3],
                   double vert0[3], double vert1[3], double vert2[3],
                   double *t, double *u, double *v);

bool intersectRayAABB( const double MinB[3], const double MaxB[3],
                   const double origin[3], const double dir[3],
                   double coord[3] );

#endif
