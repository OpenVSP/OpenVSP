# What the rest of OpenVSP makes of a Clone.
#
# A Clone's type is always Clone, but it behaves like what it shows, so code that asks "is this
# a wing" must ask the behaviour.  Each consumer that does so is measured here.

import openvsp as vsp
import pytest
import glob
import os
import tempfile

from clonehelp import ( box, switch, scratch_output, drop_errors,
                        assert_no_errors )


def a_wing( span, offset = None ):
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( wing, "Span", "XSec_1" ), span )
    if offset is not None:
        for parm, val in zip( ( "X_Rel_Location", "Y_Rel_Location", "Z_Rel_Location" ), offset ):
            vsp.SetParmVal( vsp.FindParm( wing, parm, "XForm" ), val )
    vsp.Update()
    return wing


def rgb( colour ):
    return ( colour.x(), colour.y(), colour.z() )


def sref():
    return vsp.GetParmVal( vsp.FindParm( vsp.FindContainer( "VSPAEROSettings", 0 ), "Sref", "VSPAERO" ) )


def bref():
    return vsp.GetParmVal( vsp.FindParm( vsp.FindContainer( "VSPAEROSettings", 0 ), "bref", "VSPAERO" ) )


def testVspAeroTakesItsReferenceAreaFromACloneOfAWing():
    """A Clone of a wing can supply the reference area."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    small = a_wing( 9.0 )
    big = a_wing( 24.0, ( 0.0, 0.0, 5.0 ) )
    clone = vsp.CloneGeomVec( [ big ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 40.0 )
    vsp.Update()

    settings = vsp.FindContainer( "VSPAEROSettings", 0 )
    vsp.SetParmVal( vsp.FindParm( settings, "RefFlag", "VSPAERO" ), vsp.COMPONENT_REF )

    vsp.SetVSPAERORefWingID( small )
    vsp.Update()
    from_small = ( sref(), bref() )

    vsp.SetVSPAERORefWingID( clone )
    vsp.Update()
    from_clone = ( sref(), bref() )

    vsp.SetVSPAERORefWingID( big )
    vsp.Update()
    from_big = ( sref(), bref() )

    assert from_small != from_big, "the two wings measure the same, so the test proves nothing"
    assert from_clone == pytest.approx( from_big )
    assert_no_errors()


def testAConformalOfACloneTakesTheClonesShape():
    """A conformal on a Clone lofts from the Clone's placed surfaces."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    dx = 30.0
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), dx )
    vsp.Update()

    on_pod = vsp.AddGeom( "CONFORMAL", pod )
    on_clone = vsp.AddGeom( "CONFORMAL", clone )
    vsp.Update()

    assert box( on_pod )[1] > 0.0, "the conformal on the pod has no size, so nothing is measured"
    assert box( on_clone )[0] == pytest.approx( box( on_pod )[0] + dx )
    for i in ( 1, 3, 5 ):
        assert box( on_clone )[i] == pytest.approx( box( on_pod )[i] )
    assert_no_errors()


