# Bugs in the Geoms themselves, found while the Clone work went on.  Most of these are
# pre-existing faults it walked into, kept here so they stay fixed.  Two are not bug guards:
# the landing gear box inflated by rotation came from this branch moving the bounding boxes
# ahead of placement, and its test guards that fix; the Matrix4d case pins arithmetic this
# branch relies on and does not change.

import openvsp as vsp
import pytest
import math

from testhelp import ( drop_errors, assert_no_errors, a_wire_file )
import os
import tempfile
import glob
import xml.etree.ElementTree


def testAWireFrameMovesWhenItIsPlaced():
    """Placing a wireframe used to happen where the rearranging happens.

    That runs only when the shape is dirty, so moving one did nothing at all.  The two are
    separate steps now, which is also what lets another Geom borrow the rearranged grid.
    """
    vsp.VSPRenew()
    drop_errors()
    vsp.ImportFile( a_wire_file(), vsp.IMPORT_P3D_WIRE, "" )
    vsp.Update()
    wire = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "WireFrame" ][0]

    x_min = vsp.GetParmVal( vsp.FindParm( wire, "X_Min", "BBox" ) )
    x_len = vsp.GetParmVal( vsp.FindParm( wire, "X_Len", "BBox" ) )
    assert x_len > 0.0

    vsp.SetParmVal( vsp.FindParm( wire, "X_Rel_Location", "XForm" ), 100.0 )
    vsp.Update()

    assert vsp.GetParmVal( vsp.FindParm( wire, "X_Min", "BBox" ) ) == pytest.approx( x_min + 100.0 )
    assert vsp.GetParmVal( vsp.FindParm( wire, "X_Len", "BBox" ) ) == pytest.approx( x_len )
    assert_no_errors()


def testALandingGearsBoxHoldsItsOwnPosition():
    """The Geom's position stands in for the ground plane, which is not part of the shape."""
    vsp.VSPRenew()
    drop_errors()
    gear = vsp.AddGeom( "GEAR" )
    vsp.SetParmVal( vsp.FindParm( gear, "X_Rel_Location", "XForm" ), 20.0 )
    vsp.Update()

    # Nothing but the plane, so the box is the single point where the gear sits.
    assert vsp.GetParmVal( vsp.FindParm( gear, "X_Min", "BBox" ) ) == pytest.approx( 20.0 )
    assert vsp.GetParmVal( vsp.FindParm( gear, "X_Len", "BBox" ) ) == pytest.approx( 0.0 )

    # One point per symmetric copy, each placed.
    vsp.SetParmVal( vsp.FindParm( gear, "Y_Rel_Location", "XForm" ), 6.0 )
    vsp.SetParmVal( vsp.FindParm( gear, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( gear, "Y_Min", "BBox" ) ) == pytest.approx( -6.0 )
    assert vsp.GetParmVal( vsp.FindParm( gear, "Y_Len", "BBox" ) ) == pytest.approx( 12.0 )
    assert_no_errors()


def testALandingGearsBoxIsNotInflatedByRotation():
    """That point is added after the transform, not before it.

    Stretching the box to reach the origin and then rotating that box gives a bigger box than
    the gear occupies -- and this box sizes the ground plane, the burst zones, the engine
    extensions and the view.  With the wheel far out along x and the gear turned, nothing should
    reach behind the origin.
    """
    vsp.VSPRenew()
    drop_errors()
    gear = vsp.AddGeom( "GEAR" )
    bogie = vsp.CreateAndAddBogie( gear )
    vsp.SetParmVal( vsp.FindParm( bogie, "XContactPt", "Bogie" ), 20.0 )
    vsp.SetParmVal( vsp.FindParm( bogie, "YContactPt", "Bogie" ), 0.0 )
    vsp.SetParmVal( vsp.FindParm( bogie, "Symmetrical", "Bogie" ), 0 )
    vsp.SetParmVal( vsp.FindParm( gear, "Z_Rel_Rotation", "XForm" ), 45.0 )
    vsp.Update()

    assert vsp.GetParmVal( vsp.FindParm( gear, "X_Min", "BBox" ) ) == pytest.approx( 0.0, abs = 1e-9 )
    assert vsp.GetParmVal( vsp.FindParm( gear, "X_Len", "BBox" ) ) > 10.0
    assert_no_errors()


def testAControlSurfaceTagFileKeepsTheWholeGeomName():
    """Tag names used to be split on the first underscore, mangling any name containing one.

    A wing called Left_Wing was filed under Left, and a copy called WingGeom_Clone came out as
    WingGeomlone -- the two characters after the split were taken for the plate token.
    """
    vsp.VSPRenew()
    drop_errors()
    wing = vsp.AddGeom( "WING" )
    vsp.SetGeomName( wing, "Left_Wing" )
    ss = vsp.AddSubSurf( wing, vsp.SS_CONTROL )
    vsp.SetSubSurfName( ss, "Aileron" )
    vsp.Update()

    out = tempfile.mkdtemp()
    vsp.ExportFile( os.path.join( out, "tagtest.vspgeom" ), vsp.SET_ALL, vsp.EXPORT_VSPGEOM )

    listed = glob.glob( os.path.join( out, "*.ControlSurfaces.taglist" ) )
    assert listed, "the export wrote no control surface taglist"
    entries = [ l.strip() for l in open( listed[0] ).read().split( "\n" ) if l.strip() ]

    # A wing has two surfaces, so the file is a count and one name per surface -- all of it
    # spelled out, because the mangling showed up in the names themselves.
    assert entries == [ "2", "tagtest.Left_Wing_Surf0_Aileron",
                             "tagtest.Left_Wing_Surf1_Aileron" ], entries
    assert_no_errors()


def testExportsOfAnEmptyModelDoNotCrash():
    """The exporters used to read element zero of an empty vector."""
    vsp.VSPRenew()
    drop_errors()
    vsp.Update()
    assert len( vsp.FindGeoms() ) == 0

    out = tempfile.mkdtemp()
    for ext, kind in ( ( "stl", vsp.EXPORT_STL ),
                       ( "vspgeom", vsp.EXPORT_VSPGEOM ),
                       ( "facet", vsp.EXPORT_FACET ),
                       ( "tri", vsp.EXPORT_CART3D ),
                       ( "obj", vsp.EXPORT_OBJ ),
                       ( "msh", vsp.EXPORT_GMSH ),
                       ( "x3d", vsp.EXPORT_X3D ) ):
        vsp.ExportFile( os.path.join( out, "empty." + ext ), vsp.SET_ALL, kind )

    # Whatever the exporters make of an empty model, they have to come back.
    vsp.Update()
    assert_no_errors()


def testMatrix4dPlacesPointsAndTurnsNormals():
    """The placement arithmetic the rest of this work leans on, pinned against a change to it.

    Matrix4d is not touched by this branch -- the in-place forms date from long before it.  A
    point picks up the translation, a normal does not -- and the vector forms have to agree
    with the single-value ones.
    """
    mat = vsp.Matrix4d()
    mat.loadIdentity()
    mat.translatef( 7.0, -3.0, 2.0 )
    mat.rotateZ( 30.0 )
    mat.rotateX( 15.0 )

    p = vsp.vec3d( 1.0, 2.0, 3.0 )
    origin = mat.xform( vsp.vec3d( 0.0, 0.0, 0.0 ) )
    placed = mat.xform( p )
    turned = mat.xformnorm( p )

    # A normal is the point transform with the translation taken back out.
    assert turned.x() == pytest.approx( placed.x() - origin.x() )
    assert turned.y() == pytest.approx( placed.y() - origin.y() )
    assert turned.z() == pytest.approx( placed.z() - origin.z() )

    # Rotation and translation leave a direction's length alone.
    assert turned.mag() == pytest.approx( p.mag() )

    # The in-place vector form agrees with the by-value one, point for point.
    pts = vsp.Vec3dVec()
    for i in range( 4 ):
        pts.push_back( vsp.vec3d( i * 1.5, 2.0 - i, 0.5 * i ) )
    expected = [ mat.xform( pts[i] ) for i in range( 4 ) ]
    mat.xformvec( pts )
    for i in range( 4 ):
        assert pts[i].x() == pytest.approx( expected[i].x() )
        assert pts[i].y() == pytest.approx( expected[i].y() )
        assert pts[i].z() == pytest.approx( expected[i].z() )

    norms = vsp.Vec3dVec()
    for i in range( 3 ):
        norms.push_back( vsp.vec3d( 1.0, i, -1.0 ) )
    expected = [ mat.xformnorm( norms[i] ) for i in range( 3 ) ]
    mat.xformnormvec( norms )
    for i in range( 3 ):
        assert norms[i].x() == pytest.approx( expected[i].x() )
        assert norms[i].y() == pytest.approx( expected[i].y() )
        assert norms[i].z() == pytest.approx( expected[i].z() )


def testAWireFramesFourPatchTypesAreTheirOwnParms():
    """All four were initialised with the same name, so only the first was ever found."""
    vsp.VSPRenew()
    vsp.ImportFile( a_wire_file(), vsp.IMPORT_P3D_WIRE, "" )
    vsp.Update()
    wire = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "WireFrame" ][0]

    names = ( "IStartPatchType", "IEndPatchType", "JStartPatchType", "JEndPatchType" )
    ids = [ vsp.FindParm( wire, n, "WireFrame" ) for n in names ]
    assert all( ids ), "missing: %s" % [ n for n, i in zip( names, ids ) if not i ]
    assert len( set( ids ) ) == 4, "two of them are the same Parm"

    # And each one keeps its own value through a file.
    for i, pid in enumerate( ids ):
        vsp.SetParmVal( pid, i % 3 )
    vsp.Update()
    f = os.path.join( tempfile.mkdtemp(), "patch.vsp3" )
    vsp.WriteVSPFile( f )
    vsp.VSPRenew()
    vsp.ReadVSPFile( f )
    vsp.Update()

    wire = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "WireFrame" ][0]
    for i, n in enumerate( names ):
        assert vsp.GetParmVal( vsp.FindParm( wire, n, "WireFrame" ) ) == pytest.approx( i % 3 ), n
    assert_no_errors()


