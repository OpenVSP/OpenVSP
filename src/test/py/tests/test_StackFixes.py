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


def engine_val( sid, name ):
    return vsp.GetParmVal( vsp.FindParm( sid, name, "EngineModel" ) )


def stack_points( sid ):
    vsp.Update()
    return [ vsp.CompPnt01( sid, s, i / 10.0, j / 8.0 )
             for s in range( vsp.GetNumMainSurfs( sid ) ) for i in range( 11 ) for j in range( 9 ) ]


def test_the_flowpath_outlet_preset_shows_the_outlet_it_builds():
    sid, surf = new_stack()
    vsp.InitStackPreset( sid, vsp.STACK_PRESET_FLOWPATH_OUT )
    vsp.Update()
    assert engine_val( sid, "GeomOutType" ) == vsp.ENGINE_GEOM_FLOWPATH


def test_a_flowthrough_preset_after_another_builds_the_open_nacelle():
    # Each sets its outlet, so what an earlier preset or a user left there does not make it a duct.
    for preset in ( vsp.STACK_PRESET_FLOWTHRU_OUTLIP_ORIG, vsp.STACK_PRESET_FLOWTHRU_INLIP_ORIG,
                    vsp.STACK_PRESET_FLOWTHRU_MID_ORIG ):
        sid, surf = new_stack()
        vsp.InitStackPreset( sid, preset )
        fresh = stack_points( sid )

        sid, surf = new_stack()
        vsp.InitStackPreset( sid, vsp.STACK_PRESET_FLOWPATH_OUT )
        vsp.SetParmVal( vsp.FindParm( sid, "OutletModeType", "EngineModel" ), vsp.ENGINE_MODE_EXTEND )
        vsp.Update()
        vsp.InitStackPreset( sid, preset )
        after = stack_points( sid )

        assert engine_val( sid, "GeomOutType" ) == vsp.ENGINE_GEOM_TO_LIP
        assert engine_val( sid, "OutletModeType" ) == vsp.ENGINE_MODE_TO_LIP
        assert len( fresh ) == len( after )
        assert max( vsp.dist( p, q ) for p, q in zip( fresh, after ) ) < 1e-12


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


def test_the_flowthrough_presets_are_plain_flowthrough():
    for preset in ( vsp.STACK_PRESET_FLOWTHRU_OUTLIP_ORIG, vsp.STACK_PRESET_FLOWTHRU_INLIP_ORIG,
                    vsp.STACK_PRESET_FLOWTHRU_MID_ORIG ):
        sid, surf = new_stack()
        vsp.InitStackPreset( sid, preset )
        vsp.Update()
        assert vsp.GetParmVal( sid, "InletModeType", "EngineModel" ) == vsp.ENGINE_MODE_FLOWTHROUGH, preset
        assert vsp.GetNumMainSurfs( sid ) == 1, preset
        assert vsp.GetGeomVSPSurfCfdType( sid, 0 ) == vsp.CFD_NORMAL, preset


def length_along( sid, w, n ):
    pts = [ vsp.CompPnt01( sid, 0, i / float( n ), w ) for i in range( n + 1 ) ]
    return sum( vsp.dist( pts[i], pts[i + 1] ) for i in range( n ) )


def test_the_mid_origin_nacelle_is_the_outlet_lip_origin_one():
    # The two presets list the same sections from a different first one, so with the mid planes
    # put together every point of the one lies on the other, and a line run once around the
    # nacelle along its length is as long on each: no stretch of it is passed over twice.
    vsp.VSPRenew()
    ref = vsp.AddGeom( "STACK" )
    vsp.InitStackPreset( ref, vsp.STACK_PRESET_FLOWTHRU_OUTLIP_ORIG )
    surf = vsp.GetXSecSurf( ref, 0 )
    midx = sum( vsp.GetParmVal( vsp.GetXSecParm( vsp.GetXSec( surf, i ), "XDelta" ) ) for i in ( 1, 2 ) )
    vsp.SetParmVal( ref, "X_Rel_Location", "XForm", -midx )
    mid = vsp.AddGeom( "STACK" )
    vsp.InitStackPreset( mid, vsp.STACK_PRESET_FLOWTHRU_MID_ORIG )
    vsp.Update()

    worst = 0.0
    for i in range( 201 ):
        for j in range( 5 ):
            d, u, w = vsp.ProjPnt01( ref, 0, vsp.CompPnt01( mid, 0, i / 200.0, j / 4.0 ) )
            worst = max( worst, d )
    assert worst < 5e-3

    for w in ( 0.0, 0.3 ):
        assert abs( length_along( mid, w, 2000 ) - length_along( ref, w, 2000 ) ) < 1e-2, w


def test_the_flowthrough_loop_presets_trace_one_nacelle():
    # The three flowthrough presets list the same sections around the loop from a different first
    # one.  With their mid planes put together, every point of each lies on the out-lip-origin
    # one, the strengths at the seam included.
    vsp.VSPRenew()
    geoms = []
    for preset, imid in ( ( vsp.STACK_PRESET_FLOWTHRU_OUTLIP_ORIG, 2 ), ( vsp.STACK_PRESET_FLOWTHRU_INLIP_ORIG, 4 ),
                          ( vsp.STACK_PRESET_FLOWTHRU_MID_ORIG, 0 ) ):
        g = vsp.AddGeom( "STACK" )
        vsp.InitStackPreset( g, preset )
        surf = vsp.GetXSecSurf( g, 0 )
        midx = sum( vsp.GetParmVal( vsp.GetXSecParm( vsp.GetXSec( surf, i ), "XDelta" ) ) for i in range( 1, imid + 1 ) )
        vsp.SetParmVal( g, "X_Rel_Location", "XForm", -midx )
        geoms.append( g )
    vsp.Update()

    ref = geoms[0]
    for g in geoms[1:]:
        worst = 0.0
        for i in range( 301 ):
            for j in range( 9 ):
                d, u, w = vsp.ProjPnt01( ref, 0, vsp.CompPnt01( g, 0, i / 300.0, j / 8.0 ) )
                worst = max( worst, d )
        assert worst < 1e-9, worst
