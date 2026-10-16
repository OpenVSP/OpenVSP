"""Figures for BOR.md: a Flowthrough BOR, its Angle and its areas, and the Upper and Lower bodies.

    <build>/venv/bin/python bor_figs.py [flowthrough areas upperlower]

The drawing helpers are shared with the other body pages, in ../fuselagestack/bodyfig.py.
"""

import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "fuselagestack" ) )

import numpy as np

import bodyfig as bf
from bodyfig import vsp, X, Y, Z, FS


def xsc( b, name ):
    ids = [ q for q in vsp.GetGeomParmIDs( b ) if vsp.GetParmName( q ) == name and vsp.GetParmGroupName( q ) == "XSecCurve" ]
    assert len( ids ) == 1
    return ids[0]


def bor( mode = vsp.BOR_FLOWTHROUGH, angle = 0.0 ):
    """A BOR with a NACA 4412 section of chord 1."""
    bf.renew()
    b = vsp.AddGeom( "BODYOFREVOLUTION" )
    vsp.Update()
    vsp.ChangeBORXSecShape( b, vsp.XS_FOUR_SERIES )
    vsp.Update()
    vsp.SetParmVal( xsc( b, "Camber" ), 0.04 )
    vsp.SetParmVal( xsc( b, "CamberLoc" ), 0.4 )
    vsp.SetParmVal( xsc( b, "ThickChord" ), 0.12 )
    vsp.SetParmVal( b, "Mode", "Design", mode )
    vsp.SetParmVal( b, "Angle", "Design", angle )
    vsp.Update()
    return b


def section( b, nw = 401 ):
    """The section as swept, cut by the plane Y = 0 above the axis: the w-line at the u where the sweep is at +Z."""
    # u runs around the axis; find the u whose points lie at +Z.
    us = np.linspace( 0.0, 1.0, 401 )
    k = np.argmax( [ vsp.CompPnt01( b, 0, u, 0.75 ).z() for u in us ] )
    return np.array( [ list( vsp.CompPnt01( b, 0, us[k], w ) ) for w in np.linspace( 0.0, 1.0, nw ) ] ), us[k]


