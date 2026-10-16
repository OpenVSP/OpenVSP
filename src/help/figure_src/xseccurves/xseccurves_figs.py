"""Figures for src/help/XSecCurves.md.

    <build>/venv/bin/python xseccurves_figs.py [names...]      # writes ../../images/XSecCurves_*.png

Every section is VSP's own, sampled with ComputeXSecPnt on an unmoved Stack section and drawn from behind, looking
forward (Y right, Z up).
"""

import sys

import numpy as np
import openvsp as vsp

import xsfig as xf
from xsfig import TRIO, INK, GHOST, MARK


def param():
    """Where the curve parameter starts and which way it runs."""
    P, f, xs = xf.body_section( vsp.XS_ELLIPSE, w = 3.0, h = 2.0 )
    fig = xf.new()
    c = xf.canvas( fig )
    xf.curve( c, P, lw = 1.6 )
    for t in ( 0.08, 0.33, 0.58, 0.83 ):
        i = int( np.argmin( np.abs( f - t ) ) )
        xf.arrow_along( c, P, i, n = 4 )
    names = ( ( 0.0, "Right, 0", 10, 0, "left" ), ( 0.25, "Bottom, 0.25", 0, -11, "center" ),
              ( 0.5, "Left, 0.5", -10, 0, "right" ), ( 0.75, "Top, 0.75", 0, 11, "center" ) )
    for t, s, dx, dy, ha in names:
        p = xf.at( P, f, t )
        xf.dot( c, p, color = MARK )
        xf.label( c, p, s, dx = dx, dy = dy, ha = ha )
    xf.dim( c, [ -1.5, 0 ], [ 1.5, 0 ], "Width", off = ( 0, -1.35 ), dy = -8 )
    xf.dim( c, [ 0, -1.0 ], [ 0, 1.0 ], "Height", off = ( 2.6, 0 ), dx = 22 )
    xf.key_rear( c, at = ( 0.0, 0.0 ), dx = -30, dy = -10 )
    xf.save( fig, "XSecCurves_param", [ [ c ] ], hmax = 2.6 )


def super_exp():
    """Super ellipse exponents, equal."""
    fig = xf.new()
    row = []
    for m in ( 1.0, 1.5, 2.0, 4.0, 10.0 ):
        P, f, xs = xf.body_section( vsp.XS_SUPER_ELLIPSE, { "Super_M": m, "Super_N": m }, w = 1.0, h = 1.0 )
        c = xf.canvas( fig )
        xf.curve( c, P )
        c.bounds = ( np.array( [ -0.55, -0.55 ] ), np.array( [ 0.55, 0.55 ] ) )
        xf.title( c, "M = N = %g" % m )
        row.append( c )
    xf.key_rear( row[0], at = ( 0.0, 0.0 ), dx = -6, dy = -16 )
    xf.save( fig, "XSecCurves_super_exp", [ row ] )


def super_more():
    """Unequal exponents, the widest point moved, and a bottom of its own."""
    cases = ( ( "M = 4, N = 2", { "Super_M": 4.0, "Super_N": 2.0 } ),
              ( "M = 2, N = 4", { "Super_M": 2.0, "Super_N": 4.0 } ),
              ( "MaxWLoc = 0.5", { "Super_MaxWidthLoc": 0.5 } ),
              ( "M, N 2; Bottom 6", { "Super_TopBotSym": 0.0, "Super_M_bot": 6.0, "Super_N_bot": 6.0 } ) )
    fig = xf.new()
    row = []
    for name, parms in cases:
        P, f, xs = xf.body_section( vsp.XS_SUPER_ELLIPSE, parms, w = 1.0, h = 1.0 )
        c = xf.canvas( fig )
        xf.curve( c, P )
        if "Super_MaxWidthLoc" in parms:
            yw = parms[ "Super_MaxWidthLoc" ] * 0.5
            c.line( xf.to3( [ [ -0.6, yw ], [ 0.6, yw ] ] ), color = MARK, lw = 0.9, ls = ( 0, ( 4, 2 ) ) )
            c.line( xf.to3( [ [ -0.6, 0 ], [ 0.6, 0 ] ] ), color = GHOST, lw = 0.7, ls = ( 0, ( 1, 2 ) ) )
        c.bounds = ( np.array( [ -0.6, -0.55 ] ), np.array( [ 0.6, 0.55 ] ) )
        xf.title( c, name )
        row.append( c )
    xf.key_rear( row[0], at = ( 0.0, 0.0 ), dx = -6, dy = -16 )
    xf.save( fig, "XSecCurves_super_more", [ row ] )


