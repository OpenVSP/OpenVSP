# Measures, Geometry Analysis cases and bogies name Geoms by ID.

import openvsp as vsp

from testhelp import ( pop_errors, drop_errors, assert_no_errors )


def testAMeasureHasOnlyThePointsItHas():
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    rid = vsp.AddRuler( pod, 0, 0.2, 0.0, pod, 0, 0.8, 0.0, "Span" )
    pid = vsp.AddProbe( pod, 0, 0.5, 0.5, "Probe" )
    drop_errors()
    vsp.GetMeasureMidGeomID( rid )
    assert pop_errors(), "a ruler answered for a middle point"
    vsp.SetMeasureEndGeomID( pid, pod )
    assert pop_errors(), "a probe took an end point"
    vsp.SetMeasureOriginGeomID( rid, "no such geom" )
    assert pop_errors(), "a ruler took a missing Geom"
    assert vsp.GetMeasureOriginGeomID( rid ) == pod
    assert_no_errors()


def testAGeometryAnalysisRefusesWhatIsNotThere():
    vsp.VSPRenew()
    ga = vsp.AddGeometryAnalysis()
    pod = vsp.AddGeom( "POD" )
    drop_errors()
    vsp.SetGeometryAnalysisPrimaryModeID( ga, "no such mode" )
    assert pop_errors(), "a missing mode was taken"
    vsp.SetGeometryAnalysisSecondaryGeomID( ga, "no such geom" )
    assert pop_errors(), "a missing Geom was taken"
    vsp.AddGeometryAnalysisCutout( ga, pod )
    assert pop_errors(), "a Geom was taken as a cutout"
    vsp.DeleteGeometryAnalysisCutout( ga, pod )
    assert pop_errors(), "deleting a cutout that is not there raised no error"
    vsp.SetGeometryAnalysisSecondaryGeomID( ga, "" )
    assert_no_errors()


def testABogieRefusesAMissingGeom():
    vsp.VSPRenew()
    gear = vsp.AddGeom( "GEAR" )
    bogie = vsp.CreateAndAddBogie( gear )
    drop_errors()
    vsp.SetBogieStowParentID( bogie, "no such geom" )
    assert pop_errors(), "a bogie took a missing Geom"
    vsp.GetBogieStowParentID( gear )
    assert pop_errors(), "a Geom was taken as a bogie"
    cloud = vsp.AddGeom( "PTCLOUD" )
    vsp.SetBogieStowParentID( bogie, cloud )
    assert pop_errors(), "a bogie took a point cloud"


def testABogieRefusesAGeomBuiltFromItsGear():
    """SetBogieStowParentID refuses a Geom built from the bogie's gear, with an error."""
    vsp.VSPRenew()
    gear = vsp.AddGeom( "GEAR" )
    child = vsp.AddGeom( "POD", gear )
    bogie = vsp.CreateAndAddBogie( gear )
    drop_errors()
    vsp.SetBogieStowParentID( bogie, child )
    assert pop_errors(), "a bogie took a Geom built from its gear"
    assert vsp.GetBogieStowParentID( bogie ) != child


def testADragReferenceWingIsAWingAndIsUsed():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    pod = vsp.AddGeom( "POD" )
    drop_errors()
    for setter, getter, group in ( ( vsp.SetParasiteDragRefWingID, vsp.GetParasiteDragRefWingID, "ParasiteDrag" ),
                                   ( vsp.SetWaveDragRefWingID, vsp.GetWaveDragRefWingID, "WaveDrag" ) ):
        setter( pod )
        assert pop_errors(), "a Pod was taken as a reference wing"
        setter( wid )
        assert_no_errors()
        assert getter() == wid
        flag = vsp.FindParm( vsp.FindContainer( group + "Settings", 0 ), "RefFlag", group )
        assert vsp.GetIntParmVal( flag ) == vsp.COMPONENT_REF, "setting a reference wing left the reference manual"
