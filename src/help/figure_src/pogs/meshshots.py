"""Pictures of the surfaces CFD Mesh gives POGS, for POGS.md, taken in OpenVSP as ../cfdmesh/meshshots.py
takes them (see there): each picture in a process of its own, the model meshed before the GUI starts.

    <build>/venv/bin/python meshshots.py
"""

import os
import sys
import subprocess

HERE = os.path.dirname( os.path.abspath( __file__ ) )
CFD = os.path.join( HERE, "..", "cfdmesh" )
sys.path.insert( 0, CFD )


def patches_one( split, name, view, center, zoom, rot, crop, size = ( 1600, 1000 ) ):
    """The aircraft of pogsmodel meshed with Split and Join Surfaces off or on, drawn as its curves alone: the
    intersection curves and the border curves, the lines that bound each patch, a face of the POGS files.  The mesh is
    not drawn, the faces are not colored, and the patch join curves (Show Patch Join Curves) are off, so a joined
    boundary does not show.  The view is zoomed on center through the view Parms, turned to rot."""
    import openvsp as vsp
    import cfdmodel as cm
    import meshshots as ms
    import pogsmodel as pm
    pm.blunt_aircraft()
    s = vsp.FindContainer( "CFDMeshSettings", 0 )
    vsp.SetParmVal( vsp.FindParm( s, "SplitJoinSurfsFlag", "Global" ), float( split ) )
    vsp.Update()
    cm.mesh( name )
    for n, v in ( ( "DrawIsectFlag", 1.0 ), ( "DrawBorderFlag", 1.0 ), ( "DrawJoinFlag", 0.0 ),
                  ( "DrawCurveFlag", 1.0 ), ( "DrawPntsFlag", 0.0 ), ( "DrawMeshFlag", 0.0 ) ):
        vsp.SetParmVal( vsp.FindParm( s, n, "DrawMesh" ), v )
    ms.grab( name, view, crop = crop, center = center, zoom = zoom, rot = rot, size = size )


# The two views, each framed on what Split and Join Surfaces changes: ( view, center, zoom, rotation, crop ), the crop
# given as fractions of the window from its top left, or None.  The outer wing is seen from behind, outboard and above
# (the viewer toward ( 0.58, 0.58, 0.57 ) from it), so the tip cap and the blunt trailing edge both show; the body from
# in front, a little to the left and above (toward ( -0.93, -0.26, 0.25 )).
FRAMES = {
    "wing": ( "CAM_RIGHT_ISO", ( 5.0, 4.8, -0.25 ), 14.0, ( -35.26, 45.0, -60.0 ), ( 0.0, 0.08, 1.0, 0.8 ) ),
    "body": ( "CAM_FRONT", ( 4.0, 0.0, 0.0 ), 2.4, ( 15.0, -75.0, -75.0 ), ( 0.02, 0.12, 1.0, 0.9 ) ),
}


def shot( key, split ):
    view, center, zoom, rot, crop = FRAMES[ key ]
    patches_one( split, "_pogs_%s_%d" % ( key, split ), view, center, zoom, rot, crop )


def shot_pairs():
    import meshshots as ms
    for key in FRAMES:
        for split in ( 0, 1 ):
            run( "%s_%d" % ( key, split ) )
        # The outer wing's pair is stacked, so its thin trailing edge is drawn large; the body's is side by side.
        ncol = 2
        if key == "wing":
            ncol = 1
        ms.compose( "POGS_split_join_" + key, [ "_pogs_%s_0" % key, "_pogs_%s_1" % key ],
                    [ "Split and Join Surfaces off", "Split and Join Surfaces on" ], ncol )




def run( name ):
    r = subprocess.run( [ sys.executable, os.path.abspath( __file__ ), "--one", name ], capture_output = True,
                        text = True )
    lines = [ l for l in r.stdout.splitlines() if l.startswith( "made" ) ]
    print( "\n".join( lines ) if lines else r.stdout[ -2000: ] + r.stderr[ -2000: ] )


def one( name ):
    import openvsp_config
    openvsp_config.LOAD_GRAPHICS = True
    openvsp_config.LOAD_FACADE = True
    key, split = name.rsplit( "_", 1 )
    shot( key, int( split ) )


if __name__ == "__main__":
    if len( sys.argv ) > 2 and sys.argv[ 1 ] == "--one":
        one( sys.argv[ 2 ] )
    elif len( sys.argv ) > 1:
        for k in sys.argv[ 1: ]:
            run( k )
    else:
        shot_pairs()