def testAMaterialMadeThroughTheApiSurvivesAFile():
    """Only a user material is written, and the API was not marking the ones it made."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.AddMaterial( "TestRuby", vsp.vec3d( 0.2, 0.0, 0.0 ), vsp.vec3d( 0.8, 0.1, 0.1 ),
                     vsp.vec3d( 0.9, 0.9, 0.9 ), vsp.vec3d( 0.0, 0.0, 0.0 ), 1.0, 32.0 )
    vsp.Update()
    assert "TestRuby" in list( vsp.GetMaterialNames() )

    f = os.path.join( tempfile.mkdtemp(), "mat.vsp3" )
    vsp.WriteVSPFile( f )

    # The material manager outlives any one model, so asking it after a reload would answer yes
    # whether or not anything was written.  The file is the only witness.
    root = xml.etree.ElementTree.parse( f ).getroot()
    written = [ n.findtext( "Name" ) for n in root.findall( "./Materials/Material" ) ]
    assert "TestRuby" in written, "the file lists %s" % written

    vsp.VSPRenew()
    vsp.ReadVSPFile( f )
    vsp.Update()
    assert "TestRuby" in list( vsp.GetMaterialNames() )
    assert_no_errors()


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()

def testATexturesPlacementSurvivesAFile():
    """Reading a texture back WROTE its Parms out again instead of taking the stored values.

    Texture::DecodeXml called ParmContainer::EncodeXml, so every placement -- the position, the
    two scales, the transparency and the two flips -- came back at the constructor's defaults.
    The file name survived, which is what made it look like textures worked at all.

    The values are read by writing the model out a second time and comparing the two files:
    FindParm cannot reach a texture's Parms by container ID once a file has been loaded, which
    is a separate fault and would make this test measure that instead.
    """
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()

    pod = vsp.AddGeom( "POD" )
    tex = vsp.AttachGeomTexture( pod, a_texture_file() )
    assert tex, "the texture did not attach"
    vsp.SetParmVal( vsp.FindParm( tex, "U", "Texture_Parm" ), 0.25 )
    vsp.SetParmVal( vsp.FindParm( tex, "U_Scale", "Texture_Parm" ), 0.5 )
    vsp.SetParmVal( vsp.FindParm( tex, "Transparency", "Texture_Parm" ), 0.75 )
    vsp.Update()

    first = os.path.join( out, "before.vsp3" )
    vsp.WriteVSPFile( first )

    vsp.VSPRenew()
    vsp.ReadVSPFile( first )
    vsp.Update()
    second = os.path.join( out, "after.vsp3" )
    vsp.WriteVSPFile( second )

    was = texture_placement( first )
    now = texture_placement( second )
    assert was[ "U" ] == pytest.approx( 0.25 ), was
    assert was[ "U_Scale" ] == pytest.approx( 0.5 ), was
    assert was[ "Transparency" ] == pytest.approx( 0.75 ), was
    assert now == was, "the placement came back different: %s against %s" % ( now, was )
    assert_no_errors()


def testAMeshSourceNeedsASurfaceToSitOn():
    """A source on a Geom with no surface crashed the mesher.

    AddCFDSource took any surface index, including one on a Blank, which has no surfaces at
    all.  Geom::UpdateSources then looked up the symmetric copies of a surface that was not
    there and read through an empty list -- a segfault on every mesh of the model, which is
    what adding a source to every Geom of sbw or x57 did.  The API now refuses the index, and
    a source that names a surface the Geom does not have, however it got there, is left out.
    """
    vsp.VSPRenew()
    drop_errors()

    blank = vsp.AddGeom( "BLANK" )
    pod = vsp.AddGeom( "POD" )
    vsp.Update()

    mgr = vsp.ErrorMgrSingleton.getInstance()
    for src in ( vsp.ULINE_SOURCE, vsp.WLINE_SOURCE, vsp.POINT_SOURCE ):
        before = mgr.GetNumTotalErrors()
        vsp.AddCFDSource( src, blank, 0, 0.1, 0.2, 0.5, 0.5 )
        assert mgr.GetNumTotalErrors() == before + 1
        assert mgr.PopLastError().GetErrorCode() == vsp.VSP_INDEX_OUT_RANGE

    before = mgr.GetNumTotalErrors()
    vsp.AddCFDSource( vsp.ULINE_SOURCE, pod, 1, 0.1, 0.2, 0.5, 0.0 )
    assert mgr.GetNumTotalErrors() == before + 1
    assert mgr.PopLastError().GetErrorCode() == vsp.VSP_INDEX_OUT_RANGE

    vsp.AddCFDSource( vsp.ULINE_SOURCE, pod, 0, 0.1, 0.2, 0.5, 0.0 )
    vsp.AddCFDSource( vsp.WLINE_SOURCE, pod, 0, 0.1, 0.2, 0.0, 0.5 )
    assert_no_errors()

    out = tempfile.mkdtemp()
    vsp.SetVSP3FileName( os.path.join( out, "source.vsp3" ) )
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.5 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.1 )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )
    assert os.path.getsize( os.path.join( out, "source.tri" ) ) > 0

    # A source whose surface index is out of range by the time it is meshed -- here, edited in
    # the file -- is left out rather than read through.
    path = os.path.join( out, "bad.vsp3" )
    vsp.WriteVSPFile( path )
    text = open( path ).read()
    assert text.count( '<MainSurfIndx Value="0' ) >= 2
    open( path, "w" ).write( text.replace( '<MainSurfIndx Value="0', '<MainSurfIndx Value="7' ) )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()
    drop_errors()
    vsp.SetVSP3FileName( os.path.join( out, "bad.vsp3" ) )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )
    assert os.path.getsize( os.path.join( out, "bad.tri" ) ) > 0


def testAGeomInBothSetsIsReported():
    """A Geom in both Prepare Geometry's thick and thin sets is meshed twice, and that is said.

    Its thin surfaces lie inside it and cut its skin where they meet it, leaving the panel mesh
    with doubled edges and broken wakes and nothing to say why.  The thin set defaults to Shown,
    so setting only the thick set to All does this to every Geom.
    """
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()
    vsp.SetVSP3FileName( os.path.join( out, "both.vsp3" ) )
    vsp.AddGeom( "FUSELAGE" )
    vsp.AddGeom( "WING" )
    vsp.Update()
    em = vsp.ErrorMgrSingleton.getInstance()

    def warnings( thin ):
        vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
        vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_ALL ] )
        vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ thin ] )
        drop_errors()
        vsp.ExecAnalysis( "VSPAEROComputeGeometry" )
        found = []
        while em.GetNumTotalErrors() > 0:
            e = em.PopLastError()
            if e.GetErrorCode() == vsp.VSP_GEOM_IN_BOTH_SETS:
                found.append( e.GetErrorString() )
        return found

    both = warnings( vsp.SET_SHOWN )
    assert len( both ) == 1, "expected one warning, got %r" % both
    assert "FuselageGeom" in both[0] and "WingGeom" in both[0], both[0]

    assert warnings( vsp.SET_NONE ) == []


def testPrepareGeometryWithNothingToMeshFinishes():
    """Prepare Geometry on a model with no surfaces leaves an empty mesh, not a deleted one.

    The polygon mesh's clean-up routines used to throw away a mesh with no faces from inside
    the mesh's own routines, and the routine and its caller then carried on using it.
    """
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()
    vsp.SetVSP3FileName( os.path.join( out, "blank.vsp3" ) )
    vsp.AddGeom( "BLANK" )
    vsp.Update()
    for nref in ( 0, 1 ):
        vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
        vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_ALL ] )
        vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ vsp.SET_NONE ] )
        vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "NRef", [ nref ] )
        vsp.ExecAnalysis( "VSPAEROComputeGeometry" )
    drop_errors()


@pytest.mark.parametrize( "scale, cap_tess", [ ( 1.0, 15 ), ( 1000.0, 6 ), ( 0.001, 15 ) ] )
def testAFinelyCappedBluntWingMakesAClosedPanelMesh( scale, cap_tess ):
    """Every edge of a blunt wing's panel mesh is shared by two faces, whatever its size or cap.

    A round tip cap on a blunt trailing edge is built from small but real triangles where it
    wraps the trailing edge base.  Refining the cap shrinks them below the area AddTri used to
    take for no area at all, so they were left out and the mesh opened there.  Made a thousand
    times larger and turned, the same wing grows triangles with three corners in a line past
    any fixed area, and kept they leave edges with more than two faces.  Made a thousand times
    smaller, its distinct nodes came within the fixed distance that merges coincident ones.
    """
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()
    vsp.SetVSP3FileName( os.path.join( out, "blunt.vsp3" ) )
    wing = vsp.AddGeom( "WING" )
    xsurf = vsp.GetXSecSurf( wing, 0 )
    for i in range( vsp.GetNumXSec( xsurf ) ):
        xsec = vsp.GetXSec( xsurf, i )
        vsp.SetParmVal( vsp.GetXSecParm( xsec, "TE_Trim_Type" ), 1 )
        vsp.SetParmVal( vsp.GetXSecParm( xsec, "TE_Trim_X" ), 0.1 )
    vsp.SetParmVal( vsp.FindParm( wing, "CapUMaxOption", "EndCap" ), vsp.ROUND_END_CAP )
    vsp.SetParmVal( vsp.FindParm( wing, "CapUMinTess", "EndCap" ), cap_tess )
    if scale != 1.0:
        vsp.SetParmVal( vsp.FindParm( wing, "Scale", "XForm" ), scale )
        for name, angle in ( ( "X_Rel_Rotation", 23.0 ), ( "Y_Rel_Rotation", 41.0 ), ( "Z_Rel_Rotation", 67.0 ) ):
            vsp.SetParmVal( vsp.FindParm( wing, name, "XForm" ), angle )
    vsp.Update()

    vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_ALL ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ vsp.SET_NONE ] )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )

    # One mesh: a comment, the mesh count, 'nnode nface nwake', the nodes, the face count, and
    # one line per face -- its node count followed by its 1-based node indices.
    lines = [ l for l in open( os.path.join( out, "blunt.vspgeom" ) ).read().split( "\n" ) if not l.startswith( "#" ) ]
    nnode, nface = [ int( x ) for x in lines[1].split()[:2] ]
    first = 2 + nnode + 1
    used = {}
    for line in lines[first:first + nface]:
        p = [ int( x ) for x in line.split() ]
        loop = p[1:1 + p[0]]
        for a, b in zip( loop, loop[1:] + loop[:1] ):
            key = ( min( a, b ), max( a, b ) )
            used[ key ] = used.get( key, 0 ) + 1
    open_edges = [ e for e, n in used.items() if n != 2 ]
    assert open_edges == [], "%d edges are not shared by two faces" % len( open_edges )
    drop_errors()


def testAHoleInsideOnePanelLeavesTwoFaces():
    """A small hole cut inside one panel leaves the panel in two faces, not one ringing it.

    The polygon mesh joins the pieces an intersection cuts a panel into back up.  Around a hole
    the last join would make a face whose loop runs round the hole and back, naming nodes twice
    and leaving the edges along the cut with one face each.
    """
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()
    vsp.SetVSP3FileName( os.path.join( out, "rod.vsp3" ) )
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( pod, "Tess_W", "Shape" ), 5 )
    vsp.SetParmVal( vsp.FindParm( pod, "Tess_U", "Shape" ), 5 )
    rod = vsp.AddGeom( "POD" )
    for name, group, value in ( ( "Length", "Design", 2.0 ), ( "FineRatio", "Design", 100.0 ),
                                ( "Z_Rel_Rotation", "XForm", 90.0 ), ( "X_Rel_Location", "XForm", 2.0 ),
                                ( "Y_Rel_Location", "XForm", -1.0 ), ( "Z_Rel_Location", "XForm", 0.066 ) ):
        vsp.SetParmVal( vsp.FindParm( rod, name, group ), value )
    vsp.Update()

    vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_ALL ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ vsp.SET_NONE ] )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )

    # One mesh: a comment, the mesh count, 'nnode nface nwake', the nodes, the face count, and
    # one line per face -- its node count followed by its 1-based node indices.
    lines = [ l for l in open( os.path.join( out, "rod.vspgeom" ) ).read().split( "\n" ) if not l.startswith( "#" ) ]
    nnode, nface = [ int( x ) for x in lines[1].split()[:2] ]
    first = 2 + nnode + 1
    used = {}
    pinched = 0
    unwritten = 0
    for line in lines[first:first + nface]:
        if "#" in line:
            # A face the writer could not close is written as a comment.
            unwritten += 1
            continue
        p = [ int( x ) for x in line.split() ]
        loop = p[1:1 + p[0]]
        if len( set( loop ) ) != len( loop ):
            pinched += 1
        for a, b in zip( loop, loop[1:] + loop[:1] ):
            key = ( min( a, b ), max( a, b ) )
            used[ key ] = used.get( key, 0 ) + 1
    assert unwritten == 0, "the writer could not close %d faces" % unwritten
    assert pinched == 0, "%d faces name a node twice" % pinched
    wrong = [ e for e, n in used.items() if n != 2 ]
    assert wrong == [], "%d edges are not shared by two faces" % len( wrong )
    drop_errors()


def _prepare_with_refinement( out, name, nref ):
    """Prepare Geometry with nref coarse levels, thick set All and thin set none; the .vspgeom text.

    The refinement count is set on the VSPAERO Parm, which is what reaches the mesher.
    """
    vsp.SetVSP3FileName( os.path.join( out, name + ".vsp3" ) )
    settings = vsp.FindContainer( "VSPAEROSettings", 0 )
    vsp.SetParmVal( vsp.FindParm( settings, "NRef", "VSPAERO" ), nref )
    vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_ALL ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ vsp.SET_NONE ] )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )
    return open( os.path.join( out, name + ".vspgeom" ) ).read()


def _face_sets( text ):
    """Each level's faces from .vspgeom text, as a set of loops each started at its least node.

    Per level the file holds: the face count, one loop per face, per face its part, tag and a uw
    per node, per face its index and parent, the wakes, and two blocks of triangulation data.
    """
    tokens = []
    for line in text.split( "\n" )[1:]:
        tokens.extend( line.split( "#" )[0].split() )
    it = iter( tokens )
    nxt = lambda: next( it )
    nmesh = int( nxt() )
    heads = [ ( int( nxt() ), int( nxt() ), int( nxt() ) ) for m in range( nmesh ) ]
    for k in range( 3 * max( h[0] for h in heads ) ):
        nxt()
    levels = []
    for m in range( nmesh ):
        faces = []
        for k in range( int( nxt() ) ):
            n = int( nxt() )
            faces.append( [ int( nxt() ) for q in range( n ) ] )
        for f in faces:
            for q in range( 2 + 2 * len( f ) ):
                nxt()
        for f in faces:
            nxt(); nxt()
        for k in range( int( nxt() ) ):
            for q in range( abs( int( nxt() ) ) + 1 ):
                nxt()
        ntri = []
        for f in faces:
            nxt()
            ntri.append( int( nxt() ) )
            for q in range( 3 * ntri[-1] ):
                nxt()
        for n in ntri:
            for q in range( 3 + 6 * n ):
                nxt()
        loops = set()
        for f in faces:
            i = f.index( min( f ) )
            loops.add( tuple( f[i:] + f[:i] ) )
        levels.append( loops )
    return levels


def testCoarseLevelsAreTheSameEveryRun():
    """Preparing the same model twice gives the same coarse panel meshes.

    Coarsening dropped a node between two faces of different tags whenever the faces came from
    different tessellation quads, with no look at the shape.  Which quad a merged coarse face
    remembers depends on the order faces were merged in, so control surface corners came and
    went from one run to the next.
    """
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( wing, "Sweep", "XSec_1" ), 50.0 )
    vsp.SetParmVal( vsp.FindParm( wing, "Tess_W", "Shape" ), 21 )
    vsp.Update()
    for eta0, eta1 in ( ( 0.1, 0.35 ), ( 0.4, 0.62 ), ( 0.66, 0.93 ) ):
        ss = vsp.AddSubSurf( wing, vsp.SS_CONTROL )
        vsp.Update()
        vsp.SetParmVal( vsp.FindParm( ss, "EtaFlag", "SS_Control" ), 1 )
        vsp.SetParmVal( vsp.FindParm( ss, "EtaStart", "SS_Control" ), eta0 )
        vsp.SetParmVal( vsp.FindParm( ss, "EtaEnd", "SS_Control" ), eta1 )
        vsp.Update()
    vsp.AutoGroupVSPAEROControlSurfaces()
    vsp.Update()

    first = _face_sets( _prepare_with_refinement( out, "cs", 1 ) )
    for i in range( 3 ):
        again = _face_sets( _prepare_with_refinement( out, "cs", 1 ) )
        assert again == first, "run %d gave different faces" % ( i + 2 )
    drop_errors()


def testNoCoarsePanelLosesItsShape():
    """Coarsening never leaves a face of two nodes, which the writer can only record as an error.

    A rod along a pod's axis through its nose pole leaves slivers at the pole whose coarse faces
    are triangles; taking a corner off one left two nodes.
    """
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( pod, "Tess_W", "Shape" ), 5 )
    vsp.SetParmVal( vsp.FindParm( pod, "Tess_U", "Shape" ), 5 )
    rod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( rod, "Length", "Design" ), 2.0 )
    vsp.SetParmVal( vsp.FindParm( rod, "FineRatio", "Design" ), 10.0 )
    vsp.SetParmVal( vsp.FindParm( rod, "X_Rel_Location", "XForm" ), -1.0 )
    vsp.Update()

    text = _prepare_with_refinement( out, "pole", 3 )
    assert text.count( "FACE ERROR" ) == 0, "%d faces written as errors" % text.count( "FACE ERROR" )
    drop_errors()


def testARodIntoATipCapNearItsLeadingEdgeFinishes():
    """Sealing a node where the panel mesh doubles back must not chase itself without end.

    A rod entering a flat tip cap half a percent of chord behind the leading edge split an edge
    at its very end, so the new node sat on the old one and the two handed the double back to
    each other until the stack ran out.
    """
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( wing, "Tess_W", "Shape" ), 17 )
    vsp.SetParmVal( vsp.FindParm( wing, "CapUMaxOption", "EndCap" ), vsp.FLAT_END_CAP )
    rod = vsp.AddGeom( "POD" )
    for name, group, value in ( ( "Length", "Design", 0.6 ), ( "FineRatio", "Design", 20.0 ),
                                ( "Z_Rel_Rotation", "XForm", 90.0 ), ( "X_Rel_Location", "XForm", 5.20115242270663 ),
                                ( "Y_Rel_Location", "XForm", 8.7 ) ):
        vsp.SetParmVal( vsp.FindParm( rod, name, group ), value )
    vsp.Update()

    text = _prepare_with_refinement( out, "tip", 3 )
    assert text.count( "FACE ERROR" ) == 0
    drop_errors()


def testASlenderRodThroughAWingLeavesNoHole():
    """Splitting a triangle puts each cut point on the nearest of its edges.

    A rod triangle is a long sliver, and the wing cut it close to its tip, where both long edges
    were within the on-edge tolerance of each cut point.  Each point went on the first edge
    tried, the two cut segments crossed, and the triangulator dropped the part below the cut,
    leaving a hole in the rod.
    """
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( wing, "Tess_W", "Shape" ), 17 )
    vsp.SetParmVal( vsp.FindParm( wing, "CapUMaxOption", "EndCap" ), vsp.FLAT_END_CAP )
    rod = vsp.AddGeom( "POD" )
    for name, group, value in ( ( "Length", "Design", 2.0 ), ( "FineRatio", "Design", 500.0 ),
                                ( "Y_Rel_Rotation", "XForm", 90.0 ), ( "X_Rel_Location", "XForm", 2.764400457138084 ),
                                ( "Y_Rel_Location", "XForm", 2.7 ), ( "Z_Rel_Location", "XForm", 1.0 ) ):
        vsp.SetParmVal( vsp.FindParm( rod, name, group ), value )
    vsp.Update()

    for loops in _face_sets( _prepare_with_refinement( out, "rod", 3 ) ):
        used = {}
        for loop in loops:
            for a, b in zip( loop, loop[1:] + loop[:1] ):
                key = ( min( a, b ), max( a, b ) )
                used[ key ] = used.get( key, 0 ) + 1
        assert [ e for e, n in used.items() if n != 2 ] == []
    drop_errors()


@pytest.mark.parametrize( "cap", [ vsp.ROUND_EXT_END_CAP_LE, vsp.ROUND_EXT_END_CAP_TE, vsp.ROUND_EXT_END_CAP_BOTH ] )
def testAnExtendedTipCapLeavesNoPillowInThePanelMesh( cap ):
    """Every edge of an extended tip cap's panel mesh is shared by two faces of three or more nodes.

    With two cap tessellation steps, an extended round tip cap tessellates one sliver at the
    edge it extends twice, once from each side.  The two triangles share their three nodes and
    wind opposite ways, so they enclose nothing and give one edge four faces.  Taking the third
    corner off them instead leaves two faces of two nodes.  Splitting both quads on their other
    diagonal closes the cap and keeps the point it extends to.
    """
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()
    wing = vsp.AddGeom( "WING" )
    for name, group, value in ( ( "CapUMaxOption", "EndCap", cap ), ( "CapUMinTess", "EndCap", 2 ),
                                ( "Sym_Planar_Flag", "Sym", 0 ), ( "Tess_W", "Shape", 5 ),
                                ( "SectTess_U", "XSec_1", 2 ) ):
        vsp.SetParmVal( vsp.FindParm( wing, name, group ), value )
    vsp.Update()

    text = _prepare_with_refinement( out, "cap", 0 )
    loops = _face_sets( text )[0]
    short = [ loop for loop in loops if len( set( loop ) ) < 3 ]
    assert short == [], "%d faces have fewer than three nodes" % len( short )
    used = {}
    for loop in loops:
        for a, b in zip( loop, loop[1:] + loop[:1] ):
            key = ( min( a, b ), max( a, b ) )
            used[ key ] = used.get( key, 0 ) + 1
    wrong = [ e for e, n in used.items() if n != 2 ]
    assert wrong == [], "%d edges are not shared by two faces" % len( wrong )

    # The panel mesh reaches as far as the tessellation, so the cap keeps its extension.
    lines = text.split( "\n" )
    npnt = int( lines[2].split()[0] )
    mesh = [ [ float( t ) for t in line.split()[:3] ] for line in lines[3:3 + npnt] ]
    stl = os.path.join( out, "cap.stl" )
    vsp.ExportFile( stl, vsp.SET_ALL, vsp.EXPORT_STL )
    tess = [ [ float( t ) for t in line.split()[1:4] ] for line in open( stl ) if line.strip().startswith( "vertex" ) ]
    for i in range( 3 ):
        assert min( p[i] for p in mesh ) == pytest.approx( min( p[i] for p in tess ), abs = 1e-6 )
        assert max( p[i] for p in mesh ) == pytest.approx( max( p[i] for p in tess ), abs = 1e-6 )
    drop_errors()


def testAPanelMeshSavedOnACoarseLevelReloads():
    """Reading back a model whose polygon mesh was showing a coarse level does not crash.

    The level shown is a Parm and is saved; the levels themselves are not, so the reloaded mesh
    has one, and the saved index pointed past it.
    """
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()
    vsp.AddGeom( "POD" )
    vsp.Update()
    _prepare_with_refinement( out, "levels", 1 )
    ngon = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "NGonMesh" ][0]
    vsp.SetParmVal( vsp.FindParm( ngon, "ActiveMesh", "RefMesh" ), 1 )
    vsp.Update()
    path = os.path.join( out, "saved.vsp3" )
    vsp.WriteVSPFile( path )

    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()
    assert [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "NGonMesh" ]
    drop_errors()


def a_texture_file():
    """An image from the repo, by a path that does not depend on the working directory."""
    here = os.path.dirname( os.path.abspath( __file__ ) )
    return os.path.join( here, "..", "..", "..", "examples", "textures", "nasa-logo.tga" )


def texture_placement( path ):
    """The first texture's Parm values, read out of a written file."""
    import re
    text = open( path ).read()
    start = text.find( "<Texture_Parm>" )
    assert start >= 0, "no texture was written to %s" % path
    block = text[ start : text.find( "</Texture_Parm>", start ) ]
    return { m.group( 1 ): float( m.group( 2 ) )
             for m in re.finditer( r'<(\w+) Value="([^"]+)"', block ) }

