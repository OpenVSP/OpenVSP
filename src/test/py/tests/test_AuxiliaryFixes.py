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
