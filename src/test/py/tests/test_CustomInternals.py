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


def script_subsurf_part():
    vsp.VSPRenew()
    gid = vsp.AddGeom( "ScriptSubSurfPart" )
    vsp.Update()
    return gid


def testAScriptsSubsurfaceIsReachedFromItsScript():
    gid = script_subsurf_part()
    vsp.SetParmVal( gid, "LineU", "Design", 0.3 )
    vsp.Update()
    assert abs( vsp.GetParmVal( gid, "ReadBack", "Design" ) - 0.3 ) < 1e-12


def testAScriptsSubsurfaceIsNotFoundFromOutside():
    gid = script_subsurf_part()
    ss = vsp.GetSubSurfIDVec( gid )
    assert len( ss ) == 1
    assert vsp.FindParm( gid, "Const_Line_Value", "SubSurface_1" ) == ""
    assert vsp.GetSubSurfParmIDs( ss[0] ) == ()
    assert vsp.FindContainerParmIDs( ss[0] ) == ()
    clear_errors()


def testAScriptsSubsurfaceIsNotChangedFromOutside():
    gid = script_subsurf_part()
    ss = vsp.GetSubSurfIDVec( gid )[0]
    name = vsp.GetSubSurfName( ss )
    vsp.SetSubSurfName( ss, "Renamed" )
    vsp.SetSubSurfName( gid, ss, "Renamed" )
    vsp.ReorderSubSurf( gid, ss, vsp.REORDER_MOVE_BOTTOM )
    vsp.DeleteSubSurf( ss )
    vsp.DeleteSubSurf( gid, ss )
    clear_errors()
    assert vsp.GetSubSurfName( ss ) == name
    assert vsp.GetSubSurfIDVec( gid ) == ( ss, )

    #==== Its script still drives it ====#
    vsp.SetParmVal( gid, "LineU", "Design", 0.3 )
    vsp.Update()
    assert abs( vsp.GetParmVal( gid, "ReadBack", "Design" ) - 0.3 ) < 1e-12


def testASubsurfaceAUserAddsIsChanged():
    gid = script_subsurf_part()
    user = vsp.AddSubSurf( gid, vsp.SS_LINE )
    vsp.SetSubSurfName( user, "Renamed" )
    assert vsp.GetSubSurfName( user ) == "Renamed"
    vsp.DeleteSubSurf( user )
    assert len( vsp.GetSubSurfIDVec( gid ) ) == 1


def testASubsurfaceAUserAddsIsFound():
    gid = script_subsurf_part()
    user = vsp.AddSubSurf( gid, vsp.SS_LINE )
    vsp.Update()
    assert vsp.FindParm( gid, "Const_Line_Value", "SubSurface_2" ) == vsp.GetParm( user, "Const_Line_Value", "SubSurface" )
    assert vsp.GetSubSurfParmIDs( user ) != ()


def testAScriptsSubsurfaceIsNotSavedTwice( tmp_path ):
    gid = script_subsurf_part()
    user = vsp.AddSubSurf( gid, vsp.SS_LINE )
    vsp.Update()
    fname = str( tmp_path / "ScriptSubSurfPart.vsp3" )
    for i in range( 2 ):
        vsp.WriteVSPFile( fname )
        vsp.VSPRenew()
        vsp.ReadVSPFile( fname )
        vsp.Update()
        gid = vsp.FindGeoms()[0]

        #==== The script's own, made again, and the user's, read back ====#
        assert len( vsp.GetSubSurfIDVec( gid ) ) == 2
        assert vsp.FindParm( gid, "Const_Line_Value", "SubSurface_2" ) != ""
        assert vsp.FindParm( gid, "Const_Line_Value", "SubSurface_1" ) == ""
    clear_errors()