def rrect():
    """Rounded rectangle: radii, keystone, skew and vertical skew."""
    base = { "RoundRectXSec_Radius": 0.2 }
    cases = ( ( "Radius 0.2", {} ),
              ( "Keystone 0.7", { "RoundRect_Keystone": 0.7 } ),
              ( "Skew 0.4", { "RoundRect_Skew": 0.4 } ),
              ( "VSkew 0.4", { "RoundRect_VSkew": 0.4 } ) )
    fig = xf.new()
    row = []
    for k, ( name, parms ) in enumerate( cases ):
        p0 = dict( parms )
        p0[ "RoundRectXSec_Radius" ] = 0.0
        Q, g, xs = xf.body_section( vsp.XS_ROUNDED_RECTANGLE, p0, w = 1.5, h = 1.0 )
        p1 = dict( base )
        p1.update( parms )
        P, f, xs = xf.body_section( vsp.XS_ROUNDED_RECTANGLE, p1, w = 1.5, h = 1.0 )
        c = xf.canvas( fig )
        xf.curve( c, Q, color = GHOST, lw = 0.9, ls = ( 0, ( 4, 2 ) ), z = 40 )
        xf.curve( c, P )
        if k == 0:
            for s, t, dx, dy in ( ( "BR", 0.125, 8, -8 ), ( "BL", 0.375, -8, -8 ), ( "TL", 0.625, -8, 8 ),
                                  ( "TR", 0.875, 8, 8 ) ):
                xf.label( c, xf.at( Q, g, t ), s, dx = dx, dy = dy )
        c.bounds = ( np.array( [ -1.05, -0.75 ] ), np.array( [ 1.05, 0.75 ] ) )
        xf.title( c, name )
        row.append( c )
    xf.key_rear( row[0], at = ( 0.0, 0.0 ), dx = -6, dy = -16 )
    xf.save( fig, "XSecCurves_rrect", [ row ] )


def genfuse_ctrl( w, h, mwl, tta, bta, ts, bs, us, ls ):
    """The right half's control points, as GeneralFuseXSec::UpdateCurve places them (Y, Z), section centered."""
    a = np.radians( tta )
    b = np.radians( bta )
    tp0 = np.array( [ 0.0, h / 2 ] )
    tp1 = tp0 + np.array( [ ts * w / 3, 0 ] )
    tp3 = np.array( [ w / 2, mwl * h / 2 ] )
    tp2 = tp3 + us * h / 3 * np.array( [ -np.cos( a ), np.sin( a ) ] )
    bp1 = tp3 + ls * h / 3 * np.array( [ -np.cos( b ), -np.sin( b ) ] )
    bp3 = np.array( [ 0.0, -h / 2 ] )
    bp2 = bp3 + np.array( [ bs * w / 3, 0 ] )
    return [ tp0, tp1, tp2, tp3 ], [ tp3, bp1, bp2, bp3 ]


