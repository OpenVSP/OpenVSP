"""Screenshots of the Wing's tabs for Wing.md.  See ../screenshots/snap.py."""
import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "screenshots" ) )
import snap
import openvsp as vsp

import wingmodel as wm


def model():
    vsp.AddGeom( "WING" )
    snap.add_ngon()


def blended():
    """The wing of Figure 10 (wing_figs.cranked and blending): blended at airfoil 1, which the Blending tab shows."""
    w = wm.new_wing( sym = True )
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    wm.setx( w, 1, "Span", 3.0 )
    wm.setx( w, 1, "Sweep", 15.0 )
    wm.setx( w, 1, "Tip_Chord", 3.2 )
    wm.setx( w, 2, "Span", 7.0 )
    wm.setx( w, 2, "Tip_Chord", 1.2 )
    wm.setx( w, 2, "Sweep", 30.0 )
    wm.setx( w, 2, "Dihedral", 6.0 )
    wm.setx( w, 1, "InLEMode", vsp.BLEND_ANGLES )
    wm.setx( w, 1, "InLESweep", 22.5 )
    wm.setx( w, 1, "InLEDihedral", 0.0 )
    wm.setx( w, 1, "OutLEMode", vsp.BLEND_MATCH_IN_ANGLES )
    # The Blend Airfoil index is the Wing's ActiveXSec.
    vsp.SetParmVal( w, "ActiveXSec", "Index", 1 )
    vsp.Update()
    snap.add_ngon()


snap.shots( model, "MS_WING_GEOM_SCREEN", { "Wing_sect_planform": ( "Wing_Sect", ( 4, 265, 456, 441 ) ),
                                            "Wing_plan_loft": ( "Wing_Plan", ( 4, 651, 456, 776 ) ) } )
snap.shots( blended, "MS_WING_GEOM_SCREEN", { "Wing_blending_le": ( "Wing_Blending", ( 4, 71, 456, 276 ) ) },
            name = "Wing_blending" )
