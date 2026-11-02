# CFDMesh run on one model after another in one process.
#
# The meshing managers are singletons that outlive any one model, so whatever a run leaves in
# them is still there when the next model is meshed.  Small models are meshed in turn, several
# times over, and every run of a model has to write the very mesh its first run did, byte for
# byte; the mesh written on several threads has to be the one written on one.
#
# Memory is checked against what the allocator says is in use after VSPRenew.  The allocator's
# count moves by a megabyte or two from one identical round to the next, so the check is of
# the least that any of several rounds of a larger model leaves behind, against a margin well
# above that noise and well below what the managers kept before they gave back their storage.

import ctypes
import ctypes.util
import gc
import hashlib
import sys

import openvsp as vsp
import pytest


def _mesh( path, parallel = 1 ):
    """Mesh the model with split-join on and write a .tri file to path.

    Hands back the CFDMesh counts as a plain dict and the hash of the file written.
    """
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.8 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.2 )
    vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 1 )
    vsp.SetCFDMeshVal( vsp.CFD_PARALLEL_MESH_FLAG, parallel )
    vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, str( path ) )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )

    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert rid, "the mesher wrote no CFDMesh results entry"

    out = {}
    for name in vsp.GetAllDataNames( rid ):
        # The entry also carries the run time, which is not the same twice.
        if vsp.GetResultsType( rid, name ) == vsp.INT_DATA:
            out[name] = vsp.GetIntResults( rid, name )[0]
    return out, hashlib.sha1( open( path, "rb" ).read() ).hexdigest()


def _pod_and_wing():
    """A wing run through the middle of a pod, so the two really do intersect."""
    vsp.VSPRenew()
    vsp.AddGeom( "POD", "" )
    wing = vsp.AddGeom( "WING", "" )
    vsp.SetParmVal( vsp.FindParm( wing, "X_Rel_Location", "XForm" ), 2.0 )
    vsp.SetParmVal( vsp.FindParm( wing, "TotalSpan", "WingGeom" ), 8.0 )
    vsp.Update()


def _fuselage_and_tail():
    """A fuselage with a vertical fin set into its tail."""
    vsp.VSPRenew()
    vsp.AddGeom( "FUSELAGE", "" )
    fin = vsp.AddGeom( "WING", "" )
    vsp.SetParmVal( vsp.FindParm( fin, "X_Rel_Location", "XForm" ), 25.0 )
    vsp.SetParmVal( vsp.FindParm( fin, "Z_Rel_Location", "XForm" ), 0.5 )
    vsp.SetParmVal( vsp.FindParm( fin, "X_Rel_Rotation", "XForm" ), 90.0 )
    vsp.SetParmVal( vsp.FindParm( fin, "Sym_Planar_Flag", "Sym" ), 0 )
    vsp.SetParmVal( vsp.FindParm( fin, "TotalSpan", "WingGeom" ), 6.0 )
    vsp.Update()


