"""Screenshots of the Conformal's Design tab for Conformal.md.  See ../screenshots/snap.py."""
import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "screenshots" ) )
import snap
import openvsp as vsp


def fuselage_utrim():
    """A Conformal on the default Fuselage, its ends trimmed by U at the start and by L at the end."""
    fid = vsp.AddGeom( "FUSELAGE" )
    c = vsp.AddGeom( "CONFORMAL", fid )
    vsp.SetParmVal( c, "UTrimFlag", "Design", 1 )
    vsp.SetParmVal( c, "UMinTrimTypeFalg", "Design", vsp.U_TRIM )
    vsp.SetParmVal( c, "UMaxTrimTypeFalg", "Design", vsp.L_TRIM )


def wing_tank():
    """The tank of Conformal_wing: Offset 0.1, Eta 0.2 to 0.7, Chord Min 0.2 and Chord Max 0.6 in the default Wing."""
    wg = vsp.AddGeom( "WING" )
    vsp.Update()
    c = vsp.AddGeom( "CONFORMAL", wg )
    vsp.Update()
    for k, v in ( ( "Offset", 0.1 ), ( "UTrimFlag", 1 ), ( "UMinTrimTypeFalg", vsp.ETA_TRIM ),
                  ( "UMaxTrimTypeFalg", vsp.ETA_TRIM ), ( "EtaTrimMin", 0.2 ), ( "EtaTrimMax", 0.7 ),
                  ( "ChordTrimFlag", 1 ), ( "ChordTrimMin", 0.2 ), ( "ChordTrimMax", 0.6 ) ):
        vsp.SetParmVal( c, k, "Design", v )


snap.shots( fuselage_utrim, "CONFORMAL_SCREEN", { "Conformal_gui_utrim": ( "Conformal_Design", ( 4, 161, 396, 280 ) ) },
            name = "conformal_fuselage_utrim" )
snap.shots( wing_tank, "CONFORMAL_SCREEN", { "Conformal_gui_wingtrim": ( "Conformal_Design", ( 4, 509, 396, 726 ) ) },
            name = "conformal_wing_tank" )