def genfuse():
    """General fuse: its control points, default and with a chine."""
    cases = ( ( "Default", {} ),
              ( "MaxWLoc 0.2, TopTanAng 70, BotTanAng 60", { "MaxWidthLoc": 0.2, "TopTanAngle": 70.0,
                                                               "BotTanAngle": 60.0, "UpStr": 1.2 } ) )
    fig = xf.new()
    row = []
    for k, ( name, parms ) in enumerate( cases ):
        P, f, xs = xf.body_section( vsp.XS_GENERAL_FUSE, parms, w = 2.0, h = 2.0 )
        v = { n: xf.gp( xs, n ) for n in ( "MaxWidthLoc", "TopTanAngle", "BotTanAngle", "TopStr", "BotStr", "UpStr",
                                           "LowStr" ) }
        top, bot = genfuse_ctrl( 2.0, 2.0, v[ "MaxWidthLoc" ], v[ "TopTanAngle" ], v[ "BotTanAngle" ], v[ "TopStr" ],
                                 v[ "BotStr" ], v[ "UpStr" ], v[ "LowStr" ] )
        # The control polygon's end points are on the curve: checked against VSP.
        assert np.linalg.norm( xf.at( P, f, 0.0 ) - top[3] ) < 1e-6
        assert np.linalg.norm( xf.at( P, f, 0.75 ) - top[0] ) < 1e-6
        c = xf.canvas( fig )
        xf.curve( c, P )
        for poly in ( top, bot ):
            c.line( xf.to3( poly ), color = MARK, lw = 0.8, ls = ( 0, ( 3, 2 ) ) )
            for q in poly:
                xf.dot( c, q, color = MARK, ms = 4.0 )
        if k == 1:
            xf.label( c, 0.5 * ( top[0] + top[1] ), "TopStr", dy = 9 )
            xf.label( c, 0.5 * ( top[2] + top[3] ), "UpStr", dx = 12, ha = "left" )
            xf.label( c, 0.5 * ( bot[0] + bot[1] ), "LowStr", dx = 12, ha = "left" )
            xf.label( c, 0.5 * ( bot[2] + bot[3] ), "BotStr", dy = -9 )
            c.angle( xf.pt3( top[3] ), xf.pt3( [ -1, 0 ] ), xf.pt3( top[2] - top[3] ), 0.32, INK, label = "TopTanAng",
                     label_r = 1.0, dx = -34, dy = 10 )
            c.angle( xf.pt3( bot[0] ), xf.pt3( [ -1, 0 ] ), xf.pt3( bot[1] - bot[0] ), 0.25, INK, label = "BotTanAng",
                     label_r = 1.0, dx = -36, dy = -8 )
            c.line( xf.to3( [ top[3], top[3] + np.array( [ -0.45, 0 ] ) ] ), color = GHOST, lw = 0.7 )
        c.bounds = ( np.array( [ -1.25, -1.15 ] ), np.array( [ 1.25, 1.15 ] ) )
        xf.title( c, name )
        row.append( c )
    xf.key_rear( row[0], at = ( 0.0, 0.0 ), dx = -6, dy = -16 )
    xf.save( fig, "XSecCurves_genfuse", [ row ] )


def ac25773():
    """The AC 25.773-1 pilot vision envelope, in degrees."""
    fig = xf.new()
    row = []
    for side, name in ( ( vsp.XSEC_LEFT_SIDE, "Pilot Seat Left" ), ( vsp.XSEC_RIGHT_SIDE, "Pilot Seat Right" ) ):
        P, f, xs = xf.body_section( vsp.XS_AC25_773, { "CockpitSide": side } )
        c = xf.canvas( fig )
        for yv in range( -120, 121, 20 ):
            c.line( xf.to3( [ [ yv, -30 ], [ yv, 40 ] ] ), color = "#DDDDDD", lw = 0.5, z = 5 )
        for zv in range( -30, 41, 10 ):
            c.line( xf.to3( [ [ -125, zv ], [ 125, zv ] ] ), color = "#DDDDDD", lw = 0.5, z = 5 )
        c.line( xf.to3( [ [ -125, 0 ], [ 125, 0 ] ] ), color = GHOST, lw = 0.8, z = 6 )
        c.line( xf.to3( [ [ 0, -30 ], [ 0, 40 ] ] ), color = GHOST, lw = 0.8, z = 6 )
        xf.curve( c, P )
        xf.dot( c, xf.at( P, f, 0.0 ), color = MARK )
        for yv in ( -120, -60, 0, 60, 120 ):
            xf.label( c, [ yv, -30 ], "%d" % yv, dy = -8 )
        for zv in ( -30, 0, 40 ):
            xf.label( c, [ -125, zv ], "%d" % zv, dx = -10, ha = "right" )
        xf.title( c, name )
        row.append( c )
    xf.save( fig, "XSecCurves_ac25773", [ [ row[0] ], [ row[1] ] ] )


