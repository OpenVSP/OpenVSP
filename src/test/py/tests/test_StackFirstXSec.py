# A Stack's first cross section is placed and rotated like any other.

import openvsp as vsp
import pytest

N = 20


def grid( gid ):
    return [ vsp.CompPnt01( gid, 0, i / N, j / N ) for i in range( N + 1 ) for j in range( N + 1 ) ]


def xsec( gid, i ):
    return vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), i )


def xp( gid, i, name, val ):
    vsp.SetParmVal( vsp.GetXSecParm( xsec( gid, i ), name ), val )


def xg( gid, i, name ):
    return vsp.GetParmVal( vsp.GetXSecParm( xsec( gid, i ), name ) )


def testMovingTheFirstSectionCarriesTheBody():
    """Everything after the first section is placed relative to it, so moving and rotating it
    moves the whole body rigidly."""
    vsp.VSPRenew()
    sid = vsp.AddGeom( "STACK" )
    vsp.Update()
    before = grid( sid )

    xp( sid, 0, "XDelta", 1.0 )
    xp( sid, 0, "ZDelta", 0.5 )
    xp( sid, 0, "YRotate", 10.0 )
    vsp.Update()

    mat = vsp.Matrix4d()
    mat.loadIdentity()
    mat.translatef( 1.0, 0.0, 0.5 )
    mat.rotateY( 10.0 )
    assert max( vsp.dist( mat.xform( p ), q ) for p, q in zip( before, grid( sid ) ) ) < 1e-12

    # With nothing before it, its relative and absolute placement agree.
    assert xg( sid, 0, "XAbs" ) == pytest.approx( 1.0 )
    assert xg( sid, 0, "ZAbs" ) == pytest.approx( 0.5 )
    assert xg( sid, 0, "YRotateAbs" ) == pytest.approx( 10.0 )


def testTheFirstSectionCanBePlacedAbsolutely():
    vsp.VSPRenew()
    sid = vsp.AddGeom( "STACK" )
    vsp.Update()
    for i in range( 5 ):
        xp( sid, i, "XSAbsRelFlag", vsp.ABS )
    second = xg( sid, 1, "XAbs" )

    xp( sid, 0, "XAbs", -2.0 )
    xp( sid, 0, "XRotateAbs", 15.0 )
    vsp.Update()

    assert xg( sid, 0, "XAbs" ) == pytest.approx( -2.0 )
    assert xg( sid, 0, "XRotateAbs" ) == pytest.approx( 15.0 )
    assert xg( sid, 1, "XAbs" ) == pytest.approx( second ), "an absolute section should stay put"


def testALoopClosesOntoAMovedFirstSection():
    vsp.VSPRenew()
    sid = vsp.AddGeom( "STACK" )
    vsp.SetParmVal( sid, "OrderPolicy", "Design", 1 )  # STACK_LOOP
    vsp.Update()
    xss = vsp.GetXSecSurf( sid, 0 )
    for i in range( 5 ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_CIRCLE )
    vsp.Update()

    xp( sid, 0, "YDelta", 0.3 )
    xp( sid, 0, "XRotate", 4.0 )
    xp( sid, 0, "ZRotate", 7.0 )
    vsp.Update()

    last = vsp.GetNumXSec( xss ) - 1
    for name in ( "XAbs", "YAbs", "ZAbs", "XRotateAbs", "YRotateAbs", "ZRotateAbs" ):
        assert xg( sid, last, name ) == pytest.approx( xg( sid, 0, name ), abs=1e-9 ), name

    gap = max( vsp.dist( vsp.CompPnt01( sid, 0, 0.0, j / N ), vsp.CompPnt01( sid, 0, 1.0, j / N ) ) for j in range( N + 1 ) )
    assert gap < 1e-12


def testALoopClosesInAbsoluteModeOntoAMovedFirstSection():
    """With every section placed absolutely the last one's absolute placement is what closes the
    loop, so it is that which has to follow the first section."""
    vsp.VSPRenew()
    sid = vsp.AddGeom( "STACK" )
    vsp.SetParmVal( sid, "OrderPolicy", "Design", 1 )  # STACK_LOOP
    vsp.Update()
    xss = vsp.GetXSecSurf( sid, 0 )
    for i in range( 5 ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_CIRCLE )
    vsp.Update()
    for i in range( 5 ):
        xp( sid, i, "XSAbsRelFlag", vsp.ABS )
    vsp.Update()

    xp( sid, 0, "XAbs", 0.3 )
    xp( sid, 0, "YAbs", -0.2 )
    xp( sid, 0, "XRotateAbs", 25.0 )
    xp( sid, 0, "ZRotateAbs", 45.0 )
    vsp.Update()

    last = vsp.GetNumXSec( xss ) - 1
    for name in ( "XAbs", "YAbs", "ZAbs", "XRotateAbs", "YRotateAbs", "ZRotateAbs" ):
        assert xg( sid, last, name ) == pytest.approx( xg( sid, 0, name ), abs=1e-9 ), name

    gap = max( vsp.dist( vsp.CompPnt01( sid, 0, 0.0, j / N ), vsp.CompPnt01( sid, 0, 1.0, j / N ) ) for j in range( N + 1 ) )
    assert gap < 1e-12


def testDefaultSourcesMeasureTheBodyFromItsFirstSection():
    """The default CFD sources are sized from the body's length, which runs from the first
    section, not from the Stack's origin."""
    def sources( first_delta ):
        vsp.VSPRenew()
        sid = vsp.AddGeom( "STACK" )
        xp( sid, 0, "XDelta", first_delta )
        vsp.Update()
        vsp.AddDefaultSources()
        vsp.Update()
        return [ vsp.GetParmVal( p ) for i in range( vsp.GetNumCFDSources( sid ) )
                 for p in vsp.FindContainerParmIDs( vsp.GetCFDSourceID( sid, i ) ) ]

    at_origin = sources( 0.0 )
    assert at_origin
    assert sources( 50.0 ) == pytest.approx( at_origin )
