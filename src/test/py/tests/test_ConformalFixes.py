"""Conformal components."""

import openvsp as vsp


def test_a_gear_is_refused_as_a_conformal_parent():
    vsp.VSPRenew()
    gear = vsp.AddGeom( "GEAR" )
    vsp.Update()
    assert vsp.AddGeom( "CONFORMAL", gear ) == "NONE"
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    conf = vsp.AddGeom( "CONFORMAL", pod )
    vsp.Update()
    before = ( vsp.GetGeomBBoxMin( conf ), vsp.GetGeomBBoxMax( conf ) )
    vsp.SetGeomParent( conf, gear )
    vsp.Update()
    assert ( vsp.GetGeomBBoxMin( conf ) - before[0] ).mag() < 1e-9
    assert ( vsp.GetGeomBBoxMax( conf ) - before[1] ).mag() < 1e-9
