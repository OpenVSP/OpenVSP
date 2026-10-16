"""Checks, against the openvsp module, what XSecSkinning.md says about sections and skinning.

    <build>/venv/bin/python verify.py
"""

import os
import sys

import numpy as np
import openvsp as vsp

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "xseccurves" ) )

import xsfig as xf                                         # noqa: E402
from xsecskinning_figs import stack, side, pnt, SIDES      # noqa: E402

FAILS = []


def check( ok, what ):
    print( ( "PASS  " if ok else "FAIL  " ) + what )
    if not ok:
        FAILS.append( what )


def deriv( g, u, w, n = 3, h = 1e-5 ):
    """dP/du per section: one section spans one unit of u."""
    return ( pnt( g, u + h, w ) - pnt( g, u - h, w ) ) / ( 2 * h ) / ( n - 1 )


# Angle: positive leans the surface out, away from the axis, toward the later sections.
g, s = stack()
side( vsp.GetXSec( s, 1 ), "Top", { "Angle": 20.0, "Strength": 1.0 }, { "Angle": 20.0, "Strength": 1.0 } )
vsp.Update()
d = deriv( g, 0.5 + 1e-4, 0.75 )
check( abs( np.degrees( np.arctan2( d[2], d[0] ) ) - 20 ) < 0.05, "Top Angle +20: the surface leaves rising 20 degrees" )
d = deriv( g, 0.5 - 1e-4, 0.75 )
check( abs( np.degrees( np.arctan2( d[2], d[0] ) ) - 20 ) < 0.05, "Top left Angle +20: the surface arrives rising 20 degrees" )

# Strength: the derivative is Strength times the distance to the nearer neighbouring section.
check( abs( np.linalg.norm( deriv( g, 0.5 + 1e-4, 0.75 ) ) - 2.0 ) < 1e-3, "Strength 1 with sections 2 apart: |dP/du| = 2" )
g, s = stack()
x2 = vsp.GetXSec( s, 2 )
xf.sp( x2, "Circle_Diameter", 5.0 )
xf.sp( x2, "XDelta", 4.0 )
vsp.Update()
side( vsp.GetXSec( s, 1 ), "Top", { "Angle": 0.0, "Strength": 1.0 }, { "Angle": 0.0, "Strength": 1.0 } )
vsp.Update()
check( abs( np.linalg.norm( deriv( g, 0.5 + 1e-4, 0.75 ) ) - 2.0 ) < 1e-3, "The nearer neighbour sets the scale (2, not 4.27)" )
g, s = stack()
x0 = vsp.GetXSec( s, 0 )
xf.sp( x0, "Circle_Diameter", 5.0 )
xf.sp( vsp.GetXSec( s, 2 ), "XDelta", 4.0 )
side( vsp.GetXSec( s, 1 ), "Top", { "Angle": 0.0, "Strength": 1.0 }, { "Angle": 0.0, "Strength": 1.0 } )
vsp.Update()
want = np.sqrt( 2.0 ** 2 + ( ( 5.0 - 2.0 ) / 2 ) ** 2 )
check( abs( np.linalg.norm( deriv( g, 0.5 + 1e-4, 0.75 ) ) - want ) < 1e-3,
       "Half the change in size counts in the distance (%.4f)" % want )

# Slew: Top and Bottom swing toward Right; Right and Left toward Bottom.
for sd, w, axis, sgn in ( ( "Top", 0.75, 1, 1 ), ( "Bottom", 0.25, 1, 1 ), ( "Right", 0.0, 2, -1 ), ( "Left", 0.5, 2, -1 ) ):
    g, s = stack()
    side( vsp.GetXSec( s, 1 ), sd, { "Angle": 0.0, "Strength": 1.0, "Slew": 20.0 },
          { "Angle": 0.0, "Strength": 1.0, "Slew": 20.0 } )
    vsp.Update()
    d = deriv( g, 0.5 + 1e-4, w )
    check( sgn * d[ axis ] > 0.5, "%s Slew +20 swings the surface toward %s" % ( sd, "+Y (Right)" if axis == 1 else "-Z (Bottom)" ) )

# Curvature: positive bends the surface away from the axis on both sides of the section.
g, s = stack()
side( vsp.GetXSec( s, 1 ), "Top", { "Angle": 0.0, "Strength": 1.0, "Curve": 1.0 }, { "Angle": 0.0, "Strength": 1.0, "Curve": 1.0 } )
vsp.Update()
check( pnt( g, 0.4, 0.75 )[2] > 1.0 and pnt( g, 0.6, 0.75 )[2] > 1.0, "Curvature +1: the surface curves away from the axis" )

# First section: its left values set the surface leaving it.
g, s = stack()
side( vsp.GetXSec( s, 0 ), "Top", { "Angle": 30.0, "Strength": 1.0 } )
vsp.Update()
d = deriv( g, 1e-4, 0.75 )
check( abs( np.degrees( np.arctan2( d[2], d[0] ) ) - 30 ) < 0.1, "First section: the left column sets the surface leaving it" )

