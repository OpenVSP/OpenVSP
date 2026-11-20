"""Checks, against the openvsp module, what Airfoils.md says about airfoils and the Modify tab.

    <build>/venv/bin/python verify.py

The last two checks record behaviour the page documents but that looks unintended; they pass while it lasts.
"""

import math
import os
import sys

import numpy as np
import openvsp as vsp

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "xseccurves" ) )

import xsfig as xf                     # noqa: E402

FAILS = []
FOUR = { "Camber": 0.02, "CamberLoc": 0.4, "ThickChord": 0.12 }


def check( ok, what ):
    print( ( "PASS  " if ok else "FAIL  " ) + what )
    if not ok:
        FAILS.append( what )


def wing( parms = None, groups = None, shape = vsp.XS_FOUR_SERIES ):
    return xf.wing_section( shape, parms, groups )


# Layout of the curve.
P, f, xs = wing( FOUR )
check( np.allclose( xf.at( P, f, 0.0 ), [ 1, 0 ], atol = 1e-9 ) and np.allclose( xf.at( P, f, 0.5 ), [ 0, 0 ], atol = 1e-9 )
       and xf.at( P, f, 0.25 )[1] < 0 and xf.at( P, f, 0.75 )[1] > 0,
       "Curve starts at the trailing edge, runs under the lower surface to the leading edge at 0.5, back over the top" )

# Shift, rotate, scale.
P, f, xs = wing( dict( FOUR, Theta = 10.0 ) )
te = xf.at( P, f, 0.0 )
check( np.allclose( te, [ math.cos( math.radians( 10 ) ), math.sin( math.radians( 10 ) ) ], atol = 1e-6 )
       and np.allclose( xf.at( P, f, 0.5 ), [ 0, 0 ], atol = 1e-9 ), "Theta +10 turns the airfoil about its leading edge, trailing edge up" )
P, f, xs = wing( dict( FOUR, DeltaX = 0.1, DeltaY = 0.05 ) )
check( np.allclose( xf.at( P, f, 0.5 ), [ 0.1, 0.05 ], atol = 1e-9 ), "Delta X/C and Delta Y/C move it in chords" )
P, f, xs = wing( dict( FOUR, Scale = 0.5 ) )
check( np.allclose( xf.at( P, f, 0.0 ), [ 0.5, 0 ], atol = 1e-9 ), "Scale shrinks it toward the leading edge" )
P, f, xs = wing( dict( FOUR, ShiftLE = 0.2 ) )
B, g, xs = wing( FOUR )
d = np.min( np.linalg.norm( B - xf.at( P, f, 0.5 ), axis = 1 ) )
check( xf.at( P, f, 0.5 )[1] > 0.01 and d < 2e-3, "Shift LE +0.2 moves the 0.5 point onto the upper surface; the shape stays" )

# Design lift coefficient of the four digit mean line, from thin airfoil theory.
vsp.VSPRenew()
w = vsp.AddGeom( "WING" )
s = vsp.GetXSecSurf( w, 0 )
xs = vsp.GetXSec( s, 0 )
for k, v in FOUR.items():
    xf.sp( xs, k, v )
vsp.Update()
cli = xf.gp( xs, "IdealCl" )
m, p = 0.02, 0.4
th = np.linspace( 0, np.pi, 200001 )
x = 0.5 * ( 1 - np.cos( th ) )
dz = np.where( x < p, 2 * m / p ** 2 * ( p - x ), 2 * m / ( 1 - p ) ** 2 * ( p - x ) )
A1 = 2 / np.pi * np.trapezoid( dz * np.cos( th ), th )
A0 = -1 / np.pi * np.trapezoid( dz, th )        # alpha 0
# Ideal angle of attack: A0 = 0 at alpha = mean slope; cl_i = pi ( 2 A0 + A1 ) with A0 at ideal alpha = 0 -> cl = pi A1
check( abs( cli - np.pi * A1 ) < 1e-4, "Four series Ideal CL = pi A1 of thin airfoil theory (%.5f vs %.5f)" % ( cli, np.pi * A1 ) )
xf.sp( xs, "CamberInputFlag", vsp.DESIGN_CL )
xf.sp( xs, "IdealCl", 2 * cli )
vsp.Update()
check( abs( xf.gp( xs, "Camber" ) - 0.04 ) < 1e-9, "Four series: Ideal CL drives Camber, which is proportional to it" )

