"""Figures for src/help/XSecSkinning.md.

    <build>/venv/bin/python xsecskinning_figs.py [names...]      # writes ../../images/XSecSkinning_*.png

Every surface is VSP's own: a Stack of circles two units across and two apart, sampled with CompPnt01.  Profiles are
the Top line seen from the side, looking along +Y (X right, Z up); cuts across the body are seen from behind, looking
forward (Y right, Z up).  Shared helpers are in ../xseccurves/xsfig.py.
"""

import os
import sys

import numpy as np
import openvsp as vsp

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "xseccurves" ) )

import xsfig as xf                                   # noqa: E402
from xsfig import TRIO, INK, GHOST, MARK             # noqa: E402

SIDES = ( "Top", "Right", "Bottom", "Left" )
QTYS = ( "Angle", "Slew", "Strength", "Curve" )

# The colors the 3D view draws the stations in (GeomXSec::SkinDrawColor), darkened to read on white.
SIDE_COL = { "Top": "#0097A7", "Right": "#B8A000", "Bottom": "#B000B0", "Left": "#2E9E2E", "Spine": "#1E40FF" }


def clear( xs ):
    """Nothing enforced, nothing equal, no symmetry: a cross section that leaves the loft free."""
    for s in SIDES:
        for lr in "LR":
            for q in QTYS:
                xf.sp( xs, s + lr + q + "Set", 0 )
        for q in QTYS:
            xf.sp( xs, s + "LR" + q + "Eq", 0 )
    for f in ( "AllSym", "TBSym", "RLSym" ):
        xf.sp( xs, f, 0 )


def stack( n = 3, d = 2.0, dx = 2.0, shape = vsp.XS_CIRCLE, size = None ):
    """A Stack of n like cross sections dx apart, every one left free."""
    vsp.VSPRenew()
    g = vsp.AddGeom( "STACK" )
    s = vsp.GetXSecSurf( g, 0 )
    while vsp.GetNumXSec( s ) > n:
        vsp.CutXSec( g, vsp.GetNumXSec( s ) - 1 )
    vsp.Update()
    for i in range( n ):
        vsp.ChangeXSecShape( s, i, shape )
    vsp.Update()
    for i in range( n ):
        xs = vsp.GetXSec( s, i )
        if shape == vsp.XS_CIRCLE:
            xf.sp( xs, "Circle_Diameter", d )
        elif size is not None:
            vsp.SetXSecWidthHeight( xs, size[0], size[1] )
        xf.sp( xs, "XDelta", dx if i > 0 else 0.0 )
        clear( xs )
    vsp.Update()
    return g, s


def side( xs, s, l = None, r = None ):
    """Set a side's left (l) and right (r) quantities, each a dict, and enforce what is given."""
    for lr, vals in ( ( "L", l ), ( "R", r ) ):
        if vals is None:
            continue
        for q, v in vals.items():
            xf.sp( xs, s + lr + q, v )
            if q in ( "Angle", "Curve" ):
                xf.sp( xs, s + lr + q + "Set", 1 )


def pnt( g, u, w ):
    p = vsp.CompPnt01( g, 0, u, w )
    return np.array( [ p.x(), p.y(), p.z() ] )


def profile( g, w = 0.75, n = 241 ):
    """The line at w along the body, as (X, Z)."""
    return np.array( [ pnt( g, u, w )[ [ 0, 2 ] ] for u in np.linspace( 0.0, 1.0, n ) ] )


def cut( g, u, n = 361 ):
    """The body round at u, as (Y, Z)."""
    return np.array( [ pnt( g, u, w )[ 1: ] for w in np.linspace( 0.0, 1.0, n ) ] )


def frame_profile( c ):
    for x in ( 0.0, 2.0, 4.0 ):
        c.line( xf.to3( [ [ x, -0.05 ], [ x, 1.65 ] ] ), color = "#BBBBBB", lw = 0.8, z = 5 )
    c.line( xf.to3( [ [ -0.2, 0 ], [ 4.2, 0 ] ] ), color = GHOST, lw = 0.7, ls = ( 0, ( 6, 2, 1, 2 ) ), z = 5 )
    c.bounds = ( np.array( [ -0.2, -0.05 ] ), np.array( [ 4.2, 1.65 ] ) )