def testAGeomAttachesToACloneOfAWingByEtaAndMN():
    """Eta and MN on a Clone of a wing are answered by the wing it shows."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = a_wing( 12.0 )
    dz = 9.0
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Location", "XForm" ), dz )
    vsp.Update()

    def attached_to( parent ):
        pod = vsp.AddGeom( "POD", parent )
        vsp.SetParmVal( vsp.FindParm( pod, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_EtaMN )
        vsp.SetParmVal( vsp.FindParm( pod, "Rots_Attach_Flag", "Attach" ), vsp.ATTACH_ROT_COMP )
        vsp.SetParmVal( vsp.FindParm( pod, "Eta_Attach_Location", "Attach" ), 0.6 )
        vsp.SetParmVal( vsp.FindParm( pod, "M_Attach_Location", "Attach" ), 0.5 )
        vsp.SetParmVal( vsp.FindParm( pod, "N_Attach_Location", "Attach" ), 0.25 )
        vsp.Update()
        return ( vsp.GetParmVal( vsp.FindParm( pod, "X_Location", "XForm" ) ),
                 vsp.GetParmVal( vsp.FindParm( pod, "Y_Location", "XForm" ) ),
                 vsp.GetParmVal( vsp.FindParm( pod, "Z_Location", "XForm" ) ) )

    on_wing = attached_to( wing )
    on_clone = attached_to( clone )

    assert on_wing[1] > 0.0, "the eta attachment did not move the Geom, so nothing is measured"
    assert on_clone[0] == pytest.approx( on_wing[0], abs = 1e-6 )
    assert on_clone[1] == pytest.approx( on_wing[1], abs = 1e-6 )
    assert on_clone[2] == pytest.approx( on_wing[2] + dz, abs = 1e-6 )
    assert_no_errors()


def testAStructureOnACloneOfAWingIsMeshedWhereTheCloneSits():
    """A structure on a Clone lofts its parts where the Clone stands, using the Clone's frame
    rather than the original wing's."""
    vsp.VSPRenew()
    drop_errors()
    out = scratch_output()
    wing = a_wing( 12.0 )
    dz = 9.0
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Location", "XForm" ), dz )
    vsp.Update()

    def mesh_z_extent( parent, tag ):
        struct = vsp.AddFeaStruct( parent )
        vsp.AddFeaPart( parent, struct, vsp.FEA_RIB )
        vsp.AddFeaPart( parent, struct, vsp.FEA_SPAR )
        vsp.Update()
        path = os.path.join( out, "fea_%s.stl" % tag )
        vsp.SetFeaMeshFileName( parent, struct, vsp.FEA_STL_FILE_NAME, path )
        vsp.SetFeaMeshVal( parent, struct, vsp.CFD_MAX_EDGE_LEN, 1.0 )
        vsp.ComputeFeaMesh( parent, struct, vsp.FEA_STL_FILE_NAME )
        text = open( path ).read()
        zs = [ float( line.split()[3] ) for line in text.split( "\n" )
               if line.strip().startswith( "vertex" ) ]
        assert zs, "the structure mesh wrote no vertices"
        return ( min( zs ), max( zs ) )

    on_wing = mesh_z_extent( wing, "wing" )
    on_clone = mesh_z_extent( clone, "clone" )

    assert on_wing[1] > on_wing[0]
    assert on_clone[0] == pytest.approx( on_wing[0] + dz, abs = 1e-3 )
    assert on_clone[1] == pytest.approx( on_wing[1] + dz, abs = 1e-3 )
    assert_no_errors()


def testTheSetsSwitchDecidesWhoseSetMembershipTheCloneHas():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    user_set = 3

    vsp.SetSetFlag( pod, user_set, True )
    vsp.Update()
    assert vsp.GetSetFlag( clone, user_set ), "with the switch on the Clone follows the original"

    switch( clone, "CloneSets", False )
    vsp.SetSetFlag( clone, user_set, False )
    vsp.Update()
    assert vsp.GetSetFlag( pod, user_set )
    assert not vsp.GetSetFlag( clone, user_set ), "with it off the Clone keeps its own"

    switch( clone, "CloneSets", True )
    assert vsp.GetSetFlag( clone, user_set ), "turning it back on picks the original's up again"
    assert_no_errors()


def testTheAttachSwitchDecidesWhoseAttachmentTheCloneHas():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    child = vsp.AddGeom( "POD", pod )
    vsp.SetParmVal( vsp.FindParm( child, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( vsp.FindParm( child, "U_Attach_Location", "Attach" ), 0.5 )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ child ] )[0]

    assert vsp.GetParmVal( vsp.FindParm( clone, "Trans_Attach_Flag", "Attach" ) ) == \
           pytest.approx( vsp.ATTACH_TRANS_UV )

    switch( clone, "CloneAttach", False )
    vsp.SetParmVal( vsp.FindParm( clone, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_NONE )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( clone, "Trans_Attach_Flag", "Attach" ) ) == \
           pytest.approx( vsp.ATTACH_TRANS_NONE )
    assert vsp.GetParmVal( vsp.FindParm( child, "Trans_Attach_Flag", "Attach" ) ) == \
           pytest.approx( vsp.ATTACH_TRANS_UV ), "the original's attachment was written over"
    assert_no_errors()


