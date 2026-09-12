# Clone Geom: Flip_Flag reflects the shape about the Clone's own coordinate planes, before
# placement and symmetry, so it changes the shape and not where the Clone stands.
#
# Each test uses a shape that is asymmetric about the plane flipped, or it measures nothing.
# Each also checks a volume, which reads negative if the faces were not rewound.

import openvsp as vsp
import pytest

from fliphelp import ( span, yspan, reflected, flip, a_chiral_wing, signed_volume,
                       read_stl, read_cart3d )
from clonehelp import ( switch, box, comp_geom_of,
                        comp_geom_areas_of, total_mass, scratch_output, drop_errors,
                        assert_no_errors, a_mesh, a_wireframe, a_point_cloud,
                        a_polygon_mesh, a_route )
import os


def a_symmetric_wing():
    """A wing as it comes: symmetric about XZ, so its two halves join at the root."""
    wing = vsp.AddGeom( "WING" )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( wing, "Sym_Planar_Flag", "Sym" ) ) == vsp.SYM_XZ, \
           "a wing is symmetric about XZ by default, which is what the flip stands in for"
    return wing


def a_half_wing_and_its_flip():
    """A half wing at the origin, and a Clone of it flipped about XZ.

    The flip matches the wing's own symmetry only while the root is on the Clone's XZ plane.
    """
    half = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( half, "Sym_Planar_Flag", "Sym" ), 0 )
    vsp.Update()
    assert yspan( half ) == pytest.approx( ( 0.0, 9.0 ) ), \
           "the wing is not on the origin, so its own flip is not the symmetric half"

    clone = vsp.CloneGeomVec( [ half ] )[0]
    assert yspan( clone ) == pytest.approx( ( 0.0, 9.0 ) ), "the Clone does not stand on the wing"

    flip( clone, vsp.SYM_XZ )

    # The callers' measurements would pass even if the flip did nothing.
    assert yspan( clone ) == pytest.approx( ( -9.0, 0.0 ) ), \
           "the flip did not reflect the Clone, so nothing below measures it"
    return half, clone


def testAFlippedCloneEnclosesWhatSymmetryEncloses():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = a_symmetric_wing()
    whole = box( wing )

    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    half, clone = a_half_wing_and_its_flip()

    # Together the half and the Clone span what the symmetric wing spans.
    assert yspan( clone ) == pytest.approx( reflected( yspan( half ) ) )
    assert ( min( yspan( half )[0], yspan( clone )[0] ),
             max( yspan( half )[1], yspan( clone )[1] ) ) == pytest.approx( ( whole[2], whole[2] + whole[3] ) )

    # X and Z are unchanged.
    for i in ( 0, 1, 4, 5 ):
        assert box( clone )[i] == pytest.approx( whole[i] )
    assert_no_errors()


def testAFlippedCloneWeighsWhatSymmetryWeighs():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    a_symmetric_wing()
    whole_mass = total_mass()

    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    a_half_wing_and_its_flip()

    assert total_mass() == pytest.approx( whole_mass, rel = 1e-5 )
    assert whole_mass > 0.0
    assert_no_errors()


def testAFlippedCloneFillsWhatSymmetryFills():
    """The two halves enclose the volume of the symmetric wing.

    Wetted area is larger by the two root caps, which symmetry drops and two separate Geoms keep;
    that difference is bounded.
    """
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = a_symmetric_wing()
    whole_areas, whole_vols = comp_geom_of( [ wing ] )
    whole_area = whole_areas[ vsp.GetGeomName( wing ) ]
    whole_vol = whole_vols[ vsp.GetGeomName( wing ) ]
    assert whole_vol > 0.0

    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    half, clone = a_half_wing_and_its_flip()
    areas, vols = comp_geom_of( [ half, clone ] )

    assert vols[ vsp.GetGeomName( clone ) ] == pytest.approx( vols[ vsp.GetGeomName( half ) ] )
    assert vols[ vsp.GetGeomName( half ) ] == pytest.approx( 0.5 * whole_vol )
    assert sum( vols.values() ) == pytest.approx( whole_vol )

    assert areas[ vsp.GetGeomName( clone ) ] == pytest.approx( areas[ vsp.GetGeomName( half ) ] )
    assert sum( areas.values() ) > whole_area, "the two loose root cross sections should show up"

    root_cap = sum( areas.values() ) - whole_area
    assert root_cap < 0.03 * whole_area, \
           "two root cross sections is all the difference should be, not a missing half"
    assert_no_errors()


