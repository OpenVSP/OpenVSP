//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifdef DO_GRAPHICS

/*##############################################################################

                              Set TrackBall Size

   This size should really be based on the distance from the center of
   rotation to the point on the object underneath the mouse.  That
   point would then track the mouse as closely as possible.  

##############################################################################*/

#define TRACKBALLSIZE  (1.0)

#define RENORMCOUNT 97

/*##############################################################################

                              Function vzero

Function Description:

zero out vector.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vzero(float *v);

/*##############################################################################

                              Function vset

Function Description:

Set vector components.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vset(float *v, float x, float y, float z);

/*##############################################################################

                              Function vsub

Function Description:

Subtract two vectors.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vsub(const float *src1, const float *src2, float *dst);

/*##############################################################################

                              Function vcopy

Function Description:

Copy a vector.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vcopy(const float *v1, float *v2);

/*##############################################################################

                              Function vcross

Function Description:

Cross product of two vectors.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vcross(const float *v1, const float *v2, float *cross);

/*##############################################################################

                              Function vlength

Function Description:

Norm of a vector.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

float vlength(const float *v);

/*##############################################################################

                              Function vscale

Function Description:

Scale a vector.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vscale(float *v, float div);

/*##############################################################################

                              Function vnormal

Function Description:

Normalize a vector.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vnormal(float *v);

/*##############################################################################

                              Function vdot

Function Description:

Dot product of two vectors.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

float vdot(const float *v1, const float *v2);

/*##############################################################################

                              Function vadd

Function Description:

Add two vectors.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vadd(const float *src1, const float *src2, float *dst);

/*##############################################################################

                              Function trackball

Function Description:

   Ok, simulate a track-ball.  Project the points onto the virtual
   trackball, then figure out the axis of rotation, which is the cross
   product of P1 P2 and O P1 (O is the center of the ball, 0,0,0)
   Note:  This is a deformed trackball-- is a trackball in the center,
   but is deformed into a hyperbolic sheet of rotation away from the
   center.  This particular function was chosen after trying out
   several variations.
  
   It is assumed that the arguments to this routine are in the range
   (-1.0 ... 1.0)

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void trackball(float q[4], float p1x, float p1y, float p2x, float p2y);

/*##############################################################################

                              Function axis_to_quat

Function Description:

Given an axis and angle, compute quaternion.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/


void axis_to_quat(float a[3], float phi, float q[4]);

/*##############################################################################

                              Function tb_project_to_sphere

Function Description:

   Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
   if we are away from the center of the sphere.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

static float tb_project_to_sphere(float r, float x, float y);

/*##############################################################################

                              Function add_quats

Function Description:

   Given two rotations, e1 and e2, expressed as quaternion rotations,
   figure out the equivalent single rotation and stuff it into dest.
  
   This routine also normalizes the result every RENORMCOUNT times it is
   called, to keep error from creeping in.
  
   NOTE: This routine is written so that q1 or q2 may be the same
   as dest (or each other).
  
Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void add_quats(float q1[4], float q2[4], float dest[4]);

/*##############################################################################

                              Function normalize_quat

Function Description:

   Quaternions always obey:  a^2 + b^2 + c^2 + d^2 = 1.0
   If they don't add up to 1.0, dividing by their magnitued will
   renormalize them.
  
   Note: See the following for more information on quaternions:
  
   - Shoemake, K., Animating rotation with quaternion curves, Computer
     Graphics 19, No 3 (Proc. SIGGRAPH'85), 245-254, 1985.
   - Pletinckx, D., Quaternion calculus as a basic tool in computer
     graphics, The Visual Computer 5, 2-13, 1989.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

static void normalize_quat(float q[4]);

/*##############################################################################

                              Function build_rotmatrix

Function Description:

   Build a rotation matrix, given a quaternion rotation.
  
Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/
 
void build_rotmatrix(float m[4][4], float q[4], 
     float trans_x, float trans_y, float trans_z, float scale);

#endif
