# Clone Geom: the roles it stands in for.
#
# A Clone implements every role interface, so a plain cast cannot tell whether it has a role;
# Geom::CastTo checks what it behaves like first.  A Clone of a hinge is a joint, and a Clone
# of anything else is not.

import openvsp as vsp
import pytest

from clonehelp import ( BBOX_PARMS, box, switch, comp_geom_areas,
                        scratch_output, drop_errors, assert_no_errors,
                        degen_rows )
import os
import tempfile
import re


def chain_boxes( geoms ):
    return [ box( g ) for g in geoms ]


def testACloneOfAPodIsNotAJoint():
    """A child of a Clone of a pod attaches to its surface as usual, not as if to a joint."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    def attached_child( parent ):
        child = vsp.AddGeom( "POD", parent )
        vsp.SetParmVal( vsp.FindParm( child, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_UV )
        vsp.SetParmVal( vsp.FindParm( child, "U_Attach_Location", "Attach" ), 0.5 )
        vsp.SetParmVal( vsp.FindParm( child, "V_Attach_Location", "Attach" ), 0.25 )
        vsp.Update()
        return tuple( vsp.GetParmVal( vsp.FindParm( child, n, "XForm" ) )
                      for n in ( "X_Location", "Y_Location", "Z_Location" ) )

    on_pod = attached_child( pod )
    on_clone = attached_child( clone )

    assert on_pod != pytest.approx( ( 0.0, 0.0, 0.0 ) )    # the attachment does something
    assert on_clone == pytest.approx( on_pod )
    assert_no_errors()


def testACloneOfAHingeArticulatesItsChildren():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    hinge = vsp.AddGeom( "HINGE", pod )
    child = vsp.AddGeom( "POD", hinge )
    # The hinge is attached to its parent, so the whole chain moves with the group.
    vsp.SetParmVal( vsp.FindParm( hinge, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( vsp.FindParm( hinge, "Rots_Attach_Flag", "Attach" ), vsp.ATTACH_ROT_COMP )
    vsp.Update()

    clones = vsp.CloneGeomVec( [ pod, hinge, child ] )
    vsp.Update()
    pod_clone = [ c for c in clones if vsp.GetGeomCloneOriginal( c ) == pod ][0]
    hinge_clone = [ c for c in clones if vsp.GetGeomCloneOriginal( c ) == hinge ][0]
    child_clone = [ c for c in clones if vsp.GetGeomCloneOriginal( c ) == child ][0]

    # Move the cloned group and record where the child sits before any posing.
    vsp.SetParmVal( vsp.FindParm( pod_clone, "Y_Rel_Location", "XForm" ), 40.0 )
    vsp.Update()
    unposed = box( child_clone )

    # Then pose the joint.
    vsp.SetParmVal( vsp.FindParm( hinge, "JointRotate", "Hinge" ), 35.0 )
    vsp.Update()
    assert box( child_clone ) != pytest.approx( unposed ), \
           "the deflection did not carry the Clone's child, so the rest proves nothing"

    # With CloneJoint on, the Clone takes the original's deflection, at the group's own position.
    assert vsp.GetParmVal( vsp.FindParm( hinge_clone, "JointRotate", "Hinge" ) ) == pytest.approx( 35.0 )
    for parm in ( "X_Min", "X_Len", "Z_Min", "Z_Len" ):
        assert vsp.GetParmVal( vsp.FindParm( child_clone, parm, "BBox" ) ) == \
               pytest.approx( vsp.GetParmVal( vsp.FindParm( child, parm, "BBox" ) ), abs = 1e-6 )
    assert vsp.GetParmVal( vsp.FindParm( child_clone, "Y_Min", "BBox" ) ) == \
           pytest.approx( vsp.GetParmVal( vsp.FindParm( child, "Y_Min", "BBox" ) ) + 40.0, abs = 1e-6 )

    # With it off the Clone is posed on its own: zero deflection puts its child back where it
    # started, while the original stays folded.
    switch( hinge_clone, "CloneJoint", False )
    vsp.SetParmVal( vsp.FindParm( hinge_clone, "JointRotate", "Hinge" ), 0.0 )
    vsp.Update()

    assert box( child_clone ) == pytest.approx( unposed, abs = 1e-6 )
    assert vsp.GetParmVal( vsp.FindParm( hinge, "JointRotate", "Hinge" ) ) == pytest.approx( 35.0 ), \
           "the original should still be folded"
    assert_no_errors()


def testACopiedHingeChainDoesNotCollapse():
    """Each link of a cloned hinge chain attaches at its parent's tip, as in the original."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    root = vsp.AddGeom( "WING" )
    hinge = vsp.AddGeom( "HINGE", root )
    tip = vsp.AddGeom( "WING", hinge )
    # Half wings, so each link of the chain plainly steps outboard.
    for w in ( root, tip ):
        vsp.SetParmVal( vsp.FindParm( w, "Sym_Planar_Flag", "Sym" ), 0 )
    vsp.Update()

    # The hinge sits at the outboard end of the wing it hangs off.
    vsp.SetParmVal( vsp.FindParm( hinge, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( vsp.FindParm( hinge, "U_Attach_Location", "Attach" ), 1.0 )
    vsp.SetParmVal( vsp.FindParm( hinge, "V_Attach_Location", "Attach" ), 0.5 )
    vsp.Update()

    originals = [ root, hinge, tip ]
    before = chain_boxes( originals )

    clones = vsp.CloneGeomVec( originals )
    vsp.Update()
    paired = [ [ c for c in clones if vsp.GetGeomCloneOriginal( c ) == g ][0] for g in originals ]

    dy = 60.0
    vsp.SetParmVal( vsp.FindParm( paired[0], "Y_Rel_Location", "XForm" ), dy )
    vsp.Update()

    # The chain must actually step outboard for the test to mean anything.
    assert before[2][2] != pytest.approx( before[0][2], abs = 1e-6 )

    for original, clone, original_box in zip( originals, paired, before ):
        if vsp.GetGeomTypeName( original ) == "Hinge":
            # No shape to compare, but it moved with the group.
            assert vsp.GetParmVal( vsp.FindParm( clone, "Y_Location", "XForm" ) ) == \
                   pytest.approx( vsp.GetParmVal( vsp.FindParm( original, "Y_Location", "XForm" ) ) + dy,
                                  abs = 1e-6 )
            continue
        clone_box = box( clone )
        assert clone_box[0] == pytest.approx( original_box[0], abs = 1e-6 )   # X_Min
        assert clone_box[1] == pytest.approx( original_box[1], abs = 1e-6 )   # X_Len
        assert clone_box[2] == pytest.approx( original_box[2] + dy, abs = 1e-6 )
        assert clone_box[3] == pytest.approx( original_box[3], abs = 1e-6 )   # Y_Len
    assert_no_errors()


def testACloneOfACloneStillHasTheShape():
    """A Clone of a Clone gets its shape data from the original at the end of the chain."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    first = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( first, "X_Rel_Location", "XForm" ), 20.0 )
    vsp.Update()
    second = vsp.CloneGeomVec( [ first ] )[0]
    vsp.SetParmVal( vsp.FindParm( second, "X_Rel_Location", "XForm" ), 40.0 )
    vsp.Update()

    assert vsp.GetGeomCloneOriginal( second ) == first
    areas = comp_geom_areas()
    assert areas[ vsp.GetGeomName( pod ) ] > 0.0, "nothing was measured, so the rest says nothing"
    for g in ( first, second ):
        assert areas[ vsp.GetGeomName( g ) ] == pytest.approx( areas[ vsp.GetGeomName( pod ) ] )
    assert_no_errors()


def testEtaStationsReadThroughACloneOfAWing():
    """A Clone of a wing converts eta stations."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = vsp.AddGeom( "WING" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Location", "XForm" ), 8.0 )
    vsp.Update()

    assert vsp.ConvertEtatoU( clone, 0.4 ) == pytest.approx( vsp.ConvertEtatoU( wing, 0.4 ) )
    assert vsp.ConvertUtoEta( clone, 0.7 ) == pytest.approx( vsp.ConvertUtoEta( wing, 0.7 ) )
    assert_no_errors()


