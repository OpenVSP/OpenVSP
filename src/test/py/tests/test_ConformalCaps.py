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
