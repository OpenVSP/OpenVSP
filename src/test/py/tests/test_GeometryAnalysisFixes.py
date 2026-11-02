# Geometry Analysis results that came out wrong for a model set up just so.
#
# Each test is the reproduction its fix was found with: the result it gave, and the result it
# gives now.

import os
import xml.etree.ElementTree as ET

import openvsp as vsp
import pytest

from testhelp import pop_errors, drop_errors


def _cube_tris( x0, y0, z0, s, coplanar_last ):
    """The twelve triangles of a cube, its y = y0 and y = y0 + s faces first or last."""
    def p( i, j, k ):
        return ( x0 + i * s, y0 + j * s, z0 + k * s )

    quads = [ ( p( 0, 0, 0 ), p( 1, 0, 0 ), p( 1, 0, 1 ), p( 0, 0, 1 ) ),
              ( p( 0, 1, 0 ), p( 0, 1, 1 ), p( 1, 1, 1 ), p( 1, 1, 0 ) ),
              ( p( 0, 0, 0 ), p( 0, 0, 1 ), p( 0, 1, 1 ), p( 0, 1, 0 ) ),
              ( p( 1, 0, 0 ), p( 1, 1, 0 ), p( 1, 1, 1 ), p( 1, 0, 1 ) ),
              ( p( 0, 0, 0 ), p( 0, 1, 0 ), p( 1, 1, 0 ), p( 1, 0, 0 ) ),
              ( p( 0, 0, 1 ), p( 1, 0, 1 ), p( 1, 1, 1 ), p( 0, 1, 1 ) ) ]
    if coplanar_last:
        quads = quads[2:] + quads[:2]

    tris = []
    for a, b, c, d in quads:
        tris.append( ( a, b, c ) )
        tris.append( ( a, c, d ) )
    return tris


def _write_stl( path, tris ):
    with open( path, "w" ) as f:
        f.write( "solid c\n" )
        for a, b, c in tris:
            u = [ b[k] - a[k] for k in range( 3 ) ]
            v = [ c[k] - a[k] for k in range( 3 ) ]
            n = ( u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2], u[0] * v[1] - u[1] * v[0] )
            m = sum( x * x for x in n ) ** 0.5
            f.write( " facet normal %.17g %.17g %.17g\n  outer loop\n" % tuple( x / m for x in n ) )
            for pt in ( a, b, c ):
                f.write( "   vertex %.17g %.17g %.17g\n" % pt )
            f.write( "  endloop\n endfacet\n" )
        f.write( "endsolid c\n" )


def _case_parm( case, name ):
    return vsp.FindParm( case, name, "InterferenceCase" )


def _evaluate( case ):
    a = "GeometryAnalysis"
    vsp.SetAnalysisInputDefaults( a )
    vsp.SetStringAnalysisInput( a, "CaseID", [ case ] )
    return vsp.ExecAnalysis( a )


def _set_secondary_geom( path, gid ):
    """Point the model's only case at a secondary Geom.  The ID has no API; set it in the file."""
    vsp.WriteVSPFile( path )
    tree = ET.parse( path )
    tree.getroot().find( ".//GeometryAnalysis/SecondaryGeomID" ).text = gid
    tree.write( path )

    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()
    return vsp.GetAllGeometryAnalysesIDVec()[0]


