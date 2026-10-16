"""Checks, against the openvsp module, what XSecCurves.md says about the section shapes.

    <build>/venv/bin/python verify.py
"""

import numpy as np
import openvsp as vsp

import xsfig as xf

FAILS = []


def check( ok, what ):
    print( ( "PASS  " if ok else "FAIL  " ) + what )
    if not ok:
        FAILS.append( what )


def body_frame( kind ):
    """Where W = 0, 0.25, 0.5 and 0.75 fall on a body section."""
    vsp.VSPRenew()
    g = vsp.AddGeom( kind )
    s = vsp.GetXSecSurf( g, 0 )
    vsp.ChangeXSecShape( s, 1, vsp.XS_ELLIPSE )
    xs = vsp.GetXSec( s, 1 )
    vsp.SetXSecWidthHeight( xs, 4.0, 2.0 )
    vsp.Update()
    xs = vsp.GetXSec( s, 1 )
    q = [ vsp.ComputeXSecPnt( xs, t ) for t in ( 0.0, 0.25, 0.5, 0.75 ) ]
    yz = [ ( p.y(), p.z() ) for p in q ]
    cx = 0.5 * ( yz[0][0] + yz[2][0] )
    cz = 0.5 * ( yz[1][1] + yz[3][1] )
    return [ ( a - cx, b - cz ) for a, b in yz ]


for kind in ( "STACK", "FUSELAGE" ):
    yz = body_frame( kind )
    want = [ ( 2, 0 ), ( 0, -1 ), ( -2, 0 ), ( 0, 1 ) ]
    check( all( np.allclose( a, b, atol = 1e-6 ) for a, b in zip( yz, want ) ),
           "%s: W 0 at +Y (Right), 0.25 at -Z (Bottom), 0.5 at -Y (Left), 0.75 at +Z (Top)" % kind )

# Super ellipse: M is the width exponent, N the height exponent.
P, f, xs = xf.body_section( vsp.XS_SUPER_ELLIPSE, { "Super_M": 4.0, "Super_N": 2.0 }, w = 2.0, h = 1.0 )
top = P[ P[:, 1] > 1e-6 ]
r = np.abs( top[:, 0] / 1.0 ) ** 4.0 + np.abs( top[:, 1] / 0.5 ) ** 2.0
check( np.max( np.abs( r - 1.0 ) ) < 2e-3, "Super ellipse: |y/(w/2)|^M + |z/(h/2)|^N = 1 (max err %.1e)" % np.max( np.abs( r - 1 ) ) )

P, f, xs = xf.body_section( vsp.XS_SUPER_ELLIPSE, { "Super_MaxWidthLoc": 0.5 }, w = 2.0, h = 2.0 )
zw = P[ np.argmax( P[:, 0] ), 1 ]
check( abs( zw - 0.5 ) < 1e-3, "Super ellipse: MaxWLoc 0.5 puts the widest point at 0.5 h/2 (%.4f)" % zw )

# Rounded rectangle: keystone, skew and vertical skew, with square corners.
P, f, xs = xf.body_section( vsp.XS_ROUNDED_RECTANGLE, { "RoundRectXSec_Radius": 0.0, "RoundRect_Keystone": 0.7 },
                            w = 2.0, h = 1.0 )
tw = P[ P[:, 1] > 0.499 ][:, 0]
bw = P[ P[:, 1] < -0.499 ][:, 0]
check( abs( ( tw.max() - tw.min() ) - 2 * 0.7 * 2.0 ) < 1e-3 and abs( ( bw.max() - bw.min() ) - 2 * 0.3 * 2.0 ) < 1e-3,
       "Rounded rectangle: Keystone k makes the top 2k w and the bottom 2(1-k) w wide" )
P, f, xs = xf.body_section( vsp.XS_ROUNDED_RECTANGLE, { "RoundRectXSec_Radius": 0.0, "RoundRect_Skew": 0.4 },
                            w = 2.0, h = 1.0 )
tw = P[ P[:, 1] > 0.499 ][:, 0]
check( abs( tw.mean() - 0.4 * 1.0 ) < 1e-2, "Rounded rectangle: Skew s moves the top s w/2 toward +Y (%.3f)" % tw.mean() )
P, f, xs = xf.body_section( vsp.XS_ROUNDED_RECTANGLE, { "RoundRectXSec_Radius": 0.0, "RoundRect_VSkew": 0.4 },
                            w = 2.0, h = 1.0 )
rz = P[ P[:, 0] > 0.999 ][:, 1]
check( abs( rz.mean() - 0.4 * 0.5 ) < 1e-2, "Rounded rectangle: VSkew v moves the right side v h/2 up (%.3f)" % rz.mean() )

