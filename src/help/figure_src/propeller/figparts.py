"""Shared pieces of the propeller help figures: illustration parms and small drawing helpers."""

import numpy as np

import propmodel as pm
import figlib as fl
from figlib import COL
import geom

X = np.array( [ 1.0, 0, 0 ] )

Y = np.array( [ 0, 1.0, 0 ] )

# Hinge stations spread out a little from the defaults (0.05, 0.10, 0.15, 0.20) so they can be
# told apart on paper; the order is the default order.  The blade root moves out to clear them.
ILLUS_HINGES = { "RFlapAx": 0.05, "RLagAx": 0.13, "RFeatherAx": 0.21, "RFoldAx": 0.29 }

ILLUS_ROOT = 0.36

# How far a radial axis line is drawn: a little past the tip.
AXIS_LEN = 1.1 * geom.RADIUS


def hub( c ):
    """The shaft line through the rotor center."""
    c.line( np.array( [ -3.2 * X, 3.0 * X ] ), color = "#999999", lw = 0.7, ls = ( 0, ( 6, 2, 1, 2 ) ), z = 20 )


CHAIN_DEFL = { "FoldAngle": 35.0, "Feather": 25.0, "LagAngle": 25.0, "FlapAngle": 15.0, "Precone": 8.0, "Rotate": 30.0 }

def illus_prop( extra = None, rev = False ):
    """The harmonic illustration rotor, checked against VSP."""
    p = dict( ILLUS_HINGES )
    p.update( CHAIN_DEFL )
    if extra:
        p.update( extra )
    prop = geom.Prop( p, rev = rev, root = ILLUS_ROOT, rotor = True )
    prop.check_pose()
    return prop


def hinge_draw_spec( prop ):
    """Where and how long to draw each hinge, in the zero-deflection pose."""
    hp = prop.hinges()
    spec = {}
    for k, ( pt, ax ) in hp.items():
        if k == "feather":
            # The bearing line is the whole radial axis; the point is already at its station e/R.
            spec[ k ] = dict( p = pt, axis = ax, half = 1.6 )
        else:
            spec[ k ] = dict( p = pt, axis = ax, half = 1.7 )
    return spec


LABEL_OFF = { "flap": ( -12, -4 ), "lag": ( 10, 2 ), "fold": ( 0, 9 ), "feather": ( 0, 9 ) }

def draw_hinges( c, spec, M = None, active = None, faint = False, labels = True, which = None ):
    """Each hinge of spec as its colored axis with a curl, moved by M."""
    if M is None:
        M = np.eye( 4 )
    for k in ( which or [ "flap", "lag", "feather", "fold" ] ):
        sp = spec[ k ]
        p = pm.apply( M, sp[ "p" ] )
        a = pm.apply_dir( M, sp[ "axis" ] )
        if active is not None and k != active:
            col = "#b5b5b5"
            lw = 0.9
        else:
            col = COL[ k ]
            lw = 1.6
        if k == "feather":
            # Draw the bearing line from the hub along the span.
            base = pm.apply( M, sp[ "p" ] - sp[ "p" ] @ fl.unit( sp[ "axis" ] ) * fl.unit( sp[ "axis" ] ) )
            c.line( np.array( [ base, base + AXIS_LEN * fl.unit( a ) ] ), color = col, lw = 0.6 * lw, ls = ( 0, ( 5, 2 ) ), z = 52 )
            c.hinge( p, a, col, half = 0.01, rho = 0.55, at = 0.0, lw = lw, arrow_on_axis = False,
                     label = ( k if labels else None ), label_dx = 0, label_dy = 9 )
        else:
            dx, dy = LABEL_OFF[ k ]
            c.hinge( p, a, col, half = sp[ "half" ], rho = 0.5, at = 0.8, lw = lw,
                     label = ( k if labels else None ), label_dx = dx, label_dy = dy )


SHAPE = { "Chord": 3.0, "Reverse": 1.0, "Construct": 0.25, "FeatherAxis": 0.4, "FeatherOffset": 0.2,
          "RootChord": 2.2, "RootTwist": 40.0, "Twist": 22.0, "ZRotate": -12.0, "Rake": 0.7, "Skew": 0.9,
          "Sweep": 10.0, "Axial": 0.6, "Tangential": -0.7, "Radius": 10.5 }

def sec_draw( c, P, ghost = False, color = None, z = 10 ):
    """A section outline, filled, or dashed as a ghost."""
    Q = c.v.p2( P )
    c.pts.append( Q )
    if ghost:
        c.ax.fill( Q[:, 0], Q[:, 1], facecolor = "none", edgecolor = COL[ "ghost" ], lw = 0.8, ls = ( 0, ( 3, 2 ) ), zorder = z )
    else:
        c.ax.fill( Q[:, 0], Q[:, 1], facecolor = color or COL[ "surf" ], edgecolor = "#333333", lw = 0.8, zorder = z + 1 )


def bdir( v ):
    """A direction given in blade axes, drawn in propeller axes (normal rotation)."""
    return pm.blade_to_prop( v, 1.0 )


MIRROR_PARMS = { "FoldAngle": 0.0, "Feather": 15.0, "LagAngle": 8.0, "FlapAngle": 10.0, "Precone": 4.0, "Rotate": 40.0,
                 "ElFlapDir": 25.0, "AzFlapDir": 10.0, "CantLagDir": 20.0, "OffFlapAx": 0.03, "FeatherOffsetXoC": 0.25 }

def azdir( a, rev = 1.0 ):
    """The radial direction at angle a (deg) from +Y in the direction of rotation."""
    return pm.apply_dir( pm.Rx( -rev * a ), Y )


AZ_PARMS = { "NumBlade": 3, "Rotate": 40.0, "PsiRef": 25.0, "Feather": 0.0 }

HARM = { "NumBlade": 4, "BladeMotionMode": pm.HARMONIC, "NumHarmonics": 1, "Rotate": 20.0,
         "FlapMean": 6.0, "FlapCos_1": 5.0, "FlapSin_1": -3.0, "RFlapAx": 0.05, "Feather": 0.0, "FeatherMean": 0.0 }

def tip_path( prop, n = 181 ):
    """Blade 1's tip, mid-chord, as Rotate runs once round."""
    out = []
    for r in np.linspace( 0.0, 360.0, n ):
        M = pm.product( prop.chain( 0, parms = { "Rotate": r } ) )
        out.append( pm.apply( M, prop.G0[ -1, prop.nw // 2 ] ) )
    return np.array( out )