def tangent( c, g, u, w, color, scale = 0.35 ):
    """The direction the surface leaves the section at u, drawn from it."""
    h = 1e-4
    p = pnt( g, u, w )[ [ 0, 2 ] ]
    d = pnt( g, u + h, w )[ [ 0, 2 ] ] - p
    d = d / np.linalg.norm( d )
    c.arrow( xf.pt3( p ), xf.pt3( p + scale * d ), color = color, lw = 1.1, head = 6.0 )


def family( name, base, vals, label, what ):
    """Top profiles of the middle section enforcing what( v ) for each v."""
    fig = xf.new()
    c = xf.canvas( fig )
    for k, v in enumerate( vals ):
        g, s = stack()
        xs = vsp.GetXSec( s, 1 )
        what( xs, v )
        vsp.Update()
        P = profile( g )
        xf.curve( c, P, color = TRIO[k], lw = 1.3 )
        tangent( c, g, 0.5, 0.75, TRIO[k] )
        xf.label( c, [ 4.25, 1.45 - 0.13 * k ], label % v, color = TRIO[k], ha = "left" )
    frame_profile( c )
    for i, x in enumerate( ( 0.0, 2.0, 4.0 ) ):
        xf.label( c, [ x, -0.05 ], "XSec %d" % i, dy = -8, color = "#777777" )
    xf.key_side( c, at = ( 0.0, 0.0 ), dx = -40, dy = 10 )
    xf.save( fig, name, [ [ c ] ], hmax = 2.8 )


def angle():
    def what( xs, v ):
        side( xs, "Top", { "Angle": v, "Strength": 1.0 }, { "Angle": v, "Strength": 1.0 } )
    family( "XSecSkinning_angle", None, ( -20.0, 0.0, 20.0 ), "Angle %g", what )


def strength():
    def what( xs, v ):
        side( xs, "Top", { "Angle": 20.0, "Strength": v }, { "Angle": 20.0, "Strength": v } )
    family( "XSecSkinning_strength", None, ( 0.5, 1.0, 2.0 ), "Strength %g", what )


def curvature():
    def what( xs, v ):
        side( xs, "Top", { "Angle": 0.0, "Strength": 1.0, "Curve": v }, { "Angle": 0.0, "Strength": 1.0, "Curve": v } )
    family( "XSecSkinning_curvature", None, ( -1.0, 0.0, 1.0 ), "Curvature %g", what )


ORDER_CASES = ( ( "Nothing enforced: a straight line", None, None ),
                ( "Angle 0 leaving XSec 1: a parabola", { "Angle": 0.0, "Strength": 1.0 }, None ),
                ( "Angle 0 at both ends: a cubic", { "Angle": 0.0, "Strength": 1.0 },
                  { "Angle": 0.0, "Strength": 1.0 } ) )


def order_stack( r1, l2 ):
    """The last section 3 across; r1 enforced leaving XSec 1, l2 arriving at XSec 2."""
    g, s = stack()
    xf.sp( vsp.GetXSec( s, 2 ), "Circle_Diameter", 3.0 )
    side( vsp.GetXSec( s, 1 ), "Top", None, r1 )
    side( vsp.GetXSec( s, 2 ), "Top", l2, None )
    vsp.Update()
    return g, s


def order():
    """Each condition at an end of a span raises the degree of the line across it."""
    fig = xf.new()
    c = xf.canvas( fig )
    for k, ( name, r1, l2 ) in enumerate( ORDER_CASES ):
        g, s = order_stack( r1, l2 )
        xf.curve( c, profile( g ), color = TRIO[k], lw = 1.3 )
        if r1 is not None:
            tangent( c, g, 0.5, 0.75, TRIO[k] )
        if l2 is not None:
            h = 1e-4
            p = pnt( g, 1.0, 0.75 )[ [ 0, 2 ] ]
            d = p - pnt( g, 1.0 - h, 0.75 )[ [ 0, 2 ] ]
            d = d / np.linalg.norm( d )
            c.arrow( xf.pt3( p - 0.35 * d ), xf.pt3( p ), color = TRIO[k], lw = 1.1, head = 6.0 )
        xf.label( c, [ 4.25, 1.45 - 0.13 * k ], name, color = TRIO[k], ha = "left" )
    frame_profile( c )
    for i, x in enumerate( ( 0.0, 2.0, 4.0 ) ):
        xf.label( c, [ x, -0.05 ], "XSec %d" % i, dy = -8, color = "#777777" )
    xf.key_side( c, at = ( 0.0, 0.0 ), dx = -40, dy = 10 )
    xf.save( fig, "XSecSkinning_order", [ [ c ] ], hmax = 2.8 )