# Changing type keeps the size.
vsp.VSPRenew()
g = vsp.AddGeom( "STACK" )
s = vsp.GetXSecSurf( g, 0 )
vsp.ChangeXSecShape( s, 1, vsp.XS_ELLIPSE )
vsp.SetXSecWidthHeight( vsp.GetXSec( s, 1 ), 3.0, 2.0 )
vsp.Update()
vsp.ChangeXSecShape( s, 1, vsp.XS_CIRCLE )
vsp.Update()
check( abs( xf.gp( vsp.GetXSec( s, 1 ), "Circle_Diameter" ) - 2.5 ) < 1e-9, "Ellipse 3 x 2 to Circle: diameter 2.5" )
vsp.ChangeXSecShape( s, 1, vsp.XS_ELLIPSE )
vsp.SetXSecWidthHeight( vsp.GetXSec( s, 1 ), 3.0, 0.3 )
vsp.Update()
vsp.ChangeXSecShape( s, 1, vsp.XS_FOUR_SERIES )
vsp.Update()
xs = vsp.GetXSec( s, 1 )
check( abs( xf.gp( xs, "Chord" ) - 3.0 ) < 1e-9 and abs( xf.gp( xs, "ThickChord" ) - 0.1 ) < 1e-9,
       "Ellipse 3 x 0.3 to Four Series: chord 3, T/C 0.1" )

# AC 25.773: fixed size, in degrees.
P, f, xs = xf.body_section( vsp.XS_AC25_773 )
check( abs( np.ptp( P[:, 0] ) - 140 ) < 1e-6 and abs( np.ptp( P[:, 1] ) - 62 ) < 1e-6,
       "AC 25.773: 140 by 62, whatever the Width and Height" )

# Area: the area inside the curve.
P, f, xs = xf.body_section( vsp.XS_SUPER_ELLIPSE, w = 3.0, h = 2.5 )
a = 0.5 * abs( np.sum( P[:-1, 0] * P[1:, 1] - P[1:, 0] * P[:-1, 1] ) )
check( abs( a - xf.gp( xs, "Area" ) ) < 1e-3 * a, "Area reports the area inside the curve (%.4f vs %.4f)" % ( a, xf.gp( xs, "Area" ) ) )

# General fuse corner radius rounds the side.
P0, f, xs = xf.body_section( vsp.XS_GENERAL_FUSE, { "TopTanAngle": 60.0, "BotTanAngle": 60.0 }, w = 2.0, h = 2.0 )
P1, f, xs = xf.body_section( vsp.XS_GENERAL_FUSE, { "TopTanAngle": 60.0, "BotTanAngle": 60.0, "CornerRad": 0.1 },
                             w = 2.0, h = 2.0 )
check( P1[:, 0].max() < P0[:, 0].max() - 1e-3, "General fuse: CornerRad rounds off the chine at the side" )

# File points: width and height from their extent.
vsp.VSPRenew()
g = vsp.AddGeom( "STACK" )
s = vsp.GetXSecSurf( g, 0 )
vsp.ChangeXSecShape( s, 1, vsp.XS_FILE_FUSE )
xs = vsp.GetXSec( s, 1 )
th = np.linspace( 0, -2 * np.pi, 41 )
pts = [ vsp.vec3d( 2.0 * np.cos( t ), 0.5 * np.sin( t ), 0.0 ) for t in th ]
vsp.SetXSecPnts( xs, pts )
vsp.Update()
check( abs( vsp.GetXSecWidth( xs ) - 4.0 ) < 1e-6 and abs( vsp.GetXSecHeight( xs ) - 1.0 ) < 1e-6,
       "Fuse file: Width and Height are the points' extent" )
bad = [ vsp.vec3d( 2.0 * np.cos( t ), 0.5 * np.sin( t ), 0.0 ) for t in -th ]
vsp.SetXSecPnts( xs, bad )
vsp.Update()
p = vsp.ComputeXSecPnt( xs, 0.25 )
check( p.z() < 0, "Fuse file: points running the wrong way round are refused, and the old shape stays" )

# Representation: a Circle is four cubic pieces, 0.02 percent off a true circle; a Super Ellipse much closer.
for shape, lim, name in ( ( vsp.XS_CIRCLE, 2.5e-4, "Circle" ), ( vsp.XS_SUPER_ELLIPSE, 1e-5, "Super Ellipse" ) ):
    P, f, xs = xf.body_section( shape, w = 2.0, h = 2.0, n = 20001 )
    r = np.hypot( P[:, 0], P[:, 1] )
    err = np.max( np.abs( r - 1.0 ) )
    check( err < lim and ( shape != vsp.XS_CIRCLE or err > 1.5e-4 ),
           "%s of radius 1 departs from a true circle by %.1e" % ( name, err ) )

