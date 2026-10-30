# The biconvex airfoil: on each side a parabola from the leading edge up to its highest point, at ThickLoc above and
# ThickLocLow below, and another down to the trailing edge, or circular arcs with UseCircleArc; ThickFracUp of
# ThickChord above the chord line.

import math

import openvsp as vsp
import pytest


def wing_section( t, frac, loc=0.5, loclow=0.5, n=400, circle=False ):
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    surf = vsp.GetXSecSurf( wid, 0 )
    vsp.ChangeXSecShape( surf, 0, vsp.XS_BICONVEX )
    vsp.ChangeXSecShape( surf, 1, vsp.XS_BICONVEX )
    for i in range( 2 ):
        xs = vsp.GetXSec( surf, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickChord" ), t )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickFracUp" ), frac )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickLoc" ), loc )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickLocLow" ), loclow )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "UseCircleArc" ), circle )
    vsp.Update()
    # Mid span: both sections are the same, and u = 0 is the root cap.
    pts = [ vsp.CompPnt01( wid, 0, 0.5, j / n ) for j in range( n + 1 ) ]
    xs = [ p.x() for p in pts ]
    chord = max( xs ) - min( xs )
    return chord, max( p.z() for p in pts ), min( p.z() for p in pts )


def peaks( loc, loclow, circle ):
    """The chord fractions of the highest and lowest points of a section, and their heights over the chord."""
    vsp.VSPRenew()
    chord, zhi, zlo = wing_section( 0.1, 0.5, loc, loclow, circle=circle )
    wid = vsp.FindGeoms()[0]
    pts = [ vsp.CompPnt01( wid, 0, 0.5, j / 4000 ) for j in range( 4001 ) ]
    x0 = min( p.x() for p in pts )
    hi = max( pts, key=lambda p: p.z() )
    lo = min( pts, key=lambda p: p.z() )
    return ( hi.x() - x0 ) / chord, ( lo.x() - x0 ) / chord, zhi / chord, zlo / chord


@pytest.mark.parametrize( "circle", [ False, True ] )
@pytest.mark.parametrize( "frac", [ 0.0, 0.25, 0.5, 0.7, 1.0 ] )
def testBiconvexSplitsItsThicknessAboveAndBelowTheChord( frac, circle ):
    chord, zhi, zlo = wing_section( 0.1, frac, circle=circle )
    assert zhi == pytest.approx( frac * 0.1 * chord, abs=1e-6 * chord )
    assert zlo == pytest.approx( ( frac - 1.0 ) * 0.1 * chord, abs=1e-6 * chord )


def testBiconvexIsSymmetricByDefault():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    surf = vsp.GetXSecSurf( wid, 0 )
    vsp.ChangeXSecShape( surf, 0, vsp.XS_BICONVEX )
    assert vsp.GetParmVal( vsp.GetXSecParm( vsp.GetXSec( surf, 0 ), "ThickFracUp" ) ) == 0.5
    assert vsp.GetParmVal( vsp.GetXSecParm( vsp.GetXSec( surf, 0 ), "UseCircleArc" ) ) == 0


@pytest.mark.parametrize( "circle", [ False, True ] )
@pytest.mark.parametrize( "loc,loclow", [ ( 0.5, 0.5 ), ( 0.3, 0.7 ), ( 0.2, 0.2 ), ( 0.8, 0.4 ) ] )
def testBiconvexPeaksAtItsThicknessLocations( loc, loclow, circle ):
    xhi, xlo, zhi, zlo = peaks( loc, loclow, circle )
    assert xhi == pytest.approx( loc, abs=0.002 )
    assert xlo == pytest.approx( loclow, abs=0.002 )
    assert zhi == pytest.approx( 0.05, abs=1e-6 )
    assert zlo == pytest.approx( -0.05, abs=1e-6 )


@pytest.mark.parametrize( "loc", [ 0.0, 1.0 ] )
def testBiconvexThicknessAtAnEdgeIsFinite( loc ):
    chord, zhi, zlo = wing_section( 0.1, 0.5, loc, loc )
    assert zhi == pytest.approx( 0.05 * chord, abs=1e-6 * chord )
    assert zlo == pytest.approx( -0.05 * chord, abs=1e-6 * chord )


@pytest.mark.parametrize( "loc", [ 0.0, 1.0 ] )
def testBiconvexPeakAtAnEdgeIsASemicircle( loc ):
    # An arc level at a peak on the edge is a half circle h / 2 in radius, which reaches h / 2 beyond the edge.
    chord, zhi, zlo = wing_section( 0.1, 0.5, loc, loc, circle=True )
    true_chord = chord / 1.025
    assert zhi == pytest.approx( 0.05 * true_chord, abs=1e-5 * true_chord )
    assert zlo == pytest.approx( -0.05 * true_chord, abs=1e-5 * true_chord )


@pytest.mark.parametrize( "loc", [ 0.5, 0.3, 0.8 ] )
def testBiconvexSidesAreCircularArcs( loc ):
    # Each arc is level at the peak, h high at loc, so its circle's radius is ( d^2 + h^2 ) / 2h for its run d.
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    surf = vsp.GetXSecSurf( wid, 0 )
    vsp.ChangeXSecShape( surf, 1, vsp.XS_BICONVEX )
    xs = vsp.GetXSec( surf, 1 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickChord" ), 0.1 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickLoc" ), loc )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "UseCircleArc" ), 1 )
    vsp.Update()
    le = vsp.ComputeXSecPnt( xs, 0.5 )
    chord = vsp.ComputeXSecPnt( xs, 0.0 ).x() - le.x()
    h = 0.05
    for j in range( 2001 ):
        p = vsp.ComputeXSecPnt( xs, 0.5 + 0.5 * j / 2000 )
        x = ( p.x() - le.x() ) / chord
        z = ( p.z() - le.z() ) / chord
        d = loc
        if x > loc:
            d = 1.0 - loc
        r = ( d * d + h * h ) / ( 2.0 * h )
        assert math.hypot( x - loc, z - ( h - r ) ) == pytest.approx( r, abs=1e-5 )


@pytest.mark.parametrize( "loc", [ 0.5, 0.3, 0.8 ] )
def testBiconvexSidesAreParabolas( loc ):
    # Each parabola is level at the peak, h high at loc: z = h ( 1 - ( ( x - loc ) / d )^2 ) for its run d.  At loc
    # 0.5 that is z = 2 t x ( 1 - x ).
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    surf = vsp.GetXSecSurf( wid, 0 )
    vsp.ChangeXSecShape( surf, 1, vsp.XS_BICONVEX )
    xs = vsp.GetXSec( surf, 1 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickChord" ), 0.1 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickLoc" ), loc )
    vsp.Update()
    le = vsp.ComputeXSecPnt( xs, 0.5 )
    chord = vsp.ComputeXSecPnt( xs, 0.0 ).x() - le.x()
    h = 0.05
    for j in range( 2001 ):
        p = vsp.ComputeXSecPnt( xs, 0.5 + 0.5 * j / 2000 )
        x = ( p.x() - le.x() ) / chord
        z = ( p.z() - le.z() ) / chord
        d = loc
        if x > loc:
            d = 1.0 - loc
        assert z == pytest.approx( h * ( 1.0 - ( ( x - loc ) / d ) ** 2 ), abs=1e-7 )
