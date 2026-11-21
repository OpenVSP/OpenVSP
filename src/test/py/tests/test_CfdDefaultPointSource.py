"""A default point source on a point section part way along a component sits on that section."""

import openvsp as vsp


def test_a_point_section_part_way_along_gets_its_own_u( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fid, "OrderPolicy", "Design", vsp.FUSE_LOOP )
    vsp.Update()
    xs = vsp.GetXSecSurf( fid, 0 )
    nxsec = vsp.GetNumXSec( xs )
    # In a loop the front is the section with the smallest X, here the second one.
    vsp.ChangeXSecShape( xs, 1, vsp.XS_POINT )
    vsp.Update()

    vsp.DeleteAllCFDSources()
    vsp.AddDefaultSources()

    u = {}
    for sid in vsp.GetCFDSourceIDVec( fid ):
        names = { vsp.GetParmName( p ): p for p in vsp.FindContainerParmIDs( sid ) }
        if "U_Loc" in names:
            u[ vsp.GetContainerName( sid ) ] = vsp.GetParmVal( names[ "U_Loc" ] )

    assert abs( u[ "Def_PS_1" ] - 1.0 / ( nxsec - 1 ) ) < 1e-12
