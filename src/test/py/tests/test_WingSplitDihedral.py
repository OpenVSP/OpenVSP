"""Splitting a wing section keeps the wing's shape, whichever way dihedral is given."""

import openvsp as vsp


def test_split_keeps_the_tip_with_relative_or_absolute_dihedral():
    for relative in ( 0.0, 1.0 ):
        vsp.VSPRenew()
        wid = vsp.AddGeom( "WING" )
        vsp.SetParmVal( wid, "RelativeDihedralFlag", "WingGeom", relative )
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 ), "Dihedral" ), 10.0 )
        vsp.Update()
        before = vsp.CompPnt01( wid, 0, 1.0, 0.5 )
        vsp.SplitWingXSec( wid, 1 )
        vsp.Update()
        after = vsp.CompPnt01( wid, 0, 1.0, 0.5 )
        assert vsp.dist( before, after ) < 1e-9, relative
