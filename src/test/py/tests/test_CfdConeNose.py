# A cone whose point is inside a body meshes closed where its side comes out through the body.

import os

import openvsp as vsp

from testhelp import drop_errors

MODEL = os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "CfdConeNose.vsp3" )


def test_cone_nose_coming_out_of_a_body_meshes_closed( tmp_path, monkeypatch ):
    """The dorsal fin of 2pack_1pax: a Stack from a point, its top coming out of the fuselage.

    The fin's first patch has two corners at its point.  The fin's top line leaves the fuselage
    inside that patch, so the last part of the curve where the two meet is found only there.
    """
    monkeypatch.chdir( tmp_path )

    vsp.VSPRenew()
    vsp.ReadVSPFile( MODEL )
    drop_errors()

    vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 0 )
    vsp.ComputeCFDMesh( vsp.SET_SHOWN, vsp.SET_NONE, 0 )

    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert rid

    assert vsp.GetIntResults( rid, "Num_Tris" )[0] > 0
    assert vsp.GetIntResults( rid, "Num_Border_Edges" )[0] == 0
    assert vsp.GetIntResults( rid, "Num_Over_Connected_Edges" )[0] == 0