def testAFlipTurnsTheShapeRoundAndLeavesTheCloneWhereItIs():
    """A Clone stood at y = 10 stays at y = 10 when flipped."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 10.0 )
    vsp.Update()
    before = box( clone )
    assert span( clone, "Y" ) == pytest.approx( tuple( y + 10.0 for y in span( pod, "Y" ) ) ), \
           "the Clone is not where it was put, so the rest measures nothing"

    # A pod is symmetric about its own XZ plane, so nothing changes.
    flip( clone, vsp.SYM_XZ )
    assert box( clone ) == pytest.approx( before )

    # About YZ nose and tail swap about the Clone's origin, and y is unchanged.
    flip( clone, vsp.SYM_YZ )
    assert span( clone, "X" ) == pytest.approx( reflected( span( pod, "X" ) ) )
    assert span( clone, "Y" ) == pytest.approx( ( before[2], before[2] + before[3] ) )
    assert_no_errors()


def testTheFlipIsAppliedBeforeTheRotation():
    """Flip then rotate 90 deg about Z puts a y = 0..9 wing at x = 0..9; the other order, or
    no flip, gives x = -9..0.
    """
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( wing, "Sym_Planar_Flag", "Sym" ), 0 )
    vsp.Update()

    def turned( flag ):
        clone = vsp.CloneGeomVec( [ wing ] )[0]
        vsp.SetParmVal( vsp.FindParm( clone, "Flip_Flag", "Sym" ), flag )
        vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Rotation", "XForm" ), 90.0 )
        vsp.Update()
        return span( clone, "X" )

    assert turned( 0 ) == pytest.approx( ( -9.0, 0.0 ), abs = 1e-6 )
    assert turned( vsp.SYM_XZ ) == pytest.approx( ( 0.0, 9.0 ), abs = 1e-6 )
    assert_no_errors()


def testEachPlaneReflectsTheShapeAboutItsOwnAxis():
    """Each plane reflects about the Clone's own origin and leaves the other two axes alone.

    The Clone is off the model origin in all three axes, so a reflection about the model origin
    would fail.
    """
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    here = ( 3.0, 4.0, 5.0 )
    wing = a_chiral_wing( here )
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    before = box( clone )
    assert before == pytest.approx( box( wing ) ), "the Clone does not stand on the wing"

    # Box indices: min and length of X, then Y, then Z.
    for flag, i, axis in ( ( vsp.SYM_YZ, 0, "X" ), ( vsp.SYM_XZ, 2, "Y" ), ( vsp.SYM_XY, 4, "Z" ) ):
        flip( clone, flag )
        after = box( clone )

        assert span( clone, axis ) == \
               pytest.approx( reflected( ( before[i], before[i] + before[i + 1] ), about = here[ i // 2 ] ) ), \
               "flipping about %s did not reflect the shape about the Clone's own origin" % axis
        assert after[i] != pytest.approx( before[i] ), \
               "the shape is symmetric about %s, so this measures nothing" % axis

        for j in ( 0, 2, 4 ):
            if j != i:
                assert after[j] == pytest.approx( before[j] )
                assert after[j + 1] == pytest.approx( before[j + 1] )

    assert_no_errors()


def testEveryCombinationOfFlipsLeavesTheCloneRightWayRound():
    """All eight flag combinations give a positive volume, odd and even plane counts alike."""
    for flag in range( 0, 8 ):
        vsp.VSPRenew()
        drop_errors()
        scratch_output()
        pod = vsp.AddGeom( "POD" )
        vsp.Update()
        clone = vsp.CloneGeomVec( [ pod ] )[0]
        vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 20.0 )
        vsp.Update()
        flip( clone, flag )

        areas, vols = comp_geom_of( [ pod, clone ] )
        assert vols[ vsp.GetGeomName( clone ) ] == pytest.approx( vols[ vsp.GetGeomName( pod ) ] ), \
               "flip flag %d came out inside out" % flag
        assert vols[ vsp.GetGeomName( pod ) ] > 0.0
        assert_no_errors()


def testSymmetryLaysItsCopiesOutFromTheFlippedShape():
    """Symmetry copies the flipped shape.

    A wing at y = 5 spans 5..14 plus its copy.  Flipped, it spans -4..5 and its copy -5..4.
    """
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( wing, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.SetParmVal( vsp.FindParm( wing, "Y_Rel_Location", "XForm" ), 5.0 )
    vsp.Update()

    clone = vsp.CloneGeomVec( [ wing ] )[0]
    assert yspan( clone ) == pytest.approx( ( -14.0, 14.0 ) )

    flip( clone, vsp.SYM_XZ )
    assert yspan( clone ) == pytest.approx( ( -5.0, 5.0 ) )

    # Neither copy is inside out.
    areas, vols = comp_geom_of( [ wing, clone ] )
    assert vols[ vsp.GetGeomName( clone ) ] == pytest.approx( vols[ vsp.GetGeomName( wing ) ] )
    assert vols[ vsp.GetGeomName( wing ) ] > 0.0
    assert_no_errors()


def testTheFlipTurnsTheShapeAndNotTheFrame():
    """A child attached to the Clone's frame does not move; one attached by UV follows the
    flipped surface.

    The child has no flip of its own, so a reflection in its attachment would leave it inside out.
    """
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( pod, "X_Rel_Location", "XForm" ), 3.0 )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    hung_off_the_frame = vsp.AddGeom( "POD", clone )
    vsp.SetParmVal( vsp.FindParm( hung_off_the_frame, "Trans_Attach_Flag", "Attach" ),
                    vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( vsp.FindParm( hung_off_the_frame, "Sym_Planar_Flag", "Sym" ), 0 )

    hung_off_the_surface = vsp.AddGeom( "POD", clone )
    vsp.SetParmVal( vsp.FindParm( hung_off_the_surface, "Trans_Attach_Flag", "Attach" ),
                    vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( vsp.FindParm( hung_off_the_surface, "U_Attach_Location", "Attach" ), 0.25 )
    vsp.SetParmVal( vsp.FindParm( hung_off_the_surface, "Sym_Planar_Flag", "Sym" ), 0 )
    vsp.Update()

    frame_before = span( hung_off_the_frame, "X" )
    surface_before = span( hung_off_the_surface, "X" )

    flip( clone, vsp.SYM_YZ )

    assert span( hung_off_the_frame, "X" ) == pytest.approx( frame_before )

    # The attachment point is reflected about the Clone's origin; the child's shape is not.
    after = span( hung_off_the_surface, "X" )
    assert after[0] == pytest.approx( 2.0 * 3.0 - surface_before[0] )
    assert after[1] - after[0] == pytest.approx( surface_before[1] - surface_before[0] )
    assert_no_errors()


def testAxialSymmetryCopiesTheFlippedShapeToo():
    """Every copy laid out by axial symmetry shows the flipped shape, and none is inside out.

    Uses a wing, since a pod looks the same flipped about XZ.
    """
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = a_chiral_wing()
    vsp.SetParmVal( vsp.FindParm( wing, "Sym_Axial_Flag", "Sym" ), vsp.SYM_ROT_X )
    vsp.SetParmVal( vsp.FindParm( wing, "Sym_Rot_N", "Sym" ), 3 )
    vsp.Update()

    clone = vsp.CloneGeomVec( [ wing ] )[0]
    assert vsp.GetTotalNumSurfs( clone ) == 3
    before = box( clone )

    flip( clone, vsp.SYM_XZ )
    assert box( clone ) != pytest.approx( before ), "the flip did nothing to the copies"

    assert vsp.GetTotalNumSurfs( clone ) == 3
    areas, vols = comp_geom_of( [ wing, clone ] )
    assert vols[ vsp.GetGeomName( clone ) ] == pytest.approx( vols[ vsp.GetGeomName( wing ) ] )
    assert vols[ vsp.GetGeomName( wing ) ] > 0.0
    assert_no_errors()


def testAFlipDoesNotReachAJointsChildren():
    """Flipping a Clone of a hinge leaves its child where it was and right way out."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    hinge = vsp.AddGeom( "HINGE" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ hinge ] )[0]

    child = vsp.AddGeom( "POD", clone )
    vsp.SetParmVal( vsp.FindParm( child, "Y_Rel_Location", "XForm" ), 5.0 )
    vsp.SetParmVal( vsp.FindParm( child, "Sym_Planar_Flag", "Sym" ), 0 )
    vsp.Update()

    before = box( child )
    areas, vols = comp_geom_of( [ child ] )
    volume = vols[ vsp.GetGeomName( child ) ]
    assert volume > 0.0

    flip( clone, vsp.SYM_XZ )

    assert box( child ) == pytest.approx( before )
    areas, vols = comp_geom_of( [ child ] )
    assert vols[ vsp.GetGeomName( child ) ] == pytest.approx( volume )
    assert_no_errors()


