"""A section's plain flap, on every component built from sections."""

import math

import openvsp as vsp


def points( gid ):
    vsp.Update()
    return [ vsp.CompPnt01( gid, 0, u / 20.0, w / 40.0 ) for u in range( 21 ) for w in range( 41 ) ]


def set_flap( xs, deflection, chord = 0.2, hinge = 0.5 ):
    for name, v in ( ( "TE_Flap_Type", vsp.FLAP_PLAIN ), ( "TE_Flap_Deflection", deflection ),
                     ( "TE_Flap_X_Chord", chord ), ( "TE_Flap_Y_Frc", hinge ) ):
        vsp.SetParmVal( vsp.GetXSecParm( xs, name ), v )


def finite( pts ):
    return all( math.isfinite( c ) for p in pts for c in ( p.x(), p.y(), p.z() ) )


def test_a_propeller_section_flap_moves_the_blade():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    before = points( pid )
    set_flap( vsp.GetXSec( vsp.GetXSecSurf( pid, 0 ), 1 ), 20.0 )
    after = points( pid )
    assert max( vsp.dist( p, q ) for p, q in zip( before, after ) ) > 0.05


def test_a_plain_flap_on_a_round_section():
    for shape in ( vsp.XS_CIRCLE, vsp.XS_ROUNDED_RECTANGLE ):
        for deflection in ( -40.0, 40.0 ):
            for hinge in ( 0.0, 0.5, 1.0 ):
                vsp.VSPRenew()
                wid = vsp.AddGeom( "WING" )
                surf = vsp.GetXSecSurf( wid, 0 )
                vsp.ChangeXSecShape( surf, 1, shape )
                set_flap( vsp.GetXSec( surf, 1 ), deflection, 0.05, hinge )
                assert finite( points( wid ) ), ( shape, deflection, hinge )


def test_a_flap_with_no_chord_leaves_the_wing_as_it_is():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    before = points( wid )
    set_flap( vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 ), -40.0, 0.0 )
    after = points( wid )
    assert max( vsp.dist( p, q ) for p, q in zip( before, after ) ) < 1e-12


def test_a_long_flap_deflected_far_on_a_round_section():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    surf = vsp.GetXSecSurf( wid, 0 )
    vsp.ChangeXSecShape( surf, 1, vsp.XS_CIRCLE )
    set_flap( vsp.GetXSec( surf, 1 ), -40.0, 0.9 )
    assert finite( points( wid ) )


def test_an_absolute_flap_on_a_section_made_shorter():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    xs = vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 )
    chord = vsp.GetParmVal( vsp.GetXSecParm( xs, "Tip_Chord" ) )
    for name, v in ( ( "TE_Flap_Type", vsp.FLAP_PLAIN ), ( "TE_Flap_AbsRel", vsp.ABS ), ( "TE_Flap_X", 0.9 * chord ),
                     ( "TE_Flap_Deflection", -40.0 ) ):
        vsp.SetParmVal( vsp.GetXSecParm( xs, name ), v )
    vsp.Update()
    # The flap is now longer than the section's chord.
    vsp.SetParmVal( vsp.GetXSecParm( xs, "Tip_Chord" ), 0.3 * chord )
    assert finite( points( wid ) )


def test_a_propeller_flap_follows_a_diameter_change_at_once():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    surf = vsp.GetXSecSurf( pid, 0 )
    for i in range( vsp.GetNumXSec( surf ) ):
        xs = vsp.GetXSec( surf, i )
        set_flap( xs, 30.0 )
        # A flap given in model units: its fraction of the chord changes with the diameter.
        vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_AbsRel" ), vsp.ABS )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_X" ), 0.05 )
    vsp.Update()
    diameter = vsp.FindParm( pid, "Diameter", "Design" )
    vsp.SetParmVal( diameter, 2.0 * vsp.GetParmVal( diameter ) )
    once = points( pid )
    # Another Update with nothing changed must not move the blade.
    vsp.SetParmVal( diameter, 1.000001 * vsp.GetParmVal( diameter ) )
    vsp.Update()
    vsp.SetParmVal( diameter, vsp.GetParmVal( diameter ) / 1.000001 )
    again = points( pid )
    assert max( vsp.dist( p, q ) for p, q in zip( once, again ) ) < 1e-9


def test_a_wing_flap_follows_a_chord_change_at_once():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    xs = vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 )
    set_flap( xs, 30.0 )
    # A flap given in model units: its fraction of the chord changes with the chord.
    vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_AbsRel" ), vsp.ABS )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_X" ), 0.1 )
    vsp.Update()
    chord = vsp.GetXSecParm( xs, "Tip_Chord" )
    vsp.SetParmVal( chord, 2.0 * vsp.GetParmVal( chord ) )
    once = points( wid )
    # Another Update with nothing changed must not move the wing.
    vsp.SetParmVal( chord, 1.000001 * vsp.GetParmVal( chord ) )
    vsp.Update()
    vsp.SetParmVal( chord, vsp.GetParmVal( chord ) / 1.000001 )
    again = points( wid )
    assert max( vsp.dist( p, q ) for p, q in zip( once, again ) ) < 1e-9


def test_a_file_with_the_flap_abs_rel_under_its_old_name_reads_it( tmp_path ):
    # Files written before the flap's Abs/Rel Parm was named TE_Flap_AbsRel stored it as TE_Trim_AbsRel in the Flap
    # group.
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    xs = vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 )
    set_flap( xs, 30.0 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_AbsRel" ), vsp.ABS )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_X" ), 0.1 )
    shown = points( wid )
    path = str( tmp_path / "flap.vsp3" )
    vsp.WriteVSPFile( path )
    text = open( path ).read()
    assert text.count( "<TE_Flap_AbsRel " ) >= 1
    open( path, "w" ).write( text.replace( "<TE_Flap_AbsRel ", "<TE_Trim_AbsRel " ) )

    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    wid = vsp.FindGeoms()[0]
    xs = vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 )
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "TE_Flap_AbsRel" ) ) == vsp.ABS
    assert max( vsp.dist( p, q ) for p, q in zip( shown, points( wid ) ) ) < 1e-9


def test_an_absolute_flap_length_builds_its_flap_on_the_first_update():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    xs = vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 )
    set_flap( xs, 30.0 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_AbsRel" ), vsp.ABS )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_X" ), 0.1 )
    first = points( wid )
    # Nudge a flap Parm and set it back, so the wing is built again from the same values.
    hinge = vsp.GetXSecParm( xs, "TE_Flap_Y_Frc" )
    vsp.SetParmVal( hinge, 0.6 )
    vsp.Update()
    vsp.SetParmVal( hinge, 0.5 )
    assert max( vsp.dist( p, q ) for p, q in zip( first, points( wid ) ) ) < 1e-9