def testAParmCanBeFoundByNameAndGroupAfterAFileLoad():
    """Every container except the Geom lost name-and-group lookup when a model was reopened.

    ParmContainer::DecodeXml builds the group-to-Parm map before decoding the Parms, and
    decoding a Parm remaps its ID, so every entry in that map named an ID that no longer
    existed.  A container walked as part of the linkable set has its map rebuilt by
    LinkMgr::BuildLinkableParmData and never noticed; a subsurface, a cross section, a texture,
    a mesh source, a bogie or a routing point kept the stale map for the life of the model.

    The symptom was that a Parm readable from a model just built was unreadable from the same
    model reopened -- which is what drove people to walk FindContainerParmIDs by hand and match
    on GetParmName.
    """
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()

    pod = vsp.AddGeom( "POD" )
    wing = vsp.AddGeom( "WING" )
    fuse = vsp.AddGeom( "FUSELAGE" )
    gear = vsp.AddGeom( "GEAR" )
    route = vsp.AddGeom( "ROUTING" )
    vsp.AddSubSurf( wing, vsp.SS_LINE )
    vsp.AttachGeomTexture( pod, a_texture_file() )
    vsp.AddCFDSource( vsp.POINT_SOURCE, pod, 0, 0.5, 1.0, 0.5, 0.5 )
    vsp.CreateAndAddBogie( gear )
    vsp.AddRoutingPt( route, pod, 0 )
    vsp.Update()

    written = os.path.join( out, "containers.vsp3" )
    vsp.WriteVSPFile( written )
    vsp.VSPRenew()
    vsp.ReadVSPFile( written )
    vsp.Update()
    drop_errors()

    def find( tag, gid_type ):
        gid = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == gid_type ][0]
        return gid

    pod = find( "pod", "Pod" )
    wing = find( "wing", "Wing" )
    fuse = find( "fuse", "Fuselage" )
    gear = find( "gear", "Gear" )
    route = find( "route", "Routing" )

    containers = {
        "Geom":         pod,
        "XSec":         vsp.GetXSec( vsp.GetXSecSurf( fuse, 0 ), 1 ),
        "SubSurface":   vsp.GetSubSurfIDVec( wing )[0],
        "Texture":      vsp.GetGeomTextureIDVec( pod )[0],
        "BaseSource":   vsp.GetCFDSourceID( pod, 0 ),
        "Bogie":        list( vsp.GetAllBogies( gear ) )[0],
        "RoutingPoint": list( vsp.GetAllRoutingPtIds( route ) )[0],
    }

    for tag, cid in containers.items():
        assert cid, "%s: nothing came back from its getter" % tag
        parms = list( vsp.FindContainerParmIDs( cid ) )
        assert parms, "%s: the container reports no Parms at all" % tag

        # Every Parm the container reports has to be findable by the name and group it
        # reports, which is the whole contract of the three-argument lookup.
        name = vsp.GetParmName( parms[0] )
        group = vsp.GetParmGroupName( parms[0] )
        found = vsp.FindParm( cid, name, group )
        assert found, "%s: FindParm could not find %s / %s after a file load" % ( tag, name, group )
        assert found == parms[0], "%s: FindParm found a different Parm" % tag

    assert_no_errors()


