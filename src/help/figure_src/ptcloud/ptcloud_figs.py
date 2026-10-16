"""Figures for the OpenVSP Point Cloud help page (src/help/PtCloud.md).

    <build>/venv/bin/python ptcloud_figs.py [names...]      # writes ../../images/PtCloud_*.png

The wing is VSP's own, sampled with CompPnt01, and the projected points are read back from the cloud.  3D views have
Z up, Y into the page and X to the right, seen from about (-1, -1, 1).
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )

import numpy as np
import matplotlib.pyplot as plt

import meshfig as mf
from meshfig import fl

import cloudmodel as cm

C_PT = "#0072B2"


def fig_project():
    """PtCloud_project: points above a wing projected along Z onto its surface; one misses and stays."""
    w = cm.wing()
    P = cm.cloud_points( w )
    _, Q = cm.projected( w, P )
    n = len( cm.STATIONS )
    assert np.allclose( Q[ :, :2 ], P[ :, :2 ] ) and np.allclose( Q[ n ], P[ n ] )
    assert np.all( Q[ :n, 2 ] > 0.0 ) and np.all( Q[ :n, 2 ] < 0.2 )
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = mf.panel( fig, mf.ISO )
    c.surface( cm.grid( w ), outline_lw = 0.6, cap_rows = False )
    c.flush()
    for p, q in zip( P[ :n ], Q[ :n ] ):
        c.arrow( p, q + 0.08 * mf.Z, color = C_PT, lw = 0.8, ls = ( 0, ( 3, 2 ) ), z = 60, head = 6.0 )
    for A, filled in ( ( P, False ), ( Q[ :n ], True ) ):
        q2 = c.v.p2( A )
        c.ax.plot( q2[ :, 0 ], q2[ :, 1 ], ls = "none", marker = "o", ms = 5.5, mec = C_PT, mew = 1.2,
                   mfc = C_PT if filled else "white", zorder = 70 )
        c.pts.append( q2 )
    # The miss's line along Z, which passes beyond the tip.
    m = P[ n ]
    c.line( np.array( [ m + 0.6 * mf.Z, m - ( cm.ZC + 1.5 ) * mf.Z ] ), color = C_PT, lw = 0.8, ls = ( 0, ( 1, 2 ) ), z = 55 )
    c.text( P[ 0 ], "points before", color = C_PT, dx = 10, dy = 0, ha = "left" )
    c.text( Q[ 0 ], "projected along $Z$", color = C_PT, dx = 10, dy = -2, ha = "left" )
    c.text( m, "its line along $Z$ misses\nthe wing: the point stays", color = C_PT, dx = -10, dy = 0, ha = "right" )
    # The model axes, beside the wing.
    O = np.array( [ 7.5, 0.5, 0.0 ] )
    L = 1.6
    for v, lab in ( ( mf.X, r"$X$" ), ( mf.Y, r"$Y$" ), ( mf.Z, r"$Z$" ) ):
        c.arrow( O, O + L * v, color = "k", lw = 1.0, z = 65, head = 6.0 )
        c.text( O + 1.25 * L * v, lab, z = 70 )
    mf.save( fig, "PtCloud_project", [ [ c ] ], hmax = 4.6 )


FIGS = { "project": fig_project }

if __name__ == "__main__":
    mf.style()
    for n in ( sys.argv[ 1: ] or FIGS.keys() ):
        FIGS[ n ]()