# Six series A forms take a = 0.8.
vsp.ChangeXSecShape( s, 0, vsp.XS_SIX_SERIES )
vsp.Update()
xs = vsp.GetXSec( s, 0 )
xf.sp( xs, "Series", vsp.SIX_SERIES_64A )
xf.sp( xs, "A", 0.3 )
vsp.Update()
check( abs( xf.gp( xs, "A" ) - 0.8 ) < 1e-9, "Six series 64A: a is held at 0.8" )

# Trailing edge closure.
P, f, xs = wing( FOUR, { "TE_Close_Type": ( "Close", vsp.CLOSE_SKEWLOW ), "TE_Close_AbsRel": ( "Close", vsp.REL ),
                         "TE_Close_Thick_Chord": ( "Close", 0.03 ) } )
lo = xf.at( P, f, 0.0011 )
up = xf.at( P, f, 0.9989 )
check( abs( up[1] - lo[1] - 0.03 ) < 2e-3 and abs( up[1] ) < 1e-3, "TE SKEW LOWER: lower surface sheared down to a T/C 0.03 face" )
check( np.allclose( xf.at( P, f, 0.5 ), [ 0, 0 ], atol = 1e-6 ), "TE closure leaves the leading edge where it was" )

# Leading edge closure opens a face of thickness T at the leading edge.
P, f, xs = wing( FOUR, { "LE_Close_Type": ( "Close", vsp.CLOSE_SKEWLOW ), "LE_Close_AbsRel": ( "Close", vsp.REL ),
                         "LE_Close_Thick_Chord": ( "Close", 0.03 ) } )
a = xf.at( P, f, 0.499 )
b = xf.at( P, f, 0.501 )
check( abs( np.linalg.norm( b - a ) - 0.03 ) < 2e-3, "LE SKEW LOWER: a blunt leading edge face 0.03 chords high (%.4f)" % np.linalg.norm( b - a ) )

# Trim by X measures from the trailing edge corner.
P, f, xs = wing( FOUR, { "TE_Trim_Type": ( "Trim", vsp.TRIM_X ), "TE_Trim_AbsRel": ( "Trim", vsp.REL ),
                         "TE_Trim_X_Chord": ( "Trim", 0.2 ) } )
lo = xf.at( P, f, 0.0011 )
check( abs( np.linalg.norm( lo - [ 1, 0 ] ) - 0.2 ) < 2e-3, "TE Trim X/C 0.2: the new corner is 0.2 chords from the old trailing edge" )
c0 = xf.gp( xs, "Chord" )
check( abs( c0 - vsp.GetXSecWidth( xs ) ) < 1e-12, "Trim leaves Chord unchanged" )

# Flap: positive deflection puts the trailing edge down.
P, f, xs = wing( FOUR, { "TE_Flap_Type": ( "Flap", vsp.FLAP_PLAIN ), "TE_Flap_Deflection": ( "Flap", 20.0 ) } )
check( xf.at( P, f, 0.0 )[1] < -0.05, "Plain flap +20: trailing edge down" )

# Caps: offset signs.
blunt = { "TE_Trim_Type": ( "Trim", vsp.TRIM_THICK ), "TE_Trim_AbsRel": ( "Trim", vsp.REL ),
          "TE_Trim_Thick_Chord": ( "Trim", 0.04 ), "TE_Cap_Type": ( "Cap", vsp.ROUND_END_CAP ) }
P0, f, xs = wing( FOUR, blunt )
P1, f, xs = wing( FOUR, dict( blunt, TE_Cap_Offset = ( "Cap", 0.5 ) ) )
tip0 = P0[ np.argmax( P0[:, 0] ) ]
tip1 = P1[ np.argmax( P1[:, 0] ) ]
check( tip1[1] > tip0[1] + 1e-3, "TE cap Offset +: the tip moves toward the upper surface" )
le = { "LE_Trim_Type": ( "Trim", vsp.TRIM_THICK ), "LE_Trim_AbsRel": ( "Trim", vsp.REL ),
       "LE_Trim_Thick_Chord": ( "Trim", 0.06 ), "LE_Cap_Type": ( "Cap", vsp.ROUND_END_CAP ) }
