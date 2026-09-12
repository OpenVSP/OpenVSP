# Clone Geom: the basics.
#
# A Clone shows another Geom's shape, and its Behavior switches choose which of the original's
# properties it copies.

import openvsp as vsp
import pytest

from clonehelp import ( box, switch, comp_geom_areas,
                        scratch_output, drop_errors, assert_refused,
                        assert_no_errors )
import os
import tempfile


def testCloneShowsItsOriginalsShape():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )

    # Moved and rotated, so matching placement is not just both sitting at the origin.
    for parm, val in ( ( "X_Rel_Location", 3.0 ), ( "Y_Rel_Location", 4.0 ),
                       ( "Z_Rel_Location", 5.0 ), ( "Z_Rel_Rotation", 30.0 ) ):
        vsp.SetParmVal( vsp.FindParm( pod, parm, "XForm" ), val )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    # The type stays Clone whatever it shows; the screens, browser and file key on it.
    assert vsp.GetGeomTypeName( clone ) == "Clone"
    assert vsp.GetGeomCloneOriginal( clone ) == pod

    # The shape comes across, and the Clone starts where the original sits.
    assert vsp.GetNumMainSurfs( clone ) == vsp.GetNumMainSurfs( pod )
    assert box( pod )[0] == pytest.approx( 3.0, abs = 1.0 ), "the pod did not move"
    assert box( clone ) == pytest.approx( box( pod ) )

    assert_no_errors()


def testCloneMeasuresTheSameAsADuplicate():
    """A Clone is the same component as its original to an analysis."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()

    vsp.CopyGeomToClipboard( pod )
    dup = vsp.PasteGeomClipboard()[0]
    vsp.SetGeomName( dup, "Duplicate" )
    vsp.SetParmVal( vsp.FindParm( dup, "X_Rel_Location", "XForm" ), 20.0 )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 40.0 )
    vsp.Update()

    areas = comp_geom_areas()
    assert areas[ vsp.GetGeomName( pod ) ] > 0.0, "nothing was measured, so the rest says nothing"
    assert areas[ vsp.GetGeomName( clone ) ] == pytest.approx( areas[ vsp.GetGeomName( pod ) ] )
    assert areas[ vsp.GetGeomName( clone ) ] == pytest.approx( areas[ vsp.GetGeomName( dup ) ] )
    assert_no_errors()


def testCloneXFormSwitchDecidesWhoPlacesTheClone():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    # Off, the default: the Clone keeps its own placement.
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 25.0 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( clone, "X_Min", "BBox" ) ) == pytest.approx( 25.0 )
    assert vsp.GetParmVal( vsp.FindParm( clone, "X_Len", "BBox" ) ) == \
           pytest.approx( vsp.GetParmVal( vsp.FindParm( pod, "X_Len", "BBox" ) ) )

    # On: the placement is the original's, and a value set on the Clone is overwritten.
    switch( clone, "CloneXForm", True )
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 25.0 )
    vsp.Update()
    assert box( clone ) == pytest.approx( box( pod ) )
    assert_no_errors()


def testCloneSymSwitchDecidesWhoseSymmetryApplies():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 6.0 )
    vsp.Update()
    y_len_one = vsp.GetParmVal( vsp.FindParm( clone, "Y_Len", "BBox" ) )

    # With CloneSym on, the Clone's own symmetry setting is overwritten by the original's.
    vsp.SetParmVal( vsp.FindParm( clone, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( clone, "Y_Len", "BBox" ) ) == pytest.approx( y_len_one )

    # With it off the Clone can have a symmetry the original does not.
    switch( clone, "CloneSym", False )
    vsp.SetParmVal( vsp.FindParm( clone, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()
    y_min = vsp.GetParmVal( vsp.FindParm( clone, "Y_Min", "BBox" ) )
    y_len = vsp.GetParmVal( vsp.FindParm( clone, "Y_Len", "BBox" ) )

    # The Clone at +6 and its reflection at -6 span both sides of the plane.
    assert y_len == pytest.approx( 2.0 * ( 6.0 + 0.5 * y_len_one ) )
    assert y_min + 0.5 * y_len == pytest.approx( 0.0, abs = 1e-6 )
    assert_no_errors()


def testCloneMassPropsSwitchDecidesWhosePointMassApplies():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( pod, "PointMass", "Mass_Props" ), 7.0 )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 20.0 )
    vsp.Update()

    assert vsp.GetParmVal( vsp.FindParm( clone, "PointMass", "Mass_Props" ) ) == pytest.approx( 7.0 )

    switch( clone, "CloneMassProps", False )
    vsp.SetParmVal( vsp.FindParm( clone, "PointMass", "Mass_Props" ), 3.0 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( clone, "PointMass", "Mass_Props" ) ) == pytest.approx( 3.0 )
    assert vsp.GetParmVal( vsp.FindParm( pod, "PointMass", "Mass_Props" ) ) == pytest.approx( 7.0 )
    assert_no_errors()


def testCloneGetsItsOwnCopiesOfTheSubSurfaces():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wing = vsp.AddGeom( "WING" )
    ss = vsp.AddSubSurf( wing, vsp.SS_CONTROL )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Location", "XForm" ), 10.0 )
    vsp.Update()

    # The Clone's own subsurfaces, so the mesher can tag them apart, one for each of the original's.
    assert vsp.GetNumSubSurf( clone ) == vsp.GetNumSubSurf( wing )
    mirrored = vsp.GetSubSurfIDVec( clone )[0]
    assert mirrored != ss

    # An edit on the original reaches the Clone.
    vsp.SetParmVal( vsp.FindParm( ss, "UStart", "SS_Control" ), 0.35 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( mirrored, "UStart", "SS_Control" ) ) == pytest.approx( 0.35 )
    assert_no_errors()


def testCloneFollowsTheOriginalsName():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    assert vsp.GetGeomName( clone ) == vsp.GetGeomName( pod ) + "_Clone"

    # Renaming the original renames the Clone.  A name is not a Parm, so the Geom must announce
    # the change itself.
    vsp.SetGeomName( pod, "Nacelle" )
    vsp.Update()
    assert vsp.GetGeomName( clone ) == "Nacelle_Clone"

    # Unless the user has taken the name over.
    switch( clone, "AutoName", False )
    vsp.SetGeomName( clone, "Spare" )
    vsp.SetGeomName( pod, "Fuselage" )
    vsp.Update()
    assert vsp.GetGeomName( clone ) == "Spare"
    assert_no_errors()


def testRenamingACloneThatNamesItselfIsRefused():
    """Renaming a Clone that names itself automatically is refused, as in the Geom Browser,
    since the next update would overwrite the name."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Nacelle" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    assert vsp.GetGeomName( clone ) == "Nacelle_Clone"

    vsp.SetGeomName( clone, "Typed Over" )
    assert_refused( "names itself" )
    assert vsp.GetGeomName( clone ) == "Nacelle_Clone", "the refused name was written anyway"

    # With AutoName off the rename goes through.
    drop_errors()
    switch( clone, "AutoName", False )
    vsp.SetGeomName( clone, "Typed Over" )
    vsp.Update()
    assert vsp.GetGeomName( clone ) == "Typed Over"
    assert_no_errors()


