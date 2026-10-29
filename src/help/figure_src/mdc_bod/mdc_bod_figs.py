"""Figures for MdcBod.md: the nose, centerbody and aft body, the nose shapes, bluntness, the aft body, an elliptical
body and camber.

    <build>/venv/bin/python mdc_bod_figs.py [layout noses ...]

The drawing helpers are shared with the other body pages, in ../fuselagestack/bodyfig.py.
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "fuselagestack" ) )

import numpy as np

import bodyfig as bf
import datcomfig as df
from bodyfig import vsp, X, Y, Z, FS

NOSES = [ ( "cone", 0 ), ( "ogive", 1 ), ( "power law", 2 ), ( "L-V Haack", 3 ), ( "Von Karman", 4 ) ]


def body( **kw ):
    """A new MDC_BOD with the Design parms in kw, and X its XForm X location, XO."""
    bf.renew()
    return df.add( "MDC_BOD", **kw )


def outline( g ):
    """The body cut through its axis, as one closed outline."""
    return bf.body_cut( g, 0, 801 )


def dashed( c, p0, p1 ):
    c.line( np.array( [ p0, p1 ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ) )


def layout():
    """MdcBod_layout: the nose, centerbody and aft body, with their lengths and diameters."""
    kw = dict( CENTRDefault = 0, X = 1.0, TNOSE = 1, LNOSE = 3.0, DNOSE = 1.0, LCENTR = 4.0, DCENTR = 1.4, TAFT = 0, LAFT = 2.0, DAFT = 0.8 )
    g = body( **kw )
    P = outline( g )
    xo = kw[ "X" ]
    x1 = xo + kw[ "LNOSE" ]
    x2 = x1 + kw[ "LCENTR" ]
    x3 = x2 + kw[ "LAFT" ]
    assert abs( P[ :, 0 ].max() - x3 ) < 1e-9 and abs( P[ :, 2 ].max() - 0.7 ) < 1e-6
    fig = bf.figure()
    c = bf.canvas( fig, bf.SIDE )
    bf.fill( c, P )
    bf.axis_line( c, 0.0, x3 + 0.2 )
    for x in ( x1, x2 ):
        dashed( c, np.array( [ x, 0, -0.95 ] ), np.array( [ x, 0, 0.95 ] ) )
    yd = -0.95
    bf.dim( c, np.array( [ xo, 0, yd ] ), np.array( [ x1, 0, yd ] ), "LNOSE", dy = -9, ext = False )
    bf.dim( c, np.array( [ x1, 0, yd ] ), np.array( [ x2, 0, yd ] ), "LCENTR", dy = -9, ext = False )
    bf.dim( c, np.array( [ x2, 0, yd ] ), np.array( [ x3, 0, yd ] ), "LAFT", dy = -9, ext = False )
    bf.dim( c, np.array( [ x1, 0, -0.5 ] ), np.array( [ x1, 0, 0.5 ] ), "DNOSE", dx = -28, ext = False )
    bf.dim( c, np.array( [ x2, 0, -0.7 ] ), np.array( [ x2, 0, 0.7 ] ), "DCENTR", dx = -30, ext = False )
    bf.dim( c, np.array( [ x3, 0, -0.4 ] ), np.array( [ x3, 0, 0.4 ] ), "DAFT", dx = 30, ext = False )
    bf.dim( c, np.array( [ 0, 0, 1.0 ] ), np.array( [ xo, 0, 1.0 ] ), "XO", dy = 9, ext = False )
    dashed( c, np.array( [ 0, 0, 0.9 ] ), np.array( [ 0, 0, 1.1 ] ) )
    bf.label( c, np.array( [ 0, 0, 0.9 ] ), "X = 0", dy = -8 )
    bf.label( c, np.array( [ 0.5 * ( xo + x1 ), 0, 0.7 ] ), "nose", dy = 6 )
    bf.label( c, np.array( [ 0.5 * ( x1 + x2 ), 0, 1.0 ] ), "centerbody" )
    bf.label( c, np.array( [ 0.5 * ( x2 + x3 ), 0, 0.85 ] ), "aft body", dy = 6 )
    bf.key( c, "side", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "MdcBod_layout", [ [ c ] ], hmax = 3.4 )


def top( g, n = 1201 ):
    """The upper meridian of the body, ( n, 3 )."""
    return bf.meridian( g, 0, 0.75, n )


def noses():
    """MdcBod_noses: the five nose shapes, radius against length."""
    import matplotlib.pyplot as plt
    import figlib as fl
    cols = [ "#0072B2", "#D55E00", "#009E73", "#CC79A7", "#444444" ]
    bf.style()
    fig = plt.figure( figsize = ( fl.PAGE_W, 3.2 ) )
    ax = fig.add_axes( [ 0.09, 0.16, 0.88, 0.80 ] )
    labs = [ "CONE", "OGIVE", "POWER, n = 0.5", "HAACK", "KARMAN" ]
    for ( name, t ), col, lab in zip( NOSES, cols, labs ):
        g = body( TNOSE = t, POWER = 0.5, LNOSE = 4.0, DNOSE = 1.0, LCENTR = 1.0 )
        P = top( g )
        i = P[ :, 0 ] <= 4.0 + 1e-9
        ax.plot( P[ i, 0 ] / 4.0, P[ i, 2 ] / 0.5, color = col, lw = 1.6, label = lab )
    ax.set_xlim( 0, 1 )
    ax.set_ylim( 0, 1.05 )
    ax.set_xlabel( "$x/\\mathrm{LNOSE}$", fontsize = FS )
    ax.set_ylabel( "$r/R$", fontsize = FS )
    ax.set_xticks( np.linspace( 0, 1, 11 ) )
    ax.tick_params( labelsize = FS )
    ax.legend( loc = "lower right", fontsize = FS, frameon = False )
    for sp in ( "top", "right" ):
        ax.spines[ sp ].set_visible( False )
    bf.save_plot( fig, "MdcBod_noses" )


def legend( c, x, z, items, dz = 0.22, length = 0.5, dx = 0.12 ):
    """A key drawn in the model: a short line and its label for each ( label, color ) in items, from ( x, z ) down."""
    for k, ( lab, col ) in enumerate( items ):
        zz = z - k * dz
        c.line( np.array( [ [ x, 0, zz ], [ x + length, 0, zz ] ] ), color = col, lw = 1.6, z = 30 )
        bf.label( c, np.array( [ x + length + dx, 0, zz ] ), lab, ha = "left" )


def secant():
    """MdcBod_secant: tangent and secant ogive noses."""
    fig = bf.figure()
    c = bf.canvas( fig, bf.SIDE )
    cone = 0.5 / 3.0
    items = []
    for slp, col, lab in ( ( 0.0, "#444444", "SLPNOS 0, a tangent ogive" ), ( 0.5 * cone, "#0072B2", "SLPNOS 0.083" ),
                           ( cone, "#D55E00", "SLPNOS 0.167, the cone" ) ):
        g = body( TNOSE = 1, LNOSE = 3.0, DNOSE = 1.0, LCENTR = 1.0, SLPNOS = slp )
        P = top( g )
        i = P[ :, 0 ] <= 3.0 + 1e-9
        c.line( P[ i ], color = col, lw = 1.6, z = 30 )
        items.append( ( lab, col ) )
    bf.axis_line( c, -0.2, 3.4 )
    c.line( np.array( [ [ 3.0, 0, 0 ], [ 3.0, 0, 0.5 ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ) )
    legend( c, 1.6, 0.0 - 0.1, items, dz = 0.12 )
    bf.key( c, "side", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "MdcBod_secant", [ [ c ] ], hmax = 3.0 )


def blunt():
    """MdcBod_blunt: a sharp, a blunted and a truncated nose."""
    fig = bf.figure()
    rows = []
    cases = [ ( "sharp", dict( BNOSE = 0.0, TRUNC = 0 ) ), ( "blunted: BNOSE 0.2", dict( BNOSE = 0.2, TRUNC = 0 ) ),
              ( "truncated: BNOSE 0.2, TRUNC on", dict( BNOSE = 0.2, TRUNC = 1 ) ) ]
    for k, ( lab, kw ) in enumerate( cases ):
        g = body( TNOSE = 1, LNOSE = 3.0, DNOSE = 1.0, LCENTR = 1.0, **kw )
        P = outline( g )
        c = bf.canvas( fig, bf.SIDE )
        bf.fill( c, P )
        bf.axis_line( c, -0.3, 4.3 )
        bf.title( c, lab, pad = 4 )
        rows.append( [ c ] )
    bf.save( fig, "MdcBod_blunt", rows, hmax = 5.0 )


def aft():
    """MdcBod_aft: boattails and flares."""
    fig = bf.figure()
    rows = []
    cases = [ ( "boattail, DAFT 0.6", 0.6 ), ( "flare, DAFT 1.4", 1.4 ) ]
    for lab, daft in cases:
        c = bf.canvas( fig, bf.SIDE )
        items = []
        for taft, slp, col, tag in ( ( 0, 0.0, "#444444", "TAFT CONE" ), ( 1, 0.0, "#0072B2", "TAFT OGIVE, SLPTAL 0" ),
                                     ( 1, 0.3, "#D55E00", "TAFT OGIVE, SLPTAL 0.3" ) ):
            g = body( CENTRDefault = 0, TNOSE = 1, LNOSE = 2.0, DNOSE = 1.0, LCENTR = 2.0, LAFT = 3.0, DAFT = daft, TAFT = taft, SLPTAL = slp )
            P = top( g )
            i = P[ :, 0 ] >= 3.6
            c.line( P[ i ], color = col, lw = 1.5, z = 30 )
            items.append( ( tag, col ) )
        bf.axis_line( c, 3.6, 7.2 )
        for x in ( 4.0, 7.0 ):
            c.line( np.array( [ [ x, 0, 0 ], [ x, 0, 0.8 ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ) )
        bf.dim( c, np.array( [ 4.0, 0, 0.0 ] ), np.array( [ 7.0, 0, 0.0 ] ), "LAFT", dy = -9, ext = False )
        if daft > 1.0:
            legend( c, 4.6, -0.3, items, dz = 0.17 )
        bf.title( c, lab, pad = 4 )
        rows.append( [ c ] )
    bf.save( fig, "MdcBod_aft", rows, hmax = 5.0 )


def sections():
    """MdcBod_sections: the sections of an ELLBOD at the nose base, the centerbody end and the aft end."""
    kw = dict( NAMELIST = 1, CENTRDefault = 0, TNOSE = 1, LNOSE = 2.0, WNOSE = 1.0, ENOSE = 0.6, LCENTR = 3.0, WCENTR = 1.6,
               ECENTR = 1.0, LAFT = 2.0, WAFT = 0.8, EAFT = 1.5 )
    g = body( **kw )
    xs = [ 2.0, 5.0, 7.0 ]
    us = np.linspace( 0.0, 1.0, 4001 )
    xx = np.array( [ vsp.CompPnt01( g, 0, u, 0.0 ).x() for u in us ] )
    k = int( np.argmax( xx ) )
    w = np.linspace( 0.0, 1.0, 241 )
    fig = bf.figure()
    c = bf.canvas( fig, bf.FRONT )
    cols = [ "#0072B2", "#888888", "#D55E00" ]
    names = [ "nose base", "centerbody end", "aft end" ]
    dims = [ ( 1.0, 0.6 ), ( 1.6, 1.6 ), ( 0.8, 1.2 ) ]
    for x, col, wh in zip( xs, cols, dims ):
        u = np.interp( x - 1e-6 if x == xs[ -1 ] else x, xx[ :k + 1 ], us[ :k + 1 ] )
        P = np.array( [ list( vsp.CompPnt01( g, 0, u, a ) ) for a in w ] )
        assert abs( P[ :, 1 ].max() - 0.5 * wh[0] ) < 2e-3 and abs( P[ :, 2 ].max() - 0.5 * wh[1] ) < 2e-3, ( x, P[ :, 1 ].max(), P[ :, 2 ].max() )
        c.line( P, color = col, lw = 1.6, z = 30 )
    c.line( np.array( [ [ 0, -1.0, 0 ], [ 0, 1.0, 0 ] ] ), color = "#888888", lw = 0.6, ls = ( 0, ( 8, 2, 1, 2 ) ) )
    c.line( np.array( [ [ 0, 0, -0.95 ], [ 0, 0, 0.95 ] ] ), color = "#888888", lw = 0.6, ls = ( 0, ( 8, 2, 1, 2 ) ) )
    # Each label at the right, joined to its section.
    for t, ( a_, b_ ), col, z, lab in ( ( 40.0, ( 0.5, 0.3 ), "#0072B2", 0.0, "nose base:\nWNOSE 1, ENOSE 0.6" ),
                                      ( 30.0, ( 0.8, 0.8 ), "#666666", 0.5, "centerbody end:\nWCENTR 1.6, ECENTR 1" ),
                                      ( 60.0, ( 0.4, 0.6 ), "#D55E00", 1.0, "aft end:\nWAFT 0.8, EAFT 1.5" ) ):
        p = np.array( [ 0, -a_ * np.cos( np.radians( t ) ), b_ * np.sin( np.radians( t ) ) ] )
        q = np.array( [ 0, -1.05, 0.55 * z - 0.05 + 0.0 ] )
        q[ 2 ] = [ 0.0, 0.5, 0.95 ][ [ 0.0, 0.5, 1.0 ].index( z ) ]
        c.line( np.array( [ p, q ] ), color = col, lw = 0.6, z = 35 )
        bf.label( c, q, lab, color = col, ha = "left", dx = 4 )
    bf.key( c, "front", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "MdcBod_sections", [ [ c ] ], hmax = 4.2 )


def camber():
    """MdcBod_camber: a body cambered by ZTIP, ZNOSE, ZCENTR and ZAFT."""
    kw = dict( TNOSE = 1, LNOSE = 3.0, DNOSE = 1.0, LCENTR = 4.0, LAFT = 2.0, DAFT = 0.6, ZTIP = 0.0, ZNOSE = 0.4, ZCENTR = 1.0, ZAFT = 0.6 )
    g = body( **kw )
    P = outline( g )
    xs = [ 0.0, 3.0, 7.0, 9.0 ]
    zs = [ kw[ "ZTIP" ], kw[ "ZNOSE" ], kw[ "ZCENTR" ], kw[ "ZAFT" ] ]
    assert abs( P[ :, 0 ].max() - 9.0 ) < 1e-9
    fig = bf.figure()
    c = bf.canvas( fig, bf.SIDE )
    bf.fill( c, P )
    bf.axis_line( c, -0.3, 9.4 )
    cl = np.array( [ [ x, 0, z ] for x, z in zip( xs, zs ) ] )
    c.line( cl, color = "#0072B2", lw = 1.4, z = 40 )
    for x, z, nm in zip( xs, zs, [ "ZTIP", "ZNOSE", "ZCENTR", "ZAFT" ] ):
        c.point( np.array( [ x, 0, z ] ), color = "#0072B2", z = 50 )
        if z > 0:
            bf.dim( c, np.array( [ x, 0, 0 ] ), np.array( [ x, 0, z ] ), nm, dx = -26, ext = False, color = "#0072B2" )
        else:
            bf.label( c, np.array( [ x, 0, z ] ), nm, dx = 0, dy = -12, color = "#0072B2" )
    bf.key( c, "side", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "MdcBod_camber", [ [ c ] ], hmax = 3.6 )


FIGS = { "layout": layout, "noses": noses, "secant": secant, "blunt": blunt, "aft": aft, "sections": sections, "camber": camber }

if __name__ == "__main__":
    for name in ( sys.argv[ 1: ] or FIGS ):
        FIGS[ name ]()
