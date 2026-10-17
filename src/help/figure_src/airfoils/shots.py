"""Screenshots of the Wing's Airfoil tab for Airfoils.md.  See ../screenshots/snap.py."""
import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "screenshots" ) )
import snap
import openvsp as vsp


def model():
    """A Wing whose section 1 is a CST airfoil fitted to NACA 2412, degree 5 on each surface; section 1 active."""
    g = vsp.AddGeom( "WING" )
    s = vsp.GetXSecSurf( g, 0 )
    xs = vsp.GetXSec( s, 1 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "Camber" ), 0.02 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "CamberLoc" ), 0.4 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickChord" ), 0.12 )
    vsp.Update()
    vsp.FitAfCST( s, 1, 5 )
    vsp.SetParmVal( vsp.GetParm( g, "ActiveXSec", "Index" ), 1 )
    vsp.Update()


snap.shots( model, "MS_WING_GEOM_SCREEN", { "Airfoils_gui_cst": ( "Wing_Airfoil", ( 4, 125, 456, 446 ) ) },
            name = "airfoils_cst" )
