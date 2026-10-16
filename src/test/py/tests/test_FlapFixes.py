"""A section's plain flap, on every component built from sections."""

import openvsp as vsp


def points( gid ):
    vsp.Update()
    return [ vsp.CompPnt01( gid, 0, u / 20.0, w / 40.0 ) for u in range( 21 ) for w in range( 41 ) ]


def set_flap( xs, deflection, chord = 0.2, hinge = 0.5 ):
    for name, v in ( ( "TE_Flap_Type", vsp.FLAP_PLAIN ), ( "TE_Flap_Deflection", deflection ),
                     ( "TE_Flap_X_Chord", chord ), ( "TE_Flap_Y_Frc", hinge ) ):
        vsp.SetParmVal( vsp.GetXSecParm( xs, name ), v )


def test_a_propeller_section_flap_moves_the_blade():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    before = points( pid )
    set_flap( vsp.GetXSec( vsp.GetXSecSurf( pid, 0 ), 1 ), 20.0 )
    after = points( pid )
    assert max( vsp.dist( p, q ) for p, q in zip( before, after ) ) > 0.05


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
