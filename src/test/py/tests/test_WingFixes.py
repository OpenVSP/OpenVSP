"""Wing section operations."""

import openvsp as vsp


def sect( wid, i, name ):
    return vsp.GetParmVal( vsp.GetXSecParm( vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), i ), name ) )


def test_insert_after_the_tip_adds_a_section():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.Update()
    tip = vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 )
    vsp.SetParmVal( vsp.GetXSecParm( tip, "Span" ), 7.0 )
    vsp.SetParmVal( vsp.GetXSecParm( tip, "Sweep" ), 20.0 )
    vsp.Update()
    vsp.CopyXSec( wid, 1 )
    vsp.InsertXSec( wid, 1, vsp.XS_FOUR_SERIES )
    vsp.Update()
    assert vsp.GetNumXSec( vsp.GetXSecSurf( wid, 0 ) ) == 3
    for i in ( 1, 2 ):
        assert sect( wid, i, "Span" ) == 7.0 and sect( wid, i, "Sweep" ) == 20.0, i