P0, f, xs = wing( FOUR, le )
P1, f, xs = wing( FOUR, dict( le, LE_Cap_Offset = ( "Cap", 0.5 ) ) )
tip0 = P0[ np.argmin( P0[:, 0] ) ]
tip1 = P1[ np.argmin( P1[:, 0] ) ]
check( tip1[1] < tip0[1] - 1e-3, "LE cap Offset +: the tip moves toward the lower surface" )

# File airfoil: T/C scales the thickness about the camber line.
vsp.VSPRenew()
w = vsp.AddGeom( "WING" )
s = vsp.GetXSecSurf( w, 0 )
vsp.ChangeXSecShape( s, 0, vsp.XS_FILE_AIRFOIL )
xs = vsp.GetXSec( s, 0 )
xx = 0.5 * ( 1 - np.cos( np.linspace( 0, np.pi, 81 ) ) )
cam = 0.04 * xx * ( 1 - xx )
th = 0.6 * ( 0.2969 * np.sqrt( xx ) - 0.126 * xx - 0.3516 * xx ** 2 + 0.2843 * xx ** 3 - 0.1036 * xx ** 4 )
up = [ vsp.vec3d( a, b + c, 0 ) for a, b, c in zip( xx, cam, th ) ]
lo = [ vsp.vec3d( a, b - c, 0 ) for a, b, c in zip( xx, cam, th ) ]
vsp.SetAirfoilPnts( xs, up, lo )
vsp.Update()
base = xf.gp( xs, "BaseThickChord" )
xf.sp( xs, "ThickChord", 2 * base )
vsp.Update()
c = vsp.GetXSecWidth( xs )
mid_u = vsp.ComputeXSecPnt( xs, 0.75 )
mid_l = vsp.ComputeXSecPnt( xs, 0.25 )
check( abs( base - 0.12 ) < 3e-3, "File airfoil: Base T/C is the file's thickness (%.4f)" % base )
check( abs( ( mid_u.z() - mid_l.z() ) / c - 2 * 2 * np.interp( mid_u.x() / c, xx, th ) ) < 0.02,
       "File airfoil: doubling T/C doubles the thickness" )

# Rounded leading edge: a 5% diamond with a radius of 0.005.
def upper_crest( P, f ):
    up = f >= 0.5
    i = np.flatnonzero( up )[ int( np.argmax( P[ up, 1 ] ) ) ]
    return P[ i ]


def half_angle( P, f, x0, x1 ):
    up = ( f >= 0.5 ) & ( P[ :, 0 ] > x0 ) & ( P[ :, 0 ] < x1 )
    k = np.polyfit( P[ up, 0 ], P[ up, 1 ], 1 )[ 0 ]
    return math.degrees( math.atan( abs( k ) ) )


LE = { "ThickChord": 0.05 }
S, fs, xs = wing( LE, shape = vsp.XS_WEDGE )
check( abs( half_angle( S, fs, 0.1, 0.4 ) - 2.862 ) < 0.005, "Sharp 5% diamond: half-angle 2.86 degrees" )
for shape in ( vsp.XS_WEDGE, vsp.XS_BICONVEX ):
    for keep in ( vsp.LE_RADIUS_KEEP_CREST, vsp.LE_RADIUS_KEEP_ANGLE ):
        P, f, xs = wing( dict( LE, LERadiusType = keep, LERadius = 0.005 ), shape = shape )
        front = P[ int( np.argmin( P[ :, 0 ] ) ) ]
        near = P[ P[ :, 0 ] < 0.0025 ]
        rad = np.linalg.norm( near - [ 0.005, 0.0 ], axis = 1 )
        check( abs( front[ 0 ] ) < 1e-6 and abs( front[ 1 ] ) < 2e-4 and abs( P[ :, 0 ].max() - 1.0 ) < 1e-9
               and np.allclose( xf.at( P, f, 0.5 ), [ 0, 0 ], atol = 1e-9 ) and np.all( abs( rad - 0.005 ) < 3e-6 ),
               "Rounded (%d, keep %d): runs 0 to 1, leading edge at the origin, nose a circle of radius 0.005" % ( shape, keep ) )