# The Edit Curve window draws points on the curve black and tangent points green, joined by blue lines.
ONC = "#111111"
TAN = "#2CA02C"
HANDLE = "#1F4FD0"


def edit_curve( setup = None, w = 3.0, h = 2.0, n = 1441 ):
    """A Stack section of type Edit Curve, started from the Circle with R-L Symmetry off; setup( xs ) then edits it.
    Returns the sampled curve (Y, Z), its fractions, the control points (Y, Z), their U and the section."""
    vsp.VSPRenew()
    g = vsp.AddGeom( "STACK" )
    s = vsp.GetXSecSurf( g, 0 )
    vsp.ChangeXSecShape( s, 1, vsp.XS_EDIT_CURVE )
    xs = vsp.GetXSec( s, 1 )
    xf.sp( xs, "SymType", 0 )
    vsp.SetXSecWidthHeight( xs, w, h )
    vsp.Update()
    if setup is not None:
        setup( xs )
    vsp.Update()
    xs = vsp.GetXSec( s, 1 )
    f = np.linspace( 0.0, 1.0, n )
    P = np.array( [ [ q.y(), q.z() ] for q in ( vsp.ComputeXSecPnt( xs, t ) for t in f ) ] )
    w = vsp.GetXSecWidth( xs )
    h = vsp.GetXSecHeight( xs )
    C = np.array( [ [ w * q.x(), h * q.y() ] for q in vsp.GetEditXSecCtrlVec( xs, True ) ] )
    U = np.array( vsp.GetEditXSecUVec( xs ) )
    return P, f, C, U, xs


def bezier_points( c, C ):
    """A closed cubic Bezier Edit Curve's points: on the curve every third, each joined to the tangent points either
    side of it (the last point repeats the first)."""
    for i in range( 0, len( C ) - 1, 3 ):
        for j in ( i - 1, i + 1 ):
            q = C[ j % ( len( C ) - 1 ) ]
            c.line( xf.to3( [ C[ i ], q ] ), color = HANDLE, lw = 0.8, z = 55 )
            xf.dot( c, q, color = TAN, ms = 4.5, z = 72 )
        xf.dot( c, C[ i ], color = ONC, ms = 5.0, z = 74 )


def cedit_bezier():
    """Cubic Bezier Edit Curve: points on the curve, tangent points, and what G1 does when a tangent point moves."""
    MOVE = ( -0.95, -1.45 )      # where tangent point 4 (after the Bottom point) is moved to, model units

    def g1( on ):
        def setup( xs ):
            xf.sp( xs, "G1_3", 1.0 if on else 0.0 )
            vsp.Update()
            vsp.MoveEditXSecPnt( xs, 4, vsp.vec3d( MOVE[0] / 3.0, MOVE[1] / 2.0, 0.0 ) )
        return setup

    P0, f0, C0, U0, xs = edit_curve()
    cases = ( ( "Init Shape Circle", None ), ( "G1 on at Bottom", g1( True ) ),
              ( "G1 off at Bottom", g1( False ) ) )
    fig = xf.new()
    row = []
    for k, ( name, setup ) in enumerate( cases ):
        P, f, C, U, xs = edit_curve( setup )
        c = xf.canvas( fig )
        if k:
            xf.curve( c, P0, color = GHOST, lw = 0.9, ls = ( 0, ( 4, 2 ) ), z = 40 )
            for j in ( 2, 4 ):
                c.line( xf.to3( [ C0[ 3 ], C0[ j ] ] ), color = GHOST, lw = 0.7, ls = ( 0, ( 2, 2 ) ), z = 41 )
        xf.curve( c, P )
        bezier_points( c, C )
        if k == 0:
            for i, dx, dy, ha in ( ( 0, 10, 0, "left" ), ( 3, 0, -12, "center" ), ( 6, -10, 0, "right" ),
                                   ( 9, 0, 12, "center" ) ):
                xf.label( c, C[ i ], "U %g" % round( U[ i ], 4 ), dx = dx, dy = dy, ha = ha )
            xf.label( c, C[ 10 ], "tangent point", dx = 6, dy = 9, ha = "left", color = TAN )
        else:
            xf.label( c, C[ 4 ], "moved", dx = -6, dy = -8, ha = "right", color = TAN )
            if k == 1:
                xf.label( c, [ C[ 2 ][0], -1.3 ], "turned, same length", dx = -4, ha = "left", va = "top", color = TAN )
                c.line( xf.to3( [ C[ 2 ], [ C[ 2 ][0], -1.27 ] ] ), color = TAN, lw = 0.6, z = 45 )
            else:
                xf.label( c, [ C[ 2 ][0], -1.3 ], "not moved", dx = -4, ha = "left", va = "top", color = TAN )
                c.line( xf.to3( [ C[ 2 ], [ C[ 2 ][0], -1.27 ] ] ), color = TAN, lw = 0.6, z = 45 )
        c.bounds = ( np.array( [ -1.75, -1.6 ] ), np.array( [ 1.75, 1.25 ] ) )
        xf.title( c, name )
        row.append( c )
    xf.key_rear( row[0], at = ( 0.0, 0.0 ), dx = -6, dy = -16 )
    xf.save( fig, "XSecCurves_cedit_bezier", [ row ] )


