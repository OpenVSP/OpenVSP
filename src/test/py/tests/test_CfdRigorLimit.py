# Rigorous growth limiting across 3D space.
#
# Limiting only ever lowers the target edge length, so turned on it can only add triangles,
# and on a model where one surface's fine features sit beside another's coarse ones it must add
# some.  The limited mesh still has to close, and the threaded run has to write the mesh the
# serial run writes.

import hashlib

import openvsp as vsp


def _model():
    """A wing with a fine trailing edge passing close over a coarse pod."""
    vsp.VSPRenew()
    vsp.AddGeom( "POD", "" )
    wing = vsp.AddGeom( "WING", "" )
    vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 2.0 )
    vsp.SetParmVal( wing, "Z_Rel_Location", "XForm", 0.9 )
    vsp.SetParmVal( wing, "TotalSpan", "WingGeom", 8.0 )
    vsp.Update()
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 1.0 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.02 )
    vsp.SetCFDMeshVal( vsp.CFD_GROWTH_RATIO, 1.2 )


def _set_rigor( on ):
    gd = vsp.FindContainer( "CFDGridDensity", 0 )
    vsp.SetParmVal( vsp.FindParm( gd, "RigorLimit", "CFDGridDensity" ), on )


def _mesh( path, rigor, parallel = 1 ):
    _set_rigor( rigor )
    vsp.SetCFDMeshVal( vsp.CFD_PARALLEL_MESH_FLAG, parallel )
    vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, str( path ) )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )
    rid = vsp.FindLatestResultsID( "CFDMesh" )
    res = { n: vsp.GetIntResults( rid, n )[0] for n in ( "Num_Tris", "Water_Tight" ) }
    return res, hashlib.sha1( open( str( path ), "rb" ).read() ).hexdigest()


def testRigorousLimitingRefinesAndCloses( tmp_path ):
    _model()
    off, _ = _mesh( tmp_path / "off.tri", 0 )
    on, _ = _mesh( tmp_path / "on.tri", 1 )
    assert off["Water_Tight"] == 1 and on["Water_Tight"] == 1
    assert on["Num_Tris"] > off["Num_Tris"], "rigorous limiting added no triangles: %s vs %s" % ( on, off )


def testRigorousLimitingThreadedIsSerial( tmp_path ):
    _model()
    serial = _mesh( tmp_path / "s.tri", 1, parallel = 0 )
    threaded = _mesh( tmp_path / "t.tri", 1, parallel = 1 )
    assert serial == threaded
