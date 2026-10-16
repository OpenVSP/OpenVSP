"""Figures for the OpenVSP Hinge help page (src/help/Hinge.md), written to ../../images/Hinge_*.png.

    <build>/venv/bin/python hinge_figs.py

Uses the drawing helpers of ../propeller (figlib, and the views and save() of propeller_figs).  The wing is VSP's
own, sampled with CompVecPnt01 at each hinge angle.  The 3D view is OpenVSP's frame: Z up, Y into the page, X to
the right, seen from about (-1, -1, 1).
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )

import numpy as np
import matplotlib
matplotlib.use( "Agg" )
import matplotlib.pyplot as plt

import openvsp as vsp

import figlib as fl
from figlib import COL
import propeller_figs as pf

X = np.array( [ 1.0, 0, 0 ] )
Y = np.array( [ 0, 1.0, 0 ] )
Z = np.array( [ 0, 0, 1.0 ] )
O = np.zeros( 3 )

# The colors OpenVSP draws a frame's axes in: X red, Y green, Z blue.
AXCOL = ( "#E00000", "#00A000", "#0000E0" )
GREY = "#777777"

ROTATE = 40.0        # the deflection shown, degrees
SPAN = 4.0
CHORD = 1.5


def grid( gid, isurf, nu, nw ):
    us = np.linspace( 0.0, 1.0, nu )
    ws = np.linspace( 0.0, 1.0, nw )
    U, W = np.meshgrid( us, ws, indexing = "ij" )
    pts = vsp.CompVecPnt01( gid, isurf, list( U.ravel() ), list( W.ravel() ) )
    return np.array( [ [ p.x(), p.y(), p.z() ] for p in pts ] ).reshape( nu, nw, 3 )


def model():
    """A Hinge at the origin, its axis along its own X, and a wing panel hung off it, as a folding tip is."""
    vsp.VSPRenew()
    h = vsp.AddGeom( "HINGE" )
    w = vsp.AddGeom( "WING", h )
    xs = vsp.GetXSecSurf( w, 0 )
    vsp.SetDriverGroup( w, 1, vsp.SPAN_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
    vsp.SetParmVal( w, "Span", "XSec_1", SPAN )
    vsp.SetParmVal( w, "Root_Chord", "XSec_1", CHORD )
    vsp.SetParmVal( w, "Tip_Chord", "XSec_1", CHORD )
    vsp.SetParmVal( w, "Sweep", "XSec_1", 0.0 )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0 )
    vsp.SetParmVal( w, "X_Rel_Location", "XForm", -0.5 * CHORD )
    vsp.Update()
    return h, w


def wing_at( h, w, deg ):
    vsp.SetParmVal( h, "JointRotate", "Hinge", deg )
    vsp.Update()
    return grid( w, 0, 21, 41 )


def rotx( deg ):
    t = np.radians( deg )
    return np.array( [ [ 1, 0, 0 ], [ 0, np.cos( t ), -np.sin( t ) ], [ 0, np.sin( t ), np.cos( t ) ] ] )


def hinge_motion():
    """A wing panel on a Hinge, at Rotate 0 and at Rotate 40, with the Hinge's frame and the frame it carries."""
    h, w = model()
    G0 = wing_at( h, w, 0.0 )
    G1 = wing_at( h, w, ROTATE )
    # The child is carried rigidly: a right-handed turn about the hinge's X axis.
    assert np.abs( G0 @ rotx( ROTATE ).T - G1 ).max() < 1e-9, "the wing is not turned about +X"

    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = pf.panel( fig, pf.ISO )
    c.surface( G0, ghost = True, edge = True )
    c.surface( G1, edge = True, alpha = 0.75 )
    c.flush()

    L = 1.1
    # The axis the joint turns about and slides along, from the Hinge's origin: the Hinge's X, here the Axis.
    c.hinge( O, X, "k", half = 2.0, start = 0.0, rho = 0.3, at = 0.8, label = r"Axis $X$", label_dx = 22, label_dy = 2,
             lw = 1.2, z = 40 )
    # The Hinge's own frame, solid, and the frame its children are carried by, dashed, as the 3D view draws them.
    for i, v in enumerate( ( X, Y, Z ) ):
        c.arrow( O, L * v, color = AXCOL[ i ], lw = 1.3, head = 7 )
        c.arrow( O, L * rotx( ROTATE ) @ v, color = AXCOL[ i ], lw = 1.1, head = 7, ls = ( 0, ( 4, 2 ) ) )
    c.text( 1.15 * L * Y, r"$Y$", color = AXCOL[ 1 ], dx = -2, dy = 6 )
    c.text( 1.12 * L * Z, r"$Z$", color = AXCOL[ 2 ], dx = -8 )
    c.text( 1.15 * L * rotx( ROTATE ) @ Y, r"$Y'$", color = AXCOL[ 1 ], dx = 6, dy = 2 )
    c.text( 1.12 * L * rotx( ROTATE ) @ Z, r"$Z'$", color = AXCOL[ 2 ], dx = 10, dy = 4 )
    c.text( 1.05 * L * X, r"$X$", color = AXCOL[ 0 ], dx = 2, dy = -9 )
    # The angle, at the tip of the panel's leading edge.
    tip0 = G0[ -1, 0 ]
    tip1 = G1[ -1, 0 ]
    c0 = np.array( [ tip0[0], 0, 0 ] )
    c.angle( c0, tip0 - c0, tip1 - c0, np.linalg.norm( tip0 - c0 ), "k", label = "Rotate", label_r = 1.06, dx = -14, dy = 0 )
    c.line( np.array( [ c0, tip0 ] ), color = "#888888", lw = 0.6, ls = ( 0, ( 2, 2 ) ) )
    c.line( np.array( [ c0, tip1 ] ), color = "#888888", lw = 0.6, ls = ( 0, ( 2, 2 ) ) )
    pf.leader( c, G0[ 10, 20 ], "Rotate 0", ( 30, -30 ) )
    pf.leader( c, G1[ 12, 10 ], r"Rotate %g" % ROTATE, ( 60, 10 ) )
    pf.save( fig, "Hinge_motion", [ [ c ] ] )