@pytest.mark.parametrize( "preset", [ vsp.STACK_PRESET_FLOWTHRU_OUTLIP_ORIG,
                                      vsp.STACK_PRESET_FLOWTHRU_INLIP_ORIG,
                                      vsp.STACK_PRESET_FLOWTHRU_MID_ORIG ] )
def testAFlowThroughStackPresetClosesItsLoop( preset ):
    """A loop Stack's last cross section closes onto its first.

    The flow-through presets list their sections starting from wherever the loop begins, so the
    section that lands first carried a delta meant for its place further round -- 3.0, -2.0,
    -0.75.  The first section was held at the origin, but only once the rest of the chain had
    already been placed from it, so a freshly made preset stayed open by exactly that delta
    until the model was saved and reopened.
    """
    vsp.VSPRenew()
    sid = vsp.AddGeom( "STACK" )
    vsp.InitStackPreset( sid, preset )
    vsp.Update()

    xss = vsp.GetXSecSurf( sid, 0 )
    last = vsp.GetXSec( xss, vsp.GetNumXSec( xss ) - 1 )
    for name in ( "XAbs", "YAbs", "ZAbs" ):
        assert vsp.GetParmVal( vsp.GetXSecParm( last, name ) ) == pytest.approx( 0.0, abs=1e-12 ), name


