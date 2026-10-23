# Triangle triangulates only one of two input points at the same place, and inserting a segment
# from the other reads a triangle pointer that was never set.  The mesher gives it each place once.

import os
import subprocess
import sys
import tempfile

import pytest


CORNER_FIXED_POINT = """
import sys
import openvsp as vsp
print( "READY" )
sys.stdout.flush()
vsp.VSPRenew()
pod = vsp.AddGeom( "POD" )
ind = vsp.AddFeaStruct( pod )
sid = vsp.GetFeaStructID( pod, ind )
vsp.SetFeaMeshVal( pod, ind, vsp.CFD_MAX_EDGE_LEN, 1.0 )
vsp.SetFeaMeshVal( pod, ind, vsp.CFD_MIN_EDGE_LEN, 0.2 )
vsp.SetFeaMeshStructIndex( ind )
sl = vsp.AddFeaPart( pod, ind, vsp.FEA_SLICE )
vsp.SetParmVal( vsp.FindParm( sl, "RelCenterLocation", "FeaPart" ), 0.5 )
fp = vsp.AddFeaPart( pod, ind, vsp.FEA_FIX_POINT )
vsp.SetFeaFixPointParentID( fp, sl )
vsp.SetParmVal( vsp.FindParm( fp, "PosU", "FeaFixPoint" ), 0.0 )
vsp.SetParmVal( vsp.FindParm( fp, "PosW", "FeaFixPoint" ), 0.0 )
vsp.Update()
vsp.SetFeaMeshFileName( pod, ind, vsp.FEA_NASTRAN_FILE_NAME, %r )
vsp.ComputeFeaMesh( sid, vsp.FEA_NASTRAN_FILE_NAME )
print( "MESHED" )
"""


def testFixedPointOnASliceCornerMeshes():
    """A fixed point at a slice's u,w corner is at the same place as the slice's border point
    there.  The slice meshes, and the fixed point is written."""
    out = tempfile.mkdtemp()
    nas = os.path.join( out, "corner.nas" )

    # Filled allocations make the unset pointer the same every run, so the read fails every time
    # rather than now and then.
    env = dict( os.environ, MallocScribble = "1" )

    try:
        done = subprocess.run( [ sys.executable, "-c", CORNER_FIXED_POINT % nas ], capture_output = True,
                               text = True, timeout = 300, env = env )
    except subprocess.TimeoutExpired:
        assert False, "meshing a fixed point on a slice corner never finished"

    lines = done.stdout.split( "\n" )
    if "READY" not in lines:
        pytest.skip( "the child could not load OpenVSP: %s" % done.stderr.strip()[-200:] )

    assert done.returncode >= 0, \
           "meshing a fixed point on a slice corner died on signal %d" % -done.returncode
    assert "MESHED" in lines, "the mesh did not finish: %s" % done.stderr[-400:]
    assert "spider points" in open( nas + ".bdf" ).read(), "the fixed point was not written"
