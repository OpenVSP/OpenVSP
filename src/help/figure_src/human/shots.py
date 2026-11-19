"""Screenshot of the Human's Pose tab for Human.md.  See ../screenshots/snap.py."""
import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "screenshots" ) )
import snap
import openvsp as vsp


def model():
    """A new human, standing, in meters."""
    h = vsp.AddGeom( "HUMAN" )
    vsp.SetParmVal( h, "LenUnit", "Anthropometric", vsp.LEN_M )
    vsp.Update()


snap.shots( model, "HUMAN_GEOM_SCREEN", { "Human_pose_tab": ( "Human_Pose", ( 4, 53, 506, 664 ) ) }, name = "Human" )
