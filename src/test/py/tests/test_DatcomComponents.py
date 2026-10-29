# The DATCOM components, laid out as DATCOM lays them out.
#
# Each takes the namelist's inputs under the namelist's names and has the surfaces the namelist describes.  These
# tests hold each to what its namelist says: the stations a body is lofted through, the surfaces a protuberance
# set, inlet set or fin set is made of, the Parms that follow others, and that every component saves and reads
# back to the same surfaces.

import math
import os
import tempfile

import openvsp as vsp
import pytest

from testhelp import drop_errors, pop_errors, assert_no_errors


def new( name, **parms ):
    vsp.VSPRenew()
    drop_errors()
    assert name in vsp.GetGeomTypes()
    gid = vsp.AddGeom( name )
    assert gid
    put( gid, **parms )
    return gid


# The Parms that set how many Parms of a station, panel or member there are.
COUNTS = ( "PTYPE", "NLOC", "NIN", "NSPAN", "NPANEL" )


def put( gid, **parms ):
    # The Parms of each station, panel or member are made by the update after their count, so the counts go first.
    for name in COUNTS:
        if name in parms:
            vsp.SetParmVal( vsp.FindParm( gid, name, "Design" ), parms[name] )
    vsp.Update()
    for name, val in parms.items():
        if name in COUNTS:
            continue
        pid = vsp.FindParm( gid, name, "Design" )
        assert pid, "no Parm " + name
        vsp.SetParmVal( pid, val )
    vsp.Update()


def place( gid, x ):
    vsp.SetParmVal( vsp.FindParm( gid, "X_Rel_Location", "XForm" ), x )
    vsp.Update()


def get( gid, name ):
    return vsp.GetParmVal( vsp.FindParm( gid, name, "Design" ) )


def pnt( gid, isurf, u, w ):
    p = vsp.CompPnt01( gid, isurf, u, w )
    return ( p.x(), p.y(), p.z() )


def sample( gid, nu=9, nw=17 ):
    pts = []
    for s in range( vsp.GetNumMainSurfs( gid ) ):
        for i in range( nu ):
            for j in range( nw ):
                pts.append( pnt( gid, s, i / ( nu - 1 ), j / ( nw - 1 ) ) )
    return pts


def assert_same( a, b, tol=1e-9 ):
    assert len( a ) == len( b )
    for p, q in zip( a, b ):
        assert p == pytest.approx( q, abs=tol )


def extent( gid, axis ):
    lo = [ vsp.GetGeomBBoxMin( gid, i ) for i in range( vsp.GetNumMainSurfs( gid ) ) ]
    hi = [ vsp.GetGeomBBoxMax( gid, i ) for i in range( vsp.GetNumMainSurfs( gid ) ) ]
    comp = ( lambda v: v.x(), lambda v: v.y(), lambda v: v.z() )[axis]
    return min( comp( v ) for v in lo ), max( comp( v ) for v in hi )


def roundtrip( gid ):
    path = os.path.join( tempfile.mkdtemp(), "datcom.vsp3" )
    vsp.WriteVSPFile( path, vsp.SET_ALL )
    vsp.VSPRenew()
    drop_errors()
    vsp.ReadVSPFile( path )
    ids = vsp.FindGeoms()
    assert len( ids ) == 1
    return ids[0]


#===== MDC_BOD =====#

def testBodyIsLoftedThroughItsStations():
    """The nose tip is at the origin, the base of the nose LNOSE aft, the centerbody LCENTR beyond it and the aft body
    LAFT.  XO is the X location."""
    gid = new( "MDC_BOD", LNOSE=2.0, LCENTR=5.0, LAFT=3.0, DAFT=0.5 )
    place( gid, 1.0 )
    lo, hi = extent( gid, 0 )
    assert lo == pytest.approx( 1.0, abs=1e-6 )
    assert hi == pytest.approx( 11.0, abs=1e-6 )

    # The widest the body gets is the nose's and centerbody's diameter.
    ylo, yhi = extent( gid, 1 )
    assert yhi - ylo == pytest.approx( 1.0, abs=1e-6 )


def testBodyOfAxibodIsRoundAndOfEllbodIsAnEllipse():
    gid = new( "MDC_BOD", NAMELIST=1, WNOSE=2.0, ENOSE=0.5, WCENTR=2.0, ECENTR=0.5, LAFT=0.0 )
    ylo, yhi = extent( gid, 1 )
    zlo, zhi = extent( gid, 2 )
    assert yhi - ylo == pytest.approx( 2.0, abs=1e-6 )
    assert zhi - zlo == pytest.approx( 1.0, abs=1e-6 )


@pytest.mark.parametrize( "start,sizes,other", [
    ( 0, dict( DNOSE=2.0, CENTRDefault=0, DCENTR=2.5, LAFT=2.0, DAFT=1.5 ), ( "WNOSE", "WCENTR", "WAFT" ) ),
    ( 1, dict( WNOSE=2.0, CENTRDefault=0, WCENTR=2.5, LAFT=2.0, WAFT=1.5 ), ( "DNOSE", "DCENTR", "DAFT" ) ) ] )
def testBodyChangesNamelistKeepingItsSize( start, sizes, other ):
    gid = new( "MDC_BOD", NAMELIST=start, **sizes )
    assert [ get( gid, p ) for p in other ] == pytest.approx( [ 2.0, 2.5, 1.5 ] )
    ylo, yhi = extent( gid, 1 )
    put( gid, NAMELIST=1 - start )
    assert extent( gid, 1 ) == pytest.approx( ( ylo, yhi ), abs=1e-9 )