def testTheAppearanceSwitchDecidesWhoseColourTheCloneHas():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    vsp.SetGeomWireColor( pod, 10, 20, 30 )
    vsp.Update()
    assert rgb( vsp.GetGeomWireColor( clone ) ) == rgb( vsp.GetGeomWireColor( pod ) )

    switch( clone, "CloneAppearance", False )
    vsp.SetGeomWireColor( clone, 200, 100, 50 )
    vsp.Update()
    # The exact colour set, not just one that differs from the pod's.
    assert rgb( vsp.GetGeomWireColor( clone ) ) == ( 200.0, 100.0, 50.0 )
    assert rgb( vsp.GetGeomWireColor( pod ) ) == ( 10.0, 20.0, 30.0 )
    assert_no_errors()


def testTheNegativeVolumeSwitchDecidesWhoseFlagTheCloneHas():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    vsp.SetParmVal( vsp.FindParm( pod, "Negative_Volume_Flag", "Negative_Volume_Props" ), 1.0 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( clone, "Negative_Volume_Flag", "Negative_Volume_Props" ) ) == pytest.approx( 1.0 )

    switch( clone, "CloneNegativeVolume", False )
    vsp.SetParmVal( vsp.FindParm( clone, "Negative_Volume_Flag", "Negative_Volume_Props" ), 0.0 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( clone, "Negative_Volume_Flag", "Negative_Volume_Props" ) ) == pytest.approx( 0.0 )
    assert vsp.GetParmVal( vsp.FindParm( pod, "Negative_Volume_Flag", "Negative_Volume_Props" ) ) == pytest.approx( 1.0 )
    assert_no_errors()


def testTurningSubSurfaceCopyingOffHandsTheCopiesOver():
    """The copied subsurfaces stop tracking the original and become the Clone's own."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = vsp.AddGeom( "WING" )
    ss = vsp.AddSubSurf( wing, vsp.SS_LINE )
    vsp.SetParmVal( vsp.FindParm( ss, "Const_Line_Value", "SubSurface" ), 0.3 )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.Update()

    copied = vsp.GetSubSurfIDVec( clone )
    assert len( copied ) == 1
    mirror = copied[0]
    assert vsp.GetParmVal( vsp.FindParm( mirror, "Const_Line_Value", "SubSurface" ) ) == pytest.approx( 0.3 )

    vsp.SetParmVal( vsp.FindParm( ss, "Const_Line_Value", "SubSurface" ), 0.7 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( mirror, "Const_Line_Value", "SubSurface" ) ) == pytest.approx( 0.7 )

    # The same subsurfaces stay, and stop following.
    switch( clone, "CloneSubSurfs", False )
    assert list( vsp.GetSubSurfIDVec( clone ) ) == list( copied )
    vsp.SetParmVal( vsp.FindParm( ss, "Const_Line_Value", "SubSurface" ), 0.4 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( mirror, "Const_Line_Value", "SubSurface" ) ) == pytest.approx( 0.7 )

    # They are the Clone's own, so the API will delete one.
    vsp.DeleteSubSurf( clone, mirror )
    vsp.Update()
    assert list( vsp.GetSubSurfIDVec( clone ) ) == []
    assert_no_errors()


def testTurningSubSurfaceCopyingBackOnTakesTheSameOnesUpAgain():
    """The pairing is kept while the switch is off, so turning it back on reuses the same copies
    rather than adding a second set."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = vsp.AddGeom( "WING" )
    vsp.AddSubSurf( wing, vsp.SS_CONTROL )
    vsp.AddSubSurf( wing, vsp.SS_RECTANGLE )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    assert vsp.GetNumSubSurf( clone ) == 2

    for cycle in range( 4 ):
        switch( clone, "CloneSubSurfs", False )
        switch( clone, "CloneSubSurfs", True )
        assert vsp.GetNumSubSurf( clone ) == 2, \
               "cycle %d left %d subsurfaces" % ( cycle, vsp.GetNumSubSurf( clone ) )

    # An edit on the original still reaches the Clone.
    ss = vsp.GetSubSurfIDVec( wing )[0]
    vsp.SetParmVal( vsp.FindParm( ss, "UStart", "SS_Control" ), 0.3 )
    vsp.Update()
    mine = [ s for s in vsp.GetSubSurfIDVec( clone ) if vsp.GetSubSurfType( s ) == vsp.SS_CONTROL ]
    assert len( mine ) == 1
    assert vsp.GetParmVal( vsp.FindParm( mine[0], "UStart", "SS_Control" ) ) == pytest.approx( 0.3 )

    # One deleted while the switch was off comes back when it goes on again.
    switch( clone, "CloneSubSurfs", False )
    vsp.DeleteSubSurf( clone, vsp.GetSubSurfIDVec( clone )[0] )
    vsp.Update()
    assert vsp.GetNumSubSurf( clone ) == 1
    switch( clone, "CloneSubSurfs", True )
    assert vsp.GetNumSubSurf( clone ) == 2
    assert_no_errors()