# Area driving reaches the area with corner radii given as lengths.
for shape in ( vsp.XS_ROUNDED_RECTANGLE, vsp.XS_EDIT_CURVE ):
    vsp.VSPRenew()
    g = vsp.AddGeom( "STACK" )
    s = vsp.GetXSecSurf( g, 0 )
    vsp.ChangeXSecShape( s, 1, shape )
    xs = vsp.GetXSec( s, 1 )
    vsp.SetXSecWidthHeight( xs, 3.0, 2.0 )
    if shape == vsp.XS_ROUNDED_RECTANGLE:
        xf.sp( xs, "RoundRectXSec_Radius", 0.3 )
    else:
        xf.sp( xs, "ShapeType", vsp.EDIT_XSEC_RECTANGLE )
        vsp.EditXSecInitShape( xs )
        vsp.SetXSecWidthHeight( xs, 3.0, 2.0 )
        xf.sp( xs, "R_1", 0.3 )
    vsp.Update()
    vsp.SetDriverGroupByID( xs, vsp.WIDTH_XSEC_DRIVER, vsp.AREA_XSEC_DRIVER )
    xf.sp( xs, "Area", 2.0 )
    vsp.Update()
    xs = vsp.GetXSec( s, 1 )
    f = np.linspace( 0, 1, 20001 )
    P = np.array( [ [ q.y(), q.z() ] for q in ( vsp.ComputeXSecPnt( xs, t ) for t in f ) ] )
    a = 0.5 * abs( np.sum( P[:-1, 0] * P[1:, 1] - P[1:, 0] * P[:-1, 1] ) )
    check( abs( a - 2.0 ) < 1e-4, "Area drives a shape with corner radii to its value (type %d: %.6f)" % ( shape, a ) )


# ---------------------------------------------------------------- Edit Curve

def edit( init = None, sym = 0, w = 3.0, h = 2.0, kind = "STACK" ):
    vsp.VSPRenew()
    g = vsp.AddGeom( kind )
    s = vsp.GetXSecSurf( g, 0 )
    vsp.ChangeXSecShape( s, 1, vsp.XS_EDIT_CURVE )
    xs = vsp.GetXSec( s, 1 )
    xf.sp( xs, "SymType", sym )
    if init is not None:
        xf.sp( xs, "ShapeType", init )
        vsp.EditXSecInitShape( xs )
    if kind != "WING":
        vsp.SetXSecWidthHeight( xs, w, h )
    vsp.Update()
    return s, vsp.GetXSec( s, 1 )


def sample( xs, n = 4001 ):
    f = np.linspace( 0, 1, n )
    return np.array( [ [ q.x(), q.y(), q.z() ] for q in ( vsp.ComputeXSecPnt( xs, t ) for t in f ) ] )


# Init Shape: Circle is Cubic Bezier, Ellipse Spline, Rectangle Linear; G1 off on every point of the Circle.
for init, ctype in ( ( vsp.EDIT_XSEC_CIRCLE, vsp.CEDIT ), ( vsp.EDIT_XSEC_ELLIPSE, vsp.PCHIP ),
                     ( vsp.EDIT_XSEC_RECTANGLE, vsp.LINEAR ) ):
    s, xs = edit( init )
    check( int( xf.gp( xs, "CurveType" ) ) == ctype, "Init Shape %d gives curve type %d" % ( init, ctype ) )
s, xs = edit( vsp.EDIT_XSEC_CIRCLE, sym = 1 )
n = len( vsp.GetEditXSecUVec( xs ) )
check( not any( xf.gp( xs, "G1_%d" % i ) for i in range( n ) ), "Init Shape Circle: G1 off on every point" )

# The Spline Ellipse is more than half a percent off a true ellipse.
s, xs = edit( vsp.EDIT_XSEC_ELLIPSE, w = 2.0, h = 2.0 )
P = sample( xs )
r = np.hypot( P[:, 1], P[:, 2] )
check( np.max( np.abs( r - 1 ) ) > 5e-3, "Init Shape Ellipse (Spline) departs from an ellipse by %.4f" % np.max(
    np.abs( r - 1 ) ) )

