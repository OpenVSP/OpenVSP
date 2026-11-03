# Which samples of a POGS face the trimming removed.
#
# Each POGS face is an untrimmed surface sampled on a grid, and a sample the mesher's trimming
# removed is written with an iblank of zero.  The triangulation written alongside, its triangles
# tagged by face, is the trimmed surface, so the samples are checked against it: a kept sample
# lies on a triangle of its face, and a removed one lies off them.  Samples near the edge of a
# face's triangles are left out, since the triangles only approximate a trim curve there.
#
# Every sample beside a change of iblank is checked, which includes a sample removed alone in
# the middle of a kept region, and a stride of the rest.

import os
import struct

import numpy as np
import openvsp as vsp

from testhelp import drop_errors

HERE = os.path.dirname( os.path.abspath( __file__ ) )
NEGATIVE_MODEL = os.path.join( HERE, "CfdNegativePodWing.vsp3" )
DISK_MODEL = os.path.join( HERE, "CfdActuatorDiskPodWing.vsp3" )


def _records( path ):
    """The records of a Fortran unformatted, little-endian file."""
    data = open( path, "rb" ).read()
    recs = []
    p = 0
    while p < len( data ):
        n = struct.unpack( "<i", data[p:p + 4] )[0]
        recs.append( data[p + 4:p + 4 + n] )
        p += 8 + n
    return recs


def _read_uvin( path ):
    """The faces of a POGS .uvin file: per face, its grid size, points (n, 3) and iblank, u fastest."""
    recs = _records( path )
    nface = struct.unpack( "<i", recs[0] )[0]
    dims = struct.unpack( "<%di" % ( 3 * nface ), recs[1] )
    faces = []
    for k in range( nface ):
        ni, nj = dims[3 * k], dims[3 * k + 1]
        n = ni * nj
        xyz = np.frombuffer( recs[2 + k][:24 * n], "<f8" ).reshape( 3, n ).T
        ib = np.frombuffer( recs[2 + k][24 * n:28 * n], "<i4" )
        faces.append( ( ni, nj, xyz, ib ) )
    return faces


def _read_itri( path ):
    """The points, triangles (0-based) and face tags of a POGS .i.tri file."""
    recs = _records( path )
    nv, nt = struct.unpack( "<2i", recs[0] )
    xyz = np.frombuffer( recs[1], "<f4" ).reshape( nv, 3 ).astype( float )
    tri = np.frombuffer( recs[2], "<i4" ).reshape( nt, 3 ) - 1
    tag = np.frombuffer( recs[3], "<i4" )
    return xyz, tri, tag


def _mesh_pogs( base ):
    """Mesh every Geom to a POGS file set named base, and hand back its faces and triangulation."""
    vsp.SetComputationFileName( vsp.CFD_POGS_TYPE, str( base ) + ".i.tri" )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_POGS_TYPE )
    return _read_uvin( str( base ) + ".uvin" ), _read_itri( str( base ) + ".i.tri" )


def _near_a_change( ni, nj, ib, stride ):
    """Every sample beside one with the other iblank, and every stride-th sample besides."""
    g = ib.reshape( nj, ni )
    change = np.zeros( g.shape, dtype = bool )
    change[:, 1:] |= g[:, 1:] != g[:, :-1]
    change[:, :-1] |= g[:, :-1] != g[:, 1:]
    change[1:, :] |= g[1:, :] != g[:-1, :]
    change[:-1, :] |= g[:-1, :] != g[1:, :]
    pick = change.ravel()
    pick[::stride] = True
    return np.nonzero( pick )[0]


def _seg_dist( p, a, b ):
    """Distance from each point of p (n, 3) to each of its segments a b (n, k, 3): (n, k)."""
    ab = b - a
    l2 = np.maximum( ( ab * ab ).sum( 2 ), 1.0e-300 )
    ap = p[:, None, :] - a
    t = np.clip( ( ap * ab ).sum( 2 ) / l2, 0.0, 1.0 )
    return np.linalg.norm( ap - t[..., None] * ab, axis = 2 )


