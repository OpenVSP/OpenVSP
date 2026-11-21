"""The far field box's lengths follow its scales, and a relative far field Max Edge Len is a
fraction of the diagonal of the far field made: the box, or the component used as the far field."""

import math

import openvsp as vsp


def density( name ):
    return vsp.FindParm( vsp.FindContainer( "CFDGridDensity", 0 ), name, "CFDGridDensity" )


def setting( name ):
    return vsp.FindParm( vsp.FindContainer( "CFDMeshSettings", 0 ), name, "FarField" )


def model( tmp_path ):
    vsp.VSPRenew()
    pid = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 1.0 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.1 )
    vsp.SetParmVal( setting( "FarMesh" ), 1.0 )
    vsp.SetParmVal( density( "FarAbsRel" ), vsp.REL )
    vsp.SetParmVal( density( "FarFrac" ), 0.05 )
    vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, str( tmp_path / "f.tri" ) )
    return pid


def diag( gid ):
    bmin = vsp.GetGeomBBoxMin( gid )
    bmax = vsp.GetGeomBBoxMax( gid )
    return [ bmax.x() - bmin.x(), bmax.y() - bmin.y(), bmax.z() - bmin.z() ]


def test_the_box_and_its_relative_edge_length( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    pid = model( tmp_path )
    vsp.SetParmVal( setting( "FarXScale" ), 3.0 )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )

    lwh = diag( pid )
    size = [ vsp.GetParmVal( setting( n ) ) for n in ( "FarLength", "FarWidth", "FarHeight" ) ]
    assert abs( size[ 0 ] - 3.0 * lwh[ 0 ] ) < 1e-9
    assert abs( size[ 1 ] - 4.0 * lwh[ 1 ] ) < 1e-9
    assert abs( size[ 2 ] - 4.0 * lwh[ 2 ] ) < 1e-9

    far = vsp.GetParmVal( density( "MaxFar" ) )
    assert abs( far - 0.05 * math.sqrt( sum( s * s for s in size ) ) ) < 1e-9


def test_an_absolute_box_gives_its_scales( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    pid = model( tmp_path )
    vsp.SetParmVal( setting( "FarAbsSize" ), 1.0 )
    vsp.SetParmVal( setting( "FarLength" ), 50.0 )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )

    assert abs( vsp.GetParmVal( setting( "FarXScale" ) ) - 50.0 / diag( pid )[ 0 ] ) < 1e-9


def test_a_component_far_field( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    model( tmp_path )
    big = vsp.AddGeom( "POD" )
    vsp.SetParmVal( big, "Length", "Design", 60.0 )
    vsp.SetParmVal( big, "X_Rel_Location", "XForm", -25.0 )
    vsp.SetSetFlag( big, vsp.SET_FIRST_USER, True )
    vsp.Update()
    vsp.SetParmVal( setting( "FarComp" ), 1.0 )
    vsp.SetCFDFarFieldGeomID( big )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )

    far = vsp.GetParmVal( density( "MaxFar" ) )
    assert abs( far - 0.05 * math.sqrt( sum( s * s for s in diag( big ) ) ) ) < 1e-9