P, f, xs = wing( dict( LE, LERadius = 0.005 ), shape = vsp.XS_WEDGE )
c = upper_crest( P, f )
check( abs( c[ 0 ] - 0.5 ) < 2e-3 and abs( c[ 1 ] - 0.025 ) < 1e-7 and abs( half_angle( P, f, 0.15, 0.45 ) - 2.31 ) < 0.01,
       "Keep Crest is the default: crest at 0.5, forward half-angle %.3f (page 2.31)" % half_angle( P, f, 0.15, 0.45 ) )
check( xf.gp( xs, "LERadiusType" ) == vsp.LE_RADIUS_KEEP_CREST,
       "LERadiusType defaults to LE_RADIUS_KEEP_CREST" )
P, f, xs = wing( dict( LE, LERadiusType = vsp.LE_RADIUS_KEEP_ANGLE, LERadius = 0.005 ), shape = vsp.XS_WEDGE )
c = upper_crest( P, f )
fa = half_angle( P, f, 0.1, 0.38 )
aa = half_angle( P, f, 0.45, 0.95 )
check( abs( c[ 0 ] - 0.405 ) < 2e-3 and abs( fa - 2.862 ) < 0.005 and abs( aa - 2.41 ) < 0.01,
       "Keep Angle: crest at %.3f (page 0.405), forward %.3f (2.86), aft %.3f (2.41)" % ( c[ 0 ], fa, aa ) )
sharp_t = 0.005 / math.tan( math.atan( 0.025 / 0.5 ) )
check( abs( sharp_t - 0.1 ) < 0.002, "The sides of the sharp diamond meet the circle a tenth of the chord back (%.4f)" % sharp_t )
P, f, xs = wing( dict( LE, ThickFracUp = 1.0, LERadius = 0.005 ), shape = vsp.XS_BICONVEX )
check( xf.gp( xs, "LERadius" ) == 0.0, "Up Frac 1: LE Rad/C stays 0" )
P, f, xs = wing( dict( LE, LERadius = 0.5 ), shape = vsp.XS_WEDGE )
check( 0.024 < xf.gp( xs, "LERadius" ) <= 0.0251, "LE Rad/C is held to about the crest's height (%.4f)" % xf.gp( xs, "LERadius" ) )
P, f, xs = wing( dict( LE, LERadius = 0.005 ), shape = vsp.XS_WEDGE )
check( np.allclose( xf.at( P, f, 0.75 ), [ 0.5, 0.025 ], atol = 1e-3 ), "Rounded wedge: the upper corner keeps U Fwd Up 0.75" )

vsp.VSPRenew()
p = vsp.AddGeom( "PROP" )
s = vsp.GetXSecSurf( p, 0 )
pts0 = [ vsp.CompPnt01( p, 0, 0.9, k / 20.0 ) for k in range( 21 ) ]
for i in range( vsp.GetNumXSec( s ) ):
    xs = vsp.GetXSec( s, i )
    xf.sp( xs, "TE_Flap_Type", vsp.FLAP_PLAIN )
    xf.sp( xs, "TE_Flap_Deflection", 30.0 )
vsp.Update()
pts1 = [ vsp.CompPnt01( p, 0, 0.9, k / 20.0 ) for k in range( 21 ) ]
dmax = max( math.dist( ( a.x(), a.y(), a.z() ), ( b.x(), b.y(), b.z() ) ) for a, b in zip( pts0, pts1 ) )
check( dmax > 0.01, "Propeller: a 30 degree plain flap on every section moves the blade (%.3g)" % dmax )

vsp.VSPRenew()
w = vsp.AddGeom( "WING" )
s = vsp.GetXSecSurf( w, 0 )
xs = vsp.GetXSec( s, 0 )
xf.sp( xs, "Theta", 7.0 )
vsp.Update()
vsp.FitAfCST( s, 0, 7 )
vsp.Update()
xs = vsp.GetXSec( s, 0 )
check( abs( xf.gp( xs, "Theta" ) - 7.0 ) < 1e-6, "Fit CST keeps the section's Theta (7 before, %.3g after)" % xf.gp( xs, "Theta" ) )

print()
print( "%d failed" % len( FAILS ) )
