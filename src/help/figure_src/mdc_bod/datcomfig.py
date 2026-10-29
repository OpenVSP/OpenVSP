"""Helpers shared by the DATCOM component figures (MDC_BOD, MDC_PROTUB, MDC_INLET, MDC_FIN): building a component and
a body for it to stand on, sampling every surface of a component, and drawing them.

Geometry comes from the openvsp module of the build this runs with.  Layout, text size and projection are those of
the body figures (../fuselagestack/bodyfig.py).
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "fuselagestack" ) )

import numpy as np

import bodyfig as bf
from bodyfig import vsp, X, Y, Z, FS

BODY = "#C8CCD2"        # the body a component stands on
PART = "#E3B98F"        # the component drawn
PART_EDGE = "#4a4a4a"


# The Parms that set how many Parms of a station, panel or member there are.
COUNTS = ( "PTYPE", "NLOC", "NIN", "NSPAN", "NPANEL" )


def add( kind, **kw ):
    """A new component of kind with the Design parms in kw, set in order; answers its ID.  Call bf.renew() first.
    The Parms of each station, panel or member are made by the update after their count, so the counts go first.  X
    is the XForm X location, which is XO, XPROT, XINLT or XLE(1)."""
    g = vsp.AddGeom( kind )
    vsp.Update()
    if "X" in kw:
        vsp.SetParmVal( g, "X_Rel_Location", "XForm", float( kw.pop( "X" ) ) )
    for k in COUNTS:
        if k in kw:
            vsp.SetParmVal( g, k, "Design", float( kw[ k ] ) )
    vsp.Update()
    for k, v in kw.items():
        if k not in COUNTS:
            vsp.SetParmVal( g, k, "Design", float( v ) )
    vsp.Update()
    return g


def cylinder( radius, x0, x1 ):
    """A circular cylinder body from x0 to x1, closed by flat ends."""
    return add( "MDC_BOD", X = x0, LNOSE = 1.0e-3, TNOSE = 0, DNOSE = 2.0 * radius, LCENTR = x1 - x0 - 1.0e-3 )


def surfaces( g, nu = 61, nw = 241 ):
    """The grids ( nu, nw, 3 ) of every main surface of g."""
    out = []
    for s in range( vsp.GetNumMainSurfs( g ) ):
        out.append( bf.grid( g, s, nu = nu, nw = nw ) )
    return out


def draw( c, grids, color, edge = False ):
    for G in grids:
        c.surface( G, color = color, edge = edge )


def patch( g, x0, x1, w0, w1, nu = 90, nw = 40, s = 0 ):
    """The part of surface s of body g between the stations x0 and x1 and the W fractions w0 and w1, ( nu, nw, 3 )."""
    us = np.linspace( 0.0, 1.0, 4001 )
    xs = np.array( [ vsp.CompPnt01( g, s, u, 0.5 ).x() for u in us ] )
    ua = np.interp( x0, xs, us )
    ub = np.interp( x1, xs, us )
    return bf.grid( g, s, u = np.linspace( ua, ub, nu ), w = np.linspace( w0, w1, nw ) )


# Looking forward along -X from behind the body, the way the namelists measure a roll angle: Y right, Z up.
REAR = bf.fl.View.plane( Y, Z )
bf.KEY[ "rear" ] = ( r"$Y$", r"$Z$", r"$X$" )


def circle( g, x, n = 241, s = 0 ):
    """The section of body g at station x, as a closed outline ( n, 3 )."""
    us = np.linspace( 0.0, 1.0, 4001 )
    xs = np.array( [ vsp.CompPnt01( g, s, u, 0.5 ).x() for u in us ] )
    u = np.interp( x, xs, us )
    return np.array( [ list( vsp.CompPnt01( g, s, u, w ) ) for w in np.linspace( 0.0, 1.0, n ) ] )


def arc( c, rho, a0, a1, color = "k", lw = 0.8, n = 60, head = True ):
    """An arc of radius rho about the axis from roll angle a0 to a1 degrees, clockwise from the top looking forward,
    drawn in a REAR view, with an arrowhead at the end."""
    t = np.radians( np.linspace( a0, a1, n ) )
    P = np.array( [ [ 0.0, rho * np.sin( a ), rho * np.cos( a ) ] for a in t ] )
    if head:
        c.arrow_path( P, color = color, lw = lw, head = 6.0, z = 45 )
    else:
        c.line( P, color = color, lw = lw, z = 45 )
    return P
