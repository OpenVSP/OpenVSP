"""Screenshots of the Skinning tab for XSecSkinning.md.  See ../screenshots/snap.py."""
import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "screenshots" ) )
sys.path.insert( 0, HERE )
import snap
import openvsp as vsp
import xsecskinning_figs as xk


def fuselage():
    """A new Fuselage with section 1 active."""
    g = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( vsp.GetParm( g, "ActiveXSec", "Index" ), 1 )


def spine():
    """The Stack of Figure 8: the middle section's sides at Angle 0, and a spine at W 0.1, mirrored left to right,
    enforcing Angle -25 there.  Section 1 is active."""
    g, s = xk.stack()
    xs = vsp.GetXSec( s, 1 )
    for sd in xk.SIDES:
        xk.side( xs, sd, { "Angle": 0.0, "Strength": 1.0 }, { "Angle": 0.0, "Strength": 1.0 } )
    vsp.AddSkinSpine( g, 0.1 )
    vsp.Update()
    for i in range( vsp.GetNumXSec( s ) ):
        vsp.SetParmVal( vsp.GetSkinSpineParm( vsp.GetSkinSpineID( vsp.GetXSec( s, i ), 0 ), "LRSym" ), 1.0 )
    sid = vsp.GetSkinSpineID( xs, 0 )
    for lr in "LR":
        vsp.SetParmVal( vsp.GetSkinSpineParm( sid, lr + "AngleSet" ), 1.0 )
        vsp.SetParmVal( vsp.GetSkinSpineParm( sid, lr + "Angle" ), -25.0 )
        vsp.SetParmVal( vsp.GetSkinSpineParm( sid, lr + "Strength" ), 1.0 )
    vsp.SetParmVal( vsp.GetParm( g, "ActiveXSec", "Index" ), 1 )
    vsp.Update()


snap.shots( fuselage, "FUSELAGE_GEOM_SCREEN",
            { "XSecSkinning_gui_side": ( "Fuselage_Skinning_Sides", ( 8, 230, 452, 353 ) ) }, name = "xsecskinning_sides" )
snap.shots( spine, "STACK_GEOM_SCREEN",
            { "XSecSkinning_gui_spine": ( "Stack_Skinning_Spines", ( 8, 202, 452, 511 ) ) }, name = "xsecskinning_spine" )
