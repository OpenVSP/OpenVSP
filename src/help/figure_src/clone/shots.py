"""Screenshots of the Geom Browser for Clone.md (and, from the same model, Hinge.md).  See ../screenshots/snap.py."""
import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "screenshots" ) )
import snap
import openvsp as vsp

# Geom Browser crops: the tree from Vehicle down; the Clones, and the Hinge with its child.
CLONE_BOX = ( 18, 118, 193, 236 )
HINGE_BOX = ( 18, 118, 193, 202 )


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
    mine = vsp.FindGeoms()
    snap.add_ngon()
    # The mesh hides every Geom it was made from; show this model's again.
    for g in mine:
        vsp.SetSetFlag( g, vsp.SET_SHOWN, True )
        vsp.SetSetFlag( g, vsp.SET_NOT_SHOWN, False )
    # A Conformal on the hidden pod; without one the dump adds one to the wing and puts the next Geom it adds there.
    ngon_pod = [ g for g in vsp.FindGeoms() if g not in mine and vsp.GetGeomTypeName( g ) == "Pod" ][ 0 ]
    conf = vsp.AddGeom( "CONFORMAL", ngon_pod )
    vsp.SetSetFlag( conf, vsp.SET_SHOWN, False )
    vsp.SetSetFlag( conf, vsp.SET_NOT_SHOWN, True )
    vsp.Update()


if __name__ == "__main__":
    snap.shots( model, "VSP_MANAGE_GEOM_SCREEN", { "Clone_browser": ( "Geom_Browser", CLONE_BOX ) }, name = "browser" )
