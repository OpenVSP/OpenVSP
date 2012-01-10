//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

/* NACA_6_series.f -- translated by f2c (version 20030306).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct {
    integer nml, nmu;
    real xxl[1000], xxu[1000], yyl[1000], yyu[1000];
} sixpnts_;

#define sixpnts_1 sixpnts_

/* Table of constant values */

static integer c__200 = 200;
static integer c__201 = 201;
static logical c_true = TRUE_;
static integer c__1 = 1;
static real c_b62 = 1.f;
static integer c_n1 = -1;
static integer c__0 = 0;
static integer c__251 = 251;

/* +----------------------------------------------------------------------- */

/*      PROGRAM SIXSERIES */
/* Subroutine */ int sixseries_(integer *tser, real *ttoc, real *tcli, real *
	ta)
{
    /* System generated locals */
    integer i__1;
    real r__1, r__2, r__3;

    /* Builtin functions */
    double atan(doublereal);
    /* Subroutine */ int s_copy(char *, char *, ftnlen, ftnlen);
    double log(doublereal);
    integer s_cmp(char *, char *, ftnlen, ftnlen);
    double cosh(doublereal), cos(doublereal), sinh(doublereal), sin(
	    doublereal), sqrt(doublereal);

    /* Local variables */
    static real a[10], e, f, g, h__;
    static integer i__, j, k, l;
    static real q, u, v, x, y, z__, b1, b2, s1, s2, x1, x2, z1, z2, y2, y1, 
	    an, bn, cn, as, ax[201], pi, xc, dx, yc;
    static integer it;
    static real sf, xl[200], yl[200], ym, xo, yp, tr, xt[201], xu[200], yu[
	    200], yt[201], chd, ali, cli[10], crd, pia, phi[201], xal[200], 
	    yal[200], toc;
    static integer kon;
    static real eps[201];
    static logical new__;
    static real psi[201], xlc, xau[200], yau[200], xuc, yuc, ylc, rat, ylp, 
	    rnp, xll, ypp, thp, xtp, ytp[201], yup, xym, xsv, ycp2[10];
    extern /* Subroutine */ int fd12k_(integer *, real *, real *, real *, 
	    real *, real *);
    static real frac;
    static char name__[80];
    static real axlc[201], aylc[201], ycmb[10], clis;
    static integer icky;
    static real axuc[201], ayuc[201], amxl, omxl, ymax, ylpr[200], ytpp[201], 
	    yupr[200];
    static integer if6xa;
    static real amxl1, amxl2, omxl1, omxl2, acrat;
    static integer neval;
    extern /* Subroutine */ int phep63_(real *, real *), phep64_(real *, real 
	    *), phep65_(real *, real *), phep66_(real *, real *), phep67_(
	    real *, real *);
    static real tanth[10];
    static integer ickyp;
    static real costh;
    extern /* Subroutine */ int phps63_(real *, real *), phps64_(real *, real 
	    *), phps65_(real *, real *), phps66_(real *, real *), phps67_(
	    real *, real *);
    static real sinth, tanth0[10];
    extern /* Subroutine */ int lddata_(integer *, char *, integer *, integer 
	    *, real *, real *, real *, real *, ftnlen), phep63a_(real *, real 
	    *), phep64a_(real *, real *), phep65a_(real *, real *);
    static char seriec[3];
    static real cmbnmr;
    static char method[1];
    extern /* Subroutine */ int phps63a_(real *, real *), phps64a_(real *, 
	    real *), phps65a_(real *, real *), lcsfit_(integer *, real *, 
	    real *, logical *, char *, integer *, real *, real *, real *, 
	    ftnlen);
    static char seriet[3];


/*    (formerly PROGRAM LADSON) */

/*  PURPOSE: To produce the ordinates for airfoils of any thickness, */
/*           thickness distribution, or camber in the NACA 6- and 6A-series. */

/*  NAMELIST "INPUTS": */

/*   VAR     DIM   TYPE    DESCRIPTION */
/*  NAME      -    C*80    Title desired on printed and plotted output */
/*  SERIET    -     I      NACA airfoil family (series 63 thru 67 and */
/*                         63A thru 65A) */
/*  SERIEC    -     I      Camber line (series 63 thru 66 and 63A thru 65A) */
/*  TOC       -     R      Thickness-chord ratio of airfoil */
/*  RLE       -     R      Leading-edge radius may be entered if desired */
/*                         (not used in program) */
/*  CMBNMR    -     R      Number of mean lines to be summed (>=1.) */
/*  CHD       -     R      Model chord used for listing ordinates in dimens- */
/*                         ional units */
/*  CLI       10    R      Design lift coefficient; set to 0.0 for a symmetrical */
/*                         airfoil. (Additional coefficients for up to nine */
/*                         mean lines may be added.) */
/*   A        10    R      Mean line chordwise loading (use 0.8 for 6A-series) */

/*  ENVIRONMENT:  DEC VAX/VMS FORTRAN (this version) */

/*  REFERENCE:    NASA Technical Memorandum TM X-3069 (September, 1974) */
/*                Charles Ladson and Cuyler Brooks */
/*                Langley Research Center */
/*  HISTORY: */
/*    November 1989   Liam Hardy/NASA Ames   Interpolation routine FTLUP replaced */
/*                                           by LCSFIT to eliminate non-standard */
/*                                           %LOC usage; finite difference */
/*                                           derivatives (FUNCTION DIF) replaced */
/*                                           with FDI2K. */
/* ------------------------------------------------------------------------------- */
    e = 1e-11f;
    pi = (float)atan(1.f) * 4.f;
    kon = 0;
    dx = .01f;
/*     Input parameters normalized by the chord (CHD) */
/*     TOC - T/C, Thickness, RLE - Leading edge radius, XM - X(YMAX)/CHORD */
/*     DX - Interval/Chord, CHD - Chord in desired units */
/*     CMBNMR - Number of mean lines to be summed (>=1.) */
/*  100 READ ( LUNRD, NML=INPUTS, END=999 ) */
/* L100: */
    s_copy(name__, "WHATEVER", (ftnlen)80, (ftnlen)8);
    a[0] = *ta;
    cli[0] = *tcli;
    toc = *ttoc;
    crd = 1.f;
    cmbnmr = 1.f;
    s_copy(seriet, "63", (ftnlen)3, (ftnlen)2);
    s_copy(seriec, "63", (ftnlen)3, (ftnlen)2);
    if (*tser == 63) {
	s_copy(seriet, "63", (ftnlen)3, (ftnlen)2);
    }
    if (*tser == 64) {
	s_copy(seriet, "64", (ftnlen)3, (ftnlen)2);
    }
    if (*tser == 65) {
	s_copy(seriet, "65", (ftnlen)3, (ftnlen)2);
    }
    if (*tser == 66) {
	s_copy(seriet, "66", (ftnlen)3, (ftnlen)2);
    }
    if (*tser == 67) {
	s_copy(seriet, "67", (ftnlen)3, (ftnlen)2);
    }
    if (*tser == -63) {
	s_copy(seriet, "63A", (ftnlen)3, (ftnlen)3);
    }
    if (*tser == -64) {
	s_copy(seriet, "64A", (ftnlen)3, (ftnlen)3);
    }
    if (*tser == -65) {
	s_copy(seriet, "65A", (ftnlen)3, (ftnlen)3);
    }
    s_copy(seriec, seriet, (ftnlen)3, (ftnlen)3);
    icky = (int)cmbnmr;
    if (icky < 1) {
	icky = 1;
    }
    ickyp = icky + 1;
    for (j = ickyp; j <= 10; ++j) {
	cli[j - 1] = 0.f;
	a[j - 1] = 0.f;
/* L110: */
    }
    if6xa = 0;
    ++kon;
    frac = 1.f;
/*     Slope of camberline at origin, TANTH0: */
    clis = cli[0];
    as = a[0];
    l = 0;
L200:
    ++l;
    a[0] = a[l - 1];
    cli[0] = cli[l - 1];
    x = 0.f;
    y = 0.f;
    xc = 0.f;
    yc = 0.f;
    xu[0] = 0.f;
    yu[0] = 0.f;
    xl[0] = 0.f;
    yl[0] = 0.f;
    xuc = 0.f;
    yuc = 0.f;
    xlc = 0.f;
    ylc = 0.f;
    xau[0] = 0.f;
    yau[0] = 0.f;
    xal[0] = 0.f;
    yal[0] = 0.f;
    k = 2;
    u = .005f;
    v = (float)(-(a[0] - u) / (r__1 = a[0] - u, dabs(r__1)));
    omxl = (float)((1.f - u) * log(1.f - u));
    amxl = (float)((a[0] - u) * log((r__1 = a[0] - u, dabs(r__1))));
    omxl1 = (float)(-log(1.f - u) - 1.f);
    amxl1 = (float)(-log((r__1 = a[0] - u, dabs(r__1))) + v);
    omxl2 = 1.f / (1.f - u);
    amxl2 = (float)(-v / (r__1 = a[0] - u, dabs(r__1)));
    if (a[0] >= e && (r__1 = 1.f - a[0], dabs(r__1)) >= e) {
	g = (float)(-(a[0] * a[0] * (log(a[0]) * .5f - .25f) + .25f) / (1.f - a[0]));
	q = 1.f;
/* Computing 2nd power */
	r__1 = 1.f - a[0];
/* Computing 2nd power */
	r__2 = 1.f - a[0];
	h__ = (float)((r__1 * r__1 * .5f * log(1.f - a[0]) - r__2 * r__2 * .25f) / (
		1.f - a[0]) + g);
/* Computing 2nd power */
	r__1 = a[0] - u;
/* Computing 2nd power */
	r__2 = 1.f - u;
	z__ = (a[0] - u) * .5f * amxl - (1.f - u) * .5f * omxl - r__1 * r__1 *
		 .25f + r__2 * r__2 * .25f;
	z1 = ((a[0] - u) * amxl1 - amxl - (1.f - u) * omxl1 + omxl + (a[0] - 
		u) - (1.f - u)) * .5f;
	z2 = (a[0] - u) * .5f * amxl2 - amxl1 - (1.f - u) * .5f * omxl2 + 
		omxl1;
    }
    if (a[0] < e) {
	h__ = -.5f;
	q = 1.f;
	z1 = (float)(u * log(u) - u * .5f - (1.f - u) * .5f * omxl1 + omxl * .5f - 
		.5f);
    } else if ((r__1 = a[0] - 1.f, dabs(r__1)) < e) {
	h__ = 0.f;
	q = 0.f;
	z1 = -omxl1;
    }
    tanth0[l - 1] = (float)(cli[0] * (z1 / (1.f - q * a[0]) - 1.f - log(u) - h__) / 
	    pi / (a[0] + 1.f) / 2.f);
    if (l < icky && icky > 1) {
	goto L200;
    }
    if (icky != 1) {
	i__1 = icky;
	for (j = 2; j <= i__1; ++j) {
	    tanth0[0] += tanth0[j - 1];
/* L210: */
	}
    }
/*     Slope of profile at origin, Upper and Lower: */
    yp = 1e11f;
    ypp = 1e11f;
    if (tanth0[0] != 0.f) {
	yup = -1.f / tanth0[0];
	ylp = -1.f / tanth0[0];
    } else {
	yup = 0.f;
	ylp = 0.f;
    }
/*     First station aft of origin on uncambered profile: */
    i__ = 1;
    x = 2.5e-4f;
/*     Start loop for X increment: */
L300:
/*       Skip thickness computation after first pass: */
    if (i__ > 1) {
	goto L550;
    }
/*       Select series: */
    if (s_cmp(seriet, "63", (ftnlen)3, (ftnlen)2) == 0) {
	phep63_(phi, eps);
    }
    if (s_cmp(seriet, "64", (ftnlen)3, (ftnlen)2) == 0) {
	phep64_(phi, eps);
    }
    if (s_cmp(seriet, "65", (ftnlen)3, (ftnlen)2) == 0) {
	phep65_(phi, eps);
    }
    if (s_cmp(seriet, "66", (ftnlen)3, (ftnlen)2) == 0) {
	phep66_(phi, eps);
    }
    if (s_cmp(seriet, "67", (ftnlen)3, (ftnlen)2) == 0) {
	phep67_(phi, eps);
    }
    if (s_cmp(seriet, "63", (ftnlen)3, (ftnlen)2) == 0) {
	phps63_(phi, psi);
    }
    if (s_cmp(seriet, "64", (ftnlen)3, (ftnlen)2) == 0) {
	phps64_(phi, psi);
    }
    if (s_cmp(seriet, "65", (ftnlen)3, (ftnlen)2) == 0) {
	phps65_(phi, psi);
    }
    if (s_cmp(seriet, "66", (ftnlen)3, (ftnlen)2) == 0) {
	phps66_(phi, psi);
    }
    if (s_cmp(seriet, "67", (ftnlen)3, (ftnlen)2) == 0) {
	phps67_(phi, psi);
    }
    if (s_cmp(seriet, "63A", (ftnlen)3, (ftnlen)3) == 0) {
	phep63a_(phi, eps);
    }
    if (s_cmp(seriet, "64A", (ftnlen)3, (ftnlen)3) == 0) {
	phep64a_(phi, eps);
    }
    if (s_cmp(seriet, "65A", (ftnlen)3, (ftnlen)3) == 0) {
	phep65a_(phi, eps);
    }
    if (s_cmp(seriet, "63A", (ftnlen)3, (ftnlen)3) == 0) {
	phps63a_(phi, psi);
    }
    if (s_cmp(seriet, "64A", (ftnlen)3, (ftnlen)3) == 0) {
	phps64a_(phi, psi);
    }
    if (s_cmp(seriet, "65A", (ftnlen)3, (ftnlen)3) == 0) {
	phps65a_(phi, psi);
    }
    rat = 1.f;
    it = 0;
    acrat = 1.f;
/*       Loop start for thickness iteration: */
L400:
    ++it;
    acrat *= rat;
    ymax = 0.f;
    for (j = 1; j <= 200; ++j) {
	xt[j - 1] = (float)(cosh(psi[j - 1] * acrat) * -2.f * cos(phi[j - 1] - eps[j 
		- 1] * acrat));
	yt[j - 1] = (float)(sinh(psi[j - 1] * acrat) * 2.f * sin(phi[j - 1] - eps[j - 
		1] * acrat));
	if (yt[j - 1] > ymax) {
	    xym = xt[j - 1];
	}
	if (yt[j - 1] > ymax) {
	    ymax = yt[j - 1];
	}
/* L410: */
    }
/*        Estimate first and second derivatives by finite differencing: */
    fd12k_(&c__200, xt, yt, ytp, ytpp, ytpp);
/*        Estimate location of maximum thickness: */
    xtp = 1.f;
    for (j = 3; j <= 200; ++j) {
	if (ytp[j - 1] < 0.f && ytp[j - 2] >= 0.f) {
	    xtp = xt[j - 2] + ytp[j - 2] * (xt[j - 1] - xt[j - 2]) / (ytp[j - 
		    2] - ytp[j - 1]);
	}
/* L420: */
    }
    lcsfit_(&c__201, xt, yt, &c_true, "B", &c__1, &xtp, &ym, &ym, (ftnlen)1);
    xo = xt[0];
    xl[0] = xt[199];
    tr = ym * 2.f / (xl[0] - xo);
    rat = toc / tr;
    sf = rat;
    if (toc > e && (r__1 = rat - 1.f, dabs(r__1)) > 1e-4f && it <= 10) {
	goto L400;
    }
    if (i__ == 1) {
	for (j = 1; j <= 201; ++j) {
	    xt[j - 1] = (xt[j - 1] - xo) / (xl[0] - xo);
/*           Scale linearly to exact thickness: */
	    yt[j - 1] = sf * yt[j - 1] / (xl[0] - xo);
	    ytp[j - 1] = sf * ytp[j - 1];
	    ytpp[j - 1] = sf * ytpp[j - 1] * (xl[0] - xo);
/* L500: */
	}
    }
    xtp = (xtp - xo) / (xl[0] - xo);
    ymax = ymax * sf / (xl[0] - xo);
    ym = ym * sf / (xl[0] - xo);
    xym = (xym - xo) / (xl[0] - xo);
    xl[0] = 0.f;
    if (toc > e) {
/*        Fit tilted ellipse at eleventh profile point: */
	cn = ytp[10] * 2.f - yt[10] / xt[10] + .1f;
	an = xt[10] * (ytp[10] * xt[10] - yt[10]) / (xt[10] * (ytp[10] * 2.f 
		- cn) - yt[10]);
/* Computing 2nd power */
	r__1 = yt[10] - cn * xt[10];
/* Computing 2nd power */
	r__2 = xt[10] - an;
/* Computing 2nd power */
	r__3 = an;
	bn = (float)(sqrt(r__1 * r__1 / (1.f - r__2 * r__2 / (r__3 * r__3))));
	for (j = 1; j <= 10; ++j) {
/* Computing 2nd power */
	    r__1 = xt[j - 1] - an;
/* Computing 2nd power */
	    r__2 = an;
	    yt[j - 1] = (float)(bn * sqrt(1.f - r__1 * r__1 / (r__2 * r__2)) + cn * 
		    xt[j - 1]);
	    if (xt[j - 1] <= e) {
		goto L510;
	    }
/* Computing 2nd power */
	    r__1 = bn;
/* Computing 2nd power */
	    r__2 = an;
	    ytp[j - 1] = r__1 * r__1 * (an - xt[j - 1]) / (r__2 * r__2) / (yt[
		    j - 1] - cn * xt[j - 1]) + cn;
/* Computing 4th power */
	    r__1 = bn, r__1 *= r__1;
/* Computing 2nd power */
	    r__2 = an;
/* Computing 3rd power */
	    r__3 = yt[j - 1] - cn * xt[j - 1];
	    ytpp[j - 1] = -(r__1 * r__1) / (r__2 * r__2) / (r__3 * (r__3 * 
		    r__3));
L510:
	    ;
	}
/* Computing 2nd power */
	r__1 = bn;
	rnp = r__1 * r__1 / an;
    }
    x = 0.f;
    ali = (float)(dabs(cli[0]));
    x = 2.5e-4f;
    xl[0] = 0.f;
L550:
    yupr[i__ - 1] = yup;
    ylpr[i__ - 1] = ylp;
/*     Interpolate for thickness and derivatives at desired values of X: */
    new__ = TRUE_;
    *(unsigned char *)method = 'B';
    neval = (int)1.f;
    lcsfit_(&c__200, xt, yt, &new__, method, &neval, &x, &y, &y, (ftnlen)1);
    lcsfit_(&c__200, xt, ytp, &new__, method, &neval, &x, &yp, &yp, (ftnlen)1)
	    ;
    lcsfit_(&c__200, xt, ytpp, &new__, method, &neval, &x, &ypp, &ypp, (
	    ftnlen)1);
/*     Compute camberline: */
    a[0] = as;
    cli[0] = clis;
    l = 0;
L600:
    ++l;
    a[0] = a[l - 1];
    cli[0] = cli[l - 1];
    xc = x * chd;
    yc = y * chd;
    xll = (float)(x * log(x));
    q = 1.f;
    if ((r__1 = 1.f - a[0], dabs(r__1)) < e && (r__2 = 1.f - x, dabs(r__2)) < 
	    e) {
	g = 0.f;
	h__ = 0.f;
	q = 0.f;
	z__ = 0.f;
	z1 = -1e11f;
	z2 = -1e11f;
    } else if (a[0] < e && 1.f - x < e) {
	g = -.25f;
	h__ = -.5f;
	q = 1.f;
	z__ = -.25f;
	z1 = 0.f;
	z2 = -1e11f;
    } else if ((r__1 = a[0] - x, dabs(r__1)) < e) {
/* Computing 2nd power */
	r__1 = 1.f - x;
/* Computing 2nd power */
	r__2 = 1.f - x;
	z__ = (float)(r__1 * r__1 * -.5f * log(1.f - x) + r__2 * r__2 * .25f);
	z1 = (float)((1.f - x) * -.5f * (-log(1.f - x) - 1.f) + (1.f - x) * .5f * log(
		1.f - x) - (1.f - x) * .5f);
	z2 = (float)(-log(1.f - x) - .5f);
/* Computing 2nd power */
	r__1 = a[0];
	g = (float)(-(r__1 * r__1 * (log(a[0]) * .5f - .25f) + .25f) / (1.f - a[0]));
/* Computing 2nd power */
	r__1 = 1.f - a[0];
/* Computing 2nd power */
	r__2 = 1.f - a[0];
	h__ = (float)((r__1 * r__1 * .5f * log(1.f - a[0]) - r__2 * r__2 * .25f) / (
		1.f - a[0]) + g);
    } else if ((r__1 = 1.f - x, dabs(r__1)) < e) {
/* Computing 2nd power */
	r__1 = a[0];
	g = (float)(-(r__1 * r__1 * (log(a[0]) * .5f - .25f) + .25f) / (1.f - a[0]));
/* Computing 2nd power */
	r__1 = 1.f - a[0];
/* Computing 2nd power */
	r__2 = 1.f - a[0];
	h__ = (float)((r__1 * r__1 * .5f * log(1.f - a[0]) - r__2 * r__2 * .25f) / (
		1.f - a[0]) + g);
/* Computing 2nd power */
	r__2 = a[0] - 1.f;
/* Computing 2nd power */
	r__3 = a[0] - 1.f;
	z__ = (float)(r__2 * r__2 * .5f * log((r__1 = a[0] - 1.f, dabs(r__1))) - r__3 
		* r__3 * .25f);
	z1 = (float)(-(a[0] - 1.f) * log((r__1 = a[0] - 1.f, dabs(r__1))));
	z2 = -1e11f;
    } else if ((r__1 = a[0] - 1.f, dabs(r__1)) < e) {
	g = 0.f;
	h__ = 0.f;
	q = 0.f;
	z__ = (float)(-(1.f - x) * log(1.f - x));
	z1 = (float)(log(1.f - x) + 1.f);
	z2 = -1.f / (1.f - x);
    } else {
	v = (float)(-(a[0] - x) / (r__1 = a[0] - x, dabs(r__1)));
	omxl = (float)((1.f - x) * log(1.f - x));
	amxl = (float)((a[0] - x) * log((r__1 = a[0] - x, dabs(r__1))));
	omxl1 = (float)(-log(1.f - x) - 1.f);
	amxl1 = (float)(-log((r__1 = a[0] - x, dabs(r__1))) - 1.f);
	omxl2 = 1.f / (1.f - x);
	amxl2 = 1.f / (a[0] - x);
/* Computing 2nd power */
	r__1 = a[0] - x;
/* Computing 2nd power */
	r__2 = 1.f - x;
	z__ = (a[0] - x) * .5f * amxl - (1.f - x) * .5f * omxl - r__1 * r__1 *
		 .25f + r__2 * r__2 * .25f;
	z1 = ((a[0] - x) * amxl1 - amxl - (1.f - x) * omxl1 + omxl + (a[0] - 
		x) - (1.f - x)) * .5f;
	z2 = (a[0] - x) * .5f * amxl2 - amxl1 - (1.f - x) * .5f * omxl2 + 
		omxl1;
	if (a[0] <= e) {
	    g = -.25f;
	    h__ = -.5f;
	} else {
	    g = (float)(-(a[0] * a[0] * (log(a[0]) * .5f - .25f) + .25f) / (1.f - a[0]
		    ));
/* Computing 2nd power */
	    r__1 = 1.f - a[0];
/* Computing 2nd power */
	    r__2 = 1.f - a[0];
	    h__ = (float)((r__1 * r__1 * .5f * log(1.f - a[0]) - r__2 * r__2 * .25f) /
		     (1.f - a[0]) + g);
	}
    }
    ycmb[l - 1] = cli[0] * (z__ / (1.f - q * a[0]) - xll + g - h__ * x) / pi /
	     (a[0] + 1.f) / 2.f;
    xsv = x;
    if (x < .005f) {
	x = .005f;
    }
    tanth[l - 1] = (float)(cli[0] * (z1 / (1.f - q * a[0]) - 1.f - log(x) - h__) / pi 
	    / (a[0] + 1.f) / 2.f);
    x = xsv;
    if (if6xa == 1) {
	tanth[l - 1] = -2.f;
    }
    if (x <= .005f) {
	ycp2[l - 1] = 0.f;
    } else if ((r__1 = 1.f - x, dabs(r__1)) <= e) {
	ycp2[l - 1] = 1.f / e;
    } else {
	pia = pi * (a[0] + 1.f) * 2;
	ycp2[l - 1] = cli[0] * (z2 / (1.f - q * a[0]) - 1.f / x) / pia;
    }
/*        Modified camberline option: */
    if (s_cmp(seriec, "63A", (ftnlen)3, (ftnlen)3) == 0 || s_cmp(seriec, 
	    "64A", (ftnlen)3, (ftnlen)3) == 0 || s_cmp(seriec, "65A", (ftnlen)
	    3, (ftnlen)3) == 0) {
	ycmb[l - 1] *= .97948f;
	tanth[l - 1] *= .97948f;
	if (tanth[l - 1] <= cli[0] * -.24521f) {
	    ycmb[l - 1] = cli[0] * .24521f * (1.f - x);
	}
	if (tanth[l - 1] <= cli[0] * -.24521f) {
	    ycp2[l - 1] = 0.f;
	}
	if (tanth[l - 1] <= cli[0] * -.24521f) {
	    tanth[l - 1] = cli[0] * -.24521f;
	}
	if (tanth[l - 1] <= cli[0] * -.24521f) {
	    if6xa = 1;
	}
    }
    if (icky > 1 && l < icky) {
	goto L600;
    }
    if (icky == 1) {
	goto L620;
    }
    i__1 = icky;
    for (j = 2; j <= i__1; ++j) {
	ycmb[0] += ycmb[j - 1];
	tanth[0] += tanth[j - 1];
	ycp2[0] += ycp2[j - 1];
/* L610: */
    }
L620:
/* Computing 2nd power */
    r__1 = tanth[0];
    f = (float)(sqrt(r__1 * r__1 + 1.f));
/* Computing 2nd power */
    r__1 = f;
    thp = ycp2[0] / (r__1 * r__1);
    sinth = tanth[0] / f;
    costh = 1.f / f;
/*     Camberline and derivatives computed: */
    ++i__;
/*     Combine thickness distributuion and camberline: */
    xu[i__ - 1] = x - y * sinth;
    yu[i__ - 1] = ycmb[0] + y * costh;
    xl[i__ - 1] = x + y * sinth;
    yl[i__ - 1] = ycmb[0] - y * costh;
    if (x >= .815f) {
	if (s_cmp(seriet, "63A", (ftnlen)3, (ftnlen)3) == 0 || s_cmp(seriet, 
		"64A", (ftnlen)3, (ftnlen)3) == 0 || s_cmp(seriet, "65A", (
		ftnlen)3, (ftnlen)3) == 0) {
	    if (x <= .825f) {
		x2 = 1.f;
		x1 = xu[i__ - 1];
		y2 = 0.f;
		y1 = yu[i__ - 1];
		s1 = (y2 - y1) / (x2 - x1);
		s2 = (y2 - yl[i__ - 1]) / (x2 - xl[i__ - 1]);
		b1 = y2 - s1 * x2;
		b2 = y2 - s2 * x2;
	    }
	    yu[i__ - 1] = s1 * xu[i__ - 1] + b1;
	    yl[i__ - 1] = s2 * xl[i__ - 1] + b2;
	}
    }
/*       Multiply by chord: */
    xuc = xu[i__ - 1] * chd;
    yuc = yu[i__ - 1] * chd;
    xlc = xl[i__ - 1] * chd;
    ylc = yl[i__ - 1] * chd;
    if (ali > e || icky != 1) {
/*        Find local slope of cambered profile: */
	yup = (tanth[0] * f + yp - tanth[0] * y * thp) / (f - yp * tanth[0] - 
		y * thp);
	ylp = (tanth[0] * f - yp + tanth[0] * y * thp) / (f + yp * tanth[0] + 
		y * thp);
    }
/*     Find X increment: */
    if (x <= .0975f) {
	frac = .25f;
    }
    if (x <= .01225f) {
	frac = .025f;
    }
/*     Store profile in appropriate arrays: */
    axuc[i__ - 1] = xuc;
    ayuc[i__ - 1] = yuc;
    if (ali >= e || icky != 1) {
	ax[i__ - 1] = x;
	axlc[i__ - 1] = xlc;
	aylc[i__ - 1] = ylc;
    }
/*     Increment X and return to start of X loop: */
    x += frac * dx;
    frac = 1.f;
    xau[i__ - 1] = xuc;
    yau[i__ - 1] = yuc;
    xal[i__ - 1] = xlc;
    yal[i__ - 1] = ylc;
    if (x <= 1.f) {
	goto L300;
    }
    lddata_(&c__200, name__, &i__, &i__, xu, xl, yu, yl, (ftnlen)80);
/*     Return to read for next case: */
/*      GO TO 100 */
/*  999 STOP ' ' */
/* L999: */
    return 0;
} /* sixseries_ */

