# Measures updated while a model is still being read.
#
# A Conformal Geom deletes a scratch copy of its parent while it updates, and deleting a Geom
# updates the whole Vehicle, measures included.  While a model is read that can happen before
# the Geoms later in the hierarchy have been updated at all.

import openvsp as vsp

import os
import shutil
import tempfile


def testARulerToAGeomNotYetUpdatedSurvivesTheLoad():
    """A Ruler ending on the symmetric copy of a Geom updated after a Conformal sibling.

    The Wing is read before its first update, so it has its main surface but not yet its
    symmetric copy when the Conformal's update updates the measures.
    """
    vsp.VSPRenew()
    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.AddGeom( "CONFORMAL", fuse )
    wing = vsp.AddGeom( "WING", fuse )
    vsp.Update()
    assert vsp.GetNumMainSurfs( wing ) == 1

    # Surface 1 is the Wing's symmetric copy.
    vsp.AddRuler( fuse, 0, 0.5, 0.5, wing, 1, 0.5, 0.5, "FuseToWing" )
    vsp.Update()

    out = tempfile.mkdtemp()
    try:
        fname = os.path.join( out, "ruler.vsp3" )
        vsp.WriteVSPFile( fname )
        vsp.VSPRenew()

        vsp.ReadVSPFile( fname )
        vsp.Update()
    finally:
        shutil.rmtree( out )

    assert len( vsp.GetAllRulers() ) == 1
    wing = vsp.FindGeomsWithName( "WingGeom" )[0]
    assert vsp.GetNumMainSurfs( wing ) == 1


if __name__ == "__main__":
    testARulerToAGeomNotYetUpdatedSurvivesTheLoad()
