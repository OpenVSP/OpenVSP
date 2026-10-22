# Conversion of a mesh to an NGon mesh.
#
# A mesh read back from a file carries tags that the tag tables, which describe the last mesh
# tagged, do not hold.  The conversion runs in a fresh process, where those tables are empty.

import os
import subprocess
import sys
import tempfile

import openvsp as vsp


CONVERT = """
import sys
import openvsp as vsp
vsp.ReadVSPFile( sys.argv[1] )
vsp.Update()
mesh = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Mesh" ][0]
ngon = vsp.CreateNGonMeshGeom( mesh )
assert vsp.GetGeomTypeName( ngon ) == "NGonMesh"
"""


def test_ngon_from_mesh_read_from_file():
    with tempfile.TemporaryDirectory() as tmp:
        fname = os.path.join( tmp, "compgeom.vsp3" )

        vsp.VSPRenew()
        vsp.AddGeom( "WING" )
        vsp.Update()
        vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
        vsp.WriteVSPFile( fname )
        vsp.VSPRenew()

        result = subprocess.run( [ sys.executable, "-c", CONVERT, fname ], capture_output = True, text = True )
        assert result.returncode == 0, result.stderr[-2000:]
