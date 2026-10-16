"""Figures for Cobra.md: planform and profile, and cross sections.

    <build>/venv/bin/python cobra_figs.py [views sections]

The drawing helpers are shared with the other body pages, in ../fuselagestack/bodyfig.py.
"""

import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "fuselagestack" ) )

import numpy as np

import bodyfig as bf
from bodyfig import vsp, X, Y, Z, FS

EXAMPLE = dict( XradN = 2.0, YradN = 1.0, PowX = 2.0, NoseULSymm = 0, ZradNU = 0.8, PowNU = 2.0, ZradNL = 0.5, PowNL = 2.0,
                XlenA = 4.0, YradA = 0.5, AftULSymm = 0, ZradAU = 0.5, PowAU = 4.0, ZradAL = 0.2, PowAL = 4.0 )


def cobra():
    bf.renew()
    c = vsp.AddGeom( "COBRA" )
    vsp.Update()
    for k, v in EXAMPLE.items():
        vsp.SetParmVal( c, k, "Design", v )
    vsp.Update()
    return c


def views():
    """Cobra_views: the example from above and from the side, with its dimensions."""
    g = cobra()
    G = bf.grid( g, 0, nu = 801, w = np.array( [ 0.0, 0.25, 0.5, 0.75 ] ) )
    # The surface reaches the nose station's full width, and the base's.
    xn = EXAMPLE[ "XradN" ]
    L = xn + EXAMPLE[ "XlenA" ]
    assert abs( G[ ..., 0 ].max() - L ) < 1e-9 and abs( G[ :, 0, 1 ].max() - EXAMPLE[ "YradN" ] ) < 1e-4
    fig = bf.figure()
    ct = bf.canvas( fig, bf.TOP )
    cs = bf.canvas( fig, bf.SIDE )
    plan = np.vstack( [ G[ :, 0 ], G[ ::-1, 2 ] ] )          # right (+Y) then left
    prof = np.vstack( [ G[ :, 3 ], G[ ::-1, 1 ] ] )          # top then bottom
    bf.fill( ct, plan )
    bf.fill( cs, prof )
    for c in ( ct, cs ):
        bf.axis_line( c, -0.3, L + 0.4 )
        c.line( np.array( [ [ xn, 0, 0 ], [ xn, 0, 0 ] ] ), color = "k" )
    # Planform.
    yb = -1.0
    bf.dim( ct, np.array( [ 0, yb, 0 ] ), np.array( [ xn, yb, 0 ] ), "nose Length", off = ( 0, -0.25 ), dy = -10, ext = False )
    bf.dim( ct, np.array( [ xn, yb, 0 ] ), np.array( [ L, yb, 0 ] ), "aft Length", off = ( 0, -0.25 ), dy = -10, ext = False )
    ct.line( np.array( [ [ xn, -1.35, 0 ], [ xn, 1.15, 0 ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ) )
    bf.dim( ct, np.array( [ xn, 0, 0 ] ), np.array( [ xn, EXAMPLE[ "YradN" ], 0 ] ), "Width", off = ( 0.0, 0 ), dx = -24, ext = False )
    bf.dim( ct, np.array( [ L, 0, 0 ] ), np.array( [ L, EXAMPLE[ "YradA" ], 0 ] ), "Width", off = ( 0.25, 0 ), dx = 22 )
    # Profile.
    cs.line( np.array( [ [ xn, 0, -0.65 ], [ xn, 0, 0.95 ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ) )
    bf.dim( cs, np.array( [ xn, 0, 0 ] ), np.array( [ xn, 0, EXAMPLE[ "ZradNU" ] ] ), "Upper Height", dx = -38, ext = False )
    bf.dim( cs, np.array( [ xn, 0, 0 ] ), np.array( [ xn, 0, -EXAMPLE[ "ZradNL" ] ] ), "Lower Height", dx = -38, ext = False )
    bf.dim( cs, np.array( [ L, 0, 0 ] ), np.array( [ L, 0, EXAMPLE[ "ZradAU" ] ] ), "Upper Height", off = ( 0.25, 0 ), dx = 38 )
    bf.dim( cs, np.array( [ L, 0, 0 ] ), np.array( [ L, 0, -EXAMPLE[ "ZradAL" ] ] ), "Lower Height", off = ( 0.25, 0 ), dx = 38 )
    bf.title( ct, "from above", pad = 6 )
    bf.title( cs, "from the side", pad = 6 )
    bf.key( ct, "top", ( 1.0, 0.0 ), 60, 0 )
    bf.key( cs, "side", ( 1.0, 0.0 ), 60, 0 )
    bf.save( fig, "Cobra_views", [ [ ct ], [ cs ] ], hmax = 6.0 )


def sections():
    """Cobra_sections: cross sections from the nose station to the base, from the front."""
    g = cobra()
    xn = EXAMPLE[ "XradN" ]
    L = xn + EXAMPLE[ "XlenA" ]
    w = np.linspace( 0.0, 1.0, 241 )
    us = np.linspace( 0.0, 1.0, 2001 )
    xs = np.array( [ vsp.CompPnt01( g, 0, u, 0.0 ).x() for u in us ] )
    fig = bf.figure()
    c = bf.canvas( fig, bf.FRONT )
    stations = [ xn, xn + 1.0, xn + 2.0, xn + 3.0, L - 1e-6 ]
    for k, x in enumerate( stations ):
        u = np.interp( x, xs[ :np.argmax( xs ) + 1 ], us[ :np.argmax( xs ) + 1 ] )
        P = np.array( [ list( vsp.CompPnt01( g, 0, u, a ) ) for a in w ] )
        first = k == 0
        last = k == len( stations ) - 1
        col = "#0072B2" if first else ( "#D55E00" if last else "#888888" )
        c.line( P, color = col, lw = 1.6 if ( first or last ) else 0.7, z = 30 if ( first or last ) else 20 )
    c.line( np.array( [ [ 0, -1.15, 0 ], [ 0, 1.15, 0 ] ] ), color = "#888888", lw = 0.6, ls = ( 0, ( 8, 2, 1, 2 ) ) )
    bf.label( c, np.array( [ 0, 0.62, 0.62 ] ), "nose station, exponent 2", color = "#0072B2", ha = "right", dx = -8 )
    bf.label( c, np.array( [ 0, 0.0, 0.18 ] ), "base, exponent 4", color = "#D55E00" )
    bf.key( c, "front", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "Cobra_sections", [ [ c ] ], hmax = 3.2 )


FIGS = { "views": views, "sections": sections }

if __name__ == "__main__":
    for name in ( sys.argv[ 1: ] or FIGS ):
        FIGS[ name ]()
