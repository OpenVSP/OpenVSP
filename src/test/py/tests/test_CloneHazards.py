# Cases a Clone has to survive: bad files, a Geom deleted from under it, and borrowed shapes
# that are not surfaces.

import openvsp as vsp
import pytest
import os
import re
import subprocess
import sys
import tempfile
import xml.etree.ElementTree

from clonehelp import ( box, switch, comp_geom_areas_of,
                        total_mass, scratch_output, drop_errors,
                        assert_refused, assert_no_errors, a_mesh,
                        a_wireframe, degen_rows )


def testAFileNamingARingOfClonesStillLoads():
    """A file can name two Clones as each other's original; reading it must not loop forever,
    since the chain of originals is walked before an update can refuse the ring."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    first = vsp.CloneGeomVec( [ pod ] )[0]
    second = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()

    out = tempfile.mkdtemp()
    good = os.path.join( out, "ring.vsp3" )
    vsp.WriteVSPFile( good )

    # Point each Clone at the other.
    text = open( good ).read()
    halves = text.split( "<OriginalID>%s</OriginalID>" % pod )
    assert len( halves ) == 3, "expected two Clones naming the pod"
    ringed = ( halves[0] + "<OriginalID>%s</OriginalID>" % second +
               halves[1] + "<OriginalID>%s</OriginalID>" % first + halves[2] )
    bad = os.path.join( out, "ring_edited.vsp3" )
    open( bad, "w" ).write( ringed )

    # Read in a subprocess with a time limit, so an endless walk fails rather than hangs.
    reader = """
import sys
import openvsp as vsp
print( "READY" )
sys.stdout.flush()
vsp.ReadVSPFile( %r )
vsp.Update()
clones = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Clone" ]
named = [ vsp.GetGeomCloneOriginal( c ) for c in clones ]
print( "CLONES", len( clones ) )
print( "RING", named[0] in clones and named[1] in clones )
mgr = vsp.ErrorMgrSingleton.getInstance()
said = [ mgr.PopLastError().m_ErrorString for _ in range( mgr.GetNumTotalErrors() ) ]
print( "SAID", any( "hangs off" in m for m in said ) )
""" % bad

    try:
        done = subprocess.run( [ sys.executable, "-c", reader ], capture_output = True,
                               text = True, timeout = 120 )
    except subprocess.TimeoutExpired:
        assert False, "reading a file that names a ring of Clones never finished"

    lines = done.stdout.split( "\n" )

    # Without READY the subprocess never loaded OpenVSP (e.g. a sanitizer build), which is a
    # setup failure, not the ring.
    if "READY" not in lines:
        pytest.skip( "the child could not load OpenVSP: %s" % done.stderr.strip()[-200:] )

    assert done.returncode >= 0, \
           "reading a file that names a ring of Clones died on signal %d" % -done.returncode
    assert "CLONES 2" in lines, "the file did not load: %s %s" % ( done.stdout, done.stderr[-200:] )
    assert "RING False" in lines, "both Clones still name each other, so the ring survived the read"

    # Breaking the ring leaves a Clone empty, reported with the wording for an original that is
    # present but cannot be copied.
    assert "SAID True" in lines, \
           "the ring was broken without saying so: %s" % done.stdout
    drop_errors()


def testACloneOfAWireFrameCarriesItsTrianglesIntoAnAnalysis():
    """A Clone of a wireframe is meshed into triangles, two per cell, for analyses."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wire = a_wireframe()
    clone = vsp.CloneGeomVec( [ wire ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 20.0 )
    vsp.Update()

    areas = comp_geom_areas_of( [ wire, clone ] )
    assert areas[ vsp.GetGeomName( wire ) ] > 0.0
    assert areas[ vsp.GetGeomName( clone ) ] == pytest.approx( areas[ vsp.GetGeomName( wire ) ] )
    assert_no_errors()


def testACloneOfAWireFrameReportsTheKindOfSurfaceItIs():
    """Whether a wireframe Clone is lifting comes from the grid's type."""
    for wire_type, expected in ( ( 0, "LIFTING_SURFACE" ), ( 1, "BODY" ) ):
        vsp.VSPRenew()
        drop_errors()
        scratch_output()
        wire = a_wireframe()
        vsp.SetParmVal( vsp.FindParm( wire, "WireType", "WireFrame" ), wire_type )
        vsp.Update()
        clone = vsp.CloneGeomVec( [ wire ] )[0]
        vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 20.0 )
        vsp.Update()

        vsp.ComputeDegenGeom( vsp.SET_ALL, 0 )
        kinds = { r[0] : r[1] for r in degen_rows() }

        assert kinds[ vsp.GetGeomName( wire ) ] == expected
        assert kinds[ vsp.GetGeomName( clone ) ] == expected
        assert_no_errors()