def leftright():
    """Left and right taking different angles, as continuity C0 allows."""
    def what( xs, v ):
        side( xs, "Top", { "Angle": v, "Strength": 1.0 }, { "Angle": -v, "Strength": 1.0 } )
    fig = xf.new()
    c = xf.canvas( fig )
    g, s = stack()
    xs = vsp.GetXSec( s, 1 )
    what( xs, 20.0 )
    vsp.Update()
    P = profile( g )
    xf.curve( c, P, color = INK, lw = 1.4 )
    h = 1e-4
    p = pnt( g, 0.5, 0.75 )[ [ 0, 2 ] ]
    dl = p - pnt( g, 0.5 - h, 0.75 )[ [ 0, 2 ] ]
    dr = pnt( g, 0.5 + h, 0.75 )[ [ 0, 2 ] ] - p
    dl = dl / np.linalg.norm( dl )
    dr = dr / np.linalg.norm( dr )
    # The left side's direction arrives at the section; the right side's leaves it.
    c.arrow( xf.pt3( p - 0.6 * dl - np.array( [ 0, 0.12 ] ) ), xf.pt3( p - np.array( [ 0, 0.12 ] ) ), color = TRIO[0],
             lw = 1.2, head = 6.0 )
    c.arrow( xf.pt3( p + np.array( [ 0, 0.12 ] ) ), xf.pt3( p + 0.6 * dr + np.array( [ 0, 0.12 ] ) ), color = TRIO[2],
             lw = 1.2, head = 6.0 )
    xf.label( c, p - 0.6 * dl - np.array( [ 0, 0.12 ] ), "Left: Angle 20", color = TRIO[0], dx = -6, ha = "right" )
    xf.label( c, p + 0.6 * dr + np.array( [ 0, 0.12 ] ), "Right: Angle -20", color = TRIO[2], dx = 6, ha = "left" )
    frame_profile( c )
    for i, x in enumerate( ( 0.0, 2.0, 4.0 ) ):
        xf.label( c, [ x, -0.05 ], "XSec %d" % i, dy = -8, color = "#777777" )
    xf.key_side( c, at = ( 0.0, 0.0 ), dx = -40, dy = 10 )
    xf.save( fig, "XSecSkinning_leftright", [ [ c ] ], hmax = 2.8 )


def continuity():
    """Nothing enforced; the middle section's continuity C0, C1, C2, with the last section larger."""
    fig = xf.new()
    c = xf.canvas( fig )
    for k, cont in enumerate( ( 0, 1, 2 ) ):
        g, s = stack()
        x2 = vsp.GetXSec( s, 2 )
        xf.sp( x2, "Circle_Diameter", 3.0 )
        xs = vsp.GetXSec( s, 1 )
        xf.sp( xs, "ContinuityTop", cont )
        vsp.Update()
        xf.curve( c, profile( g ), color = TRIO[k], lw = 1.3 )
        xf.label( c, [ 4.25, 1.45 - 0.13 * k ], "C%d" % cont, color = TRIO[k], ha = "left" )
    frame_profile( c )
    for i, x in enumerate( ( 0.0, 2.0, 4.0 ) ):
        xf.label( c, [ x, -0.05 ], "XSec %d" % i, dy = -8, color = "#777777" )
    xf.key_side( c, at = ( 0.0, 0.0 ), dx = -40, dy = 10 )
    xf.save( fig, "XSecSkinning_continuity", [ [ c ] ], hmax = 2.8 )