def testACloneOfAFlippedCloneIsFlippedToo():
    """A Clone of a flipped Clone inherits the flip along with the symmetry.

    Its own Flip_Flag stays 0; turning CloneSym off removes the inherited flip.
    """
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = a_chiral_wing( ( 0.0, 4.0, 0.0 ) )

    inner = vsp.CloneGeomVec( [ wing ] )[0]
    unflipped = yspan( inner )
    flip( inner, vsp.SYM_XZ )
    assert yspan( inner ) == pytest.approx( reflected( unflipped, about = 4.0 ) )

    outer = vsp.CloneGeomVec( [ inner ] )[0]
    assert vsp.GetParmVal( vsp.FindParm( outer, "Flip_Flag", "Sym" ) ) == 0
    assert yspan( outer ) == pytest.approx( yspan( inner ) )

    # Separate the three in x: CompGeom on coincident solids is unreliable.  The y spans are
    # unaffected.
    for g, xoff in ( ( inner, 20.0 ), ( outer, 40.0 ) ):
        vsp.SetParmVal( vsp.FindParm( g, "X_Rel_Location", "XForm" ), xoff )
    vsp.Update()

    areas, vols = comp_geom_of( [ wing, inner, outer ] )
    for g in ( inner, outer ):
        assert vols[ vsp.GetGeomName( g ) ] == pytest.approx( vols[ vsp.GetGeomName( wing ) ] )
    assert vols[ vsp.GetGeomName( wing ) ] > 0.0

    switch( outer, "CloneSym", False )
    assert yspan( outer ) == pytest.approx( unflipped )
    assert_no_errors()