# ====================================================================== orienting the Hinge by vectors

OR_P = np.array( [ 2.0, -4.0, 1.0 ] )  # the Hinge's place, on a left wing
OR_SWEEP = 30.0                          # the hinge line, swept back and with dihedral, degrees
OR_DIH = 25.0
OR_LA = 2.6                              # length of the hinge axis each way from the Hinge
OR_LG = 1.8                              # length of every other axis glyph


def or_axis():
    s, d = np.radians( OR_SWEEP ), np.radians( OR_DIH )
    return np.array( [ np.sin( s ), -np.cos( s ) * np.cos( d ), np.cos( s ) * np.sin( d ) ] )


def hinge_frame_vsp( h ):
    """The Hinge's X, Y and Z in model axes, read from Blank children one unit along each."""
    bs = []
    for k in range( 3 ):
        b = vsp.AddGeom( "BLANK", h )
        for i, n in enumerate( ( "X_Rel_Location", "Y_Rel_Location", "Z_Rel_Location" ) ):
            vsp.SetParmVal( b, n, "XForm", 1.0 if i == k else 0.0 )
        bs.append( b )
    vsp.Update()
    o = np.array( [ vsp.GetParmVal( h, n, "XForm" ) for n in ( "X_Location", "Y_Location", "Z_Location" ) ] )
    F = [ np.array( [ vsp.GetParmVal( b, n, "XForm" ) for n in ( "X_Location", "Y_Location", "Z_Location" ) ] ) - o
          for b in bs ]
    return o, F


def end_text( c, p, a, length, s, color, gap = 10.0, dx = 0.0, dy = 0.0 ):
    """A label just past the end of the glyph p + length a, set off along it as drawn."""
    q = c.v.p2( np.array( [ p, p + a ] ) )
    u = fl.unit( q[1] - q[0] )
    c.text( p + ( length + 0.25 ) * a, s, color = color, dx = gap * u[0] + dx, dy = gap * u[1] + dy )