def stations():
    """The four sides and one spine with both its mirrors, round a section."""
    P, f, xs = xf.body_section( vsp.XS_ELLIPSE, w = 3.0, h = 2.0 )
    fig = xf.new()
    c = xf.canvas( fig )
    xf.curve( c, P, lw = 1.4 )
    for t in ( 0.04, 0.29, 0.54, 0.79 ):
        i = int( np.argmin( np.abs( f - t ) ) )
        xf.arrow_along( c, P, i, n = 4, color = GHOST )
    for name, t, dx, dy, ha in ( ( "Right, 0", 0.0, 10, 0, "left" ), ( "Bottom, 0.25", 0.25, 0, -11, "center" ),
                                 ( "Left, 0.5", 0.5, -10, 0, "right" ), ( "Top, 0.75", 0.75, 0, 11, "center" ) ):
        p = xf.at( P, f, t )
        xf.dot( c, p, color = SIDE_COL[ name.split( "," )[0] ], ms = 7.0 )
        xf.label( c, p, name, dx = dx, dy = dy, ha = ha, color = SIDE_COL[ name.split( "," )[0] ] )
    w = 0.1
    for name, t, filled, dx, dy, ha in ( ( "spine, W 0.1", w, True, 9, -6, "left" ),
                                         ( "L/R mirror, 0.4", 0.5 - w, False, -9, -6, "right" ),
                                         ( "T/B mirror, 0.9", 1.0 - w, False, 9, 6, "left" ),
                                         ( "both, 0.6", 0.5 + w, False, -9, 6, "right" ) ):
        p = xf.at( P, f, t )
        q = c.v.p2( xf.pt3( p ) )
        if filled:
            c.ax.plot( [ q[0] ], [ q[1] ], marker = "o", ms = 7.0, color = SIDE_COL[ "Spine" ], zorder = 70, mec = "white",
                       mew = 0.6 )
        else:
            c.ax.plot( [ q[0] ], [ q[1] ], marker = "o", ms = 6.0, mfc = "white", mec = SIDE_COL[ "Spine" ], mew = 1.3,
                       zorder = 70 )
        xf.label( c, p, name, dx = dx, dy = dy, ha = ha, color = SIDE_COL[ "Spine" ] )
    xf.key_rear( c, at = ( 0.0, 0.0 ), dx = -40, dy = -10 )
    c.bounds = ( np.array( [ -1.5, -1.0 ] ), np.array( [ 1.5, 1.0 ] ) )
    xf.save( fig, "XSecSkinning_stations", [ [ c ] ], hmax = 2.8 )


def spine():
    """A spine's reach: the body halfway between the first two sections, with and without a spine."""
    fig = xf.new()
    c = xf.canvas( fig )
    g, s = stack()
    xs = vsp.GetXSec( s, 1 )
    for sd in SIDES:
        side( xs, sd, { "Angle": 0.0, "Strength": 1.0 }, { "Angle": 0.0, "Strength": 1.0 } )
    vsp.Update()
    ref = cut( g, 0.25 )
    vsp.AddSkinSpine( g, 0.1 )
    vsp.Update()
    xs = vsp.GetXSec( s, 1 )
    sid = vsp.GetSkinSpineID( xs, 0 )
    for lr in "LR":
        vsp.SetParmVal( vsp.GetSkinSpineParm( sid, lr + "AngleSet" ), 1.0 )
        vsp.SetParmVal( vsp.GetSkinSpineParm( sid, lr + "Angle" ), -25.0 )
        vsp.SetParmVal( vsp.GetSkinSpineParm( sid, lr + "Strength" ), 1.0 )
    vsp.Update()
    P = cut( g, 0.25 )
    xf.curve( c, ref, color = GHOST, lw = 1.0, ls = ( 0, ( 4, 2 ) ), z = 40 )
    xf.curve( c, P )
    r = 1.0
    for name, t, dx, dy, ha in ( ( "Right", 0.0, 9, 0, "left" ), ( "Bottom", 0.25, 0, -10, "center" ),
                                 ( "Left", 0.5, -9, 0, "right" ), ( "Top", 0.75, 0, 10, "center" ) ):
        a = -2 * np.pi * t
        xf.dot( c, [ r * np.cos( a ), r * np.sin( a ) ], color = SIDE_COL[ name ], ms = 6.0 )
        xf.label( c, [ r * np.cos( a ), r * np.sin( a ) ], name, dx = dx, dy = dy, ha = ha, color = SIDE_COL[ name ] )
    a = -2 * np.pi * 0.1
    xf.dot( c, [ r * np.cos( a ), r * np.sin( a ) ], color = SIDE_COL[ "Spine" ], ms = 7.0 )
    xf.label( c, [ r * np.cos( a ), r * np.sin( a ) ], "spine, W 0.1", dx = -8, dy = 6, ha = "right",
              color = SIDE_COL[ "Spine" ] )
    xf.key_rear( c, at = ( 0.0, 0.0 ), dx = -40, dy = -10 )
    c.bounds = ( np.array( [ -1.1, -1.1 ] ), np.array( [ 1.1, 1.1 ] ) )
    xf.save( fig, "XSecSkinning_spine", [ [ c ] ], hmax = 2.8 )


