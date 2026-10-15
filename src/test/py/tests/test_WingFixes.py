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


def split_twisted_wing( relative ):
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wid, "RelativeTwistFlag", "WingGeom", float( relative ) )
    tip = vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 )
    vsp.SetParmVal( vsp.GetXSecParm( tip, "Twist" ), -6.0 )
    vsp.Update()
    before = vsp.CompPnt01( wid, 0, 1.0, 0.0 )
    vsp.SplitWingXSec( wid, 1 )
    vsp.Update()
    after = vsp.CompPnt01( wid, 0, 1.0, 0.0 )
    return wid, before, after


def test_split_keeps_the_tip_twist():
    for relative in ( False, True ):
        wid, before, after = split_twisted_wing( relative )
        assert ( before - after ).mag() < 1e-9, relative
        if relative:
            assert sect( wid, 1, "Twist" ) == -3.0 and sect( wid, 2, "Twist" ) == -3.0
        else:
            assert sect( wid, 1, "Twist" ) == -3.0 and sect( wid, 2, "Twist" ) == -6.0


def test_blending_matched_to_inboard_angles_is_smooth():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wid, "CapUMinOption", "EndCap", vsp.NO_END_CAP )
    vsp.SetParmVal( wid, "CapUMaxOption", "EndCap", vsp.NO_END_CAP )
    vsp.Update()
    vsp.SplitWingXSec( wid, 1 )
    vsp.Update()
    xs = vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "InLEMode" ), vsp.BLEND_ANGLES )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "InLEDihedral" ), 10.0 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "OutLEMode" ), vsp.BLEND_MATCH_IN_ANGLES )
    vsp.Update()
    # The joint between the two panels is halfway along u; w 0.5 is the leading edge.
    h = 1e-4
    pm = vsp.CompPnt01( wid, 0, 0.5 - h, 0.5 )
    p0 = vsp.CompPnt01( wid, 0, 0.5, 0.5 )
    pp = vsp.CompPnt01( wid, 0, 0.5 + h, 0.5 )
    left = ( p0.z() - pm.z() ) / ( p0.y() - pm.y() )
    right = ( pp.z() - p0.z() ) / ( pp.y() - p0.y() )
    assert abs( left - right ) < 1e-3, ( left, right )


def test_a_section_edit_after_a_symmetry_change_is_kept():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.Update()
    vsp.SetParmVal( wid, "Sym_Planar_Flag", "Sym", 0.0 )
    vsp.Update()
    vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 ), "Tip_Chord" ), 0.5 )
    vsp.Update()
    assert abs( sect( wid, 1, "Span" ) - 9.0 ) < 1e-9
    assert abs( sect( wid, 1, "Root_Chord" ) - 4.0 ) < 1e-9
    assert abs( sect( wid, 1, "Tip_Chord" ) - 0.5 ) < 1e-9
