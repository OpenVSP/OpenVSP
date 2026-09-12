# What a Clone keeps when it is copied.
#
# A Clone's original and its subsurface pairings are references, so a pasted Clone follows the
# same original.  The ID rules themselves are in test_RemapIDs.py.

import openvsp as vsp
import pytest

from clonehelp import ( drop_errors, assert_refused, assert_no_errors )
import os
import tempfile


def testAPastedCloneKeepsItsOriginal():
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    clone = vsp.AddGeom( "CLONE" )
    vsp.SetGeomCloneOriginal( clone, pod )
    vsp.Update()

    vsp.CopyGeomToClipboard( clone )
    pasted = vsp.PasteGeomClipboard()[0]
    vsp.Update()

    assert vsp.GetGeomCloneOriginal( pasted ) == pod
    assert vsp.GetParmVal( vsp.FindParm( pasted, "X_Len", "BBox" ) ) == \
           pytest.approx( vsp.GetParmVal( vsp.FindParm( clone, "X_Len", "BBox" ) ) )
    assert_no_errors()


def testACloneRefusesToCopyAGeomHangingOffIt():
    """A Geom parented to the Clone would update after it, so it is refused as the original."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    clone = vsp.AddGeom( "CLONE" )
    child = vsp.AddGeom( "POD", clone )
    vsp.Update()

    # Set an original first so the refusal has something to leave alone.
    vsp.SetGeomCloneOriginal( clone, pod )
    vsp.Update()
    assert vsp.GetGeomCloneOriginal( clone ) == pod
    drop_errors()

    vsp.SetGeomCloneOriginal( clone, child )
    vsp.Update()

    # The earlier original is kept, not cleared.
    assert vsp.GetGeomCloneOriginal( clone ) == pod

    assert_refused( "hangs off" )


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
