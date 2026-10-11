"""A bogie's retraction can be given as a stowed position or as a mechanism, and switching
between the two must not move the stowed gear.

In mechanism mode every Update writes the stowed position the mechanism reaches.  In stowed
mode every Update writes a single hinge -- axis, angle and trunnion -- that carries the bogie's
axle from deployed to stowed, and a bogie angle for the turn about the axle.  The checks here
switch modes and compare the stowed tires, over random gears: symmetrical or not, several tires
and bogies, a moved and flipped Geom, and rotations at 0, 180, about each axis and at Y = +-90.

When the axle keeps its direction the hinge is parallel to it, on the strut line.  A slide
along such an axle is beyond any hinge: every point must land exactly that distance away.
"""

import math
import random

import openvsp as vsp

STOW = vsp.GEAR_STOWED_POSITION
MECH = vsp.GEAR_MECHANISM
UW = [ ( u, w ) for u in ( 0.0, 0.3, 0.7 ) for w in ( 0.1, 0.4, 0.8 ) ]
TOL = 1e-9


def tires( gid ):
    """Points on every tire surface."""
    pts = []
    for s in range( vsp.GetNumMainSurfs( gid ) ):
        for u, w in UW:
            p = vsp.CompPnt01( gid, s, u, w )
            pts.append( ( p.x(), p.y(), p.z() ) )
    return pts


def moves( a, b ):
    return [ math.dist( p, q ) for p, q in zip( a, b ) ]


def stowed( gid, bogies, mode ):
    for b in bogies:
        vsp.SetParmVal( b, "RetMode", "Retract", mode )
    vsp.Update()
    return tires( gid )


def new_gear( rng, nbogie = 1, sym = False, multi = False, xform = False, flip = 0 ):
    vsp.VSPRenew()
    gid = vsp.AddGeom( "GEAR" )
    vsp.SetParmVal( gid, "ShowNominalGroundPlane", "GroundPlane", 0.0 )
    vsp.SetParmVal( gid, "GearConfigMode", "Gear", vsp.GEAR_CONFIGURATION_UP )
    while len( vsp.GetAllBogies( gid ) ) < nbogie:
        vsp.CreateAndAddBogie( gid )
    bogies = vsp.GetAllBogies( gid )
    for b in bogies:
        vsp.SetParmVal( b, "Symmetrical", "Bogie", float( sym ) )
        vsp.SetParmVal( b, "XContactPt", "Bogie", rng.uniform( -5.0, 20.0 ) )
        vsp.SetParmVal( b, "YContactPt", "Bogie", rng.uniform( 0.5, 10.0 ) )
        vsp.SetParmVal( b, "ZAboveGround", "Bogie", rng.uniform( 0.0, 2.0 ) )
        vsp.SetParmVal( b, "TravelX", "Bogie", rng.uniform( -0.3, 0.3 ) )
        vsp.SetParmVal( b, "TravelY", "Bogie", rng.uniform( -0.3, 0.3 ) )
        if multi:
            vsp.SetParmVal( b, "NumAcross", "Bogie", float( rng.randint( 1, 3 ) ) )
            vsp.SetParmVal( b, "NumTandem", "Bogie", float( rng.randint( 1, 3 ) ) )
    if xform:
        for n in ( "X_Rel_Location", "Y_Rel_Location", "Z_Rel_Location" ):
            vsp.SetParmVal( gid, n, "XForm", rng.uniform( -10.0, 10.0 ) )
        for n in ( "X_Rel_Rotation", "Y_Rel_Rotation", "Z_Rel_Rotation" ):
            vsp.SetParmVal( gid, n, "XForm", rng.uniform( -60.0, 60.0 ) )
    vsp.SetParmVal( gid, "Flip_Flag", "Sym", float( flip ) )
    vsp.Update()
    return gid, bogies


def random_gear( rng ):
    return new_gear( rng, nbogie = rng.randint( 1, 3 ), sym = rng.random() < 0.5, multi = rng.random() < 0.5,
                     xform = rng.random() < 0.5, flip = rng.choice( [ 0, 0, 0, 1, 2, 4 ] ) )


