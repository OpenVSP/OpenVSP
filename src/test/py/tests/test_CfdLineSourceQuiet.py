"""A line source longer than Max Edge Len is meshed without printing to the console."""

import openvsp as vsp


def test_a_long_line_source_prints_nothing_of_its_own( capfd, tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    pid = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.DeleteAllCFDSources()
    vsp.AddCFDSource( vsp.LINE_SOURCE, pid, 0, 2.0, 1.0, 0.2, 0.0, 2.0, 1.0, 0.8, 0.0 )
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.5 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.05 )
    vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, str( tmp_path / "l.tri" ) )

    capfd.readouterr()
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )
    out = capfd.readouterr().out

    assert "Ret Len" not in out