/* +---------------------------------------------------------------------- */

doublereal bessel_(integer *j, real *h__, real *del)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real weight;


/*     One-liner: First derivative using central 3-point formula */
/*     ---------- */

/*     Description and usage: */
/*     ---------------------- */

/*        Computes a first derivative approximation using the central */
/*     3-point formula.  The data must be in the form of arrays containing */
/*     finite difference interval lengths and 2-point forward difference */
/*     derivatives.  BESSEL is intended to be used by PLSFIT for determin- */
/*     ing end conditions on an interval for (non-monotonic) interpolation */
/*     by piecewise cubics.  See the PLSFIT header for more details. */

/*     Arguments: */
/*     ---------- */

/*     Name    Type/Dimension  I/O/S  Description */
/*     J       I               I      Indicates at which end of the */
/*                                    interval the derivative is to be */
/*                                    estimated. J = 0 means left-hand */
/*                                    side, J = 1 means right. */

/*     H       R (-1:1)        I      Array of interval lengths. The 0th */
/*                                    element is the length of the interval */
/*                                    on which the cubic is to be deter- */
/*                                    mined. */

/*     DEL     R (-1:1)        I      Array of derivative estimates. The */
/*                                    0th element is the forward difference */
/*                                    derivative over the interval on which */
/*                                    the cubic is to be determined. */

/*     BESSEL  R                 O    The function value is the adjusted */
/*                                    derivative. */

/*     Notes: */
/*     ------ */

/*     (1)  IMPLICIT NONE is non-standard. */

/*     Author:  Robert Kennelly, Sterling Federal Systems/NASA-Ames */
/*     ------- */

/*     Development history: */
/*     -------------------- */

/*     18 Feb. 1987    RAK    Initial design and coding. */

/* ----------------------------------------------------------------------- */
/*     Declarations. */
/*     ------------- */
/*     Constants. */
/*     Arguments. */
/*     Local variables. */
/*     Execution. */
/*     ---------- */
/*     Estimate first derivative on left (J = 0) or right side (J = 1) of */
/*     an interval. */
    /* Parameter adjustments */
    ++del;
    ++h__;

    /* Function Body */
    weight = h__[*j] / (h__[*j] + h__[*j - 1]);
    ret_val = weight * del[*j - 1] + (1.f - weight) * del[*j];
/*     Termination. */
/*     ------------ */
    return ret_val;
} /* bessel_ */

/* +---------------------------------------------------------------------- */

doublereal brodlie_(integer *j, real *h__, real *del)
{
    /* System generated locals */
    real ret_val, r__1;

    /* Builtin functions */
    double r_sign(real *, real *);

    /* Local variables */
    static real alpha;


/*     One-liner: First derivative, adjusted for monotonicity */
/*     ---------- */

/*     Description and usage: */
/*     ---------------------- */

/*        BRODLIE is intended to be used by PLSFIT for determining end */
/*     conditions on an interval for monotonic interpolation by piecewise */
/*     cubics. The data must be in the form of arrays containing finite */
/*     difference interval lengths and 2-point forward difference deriva- */
/*     tives. See the PLSFIT header for more details. */

/*        The method is due to Brodlie, Butland, Carlson, and Fritsch, */
/*     as referenced in the PLSFIT header. */

/*     Arguments: */
/*     ---------- */

/*     Name    Type/Dimension  I/O/S  Description */
/*     J       I               I      Indicates at which end of the */
/*                                    interval the derivative is to be */
/*                                    estimated. J = 0 means left-hand */
/*                                    side, J = 1 means right. */

/*     H       R (-1:1)        I      Array of interval lengths. The 0th */
/*                                    element is the length of the interval */
/*                                    on which the cubic is to be deter- */
/*                                    mined. */

/*     DEL     R (-1:1)        I      Array of derivative estimates. The */
/*                                    0th element is the forward difference */
/*                                    derivative over the interval on which */
/*                                    the cubic is to be determined. */

/*     BRODLIE R                 O    The function value is the adjusted */
/*                                    derivative. */

/*     Notes: */
/*     ------ */

/*     (1)  IMPLICIT NONE and 8-character symbolic names are non-standard. */

/*     Author:  Robert Kennelly, Sterling Federal Systems/NASA-Ames */
/*     ------- */

/*     Development history: */
/*     -------------------- */

/*     18 Feb. 1987    RAK    Initial design and coding. */

/* ----------------------------------------------------------------------- */
/*     Declarations. */
/*     ------------- */
/*     Constants. */
/*     Arguments. */
/*     Local variables. */
/*     Execution. */
/*     ---------- */
/*     Compare the algebraic signs of the two DEL's.  Have to test that */
/*     at least one is positive to avoid a zero denominator (this fancy */
/*     test permits one term to be zero, but the answer below is zero */
/*     anyway in these cases).  The trick is to work around the SIGN */
/*     function, which returns positive even if its 2nd argument is zero. */
    /* Parameter adjustments */
    ++del;
    ++h__;

    /* Function Body */
    r__1 = -del[*j - 1];
    if (r_sign(&c_b62, &r__1) != r_sign(&c_b62, &del[*j])) {
/*        Form "weighted harmonic mean" of the two finite-difference */
/*        derivative approximations.  Note that we try to avoid overflow */
/*        by not multiplying them together directly. */
	alpha = (h__[*j] / (h__[*j - 1] + h__[*j]) + 1.f) * 
		.33333333333333331f;
	ret_val = del[*j - 1] * (del[*j] / (alpha * del[*j] + (1.f - alpha) * 
		del[*j - 1]));
    } else {
/*        The signs differ, so make this point a local extremum. */
	ret_val = 0.f;
    }
/*     Termination. */
/*     ------------ */
    return ret_val;
} /* brodlie_ */

/* +---------------------------------------------------------------------- */

doublereal butland_(integer *j, real *h__, real *del)
{
    /* System generated locals */
    real ret_val;

    /* Builtin functions */
    double r_sign(real *, real *);

    /* Local variables */
    static real dmax__;
    static integer step;
    static real weight;


/*     One-liner: First derivative, non-central 3-point formula, adjusted */
/*     ---------- */

/*     Description and usage: */
/*     ---------------------- */

/*        Computes a first derivative approximation for PLSFIT over an */
/*     interval at a data boundary, using a modified forward or backward */
/*     3-point formula.  The data must be in the form of arrays containing */
/*     finite difference interval lengths and 2-point forward difference */
/*     derivatives, and the differencing direction is controlled by a flag. */
/*     See PLSFIT for more details, or THREEPT for the pure 3-pt. formula. */

/*        The "shape preserving adjustments" are from PCHIP, a monotone */
/*     piecewise cubic interpolation package by F. N. Fritsch. */

/*     Arguments: */
/*     ---------- */

/*     Name    Type/Dimension  I/O/S  Description */
/*     J       I               I      Indicates at which end of the */
/*                                    interval the derivative is to be */
/*                                    estimated. J = 0 means left-hand */
/*                                    side, J = 1 means right. */

/*     H       R (-1:1)        I      Array of interval lengths. The 0th */
/*                                    element is the length of the interval */
/*                                    on which the cubic is to be deter- */
/*                                    mined. */

/*     DEL     R (-1:1)        I      Array of derivative estimates. The */
/*                                    0th element is the forward difference */
/*                                    derivative over the interval on which */
/*                                    the cubic is to be determined. */

/*     BUTLAND R                 O    The function value is the adjusted */
/*                                    derivative. */

/*     Environment:  VAX/VMS; FORTRAN 77 */
/*     ------------ */

/*     IMPLICIT NONE and 8-character symbolic names are non-standard. */

/*     Author:  Robert Kennelly, Sterling Federal Systems/NASA-Ames */
/*     ------- */

/*     History: */
/*     -------- */

/*     18 Feb. 1987    RAK    Initial design and coding, as THREEPT. */
/*     20 June 1991    DAS    Monotonic form renamed BUTLAND; THREEPT */
/*                            is now the pure 3-point formula. */

/* ----------------------------------------------------------------------- */
/*     Declarations. */
/*     ------------- */
/*     Arguments. */
/*     Local constants. */
/*     Local variables. */
/*     Execution. */
/*     ---------- */
/*     Estimate first derivative on a left-hand boundary using a 3-point */
/*     forward difference (STEP = +1), or with a backward difference for */
/*     the right-hand boundary (STEP = -1). */
    /* Parameter adjustments */
    ++del;
    ++h__;

    /* Function Body */
    step = 1 - *j - *j;
/*     In {H, DEL} form, the derivative looks like a weighted average. */
/* J here is consistent with related modules. */
    weight = -h__[0] / (h__[0] + h__[step]);
    ret_val = weight * del[step] + (1.f - weight) * del[0];
/*     Shape-preserving adjustments.  Note that we try to avoid overflow */
/*     by not multiplying quantities directly. */
    if (r_sign(&c_b62, &ret_val) != r_sign(&c_b62, del) || del[0] == 0.f) {
/*        Defer to the estimate closest to the boundary. */
	ret_val = 0.f;
    } else if (r_sign(&c_b62, del) != r_sign(&c_b62, &del[step])) {
/*        If the monotonicity switches, may need to bound the estimate. */
	dmax__ = del[0] * 3.f;
	if (dabs(ret_val) > dabs(dmax__)) {
	    ret_val = dmax__;
	}
    }
/*     Termination. */
/*     ------------ */
    return ret_val;
} /* butland_ */

/* +---------------------------------------------------------------------- */

/* Subroutine */ int fd12k_(integer *n, real *x, real *f, real *fp, real *fpp,
	 real *fk)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__;
    static real fpi, fppi;
    extern /* Subroutine */ int fd1sid_(integer *, integer *, real *, real *, 
	    real *, real *), fdcntr_(integer *, real *, real *, real *, real *
	    ), fdcurv_(real *, real *, real *);


/*  PURPOSE: FD12K returns estimates of 1st and 2nd derivatives and of */
/*           curvature, by finite differencing, for each of the points */
/*           (X(I), F(I)), I = 1 : N.  The abscissas are assumed to be */
/*           nonuniform, and they must be monotonic. */

/*           This routine combines calls to FDCNTR, FD1SID, FDCURV for */
/*           the common case of needing results for N >= 2 points. */

/*           If (say) curvature is wanted at a single point only, call */
/*           either FDCNTR or FD1SID and FDCURV directly. */

/*  INPUTS:  X(*) & F(*) are N-vectors defining some curve in 2-space. */
/*           For N > 2, the 3-pt formulas are used for all I (with the */
/*                      one-sided forms used at the end-points). */
/*           For N = 2, the 2-pt formulas are used. */

/*  OUTPUTS: FP, FPP, FK are N-vectors representing 1st and 2nd deriv- */
/*           atives and curvature respectively.  These are assigned in */
/*           reverse order (FK, FPP, FP) so that a call such as */

/*                     CALL FD12K (N, X, Y, YP, YP, YP) */

/*           can be used if just 1st derivatives are desired, to avoid */
/*           declaring storage for FPP and FK. (Similarly for the case */
/*           when 1st and 2nd derivatives are desired but curvature is */
/*           not. The unnecessary arithmetic in these cases is consid- */
/*           ered preferable to another argument and extra logic.) */

/*  METHOD:  Central differencing is used at all interior points, with */
/*           one-sided 3-point formulas used at each end-point. */

/*           The curvature formula is safeguarded against overflow  in */
/*           the presence of large 1st derivatives.  The basic formula */
/*           used here is: */

/*               FK (I) = FPP (I) / (1 + FP(I) ** 2) ** 3/2 */

/*           Note that if X is not necessarily monotonic, curvature is */
/*           defined as */

/*               CURVATURE = (X" ** 2  +  Y" ** 2) ** 1/2 */

/*           where " means 2nd derivative with respect to  arc-length. */
/*           See modules CURV2D and CURV3D for these parametric cases. */

/*  NOTES:   1. Finite differencing errors can be large if the delta-X */
/*              values are too small,  especially if the precision  in */
/*              the function values is less than full. */
/*           2. Nevertheless, finite differences have been observed to */
/*              behave better than the analytic derivatives of splines */
/*              in airfoil geometry applications. */

/*  EXTERNALS: */
/*           FDCNTR modularizes the central 3-point formulas for first */
/*                  and second derivatives by finite differences. */
/*           FDCURV modularizes the curvature formula (safe-guarded). */
/*           FD1SID modularizes the 1-sided forward and backward 3-pt. */
/*                  formulas for first and second derivatives. */

/*  HISTORY: */
/*           09/15/83   DAS   Initial implementation (interior pts only). */
/*           12/27/85   DAS   End points are handled by FD1SID now. */
/*           09/18/87   DAS   The N=2 case is handled now. */
/*           08/21/89   DAS   Formulation revised to use separate dF/dX */
/*                            terms instead of a common denominator. */
/*           08/17/91   DAS   Introduced FDCNTR and FDCURV when it was */
/*                            found that FD12K did not lend itself to */
/*                            application to one point at a time. */

/*  AUTHOR:  David Saunders, Sterling Software/NASA Ames, Palo Alto, CA. */

/* ----------------------------------------------------------------------- */
/*     Arguments: */
/*     Local constants: */
/*     Local variables: */
/*     Procedures: */
/*     Execution: */
/*     Assign values in the order  curvature, f", f'  so that the */
/*     user can pass the same array for, say, curvature and f" if */
/*     curvature is not wanted: */
    /* Parameter adjustments */
    --fk;
    --fpp;
    --fp;
    --f;
    --x;

    /* Function Body */
    if (*n == 2) {
	fk[1] = 0.f;
	fpp[1] = 0.f;
	fp[1] = (f[2] - f[1]) / (x[2] - x[1]);
	fk[2] = 0.f;
	fpp[2] = 0.f;
	fp[2] = fp[1];
    } else {
/*        Forward 3-pt. differencing for the first point: */
	fd1sid_(&c__1, &c__1, &x[1], &f[1], &fpi, &fppi);
	fdcurv_(&fpi, &fppi, &fk[1]);
	fpp[1] = fppi;
	fp[1] = fpi;
/*        Central 3-pt. differencing for the bulk of the points: */
	i__1 = *n - 1;
	for (i__ = 2; i__ <= i__1; ++i__) {
	    fdcntr_(&i__, &x[1], &f[1], &fpi, &fppi);
	    fdcurv_(&fpi, &fppi, &fk[i__]);
	    fpp[i__] = fppi;
	    fp[i__] = fpi;
/* L20: */
	}
/*        Backward differencing for the last point: */
	fd1sid_(n, &c_n1, &x[1], &f[1], &fpi, &fppi);
	fdcurv_(&fpi, &fppi, &fk[*n]);
	fpp[*n] = fppi;
	fp[*n] = fpi;
    }
    return 0;
} /* fd12k_ */

/* +---------------------------------------------------------------------- */

/* Subroutine */ int fd1sid_(integer *i__, integer *inc, real *x, real *f, 
	real *fp, real *fpp)
{
    static real w;
    static integer i1, i2;
    static real dx1, dx2, div, del1, del2;


/*  PURPOSE: FD1SID returns one-sided 3-point finite-difference estimates */
/*           of the 1st and 2nd derivatives at the point  ( X(I), F(I) ). */
/*           If INC = 1, points I, I+1, I+2 are used,  while if INC = -1, */
/*           points  I-2, I-1, I are used. The abscissas need not be uni- */
/*           formly spaced. */

/*  ARGS:    Obvious from PURPOSE. */

/*  METHOD:  FPP is computed first,  in case only FP is desired,  so that */
/*           the same item may be passed for both arguments. The formula- */
/*           tion is similar to that of central differencing - see FD12K. */

/*  HISTORY: 12/27/85  DAS  Initial implementation  (prompted by the need */
/*                          to approximate an airfoil leading edge with a */
/*                          cubic having specified slope at one end). */
/*           08/21/89  DAS  Formulation revised as for centrals (FD12K). */

/*  AUTHOR:  David Saunders, Sterling Software/NASA Ames, Palo Alto, CA. */

/* ----------------------------------------------------------------------- */
/*     Arguments: */
/*     Local constants: */
/*     Local variables: */
/*     Execution: */
/*     The minus signs take care of themselves for the backward case. */
    /* Parameter adjustments */
    --f;
    --x;

    /* Function Body */
    i1 = *i__ + *inc;
    i2 = i1 + *inc;
    dx1 = x[i1] - x[*i__];
    del1 = (f[i1] - f[*i__]) / dx1;
    dx2 = x[i2] - x[i1];
    del2 = (f[i2] - f[i1]) / dx2;
    div = 1.f / (dx1 + dx2);
    w = -dx1 * div;
    *fpp = (del2 - del1) * (div + div);
    *fp = w * del2 + (1.f - w) * del1;
    return 0;
} /* fd1sid_ */

/* +---------------------------------------------------------------------- */

/* Subroutine */ int fdcntr_(integer *i__, real *x, real *f, real *fp, real *
	fpp)
{
    static real w, dx1, dx2, div, del1, del2;


/*  PURPOSE: FDCNTR returns central 3-point finite-difference estimates */
/*           of the 1st and 2nd derivatives at the point  (X(I), F(I)). */
/*           Use FD1SID for the end-point cases. */

/*  ARGS:    Obvious from PURPOSE. */

/*  METHOD:  FPP is computed first, in case only FP is desired, so that */
/*           the same item may be passed for both arguments. */

/*  HISTORY: 08/17/91  DAS  FDCNTR adapted from FD12K's in-line code, */
/*                          as for FD1SID, for the case of a single I */
/*                          at a time (which FD12K can't do). */

/*  AUTHOR:  David Saunders, Sterling Software/NASA Ames, Palo Alto, CA. */

/* ----------------------------------------------------------------------- */
/*     Arguments: */
/*     Local constants: */
/*     Local variables: */
/*     Execution: */
    /* Parameter adjustments */
    --f;
    --x;

    /* Function Body */
    dx1 = x[*i__] - x[*i__ - 1];
    del1 = (f[*i__] - f[*i__ - 1]) / dx1;
    dx2 = x[*i__ + 1] - x[*i__];
    del2 = (f[*i__ + 1] - f[*i__]) / dx2;
    div = 1.f / (dx1 + dx2);
    w = dx2 * div;
    *fpp = (del2 - del1) * (div + div);
    *fp = w * del1 + (1.f - w) * del2;
    return 0;
} /* fdcntr_ */