def _airplane():
    """A fuselage with a wing, a tail and three pairs of pods through the wing: thirty surfaces
    and a few dozen intersection chains, several on each surface."""
    vsp.VSPRenew()
    vsp.AddGeom( "FUSELAGE", "" )

    wing = vsp.AddGeom( "WING", "" )
    vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 10.0 )
    vsp.SetParmVal( wing, "Z_Rel_Location", "XForm", -0.5 )

    htail = vsp.AddGeom( "WING", "" )
    vsp.SetParmVal( htail, "X_Rel_Location", "XForm", 26.0 )
    vsp.SetParmVal( htail, "TotalSpan", "WingGeom", 8.0 )

    fin = vsp.AddGeom( "WING", "" )
    vsp.SetParmVal( fin, "X_Rel_Location", "XForm", 25.0 )
    vsp.SetParmVal( fin, "Z_Rel_Location", "XForm", 0.5 )
    vsp.SetParmVal( fin, "X_Rel_Rotation", "XForm", 90.0 )
    vsp.SetParmVal( fin, "Sym_Planar_Flag", "Sym", 0 )
    vsp.SetParmVal( fin, "TotalSpan", "WingGeom", 5.0 )

    for y, z in ( ( 1.5, -0.2 ), ( 4.0, -0.3 ), ( 6.5, -0.2 ) ):
        pod = vsp.AddGeom( "POD", wing )
        vsp.SetParmVal( pod, "X_Rel_Location", "XForm", -2.0 )
        vsp.SetParmVal( pod, "Y_Rel_Location", "XForm", y )
        vsp.SetParmVal( pod, "Z_Rel_Location", "XForm", z )
        vsp.SetParmVal( pod, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
    vsp.Update()


MODELS = ( ( "pod and wing", _pod_and_wing ), ( "fuselage and tail", _fuselage_and_tail ) )

NROUND = 4


def _rounds( tmp_path ):
    """Mesh the models in turn NROUND times.  Hands back the counts and hash of every run by model."""
    runs = {}
    for r in range( NROUND ):
        for name, build in MODELS:
            build()
            runs.setdefault( name, [] ).append( _mesh( tmp_path / "repeat.tri" ) )
    return runs


def testEveryRunOfAModelAgrees( tmp_path ):
    """Each model meshes the same way every time, whatever was meshed before it."""
    runs = _rounds( tmp_path )

    for name, results in runs.items():
        counts, digest = results[0]
        assert counts["Num_Tris"] > 0, "%s produced no triangles" % name
        assert counts["Water_Tight"] == 1, "%s did not close: %s" % ( name, counts )
        for i in range( 1, len( results ) ):
            assert results[i][0] == counts, \
                   "run %d of %s disagrees with its first: %s vs %s" % ( i + 1, name, results[i][0], counts )
            assert results[i][1] == digest, "run %d of %s wrote a .tri file unlike its first's" % ( i + 1, name )


@pytest.mark.parametrize( "name, build", MODELS + ( ( "airplane", _airplane ), ) )
def testTheThreadsWriteTheMeshOneThreadDoes( name, build, tmp_path ):
    """The .tri file written on several threads is the one written on one, byte for byte."""
    build()
    serial = _mesh( tmp_path / "serial.tri", parallel = 0 )
    threaded = _mesh( tmp_path / "threaded.tri", parallel = 1 )

    assert serial[0]["Water_Tight"] == 1, "%s did not close: %s" % ( name, serial[0] )
    assert threaded[0] == serial[0], "%s on several threads: %s, on one: %s" % ( name, threaded[0], serial[0] )
    assert threaded[1] == serial[1], "%s wrote a different .tri file on several threads than on one" % name


class _MallocStatistics( ctypes.Structure ):
    _fields_ = [ ( "blocks_in_use", ctypes.c_uint ), ( "size_in_use", ctypes.c_size_t ),
                 ( "max_size_in_use", ctypes.c_size_t ), ( "size_allocated", ctypes.c_size_t ) ]


def _heap_in_use():
    """The bytes the C heap holds, from the macOS malloc zones; None elsewhere."""
    if sys.platform != "darwin":
        return None
    libc = ctypes.CDLL( ctypes.util.find_library( "c" ) )
    libc.malloc_zone_statistics.argtypes = [ ctypes.c_void_p, ctypes.POINTER( _MallocStatistics ) ]
    gc.collect()
    stats = _MallocStatistics()
    libc.malloc_zone_statistics( None, ctypes.byref( stats ) )
    return stats.size_in_use


def testAMeshLeavesNoStorageBehind( tmp_path ):
    """After VSPRenew, the heap is back near where a small model's mesh left it.

    The pod is meshed first so that whatever the meshers set up once is in the baseline.  Before
    CleanUp gave back the intersection curves and point lists, and before the chains, curves
    and Triangle lists lost along the way were freed, a round of the airplane left 3.9 MB to
    8.6 MB behind; since then, -1.2 MB to 2.8 MB.
    """
    if _heap_in_use() is None:
        pytest.skip( "the heap is measured through the macOS malloc zones" )

    vsp.VSPRenew()
    vsp.AddGeom( "POD", "" )
    vsp.Update()
    _mesh( tmp_path / "pod.tri" )
    vsp.VSPRenew()
    vsp.DeleteAllResults()
    base = _heap_in_use()

    kept = []
    for r in range( NROUND ):
        _airplane()
        _mesh( tmp_path / "airplane.tri" )
        vsp.VSPRenew()
        vsp.DeleteAllResults()
        kept.append( ( _heap_in_use() - base ) / 1.0e6 )

    assert min( kept ) < 3.0, "every round of the airplane left more than 3 MB behind: %s" % \
                              ", ".join( "%.2f" % k for k in kept )
