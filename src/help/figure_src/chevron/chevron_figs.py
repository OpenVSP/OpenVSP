"""Figures for Chevron.md: chevrons on the end of a cylinder, their waveform, Angle and Slew.

    <build>/venv/bin/python chevron_figs.py [3d waveform shaping]

The drawing helpers are shared with the other body pages, in ../fuselagestack/bodyfig.py.
"""

import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "fuselagestack" ) )

import numpy as np
import matplotlib.pyplot as plt

import bodyfig as bf
from bodyfig import vsp, FS, X, Y, Z
import figlib as fl


def tube():
    """A cylinder Stack, open at its aft end, ready for chevrons on section 1."""
    bf.renew()
    s = vsp.AddGeom( "STACK" )
    vsp.Update()
    vsp.InitStackPreset( s, vsp.STACK_PRESET_CYLINDER_ENDCAPS )
    vsp.SetParmVal( s, "CapUMaxOption", "EndCap", vsp.NO_END_CAP )
    vsp.SetParmVal( s, "CapUMinOption", "EndCap", vsp.NO_END_CAP )
    vsp.Update()
    return s


def chev( s, **kw ):
    for k, v in kw.items():
        bf.setx( s, 1, k, v, "Chevron" )
    vsp.Update()


def chevron3d():
    """Chevron_3d: eight full chevrons on the end of a cylinder."""
    s = tube()
    chev( s, Chevron_Type = vsp.CHEVRON_FULL, Number = 8, TopAmplitude = 0.5, On_Duty = 0.1, Off_Duty = 0.1,
          Peak_Radius = 0.1, Valley_Radius = 0.1 )
    G = bf.grid( s, 0, u = np.linspace( 0.0, 1.0, 41 ), w = np.linspace( 0.0, 1.0, 321 ) )
    assert G[ ..., 0 ].max() > 3.49
    fig = bf.figure()
    c = bf.canvas( fig, bf.ISO )
    c.surface( G, edge = False )
    c.flush()
    edge = G[ -1 ]
    c.line( edge, color = "#222222", lw = 1.4, z = 40 )
    c.line( G[ 0 ], color = "#4a4a4a", lw = 0.8, z = 40 )
    bf.triad( c, np.array( [ -0.6, 1.0, -2.2 ] ), length = 1.0 )
    bf.save( fig, "Chevron_3d", [ [ c ] ], hmax = 3.6 )


def waveform():
    """Chevron_waveform: how far each point of the section moves, around the section."""
    s = tube()
    chev( s, Chevron_Type = vsp.CHEVRON_PARTIAL, Number = 3, TopAmplitude = 0.5, On_Duty = 0.2, Off_Duty = 0.3,
          W01_Start = 0.25, W01_End = 0.75 )
    w = np.linspace( 0.0, 1.0, 1601 )
    dx = np.array( [ vsp.CompPnt01( s, 0, 1.0, a ).x() for a in w ] ) - 3.0
    assert abs( dx.max() - 0.5 ) < 1e-6
    bf.style()
    fig = plt.figure( figsize = ( fl.PAGE_W, 2.9 ) )
    ax = fig.add_axes( [ 0.09, 0.17, 0.88, 0.70 ] )
    ax.plot( w, dx, color = "#0072B2", lw = 1.8 )
    ax.set_xlim( 0, 1 )
    ax.set_ylim( -0.05, 0.72 )
    ax.set_xticks( [ 0, 0.25, 0.5, 0.75, 1.0 ] )
    ax.set_xticklabels( [ "0\nright", "0.25\nbottom", "0.5\nleft", "0.75\ntop", "1\nright" ] )
    ax.set_xlabel( r"$W$", fontsize = FS )
    ax.set_ylabel( "move along the body", fontsize = FS )
    ax.tick_params( labelsize = FS )
    for sp in ( "top", "right" ):
        ax.spines[ sp ].set_visible( False )
    # One period: 1/6 of W here; its parts.
    per = 0.5 / 3.0
    t0 = 0.25
    off2 = 0.5 * 0.3 * per
    on = 0.2 * per
    rise = 0.5 * ( 1.0 - 0.2 - 0.3 ) * per
    marks = [ t0, t0 + off2, t0 + off2 + rise, t0 + off2 + rise + on, t0 + off2 + 2 * rise + on, t0 + per ]
    for m in marks:
        ax.plot( [ m, m ], [ -0.05, 0.6 ], color = "#999999", lw = 0.6, ls = ( 0, ( 2, 2 ) ), zorder = 1 )
    def span( a, b, y, s ):
        ax.annotate( "", ( a, y ), ( b, y ), arrowprops = dict( arrowstyle = "<|-|>", lw = 0.7, color = "k",
                                                               mutation_scale = 6, shrinkA = 0, shrinkB = 0 ) )
        ax.text( 0.5 * ( a + b ), y + 0.025, s, ha = "center", va = "bottom", fontsize = FS )
    span( marks[2], marks[3], 0.57, "On" )
    span( marks[0], marks[1], 0.12, "Off/2" )
    span( marks[4], marks[5], 0.12, "Off/2" )
    span( marks[0], marks[5], 0.66, "one period" )
    ax.annotate( "W Start", ( 0.25, 0.0 ), ( 0.17, 0.30 ), fontsize = FS, ha = "center",
                 arrowprops = dict( arrowstyle = "-|>", lw = 0.7, color = "k", mutation_scale = 6 ) )
    ax.annotate( "W End", ( 0.75, 0.0 ), ( 0.83, 0.30 ), fontsize = FS, ha = "center",
                 arrowprops = dict( arrowstyle = "-|>", lw = 0.7, color = "k", mutation_scale = 6 ) )
    ax.axhline( 0.5, color = "#999999", lw = 0.6, ls = ( 0, ( 4, 2 ) ), xmin = 0.0, xmax = 0.24 )
    ax.text( 0.01, 0.5, "Amplitude", va = "bottom", fontsize = FS )
    bf.save_plot( fig, "Chevron_waveform" )


