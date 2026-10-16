"""A gear's lower strut turns about the knee axis fixed to its upper strut.

The gear here has its trunnion straight above the tire and its retraction axis along Y, so the
unbent strut runs along Z and its frame is u = Z, n = X, v = Y.  The axis the tire turns about is
measured from the tire surface: every point moves at right angles to it.
"""

import openvsp as vsp


def knee_gear( azimuth, elevation ):
    vsp.VSPRenew()
    gid = vsp.AddGeom( "GEAR" )
    vsp.SetParmVal( gid, "ShowNominalGroundPlane", "GroundPlane", 0.0 )
    vsp.SetParmVal( gid, "GearConfigMode", "Gear", vsp.GEAR_CONFIGURATION_INTERMEDIATE )
    b = vsp.CreateAndAddBogie( gid )
    for n, g, v in ( ( "Symmetrical", "Bogie", 0 ), ( "YContactPt", "Bogie", 3.0 ),
                     ( "RetMode", "Retract", vsp.GEAR_MECHANISM ), ( "MechAbsRelFlag", "Retract", vsp.REL ),
                     ( "MechZRelLoc", "Retract", 4.0 ), ( "Mech_RetAngle", "Retract", 0.0 ),
                     ( "MechKneePos", "Retract", 0.5 ), ( "MechKneeDownAngle", "Retract", 120.0 ),
                     ( "MechKneeAzimuthAngle", "Retract", azimuth ),
                     ( "MechKneeElevationAngle", "Retract", elevation ), ( "MechKneeAngle", "Retract", 120.0 ) ):
        vsp.SetParmVal( b, n, g, v )
    vsp.Update()
    return gid, b


def turn_axis( gid, b ):
    def tire( k ):
        vsp.SetParmVal( b, "MechKRetract", "Retract", k )
        vsp.Update()
        s = vsp.GetNumMainSurfs( gid ) - 1
        return [ vsp.CompPnt01( gid, s, u, w ) for u in ( 0.0, 0.3, 0.7 ) for w in ( 0.1, 0.4, 0.8 ) ]
    d = [ q - p for p, q in zip( tire( 0.0 ), tire( 0.5 ) ) ]
    best = max( ( vsp.cross( d[ i ], d[ j ] ) for i in range( len( d ) ) for j in range( i + 1, len( d ) ) ),
                key = lambda c: c.mag() )
    best.normalize()
    return best


def parallel( a, b ):
    return abs( abs( vsp.dot( a, b ) ) - 1.0 ) < 1e-9


def test_the_knee_axis_turns_with_the_upper_strut():
    # Azimuth 90 puts the unbent knee axis along X, at right angles to the retraction axis; the knee
    # bend turns it about Y, out of the X axis.
    gid, b = knee_gear( 90.0, 0.0 )
    ax = turn_axis( gid, b )
    assert abs( ax.y() ) < 1e-9
    assert abs( ax.z() ) > 0.1