def testAParmCanBeFoundByNameAndGroupAfterItsXSecChangesShape():
    """Changing a cross section's shape keeps its Parm IDs by swapping them onto the new cross
    section.  The swap moved the IDs but not the map each container finds its Parms by, so a
    lookup by name and group answered an ID that no longer belonged to it, while a lookup by
    name alone still worked."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    xss = vsp.GetXSecSurf( fid, 0 )
    xs = vsp.GetXSec( xss, 2 )
    before = { key: vsp.FindParm( xs, *key ) for key in ( ( "TopLStrength", "XSec" ), ( "SectTess_U", "XSec" ) ) }

    vsp.ChangeXSecShape( xss, 2, vsp.XS_SUPER_ELLIPSE )
    vsp.Update()
    xs = vsp.GetXSec( xss, 2 )

    for key, pid in before.items():
        assert vsp.ValidParm( pid ), key
        assert vsp.FindParm( xs, *key ) == pid, key


def testAttributesThatShareANameAreEachFound():
    """A collection may hold two attributes with the same name.  The search paired a list with
    one entry per attribute against a list with one entry per distinct name, so past the first
    repeat every attribute was matched against another's name -- and the last read ran off the
    end of the shorter list."""
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    coll = vsp.GetChildCollection( pod )
    first = vsp.AddAttributeString( coll, "Note", "first" )
    second = vsp.AddAttributeString( coll, "Note", "second" )
    other = vsp.AddAttributeString( coll, "Other", "third" )

    notes = [ vsp.FindAttributeInCollection( pod, "Note", i ) for i in range( 2 ) ]
    assert sorted( notes ) == sorted( [ first, second ] )
    assert vsp.FindAttributeInCollection( pod, "Other", 0 ) == other


def testAnAuxiliaryGeomWithNoSurfaceAnswersWithoutCrashing():
    """An auxiliary Geom with nothing to hang off has a surface with no patches.  Asking it for
    a point, a normal, a curvature or a tangent read past an empty patch map, which is what
    adding a subsurface to one did."""
    vsp.VSPRenew()
    aux = vsp.AddGeom( "AUXILIARY" )
    vsp.Update()

    assert vsp.AddSubSurf( aux, vsp.SS_LINE )
    vsp.Update()

    zero = vsp.vec3d()
    assert vsp.dist( vsp.CompPnt01( aux, 0, 0.5, 0.5 ), zero ) == 0.0
    assert vsp.dist( vsp.CompNorm01( aux, 0, 0.5, 0.5 ), zero ) == 0.0
    assert list( vsp.CompCurvature01( aux, 0, 0.5, 0.5 ) ) == [ 0.0, 0.0, 0.0, 0.0 ]


@pytest.mark.parametrize( "points", [ "YZ plane", "counter-clockwise", "none" ] )
def testAFileSectionRefusesPointsItCannotShape( points ):
    """A file section finds its bottom, side and top by walking the points from the right-hand
    one down through the bottom.  Points in the YZ plane all have x = 0, so none of the three
    was ever found and the curve was built from a division by zero -- which crashed.  Points
    running the other way round were read past the end of the list, and made a wrong shape
    with nothing said.  Either is refused now, and the section is left as it was."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    xss = vsp.GetXSecSurf( fid, 0 )
    vsp.ChangeXSecShape( xss, 1, vsp.XS_FILE_FUSE )
    vsp.Update()
    xs = vsp.GetXSec( xss, 1 )
    before = [ ( p.x(), p.y(), p.z() ) for p in vsp.GetXSecPnts( xs ) ]

    angles = [ 2.0 * math.pi * k / 20 for k in range( 21 ) ]
    if points == "YZ plane":
        pnts = [ vsp.vec3d( 0.0, 0.5 * math.cos( -a ), 0.5 * math.sin( -a ) ) for a in angles ]
    elif points == "counter-clockwise":
        pnts = [ vsp.vec3d( 0.5 * math.cos( a ), 0.5 * math.sin( a ), 0.0 ) for a in angles ]
    else:
        pnts = []

    em = vsp.ErrorMgrSingleton.getInstance()
    while em.GetNumTotalErrors() > 0:
        em.PopLastError()
    vsp.SetXSecPnts( xs, pnts )
    vsp.Update()

    assert em.GetNumTotalErrors() == 1
    assert em.PopLastError().GetErrorCode() == vsp.VSP_INVALID_INPUT_VAL
    assert [ ( p.x(), p.y(), p.z() ) for p in vsp.GetXSecPnts( xs ) ] == before

    # The same circle the right way round is taken.
    pnts = [ vsp.vec3d( 0.5 * math.cos( -a ), 0.5 * math.sin( -a ), 0.0 ) for a in angles ]
    vsp.SetXSecPnts( xs, pnts )
    vsp.Update()
    assert em.GetNumTotalErrors() == 0
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "Width" ) ) == pytest.approx( 1.0 )