def hinge_orient():
    """A Hinge oriented by Vectors: a VECTOR3D hinge axis along the swept hinge line of a left wing, Axis X, Reference Y, and the
    reference vector along the model's Z."""
    d = or_axis()
    vsp.VSPRenew()
    h = vsp.AddGeom( "HINGE" )
    for n, v in zip( ( "X_Rel_Location", "Y_Rel_Location", "Z_Rel_Location" ), OR_P ):
        vsp.SetParmVal( h, n, "XForm", v )
    vsp.SetParmVal( h, "OrientRotFlag", "Hinge", vsp.HINGE_ORIENT_VEC )
    vsp.SetParmVal( h, "PrimType", "Hinge", vsp.HINGE_PRIM_VECTOR3D )
    vsp.SetParmVal( h, "PrimVecAbsRelFlag", "Hinge", vsp.ABS )
    for n, v in zip( ( "PrimXVec", "PrimYVec", "PrimZVec" ), d ):
        vsp.SetParmVal( h, n, "Hinge", v )
    vsp.SetParmVal( h, "SecondaryVecDir", "Hinge", vsp.Z_DIR )
    vsp.SetParmVal( h, "SecVecAbsRelFlag", "Hinge", vsp.ABS )
    assert vsp.GetParmVal( h, "PrimaryDir", "Hinge" ) == vsp.X_DIR and vsp.GetParmVal( h, "SecondaryDir", "Hinge" ) == vsp.Y_DIR
    o, ( hx, hy, hz ) = hinge_frame_vsp( h )
    # The hinge axis is the direction; the Reference axis lies in the plane of the hinge axis and the reference vector,
    # square to the hinge axis; the third completes a right-handed set.
    yref = fl.unit( Z - ( Z @ d ) * d )
    assert np.abs( o - OR_P ).max() < 1e-9
    assert np.abs( hx - d ).max() < 1e-9, hx
    assert np.abs( hy - yref ).max() < 1e-9, hy
    assert np.abs( hz - np.cross( hx, hy ) ).max() < 1e-9, hz
    # The turn from the reference vector to the Reference axis is right-handed about the third axis.
    tilt = np.degrees( np.arccos( Z @ hy ) )
    assert np.abs( rotabout( hz, tilt ) @ Z - hy ).max() < 1e-9

    P = OR_P
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = pf.panel( fig, pf.ISO )
    # The plane of the hinge axis and the reference vector.
    A, B = 0.8 * OR_LA, 1.25 * OR_LG
    quad = np.array( [ P - A * d - 0.35 * B * hy, P + A * d - 0.35 * B * hy, P + A * d + B * hy, P - A * d + B * hy ] )
    q = c.v.p2( quad )
    c.ax.fill( q[ :, 0 ], q[ :, 1 ], color = "#E6E6EE", zorder = 2, lw = 0 )
    c.pts.append( q )
    c.text( P - 0.75 * A * d + 0.85 * B * hy, "plane of the hinge axis\nand the reference vector", color = "#666677",
            dx = -6, ha = "right", size = fl.FS )
    # The model's axes at their origin; the axes at the Hinge show directions only.
    pf.model_triad( c, O, length = 1.2 )
    # The reference vector (dashed) and the Reference axis it gives (solid), each through the Hinge both ways, and the
    # path of the tip of one vector turned from the first to the second.
    c.line( np.array( [ P - OR_LG * Z, P + OR_LG * Z ] ), color = GREY, lw = 1.3, ls = ( 0, ( 4, 2 ) ), z = 52 )
    c.arrow( P, P + ( OR_LG + 0.25 ) * Z, color = GREY, lw = 1.3, head = 8, ls = ( 0, ( 4, 2 ) ), z = 52 )
    end_text( c, P, Z, OR_LG, "reference vector,\nmodel $Z$", GREY, gap = 16 )
    c.line( np.array( [ P - OR_LG * hy, P ] ), color = AXCOL[ 1 ], lw = 1.6, z = 53 )
    c.arrow( P, P + ( OR_LG + 0.25 ) * hy, color = AXCOL[ 1 ], lw = 1.8, head = 9, z = 53 )
    end_text( c, P, hy, OR_LG, r"$Y$ (Reference)", AXCOL[ 1 ], gap = 12, dx = -30 )
    t = np.linspace( 0.0, 1.0, 30 )
    T = np.array( [ P + OR_LG * rotabout( hz, tilt * k ) @ Z for k in t ] )
    c.arrow_path( T, color = AXCOL[ 1 ], lw = 1.6, head = 8, z = 58 )
    c.text( T[ 0 ], "turned square\nto the hinge axis", color = AXCOL[ 1 ], dx = 10, dy = -20, ha = "left" )
    # The hinge axis, both ways through the Hinge.
    c.line( np.array( [ P - OR_LA * d, P ] ), color = AXCOL[ 0 ], lw = 2.2, z = 54 )
    c.arrow( P, P + ( OR_LA + 0.25 ) * d, color = AXCOL[ 0 ], lw = 2.4, head = 11, z = 54 )
    end_text( c, P, d, OR_LA, r"hinge axis, $X$ (Axis)", AXCOL[ 0 ], gap = 12, dx = 40 )
    # The third axis, about which the reference vector is turned.
    c.hinge( P, hz, AXCOL[ 2 ], half = OR_LG, start = 0.0, rho = 0.22, at = 0.6, lw = 1.3, z = 55 )
    end_text( c, P, hz, OR_LG, r"$Z$", AXCOL[ 2 ] )
    # Square: the Reference axis to the hinge axis.
    r = 0.28
    c.line( np.array( [ P + r * d, P + r * d + r * hy, P + r * hy ] ), color = "k", lw = 0.7, z = 56 )
    c.point( P, color = "k", s = 14 )
    c.text( P, "Hinge", dx = -10, dy = 12, ha = "right" )
    pf.save( fig, "Hinge_orient", [ [ c ] ] )