def testTheFlipSurvivesARoundTrip():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    import os
    import tempfile

    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( pod, "X_Rel_Location", "XForm" ), 2.0 )
    vsp.SetParmVal( vsp.FindParm( pod, "Y_Rel_Location", "XForm" ), 3.0 )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    flip( clone, vsp.SYM_XZ | vsp.SYM_YZ )
    before = box( clone )

    path = os.path.join( tempfile.mkdtemp(), "flip.vsp3" )
    vsp.WriteVSPFile( path, vsp.SET_ALL )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()

    read = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Clone" ][0]
    assert vsp.GetParmVal( vsp.FindParm( read, "Flip_Flag", "Sym" ) ) == vsp.SYM_XZ | vsp.SYM_YZ
    assert box( read ) == pytest.approx( before )
    assert_no_errors()


#==== Every kind of shape a Clone can borrow ====#
# Non-surface shapes are placed by a single matrix, not by symmetry, so each is tested.

@pytest.mark.parametrize( "make", [ a_mesh,
                                    lambda: vsp.AddGeom( "HUMAN" ),
                                    a_point_cloud,
                                    a_wireframe,
                                    a_polygon_mesh ] )
def testAFlipReflectsEveryBorrowedShape( make ):
    """Flipping about YZ reflects the x span; every one of these is asymmetric in x."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    original = make()
    vsp.Update()

    clone = vsp.CloneGeomVec( [ original ] )[0]
    before = span( clone, "X" )
    assert before != pytest.approx( reflected( before ) ), \
           "the shape is symmetric in x, so a flip about YZ would measure nothing"

    # A polygon mesh reports no bounding box, so skip this check for it.
    on_the_original = span( original, "X" )
    if on_the_original[1] > on_the_original[0]:
        assert before == pytest.approx( on_the_original ), "the Clone does not stand on the original"

    flip( clone, vsp.SYM_YZ )
    assert span( clone, "X" ) == pytest.approx( reflected( before ) )
    assert_no_errors()


def testACloneOfARouteIsNotFlipped():
    """A route's points sit on other Geoms, so flipping a Clone of one does not move it."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    clone = vsp.CloneGeomVec( [ a_route() ] )[0]
    vsp.Update()
    before = box( clone )
    assert span( clone, "X" ) != pytest.approx( reflected( span( clone, "X" ) ) ), \
           "the route is symmetric in x, so a flip about YZ would measure nothing"

    flip( clone, vsp.SYM_YZ )
    assert box( clone ) == pytest.approx( before )
    assert_no_errors()


@pytest.mark.parametrize( "make", [ a_mesh,
                                    lambda: vsp.AddGeom( "HUMAN" ),
                                    a_wireframe,
                                    a_polygon_mesh ] )