def testAHumanWithAxialSymmetryOnAReflectedCopyIsNotInsideOut():
    """Axial copies of a reflected Human keep the reflection's winding, so planar plus axial
    symmetry gives twice the volume of axial alone."""
    vsp.VSPRenew()
    drop_errors()
    scratch = tempfile.mkdtemp()
    vsp.SetComputationFileName( vsp.COMP_GEOM_TXT_TYPE, os.path.join( scratch, "cg.txt" ) )
    vsp.SetComputationFileName( vsp.COMP_GEOM_CSV_TYPE, os.path.join( scratch, "cg.csv" ) )

    def volume( planar ):
        vsp.VSPRenew()
        human = vsp.AddGeom( "HUMAN" )
        for parm, val in ( ( "Y_Rel_Location", 5.0 ), ( "Z_Rel_Location", 5.0 ) ):
            vsp.SetParmVal( vsp.FindParm( human, parm, "XForm" ), val )
        vsp.SetParmVal( vsp.FindParm( human, "Sym_Planar_Flag", "Sym" ), planar )
        vsp.SetParmVal( vsp.FindParm( human, "Sym_Axial_Flag", "Sym" ), vsp.SYM_ROT_X )
        vsp.SetParmVal( vsp.FindParm( human, "Sym_Rot_N", "Sym" ), 2 )
        vsp.Update()
        vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
        res = vsp.FindLatestResultsID( "Comp_Geom" )
        return sum( vsp.GetDoubleResults( res, "Theo_Vol" ) )

    axial = volume( 0 )
    assert axial > 0.0
    assert volume( vsp.SYM_XZ ) == pytest.approx( 2.0 * axial )
    drop_errors()


