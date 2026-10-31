"""Auxiliary components."""

import openvsp as vsp


def test_scaling_keeps_the_spray_ratios():
    vsp.VSPRenew()
    aux = vsp.AddGeom( "AUXILIARY" )
    vsp.Update()
    names = ( "SprayTireContactWidth", "SprayTireContactHalfLength", "SprayCenterWidth" )
    before = [ vsp.GetParmVal( vsp.FindParm( aux, n, "Design" ) ) for n in names ]
    vsp.SetParmVal( aux, "Scale", "XForm", 2.0 )
    vsp.Update()
    after = [ vsp.GetParmVal( vsp.FindParm( aux, n, "Design" ) ) for n in names ]
    assert after == before


def test_tip_path_takes_the_propeller_flap_hinge():
    vsp.VSPRenew()
    prop = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( prop, "RFlapAx", "Articulation", 0.2 )
    aux = vsp.AddGeom( "AUXILIARY", prop )
    vsp.SetParmVal( aux, "AuxiliaryGeomType", "Design", vsp.AUX_GEOM_ROTOR_TIP_PATH )
    vsp.Update()
    assert abs( vsp.GetParmVal( aux, "FlapRadiusFract", "Design" ) - 0.2 ) < 1e-12

    vsp.SetParmVal( aux, "AutoFlapRadius", "Design", 0 )
    vsp.SetParmVal( aux, "FlapRadiusFract", "Design", 0.3 )
    vsp.Update()
    assert abs( vsp.GetParmVal( aux, "FlapRadiusFract", "Design" ) - 0.3 ) < 1e-12


def cce_track( offset, main2_x, cce = "-10 1\n0 0\n10 1\n" ):
    """A 3pt Composite Clearance Envelope with a V-shaped profile over a tricycle gear; returns the line where the
    profile's lateral position is zero, the main gear's center and the second main gear's contact point."""
    import os
    import tempfile
    vsp.VSPRenew()
    gear = vsp.AddGeom( "GEAR" )
    vsp.SetParmVal( gear, "ShowNominalGroundPlane", "GroundPlane", 0 )
    places = ( ( 0.0, 0.0 ), ( 12.0, 4.0 ), ( main2_x, -4.0 ) )
    bogies = []
    for x, y in places:
        b = vsp.CreateAndAddBogie( gear )
        vsp.SetParmVal( vsp.FindParm( b, "XContactPt", "Bogie" ), x )
        vsp.SetParmVal( vsp.FindParm( b, "YContactPt", "Bogie" ), y )
        bogies.append( b )
    vsp.Update()
    aux = vsp.AddGeom( "AUXILIARY", gear )
    vsp.SetParmVal( aux, "AuxiliaryGeomType", "Design", vsp.AUX_GEOM_THREE_PT_CCE )
    vsp.Update()
    for i, b in enumerate( bogies ):
        vsp.SetAuxiliaryGeomContactPtID( aux, i, b )
    vsp.SetParmVal( aux, "CCEMainGearOffset", "Design", offset )
    path = os.path.join( tempfile.mkdtemp(), "v.cce" )
    open( path, "w" ).write( cce )
    vsp.ReadAuxiliaryGeomCCEFile( aux, path )
    vsp.Update()
    n = 2001
    pts = vsp.CompVecPnt01( aux, 0, [ i / ( n - 1.0 ) for i in range( n ) ], [ 0.0 ] * n )
    # The profile's vertex, at lateral position zero, is its only point on the ground.
    u = min( range( n ), key = lambda i: abs( pts[ i ].z() ) ) / ( n - 1.0 )
    a, b = vsp.CompVecPnt01( aux, 0, [ u, u ], [ 0.0, 1.0 ] )
    main_center = vsp.vec3d( 0.5 * ( 12.0 + main2_x ), 0.0, 0.0 )
    return a, b, main_center, vsp.vec3d( main2_x, -4.0, 0.0 )


def test_clearance_envelope_main_gear_offset_is_square_to_the_track():
    # The main gear's center lies Main Gear Offset from the track, measured square to it, toward Main Bogie 2 for a
    # positive offset -- with the main gear's axle line square to the centerline or not.
    for main2_x in ( 12.0, 13.0 ):
        for offset in ( 0.0, 2.0, -3.0 ):
            a, b, c, m2 = cce_track( offset, main2_x )
            d = b - a
            d.normalize()
            side = vsp.cross( d, c - a )
            assert abs( side.mag() - abs( offset ) ) < 1e-6, ( main2_x, offset, side.mag() )
            if offset != 0.0:
                # Positive moves the track toward Main Bogie 2: the main gear's center is then on Main Bogie 1's side.
                toward2 = vsp.cross( d, m2 - a ).z() * side.z() < 0
                assert toward2 == ( offset > 0 ), ( main2_x, offset )


def test_clearance_envelope_file_skips_lines_without_two_numbers():
    # A header line and a line of one number are skipped; the profile is the same as without them.
    plain = cce_track( 0.0, 12.0 )
    noisy = cce_track( 0.0, 12.0, "lateral height\n-10 1\n0\n0 0\n10, 1\n" )
    for p, q in zip( plain[:2], noisy[:2] ):
        assert vsp.dist( p, q ) < 1e-12