# Continuity C1 makes the right side follow the left.
g, s = stack()
xs = vsp.GetXSec( s, 1 )
side( xs, "Top", { "Angle": 10.0, "Strength": 1.0 }, { "Angle": -10.0, "Strength": 1.0 } )
xf.sp( xs, "ContinuityTop", 1 )
vsp.Update()
check( abs( xf.gp( xs, "TopRAngle" ) - 10.0 ) < 1e-9 and xf.gp( xs, "TopLRAngleEq" ) == 1.0, "C1: right Angle takes the left's" )
check( xf.gp( xs, "ContinuityRight" ) == 1.0, "Continuity is one setting for the whole section" )

# Nothing enforced at C0: straight lines between sections.
g, s = stack()
x2 = vsp.GetXSec( s, 2 )
xf.sp( x2, "Circle_Diameter", 4.0 )
vsp.Update()
Q = np.array( [ pnt( g, u, 0.75 ) for u in np.linspace( 0.5, 1.0, 11 ) ] )
lin = Q[0] + np.outer( np.linspace( 0, 1, 11 ), Q[-1] - Q[0] )
check( np.max( np.abs( Q - lin ) ) < 1e-9, "Nothing enforced, C0: the surface runs straight from section to section" )

# Each condition at an end of a span raises the degree of the line across it by one.
def degree( g, u0, u1, w = 0.75 ):
    us = np.linspace( u0, u1, 41 )
    P = np.array( [ pnt( g, u, w ) for u in us ] )
    t = ( us - u0 ) / ( u1 - u0 )
    for k in range( 8 ):
        r = max( np.max( np.abs( np.polyval( np.polyfit( t, P[:, j], k ), t ) - P[:, j] ) ) for j in ( 0, 2 ) )
        if r < 1e-9:
            return k
    return -1


A0 = { "Angle": 0.0, "Strength": 1.0 }
AC = { "Angle": 0.0, "Strength": 1.0, "Curve": 1.0 }
for r1, l2, want, what in ( ( None, None, 1, "nothing" ), ( A0, None, 2, "an angle at one end" ),
                            ( A0, A0, 3, "angles at both ends" ), ( AC, AC, 5, "angle and curvature at both ends" ) ):
    g, s = stack()
    xf.sp( vsp.GetXSec( s, 2 ), "Circle_Diameter", 3.0 )
    side( vsp.GetXSec( s, 1 ), "Top", None, r1 )
    side( vsp.GetXSec( s, 2 ), "Top", l2, None )
    vsp.Update()
    got = degree( g, 0.5, 1.0 )
    check( got == want, "Span degree with %s: %d (want %d)" % ( what, got, want ) )

# C1 with nothing set bends the span with fewer conditions; on a tie, the one before the section.
for d0, d2, what in ( ( 2.0, 3.0, "last section larger" ), ( 3.0, 2.0, "first section larger" ) ):
    g, s = stack()
    xf.sp( vsp.GetXSec( s, 0 ), "Circle_Diameter", d0 )
    xf.sp( vsp.GetXSec( s, 2 ), "Circle_Diameter", d2 )
    xf.sp( vsp.GetXSec( s, 1 ), "ContinuityTop", 1 )
    vsp.Update()
    check( degree( g, 0.0, 0.5 ) == 2 and degree( g, 0.5, 1.0 ) == 1,
           "C1, nothing set, %s: the span before bends, the span after stays straight" % what )
g, s = stack()
xf.sp( vsp.GetXSec( s, 2 ), "Circle_Diameter", 3.0 )
side( vsp.GetXSec( s, 0 ), "Top", A0 )
xf.sp( vsp.GetXSec( s, 1 ), "ContinuityTop", 1 )
vsp.Update()
check( abs( pnt( g, 0.25, 0.75 )[2] - 1.0 ) < 1e-9 and degree( g, 0.5, 1.0 ) == 2,
       "C1, angle set at the first section only: the span after, with fewer conditions, bends" )
g, s = stack()
xf.sp( vsp.GetXSec( s, 2 ), "Circle_Diameter", 3.0 )
xf.sp( vsp.GetXSec( s, 1 ), "ContinuityTop", 1 )
vsp.Update()
check( pnt( g, 0.25, 0.75 )[2] < 1.0 - 0.05, "C1, nothing set: the span before sags below two equal sections" )
xf.sp( vsp.GetXSec( s, 1 ), "ContinuityTop", 2 )
vsp.Update()
check( degree( g, 0.0, 0.5 ) > 1 and degree( g, 0.5, 1.0 ) > 1, "C2, nothing set: both spans bend" )

# U: one equal share per span, however long the span.
g, s = stack()
xf.sp( vsp.GetXSec( s, 1 ), "XDelta", 0.5 )
vsp.Update()
check( abs( pnt( g, 0.5, 0.75 )[0] - 0.5 ) < 1e-12,
       "U = 0.5 is on the middle section, though it is 0.5 from the first and 2 from the last" )

