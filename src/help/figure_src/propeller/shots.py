"""Screenshots of the Propeller's tabs for Propeller.md.  See ../screenshots/snap.py."""
import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "screenshots" ) )
import snap
import openvsp as vsp

# The harmonic flap of Figure 25 (figparts.HARM), with a second harmonic left at zero.
HARM = { "NumBlade": 4, "BladeMotionMode": vsp.PROP_BLADE_HARMONIC, "NumHarmonics": 2, "Rotate": 20.0,
         "FlapMean": 6.0, "FlapCos_1": 5.0, "FlapSin_1": -3.0 }


def parm_ids( pid ):
    return { vsp.GetParmName( q ): q for q in vsp.GetGeomParmIDs( pid )
             if vsp.GetParmGroupName( q ) in ( "Design", "Articulation" ) }


def model():
    """The default propeller, its blade curves untouched, in Harmonic mode."""
    pid = vsp.AddGeom( "PROP" )
    vsp.Update()
    # Counts first: NumBlade and NumHarmonics make the per-blade and harmonic parms in an Update.
    for first in ( True, False ):
        ids = parm_ids( pid )
        for k, v in HARM.items():
            if k.startswith( "Num" ) == first:
                vsp.SetParmVal( ids[ k ], v )
        vsp.Update()


snap.shots( model, "PROP_GEOM_SCREEN",
            { "Propeller_blade_curve": ( "Propeller_Blade", ( 4, 54, 507, 560 ) ),
              "Propeller_motion_harmonic": ( "Propeller_Motion", ( 4, 115, 507, 341 ) ) },
            name = "propeller" )
