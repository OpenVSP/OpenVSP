"""Which engine modes the GUI greys."""

import openvsp as vsp


def new_stack():
    vsp.VSPRenew()
    sid = vsp.AddGeom( "STACK" )
    vsp.Update()
    return sid, vsp.GetXSecSurf( sid, 0 )


def test_outlet_mode_is_greyed_for_a_flowthrough_inlet_only_where_it_is_ignored():
    for preset, active in ( ( vsp.STACK_PRESET_FLOWTHRU_MID_ORIG, False ), ( vsp.STACK_PRESET_BOTHFACEFACE, False ),
                            ( vsp.STACK_PRESET_FLOWPATH_BOTH, True ), ( vsp.STACK_PRESET_FLOWPATH_OUT, True ) ):
        sid, surf = new_stack()
        vsp.InitStackPreset( sid, preset )
        vsp.SetParmVal( sid, "InletModeType", "EngineModel", vsp.ENGINE_MODE_FLOWTHROUGH_NEG )
        vsp.Update()
        assert vsp.GetParmActiveFlag( vsp.GetParm( sid, "OutletModeType", "EngineModel" ) ) == active, preset
