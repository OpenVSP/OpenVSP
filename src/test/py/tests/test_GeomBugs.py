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
