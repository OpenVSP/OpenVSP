# A custom component's internal containers -- those not listed with it -- are found only by its own script.

import openvsp as vsp


def cone():
    vsp.VSPRenew()
    gid = vsp.AddGeom( "Cone" )
    vsp.Update()
    return gid


def clear_errors():
    mgr = vsp.ErrorMgrSingleton.getInstance()
    while mgr.GetNumTotalErrors() > 0:
        mgr.PopLastError()


def testACustomComponentsSectionsAreNotFoundFromOutside():
    gid = cone()
    assert vsp.GetNumXSecSurfs( gid ) == 1
    assert vsp.GetXSecSurf( gid, 0 ) == ""
    assert vsp.GetXSecSurfIDVec( gid ) == ()
    assert vsp.FindParm( gid, "Circle_Diameter", "XSecCurve" ) == ""
    clear_errors()

    #==== A built-in component's are ====#
    fid = vsp.AddGeom( "FUSELAGE" )
    assert vsp.GetXSecSurf( fid, 0 ) != ""
    assert len( vsp.GetXSecSurfIDVec( fid ) ) == 1


def testACustomComponentsOwnParmsAreFound():
    gid = cone()
    assert vsp.FindParm( gid, "Diameter", "Design" ) != ""
    assert vsp.FindParm( gid, "X_Rel_Location", "XForm" ) != ""


def testItsScriptStillReachesItsSections():
    gid = cone()
    vsp.SetParmVal( gid, "Diameter", "Design", 1.0 )
    vsp.Update()
    small = vsp.GetGeomBBoxMax( gid ).y() - vsp.GetGeomBBoxMin( gid ).y()
    vsp.SetParmVal( gid, "Diameter", "Design", 4.0 )
    vsp.Update()
    large = vsp.GetGeomBBoxMax( gid ).y() - vsp.GetGeomBBoxMin( gid ).y()
    assert large > 3.0 * small