def testACloneWithNothingToCopyOwnsItsName():
    """Losing the original turns AutoName off, so the name becomes the user's to edit and is
    kept if a new original is chosen."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Nacelle" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    assert vsp.GetGeomName( clone ) == "Nacelle_Clone"

    vsp.DeleteGeom( pod )
    vsp.Update()
    drop_errors()

    assert vsp.GetGeomCloneOriginal( clone ) == "", "the Clone still thinks it has an original"

    # The switch itself is off, not just the rename allowed.
    assert vsp.GetParmVal( vsp.FindParm( clone, "AutoName", "Behavior" ) ) == 0.0, \
           "the Clone still says it writes its own name"

    vsp.SetGeomName( clone, "My Own Name" )
    vsp.Update()
    assert vsp.GetGeomName( clone ) == "My Own Name", \
           "a Clone with nothing to name itself after was refused its own name"
    assert_no_errors()



def testANameTypedOnACloneThatHasNotChosenYetIsRefused():
    """A rename is refused on a Clone with AutoName on even before it has an original, since
    ResolveOriginal may give it its parent and overwrite the name on the next update."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Nacelle" )

    # Top level, so no parent and no original yet.
    clone = vsp.AddGeom( "CLONE" )
    vsp.Update()
    assert vsp.GetGeomCloneOriginal( clone ) == "", "the Clone already chose an original"

    vsp.SetGeomName( clone, "Spare" )
    assert_refused( "names itself" )

    # Once it has an original, the name is built from it.
    vsp.SetGeomParent( clone, pod )
    vsp.Update()
    assert vsp.GetGeomName( clone ) == "Nacelle_Clone"