def testACloneKeepsOneSubSurfaceEachThroughAFile():
    """The pairing is saved, so a file written with copying off does not reload doubled."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = vsp.AddGeom( "WING" )
    vsp.AddSubSurf( wing, vsp.SS_CONTROL )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    switch( clone, "CloneSubSurfs", False )

    path = os.path.join( tempfile.mkdtemp(), "ss.vsp3" )
    vsp.WriteVSPFile( path, vsp.SET_ALL )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()

    read = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Clone" ][0]
    assert vsp.GetNumSubSurf( read ) == 1
    switch( read, "CloneSubSurfs", True )
    assert vsp.GetNumSubSurf( read ) == 1
    assert_no_errors()


def testAConformalOnACloneOfAWingTrimsByEta():
    """A conformal on a Clone of a wing maps its eta trim through the wing being shown."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = vsp.AddGeom( "WING" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]

    conformal = vsp.AddGeom( "CONFORMAL", clone )
    vsp.SetParmVal( vsp.FindParm( conformal, "UTrimFlag", "Design" ), 1 )
    vsp.SetParmVal( vsp.FindParm( conformal, "UMinTrimTypeFalg", "Design" ), vsp.ETA_TRIM )
    vsp.Update()
    before = box( conformal )

    vsp.SetParmVal( vsp.FindParm( conformal, "EtaTrimMin", "Design" ), 0.5 )
    vsp.Update()

    assert box( conformal ) != pytest.approx( before ), "the eta trim did nothing"
    assert box( conformal )[0] > before[0], "trimming inboard should move the near edge out"
    assert_no_errors()