def testVspAeroSeesADiskForACloneOfAProp():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    prop = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( vsp.FindParm( prop, "PropMode", "Design" ), vsp.PROP_DISK )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ prop ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 12.0 )
    vsp.Update()

    vsp.SetParmVal( vsp.FindParm( vsp.FindContainer( "VSPAEROSettings", 0 ), "GeomSet", "VSPAERO" ),
                    vsp.SET_ALL )
    vsp.Update()

    assert vsp.GetNumActuatorDisks() == 2
    diameters = [ vsp.GetParmVal( vsp.FindParm( vsp.FindActuatorDisk( i ), "RotorDiameter", "Rotor" ) )
                  for i in range( 2 ) ]
    assert diameters[1] == pytest.approx( diameters[0] )
    assert diameters[0] == pytest.approx( vsp.GetParmVal( vsp.FindParm( prop, "Diameter", "Design" ) ) )
    assert_no_errors()


def testAnAuxiliaryGeomHangsOffACloneOfALandingGear():
    """An auxiliary geom on a Clone of a gear gets contact points placed where the Clone stands."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    gear = vsp.AddGeom( "GEAR" )
    bogies = []
    for x, y in ( ( 2.0, 0.0 ), ( 12.0, 4.0 ), ( 12.0, -4.0 ) ):
        b = vsp.CreateAndAddBogie( gear )
        vsp.SetParmVal( vsp.FindParm( b, "XContactPt", "Bogie" ), x )
        vsp.SetParmVal( vsp.FindParm( b, "YContactPt", "Bogie" ), y )
        vsp.SetParmVal( vsp.FindParm( b, "Symmetrical", "Bogie" ), 0 )
        bogies.append( b )
    vsp.Update()

    dx = 50.0
    gear_clone = vsp.CloneGeomVec( [ gear ] )[0]
    vsp.SetParmVal( vsp.FindParm( gear_clone, "X_Rel_Location", "XForm" ), dx )
    vsp.Update()

    def ground_plane( parent ):
        aux = vsp.AddGeom( "AUXILIARY", parent )
        vsp.SetParmVal( vsp.FindParm( aux, "AuxiliaryGeomType", "Design" ),
                        vsp.AUX_GEOM_THREE_PT_GROUND )
        vsp.Update()
        for i, bid in enumerate( bogies ):
            vsp.SetAuxiliaryGeomContactPtID( aux, i, bid )
        vsp.Update()
        lo = vsp.GetGeomBBoxMin( aux, 0, True )
        hi = vsp.GetGeomBBoxMax( aux, 0, True )
        return lo.x(), hi.x()

    on_gear = ground_plane( gear )
    on_clone = ground_plane( gear_clone )

    assert on_clone[0] == pytest.approx( on_gear[0] + dx, abs = 1e-4 )
    assert on_clone[1] == pytest.approx( on_gear[1] + dx, abs = 1e-4 )
    assert_no_errors()


def testACloneOfARouteWeighsWhatTheRouteWeighs():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    route = vsp.AddGeom( "ROUTING" )
    for u in ( 0.1, 0.5, 0.9 ):
        pt = vsp.AddRoutingPt( route, pod, 0 )
        vsp.SetParmVal( vsp.FindParm( pt, "U", "RoutePt" ), u )
        vsp.SetParmVal( vsp.FindParm( pt, "W", "RoutePt" ), 0.25 )
    vsp.SetParmVal( vsp.FindParm( route, "LinearDensity", "Mass_Props" ), 2.0 )
    vsp.Update()

    dz = 3.0
    clone = vsp.CloneGeomVec( [ route ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Location", "XForm" ), dz )
    vsp.Update()

    # A route carries mass, and the Clone carries it where the Clone sits.
    vsp.SetAnalysisInputDefaults( "MassProp" )
    res = vsp.ExecAnalysis( "MassProp" )
    names = vsp.GetStringResults( res, "Comp_Name" )
    masses = vsp.GetDoubleResults( res, "Comp_Mass" )
    cgs = vsp.GetVec3dResults( res, "Comp_CG" )

    # Matched by name, since row order is not guaranteed.
    rows = { n : i for i, n in enumerate( names ) if n.endswith( "_rg" ) }
    assert len( rows ) == 2
    a = rows[ vsp.GetGeomName( route ) + "_rg" ]
    b = rows[ vsp.GetGeomName( clone ) + "_rg" ]

    assert masses[b] == pytest.approx( masses[a] )
    assert masses[a] > 0.0
    assert cgs[b].z() == pytest.approx( cgs[a].z() + dz, abs = 1e-6 )
    assert cgs[b].x() == pytest.approx( cgs[a].x(), abs = 1e-6 )
    assert_no_errors()


def testACloneKeepsTheCfdTypeOfEachSurface():
    """A Clone keeps the per-surface CFD types of Geoms that set them (engine, auxiliary,
    scripted), rather than applying the whole-Geom negative volume flag."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    fuse = vsp.AddGeom( "FUSELAGE" )
    for parm, val in ( ( "GeomIOType", vsp.ENGINE_GEOM_INLET_OUTLET ),
                       ( "InletModeType", vsp.ENGINE_MODE_TO_FACE_NEG ),
                       ( "OutletModeType", vsp.ENGINE_MODE_TO_FACE_NEG ),
                       ( "InletFaceIndex", 0 ), ( "InletLipIndex", 1 ),
                       ( "OutletLipIndex", 2 ), ( "OutletFaceIndex", 3 ) ):
        vsp.SetParmVal( vsp.FindParm( fuse, parm, "EngineModel" ), val )
    vsp.Update()

    clone = vsp.CloneGeomVec( [ fuse ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 40.0 )
    vsp.Update()

    # Compared row by row, since one flipped sign of three could survive a comparison of totals.
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    res = vsp.FindLatestResultsID( "Comp_Geom" )
    names = list( vsp.GetStringResults( res, "Comp_Name" ) )
    vols = list( vsp.GetDoubleResults( res, "Theo_Vol" ) )

    mine = [ v for n, v in zip( names, vols ) if n == vsp.GetGeomName( fuse ) ]
    copied = [ v for n, v in zip( names, vols ) if n == vsp.GetGeomName( clone ) ]

    assert len( mine ) == 3, "expected one row per surface, got %s" % mine
    assert len( copied ) == len( mine )
    for a, b in zip( mine, copied ):
        assert b == pytest.approx( a )
    assert_no_errors()


def testACloneOfAnArrangementKeepsItsNegativeSurfaces():
    """The same for an auxiliary geom, which fixes the type of each surface."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    gear = vsp.AddGeom( "GEAR" )
    vsp.CreateAndAddBogie( gear )
    vsp.Update()
    aux = vsp.AddGeom( "AUXILIARY", gear )
    vsp.SetParmVal( vsp.FindParm( aux, "AuxiliaryGeomType", "Design" ),
                    vsp.AUX_GEOM_WHEEL_TIRE_FAILURE )
    vsp.Update()

    clone = vsp.CloneGeomVec( [ aux ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 60.0 )
    vsp.Update()

    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    res = vsp.FindLatestResultsID( "Comp_Geom" )
    names = list( vsp.GetStringResults( res, "Comp_Name" ) )
    vols = list( vsp.GetDoubleResults( res, "Theo_Vol" ) )

    mine = [ v for n, v in zip( names, vols ) if n == vsp.GetGeomName( aux ) ]
    copied = [ v for n, v in zip( names, vols ) if n == vsp.GetGeomName( clone ) ]

    assert len( mine ) > 1, "expected several surfaces, got %s" % mine
    assert any( v < 0.0 for v in mine ), "none of them is negative, so nothing is being measured"
    assert len( copied ) == len( mine )
    for a, b in zip( mine, copied ):
        assert b == pytest.approx( a )
    assert_no_errors()


def testACloneOfALandingGearReachesAsFarAsTheGear():
    """A Clone of a gear has the gear's box, which includes the gear's origin standing in for
    its ground plane."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    gear = vsp.AddGeom( "GEAR" )
    bogie = vsp.CreateAndAddBogie( gear )
    vsp.SetParmVal( vsp.FindParm( bogie, "XContactPt", "Bogie" ), 2.0 )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ gear ] )[0]

    assert vsp.GetParmVal( vsp.FindParm( gear, "X_Min", "BBox" ) ) == pytest.approx( 0.0, abs = 1e-6 ), \
           "the gear's own box does not reach its origin, so there is nothing to match"
    for parm in BBOX_PARMS:
        assert vsp.GetParmVal( vsp.FindParm( clone, parm, "BBox" ) ) == \
               pytest.approx( vsp.GetParmVal( vsp.FindParm( gear, parm, "BBox" ) ) )
    assert_no_errors()


