# A Conformal of a Fuselage or Stack whose end cap carries the body on from its end -- round, point, cone, ogive
# or profile -- is offset inside the parent's cap as well as its body.

import openvsp as vsp
import pytest

from testhelp import drop_errors


def cap_body( geom, cap, **parms ):
    vsp.VSPRenew()
    drop_errors()
    g = vsp.AddGeom( geom )
    vsp.Update()
    # A body that ends in sections with an area, so its caps have an end to stand on.
    surf = vsp.GetXSecSurf( g, 0 )
    vsp.CutXSec( g, vsp.GetNumXSec( surf ) - 1 )
    vsp.CutXSec( g, 0 )
    vsp.Update()
    vsp.SetParmVal( vsp.FindParm( g, "CapUMinOption", "EndCap" ), cap )
    for name, val in parms.items():
        vsp.SetParmVal( vsp.FindParm( g, name, "EndCap" ), val )
    vsp.Update()
    return g


def conformal( g, off ):
    c = vsp.AddGeom( "CONFORMAL", g )
    vsp.Update()
    vsp.SetParmVal( vsp.FindParm( c, "Offset", "Design" ), off )
    vsp.Update()
    return c


def worst_nose_error( g, c, off ):
    """The greatest difference from the offset of the Conformal's distance to its parent, over its front third."""
    x0 = vsp.GetGeomBBoxMin( g, 0, True ).x()
    x1 = vsp.GetGeomBBoxMax( g, 0, True ).x()
    worst = 0.0
    for i in range( 81 ):
        for j in range( 0, 41, 5 ):
            p = vsp.CompPnt01( c, 0, i / 80, j / 40 )
            if p.x() > x0 + ( x1 - x0 ) / 3.0:
                continue
            worst = max( worst, abs( vsp.ProjPnt01I( g, p )[0] - off ) )
    return worst


def testConformalOfTangentOgiveCapIsExact():
    # The Stack of OgiveConformal.vsp3: a blunt tangent ogive whose length is relative to its end.
    g = cap_body( "STACK", vsp.OGIVE_END_CAP, CapUMinLength=4.5, CapUMinRadius=0.25 )
    c = conformal( g, 0.1 )
    assert worst_nose_error( g, c, 0.1 ) < 1e-3


CASES = [
    ( "round", vsp.ROUND_END_CAP, {}, 0.05 ),
    ( "point", vsp.POINT_END_CAP, {}, 0.1 ),
    ( "cone", vsp.CONE_END_CAP, { "CapUMinRadius": 0.15 }, 0.05 ),
    ( "sharp cone", vsp.CONE_END_CAP, { "CapUMinRadius": 0.0 }, 0.1 ),
    ( "truncated cone", vsp.CONE_END_CAP, { "CapUMinRadius": 0.15, "CapUMinTruncFlag": 1 }, 0.1 ),
    ( "ogive", vsp.OGIVE_END_CAP, { "CapUMinRadius": 0.15 }, 0.05 ),
    ( "sharp ogive", vsp.OGIVE_END_CAP, { "CapUMinRadius": 0.0 }, 0.1 ),
    ( "secant ogive", vsp.OGIVE_END_CAP, { "CapUMinRadius": 0.15, "CapUMinOgiveAngle": 10.0 }, 0.05 ),
    ( "power law", vsp.POWER_LAW_END_CAP, {}, 0.25 ),
    ( "von karman", vsp.VON_KARMAN_END_CAP, {}, 0.25 ),
    ( "sears-haack", vsp.SEARS_HAACK_END_CAP, {}, 0.25 ),
    ( "flat", vsp.FLAT_END_CAP, {}, 0.05 ),
]


@pytest.mark.parametrize( "geom", [ "FUSELAGE", "STACK" ] )
@pytest.mark.parametrize( "name,cap,parms,frac", CASES, ids=[ c[0] for c in CASES ] )
def testConformalCapIsOffsetInsideItsParent( geom, name, cap, parms, frac ):
    # frac is the error allowed as a fraction of the offset: exact for the cone and ogive caps but for the
    # cubics' fit, near for the profile caps, whose offsets are not of their families.
    p = dict( CapUMinLength=1.5 )
    p.update( parms )
    g = cap_body( geom, cap, **p )
    c = conformal( g, 0.05 )
    assert worst_nose_error( g, c, 0.05 ) < frac * 0.05