def testAStructureOnACloneMeasuresTheChordOfTheClonesOwnSurface():
    """A spar on a Clone sizes itself from the Clone's surface, not the original wing's."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = a_wing( 12.0 )
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Location", "XForm" ), 9.0 )
    vsp.Update()

    def part_numbers( parent, kind ):
        struct = vsp.AddFeaStruct( parent )
        part = vsp.AddFeaPart( parent, struct, kind )
        vsp.Update()
        return ( vsp.GetParmVal( vsp.FindParm( part, "RelCenterLocation", "FeaPart" ) ),
                 vsp.GetParmVal( vsp.FindParm( part, "AbsCenterLocation", "FeaPart" ) ) )

    spar_wing = part_numbers( wing, vsp.FEA_SPAR )
    spar_clone = part_numbers( clone, vsp.FEA_SPAR )
    rib_wing = part_numbers( wing, vsp.FEA_RIB )
    rib_clone = part_numbers( clone, vsp.FEA_RIB )

    assert spar_wing[1] > 0.0, "the spar reported no chord, so nothing is measured"
    assert rib_wing[1] > 0.0, "the rib reported no span position, so nothing is measured"
    assert spar_clone == pytest.approx( spar_wing ), ( spar_wing, spar_clone )
    assert rib_clone == pytest.approx( rib_wing ), ( rib_wing, rib_clone )
    assert_no_errors()


def testExportPropMainSurfLeavesOutTheBladesOfACloneToo():
    """Exporting one blade instead of the whole rotor works on a Clone of a propeller too."""
    vsp.VSPRenew()
    drop_errors()
    out = scratch_output()

    def triangles( main_surf_only, with_clone ):
        vsp.VSPRenew()
        drop_errors()
        prop = vsp.AddGeom( "PROP" )
        # Symmetry on, so the whole rotor differs from the main surfaces.
        vsp.SetParmVal( vsp.FindParm( prop, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
        vsp.Update()
        if with_clone:
            vsp.CloneGeomVec( [ prop ] )
        vsp.Update()
        veh = vsp.FindContainer( "Vehicle", 0 )
        vsp.SetParmVal( vsp.FindParm( veh, "ExportPropMainSurf", "STLSettings" ),
                        1.0 if main_surf_only else 0.0 )
        vsp.Update()
        path = os.path.join( out, "prop.stl" )
        vsp.ExportFile( path, vsp.SET_ALL, vsp.EXPORT_STL )
        return open( path ).read().count( "facet normal" )

    prop_all = triangles( False, False )
    prop_main = triangles( True, False )
    both_all = triangles( False, True )
    both_main = triangles( True, True )

    assert prop_main < prop_all, "the flag did nothing even for the propeller itself"
    assert both_all == 2 * prop_all, ( both_all, prop_all )

    # The Clone shrinks by the same factor as the propeller.
    assert both_main == 2 * prop_main, ( both_main, prop_main )
    assert_no_errors()


def testAirfoilExportWritesTheClonesOwnFilesFromTheWingItShows():
    """A Set holding only a Clone exports its airfoils, named with the Clone's name and ID."""
    vsp.VSPRenew()
    drop_errors()
    out = scratch_output()
    wing = a_wing( 12.0 )
    vsp.SetGeomName( wing, "Mainplane" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]

    # A Clone copies Set membership, so turn that off to put the Clone in a Set alone.
    switch( clone, "CloneSets", False )
    only_clone = 4
    vsp.SetSetFlag( clone, only_clone, True )
    vsp.Update()
    assert vsp.GetSetFlag( clone, only_clone ) and not vsp.GetSetFlag( wing, only_clone )

    veh = vsp.FindContainer( "Vehicle", 0 )
    vsp.SetParmVal( vsp.FindParm( veh, "AFExportType", "AirfoilExport" ), vsp.SELIG_AF_EXPORT )
    meta = os.path.join( out, "af_clone.csv" )
    vsp.ExportFile( meta, only_clone, vsp.EXPORT_SELIG_AIRFOIL )

    dats = sorted( glob.glob( os.path.join( out, "*.dat" ) ) )
    assert dats, "a Set holding only the Clone exported no airfoils"

    text = open( meta ).read()
    names = set( line.split( ",", 1 )[1].strip() for line in text.split( "\n" )
                 if line.startswith( "Geom Name" ) )
    ids = set( line.split( ",", 1 )[1].strip() for line in text.split( "\n" )
               if line.startswith( "Geom ID" ) )

    assert names == { vsp.GetGeomName( clone ) }, names
    assert ids == { clone }, ids
    for path in dats:
        assert clone in os.path.basename( path ), os.path.basename( path )
    assert_no_errors()


