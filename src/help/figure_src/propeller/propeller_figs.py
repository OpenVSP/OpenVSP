"""Figures for the OpenVSP Propeller help page (src/help/Propeller.md): one panel per image, sized for a web page.

    <build>/venv/bin/python propeller_figs.py [names...]      # writes ../../images/Propeller_*.png

Every 3D view is OpenVSP's: Z up, Y into the page, X to the right, seen from about (-1, -1, 1).  A rotor is drawn in
helicopter position, turned by Y Rotation 90, so its thrust points up.  2D views keep the same frame: the front view
looks along +Y (X right, Z up), the top view along -Z (X right, Y up), the view from upstream along +X (-Y right, Z up).
Blades are VSP's own, placed by propmodel's chain, and checked against VSP as each figure is drawn.
"""

import os
import sys

import numpy as np
import matplotlib
matplotlib.use( "Agg" )
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from matplotlib.transforms import offset_copy

import propmodel as pm
import figlib as fl
from figlib import COL, FS
import geom
import figparts as fp

OUT = os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "..", "images" )
R = geom.RADIUS
X = np.array( [ 1.0, 0, 0 ] )
Y = np.array( [ 0, 1.0, 0 ] )
Z = np.array( [ 0, 0, 1.0 ] )
O = np.zeros( 3 )

DPI = 200        # fl.PAGE_W inches wide: 1400 pixels

# The colors OpenVSP draws the hinges in (PropGeom::HingeDrawColor, DrawObj::Color).
COL.update( { "flap": "#1E90FF", "lag": "#FF4500", "feather": "#32CD32", "fold": "#BA55D3" } )


# ---------------------------------------------------------------- views

def view( right, up, rot = None ):
    """An orthographic view with model directions right and up; rot maps propeller axes to model axes."""
    right = np.asarray( right, float )
    up = np.asarray( up, float )
    if rot is not None:
        right = rot.T @ right
        up = rot.T @ up
    return fl.View.plane( right, up )


HELI = pm.R3( Y, 90.0 )                               # Y Rotation 90: the thrust (-X) points up (+Z)
ISO = view( [ 1, -1, 0 ], [ 1, 1, 2 ] )               # from (-1, -1, 1), Z up
ISO_H = view( [ 1, -1, 0 ], [ 1, 1, 2 ], HELI )
FRONT = view( X, Z )                                  # looking along +Y
TOP = view( X, Y )                                    # looking along -Z
UPSTREAM = view( -Y, Z )                              # from -X, the thrust side, looking downstream
TOP_H = view( X, Y, HELI )
NOSE_H = view( -Y, Z, HELI )                          # from the nose (-X), looking aft: -Y right, Z up

KEY = { "front": ( r"$X$", r"$Z$", r"$-Y$" ), "top": ( r"$X$", r"$Y$", r"$Z$" ), "upstream": ( r"$-Y$", r"$Z$", r"$-X$" ) }


def style():
    """Matplotlib settings for the web figures."""
    fl.style()


def new( v = ISO ):
    """A one-panel figure with view v; save() sizes it."""
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = panel( fig, v )
    return fig, c.ax, c


def panel( fig, v ):
    """A canvas with view v on fig; fl.fit places it."""
    return fl.Canvas( fig.add_axes( [ 0, 0, 1, 1 ] ), v )



def save( fig, name, rows = None, scale = None, hmax = fl.HMAX ):
    """Lay out rows of canvases across the page width (fl.fit), then write the whole figure."""
    if rows is not None:
        fl.fit( fig, rows, scale = scale, hmax = hmax )
    os.makedirs( OUT, exist_ok = True )
    # No Software tag, so the same picture from another matplotlib is the same file.
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


def model_triad( c, o, rot = None, length = 3.0, labels = ( r"$X$", r"$Y$", r"$Z$" ), color = "k" ):
    """The model's X, Y, Z at o, drawn in propeller axes when the rotor is turned by rot."""
    vecs = [ X, Y, Z ]
    if rot is not None:
        vecs = [ rot.T @ v for v in vecs ]
    c.triad( o, vecs, labels, color, length = length, lw = 1.3 )