def _tri_dist( p, a, b, c ):
    """Distance from each point of p (n, 3) to each of its triangles a b c (n, k, 3): (n, k)."""
    n = np.cross( b - a, c - a )
    n = n / np.maximum( np.linalg.norm( n, axis = 2 ), 1.0e-300 )[..., None]
    h = ( ( p[:, None, :] - a ) * n ).sum( 2 )
    q = p[:, None, :] - h[..., None] * n

    def side( u, v ):
        return ( np.cross( v - u, q - u ) * n ).sum( 2 )

    inside = ( side( a, b ) >= 0 ) & ( side( b, c ) >= 0 ) & ( side( c, a ) >= 0 )
    edge = np.minimum( np.minimum( _seg_dist( p, a, b ), _seg_dist( p, b, c ) ), _seg_dist( p, c, a ) )
    return np.where( inside, np.abs( h ), edge )


def _nearest( p, cen, k ):
    """The k items whose centres cen (m, 3) lie nearest each point of p (n, 3): (n, k) indices."""
    d2 = ( p * p ).sum( 1 )[:, None] + ( cen * cen ).sum( 1 )[None] - 2.0 * p @ cen.T
    k = min( k, len( cen ) )
    return np.argpartition( d2, k - 1, axis = 1 )[:, :k]


