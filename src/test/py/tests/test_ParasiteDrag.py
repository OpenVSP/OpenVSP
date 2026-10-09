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
