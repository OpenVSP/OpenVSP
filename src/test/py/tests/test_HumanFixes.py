"""Human components."""

import openvsp as vsp


def test_stature_limits_follow_the_sex():
    vsp.VSPRenew()
    hid = vsp.AddGeom( "HUMAN" )
    vsp.Update()
    vsp.SetParmVal( hid, "LenUnit", "Anthropometric", vsp.LEN_M )
    vsp.Update()
    vsp.SetParmVal( hid, "GenderFlag", "Anthropometric", vsp.FEMALE )
    vsp.Update()
    st = vsp.FindParm( hid, "Stature", "Anthropometric" )
    assert abs( vsp.GetParmLowerLimit( st ) - 1.498 ) < 1e-9
    assert abs( vsp.GetParmUpperLimit( st ) - 1.735 ) < 1e-9
    assert abs( vsp.GetParmVal( st ) - 1.619 ) < 1e-3
