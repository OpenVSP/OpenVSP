"""Hinges at the hub, r/R 0.  A flap hinge at the hub with first-harmonic flap makes a two-bladed
rotor teeter: both blades move as one rigid body.  With more blades it stands in for a gimbal,
to second order in the tilt, when the feather follows the flapping-feathering equivalence
theta_1c = beta_1s, theta_1s = -beta_1c.
"""

import itertools
import math

import openvsp as vsp

from prophelp import group

UW = [ ( u, w ) for u in ( 0.05, 0.5, 1.0 ) for w in ( 0.0, 0.25, 0.5, 0.75 ) ]
HUB = { "RFlapAx": 0.0, "RFeatherAx": 0.0, "BladeMotionMode": vsp.PROP_BLADE_HARMONIC }


def new_prop( nblade, parms, rev = False ):
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( pid, "UseBeta34Flag", "Design", 0.0 )
    vsp.SetParmVal( pid, "ReverseFlag", "Design", float( rev ) )
    vsp.SetParmVal( pid, "NumBlade", "Design", nblade )
    vsp.Update()
    for name, val in parms.items():
        vsp.SetParmVal( pid, name, group( name ), val )
    vsp.Update()
    return pid


def rotor_points( nblade, parms, rev = False ):
    """Points on every blade, and the hub."""
    pid = new_prop( nblade, parms, rev )
    pts = [ ( 0.0, 0.0, 0.0 ) ]
    for i in range( nblade ):
        for u, w in UW:
            p = vsp.CompPnt01( pid, i, u, w )
            pts.append( ( p.x(), p.y(), p.z() ) )
    return pts


def distortion( a, b ):
    """The largest change in distance between any two points: zero for a rigid motion."""
    return max( abs( math.dist( a[i], a[j] ) - math.dist( b[i], b[j] ) )
                for i, j in itertools.combinations( range( len( a ) ), 2 ) )


def moved( a, b ):
    return max( math.dist( p, q ) for p, q in zip( a, b ) )


def test_two_blades_teeter_as_one_body():
    for rev in ( False, True ):
        for rot in ( 0.0, 37.0, 125.0 ):
            still = rotor_points( 2, dict( HUB, Rotate = rot ), rev )
            teeter = rotor_points( 2, dict( HUB, Rotate = rot, FlapCos_1 = 6.0, FlapSin_1 = -4.0 ), rev )
            assert moved( still, teeter ) > 0.5, ( rev, rot )
            assert distortion( still, teeter ) < 1e-9, ( rev, rot )


def test_hub_flap_and_feather_stand_in_for_a_gimbal():
    for nblade in ( 3, 4 ):
        still = rotor_points( nblade, dict( HUB, Rotate = 20.0 ) )
        for flap, feather in ( ( "FlapCos_1", "FeatherSin_1" ), ( "FlapSin_1", "FeatherCos_1" ) ):
            sign = -1.0 if flap == "FlapCos_1" else 1.0
            err = {}
            for deg in ( 1.0, 2.0 ):
                alone = rotor_points( nblade, dict( HUB, Rotate = 20.0, **{ flap: deg } ) )
                paired = rotor_points( nblade, dict( HUB, Rotate = 20.0, **{ flap: deg, feather: sign * deg } ) )
                err[ deg ] = distortion( still, paired )
                # Flap alone is off at first order; with the feather paired the error is second order.
                assert err[ deg ] < 0.25 * distortion( still, alone ), ( nblade, flap, deg )
            assert 3.5 < err[ 2.0 ] / err[ 1.0 ] < 4.5, ( nblade, flap, err )
