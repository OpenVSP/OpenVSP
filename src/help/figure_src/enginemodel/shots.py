"""Screenshots of the Stack's Design tab for EngineModel.md.  See ../screenshots/snap.py."""
import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "screenshots" ) )
import snap
import openvsp as vsp


def model():
    """The Nacelle Flowthrough Mid Origin preset with its Inlet Face given by U."""
    g = vsp.AddGeom( "STACK" )
    vsp.InitStackPreset( g, vsp.STACK_PRESET_FLOWTHRU_MID_ORIG )
    vsp.Update()
    vsp.SetParmVal( vsp.GetParm( g, "InletFaceMode", "EngineModel" ), vsp.ENGINE_LOC_U )
    vsp.Update()


snap.shots( model, "STACK_GEOM_SCREEN", { "EngineModel_gui_stations": ( "Stack_Design", ( 8, 390, 456, 633 ) ) },
            name = "enginemodel_stations" )