def set_hinge( b, axis, angle, rel ):
    vsp.SetParmVal( b, "RetMode", "Retract", MECH )
    vsp.Update()
    vsp.SetParmVal( b, "MechAbsRelFlag", "Retract", vsp.REL )
    for n, v in zip( ( "Mech_X_Axis", "Mech_Y_Axis", "Mech_Z_Axis" ), axis ):
        vsp.SetParmVal( b, n, "Retract", float( v ) )
    vsp.SetParmVal( b, "Mech_RetAngle", "Retract", angle )
    for n, v in zip( ( "MechXRelLoc", "MechYRelLoc", "MechZRelLoc" ), rel ):
        vsp.SetParmVal( b, n, "Retract", float( v ) )


def set_stow( b, rot, rel ):
    vsp.SetParmVal( b, "RetMode", "Retract", STOW )
    vsp.Update()
    vsp.SetParmVal( b, "StowAbsRelFlag", "Retract", vsp.REL )
    for n, v in zip( ( "StowXRelRot", "StowYRelRot", "StowZRelRot" ), rot ):
        vsp.SetParmVal( b, n, "Retract", float( v ) )
    for n, v in zip( ( "StowXRelLoc", "StowYRelLoc", "StowZRelLoc" ), rel ):
        vsp.SetParmVal( b, n, "Retract", float( v ) )


def rand_axis( rng ):
    v = [ rng.gauss( 0.0, 1.0 ) for i in range( 3 ) ]
    n = math.sqrt( sum( x * x for x in v ) )
    return [ x / n for x in v ]


def rand_rel( rng ):
    return [ rng.uniform( -6.0, 6.0 ) for i in range( 3 ) ]


def rand_angle( rng ):
    kind = rng.randint( 0, 5 )
    if kind == 0:
        return rng.choice( [ -180.0, 180.0 ] )
    if kind == 1:
        return rng.choice( [ -1.0, 1.0 ] ) * ( 180.0 - 10.0 ** rng.uniform( -9.0, -1.0 ) )
    if kind == 2:
        return rng.choice( [ -1.0, 1.0 ] ) * 10.0 ** rng.uniform( -3.0, -1.0 )
    return rng.uniform( -180.0, 180.0 )


def rand_hinge_axis( rng ):
    if rng.random() < 0.4:
        axis = [ 0.0, 0.0, 0.0 ]
        axis[ rng.randint( 0, 2 ) ] = rng.choice( [ -1.0, 1.0 ] )
        return axis
    return rand_axis( rng )


def rand_rot( rng ):
    if rng.random() < 0.2:
        return ( rng.uniform( -180.0, 180.0 ), rng.choice( [ -90.0, 90.0 ] ), rng.uniform( -180.0, 180.0 ) )
    if rng.random() < 0.3:
        rot = [ 0.0, 0.0, 0.0 ]
        rot[ rng.randint( 0, 2 ) ] = rand_angle( rng )
        if abs( rot[1] ) > 90.0:
            rot[1] = rng.uniform( -90.0, 90.0 )
        return tuple( rot )
    return ( rng.uniform( -180.0, 180.0 ), rng.uniform( -90.0, 90.0 ), rng.uniform( -180.0, 180.0 ) )


def test_symmetric_stowed_gear_switches_to_its_hinge():
    """A symmetrical bogie folded 90 degrees about X -- the hinge used to be fitted to its mirror image."""
    rng = random.Random( 1 )
    gid, bogies = new_gear( rng, sym = True )
    b = bogies[0]
    vsp.SetParmVal( b, "XContactPt", "Bogie", 18.8 )
    vsp.SetParmVal( b, "YContactPt", "Bogie", 8.6 )
    set_stow( b, ( -90.0, 0.0, 0.0 ), ( 0.0, -6.25, 7.0 ) )

    s0 = stowed( gid, bogies, STOW )
    m1 = stowed( gid, bogies, MECH )
    assert max( moves( s0, m1 ) ) < TOL
    assert abs( abs( vsp.GetParmVal( b, "Mech_RetAngle", "Retract" ) ) - 90.0 ) < TOL