def testAContainerRenameIsRefusedTheSameWayAGeomRenameIs():
    """Renaming through the ParmContainer API is refused the same way."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Nacelle" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    assert vsp.GetGeomName( clone ) == "Nacelle_Clone"

    vsp.SetContainerName( clone, "Sneaky" )
    assert_refused( "names itself" )

    vsp.Update()
    assert vsp.GetGeomName( clone ) == "Nacelle_Clone"


def testRenamingAnOriginalReachesItsClonesWithoutAnotherUpdate():
    """SetGeomName updates the model, so a Clone named after the Geom follows at once."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()
    before = vsp.GetGeomName( clone )

    vsp.SetGeomName( pod, "Nacelle" )

    # No Update() here on purpose.
    assert vsp.GetGeomName( clone ) == "Nacelle_Clone", \
           "the Clone kept %r after its original was renamed" % before
    assert_no_errors()


def testASuffixWithASlashInAFileLoadsSanitised():
    """A name suffix read from a file has forward slashes stripped, as names do."""
    vsp.VSPRenew()
    drop_errors()
    out = scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Nacelle" )
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetGeomCloneNameSuffix( clone, "_A/B" )
    vsp.Update()
    assert vsp.GetGeomCloneNameSuffix( clone ) == "_AB", "the setter did not clean the suffix"

    written = os.path.join( out, "slashsuffix.vsp3" )
    vsp.WriteVSPFile( written )

    # Put a slash into the file by hand.
    text = open( written ).read()
    assert "<NameSuffix" in text, "the suffix was not written"
    open( written, "w" ).write( text.replace( ">_AB<", ">_A/B<" ) )

    vsp.VSPRenew()
    vsp.ReadVSPFile( written )
    vsp.Update()
    drop_errors()

    loaded = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Clone" ][0]
    assert vsp.GetGeomCloneNameSuffix( loaded ) == "_AB", \
           "a suffix out of a file kept a slash the name can never carry"
    assert vsp.GetGeomName( loaded ) == "Nacelle_AB"


