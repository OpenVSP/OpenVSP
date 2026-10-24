# A body joined by Split and Join meshes to size where a wing root lies across it.

import math
import os

import openvsp as vsp

from testhelp import drop_errors

MODEL = os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "CfdJoinedLadder.vsp3" )


def test_joined_body_under_a_wing_root_meshes_to_size( tmp_path, monkeypatch ):
    """The fuselage and horizontal tail of 2pack_1pax, Split and Join on.

    The joined aft fuselage starts as a ladder of triangles from its bottom line to the tail root,
    each edge across a quarter of the fuselage.  Splitting such an edge alone turns a face over.
    """
    monkeypatch.chdir( tmp_path )

    vsp.VSPRenew()
    vsp.ReadVSPFile( MODEL )
    drop_errors()

    vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 1 )
    vsp.SetComputationFileName( vsp.CFD_STL_TYPE, str( tmp_path / "mesh.stl" ) )
    vsp.ComputeCFDMesh( vsp.SET_SHOWN, vsp.SET_NONE, vsp.CFD_STL_TYPE )

    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert rid
    assert vsp.GetIntResults( rid, "Num_Border_Edges" )[0] == 0
    assert vsp.GetIntResults( rid, "Num_Over_Connected_Edges" )[0] == 0

    pnts = []
    for line in open( tmp_path / "mesh.stl" ):
        words = line.split()
        if words and words[0] == "vertex":
            pnts.append( tuple( float( w ) for w in words[1:4] ) )

    # The fuselage bottom under the tail root, where edges are 0.04 long.
    longest = 0.0
    for i in range( 0, len( pnts ), 3 ):
        tri = pnts[i:i + 3]
        for a, b in ( ( tri[0], tri[1] ), ( tri[1], tri[2] ), ( tri[2], tri[0] ) ):
            mid = [ 0.5 * ( a[k] + b[k] ) for k in range( 3 ) ]
            if 17.5 < mid[0] < 19.2 and abs( mid[1] ) < 0.45 and mid[2] < 0.05:
                longest = max( longest, math.dist( a, b ) )

    assert 0.0 < longest < 0.3
