//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifdef DO_GRAPHICS

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
//#include "utils.H"

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __APPLE__

#include <OpenGL/glu.h>

#else

#include <GL/glu.h>

#endif

#include "trackball.H"


/*##############################################################################

                              Function vzero

Function Description:

zero out vector.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vzero(float *v)
{

    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 0.0;

}

/*##############################################################################

                              Function vset

Function Description:

Set vector components.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vset(float *v, float x, float y, float z)
{

    v[0] = x;
    v[1] = y;
    v[2] = z;

}

/*##############################################################################

                              Function vsub

Function Description:

Subtract two vectors.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vsub(const float *src1, const float *src2, float *dst)
{

    dst[0] = src1[0] - src2[0];
    dst[1] = src1[1] - src2[1];
    dst[2] = src1[2] - src2[2];

}

/*##############################################################################

                              Function vcopy

Function Description:

Copy a vector.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vcopy(const float *v1, float *v2)
{

    int i;

    for (i = 0 ; i < 3 ; i++) {

       v2[i] = v1[i];

   }

}

/*##############################################################################

                              Function vcross

Function Description:

Cross product of two vectors.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vcross(const float *v1, const float *v2, float *cross)
{

    float temp[3];

    temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);

    vcopy(temp, cross);

}

/*##############################################################################

                              Function vlength

Function Description:

Norm of a vector.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

float vlength(const float *v)
{

    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

}

/*##############################################################################

                              Function vscale

Function Description:

Scale a vector.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vscale(float *v, float div)
{

    v[0] *= div;
    v[1] *= div;
    v[2] *= div;

}

/*##############################################################################

                              Function vnormal

Function Description:

Normalize a vector.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vnormal(float *v)
{

    vscale(v,1.0/vlength(v));

}

/*##############################################################################

                              Function vdot

Function Description:

Dot product of two vectors.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

float vdot(const float *v1, const float *v2)
{

    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];

}

/*##############################################################################

                              Function vadd

Function Description:

Add two vectors.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void vadd(const float *src1, const float *src2, float *dst)
{

    dst[0] = src1[0] + src2[0];
    dst[1] = src1[1] + src2[1];
    dst[2] = src1[2] + src2[2];
}

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

void trackball(float q[4], float p1x, float p1y, float p2x, float p2y)
{

    float a[3]; /* Axis of rotation */
    float phi;  /* how much to rotate about axis */
    float p1[3], p2[3], d[3];
    float t;

    if (p1x == p2x && p1y == p2y) {
        /* Zero rotation */
        vzero(q);
        q[3] = 1.0;
        return;
    }

    /* Figure out z-coordinates for projection of P1 and P2 to deformed sphere */

    vset(p1,p1x,p1y,tb_project_to_sphere(TRACKBALLSIZE,p1x,p1y));

    vset(p2,p2x,p2y,tb_project_to_sphere(TRACKBALLSIZE,p2x,p2y));

    /* Now, we want the cross product of P1 and P2 */

    vcross(p2,p1,a);

    /* Figure out how much to rotate around that axis. */

    vsub(p1,p2,d);
    t = vlength(d) / (2.0*TRACKBALLSIZE);

    /* Avoid problems with out-of-control values... */

    if (t >  1.0) t =  1.0;
    if (t < -1.0) t = -1.0;
    phi = 2.0 * asin(t);

    axis_to_quat(a,phi,q);

}

/*##############################################################################

                              Function axis_to_quat

Function Description:

Given an axis and angle, compute quaternion.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/


void axis_to_quat(float a[3], float phi, float q[4])
{

    vnormal(a);

    vcopy(a,q);

    vscale(q,sin(phi/2.0));

    q[3] = cos(phi/2.0);

}

/*##############################################################################

                              Function tb_project_to_sphere

Function Description:

   Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
   if we are away from the center of the sphere.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

static float tb_project_to_sphere(float r, float x, float y)
{

    float d, t, z;

    d = sqrt(x*x + y*y);

    /* Inside sphere */

    if (d < r * 0.70710678118654752440) {

       z = sqrt(r*r - d*d);

    /* On hyperbola */

    } else {

        t = r/1.41421356237309504880;

        z = t*t/d;

    }

    return z;

}

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

void add_quats(float q1[4], float q2[4], float dest[4])
{

    static int count=0;
    float t1[4], t2[4], t3[4];
    float tf[4];

    vcopy(q1,t1);
    vscale(t1,q2[3]);

    vcopy(q2,t2);
    vscale(t2,q1[3]);

    vcross(q2,q1,t3);
    vadd(t1,t2,tf);
    vadd(t3,tf,tf);
    tf[3] = q1[3] * q2[3] - vdot(q1,q2);

    dest[0] = tf[0];
    dest[1] = tf[1];
    dest[2] = tf[2];
    dest[3] = tf[3];

    if (++count > RENORMCOUNT) {

        count = 0;

        normalize_quat(dest);

    }

}

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

static void normalize_quat(float q[4])
{

    int   i;
    float mag;

    mag = (q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);

    for (i = 0; i < 4; i++) {

       q[i] /= mag;

    }

}

/*##############################################################################

                              Function build_rotmatrix

Function Description:

   Build a rotation matrix, given a quaternion rotation.

Coded By: David J. Kinney
    Date: 11 - 3 - 1997

##############################################################################*/

void build_rotmatrix(float m[4][4], float q[4],
     float trans_x, float trans_y, float trans_z, float scale)
{

    m[0][0] = ( 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]) )*scale;
    m[0][1] = (       2.0 * (q[0] * q[1] - q[2] * q[3]) )*scale;
    m[0][2] = (       2.0 * (q[2] * q[0] + q[1] * q[3]) )*scale;
    m[0][3] = 0.0;

    m[1][0] = (       2.0 * (q[0] * q[1] + q[2] * q[3]) )*scale;
    m[1][1] = ( 1.0 - 2.0 * (q[2] * q[2] + q[0] * q[0]) )*scale;
    m[1][2] = (       2.0 * (q[1] * q[2] - q[0] * q[3]) )*scale;
    m[1][3] = 0.0;

    m[2][0] = (       2.0 * (q[2] * q[0] - q[1] * q[3]) )*scale;
    m[2][1] = (       2.0 * (q[1] * q[2] + q[0] * q[3]) )*scale;
    m[2][2] = ( 1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]) )*scale;
    m[2][3] = 0.0;

    m[3][0] = trans_x*scale;
    m[3][1] = trans_y*scale;
    m[3][2] = trans_z*scale;
    m[3][3] = 1.0;

}

#endif