# Splitting: Bezier and Linear keep the shape, Spline changes it; a Bezier split point has G1 off.
for init, keeps in ( ( vsp.EDIT_XSEC_CIRCLE, True ), ( vsp.EDIT_XSEC_RECTANGLE, True ),
                     ( vsp.EDIT_XSEC_ELLIPSE, False ) ):
    s, xs = edit( init, w = 2.0, h = 2.0 )
    A = sample( xs, 1601 )
    vsp.EditXSecSplit01( xs, 0.1 )
    vsp.Update()
    xs = vsp.GetXSec( s, 1 )
    d = np.max( np.linalg.norm( A - sample( xs, 1601 ), axis = 1 ) )
    check( ( d < 1e-9 ) == keeps, "Split at U 0.1 of init %d moves the curve by %.1e" % ( init, d ) )
    if init == vsp.EDIT_XSEC_CIRCLE:
        U = vsp.GetEditXSecUVec( xs )
        i = int( np.argmin( np.abs( np.array( U ) - 0.1 ) ) )
        check( xf.gp( xs, "G1_%d" % i ) == 0, "A point made by Split has G1 off" )

# G1: moving a tangent point turns the opposite one into line, keeping its length; off, the other stays.
for on in ( True, False ):
    s, xs = edit( vsp.EDIT_XSEC_CIRCLE )
    xf.sp( xs, "G1_3", 1.0 if on else 0.0 )
    vsp.Update()
    C0 = [ np.array( [ q.x(), q.y() ] ) for q in vsp.GetEditXSecCtrlVec( xs, True ) ]
    vsp.MoveEditXSecPnt( xs, 4, vsp.vec3d( -0.3, -0.7, 0.0 ) )
    vsp.Update()
    C1 = [ np.array( [ q.x(), q.y() ] ) for q in vsp.GetEditXSecCtrlVec( xs, True ) ]
    a = C1[2] - C1[3]
    b = C1[4] - C1[3]
    inline = abs( a[0] * b[1] - a[1] * b[0] ) < 1e-9 and np.dot( a, b ) < 0
    same = abs( np.linalg.norm( a ) - np.linalg.norm( C0[2] - C0[3] ) ) < 1e-9
    if on:
        check( inline and same, "G1: the opposite tangent point turns into line and keeps its length" )
    else:
        check( np.linalg.norm( C1[2] - C0[2] ) < 1e-12, "G1 off: the opposite tangent point does not move" )

# Switching G1 on lines the two tangents up along their average direction, keeping their lengths.
s, xs = edit( vsp.EDIT_XSEC_CIRCLE )
vsp.MoveEditXSecPnt( xs, 4, vsp.vec3d( -0.3, -0.7, 0.0 ) )
vsp.Update()
C0 = [ np.array( [ q.x(), q.y() ] ) for q in vsp.GetEditXSecCtrlVec( xs, True ) ]
xf.sp( xs, "G1_3", 1.0 )
vsp.Update()
C1 = [ np.array( [ q.x(), q.y() ] ) for q in vsp.GetEditXSecCtrlVec( xs, True ) ]
ang = lambda v: np.degrees( np.arctan2( v[1], v[0] ) )  # noqa: E731
avg = 0.5 * ( ang( C0[2] - C0[3] ) + ang( C0[3] - C0[4] ) )
check( abs( ang( C1[2] - C1[3] ) - avg ) < 1e-9 and abs( ang( C1[3] - C1[4] ) - avg ) < 1e-9
       and abs( np.linalg.norm( C1[4] - C1[3] ) - np.linalg.norm( C0[4] - C0[3] ) ) < 1e-12,
       "Switching G1 on turns both tangents to their average direction (%.2f deg), lengths kept" % avg )

# Corner radius: a circular arc of that radius in model units, unchanged by Width; none on a Spline.
s, xs = edit( vsp.EDIT_XSEC_RECTANGLE, sym = 1 )
xf.sp( xs, "R_1", 0.3 )
vsp.Update()
for w, h in ( ( 3.0, 2.0 ), ( 6.0, 4.0 ) ):
    vsp.SetXSecWidthHeight( xs, w, h )
    vsp.Update()
    want = w * h - 2 * ( 1 - np.pi / 4 ) * 0.3 ** 2
    check( abs( xf.gp( xs, "Area" ) - want ) < 1e-4,
           "Linear corner radius 0.3 (and its mirror) is a circular arc of radius 0.3 at Width %g" % w )
P = sample( xs, 40001 )
m = ( P[:, 1] > 2.7 ) & ( P[:, 2] < -1.7 )
d = np.hypot( P[m, 1] - 2.7, P[m, 2] + 1.7 )
check( np.max( np.abs( d - 0.3 ) ) < 1e-4, "Corner arc stays at radius 0.3 about its center (%.1e)" % np.max(
    np.abs( d - 0.3 ) ) )