def test_mechanism_to_stowed_and_back():
    rng = random.Random( 20261010 )
    for trial in range( 40 ):
        gid, bogies = random_gear( rng )
        for b in bogies:
            set_hinge( b, rand_hinge_axis( rng ), rand_angle( rng ), rand_rel( rng ) )

        m0 = stowed( gid, bogies, MECH )
        s1 = stowed( gid, bogies, STOW )
        m2 = stowed( gid, bogies, MECH )
        assert max( moves( m0, s1 ) ) < TOL, "trial %d: mechanism -> stowed" % trial
        assert max( moves( m0, m2 ) ) < TOL, "trial %d: mechanism -> stowed -> mechanism" % trial


def test_stowed_to_mechanism_and_back():
    rng = random.Random( 4242 )
    for trial in range( 40 ):
        gid, bogies = random_gear( rng )
        for b in bogies:
            rot = rand_rot( rng )
            rel = rand_rel( rng )
            if rot[0] == 0.0 and rot[2] == 0.0:
                # The axle keeps its direction; leave it no slide along itself.
                rel[1] = 0.0
            set_stow( b, rot, rel )

        s0 = stowed( gid, bogies, STOW )
        m1 = stowed( gid, bogies, MECH )
        s2 = stowed( gid, bogies, STOW )
        assert max( moves( s0, m1 ) ) < TOL, "trial %d: stowed -> mechanism" % trial
        assert max( moves( s0, s2 ) ) < TOL, "trial %d: stowed -> mechanism -> stowed" % trial


def test_slide_along_a_kept_axle_is_dropped():
    """No hinge reaches these; every point must land the slide away."""
    rng = random.Random( 77 )
    for trial in range( 20 ):
        gid, bogies = new_gear( rng, sym = rng.random() < 0.5, multi = rng.random() < 0.5 )
        b = bogies[0]
        rot = ( 0.0, rng.choice( [ 0.0, rng.uniform( -80.0, 80.0 ) ] ), 0.0 )
        rel = rand_rel( rng )
        d = abs( rel[1] )
        set_stow( b, rot, rel )

        m = moves( stowed( gid, bogies, STOW ), stowed( gid, bogies, MECH ) )
        assert max( m ) - d < TOL and d - min( m ) < TOL, "trial %d: slide %.6g, moved %.6g to %.6g" % ( trial, d, min( m ), max( m ) )


def test_nose_gear_folds_about_the_top_of_its_strut():
    """A tire moved forward and up by L, unturned, folds 90 degrees about a hinge L up its strut."""
    rng = random.Random( 5 )
    gid, bogies = new_gear( rng )
    b = bogies[0]
    for n, v in ( ( "TravelX", 0.0 ), ( "TravelY", 0.0 ), ( "TravelZ", 1.0 ) ):
        vsp.SetParmVal( b, n, "Bogie", v )
    length = 3.0
    set_stow( b, ( 0.0, 0.0, 0.0 ), ( length, 0.0, length ) )

    s0 = stowed( gid, bogies, STOW )
    m1 = stowed( gid, bogies, MECH )
    assert max( moves( s0, m1 ) ) < TOL

    axis = [ vsp.GetParmVal( b, n, "Retract" ) for n in ( "Mech_X_Axis", "Mech_Y_Axis", "Mech_Z_Axis" ) ]
    trunnion = [ vsp.GetParmVal( b, n, "Retract" ) for n in ( "MechXRelLoc", "MechYRelLoc", "MechZRelLoc" ) ]
    assert abs( axis[0] ) < TOL and abs( abs( axis[1] ) - 1.0 ) < TOL and abs( axis[2] ) < TOL
    assert math.dist( trunnion, [ 0.0, 0.0, length ] ) < TOL
    assert abs( abs( vsp.GetParmVal( b, "Mech_RetAngle", "Retract" ) ) - 90.0 ) < TOL
