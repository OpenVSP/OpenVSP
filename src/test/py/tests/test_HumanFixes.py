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


def super_cone_bounds( size_first ):
    vsp.VSPRenew()
    hid = vsp.AddGeom( "HUMAN" )
    vsp.Update()
    if size_first:
        vsp.SetParmVal( hid, "Stature_pct", "Anthropometric", 0.95 )
        vsp.Update()
    cone = vsp.AddGeom( "AUXILIARY", hid )
    vsp.Update()
    vsp.SetParmVal( cone, "AuxiliaryGeomType", "Design", vsp.AUX_GEOM_SUPER_CONE )
    vsp.Update()
    if not size_first:
        vsp.SetParmVal( hid, "Stature_pct", "Anthropometric", 0.95 )
        vsp.Update()
    return vsp.GetGeomBBoxMin( cone ), vsp.GetGeomBBoxMax( cone )


def test_a_super_cone_follows_its_human():
    fresh = super_cone_bounds( True )
    followed = super_cone_bounds( False )
    assert ( fresh[0] - followed[0] ).mag() < 1e-9
    assert ( fresh[1] - followed[1] ).mag() < 1e-9
