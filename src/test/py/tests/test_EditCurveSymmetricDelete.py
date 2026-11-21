# With R-L Symmetry on, an edit curve's left half follows its right: a point and its mirror are deleted together,
# and the Bottom, Left and Top points stay.

import openvsp as vsp


def symmetric_circle():
    vsp.VSPRenew()
    gid = vsp.AddGeom( "STACK" )
    surf = vsp.GetXSecSurf( gid, 0 )
    vsp.ChangeXSecShape( surf, 1, vsp.XS_EDIT_CURVE )
    xs = vsp.GetXSec( surf, 1 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "SymType" ), vsp.SYM_RL )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "ShapeType" ), vsp.EDIT_XSEC_CIRCLE )
    vsp.EditXSecInitShape( xs )
    vsp.Update()
    return surf, vsp.GetXSec( surf, 1 )


def split_right( surf, xs ):
    """A point at U 0.1 on the right and its mirror at U 0.4 on the left."""
    vsp.EditXSecSplit01( xs, 0.1 )
    vsp.Update()
    xs = vsp.GetXSec( surf, 1 )
    u = vsp.GetEditXSecUVec( xs )
    right = [ i for i in range( 0, len( u ), 3 ) if abs( u[i] - 0.1 ) < 1e-9 ][0]
    left = [ i for i in range( 0, len( u ), 3 ) if abs( u[i] - 0.4 ) < 1e-9 ][0]
    return xs, len( u ), right, left


def testAPointAndItsMirrorAreDeletedTogether():
    for side in ( 0, 1 ):
        surf, xs = symmetric_circle()
        xs, n, right, left = split_right( surf, xs )
        vsp.EditXSecDelPnt( xs, ( right, left )[side] )
        vsp.Update()
        u = vsp.GetEditXSecUVec( xs )
        assert len( u ) == n - 6
        assert all( abs( v - 0.1 ) > 1e-6 and abs( v - 0.4 ) > 1e-6 for v in u )


def testTheBottomLeftAndTopPointsStay():
    for pick in ( 3, 6, 9 ):
        surf, xs = symmetric_circle()
        pid = vsp.GetXSecParm( xs, "X_%d" % pick )
        vsp.EditXSecDelPnt( xs, pick )
        vsp.Update()
        assert len( vsp.GetEditXSecUVec( xs ) ) == 13
        assert vsp.GetXSecParm( xs, "X_%d" % pick ) == pid
