//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

/* Header for
 *
 * Triangle-Triangle Overlap Test Routines ,
 * by P. Guigue - O. Devillers, 1997.
 * See article "Fast and Robust Triangle-Triangle Overlap Test 
 *  Using Orientation Predicates",
 * Journal of Graphics Tools, 8(1), 2003
 * Updated December 2003
 *
 * Header created by Justin Gravett, ESAero (20 May, 2020)
 * 
 * Replaced Triangle/triangle intersection test routine,
 * by Tomas Moller, 1997.
 * See article "A Fast Triangle-Triangle Intersection Test",
 * Journal of Graphics Tools, 2(2), 1997
 * updated: 2001-06-20 (added line of intersection)
 *
 *
 */

#ifndef TRI_TRI_INTERSECT
#define TRI_TRI_INTERSECT


// Three-dimensional Triangle-Triangle Overlap Test
int tri_tri_overlap_test_3d(double p1[3], double q1[3], double r1[3], 
          double p2[3], double q2[3], double r2[3]);


// Three-dimensional Triangle-Triangle Overlap Test
// additionaly computes the segment of intersection of the two triangles if it exists. 
// coplanar returns whether the triangles are coplanar, 
// source and target are the endpoints of the line segment of intersection 
int tri_tri_intersection_test_3d(double p1[3], double q1[3], double r1[3], 
								 double p2[3], double q2[3], double r2[3],
								 int * coplanar, 
								 double source[3],double target[3]);


int coplanar_tri_tri3d(double  p1[3], double  q1[3], double  r1[3],
           double  p2[3], double  q2[3], double  r2[3],
           double  N1[3], double  N2[3]);


// Two dimensional Triangle-Triangle Overlap Test
int tri_tri_overlap_test_2d(double p1[2], double q1[2], double r1[2], 
          double p2[2], double q2[2], double r2[2]);

int ccw_tri_tri_intersection_2d( double p1[2], double q1[2], double r1[2],
                                 double p2[2], double q2[2], double r2[2] );

#endif # TRI_TRI_INTERSECT
