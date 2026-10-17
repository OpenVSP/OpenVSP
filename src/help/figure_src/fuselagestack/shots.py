"""Screenshots of the Stack's XSec tab for FuselageStack.md.  See ../screenshots/snap.py."""
import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "screenshots" ) )
import snap
import openvsp as vsp


def model():
    """A new Stack with Rot Y 10 on section 1 and section 2 active."""
    g = vsp.AddGeom( "STACK" )
    s = vsp.GetXSecSurf( g, 0 )
    vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( s, 1 ), "YRotate" ), 10.0 )
    vsp.SetParmVal( vsp.GetParm( g, "ActiveXSec", "Index" ), 2 )
    vsp.Update()


snap.shots( model, "STACK_GEOM_SCREEN", { "FuselageStack_gui_coord": ( "Stack_XSec", ( 8, 189, 456, 355 ) ) },
            name = "fuselagestack_coord" )