def lete( c, G, frac, nw, d = 10 ):
    """LE and TE labels pushed apart along the chord as seen."""
    i = int( round( frac * ( G.shape[0] - 1 ) ) )
    le = G[ i, nw // 2 ]
    te = G[ i, 0 ]
    q = c.v.p2( np.array( [ te, le ] ) )
    u = ( q[1] - q[0] ) / max( np.linalg.norm( q[1] - q[0] ), 1e-12 )
    c.text( le, "LE", dx = d * u[0], dy = d * u[1] )
    c.text( te, "TE", dx = -d * u[0], dy = -d * u[1] )


def te_edge( c, G, nw, lw = 2.0 ):
    """The trailing edge, drawn heavy."""
    c.line( G[ :, 0 ], color = "#222222", lw = lw )


# ====================================================================== axes

def axes_prop():
    """Propeller_axes: the default propeller, its axes, thrust, rotation and Rotate."""
    rot = 60.0
    prop = geom.Prop( { "Rotate": rot, "Feather": 0.0 } )
    prop.check_pose()
    fig, ax, c = new()
    for i in range( prop.nblade ):
        c.surface( prop.vsp_blades[ i ], outline_lw = 0.6 )
    c.flush()
    for i in range( prop.nblade ):
        te_edge( c, prop.vsp_blades[ i ], prop.nw )
    fp.hub( c )
    L = 5.0
    c.triad( O, [ X, Y, Z ], [ r"$X$", "", "" ], "k", length = L, lw = 1.5 )
    c.text( 1.2 * L * Y, r"$Y$", dx = 10, dy = -8 )
    c.text( 1.2 * L * Z, r"$Z$", dx = 12, dy = 0 )
    c.arrow( -1.3 * L * X, -2.8 * L * X, color = "k", lw = 2.6, head = 13 )
    c.text( -2.8 * L * X, "thrust", dx = -10, dy = -12, ha = "right" )
    A = c.arc_pts( -2.3 * L * X, -X, 3.6, 300, 430, ref = Y )
    c.arrow_path( A, color = COL[ "shaft" ], lw = 1.8, head = 10 )
    c.text( A[ -1 ], r"$\Omega$", color = COL[ "shaft" ], dx = 10, dy = 4 )
    d1 = pm.apply_dir( pm.Rx( -rot ), Y )
    d2 = pm.apply_dir( pm.Rx( -rot - 120.0 ), Y )
    c.line( np.array( [ O, 17.0 * Y ] ), color = "#888888", lw = 0.8, ls = ( 0, ( 2, 2 ) ) )
    c.line( np.array( [ O, 14.0 * d2 ] ), color = "#888888", lw = 0.8, ls = ( 0, ( 2, 2 ) ) )
    c.angle( O, Y, d1, 12.0, COL[ "shaft" ], label = r"Rotate $\psi$", lw = 1.4, label_r = 1.08, dx = 0, dy = 12 )
    c.angle( O, d1, d2, 7.0, COL[ "move" ], label = r"$\phi_2$", lw = 1.4, label_r = 1.12 )
    for i, ( frac, dx, dy ) in enumerate( ( ( 1.0, 10, 10 ), ( 1.0, -10, 10 ), ( 1.0, 0, -14 ) ) ):
        c.text( prop.vsp_blades[ i ][ -1, prop.nw // 2 ], "blade %d" % ( i + 1 ), dx = dx, dy = dy )
    lete( c, prop.vsp_blades[ 0 ], 0.5, prop.nw )
    save( fig, "Propeller_axes", [ [ c ] ] )


def axes_blade():
    """The blade axes at the rotor center, on a rotor in helicopter position seen mostly from above."""
    fig, ax, c = new_h()
    rotor_silhouette( c )
    blade_triad( c, length = 4.5 )
    hinge_frame( c )
    save( fig, "Propeller_blade_axes" )


# ====================================================================== shaping, one image per step

def axes2d( c, half = 3.2, plane = None, labels = True ):
    """The rotation axis from the origin, and the plane of rotation, edge on (front view)."""
    if plane is None:
        plane = half
    c.arrow( O, -half * X, color = COL[ "shaft" ], lw = 1.4, head = 9 )
    c.line( np.array( [ -plane * Z, plane * Z ] ), color = "#9AA7B5", lw = 1.0, ls = ( 0, ( 6, 3 ) ) )
    if labels:
        c.text( -half * X, "rotation axis", color = COL[ "shaft" ], dx = 0, dy = 10 )
        c.text( plane * Z, "plane of rotation", color = "#6A7F95", dx = 8, dy = -2, ha = "left" )
    q = c.v.p2( O )
    c.ax.plot( [ q[0] ], [ q[1] ], marker = "+", ms = 12, color = "k", mew = 1.2, zorder = 40 )


def dot( c, p, color = None, ms = 6 ):
    """A marked point."""
    q = c.v.p2( p )
    c.ax.plot( [ q[0] ], [ q[1] ], marker = "o", ms = ms, color = color or COL[ "move" ], zorder = 45, mec = "white", mew = 0.6 )


ATCOL = "#6A3D9A"                                     # Axial and Tangential, beside Rake and Skew
REAR = view( Y, Z )                                   # VSP Rear: from downstream, Y right, Z up
KEY[ "rear" ] = ( r"$Y$", r"$Z$", r"$X$" )
SEC_COL = { "root": "#003F5C", "mid": "#7A5195", "tip": "#EF5675" }


def title( c, s, pad = 10.0 ):
    """A panel title centered over what the canvas holds."""
    lo, hi = c.limits()
    c.ax.annotate( s, xy = ( 0.5 * ( lo[0] + hi[0] ), hi[1] ), xytext = ( 0, pad ), textcoords = "offset points", ha = "center",
                   va = "bottom", annotation_clip = False )



def construct_pt( G, i, nw, cx ):
    """A section's Construct X/C point: that fraction of the way from its LE to its TE."""
    le = G[ i, nw // 2 ]
    te = G[ i, 0 ]
    return le + cx * ( te - le )


# ====================================================================== Construct X/C, compared

CX_SET = ( 0.0, 0.5, 1.0 )
CX_NW = 145


def construct_blades():
    """The default blade at each Construct X/C of CX_SET, with its root, mid and tip rows."""
    out = []
    for cx in CX_SET:
        prop = geom.Prop( { "Feather": 0.0, "ConstructXoC": cx }, nw = CX_NW )
        prop.check_pose()
        rows = { "root": prop.iu( 0.0 ), "mid": prop.iu( 0.5 ), "tip": prop.iu( 1.0 ) }
        out.append( ( cx, prop, rows ) )
    return out


def construct_left():
    """Root, mid and tip sections seen from the root toward the tip (VSP Left)."""
    fig = plt.figure()
    cs = []
    for cx, prop, rows in construct_blades():
        c = panel( fig, FRONT )
        ax = c.ax
        G = prop.vsp_blades[0]
        first = not cs
        axes2d( c, 1.0, 2.3, labels = False )
        if first:
            c.text( -1.0 * X, "rotation axis", color = COL[ "shaft" ], dx = 0, dy = 10 )
            c.text( 2.3 * Z, "plane of rotation", color = "#6A7F95", dx = -6, dy = -6, ha = "right" )
        for k in ( "root", "mid", "tip" ):
            i = rows[ k ]
            Q = c.v.p2( G[ i ] )
            c.pts.append( Q )
            ax.fill( Q[:, 0], Q[:, 1], facecolor = SEC_COL[ k ], alpha = 0.18, zorder = 20 )
            ax.plot( Q[:, 0], Q[:, 1], color = SEC_COL[ k ], lw = 1.8, zorder = 21 )
            # Label each section at the end away from the construction line, where the sections fan apart.
            if cx > 0.5:
                c.text( G[ i, prop.nw // 2 ], k, color = SEC_COL[ k ], dx = -12, dy = 0, ha = "right" )
            else:
                c.text( G[ i, 0 ], k, color = SEC_COL[ k ], dx = 12, dy = 0, ha = "left" )
        for k in ( "root", "mid", "tip" ):
            dot( c, construct_pt( G, rows[ k ], prop.nw, cx ), color = "k", ms = 7 )
        c.hinge( O, Y, COL[ "feather" ], half = 0.01, rho = 0.32, at = 0.0, arrow_on_axis = False, sweep = 290, lw = 1.6 )
        if first:
            p = construct_pt( G, rows[ "root" ], prop.nw, cx )
            ax.annotate( "construction line", xy = c.v.p2( p ), xytext = ( 0, -95 ), textcoords = "offset points", ha = "center",
                         va = "top", zorder = 90, arrowprops = dict( arrowstyle = "-", lw = 0.7, color = "k", shrinkA = 2, shrinkB = 4 ) )
            leader( c, O + 0.32 * X, "feather axis", ( 30, -80 ), COL[ "feather" ] )
        cs.append( c )
    # One box for all three, so the rotation axis is in the same place in each.
    box = ( np.min( [ c.limits()[0] for c in cs ], 0 ), np.max( [ c.limits()[1] for c in cs ], 0 ) )
    for c, cx in zip( cs, CX_SET ):
        c.bounds = box
        title( c, "Construct X/C = %g" % cx )
    key( cs[2], "front", ( 1.0, 0.0 ), -10, 0 )

    save( fig, "Propeller_construct_left", [ cs ], hmax = 5.0 )



def construct_rear():
    """The same blades' planforms from downstream (VSP Rear)."""
    fig = plt.figure()
    cs = []
    for cx, prop, rows in construct_blades():
        c = panel( fig, REAR )
        ax = c.ax
        G = prop.vsp_blades[0]
        c.surface( G )
        c.flush()
        te_edge( c, G, prop.nw )
        q = c.v.p2( O )
        fl.out_of_page( ax, q, "" )
        c.text( O, "rotation\naxis", color = COL[ "shaft" ], dx = 0, dy = -20 )
        tip = G[ :, :, 1 ].max()
        c.line( np.array( [ O, ( tip + 1.0 ) * Y ] ), color = COL[ "feather" ], lw = 1.4, ls = ( 0, ( 5, 2 ) ), z = 60 )
        c.text( ( tip + 1.0 ) * Y, "feather\naxis", color = COL[ "feather" ], dx = 6, dy = 0, ha = "left" )
        lo = rows[ "root" ]
        hi = rows[ "tip" ]
        cl = np.array( [ construct_pt( G, i, prop.nw, cx ) for i in range( lo, hi + 1 ) ] )
        c.line( cl, color = "k", lw = 1.2, ls = ( 0, ( 2, 2 ) ), z = 62 )
        for k in ( "root", "mid", "tip" ):
            i = rows[ k ]
            c.line( np.array( [ G[ i, prop.nw // 2 ], G[ i, 0 ] ] ), color = SEC_COL[ k ], lw = 2.0, z = 61 )
            if k == "tip":
                c.text( G[ i, 0 ], k, color = SEC_COL[ k ], dx = 14, dy = 6, ha = "left" )
            else:
                c.text( G[ i, 0 ], k, color = SEC_COL[ k ], dx = 0, dy = 10 )
            c.pts.append( c.v.p2( G[ i, 0 ] + 1.2 * Z )[ None, : ] )
            dot( c, construct_pt( G, i, prop.nw, cx ), color = "k", ms = 6 )
        im = rows[ "mid" ]
        c.text( G[ im, prop.nw // 2 ], "LE", dx = 0, dy = -12 )
        c.text( G[ prop.iu( 0.8 ), 0 ], "TE", dx = 0, dy = 10 )
        title( c, "Construct X/C = %g" % cx )
        cs.append( c )
    key( cs[0], "rear", ( 0.0, 1.0 ), 10, -10 )
    save( fig, "Propeller_construct_rear", [ [ c ] for c in cs ], hmax = 8.0 )


def wrap_planform():
    """The default blade from behind, flat and with Cylindrical Foils: each section lies on the cylinder of its radius."""
    fig = plt.figure()
    cs = []
    fr = ( 0.0, 0.25, 0.5, 0.75, 1.0 )
    for flag, name in ( ( 0.0, "Cylindrical Foils off" ), ( 1.0, "Cylindrical Foils on" ) ):
        c = panel( fig, REAR )
        ax = c.ax
        prop = geom.Prop( { "Feather": 0.0, "CylindricalSectionsFlag": flag } )
        prop.check_pose()
        G = prop.vsp_blades[0]
        c.surface( G )
        c.flush()
        te_edge( c, G, prop.nw )
        fl.out_of_page( ax, c.v.p2( O ), "" )
        c.text( O, "rotation\naxis", color = COL[ "shaft" ], dx = 0, dy = -22 )
        c.pts.append( c.v.p2( np.array( [ -1.0 * Y ] ) ) )
        for f in fr:
            i = prop.iu( f )
            # The section's upper surface, LE to TE, and the circle of its radius through it.
            c.line( G[ i, prop.nw // 2:, : ], color = "#C0392B", lw = 1.6, z = 61 )
            r = np.linalg.norm( G[ i, prop.nw // 2, 1: ] )
            th = np.radians( np.linspace( -16.0, 16.0, 60 ) )
            c.line( np.column_stack( [ np.zeros_like( th ), r * np.cos( th ), r * np.sin( th ) ] ), color = COL[ "shaft" ], lw = 0.7,
                    ls = ( 0, ( 1, 2 ) ), z = 60 )
        title( c, name )
        cs.append( c )
    key( cs[0], "rear", ( 0.0, 1.0 ), 10, -10 )
    save( fig, "Propeller_shape_wrap_planform", [ [ c ] for c in cs ], hmax = 8.0 )


# ====================================================================== the tilt kappa on a curved rake

KAPPA_RAKE = ( [ 0.2, 0.4, 0.6, 0.8, 1.0 ], [ 0.0, 0.0075, 0.03, 0.0675, 0.12 ] )


def kappa():
    """A blade raked along a curve: in 3D, and from behind the trailing edge looking toward the leading edge."""
    import openvsp as vsp
    pc = { vsp.PROP_RAKE: ( KAPPA_RAKE[0], KAPPA_RAKE[1], vsp.PCHIP ), vsp.PROP_TWIST: ( [ 0.2, 1.0 ], [ 0.0, 0.0 ], vsp.LINEAR ) }
    prop = geom.Prop( { "Feather": 0.0 }, pcurves = pc )
    prop.check_pose()
    G = prop.vsp_blades[0]
    nw = prop.nw
    cx = prop.p[ "ConstructXoC" ]
    r1 = G[ :, :, 1 ].max()
    lo = prop.iu( 0.0 )
    hi = prop.iu( 1.0 )
    curve = np.array( [ construct_pt( G, i, nw, cx ) for i in range( lo, hi + 1 ) ] )
    fracs = ( 0.0, 0.25, 0.5, 0.75, 0.97 )
    stations = [ prop.iu( f ) for f in fracs ]

    fig = plt.figure()
    c1 = panel( fig, ISO )
    c2 = panel( fig, TOP )
    ax2 = c2.ax

    # 3D: the blade, its sections at the stations, and the curved construction line.
    c1.surface( G )
    c1.flush()
    te_edge( c1, G, nw )
    for i in stations:
        c1.line( G[ i ], color = COL[ "zrot" ], lw = 1.2, z = 61 )
    c1.line( curve, color = "k", lw = 1.2, ls = ( 0, ( 2, 2 ) ), z = 62 )
    c1.line( np.array( [ O, ( r1 + 1.0 ) * Y ] ), color = "#888888", lw = 0.8, ls = ( 0, ( 6, 2, 1, 2 ) ) )
    c1.triad( O, [ X, Y, Z ], [ r"$X$", r"$Y$", r"$Z$" ], "k", length = 2.0, lw = 1.2 )
    lete( c1, G, 0.6, nw )

    # From behind the trailing edge: the construction line is Rake(r); each section is seen edge on.
    c2.line( curve, color = "k", lw = 1.2, ls = ( 0, ( 2, 2 ) ), z = 62 )
    c2.text( curve[ int( 0.55 * len( curve ) ) ], "Rake", dx = 10, dy = 0, ha = "left" )
    c2.line( np.array( [ curve[ 0 ] @ Y * Y - 0.5 * Y, ( r1 + 1.0 ) * Y ] ), color = "#888888", lw = 0.8, ls = ( 0, ( 6, 2, 1, 2 ) ) )
    c2.text( ( r1 + 1.0 ) * Y, "radial", color = "#666666", dx = 0, dy = 8 )
    half = 1.1
    for i in stations:
        P = G[ i ] - G[ i ].mean( 0 )
        n = np.linalg.svd( P )[2][2]
        t = fl.unit( np.cross( n, Z ) )
        p = construct_pt( G, i, nw, cx )
        Q = c2.v.p2( G[ i ] )
        ax2.fill( Q[:, 0], Q[:, 1], facecolor = COL[ "surf" ], edgecolor = "#333333", lw = 0.8, zorder = 40 )
        c2.line( np.array( [ p - half * t, p + half * t ] ), color = COL[ "zrot" ], lw = 1.4, z = 61 )
    # The tangent, and kappa from the radial direction, at three quarters of the span.
    i = prop.iu( 0.6 )
    k = list( range( lo, hi + 1 ) ).index( i )
    p = curve[ k ]
    tng = fl.unit( curve[ k + 1 ] - curve[ k - 1 ] )
    tng = tng - ( tng @ Z ) * Z
    tng = fl.unit( tng )
    c2.line( np.array( [ p - 3.0 * tng, p + 4.5 * tng ] ), color = COL[ "zrot" ], lw = 1.0, ls = ( 0, ( 5, 2 ) ) )
    c2.line( np.array( [ p, p + 4.5 * Y ] ), color = "#888888", lw = 0.8, ls = ( 0, ( 1, 2 ) ) )
    c2.angle( p, Y, tng, 3.6, COL[ "zrot" ], label = r"$\kappa$", label_r = 1.0, dx = 0, dy = 14, lw = 1.3 )
    c2.text( p - 3.0 * tng, "tangent", color = COL[ "zrot" ], dx = -4, dy = -6, ha = "right" )
    for j in stations[ 1: ]:
        P = G[ j ] - G[ j ].mean( 0 )
        nn = np.linalg.svd( P )[2][2]
        kk = list( range( lo, hi + 1 ) ).index( j )
        tt = curve[ min( kk + 1, len( curve ) - 1 ) ] - curve[ max( kk - 1, 0 ) ]
        assert abs( fl.unit( tt ) @ fl.unit( np.cross( nn, Z ) ) ) < 0.02, j
    ang = np.degrees( np.arctan2( -tng[0], tng[1] ) )
    rr = ( p[1] - 0.0 ) / geom.RADIUS
    sl = np.interp( rr, KAPPA_RAKE[0], np.gradient( KAPPA_RAKE[1], KAPPA_RAKE[0] ) )
    print( "kappa at r/R %.3f: %.2f deg (VSP sections), slope %.3f" % ( rr, ang, sl ) )
    key( c2, "top", ( 1.0, 0.0 ), -10, 0 )
    save( fig, "Propeller_shape_kappa", [ [ c1, c2 ] ] )


# ====================================================================== the feather axis at the root

def leader( c, p, s, off, color = "k" ):
    """A label set off from its point, with a thin line back to it."""
    c.ax.annotate( s, xy = c.v.p2( p ), xytext = off, textcoords = "offset points", ha = "left", va = "center", color = color,
                   zorder = 90, arrowprops = dict( arrowstyle = "-", lw = 0.7, color = color, shrinkA = 2, shrinkB = 2 ) )


FA_SET = ( 0.3, 0.25 )      # Feather Axis X/C and Feather Offset for the feather axis figure


def feather_axis():
    """The default blade from the hub: its root section (outline) sets where the feather axis is; every section moves with it."""
    fa, fo = FA_SET
    before = geom.Prop( { "Feather": 0.0 }, nw = CX_NW )
    after = geom.Prop( { "Feather": 0.0, "FeatherAxisXoC": fa, "FeatherOffsetXoC": fo }, nw = CX_NW )
    after.check_pose()
    nw = after.nw
    ir = after.iu( 0.0 )
    im = after.iu( 0.5 )
    Gb = before.vsp_blades[0]
    Ga = after.vsp_blades[0]
    le = Ga[ ir, nw // 2 ]
    te = Ga[ ir, 0 ]
    fpt = le + fa * ( te - le )
    off = after.croot * fo * Z
    assert np.abs( ( fpt - off )[ [ 0, 2 ] ] ).max() < 1e-9
    # Every section moves by the same amount.
    assert np.abs( ( Ga[ im ] - Gb[ im ] ) - ( Ga[ ir ] - Gb[ ir ] ) ).max() < 1e-9
    u = fl.unit( te - le )
    fig, ax, c = new( FRONT )
    axes2d( c, 2.0 )
    fp.sec_draw( c, Gb[ im ], ghost = True )
    fp.sec_draw( c, Ga[ im ] )
    Q = c.v.p2( Ga[ ir ] )
    c.pts.append( Q )
    ax.plot( Q[:, 0], Q[:, 1], color = SEC_COL[ "root" ], lw = 1.8, zorder = 30 )
    c.text( Gb[ im, 0 ], "mid section, before", color = "#777777", dx = 10, dy = 0, ha = "left" )
    c.text( Ga[ im, 0 ], "mid section", dx = 10, dy = 0, ha = "left" )
    c.text( te, "root section", color = SEC_COL[ "root" ], dx = 10, dy = -4, ha = "left" )
    c.line( np.array( [ le - 0.8 * u, te + 0.8 * u ] ), color = SEC_COL[ "root" ], lw = 0.9, ls = ( 0, ( 6, 2, 1, 2 ) ) )
    # Both are moves, so both are black arrows: Feather Axis X/C along the root chord, Feather Offset along Z.
    c.arrow( le, fpt, color = "k", lw = 1.8, head = 10 )
    leader( c, 0.5 * ( le + fpt ), r"Feather Axis X/C $\times\,c_r$, along the root chord", ( -250, 40 ) )
    c.arrow( O, off, color = "k", lw = 1.8, head = 10 )
    leader( c, 0.5 * off, r"Feather Offset $\times\,c_r$, along $Z$", ( 110, -110 ) )
    c.hinge( off, Y, COL[ "feather" ], half = 0.01, rho = 0.22, at = 0.0, arrow_on_axis = False, sweep = 290, lw = 1.8 )
    leader( c, off + 0.22 * X, "feather axis", ( 120, -60 ), COL[ "feather" ] )
    key( c, "front", ( 1.0, 0.0 ), -30, 0 )
    save( fig, "Propeller_shape_feather", [ [ c ] ] )


def shaping():
    # One offset pair only, Rake and Skew, as a blade is usually laid out.
    """The blade-shaping figures: Propeller_shape_twist, _rakeskew, _sweep, _kappa, _feather, _wrap_planform, _result."""
    s = dict( fp.SHAPE, Axial = 0.0, Tangential = 0.0, Construct = 0.5, FeatherAxis = 0.5, FeatherOffset = 0.0, Twist = 35.0,
              Rake = 0.8, Skew = 2.2 )
    af = pm.naca4( 0.12, 0.04, 0.4, 61 )
    steps = pm.shaping_steps( s )
    M = dict( steps )
    chordpts = np.array( [ [ 0.0, 0.0 ], [ s[ "Construct" ], 0.0 ], [ 1.0, 0.0 ] ] )   # LE, Construct X/C, TE

    def run( upto, pts2, skip = () ):
        P = np.column_stack( [ pts2, np.zeros( len( pts2 ) ) ] )
        for n, m in steps:
            if n not in skip:
                P = pm.apply( m, P )
            if n == upto:
                return P
        return P

    def st( upto, skip = () ):
        return run( upto, af, skip ), run( upto, chordpts, skip )

    ux = pm.apply_dir( M[ "twist" ], X )
    uz = pm.apply_dir( M[ "twist" ], Z )

    def front( name, draw ):
        fig, ax, c = new( FRONT )
        draw( c )
        c.bounds = bounds0
        key( c, "front", ( 1.0, 0.0 ), -30, 10 )
        save( fig, name, [ [ c ] ] )

    bounds0 = ( ( -4.2, -3.9 ), ( 4.2, 4.2 ) )

    # The section laid in place: chord along Z, its Construct X/C point at the origin.
    P, CP = st( "construct" )
    def d_section( c ):
        axes2d( c )
        fp.sec_draw( c, P )
        dot( c, CP[1] )
        c.text( CP[0], "LE", dx = 14, dy = 0 )
        c.text( CP[2], "TE", dx = 14, dy = 0 )
        c.text( CP[1], "Construct X/C", color = COL[ "move" ], dx = 12, dy = 0, ha = "left" )

    # The twist about the radial axis, through the Construct X/C point.
    Ptw, CPtw = st( "twist", skip = ( "zrotate", "rakeskew" ) )
    def d_twist( c ):
        axes2d( c )
        fp.sec_draw( c, P, ghost = True )
        fp.sec_draw( c, Ptw )
        c.line( np.array( [ -3.0 * uz, 3.0 * uz ] ), color = COL[ "feather" ], lw = 1.1, ls = ( 0, ( 8, 3, 2, 3 ) ) )
        c.angle( O, -Z, -uz, 2.6, COL[ "feather" ], label = "Twist", label_r = 1.2, lw = 1.3 )
    front( "Propeller_shape_twist", d_twist )

    # Rake and Skew, normal to and along the twisted chord line.
    Prs, CPrs = st( "twist", skip = ( "zrotate", ) )
    def d_rakeskew( c ):
        axes2d( c )
        fp.sec_draw( c, Ptw, ghost = True )
        fp.sec_draw( c, Prs )
        c.line( np.array( [ -3.0 * uz, 3.0 * uz ] ), color = COL[ "feather" ], lw = 1.1, ls = ( 0, ( 8, 3, 2, 3 ) ) )
        a = s[ "Rake" ] * ux
        b = a + s[ "Skew" ] * uz
        c.arrow( O, a, color = COL[ "move" ], lw = 1.8, head = 10 )
        c.arrow( a, b, color = COL[ "move" ], lw = 1.8, head = 10 )
        c.text( 0.5 * a, "Rake", dx = -6, dy = -14, ha = "right" )
        c.text( 0.5 * ( a + b ), "Skew", dx = -12, dy = 0, ha = "right" )
        # The same point by Axial and Tangential, which act along X and Z after the twist.
        ax_ = ( b @ X ) * X
        at = dict( s, Rake = 0.0, Skew = 0.0, Axial = b @ X, Tangential = b @ Z )
        Pat = pm.shape_section( dict( at, ZRotate = 0.0 ), np.array( [ [ at[ "Construct" ], 0.0 ] ] ), upto = "axialtan" )[0]
        assert np.abs( Pat - b ).max() < 1e-9
        c.arrow( O, ax_, color = ATCOL, lw = 1.6, head = 10, ls = ( 0, ( 4, 2 ) ) )
        c.arrow( ax_, b, color = ATCOL, lw = 1.6, head = 10, ls = ( 0, ( 4, 2 ) ) )
        c.text( 0.75 * ax_, "Axial", color = ATCOL, dx = 0, dy = -13, style = "italic" )
        c.text( 0.5 * ( ax_ + b ), "Tangential", color = ATCOL, dx = 10, dy = 0, ha = "left", style = "italic" )
        dot( c, b )
        assert np.abs( b - CPrs[1] ).max() < 1e-9
    front( "Propeller_shape_rakeskew", d_rakeskew )

    # Out to the radius and around by Sweep, from upstream: the shaft points at the reader.
    # A plain section, Construct X/C 0.5 and no offsets, so it starts centered on the radial line.
    ssw = dict( s, Rake = 0.0, Skew = 0.0, ZRotate = 0.0, RootChord = s[ "Chord" ], RootTwist = s[ "Twist" ] )
    Pr = pm.shape_section( ssw, af, upto = "radius" )
    Psw = pm.shape_section( ssw, af, upto = "sweep" )
    cr0 = pm.shape_section( ssw, np.array( [ [ 0.5, 0.0 ] ] ), upto = "radius" )[0]
    assert np.abs( cr0 - ssw[ "Radius" ] * Y ).max() < 1e-12
    fig, ax, c = new( UPSTREAM )
    q = c.v.p2( O )
    fl.out_of_page( ax, q, "" )
    c.text( O, "rotation axis", color = COL[ "shaft" ], dx = 14, dy = -14, ha = "left" )
    # Seen along the shaft, a section is nearly edge on: draw it as a heavy line.
    c.line( Pr, color = "#777777", lw = 1.6, ls = ( 0, ( 3, 2 ) ) )
    c.line( Psw, color = "#222222", lw = 2.6 )
    d = pm.apply_dir( pm.Rx( s[ "Sweep" ] ), Y )
    rs = ssw[ "Radius" ]
    c.line( np.array( [ O, rs * Y ] ), color = COL[ "ghost" ], lw = 0.9, ls = ( 0, ( 1, 2 ) ) )
    c.line( np.array( [ O, rs * d ] ), color = COL[ "shaft" ], lw = 0.9, ls = ( 0, ( 1, 2 ) ) )
    c.text( 0.55 * rs * Y, r"$r$", dx = 0, dy = -12 )
    # The Sweep angle at the shaft, and the path of the section's center: the same point, turned about the shaft.
    arc_label( c, Y, d, 0.4 * rs, "Sweep", COL[ "shaft" ] )
    c.angle( O, Y, d, rs, COL[ "shaft" ], lw = 1.4 )
    c.bounds = ( ( -s[ "Radius" ] - 2.5, -3.0 ), ( 2.0, 4.6 ) )
    key( c, "upstream", ( 1.0, 0.0 ), -30, 10 )
    save( fig, "Propeller_shape_sweep", [ [ c ] ] )

    kappa()
    feather_axis()
    wrap_planform()


    # The result: the whole blade.
    prop = geom.Prop( { "Feather": 0.0 } )
    fig, ax, c = new()
    c.surface( prop.vsp_blades[0] )
    c.flush()
    te_edge( c, prop.vsp_blades[0], prop.nw )
    fp.hub( c )
    c.line( np.array( [ O, 17.0 * Y ] ), color = COL[ "feather" ], lw = 1.4, ls = ( 0, ( 5, 2 ) ) )
    c.text( 17.0 * Y, "feather axis", color = COL[ "feather" ], dx = -6, dy = -12, ha = "right" )
    c.triad( O, [ X, Y, Z ], [ r"$X$", r"$Y$", r"$Z$" ], "k", length = 2.0, lw = 1.2 )
    lete( c, prop.vsp_blades[0], 0.7, prop.nw )
    save( fig, "Propeller_shape_result", [ [ c ] ] )


# ====================================================================== hinges

ex = np.array( [ 1.0, 0, 0 ] )
ey = np.array( [ 0, 1.0, 0 ] )
ez = np.array( [ 0, 0, 1.0 ] )
bdir = fp.bdir
ANG = { "flap": ( 20.0, 30.0 ), "lag": ( 20.0, 25.0 ), "fold": ( 15.0, 20.0 ) }


def hinge_dirs( kind ):
    """The starting axis, the axis after the first turn, and the final axis, in blade axes."""
    az, el = ANG[ kind ]
    if kind == "lag":
        base = -ez
        mid = pm.R3( ex, -el ) @ base
    else:
        base = ex if kind == "flap" else -ex
        mid = pm.R3( ez, -el ) @ base
    return base, mid, pm.R3( ey, az ) @ mid


NAMES = { "flap": ( r"$\delta_3$", r"$\theta_F$" ), "lag": ( r"$\alpha_1$", r"$\theta_L$" ), "fold": ( "El", "Az" ) }


def view_from( eye, rot = None, up = Z ):
    """Looking from the model direction eye toward the origin, with up (model) up the page."""
    out = pm.unit_or( eye )
    right = pm.unit_or( np.cross( up, out ) )
    return view( right, np.cross( out, right ), rot )


ABOVE_H = view_from( [ -1.0, -1.0, 2.6 ], HELI )      # a helicopter rotor, mostly from above
HINGE_E = 0.5                                      # hinge place for these figures, out from the defaults so it shows


_STILL = {}


def still_blade():
    """The one-blade rotor of the hinge figures, with no motion."""
    if "p" not in _STILL:
        _STILL[ "p" ] = geom.Prop( dict( fp.ILLUS_HINGES, NumBlade = 1, Feather = 0.0 ), root = fp.ILLUS_ROOT, rotor = True )
    return _STILL[ "p" ]


def new_h():
    """A hinge figure: one view across the page width; hinge_frame sizes it."""
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    ax = fig.add_axes( [ 0.0, 0.0, 1.0, 1.0 ] )
    return fig, ax, fl.Canvas( ax, ABOVE_H )


def hinge_frame( c, pad = 0.16, hmax = 5.4 ):
    """One scope and scale for every hinge figure: the rotor center and the whole blade at rest, in ABOVE_H.  Sets the
    figure's height."""
    G = still_blade().vsp_blades[0]
    q = c.v.p2( np.vstack( [ G.reshape( -1, 3 ), [ O ], [ -2.0 * bdir( ey ) ] ] ) )
    lo = q.min( 0 )
    hi = q.max( 0 )
    span = hi - lo
    lo = lo - pad * span
    hi = hi + pad * span
    d = hi - lo
    w = c.ax.get_position( original = True ).width * fl.PAGE_W
    s = min( w / d[0], hmax / d[1] )

    h = s * d[1]
    c.ax.figure.set_size_inches( fl.PAGE_W, h )
    mid = 0.5 * ( lo + hi )
    c.finish( xlim = ( mid[0] - 0.5 * w / s, mid[0] + 0.5 * w / s ), ylim = ( mid[1] - 0.5 * h / s, mid[1] + 0.5 * h / s ) )



def rotor_silhouette( c ):
    """The blade, greyed, for orientation: one plain blade with no motion."""
    sil = still_blade()
    for i in range( sil.nblade ):
        c.surface( sil.vsp_blades[ i ], color = "#ECECEC", edge = False )
    c.flush()
    for i in range( sil.nblade ):
        c.line( sil.vsp_blades[ i ][ :, 0 ], color = "#888888", lw = 1.8 )


def blade_triad( c, length = 3.0 ):
    """The blade axes x_B, y_B, z_B at the rotor center."""
    c.triad( O, [ bdir( ex ), bdir( ey ), bdir( ez ) ], [ r"$x_B$", r"$y_B$", r"$z_B$" ], COL[ "blade" ], length = length,
             lw = 1.6, label_pad = 1.22 )


LG = 2.6                   # length of every axis glyph from the hinge point
LA = 4.0                   # length of the hinge axis vector
C1 = "#1F4E79"             # the first turn
GREY = "#777777"


def end_label( c, p, a, s, color, gap = 9.0 ):
    """A label just past the arrow tip at p + ( LG + 0.25 ) a, set off along the axis as drawn."""
    q = c.v.p2( np.array( [ p, p + a ] ) )
    u = q[1] - q[0]
    u = u / max( np.linalg.norm( u ), 1e-9 )
    c.text( p + ( LG + 0.25 ) * a, s, color = color, dx = gap * u[0], dy = gap * u[1] )


def hinge_construction( kind, scale = None ):
    """The hinge axis built by two turns of one vector of length LA from the hinge point; each angle is the path of
    its tip.  Returns the scale, so the three hinges share one."""
    base, mid, fin = hinge_dirs( kind )
    az, el = ANG[ kind ]
    n1, n2 = NAMES[ kind ]
    col = COL[ kind ]
    # VSP turns the lag axis by -alpha_1 about x_B and the others by -delta_3 or -El about z_B, so the first turn is
    # drawn positive about -x_B or -z_B.  The third axis is the one nothing turns about.
    if kind == "lag":
        a1, a1_name, a3, a3_name = -ex, r"$-x_B$", ez, r"$z_B$"
    else:
        a1, a1_name, a3, a3_name = -ez, r"$-z_B$", ex, r"$x_B$"
    assert np.abs( pm.R3( a1, el ) @ base - mid ).max() < 1e-12
    assert np.abs( pm.R3( ey, az ) @ mid - fin ).max() < 1e-12
    hp = HINGE_E * R * bdir( ey )
    foot = hp + LA * ( mid @ ey ) * bdir( ey )

    def tip( v ):
        return hp + LA * bdir( v )

    fig = plt.figure()
    c = panel( fig, ABOVE_H )
    rotor_silhouette( c )
    # The blade axes at their origin, the rotor center; the axes at the hinge point show only their directions.
    blade_triad( c )
    # Three axes from the hinge point, one length: the two turned about with their curls, the third plain.
    for a, name, ac in ( ( a1, a1_name, C1 ), ( ey, r"$y_B$", col ) ):
        c.hinge( hp, bdir( a ), ac, half = LG, start = 0.0, rho = 0.28, at = 0.55, lw = 1.3 )
        end_label( c, hp, bdir( a ), name, ac )
    c.arrow( hp, hp + ( LG + 0.25 ) * bdir( a3 ), color = GREY, lw = 1.3, head = 7 )
    end_label( c, hp, bdir( a3 ), a3_name, GREY )
    # The start (dashed), after the first turn (dotted) and the final axis, each LA each way through the hinge point.
    c.arrow( hp - LA * bdir( base ), tip( base ), color = GREY, lw = 1.3, head = 8, ls = ( 0, ( 4, 2 ) ) )
    c.arrow( hp - LA * bdir( mid ), tip( mid ), color = GREY, lw = 1.3, head = 8, ls = ( 0, ( 1, 1.5 ) ) )
    c.arrow( hp - LA * bdir( fin ), tip( fin ), color = col, lw = 2.4, head = 11 )
    c.point( hp, color = col, s = 18 )
    # The second turn's circle about y_B: its center on the y_B line, and spokes to the two tips.
    c.line( np.array( [ hp, foot ] ), color = col, lw = 0.8, ls = ( 0, ( 6, 2, 1, 2 ) ) )
    for v in ( mid, fin ):
        c.line( np.array( [ foot, tip( v ) ] ), color = col, lw = 0.9, ls = ( 0, ( 2, 2 ) ) )
    c.point( foot, color = col, s = 8 )
    # The paths of the tip, each labeled outside its arc.
    t = np.linspace( 0.0, 1.0, 40 )
    P1 = np.array( [ tip( pm.R3( a1, el * k ) @ base ) for k in t ] )
    P2 = np.array( [ tip( pm.R3( ey, az * k ) @ mid ) for k in t ] )
    for P, name, ac in ( ( P1, n1, C1 ), ( P2, n2, col ) ):
        c.arrow_path( P, color = ac, lw = 2.0, head = 9 )
        q = c.v.p2( np.array( [ hp, P[ 20 ] ] ) )
        u = fl.unit( q[1] - q[0] )
        c.text( P[ 20 ], name, color = ac, dx = 12 * u[0], dy = 12 * u[1] )

    B = [ tip( v ) for v in ( base, mid, fin ) ] + [ hp - LA * bdir( v ) for v in ( base, mid, fin ) ]
    B += [ hp + ( LG + 0.6 ) * bdir( a ) for a in ( a1, ey, a3 ) ]
    B += [ O ] + [ 3.6 * bdir( a ) for a in ( ex, ey, ez ) ]
    q = c.v.p2( np.array( B ) )
    c.bounds = ( q.min( 0 ), q.max( 0 ) )
    s = fl.fit( fig, [ [ c ] ], scale = scale, hmax = 4.4 )
    save( fig, "Propeller_%s_axis" % kind )
    return s


def hinge_point():
    """Propeller_hinge_point: a hinge point placed by its Radial, Offset and Axial fractions of R."""
    fig, ax, c = new_h()
    rotor_silhouette( c )
    blade_triad( c )
    hpB = np.array( [ 2.6, HINGE_E * R, -2.4 ] )
    P = bdir( hpB )
    a1 = bdir( [ 0, hpB[1], 0 ] )
    a2 = bdir( [ hpB[0], hpB[1], 0 ] )
    c.arrow( O, a1, color = COL[ "move" ], lw = 1.8, head = 10 )
    c.arrow( a1, a2, color = COL[ "move" ], lw = 1.8, head = 10 )
    c.arrow( a2, P, color = COL[ "move" ], lw = 1.8, head = 10 )
    c.text( 0.6 * a1, r"Radial$\,R$", dx = -12, dy = -12, ha = "right" )
    c.text( 0.5 * ( a1 + a2 ), r"Offset$\,R$", dx = 0, dy = 14 )
    c.text( 0.5 * ( a2 + P ), r"Axial$\,R$", dx = 14, dy = 0, ha = "left" )
    c.point( P, color = "k", s = 18 )
    c.text( P, "hinge point", dx = 12, dy = -10, ha = "left" )
    hinge_frame( c )
    save( fig, "Propeller_hinge_point" )


HINGE_DEFL = { "flap": ( "FlapAngle", 20.0 ), "lag": ( "LagAngle", 15.0 ), "fold": ( "FoldAngle", 40.0 ), "feather": ( "Feather", 30.0 ) }


def hinge_motion( kind ):
    """The blade turned about one hinge alone, its undeflected self as a ghost; VSP's own blade, checked."""
    parm, val = HINGE_DEFL[ kind ]
    base = dict( fp.ILLUS_HINGES, NumBlade = 1, Feather = 0.0 )
    still = still_blade()
    moved = geom.Prop( dict( base, **{ parm: val } ), root = fp.ILLUS_ROOT, rotor = True )
    moved.check_pose()
    spec = fp.hinge_draw_spec( moved )
    fig, ax, c = new_h()
    c.surface( still.vsp_blades[0], ghost = True )
    c.surface( moved.vsp_blades[0] )
    c.flush()
    te_edge( c, moved.vsp_blades[0], moved.nw )
    blade_triad( c )
    sp = spec[ kind ]
    if kind == "feather":
        c.line( np.array( [ O, fp.AXIS_LEN * fl.unit( sp[ "axis" ] ) ] ), color = COL[ kind ], lw = 1.2, ls = ( 0, ( 5, 2 ) ) )
        c.hinge( sp[ "p" ], sp[ "axis" ], COL[ kind ], half = 0.01, rho = 0.7, at = 0.0, arrow_on_axis = False, lw = 2.2 )
    else:
        c.hinge( sp[ "p" ], sp[ "axis" ], COL[ kind ], half = 2.4, rho = 0.6, at = 0.85, lw = 2.4 )
    hinge_frame( c )
    save( fig, "Propeller_%s_motion" % kind )


def hinges():
    """The hinge figures: Propeller_<flap|lag|fold>_axis and _motion, Propeller_hinge_point, Propeller_feather_motion."""
    s = None
    for k in ( "flap", "lag", "fold" ):
        s = hinge_construction( k, s )
        hinge_motion( k )
    hinge_point()
    hinge_motion( "feather" )


# ====================================================================== the placement chain, one image per factor

def chain():
    """The four hinges on the blade as built, the precone, and the whole rotor with every motion."""
    still = still_blade()
    spec = fp.hinge_draw_spec( still )
    fig, ax, c = new_h()
    c.surface( still.vsp_blades[0] )
    c.flush()
    te_edge( c, still.vsp_blades[0], still.nw )
    blade_triad( c )
    fp.draw_hinges( c, spec, labels = False )
    # The hinges sit close together inboard of the blade root: label each from a clear spot.
    for k, off in ( ( "flap", ( 40, -50 ) ), ( "lag", ( -70, -60 ) ), ( "fold", ( -90, 10 ) ), ( "feather", ( 50, 40 ) ) ):
        leader( c, spec[ k ][ "p" ], k, off, COL[ k ] )
    hinge_frame( c )
    save( fig, "Propeller_place_hinges" )

    pre = geom.Prop( dict( fp.ILLUS_HINGES, NumBlade = 1, Feather = 0.0, Precone = 10.0 ), root = fp.ILLUS_ROOT, rotor = True )
    pre.check_pose()
    fig, ax, c = new_h()
    c.surface( still.vsp_blades[0], ghost = True )
    c.surface( pre.vsp_blades[0] )
    c.flush()
    te_edge( c, pre.vsp_blades[0], pre.nw )
    blade_triad( c )
    c.hinge( O, Z, COL[ "precone" ], half = 2.0, rho = 0.6, at = 0.85, lw = 2.2 )
    hinge_frame( c )
    save( fig, "Propeller_place_precone" )

    prop = fp.illus_prop()
    G0 = prop.G0
    steps = [ st for st in prop.chain( 0 ) if st[0] != "PsiRef" ]
    fig, ax, c = new( ABOVE_H )
    for ib in range( prop.nblade ):
        G = pm.apply( pm.product( prop.chain( ib ) ), G0 )
        c.surface( G, color = COL[ "surf" ] if ib == 0 else "#DADDE1" )
    c.flush()
    acc = np.eye( 4 )
    for n, m in steps:
        if n in spec:
            fp.draw_hinges( c, spec, M = acc, which = [ n ], labels = False )
        acc = acc @ m
    for ib in range( prop.nblade ):
        G = pm.apply( pm.product( prop.chain( ib ) ), G0 )
        te_edge( c, G, prop.nw, lw = 1.4 )
        c.text( G[ -1, prop.nw // 2 ], "%d" % ( ib + 1 ), dx = 0, dy = 12 )
    model_triad( c, -14.0 * Y + 4.0 * Z, HELI, length = 2.0 )
    save( fig, "Propeller_place_rotor", [ [ c ] ] )


# ====================================================================== azimuth, in helicopter position, seen from above

def arc_label( c, d_from, d_to, rho, text, color, pad = 12.0, axis = None ):
    """An angle arc about the shaft at radius rho, labelled just outside its middle."""
    c.angle( O, d_from, d_to, rho, color, lw = 1.4 )
    mid = pm.unit_or( d_from + d_to )
    q = c.v.p2( np.array( [ O, mid ] ) )
    u = ( q[1] - q[0] ) / max( np.linalg.norm( q[1] - q[0] ), 1e-12 )
    ha = "left" if u[0] > 0.3 else ( "right" if u[0] < -0.3 else "center" )
    c.text( rho * mid, text, color = color, dx = pad * u[0], dy = pad * u[1], ha = ha )


def draw_azimuth( c, prop, rev ):
    """One azimuth figure, normal or reverse."""
    for i in range( prop.nblade ):
        c.surface( prop.vsp_blades[ i ], color = COL[ "surf" ] if i == 0 else "#DADDE1", edge = False )
    c.flush()
    for i in range( prop.nblade ):
        G = prop.vsp_blades[ i ]
        c.line( G[ :, 0 ], color = "#222222", lw = 2.2 )
        c.line( G[ :, prop.nw // 2 ], color = "#777777", lw = 0.6 )
    p = prop.p
    ref = p[ "PsiRef" ]
    rot = p[ "Rotate" ]
    d_ref = fp.azdir( ref, rev )
    d1 = fp.azdir( ref + rot, rev )
    d2 = fp.azdir( ref + rot + p[ "BladeAz_2" ], rev )
    c.line( np.array( [ O, 21.0 * Y ] ), color = "k", lw = 1.0 )
    c.text( 21.0 * Y, r"$+Y$", dx = 0, dy = 12 )
    c.line( np.array( [ O, 21.0 * d_ref ] ), color = COL[ "shaft" ], lw = 1.1, ls = ( 0, ( 6, 2 ) ) )
    q = c.v.p2( np.array( [ O, d_ref ] ) )
    u = ( q[1] - q[0] ) / np.linalg.norm( q[1] - q[0] )
    c.text( 21.0 * d_ref, r"$\psi = 0$", color = COL[ "shaft" ], dx = 16 * u[0], dy = 16 * u[1] )
    for d in ( d1, d2 ):
        c.line( np.array( [ O, 19.0 * d ] ), color = "#888888", lw = 0.8, ls = ( 0, ( 2, 2 ) ) )
    arc_label( c, Y, d_ref, 18.5, "PsiRef", COL[ "shaft" ] )
    arc_label( c, d_ref, d1, 16.0, r"Rotate $\psi$", COL[ "shaft" ] )
    arc_label( c, d1, d2, 11.0, r"$\phi_2$", COL[ "move" ] )
    A = c.arc_pts( O, -rev * X, 6.0, 0.0, rot + p[ "BladeAz_2" ], ref = d_ref )
    c.arrow_path( A, color = "k", lw = 1.3, head = 8 )
    c.text( A[ len( A ) // 2 ] * 0.6, r"$\psi_2$" )
    A = c.arc_pts( O, -rev * X, 22.0, 200.0, 250.0, ref = Y )
    c.arrow_path( A, color = COL[ "shaft" ], lw = 2.0, head = 11 )
    q = c.v.p2( np.array( [ O, A[ len( A ) // 2 ] ] ) )
    u = ( q[1] - q[0] ) / np.linalg.norm( q[1] - q[0] )
    c.text( A[ len( A ) // 2 ], r"$\Omega$", color = COL[ "shaft" ], dx = 16 * u[0], dy = 16 * u[1] )
    for i in range( prop.nblade ):
        G = prop.vsp_blades[ i ]
        q = c.v.p2( np.array( [ O, G[ -1, prop.nw // 2 ] ] ) )
        u = ( q[1] - q[0] ) / np.linalg.norm( q[1] - q[0] )
        c.text( G[ -1, prop.nw // 2 ], "%d" % ( i + 1 ), dx = 12 * u[0], dy = 12 * u[1] )


def azimuth():
    """Propeller_azimuth_normal and _reverse: PsiRef, Rotate and the blade azimuths, from above."""
    for rev in ( False, True ):
        prop = geom.Prop( fp.AZ_PARMS, rev = rev, rotor = True )
        prop.check_pose()
        fig, ax, c = new( TOP_H )
        draw_azimuth( c, prop, -1.0 if rev else 1.0 )
        key( c, "top", ( 0.0, 0.0 ), 10, 10 )
        save( fig, "Propeller_azimuth_%s" % ( "reverse" if rev else "normal" ), [ [ c ] ] )


# ====================================================================== harmonic motion

def harmonic():
    """Propeller_harmonic_curve, _tip_path and _side: harmonic flap against azimuth, and the tip path."""
    prop = geom.Prop( fp.HARM, rotor = True )
    prop.check_pose()
    p = prop.p

    fig = plt.figure( figsize = ( fl.PAGE_W, 3.2 ) )
    ax = fig.add_axes( [ 0.09, 0.15, 0.86, 0.80 ] )

    psi = np.linspace( 0.0, 360.0, 361 )
    ax.plot( psi, [ pm.harmonic( p, "Flap", s_ ) for s_ in psi ], color = COL[ "flap" ], lw = 2.0 )
    ax.axhline( p[ "FlapMean" ], color = COL[ "flap" ], lw = 0.9, ls = ( 0, ( 4, 2 ) ) )
    ax.text( 362, p[ "FlapMean" ], r" $\beta_0$", color = COL[ "flap" ], va = "center", fontsize = FS )
    for i in range( prop.nblade ):
        s_ = pm.blade_psi( p, i, p[ "Rotate" ] ) % 360.0
        b_ = pm.blade_angles( p, i, p[ "Rotate" ] )[ "flap" ]
        ax.plot( [ s_, s_ ], [ 0, b_ ], color = "#999999", lw = 0.8, ls = ( 0, ( 2, 2 ) ), zorder = 1 )
        ax.plot( [ s_ ], [ b_ ], "o", color = COL[ "flap" ], ms = 8, mec = "white", mew = 1.0, zorder = 5 )
        ax.annotate( "blade %d" % ( i + 1 ), ( s_, b_ ), xytext = ( 0, 10 ), textcoords = "offset points", ha = "center", fontsize = FS )
    pk = np.degrees( np.arctan2( p[ "FlapSin_1" ], p[ "FlapCos_1" ] ) ) % 360.0
    ax.axvline( pk, color = COL[ "flap" ], lw = 0.8, ls = ( 0, ( 1, 2 ) ) )
    ax.text( pk - 4, 0.6, r"peak, $\mathrm{atan2}(\beta_{1s}, \beta_{1c})$", ha = "right", fontsize = FS, color = COL[ "flap" ] )
    ax.set_xlim( 0, 360 )
    ax.set_ylim( 0, None )
    ax.set_xticks( range( 0, 361, 45 ) )
    ax.set_xlabel( r"blade azimuth $\psi_i$ (deg)", fontsize = FS )
    ax.set_ylabel( r"flap angle $\beta$ (deg)", fontsize = FS )
    ax.tick_params( labelsize = FS )
    for sp in ( "top", "right" ):
        ax.spines[ sp ].set_visible( False )
    save( fig, "Propeller_harmonic_curve" )

    path = fp.tip_path( prop )
    ring = np.array( [ 15.0 * fp.azdir( a_ ) for a_ in np.linspace( 0, 360, 121 ) ] )

    fig, ax, c = new( ISO_H )
    for i in range( prop.nblade ):
        c.surface( prop.vsp_blades[ i ], color = COL[ "surf" ] if i == 0 else "#DADDE1", outline_lw = 0.6 )
    c.flush()
    c.line( path, color = COL[ "flap" ], lw = 1.5, ls = ( 0, ( 5, 2 ) ) )
    c.line( ring, color = "#AAAAAA", lw = 0.8, ls = ( 0, ( 1, 2 ) ) )
    fp.hub( c )
    c.line( np.array( [ O, 0.9 * R * Y ] ), color = "k", lw = 0.8, ls = ( 0, ( 3, 2 ) ) )
    for a_, lab in ( ( 0.0, r"$\psi = 0$" ), ( 90.0, r"$\psi = 90^\circ$" ) ):
        d = fp.azdir( a_ )
        c.line( np.array( [ 0.9 * R * d, fp.AXIS_LEN * d ] ), color = "k", lw = 1.2 )
        c.text( 17.5 * d, lab )
    for i in range( prop.nblade ):
        c.text( prop.vsp_blades[ i ][ -1, prop.nw // 2 ], "%d" % ( i + 1 ), dx = 0, dy = 12 )
    model_triad( c, -7.0 * Y + 6.0 * Z, HELI, length = 3.0 )
    save( fig, "Propeller_harmonic_tip_path", [ [ c ] ] )

    fig, ax, c = new( NOSE_H )
    c.line( path, color = COL[ "flap" ], lw = 1.6 )
    c.line( np.array( [ -16.0 * Y, 16.0 * Y ] ), color = "#AAAAAA", lw = 0.8, ls = ( 0, ( 1, 2 ) ) )
    c.line( np.array( [ 3.0 * X, -6.0 * X ] ), color = "#999999", lw = 0.9, ls = ( 0, ( 6, 2, 1, 2 ) ) )
    c.point( path[ 0 ], COL[ "flap" ], s = 14 )
    c.point( path[ len( path ) // 2 ], COL[ "flap" ], s = 14 )
    c.text( path[ 0 ], r"$\psi = 0$", dx = 0, dy = 14 )
    c.text( path[ len( path ) // 2 ], r"$\psi = 180^\circ$", dx = 0, dy = 14 )
    key( c, "upstream", ( 1.0, 0.0 ), 20, 0 )
    save( fig, "Propeller_harmonic_side", [ [ c ] ] )


# ====================================================================== reverse rotation

REV_D = 1.2 * R                                       # each rotor's distance from the mirror plane Y = 0
PSI_TAIL = { False: 270.0, True: 90.0 }               # the PsiRef that puts psi = 0 over the tail (+X), normal and Reverse
MIRROR_Y = np.array( [ 1.0, -1.0, 1.0 ] )


def to_model( P, off ):
    """Points in propeller axes, placed in the model: turned by YRot 90 and moved by off."""
    return np.asarray( P, float ) @ HELI.T + off


def rev_rotors():
    """Side-by-side rotors in helicopter position: normal at Y = -REV_D, Reverse at +REV_D with PsiRef 180 more, so each
    is the other's mirror image in Y = 0, blade by blade."""
    out = []
    for r in ( False, True ):
        prop = fp.illus_prop( dict( fp.MIRROR_PARMS, PsiRef = PSI_TAIL[ r ] ), rev = r )
        side = -1.0
        if r:
            side = 1.0
        out.append( ( prop, side * REV_D * Y ) )
        assert np.abs( HELI @ fp.azdir( PSI_TAIL[ r ], -side ) - X ).max() < 1e-12
    ( a, oa ), ( b, ob ) = out
    err = max( np.abs( to_model( a.vsp_blades[i], oa ) * MIRROR_Y - to_model( b.vsp_blades[i], ob ) ).max() for i in range( a.nblade ) )
    assert err < 1e-9, err
    return out


def blade_frame( prop ):
    """Blade 1's axes, placed by the chain as far as the flap hinge, at 0.55 R along the blade (propeller axes)."""
    rev = 1.0
    if prop.rev:
        rev = -1.0
    P = pm.product( prop.chain( 0 )[ :3 ] )
    return pm.apply( P, 0.55 * R * Y ), [ pm.apply_dir( P, pm.blade_to_prop( e, rev ) ) for e in np.eye( 3 ) ]


def omega( c, hub, rev, rho, lift = 0.0 ):
    """The rotation sense about a rotor's shaft (model Z, thrust up): counterclockwise from above when normal."""
    axis = Z
    if rev:
        axis = -Z
    A = c.arc_pts( hub + lift * Z, axis, rho, 20.0, 290.0, ref = X )
    c.arrow_path( A, color = COL[ "shaft" ], lw = 1.8, head = 10 )
    q = c.v.p2( np.array( [ hub + lift * Z, A[ 20 ] ] ) )
    u = ( q[1] - q[0] ) / np.linalg.norm( q[1] - q[0] )
    c.text( A[ 20 ], r"$\Omega$", color = COL[ "shaft" ], dx = 10 * u[0], dy = 10 * u[1] )


def reverse():
    """Propeller_reverse_3d, _top and _lag: a Reverse rotor, with PsiRef 180 more, as the mirror image in Y = 0."""
    rotors = rev_rotors()
    tint = { False: COL[ "surf" ], True: COL[ "surf2" ] }
    name = { False: r"normal, PsiRef $270^\circ$", True: r"Reverse, PsiRef $90^\circ$" }
    ( pn, on ), ( pr, orr ) = rotors
    # The blade axes mirror with the blades.
    for k in range( 3 ):
        a = HELI @ blade_frame( pn )[1][k]
        b = HELI @ blade_frame( pr )[1][k]
        assert np.abs( a * MIRROR_Y - b ).max() < 1e-12

    fig, ax, c = new( ISO )
    pl = np.array( [ [ -1.2 * R, 0, -0.2 * R ], [ -1.2 * R, 0, 0.3 * R ], [ 1.2 * R, 0, 0.3 * R ], [ 1.2 * R, 0, -0.2 * R ] ] )
    # Seen from the -Y side: the Reverse rotor, then the plane, then the normal rotor.
    for prop, off, z0 in ( ( pr, orr, 1.0 ), ( pn, on, 3.0 ) ):
        for i in range( prop.nblade ):
            c.surface( to_model( prop.vsp_blades[ i ], off ), color = tint[ prop.rev ], edge = False )
        c.flush( z0 )
        if not prop.rev:
            continue
        ax.add_patch( Polygon( c.v.p2( pl ), closed = True, facecolor = "#DDE6EF", edgecolor = "#A9B8C8", lw = 0.8, alpha = 0.7,
                               zorder = 2.0 ) )
        c.pts.append( c.v.p2( pl ) )
        c.text( pl[1], r"$Y = 0$", color = "#6A7F95", dx = 6, dy = -10, ha = "left", halo = False )
    for prop, off in rotors:
        for i in range( prop.nblade ):
            G = to_model( prop.vsp_blades[ i ], off )
            te_edge( c, G, prop.nw, lw = 1.4 )
        c.line( np.array( [ off - 0.25 * R * Z, off + 0.45 * R * Z ] ), color = "#999999", lw = 0.8, ls = ( 0, ( 6, 2, 1, 2 ) ) )

        omega( c, off, prop.rev, 0.22 * R, 0.4 * R )
        c.text( off - 0.25 * R * Z, name[ prop.rev ], dx = 0, dy = -10, va = "top" )
    model_triad( c, -2.4 * REV_D * Y - 0.2 * R * Z, length = 3.0 )
    save( fig, "Propeller_reverse_3d", [ [ c ] ] )

    fig, ax, c = new( TOP )
    c.line( np.array( [ -1.3 * R * X, 1.3 * R * X ] ), color = "#6A7F95", lw = 1.0, ls = ( 0, ( 8, 3 ) ) )
    c.text( 1.3 * R * X, r"$Y = 0$", color = "#6A7F95", dx = 6, dy = 0, ha = "left", halo = False )
    for prop, off in rotors:
        for i in range( prop.nblade ):
            c.surface( to_model( prop.vsp_blades[ i ], off ), color = tint[ prop.rev ], edge = False )
    c.flush()
    for prop, off in rotors:
        for i in range( prop.nblade ):
            G = to_model( prop.vsp_blades[ i ], off )
            te_edge( c, G, prop.nw, lw = 1.4 )
            tip = G[ -1, prop.nw // 2 ]
            q = c.v.p2( np.array( [ off, tip ] ) )
            u = ( q[1] - q[0] ) / np.linalg.norm( q[1] - q[0] )
            c.text( tip, "%d" % ( i + 1 ), dx = 10 * u[0], dy = 10 * u[1] )
        c.line( np.array( [ off, off + 1.15 * R * X ] ), color = COL[ "shaft" ], lw = 1.0, ls = ( 0, ( 6, 2 ) ) )
        c.text( off + 1.15 * R * X, r"$\psi = 0$", color = COL[ "shaft" ], dx = 8, dy = 0, ha = "left" )
        omega( c, off, prop.rev, 0.3 * R )
        c.text( off - 1.1 * R * X, name[ prop.rev ], dx = -6, dy = 0, ha = "right" )
    key( c, "top", ( 0.0, 0.0 ), 0, -30 )
    save( fig, "Propeller_reverse_top", [ [ c ] ] )

    # A turn's axis under the mirror: the curl mirrors, so the axis points opposite to the arrow's plain mirror image.
    fig, ax, c = new()
    pl = np.array( [ [ -2.4, 0, -2.6 ], [ -2.4, 0, 2.6 ], [ 2.4, 0, 2.6 ], [ 2.4, 0, -2.6 ] ], float )
    ax.add_patch( Polygon( c.v.p2( pl ), closed = True, facecolor = "#EEF3F8", edgecolor = "#A9B8C8", lw = 0.8, zorder = 0.5 ) )
    c.pts.append( c.v.p2( pl ) )
    c.text( pl[2], r"$Y = 0$", color = "#6A7F95", dx = 6, dy = -8, ha = "left", halo = False )
    c.hinge( -2.0 * Y, Z, COL[ "lag" ], half = 2.0, rho = 0.7, at = 0.7, lw = 2.0 )
    c.text( -2.0 * Y - 2.0 * Z, "normal", color = COL[ "lag" ], dy = -12 )
    c.hinge( 2.0 * Y, -Z, COL[ "lag" ], half = 2.0, rho = 0.7, at = 0.7, lw = 2.0 )
    c.text( 2.0 * Y + 2.0 * Z, "Reverse", color = COL[ "lag" ], dy = 22 )
    c.arrow( 2.0 * Y - 2.0 * Z + 1.0 * X, 2.0 * Y + 2.0 * Z + 1.0 * X, color = COL[ "lag" ], lw = 1.2, head = 8, ls = ( 0, ( 2, 2 ) ) )
    c.triad( -4.5 * Y - 2.0 * Z - 1.5 * X, [ X, Y, Z ], [ r"$X$", r"$Y$", r"$Z$" ], "k", length = 1.4, lw = 1.0 )
    save( fig, "Propeller_reverse_lag", [ [ c ] ] )


# ====================================================================== blade matrices

XF_PLACE = { ( "XForm", "X_Rel_Location" ): 30.0, ( "XForm", "Y_Rel_Location" ): 6.0, ( "XForm", "Z_Rel_Location" ): 12.0,
             ( "XForm", "Y_Rel_Rotation" ): 90.0 }


def xform():
    """The blade matrices of a rotor placed away from the origin: the blade as built, at psi = 0, and at psi."""
    import openvsp as vsp
    rot = 50.0
    parms = { "NumBlade": 4, "Feather": 0.0 }
    built = geom.Prop( parms, rotor = True )
    pid = geom.new_prop( parms, rotor = True, curves = XF_PLACE )

    def mat( m ):
        o = m.xform( vsp.vec3d( 0.0, 0.0, 0.0 ) )
        M = np.eye( 4 )
        M[ :3, 3 ] = [ o.x(), o.y(), o.z() ]
        for j, e in enumerate( ( ( 1.0, 0.0, 0.0 ), ( 0.0, 1.0, 0.0 ), ( 0.0, 0.0, 1.0 ) ) ):
            q = m.xform( vsp.vec3d( *e ) )
            M[ :3, j ] = [ q.x() - o.x(), q.y() - o.y(), q.z() - o.z() ]
        return M
    T0 = mat( vsp.GetPropBladeXForm( pid, 0, 0, 0.0 ) )
    Tr = mat( vsp.GetPropBladeXForm( pid, 0, 0, rot ) )
    Mm = mat( vsp.GetPropBladeMotionXForm( pid, 0, 0, rot ) )
    G0 = built.G0
    # Checks: T(0) places the blade VSP draws, is the placement alone (a turn of 90 about Y), and the motion is T(psi) T(0)^-1.
    assert np.abs( pm.apply( T0, G0 ) - geom.grid( pid, 0, built.u ) ).max() < 1e-9
    assert np.abs( T0[ :3, :3 ] - pm.R3( Y, 90.0 ) ).max() < 1e-12
    assert np.abs( Mm - Tr @ np.linalg.inv( T0 ) ).max() < 1e-9
    hub = T0[ :3, 3 ]

    fig, ax, c = new( ISO )
    c.surface( G0, color = "#CDB8E8" )
    for i in range( 1, 4 ):
        c.surface( geom.grid( pid, i, built.u ), color = "#ECECEC", edge = False )
    c.surface( pm.apply( T0, G0 ), color = "#DADDE1" )
    c.surface( pm.apply( Tr, G0 ), color = COL[ "surf" ] )
    c.flush()
    te_edge( c, pm.apply( Tr, G0 ), built.nw, lw = 1.4 )

    def frame( M, color, labels, length = 5.0 ):
        o = M[ :3, 3 ]
        vecs = [ M[ :3, :3 ] @ bdir( e ) for e in ( ex, ey, ez ) ]
        c.triad( o, vecs, labels, color, length = length, lw = 1.5, label_pad = 1.3 )
    names = [ r"$x_B$", r"$y_B$", r"$z_B$" ]
    frame( np.eye( 4 ), COL[ "blade" ], names )
    frame( T0, "#9A9A9A", [ "", "", "" ] )
    frame( Tr, COL[ "blade" ], names )
    model_triad( c, -14.0 * Y - 6.0 * X, length = 4.0 )
    c.text( -14.0 * Y - 6.0 * X, "model axes", dx = 0, dy = -14 )
    mid = G0[ built.iu( 0.7 ), built.nw // 2 ]
    c.text( mid, "as built", dx = 0, dy = 16 )
    c.text( pm.apply( T0, mid ), r"$\psi = 0$", dx = 0, dy = -14 )
    c.text( pm.apply( Tr, mid ), r"$\psi = %g^\circ$" % rot, dx = 0, dy = 14 )
    # T(psi): from the blade as built to the blade at psi.
    d = fl.unit( hub )
    c.arrow( 1.0 * d, hub - 1.0 * d, color = "#6A3D9A", lw = 1.6, head = 11, ls = ( 0, ( 5, 2 ) ) )
    c.text( 0.5 * hub, r"$T(\psi)$", color = "#6A3D9A", dx = -10, dy = 10, ha = "right" )
    # T(psi) T(0)^-1: from the blade at psi = 0 to the blade at psi, about the shaft.
    shaft = T0[ :3, :3 ] @ X
    t0 = pm.apply( T0, G0[ built.iu( 1.0 ), built.nw // 2 ] )
    rad = t0 - hub - ( ( t0 - hub ) @ shaft ) * shaft
    A = c.arc_pts( hub, -shaft, np.linalg.norm( rad ) + 1.5, 4.0, rot - 4.0, ref = rad )
    c.arrow_path( A, color = "k", lw = 1.5, head = 11 )
    c.text( A[ len( A ) // 2 ], r"$T(\psi)\,T(0)^{-1}$", dx = -14, dy = 0, ha = "right" )
    save( fig, "Propeller_xform", [ [ c ] ] )


FIGS = {
    "axes": ( axes_prop, axes_blade ),
    "construct": ( construct_left, construct_rear ),
    "shaping": ( shaping, ),
    "hinges": ( hinges, ),
    "chain": ( chain, ),
    "azimuth": ( azimuth, ),
    "harmonic": ( harmonic, ),
    "reverse": ( reverse, ),
    "xform": ( xform, ),
}

if __name__ == "__main__":
    names = sys.argv[ 1: ] or list( FIGS )
    bad = [ n for n in names if n not in FIGS ]
    if bad:
        sys.exit( "unknown group %s; the groups are: %s" % ( ", ".join( bad ), " ".join( FIGS ) ) )
    style()
    for n in names:
        for fn in FIGS[ n ]:
            fn()
