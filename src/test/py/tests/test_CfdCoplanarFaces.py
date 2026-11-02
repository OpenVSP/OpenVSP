# Two bodies meeting at flat ends that lie in one plane.
#
# A smaller body's flat end laid against a larger body's flat end leaves the two faces in one
# plane, overlapping but not alike.  CFDMesh intersects such a pair in their plane and copies
# the borders of one onto the other.  Those copies are made after every other intersection
# curve, and they have to be whole curves all the same: the POGS and CAD writers read them.
#
# The mesh runs in a process of its own, so that a crash fails this test and not the suite.

import os
import subprocess
import sys

SCRIPT = r'''
import sys
import openvsp as vsp

def body( x, dia, capmin, capmax ):
    s = vsp.AddGeom( "STACK" )
    vsp.SetParmVal( s, "X_Rel_Location", "XForm", x )
    xs_surf = vsp.GetXSecSurf( s, 0 )
    for i in range( vsp.GetNumXSec( xs_surf ) ):
        vsp.ChangeXSecShape( xs_surf, i, vsp.XS_CIRCLE )
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xs_surf, i ), "Circle_Diameter" ), dia )
    vsp.SetParmVal( s, "CapUMinOption", "EndCap", capmin )
    vsp.SetParmVal( s, "CapUMaxOption", "EndCap", capmax )
    return s

out = sys.argv[1]

big = body( 0.0, 2.0, vsp.ROUND_END_CAP, vsp.FLAT_END_CAP )
vsp.Update()
end = vsp.GetGeomBBoxMax( big, 0, True ).x()
body( end, 1.0, vsp.FLAT_END_CAP, vsp.ROUND_END_CAP )
vsp.Update()

vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.3 )
vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.05 )
vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 1 )
vsp.SetComputationFileName( vsp.CFD_POGS_TYPE, out + "/faces.pogs" )
vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, out + "/faces.tri" )
vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_POGS_TYPE | vsp.CFD_TRI_TYPE )

rid = vsp.FindLatestResultsID( "CFDMesh" )
print( "RESULT", vsp.GetIntResults( rid, "Num_Border_Edges" )[0], vsp.GetIntResults( rid, "Num_Over_Connected_Edges" )[0] )
'''


def testBodiesMeetingAtFlatEndsWritePOGS( tmp_path ):
    """The mesh closes and every POGS file is written."""
    r = subprocess.run( [ sys.executable, "-c", SCRIPT, str( tmp_path ) ], capture_output = True, text = True )

    assert r.returncode == 0, "meshing two bodies meeting at flat ends failed with %d:\n%s" % \
           ( r.returncode, r.stderr[-2000:] )

    lines = [ l for l in r.stdout.splitlines() if l.startswith( "RESULT" ) ]
    assert lines, "the mesh reported no results:\n%s" % r.stdout[-2000:]
    nborder, nover = [ int( v ) for v in lines[-1].split()[1:] ]
    assert nborder == 0 and nover == 0, "%d border and %d over-connected edges" % ( nborder, nover )

    for ext in ( ".pogs", ".pogs.topo", ".pogs.cur" ):
        assert os.path.exists( os.path.join( str( tmp_path ), "faces" + ext ) ), "no POGS file faces%s" % ext