# Each station's Set is its own.
g, s = stack()
xs = vsp.GetXSec( s, 1 )
side( xs, "Top", A0, A0 )
vsp.Update()
check( xf.gp( xs, "TopLAngleSet" ) == 1.0 and xf.gp( xs, "RightLAngleSet" ) == 0.0,
       "The Top angle can be set with the Right left free" )

# New Fuselage and Stack defaults.
for kind in ( "FUSELAGE", "STACK" ):
    vsp.VSPRenew()
    g = vsp.AddGeom( kind )
    s = vsp.GetXSecSurf( g, 0 )
    vsp.Update()
    n = vsp.GetNumXSec( s )
    ok = True
    for i in range( n ):
        xs = vsp.GetXSec( s, i )
        ok = ok and xf.gp( xs, "ContinuityTop" ) == 0 and xf.gp( xs, "TBSym" ) == 1 and xf.gp( xs, "RLSym" ) == 1
        ok = ok and xf.gp( xs, "TopLCurveSet" ) == 0 and xf.gp( xs, "TopLAngleSet" ) == 1
        if 0 < i < n - 1:
            ok = ok and xf.gp( xs, "TopLAngle" ) == 0 and xf.gp( xs, "TopLRAngleEq" ) == 1
            ok = ok and xf.gp( xs, "TopLStrength" ) == 1 and xf.gp( xs, "TopRStrength" ) == 1
            ok = ok and xf.gp( xs, "TopLRStrengthEq" ) == 0
    ok = ok and vsp.GetXSecShape( vsp.GetXSec( s, 0 ) ) == vsp.XS_POINT
    ok = ok and xf.gp( vsp.GetXSec( s, 0 ), "TopLAngle" ) == 45 and xf.gp( vsp.GetXSec( s, n - 1 ), "TopLAngle" ) == -45
    check( ok, "New %s: T/B and R/L Sym, C0, no curvature; Angle 0 with = and Strength 1 each side between Point "
               "ends at 45 and -45" % kind )

# Unset values read back what the loft did.
g, s = stack()
xs = vsp.GetXSec( s, 1 )
x2 = vsp.GetXSec( s, 2 )
xf.sp( x2, "Circle_Diameter", 4.0 )
vsp.Update()
check( abs( xf.gp( xs, "TopRAngle" ) - np.degrees( np.arctan2( 1.0, 2.0 ) ) ) < 0.05,
       "An unset Angle shows the angle the loft took (%.3f)" % xf.gp( xs, "TopRAngle" ) )

# Spines: position shared, mirrors.
g, s = stack()
vsp.AddSkinSpine( g, 0.1 )
vsp.Update()
ids = [ vsp.GetSkinSpineID( vsp.GetXSec( s, i ), 0 ) for i in range( 3 ) ]
vsp.SetParmVal( vsp.GetSkinSpineParm( ids[0], "W01" ), 0.15 )
vsp.Update()
ws = [ vsp.GetParmVal( vsp.GetSkinSpineParm( vsp.GetSkinSpineID( vsp.GetXSec( s, i ), 0 ), "W01" ) ) for i in range( 3 ) ]
check( np.allclose( ws, 0.15 ), "A spine sits at the same W on every section (%s)" % ws )
check( vsp.GetNumSkinSpines( g ) == 1, "Adding a spine adds it to every section" )

# Sections: insert, paste, cut.
vsp.VSPRenew()
f = vsp.AddGeom( "FUSELAGE" )
s = vsp.GetXSecSurf( f, 0 )
xf.sp( vsp.GetXSec( s, 1 ), "ZLocPercent", 0.05 )
vsp.InsertXSec( f, 1, vsp.XS_SUPER_ELLIPSE )
vsp.Update()
x = vsp.GetXSec( s, 2 )
check( abs( xf.gp( x, "XLocPercent" ) - 0.375 ) < 1e-9 and abs( xf.gp( x, "ZLocPercent" ) - 0.05 ) < 1e-9,
       "Fuselage Insert: a copy of the section, halfway to the next in X" )
vsp.CopyXSec( f, 1 )
vsp.PasteXSec( f, 4 )
vsp.Update()
x = vsp.GetXSec( s, 4 )
check( abs( xf.gp( x, "XLocPercent" ) - 0.75 ) < 1e-9 and abs( xf.gp( x, "ZLocPercent" ) ) < 1e-9,
       "Paste: the shape and skinning come over, the position stays" )
vsp.VSPRenew()
st = vsp.AddGeom( "STACK" )
s = vsp.GetXSecSurf( st, 0 )
for k in range( 6 ):
    vsp.CutXSec( st, 0 )
check( vsp.GetNumXSec( s ) == 2, "Cut stops at two sections" )

print()
print( "%d failed" % len( FAILS ) )