def _pov_signed_volume( path ):
    """Signed volume of the triangles in a POV-Ray include file."""
    import re
    total = 0.0
    for block in re.findall( r'smooth_triangle \{(.*?)\}', open( path ).read(), re.S ):
        vecs = re.findall( r'<([^>]*)>', block )
        a, b, c = ( tuple( float( t ) for t in vecs[i].split( ',' ) ) for i in ( 0, 2, 4 ) )
        total += ( a[0] * ( b[1] * c[2] - b[2] * c[1] )
                 - a[1] * ( b[0] * c[2] - b[2] * c[0] )
                 + a[2] * ( b[0] * c[1] - b[1] * c[0] ) ) / 6.0
    return total


def testAPovRayFileWindsAReflectedCopyTheWayItsNormalsPoint():
    """A POV-Ray export winds a symmetric copy outward, so it adds to the volume."""
    vsp.VSPRenew()
    drop_errors()
    scratch = tempfile.mkdtemp()

    def written( planar ):
        vsp.VSPRenew()
        pod = vsp.AddGeom( "POD" )
        vsp.SetParmVal( vsp.FindParm( pod, "Sym_Planar_Flag", "Sym" ), planar )
        vsp.SetParmVal( vsp.FindParm( pod, "Y_Rel_Location", "XForm" ), 3.0 )
        vsp.Update()
        path = os.path.join( scratch, "pod_%d.pov" % planar )
        vsp.ExportFile( path, vsp.SET_ALL, vsp.EXPORT_POVRAY )
        return _pov_signed_volume( path.replace( ".pov", ".inc" ) )

    one = written( 0 )
    assert one > 0.0
    assert written( vsp.SYM_XZ ) == pytest.approx( 2.0 * one, rel = 1e-9 )
    drop_errors()