def testASuffixCarryingASlashCannotStrandTheName():
    """SetName strips forward slashes, so a suffix is stripped too; otherwise the built name
    never matches the stored one and the name never settles."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Nacelle" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    vsp.SetGeomCloneNameSuffix( clone, "_A/B" )
    vsp.Update()

    assert vsp.GetGeomCloneNameSuffix( clone ) == "_AB", "the suffix kept a slash SetName strips"
    assert vsp.GetGeomName( clone ) == "Nacelle_AB", \
           "the name and the suffix disagree, so the name can never settle"
    assert_no_errors()


def testCloneRefusesToCopyItselfOrARing():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    first = vsp.CloneGeomVec( [ pod ] )[0]
    second = vsp.CloneGeomVec( [ pod ] )[0]

    # A Clone of itself, or a ring of Clones, is refused.
    vsp.SetGeomCloneOriginal( first, first )
    vsp.Update()
    assert vsp.GetGeomCloneOriginal( first ) == pod
    assert_refused( "itself" )

    vsp.SetGeomCloneOriginal( second, first )
    vsp.Update()
    assert vsp.GetGeomCloneOriginal( second ) == first

    vsp.SetGeomCloneOriginal( first, second )     # would close the ring
    vsp.Update()
    assert vsp.GetGeomCloneOriginal( first ) == pod

    # The refusal names the ring.
    assert_refused( "ring" )


def testCloneGeomVecMirrorsAHierarchy():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    parent = vsp.AddGeom( "POD" )
    child = vsp.AddGeom( "POD", parent )
    vsp.Update()

    clones = vsp.CloneGeomVec( [ parent, child ] )
    vsp.Update()
    assert len( clones ) == 2

    # The Clone of the child hangs off the Clone of the parent, not off the original.
    parent_clone = [ c for c in clones if vsp.GetGeomCloneOriginal( c ) == parent ][0]
    child_clone = [ c for c in clones if vsp.GetGeomCloneOriginal( c ) == child ][0]
    assert vsp.GetGeomParent( child_clone ) == parent_clone

    # The top Clone shares its original's parent.
    assert vsp.GetGeomParent( parent_clone ) == vsp.GetGeomParent( parent )
    assert_no_errors()


def testCloneGeomVecLetsTheTopOfEachHierarchyBePlaced():
    """The top of each cloned hierarchy places itself, starting where the original sits;
    everything below copies its placement, relative to the cloned parent."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()

    def xform_copied( gid ):
        return vsp.GetParmVal( vsp.FindParm( gid, "CloneXForm", "Behavior" ) ) > 0.5

    # A lone Geom is the top of its own hierarchy.  Moved and rotated, so matching placement is
    # not just both sitting at the origin.
    pod = vsp.AddGeom( "POD" )
    for parm, val in ( ( "X_Rel_Location", 3.0 ), ( "Y_Rel_Location", 4.0 ),
                       ( "Z_Rel_Location", 5.0 ), ( "Z_Rel_Rotation", 30.0 ) ):
        vsp.SetParmVal( vsp.FindParm( pod, parm, "XForm" ), val )
    vsp.Update()
    alone = vsp.CloneGeomVec( [ pod ] )
    vsp.Update()
    assert len( alone ) == 1
    assert not xform_copied( alone[0] )
    assert box( alone[0] ) == pytest.approx( box( pod ) ), "the Clone did not start on the original"

    # Not copying the placement, it can be moved.
    vsp.SetParmVal( vsp.FindParm( alone[0], "X_Rel_Location", "XForm" ), 25.0 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( alone[0], "X_Location", "XForm" ) ) == pytest.approx( 25.0 )
    assert vsp.GetParmVal( vsp.FindParm( alone[0], "Y_Location", "XForm" ) ) == pytest.approx( 4.0 )

    # Three hierarchies at once.  A skips its middle Geom, B's parent is not asked for, and C
    # is taken whole.
    a1 = vsp.AddGeom( "POD" )
    a2 = vsp.AddGeom( "POD", a1 )
    a3 = vsp.AddGeom( "POD", a2 )
    b1 = vsp.AddGeom( "POD" )
    b2 = vsp.AddGeom( "POD", b1 )
    c1 = vsp.AddGeom( "POD" )
    c2 = vsp.AddGeom( "POD", c1 )
    vsp.SetParmVal( vsp.FindParm( c2, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( vsp.FindParm( c2, "X_Rel_Location", "XForm" ), 5.0 )
    vsp.Update()

    clones = vsp.CloneGeomVec( [ a1, a3, b2, c1, c2 ] )
    vsp.Update()
    assert len( clones ) == 5
    by_original = { vsp.GetGeomCloneOriginal( c ) : c for c in clones }

    assert not xform_copied( by_original[a1] )
    assert xform_copied( by_original[a3] ), "a Geom under one asked for is not a top, gap or not"
    assert not xform_copied( by_original[b2] ), "a Geom whose parent was not asked for is a top"
    assert not xform_copied( by_original[c1] )
    assert xform_copied( by_original[c2] )

    # Moving the top carries an attached child, which keeps the original child's relative placement.
    vsp.SetParmVal( vsp.FindParm( by_original[c1], "Y_Rel_Location", "XForm" ), 20.0 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( by_original[c2], "X_Location", "XForm" ) ) == pytest.approx( 5.0 )
    assert vsp.GetParmVal( vsp.FindParm( by_original[c2], "Y_Location", "XForm" ) ) == pytest.approx( 20.0 )
    assert_no_errors()


def testCloneSurvivesASaveAndReload():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 30.0 )
    vsp.Update()
    before = box( clone )

    f = os.path.join( tempfile.mkdtemp(), "clone_round_trip.vsp3" )
    vsp.WriteVSPFile( f )
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    vsp.ReadVSPFile( f )
    vsp.Update()

    # The link is resolved by ID on every update, so file order does not matter.
    reloaded = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Clone" ][0]
    original = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Pod" ][0]
    assert vsp.GetGeomCloneOriginal( reloaded ) == original
    assert box( reloaded ) == pytest.approx( before )
    assert_no_errors()


def testCloneOutlivesItsOriginal():
    """Deleting the original empties the Clone and reports it as an error."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]

    vsp.DeleteGeom( pod )
    vsp.Update()

    assert clone in vsp.FindGeoms()

    # The reference is cleared, not left dangling.
    assert vsp.GetGeomCloneOriginal( clone ) == ""
    assert pod not in vsp.FindGeoms()
    assert_refused( "lost the Geom it was copying" )

    # Reported once, not on every update.
    drop_errors()
    vsp.Update()
    vsp.Update()
    assert_no_errors()


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
