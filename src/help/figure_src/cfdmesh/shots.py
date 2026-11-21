"""Screenshots of the CFD Mesh screen for CFDMesh.md.  See ../screenshots/snap.py.

    VSP_SNAP_EXE=<build>/src/vsp/vsp <build>/venv/bin/python shots.py

The Output tab shows the export file names, which are made from the folder the model is in; keep the dump in a
folder whose path reads well, with SNAP_CACHE.
"""
import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "screenshots" ) )
sys.path.insert( 0, HERE )
import snap
import openvsp as vsp
import cfdmodel as cm


def model():
    """The small aircraft, Wing first so the Sources tab opens on it, with the Wing's default sources, its wake, and a
    far field box around a half mesh."""
    f, w, h = cm.aircraft( wing_first = True )
    vsp.SetCFDWakeFlag( w, True )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 1.0 )
    vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, 1.0 )
    vsp.Update()
    vsp.DeleteAllCFDSources()
    vsp.AddDefaultSources()


# Crops in screen units: the tab's controls, without the console below them.
W = 550
CROPS = {
    "CFDMesh_gui_global": ( "CFD_Mesh_Global", ( 0, 0, W, 532 ) ),
    "CFDMesh_gui_display": ( "CFD_Mesh_Display", ( 0, 0, W, 382 ) ),
    "CFDMesh_gui_output": ( "CFD_Mesh_Output", ( 0, 0, W, 400 ) ),
    "CFDMesh_gui_sources": ( "CFD_Mesh_Sources", ( 0, 0, W, 372 ) ),
    "CFDMesh_gui_domain": ( "CFD_Mesh_Domain", ( 0, 0, W, 470 ) ),
}

snap.shots( model, "VSP_CFD_MESH_SCREEN", CROPS, name = "MyPlane" )
