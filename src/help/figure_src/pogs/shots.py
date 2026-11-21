"""Screenshots of the CFD Mesh screen set up for POGS, for POGS.md.  See ../screenshots/snap.py.

    VSP_SNAP_EXE=<build>/src/vsp/vsp <build>/venv/bin/python shots.py

The Output tab shows the export file names, which are made from the folder the model is in; keep the dump in a
folder whose path reads well, with SNAP_CACHE.
"""
import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "screenshots" ) )
sys.path.insert( 0, os.path.join( HERE, "..", "cfdmesh" ) )
import snap
import openvsp as vsp
import cfdmodel as cm

# Every file but POGS turned off on the Output tab.
OFF = [ "STL", "POLY", "TRI", "FACET", "OBJ", "GMSH", "VSPGEOM", "DAT", "KEY", "TKEY" ]


def model():
    """The small aircraft of the CFD Mesh page, set up for POGS: Max Edge Len, Min Edge Len and Max Gap relative at
    0.01, Num Circle Segments 90, the Set All meshed, Split and Join Surfaces on, and the POGS files the only output."""
    cm.aircraft()
    for absrel, frac in ( ( "BaseAbsRel", "BaseLenFrac" ), ( "MinAbsRel", "MinFrac" ),
                          ( "MaxGapAbsRel", "MaxGapFrac" ) ):
        vsp.SetParmVal( cm.density( absrel ), vsp.REL )
        vsp.SetParmVal( cm.density( frac ), 0.01 )
    vsp.SetCFDMeshVal( vsp.CFD_NUM_CIRCLE_SEGS, 90.0 )
    s = vsp.FindContainer( "CFDMeshSettings", 0 )
    vsp.SetParmVal( vsp.FindParm( s, "Set", "Global" ), float( vsp.SET_ALL ) )
    vsp.SetParmVal( vsp.FindParm( s, "SplitJoinSurfsFlag", "Global" ), 1.0 )
    for f in OFF:
        vsp.SetParmVal( vsp.FindParm( s, f + "_Export", "ExportCFD" ), 0.0 )
    vsp.SetParmVal( vsp.FindParm( s, "POGS_Export", "ExportCFD" ), 1.0 )
    vsp.Update()


# Crops in screen units: the tab's controls, without the console below them.
W = 550
CROPS = {
    "POGS_gui_global": ( "CFD_Mesh_Global", ( 0, 0, W, 532 ) ),
    "POGS_gui_output": ( "CFD_Mesh_Output", ( 0, 0, W, 400 ) ),
}

snap.shots( model, "VSP_CFD_MESH_SCREEN", CROPS, name = "MyPlane" )
