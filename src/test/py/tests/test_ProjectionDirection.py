# Projected area along the direction taken from a Geom.
#
# The direction is minus the normal at u = 0, w = 0 of the Geom's first placed surface.  Each
# area is checked against the same projection along an explicit vector.

import openvsp as vsp

import math
import os
import shutil
import tempfile
import xml.etree.ElementTree as ET


def unit( v ):
    m = math.sqrt( sum( c * c for c in v ) )
    return tuple( c / m for c in v )


def xyz( v ):
    return ( v.x(), v.y(), v.z() )


def project( dirtype, dirgeom = "", vec = None, target = "", mode = "" ):
    """Run the Projection analysis; return ( area, direction ), or None when there is no result."""
    a = "Projection"
    vsp.SetAnalysisInputDefaults( a )
    if mode:
        vsp.SetIntAnalysisInput( a, "TargetType", [ vsp.MODE_TARGET ] )
        vsp.SetStringAnalysisInput( a, "TargetModeID", [ mode ] )
    else:
        vsp.SetIntAnalysisInput( a, "TargetType", [ vsp.GEOM_TARGET ] )
        vsp.SetStringAnalysisInput( a, "TargetGeomID", [ target ] )
    vsp.SetIntAnalysisInput( a, "DirectionType", [ dirtype ] )
    vsp.SetStringAnalysisInput( a, "DirectionGeomID", [ dirgeom ] )
    if vec is not None:
        vsp.SetVec3dAnalysisInput( a, "Direction", [ vsp.vec3d( *vec ) ] )
    rid = vsp.ExecAnalysis( a )
    if not rid:
        return None
    area = vsp.GetDoubleResults( rid, "Area" )[0]
    d = xyz( vsp.GetVec3dResults( rid, "Direction" )[0] )
    mesh = vsp.GetStringResults( rid, "Mesh_GeomID" )
    if mesh and mesh[0]:
        vsp.DeleteGeom( mesh[0] )
    return area, d


def placedDirection( gid ):
    return unit( tuple( -c for c in xyz( vsp.CompNorm01( gid, 0, 0.0, 0.0 ) ) ) )


def assertClose( a, b, tol = 1e-6 ):
    assert abs( a - b ) <= tol * max( 1.0, abs( b ) ), ( a, b )


def assertSameDirection( d, e ):
    assert max( abs( x - y ) for x, y in zip( unit( d ), unit( e ) ) ) < 1e-9, ( d, e )


def popErrors():
    mgr = vsp.ErrorMgrSingleton.getInstance()
    n = mgr.GetNumTotalErrors()
    for _ in range( n ):
        mgr.PopLastError()
    return n


def makeModel():
    """A rotated Pod to project and a disk-mode Propeller tilted 75 deg in pitch."""
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( pod, "Length", "Design", 6.0 )
    vsp.SetParmVal( pod, "Z_Rel_Rotation", "XForm", 15.0 )

    prop = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( prop, "PropMode", "Design", vsp.PROP_DISK )
    vsp.SetParmVal( prop, "X_Rel_Location", "XForm", 2.0 )
    vsp.SetParmVal( prop, "Y_Rel_Rotation", "XForm", 75.0 )
    vsp.Update()
    popErrors()
    return pod, prop


def checkGeomDirection( gid, target ):
    expected = placedDirection( gid )
    area, d = project( vsp.GEOM_PROJ, gid, target = target )
    assertSameDirection( d, expected )
    assertClose( area, project( vsp.VEC_PROJ, vec = expected, target = target )[0] )


def testDirectionFollowsThePlacedSurface():
    pod, prop = makeModel()
    checkGeomDirection( prop, pod )

    # The disk's normal is its axis, which the pitch rotation tilts.
    t = math.radians( 75.0 )
    d = project( vsp.GEOM_PROJ, prop, target = pod )[1]
    assertSameDirection( tuple( abs( c ) for c in d ), ( math.cos( t ), 0.0, math.sin( t ) ) )

    # Through a parent's rotation and the Geom's own, and on a body as well as a disk.
    vsp.SetGeomParent( prop, pod )
    vsp.SetParmVal( prop, "Z_Rel_Rotation", "XForm", 40.0 )
    vsp.Update()
    checkGeomDirection( prop, pod )

    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fuse, "X_Rel_Rotation", "XForm", 20.0 )
    vsp.SetParmVal( fuse, "Y_Rel_Rotation", "XForm", -35.0 )
    vsp.Update()
    checkGeomDirection( fuse, pod )


