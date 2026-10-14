"""Harmonic blade motion.  In harmonic mode a hinge's angle on each blade is a mean plus harmonics
of the blade's azimuth psi, measured from PsiRef in the direction of rotation:

    angle = mean + sum over n of ( cos_n cos( n psi ) + sin_n sin( n psi ) )

Blade 1's angle parm and the other blades' own parms show what each blade takes.
"""

import math
import os
import tempfile

import openvsp as vsp

from prophelp import group

UW = [ ( u, w ) for u in ( 0.05, 0.3, 0.6, 0.95 ) for w in ( 0.0, 0.2, 0.5, 0.7, 0.9 ) ]
TOL = 1e-9

# Blade 1's angle parm and the prefix of the other blades' own parms, for each hinge.
ANGLE = { "Flap": ( "FlapAngle", "FlapAngle_" ), "Lag": ( "LagAngle", "LagAngle_" ), "Feather": ( "Feather", "Feather_" ) }

# Two harmonics on each hinge, small enough to keep the blades apart.  Coefficients not given are zero.
MOTION = {
    "BladeMotionMode": vsp.PROP_BLADE_HARMONIC, "NumHarmonics": 2,
    "FlapMean": 3.0, "FlapCos_1": 2.0, "FlapSin_1": -1.5, "FlapCos_2": 0.5, "FlapSin_2": 0.25,
    "LagMean": -2.0, "LagCos_1": 1.0, "LagSin_1": 2.5, "LagCos_2": -0.5,
    "FeatherMean": 12.0, "FeatherCos_1": -3.0, "FeatherSin_1": 4.0, "FeatherSin_2": 0.75,
}


def new_prop( nblade, parms, rev = False ):
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( pid, "UseBeta34Flag", "Design", 0.0 )
    vsp.SetParmVal( pid, "ReverseFlag", "Design", float( rev ) )
    vsp.SetParmVal( pid, "NumBlade", "Design", nblade )
    vsp.Update()
    set_parms( pid, parms )
    return pid


def set_parms( pid, parms ):
    # A harmonic count makes its coefficient parms in an Update, so counts go first.
    for first in ( True, False ):
        for name, val in parms.items():
            if name.startswith( "Num" ) == first:
                assert vsp.FindParm( pid, name, group( name ) ), name
                vsp.SetParmVal( pid, name, group( name ), val )
        vsp.Update()


def blade( pid, surf ):
    pts = []
    for u, w in UW:
        p = vsp.CompPnt01( pid, surf, u, w )
        pts.append( ( p.x(), p.y(), p.z() ) )
    return pts


def furthest( a, b ):
    return max( math.dist( p, q ) for p, q in zip( a, b ) )


def blades( pid, nblade ):
    return [ blade( pid, s ) for s in range( nblade ) ]


def blades_error( a, b ):
    return max( furthest( p, q ) for p, q in zip( a, b ) )


def harmonic( parms, hinge, psi ):
    val = parms[ hinge + "Mean" ]
    for n in range( 1, parms[ "NumHarmonics" ] + 1 ):
        p = math.radians( n * psi )
        val += parms.get( "%sCos_%d" % ( hinge, n ), 0.0 ) * math.cos( p ) + parms.get( "%sSin_%d" % ( hinge, n ), 0.0 ) * math.sin( p )
    return val


def blade_angles( pid, hinge, nblade ):
    master, own = ANGLE[ hinge ]
    vals = [ vsp.GetParmVal( pid, master, group( master ) ) ]
    vals += [ vsp.GetParmVal( pid, own + str( i ), group( own ) ) for i in range( 1, nblade ) ]
    return vals


def names( pid ):
    return set( vsp.GetParmName( p ) for p in vsp.GetGeomParmIDs( pid ) )


def test_angles_follow_the_harmonics():
    parms = dict( MOTION, Rotate = 25.0, PsiRef = 10.0 )
    pid = new_prop( 4, parms )
    for hinge in ( "Flap", "Lag", "Feather" ):
        for i, got in enumerate( blade_angles( pid, hinge, 4 ) ):
            assert abs( got - harmonic( parms, hinge, 25.0 + 90.0 * i ) ) < 1e-12, ( hinge, i )


def test_uneven_blades_take_their_own_azimuth():
    parms = dict( MOTION, BladeAzimuthMode = vsp.PROP_AZI_FREE, BladeAz_2 = 100.0, BladeAz_3 = 250.0 )
    pid = new_prop( 3, parms )
    for i, az in enumerate( ( 0.0, 100.0, 250.0 ) ):
        assert abs( blade_angles( pid, "Flap", 3 )[i] - harmonic( parms, "Flap", az ) ) < 1e-12, i


