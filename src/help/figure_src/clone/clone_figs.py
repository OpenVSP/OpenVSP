"""Figures for the OpenVSP Clone help page (src/help/Clone.md).

    <build>/venv/bin/python clone_figs.py [names...]      # writes ../../images/Clone_*.png

Every surface is VSP's own, sampled with CompPnt01.  2D views keep the model frame (the top view looks down -Z, X
right, Y up) and carry an axis key.
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )

import numpy as np
import matplotlib.pyplot as plt
from scipy.spatial import ConvexHull

import meshfig as mf
from meshfig import fl, vsp

import clonemodel as cm

C_PLANE = "#666666"
C_ORIG = "#9AA3AE"
C_CLONE = "#C49A6C"


def draw( c, g, color ):
    """Each surface of g as its outline from above, filled with a translucent tint, so surfaces that lie on top of each
    other show darker."""
    for G in cm.surfs( g ):
        Q = c.v.p2( G.reshape( -1, 3 ) )
        H = Q[ ConvexHull( Q ).vertices ]
        c.pts.append( Q )
        c.ax.fill( H[ :, 0 ], H[ :, 1 ], facecolor = color, alpha = 0.45, edgecolor = "none", zorder = 20 )
        c.ax.fill( H[ :, 0 ], H[ :, 1 ], facecolor = "none", edgecolor = "#333333", lw = 0.8, zorder = 21 )


def fig_symmetry():
    """Clone_symmetry: a mirrored nacelle and two Clones moved aft, one of them onto the mirror plane; then that one
    with its symmetry no longer copied and turned off."""
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    cans = []
    for remedy in ( False, True ):
        p = cm.nacelle()
        c1 = cm.clone( p, cm.DX, cm.Y0 )
        c2 = cm.clone( p, 2 * cm.DX, 0.0 )
        Q1 = cm.centers( c1 )
        assert np.allclose( [ Q1[ 0 ][ 1 ], Q1[ 1 ][ 1 ] ], [ cm.Y0, -cm.Y0 ] )
        if remedy:
            cm.single( c2 )
            assert vsp.GetTotalNumSurfs( c2 ) == 1
        else:
            assert vsp.GetTotalNumSurfs( c2 ) == 2
            assert np.allclose( [ q[ 1 ] for q in cm.centers( c2 ) ], [ 0.0, 0.0 ] )
        c = mf.panel( fig, mf.TOP )
        draw( c, p, C_ORIG )
        draw( c, c1, C_CLONE )
        draw( c, c2, C_CLONE )
        Po = cm.centers( p )
        x0 = Po[ 0 ][ 0 ] - 4.5
        x1 = cm.centers( c2 )[ 0 ][ 0 ] + 5.0
        c.line( np.array( [ [ x0, 0, 0 ], [ x1, 0, 0 ] ] ), color = C_PLANE, lw = 0.8, ls = ( 0, ( 8, 2, 1, 2 ) ), z = 40 )
        c.text( np.array( [ x0, 0, 0 ] ), "$Y = 0$", color = C_PLANE, dx = -4, ha = "right" )
        r = 1.0
        c.text( Po[ 0 ] + r * mf.Y, "original", dy = 7, va = "bottom" )
        c.text( Po[ 1 ] - r * mf.Y, "its mirror image", dy = -7, va = "top" )
        c.text( Q1[ 0 ] + r * mf.Y, "first Clone", dy = 7, va = "bottom" )
        c.text( Q1[ 1 ] - r * mf.Y, "its mirror image", dy = -7, va = "top" )
        q2 = cm.centers( c2 )[ 0 ]
        if remedy:
            c.text( q2 + r * mf.Y, "second Clone, on $Y = 0$:\none nacelle", dy = 7, va = "bottom" )
        else:
            c.text( q2 + r * mf.Y, "second Clone, on $Y = 0$: two\nhalves on top of each other", dy = 7, va = "bottom" )
        lo, hi = c.limits()
        if remedy:
            title = "(b) Symmetry and Flip unchecked on the second Clone, and its own symmetry off"
        else:
            title = "(a) Symmetry and Flip copied by both Clones"
        c.ax.annotate( title, xy = ( 0.5 * ( lo[ 0 ] + hi[ 0 ] ), lo[ 1 ] ), xytext = ( 0, -26 ), textcoords = "offset points",
                       ha = "center", va = "top", fontsize = fl.FS )
        mf.key( c, "top", at = ( 1.0, 0.0 ), dx = 26, dy = 0 )
        cans.append( c )
    box = ( np.min( [ c.limits()[ 0 ] for c in cans ], 0 ), np.max( [ c.limits()[ 1 ] for c in cans ], 0 ) )
    for c in cans:
        c.bounds = box
    mf.save( fig, "Clone_symmetry", [ [ c ] for c in cans ], hmax = 6.0 )


FIGS = { "symmetry": fig_symmetry }

if __name__ == "__main__":
    mf.style()
    for n in ( sys.argv[ 1: ] or FIGS.keys() ):
        FIGS[ n ]()
