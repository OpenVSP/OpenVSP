# A cross section curve sub-surface's curve Parms are found through the sub-surface's own ID by the curve's own
# group, wherever the sub-surface sits among the Geom's.

import openvsp as vsp


def curve_area_id( ss ):
    for pid in vsp.GetSubSurfParmIDs( ss ):
        if vsp.GetParmName( pid ) == "Area" and vsp.GetParmGroupName( pid ) == "XSecCurve":
            return pid
    return ""


def testACurvesParmIsFoundThroughItsSubSurface():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.AddSubSurf( wid, vsp.SS_RECTANGLE )
    s1 = vsp.AddSubSurf( wid, vsp.SS_XSEC_CURVE )
    s2 = vsp.AddSubSurf( wid, vsp.SS_XSEC_CURVE )
    vsp.Update()
    area = curve_area_id( s2 )
    assert area != ""
    assert vsp.GetParm( s2, "Area", "XSecCurve" ) == area

    #==== Still once an earlier one is gone ====#
    vsp.DeleteSubSurf( s1 )
    vsp.Update()
    assert vsp.GetParm( s2, "Area", "XSecCurve" ) == area