def testAirfoilExportGivesTheWingAndItsCloneASetOfFilesEach():
    """Identical contents, distinct names: the Clone does not overwrite the original's files."""
    vsp.VSPRenew()
    drop_errors()
    out = scratch_output()
    wing = a_wing( 12.0 )
    vsp.SetGeomName( wing, "Mainplane" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.Update()

    veh = vsp.FindContainer( "Vehicle", 0 )
    vsp.SetParmVal( vsp.FindParm( veh, "AFExportType", "AirfoilExport" ), vsp.SELIG_AF_EXPORT )
    vsp.ExportFile( os.path.join( out, "af_both.csv" ), vsp.SET_ALL, vsp.EXPORT_SELIG_AIRFOIL )

    dats = [ os.path.basename( f ) for f in glob.glob( os.path.join( out, "*.dat" ) ) ]
    from_wing = sorted( f for f in dats if wing in f )
    from_clone = sorted( f for f in dats if clone in f )

    assert from_wing, "the wing exported nothing"
    assert len( from_clone ) == len( from_wing ), ( from_wing, from_clone )

    # The coordinates agree; only the name on a Selig file's first line differs.
    for a, b in zip( from_wing, from_clone ):
        wing_lines = open( os.path.join( out, a ) ).read().split( "\n" )
        clone_lines = open( os.path.join( out, b ) ).read().split( "\n" )
        assert a in wing_lines[0] and b in clone_lines[0], ( wing_lines[0], clone_lines[0] )
        assert wing_lines[0] != clone_lines[0]
        assert wing_lines[1:] == clone_lines[1:]
    assert_no_errors()

def testVspAeroPreparesGeometryFromACloneAsWellAsTheOriginal():
    """The mesh VSPAERO is given holds the Clones, not just the originals.

    Every Geom in the set must appear by ID in the tag key Prepare Geometry writes.  The model
    is a wing on a pod plus a Clone of each, the cloned wing on the cloned pod, to exercise
    attachment as well.
    """
    vsp.VSPRenew()
    drop_errors()
    out = scratch_output()
    vsp.SetComputationFileName( vsp.VSPAERO_VSPGEOM_TYPE, os.path.join( out, "prep.vspgeom" ) )

    pod = vsp.AddGeom( "POD" )
    wing = vsp.AddGeom( "WING", pod )
    vsp.Update()

    pod_clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( pod_clone, "Y_Rel_Location", "XForm" ), 20.0 )
    vsp.Update()
    wing_clone = vsp.AddGeom( "CLONE", pod_clone )
    vsp.SetGeomCloneOriginal( wing_clone, wing )
    vsp.Update()

    # SET_ALL is always true, so check visibility and that the Clones are Clones instead.
    wanted = { pod, wing, pod_clone, wing_clone }
    for gid in wanted:
        assert vsp.GetSetFlag( gid, vsp.SET_SHOWN ), "%s is not shown, so it is not meshed" % gid
    assert vsp.GetGeomTypeName( pod_clone ) == "Clone"
    assert vsp.GetGeomTypeName( wing_clone ) == "Clone"

    vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", ( vsp.SET_ALL, ) )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", ( -1, ) )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )
    vsp.Update()

    keys = glob.glob( os.path.join( out, "*.vkey" ) )
    assert keys, "Prepare Geometry wrote no tag key file"
    key = open( keys[0] ).read()

    missing = [ gid for gid in wanted if gid not in key ]
    assert not missing, "not in the prepared geometry: %s" % \
           [ ( gid, vsp.GetGeomName( gid ) ) for gid in missing ]

    # The Clones are separate parts, not one part counted twice.
    parts = [ line for line in key.split( "\n" )
              if line and line[0].isdigit() and line.count( "," ) >= 8 ]
    assert len( parts ) >= 4, "expected a part per surface of all four Geoms, got %d" % len( parts )
    assert_no_errors()


