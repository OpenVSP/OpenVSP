"""A section with a plain flap that cannot be deflected is laid out as one with no flap."""

import openvsp as vsp

TMAGIC = 0.004


def test_a_plain_flap_with_no_chord_is_laid_out_as_no_flap():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wid, "CapUMinOption", "EndCap", vsp.NO_END_CAP )
    vsp.SetParmVal( wid, "CapUMaxOption", "EndCap", vsp.NO_END_CAP )
    vsp.Update()
    vsp.SplitWingXSec( wid, 1 )
    vsp.Update()
    surf = vsp.GetXSecSurf( wid, 0 )
    for i, chord in ( ( 1, 0.0 ), ( 2, 0.3 ) ):
        xs = vsp.GetXSec( surf, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_Type" ), vsp.FLAP_PLAIN )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_X_Chord" ), chord )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_Deflection" ), 40.0 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_Y_Frc" ), 0.0 )
    vsp.Update()
    xs = vsp.GetXSec( surf, 1 )
    t = vsp.GetParmVal( vsp.GetXSecParm( xs, "TE_Flap_T" ) )
    dt = vsp.GetParmVal( vsp.GetXSecParm( xs, "TE_Flap_DT" ) )
    assert dt > 0.001
    # Every section of a flapped wing has a piece of no length where the flap's arc is, on each side;
    # on the lower side it spans curve parameter TMAGIC + 2 T to TMAGIC + 2 T + 2 DT, and w is a quarter of that.
    w0 = ( TMAGIC + 2.0 * t ) / 4.0
    w1 = ( TMAGIC + 2.0 * t + 2.0 * dt ) / 4.0
    pts = [ vsp.CompPnt01( wid, 0, 0.5, w0 + ( w1 - w0 ) * k / 10.0 ) for k in range( 11 ) ]
    assert max( vsp.dist( p, pts[ 0 ] ) for p in pts ) < 1e-9