def test_blades_placed_as_their_own_angles():
    pid = new_prop( 3, dict( MOTION, Rotate = 40.0 ) )
    want = blades( pid, 3 )
    own = { "BladeMotionMode": vsp.PROP_BLADE_INDIVIDUAL, "Rotate": 40.0 }
    for hinge in ( "Flap", "Lag", "Feather" ):
        master, prefix = ANGLE[ hinge ]
        vals = blade_angles( pid, hinge, 3 )
        own[ master ] = vals[0]
        for i in range( 1, 3 ):
            own[ prefix + str( i ) ] = vals[i]
    assert blades_error( blades( new_prop( 3, own ), 3 ), want ) < TOL


def test_no_harmonics_is_collective():
    parms = { "BladeMotionMode": vsp.PROP_BLADE_HARMONIC, "FlapMean": 7.0, "NumHarmonics": 0 }
    got = blades( new_prop( 3, parms ), 3 )
    want = blades( new_prop( 3, { "FlapAngle": 7.0 } ), 3 )
    assert blades_error( got, want ) == 0.0


def test_blade_angle_depends_only_on_its_azimuth():
    # Turning the rotor carries each blade to where the next one was; it takes that blade's
    # angles, so the tip path stays put.
    a = blades( new_prop( 3, dict( MOTION, Rotate = 10.0 ) ), 3 )
    b = blades( new_prop( 3, dict( MOTION, Rotate = 130.0 ) ), 3 )
    for i in range( 3 ):
        assert furthest( b[i], a[ ( i + 1 ) % 3 ] ) < TOL, i


def turn( pts, deg ):
    """Turn points about the propeller axis the way Rotate does."""
    c = math.cos( math.radians( deg ) )
    s = math.sin( math.radians( deg ) )
    return [ ( x, c * y + s * z, -s * y + c * z ) for x, y, z in pts ]


def test_psiref_turns_the_rotor():
    # PsiRef adds to Rotate where the blades sit but not to psi, so it turns the whole rotor, the
    # harmonic pattern with it, and leaves each blade's angles alone.
    for rev in ( False, True ):
        sgn = -1.0 if rev else 1.0
        base = new_prop( 3, dict( MOTION, Rotate = 25.0 ), rev )
        want = [ turn( b, sgn * 35.0 ) for b in blades( base, 3 ) ]
        angles = blade_angles( base, "Flap", 3 )
        pid = new_prop( 3, dict( MOTION, Rotate = 25.0, PsiRef = 35.0 ), rev )
        assert blade_angles( pid, "Flap", 3 ) == angles
        assert blades_error( blades( pid, 3 ), want ) < TOL, rev


def test_psiref_places_blade_one():
    # With Rotate zero, PsiRef alone says where blade 1 points.
    a = blades( new_prop( 3, { "PsiRef": 40.0 } ), 3 )
    b = blades( new_prop( 3, { "Rotate": 40.0 } ), 3 )
    assert blades_error( a, b ) < TOL


def mirror_error( a, b ):
    """Farthest point of a from the mirror image of b, in the plane z = 0."""
    mb = [ ( x, y, -z ) for x, y, z in b ]
    return max( min( math.dist( p, q ) for q in mb ) for p in a )


def test_reverse_rotation_mirrors():
    parms = dict( MOTION, Rotate = 30.0, PsiRef = 20.0 )
    a = blades( new_prop( 3, parms ), 3 )
    b = blades( new_prop( 3, parms, True ), 3 )
    assert max( mirror_error( a[i], b[i] ) for i in range( 3 ) ) < TOL


def test_harmonic_count_makes_and_removes_parms():
    pid = new_prop( 2, { "BladeMotionMode": vsp.PROP_BLADE_HARMONIC, "NumHarmonics": 3 } )
    assert { "FlapCos_3", "FlapSin_3", "LagCos_3", "FeatherSin_3" } <= names( pid )
    set_parms( pid, { "NumHarmonics": 1 } )
    assert "FlapCos_1" in names( pid )
    assert not { "FlapCos_2", "FlapSin_2", "FlapCos_3", "FlapSin_3" } & names( pid )


def test_round_trip():
    pid = new_prop( 3, dict( MOTION, Rotate = 15.0, PsiRef = 5.0 ) )
    want = blades( pid, 3 )
    path = os.path.join( tempfile.mkdtemp(), "harmonic.vsp3" )
    vsp.WriteVSPFile( path )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()
    pid = vsp.FindGeoms()[0]
    assert vsp.GetParmVal( pid, "FlapCos_2", "Articulation" ) == MOTION[ "FlapCos_2" ]
    assert blades_error( blades( pid, 3 ), want ) < TOL


