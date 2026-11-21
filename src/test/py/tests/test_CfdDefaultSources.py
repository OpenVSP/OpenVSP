"""AddDefaultSources looks at the model's own sources and Max Edge Len, not those of a
previous run or model."""

import openvsp as vsp


def source_lens( gid ):
    return [ vsp.GetParmVal( vsp.FindParm( s, "SrcLen", "Source" ) ) for s in vsp.GetCFDSourceIDVec( gid ) ]


def test_a_new_model_gets_its_default_sources( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    pid = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.AddDefaultSources()
    assert len( vsp.GetCFDSourceIDVec( pid ) ) > 0
    vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, str( tmp_path / "a.tri" ) )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )

    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.Update()
    vsp.AddDefaultSources()
    assert len( vsp.GetCFDSourceIDVec( wid ) ) > 0


def test_default_sources_follow_max_edge_len( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    lens = []
    for base in ( 0.5, 2.0 ):
        vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, base )
        vsp.DeleteAllCFDSources()
        vsp.AddDefaultSources()
        lens.append( source_lens( fid ) )
    assert len( lens[ 0 ] ) > 0
    for a, b in zip( lens[ 0 ], lens[ 1 ] ):
        assert abs( b - 4.0 * a ) < 1e-12