vsp.EditXSecConvertTo( xs, vsp.CEDIT )
vsp.Update()
xs = vsp.GetXSec( s, 1 )
check( abs( xf.gp( xs, "Area" ) - want ) < 1e-4, "Converting Linear to Cubic Bezier keeps the corner radii" )
s, xs = edit( vsp.EDIT_XSEC_ELLIPSE )
xf.sp( xs, "R_2", 0.2 )
vsp.Update()
check( xf.gp( xs, "R_2" ) == 0.0, "A Spline point cannot carry a corner radius" )
s, xs = edit( vsp.EDIT_XSEC_CIRCLE )
xf.sp( xs, "G1_3", 1.0 )
xf.sp( xs, "R_3", 0.2 )
vsp.Update()
check( xf.gp( xs, "R_3" ) == 0.0, "A Bezier point with G1 on cannot carry a corner radius" )

# Fix U: Reparameterize by Arc Length keeps a fixed U and moves an unfixed Bottom.
for fix in ( False, True ):
    s, xs = edit( vsp.EDIT_XSEC_RECTANGLE, w = 4.0, h = 1.0 )
    xf.sp( xs, "U_1", 0.1 )
    xf.sp( xs, "FixedU_1", 1.0 if fix else 0.0 )
    vsp.Update()
    vsp.ReparameterizeEditXSec( xs )
    vsp.Update()
    U = vsp.GetEditXSecUVec( xs )
    if fix:
        check( abs( U[1] - 0.1 ) < 1e-12 and abs( U[2] - 0.25 ) > 1e-3,
               "Reparameterize keeps a Fix U point's U (%.3f) and moves an unfixed Bottom (%.3f)" % ( U[1], U[2] ) )
    else:
        check( abs( U[1] - 0.05 ) < 1e-9, "Reparameterize spaces unfixed points by arc length (%.3f)" % U[1] )
s, xs = edit( vsp.EDIT_XSEC_RECTANGLE, sym = 1, w = 4.0, h = 1.0 )
xf.sp( xs, "U_1", 0.1 )
vsp.Update()
vsp.ReparameterizeEditXSec( xs )
vsp.Update()
U = vsp.GetEditXSecUVec( xs )
check( abs( U[2] - 0.25 ) < 1e-12, "R-L Symmetry holds the Bottom at U 0.25 through Reparameterize" )

# Depth: Z times Depth, toward +X on a Stack or Fuselage, +Y on a Wing.
for kind, axis in ( ( "STACK", 0 ), ( "FUSELAGE", 0 ), ( "WING", 1 ) ):
    s, xs = edit( kind = kind )
    p0 = vsp.ComputeXSecPnt( xs, 0.25 )
    xf.sp( xs, "Depth", 2.0 )
    xf.sp( xs, "Z_3", 0.3 )
    vsp.Update()
    p1 = vsp.ComputeXSecPnt( xs, 0.25 )
    dv = np.array( [ p1.x() - p0.x(), p1.y() - p0.y(), p1.z() - p0.z() ] )
    want = np.zeros( 3 )
    want[ axis ] = 0.6
    check( np.allclose( dv, want, atol = 1e-9 ), "%s: Z 0.3 at Depth 2 moves a point %s" % ( kind, np.round( dv, 6 ) ) )

# Split and convert rebuild the points' Parms; so does delete with R-L Symmetry on, while off it renumbers them.
for op, sym in ( ( "split", 0 ), ( "convert", 0 ), ( "delete", 1 ) ):
    s, xs = edit( vsp.EDIT_XSEC_CIRCLE, sym = sym )
    pid = vsp.GetXSecParm( xs, "X_9" )
    if op == "split":
        vsp.EditXSecSplit01( xs, 0.1 )
    elif op == "convert":
        vsp.EditXSecConvertTo( xs, vsp.LINEAR )
    else:
        vsp.EditXSecDelPnt( xs, 6 )
    vsp.Update()
    check( not vsp.ValidParm( pid ), "%s (R-L Symmetry %d) replaces the points' Parms" % ( op, sym ) )
s, xs = edit( vsp.EDIT_XSEC_CIRCLE, sym = 0 )
pid = vsp.GetXSecParm( xs, "X_9" )
vsp.EditXSecDelPnt( xs, 6 )
vsp.Update()
check( vsp.ValidParm( pid ) and vsp.GetParmName( pid ) == "X_6",
       "delete with R-L Symmetry off keeps the later points' Parms, renumbered" )

print()
print( "%d failed" % len( FAILS ) )