@pytest.mark.parametrize( "coplanar_last", [ False, True ] )
def testCubesThatCrossAndShareAFacePlaneIntersect( coplanar_last, tmp_path ):
    """Two unit cubes from STL, half a side apart in x and z, so their y = 0 and y = 1 faces are
    coplanar and overlap and the cubes also truly cross.

    A coplanar pair met first in a box marked every later pair there coplanar, so with the
    coplanar faces listed first the cubes were reported apart.
    """
    order = "last" if coplanar_last else "first"
    vsp.VSPRenew()
    vsp.ErrorMgrSingleton.getInstance().SilenceErrors()
    paths = []
    for name, x0, z0 in ( ( "a", 0.0, 0.0 ), ( "b", 0.5, 0.5 ) ):
        path = str( tmp_path / ( name + ".stl" ) )
        _write_stl( path, _cube_tris( x0, 0.0, z0, 1.0, coplanar_last ) )
        paths.append( path )

    ga = vsp.ImportFile( paths[0], vsp.IMPORT_STL, "" )
    gb = vsp.ImportFile( paths[1], vsp.IMPORT_STL, "" )
    vsp.SetSetFlag( ga, 3, True )
    vsp.SetSetFlag( gb, 4, True )
    vsp.Update()

    case = vsp.AddGeometryAnalysis()
    vsp.SetParmVal( _case_parm( case, "IntererenceCheckType" ), vsp.EXTERNAL_INTERFERENCE )
    vsp.SetParmVal( _case_parm( case, "PrimarySet" ), 3 )
    vsp.SetParmVal( _case_parm( case, "SecondarySet" ), 4 )
    vsp.Update()

    rid = _evaluate( case )
    drop_errors()
    assert vsp.GetIntResults( rid, "Intersection" )[0] == 1, \
           "crossing cubes with their coplanar faces %s were reported apart" % order
    vol = vsp.GetDoubleResults( rid, "InterferenceVol" )[0]
    assert abs( vol - 0.3125 ) < 1e-9, \
           "crossing cubes with their coplanar faces %s overlap by %g, expected 0.3125" % ( order, vol )


def _turn( steer, tmp_path, drop_bogie = False ):
    """A gear turn analysis of a nose bogie at the origin steered by steer degrees, and two fixed
    mains at x = 12, y = +-4.  Hands back the Gear_Turn results ID, or None, and the errors."""
    vsp.VSPRenew()
    vsp.ErrorMgrSingleton.getInstance().SilenceErrors()
    gear = vsp.AddGeom( "GEAR" )
    bogies = []
    for x, y, angle in ( ( 0.0, 0.0, steer ), ( 12.0, 4.0, 0.0 ), ( 12.0, -4.0, 0.0 ) ):
        b = vsp.CreateAndAddBogie( gear )
        vsp.SetParmVal( vsp.FindParm( b, "XContactPt", "Bogie" ), x )
        vsp.SetParmVal( vsp.FindParm( b, "YContactPt", "Bogie" ), y )
        vsp.SetParmVal( vsp.FindParm( b, "SteeringAngle", "Bogie" ), angle )
        bogies.append( b )
    vsp.Update()

    aux = vsp.AddGeom( "AUXILIARY", gear )
    vsp.SetParmVal( aux, "AuxiliaryGeomType", "Design", vsp.AUX_GEOM_THREE_PT_GROUND )
    vsp.Update()
    for i, b in enumerate( bogies ):
        vsp.SetAuxiliaryGeomContactPtID( aux, i, b )
    vsp.Update()

    case = vsp.AddGeometryAnalysis()
    vsp.SetParmVal( _case_parm( case, "IntererenceCheckType" ), vsp.GEAR_TURN_ANALYSIS )
    vsp.Update()
    case = _set_secondary_geom( str( tmp_path / "turn.vsp3" ), aux )

    if drop_bogie:
        vsp.DelBogie( gear, vsp.GetAllBogies( gear )[0] )
        vsp.Update()

    drop_errors()
    _evaluate( case )
    errors = pop_errors()
    rid = vsp.FindResultsID( "Gear_Turn" )
    if rid and "COR" not in vsp.GetAllDataNames( rid ):
        rid = None
    return rid, errors


def testASteeredGearTurnsAboutWhereTheAxlesMeet( tmp_path ):
    """Steered 30 deg, the nose axle meets the main axle line 24 from the nose wheel."""
    rid, errors = _turn( 30.0, tmp_path )
    assert errors == [], "the turn analysis of a steered gear failed: %s" % errors
    assert rid, "the turn analysis of a steered gear gave no center of rotation"

    cor = vsp.GetVec3dResults( rid, "COR" )[0]
    assert vsp.dist( cor, vsp.vec3d( 12.0, 24.0 * 0.75 ** 0.5, 0.0 ) ) < 1e-9, \
           "the steered gear turns about ( %g, %g, %g )" % ( cor.x(), cor.y(), cor.z() )
    radii = vsp.GetDoubleResults( rid, "GearTurnRadii" )
    expect = ( 24.0, 24.0 * 0.75 ** 0.5 - 4.0, 24.0 * 0.75 ** 0.5 + 4.0 )
    assert all( abs( r - e ) < 1e-9 for r, e in zip( radii, expect ) ) and len( radii ) == 3, \
           "the steered gear's wheels turn on radii %s, expected %s" % ( list( radii ), expect )