def testBodyCenterbodyDiameterFollowsTheNoseUntilItIsGiven():
    gid = new( "MDC_BOD", DNOSE=2.0, DCENTR=3.0 )
    assert get( gid, "DCENTR" ) == pytest.approx( 2.0 )

    # Without the default the diameter is the input.
    put( gid, CENTRDefault=0, DCENTR=1.0 )
    assert get( gid, "DCENTR" ) == pytest.approx( 1.0 )
    ylo, yhi = extent( gid, 1 )
    assert yhi - ylo == pytest.approx( 2.0, abs=1e-6 )


def testBodyCamberRaisesItsStations():
    plain = new( "MDC_BOD" )
    zlo, zhi = extent( plain, 2 )
    gid = new( "MDC_BOD", ZTIP=0.5, ZNOSE=0.5, ZCENTR=0.5 )
    clo, chi = extent( gid, 2 )
    assert clo - zlo == pytest.approx( 0.5, abs=1e-6 )
    assert chi - zhi == pytest.approx( 0.5, abs=1e-6 )


@pytest.mark.parametrize( "tnose", range( 5 ) )
def testBodyNoseShapesAllBuildAClosedNose( tnose ):
    gid = new( "MDC_BOD", TNOSE=tnose, POWER=0.7 )
    tip = pnt( gid, 0, 0.0, 0.0 )
    assert tip[1] == pytest.approx( 0.0, abs=1e-9 )
    assert tip[2] == pytest.approx( 0.0, abs=1e-9 )
    assert_no_errors()


def testBodyBluntNoseIsTallerNearItsTipThanASharpOne():
    """A nose blunted by BNOSE is a sphere's cap, so at the same distance from the tip it is fuller than a point."""
    def height_at( x, **parms ):
        gid = new( "MDC_BOD", NAMELIST=1, ENOSE=0.5, TNOSE=0, LCENTR=0.0, **parms )
        rows = []
        for i in range( 201 ):
            ring = [ pnt( gid, 0, i / 200.0, j / 32.0 ) for j in range( 33 ) ]
            rows.append( ( ring[0][0], max( p[2] for p in ring ) - min( p[2] for p in ring ) ) )

        # Linear in the two rows either side of x.
        for ( x0, h0 ), ( x1, h1 ) in zip( rows, rows[1:] ):
            if x0 <= x <= x1 and x1 > x0:
                return h0 + ( h1 - h0 ) * ( x - x0 ) / ( x1 - x0 )
        raise AssertionError( "no row at x" )

    assert height_at( 0.05, BNOSE=0.1 ) > 2.0 * height_at( 0.05, BNOSE=0.0 )


@pytest.mark.parametrize( "dexit,ring", [ ( 3.0, True ), ( 3.75, False ), ( 5.0, False ) ] )
def testBodyNozzleIsAConeInTheBaseAQuarterOfDexitDeep( dexit, ring ):
    # The base, 3.75 across at x = 37.5, is flat out to the exit, and the exit a cone to a point DEXIT / 4 inside it;
    # an exit no smaller than the base has no ring.
    gid = new( "MDC_BOD", LNOSE=11.25, DNOSE=3.75, LCENTR=26.25, DEXIT=dexit )
    d = min( dexit, 3.75 )
    assert vsp.GetParmVal( vsp.FindParm( gid, "CapUMaxOption", "EndCap" ) ) == vsp.NO_END_CAP
    tip = vsp.CompPnt01( gid, 0, 1.0, 0.0 )
    assert ( tip.x(), tip.y(), tip.z() ) == pytest.approx( ( 37.5 - 0.25 * d, 0.0, 0.0 ), abs=1e-9 )

    # Every point of the base is at x = 37.5 or on the cone, to the round of the exit drawn as cubic arcs.
    for j in range( 16 ):
        for i in range( 1, 40 ):
            p = vsp.CompPnt01( gid, 0, 1.0 - i / 400.0, j / 16 )
            r = math.hypot( p.y(), p.z() )
            if p.x() < 37.5 - 1e-9 and r < 0.5 * d - 1e-6:
                assert p.x() == pytest.approx( 37.5 - 0.25 * d * ( 1.0 - r / ( 0.5 * d ) ), abs=1e-4 )
    ring_pts = [ vsp.CompPnt01( gid, 0, 1.0 - i / 400.0, 0.1 ) for i in range( 1, 200 ) ]
    on_ring = [ p for p in ring_pts if abs( p.x() - 37.5 ) < 1e-9 and 0.5 * d + 1e-6 < math.hypot( p.y(), p.z() ) ]
    assert bool( on_ring ) == ring


def testBodyScaleScalesItsLengths():
    gid = new( "MDC_BOD", LAFT=2.0 )
    vsp.SetParmVal( vsp.FindParm( gid, "Scale", "XForm" ), 2.0 )
    vsp.Update()
    lo, hi = extent( gid, 0 )
    assert hi - lo == pytest.approx( 2.0 * 12.0, rel=1e-6 )


def testBodyRotatesAboutOriginAlongItsLengthAndKeepsItWhenReadBack():
    gid = new( "MDC_BOD", LAFT=2.0, DAFT=0.6, ZTIP=0.1, ZNOSE=0.05, ZCENTR=0.2 )
    for name, val in ( ( "Origin", 0.5 ), ( "Y_Rel_Rotation", 12.0 ), ( "X_Rel_Location", 1.0 ) ):
        vsp.SetParmVal( vsp.FindParm( gid, name, "XForm" ), val )
    vsp.Update()
    assert_same( sample( gid ), sample( roundtrip( gid ) ) )