def trimmed( before, umin, umax, maxcap ):
    g = cap_body( "STACK", vsp.OGIVE_END_CAP, CapUMinLength=4.5, CapUMinRadius=0.25 )
    c = conformal( g, 0.1 )
    for name, val in ( ( "UTrimFlag", 1 ), ( "CapBeforeTrimFlag", before ), ( "UTrimMin", umin ),
                       ( "UTrimMax", umax ) ):
        vsp.SetParmVal( vsp.FindParm( c, name, "Design" ), val )
    vsp.SetParmVal( vsp.FindParm( c, "CapUMaxTrimOption", "EndCap" ), maxcap )
    vsp.Update()
    return g, c


def testTrimmedConformalOfTheBodyLosesItsNose():
    # Trimmed before it is capped, a Conformal is a piece of the body, its trimmed ends capped by the trim caps.
    g, c = trimmed( 0, 0.0, 0.3, vsp.FLAT_END_CAP )
    assert vsp.GetGeomBBoxMin( c, 0, True ).x() > vsp.GetGeomBBoxMin( g, 0, True ).x() + 1.0


def testConformalTrimsItsCappedSurface():
    # Capped as its parent first, the trim can keep the nose, offset inside the parent's, and cap the cut.
    g, flat = trimmed( 1, 0.0, 0.3, vsp.FLAT_END_CAP )
    tip = vsp.CompPnt01( flat, 0, 0.0, 0.0 )
    assert vsp.ProjPnt01I( g, tip )[0] == pytest.approx( 0.1, abs=1e-3 )
    flat_end = vsp.GetGeomBBoxMax( flat, 0, True ).x()

    # A round trim cap on the cut reaches past where the flat one stops.
    g, round_cap = trimmed( 1, 0.0, 0.3, vsp.ROUND_END_CAP )
    assert vsp.GetGeomBBoxMax( round_cap, 0, True ).x() > flat_end + 0.5


def testConformalOfMdcBodIsOffsetAllOver():
    # The DATCOM body's nose is an ogive cap with scaled rows and its base a point cap of no length.
    vsp.VSPRenew()
    drop_errors()
    g = vsp.AddGeom( "MDC_BOD" )
    vsp.Update()
    c = conformal( g, 0.05 )
    worst = 0.0
    for i in range( 41 ):
        for j in range( 0, 41, 4 ):
            p = vsp.CompPnt01( c, 0, i / 40, j / 40 )
            worst = max( worst, abs( vsp.ProjPnt01I( g, p )[0] - 0.05 ) )
    assert worst < 1e-3


@pytest.mark.parametrize( "sectyp", [ 0, 1, 2 ] )
def testConformalOfMdcFinIsItsWingsConformal( sectyp ):
    # A fin set's Conformal is offset as a Wing's is, faces out as its parent does, and is the Conformal of the Wing
    # the fin set converts to.
    vsp.VSPRenew()
    drop_errors()
    fid = vsp.AddGeom( "MDC_FIN" )
    vsp.Update()
    vsp.SetParmVal( vsp.FindParm( fid, "SECTYP", "Design" ), sectyp )
    vsp.Update()
    wings = vsp.ConvertMdcFinToWing( fid, False )
    vsp.Update()
    cf = conformal( fid, 0.01 )
    cw = conformal( wings[0], 0.01 )
    worst = 0.0
    for i in range( 21 ):
        for j in range( 41 ):
            p = vsp.CompPnt01( cf, 0, i / 20, j / 40 )
            worst = max( worst, vsp.ProjPnt01I( cw, p )[0] )
    assert worst < 1e-9

    for j in ( 0.1, 0.4, 0.6, 0.9 ):
        p = vsp.CompPnt01( cf, 0, 0.5, j )
        d, s, u, w = vsp.ProjPnt01I( fid, p )
        assert vsp.dot( vsp.CompNorm01( cf, 0, 0.5, j ), vsp.CompNorm01( fid, s, u, w ) ) > 0.9
