"""Models whose surfaces meet exactly, or nearly so, must still mesh closed.

Each case here is one of a catalog of near-coincident models -- surfaces that coincide, touch,
or meet where they are tangent -- that the CFD mesher once left open or empty.  The mesh is
coarse: these are about whether the surfaces are cut and joined, not about the mesh on them.
"""

import openvsp as vsp
import pytest


def _mesh( size ):
    """Mesh everything at edge lengths a tenth and a hundredth of size; the CFDMesh results."""
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, size / 10.0 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, size / 100.0 )
    vsp.SetCFDMeshVal( vsp.CFD_MAX_GAP, size / 200.0 )
    vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 1 )

    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, 0 )

    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert rid, "the mesher wrote no CFDMesh results entry"

    out = {}
    for name in ( "Num_Tris", "Num_Border_Edges", "Num_Over_Connected_Edges" ):
        out[name] = vsp.GetIntResults( rid, name )[0]
    return out


def _add( kind ):
    gid = vsp.AddGeom( kind )
    if kind == "POD":
        vsp.SetParmVal( gid, "Length", "Design", 4.0 )
        vsp.SetParmVal( gid, "FineRatio", "Design", 6.0 )
    return gid


@pytest.mark.parametrize( "kind", [ "POD", "WING" ] )
def test_duplicate_geom_meshes_once( kind, tmp_path, monkeypatch ):
    """A Geom given twice, one exactly on the other, is meshed as the one body it makes."""
    monkeypatch.chdir( tmp_path )

    vsp.VSPRenew()
    _add( kind )
    vsp.Update()
    single = _mesh( 4.0 )

    vsp.VSPRenew()
    _add( kind )
    _add( kind )
    vsp.Update()
    double = _mesh( 4.0 )

    assert double["Num_Tris"] > 0, "both copies were thrown away"
    assert double["Num_Border_Edges"] == 0
    assert double["Num_Over_Connected_Edges"] == 0
    assert double["Num_Tris"] == single["Num_Tris"]


def _widest( pid, n=401 ):
    """The largest radius of a pod lying along X, and the x it is reached at."""
    best_r = -1.0
    best_x = 0.0
    for k in range( n ):
        p = vsp.CompPnt01( pid, 0, k / ( n - 1.0 ), 0.25 )
        r = ( p.y() ** 2 + p.z() ** 2 ) ** 0.5
        if r > best_r:
            best_r = r
            best_x = p.x()
    return best_r, best_x


def test_fin_root_touching_pod( tmp_path, monkeypatch ):
    """A fin standing on a pod, its root chord's leading edge on the pod's widest point.

    The fin's root airfoil touches the pod there and at no other point, but a few of its control
    points lie within the tolerance of the pod's surface, and that was taken to mean the whole
    curve lay on it: the fin and the pod were never intersected.
    """
    monkeypatch.chdir( tmp_path )

    vsp.VSPRenew()
    pid = _add( "POD" )
    vsp.Update()
    r, x = _widest( pid )

    fin = vsp.AddGeom( "WING" )
    vsp.SetParmVal( fin, "Sym_Planar_Flag", "Sym", vsp.SYM_NONE )
    xs = vsp.GetXSec( vsp.GetXSecSurf( fin, 0 ), 1 )
    for name, v in ( ( "Span", 1.5 ), ( "Root_Chord", 1.0 ), ( "Tip_Chord", 0.6 ), ( "Sweep", 20.0 ) ):
        vsp.SetParmVal( vsp.GetXSecParm( xs, name ), v )
    vsp.SetParmVal( fin, "X_Rel_Location", "XForm", x )
    vsp.SetParmVal( fin, "Z_Rel_Location", "XForm", r )
    vsp.SetParmVal( fin, "X_Rel_Rotation", "XForm", 90.0 )
    vsp.Update()

    res = _mesh( 4.0 )

    assert res["Num_Border_Edges"] == 0
    assert res["Num_Over_Connected_Edges"] == 0


def _tail( sym, rot, scale=1.0 ):
    """A tail surface of span 2, chord 2 to 1, swept 30 degrees, turned about X by rot."""
    wid = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wid, "Sym_Planar_Flag", "Sym", sym )
    xs = vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 )
    for name, v in ( ( "Span", 2.0 * scale ), ( "Root_Chord", 2.0 * scale ), ( "Tip_Chord", 1.0 * scale ),
                     ( "Sweep", 30.0 ) ):
        vsp.SetParmVal( vsp.GetXSecParm( xs, name ), v )
    vsp.SetParmVal( wid, "X_Rel_Rotation", "XForm", rot )
    return wid


