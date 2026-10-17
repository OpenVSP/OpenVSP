"""Screenshots of the Geom Browser for Clone.md (and, from the same model, Hinge.md).  See ../screenshots/snap.py."""
import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "screenshots" ) )
import snap
import openvsp as vsp

# Geom Browser crops: the tree from Vehicle down; the Clones, and the Hinge with its child.
CLONE_BOX = ( 18, 102, 193, 220 )
HINGE_BOX = ( 18, 102, 193, 186 )


def model():
    """A pod carrying a wing that carries a Hinge with a flap; the pod and wing are cloned together."""
    pod = vsp.AddGeom( "POD" )
    vsp.SetGeomName( pod, "Pod" )
    wing = vsp.AddGeom( "WING", pod )
    vsp.SetGeomName( wing, "Wing" )
    hinge = vsp.AddGeom( "HINGE", wing )
    vsp.SetGeomName( hinge, "Hinge" )
    flap = vsp.AddGeom( "WING", hinge )
    vsp.SetGeomName( flap, "Flap" )
    vsp.Update()
    vsp.CloneGeomVec( [ pod, wing ] )
    vsp.Update()


if __name__ == "__main__":
    snap.shots( model, "VSP_MANAGE_GEOM_SCREEN", { "Clone_browser": ( "Geom_Browser", CLONE_BOX ) }, name = "browser" )
