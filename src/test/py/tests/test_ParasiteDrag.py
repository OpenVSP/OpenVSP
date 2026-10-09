# Parasite Drag on models whose Geoms do not line up one-for-one with their DegenGeoms.
#
# Reference length, fineness ratio or t/c, and form factor come from each Geom's DegenGeom.
# They depend only on that Geom's shape, so a Geom must get the same values whatever else is
# in the model.

import openvsp as vsp
import os
import tempfile

from testhelp import ( drop_errors )


GEOM_RESULTS = ( "Comp_Lref", "Comp_FineRat", "Comp_FFOut" )


def fresh():
    vsp.VSPRenew()
    out = tempfile.mkdtemp()
    for t in ( vsp.COMP_GEOM_TXT_TYPE, vsp.COMP_GEOM_CSV_TYPE, vsp.DRAG_BUILD_CSV_TYPE, vsp.DEGEN_GEOM_CSV_TYPE ):
        vsp.SetComputationFileName( t, os.path.join( out, "f%d" % t ) )
    drop_errors()


def build_up():
    """Run the analysis; {label: {result name: value}} for each Geom row."""
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "ParasiteDrag" )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    labels = vsp.GetStringResults( rid, "Comp_Label" )
    rows = {}
    for n in GEOM_RESULTS + ( "Comp_Swet", ):
        vals = vsp.GetDoubleResults( rid, n )
        assert len( vals ) == len( labels ), "%s has %d values for %d rows" % ( n, len( vals ), len( labels ) )
        for lab, v in zip( labels, vals ):
            rows.setdefault( lab, {} )[n] = v
    drop_errors()
    return rows


def place( gid, x ):
    vsp.SetParmVal( gid, "X_Rel_Location", "XForm", x )


def alone( add ):
    """Rows for a model of just the Geoms add() makes."""
    fresh()
    add()
    return build_up()


def a_pod():
    vsp.SetGeomName( vsp.AddGeom( "POD" ), "ThePod" )


def a_wing():
    w = vsp.AddGeom( "WING" )
    vsp.SetGeomName( w, "TheWing" )
    place( w, 50.0 )


def assert_same( got, want, names ):
    for lab in names:
        assert lab in got, "no row for %s: %s" % ( lab, sorted( got ) )
        for n in GEOM_RESULTS:
            assert abs( got[lab][n] - want[lab][n] ) <= 1e-9 * max( 1.0, abs( want[lab][n] ) ), \
                   "%s %s is %g, alone it is %g" % ( lab, n, got[lab][n], want[lab][n] )


def testAnAuxiliaryGeomDoesNotShiftTheOthers():
    want = alone( a_pod )
    fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "ThePod" )
    vsp.AddGeom( "AUXILIARY", pod )
    assert_same( build_up(), want, [ "ThePod" ] )


def testADiskPropFirstInTheModel():
    """Used to loop forever: the disk has a DegenGeom but no row."""
    want = alone( a_pod )
    fresh()
    p = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( p, "PropMode", "Design", vsp.PROP_DISK )
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "ThePod" )
    place( pod, 20.0 )
    assert_same( build_up(), want, [ "ThePod" ] )


def testADiskPropOnTheNoseOfASymmetricModel():
    """Used to overflow the DegenGeom vector."""
    want = alone( a_wing )[ "TheWing" ]
    fresh()
    f = vsp.AddGeom( "FUSELAGE" )
    p = vsp.AddGeom( "PROP", f )
    vsp.SetParmVal( p, "PropMode", "Design", vsp.PROP_DISK )
    names = [ "W", "H", "V" ]
    for i, n in enumerate( names ):
        w = vsp.AddGeom( "WING" )
        vsp.SetGeomName( w, n )
        place( w, 50.0 * ( i + 1 ) )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0 )
    assert_same( build_up(), dict( ( n, want ) for n in names ), names )


def testAPropShowingBladesAndDisk():
    want = alone( a_wing )
    fresh()
    p = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( p, "PropMode", "Design", vsp.PROP_BOTH )
    w = vsp.AddGeom( "WING" )
    vsp.SetGeomName( w, "TheWing" )
    place( w, 50.0 )
    assert_same( build_up(), want, [ "TheWing" ] )


def testAHumanWithManySurfaces():
    want = alone( a_wing )
    fresh()
    h = vsp.AddGeom( "HUMAN" )
    place( h, -50.0 )
    w = vsp.AddGeom( "WING" )
    vsp.SetGeomName( w, "TheWing" )
    place( w, 50.0 )
    assert_same( build_up(), want, [ "TheWing" ] )


def a_prop_with_a_subsurface():
    p = vsp.AddGeom( "PROP" )
    vsp.Update()
    vsp.AddSubSurf( p, vsp.SS_RECTANGLE, 0 )
    return p


def testAMultiBladePropWithASubsurface():
    """The row count took one row per symmetry copy for each subsurface, the rows made one per
    surface: the Geoms after a three-blade Prop lost their rows."""
    want = alone( a_pod )
    fresh()
    a_prop_with_a_subsurface()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "ThePod" )
    place( pod, 20.0 )
    got = build_up()
    assert_same( got, want, [ "ThePod" ] )
    assert abs( got["ThePod"]["Comp_Swet"] - want["ThePod"]["Comp_Swet"] ) < 1e-6 * want["ThePod"]["Comp_Swet"]


def testACloneOfAPropWithASubsurface():
    want = alone( a_pod )
    fresh()
    p = a_prop_with_a_subsurface()
    vsp.Update()
    vsp.CloneGeomVec( [ p ] )
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "ThePod" )
    place( pod, 20.0 )
    assert_same( build_up(), want, [ "ThePod" ] )


def testAnAuxiliaryAlone():
    """CompGeom makes no results for an Auxiliary alone; the wetted areas were read from them."""
    fresh()
    vsp.AddGeom( "AUXILIARY" )
    build_up()


def testDeletingTheResultsBeforeReusingTheGeometry():
    """The wetted areas were read from the CompGeom results each time, so deleting the results
    and then reusing the geometry read freed memory."""
    want = alone( a_pod )
    vsp.DeleteAllResults()
    vsp.SetIntAnalysisInput( "ParasiteDrag", "RecomputeGeom", [ 0 ] )
    rid = vsp.ExecAnalysis( "ParasiteDrag" )
    swet = vsp.GetDoubleResults( rid, "Comp_Swet" )
    assert abs( swet[0] - want["ThePod"]["Comp_Swet"] ) < 1e-9 * want["ThePod"]["Comp_Swet"]


def testAnOlderCompGeomIsNotUsed():
    """A CompGeom that makes nothing must not pick up the last one that did."""
    fresh()
    w = vsp.AddGeom( "WING" )
    vsp.SetGeomName( w, "TheWing" )
    vsp.Update()
    assert build_up()["TheWing"]["Comp_Swet"] > 0.0
    vsp.SetParmVal( w, "Span", "XSec_1", 0.0 )
    vsp.Update()
    swet = build_up()["TheWing"]["Comp_Swet"]
    assert swet == 0.0, "a wing of no span has wetted area %g from the older CompGeom" % swet
