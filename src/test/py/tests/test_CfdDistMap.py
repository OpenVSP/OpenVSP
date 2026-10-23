"""The CFD mesher's distance map must survive the thin strips a wing's patches can make.

Each surface is sampled on an 11 x 11 grid, coincident points are merged, and the grid is
flattened by OpenABF.  A wing whose root and tip airfoils put a corner at nearly, but not
exactly, the same parameter gets a patch a few millionths wide between the two, and on its
tip cap that patch samples to a grid only one cell across, or to a single line of points.
"""

import subprocess
import sys

import pytest


# A blunt trailing edge fin, the shape of the one on HARV_airfoilTest.vsp3: a swept section
# with a linear edit curve airfoil whose first corner sits at u1 at the root and 0.005 at the tip.
FIN = """
import sys
import openvsp as vsp
print( "READY" )
sys.stdout.flush()
u1r = %r
vsp.VSPRenew()
wid = vsp.AddGeom( "WING" )
vsp.SetDriverGroup( wid, 1, vsp.SPAN_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
vsp.SetParmVal( wid, "Span", "XSec_1", 0.1576 )
vsp.SetParmVal( wid, "Root_Chord", "XSec_1", 1.1065 )
vsp.SetParmVal( wid, "Tip_Chord", "XSec_1", 0.2125 )
vsp.SetParmVal( wid, "Sweep", "XSec_1", 80.0 )
vsp.Update()
xsurf = vsp.GetXSecSurf( wid, 0 )
pts = [ ( 0.5, 0.0 ), ( 0.5, -0.5 ), ( -0.25, -0.5 ), ( -0.5, 0.0 ), ( -0.25, 0.5 ), ( 0.5, 0.5 ),
        ( 0.5, 0.0 ) ]
for i, u1 in ( ( 0, u1r ), ( 1, 0.005 ) ):
    vsp.ChangeXSecShape( xsurf, i, vsp.XS_EDIT_CURVE )
    xs = vsp.GetXSec( xsurf, i )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "CurveType" ), vsp.LINEAR )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "Height" ), 0.0224 )
    u = [ 0.0, u1, 0.35, 0.5, 0.65, 1.0 - u1, 1.0 ]
    vsp.SetEditXSecPnts( xs, u, [ vsp.vec3d( x, y, 0.0 ) for x, y in pts ], [ 0.0 ] * 7 )
vsp.Update()
vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.1 )
vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.01 )
vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, 0 )
rid = vsp.FindLatestResultsID( "CFDMesh" )
print( "TRIS", vsp.GetIntResults( rid, "Num_Tris" )[0] )
"""


def _mesh_fin( u1r ):
    """Mesh the fin in a subprocess with a time limit; its stdout lines and return code."""
    try:
        done = subprocess.run( [ sys.executable, "-c", FIN % u1r ], capture_output = True,
                               text = True, timeout = 300 )
    except subprocess.TimeoutExpired:
        assert False, "meshing the fin never finished"

    lines = done.stdout.split( "\n" )
    if "READY" not in lines:
        pytest.skip( "the child could not load OpenVSP: %s" % done.stderr.strip()[-200:] )

    assert done.returncode >= 0, "meshing the fin died on signal %d" % -done.returncode
    return lines


def test_dist_map_grid_with_points_on_no_face( tmp_path, monkeypatch ):
    """A tip cap strip so thin that each row of its grid merges to one point has no faces."""
    monkeypatch.chdir( tmp_path )
    lines = _mesh_fin( 0.005001 )
    tris = [ int( l.split()[1] ) for l in lines if l.startswith( "TRIS " ) ]
    assert tris and tris[0] > 0, "the fin was not meshed: %s" % lines


def test_dist_map_grid_with_no_interior_point( tmp_path, monkeypatch ):
    """A tip cap strip whose grid is one cell across has no interior point to flatten around."""
    monkeypatch.chdir( tmp_path )
    lines = _mesh_fin( 0.005004 )
    tris = [ int( l.split()[1] ) for l in lines if l.startswith( "TRIS " ) ]
    assert tris and tris[0] > 0, "the fin was not meshed: %s" % lines