def testAWireFrameSaysWhichWayItFacesTheSameWayEverywhere():
    """Rearranging a grid turns it over, and the triangles, normals and degenerate entry agree."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wire = a_wireframe()
    clone = vsp.CloneGeomVec( [ wire ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 20.0 )
    vsp.Update()

    def flips():
        # Clear results so an earlier run's entries are not read.
        vsp.DeleteAllResults()
        vsp.ComputeDegenGeom( vsp.SET_ALL, 0 )
        n = vsp.GetNumResults( "Degen_DegenGeom" )
        assert n == 2, "expected one entry each for the Geom and its Clone, got %d" % n

        out = {}
        for i in range( n ):
            res = vsp.FindResultsID( "Degen_DegenGeom", i )
            name = list( vsp.GetStringResults( res, "name" ) )[0]
            out[ name ] = list( vsp.GetIntResults( res, "flip_normal" ) )[0]
        return out

    before = flips()
    assert vsp.GetGeomName( wire ) in before and vsp.GetGeomName( clone ) in before

    # Swapping the grid's two directions turns the surface over.
    vsp.SetParmVal( vsp.FindParm( wire, "FlipIJFlag", "Wireframe" ), 1.0 )
    vsp.Update()
    after = flips()

    assert after[ vsp.GetGeomName( wire ) ] != before[ vsp.GetGeomName( wire ) ], \
           "swapping the grid did not change which way the Geom says it faces"
    assert after[ vsp.GetGeomName( clone ) ] == after[ vsp.GetGeomName( wire ) ], \
           "the Clone and the Geom disagree about which way they face"
    assert_no_errors()


def testACloneOfABorrowedShapeWeighsOnce():
    """A Clone of a mesh or wireframe ignores symmetry, as its original does, so its point mass
    is counted once."""
    for maker in ( a_mesh, a_wireframe ):
        vsp.VSPRenew()
        drop_errors()
        scratch_output()
        shape = maker()
        vsp.SetParmVal( vsp.FindParm( shape, "Density", "Mass_Props" ), 0.0 )
        vsp.Update()
        clone = vsp.CloneGeomVec( [ shape ] )[0]
        vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 30.0 )
        vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 6.0 )
        vsp.Update()
        switch( clone, "CloneMassProps", False )
        switch( clone, "CloneSym", False )
        vsp.SetParmVal( vsp.FindParm( clone, "PointMass", "Mass_Props" ), 5.0 )
        vsp.SetParmVal( vsp.FindParm( clone, "Density", "Mass_Props" ), 0.0 )
        vsp.Update()

        plain = total_mass()
        assert plain == pytest.approx( 5.0 )

        y_before = vsp.GetParmVal( vsp.FindParm( clone, "Y_Len", "BBox" ) )
        vsp.SetParmVal( vsp.FindParm( clone, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
        vsp.Update()

        # The shape is not copied, so the mass is not either.
        assert vsp.GetParmVal( vsp.FindParm( clone, "Y_Len", "BBox" ) ) == pytest.approx( y_before )
        assert total_mass() == pytest.approx( plain )
        assert_no_errors()


def testACloneOfASurfaceGeomStillMirrorsAndWeighsTwice():
    """A Clone of a surface Geom still gets a symmetric copy, and its point mass counts twice."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( pod, "Density", "Mass_Props" ), 0.0 )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 30.0 )
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 6.0 )
    vsp.Update()
    switch( clone, "CloneMassProps", False )
    switch( clone, "CloneSym", False )
    vsp.SetParmVal( vsp.FindParm( clone, "PointMass", "Mass_Props" ), 5.0 )
    vsp.SetParmVal( vsp.FindParm( clone, "Density", "Mass_Props" ), 0.0 )
    vsp.Update()

    y_before = vsp.GetParmVal( vsp.FindParm( clone, "Y_Len", "BBox" ) )
    plain = total_mass()
    vsp.SetParmVal( vsp.FindParm( clone, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()

    # The Clone at y = 6 and its copy at -6 span both sides of the plane.
    assert vsp.GetParmVal( vsp.FindParm( clone, "Y_Len", "BBox" ) ) == \
           pytest.approx( 2.0 * ( 6.0 + 0.5 * y_before ) )
    assert total_mass() == pytest.approx( 2.0 * plain )
    assert_no_errors()


def testLosingItsOriginalDoesNotMakeItACloneOfSomethingElse():
    """A Clone whose original is deleted stays empty; it does not take its parent as original."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = vsp.AddGeom( "WING" )
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.AddGeom( "CLONE", wing )
    vsp.SetGeomCloneOriginal( clone, pod )
    vsp.Update()
    assert vsp.GetGeomCloneOriginal( clone ) == pod
    assert box( clone )[1] == pytest.approx( box( pod )[1] )

    vsp.DeleteGeom( pod )
    vsp.Update()
    vsp.Update()

    assert vsp.GetGeomCloneOriginal( clone ) == ""
    assert box( clone )[1] != pytest.approx( box( wing )[1] ), "it took its parent instead"
    assert box( clone ) == pytest.approx( ( 0.0, ) * 6 )

    # Empty, and reported.
    assert_refused( "lost the Geom it was copying" )


def testAnOriginalCanBeClearedThroughTheApi():
    """Clearing the original is not an error, and the cleared Clone does not take its parent."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = vsp.AddGeom( "WING" )
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.AddGeom( "CLONE", wing )
    vsp.SetGeomCloneOriginal( clone, pod )
    vsp.Update()
    assert vsp.GetGeomCloneOriginal( clone ) == pod

    vsp.SetGeomCloneOriginal( clone, "" )
    vsp.Update()
    vsp.Update()

    assert vsp.GetGeomCloneOriginal( clone ) == ""
    assert box( clone ) == pytest.approx( ( 0.0, ) * 6 )
    assert_no_errors()


def testAClearedOriginalStaysClearedThroughAFile():
    """A cleared original is written and read back empty, not as "not chosen yet"."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = vsp.AddGeom( "WING" )
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.AddGeom( "CLONE", wing )
    vsp.SetGeomCloneOriginal( clone, pod )
    vsp.Update()
    vsp.SetGeomCloneOriginal( clone, "" )
    vsp.Update()
    assert vsp.GetGeomCloneOriginal( clone ) == ""

    path = os.path.join( tempfile.mkdtemp(), "cleared.vsp3" )
    vsp.WriteVSPFile( path )
    vsp.VSPRenew()
    scratch_output()
    vsp.ReadVSPFile( path )
    vsp.Update()
    vsp.Update()

    reloaded = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Clone" ][0]
    assert vsp.GetGeomCloneOriginal( reloaded ) == "", "it took its parent on the way back in"
    assert box( reloaded ) == pytest.approx( ( 0.0, ) * 6 )
    assert_no_errors()


def testAStepChildListThatIsAlreadyTooLongComesBackPruned():
    """Duplicate step-child entries in a file are removed on read."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.CloneGeomVec( [ pod ] )
    vsp.Update()

    out = tempfile.mkdtemp()
    path = os.path.join( out, "step.vsp3" )
    vsp.WriteVSPFile( path )

    # Triple every entry.
    text = open( path ).read()
    def triple( match ):
        inner = match.group( 1 )
        return "<Step_Child_List>" + inner * 3 + "</Step_Child_List>"
    bloated = re.sub( r"<Step_Child_List>(.*?)</Step_Child_List>", triple, text, flags = re.S )
    bad = os.path.join( out, "step_bloated.vsp3" )
    open( bad, "w" ).write( bloated )
    assert bloated != text

    vsp.VSPRenew()
    scratch_output()
    vsp.ReadVSPFile( bad )
    vsp.Update()
    vsp.WriteVSPFile( path )

    after = open( path ).read()
    for listing in re.findall( r"<Step_Child_List>(.*?)</Step_Child_List>", after, re.S ):
        ids = re.findall( r"<ID>(\w+)</ID>", listing )
        assert len( ids ) == len( set( ids ) ), "duplicates survived the read: %s" % ids
    drop_errors()


def testTheStepChildListDoesNotGrowOnEverySaveAndOpen():
    """The step-child list does not grow over save-and-open cycles."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.CloneGeomVec( [ pod ] )
    vsp.AddGeom( "CONFORMAL", pod )
    route = vsp.AddGeom( "ROUTING" )
    vsp.Update()
    vsp.AddRoutingPt( route, pod, 0 )
    vsp.AddRoutingPt( route, pod, 0 )
    vsp.Update()

    path = os.path.join( tempfile.mkdtemp(), "step.vsp3" )
    sizes = []
    for cycle in range( 5 ):
        vsp.WriteVSPFile( path )
        vsp.VSPRenew()
        scratch_output()
        vsp.ReadVSPFile( path )
        vsp.Update()

        text = open( path ).read()
        biggest = 0
        for listing in re.findall( r"<Step_Child_List>(.*?)</Step_Child_List>", text, re.S ):
            biggest = max( biggest, len( re.findall( r"<ID>\w+</ID>", listing ) ) )
        sizes.append( biggest )

    # A Clone, a Conformal and a Routing point name the pod; assert the list does not grow
    # rather than an exact count.
    assert sizes[0] >= 3, "expected the pod to hold at least three step children, got %d" % sizes[0]
    assert sizes == [ sizes[0] ] * len( sizes ), "the list grew: %s" % sizes
    drop_errors()


def testTheApiRefusesToChangeWhatACloneOnlyShows():
    """Edits a Clone cannot make are refused, not reported as success."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = vsp.AddGeom( "WING" )
    vsp.AddSubSurf( wing, vsp.SS_CONTROL )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.Update()
    drop_errors()

    copied = vsp.GetSubSurfIDVec( clone )
    assert len( copied ) == 1

    # A copied subsurface is rebuilt from the original every update, so deleting it is refused.
    vsp.DeleteSubSurf( clone, copied[0] )
    vsp.Update()
    assert_refused( "copied from" )
    assert list( vsp.GetSubSurfIDVec( clone ) ) == list( copied )

    # One the Clone added for itself is its own to delete.
    own = vsp.AddSubSurf( clone, vsp.SS_LINE )
    vsp.Update()
    assert len( vsp.GetSubSurfIDVec( clone ) ) == 2
    vsp.DeleteSubSurf( clone, own )
    vsp.Update()
    assert list( vsp.GetSubSurfIDVec( clone ) ) == list( copied )
    assert_no_errors()

    # The overload that finds the Geom from the subsurface refuses the same way.
    vsp.DeleteSubSurf( copied[0] )
    vsp.Update()
    assert_refused( "copied from" )
    assert list( vsp.GetSubSurfIDVec( clone ) ) == list( copied )

    # Cross sections belong to the original too.  Match the refusal's wording, since "Clone"
    # alone matches any message naming the Clone.
    before = vsp.GetNumXSec( vsp.GetXSecSurf( clone, 0 ) )
    vsp.InsertXSec( clone, 0, vsp.XS_SUPER_ELLIPSE )
    assert_refused( "change the Geom it copies" )
    assert vsp.GetNumXSec( vsp.GetXSecSurf( clone, 0 ) ) == before, "InsertXSec took effect"

    vsp.CutXSec( clone, 0 )
    assert_refused( "change the Geom it copies" )
    assert vsp.GetNumXSec( vsp.GetXSecSurf( clone, 0 ) ) == before, "CutXSec took effect"

    vsp.PasteXSec( clone, 0 )
    assert_refused( "change the Geom it copies" )
    assert vsp.GetNumXSec( vsp.GetXSecSurf( clone, 0 ) ) == before, "PasteXSec took effect"


def testAPartialFileOpenedIntoAModelKeepsItsReferences():
    """References in a partial file opened into a model keep their IDs, not fresh ones."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.AddGeom( "WING" )
    vsp.Update()

    out = tempfile.mkdtemp()
    full = os.path.join( out, "full.vsp3" )
    vsp.WriteVSPFile( full )

    tree = xml.etree.ElementTree.parse( full )
    root = tree.getroot()

    def name_the_pod( parent_tag, tag ):
        parent = root.find( ".//%s" % parent_tag )
        assert parent is not None, parent_tag
        node = parent.find( tag )
        if node is None:
            node = xml.etree.ElementTree.SubElement( parent, tag )
        node.text = pod

    name_the_pod( "CFDMeshSettings", "FarGeomID" )
    name_the_pod( "WaveDrag", "ReferenceGeomID" )

    # Drop the pod, so the references point at a Geom already in the model, as in a partial save.
    vehicle = root.find( "Vehicle" )
    dropped = 0
    for geom_node in list( vehicle.findall( "Geom" ) ):
        base = geom_node.find( "GeomBase" )
        if base is not None and base.findtext( "TypeName" ) == "Pod":
            vehicle.remove( geom_node )
            dropped += 1
    assert dropped == 1

    partial = os.path.join( out, "partial.vsp3" )
    tree.write( partial )

    vsp.VSPRenew()
    scratch_output()
    vsp.ReadVSPFile( full )
    vsp.ReadVSPFile( partial )
    vsp.Update()

    saved = os.path.join( out, "after.vsp3" )
    vsp.WriteVSPFile( saved )
    after = xml.etree.ElementTree.parse( saved ).getroot()

    live = vsp.FindGeoms()
    found = 0
    for node in after.iter():
        for tag in ( "FarGeomID", "ReferenceGeomID" ):
            value = node.findtext( tag )
            if value:
                found += 1
                assert value == pod, "%s was minted a new ID: %s" % ( tag, value )
                assert value in live
    assert found > 0, "neither reference was written back, so nothing was checked"
    drop_errors()


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
