"""The Rel forms of the CFD Mesh edge lengths are used by a run started from the API, as they
are by the screen."""

import math

import openvsp as vsp


def density( name ):
    return vsp.FindParm( vsp.FindContainer( "CFDGridDensity", 0 ), name, "CFDGridDensity" )


def test_a_relative_max_edge_len_is_a_fraction_of_the_model( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    pid = vsp.AddGeom( "POD" )
    vsp.Update()

    vsp.SetParmVal( density( "BaseAbsRel" ), vsp.REL )
    vsp.SetParmVal( density( "BaseLenFrac" ), 0.05 )
    vsp.SetParmVal( density( "MinAbsRel" ), vsp.REL )
    vsp.SetParmVal( density( "MinFrac" ), 0.1 )

    vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, str( tmp_path / "p.tri" ) )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )

    bmin = vsp.GetGeomBBoxMin( pid )
    bmax = vsp.GetGeomBBoxMax( pid )
    diag = math.sqrt( ( bmax.x() - bmin.x() ) ** 2 + ( bmax.y() - bmin.y() ) ** 2 + ( bmax.z() - bmin.z() ) ** 2 )

    base = vsp.GetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN )
    assert abs( base - 0.05 * diag ) < 1e-9 * diag
    assert abs( vsp.GetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN ) - 0.1 * base ) < 1e-12 * diag