@pytest.mark.parametrize( "flag", [ vsp.SYM_XZ, vsp.SYM_XY | vsp.SYM_YZ,
                                    vsp.SYM_XY | vsp.SYM_XZ | vsp.SYM_YZ ] )
def testAFlippedBorrowedShapeIsNotInsideOut( make, flag ):
    """The triangles handed to an analysis are wound the right way.

    Checked by volume, which goes negative on misordered triangles; area does not.  Covers odd
    and even numbers of planes.
    """
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    original = make()
    clone = vsp.CloneGeomVec( [ original ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 30.0 )
    vsp.Update()
    flip( clone, flag )

    areas, vols = comp_geom_of( [ original, clone ] )
    on_original = vsp.GetGeomName( original )
    on_clone = vsp.GetGeomName( clone )

    assert vols[ on_original ] > 0.0, "the shape enclosed no volume, so nothing is measured"
    assert vols[ on_clone ] == pytest.approx( vols[ on_original ] ), \
           "the flipped Clone is inside out: %+f against %+f" % ( vols[ on_clone ], vols[ on_original ] )
    assert areas[ on_clone ] == pytest.approx( areas[ on_original ] )
    assert_no_errors()


def testAShapeBuiltFromAFlippedOneIsFlippedToo():
    """A conformal on a flipped Clone is flipped with it and stays inside it."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( pod, "X_Rel_Location", "XForm" ), 5.0 )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    conformal = vsp.AddGeom( "CONFORMAL", clone )
    vsp.Update()
    before = span( conformal, "X" )
    assert before[1] > before[0], "the conformal has no size, so nothing is measured"

    flip( clone, vsp.SYM_YZ )

    after = span( conformal, "X" )
    assert after == pytest.approx( reflected( before, about = 5.0 ) )

    assert after[0] > span( clone, "X" )[0]
    assert after[1] < span( clone, "X" )[1]

    areas, vols = comp_geom_of( [ pod, clone, conformal ] )
    assert vols[ vsp.GetGeomName( conformal ) ] > 0.0
    assert_no_errors()


def testAnArrangementBuiltOnAFlippedGearIsFlippedToo():
    """A ground plane on a flipped Clone of a gear is built from the flipped contact points."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    gear = vsp.AddGeom( "GEAR" )
    bogies = []
    for x, y in ( ( 2.0, 6.0 ), ( 12.0, 10.0 ), ( 12.0, 2.0 ) ):
        b = vsp.CreateAndAddBogie( gear )
        vsp.SetParmVal( vsp.FindParm( b, "XContactPt", "Bogie" ), x )
        vsp.SetParmVal( vsp.FindParm( b, "YContactPt", "Bogie" ), y )
        vsp.SetParmVal( vsp.FindParm( b, "Symmetrical", "Bogie" ), 0 )
        bogies.append( b )

    # The nominal ground plane is sized from the whole model and would swamp the measurement.
    vsp.SetParmVal( vsp.FindParm( gear, "ShowNominalGroundPlane", "GroundPlane" ), 0 )
    vsp.Update()

    clone = vsp.CloneGeomVec( [ gear ] )[0]

    # The plane's extent is sized from the model, so compare its centre.
    def plane_centre_on( parent ):
        aux = vsp.AddGeom( "AUXILIARY", parent )
        vsp.SetParmVal( vsp.FindParm( aux, "AuxiliaryGeomType", "Design" ),
                        vsp.AUX_GEOM_THREE_PT_GROUND )
        vsp.Update()
        for i, bid in enumerate( bogies ):
            vsp.SetAuxiliaryGeomContactPtID( aux, i, bid )
        vsp.Update()
        span = yspan( aux )
        assert span[1] > span[0], "the ground plane has no size, so nothing is measured"
        return 0.5 * ( span[0] + span[1] )

    # Contact points at y = 6, 10 and 2 centre the plane at 6.
    on_gear = plane_centre_on( gear )
    assert on_gear == pytest.approx( 6.0, abs = 1e-4 )

    flip( clone, vsp.SYM_XZ )
    on_clone = plane_centre_on( clone )

    assert on_clone == pytest.approx( -on_gear, abs = 1e-4 )
    assert_no_errors()


def testAFlippedWireFrameFacesTheOtherWay():
    """Flipping a wireframe's shape toggles its flip_normal flag, which also sets triangle winding."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wire = a_wireframe()
    clone = vsp.CloneGeomVec( [ wire ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 30.0 )
    vsp.Update()

    vsp.SetAnalysisInputDefaults( "DegenGeom" )
    vsp.SetIntAnalysisInput( "DegenGeom", "WriteMFileFlag", [ 0 ] )
    vsp.SetIntAnalysisInput( "DegenGeom", "WriteCSVFlag", [ 0 ] )

    def facing( gid ):
        # Clear an earlier run's entry for this Geom.
        vsp.DeleteAllResults()
        vsp.ExecAnalysis( "DegenGeom" )
        for i in range( vsp.GetNumResults( "Degen_DegenGeom" ) ):
            res = vsp.FindResultsID( "Degen_DegenGeom", i )
            if list( vsp.GetStringResults( res, "geom_id" ) )[0] == gid:
                return list( vsp.GetIntResults( res, "flip_normal" ) )[0]
        assert False, "the Geom described itself nowhere"

    assert facing( clone ) == facing( wire )
    flip( clone, vsp.SYM_XZ )
    assert facing( clone ) != facing( wire )
    assert_no_errors()


def testAFlippedRotorThrowsItsBladeTheOtherWay():
    """A thrown blade on a flipped Clone of a prop is the prop's blade reflected about the
    Clone's XZ plane.

    Throw distance is sized from the whole model, so the blades are compared after flipping.
    """
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    prop = vsp.AddGeom( "PROP" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ prop ] )[0]
    here = 10.0
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), here )
    vsp.Update()

    def a_blade_on( parent ):
        aux = vsp.AddGeom( "AUXILIARY", parent )
        vsp.SetParmVal( vsp.FindParm( aux, "AuxiliaryGeomType", "Design" ), vsp.AUX_GEOM_THROWN_BLADE )
        vsp.Update()
        return aux

    on_prop = a_blade_on( prop )
    on_clone = a_blade_on( clone )

    def moved():
        """The prop's blade, moved to where the Clone stands."""
        return tuple( y + here for y in yspan( on_prop ) )

    assert yspan( on_clone ) == pytest.approx( moved() )
    assert moved() != pytest.approx( reflected( moved(), about = here ) ), \
           "the blade is symmetric about the Clone's XZ plane, so a flip would measure nothing"

    flip( clone, vsp.SYM_XZ )

    assert yspan( on_clone ) == pytest.approx( reflected( moved(), about = here ) )
    for i in ( 0, 1, 4, 5 ):
        assert box( on_clone )[i] == pytest.approx( box( on_prop )[i] )
    assert_no_errors()


#==== The written files, not just the analyses ====#

def _only_in_a_set( gid, others ):
    """Put one Geom alone in a Set.  Turns off CloneSets so the Set is not copied."""
    switch( gid, "CloneSets", False )
    only = 4
    vsp.SetSetFlag( gid, only, True )
    for other in others:
        vsp.SetSetFlag( other, only, False )
    vsp.Update()
    assert vsp.GetSetFlag( gid, only )
    return only


@pytest.mark.parametrize( "kind, export, read", [
    ( "stl", vsp.EXPORT_STL, read_stl ),
    ( "tri", vsp.EXPORT_CART3D, read_cart3d ) ] )
def testAFlippedCloneIsWrittenOutRightWayRound( kind, export, read ):
    """An exported flipped Clone has the same signed volume as an unflipped one.

    Writers apply the placement matrix themselves, so each must reverse the winding itself.
    """
    out = scratch_output()

    def written_volume( flipped ):
        vsp.VSPRenew()
        drop_errors()
        original = a_mesh()
        vsp.Update()
        clone = vsp.CloneGeomVec( [ original ] )[0]
        vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 20.0 )
        vsp.Update()
        if flipped:
            flip( clone, vsp.SYM_XZ )
        only = _only_in_a_set( clone, [ original ] )
        path = os.path.join( out, "flip_%s_%d.%s" % ( kind, flipped, kind ) )
        vsp.ExportFile( path, only, export )
        nodes, tris = read( path )
        assert tris, "the export wrote no triangles"
        return signed_volume( nodes, tris )

    plain = written_volume( False )
    flipped = written_volume( True )

    assert abs( plain ) > 1e-6, "the mesh enclosed no volume, so nothing is measured"
    assert flipped == pytest.approx( plain ), \
           "the flipped Clone was written inside out: %+f against %+f" % ( flipped, plain )
    assert_no_errors()