def test_individual_starts_from_the_harmonics():
    pid = new_prop( 3, MOTION )
    want = blades( pid, 3 )
    set_parms( pid, { "BladeMotionMode": vsp.PROP_BLADE_INDIVIDUAL } )
    assert blades_error( blades( pid, 3 ), want ) < TOL


def test_beta34_drives_every_blade():
    # While Beta34 drives every blade takes its feather; the flap and lag keep the harmonic mode.
    pid = new_prop( 3, dict( MOTION, UseBeta34Flag = 1.0, Beta34 = 25.0 ) )
    assert vsp.GetParmVal( pid, "BladeMotionMode", "Articulation" ) == vsp.PROP_BLADE_HARMONIC
    feathers = blade_angles( pid, "Feather", 3 )
    flaps = blade_angles( pid, "Flap", 3 )
    collective = new_prop( 3, { "UseBeta34Flag": 1.0, "Beta34": 25.0 } )
    assert feathers == [ vsp.GetParmVal( collective, "Feather", "Design" ) ] * 3
    assert abs( flaps[1] - harmonic( MOTION, "Flap", 120.0 ) ) < 1e-12


def test_harmonic_feather_gives_beta34():
    pid = new_prop( 3, { "Feather": 0.0 } )
    twist34 = vsp.GetParmVal( pid, "Beta34", "Design" )
    set_parms( pid, dict( MOTION, Rotate = 50.0 ) )
    feather = vsp.GetParmVal( pid, "Feather", "Design" )
    assert abs( feather - harmonic( MOTION, "Feather", 50.0 ) ) < 1e-12
    assert abs( vsp.GetParmVal( pid, "Beta34", "Design" ) - ( feather + twist34 ) ) < 1e-12


def test_harmonic_mode_has_no_active_blade():
    # Harmonic motion gives every blade its angles, so the blade picked for its own goes back to 1.
    pid = new_prop( 3, { "BladeMotionMode": vsp.PROP_BLADE_INDIVIDUAL } )
    vsp.SetParmVal( pid, "ActiveBlade", "Index", 3 )
    set_parms( pid, { "BladeMotionMode": vsp.PROP_BLADE_HARMONIC } )
    assert vsp.GetParmVal( pid, "ActiveBlade", "Index" ) == 1


def test_advanced_link_names_the_mode():
    # The mode names are registered for AngelScript, which advanced links run.
    pid = new_prop( 3, {} )
    vsp.DelAllAdvLinks()
    vsp.AddAdvLink( "Mode" )
    indx = vsp.GetLinkIndex( "Mode" )
    vsp.AddAdvLinkInput( indx, vsp.FindParm( pid, "Diameter", "Design" ), "d" )
    vsp.AddAdvLinkOutput( indx, vsp.FindParm( pid, "BladeMotionMode", "Articulation" ), "mode" )
    vsp.SetAdvLinkCode( indx, "mode = PROP_BLADE_HARMONIC + 0.0 * d;" )
    assert vsp.BuildAdvLinkScript( indx )
    vsp.SetParmValUpdate( vsp.FindParm( pid, "Diameter", "Design" ), 6.0 )
    vsp.Update()
    assert vsp.GetParmVal( pid, "BladeMotionMode", "Articulation" ) == vsp.PROP_BLADE_HARMONIC
    vsp.DelAllAdvLinks()


def test_large_harmonic_angles_are_not_clamped():
    # A harmonic sum can pass any one turn; the angles it sets show it, and Individual starts from it.
    big = { "BladeMotionMode": vsp.PROP_BLADE_HARMONIC, "NumHarmonics": 1, "FlapMean": 150.0, "FlapCos_1": 120.0,
            "LagMean": -150.0, "LagCos_1": -120.0, "FeatherMean": 380.0, "FeatherCos_1": 150.0 }
    pid = new_prop( 3, big )
    assert vsp.GetParmVal( pid, "FlapAngle", "Articulation" ) == 270.0
    assert vsp.GetParmVal( pid, "LagAngle", "Articulation" ) == -270.0
    assert vsp.GetParmVal( pid, "Feather", "Design" ) == 530.0
    want = blades( pid, 3 )
    set_parms( pid, { "BladeMotionMode": vsp.PROP_BLADE_INDIVIDUAL } )
    assert blades_error( blades( pid, 3 ), want ) < TOL


def clone_of( pid ):
    clone = vsp.CloneGeomVec( [ pid ] )[0]
    vsp.Update()
    return clone


