"""Figures for the OpenVSP Wing help page (src/help/Wing.md).

    <build>/venv/bin/python wing_figs.py [names...]      # writes ../../images/Wing_*.png

Every surface is VSP's own, sampled with CompPnt01.  3D views have Z up, Y into the page and X to the right, seen from
about (-1, -1, 1).  2D views keep that frame: the top view looks down -Z (X right, Y up), the side view looks along
+Y (X right, Z up), and the view from upstream looks along +X (-Y right, Z up).
"""

import os
import sys

import numpy as np
import matplotlib
matplotlib.use( "Agg" )
import matplotlib.pyplot as plt
from matplotlib.transforms import offset_copy

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )

import figlib as fl                     # noqa: E402  (shared drawing style, from the propeller figures)
from figlib import COL, FS              # noqa: E402
import openvsp as vsp                   # noqa: E402

import wingmodel as wm                  # noqa: E402

OUT = os.path.join( HERE, "..", "..", "images" )
DPI = 200

X = np.array( [ 1.0, 0, 0 ] )
Y = np.array( [ 0, 1.0, 0 ] )
Z = np.array( [ 0, 0, 1.0 ] )
O = np.zeros( 3 )

ISO = fl.View.plane( [ 1, -1, 0 ], [ 1, 1, 2 ] )
TOP = fl.View.plane( X, Y )
SIDE = fl.View.plane( X, Z )
UPSTREAM = fl.View.plane( -Y, Z )
KEY = { "top": ( r"$X$", r"$Y$", r"$Z$" ), "side": ( r"$X$", r"$Z$", r"$-Y$" ), "upstream": ( r"$-Y$", r"$Z$", r"$-X$" ) }

C_SWEEP = COL[ "shaft" ]
C_DIH = COL[ "precone" ]
C_TWIST = COL[ "feather" ]
C_DIM = "#333333"
C_FOIL = "#C0392B"
C_BLEND = "#C000C0"      # the fuchsia the 3D view draws blending vectors in, darkened for print
C_REF = "#888888"


def new():
    fl.style()
    return plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )


def panel( fig, v ):
    return fl.Canvas( fig.add_axes( [ 0, 0, 1, 1 ] ), v )