def tips( s, n = 3200 ):
    """The tips of the teeth on the end of the tube: the points of the end edge farthest aft, ( k, 3 )."""
    w = np.linspace( 0.0, 1.0, n + 1 )[ :-1 ]
    E = np.array( [ list( vsp.CompPnt01( s, 0, 1.0, a ) ) for a in w ] )
    k = [ i for i in range( n ) if E[i, 0] >= E[i - 1, 0] and E[i, 0] > E[ ( i + 1 ) % n, 0 ] and E[i, 0] > 3.3 ]
    T = E[ k ]
    # One tip per tooth: a rounded peak can top out at two neighboring points.
    keep = [ T[0] ]
    for p in T[ 1: ]:
        if np.linalg.norm( p - keep[-1] ) > 0.1 and np.linalg.norm( p - keep[0] ) > 0.1:
            keep.append( p )
    assert len( keep ) == 8, len( keep )
    return np.array( keep )


def shaped( angle = 0.0, slew = 0.0 ):
    """The eight teeth of Chevron_3d, turned so a tooth peaks at the top, with the given Angle and Slew on every side."""
    s = tube()
    chev( s, Chevron_Type = vsp.CHEVRON_FULL, Number = 8, TopAmplitude = 0.5, On_Duty = 0.1, Off_Duty = 0.1,
          Peak_Radius = 0.1, Valley_Radius = 0.1, W01_Start = 0.75 - 1.0 / 16.0, Top_Angle = angle, Top_Slew = slew )
    return s


