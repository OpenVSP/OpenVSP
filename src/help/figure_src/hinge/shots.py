"""Screenshots of the Hinge's tabs and of the Geom Browser for Hinge.md.  See ../screenshots/snap.py."""
import importlib.util
import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "screenshots" ) )
import snap
import openvsp as vsp

# The Geom Browser model of ../clone/shots.py, which holds a Hinge with a child.
spec = importlib.util.spec_from_file_location( "clone_shots", os.path.join( HERE, "..", "clone", "shots.py" ) )
clone_shots = importlib.util.module_from_spec( spec )
spec.loader.exec_module( clone_shots )


def model():
    """A Hinge on a wing, oriented by Vectors toward a point of the wing's surface, with Translate allowed and Rotate
    limited to -30..60 degrees."""
    wing = vsp.AddGeom( "WING" )
    hinge = vsp.AddGeom( "HINGE", wing )
    parms = { "OrientRotFlag": vsp.HINGE_ORIENT_VEC, "PrimType": vsp.HINGE_PRIM_SURFPT, "PrimULoc": 0.6,
              "PrimWLoc": 0.5, "JointTranslateFlag": 1, "JointRotMinFlag": 1, "JointRotMaxFlag": 1,
              "JointRotMin": -30.0, "JointRotMax": 60.0, "JointRotate": 20.0 }
    for k, v in parms.items():
        vsp.SetParmVal( hinge, k, "Hinge", v )
    vsp.Update()


snap.shots( model, "HINGE_GEOM_SCREEN",
            { "Hinge_gui_motion": ( "Hinge_Motion", ( 4, 55, 396, 233 ) ),
              "Hinge_gui_orient": ( "Hinge_Orient", ( 4, 115, 396, 504 ) ) },
            name = "hinge" )
snap.shots( clone_shots.model, "VSP_MANAGE_GEOM_SCREEN", { "Hinge_browser": ( "Geom_Browser", clone_shots.HINGE_BOX ) },
            name = "browser" )
