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


def test_cut_moves_the_later_sections_at_once():
    sid, surf = new_stack()
    before = vsp.GetGeomBBoxMax( sid ).x()
    cut_dx = vsp.GetParmVal( vsp.GetXSecParm( vsp.GetXSec( surf, 1 ), "XDelta" ) )
    vsp.CutXSec( sid, 1 )
    vsp.Update()
    assert abs( vsp.GetGeomBBoxMax( sid ).x() - ( before - cut_dx ) ) < 1e-9


def test_a_flowpath_in_negative_only_mode_is_negative():
    sid, surf = new_stack()
    vsp.InitStackPreset( sid, vsp.STACK_PRESET_FLOWPATH_BOTH )
    vsp.Update()
    for mode in ( vsp.ENGINE_MODE_FLOWTHROUGH_NEG, vsp.ENGINE_MODE_FLOWTHROUGH_NEG_ONLY ):
        vsp.SetParmVal( sid, "InletModeType", "EngineModel", mode )
        vsp.Update()
        assert vsp.GetNumMainSurfs( sid ) == 1, mode
        assert vsp.GetGeomVSPSurfCfdType( sid, 0 ) == vsp.CFD_NEGATIVE, mode


def test_the_flowpath_presets_are_the_duct_as_drawn():
    for preset in ( vsp.STACK_PRESET_FLOWPATH_IN, vsp.STACK_PRESET_FLOWPATH_OUT, vsp.STACK_PRESET_FLOWPATH_BOTH ):
        sid, surf = new_stack()
        vsp.InitStackPreset( sid, preset )
        vsp.Update()
        assert vsp.GetParmVal( sid, "InletModeType", "EngineModel" ) == vsp.ENGINE_MODE_TO_LIP, preset
        assert vsp.GetParmVal( sid, "OutletModeType", "EngineModel" ) == vsp.ENGINE_MODE_TO_LIP, preset
        assert vsp.GetNumMainSurfs( sid ) == 1, preset
        assert vsp.GetGeomVSPSurfCfdType( sid, 0 ) == vsp.CFD_NORMAL, preset


def test_a_child_attached_past_a_trimmed_engine_surface_stays_on_it():
    sid, surf = new_stack()
    vsp.InitStackPreset( sid, vsp.STACK_PRESET_FLOWTHRU_MID_ORIG )
    vsp.Update()
    pod = vsp.AddGeom( "POD", sid )
    vsp.SetParmVal( pod, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( pod, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_UV )
    vsp.SetParmVal( pod, "V_Attach_Location", "Attach", 0.75 )
    for u in ( 0.5, 0.9, 1.0 ):
        vsp.SetParmVal( pod, "U_Attach_Location", "Attach", u )
        vsp.Update()
        p = vsp.CompPnt01( pod, 0, 0.0, 0.0 )
        assert abs( p.x() ) < 1e3 and abs( p.y() ) < 1e3 and abs( p.z() ) < 1e3, u


def test_a_ruler_past_a_trimmed_engine_surface_stays_on_it():
    sid, surf = new_stack()
    vsp.InitStackPreset( sid, vsp.STACK_PRESET_FLOWTHRU_MID_ORIG )
    vsp.Update()
    rid = vsp.AddRuler( sid, 0, 0.9, 0.75, sid, 0, 0.1, 0.75, "ruler" )
    vsp.Update()
    assert rid