def shaping():
    """Chevron_shaping: Angle 20 from the side, Slew 20 from aft."""
    fig = bf.figure()
    amp = 0.5
    r = 1.25
    # Angle: each tooth turns outward about its base on the section, so its tip moves out by A sin(Angle).
    s0 = shaped()
    T0 = tips( s0 )
    # The near half, -Y (W 0.25 to 0.75), so the top and bottom teeth are seen in profile.
    near = np.linspace( 0.25, 0.75, 321 )
    E0 = np.array( [ list( vsp.CompPnt01( s0, 0, 1.0, a ) ) for a in near ] )
    s = shaped( angle = 20.0 )
    T = tips( s )
    a = np.radians( 20.0 )
    assert np.abs( np.hypot( T[:, 1], T[:, 2] ) - ( r + amp * np.sin( a ) ) ).max() < 1e-3
    assert np.abs( T[:, 0] - ( 3.0 + amp * np.cos( a ) ) ).max() < 1e-3
    G = bf.grid( s, 0, u = np.linspace( 0.5, 1.0, 41 ), w = near )
    c = bf.canvas( fig, bf.SIDE )
    c.surface( G, edge = False )
    c.flush()
    c.line( G[ -1 ], color = "#222222", lw = 1.2, z = 40 )
    c.line( E0, color = bf.GHOST, lw = 0.8, ls = ( 0, ( 4, 2 ) ), z = 41 )
    base = np.array( [ 3.0, 0.0, r ] )
    tip = T[ np.argmax( T[:, 2] ) ]
    assert abs( tip[1] ) < 0.02
    c.line( np.array( [ base, base + 0.8 * X ] ), color = bf.GHOST, lw = 0.7, ls = ( 0, ( 2, 2 ) ), z = 45 )
    c.line( np.array( [ base, base + 0.8 * ( tip - base ) / np.linalg.norm( tip - base ) ] ), color = "#0072B2", lw = 0.9, z = 46 )
    c.angle( base, X, tip - base, 0.65, "#0072B2", label = "Angle", label_r = 1.0, dx = 28, dy = 6 )
    bf.title( c, "Angle 20", pad = 10 )
    bf.key( c, "side", ( 1.0, 0.0 ), 10, 0 )
    # Slew: seen from aft, each tip moves around the section.
    s = shaped( slew = 20.0 )
    T = tips( s )
    E = np.array( [ list( vsp.CompPnt01( s, 0, 1.0, a ) ) for a in np.linspace( 0.0, 1.0, 641 ) ] )
    AFT = bf.fl.View.plane( Y, Z )
    bf.KEY.setdefault( "aft", ( r"$Y$", r"$Z$", r"$X$" ) )
    d = bf.canvas( fig, AFT )
    d.line( E, color = "#222222", lw = 1.0, z = 30 )
    pairs = []
    for p in T:
        q = T0[ np.argmin( np.linalg.norm( T0[:, 1:] - p[ 1: ], axis = 1 ) ) ]
        pairs.append( ( q, p ) )
        d.point( q, bf.GHOST, s = 8, z = 50 )
        if np.linalg.norm( p[ 1: ] - q[ 1: ] ) > 0.03:
            d.arrow( q, p, color = "#0072B2", lw = 1.1, head = 6.0, z = 55 )
    side = { "top": ( 0.0, 1.0 ), "right": ( 1.0, 0.0 ), "bottom": ( 0.0, -1.0 ), "left": ( -1.0, 0.0 ) }
    move = {}
    for name, ( y, z ) in side.items():
        q, p = min( pairs, key = lambda qp: np.hypot( qp[0][1] - r * y, qp[0][2] - r * z ) )
        move[ name ] = p - q
    # Positive slew: the top and bottom tips toward +Y, the right and left tips toward -Z.
    assert move[ "top" ][1] > 0.1 and move[ "bottom" ][1] > 0.1
    assert move[ "right" ][2] < -0.1 and move[ "left" ][2] < -0.1
    q, p = min( pairs, key = lambda qp: -qp[0][2] )
    bf.label( d, p, r"$+Y$", dx = 4, dy = -12, color = "#0072B2", ha = "left" )
    q, p = min( pairs, key = lambda qp: -qp[0][1] )
    bf.label( d, p, r"$-Z$", dx = 10, dy = -2, color = "#0072B2", ha = "left" )
    # The tooth at the lower right hardly moves: its two neighbors' slews cancel there.
    q, p = min( pairs, key = lambda qp: np.hypot( qp[0][1] - 0.88, qp[0][2] + 0.88 ) )
    assert np.linalg.norm( p[ 1: ] - q[ 1: ] ) < 0.03
    # The tooth at the upper right, between two sides that lean the same way, moves the most.
    big = max( pairs, key = lambda qp: np.linalg.norm( qp[1][ 1: ] - qp[0][ 1: ] ) )
    assert big[0][1] > 0.5 and big[0][2] > 0.5 or big[0][1] < -0.5 and big[0][2] < -0.5
    bf.label( d, q, "tip at Slew 0", dx = -8, dy = 2, color = "#666666", ha = "right" )
    bf.title( d, "Slew 20", pad = 10 )
    bf.key( d, "aft", ( 1.0, 0.0 ), 10, 0 )
    bf.save( fig, "Chevron_shaping", [ [ c, d ] ], hmax = 4.0 )


FIGS = { "3d": chevron3d, "waveform": waveform, "shaping": shaping }

if __name__ == "__main__":
    for name in ( sys.argv[ 1: ] or FIGS ):
        FIGS[ name ]()