/* +---------------------------------------------------------------------- */

/* Subroutine */ int fdcurv_(real *dydx, real *d2ydx2, real *kappa)
{
    /* System generated locals */
    real r__1, r__2;

    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    static real term;


/*  PURPOSE: FDCURV (Finite-Difference CURVature estimate) returns a */
/*           safe-guarded value for curvature at one point on the */
/*           curve Y = Y (X) given the 1st and 2nd derivatives at */
/*           that point, using the formula */

/*               KAPPA = D2YDX2 / (1 + DYDX ** 2) ** 3/2 */

/*           The sign of KAPPA is clearly that of the 2nd derivative. */
/*           The derivatives could well be obtained from a spline, but */
/*           experience shows finite differencing can be preferable. */

/*           See modules CURV2D and CURV3D for the parametric cases. */

/*  ARGUMENTS:  Obvious from the description.  KAPPA is REAL. */

/*  HISTORY: 08/17/91  Derived FDCURV from FD12K, along with FDCNTR */
/*                     when it was found that FD12K did not lend */
/*                     itself to application to one point at a time. */

/*  AUTHOR:  David Saunders, Sterling Software/NASA Ames, Palo Alto, CA. */

/* ----------------------------------------------------------------------- */
/*     Arguments: */
/*     Local constants: */
/*     Local variables: */
/*     Execution: */
/* Computing MIN */
    r__2 = (float)(dabs(*dydx));
/* Computing 2nd power */
    r__1 = (float)(dmin(r__2,1e10f));
    term = r__1 * r__1 + 1.f;
    *kappa = (float)(*d2ydx2 / (term * sqrt(term)));
    return 0;
} /* fdcurv_ */

/* +---------------------------------------------------------------------- */

/* Subroutine */ int interval_(integer *nx, real *x, real *xfind, real *arrow,
	 integer *left)
{
    /* System generated locals */
    integer i__1, i__2, i__3, i__4;

    /* Local variables */
    static real xbyarrow;
    static integer trial, right, length, nxless1;


/*     One-liner: Interpolation search for interval containing a point. */
/*     ---------- */

/*     Description and usage: */
/*     ---------------------- */

/*        Written primarily for interval-based interpolations such as */
/*     piecewise linear or cubic spline, INTERVAL performs a search to */
/*     locate the best interval for evaluating the interpolant at a */
/*     given point. The normal case returns the "left-hand" endpoint of */
/*     the interval bracketing the point, but for the out-of-range cases */
/*     below or above the range of the knots, the interval to be used is */
/*     the first or last. The array of knots must be monotonic, either */
/*     increasing or decreasing. Diagrammatically, LEFT is returned as */
/*     shown below for the normal case (no extrapolation): */

/*          X (1)  ...   X (LEFT)   X (LEFT+1)   ...      X (NX) */
/*                                ^ */
/*                              XFIND */

/*     And for extrapolation: */

/*                     X (LEFT = 1)  ...   X (NX) */
/*             ^ */
/*           XFIND */

/*     or, */
/*                X (1)  ...   X (LEFT = NX-1)    X (NX) */
/*                                                           ^ */
/*                                                         XFIND */

/*     If the point to be bracketed (XFIND) matches one of the knots, the */
/*     index of that knot is returned as LEFT, i.e., the condition for a */
/*     bracket of an interior point is: */

/*        X (LEFT) <= XFIND < X (LEFT+1)  if  ARROW = +1.0,  or */
/*        X (LEFT) >= XFIND > X (LEFT+1)  if  ARROW = -1.0. */

/*        This is a low-level routine with minimal error checking. The */
/*     calling program is assumed to have verified the following: */

/*     (1)  NX >= 2 */
/*     (2)  X strictly monotonic */
/*     (3)  ARROW = +1.0 or -1.0 */

/*     Subroutine PROTECT is available from the author for easily checking */
/*     conditions (2) and (3). LEFT is verified on input, but efficiency in */
/*     loops will benefit from passing the best estimate available, usually */
/*     just the result of the last call. */

/*        INTERVAL was originally written for use with CSEVAL and TABLE1. */
/*     The interpolation search was adapted from ideas in Sedgewick's book */
/*     referenced below. */

/*     Arguments: */
/*     ---------- */

/*     Name  Dimension  Type  I/O/S  Description */
/*     NX                I    I      Number of points in array X; must */
/*                                   be >= 2 (no check performed). */

/*     X        NX       R    I      Array of points defining the set */
/*                                   of intervals to be examined. Only */
/*                                   the first NX-1 points are required. */

/*     XFIND             R    I      The point for which a bracketing */
/*                                   interval is sought. */

/*     ARROW             R    I      Monotonicity indicator for input */
/*                                   array X: */
/*                                     -1.0  strictly decreasing */
/*                                      0.0  NOT ALLOWED! */
/*                                     +1.0  strictly increasing */
/*                                   Supplied by the calling routine for */
/*                                   reasons of speed (not checked). */

/*     LEFT              I    I/O    Input: guessed index of left-hand */
/*                                   endpoint of the interval containing */
/*                                   the specified point. */

/*                                   Output: index of the largest array */
/*                                   value <= specified point (if ARROW=+1.0). */
/*                                   Special case for data out of range: */
/*                                   return left endpoint of closest interval. */
/*                                   Thus, LEFT = 1 for XFIND < X (2), and */
/*                                   LEFT = NX-1 for XFIND >= X (NX-1). */
/*                                   (If ARROW=-1.0, reverse the inequalities.) */

/*     Environment:  Digital VAX-11/780, VMS FORTRAN */
/*     ------------  Apple Macintosh, Absoft MacFORTRAN/020 v2.3 */

/*     Notes: */
/*     ------ */

/*     (1)  IMPLICIT NONE and eight character symbols are not (yet) standard. */

/*     (2)  In speed-critical applications, it might be a good idea to build */
/*          this algorithm in-line since it is typically called many times */
/*          from within a loop. Another potential speed-up is removal of the */
/*          ARROW multiplies, which restricts the method to increasing data. */
/*          So far, the simplicity of separating out the messy search details */
/*          and the generality of bi-directional searching have outweighed */
/*          the modest speed penalty incurred. */

/*     Bibliography: */
/*     ------------- */

/*     (1) Sedgewick, R.  Algorithms.  Reading: Addison-Wesley, 1983. */
/*            (Chap. 14) */

/*     Author:  Robert Kennelly and David Saunders, Sterling Federal Systems */
/*     ------- */

/*     Development history: */
/*     -------------------- */

/*     20 Oct. 1987    RAK    Interpolation search adapted (with mods. */
/*                            for bidirectional search and some minor */
/*                            repair) from CSEVAL (RAK) and TABLE1 (DAS). */
/*     08 Aug. 1988    DAS    Clarified descriptions of bracketing, where */
/*                            the inequalities depend upon ARROW. */

/* ----------------------------------------------------------------------- */
/*     Declarations. */
/*     ------------- */
/*     Constants. */
/*     Arguments. */
/*     Local variables. */
/*     Execution. */
/*     ---------- */
    /* Parameter adjustments */
    --x;

    /* Function Body */
    xbyarrow = *xfind * *arrow;
/*     Simplify things by disposing of two important special cases so that */
/*     X (LEFT) and X (RIGHT) can really bracket XFIND. As a by-product, */
/*     this also takes care of the NX = 2, 3 cases. */
    nxless1 = *nx - 1;
    if (xbyarrow >= x[nxless1] * *arrow) {
	*left = nxless1;
	goto L990;
    } else if (xbyarrow < x[2] * *arrow) {
	*left = 1;
	goto L990;
    }
/*      --------------------------------- */
/*     |                                 | */
/*     |   X (2) <= XFIND < X (NX - 1)   | */
/*     |            - or -               | */
/*     |   X (2) > XFIND >= X (NX - 1)   | */
/*     |                                 | */
/*     |   NX > 3                        | */
/*     |                                 | */
/*      --------------------------------- */
/*     Adjust the pointers. We hope that the calling routine has provided */
/*     a reasonable guess (since it's probably working on an ordered array */
/*     of points to evaluate), but check anyway. */
/* Computing MIN */
    i__1 = max(2,*left), i__2 = *nx - 2;
    *left = min(i__1,i__2);
    if (xbyarrow >= x[*left] * *arrow) {
	if (xbyarrow < x[*left + 1] * *arrow) {
/*           XFIND is in the original guessed-at interval. */
	    goto L990;
	} else {
/*           We'll look farther to the right. Evidently LEFT was < NX - 2. */
	    right = nxless1;
	    ++(*left);
	}
    } else {
/*        Look to the left of the guess. Evidently LEFT was > 2. */
	right = *left;
	*left = 2;
    }
/*      ---------------------------------- */
/*     |                                  | */
/*     |   2 <= LEFT < RIGHT <= NX - 1    | */
/*     |                                  | */
/*      ---------------------------------- */
/*     The interval length must decrease each time through - terminate */
/*     when the correct interval is found or when the interval length */
/*     cannot be decreased. */
L10:
    length = right - *left;
    if (length > 1) {
/*           The trial value is a "linear" estimate of the left-hand endpoint */
/*           of the interval bracketing the target XFIND, with protection */
/*           against round-off (which can affect convergence). */
/* Computing MIN */
/* Computing MAX */
	i__3 = 0, i__4 = (integer) ((real) length * (*xfind - x[*left]) / (x[
		right] - x[*left]));
	i__1 = right - 1, i__2 = *left + max(i__3,i__4);
	trial = min(i__1,i__2);
/*            ------------------------------------------ */
/*           |                                          | */
/*           |   2 <= LEFT <= TRIAL < RIGHT <= NX - 1   | */
/*           |                                          | */
/*            ------------------------------------------ */
/*           Adjust pointers. Increase LEFT or decrease RIGHT until done. */
	if (xbyarrow >= x[trial + 1] * *arrow) {
	    *left = trial + 1;
	} else if (xbyarrow < x[trial] * *arrow) {
	    right = trial;
	} else {
/*              We're done: XFIND is in the interval [X (TRIAL), X (TRIAL+1)). */
	    *left = trial;
	    goto L990;
	}
	goto L10;
    }
/*     Termination. */
/*     ------------ */
L990:
    return 0;
} /* interval_ */

/* +---------------------------------------------------------------------- */

/* Subroutine */ int lcsfit_(integer *ndata, real *x, real *y, logical *new__,
	 char *method, integer *neval, real *xeval, real *yeval, real *ypeval,
	 ftnlen method_len)
{
    /* System generated locals */
    real r__1;

    /* Builtin functions */
    /* Subroutine */ int s_stop(char *, ftnlen);
    double r_sign(real *, real *);

    /* Local variables */
    extern /* Subroutine */ int interval_(integer *, real *, real *, real *, 
	    integer *);
    static real xbyarrow, b[2], c__, d__, h__[3];
    static integer j, k;
    static real dx, xe;
    static integer left;
    static real dely[3];
    static logical mono;
    static integer ieval, right;
    static real arrow;
    static logical cyclic;
    extern doublereal bessel_(integer *, real *, real *);
    static logical memory;
    extern doublereal brodlie_(integer *, real *, real *), butland_(integer *,
	     real *, real *), threept_(integer *, real *, real *);


/*     Two-liner:  Storage-efficient local cubic spline fit (2-space) */
/*     ----------  (monotonic and piecewise linear options too) */

/*     Description and usage: */
/*     ---------------------- */

/*        LCSFIT is the non-parametric analog of PLSFIT (parametric). */
/*     It is intended for spline applications which do not require the */
/*     spline coefficients as output.  It is efficient for repeated */
/*     calls with the same data, so repeated use with NEVAL = 1 may be */
/*     preferable to storing vectors of results. */

/*        LCSFIT offers monotonic spline and piecewise linear options */
/*     also.  And it returns an interpolated first derivative along */
/*     with the function value.  (The second derivative is omitted */
/*     because Y" is not guaranteed to be continuous by local methods.) */

/*        See PLSFIT for more details on local methods.  As with most */
/*     numerical methods, scaling of the data to the unit interval (and */
/*     unscaling of the result) is recommended to avoid unnecessary */
/*     effects attributable to the data units.  Utilities GETSCALE and */
/*     USESCALE from the present authors are appropriate.  The data */
/*     abscissas should be distinct and either ascending or descending. */
/*     PROTECT is available to check this.  Extrapolation is permitted */
/*     (mainly in case of round-off; it is normally inadvisable). */

/*        The CSFIT/CSEVAL or CSDVAL pair are probably preferable if */
/*     efficiency is not an issue, since CSFIT gives Y" continuity. */

/*     Arguments: */
/*     ---------- */

/*     Name    Type/Dimension  I/O/S  Description */
/*     NDATA   I               I      Length of X, Y input data arrays. */

/*     X,      R (NDATA)       I      Input data coordinates.  The Xs */
/*     Y                              must be distinct and monotonic, */
/*                                    either ascending or descending. */
/*                                    (No check here.) */

/*     NEW     L               I      If control flag NEW is .TRUE., the */
/*                                    search for a bracket starts from */
/*                                    scratch, otherwise locally-saved */
/*                                    search and fit information will be */
/*                                    assumed to be correct. If calling */
/*                                    LCSFIT from within a loop, set */
/*                                    NEW = .FALSE. after the first call. */

/*     METHOD   C*1            I      (Uppercase) Type of fit to be used: */
/*                                    'M' means Monotonic piecewise cubics; */
/*                                    'B' means non-monotonic "Bessel"-type */
/*                                        piecewise cubics (looser fit); */
/*                                    'L' means piecewise Linear fit; */
/*                                    'C' means Cyclic (periodic) end */
/*                                        conditions: loose fit assumed. */

/*     NEVAL   I               I      Number of interpolations requested. */
/*                                    NEVAL >= 1.  One call per result */
/*                                    (NEVAL = 1) may save storage, and is */
/*                                    not too inefficient as long as NEW */
/*                                    is set to .FALSE. after the first. */

/*     XEVAL   R (NEVAL)       I      Abscissa(s) to interpolate to.  These */
/*                                    are normally in the data range, but */
/*                                    extrapolation - probably due to */
/*                                    round-off - is not prevented. */

/*     YEVAL   R (NEVAL)       O      Interpolated function value(s). */

/*     YPEVAL  R (NEVAL)       O      Interpolated 1st derivative value(s). */
/*                                    Pass the same storage as for YEVAL */
/*                                    if no derivatives are required. */

/*     Significant local variables: */
/*     ---------------------------- */

/*     MEMORY         Indicates that coefficients are correct for the */
/*                    current point. */

/*     H, DEL         Delta X and forward difference derivative arrays. */

/*     B, C, D        Coefficients of cubic on the bracketing interval. */

/*     Procedures: */
/*     ----------- */

/*     INTERVAL  1-D "interpolation" search. */
/*     BESSEL    First derivative (central 3-point formula). */
/*     BRODLIE   First derivative (central), adjusted for monotonicity. */
/*     BUTLAND   First derivative (non-central), adjusted for monotonicity. */
/*     THREEPT   First derivative (non-central 3-point formula). */

/*     Environment:  Digital VAX-11/780, VMS FORTRAN 77 */
/*     ------------ */

/*     Error handling:  None */
/*     --------------- */

/*     Notes: */
/*     ------ */

/*     (1)  IMPLICIT NONE, 8-character symbolic names, and "!" as comment */
/*          character are not (yet) standard. */

/*     (2)  Since many of the calculations must be repeated at both ends */
/*          of an interval, the various finite difference quantities used */
/*          are stored as arrays. The following "map" of a typical interior */
/*          interval and its neighbors should help in understanding the */
/*          notation.  The local array indices are all numbered relative */
/*          to the left-hand end of the interval which brackets the point */
/*          to be evaluated. */

/*                                  LEFT       RIGHT */

/*          Point         -1          0         +1          +2 */

/*          Data           x -------- x -------- x --------- x */

/*          Interval      -1          0         +1 */


/*     Author: Robert Kennelly, Sterling Software/NASA Ames  (PLSFIT) */
/*     ------- */

/*     History: */
/*     -------- */

/*     27 Feb. 1987  R.A.Kennelly  Initial design and coding of PLSFIT. */
/*     23 Aug. 1989  D.A.Saunders  LCSFIT adapted as non-parametric form, */
/*                                 for embedding in other utilities where */
/*                                 minimizing work-space is desirable. */
/*     20 June 1991    "    "      THREEPT (monotonic) renamed BUTLAND; */
/*                                 THREEPT (pure 3-pt. formula) now used */
/*                                 for nonmonotonic end-point handling; */
/*                                 METHOD='C' case belatedly added, as */
/*                                 needed by PLSINTRP for closed curves. */
/*     23 July 1991    "    "      The tests for being in the same interval */
/*                                 as before were not allowing for the */
/*                                 descending-Xs case. */
/* ----------------------------------------------------------------------- */
/*     Declarations. */
/*     ------------- */
/*     Arguments. */
/*     Local constants. */
/*     Local variables. */
/*     Procedures. */
/*     Storage. */
/*     Execution. */
/*     ---------- */
    /* Parameter adjustments */
    --y;
    --x;
    --ypeval;
    --yeval;
    --xeval;

    /* Function Body */
    mono = *(unsigned char *)method == 'M';
    cyclic = *(unsigned char *)method == 'C';
    if (cyclic) {
	if (y[*ndata] != y[1]) {
	    s_stop("LCSFIT: End points must match.", (ftnlen)30);
	}
    }
/*     Initialize search or avoid it if possible. */
    if (*new__) {
	memory = FALSE_;
	r__1 = x[2] - x[1];
	arrow = (float)(r_sign(&c_b62, &r__1));
	left = 1;
    }
    ieval = 1;
    xe = xeval[1];
    xbyarrow = xe * arrow;
    if (! (*new__)) {
/*        We can save a lot of time when LCSFIT is being called from within */
/*        a loop by setting MEMORY if possible. The out-of-range checking */
/*        relies on the fact that RIGHT = LEFT + 1 after the last return. */
/*        Cater to the more likely case of XE in the previous, interior */
/*        interval. */
	memory = xbyarrow >= x[left] * arrow && xbyarrow < x[right] * arrow;
	if (! memory) {
	    memory = left == 1 && xbyarrow < x[right] * arrow || right == *
		    ndata && xbyarrow >= x[left] * arrow;
	}
    }
    if (memory) {
	goto L70;
    }
/*     Loop over evaluation points requiring a new search. */
/*     --------------------------------------------------- */
/* Skip the bulk of the computation. */
L10:
/*        Interpolation search for bracketing interval. */
/*        --------------------------------------------- */
    interval_(ndata, &x[1], &xe, &arrow, &left);
    right = left + 1;
/*         ------------------------------------------- */
/*        |                                           | */
/*        |   1 <= LEFT < RIGHT = LEFT + 1 <= NDATA   | */
/*        |                                           | */
/*         ------------------------------------------- */
/*        Compute derivatives by finite-differences. */
/*        ------------------------------------------ */
    if (*ndata > 2 && *(unsigned char *)method != 'L') {
/*           Interval and derivative approximations. */
/*           --------------------------------------- */
/*           The following duplicates more code than PLSFIT's approach, */
/*           but eliminates some indirection - no need to wrap-around here. */
/*           Handle the end conditions first to minimize testing LEFT, RIGHT. */
	if (left == 1) {
	    h__[1] = x[2] - x[1];
	    dely[1] = (y[2] - y[1]) / h__[1];
	    h__[2] = x[3] - x[2];
	    dely[2] = (y[3] - y[2]) / h__[2];
	    if (cyclic) {
/* Loose fit assumed */
		h__[0] = x[*ndata] - x[*ndata - 1];
		dely[0] = (y[*ndata] - y[*ndata - 1]) / h__[0];
		b[0] = (float)(bessel_(&c__0, h__, dely));
		b[1] = (float)(bessel_(&c__1, h__, dely));
	    } else {
		if (mono) {
		    b[0] = (float)(butland_(&c__0, h__, dely));
		    b[1] = (float)(brodlie_(&c__1, h__, dely));
		} else {
		    b[0] = (float)(threept_(&c__0, h__, dely));
		    b[1] = (float)(bessel_(&c__1, h__, dely));
		}
	    }
	} else if (right == *ndata) {
	    h__[0] = x[left] - x[left - 1];
	    dely[0] = (y[left] - y[left - 1]) / h__[0];
	    h__[1] = x[right] - x[left];
	    dely[1] = (y[right] - y[left]) / h__[1];
	    if (cyclic) {
		h__[2] = x[2] - x[1];
		dely[2] = (y[2] - y[1]) / h__[2];
		b[0] = (float)bessel_(&c__0, h__, dely);
		b[1] = (float)bessel_(&c__1, h__, dely);
	    } else {
		if (mono) {
		    b[0] = (float)brodlie_(&c__0, h__, dely);
		    b[1] = (float)butland_(&c__1, h__, dely);
		} else {
		    b[0] = (float)bessel_(&c__0, h__, dely);
		    b[1] = (float)threept_(&c__1, h__, dely);
		}
	    }
	} else {
	    k = left;
	    for (j = -1; j <= 1; ++j) {
		h__[j + 1] = x[k] - x[k - 1];
		dely[j + 1] = (y[k] - y[k - 1]) / h__[j + 1];
		++k;
/* L40: */
	    }
/*              Select interpolation scheme. */
/*              ---------------------------- */
/*              Compute (possibly adjusted) first derivatives at both */
/*              left- and right-hand endpoints of the interval. */
	    if (mono) {
/*                 Monotone - use Brodlie modification of Butland's */
/*                 formula to adjust the derivatives at the knots. */
		b[0] = (float)brodlie_(&c__0, h__, dely);
		b[1] = (float)brodlie_(&c__1, h__, dely);
	    } else {
/*                 Bessel - use central difference formula at the knots. */
/* IF (METHOD .EQ. 'B') THEN */
		b[0] = (float)bessel_(&c__0, h__, dely);
		b[1] = (float)bessel_(&c__1, h__, dely);
	    }
	}
/*           Compute the remaining cubic coefficients. */
	c__ = (dely[1] * 3.f - b[0] * 2.f - b[1]) / h__[1];
/* Computing 2nd power */
	r__1 = h__[1];
	d__ = (dely[1] * -2.f + b[0] + b[1]) / (r__1 * r__1);
    } else {
/*           Degenerate case (linear). */
/*           ------------------------- */
/* IF (NDATA .EQ. 2 .OR. METHOD .EQ. 'L') THEN */
	b[0] = (y[right] - y[left]) / (x[right] - x[left]);
	c__ = 0.f;
	d__ = 0.f;
    }
/*        Evaluate the cubic (derivative first in case only YEVAL is reqd.) */
/*        ----------------------------------------------------------------- */
L70:
/* Start of same-interval loop inside new-interval l */
    dx = xe - x[left];
    ypeval[ieval] = b[0] + dx * (c__ * 2.f + dx * 3.f * d__);
    yeval[ieval] = y[left] + dx * (b[0] + dx * (c__ + dx * d__));
/*           The next evaluation point may be in the same interval. */
/*           ------------------------------------------------------ */
    if (ieval < *neval) {
/* Skips this if NEVAL = 1. */
	++ieval;
	xe = xeval[ieval];
	xbyarrow = xe * arrow;
	if (xbyarrow >= x[left] * arrow && xbyarrow < x[right] * arrow) {
	    goto L70;
	}
	goto L10;
/* Else much more work requir */
    }
/*     Termination. */
/*     ------------ */
    return 0;
} /* lcsfit_ */