def testVspAeroPreparesGeometryFromClonesInBothTheThickAndThinSets():
    """The same, with bodies solid and lifting surfaces as plates.

    Thick and thin Geoms are gathered by separate branches of Vehicle::AddMeshGeom, so both
    pairs are checked.  Each Clone is moved off its original (with CloneAttach off and the
    attach flag cleared), since the intersection drops one of two coincident plates.
    """
    vsp.VSPRenew()
    drop_errors()
    out = scratch_output()
    vsp.SetComputationFileName( vsp.VSPAERO_VSPGEOM_TYPE, os.path.join( out, "mixed.vspgeom" ) )

    thick_set = 3
    thin_set = 4

    pod = vsp.AddGeom( "POD" )
    wing = vsp.AddGeom( "WING", pod )
    vsp.Update()

    pod_clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( pod_clone, "Y_Rel_Location", "XForm" ), 40.0 )
    vsp.Update()
    wing_clone = vsp.AddGeom( "CLONE" )
    vsp.SetGeomCloneOriginal( wing_clone, wing )
    vsp.Update()

    switch( wing_clone, "CloneAttach", False )
    vsp.SetParmVal( vsp.FindParm( wing_clone, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_NONE )
    vsp.SetParmVal( vsp.FindParm( wing_clone, "Y_Rel_Location", "XForm" ), 40.0 )
    vsp.Update()

    assert abs( box( wing )[2] - box( wing_clone )[2] ) > 1.0, \
           "the cloned wing is still sitting on the wing, so nothing below is measured"

    for gid in ( pod, pod_clone ):
        vsp.SetSetFlag( gid, thick_set, True )
    for gid in ( wing, wing_clone ):
        vsp.SetSetFlag( gid, thin_set, True )
    vsp.Update()

    vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", ( thick_set, ) )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", ( thin_set, ) )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )
    vsp.Update()

    keys = glob.glob( os.path.join( out, "*.vkey" ) )
    assert keys, "Prepare Geometry wrote no tag key file"
    key = open( keys[0] ).read()

    missing = [ ( gid, vsp.GetGeomName( gid ) ) for gid in ( pod, wing, pod_clone, wing_clone )
                if gid not in key ]
    assert not missing, "not in the prepared geometry: %s" % missing
    assert_no_errors()


#==== Textures ====#
# A Clone borrows its original's textures for drawing without copying them, so its own list
# (the one the Texture editor uses) stays empty while borrowing.  Drawing is only visible in the
# GUI; these tests check ownership and what a replacement ends up with.


def a_texture_file():
    """An image from the repo, by a path independent of the working directory."""
    here = os.path.dirname( os.path.abspath( __file__ ) )
    return os.path.join( here, "..", "..", "..", "examples", "textures", "nasa-logo.tga" )


def testACloneBorrowingTexturesOwnsNoneOfItsOwn():
    """Borrowed textures are not copied onto the Clone."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    tex = vsp.AttachGeomTexture( pod, a_texture_file() )
    assert tex, "the texture did not attach to the original"
    vsp.Update()

    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()
    assert list( vsp.GetGeomTextureIDVec( clone ) ) == [], \
           "the Clone kept a texture of its own instead of borrowing"
    assert list( vsp.GetGeomTextureIDVec( pod ) ) == [ tex ]
    assert_no_errors()


def testTheAppearanceSwitchDecidesWhoseTexturesTheCloneShows():
    """With the switch off the Clone has its own textures, and turning it on keeps them."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.AttachGeomTexture( pod, a_texture_file() )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()

    switch( clone, "CloneAppearance", False )
    own = vsp.AttachGeomTexture( clone, a_texture_file() )
    vsp.Update()
    assert list( vsp.GetGeomTextureIDVec( clone ) ) == [ own ]

    # Borrowing again keeps what was attached while it was off.
    switch( clone, "CloneAppearance", True )
    vsp.Update()
    assert list( vsp.GetGeomTextureIDVec( clone ) ) == [ own ], \
           "borrowing threw away the texture the user attached"
    assert_no_errors()


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