def rotabout( a, deg ):
    """The right-handed turn by deg about the unit vector a."""
    a = fl.unit( a )
    t = np.radians( deg )
    K = np.array( [ [ 0, -a[2], a[1] ], [ a[2], 0, -a[0] ], [ -a[1], a[0], 0 ] ] )
    return np.eye( 3 ) + np.sin( t ) * K + ( 1 - np.cos( t ) ) * K @ K


# ====================================================================== a chain: an aileron on a folding wing tip

CH_CHORD = 1.5
CH_INNER = 2.5          # span of the inner wing
CH_OUTER = 2.5          # span of the folding tip
CH_AIL = 0.3            # the aileron's fraction of the chord
CH_FOLD = 60.0
CH_DEFL = -20.0
CH_MAG = 3.0           # enlargement of the section view


def ch_wing( parent, span, chord ):
    w = vsp.AddGeom( "WING", parent )
    vsp.SetDriverGroup( w, 1, vsp.SPAN_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
    vsp.SetParmVal( w, "Span", "XSec_1", span )
    vsp.SetParmVal( w, "Root_Chord", "XSec_1", chord )
    vsp.SetParmVal( w, "Tip_Chord", "XSec_1", chord )
    vsp.SetParmVal( w, "Sweep", "XSec_1", 0.0 )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0 )
    return w