def testAStackOrFuselageEndCapStopsAtSharp():
    """The extended round caps are wing caps; on a Stack they crashed Update."""
    vsp.VSPRenew()
    drop_errors()
    for gtype in ( "STACK", "FUSELAGE" ):
        geom = vsp.AddGeom( gtype )
        for name in ( "CapUMinOption", "CapUMaxOption" ):
            parm = vsp.FindParm( geom, name, "EndCap" )
            vsp.SetParmVal( parm, vsp.ROUND_EXT_END_CAP_BOTH )
            assert vsp.GetParmVal( parm ) == vsp.SHARP_END_CAP
    vsp.Update()
    assert_no_errors()


def _vspgeom_parts( path ):
    """The part and tag numbers of every triangle in a one-mesh vspgeom file."""
    lines = open( path ).read().split( "\n" )
    npnt, ntri = ( int( t ) for t in lines[2].split()[:2] )
    first = 3 + npnt + 1 + ntri
    return [ tuple( int( t ) for t in lines[first + i].split()[:2] ) for i in range( ntri ) ]


@pytest.mark.parametrize( "before", [ "stl", "massprops", "planeslice", "projection", "stale", "import" ] )
def testAVSPGEOMExportWritesWhateverMeshIsThere( before ):
    """A VSPGEOM export writes an existing mesh in its set, and looks its triangles up in the
    tag tables.  Those describe the last mesh tagged, so a mesh made untagged, read from a file,
    or tagged before another mesh crashed the export."""
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()
    vsp.SetVSP3FileName( os.path.join( out, "m.vsp3" ) )
    pod = vsp.AddGeom( "POD" )
    wing = vsp.AddGeom( "WING" )
    vsp.Update()

    if before == "stl":
        vsp.ExportFile( os.path.join( out, "m.stl" ), vsp.SET_ALL, vsp.EXPORT_STL )
    elif before == "massprops":
        vsp.ComputeMassProps( vsp.SET_ALL, 10, vsp.X_DIR )
    elif before == "planeslice":
        vsp.ComputePlaneSlice( vsp.SET_ALL, 10, vsp.vec3d( 1, 0, 0 ), True )
    elif before == "projection":
        vsp.SetAnalysisInputDefaults( "Projection" )
        vsp.ExecAnalysis( "Projection" )
    elif before == "stale":
        vsp.ExportFile( os.path.join( out, "m.stl" ), vsp.SET_ALL, vsp.EXPORT_STL )
        vsp.SetSetFlag( pod, vsp.SET_FIRST_USER, True )
        vsp.ComputeCompGeom( vsp.SET_FIRST_USER, False, 0 )
    elif before == "import":
        vsp.ExportFile( os.path.join( out, "m.tri" ), vsp.SET_ALL, vsp.EXPORT_CART3D )
        vsp.DeleteGeomVec( [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Mesh" ] )
        vsp.ImportFile( os.path.join( out, "m.tri" ), vsp.IMPORT_CART3D_TRI, "" )

    path = os.path.join( out, "m.vspgeom" )
    vsp.ExportFile( path, vsp.SET_ALL, vsp.EXPORT_VSPGEOM )
    parts = _vspgeom_parts( path )
    assert parts
    assert min( p for p, t in parts ) >= 1
    assert min( t for p, t in parts ) >= 1
    drop_errors()
