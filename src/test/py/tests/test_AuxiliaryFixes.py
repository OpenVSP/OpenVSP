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
