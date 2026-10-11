# Settings a Geometry Analysis case forces from its analysis type.
#
# The case applies these rules when it is evaluated, so a case set up through the API ends up
# where the Geometry Analysis screen would put it.

import openvsp as vsp

import os
import shutil
import tempfile
import xml.etree.ElementTree as ET


def parm( case, name ):
    return vsp.FindParm( case, name, "InterferenceCase" )


def setVal( case, name, val ):
    vsp.SetParmVal( parm( case, name ), val )


def getVal( case, name ):
    return int( vsp.GetParmVal( parm( case, name ) ) )


def evaluate( case ):
    a = "GeometryAnalysis"
    vsp.SetAnalysisInputDefaults( a )
    vsp.SetStringAnalysisInput( a, "CaseID", [ case ] )
    vsp.ExecAnalysis( a )
    mgr = vsp.ErrorMgrSingleton.getInstance()
    for _ in range( mgr.GetNumTotalErrors() ):
        mgr.PopLastError()


def newCase( analysis_type ):
    case = vsp.AddGeometryAnalysis()
    setVal( case, "IntererenceCheckType", analysis_type )
    vsp.Update()
    return case


def setSecondaryGeom( gid ):
    """Point the model's only case at a secondary Geom.  The ID has no API; set it in the file."""
    out = tempfile.mkdtemp()
    try:
        fname = os.path.join( out, "ga.vsp3" )
        vsp.WriteVSPFile( fname )
        tree = ET.parse( fname )
        tree.getroot().find( ".//GeometryAnalysis/SecondaryGeomID" ).text = gid
        tree.write( fname )

        vsp.VSPRenew()
        vsp.ReadVSPFile( fname )
        vsp.Update()
    finally:
        shutil.rmtree( out )
    return vsp.GetAllGeometryAnalysesIDVec()[0]


def makeModel():
    vsp.VSPRenew()
    vsp.ErrorMgrSingleton.getInstance().SilenceErrors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    return pod


def testPlaneMinMaxSecondaryIsAGeomOrAZPlane():
    makeModel()
    case = newCase( vsp.PLANE_STATIC_DISTANCE_INTERFERENCE )
    assert getVal( case, "SecondaryType" ) == vsp.SET_TARGET
    evaluate( case )
    assert getVal( case, "SecondaryType" ) == vsp.GEOM_TARGET

    setVal( case, "SecondaryType", vsp.Z_TARGET )
    evaluate( case )
    assert getVal( case, "SecondaryType" ) == vsp.Z_TARGET


def testAZPlaneOrPointSecondaryIsDroppedWithTheTypeThatTakesIt():
    makeModel()
    case = newCase( vsp.PLANE_STATIC_DISTANCE_INTERFERENCE )
    setVal( case, "SecondaryType", vsp.Z_TARGET )
    setVal( case, "IntererenceCheckType", vsp.EXTERNAL_INTERFERENCE )
    evaluate( case )
    assert getVal( case, "SecondaryType" ) == vsp.SET_TARGET

    setVal( case, "IntererenceCheckType", vsp.VISIBLE_FROM_POINT_ANALYSIS )
    setVal( case, "SecondaryType", vsp.XYZ_TARGET )
    evaluate( case )
    assert getVal( case, "SecondaryType" ) == vsp.XYZ_TARGET

    setVal( case, "IntererenceCheckType", vsp.PACKAGING_INTERFERENCE )
    evaluate( case )
    assert getVal( case, "SecondaryType" ) == vsp.SET_TARGET


def testSingleGeomAnalysesTakeAGeomPrimary():
    makeModel()
    for analysis_type in ( vsp.GEAR_CG_TIPBACK_ANALYSIS, vsp.GEAR_TIPOVER_ANALYSIS,
                           vsp.GEAR_WEIGHT_DISTRIBUTION_ANALYSIS, vsp.AERO_CENTER ):
        case = newCase( analysis_type )
        setVal( case, "PrimaryType", vsp.SET_TARGET )
        evaluate( case )
        assert getVal( case, "PrimaryType" ) == vsp.GEOM_TARGET, analysis_type


def testAModePrimaryWithoutModesIsASet():
    makeModel()
    case = newCase( vsp.COMP_GEOM )
    setVal( case, "PrimaryType", vsp.MODE_TARGET )
    evaluate( case )
    assert getVal( case, "PrimaryType" ) == vsp.SET_TARGET


def testSweptVolumeExtentFollowsTheSecondary():
    makeModel()
    hinge = vsp.AddGeom( "HINGE" )
    vsp.Update()

    case = newCase( vsp.LINEAR_SWEPT_VOLUME_ANALYSIS )

    # A Set secondary moves by the displacement; a slider extent falls back to the first.
    setVal( case, "ExtentType", vsp.EXTENT_SLIDER_FULL )
    evaluate( case )
    assert getVal( case, "ExtentType" ) == vsp.EXTENT_FORWARD_INF

    setVal( case, "ExtentType", vsp.EXTENT_REVERSE_FINITE )
    evaluate( case )
    assert getVal( case, "ExtentType" ) == vsp.EXTENT_REVERSE_FINITE

    # A Hinge secondary that does not translate is moved by the displacement too.
    setVal( case, "SecondaryType", vsp.GEOM_TARGET )
    case = setSecondaryGeom( hinge )
    evaluate( case )
    assert getVal( case, "ExtentType" ) == vsp.EXTENT_REVERSE_FINITE

    # A translating Hinge slides between its limits, so the finite extents go.
    vsp.SetParmVal( hinge, "JointTranslateFlag", "Hinge", 1 )
    vsp.SetParmVal( hinge, "JointTransMinFlag", "Hinge", 1 )
    vsp.SetParmVal( hinge, "JointTransMaxFlag", "Hinge", 1 )
    vsp.Update()
    evaluate( case )
    assert getVal( case, "ExtentType" ) == vsp.EXTENT_FORWARD_INF

    setVal( case, "ExtentType", vsp.EXTENT_SLIDER_FULL )
    evaluate( case )
    assert getVal( case, "ExtentType" ) == vsp.EXTENT_SLIDER_FULL

    # Clearing a limit takes away the full range.
    vsp.SetParmVal( hinge, "JointTransMaxFlag", "Hinge", 0 )
    vsp.Update()
    evaluate( case )
    assert getVal( case, "ExtentType" ) == vsp.EXTENT_FORWARD_INF


if __name__ == "__main__":
    testPlaneMinMaxSecondaryIsAGeomOrAZPlane()
    testAZPlaneOrPointSecondaryIsDroppedWithTheTypeThatTakesIt()
    testSingleGeomAnalysesTakeAGeomPrimary()
    testAModePrimaryWithoutModesIsASet()
    testSweptVolumeExtentFollowsTheSecondary()
