# Import of a Cart3D .tri file holding a triangle with no area.
#
# The mesh drops a triangle with no area.  Three triangles on four points, the first through
# three points on a line, give a mesh of the other two.

import os
import tempfile

import openvsp as vsp


def test_tri_import_drops_degenerate_first_triangle():
    with tempfile.TemporaryDirectory() as tmp:
        tri = os.path.join( tmp, "degen.tri" )
        with open( tri, "w" ) as f:
            f.write( "4 3\n"
                     "0 0 0\n1 0 0\n2 0 0\n0 1 0\n"
                     "1 2 3\n1 2 4\n2 3 4\n"
                     "5\n7\n9\n" )

        vsp.VSPRenew()
        mesh = vsp.ImportFile( tri, vsp.IMPORT_CART3D_TRI, "" )
        assert vsp.GetGeomTypeName( mesh ) == "Mesh"
        vsp.Update()

        stl = os.path.join( tmp, "out.stl" )
        vsp.ExportFile( stl, vsp.SET_ALL, vsp.EXPORT_STL )
        with open( stl ) as f:
            assert f.read().count( "facet normal" ) == 2