def test_crossed_tails_close( tmp_path, monkeypatch ):
    """A horizontal and a vertical tail of the same section, crossed at a shared root.

    Near the leading edge the two tails are tangent where their intersection curves cross, and
    each crossing is traced as two curves that touch at one point of both without crossing.
    Not cut there, the curves lose the point once each is resampled, and the triangles around
    it are kept or thrown away on the wrong side.
    """
    monkeypatch.chdir( tmp_path )

    vsp.VSPRenew()
    _tail( vsp.SYM_XZ, 0.0 )
    _tail( vsp.SYM_NONE, 90.0 )
    vsp.Update()

    res = _mesh( 4.0 )

    assert res["Num_Border_Edges"] == 0
    assert res["Num_Over_Connected_Edges"] == 0


def _control_surfaces( gap ):
    """A wing with two control surfaces side by side, the second starting gap past the first."""
    wid = vsp.AddGeom( "WING" )
    for e0, e1 in ( ( 0.2, 0.5 ), ( 0.5 + gap, 0.8 ) ):
        ss = vsp.AddSubSurf( wid, vsp.SS_CONTROL )
        for pid in vsp.GetSubSurfParmIDs( ss ):
            name = vsp.GetParmName( pid )
            if name == "EtaFlag":
                vsp.SetParmVal( pid, 1.0 )
            elif name == "EtaStart":
                vsp.SetParmVal( pid, e0 )
            elif name == "EtaEnd":
                vsp.SetParmVal( pid, e1 )
    vsp.Update()


def test_control_surfaces_sharing_an_edge( tmp_path, monkeypatch ):
    """Two control surfaces that share an edge mesh like two that are a hair apart.

    The two edges lie on each other, so the curves they make run together, point for point,
    along the whole chord.  Curves that run together are not cut where they touch.
    """
    monkeypatch.chdir( tmp_path )

    vsp.VSPRenew()
    _control_surfaces( 1.0e-3 )
    apart = _mesh( 4.0 )

    vsp.VSPRenew()
    _control_surfaces( 0.0 )
    shared = _mesh( 4.0 )

    assert shared["Num_Border_Edges"] == 0
    assert shared["Num_Over_Connected_Edges"] == 0
    assert shared["Num_Tris"] < 1.1 * apart["Num_Tris"]


def test_pod_along_fin_root( tmp_path, monkeypatch ):
    """A pod lying along a fin's root chord, its nose on the leading edge, its tail on the trailing.

    The pod and the fin are tangent at the pod's nose, and the curves they meet along there are
    traced only to within a few thousandths of it; their ends met nothing and left holes.
    """
    monkeypatch.chdir( tmp_path )

    vsp.VSPRenew()
    _tail( vsp.SYM_NONE, 90.0 )
    pid = vsp.AddGeom( "POD" )
    vsp.SetParmVal( pid, "Length", "Design", 2.0 )
    vsp.SetParmVal( pid, "FineRatio", "Design", 8.0 )
    vsp.Update()

    res = _mesh( 4.0 )

    assert res["Num_Border_Edges"] == 0
    assert res["Num_Over_Connected_Edges"] == 0


@pytest.mark.parametrize( "dz", [ 4.0e-9, 4.0e-7, 4.0e-5 ] )
def test_crossed_tails_nearly_meeting( dz, tmp_path, monkeypatch ):
    """The crossed tails with the vertical one raised the least bit off the shared root.

    The intersection curves that crossed now pass close by each other instead, 2.6e-5 apart for a
    rise of 4e-7, and the strip between them is far too narrow to mesh.
    """
    monkeypatch.chdir( tmp_path )

    vsp.VSPRenew()
    _tail( vsp.SYM_XZ, 0.0 )
    vt = _tail( vsp.SYM_NONE, 90.0 )
    vsp.SetParmVal( vt, "Z_Rel_Location", "XForm", dz )
    vsp.Update()

    res = _mesh( 4.0 )

    assert res["Num_Border_Edges"] == 0
    assert res["Num_Over_Connected_Edges"] == 0


def test_small_pod_along_fin_root( tmp_path, monkeypatch ):
    """The pod along the fin root, turned 60 degrees about its axis, at 1/100 scale.

    The model meshes closed at full scale.  At 1/100 a piece of the pod's intersection with the
    fin near the nose is shorter than 1e-4 and was thrown away, which a curve that short at full
    scale would not have been, and the mesh had 41 border edges.
    """
    monkeypatch.chdir( tmp_path )

    vsp.VSPRenew()
    _tail( vsp.SYM_NONE, 90.0, 0.01 )
    pid = vsp.AddGeom( "POD" )
    vsp.SetParmVal( pid, "Length", "Design", 0.02 )
    vsp.SetParmVal( pid, "FineRatio", "Design", 8.0 )
    vsp.SetParmVal( pid, "X_Rel_Rotation", "XForm", 60.0 )
    vsp.Update()

    res = _mesh( 0.04 )

    assert res["Num_Border_Edges"] == 0
    assert res["Num_Over_Connected_Edges"] == 0