@pytest.mark.parametrize( "case, steer, drop_bogie", [ ( "no wheel steered", 0.0, False ),
                                                       ( "a bogie deleted", 30.0, True ) ] )
def testAGearThatCannotTurnSaysSo( case, steer, drop_bogie, tmp_path ):
    """With no wheel steered, the axles are parallel and meet nowhere; with a contact point's
    bogie deleted, there is no wheel to turn.  Either way there is no center of rotation, the
    analysis says so and gives none, rather than one from an unset intersection."""
    rid, errors = _turn( steer, tmp_path, drop_bogie )
    assert rid is None, "with %s, the turn analysis gave a center of rotation" % case
    assert any( "has no center of rotation" in e for e in errors ), \
           "with %s, the turn analysis said: %s" % ( case, errors )


RISK = """
import sys
import xml.etree.ElementTree as ET
import openvsp as vsp
print( "READY", flush = True )
vsp.VSPRenew()
vsp.ErrorMgrSingleton.getInstance().SilenceErrors()
cube = vsp.ImportFile( sys.argv[1], vsp.IMPORT_STL, "" )
vsp.SetSetFlag( cube, 3, True )
aux = vsp.AddGeom( "AUXILIARY" )
vsp.Update()
case = vsp.AddGeometryAnalysis()
vsp.SetParmVal( vsp.FindParm( case, "IntererenceCheckType", "InterferenceCase" ), vsp.RISK_ANGLE )
vsp.SetParmVal( vsp.FindParm( case, "PrimarySet", "InterferenceCase" ), 3 )
vsp.SetParmVal( vsp.FindParm( case, "SecondaryType", "InterferenceCase" ), vsp.GEOM_TARGET )
vsp.Update()
vsp.WriteVSPFile( sys.argv[2] )
tree = ET.parse( sys.argv[2] )
tree.getroot().find( ".//GeometryAnalysis/SecondaryGeomID" ).text = aux
tree.write( sys.argv[2] )
vsp.VSPRenew()
vsp.ReadVSPFile( sys.argv[2] )
vsp.Update()
vsp.SetAnalysisInputDefaults( "GeometryAnalysis" )
vsp.SetStringAnalysisInput( "GeometryAnalysis", "CaseID", vsp.GetAllGeometryAnalysesIDVec() )
rid = vsp.ExecAnalysis( "GeometryAnalysis" )
print( "RESULT %.17g" % vsp.GetDoubleResults( rid, "Result" )[0], flush = True )
"""


def testARiskAngleWithAFaceSquareToTheAxisFinishes( tmp_path ):
    """A unit cube from x = -1.6 to -0.6 straddles the burst axis, so faces of it lie square to
    the axis.  Where such a face's plane holds the axis point no angle was found, the minimum
    took the 1e12 that stands for none, and clamping it into a turn counted down by 2 pi at a
    time: the analysis ran for minutes.  Run in a child process with a time limit."""
    import subprocess
    import sys

    stl = str( tmp_path / "cube.stl" )
    _write_stl( stl, _cube_tris( -1.6, -0.9, -0.9, 1.0, False ) )
    try:
        done = subprocess.run( [ sys.executable, "-c", RISK, stl, str( tmp_path / "risk.vsp3" ) ],
                               capture_output = True, text = True, timeout = 60 )
    except subprocess.TimeoutExpired:
        assert False, "the risk angle analysis of the cube ran past 60 s"

    lines = done.stdout.split( "\n" )
    if "READY" not in lines:
        pytest.skip( "the child could not load OpenVSP: %s" % done.stderr.strip()[-200:] )

    assert done.returncode == 0, "the risk angle analysis of the cube ended with code %d" % done.returncode
    results = [ float( l.split()[1] ) for l in lines if l.startswith( "RESULT " ) ]
    assert len( results ) == 1 and 0.0 <= results[0] <= 360.0, "the risk angle analysis gave %s" % results