/*     Machine dependent interpolation routine FTLUP replaced with LCSFIT. */
/*     20 Nov. 1989  L. Hardy   RAA Branch NASA Ames */
/* Subroutine */ int phep63_(real *phi, real *eps)
{
    /* Initialized data */

    static real a[402] = { 0.f,0.f,.01569f,.00164f,.03139f,.00327f,.04708f,
	    .00487f,.06278f,.00641f,.07848f,.00789f,.09419f,.00928f,.1099f,
	    .01057f,.12562f,.01174f,.14135f,.01278f,.15708f,.01367f,.17277f,
	    .01439f,.18846f,.01497f,.20416f,.01542f,.21987f,.01576f,.23558f,
	    .01601f,.25129f,.01619f,.26701f,.01632f,.28273f,.01642f,.29844f,
	    .01651f,.31416f,.01661f,.32987f,.01673f,.34559f,.01688f,.3613f,
	    .01705f,.377f,.01725f,.39271f,.01747f,.40842f,.01771f,.42412f,
	    .01797f,.43983f,.01824f,.45553f,.01853f,.47124f,.01884f,.48695f,
	    .01916f,.50266f,.01949f,.51837f,.01984f,.53407f,.0202f,.54978f,
	    .02058f,.56549f,.02097f,.5812f,.02137f,.59691f,.02179f,.61261f,
	    .02223f,.62832f,.02268f,.64403f,.02315f,.65974f,.02363f,.67545f,
	    .02413f,.69116f,.02464f,.70687f,.02517f,.72257f,.02571f,.73828f,
	    .02626f,.75399f,.02683f,.76969f,.02741f,.7854f,.02801f,.80111f,
	    .02862f,.81682f,.02924f,.83253f,.02988f,.84824f,.03052f,.86395f,
	    .03118f,.87965f,.03185f,.89536f,.03253f,.91107f,.03323f,.92677f,
	    .03393f,.94248f,.03465f,.95819f,.03538f,.9739f,.03611f,.98961f,
	    .03686f,1.00532f,.03762f,1.02103f,.03839f,1.03673f,.03917f,
	    1.05244f,.03995f,1.06815f,.04075f,1.08385f,.04156f,1.09956f,
	    .04237f,1.11527f,.04319f,1.13098f,.04402f,1.14669f,.04486f,
	    1.1624f,.04571f,1.17811f,.04657f,1.19381f,.04743f,1.20952f,
	    .04831f,1.22523f,.04919f,1.24093f,.05008f,1.25664f,.05098f,
	    1.27235f,.05189f,1.28806f,.0528f,1.30376f,.05372f,1.31947f,
	    .05464f,1.33518f,.05556f,1.35089f,.05648f,1.36659f,.0574f,1.3823f,
	    .05831f,1.39801f,.05921f,1.41372f,.06011f,1.42942f,.06099f,
	    1.44513f,.06187f,1.46084f,.06273f,1.47654f,.06357f,1.49225f,
	    .0644f,1.50796f,.06522f,1.52367f,.06602f,1.53938f,.06681f,
	    1.55509f,.06757f,1.5708f,.06832f,1.5865f,.06905f,1.60221f,.06976f,
	    1.61791f,.07044f,1.63362f,.07111f,1.64933f,.07176f,1.66504f,
	    .07238f,1.68075f,.07298f,1.69646f,.07356f,1.71217f,.07411f,
	    1.72788f,.07464f,1.74358f,.07514f,1.75929f,.07562f,1.775f,.07607f,
	    1.7907f,.0765f,1.80641f,.0769f,1.82212f,.07727f,1.83783f,.07761f,
	    1.85354f,.07793f,1.86925f,.07822f,1.88496f,.07848f,1.90067f,
	    .07871f,1.91637f,.07891f,1.93208f,.07908f,1.94779f,.07922f,
	    1.9635f,.07933f,1.97921f,.07941f,1.99491f,.07945f,2.01062f,
	    .07946f,2.02633f,.07944f,2.04204f,.07938f,2.05775f,.07929f,
	    2.07346f,.07916f,2.08917f,.079f,2.10487f,.0788f,2.12058f,.07856f,
	    2.13629f,.07829f,2.152f,.07799f,2.1677f,.07764f,2.18341f,.07726f,
	    2.19911f,.07685f,2.21482f,.0764f,2.23054f,.07591f,2.24625f,
	    .07539f,2.26196f,.07483f,2.27767f,.07423f,2.29338f,.07359f,
	    2.30908f,.07293f,2.32479f,.07222f,2.34049f,.07148f,2.35619f,
	    .0707f,2.37191f,.06989f,2.38762f,.06904f,2.40334f,.06815f,
	    2.41905f,.06723f,2.43476f,.06628f,2.45046f,.06529f,2.46617f,
	    .06427f,2.48187f,.06322f,2.49757f,.06214f,2.51327f,.06103f,
	    2.52899f,.05989f,2.5447f,.05871f,2.56042f,.05751f,2.57613f,
	    .05628f,2.59184f,.05502f,2.60754f,.05374f,2.62325f,.05243f,
	    2.63895f,.05109f,2.65465f,.04973f,2.67035f,.04834f,2.68607f,
	    .04693f,2.70178f,.04549f,2.71749f,.04404f,2.7332f,.04256f,
	    2.74891f,.04106f,2.76462f,.03955f,2.78032f,.03802f,2.79603f,
	    .03647f,2.81173f,.03491f,2.82743f,.03333f,2.84314f,.03174f,
	    2.85885f,.03014f,2.87456f,.02853f,2.89027f,.0269f,2.90598f,
	    .02527f,2.92169f,.02363f,2.9374f,.02198f,2.9531f,.02032f,2.96881f,
	    .01865f,2.98451f,.01698f,3.00022f,.0153f,3.01593f,.01361f,
	    3.03164f,.01192f,3.04735f,.01023f,3.06305f,.00853f,3.07876f,
	    .00683f,3.09447f,.00512f,3.11018f,.00342f,3.12588f,.00171f,
	    3.14159f,0.f };

    static integer j;

/*     DIMENSION PHILD(201),EPSLD(201) */
/*     DATA (PHILD(I),EPSLD(I),I=1,25 )/ */
    /* Parameter adjustments */
    --eps;
    --phi;

    /* Function Body */
/*     DATA (PHILD(I),EPSLD(I),I=26,50)/ */
/*     DATA (PHILD(I),EPSLD(I),I=51,75)/ */
/*     DATA (PHILD(I),EPSLD(I),I=76,100)/ */
/*     DATA (PHILD(I),EPSLD(I),I=101,125)/ */
/*     DATA (PHILD(I),EPSLD(I),I=126,150)/ */
/*     DATA (PHILD(I),EPSLD(I),I=151,175)/ */
/*     DATA (PHILD(I),EPSLD(I),I=176,201)/ */
    for (j = 1; j <= 201; ++j) {
	phi[j] = a[(j << 1) - 2];
	eps[j] = a[(j << 1) - 1];
/* L201: */
    }
    return 0;
} /* phep63_ */