# A Linear Edit Curve with flat sides: U and nondimensional (x, y) of each point, and its corner radius.
POLY = ( ( 0.0, 0.5, 0.0, 0.0 ), ( 0.1, 0.42, -0.5, 0.25 ), ( 0.25, 0.0, -0.5, 0.0 ), ( 0.4, -0.42, -0.5, 0.25 ),
         ( 0.5, -0.5, 0.0, 0.0 ), ( 0.6, -0.3, 0.5, 0.45 ), ( 0.75, 0.0, 0.5, 0.0 ), ( 0.9, 0.3, 0.5, 0.45 ),
         ( 1.0, 0.5, 0.0, 0.0 ) )


def poly_curve( radii = True ):
    def setup( xs ):
        u = [ p[0] for p in POLY ]
        r = [ p[3] if radii else 0.0 for p in POLY ]
        xf.sp( xs, "CurveType", vsp.LINEAR )
        vsp.SetEditXSecPnts( xs, u, [ vsp.vec3d( p[1], p[2], 0.0 ) for p in POLY ], r )
    return edit_curve( setup )


def cedit_radius():
    """Corner radii on a Linear Edit Curve."""
    Q, g, CQ, UQ, xs = poly_curve( radii = False )
    P, f, C, U, xs = poly_curve()
    fig = xf.new()
    c = xf.canvas( fig )
    xf.curve( c, Q, color = GHOST, lw = 0.9, ls = ( 0, ( 4, 2 ) ), z = 40 )
    xf.curve( c, P )
    for i in range( len( C ) - 1 ):
        xf.dot( c, C[ i ], color = ONC, ms = 5.0, z = 74 )
    for i, dx, dy, ha in ( ( 1, 8, -8, "left" ), ( 3, -8, -8, "right" ), ( 5, -8, 8, "right" ), ( 7, 8, 8, "left" ) ):
        xf.label( c, C[ i ], "R %g" % POLY[ i ][3], dx = dx, dy = dy, ha = ha )
    xf.key_rear( c, at = ( 0.0, 0.0 ), dx = -110, dy = 0 )
    xf.save( fig, "XSecCurves_cedit_radius", [ [ c ] ], hmax = 2.6 )

FIGS = { "param": param, "super_exp": super_exp, "super_more": super_more, "rrect": rrect, "genfuse": genfuse,
         "ac25773": ac25773,
         "cedit_bezier": cedit_bezier, "cedit_radius": cedit_radius }

if __name__ == "__main__":
    xf.style()
    names = sys.argv[1:] or list( FIGS )
    for n in names:
        FIGS[ n ]()
