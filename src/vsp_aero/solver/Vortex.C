//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "Vortex.H"

/*##############################################################################
#                                                                              #
#                              VORTEX constructor                              #
#                                                                              #
##############################################################################*/

VORTEX::VORTEX(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                VORTEX init                                   #
#                                                                              #
##############################################################################*/

void VORTEX::init(void)
{

}

/*##############################################################################
#                                                                              #
#                                 VORTEX Copy                                  #
#                                                                              #
##############################################################################*/

VORTEX::VORTEX(const VORTEX &Vortex)
{

    init();

    // Just * use the operator = code

    *this = Vortex;

}

/*##############################################################################
#                                                                              #
#                              VORTEX operator=                                #
#                                                                              #
##############################################################################*/

VORTEX& VORTEX::operator=(const VORTEX &Vortex)
{

    return *this;

}

/*##############################################################################
#                                                                              #
#                               VORTEX destructor                              #
#                                                                              #
##############################################################################*/

VORTEX::~VORTEX(void)
{


}

/*##############################################################################
#                                                                              #
#                               VORTEX Velocity                                #
#                                                                              #
##############################################################################*/

void VORTEX::Velocity(float xyz_1[3], float xyz_2[3], float xyz_p[3], float Mach, float Gamma, float q[3])
{

    float xa, ya, za, xb, yb, zb, xc, yc, zc, xo, yo, zo;
    float theta, costheta, sintheta, s, t, d1, d2, ds, ctot, arg1, arg2;
    float x1bar, y1bar, z1bar, x2bar, y2bar, z2bar, xobar, yobar, zobar;
    float beta_2, x1, x2, y1, y2, xs, f1, f2, g1, g2;
    float ub, vb, wb, kappa;

    if ( Mach < 1. ) Mach = MIN(Mach,0.95);

    if ( Mach > 1. ) Mach = MAX(Mach,1.05);
    
    beta_2 = 1. - SQR(Mach);

    if ( beta_2 > 0. ) {

       kappa = 2.;

    }

    else {

       kappa = 1.;

    }

    /* get beginning and end points of the bound vortex */

    xa = xyz_1[0];
    ya = xyz_1[1];
    za = xyz_1[2];

    xb = xyz_2[0];
    yb = xyz_2[1];
    zb = xyz_2[2];

    /* get center of filament */

    xc = 0.5*( xa + xb );
    yc = 0.5*( ya + yb );
    zc = 0.5*( za + zb );

    /* get receiving point */

    xo = xyz_p[0];
    yo = xyz_p[1];
    zo = xyz_p[2];

    /* get orientation of vortex filament */

    theta = atan2( zb - za , yb - ya );

    costheta = cos(theta);
    sintheta = sin(theta);

    /* rotate axis system to simplify the calculations */

    x1bar =  ( xa - xc );
    y1bar =  ( ya - yc )*costheta + ( za - zc )*sintheta;
    z1bar = -( ya - yc )*sintheta + ( za - zc )*costheta;

    x2bar =  ( xb - xc );
    y2bar =  ( yb - yc )*costheta + ( zb - zc )*sintheta;
    z2bar = -( yb - yc )*sintheta + ( zb - zc )*costheta;

    xobar =  ( xo - xc );
    yobar =  ( yo - yc )*costheta + ( zo - zc )*sintheta;
    zobar = -( yo - yc )*sintheta + ( zo - zc )*costheta;

    /* calculate all the geometry stuff */

    s = ABS(y1bar);
      
    t = x1bar/y1bar;;

    x1 = xobar + t*s;
    y1 = yobar +   s;

    x2 = xobar - t*s;
    y2 = yobar -   s;

    xs = xobar - t*yobar;

    /* evaluate integral at limit 1 */

    arg1 = SQR(x1) + beta_2*( SQR(y1) + SQR(zobar) );

    f1 = 0.;
    g1 = 0.;

    if ( arg1 > 0. ) {

       d1 = sqrt( SQR(x1) + beta_2*( SQR(y1) + SQR(zobar) ) );

       f1 = ( t*x1 + beta_2*y1 )/d1;

       if ( Mach <= 1. ) {

          g1 = x1/d1 + 1.;

       }

       else {

          g1 = x1/d1;

       }

    }
    
    /* evaluate integral at limit 2 */

    arg2 = SQR(x2) + beta_2*( SQR(y2) + SQR(zobar) );

    f2 = 0.;
    g2 = 0.;

    if ( arg2 > 0. ) {

       d2 = sqrt( SQR(x2) + beta_2*( SQR(y2) + SQR(zobar) ) );

       f2 = ( t*x2 + beta_2*y2 )/d2;

       if ( Mach <= 1. ) {

          g2 = x2/d2 + 1.;

       }

       else {

          g2 = x2/d2;

       }

    }

    /* Circulation strength */

    ctot = Gamma/(4.*PI*kappa);

    /* get (ub,vb,wb) in rotated coordinate system */

    ds = SQR(xs) + ( SQR(t) + SQR(beta_2) )*SQR(zobar);

    d1 = SQR(y1) + SQR(zobar);

    d2 = SQR(y2) + SQR(zobar);
    
    ub =  ctot *         ( zobar*( f1 - f2 )/ds                       );
           
    vb =  ctot * zobar * (    -t*( f1 - f2 )/ds +    g1/d1 -    g2/d2 );
          
    wb = -ctot *         (    xs*( f1 - f2 )/ds + y1*g1/d1 - y2*g2/d2 );

    /* extract q from above */

    q[0] = ub;

    q[1] = vb*costheta - wb*sintheta;

    q[2] = vb*sintheta + wb*costheta;

}
