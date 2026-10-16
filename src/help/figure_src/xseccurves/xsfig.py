"""Shared pieces of the cross section help figures (XSecCurves.md, Airfoils.md, XSecSkinning.md).

Every curve drawn comes from the openvsp module.  A body section (Stack or Fuselage) is sampled with ComputeXSecPnt
on a Stack cross section that is neither moved nor turned, so its points come back in the Stack's axes: Y across the
section's width, Z up.  That is the view from behind, looking forward (Y right, Z up, X toward the reader).  An
airfoil is sampled the same way on the root section of a Wing, whose axes put the chord along X and the thickness
along Z: the side view, looking along +Y (X right, Z up).

The drawing itself uses figlib from ../propeller: one page width, one text size.
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

import figlib as fl                       # noqa: E402
from figlib import COL, FS                # noqa: E402,F401

import openvsp as vsp                     # noqa: E402

OUT = os.path.join( HERE, "..", "..", "images" )
DPI = 200

# Three values of one parameter, low to high.
TRIO = ( "#003F5C", "#7A5195", "#EF5675" )
INK = "#222222"
GHOST = "#9A9A9A"
MARK = "#D55E00"

XY = fl.View.plane( [ 1.0, 0, 0 ], [ 0, 1.0, 0 ] )


def style():
    fl.style()


def to3( P ):
    """2D points as 3D points in the z = 0 plane, which XY views face on."""
    P = np.asarray( P, float )
    return np.column_stack( [ P[:, 0], P[:, 1], np.zeros( len( P ) ) ] )


def pt3( p ):
    return np.array( [ p[0], p[1], 0.0 ] )


def canvas( fig ):
    return fl.Canvas( fig.add_axes( [ 0, 0, 1, 1 ] ), XY )


def new():
    return plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )


def save( fig, name, rows, scale = None, hmax = fl.HMAX ):
    fl.fit( fig, rows, scale = scale, hmax = hmax )
    os.makedirs( OUT, exist_ok = True )
    fig.savefig( os.path.join( OUT, name + ".png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made", name )


def key( c, right, up, out, at = ( 1.0, 0.0 ), dx = 0.0, dy = 0.0 ):
    """A small 2D axis key, dx, dy points from the point at fractions 'at' of the canvas's content box.  The circled
    dot names the axis toward the reader.  (After propeller_figs.key.)"""
    lo, hi = c.limits()
    q = lo + np.asarray( at, float ) * ( hi - lo )
    fig = c.ax.figure

    def T( x, y ):
        return offset_copy( c.ax.transData, fig, x = dx + x, y = dy + y, units = "points" )
    L = 22.0
    for ( x, y ), lab, ha, va, tx, ty in ( ( ( L, 0 ), right, "left", "center", L + 3, 0 ),
                                           ( ( 0, L ), up, "center", "bottom", 0, L + 2 ) ):
        c.ax.annotate( "", xy = q, xycoords = T( x, y ), xytext = q, textcoords = T( 0, 0 ), annotation_clip = False,
                       arrowprops = dict( arrowstyle = "-|>", lw = 0.8, color = "k", mutation_scale = 6, shrinkA = 0,
                                          shrinkB = 0 ) )
        c.ax.text( q[0], q[1], lab, transform = T( tx, ty ), ha = ha, va = va )
    c.ax.plot( [ q[0] ], [ q[1] ], transform = T( 0, 0 ), marker = "o", ms = 5.5, mfc = "white", mec = "k", mew = 0.7,
               zorder = 90 )
    c.ax.plot( [ q[0] ], [ q[1] ], transform = T( 0, 0 ), marker = "o", ms = 1.4, mfc = "k", mec = "k", zorder = 91 )
    c.ax.text( q[0], q[1], out, transform = T( -5, -5 ), ha = "right", va = "top" )


def key_rear( c, **kw ):
    key( c, r"$Y$", r"$Z$", r"$X$", **kw )


def key_side( c, **kw ):
    key( c, r"$X$", r"$Z$", r"$-Y$", **kw )


def title( c, s, pad = 8.0 ):
    lo, hi = c.limits()
    c.ax.annotate( s, xy = ( 0.5 * ( lo[0] + hi[0] ), hi[1] ), xytext = ( 0, pad ), textcoords = "offset points",
                   ha = "center", va = "bottom", annotation_clip = False )


def dot( c, p, color = INK, ms = 5.0, z = 70 ):
    q = c.v.p2( pt3( p ) )
    c.ax.plot( [ q[0] ], [ q[1] ], marker = "o", ms = ms, color = color, zorder = z, mec = "white", mew = 0.6 )


def curve( c, P, color = INK, lw = 1.4, ls = "-", z = 50 ):
    c.line( to3( P ), color = color, lw = lw, ls = ls, z = z )


def arrow_along( c, P, i, color = INK, lw = 1.2, head = 7.0, n = 3 ):
    """An arrowhead on polyline P at index i, pointing the way P runs."""
    c.arrow_path( to3( P[ i:i + n ] ), color = color, lw = lw, head = head, z = 60 )


def label( c, p, s, dx = 0.0, dy = 0.0, color = "k", ha = "center", va = "center" ):
    return c.text( pt3( p ), s, color = color, dx = dx, dy = dy, ha = ha, va = va )


def dim( c, p0, p1, s, off = ( 0.0, 0.0 ), dx = 0.0, dy = 0.0, color = "#555555" ):
    """A two-headed dimension line from p0 to p1, moved by off (model units), labelled s."""
    off = np.asarray( off, float )
    a = np.asarray( p0, float ) + off
    b = np.asarray( p1, float ) + off
    c.arrow_path( to3( np.array( [ a, b ] ) ), color = color, lw = 0.8, head = 6.0, both = True )
    c.text( pt3( 0.5 * ( a + b ) ), s, color = color, dx = dx, dy = dy )


# ---------------------------------------------------------------- geometry from VSP

def sp( xs, name, val ):
    pid = vsp.GetXSecParm( xs, name )
    assert pid, name
    vsp.SetParmVal( pid, val )


def gp( xs, name ):
    return vsp.GetParmVal( vsp.GetXSecParm( xs, name ) )


def sp_group( xs, name, group, val ):
    """A Parm found by its group as well, for the names two groups share."""
    cid = vsp.GetParmContainer( vsp.GetXSecParm( xs, "Theta" ) )
    pid = vsp.GetParm( cid, name, group )
    assert pid, name
    vsp.SetParmVal( pid, val )


def body_section( shape, parms = None, w = None, h = None, n = 721 ):
    """A body cross section of the given shape, sampled all the way round.  Returns the points (Y, Z) and the
    fractions they were taken at; the curve parameter runs from 0 to 1 round the section."""
    vsp.VSPRenew()
    g = vsp.AddGeom( "STACK" )
    s = vsp.GetXSecSurf( g, 0 )
    vsp.ChangeXSecShape( s, 1, shape )
    xs = vsp.GetXSec( s, 1 )
    if w is not None:
        vsp.SetXSecWidthHeight( xs, w, h )
    for k, v in ( parms or {} ).items():
        sp( xs, k, v )
    vsp.Update()
    xs = vsp.GetXSec( s, 1 )
    f = np.linspace( 0.0, 1.0, n )
    P = np.array( [ [ q.y(), q.z() ] for q in ( vsp.ComputeXSecPnt( xs, t ) for t in f ) ] )
    return P, f, xs


def wing_section( shape = None, parms = None, groups = None, n = 1201, prep = None ):
    """The root airfoil of a Wing, sampled all the way round, as (X/c, Z/c) with the leading edge at the origin.  The
    curve starts at the trailing edge on the lower surface, passes the leading edge at 0.5 and ends on the upper
    surface.  prep( xs ) runs first; then parms are set by name, and groups maps a name to (group, value) for the names
    two groups share."""
    vsp.VSPRenew()
    g = vsp.AddGeom( "WING" )
    s = vsp.GetXSecSurf( g, 0 )
    if shape is not None:
        vsp.ChangeXSecShape( s, 0, shape )
    xs = vsp.GetXSec( s, 0 )
    if prep is not None:
        prep( xs )
    for k, v in ( parms or {} ).items():
        sp( xs, k, v )
    for k, ( grp, v ) in ( groups or {} ).items():
        sp_group( xs, k, grp, v )
    vsp.Update()
    xs = vsp.GetXSec( s, 0 )
    f = fractions( n )
    c = vsp.GetXSecWidth( xs )
    P = np.array( [ [ q.x(), q.z() ] for q in ( vsp.ComputeXSecPnt( xs, t ) for t in f ) ] ) / c
    return P, f, xs


def fractions( n ):
    """n fractions round a section, plus dense runs at its ends and middle: a trailing edge face, its cap and a
    leading edge face occupy a thousandth of the curve parameter each."""
    f = np.concatenate( [ np.linspace( 0.0, 1.0, n ), np.linspace( 0.0, 0.004, 161 ), np.linspace( 0.996, 1.0, 161 ),
                          np.linspace( 0.496, 0.504, 161 ) ] )
    return np.unique( f )


def at( P, f, t ):
    """The sampled point nearest the fraction t."""
    return P[ int( np.argmin( np.abs( f - t ) ) ) ]
