"""A border chain is split only away from its ends.

A horizontal and a vertical tail of the same root airfoil cross at a shared root, the vertical
tail's sections turned to match its dihedral.  Its root cap collapses to the camber line, which
reaches the leading edge at a second u,w on the cap; the chain ends there split the borders of
the patches beside the cap.  Each patch must triangulate as asked, and the mesher says when one
does not.
"""

import openvsp as vsp


def test_crossed_tail_roots_triangulate( capfd, tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )

    vsp.VSPRenew()
    for dihedral, sym in ( ( 0.0, vsp.SYM_XZ ), ( 90.0, vsp.SYM_NONE ) ):
        wid = vsp.AddGeom( "WING" )
        vsp.SetParmVal( wid, "Sym_Planar_Flag", "Sym", sym )
        vsp.SetParmVal( wid, "RotateAirfoilMatchDideralFlag", "WingGeom", 0.0 )
        surf = vsp.GetXSecSurf( wid, 0 )
        for i in range( 2 ):
            vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( surf, i ), "RotateMatchDideralFlag" ), 1.0 )
        xs = vsp.GetXSec( surf, 1 )
        for name, v in ( ( "Span", 2.0 ), ( "Root_Chord", 2.0 ), ( "Tip_Chord", 1.0 ), ( "Sweep", 30.0 ),
                         ( "Dihedral", dihedral ) ):
            vsp.SetParmVal( vsp.GetXSecParm( xs, name ), v )
    vsp.Update()

    vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 1 )

    capfd.readouterr()
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, 0 )
    out = capfd.readouterr().out

    assert "still cross" not in out
    assert "second triangulator" not in out
    assert "retried" not in out
