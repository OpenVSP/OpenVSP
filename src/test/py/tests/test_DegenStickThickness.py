# DegenStick toc2 and tLoc2, the thickness to chord and its location found on the section curve.

import openvsp as vsp


def testThicknessIsToTheSectionsOwnChord():
    """The curve shrinks with the chord toward a point tip; toc2 and tLoc2 were in its units."""
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0 )
    vsp.ChangeXSecShape( vsp.GetXSecSurf( w, 0 ), 1, vsp.XS_POINT )
    vsp.Update()
    vsp.ComputeDegenGeom( vsp.SET_ALL, vsp.DEGEN_GEOM_CSV_TYPE )
    sid = vsp.FindLatestResultsID( "Degen_stick" )
    chord = vsp.GetDoubleResults( sid, "chord" )
    toc2 = vsp.GetDoubleResults( sid, "toc2" )
    tloc2 = vsp.GetDoubleResults( sid, "tLoc2" )
    for c, t, x in zip( chord, toc2, tloc2 ):
        if c > 1e-6:
            assert abs( t - 0.1 ) < 1e-3, list( toc2 )
            assert abs( x - 0.3 ) < 1e-2, list( tloc2 )


def testAPointMidSpanGivesEachPanelItsNeighbour():
    """Each panel's sections shrink toward the point as copies of the section at its other end."""
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0 )
    vsp.InsertXSec( w, 1, vsp.XS_FOUR_SERIES )
    xs = vsp.GetXSecSurf( w, 0 )
    vsp.ChangeXSecShape( xs, 0, vsp.XS_SIX_SERIES )
    vsp.ChangeXSecShape( xs, 1, vsp.XS_POINT )
    tip = vsp.GetXSec( xs, 2 )
    vsp.SetParmVal( vsp.GetXSecParm( tip, "ThickChord" ), 0.16 )
    vsp.Update()
    vsp.ComputeDegenGeom( vsp.SET_ALL, vsp.DEGEN_GEOM_CSV_TYPE )
    sid = vsp.FindLatestResultsID( "Degen_stick" )
    chord = vsp.GetDoubleResults( sid, "chord" )
    toc2 = vsp.GetDoubleResults( sid, "toc2" )
    tloc2 = vsp.GetDoubleResults( sid, "tLoc2" )
    point = [ i for i, c in enumerate( chord ) if c < 1e-6 ]
    assert len( point ) == 1, list( chord )
    for i, ( t, x ) in enumerate( zip( toc2, tloc2 ) ):
        if i < point[0]:
            assert abs( t - toc2[0] ) < 1e-6 and abs( x - tloc2[0] ) < 1e-6, ( list( toc2 ), list( tloc2 ) )
        elif i > point[0]:
            assert abs( t - toc2[-1] ) < 1e-6 and abs( x - tloc2[-1] ) < 1e-6, ( list( toc2 ), list( tloc2 ) )
    assert abs( toc2[0] - 0.10 ) < 1e-3 and abs( toc2[-1] - 0.16 ) < 1e-3, list( toc2 )