BASIS_W, BASIS_H = 3.0, 1.0
BASIS_COL = { 0.0: "#D55E00", 1.0: SIDE_COL[ "Spine" ] }


def basis_arrows( flag ):
    """A rounded rectangle three times as wide as it is high, with Angle 30 enforced at its sides and at twelve spines,
    with the angle basis from a circle (flag 0) or from the curve (flag 1).  Returns the section cut, the stations and
    the part of the surface's direction across the body at each."""
    ws = [ k / 16.0 for k in range( 16 ) ]
    g, s = stack( shape = vsp.XS_ROUNDED_RECTANGLE, size = ( BASIS_W, BASIS_H ) )
    xs = vsp.GetXSec( s, 1 )
    xf.sp( xs, "RoundRectXSec_Radius", 0.2 )
    xf.sp( xs, "CurveBasis", flag )
    for sd in SIDES:
        side( xs, sd, { "Angle": 30.0, "Strength": 1.0 }, { "Angle": 30.0, "Strength": 1.0 } )
    vsp.Update()
    for w in ws:
        if abs( w * 4 - round( w * 4 ) ) > 1e-9:
            vsp.AddSkinSpine( g, w )
    vsp.Update()
    xs = vsp.GetXSec( s, 1 )
    for k in range( vsp.GetNumSkinSpines( g ) ):
        sid = vsp.GetSkinSpineID( xs, k )
        for lr in "LR":
            vsp.SetParmVal( vsp.GetSkinSpineParm( sid, lr + "AngleSet" ), 1.0 )
            vsp.SetParmVal( vsp.GetSkinSpineParm( sid, lr + "Angle" ), 30.0 )
            vsp.SetParmVal( vsp.GetSkinSpineParm( sid, lr + "Strength" ), 1.0 )
    vsp.Update()
    sec = cut( g, 0.5 )
    h = 1e-4
    pts, dirs = [], []
    for w in ws:
        p = pnt( g, 0.5, w )
        d = ( pnt( g, 0.5 + h, w ) - p ) / h
        d = d / np.linalg.norm( d )
        pts.append( p[ 1: ] )
        dirs.append( d[ 1: ] )
    return sec, pts, dirs


def basis_panel( c, sec, arrows ):
    xf.curve( c, sec )
    for pts, dirs, color in arrows:
        for p, q in zip( pts, dirs ):
            c.arrow( xf.pt3( p ), xf.pt3( p + 0.9 * q ), color = color, lw = 1.0, head = 5.0 )
            xf.dot( c, p, color = INK, ms = 3.5 )
    c.bounds = ( np.array( [ -2.4, -1.4 ] ), np.array( [ 2.4, 1.4 ] ) )


def basis():
    """Where Angle tips the surface with each frame, apart and laid over each other."""
    fig = xf.new()
    got = { flag: basis_arrows( flag ) for flag in ( 0.0, 1.0 ) }
    top = []
    for flag, name in ( ( 0.0, "Angle basis from a circle" ), ( 1.0, "Angle Basis From Curve" ) ):
        sec, pts, dirs = got[ flag ]
        c = xf.canvas( fig )
        basis_panel( c, sec, [ ( pts, dirs, BASIS_COL[ flag ] ) ] )
        xf.title( c, name )
        top.append( c )
    c = xf.canvas( fig )
    sec = got[ 0.0 ][ 0 ]
    basis_panel( c, sec, [ ( got[ f ][ 1 ], got[ f ][ 2 ], BASIS_COL[ f ] ) for f in ( 0.0, 1.0 ) ] )
    xf.title( c, "Both, laid over each other" )
    xf.label( c, np.array( [ -2.35, -1.3 ] ), "from a circle", color = BASIS_COL[ 0.0 ], ha = "left" )
    xf.label( c, np.array( [ 2.35, -1.3 ] ), "from the curve", color = BASIS_COL[ 1.0 ], ha = "right" )
    xf.key_rear( top[ 0 ], at = ( 0.0, 0.0 ), dx = -10, dy = -16 )
    xf.save( fig, "XSecSkinning_basis", [ top, [ c ] ] )


FIGS = { "order": order, "stations": stations, "angle": angle, "strength": strength, "curvature": curvature, "leftright": leftright,
         "continuity": continuity, "spine": spine, "basis": basis }

if __name__ == "__main__":
    xf.style()
    names = sys.argv[1:] or list( FIGS )
    for n in names:
        FIGS[ n ]()
