# Each API call sets the error flag when it fails and clears it when it succeeds.

import os
import tempfile

import openvsp as vsp

from testhelp import ( pop_errors, drop_errors, assert_no_errors )


def last_call_failed():
    return vsp.ErrorMgrSingleton.getInstance().GetErrorLastCallFlag()


def fail_a_call():
    vsp.GetGeomName( "no such geom" )
    assert last_call_failed()
    drop_errors()


def testDeletingAPCurveEndPointIsRefused():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.Update()
    vsp.PCurveConvertTo( pid, vsp.PROP_CHORD, vsp.LINEAR )
    n = len( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) )
    drop_errors()

    vsp.PCurveDeletePt( pid, vsp.PROP_CHORD, 0 )
    assert last_call_failed()
    vsp.PCurveDeletePt( pid, vsp.PROP_CHORD, n - 1 )
    assert last_call_failed()
    vsp.PCurveDeletePt( pid, vsp.PROP_CHORD, n )
    assert last_call_failed()
    drop_errors()
    assert len( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == n

    fail_a_call()
    vsp.PCurveDeletePt( pid, vsp.PROP_CHORD, 1 )
    assert not last_call_failed(), pop_errors()
    assert len( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == n - 1


def testDeletingACEDITControlPointIsRefused():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.Update()
    vsp.PCurveConvertTo( pid, vsp.PROP_CHORD, vsp.CEDIT )
    n = len( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) )
    assert n > 4, n
    drop_errors()

    vsp.PCurveDeletePt( pid, vsp.PROP_CHORD, 1 )
    assert last_call_failed()
    drop_errors()
    assert len( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == n

    vsp.PCurveDeletePt( pid, vsp.PROP_CHORD, 3 )
    assert not last_call_failed(), pop_errors()
    assert len( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == n - 3


def testActuatorDiskAndUnsteadyGroupLookupsClearTheErrorFlag():
    vsp.VSPRenew()
    vsp.AddGeom( "PROP" )
    disk = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( disk, "PropMode", "Design", vsp.PROP_DISK )
    vsp.Update()
    drop_errors()

    fail_a_call()
    assert vsp.FindActuatorDisk( 0 )
    assert not last_call_failed(), pop_errors()

    fail_a_call()
    assert vsp.FindUnsteadyGroup( 0 )
    assert not last_call_failed(), pop_errors()

    fail_a_call()
    assert vsp.GetUnsteadyGroupName( 0 )
    assert not last_call_failed(), pop_errors()


def testControlSurfaceGroupGettersClearTheErrorFlag():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.AddSubSurf( wid, vsp.SS_CONTROL )
    vsp.Update()
    vsp.AutoGroupVSPAEROControlSurfaces()
    drop_errors()
    assert vsp.GetNumControlSurfaceGroups() > 0

    fail_a_call()
    assert vsp.GetVSPAEROControlGroupName( 0 )
    assert not last_call_failed(), pop_errors()

    fail_a_call()
    assert vsp.GetActiveCSNameVec( 0 )
    assert not last_call_failed(), pop_errors()

    fail_a_call()
    assert vsp.GetCompleteCSNameVec()
    assert not last_call_failed(), pop_errors()

    fail_a_call()
    vsp.GetAvailableCSNameVec( 0 )
    assert not last_call_failed(), pop_errors()


def testAModeSettingPastTheLastIsAnError():
    vsp.VSPRenew()
    vsp.AddGeom( "POD" )
    mid = vsp.CreateAndAddMode( "Mode", vsp.SET_ALL, vsp.SET_NONE )
    drop_errors()

    vsp.ModeGetGroup( mid, 0 )
    assert last_call_failed()
    vsp.ModeGetSetting( mid, 0 )
    assert last_call_failed()
    vsp.RemoveGroupSetting( mid, 0 )
    assert last_call_failed()
    drop_errors()


def testAMainSurfacePastTheLastIsAnError():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.Update()
    drop_errors()

    assert vsp.GetGeomVSPSurfType( wid, 1 ) == -1
    assert last_call_failed()
    assert vsp.GetGeomVSPSurfCfdType( wid, 1 ) == -1
    assert last_call_failed()
    drop_errors()

    assert vsp.GetGeomVSPSurfType( wid, 0 ) == vsp.WING_SURF
    assert not last_call_failed(), pop_errors()
    assert vsp.GetGeomVSPSurfCfdType( wid, 0 ) == vsp.CFD_NORMAL
    assert not last_call_failed(), pop_errors()


def testControlSurfaceGroupEditsClearTheErrorFlag():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.AddSubSurf( wid, vsp.SS_CONTROL )
    vsp.Update()
    vsp.CreateVSPAEROControlSurfaceGroup()
    drop_errors()

    fail_a_call()
    vsp.AddAllToVSPAEROControlSurfaceGroup( 0 )
    assert not last_call_failed(), pop_errors()

    fail_a_call()
    vsp.RemoveSelectedFromCSGroup( [ 1 ], 0 )
    assert not last_call_failed(), pop_errors()

    fail_a_call()
    vsp.AddSelectedToCSGroup( [ 1 ], 0 )
    assert not last_call_failed(), pop_errors()

    fail_a_call()
    vsp.RemoveAllFromVSPAEROControlSurfaceGroup( 0 )
    assert not last_call_failed(), pop_errors()

    fail_a_call()
    vsp.SetVSPAEROControlGroupName( "Flaps", 0 )
    assert not last_call_failed(), pop_errors()


def testUnsteadyGroupVectorsClearTheErrorFlag():
    vsp.VSPRenew()
    vsp.AddGeom( "PROP" )
    vsp.Update()
    drop_errors()

    fail_a_call()
    assert vsp.GetUnsteadyGroupCompIDs( 0 )
    assert not last_call_failed(), pop_errors()

    fail_a_call()
    assert vsp.GetUnsteadyGroupSurfIndexes( 0 )
    assert not last_call_failed(), pop_errors()


def testACallThatSucceedsClearsTheErrorFlag():
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    prop = vsp.AddGeom( "PROP" )
    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    xsec_surf = vsp.GetXSecSurf( fuse, 0 )
    vsp.WriteTestResults()
    res = vsp.FindLatestResultsID( "Test_Results" )
    out = tempfile.mkdtemp()
    pts = vsp.GetEllipsoidSurfPnts( vsp.vec3d( 0, 0, 0 ), vsp.vec3d( 1, 2, 3 ) )
    vkt = vsp.GetVKTAirfoilPnts( 20, 0.0, 0.1, 0.0, 0.0 )

    calls = [
        ( "GetAndResetUpdateCount", lambda: vsp.GetAndResetUpdateCount() ),
        ( "SetBEMPropID", lambda: vsp.SetBEMPropID( prop ) ),
        ( "GetVSPAERORefWingID", lambda: vsp.GetVSPAERORefWingID() ),
        ( "AutoGroupVSPAEROControlSurfaces", lambda: vsp.AutoGroupVSPAEROControlSurfaces() ),
        ( "CreateVSPAEROControlSurfaceGroup", lambda: vsp.CreateVSPAEROControlSurfaceGroup() ),
        ( "GetNumControlSurfaceGroups", lambda: vsp.GetNumControlSurfaceGroups() ),
        ( "GetNumUnsteadyGroups", lambda: vsp.GetNumUnsteadyGroups() ),
        ( "GetNumUnsteadyRotorGroups", lambda: vsp.GetNumUnsteadyRotorGroups() ),
        ( "GetNumActuatorDisks", lambda: vsp.GetNumActuatorDisks() ),
        ( "GetNumAnalysis", lambda: vsp.GetNumAnalysis() ),
        ( "ListAnalysis", lambda: vsp.ListAnalysis() ),
        ( "GetAnalysisInputNames", lambda: vsp.GetAnalysisInputNames( "CompGeom" ) ),
        ( "GetAnalysisDoc", lambda: vsp.GetAnalysisDoc( "CompGeom" ) ),
        ( "SummarizeAttributes", lambda: vsp.SummarizeAttributes() ),
        ( "SummarizeAttributesAsTree", lambda: vsp.SummarizeAttributesAsTree() ),
        ( "FindAllAttributes", lambda: vsp.FindAllAttributes() ),
        ( "FindAttributesByName", lambda: vsp.FindAttributesByName( "x" ) ),
        ( "FindAttributeByName", lambda: vsp.FindAttributeByName( "x", 0 ) ),
        ( "FindAttributedObjects", lambda: vsp.FindAttributedObjects() ),
        ( "GetGeomSetCollection", lambda: vsp.GetGeomSetCollection( 0 ) ),
        ( "GetAllResultsNames", lambda: vsp.GetAllResultsNames() ),
        ( "GetAllDataNames", lambda: vsp.GetAllDataNames( res ) ),
        ( "GetNumResults", lambda: vsp.GetNumResults( "Test_Results" ) ),
        ( "GetResultsName", lambda: vsp.GetResultsName( res ) ),
        ( "GetResultsSetDoc", lambda: vsp.GetResultsSetDoc( res ) ),
        ( "WriteTestResults", lambda: vsp.WriteTestResults() ),
        ( "GetGeomTypeName", lambda: vsp.GetGeomTypeName( pod ) ),
        ( "GetGeomParent", lambda: vsp.GetGeomParent( pod ) ),
        ( "GetGeomChildren", lambda: vsp.GetGeomChildren( pod ) ),
        ( "GetGeomBBoxMax", lambda: vsp.GetGeomBBoxMax( pod ) ),
        ( "GetGeomBBoxMin", lambda: vsp.GetGeomBBoxMin( pod ) ),
        ( "NumFeaStructures", lambda: vsp.NumFeaStructures() ),
        ( "GetXSecSurfGlobalXForm", lambda: vsp.GetXSecSurfGlobalXForm( xsec_surf ) ),
        ( "SetXSecSurfGlobalXForm", lambda: vsp.SetXSecSurfGlobalXForm( xsec_surf, vsp.Matrix4d() ) ),
        ( "GetHersheyBarLiftDist", lambda: vsp.GetHersheyBarLiftDist( 10, 0.1, 1.0, 10.0 ) ),
        ( "GetHersheyBarDragDist", lambda: vsp.GetHersheyBarDragDist( 10, 0.1, 1.0, 10.0 ) ),
        ( "GetVKTAirfoilPnts", lambda: vsp.GetVKTAirfoilPnts( 20, 0.0, 0.1, 0.0, 0.0 ) ),
        ( "GetVKTAirfoilCpDist", lambda: vsp.GetVKTAirfoilCpDist( 0.0, 0.1, 0.0, 0.0, vkt ) ),
        ( "GetEllipsoidSurfPnts", lambda: vsp.GetEllipsoidSurfPnts( vsp.vec3d( 0, 0, 0 ), vsp.vec3d( 1, 2, 3 ) ) ),
        ( "GetEllipsoidCpDist", lambda: vsp.GetEllipsoidCpDist( pts, vsp.vec3d( 1, 2, 3 ), vsp.vec3d( 1, 0, 0 ) ) ),
        ( "IntegrateEllipsoidFlow", lambda: vsp.IntegrateEllipsoidFlow( vsp.vec3d( 1, 2, 3 ), 0 ) ),
        ( "ValidParm", lambda: vsp.ValidParm( "BADID" ) ),
        ( "WriteAtmosphereCSVFile", lambda: vsp.WriteAtmosphereCSVFile( os.path.join( out, "atmos.csv" ), vsp.ATMOS_TYPE_US_STANDARD_1976 ) ),
        ( "WriteBodyFFCSVFile", lambda: vsp.WriteBodyFFCSVFile( os.path.join( out, "body.csv" ) ) ),
        ( "WriteWingFFCSVFile", lambda: vsp.WriteWingFFCSVFile( os.path.join( out, "wing.csv" ) ) ),
        ( "WriteCfEqnCSVFile", lambda: vsp.WriteCfEqnCSVFile( os.path.join( out, "cf.csv" ) ) ),
        ( "WritePartialCfMethodCSVFile", lambda: vsp.WritePartialCfMethodCSVFile( os.path.join( out, "pcf.csv" ) ) ),
        ( "Min", lambda: vsp.Min( 1.0, 2.0 ) ),
        ( "Max", lambda: vsp.Max( 1.0, 2.0 ) ),
        ( "Rad2Deg", lambda: vsp.Rad2Deg( 1.0 ) ),
        ( "Deg2Rad", lambda: vsp.Deg2Rad( 1.0 ) ),
        ( "GetVSPVersion", lambda: vsp.GetVSPVersion() ),
        ( "GetVSPVersionMajor", lambda: vsp.GetVSPVersionMajor() ),
        ( "GetVSPVersionMinor", lambda: vsp.GetVSPVersionMinor() ),
        ( "GetVSPVersionChange", lambda: vsp.GetVSPVersionChange() ),
        ( "GetVSPExePath", lambda: vsp.GetVSPExePath() ),
        ( "GetVSPAEROPath", lambda: vsp.GetVSPAEROPath() ),
        ( "CheckForVSPAERO", lambda: vsp.CheckForVSPAERO( vsp.GetVSPAEROPath() ) ),
        ( "GetVSPHelpPath", lambda: vsp.GetVSPHelpPath() ),
        ( "CheckForVSPHelp", lambda: vsp.CheckForVSPHelp( vsp.GetVSPHelpPath() ) ),
        ( "GetAllProtractors", lambda: vsp.GetAllProtractors() ),
        ( "DeleteAllProtractors", lambda: vsp.DeleteAllProtractors() ),
        ( "GetAllRulers", lambda: vsp.GetAllRulers() ),
        ( "DeleteAllRulers", lambda: vsp.DeleteAllRulers() ),
        ( "GetAllProbes", lambda: vsp.GetAllProbes() ),
        ( "DeleteAllProbes", lambda: vsp.DeleteAllProbes() ),
        ( "GetAllRSTProbes", lambda: vsp.GetAllRSTProbes() ),
        ( "DeleteAllRSTProbes", lambda: vsp.DeleteAllRSTProbes() ),
        ( "GetNumSets", lambda: vsp.GetNumSets() ),
        ( "GetComputationFileName", lambda: vsp.GetComputationFileName( vsp.CFD_STL_TYPE ) ),
    ]
    stuck = []
    for name, call in calls:
        vsp.GetGeomName( "BADID" )
        drop_errors()
        call()
        if last_call_failed():
            stuck.append( name )
    assert not stuck, stuck