/* Subroutine */ int phep63a_(real *phi, real *eps)
{
    /* Initialized data */

    static real a[502] = { 0.f,0.f,.01459f,.00138f,.02917f,.00276f,.04376f,
	    .00413f,.05835f,.00548f,.07294f,.00683f,.08754f,.00815f,.10213f,
	    .00945f,.11673f,.01072f,.13133f,.01196f,.14594f,.01317f,.14925f,
	    .01344f,.15256f,.0137f,.15587f,.01396f,.15919f,.01421f,.1625f,
	    .01445f,.16581f,.01469f,.16912f,.0149f,.17244f,.0151f,.17575f,
	    .01529f,.17907f,.01546f,.1842f,.01568f,.18933f,.01585f,.19447f,
	    .01599f,.1996f,.0161f,.20474f,.01617f,.20988f,.01623f,.21501f,
	    .01628f,.22015f,.01631f,.22529f,.01635f,.23043f,.01638f,.24002f,
	    .01647f,.2496f,.01657f,.25918f,.0167f,.26877f,.01684f,.27835f,
	    .01698f,.28793f,.01713f,.29751f,.01728f,.30709f,.01741f,.31667f,
	    .01754f,.32626f,.01764f,.33981f,.01775f,.35335f,.01781f,.36691f,
	    .01785f,.38046f,.01787f,.39401f,.01787f,.40756f,.01788f,.42111f,
	    .01788f,.43466f,.01791f,.44821f,.01796f,.46176f,.01804f,.47241f,
	    .01814f,.48306f,.01826f,.49371f,.01841f,.50436f,.01857f,.51501f,
	    .01875f,.52566f,.01895f,.53631f,.01917f,.54696f,.01939f,.55761f,
	    .01963f,.56826f,.01987f,.57741f,.02009f,.58655f,.02031f,.5957f,
	    .02053f,.60485f,.02075f,.61399f,.02098f,.62314f,.02121f,.63229f,
	    .02144f,.64143f,.02168f,.65058f,.02192f,.65973f,.02216f,.67542f,
	    .02257f,.69111f,.02299f,.7068f,.02342f,.72249f,.02385f,.73818f,
	    .02429f,.75386f,.02475f,.76955f,.02521f,.78524f,.02569f,.80093f,
	    .02618f,.81662f,.02669f,.83032f,.02715f,.84402f,.02762f,.85772f,
	    .0281f,.87143f,.02859f,.88513f,.02909f,.89883f,.0296f,.91253f,
	    .03011f,.92623f,.03063f,.93993f,.03115f,.95363f,.03168f,.96612f,
	    .03216f,.9786f,.03265f,.99109f,.03314f,1.00358f,.03363f,1.01607f,
	    .03412f,1.02856f,.03462f,1.04104f,.03513f,1.05353f,.03564f,
	    1.06602f,.03616f,1.0785f,.03669f,1.09023f,.03719f,1.10196f,
	    .03769f,1.11369f,.03821f,1.12541f,.03873f,1.13714f,.03925f,
	    1.14887f,.03979f,1.1606f,.04033f,1.17232f,.04087f,1.18405f,
	    .04143f,1.19577f,.04198f,1.20699f,.04253f,1.21821f,.04307f,
	    1.22942f,.04362f,1.24064f,.04418f,1.25186f,.04473f,1.26307f,
	    .04528f,1.27429f,.04584f,1.28551f,.04639f,1.29672f,.04694f,
	    1.30794f,.04749f,1.31876f,.04801f,1.32958f,.04853f,1.34041f,
	    .04904f,1.35123f,.04955f,1.36205f,.05005f,1.37288f,.05056f,
	    1.3837f,.05106f,1.39453f,.05155f,1.40535f,.05205f,1.41617f,
	    .05254f,1.42675f,.05302f,1.43732f,.0535f,1.44789f,.05398f,
	    1.45847f,.05445f,1.46904f,.05492f,1.47961f,.05538f,1.49019f,
	    .05585f,1.50076f,.0563f,1.51133f,.05676f,1.52191f,.05721f,
	    1.53232f,.05764f,1.54273f,.05807f,1.55315f,.0585f,1.56356f,
	    .05892f,1.57397f,.05933f,1.58439f,.05973f,1.5948f,.06012f,
	    1.60522f,.0605f,1.61563f,.06088f,1.62605f,.06124f,1.63637f,
	    .06158f,1.64669f,.06192f,1.65701f,.06224f,1.66733f,.06255f,
	    1.67765f,.06286f,1.68797f,.06315f,1.69829f,.06344f,1.70862f,
	    .06372f,1.71894f,.06399f,1.72926f,.06425f,1.73961f,.06451f,
	    1.74996f,.06476f,1.76031f,.065f,1.77066f,.06523f,1.78101f,.06546f,
	    1.79136f,.06567f,1.80171f,.06588f,1.81206f,.06608f,1.82241f,
	    .06626f,1.83276f,.06643f,1.84322f,.0666f,1.85368f,.06675f,
	    1.86414f,.06689f,1.87461f,.06702f,1.88507f,.06714f,1.89553f,
	    .06724f,1.906f,.06733f,1.91646f,.0674f,1.92692f,.06746f,1.93738f,
	    .06751f,1.94807f,.06754f,1.95875f,.06755f,1.96943f,.06756f,
	    1.98011f,.06754f,1.9908f,.06752f,2.00148f,.06748f,2.01216f,
	    .06742f,2.02284f,.06736f,2.03353f,.06728f,2.04421f,.06718f,
	    2.05529f,.06707f,2.06637f,.06695f,2.07745f,.06681f,2.08853f,
	    .06667f,2.09961f,.0665f,2.11068f,.06632f,2.12176f,.06613f,
	    2.13284f,.06593f,2.14392f,.06571f,2.155f,.06548f,2.16671f,.06522f,
	    2.17842f,.06494f,2.19013f,.06464f,2.20184f,.06434f,2.21355f,
	    .06401f,2.22526f,.06367f,2.23697f,.06332f,2.24868f,.06295f,
	    2.26038f,.06257f,2.27209f,.06217f,2.28483f,.06173f,2.29758f,
	    .06126f,2.31032f,.06078f,2.32306f,.06029f,2.3358f,.05979f,
	    2.34855f,.05927f,2.36129f,.05875f,2.37403f,.05821f,2.38677f,
	    .05767f,2.39951f,.05712f,2.41402f,.05648f,2.42853f,.05583f,
	    2.44304f,.05517f,2.45755f,.05448f,2.47205f,.05376f,2.48656f,.053f,
	    2.50106f,.05222f,2.51556f,.05138f,2.53005f,.0505f,2.54455f,
	    .04957f,2.56243f,.04834f,2.58031f,.04703f,2.59819f,.04565f,
	    2.61606f,.04421f,2.63393f,.04273f,2.6518f,.0412f,2.66966f,.03965f,
	    2.68753f,.03807f,2.70539f,.03649f,2.72325f,.03491f,2.76507f,
	    .03125f,2.8069f,.02764f,2.84873f,.02408f,2.89056f,.02056f,2.9324f,
	    .01708f,2.97423f,.01363f,3.01607f,.0102f,3.05791f,.00679f,
	    3.09975f,.00339f,3.14159f,0.f };

    static integer j;
    static real phild[251], epsld[251];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --eps;
    --phi;

    /* Function Body */
    for (j = 1; j <= 251; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	epsld[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	phi[j] = (real) (j - 1) * 3.141592654f / 200.f;
/* L201: */
	lcsfit_(&c__251, phild, epsld, &c_true, "B", &c__201, &phi[1], &eps[1]
		, &eps[1], (ftnlen)1);
    }
    return 0;
} /* phep63a_ */

/* Subroutine */ int phep64_(real *phi, real *eps)
{
    /* Initialized data */

    static real a[402] = { 0.f,0.f,.01568f,.00233f,.03136f,.00464f,.04705f,
	    .00692f,.06274f,.00914f,.07843f,.01129f,.09414f,.01336f,.10985f,
	    .01531f,.12558f,.01714f,.14132f,.01883f,.15708f,.02035f,.17274f,
	    .02169f,.18842f,.02287f,.20411f,.0239f,.21981f,.0248f,.23552f,
	    .02557f,.25124f,.02624f,.26697f,.02682f,.2827f,.02731f,.29843f,
	    .02774f,.31416f,.02812f,.32987f,.02846f,.34558f,.02877f,.36129f,
	    .02905f,.377f,.02931f,.39271f,.02957f,.40842f,.02982f,.42412f,
	    .03007f,.43983f,.03033f,.45553f,.0306f,.47124f,.0309f,.48695f,
	    .03122f,.50266f,.03158f,.51837f,.03196f,.53408f,.03236f,.54979f,
	    .0328f,.56549f,.03326f,.5812f,.03375f,.59691f,.03427f,.61262f,
	    .03481f,.62832f,.03538f,.64403f,.03598f,.65975f,.0366f,.67546f,
	    .03725f,.69117f,.03792f,.70688f,.03862f,.72258f,.03935f,.73829f,
	    .0401f,.75399f,.04087f,.7697f,.04167f,.7854f,.0425f,.80111f,
	    .04335f,.81683f,.04423f,.83254f,.04512f,.84825f,.04605f,.86396f,
	    .04699f,.87967f,.04796f,.89537f,.04896f,.91108f,.04998f,.92678f,
	    .05102f,.94248f,.05208f,.9582f,.05317f,.97391f,.05428f,.98962f,
	    .05541f,1.00533f,.05657f,1.02104f,.05774f,1.03675f,.05894f,
	    1.05245f,.06016f,1.06816f,.0614f,1.08386f,.06267f,1.09956f,
	    .06395f,1.11528f,.06526f,1.13099f,.06658f,1.14671f,.06793f,
	    1.16242f,.06931f,1.17813f,.0707f,1.19384f,.07213f,1.20954f,
	    .07357f,1.22524f,.07505f,1.24094f,.07655f,1.25664f,.07808f,
	    1.27236f,.07964f,1.28807f,.08123f,1.30378f,.08284f,1.31949f,
	    .08447f,1.3352f,.08613f,1.35091f,.0878f,1.36661f,.08949f,1.38232f,
	    .09119f,1.39802f,.0929f,1.41372f,.09462f,1.42942f,.09635f,
	    1.44512f,.09808f,1.46083f,.0998f,1.47653f,.10151f,1.49223f,
	    .10321f,1.50794f,.10488f,1.52365f,.10653f,1.53936f,.10815f,
	    1.55508f,.10972f,1.5708f,.11125f,1.58649f,.11273f,1.60218f,
	    .11415f,1.61788f,.11553f,1.63358f,.11686f,1.64929f,.11814f,1.665f,
	    .11938f,1.68072f,.12057f,1.69643f,.12171f,1.71216f,.12281f,
	    1.72788f,.12386f,1.74358f,.12487f,1.75928f,.12583f,1.77498f,
	    .12675f,1.79068f,.12762f,1.80639f,.12844f,1.8221f,.12922f,
	    1.83781f,.12994f,1.85353f,.13062f,1.86924f,.13125f,1.88496f,
	    .13182f,1.90066f,.13234f,1.91637f,.13281f,1.93207f,.13322f,
	    1.94778f,.13358f,1.96349f,.13389f,1.9792f,.13414f,1.99491f,
	    .13434f,2.01062f,.13448f,2.02633f,.13456f,2.04204f,.13459f,
	    2.05775f,.13456f,2.07346f,.13447f,2.08917f,.13433f,2.10488f,
	    .13413f,2.12059f,.13387f,2.1363f,.13354f,2.152f,.13316f,2.16771f,
	    .13272f,2.18341f,.13222f,2.19911f,.13166f,2.21483f,.13104f,
	    2.23055f,.13035f,2.24627f,.1296f,2.26198f,.12879f,2.2777f,.12792f,
	    2.2934f,.12698f,2.30911f,.12598f,2.32481f,.12492f,2.3405f,.1238f,
	    2.35619f,.12261f,2.37192f,.12136f,2.38765f,.12004f,2.40337f,
	    .11866f,2.41909f,.11722f,2.4348f,.11572f,2.45051f,.11416f,
	    2.46621f,.11254f,2.4819f,.11087f,2.49759f,.10914f,2.51327f,
	    .10735f,2.52901f,.1055f,2.54474f,.1036f,2.56047f,.10164f,2.57619f,
	    .09963f,2.5919f,.09757f,2.6076f,.09545f,2.6233f,.09328f,2.63899f,
	    .09105f,2.65467f,.08878f,2.67035f,.08645f,2.68609f,.08406f,
	    2.70182f,.08162f,2.71755f,.07913f,2.73327f,.0766f,2.74898f,
	    .07402f,2.76468f,.07139f,2.78038f,.06873f,2.79607f,.06603f,
	    2.81175f,.06329f,2.82743f,.06052f,2.84316f,.0577f,2.85889f,
	    .05486f,2.87461f,.05198f,2.89032f,.04907f,2.90603f,.04614f,
	    2.92173f,.04318f,2.93743f,.0402f,2.95313f,.0372f,2.96882f,.03417f,
	    2.98451f,.03113f,3.00023f,.02807f,3.01594f,.02499f,3.03165f,
	    .02189f,3.04736f,.01879f,3.06307f,.01567f,3.07878f,.01255f,
	    3.09448f,.00942f,3.11018f,.00628f,3.12589f,.00314f,3.14159f,0.f };

    static integer j;

    /* Parameter adjustments */
    --eps;
    --phi;

    /* Function Body */
    for (j = 1; j <= 201; ++j) {
	phi[j] = a[(j << 1) - 2];
	eps[j] = a[(j << 1) - 1];
/* L201: */
    }
    return 0;
} /* phep64_ */

/* Subroutine */ int phep64a_(real *phi, real *eps)
{
    /* Initialized data */

    static real a[502] = { 0.f,0.f,.01468f,.00149f,.02936f,.00297f,.04403f,
	    .00445f,.05872f,.00591f,.0734f,.00736f,.08808f,.00878f,.10277f,
	    .01018f,.11746f,.01156f,.13216f,.0129f,.14686f,.0142f,.15019f,
	    .01449f,.15352f,.01477f,.15685f,.01505f,.16018f,.01532f,.16351f,
	    .01558f,.16684f,.01583f,.17017f,.01606f,.1735f,.01628f,.17684f,
	    .01648f,.18017f,.01667f,.18528f,.0169f,.19039f,.01709f,.19551f,
	    .01724f,.20062f,.01736f,.20574f,.01745f,.21086f,.01752f,.21598f,
	    .01758f,.22109f,.01762f,.22621f,.01767f,.23133f,.01772f,.2409f,
	    .01782f,.25046f,.01796f,.26003f,.01811f,.26959f,.01828f,.27915f,
	    .01845f,.28871f,.01861f,.29827f,.01877f,.30783f,.01891f,.31739f,
	    .01903f,.32695f,.01912f,.3404f,.01919f,.35384f,.0192f,.36728f,
	    .01916f,.38072f,.01908f,.39417f,.01897f,.40761f,.01886f,.42105f,
	    .01874f,.4345f,.01862f,.44794f,.01854f,.46139f,.01848f,.47194f,
	    .01846f,.48249f,.01847f,.49304f,.01851f,.50359f,.01856f,.51414f,
	    .01864f,.52469f,.01873f,.53523f,.01883f,.54578f,.01895f,.55633f,
	    .01907f,.56688f,.01921f,.57595f,.01933f,.58502f,.01945f,.5941f,
	    .01958f,.60317f,.01972f,.61224f,.01985f,.62131f,.01999f,.63038f,
	    .02013f,.63945f,.02028f,.64852f,.02043f,.65759f,.02058f,.67317f,
	    .02084f,.68874f,.02111f,.70432f,.02139f,.71989f,.02169f,.73546f,
	    .02199f,.75104f,.02231f,.76661f,.02264f,.78218f,.02299f,.79776f,
	    .02335f,.81333f,.02373f,.82695f,.02408f,.84057f,.02444f,.85419f,
	    .02482f,.86781f,.0252f,.88142f,.0256f,.89504f,.02601f,.90866f,
	    .02643f,.92228f,.02685f,.93589f,.02728f,.94951f,.02772f,.96194f,
	    .02812f,.97437f,.02852f,.98679f,.02893f,.99922f,.02935f,1.01165f,
	    .02977f,1.02407f,.0302f,1.0365f,.03064f,1.04892f,.03108f,1.06135f,
	    .03153f,1.07377f,.03198f,1.08546f,.03242f,1.09714f,.03286f,
	    1.10882f,.03332f,1.1205f,.03378f,1.13219f,.03425f,1.14387f,
	    .03472f,1.15555f,.03521f,1.16723f,.03571f,1.17891f,.03622f,
	    1.19059f,.03674f,1.2018f,.03724f,1.21301f,.03776f,1.22423f,
	    .03828f,1.23544f,.03882f,1.24665f,.03936f,1.25786f,.0399f,
	    1.26907f,.04045f,1.28028f,.041f,1.29149f,.04156f,1.3027f,.04212f,
	    1.31357f,.04266f,1.32444f,.0432f,1.33531f,.04375f,1.34618f,
	    .04429f,1.35705f,.04484f,1.36792f,.04539f,1.37878f,.04594f,
	    1.38965f,.04649f,1.40052f,.04704f,1.41139f,.0476f,1.42204f,
	    .04814f,1.43268f,.04869f,1.44333f,.04923f,1.45397f,.04978f,
	    1.46462f,.05032f,1.47527f,.05087f,1.48591f,.05141f,1.49656f,
	    .05194f,1.50721f,.05248f,1.51785f,.05301f,1.52834f,.05352f,
	    1.53883f,.05403f,1.54932f,.05454f,1.5598f,.05504f,1.57029f,
	    .05552f,1.58078f,.056f,1.59127f,.05647f,1.60176f,.05693f,1.61225f,
	    .05738f,1.62274f,.05781f,1.63312f,.05822f,1.64349f,.05862f,
	    1.65387f,.05901f,1.66425f,.05939f,1.67463f,.05976f,1.68501f,
	    .06011f,1.69539f,.06045f,1.70577f,.06079f,1.71615f,.06111f,
	    1.72653f,.06143f,1.73692f,.06174f,1.74732f,.06204f,1.75771f,
	    .06233f,1.76811f,.06262f,1.77851f,.06289f,1.7889f,.06316f,1.7993f,
	    .06341f,1.8097f,.06365f,1.82009f,.06389f,1.83049f,.06411f,1.841f,
	    .06432f,1.8515f,.06452f,1.86201f,.06471f,1.87252f,.06489f,
	    1.88303f,.06505f,1.89354f,.0652f,1.90405f,.06534f,1.91455f,
	    .06546f,1.92506f,.06557f,1.93557f,.06567f,1.9463f,.06575f,
	    1.95704f,.06582f,1.96777f,.06587f,1.9785f,.06591f,1.98923f,
	    .06594f,1.99996f,.06595f,2.01069f,.06594f,2.02142f,.06593f,
	    2.03215f,.0659f,2.04288f,.06586f,2.05401f,.0658f,2.06514f,.06573f,
	    2.07627f,.06565f,2.0874f,.06555f,2.09853f,.06544f,2.10965f,
	    .06531f,2.12078f,.06517f,2.13191f,.06502f,2.14304f,.06485f,
	    2.15416f,.06466f,2.16592f,.06445f,2.17767f,.06422f,2.18943f,
	    .06398f,2.20118f,.06371f,2.21293f,.06344f,2.22469f,.06314f,
	    2.23644f,.06283f,2.24819f,.06251f,2.25994f,.06216f,2.27169f,
	    .0618f,2.28447f,.0614f,2.29725f,.06097f,2.31002f,.06053f,2.32279f,
	    .06007f,2.33557f,.0596f,2.34834f,.05912f,2.36111f,.05862f,
	    2.37389f,.05811f,2.38666f,.0576f,2.39943f,.05707f,2.41397f,
	    .05646f,2.4285f,.05584f,2.44304f,.0552f,2.45757f,.05454f,2.4721f,
	    .05384f,2.48663f,.05312f,2.50115f,.05235f,2.51568f,.05154f,
	    2.5302f,.05068f,2.54471f,.04977f,2.56262f,.04856f,2.58052f,
	    .04728f,2.59842f,.04592f,2.61631f,.04451f,2.6342f,.04304f,
	    2.65209f,.04154f,2.66997f,.04f,2.68785f,.03844f,2.70573f,.03687f,
	    2.72362f,.0353f,2.76541f,.03165f,2.8072f,.02804f,2.84899f,.02446f,
	    2.89079f,.02091f,2.93259f,.01739f,2.97439f,.01389f,3.01619f,
	    .0104f,3.05799f,.00693f,3.09979f,.00346f,3.14159f,0.f };

    static integer j;
    static real phild[251], epsld[251];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --eps;
    --phi;

    /* Function Body */
    for (j = 1; j <= 251; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	epsld[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	phi[j] = (real) (j - 1) * 3.141592654f / 200.f;
/* L201: */
	lcsfit_(&c__251, phild, epsld, &c_true, "B", &c__201, &phi[1], &eps[1]
		, &eps[1], (ftnlen)1);
    }
    return 0;
} /* phep64a_ */

/* Subroutine */ int phep65_(real *phi, real *eps)
{
    /* Initialized data */

    static real a[502] = { 0.f,0.f,.01484f,.00156f,.02967f,.00312f,.04451f,
	    .00466f,.05935f,.00618f,.0742f,.00767f,.08904f,.00913f,.10389f,
	    .01054f,.11875f,.01191f,.13361f,.01322f,.14848f,.01446f,.15175f,
	    .01473f,.15503f,.01499f,.1583f,.01524f,.16157f,.01549f,.16485f,
	    .01572f,.16812f,.01595f,.1714f,.01616f,.17468f,.01636f,.17795f,
	    .01655f,.18123f,.01671f,.18623f,.01693f,.19123f,.01712f,.19623f,
	    .01727f,.20123f,.0174f,.20623f,.0175f,.21124f,.01759f,.21624f,
	    .01766f,.22124f,.01773f,.22625f,.0178f,.23125f,.01788f,.24072f,
	    .01804f,.25018f,.01822f,.25964f,.01842f,.2691f,.01863f,.27856f,
	    .01884f,.28802f,.01903f,.29748f,.0192f,.30694f,.01934f,.3164f,
	    .01945f,.32586f,.0195f,.33918f,.01949f,.3525f,.01938f,.36582f,
	    .01919f,.37915f,.01895f,.39247f,.01866f,.4058f,.01836f,.41912f,
	    .01805f,.43245f,.01775f,.44577f,.01749f,.4591f,.01728f,.46962f,
	    .01716f,.48015f,.01708f,.49068f,.01703f,.5012f,.01702f,.51173f,
	    .01704f,.52226f,.01708f,.53278f,.01714f,.54331f,.01722f,.55384f,
	    .01731f,.56436f,.01741f,.57344f,.01751f,.58252f,.0176f,.5916f,
	    .01771f,.60068f,.01782f,.60976f,.01794f,.61884f,.01806f,.62792f,
	    .0182f,.637f,.01834f,.64607f,.0185f,.65515f,.01867f,.67086f,
	    .01898f,.68656f,.01933f,.70226f,.01971f,.71797f,.02012f,.73367f,
	    .02055f,.74937f,.021f,.76508f,.02147f,.78078f,.02196f,.79648f,
	    .02246f,.81218f,.02298f,.8259f,.02343f,.83962f,.0239f,.85333f,
	    .02437f,.86705f,.02485f,.88077f,.02534f,.89448f,.02584f,.9082f,
	    .02636f,.92192f,.02689f,.93563f,.02744f,.94934f,.028f,.96192f,
	    .02853f,.97449f,.02907f,.98706f,.02963f,.99964f,.0302f,1.01221f,
	    .03078f,1.02478f,.03137f,1.03735f,.03198f,1.04992f,.03259f,
	    1.06249f,.03321f,1.07506f,.03385f,1.08689f,.03445f,1.09872f,
	    .03506f,1.11055f,.03568f,1.12238f,.03631f,1.13421f,.03694f,
	    1.14603f,.03759f,1.15786f,.03824f,1.16969f,.0389f,1.18151f,
	    .03957f,1.19334f,.04025f,1.20469f,.04091f,1.21605f,.04158f,
	    1.2274f,.04225f,1.23876f,.04294f,1.25011f,.04363f,1.26146f,
	    .04434f,1.27281f,.04505f,1.28416f,.04577f,1.29551f,.0465f,
	    1.30686f,.04724f,1.31793f,.04798f,1.329f,.04872f,1.34007f,.04947f,
	    1.35114f,.05023f,1.36221f,.05099f,1.37328f,.05177f,1.38434f,
	    .05255f,1.39541f,.05334f,1.40648f,.05414f,1.41754f,.05495f,
	    1.42845f,.05575f,1.43935f,.05655f,1.45025f,.05737f,1.46116f,
	    .05819f,1.47206f,.05901f,1.48296f,.05984f,1.49387f,.06067f,
	    1.50477f,.0615f,1.51567f,.06233f,1.52657f,.06316f,1.53737f,
	    .06399f,1.54816f,.06481f,1.55896f,.06563f,1.56975f,.06645f,
	    1.58055f,.06727f,1.59134f,.06808f,1.60214f,.06889f,1.61293f,
	    .06969f,1.62373f,.07049f,1.63452f,.07127f,1.64525f,.07205f,
	    1.65598f,.07282f,1.66671f,.07358f,1.67744f,.07433f,1.68817f,
	    .07507f,1.6989f,.0758f,1.70963f,.07651f,1.72036f,.07722f,1.73109f,
	    .07792f,1.74182f,.07861f,1.75254f,.07928f,1.76325f,.07994f,
	    1.77396f,.08058f,1.78468f,.08121f,1.79539f,.08182f,1.80611f,
	    .08242f,1.81683f,.08299f,1.82754f,.08355f,1.83826f,.08408f,
	    1.84898f,.08459f,1.85972f,.08508f,1.87046f,.08555f,1.8812f,
	    .08599f,1.89194f,.08641f,1.90268f,.0868f,1.91343f,.08718f,
	    1.92417f,.08754f,1.93491f,.08787f,1.94566f,.08819f,1.9564f,
	    .08848f,1.96732f,.08876f,1.97823f,.08903f,1.98915f,.08927f,
	    2.00006f,.08949f,2.01098f,.08969f,2.0219f,.08988f,2.03282f,
	    .09004f,2.04373f,.09019f,2.05465f,.09031f,2.06557f,.09041f,
	    2.07682f,.0905f,2.08806f,.09056f,2.09931f,.09061f,2.11056f,
	    .09062f,2.1218f,.09061f,2.13305f,.09058f,2.14429f,.09052f,
	    2.15554f,.09043f,2.16679f,.09032f,2.17803f,.09017f,2.18979f,
	    .08999f,2.20155f,.08977f,2.2133f,.08953f,2.22506f,.08925f,
	    2.23681f,.08894f,2.24856f,.08861f,2.26032f,.08824f,2.27207f,
	    .08785f,2.28382f,.08743f,2.29557f,.08698f,2.30822f,.08647f,
	    2.32088f,.08593f,2.33353f,.08536f,2.34618f,.08477f,2.35883f,
	    .08414f,2.37148f,.08348f,2.38412f,.08279f,2.39677f,.08208f,
	    2.40942f,.08134f,2.42206f,.08056f,2.43618f,.07967f,2.45029f,
	    .07872f,2.4644f,.07772f,2.47851f,.07666f,2.4926f,.07552f,2.50669f,
	    .07429f,2.52077f,.07297f,2.53483f,.07155f,2.54888f,.07f,2.56292f,
	    .06833f,2.57978f,.06615f,2.59661f,.0638f,2.61343f,.06132f,
	    2.63024f,.05873f,2.64704f,.05606f,2.66383f,.05334f,2.68062f,
	    .0506f,2.6974f,.04788f,2.7142f,.04519f,2.731f,.04258f,2.77185f,
	    .03664f,2.81276f,.03122f,2.85373f,.02628f,2.89475f,.02176f,
	    2.93582f,.01759f,2.97693f,.01371f,3.01807f,.01008f,3.05923f,
	    .00662f,3.10041f,.00328f,3.14159f,0.f };

    static integer j;
    static real phild[251], epsld[251];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --eps;
    --phi;

    /* Function Body */
    for (j = 1; j <= 251; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	epsld[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	phi[j] = (real) (j - 1) * 3.141592654f / 200.f;
/* L201: */
	lcsfit_(&c__251, phild, epsld, &c_true, "B", &c__201, &phi[1], &eps[1]
		, &eps[1], (ftnlen)1);
    }
    return 0;
} /* phep65_ */

/* Subroutine */ int phep65a_(real *phi, real *eps)
{
    /* Initialized data */

    static real a[502] = { 0.f,0.f,.01453f,.00137f,.02907f,.00274f,.0436f,
	    .0041f,.05814f,.00545f,.07267f,.00679f,.08721f,.00811f,.10175f,
	    .00941f,.1163f,.0107f,.13084f,.01195f,.14539f,.01318f,.14877f,
	    .01346f,.15214f,.01374f,.15552f,.01401f,.15889f,.01427f,.16227f,
	    .01453f,.16564f,.01477f,.16902f,.015f,.1724f,.01521f,.17578f,
	    .01541f,.17916f,.01559f,.18436f,.01583f,.18956f,.01602f,.19477f,
	    .01618f,.19997f,.0163f,.20518f,.0164f,.21039f,.01648f,.21559f,
	    .01654f,.2208f,.0166f,.22601f,.01665f,.23122f,.01671f,.24077f,
	    .01683f,.25032f,.01698f,.25987f,.01715f,.26941f,.01732f,.27896f,
	    .01749f,.28851f,.01766f,.29805f,.01782f,.3076f,.01796f,.31714f,
	    .01807f,.32669f,.01814f,.33996f,.01819f,.35322f,.01816f,.36649f,
	    .01807f,.37976f,.01793f,.39302f,.01775f,.40629f,.01755f,.41956f,
	    .01732f,.43283f,.01708f,.4461f,.01684f,.45937f,.01661f,.46976f,
	    .01644f,.48015f,.01628f,.49054f,.01614f,.50093f,.01601f,.51132f,
	    .01589f,.52171f,.01578f,.5321f,.01569f,.54249f,.01561f,.55288f,
	    .01554f,.56327f,.01549f,.57223f,.01546f,.58118f,.01543f,.59014f,
	    .01541f,.59909f,.01541f,.60805f,.01541f,.617f,.01542f,.62596f,
	    .01544f,.63491f,.01547f,.64387f,.01551f,.65282f,.01555f,.66829f,
	    .01565f,.68376f,.01577f,.69923f,.0159f,.7147f,.01607f,.73017f,
	    .01625f,.74564f,.01645f,.7611f,.01668f,.77657f,.01693f,.79204f,
	    .0172f,.8075f,.0175f,.82106f,.01778f,.83461f,.01807f,.84816f,
	    .01837f,.86172f,.01869f,.87527f,.01902f,.88882f,.01936f,.90237f,
	    .0197f,.91593f,.02005f,.92948f,.0204f,.94303f,.02076f,.95538f,
	    .02108f,.96772f,.02141f,.98007f,.02174f,.99242f,.02207f,1.00476f,
	    .0224f,1.01711f,.02274f,1.02945f,.02309f,1.0418f,.02344f,1.05415f,
	    .02381f,1.06649f,.02418f,1.0781f,.02454f,1.08971f,.02491f,
	    1.10132f,.02528f,1.11293f,.02567f,1.12454f,.02607f,1.13615f,
	    .02647f,1.14776f,.02689f,1.15937f,.02731f,1.17098f,.02775f,
	    1.18259f,.02819f,1.19373f,.02863f,1.20486f,.02907f,1.216f,.02952f,
	    1.22713f,.02998f,1.23827f,.03044f,1.2494f,.03091f,1.26054f,
	    .03138f,1.27167f,.03185f,1.28281f,.03232f,1.29394f,.0328f,
	    1.30473f,.03326f,1.31552f,.03371f,1.32631f,.03417f,1.33711f,
	    .03463f,1.3479f,.0351f,1.35869f,.03557f,1.36948f,.03604f,1.38027f,
	    .03652f,1.39106f,.03701f,1.40185f,.0375f,1.41248f,.038f,1.42312f,
	    .03851f,1.43375f,.03902f,1.44438f,.03954f,1.45501f,.04007f,
	    1.46564f,.0406f,1.47628f,.04114f,1.48691f,.04169f,1.49754f,
	    .04224f,1.50817f,.04279f,1.51872f,.04335f,1.52927f,.04391f,
	    1.53983f,.04447f,1.55038f,.04502f,1.56093f,.04558f,1.57148f,
	    .04614f,1.58203f,.04669f,1.59258f,.04723f,1.60314f,.04777f,
	    1.61369f,.0483f,1.62418f,.04882f,1.63467f,.04933f,1.64516f,
	    .04984f,1.65565f,.05033f,1.66614f,.05082f,1.67663f,.0513f,
	    1.68712f,.05177f,1.69762f,.05224f,1.70811f,.0527f,1.7186f,.05316f,
	    1.72914f,.05361f,1.73967f,.05406f,1.7502f,.05449f,1.76074f,
	    .05493f,1.77127f,.05535f,1.78181f,.05577f,1.79234f,.05618f,
	    1.80288f,.05657f,1.81341f,.05696f,1.82395f,.05734f,1.8346f,
	    .05771f,1.84526f,.05807f,1.85591f,.05842f,1.86657f,.05876f,
	    1.87722f,.05908f,1.88788f,.05939f,1.89853f,.05969f,1.90919f,
	    .05997f,1.91985f,.06023f,1.9305f,.06049f,1.94138f,.06073f,
	    1.95225f,.06095f,1.96313f,.06116f,1.97401f,.06135f,1.98488f,
	    .06153f,1.99576f,.0617f,2.00664f,.06185f,2.01751f,.06199f,
	    2.02839f,.06212f,2.03927f,.06224f,2.04786f,.06232f,2.05645f,
	    .0624f,2.06504f,.06247f,2.07363f,.06254f,2.08222f,.06259f,
	    2.09081f,.06264f,2.0994f,.06268f,2.10799f,.06272f,2.11658f,
	    .06274f,2.12517f,.06276f,2.13979f,.06277f,2.15441f,.06275f,
	    2.16903f,.06271f,2.18365f,.06264f,2.19826f,.06255f,2.21288f,
	    .06242f,2.2275f,.06226f,2.24211f,.06207f,2.25673f,.06185f,
	    2.27134f,.06159f,2.28424f,.06134f,2.29714f,.06106f,2.31003f,
	    .06075f,2.32292f,.06042f,2.33582f,.06007f,2.34871f,.0597f,2.3616f,
	    .05932f,2.37449f,.05891f,2.38739f,.0585f,2.40028f,.05807f,2.4149f,
	    .05756f,2.42952f,.05704f,2.44413f,.0565f,2.45875f,.05592f,
	    2.47336f,.05531f,2.48798f,.05466f,2.50259f,.05397f,2.51719f,
	    .05323f,2.5318f,.05244f,2.5464f,.05159f,2.56435f,.05046f,2.58229f,
	    .04925f,2.60023f,.04795f,2.61816f,.04659f,2.6361f,.04517f,
	    2.65402f,.0437f,2.67195f,.0422f,2.68987f,.04065f,2.70779f,.03909f,
	    2.72571f,.03751f,2.76731f,.03383f,2.80891f,.03013f,2.8505f,.0264f,
	    2.89209f,.02266f,2.93368f,.01891f,2.97526f,.01514f,3.01684f,
	    .01136f,3.05843f,.00758f,3.10001f,.00379f,3.14159f,0.f };

    static integer j;
    static real phild[251], epsld[251];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --eps;
    --phi;

    /* Function Body */
    for (j = 1; j <= 251; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	epsld[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	phi[j] = (real) (j - 1) * 3.141592654f / 200.f;
/* L201: */
	lcsfit_(&c__251, phild, epsld, &c_true, "B", &c__201, &phi[1], &eps[1]
		, &eps[1], (ftnlen)1);
    }
    return 0;
} /* phep65a_ */

/* Subroutine */ int phep66_(real *phi, real *eps)
{
    /* Initialized data */

    static real a[402] = { 0.f,0.f,.0157f,.00145f,.03139f,.0029f,.04709f,
	    .00433f,.06279f,.00574f,.07849f,.00712f,.0942f,.00847f,.10991f,
	    .00978f,.12563f,.01105f,.14135f,.01225f,.15708f,.0134f,.17277f,
	    .01447f,.18847f,.01547f,.20417f,.01638f,.21987f,.01719f,.23559f,
	    .01789f,.2513f,.01847f,.26701f,.01893f,.28273f,.01924f,.29844f,
	    .0194f,.31416f,.0194f,.32987f,.01924f,.34558f,.01893f,.36129f,
	    .0185f,.37699f,.01799f,.3927f,.01741f,.40841f,.01679f,.42411f,
	    .01616f,.43982f,.01556f,.45553f,.01499f,.47124f,.0145f,.48694f,
	    .0141f,.50265f,.01379f,.51836f,.01356f,.53406f,.0134f,.54977f,
	    .01331f,.56548f,.01327f,.58119f,.01328f,.5969f,.01333f,.61261f,
	    .0134f,.62832f,.0135f,.64403f,.01361f,.65974f,.01373f,.67545f,
	    .01387f,.69116f,.01402f,.70686f,.01419f,.72257f,.01438f,.73828f,
	    .01458f,.75399f,.0148f,.76969f,.01504f,.7854f,.0153f,.80111f,
	    .01558f,.81682f,.01588f,.83253f,.0162f,.84824f,.01654f,.86394f,
	    .01689f,.87965f,.01726f,.89536f,.01765f,.91107f,.01805f,.92677f,
	    .01847f,.94248f,.0189f,.95819f,.01934f,.9739f,.0198f,.98961f,
	    .02026f,1.00532f,.02074f,1.02103f,.02124f,1.03673f,.02174f,
	    1.05244f,.02226f,1.06815f,.02279f,1.08385f,.02334f,1.09956f,
	    .0239f,1.11527f,.02447f,1.13098f,.02506f,1.14669f,.02566f,1.1624f,
	    .02627f,1.17811f,.0269f,1.19381f,.02754f,1.20952f,.02819f,
	    1.22523f,.02885f,1.24093f,.02952f,1.25664f,.0302f,1.27235f,
	    .03089f,1.28806f,.0316f,1.30377f,.03231f,1.31948f,.03304f,
	    1.33519f,.03378f,1.3509f,.03453f,1.3666f,.0353f,1.38231f,.03608f,
	    1.39802f,.03688f,1.41372f,.0377f,1.42943f,.03853f,1.44514f,
	    .03938f,1.46085f,.04025f,1.47656f,.04113f,1.49227f,.04202f,
	    1.50798f,.04293f,1.52368f,.04386f,1.53939f,.04479f,1.5551f,
	    .04574f,1.5708f,.0467f,1.58651f,.04767f,1.60223f,.04866f,1.61794f,
	    .04966f,1.63365f,.05067f,1.64936f,.05171f,1.66507f,.05277f,
	    1.68077f,.05386f,1.69648f,.05498f,1.71218f,.05612f,1.72788f,
	    .0573f,1.74359f,.05851f,1.75931f,.05976f,1.77501f,.06103f,
	    1.79072f,.06231f,1.80643f,.06362f,1.82214f,.06493f,1.83784f,
	    .06625f,1.85355f,.06758f,1.86925f,.06889f,1.88496f,.0702f,
	    1.90066f,.07149f,1.91636f,.07277f,1.93206f,.07402f,1.94776f,
	    .07524f,1.96347f,.07644f,1.97918f,.0776f,1.99489f,.07872f,2.0106f,
	    .07979f,2.02632f,.08082f,2.04204f,.0818f,2.05773f,.08272f,
	    2.07343f,.08359f,2.08913f,.0844f,2.10484f,.08515f,2.12054f,
	    .08585f,2.13625f,.08649f,2.15196f,.08708f,2.16768f,.08761f,
	    2.18339f,.08808f,2.19911f,.0885f,2.21482f,.08886f,2.23052f,
	    .08916f,2.24623f,.08941f,2.26194f,.08959f,2.27765f,.08972f,
	    2.29336f,.08978f,2.30906f,.08978f,2.32477f,.08972f,2.34048f,
	    .08959f,2.35619f,.0894f,2.37191f,.08914f,2.38762f,.08882f,
	    2.40333f,.08843f,2.41905f,.08797f,2.43476f,.08745f,2.45046f,
	    .08687f,2.46617f,.08622f,2.48187f,.08551f,2.49757f,.08474f,
	    2.51327f,.0839f,2.529f,.083f,2.54473f,.08203f,2.56045f,.08101f,
	    2.57616f,.07991f,2.59188f,.07875f,2.60758f,.07752f,2.62328f,
	    .07622f,2.63898f,.07485f,2.65467f,.07341f,2.67035f,.0719f,
	    2.68609f,.07031f,2.70183f,.06864f,2.71756f,.06691f,2.73328f,
	    .0651f,2.74899f,.06323f,2.76469f,.06129f,2.78039f,.05928f,
	    2.79608f,.05722f,2.81176f,.05509f,2.82743f,.0529f,2.84317f,
	    .05064f,2.85891f,.04833f,2.87463f,.04596f,2.89035f,.04354f,
	    2.90606f,.04108f,2.92176f,.03856f,2.93746f,.03601f,2.95315f,
	    .03341f,2.96883f,.03077f,2.98451f,.0281f,3.00023f,.02539f,
	    3.01595f,.02264f,3.03167f,.01987f,3.04738f,.01707f,3.06309f,
	    .01426f,3.07879f,.01143f,3.09449f,.00858f,3.11019f,.00573f,
	    3.12589f,.00287f,3.14159f,0.f };

    static integer j;

    /* Parameter adjustments */
    --eps;
    --phi;

    /* Function Body */
    for (j = 1; j <= 201; ++j) {
	phi[j] = a[(j << 1) - 2];
	eps[j] = a[(j << 1) - 1];
/* L201: */
    }
    return 0;
} /* phep66_ */

/* Subroutine */ int phep67_(real *phi, real *eps)
{
    /* Initialized data */

    static real a[502] = { 0.f,0.f,.01495f,.00161f,.0299f,.00322f,.04485f,
	    .00482f,.0598f,.0064f,.07476f,.00796f,.08971f,.00949f,.10468f,
	    .011f,.11964f,.01246f,.13461f,.01388f,.14959f,.01525f,.1528f,
	    .01554f,.15601f,.01582f,.15922f,.0161f,.16243f,.01637f,.16565f,
	    .01664f,.16886f,.0169f,.17207f,.01714f,.17529f,.01738f,.1785f,
	    .01761f,.18172f,.01782f,.18687f,.01813f,.19201f,.01841f,.19716f,
	    .01865f,.20231f,.01888f,.20747f,.01908f,.21262f,.01927f,.21777f,
	    .01945f,.22293f,.01962f,.22808f,.01979f,.23323f,.01996f,.24276f,
	    .02029f,.25229f,.02063f,.26182f,.02097f,.27135f,.02131f,.28088f,
	    .02164f,.29041f,.02195f,.29994f,.02224f,.30947f,.02249f,.319f,
	    .02269f,.32853f,.02285f,.3418f,.02298f,.35507f,.02301f,.36834f,
	    .02296f,.38162f,.02284f,.39489f,.02267f,.40817f,.02245f,.42144f,
	    .0222f,.43472f,.02194f,.44799f,.02168f,.46127f,.02143f,.47164f,
	    .02125f,.48201f,.02108f,.49238f,.02093f,.50274f,.02079f,.51311f,
	    .02067f,.52348f,.02055f,.53385f,.02045f,.54422f,.02035f,.55458f,
	    .02026f,.56495f,.02017f,.57385f,.0201f,.58275f,.02004f,.59165f,
	    .01998f,.60055f,.01992f,.60945f,.01987f,.61835f,.01983f,.62725f,
	    .01979f,.63615f,.01975f,.64505f,.01972f,.65395f,.0197f,.66928f,
	    .01967f,.68461f,.01966f,.69994f,.01967f,.71527f,.01969f,.7306f,
	    .01973f,.74593f,.01978f,.76126f,.01985f,.77658f,.01992f,.79191f,
	    .02001f,.80724f,.0201f,.82061f,.02019f,.83397f,.02029f,.84734f,
	    .02039f,.8607f,.02051f,.87407f,.02062f,.88743f,.02075f,.9008f,
	    .02088f,.91416f,.02102f,.92753f,.02117f,.94089f,.02133f,.95309f,
	    .02148f,.9653f,.02164f,.9775f,.0218f,.9897f,.02198f,1.0019f,
	    .02216f,1.0141f,.02234f,1.0263f,.02254f,1.0385f,.02274f,1.0507f,
	    .02295f,1.06289f,.02316f,1.07435f,.02337f,1.0858f,.02359f,
	    1.09726f,.02381f,1.10871f,.02404f,1.12017f,.02427f,1.13162f,
	    .02451f,1.14308f,.02476f,1.15453f,.02501f,1.16599f,.02527f,
	    1.17744f,.02553f,1.1884f,.02579f,1.19936f,.02606f,1.21033f,
	    .02632f,1.22129f,.0266f,1.23225f,.02688f,1.24321f,.02716f,
	    1.25417f,.02745f,1.26513f,.02775f,1.27609f,.02805f,1.28706f,
	    .02836f,1.2977f,.02866f,1.30835f,.02897f,1.319f,.02928f,1.32965f,
	    .0296f,1.3403f,.02992f,1.35094f,.03025f,1.36159f,.03059f,1.37224f,
	    .03093f,1.38289f,.03128f,1.39353f,.03163f,1.40401f,.03199f,
	    1.4145f,.03235f,1.42498f,.03271f,1.43546f,.03309f,1.44594f,
	    .03346f,1.45642f,.03385f,1.4669f,.03424f,1.47738f,.03464f,
	    1.48786f,.03505f,1.49834f,.03546f,1.50878f,.03588f,1.51921f,
	    .0363f,1.52965f,.03673f,1.54008f,.03717f,1.55052f,.03762f,
	    1.56095f,.03807f,1.57139f,.03852f,1.58182f,.03899f,1.59226f,
	    .03945f,1.60269f,.03993f,1.61319f,.04041f,1.62369f,.0409f,1.6342f,
	    .04139f,1.6447f,.0419f,1.6552f,.04241f,1.6657f,.04293f,1.6762f,
	    .04346f,1.6867f,.044f,1.6972f,.04455f,1.7077f,.04511f,1.71841f,
	    .0457f,1.72913f,.04629f,1.73984f,.0469f,1.75056f,.04752f,1.76127f,
	    .04815f,1.77199f,.0488f,1.7827f,.04945f,1.79341f,.05011f,1.80413f,
	    .05078f,1.81484f,.05145f,1.82589f,.05216f,1.83694f,.05288f,
	    1.84799f,.0536f,1.85904f,.05434f,1.87009f,.05509f,1.88113f,
	    .05584f,1.89218f,.05661f,1.90323f,.05739f,1.91427f,.05818f,
	    1.92531f,.05899f,1.93688f,.05985f,1.94844f,.06072f,1.95999f,
	    .0616f,1.97155f,.0625f,1.98311f,.0634f,1.99467f,.06432f,2.00622f,
	    .06524f,2.01778f,.06617f,2.02933f,.06711f,2.04089f,.06805f,
	    2.05305f,.06905f,2.06522f,.07005f,2.07739f,.07105f,2.08956f,
	    .07204f,2.10172f,.07302f,2.11389f,.07398f,2.12606f,.07493f,
	    2.13824f,.07586f,2.15041f,.07677f,2.16259f,.07765f,2.17534f,
	    .07854f,2.1881f,.07939f,2.20085f,.0802f,2.21361f,.08097f,2.22637f,
	    .08171f,2.23914f,.0824f,2.2519f,.08305f,2.26467f,.08365f,2.27744f,
	    .08421f,2.29021f,.08471f,2.30376f,.0852f,2.31733f,.08563f,
	    2.33089f,.08601f,2.34445f,.08634f,2.35802f,.08662f,2.37159f,
	    .08685f,2.38516f,.08703f,2.39874f,.08716f,2.41232f,.08726f,
	    2.4259f,.08731f,2.44078f,.08731f,2.45566f,.08725f,2.47054f,.0871f,
	    2.48542f,.08684f,2.50029f,.08646f,2.51515f,.08594f,2.53f,.08525f,
	    2.54483f,.08439f,2.55965f,.08333f,2.57444f,.08205f,2.59147f,
	    .08029f,2.60847f,.07826f,2.62544f,.07599f,2.64239f,.07351f,
	    2.65933f,.07088f,2.67625f,.06811f,2.69317f,.06526f,2.71007f,
	    .06235f,2.72698f,.05943f,2.74389f,.05653f,2.78356f,.04995f,
	    2.82326f,.04366f,2.86298f,.03763f,2.90274f,.03183f,2.94251f,
	    .02623f,2.98231f,.02079f,3.02211f,.01548f,3.06193f,.01026f,
	    3.10176f,.00511f,3.14159f,0.f };

    static integer j;
    static real phild[251], epsld[251];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --eps;
    --phi;

    /* Function Body */
    for (j = 1; j <= 251; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	epsld[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	phi[j] = (real) (j - 1) * 3.141592654f / 200.f;
/* L201: */
	lcsfit_(&c__251, phild, epsld, &c_true, "B", &c__201, &phi[1], &eps[1]
		, &eps[1], (ftnlen)1);
    }
    return 0;
} /* phep67_ */

/* Subroutine */ int phps63_(real *phi, real *psi)
{
    /* Initialized data */

    static real a[402] = { 0.f,.15066f,.01571f,.15058f,.03142f,.15035f,
	    .04713f,.14999f,.06284f,.1495f,.07855f,.14891f,.09426f,.14823f,
	    .10997f,.14748f,.12567f,.14668f,.14138f,.14583f,.15708f,.14497f,
	    .17279f,.1441f,.1885f,.14323f,.2042f,.14238f,.21991f,.14155f,
	    .23561f,.14074f,.25132f,.13998f,.26703f,.13927f,.28274f,.13862f,
	    .29845f,.13804f,.31416f,.13753f,.32986f,.13711f,.34556f,.13676f,
	    .36127f,.13648f,.37698f,.13627f,.39268f,.1361f,.40839f,.13598f,
	    .42411f,.1359f,.43982f,.13584f,.45553f,.13579f,.47124f,.13576f,
	    .48695f,.13573f,.50266f,.1357f,.51837f,.13567f,.53408f,.13564f,
	    .54978f,.13561f,.56549f,.13558f,.5812f,.13555f,.59691f,.13552f,
	    .61261f,.1355f,.62832f,.13547f,.64403f,.13544f,.65974f,.13542f,
	    .67544f,.13539f,.69115f,.13536f,.70686f,.13533f,.72257f,.13529f,
	    .73828f,.13525f,.75398f,.13521f,.76969f,.13516f,.7854f,.13511f,
	    .80111f,.13505f,.81682f,.13499f,.83252f,.13491f,.84823f,.13483f,
	    .86394f,.13475f,.87965f,.13465f,.89536f,.13454f,.91106f,.13442f,
	    .92677f,.13428f,.94248f,.13414f,.95819f,.13398f,.9739f,.13381f,
	    .98961f,.13363f,1.00531f,.13343f,1.02102f,.13321f,1.03673f,
	    .13299f,1.05244f,.13275f,1.06815f,.13249f,1.08385f,.13222f,
	    1.09956f,.13194f,1.11527f,.13164f,1.13098f,.13133f,1.14669f,.131f,
	    1.1624f,.13065f,1.17811f,.13028f,1.19381f,.12988f,1.20952f,
	    .12947f,1.22523f,.12903f,1.24093f,.12857f,1.25664f,.12808f,
	    1.27235f,.12756f,1.28807f,.12702f,1.30378f,.12644f,1.31949f,
	    .12584f,1.3352f,.12521f,1.3509f,.12455f,1.36661f,.12385f,1.38231f,
	    .12313f,1.39802f,.12238f,1.41372f,.1216f,1.42944f,.12079f,
	    1.44515f,.11994f,1.46086f,.11907f,1.47657f,.11817f,1.49228f,
	    .11724f,1.50799f,.11628f,1.52369f,.11529f,1.5394f,.11428f,1.5551f,
	    .11324f,1.5708f,.11218f,1.58652f,.11109f,1.60223f,.10998f,
	    1.61794f,.10884f,1.63365f,.10768f,1.64936f,.1065f,1.66507f,.1053f,
	    1.68077f,.10407f,1.69648f,.10283f,1.71218f,.10157f,1.72788f,
	    .10029f,1.74359f,.09899f,1.75931f,.09767f,1.77502f,.09634f,
	    1.79073f,.09499f,1.80644f,.09363f,1.82214f,.09224f,1.83785f,
	    .09085f,1.85355f,.08944f,1.86926f,.08801f,1.88496f,.08657f,
	    1.90067f,.08512f,1.91638f,.08365f,1.93209f,.08217f,1.9478f,
	    .08068f,1.96351f,.07917f,1.97922f,.07766f,1.99493f,.07614f,
	    2.01063f,.07461f,2.02634f,.07307f,2.04204f,.07153f,2.05775f,
	    .06998f,2.07346f,.06842f,2.08916f,.06687f,2.10487f,.0653f,
	    2.12058f,.06374f,2.13628f,.06217f,2.15199f,.0606f,2.1677f,.05904f,
	    2.1834f,.05747f,2.19911f,.05591f,2.21481f,.05435f,2.23052f,.0528f,
	    2.24623f,.05125f,2.26193f,.0497f,2.27764f,.04817f,2.29335f,
	    .04664f,2.30906f,.04512f,2.32477f,.04362f,2.34048f,.04213f,
	    2.35619f,.04065f,2.37189f,.03919f,2.38759f,.03774f,2.4033f,
	    .03631f,2.419f,.0349f,2.43471f,.0335f,2.45042f,.03213f,2.46613f,
	    .03077f,2.48184f,.02943f,2.49755f,.02811f,2.51327f,.02682f,
	    2.52897f,.02555f,2.54467f,.0243f,2.56037f,.02308f,2.57608f,
	    .02188f,2.59179f,.02071f,2.60749f,.01956f,2.62321f,.01844f,
	    2.63892f,.01735f,2.65463f,.0163f,2.67035f,.01527f,2.68605f,
	    .01428f,2.70175f,.01331f,2.71745f,.01239f,2.73316f,.01149f,
	    2.74887f,.01062f,2.76458f,.00979f,2.78029f,.00899f,2.796f,.00823f,
	    2.81171f,.0075f,2.82743f,.0068f,2.84313f,.00614f,2.85884f,.00551f,
	    2.87454f,.00491f,2.89025f,.00435f,2.90596f,.00382f,2.92167f,
	    .00332f,2.93737f,.00286f,2.95309f,.00244f,2.9688f,.00205f,
	    2.98451f,.00169f,3.00022f,.00137f,3.01592f,.00108f,3.03163f,
	    8.3e-4f,3.04734f,6.1e-4f,3.06305f,4.2e-4f,3.07875f,2.7e-4f,
	    3.09446f,1.5e-4f,3.11017f,7e-5f,3.12588f,2e-5f,3.14159f,0.f };

    static integer j;
    static real phild[201], psild[201];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --psi;
    --phi;

    /* Function Body */
    for (j = 1; j <= 201; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	psild[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	lcsfit_(&c__201, phild, psild, &c_true, "B", &c__201, &phi[1], &psi[1]
		, &psi[1], (ftnlen)1);
/* L202: */
    }
    return 0;
} /* phps63_ */

/* Subroutine */ int phps63a_(real *phi, real *psi)
{
    /* Initialized data */

    static real a[502] = { 0.f,.15517f,.01461f,.15507f,.02922f,.15478f,
	    .04383f,.15431f,.05843f,.15368f,.07303f,.1529f,.08763f,.15199f,
	    .10222f,.15097f,.1168f,.14984f,.13138f,.14863f,.14594f,.14735f,
	    .14925f,.14705f,.15257f,.14675f,.15588f,.14645f,.15919f,.14615f,
	    .1625f,.14585f,.16582f,.14555f,.16913f,.14525f,.17244f,.14496f,
	    .17576f,.14468f,.17907f,.1444f,.1842f,.14397f,.18933f,.14357f,
	    .19447f,.14318f,.1996f,.14281f,.20474f,.14245f,.20987f,.1421f,
	    .21501f,.14177f,.22015f,.14145f,.22529f,.14115f,.23043f,.14086f,
	    .24001f,.14035f,.24958f,.13987f,.25916f,.13944f,.26875f,.13904f,
	    .27833f,.13867f,.28791f,.13833f,.2975f,.13802f,.30708f,.13773f,
	    .31667f,.13746f,.32626f,.13722f,.3398f,.1369f,.35335f,.13662f,
	    .3669f,.13637f,.38045f,.13614f,.39401f,.13594f,.40756f,.13577f,
	    .42111f,.13562f,.43466f,.13548f,.44821f,.13537f,.46176f,.13527f,
	    .47241f,.1352f,.48306f,.13514f,.49371f,.13509f,.50436f,.13504f,
	    .51501f,.135f,.52566f,.13496f,.53631f,.13492f,.54696f,.13489f,
	    .55761f,.13486f,.56826f,.13483f,.57741f,.13481f,.58655f,.13478f,
	    .5957f,.13476f,.60485f,.13474f,.61399f,.13471f,.62314f,.13469f,
	    .63229f,.13467f,.64143f,.13464f,.65058f,.13462f,.65973f,.1346f,
	    .67541f,.13456f,.6911f,.13452f,.70679f,.13447f,.72248f,.13442f,
	    .73817f,.13438f,.75386f,.13432f,.76955f,.13427f,.78524f,.1342f,
	    .80093f,.13414f,.81662f,.13406f,.83032f,.134f,.84402f,.13392f,
	    .85772f,.13384f,.87142f,.13376f,.88512f,.13367f,.89882f,.13357f,
	    .91253f,.13347f,.92623f,.13336f,.93993f,.13324f,.95363f,.13312f,
	    .96612f,.13299f,.9786f,.13287f,.99109f,.13274f,1.00358f,.1326f,
	    1.01607f,.13245f,1.02855f,.1323f,1.04104f,.13214f,1.05353f,
	    .13197f,1.06602f,.1318f,1.0785f,.13162f,1.09023f,.13145f,1.10196f,
	    .13127f,1.11369f,.13108f,1.12541f,.13089f,1.13714f,.13068f,
	    1.14887f,.13047f,1.16059f,.13024f,1.17232f,.13001f,1.18405f,
	    .12976f,1.19577f,.1295f,1.20699f,.12923f,1.21821f,.12896f,
	    1.22943f,.12867f,1.24064f,.12837f,1.25186f,.12806f,1.26308f,
	    .12774f,1.27429f,.12741f,1.28551f,.12706f,1.29672f,.1267f,
	    1.30794f,.12634f,1.31876f,.12597f,1.32959f,.1256f,1.34041f,
	    .12521f,1.35124f,.12481f,1.36206f,.1244f,1.37289f,.12398f,
	    1.38371f,.12355f,1.39453f,.1231f,1.40535f,.12264f,1.41617f,
	    .12217f,1.42675f,.1217f,1.43733f,.12121f,1.4479f,.12072f,1.45847f,
	    .12021f,1.46905f,.11969f,1.47962f,.11916f,1.49019f,.11862f,
	    1.50077f,.11807f,1.51134f,.1175f,1.52191f,.11693f,1.53232f,
	    .11636f,1.54274f,.11577f,1.55316f,.11518f,1.56357f,.11458f,
	    1.57399f,.11397f,1.5844f,.11335f,1.59481f,.11272f,1.60523f,
	    .11208f,1.61564f,.11143f,1.62605f,.11078f,1.63637f,.11012f,
	    1.6467f,.10945f,1.65702f,.10878f,1.66734f,.1081f,1.67766f,.10741f,
	    1.68798f,.10671f,1.6983f,.10601f,1.70862f,.1053f,1.71894f,.10458f,
	    1.72926f,.10385f,1.73961f,.10312f,1.74996f,.10238f,1.76031f,
	    .10164f,1.77067f,.10089f,1.78101f,.10013f,1.79136f,.09936f,
	    1.80171f,.0986f,1.81206f,.09782f,1.82241f,.09704f,1.83276f,
	    .09625f,1.84322f,.09545f,1.85369f,.09465f,1.86415f,.09384f,
	    1.87461f,.09302f,1.88508f,.0922f,1.89554f,.09137f,1.906f,.09054f,
	    1.91646f,.08971f,1.92692f,.08887f,1.93738f,.08802f,1.94807f,
	    .08716f,1.95875f,.08629f,1.96943f,.08541f,1.98012f,.08454f,
	    1.9908f,.08366f,2.00148f,.08278f,2.01216f,.08189f,2.02284f,.081f,
	    2.03353f,.08012f,2.04421f,.07923f,2.05529f,.0783f,2.06637f,
	    .07738f,2.07744f,.07646f,2.08852f,.07553f,2.0996f,.07461f,
	    2.11068f,.07369f,2.12176f,.07278f,2.13284f,.07186f,2.14392f,
	    .07095f,2.155f,.07005f,2.16671f,.0691f,2.17841f,.06815f,2.19012f,
	    .06721f,2.20183f,.06628f,2.21354f,.06535f,2.22525f,.06443f,
	    2.23696f,.06351f,2.24867f,.06259f,2.26038f,.06168f,2.27209f,
	    .06078f,2.28483f,.0598f,2.29757f,.05882f,2.31031f,.05785f,
	    2.32305f,.05688f,2.33579f,.05593f,2.34854f,.05497f,2.36128f,
	    .05403f,2.37402f,.05309f,2.38676f,.05216f,2.39951f,.05123f,
	    2.41401f,.05019f,2.42851f,.04916f,2.44302f,.04813f,2.45752f,
	    .04711f,2.47203f,.04608f,2.48653f,.04506f,2.50103f,.04403f,
	    2.51554f,.04299f,2.53004f,.04195f,2.54455f,.0409f,2.56241f,
	    .03958f,2.58028f,.03826f,2.59814f,.03694f,2.61601f,.03563f,
	    2.63388f,.03435f,2.65175f,.03311f,2.66962f,.03191f,2.6875f,
	    .03077f,2.70537f,.0297f,2.72325f,.02871f,2.76503f,.02675f,
	    2.80682f,.02524f,2.84864f,.02412f,2.89046f,.02333f,2.9323f,
	    .02282f,2.97414f,.02253f,3.016f,.0224f,3.05786f,.02238f,3.09973f,
	    .02241f,3.14159f,.02243f };

    static integer j;
    static real phild[251], psild[251];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --psi;
    --phi;

    /* Function Body */
    for (j = 1; j <= 251; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	psild[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	lcsfit_(&c__251, phild, psild, &c_true, "B", &c__201, &phi[1], &psi[1]
		, &psi[1], (ftnlen)1);
/* L202: */
    }
    return 0;
} /* phps63a_ */

/* Subroutine */ int phps64_(real *phi, real *psi)
{
    /* Initialized data */

    static real a[402] = { 0.f,.25269f,.01573f,.25265f,.03145f,.25251f,
	    .04717f,.25227f,.06289f,.25193f,.07861f,.25147f,.09432f,.2509f,
	    .11002f,.2502f,.12572f,.24937f,.1414f,.24841f,.15708f,.2473f,
	    .1728f,.24605f,.18851f,.24467f,.20422f,.24321f,.21992f,.2417f,
	    .23562f,.24016f,.25132f,.23864f,.26702f,.23715f,.28272f,.23573f,
	    .29844f,.23442f,.31416f,.23325f,.32984f,.23224f,.34553f,.23138f,
	    .36122f,.23066f,.37693f,.23006f,.39264f,.22956f,.40836f,.22916f,
	    .42408f,.22884f,.4398f,.22858f,.45552f,.22836f,.47124f,.22818f,
	    .48695f,.22802f,.50266f,.22788f,.51837f,.22775f,.53408f,.22764f,
	    .54979f,.22755f,.56549f,.22747f,.5812f,.2274f,.59691f,.22736f,
	    .61261f,.22732f,.62832f,.2273f,.64403f,.22729f,.65973f,.2273f,
	    .67544f,.22731f,.69115f,.22733f,.70686f,.22736f,.72257f,.22739f,
	    .73827f,.22742f,.75398f,.22745f,.76969f,.22748f,.7854f,.22751f,
	    .80111f,.22753f,.81682f,.22755f,.83252f,.22756f,.84823f,.22756f,
	    .86394f,.22755f,.87965f,.22753f,.89536f,.22751f,.91106f,.22747f,
	    .92677f,.22742f,.94248f,.22736f,.95819f,.22729f,.9739f,.2272f,
	    .98961f,.22709f,1.00531f,.22697f,1.02102f,.22683f,1.03673f,
	    .22668f,1.05244f,.2265f,1.06815f,.2263f,1.08385f,.22608f,1.09956f,
	    .22584f,1.11527f,.22557f,1.13098f,.22528f,1.14669f,.22497f,
	    1.1624f,.22462f,1.17811f,.22426f,1.19381f,.22386f,1.20952f,
	    .22345f,1.22523f,.223f,1.24093f,.22253f,1.25664f,.22203f,1.27236f,
	    .2215f,1.28808f,.22094f,1.30379f,.22034f,1.31951f,.21969f,
	    1.33522f,.21899f,1.35093f,.21823f,1.36663f,.21741f,1.38233f,
	    .21652f,1.39803f,.21554f,1.41372f,.21449f,1.42946f,.21334f,
	    1.44519f,.21211f,1.46091f,.2108f,1.47663f,.2094f,1.49234f,.20794f,
	    1.50804f,.20641f,1.52374f,.20481f,1.53943f,.20315f,1.55512f,
	    .20143f,1.5708f,.19966f,1.58653f,.19784f,1.60226f,.19596f,
	    1.61798f,.19405f,1.6337f,.19209f,1.64941f,.19008f,1.66511f,
	    .18804f,1.68081f,.18596f,1.69651f,.18384f,1.7122f,.18169f,
	    1.72788f,.1795f,1.74361f,.17727f,1.75933f,.17501f,1.77505f,
	    .17273f,1.79076f,.17041f,1.80647f,.16806f,1.82218f,.16569f,
	    1.83788f,.16329f,1.85357f,.16087f,1.86927f,.15843f,1.88496f,
	    .15596f,1.90068f,.15347f,1.9164f,.15095f,1.93211f,.14842f,
	    1.94782f,.14587f,1.96353f,.1433f,1.97924f,.14072f,1.99494f,
	    .13812f,2.01064f,.13551f,2.02634f,.13288f,2.04204f,.13024f,
	    2.05775f,.12759f,2.07346f,.12492f,2.08917f,.12225f,2.10488f,
	    .11957f,2.12059f,.11688f,2.13629f,.11418f,2.152f,.11149f,2.1677f,
	    .10878f,2.18341f,.10608f,2.19911f,.10338f,2.21481f,.10068f,
	    2.23051f,.09798f,2.24622f,.09529f,2.26192f,.0926f,2.27763f,
	    .08992f,2.29334f,.08725f,2.30905f,.08459f,2.32476f,.08195f,
	    2.34047f,.07932f,2.35619f,.07671f,2.37188f,.07412f,2.38757f,
	    .07156f,2.40327f,.06901f,2.41897f,.06649f,2.43468f,.06399f,
	    2.45039f,.06152f,2.4661f,.05908f,2.48182f,.05667f,2.49754f,
	    .05428f,2.51327f,.05193f,2.52895f,.04962f,2.54464f,.04734f,
	    2.56034f,.0451f,2.57604f,.04289f,2.59174f,.04073f,2.60745f,.0386f,
	    2.62317f,.03651f,2.63889f,.03447f,2.65462f,.03247f,2.67035f,
	    .03051f,2.68603f,.02861f,2.70172f,.02675f,2.71741f,.02494f,
	    2.73311f,.02318f,2.74882f,.02148f,2.76453f,.01983f,2.78024f,
	    .01825f,2.79597f,.01672f,2.8117f,.01525f,2.82743f,.01385f,
	    2.84312f,.01252f,2.85881f,.01125f,2.87451f,.01006f,2.89021f,
	    .00892f,2.90591f,.00786f,2.92163f,.00686f,2.93734f,.00593f,
	    2.95306f,.00506f,2.96878f,.00426f,2.98451f,.00353f,3.00021f,
	    .00287f,3.01591f,.00227f,3.03162f,.00174f,3.04732f,.00128f,
	    3.06303f,8.9e-4f,3.07874f,5.7e-4f,3.09445f,3.2e-4f,3.11016f,
	    1.4e-4f,3.12588f,4e-5f,3.14159f,0.f };

    static integer j;
    static real phild[201], psild[201];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --psi;
    --phi;

    /* Function Body */
    for (j = 1; j <= 201; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	psild[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	lcsfit_(&c__201, phild, psild, &c_true, "B", &c__201, &phi[1], &psi[1]
		, &psi[1], (ftnlen)1);
/* L202: */
    }
    return 0;
} /* phps64_ */

/* Subroutine */ int phps64a_(real *phi, real *psi)
{
    /* Initialized data */

    static real a[502] = { 0.f,.15365f,.0147f,.15355f,.0294f,.15324f,.0441f,
	    .15275f,.0588f,.15209f,.07349f,.15129f,.08818f,.15035f,.10286f,
	    .1493f,.11753f,.14816f,.1322f,.14693f,.14686f,.14565f,.15019f,
	    .14535f,.15352f,.14505f,.15685f,.14475f,.16019f,.14444f,.16352f,
	    .14414f,.16685f,.14384f,.17018f,.14353f,.17351f,.14323f,.17684f,
	    .14293f,.18017f,.14263f,.18528f,.14218f,.1904f,.14174f,.19551f,
	    .1413f,.20063f,.14087f,.20574f,.14044f,.21086f,.14003f,.21598f,
	    .13963f,.22109f,.13924f,.22621f,.13886f,.23133f,.13849f,.24088f,
	    .13783f,.25044f,.13722f,.26f,.13665f,.26956f,.13612f,.27912f,
	    .13562f,.28869f,.13516f,.29825f,.13473f,.30782f,.13432f,.31739f,
	    .13395f,.32695f,.13359f,.34039f,.13313f,.35384f,.13271f,.36728f,
	    .13233f,.38072f,.13199f,.39416f,.13167f,.40761f,.13139f,.42105f,
	    .13113f,.4345f,.1309f,.44794f,.13069f,.46139f,.13051f,.47194f,
	    .13037f,.48249f,.13024f,.49303f,.13013f,.50358f,.13003f,.51413f,
	    .12993f,.52468f,.12985f,.53523f,.12977f,.54578f,.12971f,.55633f,
	    .12966f,.56688f,.12961f,.57595f,.12958f,.58502f,.12955f,.59409f,
	    .12953f,.60317f,.12952f,.61224f,.12951f,.62131f,.1295f,.63038f,
	    .12949f,.63945f,.12949f,.64852f,.12949f,.65759f,.12949f,.67316f,
	    .12948f,.68874f,.12947f,.70431f,.12947f,.71989f,.12946f,.73546f,
	    .12945f,.75103f,.12944f,.76661f,.12943f,.78218f,.12942f,.79776f,
	    .12941f,.81333f,.1294f,.82695f,.12939f,.84057f,.12939f,.85418f,
	    .12938f,.8678f,.12937f,.88142f,.12936f,.89504f,.12934f,.90866f,
	    .12932f,.92228f,.1293f,.93589f,.12927f,.94951f,.12923f,.96194f,
	    .1292f,.97436f,.12915f,.98679f,.1291f,.99922f,.12905f,1.01164f,
	    .12899f,1.02407f,.12892f,1.0365f,.12885f,1.04892f,.12878f,
	    1.06135f,.12869f,1.07377f,.12861f,1.08546f,.12852f,1.09714f,
	    .12843f,1.10882f,.12833f,1.1205f,.12822f,1.13218f,.12811f,
	    1.14386f,.128f,1.15554f,.12788f,1.16723f,.12775f,1.17891f,.12761f,
	    1.19059f,.12747f,1.2018f,.12733f,1.21301f,.12718f,1.22422f,
	    .12702f,1.23543f,.12685f,1.24665f,.12668f,1.25786f,.1265f,
	    1.26907f,.12631f,1.28028f,.12611f,1.29149f,.1259f,1.3027f,.12569f,
	    1.31357f,.12547f,1.32444f,.12524f,1.33531f,.125f,1.34618f,.12475f,
	    1.35705f,.12448f,1.36792f,.1242f,1.37879f,.1239f,1.38966f,.12358f,
	    1.40052f,.12325f,1.41139f,.12289f,1.42204f,.12252f,1.43269f,
	    .12212f,1.44334f,.12171f,1.45398f,.12128f,1.46463f,.12083f,
	    1.47528f,.12036f,1.48592f,.11988f,1.49657f,.11938f,1.50721f,
	    .11887f,1.51785f,.11834f,1.52835f,.11781f,1.53884f,.11727f,
	    1.54933f,.11672f,1.55982f,.11615f,1.5703f,.11557f,1.58079f,
	    .11499f,1.59128f,.11439f,1.60177f,.11378f,1.61225f,.11316f,
	    1.62274f,.11253f,1.63312f,.11189f,1.6435f,.11125f,1.65388f,
	    .11059f,1.66426f,.10993f,1.67464f,.10925f,1.68502f,.10857f,
	    1.6954f,.10788f,1.70578f,.10719f,1.71615f,.10648f,1.72653f,
	    .10577f,1.73693f,.10505f,1.74732f,.10433f,1.75772f,.1036f,
	    1.76812f,.10286f,1.77851f,.10212f,1.78891f,.10137f,1.79931f,
	    .10061f,1.8097f,.09985f,1.82009f,.09908f,1.83049f,.09831f,1.841f,
	    .09752f,1.85151f,.09672f,1.86202f,.09592f,1.87253f,.09512f,
	    1.88304f,.09431f,1.89354f,.09349f,1.90405f,.09267f,1.91456f,
	    .09184f,1.92507f,.09101f,1.93557f,.09017f,1.94631f,.08931f,
	    1.95704f,.08845f,1.96777f,.08758f,1.9785f,.0867f,1.98923f,.08583f,
	    1.99996f,.08495f,2.01069f,.08406f,2.02142f,.08318f,2.03215f,
	    .08229f,2.04288f,.0814f,2.05401f,.08048f,2.06514f,.07955f,
	    2.07627f,.07863f,2.0874f,.0777f,2.09852f,.07678f,2.10965f,.07585f,
	    2.12078f,.07493f,2.13191f,.07401f,2.14304f,.07309f,2.15416f,
	    .07217f,2.16592f,.0712f,2.17767f,.07023f,2.18942f,.06927f,
	    2.20117f,.06831f,2.21293f,.06735f,2.22468f,.06639f,2.23643f,
	    .06545f,2.24819f,.0645f,2.25994f,.06356f,2.27169f,.06262f,
	    2.28447f,.06161f,2.29724f,.0606f,2.31001f,.0596f,2.32278f,.05861f,
	    2.33556f,.05762f,2.34833f,.05664f,2.36111f,.05567f,2.37388f,
	    .0547f,2.38666f,.05375f,2.39943f,.05279f,2.41396f,.05172f,
	    2.42849f,.05066f,2.44302f,.0496f,2.45754f,.04855f,2.47207f,
	    .04749f,2.4866f,.04644f,2.50113f,.04538f,2.51566f,.04432f,
	    2.53019f,.04324f,2.54471f,.04216f,2.5626f,.04081f,2.58049f,
	    .03945f,2.59837f,.03809f,2.61626f,.03675f,2.63415f,.03543f,
	    2.65203f,.03414f,2.66993f,.03291f,2.68782f,.03173f,2.70572f,
	    .03061f,2.72362f,.02958f,2.76535f,.02751f,2.80711f,.02589f,
	    2.84888f,.02465f,2.89067f,.02374f,2.93247f,.02312f,2.97428f,
	    .02273f,3.0161f,.02252f,3.05793f,.02243f,3.09976f,.02242f,
	    3.14159f,.02243f };

    static integer j;
    static real phild[251], psild[251];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --psi;
    --phi;

    /* Function Body */
    for (j = 1; j <= 251; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	psild[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	lcsfit_(&c__251, phild, psild, &c_true, "B", &c__201, &phi[1], &psi[1]
		, &psi[1], (ftnlen)1);
/* L202: */
    }
    return 0;
} /* phps64a_ */

/* Subroutine */ int phps65_(real *phi, real *psi)
{
    /* Initialized data */

    static real a[502] = { 0.f,.17464f,.01486f,.17457f,.02972f,.17437f,
	    .04458f,.17404f,.05944f,.1736f,.07429f,.17303f,.08914f,.17236f,
	    .10398f,.17159f,.11882f,.17073f,.13365f,.16977f,.14848f,.16872f,
	    .15176f,.16848f,.15503f,.16824f,.15831f,.16799f,.16159f,.16773f,
	    .16486f,.16747f,.16814f,.16721f,.17141f,.16694f,.17469f,.16666f,
	    .17796f,.16638f,.18123f,.16609f,.18624f,.16563f,.19124f,.16516f,
	    .19624f,.16468f,.20124f,.1642f,.20624f,.16371f,.21124f,.16321f,
	    .21625f,.16272f,.22125f,.16223f,.22625f,.16175f,.23125f,.16127f,
	    .2407f,.16041f,.25015f,.15959f,.2596f,.15881f,.26906f,.15808f,
	    .27852f,.1574f,.28798f,.15676f,.29744f,.15616f,.30691f,.15562f,
	    .31638f,.15512f,.32586f,.15468f,.33917f,.15413f,.35249f,.15368f,
	    .36581f,.15331f,.37913f,.15301f,.39246f,.15278f,.40578f,.15261f,
	    .41911f,.15249f,.43244f,.15241f,.44577f,.15237f,.4591f,.15235f,
	    .46962f,.15235f,.48015f,.15236f,.49068f,.15238f,.50121f,.15241f,
	    .51173f,.15245f,.52226f,.15251f,.53279f,.15257f,.54331f,.15264f,
	    .55384f,.15272f,.56436f,.15281f,.57344f,.15289f,.58252f,.15298f,
	    .5916f,.15308f,.60068f,.15318f,.60976f,.15328f,.61884f,.15339f,
	    .62791f,.1535f,.63699f,.15361f,.64607f,.15372f,.65515f,.15383f,
	    .67085f,.15402f,.68656f,.15422f,.70226f,.1544f,.71796f,.15459f,
	    .73366f,.15477f,.74937f,.15496f,.76507f,.15513f,.78077f,.15531f,
	    .79648f,.15548f,.81218f,.15564f,.8259f,.15578f,.83961f,.15592f,
	    .85333f,.15605f,.86704f,.15618f,.88076f,.1563f,.89448f,.15642f,
	    .90819f,.15653f,.92191f,.15663f,.93563f,.15673f,.94934f,.15682f,
	    .96192f,.1569f,.97449f,.15697f,.98706f,.15704f,.99963f,.15709f,
	    1.0122f,.15714f,1.02477f,.15719f,1.03734f,.15723f,1.04992f,
	    .15726f,1.06249f,.15728f,1.07506f,.1573f,1.08689f,.15731f,
	    1.09872f,.15731f,1.11054f,.1573f,1.12237f,.15729f,1.1342f,.15727f,
	    1.14603f,.15724f,1.15786f,.15721f,1.16968f,.15716f,1.18151f,
	    .15711f,1.19334f,.15704f,1.20469f,.15697f,1.21604f,.15689f,
	    1.2274f,.1568f,1.23875f,.15671f,1.2501f,.1566f,1.26145f,.15648f,
	    1.27281f,.15636f,1.28416f,.15623f,1.29551f,.15609f,1.30686f,
	    .15594f,1.31793f,.15578f,1.329f,.15562f,1.34007f,.15544f,1.35114f,
	    .15526f,1.36221f,.15506f,1.37327f,.15484f,1.38434f,.15462f,
	    1.39541f,.15437f,1.40647f,.15411f,1.41754f,.15383f,1.42845f,
	    .15354f,1.43935f,.15322f,1.45026f,.15289f,1.46116f,.15254f,
	    1.47207f,.15216f,1.48297f,.15177f,1.49387f,.15135f,1.50477f,
	    .15091f,1.51567f,.15046f,1.52657f,.14998f,1.53737f,.14948f,
	    1.54817f,.14896f,1.55897f,.14842f,1.56977f,.14785f,1.58057f,
	    .14726f,1.59136f,.14665f,1.60215f,.14602f,1.61295f,.14536f,
	    1.62374f,.14467f,1.63452f,.14396f,1.64526f,.14323f,1.656f,.14247f,
	    1.66673f,.14169f,1.67746f,.14088f,1.6882f,.14005f,1.69892f,.1392f,
	    1.70965f,.13833f,1.72038f,.13744f,1.7311f,.13652f,1.74182f,
	    .13559f,1.75255f,.13464f,1.76327f,.13367f,1.77399f,.13268f,
	    1.78471f,.13167f,1.79542f,.13065f,1.80614f,.12961f,1.81685f,
	    .12856f,1.82756f,.12749f,1.83828f,.12641f,1.84898f,.12531f,
	    1.85973f,.1242f,1.87048f,.12307f,1.88122f,.12194f,1.89197f,
	    .12079f,1.90271f,.11962f,1.91345f,.11845f,1.92419f,.11726f,
	    1.93493f,.11606f,1.94567f,.11485f,1.9564f,.11363f,1.96732f,
	    .11237f,1.97824f,.11111f,1.98916f,.10983f,2.00008f,.10854f,2.011f,
	    .10725f,2.02191f,.10594f,2.03283f,.10463f,2.04374f,.10332f,
	    2.05466f,.10199f,2.06557f,.10066f,2.07682f,.09929f,2.08807f,
	    .09791f,2.09932f,.09652f,2.11056f,.09513f,2.12181f,.09374f,
	    2.13305f,.09234f,2.1443f,.09094f,2.15554f,.08953f,2.16679f,
	    .08811f,2.17803f,.08669f,2.18979f,.0852f,2.20154f,.0837f,2.2133f,
	    .0822f,2.22505f,.0807f,2.2368f,.07919f,2.24856f,.07768f,2.26031f,
	    .07617f,2.27206f,.07465f,2.28382f,.07314f,2.29557f,.07163f,
	    2.30822f,.07f,2.32086f,.06838f,2.33351f,.06675f,2.34616f,.06514f,
	    2.35881f,.06352f,2.37146f,.06191f,2.38411f,.0603f,2.39676f,.0587f,
	    2.40941f,.0571f,2.42206f,.05551f,2.43614f,.05374f,2.45023f,
	    .05199f,2.46431f,.05024f,2.47839f,.0485f,2.49248f,.04677f,
	    2.50656f,.04504f,2.52065f,.04333f,2.53474f,.04162f,2.54883f,
	    .03992f,2.56292f,.03824f,2.57971f,.03624f,2.59651f,.03426f,
	    2.6133f,.0323f,2.6301f,.03037f,2.64691f,.02848f,2.66372f,.02662f,
	    2.68053f,.02481f,2.69735f,.02305f,2.71417f,.02134f,2.731f,.0197f,
	    2.77194f,.01597f,2.81292f,.01263f,2.85393f,.00968f,2.89497f,
	    .00712f,2.93604f,.00495f,2.97712f,.00317f,3.01823f,.00178f,
	    3.05934f,7.9e-4f,3.10046f,2e-4f,3.14159f,0.f };

    static integer j;
    static real phild[251], psild[251];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --psi;
    --phi;

    /* Function Body */
    for (j = 1; j <= 251; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	psild[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	lcsfit_(&c__251, phild, psild, &c_true, "B", &c__201, &phi[1], &psi[1]
		, &psi[1], (ftnlen)1);
/* L202: */
    }
    return 0;
} /* phps65_ */

/* Subroutine */ int phps65a_(real *phi, real *psi)
{
    /* Initialized data */

    static real a[502] = { 0.f,.14761f,.01455f,.14748f,.0291f,.14709f,.04364f,
	    .14649f,.05819f,.1457f,.07273f,.14476f,.08727f,.14369f,.10181f,
	    .14253f,.11634f,.14131f,.13087f,.14007f,.14539f,.13882f,.14877f,
	    .13854f,.15214f,.13826f,.15552f,.13798f,.1589f,.1377f,.16227f,
	    .13743f,.16565f,.13716f,.16903f,.1369f,.1724f,.13665f,.17578f,
	    .1364f,.17916f,.13616f,.18436f,.13581f,.18956f,.13547f,.19477f,
	    .13515f,.19997f,.13484f,.20518f,.13455f,.21039f,.13426f,.2156f,
	    .13398f,.2208f,.13371f,.22601f,.13345f,.23122f,.13318f,.24077f,
	    .1327f,.25032f,.13221f,.25986f,.13172f,.26941f,.13124f,.27896f,
	    .13075f,.2885f,.13026f,.29805f,.12976f,.3076f,.12926f,.31714f,
	    .12876f,.32669f,.12825f,.33995f,.12753f,.35322f,.12681f,.36648f,
	    .1261f,.37974f,.12541f,.39301f,.12475f,.40628f,.12415f,.41955f,
	    .12359f,.43282f,.12311f,.44609f,.1227f,.45937f,.12239f,.46975f,
	    .12221f,.48014f,.12209f,.49053f,.12202f,.50092f,.12199f,.51131f,
	    .12199f,.5217f,.12203f,.5321f,.12209f,.54249f,.12216f,.55288f,
	    .12224f,.56327f,.12233f,.57223f,.1224f,.58118f,.12246f,.59014f,
	    .12253f,.59909f,.12259f,.60805f,.12265f,.617f,.12271f,.62596f,
	    .12277f,.63491f,.12283f,.64387f,.12289f,.65282f,.12295f,.66829f,
	    .12305f,.68376f,.12316f,.69923f,.12326f,.7147f,.12337f,.73016f,
	    .12348f,.74563f,.12359f,.7611f,.1237f,.77657f,.12381f,.79204f,
	    .12391f,.8075f,.12401f,.82106f,.1241f,.83461f,.12418f,.84816f,
	    .12426f,.86171f,.12434f,.87527f,.12441f,.88882f,.12448f,.90237f,
	    .12455f,.91592f,.12461f,.92948f,.12467f,.94303f,.12472f,.95538f,
	    .12477f,.96772f,.12482f,.98007f,.12486f,.99241f,.12489f,1.00476f,
	    .12492f,1.01711f,.12495f,1.02945f,.12498f,1.0418f,.125f,1.05414f,
	    .12501f,1.06649f,.12502f,1.0781f,.12503f,1.08971f,.12503f,
	    1.10132f,.12502f,1.11293f,.12501f,1.12454f,.125f,1.13615f,.12498f,
	    1.14776f,.12495f,1.15937f,.12492f,1.17098f,.12489f,1.18259f,
	    .12485f,1.19372f,.12481f,1.20486f,.12476f,1.21599f,.12471f,
	    1.22713f,.12465f,1.23827f,.12459f,1.2494f,.12452f,1.26054f,
	    .12445f,1.27167f,.12437f,1.28281f,.12428f,1.29394f,.12419f,
	    1.30473f,.12409f,1.31552f,.12398f,1.32631f,.12387f,1.3371f,
	    .12375f,1.3479f,.12362f,1.35869f,.12349f,1.36948f,.12334f,
	    1.38027f,.12319f,1.39106f,.12303f,1.40185f,.12286f,1.41248f,
	    .12268f,1.42311f,.12249f,1.43375f,.12229f,1.44438f,.12208f,
	    1.45501f,.12186f,1.46564f,.12163f,1.47628f,.12138f,1.48691f,
	    .12113f,1.49754f,.12085f,1.50817f,.12057f,1.51872f,.12027f,
	    1.52928f,.11996f,1.53983f,.11964f,1.55038f,.1193f,1.56094f,
	    .11894f,1.57149f,.11857f,1.58204f,.11818f,1.59259f,.11778f,
	    1.60314f,.11736f,1.61369f,.11693f,1.62418f,.11648f,1.63468f,
	    .11602f,1.64517f,.11554f,1.65566f,.11504f,1.66616f,.11453f,
	    1.67665f,.114f,1.68714f,.11346f,1.69763f,.11291f,1.70811f,.11234f,
	    1.7186f,.11175f,1.72914f,.11115f,1.73968f,.11054f,1.75021f,
	    .10991f,1.76075f,.10928f,1.77128f,.10863f,1.78182f,.10797f,
	    1.79235f,.1073f,1.80289f,.10662f,1.81342f,.10594f,1.82395f,
	    .10524f,1.83461f,.10453f,1.84527f,.10381f,1.85592f,.10308f,
	    1.86658f,.10235f,1.87723f,.10161f,1.88789f,.10086f,1.89854f,
	    .1001f,1.9092f,.09933f,1.91985f,.09856f,1.9305f,.09778f,1.94138f,
	    .09698f,1.95226f,.09617f,1.96314f,.09535f,1.97402f,.09452f,
	    1.98489f,.09369f,1.99577f,.09285f,2.00664f,.09201f,2.01752f,
	    .09116f,2.02839f,.0903f,2.03927f,.08943f,2.05056f,.08853f,
	    2.06185f,.08763f,2.07314f,.08671f,2.08444f,.08579f,2.09573f,
	    .08487f,2.10702f,.08394f,2.11831f,.08301f,2.1296f,.08207f,
	    2.14089f,.08113f,2.15217f,.08019f,2.16409f,.07919f,2.17601f,
	    .07818f,2.18793f,.07717f,2.19985f,.07616f,2.21176f,.07515f,
	    2.22368f,.07413f,2.2356f,.07311f,2.24751f,.07209f,2.25943f,
	    .07106f,2.27134f,.07004f,2.28424f,.06892f,2.29713f,.06781f,
	    2.31002f,.0667f,2.32292f,.06559f,2.33581f,.06448f,2.3487f,.06338f,
	    2.36159f,.06229f,2.37449f,.06119f,2.38738f,.06011f,2.40028f,
	    .05904f,2.41489f,.05783f,2.4295f,.05664f,2.44411f,.05545f,
	    2.45872f,.05427f,2.47334f,.05309f,2.48795f,.05191f,2.50256f,
	    .05072f,2.51717f,.04953f,2.53178f,.04834f,2.5464f,.04713f,
	    2.56432f,.04564f,2.58225f,.04413f,2.60018f,.04263f,2.6181f,
	    .04114f,2.63603f,.03967f,2.65396f,.03823f,2.67189f,.03683f,
	    2.68983f,.03549f,2.70777f,.03421f,2.72571f,.033f,2.76723f,.03052f,
	    2.80877f,.02844f,2.85033f,.02675f,2.8919f,.02539f,2.9335f,.02434f,
	    2.9751f,.02355f,3.01671f,.02301f,3.05834f,.02266f,3.09996f,
	    .02248f,3.14159f,.02242f };

    static integer j;
    static real phild[251], psild[251];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --psi;
    --phi;

    /* Function Body */
    for (j = 1; j <= 251; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	psild[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	lcsfit_(&c__251, phild, psild, &c_true, "B", &c__201, &phi[1], &psi[1]
		, &psi[1], (ftnlen)1);
/* L202: */
    }
    return 0;
} /* phps65a_ */

/* Subroutine */ int phps66_(real *phi, real *psi)
{
    /* Initialized data */

    static real a[402] = { 0.f,.16457f,.01573f,.16455f,.03145f,.16449f,
	    .04718f,.16437f,.0629f,.16416f,.07862f,.16386f,.09433f,.16345f,
	    .11003f,.16292f,.12572f,.16223f,.14141f,.16139f,.15708f,.16037f,
	    .1728f,.15916f,.18851f,.15779f,.20422f,.15631f,.21992f,.15475f,
	    .23562f,.15316f,.25131f,.15157f,.26701f,.15002f,.28272f,.14856f,
	    .29844f,.14722f,.31416f,.14604f,.32984f,.14506f,.34552f,.14427f,
	    .36122f,.14364f,.37693f,.14316f,.39264f,.14281f,.40835f,.14257f,
	    .42407f,.14242f,.4398f,.14235f,.45552f,.14233f,.47124f,.14236f,
	    .48696f,.14241f,.50267f,.14248f,.51838f,.14257f,.53409f,.14267f,
	    .54979f,.1428f,.5655f,.14294f,.58121f,.1431f,.59691f,.14327f,
	    .61262f,.14346f,.62832f,.14366f,.64403f,.14387f,.65973f,.1441f,
	    .67544f,.14433f,.69115f,.14457f,.70686f,.14481f,.72257f,.14506f,
	    .73827f,.1453f,.75398f,.14554f,.76969f,.14578f,.7854f,.14601f,
	    .80111f,.14623f,.81682f,.14645f,.83252f,.14665f,.84823f,.14685f,
	    .86394f,.14704f,.87965f,.14722f,.89536f,.1474f,.91106f,.14757f,
	    .92677f,.14774f,.94248f,.1479f,.95819f,.14806f,.9739f,.14821f,
	    .9896f,.14835f,1.00531f,.14849f,1.02102f,.14862f,1.03673f,.14875f,
	    1.05244f,.14886f,1.06814f,.14897f,1.08385f,.14908f,1.09956f,
	    .14917f,1.11527f,.14925f,1.13098f,.14933f,1.14668f,.1494f,
	    1.16239f,.14945f,1.1781f,.1495f,1.19381f,.14954f,1.20952f,.14957f,
	    1.22522f,.14959f,1.24093f,.14961f,1.25664f,.14961f,1.27235f,
	    .1496f,1.28806f,.14959f,1.30376f,.14956f,1.31947f,.14953f,
	    1.33518f,.14948f,1.35089f,.14943f,1.3666f,.14936f,1.3823f,.14928f,
	    1.39801f,.14918f,1.41372f,.14908f,1.42943f,.14896f,1.44514f,
	    .14883f,1.46085f,.14869f,1.47655f,.14853f,1.49226f,.14835f,
	    1.50797f,.14816f,1.52368f,.14796f,1.53939f,.14774f,1.55509f,
	    .1475f,1.5708f,.14725f,1.58651f,.14698f,1.60222f,.14669f,1.61793f,
	    .14638f,1.63364f,.14606f,1.64934f,.14571f,1.66505f,.14533f,
	    1.68076f,.14494f,1.69647f,.14452f,1.71217f,.14407f,1.72788f,
	    .1436f,1.7436f,.1431f,1.75932f,.14256f,1.77504f,.14198f,1.79075f,
	    .14135f,1.80647f,.14067f,1.82217f,.13992f,1.83788f,.1391f,
	    1.85358f,.1382f,1.86927f,.13722f,1.88496f,.13615f,1.9007f,.13498f,
	    1.91642f,.13371f,1.93214f,.13236f,1.94786f,.13093f,1.96357f,
	    .12942f,1.97927f,.12786f,1.99497f,.12623f,2.01067f,.12456f,
	    2.02636f,.12284f,2.04204f,.12108f,2.05777f,.11929f,2.07349f,
	    .11746f,2.0892f,.11561f,2.10492f,.11373f,2.12062f,.11182f,
	    2.13633f,.10987f,2.15203f,.1079f,2.16773f,.1059f,2.18342f,.10386f,
	    2.19911f,.1018f,2.21483f,.0997f,2.23055f,.09758f,2.24626f,.09542f,
	    2.26197f,.09325f,2.27768f,.09106f,2.29338f,.08885f,2.30909f,
	    .08662f,2.32479f,.08439f,2.34049f,.08214f,2.35619f,.07989f,
	    2.3719f,.07763f,2.38761f,.07537f,2.40331f,.07311f,2.41902f,
	    .07085f,2.43473f,.06859f,2.45043f,.06633f,2.46614f,.06407f,
	    2.48185f,.06182f,2.49756f,.05957f,2.51327f,.05733f,2.52897f,
	    .0551f,2.54467f,.05287f,2.56037f,.05066f,2.57607f,.04846f,
	    2.59178f,.04627f,2.60749f,.04411f,2.6232f,.04196f,2.63891f,
	    .03983f,2.65463f,.03773f,2.67035f,.03566f,2.68604f,.03362f,
	    2.70173f,.03161f,2.71742f,.02964f,2.73312f,.0277f,2.74882f,.0258f,
	    2.76453f,.02395f,2.78025f,.02215f,2.79597f,.02039f,2.8117f,
	    .01869f,2.82743f,.01705f,2.84311f,.01547f,2.8588f,.01396f,
	    2.87449f,.0125f,2.89019f,.01112f,2.9059f,.00981f,2.92161f,.00857f,
	    2.93733f,.0074f,2.95305f,.00631f,2.96878f,.00531f,2.98451f,
	    .00439f,3.0002f,.00356f,3.0159f,.00281f,3.0316f,.00216f,3.04731f,
	    .00159f,3.06302f,.0011f,3.07873f,7.1e-4f,3.09444f,4e-4f,3.11016f,
	    1.8e-4f,3.12587f,4e-5f,3.14159f,0.f };

    static integer j;
    static real phild[201], psild[201];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --psi;
    --phi;

    /* Function Body */
    for (j = 1; j <= 201; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	psild[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	lcsfit_(&c__201, phild, psild, &c_true, "B", &c__201, &phi[1], &psi[1]
		, &psi[1], (ftnlen)1);
/* L202: */
    }
    return 0;
} /* phps66_ */

/* Subroutine */ int phps67_(real *phi, real *psi)
{
    /* Initialized data */

    static real a[502] = { 0.f,.18028f,.01499f,.18026f,.02998f,.18018f,
	    .04497f,.18002f,.05995f,.17976f,.07492f,.17938f,.08988f,.17885f,
	    .10483f,.17815f,.11976f,.17725f,.13468f,.17614f,.14959f,.1748f,
	    .1528f,.17447f,.15601f,.17414f,.15923f,.1738f,.16244f,.17345f,
	    .16565f,.1731f,.16886f,.17276f,.17208f,.17241f,.17529f,.17207f,
	    .1785f,.17174f,.18172f,.17141f,.18686f,.17092f,.19201f,.17045f,
	    .19716f,.17001f,.20231f,.16959f,.20746f,.16918f,.21262f,.16879f,
	    .21777f,.1684f,.22293f,.16802f,.22808f,.16764f,.23323f,.16726f,
	    .24277f,.16653f,.25229f,.16578f,.26182f,.16502f,.27135f,.16426f,
	    .28088f,.16349f,.29041f,.16272f,.29993f,.16196f,.30946f,.16121f,
	    .31899f,.16048f,.32853f,.15978f,.34178f,.15885f,.35505f,.15798f,
	    .36832f,.15717f,.38159f,.15641f,.39486f,.15572f,.40814f,.15507f,
	    .42142f,.15449f,.4347f,.15396f,.44798f,.15348f,.46127f,.15306f,
	    .47164f,.15277f,.482f,.15251f,.49237f,.15228f,.50274f,.15207f,
	    .51311f,.15188f,.52348f,.15171f,.53384f,.15156f,.54421f,.15142f,
	    .55458f,.15129f,.56495f,.15116f,.57385f,.15105f,.58275f,.15095f,
	    .59165f,.15085f,.60055f,.15075f,.60945f,.15066f,.61835f,.15057f,
	    .62725f,.15049f,.63615f,.15041f,.64505f,.15034f,.65395f,.15027f,
	    .66928f,.15016f,.68461f,.15007f,.69994f,.15f,.71527f,.14994f,
	    .7306f,.1499f,.74593f,.14986f,.76125f,.14984f,.77658f,.14982f,
	    .79191f,.14982f,.80724f,.14982f,.82061f,.14982f,.83397f,.14983f,
	    .84734f,.14985f,.8607f,.14986f,.87407f,.14988f,.88743f,.14991f,
	    .9008f,.14993f,.91416f,.14996f,.92753f,.14999f,.94089f,.15002f,
	    .95309f,.15005f,.96529f,.15008f,.97749f,.15011f,.98969f,.15014f,
	    1.00189f,.15018f,1.01409f,.15021f,1.02629f,.15024f,1.03849f,
	    .15028f,1.05069f,.15031f,1.06289f,.15035f,1.07435f,.15038f,
	    1.0858f,.15042f,1.09726f,.15045f,1.10871f,.15048f,1.12017f,
	    .15052f,1.13162f,.15056f,1.14308f,.15059f,1.15453f,.15063f,
	    1.16599f,.15067f,1.17744f,.1507f,1.1884f,.15074f,1.19936f,.15077f,
	    1.21032f,.15081f,1.22129f,.15084f,1.23225f,.15088f,1.24321f,
	    .15091f,1.25417f,.15094f,1.26513f,.15096f,1.27609f,.15099f,
	    1.28706f,.15101f,1.2977f,.15103f,1.30835f,.15104f,1.319f,.15106f,
	    1.32965f,.15107f,1.34029f,.15108f,1.35094f,.15108f,1.36159f,
	    .15109f,1.37224f,.15109f,1.38289f,.1511f,1.39353f,.1511f,1.40401f,
	    .1511f,1.41449f,.1511f,1.42498f,.1511f,1.43546f,.15109f,1.44594f,
	    .15109f,1.45642f,.15108f,1.4669f,.15107f,1.47738f,.15106f,
	    1.48786f,.15104f,1.49834f,.15102f,1.50878f,.151f,1.51921f,.15097f,
	    1.52965f,.15094f,1.54008f,.1509f,1.55052f,.15086f,1.56095f,
	    .15081f,1.57139f,.15076f,1.58182f,.15071f,1.59226f,.15065f,
	    1.60269f,.15059f,1.61319f,.15053f,1.62369f,.15046f,1.63419f,
	    .15038f,1.64469f,.1503f,1.65519f,.15022f,1.66569f,.15013f,
	    1.67619f,.15003f,1.68669f,.14993f,1.69719f,.14982f,1.7077f,.1497f,
	    1.71841f,.14957f,1.72912f,.14943f,1.73984f,.14929f,1.75055f,
	    .14914f,1.76127f,.14897f,1.77198f,.1488f,1.7827f,.14862f,1.79341f,
	    .14842f,1.80412f,.14822f,1.81484f,.148f,1.82589f,.14776f,1.83694f,
	    .14752f,1.84798f,.14725f,1.85903f,.14698f,1.87008f,.14669f,
	    1.88113f,.14639f,1.89218f,.14607f,1.90322f,.14574f,1.91427f,
	    .14539f,1.92531f,.14502f,1.93688f,.14462f,1.94844f,.1442f,1.96f,
	    .14376f,1.97156f,.14329f,1.98312f,.14279f,1.99468f,.14227f,
	    2.00623f,.14171f,2.01779f,.14111f,2.02934f,.14048f,2.04089f,
	    .13981f,2.05308f,.13905f,2.06526f,.13825f,2.07745f,.13739f,
	    2.08962f,.13648f,2.1018f,.13551f,2.11397f,.13449f,2.12613f,
	    .13341f,2.13829f,.13227f,2.15044f,.13106f,2.16259f,.1298f,
	    2.17538f,.12839f,2.18816f,.12692f,2.20093f,.12538f,2.2137f,.1238f,
	    2.22646f,.12216f,2.23922f,.12048f,2.25197f,.11876f,2.26472f,.117f,
	    2.27746f,.11523f,2.29021f,.11343f,2.30379f,.1115f,2.31737f,
	    .10955f,2.33095f,.10758f,2.34453f,.10559f,2.3581f,.10358f,
	    2.37167f,.10154f,2.38523f,.09948f,2.39879f,.09738f,2.41235f,
	    .09526f,2.4259f,.0931f,2.44078f,.0907f,2.45566f,.08825f,2.47053f,
	    .08576f,2.48539f,.08324f,2.50025f,.08068f,2.5151f,.07808f,
	    2.52995f,.07545f,2.54478f,.07278f,2.55962f,.07008f,2.57444f,
	    .06735f,2.59138f,.0642f,2.60831f,.06102f,2.62523f,.05782f,
	    2.64216f,.05463f,2.65909f,.05145f,2.67603f,.0483f,2.69297f,
	    .04518f,2.70993f,.04212f,2.7269f,.03912f,2.74389f,.0362f,2.78335f,
	    .02979f,2.8229f,.02391f,2.86255f,.01859f,2.90227f,.01386f,
	    2.94205f,.00977f,2.98189f,.00635f,3.02178f,.00362f,3.0617f,
	    .00163f,3.10164f,4.1e-4f,3.14159f,0.f };

    static integer j;
    static real phild[251], psild[251];
    extern /* Subroutine */ int lcsfit_(integer *, real *, real *, logical *, 
	    char *, integer *, real *, real *, real *, ftnlen);

    /* Parameter adjustments */
    --psi;
    --phi;

    /* Function Body */
    for (j = 1; j <= 251; ++j) {
	phild[j - 1] = a[(j << 1) - 2];
	psild[j - 1] = a[(j << 1) - 1];
/* L100: */
    }
    for (j = 1; j <= 201; ++j) {
	lcsfit_(&c__251, phild, psild, &c_true, "B", &c__201, &phi[1], &psi[1]
		, &psi[1], (ftnlen)1);
/* L202: */
    }
    return 0;
} /* phps67_ */

/* +--------------------------------------------------------------------- */

/* +---------------------------------------------------------------------- */

doublereal threept_(integer *j, real *h__, real *del)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static integer step;
    static real weight;


/*     One-liner: First derivative, non-central 3-point formula */
/*     ---------- */

/*     Description and usage: */
/*     ---------------------- */

/*        Computes a first derivative approximation for PLSFIT over an */
/*     interval at a data boundary, using a forward or backward 3-point */
/*     formula.  The data must be in the form of arrays containing finite */
/*     difference interval lengths and 2-point forward difference deriva- */
/*     tives, and the differencing direction is controlled by a flag. See */
/*     PLSFIT for more details. */

/*        See module BUTLAND for a version with "shape-preserving" */
/*     adjustments. */

/*     Arguments: */
/*     ---------- */

/*     Name    Type/Dimension  I/O/S  Description */
/*     J       I               I      Indicates at which end of the */
/*                                    interval the derivative is to be */
/*                                    estimated. J = 0 means left-hand */
/*                                    side, J = 1 means right. */

/*     H       R (-1:1)        I      Array of interval lengths. The 0th */
/*                                    element is the length of the interval */
/*                                    on which the cubic is to be deter- */
/*                                    mined. */

/*     DEL     R (-1:1)        I      Array of derivative estimates. The */
/*                                    0th element is the forward difference */
/*                                    derivative over the interval on which */
/*                                    the cubic is to be determined. */

/*     THREEPT R                 O    The function value is the derivative. */

/*     Environment:  VAX/VMS; FORTRAN 77 */
/*     ------------ */

/*     IMPLICIT NONE and 8-character symbolic names are non-standard. */

/*     Author:  Robert Kennelly, Sterling Federal Systems/NASA-Ames */
/*     ------- */

/*     History: */
/*     -------- */

/*     18 Feb. 1987    RAK    Initial design and coding. */
/*     06 June 1991    DAS    Original THREEPT renamed BUTLAND; THREEPT */
/*                            now gives unmodified 1-sided 3-pt. results. */

/* ----------------------------------------------------------------------- */
/*     Arguments. */
/*     Local constants. */
/*     Local variables. */
/*     Execution. */
/*     ---------- */
/*     Estimate first derivative on a left-hand boundary using a 3-point */
/*     forward difference (STEP = +1), or with a backward difference for */
/*     the right-hand boundary (STEP = -1). */
    /* Parameter adjustments */
    ++del;
    ++h__;

    /* Function Body */
    step = 1 - *j - *j;
/*     In {H, DEL} form, the derivative looks like a weighted average. */
/* J here is consistent with related modules. */
    weight = -h__[0] / (h__[0] + h__[step]);
    ret_val = weight * del[step] + (1.f - weight) * del[0];
/*     Termination. */
/*     ------------ */
    return ret_val;
} /* threept_ */

/* +--------------------------------------------------------------------- */

/* Subroutine */ int lddata_(integer *maxpts, char *title, integer *nu, 
	integer *nl, real *xu, real *xl, real *yu, real *yl, ftnlen title_len)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__;


/*  PARAMETERS: */
/*   ARG     DIM   TYPE  I/O/S   DESCRIPTION */
/*   MAXPTS   -      I     I     Max. no. of points on any one surface. */
/*   TITLE    -     C*(*)  I     Variable-length title for profile. */
/*   NU,NL    -      I     I     Number of data points for upper and lower */
/*                               surfaces (NU > 0; NL=0 is OK if FORMAT=1; */
/*                               NU=NL=no. of pts. in 3-column format and */
/*                               the camber/thickness distributions - FORMATs */
/*                               4 & 5). */
/*   XU     MAXPTS   R     I     Upper surface abscissas. */
/*   XL     MAXPTS   R     I     Lower surface abscissas (if any). */
/*   YU,YL  MAXPTS   R     I     Corresponding ordinates, y" values, or */
/* ---------------------------------------------------------------------- */
/*  *  Arguments: */
/*  *  Local variables: */
/*  *  Execution: */
    /* Parameter adjustments */
    --yl;
    --yu;
    --xl;
    --xu;

    /* Function Body */
    sixpnts_1.nml = *nl;
    sixpnts_1.nmu = *nu;
    i__1 = *nl;
    for (i__ = 1; i__ <= i__1; ++i__) {
	sixpnts_1.xxl[i__ - 1] = xl[i__];
	sixpnts_1.yyl[i__ - 1] = yl[i__];
/* L780: */
    }
    i__1 = *nu;
    for (i__ = 1; i__ <= i__1; ++i__) {
	sixpnts_1.xxu[i__ - 1] = xu[i__];
	sixpnts_1.yyu[i__ - 1] = yu[i__];
/* L790: */
    }
    return 0;
} /* lddata_ */

