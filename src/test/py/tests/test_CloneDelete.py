# What becomes of a Clone when the Geom it copies is deleted or cut.
#
# The choice is an argument to the delete, since replacing a Clone needs the original to copy.
# The default leaves the Clone empty and reports it from the delete call itself.

import openvsp as vsp
import pytest

from clonehelp import ( box, drop_errors, assert_refused, assert_no_errors )
from fliphelp import ( a_chiral_wing, flip )


def pod_and_clones():
    """A pod moved off the origin, a Clone of it, and a Clone of that Clone."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( pod, "X_Rel_Location", "XForm" ), 3.0 )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    inner = vsp.CloneGeomVec( [ clone ] )[0]
    vsp.Update()
    drop_errors()
    return pod, clone, inner


def last_call_clean():
    return not vsp.ErrorMgrSingleton.getInstance().GetErrorLastCallFlag()


# Every call that removes a Geom and takes a choice for its Clones.
REMOVERS = [ pytest.param( lambda gid, how: vsp.DeleteGeom( gid, how ), id = "DeleteGeom" ),
             pytest.param( lambda gid, how: vsp.DeleteGeomVec( [ gid ], how ), id = "DeleteGeomVec" ),
             pytest.param( lambda gid, how: vsp.CutGeomToClipboard( gid, how ), id = "CutGeomToClipboard" ) ]


def testFindGeomClonesFindsOnlyTheClonesLeftBehind():
    pod, clone, inner = pod_and_clones()

    # The Clone of the Clone still has its original if only the pod goes.
    assert list( vsp.FindGeomClones( [ pod ] ) ) == [ clone ]

    # A Clone deleted with its original is not left behind; the Clone of it is.
    assert list( vsp.FindGeomClones( [ pod, clone ] ) ) == [ inner ]

    assert list( vsp.FindGeomClones( [ pod, clone, inner ] ) ) == []
    assert_no_errors()


def testLeftEmptyIsSaidAtTheDeleteNotAtSomeLaterUpdate():
    pod, clone, inner = pod_and_clones()

    vsp.DeleteGeom( pod )
    assert not last_call_clean(), "a Clone was emptied and the call reported clean"

    # Without an update, the Clone is already empty when the delete returns.
    assert vsp.GetGeomCloneOriginal( clone ) == ""
    assert box( clone ) == pytest.approx( ( 0.0, ) * 6 )

    mgr = vsp.ErrorMgrSingleton.getInstance()
    errs = [ mgr.PopLastError() for _ in range( mgr.GetNumTotalErrors() ) ]
    assert [ e.m_ErrorCode for e in errs ] == [ vsp.VSP_CLONE_ORIGINAL_LOST ], \
           [ e.m_ErrorString for e in errs ]

    # Reported once; later updates report nothing.
    vsp.Update()
    vsp.Update()
    assert_no_errors()

    # The Clone of the Clone keeps its original, which is now empty.
    assert vsp.GetGeomCloneOriginal( inner ) == clone


def testDeletedWithTheOriginalTakesTheWholeChain():
    pod, clone, inner = pod_and_clones()
    other = vsp.AddGeom( "WING" )
    vsp.Update()

    vsp.DeleteGeom( pod, vsp.CLONE_DELETE_WITH_ORIGINAL )

    # The Clone of the Clone goes too.
    assert list( vsp.FindGeoms() ) == [ other ]
    assert last_call_clean()
    assert_no_errors()


@pytest.mark.parametrize( "remove", REMOVERS )
def testReplacedClonesStandWhereTheyStoodAsRealGeoms( remove ):
    pod, clone, inner = pod_and_clones()

    # Move the Clone away from the pod so the two positions differ.
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 10.0 )
    vsp.Update()
    before = box( clone )

    remove( pod, vsp.CLONE_DELETE_REPLACE )
    assert last_call_clean()
    assert_no_errors()

    # The same ID is now a pod, where the Clone stood, with the pod's shape.
    assert pod not in vsp.FindGeoms()
    assert vsp.GetGeomTypeName( clone ) == "Pod"
    assert box( clone ) == pytest.approx( before )

    # The Clone of the Clone follows what took its original's place.
    vsp.Update()
    assert vsp.GetGeomCloneOriginal( inner ) == clone
    assert [ box( inner )[i] for i in ( 1, 3, 5 ) ] == pytest.approx( [ before[i] for i in ( 1, 3, 5 ) ] )
    assert_no_errors()


@pytest.mark.parametrize( "remove", REMOVERS )
def testAReplacedFlippedCloneKeepsItsFlip( remove ):
    """A flipped Clone is replaced by a wing with the Clone's Flip_Flag and box."""
    vsp.VSPRenew()
    drop_errors()
    wing = a_chiral_wing( ( 3.0, 4.0, 5.0 ) )
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), -10.0 )
    vsp.Update()
    unflipped = box( clone )

    flip( clone, vsp.SYM_XZ )
    before = box( clone )
    assert before != pytest.approx( unflipped ), "the flip did nothing, so nothing is measured"
    drop_errors()

    remove( wing, vsp.CLONE_DELETE_REPLACE )
    assert last_call_clean()
    assert_no_errors()

    assert wing not in vsp.FindGeoms()
    assert vsp.GetGeomTypeName( clone ) == "Wing"
    assert vsp.GetParmVal( vsp.FindParm( clone, "Flip_Flag", "Sym" ) ) == vsp.SYM_XZ
    vsp.Update()
    assert box( clone ) == pytest.approx( before )
    assert_no_errors()