def test_clone_takes_the_original_rotation():
    pid = new_prop( 3, MOTION )
    clone = clone_of( pid )
    set_parms( pid, { "Rotate": 70.0 } )
    assert vsp.GetParmVal( clone, "Rotate", "Rotor" ) == 70.0
    assert blades_error( blades( clone, 3 ), blades( pid, 3 ) ) == 0.0


def test_clone_turned_apart():
    # A Clone with its own rotation places the original's blades as the original would at that
    # angle, harmonic motion and all, and leaves the original alone.
    pid = new_prop( 3, dict( MOTION, Rotate = 10.0 ) )
    original = blades( pid, 3 )
    clone = clone_of( pid )
    vsp.SetParmVal( clone, "CloneRotate", "Behavior", 0.0 )
    vsp.SetParmVal( clone, "Rotate", "Rotor", 55.0 )
    vsp.Update()
    got = blades( clone, 3 )
    assert blades_error( blades( pid, 3 ), original ) == 0.0
    set_parms( pid, { "Rotate": 55.0 } )
    assert blades_error( got, blades( pid, 3 ) ) == 0.0


def test_clone_turned_apart_follows_the_original_shape():
    pid = new_prop( 3, MOTION )
    clone = clone_of( pid )
    vsp.SetParmVal( clone, "CloneRotate", "Behavior", 0.0 )
    vsp.SetParmVal( clone, "Rotate", "Rotor", 25.0 )
    vsp.Update()
    set_parms( pid, { "FlapCos_1": 6.0, "Diameter": 9.0 } )
    got = blades( clone, 3 )
    set_parms( pid, { "Rotate": 25.0 } )
    assert blades_error( got, blades( pid, 3 ) ) == 0.0


def test_clone_of_clone_turned_apart():
    pid = new_prop( 3, MOTION )
    inner = clone_of( pid )
    vsp.SetParmVal( inner, "CloneRotate", "Behavior", 0.0 )
    vsp.SetParmVal( inner, "Rotate", "Rotor", 40.0 )
    outer = clone_of( inner )
    assert blades_error( blades( outer, 3 ), blades( inner, 3 ) ) == 0.0
    vsp.SetParmVal( outer, "CloneRotate", "Behavior", 0.0 )
    vsp.SetParmVal( outer, "Rotate", "Rotor", 80.0 )
    vsp.Update()
    got = blades( outer, 3 )
    set_parms( pid, { "Rotate": 80.0 } )
    assert blades_error( got, blades( pid, 3 ) ) == 0.0


def test_clone_rotation_round_trip():
    pid = new_prop( 3, MOTION )
    clone = clone_of( pid )
    vsp.SetParmVal( clone, "CloneRotate", "Behavior", 0.0 )
    vsp.SetParmVal( clone, "Rotate", "Rotor", 33.0 )
    vsp.Update()
    want = blades( clone, 3 )
    path = os.path.join( tempfile.mkdtemp(), "clone.vsp3" )
    vsp.WriteVSPFile( path )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()
    clone = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Clone" ][0]
    assert vsp.GetParmVal( clone, "Rotate", "Rotor" ) == 33.0
    assert blades_error( blades( clone, 3 ), want ) < TOL


def bbox( gid ):
    # The Geom's own box, as the model's box for a set holding only it.
    vsp.SetSetFlag( gid, vsp.SET_FIRST_USER, True )
    ok, x0, y0, z0, dx, dy, dz = vsp.GetBBoxSet( vsp.SET_FIRST_USER )
    vsp.SetSetFlag( gid, vsp.SET_FIRST_USER, False )
    assert ok
    return ( x0, y0, z0, x0 + dx, y0 + dy, z0 + dz )


def test_clone_turned_apart_has_its_own_box():
    # Two blades, so the box swings with Rotate.
    pid = new_prop( 2, MOTION )
    clone = clone_of( pid )
    vsp.SetParmVal( clone, "CloneRotate", "Behavior", 0.0 )
    vsp.SetParmVal( clone, "Rotate", "Rotor", 90.0 )
    vsp.Update()
    got = bbox( clone )
    set_parms( pid, { "Rotate": 90.0 } )
    want = bbox( pid )
    assert max( abs( a - b ) for a, b in zip( got, want ) ) < TOL


def test_replaced_clone_keeps_its_rotation():
    pid = new_prop( 3, MOTION )
    clone = clone_of( pid )
    vsp.SetParmVal( clone, "CloneRotate", "Behavior", 0.0 )
    vsp.SetParmVal( clone, "Rotate", "Rotor", 55.0 )
    vsp.Update()
    want = blades( clone, 3 )
    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()
    assert vsp.GetParmVal( real, "Rotate", "Design" ) == 55.0
    assert blades_error( blades( real, 3 ), want ) < TOL
