# Projected area and visibility from a point, the two analyses built on Clipper2.
#
# Projected areas are checked against closed-form areas of the tessellated shapes.  Visibility
# fractions are checked against what symmetry and complementing the domain require.

import math

import openvsp as vsp
import pytest


AFT_OCTANTS = ( 0, 3, 4, 7 )
FWD_OCTANTS = ( 1, 2, 5, 6 )


def newModel():
    vsp.VSPRenew()
    vsp.ErrorMgrSingleton.getInstance().SilenceErrors()


def projectedArea( gid, direction ):
    vsp.SetAnalysisInputDefaults( "Projection" )
    vsp.SetIntAnalysisInput( "Projection", "TargetType", [ vsp.GEOM_TARGET ] )
    vsp.SetStringAnalysisInput( "Projection", "TargetGeomID", [ gid ] )
    vsp.SetIntAnalysisInput( "Projection", "DirectionType", [ direction ] )
    rid = vsp.ExecAnalysis( "Projection" )
    return vsp.GetDoubleResults( rid, "Area" )[0]


def addEllipsoid( a, b, c, tess ):
    gid = vsp.AddGeom( "ELLIPSOID" )
    vsp.SetParmVal( gid, "A_Radius", "Design", a )
    vsp.SetParmVal( gid, "B_Radius", "Design", b )
    vsp.SetParmVal( gid, "C_Radius", "Design", c )
    vsp.SetParmVal( gid, "Tess_U", "Shape", tess )
    vsp.SetParmVal( gid, "Tess_W", "Shape", tess )
    vsp.Update()
    return gid


def octantFractions( x, y, z, poly_visible ):
    case = vsp.AddGeometryAnalysis()
    vsp.SetParmVal( vsp.FindParm( case, "IntererenceCheckType", "InterferenceCase" ), vsp.VISIBLE_FROM_POINT_ANALYSIS )
    vsp.Update()
    for name, val in ( ( "SecondaryType", vsp.XYZ_TARGET ), ( "SecondaryX", x ), ( "SecondaryY", y ),
                       ( "SecondaryZ", z ), ( "PolyVisibleFlag", poly_visible ) ):
        vsp.SetParmVal( vsp.FindParm( case, name, "InterferenceCase" ), val )
    vsp.SetAnalysisInputDefaults( "GeometryAnalysis" )
    vsp.SetStringAnalysisInput( "GeometryAnalysis", "CaseID", [ case ] )
    vsp.ExecAnalysis( "GeometryAnalysis" )
    rid = vsp.FindLatestResultsID( "Point_Visibility" )
    return vsp.GetDoubleResults( rid, "Octant_Area_Frac" )


def sphereSeenFrom( distance, poly_visible ):
    """Fractions seen from the origin of a unit sphere centered distance along +X."""
    newModel()
    gid = addEllipsoid( 1.0, 1.0, 1.0, 49 )
    vsp.SetParmVal( gid, "X_Rel_Location", "XForm", distance )
    vsp.Update()
    return octantFractions( 0.0, 0.0, 0.0, poly_visible )


def testARectangularWingProjectsToItsPlanform():
    newModel()
    wid = vsp.AddGeom( "WING" )
    vsp.SetDriverGroup( wid, 1, vsp.SPAN_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
    vsp.SetParmVal( wid, "Span", "XSec_1", 4.0 )
    vsp.SetParmVal( wid, "Root_Chord", "XSec_1", 1.5 )
    vsp.SetParmVal( wid, "Tip_Chord", "XSec_1", 1.5 )
    vsp.SetParmVal( wid, "Sweep", "XSec_1", 0.0 )
    vsp.Update()
    assert projectedArea( wid, vsp.Z_PROJ ) == pytest.approx( 2.0 * 4.0 * 1.5, rel = 1e-12 )


def testAnEllipsoidProjectsToItsEllipse():
    newModel()
    gid = addEllipsoid( 3.0, 1.5, 0.75, 32 )
    # Along X the outline is the polygon of the widest section, 32 sides inscribed in the ellipse.
    sides = 32
    inscribed = 0.5 * sides * math.sin( 2.0 * math.pi / sides ) * 1.5 * 0.75
    assert projectedArea( gid, vsp.X_PROJ ) == pytest.approx( inscribed, rel = 1e-3 )
    assert projectedArea( gid, vsp.Z_PROJ ) == pytest.approx( math.pi * 3.0 * 1.5, rel = 1e-3 )


@pytest.mark.parametrize( "distance", [ 2.0, 4.0 ] )
def testASphereAftIsSeenInTheFourAftOctantsAlike( distance ):
    frac = sphereSeenFrom( distance, 0 )
    for i in FWD_OCTANTS:
        assert frac[i] == 0.0
    assert frac[0] > 0.0
    for i in AFT_OCTANTS:
        assert frac[i] == pytest.approx( frac[0], rel = 1e-9 )


def testTheVisibleAndOccludedFractionsMakeTheWhole():
    occluded = sphereSeenFrom( 2.0, 0 )
    visible = sphereSeenFrom( 2.0, 1 )
    for i in range( 8 ):
        assert occluded[i] + visible[i] == pytest.approx( 1.0, abs = 1e-9 )


def testANearerSphereHidesMore():
    assert sphereSeenFrom( 2.0, 0 )[0] > sphereSeenFrom( 4.0, 0 )[0]


@pytest.mark.xfail( strict = True, reason = "the octant fraction is not the fraction of the octant's solid angle" )
def testTheFractionIsTheSolidAngleFraction():
    # The sphere hides a cone of half angle asin( 1 / d ), split evenly over the four aft octants,
    # each of solid angle pi / 2.
    d = 2.0
    expected = 1.0 - math.sqrt( 1.0 - 1.0 / ( d * d ) )
    assert sphereSeenFrom( d, 0 )[0] == pytest.approx( expected, rel = 0.02 )


@pytest.mark.xfail( strict = True, reason = "a sphere straight below is seen only in the aft lower octants" )
def testASphereBelowIsSeenInTheFourLowerOctantsAlike():
    newModel()
    addEllipsoid( 1.0, 1.0, 1.0, 49 )
    frac = octantFractions( 0.0, 0.0, 2.0, 0 )
    for i in ( 4, 5, 6, 7 ):
        assert frac[i] == pytest.approx( frac[4], rel = 1e-6 )