def testDirectionIsTakenAfterTheMode():
    pod, prop = makeModel()
    p = vsp.FindParm( prop, "Y_Rel_Rotation", "XForm" )
    gid = vsp.AddVarPresetGroup( "Tilt" )
    vsp.AddVarPresetParm( gid, p )
    hover = vsp.AddVarPresetSetting( gid, "Hover" )
    vsp.SetVarPresetParmVal( gid, hover, p, 90.0 )
    cruise = vsp.AddVarPresetSetting( gid, "Cruise" )
    vsp.SetVarPresetParmVal( gid, cruise, p, 0.0 )
    m_hover = vsp.CreateAndAddMode( "HoverMode", vsp.SET_ALL, vsp.SET_NONE )
    vsp.ModeAddGroupSetting( m_hover, gid, hover )
    m_cruise = vsp.CreateAndAddMode( "CruiseMode", vsp.SET_ALL, vsp.SET_NONE )
    vsp.ModeAddGroupSetting( m_cruise, gid, cruise )
    vsp.Update()

    # Each run starts in the other Mode's pose.
    for mid in ( m_hover, m_cruise, m_hover ):
        area, d = project( vsp.GEOM_PROJ, prop, mode = mid )
        expected = placedDirection( prop )
        assertSameDirection( d, expected )
        assertClose( area, project( vsp.VEC_PROJ, vec = expected, mode = mid )[0] )


def evaluateGeometryAnalysis( case ):
    a = "GeometryAnalysis"
    vsp.SetAnalysisInputDefaults( a )
    vsp.SetStringAnalysisInput( a, "CaseID", [ case ] )
    rid = vsp.ExecAnalysis( a )
    return vsp.GetDoubleResults( rid, "Result" )


def testGeometryAnalysisDirectionWithoutItsScreen():
    pod, prop = makeModel()
    vsp.SetSetFlag( pod, vsp.SET_FIRST_USER, True )

    case = vsp.AddGeometryAnalysis()
    vsp.SetParmVal( vsp.FindParm( case, "IntererenceCheckType", "InterferenceCase" ), vsp.PROJ_AREA )
    vsp.SetParmVal( vsp.FindParm( case, "PrimaryType", "InterferenceCase" ), vsp.SET_TARGET )
    vsp.SetParmVal( vsp.FindParm( case, "PrimarySet", "InterferenceCase" ), vsp.SET_FIRST_USER )
    vsp.SetParmVal( vsp.FindParm( case, "DirectionType", "InterferenceCase" ), vsp.GEOM_PROJ )
    vsp.Update()

    out = tempfile.mkdtemp()
    try:
        # The direction Geom's ID has no API; set it in the file.
        fname = os.path.join( out, "proj.vsp3" )
        vsp.WriteVSPFile( fname )
        tree = ET.parse( fname )
        tree.getroot().find( ".//GeometryAnalysis/DirectionGeomID" ).text = prop
        tree.write( fname )

        vsp.VSPRenew()
        vsp.ReadVSPFile( fname )
        vsp.Update()
    finally:
        shutil.rmtree( out )

    pod = vsp.FindGeomsWithName( "PodGeom" )[0]
    prop = vsp.FindGeomsWithName( "PropGeom" )[0]
    case = vsp.GetAllGeometryAnalysesIDVec()[0]

    for yrot in ( None, 30.0 ):
        if yrot is not None:
            vsp.SetParmVal( prop, "Y_Rel_Rotation", "XForm", yrot )
            vsp.Update()
        area = evaluateGeometryAnalysis( case )
        assert len( area ) == 1
        assertClose( area[0], project( vsp.VEC_PROJ, vec = placedDirection( prop ), target = pod )[0] )


def testABadDirectionIsReported():
    pod, prop = makeModel()

    assert project( vsp.GEOM_PROJ, "NoSuchGeom", target = pod ) is None
    assert popErrors() > 0

    # A blade's first surface has no normal at its u = 0, w = 0 corner.
    vsp.SetParmVal( prop, "PropMode", "Design", vsp.PROP_BLADES )
    vsp.Update()
    popErrors()
    assert project( vsp.GEOM_PROJ, prop, target = pod ) is None
    assert popErrors() > 0

    assert project( vsp.VEC_PROJ, vec = ( 0.0, 0.0, 0.0 ), target = pod ) is None
    assert popErrors() > 0


if __name__ == "__main__":
    testDirectionFollowsThePlacedSurface()
    testDirectionIsTakenAfterTheMode()
    testGeometryAnalysisDirectionWithoutItsScreen()
    testABadDirectionIsReported()