def save( fig, name, rows, scale = None, hmax = fl.HMAX ):
    fl.fit( fig, rows, scale = scale, hmax = hmax )
    os.makedirs( OUT, exist_ok = True )
    fig.savefig( os.path.join( OUT, name + ".png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made", name )


def key( c, which, at = ( 1.0, 0.0 ), dx = 0.0, dy = 0.0 ):
    """A small 2D axis key, dx, dy points from the point at fractions 'at' of the canvas's content box."""
    lo, hi = c.limits()
    q = lo + np.asarray( at, float ) * ( hi - lo )
    fig = c.ax.figure

    def T( x, y ):
        return offset_copy( c.ax.transData, fig, x = dx + x, y = dy + y, units = "points" )
    L = 22.0
    r, u, o = KEY[ which ]
    for ( x, y ), lab, ha, va, tx, ty in ( ( ( L, 0 ), r, "left", "center", L + 3, 0 ), ( ( 0, L ), u, "center", "bottom", 0, L + 2 ) ):
        c.ax.annotate( "", xy = q, xycoords = T( x, y ), xytext = q, textcoords = T( 0, 0 ), annotation_clip = False,
                       arrowprops = dict( arrowstyle = "-|>", lw = 0.8, color = "k", mutation_scale = 6, shrinkA = 0, shrinkB = 0 ) )
        c.ax.text( q[0], q[1], lab, transform = T( tx, ty ), ha = ha, va = va )
    c.ax.plot( [ q[0] ], [ q[1] ], transform = T( 0, 0 ), marker = "o", ms = 5.5, mfc = "white", mec = "k", mew = 0.7, zorder = 90 )
    c.ax.plot( [ q[0] ], [ q[1] ], transform = T( 0, 0 ), marker = "o", ms = 1.4, mfc = "k", mec = "k", zorder = 91 )
    c.ax.text( q[0], q[1], o, transform = T( -5, -5 ), ha = "right", va = "top" )


def title( c, s, pad = 10.0 ):
    lo, hi = c.limits()
    c.ax.annotate( s, xy = ( 0.5 * ( lo[0] + hi[0] ), hi[1] ), xytext = ( 0, pad ), textcoords = "offset points", ha = "center",
                   va = "bottom", annotation_clip = False )


def dim( c, p0, p1, text, color = C_DIM, off = None, dx = 0, dy = 0, ha = "center" ):
    """A dimension line between p0 and p1 (arrows both ends), with its text at the middle."""
    c.arrow( p0, p1, color = color, lw = 0.9, head = 6.0, both = True )
    c.text( 0.5 * ( p0 + p1 ), text, color = color, dx = dx, dy = dy, ha = ha )


def ext( c, p0, p1, color = C_REF ):
    """A thin extension line."""
    c.line( np.array( [ p0, p1 ] ), color = color, lw = 0.6, z = 40 )


def angle( c, p, v0, v1, rho, color, **kw ):
    """An angle arc about p in the plane of a 2D view: both directions are projected onto the view's plane, so the arc
    is a circle about p as seen."""
    v0 = np.asarray( v0, float )
    v1 = np.asarray( v1, float )
    v0 = v0 - ( v0 @ c.v.out ) * c.v.out
    v1 = v1 - ( v1 @ c.v.out ) * c.v.out
    c.angle( p, v0, v1, rho, color, **kw )


def dot( c, p, color = "k", ms = 5 ):
    q = c.v.p2( p )
    c.ax.plot( [ q[0] ], [ q[1] ], marker = "o", ms = ms, color = color, zorder = 75, mec = "white", mew = 0.6 )


def outline( c, G, color = "#333333", lw = 1.0 ):
    """Planform outline of a sampled surface seen from above: LE, TE and the end airfoils."""
    nw = G.shape[1]
    c.line( G[ :, nw // 2 ], color = color, lw = lw, z = 52 )
    c.line( G[ :, 0 ], color = color, lw = lw, z = 52 )


def fill_planform( c, G, color = None, alpha = 1.0 ):
    nw = G.shape[1]
    P = np.vstack( [ G[ :, nw // 2 ], G[ ::-1, 0 ] ] )
    Q = c.v.p2( P )
    c.pts.append( Q )
    c.ax.fill( Q[:, 0], Q[:, 1], facecolor = color or COL[ "surf" ], edgecolor = "none", alpha = alpha, zorder = 20 )


# ====================================================================== sections and airfoils

def cranked():
    """A two-section wing: an inboard panel and a swept, tapered outboard panel with dihedral."""
    w = wm.new_wing( sym = True )
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    wm.setx( w, 1, "Span", 3.0 )
    wm.setx( w, 1, "Sweep", 15.0 )
    wm.setx( w, 1, "Tip_Chord", 3.2 )
    wm.setx( w, 2, "Span", 7.0 )
    wm.setx( w, 2, "Tip_Chord", 1.2 )
    wm.setx( w, 2, "Sweep", 30.0 )
    wm.setx( w, 2, "Dihedral", 6.0 )
    return w


def parts():
    """Wing_parts: airfoils and the sections between them, on a two-section wing."""
    w = cranked()
    fig = new()
    c = panel( fig, ISO )
    for s in ( 0, 1 ):
        c.surface( wm.grid( w, 0, 2, nu = 41, nw = 61, surf = s ), outline_lw = 0.5 )
    c.flush()
    n = wm.nsect( w )
    for i in range( n + 1 ):
        F = wm.foil( w, i, 161 )
        c.line( F, color = C_FOIL, lw = 1.8, z = 60 )
    labels = { 0: ( "airfoil 0 (root)", 10, -4, "left" ), 1: ( "airfoil 1", 10, 0, "left" ), 2: ( "airfoil 2 (tip)", 10, 4, "left" ) }
    for i, ( s, dx, dy, ha ) in labels.items():
        le, te = wm.lete( w, i )
        c.text( te, s, color = C_FOIL, dx = dx, dy = dy, ha = ha )
    for k in ( 1, 2 ):
        G = wm.grid( w, k - 1, k, nu = 3, nw = 61 )
        p = 0.4 * G[ 1, 0 ] + 0.6 * G[ 1, 30 ]
        c.text( p, "section %d" % k, dx = 0, dy = 0 )
    save( fig, "Wing_parts", [ [ c ] ] )


def shared_chord():
    """Wing_shared_chord: a two-section wing before and after a change to section 1's Tip C; section 2, driven by AR,
    Area and Taper, keeps its span and tip chord."""
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    vsp.SetDriverGroup( w, 2, vsp.AR_WSECT_DRIVER, vsp.AREA_WSECT_DRIVER, vsp.TAPER_WSECT_DRIVER )
    vsp.Update()
    names = ( "Span", "Root_Chord", "Tip_Chord", "Area", "Aspect", "Taper" )
    q0 = { n: wm.getx( w, 2, n ) for n in names }
    ct0 = wm.getx( w, 1, "Tip_Chord" )
    G0 = wm.grid( w, 0, 2, nu = 5, nw = 41 )
    E0 = [ wm.lete( w, i ) for i in range( 3 ) ]
    CT = 3.5
    wm.setx( w, 1, "Tip_Chord", CT )
    q1 = { n: wm.getx( w, 2, n ) for n in names }
    G1 = wm.grid( w, 0, 2, nu = 5, nw = 41 )
    E1 = [ wm.lete( w, i ) for i in range( 3 ) ]
    # Section 2 keeps its span and tip chord, takes the new root chord, and its drivers are recomputed.
    assert np.allclose( [ q1[ "Span" ], q1[ "Tip_Chord" ], q1[ "Root_Chord" ] ], [ q0[ "Span" ], q0[ "Tip_Chord" ], CT ] )
    assert np.allclose( E1[2], E0[2] ) and np.allclose( E1[1][0], E0[1][0] ) and np.allclose( E1[0], E0[0] )
    assert np.isclose( q1[ "Area" ], q1[ "Span" ] * ( CT + q1[ "Tip_Chord" ] ) / 2 )
    assert tuple( vsp.GetDriverGroup( w, 2 ) ) == ( vsp.AR_WSECT_DRIVER, vsp.AREA_WSECT_DRIVER, vsp.TAPER_WSECT_DRIVER )
    fig = new()
    c = panel( fig, TOP )
    fill_planform( c, G1 )
    outline( c, G1 )
    nw = G0.shape[1]
    c.line( G0[ :, 0 ], color = "#333333", lw = 1.0, ls = ( 0, ( 4, 2 ) ), z = 54 )
    for i in range( 3 ):
        c.line( np.array( E1[i] ), color = C_FOIL, lw = 2.0, z = 56 )
        c.text( E1[i][1], "airfoil %d" % i, color = C_FOIL, dx = 8, dy = 0, ha = "left" )
    # Airfoil 1's chord before (dashed, above) and after (below).
    o = 0.45
    le1 = E1[1][0]
    dim( c, le1 + o * Y, E0[1][1] + o * Y, "Tip C %g" % ct0, color = C_REF, dy = 8 )
    ext( c, E0[1][1], E0[1][1] + 1.3 * o * Y )
    dim( c, le1 - o * Y, E1[1][1] - o * Y, "Tip C %g" % CT, dy = -8 )
    ext( c, le1 + 1.3 * o * Y, le1 - 1.3 * o * Y )
    ext( c, E1[1][1], E1[1][1] - 1.3 * o * Y )
    m1 = 0.25 * ( E1[0][0] + E1[0][1] + E1[1][0] + E1[1][1] )
    c.text( m1, "section 1\nSpan, Root C, Tip C", ha = "center" )
    m2 = 0.25 * ( E1[1][0] + E1[1][1] + E1[2][0] + E1[2][1] )
    c.text( m2 - 0.3 * X + 0.6 * Y, "section 2\nAR, Area, Taper", ha = "center" )

    def r( v ):
        return round( v, 3 )
    c.text( E1[2][1] + 1.0 * X - 1.0 * Y,
            "section 2:\nSpan %g and Tip C %g kept\nArea %g to %g\nAR %g to %g\nTaper %g to %g"
            % ( q1[ "Span" ], q1[ "Tip_Chord" ], r( q0[ "Area" ] ), r( q1[ "Area" ] ), r( q0[ "Aspect" ] ), r( q1[ "Aspect" ] ),
                r( q0[ "Taper" ] ), r( q1[ "Taper" ] ) ), ha = "left", va = "top", linespacing = 1.3 )
    key( c, "top", ( 1.0, 0.0 ), 10, 0 )
    save( fig, "Wing_shared_chord", [ [ c ] ], hmax = 5.0 )


# ====================================================================== one section's planform

def planform():
    """Wing_planform: the default wing section seen from above, with the quantities that define it."""
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    b = wm.getx( w, 1, "Span" )
    cr = wm.getx( w, 1, "Root_Chord" )
    ct = wm.getx( w, 1, "Tip_Chord" )
    G = wm.grid( w, 0, 1, nu = 3, nw = 41 )
    le0, te0 = wm.lete( w, 0 )
    le1, te1 = wm.lete( w, 1 )
    assert np.allclose( [ le1[1], cr, ct ], [ b, np.linalg.norm( te0 - le0 ), np.linalg.norm( te1 - le1 ) ] )
    fig = new()
    c = panel( fig, TOP )
    fill_planform( c, G )
    outline( c, G )
    c.line( np.array( [ le0, te0 ] ), color = C_FOIL, lw = 2.0, z = 55 )
    c.line( np.array( [ le1, te1 ] ), color = C_FOIL, lw = 2.0, z = 55 )
    c.text( te0, "airfoil 0", color = C_FOIL, dx = 8, dy = -10, ha = "left" )
    c.text( te1, "airfoil 1", color = C_FOIL, dx = 8, dy = 0, ha = "left" )
    c.text( 0.5 * ( le0 + te1 ) + 0.4 * X, "Area", dx = 0, dy = 0 )
    # Chords at each end, set off upstream of the airfoils.
    dim( c, le0 - 1.0 * Y, te0 - 1.0 * Y, "Root C", dy = -10 )
    ext( c, le0, le0 - 1.3 * Y )
    ext( c, te0, te0 - 1.3 * Y )
    dim( c, le1 + 1.0 * Y, te1 + 1.0 * Y, "Tip C", dy = 10 )
    ext( c, le1, le1 + 1.3 * Y )
    ext( c, te1, te1 + 1.3 * Y )
    # Span, at the side, perpendicular to the root chord.
    xs = le0[0] - 1.2
    dim( c, np.array( [ xs, 0, 0 ] ), np.array( [ xs, b, 0 ] ), "Span", dx = -6, ha = "right" )
    ext( c, le0, np.array( [ xs - 0.3, 0, 0 ] ) )
    ext( c, le1, np.array( [ xs - 0.3, b, 0 ] ) )
    # Sweep at Sweep Loc (the LE by default) and Sec Sweep at Sec SW Loc (the TE by default), from the Y direction.
    R = 5.0
    c.line( np.array( [ le0, le0 + 1.15 * R * Y ] ), color = C_REF, lw = 0.7, ls = ( 0, ( 4, 2 ) ), z = 45 )
    angle( c, le0, Y, le1 - le0, R, C_SWEEP, label = "Sweep", label_r = 1.0, dx = 4, dy = 14, lw = 1.4 )
    c.line( np.array( [ te0, te0 + 1.15 * R * Y ] ), color = C_REF, lw = 0.7, ls = ( 0, ( 4, 2 ) ), z = 45 )
    angle( c, te0, Y, te1 - te0, R, C_SWEEP, label = "Sec Sweep", label_r = 1.0, dx = 44, dy = 8, lw = 1.4 )
    c.text( 0.2 * le0 + 0.8 * le1, "Sweep Loc = 0 (LE)", color = C_SWEEP, dx = -10, dy = 0, ha = "right" )
    c.text( 0.75 * te0 + 0.25 * te1, "Sec SW Loc = 1 (TE)", color = C_SWEEP, dx = 8, dy = 0, ha = "left" )
    key( c, "top", ( 1.0, 0.0 ), 10, 0 )
    save( fig, "Wing_planform", [ [ c ] ], hmax = 5.0 )


def sweep_loc():
    """Wing_sweep_loc: the default section, its Sweep at Sweep Loc 0 and the same Sweep at Sweep Loc 0.25."""
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    sw = wm.getx( w, 1, "Sweep" )
    G0 = wm.grid( w, 0, 1, nu = 3, nw = 41 )
    E0 = [ wm.lete( w, i ) for i in ( 0, 1 ) ]
    wm.setx( w, 1, "Sweep_Location", 0.25 )
    assert np.isclose( wm.getx( w, 1, "Sweep" ), sw )
    G1 = wm.grid( w, 0, 1, nu = 3, nw = 41 )
    E1 = [ wm.lete( w, i ) for i in ( 0, 1 ) ]

    def line( E, f ):
        return E[0][0] + f * ( E[0][1] - E[0][0] ), E[1][0] + f * ( E[1][1] - E[1][0] )

    def sweep_of( a, b ):
        return np.degrees( np.arctan2( b[0] - a[0], b[1] - a[1] ) )
    # The root stays; the swept line moves from the LE to the quarter chord at the same angle.
    assert np.allclose( E0[0], E1[0] )
    assert np.isclose( sweep_of( *line( E0, 0.0 ) ), sw ) and np.isclose( sweep_of( *line( E1, 0.25 ) ), sw )
    assert sweep_of( *line( E1, 0.0 ) ) > sw + 3.0
    fig = new()
    c = panel( fig, TOP )
    fill_planform( c, G0 )
    outline( c, G0 )
    nw = G1.shape[1]
    for L in ( G1[ :, nw // 2 ], G1[ :, 0 ] ):
        c.line( L, color = "#333333", lw = 1.0, ls = ( 0, ( 4, 2 ) ), z = 53 )
    c.line( np.array( [ E1[1][0], E1[1][1] ] ), color = "#333333", lw = 1.0, ls = ( 0, ( 4, 2 ) ), z = 53 )
    c.line( np.array( E0[0] ), color = C_FOIL, lw = 2.0, z = 55 )
    c.text( E0[0][1], "airfoil 0", color = C_FOIL, dx = 8, dy = -10, ha = "left" )
    for E, f, rho in ( ( E0, 0.0, 5.6 ), ( E1, 0.25, 2.6 ) ):
        a, b = line( E, f )
        c.line( np.array( [ a, b ] ), color = C_SWEEP, lw = 1.6, ls = "-" if f == 0.0 else ( 0, ( 4, 2 ) ), z = 56 )
        c.line( np.array( [ a, a + 1.15 * rho * Y ] ), color = C_REF, lw = 0.7, ls = ( 0, ( 4, 2 ) ), z = 45 )
        angle( c, a, Y, b - a, rho, C_SWEEP, lw = 1.4 )
    # The angle labels: above the first arc, and between the two leading edges above the second.
    c.text( line( E0, 0.0 )[0] + 5.6 * fl.unit( Y + 0.27 * X ), "%g$^\\circ$" % sw, color = C_SWEEP, dy = 10 )
    yl = 3.0
    xa = line( E1, 0.0 )[0][0] + yl * np.tan( np.radians( sweep_of( *line( E1, 0.0 ) ) ) )
    xb = line( E1, 0.25 )[0][0] + yl * np.tan( np.radians( sw ) )
    c.text( np.array( [ 0.5 * ( xa + xb ), yl, 0 ] ), "%g$^\\circ$" % sw, color = C_SWEEP )
    c.text( line( E0, 0.0 )[1], "Sweep Loc 0", color = C_SWEEP, dx = -8, dy = 0, ha = "right" )
    c.text( E1[1][1], "Sweep Loc 0.25", color = C_SWEEP, dx = 8, dy = 0, ha = "left" )
    key( c, "top", ( 1.0, 0.0 ), 10, 0 )
    save( fig, "Wing_sweep_loc", [ [ c ] ], hmax = 5.0 )


# ====================================================================== dihedral

def dihedral_wing( rot = 1.0, d1 = 0.0, d2 = 12.0, sym = True ):
    w = cranked()
    if not sym:
        vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0.0 )
        vsp.Update()
        vsp.Update()
    wm.setx( w, 1, "Dihedral", d1 )
    wm.setx( w, 2, "Dihedral", d2 )
    wm.setw( w, "RotateAirfoilMatchDideralFlag", rot )
    return w


def dihedral():
    """Wing_dihedral: the cranked wing from upstream, with the outboard section's dihedral and spans."""
    w = dihedral_wing()
    fig = new()
    c = panel( fig, UPSTREAM )
    for s in ( 0, 1 ):
        c.surface( wm.grid( w, 0, 2, nu = 41, nw = 61, surf = s ), outline_lw = 0.5 )
    c.flush()
    le1, _ = wm.lete( w, 1 )
    le2, _ = wm.lete( w, 2 )
    b = wm.getx( w, 2, "Span" )
    g = wm.getx( w, 2, "Dihedral" )
    pb = wm.getx( w, 2, "ProjectedSpan" )
    d = le2 - le1
    assert np.allclose( [ np.hypot( d[1], d[2] ), d[1], np.degrees( np.arctan2( d[2], d[1] ) ) ], [ b, pb, g ] )
    # The dihedral turns the section about an X line through airfoil 1's LE: without it, airfoil 2's LE sits level.
    wm.setx( w, 2, "Dihedral", 0.0 )
    d0 = wm.lete( w, 2 )[0] - wm.lete( w, 1 )[0]
    wm.setx( w, 2, "Dihedral", g )
    assert np.allclose( [ d0[1], d0[2], d0[0] ], [ b, 0.0, d[0] ] )
    assert np.allclose( wm.lete( w, 2 )[0], le2 )
    # Points at the airfoils' leading edges, seen end on.
    for p in ( le1, le2 ):
        dot( c, p, C_FOIL )
    le0, _ = wm.lete( w, 0 )
    dot( c, le0, C_FOIL )
    c.text( le0, "airfoil 0", color = C_FOIL, dx = 0, dy = -14 )
    c.text( le1, "airfoil 1", color = C_FOIL, dx = 0, dy = -14 )
    c.text( le2, "airfoil 2", color = C_FOIL, dx = 0, dy = -14 )
    # Dihedral from the horizontal.
    R = 4.5
    h = np.array( [ 0, 1.0, 0 ] )
    c.line( np.array( [ le1, le1 + 1.15 * R * h ] ), color = C_REF, lw = 0.7, ls = ( 0, ( 4, 2 ) ), z = 45 )
    angle( c, le1, h, d, R, C_DIH, label = "Dihedral", label_r = 1.0, dx = -34, dy = -8, lw = 1.4 )
    # Span along the panel, offset above it; Projected Span below.
    n = fl.unit( np.cross( X, d ) )
    if n[2] < 0:
        n = -n
    off = 1.4
    dim( c, le1 + off * n, le2 + off * n, "Span", dx = 0, dy = 12 )
    ext( c, le1, le1 + 1.2 * off * n )
    ext( c, le2, le2 + 1.2 * off * n )
    zb = min( le1[2], le2[2] ) - 1.0
    q1 = np.array( [ 0, le1[1], zb ] )
    q2 = np.array( [ 0, le2[1], zb ] )
    dim( c, q1, q2, "Projected Span", dx = 0, dy = -10 )
    ext( c, le1, q1 - 0.2 * Z )
    ext( c, le2, q2 - 0.2 * Z )
    key( c, "upstream", ( 1.0, 0.0 ), -30, 0 )
    save( fig, "Wing_dihedral", [ [ c ] ], hmax = 4.0 )


def foil_rotation():
    """Wing_foil_rotation: airfoil 1 at a dihedral break, upright and rotated to match the dihedral."""
    cs = []
    fig = new()
    for rot, name in ( ( 0.0, "Rotate To Match Dihedral off" ), ( 1.0, "Rotate To Match Dihedral on" ) ):
        w = dihedral_wing( rot = rot, d1 = 0.0, d2 = 20.0, sym = False )
        c = panel( fig, UPSTREAM )
        G = wm.grid( w, 0.75, 1.25, nu = 41, nw = 121 )
        c.surface( G, outline_lw = 0.5, cap_rows = False )
        c.flush()
        F = wm.foil( w, 1, 401 )
        le, te = wm.lete( w, 1 )
        # The airfoil seen along X is a line across its thickness.
        up = fl.unit( np.cross( te - le, fl.unit( np.linalg.svd( F - F.mean( 0 ) )[2][2] ) ) )
        if up[2] < 0:
            up = -up
        t = F @ up
        p0 = le + ( t.min() - le @ up ) * up
        p1 = le + ( t.max() - le @ up ) * up
        c.line( np.array( [ p0, p1 ] ), color = C_FOIL, lw = 2.4, z = 70 )
        c.line( np.array( [ le - 0.55 * up, le + 0.55 * up ] ), color = C_FOIL, lw = 0.7, ls = ( 0, ( 3, 2 ) ), z = 69 )
        c.text( le + 0.55 * up, "airfoil 1", color = C_FOIL, dx = 0, dy = 8 )
        title( c, name, pad = 26 )
        cs.append( c )
    box = ( np.min( [ c.limits()[0] for c in cs ], 0 ), np.max( [ c.limits()[1] for c in cs ], 0 ) )
    for c in cs:
        c.bounds = box
    key( cs[1], "upstream", ( 1.0, 0.0 ), -10, -8 )
    save( fig, "Wing_foil_rotation", [ cs ], hmax = 4.0 )


# ====================================================================== twist

TWIST = 8.0


def twist():
    """Wing_twist: the tip airfoil of the default wing, untwisted and twisted about Twist Loc."""
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    F0 = wm.foil( w, 1, 301 )
    le0, te0 = wm.lete( w, 1 )
    loc = wm.getx( w, 1, "Twist_Location" )
    q = le0 + loc * ( te0 - le0 )
    wm.setx( w, 1, "Twist", TWIST )
    F1 = wm.foil( w, 1, 301 )
    le1, te1 = wm.lete( w, 1 )
    assert np.allclose( le1 + loc * ( te1 - le1 ), q )
    assert np.isclose( np.degrees( np.arctan2( le1[2] - te1[2], te1[0] - le1[0] ) ), TWIST )
    fig = new()
    c = panel( fig, SIDE )
    Q = c.v.p2( F1 )
    c.pts.append( Q )
    c.ax.fill( Q[:, 0], Q[:, 1], facecolor = COL[ "surf" ], edgecolor = "#333333", lw = 0.9, zorder = 30 )
    c.line( F0, color = C_REF, lw = 0.9, ls = ( 0, ( 3, 2 ) ), z = 31 )
    # Chord lines, run through the twist point both ways.
    e = 0.35
    c.line( np.array( [ le0 - e * ( te0 - le0 ), te0 + e * ( te0 - le0 ) ] ), color = C_REF, lw = 0.8, ls = ( 0, ( 6, 2 ) ), z = 40 )
    c.line( np.array( [ le1 - e * ( te1 - le1 ), te1 + e * ( te1 - le1 ) ] ), color = C_TWIST, lw = 1.2, ls = ( 0, ( 6, 2, 1, 2 ) ), z = 41 )
    R = 0.95 * np.linalg.norm( q - le0 ) + 0.25
    angle( c, q, le0 - q, le1 - q, R, C_TWIST, label = "Twist", label_r = 1.0, dx = -26, dy = -2, lw = 1.4 )
    dot( c, q, "k" )
    c.ax.annotate( "Twist Loc", xy = c.v.p2( q ), xytext = ( 30, -40 ), textcoords = "offset points", ha = "left", va = "top",
                   zorder = 90, arrowprops = dict( arrowstyle = "-", lw = 0.7, color = "k", shrinkA = 2, shrinkB = 4 ) )
    c.text( le1, "LE", dx = -12, dy = 6 )
    c.text( te1, "TE", dx = 12, dy = -4 )
    key( c, "side", ( 1.0, 0.0 ), 0, -10 )
    save( fig, "Wing_twist", [ [ c ] ], hmax = 3.2 )


INCIDENCE = 8.0


def incidence():
    """Wing_incidence: the default wing's root and tip airfoils from the side, turned by Incidence with relative twist
    and by the XForm Y Rotation about the root quarter chord, each over the unturned wing."""
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    base = [ wm.foil( w, i, 301 ) for i in ( 0, 1 ) ]
    le0, te0 = wm.lete( w, 0 )
    le1, te1 = wm.lete( w, 1 )
    q0 = le0 + 0.25 * ( te0 - le0 )
    q1 = le1 + 0.25 * ( te1 - le1 )
    cases = []
    # Incidence (the root airfoil's twist) with relative twist: every airfoil turns about its own Twist Loc.
    wm.setw( w, "RelativeTwistFlag", 1.0 )
    wm.setx( w, 0, "Twist", INCIDENCE )
    a = [ wm.foil( w, i, 301 ) for i in ( 0, 1 ) ]
    l0, t0 = wm.lete( w, 0 )
    l1, t1 = wm.lete( w, 1 )
    assert np.allclose( [ l0 + 0.25 * ( t0 - l0 ), l1 + 0.25 * ( t1 - l1 ) ], [ q0, q1 ] )
    cases.append( ( a, [ q0, q1 ], [ "Incidence Loc", "Twist Loc" ], "Incidence %g$^\\circ$, Twist Reference Rel" % INCIDENCE ) )
    # XForm Y Rotation about Rot Origin(X) 0.25 of the root chord: a rigid turn about one point.
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    vsp.SetParmVal( w, "Origin", "XForm", 0.25 )
    vsp.SetParmVal( w, "Y_Rel_Rotation", "XForm", INCIDENCE )
    vsp.Update()
    b = [ wm.foil( w, i, 301 ) for i in ( 0, 1 ) ]
    l0, t0 = wm.lete( w, 0 )
    l1, t1 = wm.lete( w, 1 )
    assert np.allclose( l0 + 0.25 * ( t0 - l0 ), q0 )
    r1 = l1 + 0.25 * ( t1 - l1 )
    assert r1[2] < q1[2] - 0.5
    cases.append( ( b, [ q0 ], [ "Rot Origin" ], "Y Rotation %g$^\\circ$ about Rot Origin(X) 0.25" % INCIDENCE ) )
    fig = new()
    cs = []
    for F, pts, labs, name in cases:
        c = panel( fig, SIDE )
        for k in ( 0, 1 ):
            c.line( base[k], color = C_REF, lw = 0.9, ls = ( 0, ( 3, 2 ) ), z = 31 )
            Q = c.v.p2( F[k] )
            c.pts.append( Q )
            c.ax.fill( Q[:, 0], Q[:, 1], facecolor = COL[ "surf" ], edgecolor = C_FOIL, lw = 1.2, zorder = 30 )
        for p, lab in zip( pts, labs ):
            dot( c, p, "k" )
            c.text( p, lab, dx = 0, dy = -16 )
        c.text( F[0][ np.argmax( F[0][:, 2] ) ], "root airfoil", color = C_FOIL, dx = 0, dy = 12 )
        c.text( F[1][ np.argmax( F[1][:, 2] ) ], "tip airfoil", color = C_FOIL, dx = 0, dy = 12 )
        title( c, name, pad = 26 )
        cs.append( c )
    box = ( np.min( [ c.limits()[0] for c in cs ], 0 ), np.max( [ c.limits()[1] for c in cs ], 0 ) )
    for c in cs:
        c.bounds = box
    key( cs[1], "side", ( 1.0, 0.0 ), 0, -10 )
    save( fig, "Wing_incidence", [ [ c ] for c in cs ], hmax = 6.0 )


# ====================================================================== blending

BLEND_SWEEP = 22.5


def blend_tangents( w, i, h = 1e-5 ):
    """Directions the LE leaves airfoil i, inboard and outboard, measured on the surface."""
    p0 = wm.pnt( w, wm.u01( w, i ), 0.5 )
    pm = wm.pnt( w, wm.u01( w, i - h ), 0.5 )
    pp = wm.pnt( w, wm.u01( w, i + h ), 0.5 )
    return fl.unit( pm - p0 ), fl.unit( pp - p0 )


def blending():
    """Wing_blending: the cranked wing's leading edge, straight and blended at airfoil 1."""
    w0 = cranked()
    vsp.SetParmVal( w0, "Sym_Planar_Flag", "Sym", 0.0 )
    vsp.Update()
    vsp.Update()
    G0 = wm.grid( w0, 0, 2, nu = 81, nw = 41 )
    wm.setx( w0, 1, "InLEMode", vsp.BLEND_ANGLES )
    wm.setx( w0, 1, "InLESweep", BLEND_SWEEP )
    wm.setx( w0, 1, "InLEDihedral", 0.0 )
    wm.setx( w0, 1, "OutLEMode", vsp.BLEND_MATCH_IN_ANGLES )
    w = w0
    G = wm.grid( w, 0, 2, nu = 161, nw = 41 )
    le = [ wm.lete( w, i )[0] for i in range( 3 ) ]
    tin, tout = blend_tangents( w, 1 )
    assert np.isclose( np.degrees( np.arctan2( tout[0], np.hypot( tout[1], tout[2] ) ) ), BLEND_SWEEP, atol = 1e-3 )
    assert np.allclose( tin, -tout, atol = 1e-3 )
    fig = new()
    c = panel( fig, TOP )
    fill_planform( c, G )
    outline( c, G )
    c.line( G0[ :, G0.shape[1] // 2 ], color = C_REF, lw = 1.0, ls = ( 0, ( 4, 2 ) ), z = 53 )
    for i in range( 3 ):
        lei, tei = wm.lete( w, i )
        c.line( np.array( [ lei, tei ] ), color = C_FOIL, lw = 1.8, z = 55 )
        c.text( tei, "airfoil %d" % i, color = C_FOIL, dx = 8, dy = 0, ha = "left" )
    # The blend vectors at airfoil 1's LE, as the 3D view draws them: inboard dashed, outboard solid.
    s_in = wm.getx( w, 1, "InLEStrength" ) * np.linalg.norm( le[1] - le[0] )
    s_out = wm.getx( w, 1, "OutLEStrength" ) * np.linalg.norm( le[2] - le[1] )
    c.arrow( le[1], le[1] + s_in * tin, color = C_BLEND, lw = 1.4, ls = ( 0, ( 4, 2 ) ), head = 8 )
    c.arrow( le[1], le[1] + s_out * tout, color = C_BLEND, lw = 1.4, head = 8 )
    c.text( le[1] + s_in * tin, "inboard", color = C_BLEND, dx = -8, dy = 0, ha = "right" )
    c.text( le[1] + s_out * tout, "outboard", color = C_BLEND, dx = -8, dy = 0, ha = "right" )
    # The sweep of the blend, from the airfoil's normal.
    nrm = fl.unit( np.cross( wm.lete( w, 1 )[1] - le[1], Z ) )
    if nrm[1] < 0:
        nrm = -nrm
    R = 2.6
    c.line( np.array( [ le[1], le[1] + 1.2 * R * nrm ] ), color = C_REF, lw = 0.7, ls = ( 0, ( 4, 2 ) ), z = 45 )
    angle( c, le[1], nrm, tout, R, C_BLEND, label = "Sweep", label_r = 1.0, dx = 6, dy = 14, lw = 1.2 )
    c.text( le[1], "airfoil 1 LE", dx = 12, dy = 12, ha = "left" )
    dot( c, le[1], "k" )
    key( c, "top", ( 1.0, 0.0 ), 10, 0 )
    save( fig, "Wing_blending", [ [ c ] ], hmax = 5.0 )


def winglet():
    """Wing_winglet: the root fillet and blended winglet example, from above and from upstream, over the straight-edged
    wing it starts from."""
    w0 = wm.winglet_example( blend = False )
    n = wm.nsect( w0 )
    S0 = wm.grid( w0, 0, n, nu = 4 * 40 + 1, nw = 41 )
    E0 = [ wm.lete( w0, i ) for i in range( n + 1 ) ]
    w = wm.winglet_example()
    G = wm.grid( w, 0, n, nu = 4 * 40 + 1, nw = 81 )
    E = [ wm.lete( w, i ) for i in range( n + 1 ) ]
    # The blending moves no airfoil; the main wing and the winglet keep straight edges.
    assert np.allclose( np.array( E ), np.array( E0 ) )
    for U0 in ( 1, 3 ):
        for t in ( 0.5, 0.0 ):
            P = [ wm.pnt( w, wm.u01( w, U ), t ) for U in ( U0, U0 + 0.5, U0 + 1 ) ]
            assert np.allclose( P[1], 0.5 * ( P[0] + P[2] ), atol = 1e-6 )
    fig = new()
    cs = []
    for view, which, labs in ( ( TOP, "top", True ), ( UPSTREAM, "upstream", False ) ):
        c = panel( fig, view )
        c.surface( G, outline_lw = 0.6, cap_rows = False )
        c.flush()
        nw = S0.shape[1]
        for L in ( S0[ :, nw // 2 ], S0[ :, 0 ] ):
            c.line( L, color = "#333333", lw = 0.9, ls = ( 0, ( 4, 2 ) ), z = 53 )
        for i in range( n + 1 ):
            F = wm.foil( w, i, 161 )
            c.line( F, color = C_FOIL, lw = 1.6, z = 60 )
        cs.append( c )
    ct, cu = cs
    # Airfoil labels: from above beside the chord, clear of the short sections' crowded airfoils; from upstream beside
    # the airfoil's leading edge.
    for i, k, dx, dy, ha in ( ( 0, 1, 8, 0, "left" ), ( 1, 1, 8, 0, "left" ), ( 2, 0, -8, -6, "right" ),
                              ( 3, 0, -10, 6, "right" ), ( 4, 1, 8, 2, "left" ) ):
        ct.text( E[i][k], "airfoil %d" % i, color = C_FOIL, dx = dx, dy = dy, ha = ha )
    for i, dx, dy, ha in ( ( 0, 0, 16, "center" ), ( 1, 0, -14, "center" ), ( 2, 6, -12, "left" ), ( 3, 10, -2, "left" ),
                           ( 4, 10, 0, "left" ) ):
        cu.text( E[i][0], "airfoil %d" % i, color = C_FOIL, dx = dx, dy = dy, ha = ha )
    key( ct, "top", ( 1.0, 0.0 ), 30, 0 )
    key( cu, "upstream", ( 1.0, 0.0 ), 50, -6 )
    save( fig, "Wing_winglet", [ [ ct ], [ cu ] ], hmax = 8.0 )


# ====================================================================== tip caps

def tip_grid( w, frac = 0.06, nu = 25, nw = 121 ):
    """The outer frac of the last section and the tip cap."""
    n = wm.nsect( w )
    G1 = wm.grid( w, n - frac * n, n, nu = nu, nw = nw )
    u0 = wm.u01( w, n )
    G2 = wm.grid01( w, u0, 1.0, nu = nu, nw = nw )
    return np.concatenate( [ G1, G2[ 1: ] ], axis = 0 )


def caps_upstream():
    """Wing_caps: the default wing's tip, from upstream, with each tip cap type."""
    types = ( ( vsp.FLAT_END_CAP, "Flat" ), ( vsp.ROUND_END_CAP, "Round" ), ( vsp.EDGE_END_CAP, "Edge" ),
              ( vsp.SHARP_END_CAP, "Sharp" ) )
    fig = new()
    cs = []
    for cap, name in types:
        w = wm.new_wing( tip_cap = cap )
        G = tip_grid( w, frac = 0.025 )
        c = panel( fig, UPSTREAM )
        c.surface( G, outline_lw = 0.4, cap_rows = False )
        c.flush()
        le, te = wm.lete( w, 1 )
        F = wm.foil( w, 1, 401 )
        c.line( F, color = C_FOIL, lw = 1.6, z = 60 )
        if not cs:
            c.text( F[ np.argmax( F[:, 2] ) ], "tip airfoil", color = C_FOIL, dx = -6, dy = 6, ha = "right" )
        title( c, name, pad = 8 )
        cs.append( c )
    box = ( np.min( [ c.limits()[0] for c in cs ], 0 ), np.max( [ c.limits()[1] for c in cs ], 0 ) )
    for c in cs:
        c.bounds = box
    key( cs[3], "upstream", ( 1.0, 0.0 ), -40, -10 )
    save( fig, "Wing_caps", [ cs[ :2 ], cs[ 2: ] ], hmax = 5.0 )


def caps_ext():
    """Wing_caps_ext: the default wing's tip from above, with the Round and extended Round caps."""
    types = ( ( vsp.ROUND_END_CAP, "Round" ), ( vsp.ROUND_EXT_END_CAP_LE, "Round Ext LE" ),
              ( vsp.ROUND_EXT_END_CAP_TE, "Round Ext TE" ), ( vsp.ROUND_EXT_END_CAP_BOTH, "Round Ext Both" ) )
    fig = new()
    cs = []
    for cap, name in types:
        w = wm.new_wing( tip_cap = cap )
        G = tip_grid( w, frac = 0.012, nw = 161 )
        c = panel( fig, TOP )
        c.surface( G, outline_lw = 0.4, cap_rows = False )
        c.flush()
        F = wm.foil( w, 1, 401 )
        c.line( F, color = C_FOIL, lw = 1.6, z = 60 )
        if not cs:
            le, te = wm.lete( w, 1 )
            c.text( le, "LE", dx = -14, dy = 0 )
            c.text( te, "TE", dx = 14, dy = 0 )
            c.text( 0.5 * ( le + te ), "tip airfoil", color = C_FOIL, dx = 0, dy = -12 )
        title( c, name, pad = 8 )
        cs.append( c )
    box = ( np.min( [ c.limits()[0] for c in cs ], 0 ), np.max( [ c.limits()[1] for c in cs ], 0 ) )
    for c in cs:
        c.bounds = box
    key( cs[3], "top", ( 1.0, 0.0 ), 30, 0 )
    save( fig, "Wing_caps_ext", [ [ c ] for c in cs ], hmax = 8.0 )


# ====================================================================== eta

def eta():
    """Wing_eta: lines of constant eta on the right half of the cranked wing, with the U of the airfoils and caps."""
    w = cranked()
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0.0 )
    vsp.SetParmVal( w, "CapUMaxOption", "EndCap", vsp.ROUND_END_CAP )
    vsp.Update()
    vsp.Update()
    n = wm.nsect( w )
    umax = n + wm.ncaps( w )
    spans = [ wm.getx( w, i, "Span" ) for i in range( 1, n + 1 ) ]
    run = np.cumsum( [ 0.0 ] + spans ) / sum( spans )
    for i in range( n + 1 ):
        assert np.isclose( vsp.ConvertEtatoU( w, run[i] ), i + 1.0 )
    fig = new()
    c = panel( fig, TOP )
    G = wm.grid01( w, 0.0, 1.0, nu = 121, nw = 81 )
    fill_planform( c, G )
    outline( c, G )

    def chord( e ):
        u = vsp.ConvertEtatoU( w, e ) / umax
        return wm.pnt( w, u, 0.5 ), wm.pnt( w, u, 0.0 )
    for e in np.linspace( 0.0, 1.0, 11 ):
        le, te = chord( e )
        c.line( np.array( [ le, te ] ), color = C_REF, lw = 0.6, z = 53 )
    for i in range( n + 1 ):
        le, te = chord( run[i] )
        c.line( np.array( [ le, te ] ), color = C_FOIL, lw = 2.0, z = 55 )
        c.text( le, r"$\eta$ = %g" % round( run[i], 3 ), color = C_FOIL, dx = -8, dy = 0, ha = "right" )
        c.text( te, "U = %d" % ( i + 1 ), dx = 8, dy = 0, ha = "left" )
    le, te = chord( 0.5 )
    c.text( le, r"$\eta$ = 0.5", dx = -8, dy = 0, ha = "right" )
    # The caps: the root cap is flat and lies on airfoil 0; the round tip cap reaches beyond the tip airfoil.
    le0, te0 = chord( 0.0 )
    c.text( 0.5 * ( le0 + te0 ), "root cap, U 0 to 1 (flat, on airfoil 0)", dx = 0, dy = -12, va = "top" )
    tip = wm.pnt( w, ( n + 1.5 ) / umax, 0.5 )
    c.text( tip, "tip cap, U %d to %d" % ( n + 1, n + 2 ), dx = 0, dy = 12, va = "bottom" )
    for k in range( n ):
        a, _ = chord( 0.5 * ( run[k] + run[k + 1] ) )
        _, b = chord( 0.5 * ( run[k] + run[k + 1] ) )
        c.text( 0.5 * ( a + b ), "section %d, Span %g" % ( k + 1, spans[k] ) )
    key( c, "top", ( 1.0, 0.0 ), 10, 0 )
    save( fig, "Wing_eta", [ [ c ] ], hmax = 5.0 )


FIGS = { "parts": parts, "planform": planform, "sweep_loc": sweep_loc, "shared_chord": shared_chord, "dihedral": dihedral, "foil_rotation": foil_rotation, "twist": twist, "blending": blending, "winglet": winglet, "caps": caps_upstream, "caps_ext": caps_ext, "incidence": incidence, "eta": eta }

if __name__ == "__main__":
    names = sys.argv[ 1: ] or list( FIGS )
    for nme in names:
        FIGS[ nme ]()
