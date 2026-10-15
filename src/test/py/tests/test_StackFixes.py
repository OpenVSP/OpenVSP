"""Stack section operations."""

import openvsp as vsp

STACK_FREE = 0.0


def new_stack():
    vsp.VSPRenew()
    sid = vsp.AddGeom( "STACK" )
    vsp.Update()
    return sid, vsp.GetXSecSurf( sid, 0 )


def test_insert_after_the_last_section_places_it():
    sid, surf = new_stack()
    vsp.SetParmVal( sid, "OrderPolicy", "Design", STACK_FREE )
    vsp.Update()
    n = vsp.GetNumXSec( surf )
    before = vsp.GetGeomBBoxMax( sid ).x()
    last_dx = vsp.GetParmVal( vsp.GetXSecParm( vsp.GetXSec( surf, n - 1 ), "XDelta" ) )
    vsp.InsertXSec( sid, n - 1, vsp.XS_CIRCLE )
    vsp.Update()
    assert vsp.GetNumXSec( surf ) == n + 1
    assert vsp.GetParmVal( vsp.GetXSecParm( vsp.GetXSec( surf, n ), "XDelta" ) ) == last_dx
    assert abs( vsp.GetGeomBBoxMax( sid ).x() - ( before + last_dx ) ) < 1e-9