def _check( faces, itri, stride ):
    """Every face's iblank against its triangles; returns the samples that disagree."""
    xyz, tri, tag = itri
    wrong = []
    for f, ( ni, nj, pts, ib ) in enumerate( faces ):
        mine = tri[tag == f + 1]
        assert len( mine ), "face %d has no triangles" % ( f + 1 )
        a, b, c = xyz[mine[:, 0]], xyz[mine[:, 1]], xyz[mine[:, 2]]
        cen = ( a + b + c ) / 3.0
        size = np.maximum( np.maximum( np.linalg.norm( b - a, axis = 1 ), np.linalg.norm( c - b, axis = 1 ) ),
                           np.linalg.norm( a - c, axis = 1 ) )

        # The edges of the face's triangles used by one triangle only: its trim curves and borders.
        edges = np.sort( np.concatenate( [ mine[:, [0, 1]], mine[:, [1, 2]], mine[:, [2, 0]] ] ), axis = 1 )
        uniq, count = np.unique( edges, axis = 0, return_counts = True )
        rim = uniq[count == 1]
        ra, rb = xyz[rim[:, 0]], xyz[rim[:, 1]]
        rmid = 0.5 * ( ra + rb )
        rim_len = np.linalg.norm( rb - ra, axis = 1 )

        ks = _near_a_change( ni, nj, ib, stride )
        for k0 in range( 0, len( ks ), 4096 ):
            kk = ks[k0:k0 + 4096]
            p = pts[kk]

            # The triangles and rim edges near each sample, by their centres, measured exactly.
            t = _nearest( p, cen, 16 )
            d = _tri_dist( p, a[t], b[t], c[t] )
            best = d.argmin( 1 )
            row = np.arange( len( kk ) )
            on = d[row, best] <= 0.1 * size[t[row, best]]

            by_rim = np.zeros( len( kk ), dtype = bool )
            if len( rim ):
                r = _nearest( p, rmid, 16 )
                by_rim = ( _seg_dist( p, ra[r], rb[r] ) <= rim_len[r] ).any( 1 )

            for m in np.nonzero( ~by_rim & ( on != ( ib[kk] == 1 ) ) )[0]:
                wrong.append( ( f + 1, int( kk[m] % ni ), int( kk[m] // ni ), int( ib[kk[m]] ) ) )
    return wrong


def _assert_agrees( faces, itri, stride ):
    wrong = _check( faces, itri, stride )
    assert not wrong, "%d samples disagree with the triangulation (face, i, j, iblank), first %s" % \
           ( len( wrong ), wrong[:5] )


def test_a_wing_through_a_body_with_a_pod_on_it( tmp_path, monkeypatch ):
    """A wing through a fuselage, with a pod on each side lying through the wing."""
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()

    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fuse, "Length", "Design", 10.0 )
    vsp.SetParmVal( fuse, "Tess_W", "Shape", 49 )

    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 3.0 )
    vsp.SetParmVal( wing, "Z_Rel_Location", "XForm", -0.2 )
    vsp.SetParmVal( wing, "TotalSpan", "WingGeom", 12.0 )
    vsp.SetParmVal( wing, "Tess_W", "Shape", 41 )

    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( pod, "Length", "Design", 3.0 )
    vsp.SetParmVal( pod, "FineRatio", "Design", 6.0 )
    vsp.SetParmVal( pod, "X_Rel_Location", "XForm", 2.8 )
    vsp.SetParmVal( pod, "Y_Rel_Location", "XForm", 3.0 )
    vsp.SetParmVal( pod, "Z_Rel_Location", "XForm", -0.3 )
    vsp.SetParmVal( pod, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
    vsp.SetParmVal( pod, "Tess_W", "Shape", 33 )
    vsp.Update()

    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.5 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.05 )

    faces, itri = _mesh_pogs( tmp_path / "wb" )
    _assert_agrees( faces, itri, 7 )


def test_rotors_in_a_row( tmp_path, monkeypatch ):
    """Three three-bladed rotors one behind another beside a fuselage.

    A ray cast along x from one rotor's blade passes edge-on through the next rotor's blades.
    """
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()

    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fuse, "Length", "Design", 10.0 )

    for k in range( 3 ):
        prop = vsp.AddGeom( "PROP" )
        vsp.SetParmVal( prop, "Diameter", "Design", 4.0 )
        vsp.SetParmVal( prop, "NumBlade", "Design", 3 )
        vsp.SetParmVal( prop, "X_Rel_Location", "XForm", 2.0 + 2.5 * k )
        vsp.SetParmVal( prop, "Y_Rel_Location", "XForm", 3.0 )
        vsp.SetParmVal( prop, "Z_Rel_Location", "XForm", 1.5 )
        vsp.SetParmVal( prop, "Y_Rel_Rotation", "XForm", 90.0 )
        vsp.SetParmVal( prop, "Rotate", "Design", 17.0 * k )
        vsp.SetParmVal( prop, "PropMode", "Design", vsp.PROP_BLADES )
    vsp.Update()

    cfd = vsp.FindContainer( "CFDMeshSettings", 0 )
    vsp.SetParmVal( vsp.FindParm( cfd, "POGSNRef", "Global" ), 3 )
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.5 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.02 )

    faces, itri = _mesh_pogs( tmp_path / "rotors" )
    _assert_agrees( faces, itri, 31 )


def test_a_pod_and_wing_cut_by_negative_volumes( tmp_path, monkeypatch ):
    """A pod and wing with a hole through the pod and a divot in each wing, all negative volumes.

    The samples are refined five times so that some lie within the tolerance of a curve the
    negative volumes cut, where a sample's side of the curve is read from the negative surface.
    """
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    vsp.ReadVSPFile( NEGATIVE_MODEL )
    drop_errors()

    cfd = vsp.FindContainer( "CFDMeshSettings", 0 )
    vsp.SetParmVal( vsp.FindParm( cfd, "POGSNRef", "Global" ), 5 )
    vsp.Update()

    faces, itri = _mesh_pogs( tmp_path / "podwing" )
    _assert_agrees( faces, itri, 97 )


def test_actuator_disks_on_a_pod_and_wing( tmp_path, monkeypatch ):
    """Propellers in disk mode, one inside a duct, on a pod and wing: surfaces with no inside."""
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    vsp.ReadVSPFile( DISK_MODEL )
    drop_errors()
    vsp.Update()

    faces, itri = _mesh_pogs( tmp_path / "disks" )
    _assert_agrees( faces, itri, 31 )