def flowthrough():
    """BOR_flowthrough: the section placed at D/2, and turned by Angle."""
    b0 = bor()
    S0, u0 = section( b0 )
    b = bor( angle = 10.0 )
    S, u = section( b )
    lo = np.array( list( vsp.CompPnt01( b, 0, ( u0 + 0.5 ) % 1.0, 0.5 ) ) )
    le = S[ len( S ) // 2 ]
    te = S[ 0 ]
    assert abs( le[0] ) < 1e-3 and abs( le[2] - 1.0 ) < 1e-3          # the LE stays at X = 0, radius D/2
    assert te[2] < S0[0][2]                                             # positive Angle: the TE toward the axis
    Sb = S * np.array( [ 1, 1, -1 ] )
    S0b = S0 * np.array( [ 1, 1, -1 ] )
    fig = bf.figure()
    c = bf.canvas( fig, bf.SIDE )
    bf.fill( c, S0, face = "none", edge = bf.GHOST, lw = 0.9, ls = ( 0, ( 4, 2 ) ), z = 20 )
    bf.fill( c, S, z = 10 )
    bf.axis_line( c, -0.3, 1.4 )
    bf.dim( c, np.array( [ 0, 0, 0 ] ), np.array( [ 0, 0, 1.0 ] ), r"$D/2$", off = ( -0.18, 0 ), dx = -14 )
    # Angle about the LE, from the chord at Angle 0 to the turned chord.
    ch0 = S0[ 0 ] - S0[ len( S0 ) // 2 ]
    ch = te - le
    c.line( np.array( [ le, le + 1.15 * ch0 / np.linalg.norm( ch0 ) ] ), color = bf.GHOST, lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 25 )
    c.line( np.array( [ le, te ] ), color = "#0072B2", lw = 0.8, z = 26 )
    c.angle( le, ch0, ch, 1.1, "#0072B2", label = "Angle", label_r = 1.0, dx = 26, dy = 0 )
    bf.label( c, le, "LE", dx = -12, dy = 6 )
    bf.label( c, te, "TE", dx = 12, dy = -4 )
    bf.label( c, np.array( [ 1.35, 0, 0 ] ), "axis", dy = 8 )
    bf.key( c, "side", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "BOR_flowthrough", [ [ c ] ], hmax = 3.4 )


def areas():
    """BOR_areas: the radii of A0, Ae and Amin, on a section turned so that the three differ."""
    ang = -5.0
    b = bor( angle = ang )
    S, u = section( b, nw = 4001 )
    w = np.linspace( 0.0, 1.0, len( S ) )
    le = S[ ( len( S ) - 1 ) // 2 ]
    te = S[ 0 ]
    k = int( np.argmin( S[:, 2] ) )
    mn = S[ k ]
    A = { n: vsp.GetParmVal( b, n, "Design" ) for n in ( "A0", "Ae", "Amin", "AminW" ) }
    # Each output is the area of the circle the swept point makes about the axis.
    assert abs( A[ "A0" ] - np.pi * le[2] ** 2 ) < 1e-9
    assert abs( A[ "Ae" ] - np.pi * te[2] ** 2 ) < 1e-6
    assert abs( A[ "Amin" ] - np.pi * mn[2] ** 2 ) < 1e-5
    assert abs( A[ "AminW" ] - w[k] ) < 1e-3 and 0.0 < w[k] < 0.5        # on the inner surface, from the TE
    assert te[2] > le[2] > mn[2]
    fig = bf.figure()
    c = bf.canvas( fig, bf.SIDE )
    bf.fill( c, S, z = 10 )
    bf.axis_line( c, -0.3, 1.4 )
    col = "#0072B2"
    for p, s, dx in ( ( le, r"$r_0 = D/2$", -30 ), ( mn, r"$r_{min}$", 18 ), ( te, r"$r_e$", 12 ) ):
        c.line( np.array( [ [ p[0], 0, 0 ], p ] ), color = "#777777", lw = 0.5, z = 40 )
        c.arrow( np.array( [ p[0], 0, 0 ] ), p, color = col, lw = 0.9, head = 6.0, z = 45 )
        bf.label( c, np.array( [ p[0], 0, 0.5 * p[2] ] ), s, dx = dx, color = col )
    c.point( mn, "k", s = 12, z = 60 )
    bf.label( c, mn, "Amin W = %.2f" % A[ "AminW" ], dx = 6, dy = -10, ha = "left" )
    bf.label( c, le, "LE", dx = -12, dy = 6 )
    bf.label( c, te, "TE", dx = 12, dy = 4 )
    bf.label( c, np.array( [ 1.35, 0, 0 ] ), "axis", dy = 8 )
    bf.key( c, "side", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "BOR_areas", [ [ c ] ], hmax = 3.4 )
    return A


def upperlower():
    """BOR_upperlower: the bodies swept from the upper and the lower surface."""
    canv = []
    fig = bf.figure()
    foil = None
    for mode, name in ( ( vsp.BOR_UPPER, "Upper" ), ( vsp.BOR_LOWER, "Lower" ) ):
        b = bor( mode )
        out = bf.cut_outlines( b, 0, wtop = 0.75, wbot = 0.25 )
        G = bf.grid( b, 0, nu = 81, nw = 201 )
        r = np.hypot( G[ ..., 1 ], G[ ..., 2 ] )
        c = bf.canvas( fig, bf.SIDE )
        # The body cut by Y = 0: the meridians at +Z and -Z of the sweep.
        us = np.linspace( 0.0, 1.0, 401 )
        top = None
        k = np.argmax( [ vsp.CompPnt01( b, 0, u, 0.5 ).z() for u in us ] )
        M = np.array( [ list( vsp.CompPnt01( b, 0, us[k], w ) ) for w in np.linspace( 0.0, 1.0, 401 ) ] )
        P = np.vstack( [ M, ( M * np.array( [ 1, 1, -1 ] ) )[ ::-1 ] ] )
        bf.fill( c, P, z = 10 )
        canv.append( ( c, b, name ) )
    # The section itself, chord on the axis, from the Flowthrough BOR at Diameter 0.
    bf.renew()
    b = bor()
    vsp.SetParmVal( b, "Diameter", "Design", 0.0 )
    vsp.Update()
    S, u = section( b )
    for c, b, name in canv:
        c.line( S, color = "k", lw = 0.9, ls = ( 0, ( 4, 2 ) ), z = 30 )
        bf.axis_line( c, -0.1, 1.1 )
        bf.title( c, name, pad = 4 )
    bf.key( canv[ -1 ][0], "side", ( 1.0, 0.0 ), 20, 0 )
    bf.save( fig, "BOR_upperlower", [ [ canv[0][0] ], [ canv[1][0] ] ], hmax = 5.0 )


FIGS = { "flowthrough": flowthrough, "areas": areas, "upperlower": upperlower }

if __name__ == "__main__":
    for name in ( sys.argv[ 1: ] or FIGS ):
        FIGS[ name ]()