def testNothingLeftBehindMeansNothingSaid():
    pod, clone, inner = pod_and_clones()

    vsp.DeleteGeomVec( [ pod, clone, inner ] )

    assert list( vsp.FindGeoms() ) == []
    assert last_call_clean()
    assert_no_errors()


def testCutWithTheOriginalPastesBackAsAGroup():
    pod, clone, inner = pod_and_clones()

    vsp.CutGeomToClipboard( pod, vsp.CLONE_DELETE_WITH_ORIGINAL )
    assert list( vsp.FindGeoms() ) == []
    assert_no_errors()

    pasted = list( vsp.PasteGeomClipboard() )
    vsp.Update()
    assert len( pasted ) == 3

    # New IDs, but still a pod and two Clones of the pasted Geoms.
    new_pod = [ g for g in pasted if vsp.GetGeomTypeName( g ) == "Pod" ][0]
    clones = [ g for g in pasted if g != new_pod ]
    new_clone = [ g for g in clones if vsp.GetGeomCloneOriginal( g ) == new_pod ][0]
    new_inner = [ g for g in clones if vsp.GetGeomCloneOriginal( g ) == new_clone ][0]
    assert box( new_inner ) == pytest.approx( box( new_pod ) )
    assert_no_errors()


def testCutLeavingThemEmptySaysSo():
    pod, clone, inner = pod_and_clones()

    vsp.CutGeomToClipboard( pod )

    assert vsp.GetGeomCloneOriginal( clone ) == ""
    assert_refused( "lost the Geom it was copying" )


@pytest.mark.parametrize( "remove", REMOVERS )
def testAChoiceThatIsNotOneDeletesNothing( remove ):
    pod, clone, inner = pod_and_clones()

    remove( pod, vsp.CLONE_DELETE_NUM_TYPES )

    assert pod in vsp.FindGeoms()
    assert vsp.GetGeomCloneOriginal( clone ) == pod
    assert_refused( "CLONE_DELETE_TYPE" )


def circle_diameters():
    return [ p for c in vsp.FindContainers() for p in vsp.FindContainerParmIDs( c )
             if vsp.GetParmName( p ) == "Circle_Diameter" ]


def testAReplacedBodyOfRevolutionFollowsItsCrossSection():
    """The Geom that takes a Clone's place takes over its cross section along with its ID."""
    vsp.VSPRenew()
    drop_errors()
    bor = vsp.AddGeom( "BODYOFREVOLUTION" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ bor ] )[0]
    vsp.Update()

    vsp.DeleteGeom( bor, vsp.CLONE_DELETE_REPLACE )
    vsp.Update()
    assert_no_errors()
    assert vsp.GetGeomTypeName( clone ) == "BodyOfRevolution"

    diameters = circle_diameters()
    assert len( diameters ) == 1, "expected the one cross section of the replacement"
    before = box( clone )

    vsp.SetParmValUpdate( diameters[0], 2.0 * vsp.GetParmVal( diameters[0] ) )
    vsp.Update()
    assert box( clone ) != pytest.approx( before ), "the replacement ignored its cross section"
    assert_no_errors()


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