def testTheClonesOwnSourcesAreAddedForMeshing():
    """A Clone gets the default CFD sources the Geom it copies would make."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 20.0 )
    vsp.Update()

    # No sources yet, so the counts below are a before-and-after.
    assert vsp.GetNumCFDSources( pod ) == 0
    assert vsp.GetNumCFDSources( clone ) == 0

    vsp.AddDefaultSources()
    vsp.Update()

    # The Clone gets as many as the pod makes for itself: three.
    assert vsp.GetNumCFDSources( pod ) == 3
    assert vsp.GetNumCFDSources( clone ) == vsp.GetNumCFDSources( pod )
    assert_no_errors()


def testTheMeshExportersCarryACloneOfAMesh():
    """Every mesh exporter writes a Clone of a mesh."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    vsp.DeleteGeom( pod )
    vsp.Update()
    mesh = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Mesh" ][0]
    clone = vsp.CloneGeomVec( [ mesh ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 40.0 )
    vsp.Update()

    out = tempfile.mkdtemp()
    multi = vsp.FindParm( vsp.FindContainer( "Vehicle", 0 ), "MultiSolid", "STLSettings" )

    # One solid per tag uses a separate writer, so check both settings.
    for solids in ( 0, 1 ):
        vsp.SetParmVal( multi, solids )
        vsp.Update()
        for ext, kind in ( ( "stl", vsp.EXPORT_STL ), ( "facet", vsp.EXPORT_FACET ),
                           ( "tri", vsp.EXPORT_CART3D ), ( "obj", vsp.EXPORT_OBJ ),
                           ( "msh", vsp.EXPORT_GMSH ), ( "vspgeom", vsp.EXPORT_VSPGEOM ) ):
            path = os.path.join( out, "both_%d.%s" % ( solids, ext ) )
            vsp.ExportFile( path, vsp.SET_ALL, kind )
            assert os.path.exists( path ), ext

            # Only the Clone's triangles reach x=40.
            text = open( path, "r", errors = "ignore" ).read()
            reach = max( [ float( v ) for v in re.findall( r"-?\d+\.\d+(?:[eE][-+]?\d+)?", text ) ] + [ 0.0 ] )
            assert reach > 39.0, "%s at MultiSolid=%d stops at %g, so the Clone is missing" \
                                 % ( ext, solids, reach )
    assert_no_errors()



def testACloneDescribesItselfInDegenerateForm():
    """A Clone of a mesh records its own transform in degenerate form."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    human = vsp.AddGeom( "HUMAN" )
    vsp.SetParmVal( vsp.FindParm( human, "Y_Rel_Location", "XForm" ), 5.0 )
    vsp.SetParmVal( vsp.FindParm( human, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ human ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 30.0 )
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 5.0 )
    vsp.Update()
    switch( clone, "CloneSym", False )
    vsp.SetParmVal( vsp.FindParm( clone, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()

    vsp.ComputeDegenGeom( vsp.SET_ALL, 0 )
    rows = degen_rows()

    mine = sorted( [ r for r in rows if r[0] == vsp.GetGeomName( clone ) ], key = lambda r: r[2] )
    theirs = sorted( [ r for r in rows if r[0] == vsp.GetGeomName( human ) ], key = lambda r: r[2] )

    # One entry per symmetric copy.
    assert len( theirs ) == 2
    assert len( mine ) == len( theirs )
    assert [ r[2] for r in mine ] == [ 0, 1 ]
    for ours, other in zip( mine, theirs ):
        assert ours[1] == other[1]                                       # the same kind of entry
        assert ours[3] == pytest.approx( other[3] + 30.0, abs = 1e-6 )   # x
        assert ours[4] == pytest.approx( other[4], abs = 1e-6 )          # y, mirrored copy too
        assert ours[5] == pytest.approx( other[5], abs = 1e-6 )
    assert_no_errors()


def testACloneOfAWireFrameIsInTheDegenerateSet():
    """A Clone of a wireframe appears in the degenerate set."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    lines = [ "1", "7 5 1" ]
    for axis in range( 3 ):
        vals = []
        for j in range( 5 ):
            for i in range( 7 ):
                xyz = ( 4.0 * i / 6.0, 2.0 * j / 4.0, 0.4 * ( i % 2 ) )
                vals.append( "%.10g" % xyz[axis] )
        lines.append( " ".join( vals ) )
    path = os.path.join( tempfile.mkdtemp(), "wire.p3d" )
    open( path, "w" ).write( "\n".join( lines ) + "\n" )
    vsp.ImportFile( path, vsp.IMPORT_P3D_WIRE, "" )
    vsp.Update()
    wire = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "WireFrame" ][0]
    clone = vsp.CloneGeomVec( [ wire ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 12.0 )
    vsp.Update()

    vsp.ComputeDegenGeom( vsp.SET_ALL, 0 )
    named = [ r[0] for r in degen_rows() ]

    assert vsp.GetGeomName( wire ) in named
    assert vsp.GetGeomName( clone ) in named
    assert_no_errors()


def testASuperConeHangsOffTheEyesOfTheHumanItIsOn():
    """A super cone on a Clone of a Human is placed from the Clone's eyes."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    human = vsp.AddGeom( "HUMAN" )
    vsp.Update()
    dy = 25.0
    clone = vsp.CloneGeomVec( [ human ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), dy )
    vsp.Update()

    def cone_on( parent ):
        aux = vsp.AddGeom( "AUXILIARY", parent )
        vsp.SetParmVal( vsp.FindParm( aux, "AuxiliaryGeomType", "Design" ), vsp.AUX_GEOM_SUPER_CONE )
        vsp.Update()
        return tuple( vsp.GetParmVal( vsp.FindParm( aux, n, "BBox" ) ) for n in BBOX_PARMS )

    on_human = cone_on( human )
    on_clone = cone_on( clone )

    assert on_human[3] > 0.0

    # Offset in y only, so x and z must match; they are what move if the cone were placed from
    # the origin instead of the eyes.
    assert on_clone[0] == pytest.approx( on_human[0], abs = 1e-6 )   # X_Min
    assert on_clone[1] == pytest.approx( on_human[1], abs = 1e-6 )   # X_Len
    assert on_clone[4] == pytest.approx( on_human[4], abs = 1e-6 )   # Z_Min
    assert on_clone[5] == pytest.approx( on_human[5], abs = 1e-6 )   # Z_Len
    assert on_clone[2] == pytest.approx( on_human[2] + dy, abs = 1e-6 )
    assert on_clone[3] == pytest.approx( on_human[3], abs = 1e-6 )
    assert_no_errors()


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
