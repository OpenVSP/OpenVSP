"""Screenshots of the Mass tab for MassTab.md.  See ../screenshots/snap.py."""
import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "screenshots" ) )
import snap
import openvsp as vsp


def model():
    """The default Pod with its thin shell included."""
    pid = vsp.AddGeom( "POD" )
    vsp.SetParmVal( pid, "Shell_Flag", "Mass_Props", 1 )


snap.shots( model, "POD_GEOM_SCREEN", { "MassTab_gui_groups": ( "Pod_Mass", ( 4, 54, 396, 412 ) ) },
            name = "masstab_pod" )
