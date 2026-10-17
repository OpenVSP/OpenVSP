"""Screenshots of the Gear's Tire and Retract tabs for Gear.md.  See ../screenshots/snap.py."""
import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "screenshots" ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )
sys.path.insert( 0, HERE )
import snap
import openvsp as vsp
import gear_figs as gf


def model():
    """The tricycle gear of the page's figures; its nose bogie, the first, retracts forward by the mechanism of
    Figure 6, with the trunnion above its contact point."""
    g = gf.tricycle_gear()
    nose = vsp.GetAllBogies( g )[ 0 ]
    vsp.SetParmVal( gf.bparm( nose, "RetMode", "Retract" ), vsp.GEAR_MECHANISM )
    vsp.Update()
    vsp.SetParmVal( gf.bparm( nose, "MechAbsRelFlag", "Retract" ), vsp.ABS )
    for k, v in gf.MECH.items():
        vsp.SetParmVal( gf.bparm( nose, k, "Retract" ), v )
    vsp.SetParmVal( gf.bparm( nose, "Mech_XLoc", "Retract" ), gf.NOSE[ "XContactPt" ] )
    vsp.Update()


snap.shots( model, "GEAR_GEOM_SCREEN",
            { "Gear_tire_tab": ( "Gear_Tire", ( 4, 81, 446, 380 ) ),
              "Gear_retract_tab": ( "Gear_Retract", ( 4, 53, 446, 128 ) ),
              "Gear_retract_mech1": ( "Gear_Retract_Mechanism__1_2_", ( 4, 101, 446, 676 ) ),
              "Gear_retract_mech2": ( "Gear_Retract_Mechanism__2_2_", ( 4, 101, 446, 389 ) ) },
            name = "Gear" )
