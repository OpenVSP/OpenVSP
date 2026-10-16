"""Drawing helpers shared by the body help figures (Fuselage and Stack, Engine Model, Pod, BOR, Cobra, Conformal).

Geometry comes from the openvsp module of the build this runs with; nothing is drawn by hand.  The page layout, text
size and projection are the Propeller figures' (figure_src/propeller/figlib.py): every image is figlib.PAGE_W wide
at 200 dpi with figlib.FS text.

Views keep OpenVSP's frame: SIDE looks along +Y (X right, Z up), TOP looks down -Z (X right, Y up), FRONT looks aft
along +X from ahead of the body (-Y right, Z up), ISO is the 3D view from about (-1, -1, 1) with Z up.
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )

import numpy as np
import matplotlib
matplotlib.use( "Agg" )
import matplotlib.pyplot as plt
from matplotlib.transforms import offset_copy
import matplotlib.patheffects as pe

import openvsp as vsp
import figlib as fl
from figlib import COL, FS

OUT = os.path.join( HERE, "..", "..", "images" )
DPI = 200

X = np.array( [ 1.0, 0, 0 ] )
Y = np.array( [ 0, 1.0, 0 ] )
Z = np.array( [ 0, 0, 1.0 ] )

SIDE = fl.View.plane( X, Z )
TOP = fl.View.plane( X, Y )
FRONT = fl.View.plane( -Y, Z )
ISO = fl.View.plane( [ 1, -1, 0 ], [ 1, 1, 2 ] )

KEY = { "side": ( r"$X$", r"$Z$", r"$-Y$" ), "top": ( r"$X$", r"$Y$", r"$Z$" ), "front": ( r"$-Y$", r"$Z$", r"$-X$" ) }

POS = "#C8CCD2"      # a positive surface, cut
POS_EDGE = "#4a4a4a"
NEG = "#D55E00"      # a negative surface
CONF = "#7FB3D5"     # a Conformal
GHOST = "#9A9A9A"

# The colors the 3D view draws the engine stations in (GeomEngine::BuildMarkerDrawObjs).  Yellow is darkened a
# little so it reads on white.
STATION = { "Inlet Lip": "#00C8C8", "Inlet Face": "#E000E0", "Outlet Face": "#00B000", "Outlet Lip": "#D4B000" }


def style():
    fl.style()


def figure( n = 1 ):
    """A figure and n canvases; save() lays them out."""
    style()
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    return fig


def canvas( fig, view ):
    return fl.Canvas( fig.add_axes( [ 0, 0, 1, 1 ] ), view )


def save( fig, name, rows = None, scale = None, hmax = fl.HMAX ):
    if rows is not None:
        fl.fit( fig, rows, scale = scale, hmax = hmax )
    os.makedirs( OUT, exist_ok = True )
    fig.savefig( os.path.join( OUT, name + ".png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made", name )


def save_plot( fig, name ):
    os.makedirs( OUT, exist_ok = True )
    fig.savefig( os.path.join( OUT, name + ".png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made", name )


# ---------------------------------------------------------------- geometry from VSP

def renew():
    vsp.VSPRenew()
    vsp.ClearVSPModel()


def grid( gid, s = 0, nu = 121, nw = 65, u = None, w = None ):
    """Points of main surface s, ( nu, nw, 3 ), on a uniform u01, w01 grid unless u or w is given."""
    if u is None:
        u = np.linspace( 0.0, 1.0, nu )
    if w is None:
        w = np.linspace( 0.0, 1.0, nw )
    return np.array( [ [ list( vsp.CompPnt01( gid, s, a, b ) ) for b in w ] for a in u ] )


def meridian( gid, s, w, n = 401 ):
    """The u-line at w of main surface s."""
    return np.array( [ list( vsp.CompPnt01( gid, s, a, w ) ) for a in np.linspace( 0.0, 1.0, n ) ] )


def xparm( gid, i, name, group = None ):
    xs = vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), i )
    if group is None:
        return vsp.GetXSecParm( xs, name )
    ids = [ q for q in vsp.GetXSecParmIDs( xs ) if vsp.GetParmName( q ) == name and vsp.GetParmGroupName( q ) == group ]
    assert len( ids ) == 1, ( name, group, len( ids ) )
    return ids[0]


def setx( gid, i, name, val, group = None ):
    vsp.SetParmVal( xparm( gid, i, name, group ), val )


def getx( gid, i, name, group = None ):
    return vsp.GetParmVal( xparm( gid, i, name, group ) )


def nxsec( gid ):
    return vsp.GetNumXSec( vsp.GetXSecSurf( gid, 0 ) )


# ---------------------------------------------------------------- drawing

def fill( c, P, face = POS, edge = POS_EDGE, lw = 0.8, hatch = None, alpha = None, z = 10, ls = "-" ):
    """A closed outline P ( n, 3 ) in model axes, filled."""
    Q = c.v.p2( np.asarray( P, float ) )
    c.pts.append( Q )
    c.ax.fill( Q[:, 0], Q[:, 1], facecolor = face, edgecolor = edge, lw = lw, hatch = hatch, alpha = alpha, zorder = z,
               linestyle = ls )


def label( c, p, s, dx = 0.0, dy = 0.0, color = "k", ha = "center", va = "center", z = 90 ):
    return c.text( p, s, color = color, dx = dx, dy = dy, ha = ha, va = va, z = z )


def title( c, s, pad = 8.0 ):
    """A title above the canvas content, centered."""
    lo, hi = c.limits()
    fig = c.ax.figure
    t = offset_copy( c.ax.transData, fig, x = 0, y = pad, units = "points" )
    c.ax.text( 0.5 * ( lo[0] + hi[0] ), hi[1], s, transform = t, ha = "center", va = "bottom", fontsize = FS )


def axis_line( c, x0, x1, z = 5 ):
    """The body axis, dash-dot."""
    c.line( np.array( [ x0 * X, x1 * X ] ), color = "#888888", lw = 0.7, ls = ( 0, ( 8, 2, 1, 2 ) ), z = z )


def dim( c, p0, p1, s, off = ( 0.0, 0.0 ), color = "k", dx = 0.0, dy = 0.0, ext = True, lw = 0.7 ):
    """A dimension: a double arrow from p0 to p1 offset by off (model units, in the view's right/up), with label s at
    its middle.  Extension lines are drawn from the points to the arrow."""
    p0 = np.asarray( p0, float )
    p1 = np.asarray( p1, float )
    o = off[0] * c.v.right + off[1] * c.v.up
    a0 = p0 + o
    a1 = p1 + o
    if ext and np.linalg.norm( o ) > 0:
        for p, a in ( ( p0, a0 ), ( p1, a1 ) ):
            c.line( np.array( [ p, a + 0.15 * o / np.linalg.norm( o ) ] ), color = "#777777", lw = 0.5, z = 40 )
    c.arrow( a0, a1, color = color, lw = lw, head = 6.0, both = True, z = 45 )
    if s:
        label( c, 0.5 * ( a0 + a1 ), s, dx = dx, dy = dy, color = color )


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


def triad( c, o, length = 1.0 ):
    """The model's X, Y, Z at o."""
    c.triad( np.asarray( o, float ), [ X, Y, Z ], [ r"$X$", r"$Y$", r"$Z$" ], "k", length = length, lw = 1.1 )


def cut_halves( gid, s = 0, n = 401, wtop = 0.75, wbot = 0.25 ):
    """The surface cut by the plane Y = 0: its top and bottom meridians."""
    return meridian( gid, s, wtop, n ), meridian( gid, s, wbot, n )


def body_cut( gid, s = 0, n = 401, wtop = 0.75, wbot = 0.25 ):
    """A closed body cut through its axis: one outline, top then bottom reversed."""
    top, bot = cut_halves( gid, s, n, wtop, wbot )
    return np.vstack( [ top, bot[ ::-1 ] ] )


def cut_outlines( gid, s = 0, n = 401, wtop = 0.75, wbot = 0.25, tol = 1e-6 ):
    """The surface cut by Y = 0, as closed outlines: one for a body closed on its axis, two (above and below the
    axis) for a ring such as a nacelle wall."""
    top, bot = cut_halves( gid, s, n, wtop, wbot )
    if abs( top[0, 2] ) < tol and abs( top[-1, 2] ) < tol:
        return [ np.vstack( [ top, bot[ ::-1 ] ] ) ]
    return [ top, bot ]
