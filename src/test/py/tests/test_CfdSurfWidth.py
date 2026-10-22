"""A surface narrower than the target edge length is meshed across its width.

A vertical tail with a blunt trailing edge has little curvature near the trailing edge, so
nothing there asks for edges shorter than the maximum.  The flat trailing edge cap is a strip
far narrower than that; cut its borders at the maximum and every triangle on it, and on the
skins along it, is a sliver.  The width of the strip limits the target length, so its borders
are cut no coarser than it is wide.

The limit is a switch on the grid density, on by default, for CFD and FEA meshes alike, with
the number of edges asked across the width beside it.
"""

import math
import os
import re

import openvsp as vsp


def read_stl_triangles( fname ):
    tris = []
    pts = []
    with open( fname ) as f:
        for line in f:
            words = line.split()
            if words and words[0] == "vertex":
                pts.append( [ float( x ) for x in words[1:4] ] )
                if len( pts ) == 3:
                    tris.append( pts )
                    pts = []
    return tris


def min_angle_deg( tri ):
    amin = 180.0
    for i in range( 3 ):
        a = tri[i]
        b = tri[ ( i + 1 ) % 3 ]
        c = tri[ ( i + 2 ) % 3 ]
        u = [ b[k] - a[k] for k in range( 3 ) ]
        v = [ c[k] - a[k] for k in range( 3 ) ]
        nu = math.sqrt( sum( x * x for x in u ) )
        nv = math.sqrt( sum( x * x for x in v ) )
        if nu == 0.0 or nv == 0.0:
            return 0.0
        cosang = sum( u[k] * v[k] for k in range( 3 ) ) / ( nu * nv )
        amin = min( amin, math.degrees( math.acos( max( -1.0, min( 1.0, cosang ) ) ) ) )
    return amin


def blunt_te_tail():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wid, "Sym_Planar_Flag", "Sym", vsp.SYM_NONE )
    vsp.SetParmVal( wid, "X_Rel_Rotation", "XForm", -90.0 )

    surf = vsp.GetXSecSurf( wid, 0 )
    xs = vsp.GetXSec( surf, 1 )
    for name, val in ( ( "Span", 3.0 ), ( "Root_Chord", 2.0 ), ( "Tip_Chord", 2.0 ), ( "Sweep", 0.0 ) ):
        vsp.SetParmVal( vsp.GetXSecParm( xs, name ), val )

    for i in range( 2 ):
        xc = vsp.GetXSec( surf, i )
        vsp.SetParmVal( vsp.GetXSecParm( xc, "TE_Close_Type" ), vsp.CLOSE_SKEWBOTH )
        vsp.SetParmVal( vsp.GetXSecParm( xc, "TE_Close_AbsRel" ), vsp.ABS )
        vsp.SetParmVal( vsp.GetXSecParm( xc, "TE_Close_Thick" ), 0.035 )
        vsp.SetParmVal( vsp.GetXSecParm( xc, "TE_Cap_Type" ), vsp.FLAT_END_CAP )
    vsp.Update()
    return wid


def cfd_mesh( tmp_path, width_limit, nseg = 1.0 ):
    blunt_te_tail()

    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.25 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.001 )
    vsp.SetCFDMeshVal( vsp.CFD_MAX_GAP, 0.01 )
    vsp.SetCFDMeshVal( vsp.CFD_GROWTH_RATIO, 1.3 )

    dens_id = vsp.FindContainer( "CFDGridDensity", 0 )
    flag_id = vsp.FindParm( dens_id, "WidthLimit", "CFDGridDensity" )
    assert vsp.GetParmVal( flag_id ) == 1.0
    vsp.SetParmVal( flag_id, float( width_limit ) )

    nseg_id = vsp.FindParm( dens_id, "NWidthSeg", "CFDGridDensity" )
    assert vsp.GetParmVal( nseg_id ) == 1.0
    vsp.SetParmVal( nseg_id, nseg )

    fname = str( tmp_path / "bluntte.stl" )
    vsp.SetComputationFileName( vsp.CFD_STL_TYPE, fname )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_STL_TYPE )

    tris = read_stl_triangles( fname )
    assert len( tris ) > 0
    return tris


def cfd_worst_angle( tmp_path, width_limit, nseg = 1.0 ):
    return min( min_angle_deg( t ) for t in cfd_mesh( tmp_path, width_limit, nseg ) )


def test_blunt_trailing_edge_strip_has_no_slivers( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )

    assert cfd_worst_angle( tmp_path, True ) > 5.0


def test_width_limit_off_leaves_the_strip_coarse( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )

    assert cfd_worst_angle( tmp_path, False ) < 5.0


def test_more_width_segments_refine_the_strip( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )

    one = len( cfd_mesh( tmp_path, True, 1.0 ) )
    two = cfd_mesh( tmp_path, True, 2.0 )

    assert len( two ) > one
    assert min( min_angle_deg( t ) for t in two ) > 5.0


def fea_nodes( tmp_path, width_limit ):
    wid = blunt_te_tail()
    vsp.AddFeaStruct( wid )
    vsp.Update()

    vsp.SetFeaMeshVal( wid, 0, vsp.CFD_MAX_EDGE_LEN, 0.25 )
    vsp.SetFeaMeshVal( wid, 0, vsp.CFD_MIN_EDGE_LEN, 0.001 )

    struct_id = vsp.GetFeaStructID( wid, 0 )
    flag_id = vsp.FindParm( struct_id, "WidthLimit", "FEAGridDensity" )
    assert vsp.GetParmVal( flag_id ) == 1.0
    vsp.SetParmVal( flag_id, float( width_limit ) )

    nastran = str( tmp_path / "bluntte.nas" )
    vsp.SetFeaMeshFileName( wid, 0, vsp.FEA_NASTRAN_FILE_NAME, nastran )
    vsp.ComputeFeaMesh( struct_id, vsp.FEA_NASTRAN_FILE_NAME )

    assert os.path.exists( nastran )
    m = re.search( r"^\$ Num_Nodes:\s+(\d+)", open( nastran ).read(), re.M )
    assert m
    return int( m.group( 1 ) )


def test_width_limit_refines_an_fea_skin( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )

    assert fea_nodes( tmp_path, True ) > fea_nodes( tmp_path, False )