#==== A joint flipped along with the Geom that carries it ====#

def a_hinge( primary_dir ):
    """A hinge at the origin, free to slide as well as turn."""
    hinge = vsp.AddGeom( "HINGE" )
    vsp.SetParmVal( vsp.FindParm( hinge, "PrimaryDir", "Hinge" ), primary_dir )
    vsp.SetParmVal( vsp.FindParm( hinge, "JointTranslateFlag", "Hinge" ), 1.0 )
    vsp.Update()
    return hinge


def on_joint( parent, rel ):
    """A Geom carried by a joint, placed off the axis so the motion shows."""
    pod = vsp.AddGeom( "POD", parent )
    vsp.SetParmVal( vsp.FindParm( pod, "Sym_Planar_Flag", "Sym" ), 0.0 )
    vsp.SetParmVal( vsp.FindParm( pod, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( vsp.FindParm( pod, "Rots_Attach_Flag", "Attach" ), vsp.ATTACH_ROT_COMP )
    for axis, val in zip( "XYZ", rel ):
        vsp.SetParmVal( vsp.FindParm( pod, axis + "_Rel_Location", "XForm" ), val )
    vsp.Update()
    return pod


def placed( gid ):
    return tuple( vsp.GetParmVal( vsp.FindParm( gid, axis + "_Location", "XForm" ) )
                  for axis in "XYZ" )


def pose( gid, rotate, translate ):
    vsp.SetParmVal( vsp.FindParm( gid, "JointRotate", "Hinge" ), rotate )
    vsp.SetParmVal( vsp.FindParm( gid, "JointTranslate", "Hinge" ), translate )


@pytest.mark.parametrize( "primary_dir", [ vsp.X_DIR, vsp.Y_DIR, vsp.Z_DIR ] )
@pytest.mark.parametrize( "rotate, translate", [ ( 0.0, 0.0 ), ( 25.0, 0.0 ),
                                                 ( 0.0, 3.0 ), ( 25.0, 3.0 ) ] )
def testAFlippedJointArticulatesAsItsReflection( primary_dir, rotate, translate ):
    """At the same deflection, a flipped Clone of a hinge carries its child to the reflection
    of where the hinge carries its own (e.g. the other side of an all-moving tail).

    The flip applies to the joint motion as F * J * F, which stays rigid.  Flipping about a
    plane containing the axis reverses the rotation; about the plane normal to it, the
    translation.  Hence all three axis directions.
    """
    vsp.VSPRenew()
    drop_errors()
    scratch_output()

    hinge = a_hinge( primary_dir )
    at = ( 0.5, 1.0, 0.25 )
    kid = on_joint( hinge, at )

    clone = vsp.AddGeom( "CLONE" )
    vsp.SetGeomCloneOriginal( clone, hinge )
    vsp.Update()

    switch( clone, "CloneJoint", False )
    flip( clone, vsp.SYM_XZ )

    # Start the two children at reflected positions.
    ckid = on_joint( clone, ( at[0], -at[1], at[2] ) )

    pose( hinge, rotate, translate )
    pose( clone, rotate, translate )
    vsp.Update()

    on_hinge = placed( kid )
    on_clone = placed( ckid )

    assert on_clone[0] == pytest.approx( on_hinge[0], abs = 1e-9 )
    assert on_clone[1] == pytest.approx( -on_hinge[1], abs = 1e-9 )
    assert on_clone[2] == pytest.approx( on_hinge[2], abs = 1e-9 )
    assert_no_errors()


def testAFlippedJointsMotionIsNotJustTheUnflippedOne():
    """Guards the test above against a flip that does nothing.

    Flipping about XZ reverses rotation about x and translation along y, and leaves rotation
    about y unchanged.
    """
    vsp.VSPRenew()
    drop_errors()
    scratch_output()

    def carried_to( primary_dir, rotate, translate, flipped ):
        vsp.VSPRenew()
        drop_errors()
        hinge = a_hinge( primary_dir )
        clone = vsp.AddGeom( "CLONE" )
        vsp.SetGeomCloneOriginal( clone, hinge )
        vsp.Update()
        switch( clone, "CloneJoint", False )
        if flipped:
            flip( clone, vsp.SYM_XZ )
        kid = on_joint( clone, ( 0.5, 1.0, 0.25 ) )
        pose( clone, rotate, translate )
        vsp.Update()
        return placed( kid )

    # Rotation about x reverses.
    assert carried_to( vsp.X_DIR, 25.0, 0.0, True ) != \
           pytest.approx( carried_to( vsp.X_DIR, 25.0, 0.0, False ), abs = 1e-6 )

    # Translation along y reverses.
    assert carried_to( vsp.Y_DIR, 0.0, 3.0, True ) != \
           pytest.approx( carried_to( vsp.Y_DIR, 0.0, 3.0, False ), abs = 1e-6 )

    # Rotation about y is unchanged.
    assert carried_to( vsp.Y_DIR, 25.0, 0.0, True ) == \
           pytest.approx( carried_to( vsp.Y_DIR, 25.0, 0.0, False ), abs = 1e-9 )
    assert_no_errors()


def testAFlippedJointStillHandsItsChildrenAnUnreflectedFrame():
    """A child of a flipped joint keeps its volume for any flip flag: F * J * F is rigid."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()

    hinge = a_hinge( vsp.X_DIR )
    clone = vsp.AddGeom( "CLONE" )
    vsp.SetGeomCloneOriginal( clone, hinge )
    vsp.Update()
    switch( clone, "CloneJoint", False )

    kid = on_joint( clone, ( 0.5, 1.0, 0.25 ) )
    pose( clone, 25.0, 3.0 )
    vsp.Update()

    areas, vols = comp_geom_of( [ kid ] )
    plain = vols[ vsp.GetGeomName( kid ) ]
    assert plain > 0.0

    for flag in ( vsp.SYM_XZ, vsp.SYM_XY | vsp.SYM_YZ, vsp.SYM_XY | vsp.SYM_XZ | vsp.SYM_YZ ):
        flip( clone, flag )
        areas, vols = comp_geom_of( [ kid ] )
        assert vols[ vsp.GetGeomName( kid ) ] == pytest.approx( plain ), \
               "flip flag %d turned a child of the joint inside out" % flag
    assert_no_errors()


def testAFlipReachesAShapeSymmetryCannotLayOut():
    """A Clone of a mesh ignores symmetry, as the mesh does, but still honours the flip."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    mesh = a_mesh()
    clone = vsp.CloneGeomVec( [ mesh ] )[0]
    before = span( clone, "X" )

    switch( clone, "CloneSym", False )
    vsp.SetParmVal( vsp.FindParm( clone, "Sym_Planar_Flag", "Sym" ), vsp.SYM_YZ )
    vsp.Update()
    assert span( clone, "X" ) == pytest.approx( before )

    flip( clone, vsp.SYM_YZ )
    assert span( clone, "X" ) == pytest.approx( reflected( before ) )
    assert_no_errors()


def testACloneOfAFlippedJointMovesItsChildrenFlippedToo():
    """A Clone of a flipped Clone of a hinge moves its child as the flipped Clone does."""
    vsp.VSPRenew()
    drop_errors()
    hinge = vsp.AddGeom( "HINGE" )
    vsp.Update()
    inner = vsp.CloneGeomVec( [ hinge ] )[0]
    vsp.SetParmVal( vsp.FindParm( inner, "Flip_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()
    outer = vsp.CloneGeomVec( [ inner ] )[0]
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( outer, "Flip_Flag", "Sym" ) ) == 0

    children = []
    for joint in ( hinge, inner, outer ):
        if joint != hinge:
            switch( joint, "CloneJoint", False )
        vsp.SetParmVal( vsp.FindParm( joint, "JointRotate", "Hinge" ), 30.0 )

        pod = vsp.AddGeom( "POD", joint )
        vsp.SetParmVal( vsp.FindParm( pod, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_COMP )
        vsp.SetParmVal( vsp.FindParm( pod, "Rots_Attach_Flag", "Attach" ), vsp.ATTACH_ROT_COMP )
        vsp.SetParmVal( vsp.FindParm( pod, "Y_Rel_Location", "XForm" ), 5.0 )
        children.append( pod )
    vsp.Update()

    pts = [ vsp.CompPnt01( pod, 0, 0.5, 0.0 ) for pod in children ]

    # The flip must move the child, or this measures nothing.
    assert abs( pts[1].z() - pts[0].z() ) > 1.0

    for a, b in ( ( pts[2].x(), pts[1].x() ), ( pts[2].y(), pts[1].y() ), ( pts[2].z(), pts[1].z() ) ):
        assert a == pytest.approx( b, abs=1e-9 )
    assert_no_errors()