def ch_attach( g ):
    vsp.SetParmVal( g, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( g, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_COMP )


def ch_model():
    """An inner wing; a fold Hinge (Axis X) at its tip leading edge carrying the outer panel; an aileron Hinge (Axis Y)
    on the outer panel's trailing edge carrying the aileron."""
    vsp.VSPRenew()
    w1 = ch_wing( "", CH_INNER, CH_CHORD )
    h1 = vsp.AddGeom( "HINGE", w1 )
    ch_attach( h1 )
    vsp.SetParmVal( h1, "Y_Rel_Location", "XForm", CH_INNER )
    w2 = ch_wing( h1, CH_OUTER, ( 1.0 - CH_AIL ) * CH_CHORD )
    h2 = vsp.AddGeom( "HINGE", w2 )
    ch_attach( h2 )
    vsp.SetParmVal( h2, "X_Rel_Location", "XForm", ( 1.0 - CH_AIL ) * CH_CHORD )
    vsp.SetParmVal( h2, "PrimaryDir", "Hinge", vsp.Y_DIR )
    w3 = ch_wing( h2, CH_OUTER, CH_AIL * CH_CHORD )
    vsp.Update()
    return w1, h1, w2, h2, w3


def ch_pose( m, fold, defl ):
    w1, h1, w2, h2, w3 = m
    vsp.SetParmVal( h1, "JointRotate", "Hinge", fold )
    vsp.SetParmVal( h2, "JointRotate", "Hinge", defl )
    vsp.Update()
    return [ grid( g, 0, 21, 41 ) for g in ( w1, w2, w3 ) ]


def hinge_chain( v = None, name = "Hinge_chain" ):
    """The wing at fold 0, aileron 0 (dashed) and at fold CH_FOLD, aileron CH_DEFL."""
    if v is None:
        v = CH_VIEW
    m = ch_model()
    G0 = ch_pose( m, 0.0, 0.0 )
    Gm = ch_pose( m, CH_FOLD, 0.0 )
    G1 = ch_pose( m, CH_FOLD, CH_DEFL )
    # The fold turns the tip right-handed about X; the aileron turns about its own hinge line as the fold has carried it.
    fp = np.array( [ 0, CH_INNER, 0 ] )
    for k in ( 1, 2 ):
        assert np.abs( ( G0[ k ] - fp ) @ rotx( CH_FOLD ).T + fp - Gm[ k ] ).max() < 1e-9
    hp = G1[ 2 ][ 0, 20 ]                   # the aileron's root leading edge: on its hinge line
    ha = fl.unit( G1[ 2 ][ -1, 20 ] - hp )  # the hinge line, along the aileron's leading edge
    assert np.abs( ha - rotx( CH_FOLD ) @ Y ).max() < 1e-9
    assert np.abs( G1[ 2 ] - ( ( Gm[ 2 ] - hp ) @ rotabout( ha, CH_DEFL ).T + hp ) ).max() < 1e-9
    te0 = Gm[ 2 ][ -1, 0 ]
    te1 = G1[ 2 ][ -1, 0 ]
    # Aileron CH_DEFL < 0 lifts the trailing edge: it moves to the upper side of the folded panel.
    assert ( te1 - te0 ) @ ( rotx( CH_FOLD ) @ Z ) > 0.0

    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = pf.panel( fig, v )
    c.surface( G0[ 0 ], edge = True )
    for G in G0[ 1: ]:
        c.surface( G, ghost = True, edge = True )
    for G in G1[ 1: ]:
        c.surface( G, edge = True, alpha = 0.9 )
    c.surface( G1[ 2 ], color = COL[ "lag" ], edge = True )
    c.flush()
    # The fold hinge along X at the inner wing's tip, and the aileron hinge along the folded panel's trailing edge.
    c.hinge( fp, X, COL[ "fold" ], half = 0.9, start = -0.9, rho = 0.18, at = 0.25, lw = 1.4, label = "fold Hinge", label_dx = 30 )
    c.hinge( hp + 0.5 * CH_OUTER * ha, ha, COL[ "flap" ], half = 0.5 * CH_OUTER + 0.3, rho = 0.18, at = 0.9, lw = 1.4,
             label = "aileron Hinge", label_dx = 34 )
    c.angle( fp, G0[ 1 ][ -1, 20 ] - fp, Gm[ 1 ][ -1, 20 ] - fp, 0.75 * CH_OUTER, COL[ "fold" ],
             label = "Fold %g" % CH_FOLD, label_r = 1.12, lw = 1.3 )

    # Below, enlarged: the folded tip's section at mid span, seen outboard along the aileron's folded hinge line.
    nrm = rotx( CH_FOLD ) @ Z
    d = pf.view( X, nrm )
    c2 = pf.panel( fig, d )
    i = 10
    sp = Gm[ 2 ][ i, 20 ]                       # the aileron hinge line at this section

    def big( P ):
        return sp + CH_MAG * ( np.asarray( P ) - sp )
    for P, col in ( ( G1[ 1 ][ i ], COL[ "surf" ] ), ( G1[ 2 ][ i ], COL[ "lag" ] ) ):
        q = c2.v.p2( big( P ) )
        c2.ax.fill( q[ :, 0 ], q[ :, 1 ], color = col, ec = "#4a4a4a", lw = 0.6, zorder = 5 )
        c2.pts.append( q )
    c2.line( big( Gm[ 2 ][ i ] ), color = "#555555", lw = 0.9, ls = ( 0, ( 1, 1.5 ) ), z = 6 )
    s0 = Gm[ 2 ][ i, 0 ]
    s1 = G1[ 2 ][ i, 0 ]
    assert ( s1 - s0 ) @ nrm > 0.0
    c2.angle( big( sp ), s0 - sp, s1 - sp, CH_MAG * np.linalg.norm( s0 - sp ) * 1.06, COL[ "flap" ], lw = 1.3 )
    c2.text( big( s1 ), "Aileron %g" % CH_DEFL, color = COL[ "flap" ], dx = -10, dy = 12 )
    c2.text( big( s0 ), "Aileron 0", color = "#555555", dx = -6, dy = -14 )
    fl.out_of_page( c2.ax, c2.v.p2( big( sp ) ), "", color = COL[ "flap" ], inward = True )
    c2.text( big( sp ), "aileron Hinge", color = COL[ "flap" ], dx = 0, dy = -16 )
    c2.text( big( G1[ 1 ][ i, 10 ] ), "outer panel", dx = 0, dy = 14 )
    pf.save( fig, name, [ [ c ], [ c2 ] ], hmax = 7.0 )


CH_VIEW = pf.ISO

FIGS = { "motion": ( hinge_motion, ), "orient": ( hinge_orient, ), "chain": ( hinge_chain, ) }

if __name__ == "__main__":
    names = sys.argv[ 1: ] or list( FIGS )
    pf.style()
    for n in names:
        for fn in FIGS[ n ]:
            fn()
