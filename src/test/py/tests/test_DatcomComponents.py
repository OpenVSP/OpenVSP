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


#===== MDC_PROTUB =====#

@pytest.mark.parametrize( "ptype,nsurf", [ ( 1, 1 ), ( 2, 1 ), ( 3, 4 ), ( 4, 1 ), ( 5, 1 ), ( 6, 1 ) ] )
def testProtuberanceTypesHaveTheirSurfaces( ptype, nsurf ):
    gid = new( "MDC_PROTUB", PTYPE=ptype )
    assert vsp.GetNumMainSurfs( gid ) == nsurf


def testProtuberanceSetHasNLocCopiesSpacedEvenly():
    gid = new( "MDC_PROTUB", PTYPE=5, NLOC=4 )
    assert vsp.GetNumMainSurfs( gid ) == 4
    for k in range( 4 ):
        assert get( gid, "PHIPRO_%d" % ( k + 1 ) ) == pytest.approx( 90.0 * k )


def testProtuberanceTakesItsRollsWhenTheyAreGiven():
    gid = new( "MDC_PROTUB", PTYPE=5, NLOC=2, PHIPROFlag=1, PHIPRO_1=30.0, PHIPRO_2=100.0 )
    assert get( gid, "PHIPRO_1" ) == pytest.approx( 30.0 )
    assert get( gid, "PHIPRO_2" ) == pytest.approx( 100.0 )


def testFairingIsHalfAnEllipsoidOnTheTopLine():
    # The DATCOM manual's FAIRING: the top half of an ellipsoid LPROT long and WPROT wide at its flat base, which is
    # on the body's top line, and HPROT high above it.
    gid = new( "MDC_PROTUB", PTYPE=6, RBODY=6.0, LPROT_1=5.0, WPROT_1=2.0, HPROT_1=1.0 )
    place( gid, 14.0 )
    pts = [ vsp.CompPnt01( gid, 0, i / 64, j / 64 ) for i in range( 65 ) for j in range( 65 ) ]
    assert min( p.x() for p in pts ) == pytest.approx( 11.5, abs=1e-6 )
    assert max( p.x() for p in pts ) == pytest.approx( 16.5, abs=1e-6 )
    assert max( abs( p.y() ) for p in pts ) == pytest.approx( 1.0, abs=1e-6 )
    assert max( p.z() for p in pts ) == pytest.approx( 7.0, abs=1e-6 )
    assert min( p.z() for p in pts ) == pytest.approx( 6.0, abs=1e-9 )

    # Every point is on the base's plane, inside the ellipsoid, or on the ellipsoid to within the round cap's fit.
    for p in pts:
        f = math.sqrt( ( ( p.x() - 14.0 ) / 2.5 ) ** 2 + p.y() ** 2 + ( p.z() - 6.0 ) ** 2 )
        if abs( p.z() - 6.0 ) > 1e-9:
            assert abs( f - 1.0 ) < 5e-4

    # The round cap's poles, where its lines across gather, are at the ends of its length.
    poles = set()
    for j in range( 161 ):
        col = [ vsp.CompPnt01( gid, 0, 0.5 + 0.5 * i / 40, j / 160 ) for i in range( 41 ) ]
        if max( vsp.dist( col[0], q ) for q in col ) < 1e-9:
            poles.add( ( round( col[0].x(), 6 ), round( col[0].y(), 6 ), round( col[0].z(), 6 ) ) )
    assert poles == { ( 11.5, 0.0, 6.0 ), ( 16.5, 0.0, 6.0 ) }


def testBlockStandsOnTheMoldLineAtXprot():
    gid = new( "MDC_PROTUB", PTYPE=5, RBODY=1.0, LPROT_1=0.4, WPROT_1=0.2, HPROT_1=0.3, OPROT_1=0.1 )
    place( gid, 3.0 )
    xlo, xhi = extent( gid, 0 )
    assert xlo == pytest.approx( 2.8, abs=1e-6 )
    assert xhi == pytest.approx( 3.2, abs=1e-6 )
    zlo, zhi = extent( gid, 2 )
    assert zhi == pytest.approx( 1.0 + 0.1 + 0.3, abs=1e-6 )

    # Raised OPROT above the mold line it stands on nothing, and with no offset it stands on the body's top line.
    assert zlo == pytest.approx( 1.0 + 0.1, abs=1e-6 )
    put( gid, OPROT_1=0.0 )
    zlo, zhi = extent( gid, 2 )
    assert zlo == pytest.approx( 1.0, abs=1e-9 )
    assert zhi == pytest.approx( 1.0 + 0.3, abs=1e-6 )


# VCYL, BLOCK and FAIRING, 0.2 wide and 0.3 high on a body of radius 1.
@pytest.mark.parametrize( "ptype", ( 1, 5, 6 ) )
def testProtuberanceTouchesTheTopLineUnlessItsRootIsBuried( ptype ):
    gid = new( "MDC_PROTUB", PTYPE=ptype, RBODY=1.0, LPROT_1=0.4, WPROT_1=0.2, HPROT_1=0.3, OPROT_1=0.0 )
    zlo, zhi = extent( gid, 2 )
    assert zlo == pytest.approx( 1.0, abs=1e-9 )
    assert zhi == pytest.approx( 1.3, abs=1e-6 )

    # Buried, it reaches below where the mold line passes under its sides by a tenth of its height; its top stays.
    put( gid, BuryRoot=1 )
    zlo, zhi = extent( gid, 2 )
    assert zlo == pytest.approx( math.sqrt( 1.0 - 0.1 ** 2 ) - 0.03, abs=1e-6 )
    assert zhi == pytest.approx( 1.3, abs=1e-6 )


def testBuriedProtuberanceIsInsideANose():
    # A BLOCK 1 long on a cone nose growing 0.25 in radius per unit: its front end is over the narrower body.
    body = new( "MDC_BOD", TNOSE=0, LNOSE=4.0, DNOSE=2.0, LCENTR=6.0 )
    gid = child( "MDC_PROTUB", body, 2.0, PTYPE=5, LPROT_1=1.0, WPROT_1=0.2, HPROT_1=0.3, OPROT_1=0.0, BuryRoot=1 )
    zlo, zhi = extent( gid, 2 )
    assert zlo == pytest.approx( math.sqrt( 0.375 ** 2 - 0.1 ** 2 ) - 0.03, abs=1e-6 )


def testBuriedProtuberanceCornersAreInsideAnEllipticBody():
    # On the side of a body 2 wide and 1 high, which curves more tightly there than a circle of its radius.
    body = new( "MDC_BOD", NAMELIST=1, WNOSE=2.0, ENOSE=0.5, CENTRDefault=1, ECENTR=0.5, LNOSE=2.0, LCENTR=8.0 )
    gid = child( "MDC_PROTUB", body, 6.0, PTYPE=5, NLOC=1, PHIPROFlag=1, PHIPRO_1=90.0, LPROT_1=0.4, WPROT_1=0.6,
                 HPROT_1=0.3, OPROT_1=0.0, BuryRoot=1 )
    ylo = vsp.GetGeomBBoxMin( gid, 0 ).y()
    # The body passes under the sides, 0.3 either side of the middle, at y = 0.8.
    assert ylo == pytest.approx( 0.8 - 0.03, abs=1e-6 )


def testRaisedProtuberanceIsNotBuried():
    gid = new( "MDC_PROTUB", PTYPE=5, RBODY=1.0, LPROT_1=0.4, WPROT_1=0.2, HPROT_1=0.3, OPROT_1=0.1, BuryRoot=1 )
    zlo, zhi = extent( gid, 2 )
    assert zlo == pytest.approx( 1.1, abs=1e-6 )


# Each preset's type, the radius of the body it is for, and each member's LPROT, WPROT, HPROT and OPROT.
PRESETS = {
    1: ( 1, 6.0, ( ( 1.0, 1.0, 0.5, 0.0 ), ) ),
    2: ( 2, 6.0, ( ( 3.0, 1.0, 1.0, 0.0 ), ) ),
    3: ( 3, 6.0, ( ( 3.0, 2.0, 0.1, 0.0 ), ( 1.5, 0.25, 0.5, 0.1 ), ( 1.5, 0.25, 0.5, 0.1 ), ( 1.5, 1.0, 0.15, 0.6 ) ) ),
    4: ( 4, 6.0, ( ( 10.0, 4.0, 0.1, 0.0 ), ( 10.0, 0.25, 0.75, 0.1 ), ( 10.0, 1.0, 0.25, 0.85 ) ) ),
    5: ( 5, 6.0, ( ( 0.5, 2.0, 1.5, 0.0 ), ) ),
    6: ( 6, 6.0, ( ( 5.0, 2.0, 1.0, 0.0 ), ) ),
    7: ( 3, 5.0, ( ( 1.0, 1.0, 0.3, 0.0 ), ( 0.422, 0.155, 0.75, 0.3 ), ( 0.422, 0.155, 1.177, 0.3 ),
                   ( 0.422, 1.0, 0.48, 1.05 ) ) ),
    8: ( 3, 5.0, ( ( 1.74, 1.74, 0.1, 0.0 ), ( 0.422, 0.5, 1.0, 0.1 ), ( 0.422, 0.5, 1.0, 0.1 ),
                   ( 0.422, 1.74, 0.55, 1.1 ) ) ),
    9: ( 3, 5.0, ( ( 2.3, 2.3, 0.1, 0.0 ), ( 0.7, 0.5, 1.5, 0.1 ), ( 0.7, 0.5, 1.5, 0.1 ), ( 0.7, 2.3, 0.77, 1.6 ) ) ),
}


def member_sizes( gid, n ):
    return [ tuple( get( gid, nm + str( m ) ) for nm in ( "LPROT_", "WPROT_", "HPROT_", "OPROT_" ) )
             for m in range( 1, n + 1 ) ]


@pytest.mark.parametrize( "preset", sorted( PRESETS ) )
def testProtuberancePresetIsItsTypeSizedToRbody( preset ):
    ptype, rbody, mems = PRESETS[preset]
    gid = new( "MDC_PROTUB", RBODY=2.0, NLOC=3 )
    vsp.SetMdcPreset( gid, preset )
    assert_no_errors()
    vsp.Update()
    assert get( gid, "PTYPE" ) == ptype
    assert vsp.GetNumMainSurfs( gid ) >= 3
    k = 2.0 / rbody
    for got, want in zip( member_sizes( gid, len( mems ) ), mems ):
        assert got == pytest.approx( tuple( k * v for v in want ) )

    # Its locations are kept, and Default gives it back as new.
    assert get( gid, "NLOC" ) == 3
    vsp.SetMdcPreset( gid, 0 )
    vsp.Update()
    assert get( gid, "PTYPE" ) == 1
    assert get( gid, "NLOC" ) == 1


def testProtuberancePresetOnABodyIsSizedToTheBodyUnderIt():
    # On a body of radius 10, a lug for a body of radius 5 is twice its size.
    body = new( "MDC_BOD", DNOSE=20.0, LNOSE=30.0, LCENTR=100.0 )
    gid = child( "MDC_PROTUB", body, 60.0 )
    vsp.SetMdcPreset( gid, 9 )
    vsp.Update()
    assert get( gid, "RBODY" ) == pytest.approx( 10.0 )
    for got, want in zip( member_sizes( gid, 4 ), PRESETS[9][2] ):
        assert got == pytest.approx( tuple( 2.0 * v for v in want ) )


def testProtuberancesLimitTheIndexToTheirNumber():
    gid = new( "MDC_PROTUB", NLOC=3 )
    put( gid, LocIndex=3 )
    assert get( gid, "LocIndex" ) == 3
    put( gid, NLOC=2 )
    assert get( gid, "LocIndex" ) == 2


#===== MDC_INLET =====#

def testInletHasADiverterOnlyWhereHdivIsAboveZero():
    gid = new( "MDC_INLET", HDIV=0.1 )
    assert vsp.GetNumMainSurfs( gid ) == 2
    put( gid, HDIV=0.0 )
    assert vsp.GetNumMainSurfs( gid ) == 1
    put( gid, HDIV=-0.1 )
    assert vsp.GetNumMainSurfs( gid ) == 1


def testConformalInletTouchesTheTopLineUnlessItsRootIsBuried():
    gid = new( "MDC_INLET", RBODY=1.0, HDIV=0.0 )
    zlo, zhi = extent( gid, 2 )
    assert zlo == pytest.approx( 1.0, abs=1e-9 )

    # Buried, its inner face is lowered to a hundredth of RBODY below where the body passes under its sides, and
    # the rest of it stays where it is.
    half = vsp.GetGeomBBoxMax( gid, 0 ).y()
    put( gid, BuryRoot=1 )
    blo, bhi = extent( gid, 2 )
    assert blo == pytest.approx( math.sqrt( 1.0 - half ** 2 ) - 0.01, abs=1e-9 )
    assert bhi == pytest.approx( zhi, abs=1e-9 )
    assert vsp.GetGeomBBoxMax( gid, 0 ).y() == pytest.approx( half, abs=1e-9 )


def testConformalAxiInletIsSunkWhenItsRootIsBuried():
    gid = new( "MDC_INLET", RBODY=1.0, HDIV=0.0, INTYPE=3 )
    zlo, zhi = extent( gid, 2 )
    put( gid, BuryRoot=1 )
    blo, bhi = extent( gid, 2 )
    assert blo == pytest.approx( zlo - 0.01, abs=1e-9 )
    assert bhi == pytest.approx( zhi - 0.01, abs=1e-9 )


def diverter_rows( gid ):
    """Each row across the diverter of a 2DTOP inlet at the top, ahead of its flat aft face: its half width and its
    bottom."""
    xend = vsp.GetGeomBBoxMax( gid, 1 ).x()
    rows = []
    for i in range( 1, 200 ):
        ring = [ vsp.CompPnt01( gid, 1, i / 200.0, j / 400.0 ) for j in range( 401 ) ]
        if max( p.x() for p in ring ) < xend - 1e-9:
            rows.append( ( max( abs( p.y() ) for p in ring ), min( p.z() for p in ring ) ) )
    return rows


def testDiverterCornersAreOnTheBodyUnlessItsRootIsBuried():
    gid = new( "MDC_INLET", RBODY=1.0, HDIV=0.1 )
    rows = diverter_rows( gid )

    # Its sections' bottom corners are on the body, straight between, so never outside it.
    corner = [ math.hypot( hw, zb ) for hw, zb in rows ]
    assert max( corner ) == pytest.approx( 1.0, abs=1e-5 )
    assert max( corner ) <= 1.0 + 1e-9

    # Buried, every row's bottom is a tenth of HDIV lower and its width is the same.
    put( gid, BuryRoot=1 )
    for ( hw, zb ), ( bw, bz ) in zip( rows, diverter_rows( gid ) ):
        assert bw == pytest.approx( hw, abs=1e-9 )
        assert bz == pytest.approx( zb - 0.01, abs=1e-9 )


def testBuriedDiverterIsInsideAnEllipticBody():
    # On the side of a body 2 wide and 1 high, which curves more tightly there than a circle of its radius.
    body = new( "MDC_BOD", NAMELIST=1, WNOSE=2.0, ENOSE=0.5, CENTRDefault=1, ECENTR=0.5, LNOSE=2.0, LCENTR=8.0 )
    gid = child( "MDC_INLET", body, 3.0, PHIFlag=1, PHI_1=90.0, HDIV=0.05, BuryRoot=1 )
    xend = vsp.GetGeomBBoxMax( gid, 1 ).x()
    for i in range( 1, 100 ):
        ring = [ vsp.CompPnt01( gid, 1, i / 100.0, j / 200.0 ) for j in range( 201 ) ]
        if max( p.x() for p in ring ) >= xend - 1e-9:
            continue
        ylo = min( p.y() for p in ring )
        for p in ring:
            if p.y() < ylo + 1e-9:
                assert p.y() ** 2 + ( p.z() / 0.5 ) ** 2 < 1.0


def testInletSetHasNInCopiesSpacedEvenly():
    gid = new( "MDC_INLET", NIN=3, HDIV=0.0 )
    assert vsp.GetNumMainSurfs( gid ) == 3
    for k in range( 3 ):
        assert get( gid, "PHI_%d" % ( k + 1 ) ) == pytest.approx( 120.0 * k )


@pytest.mark.parametrize( "intype", ( 1, 2, 3 ) )
def testInletTypesReachFromXinltForTheirLength( intype ):
    gid = new( "MDC_INLET", INTYPE=intype, HDIV=0.0 )
    place( gid, 4.0 )
    lo, hi = extent( gid, 0 )
    assert lo == pytest.approx( 4.0, abs=1e-6 )
    assert hi == pytest.approx( 4.0 + 3.0, abs=1e-6 )


def testInletCoverPlugsTheRamp():
    open_inlet = new( "MDC_INLET", INTYPE=1, HDIV=0.0 )
    open_pts = sample( open_inlet )
    covered = new( "MDC_INLET", INTYPE=1, HDIV=0.0, COVER=1 )
    assert sample( covered ) != open_pts


def testInletLimitsItsIndicesToTheirNumbers():
    gid = new( "MDC_INLET", NIN=3 )
    put( gid, INLET=3 )
    assert get( gid, "INLET" ) == 3
    put( gid, NIN=2 )
    assert get( gid, "INLET" ) == 2


#===== MDC_FIN =====#

def testFinSetHasOneSurfacePerPanel():
    for npanel in ( 1, 3, 8 ):
        gid = new( "MDC_FIN", NPANEL=npanel )
        assert vsp.GetNumMainSurfs( gid ) == npanel
        assert vsp.GetGeomVSPSurfType( gid, 0 ) == vsp.WING_SURF


def testFinSetPlanformFollowsItsStations():
    gid = new( "MDC_FIN", NPANEL=1, RBODY=0.5, NSPAN=2, SSPAN_2=2.0, CHORD_1=1.5, CHORD_2=0.5, SWEEP_1=0.0, STA_1=1.0 )
    place( gid, 8.0 )

    # Sweep is measured at the trailing edge here, so the tip trailing edge is as far aft as the root's.  XLE_2 is
    # from the root's leading edge.
    assert get( gid, "XLE_2" ) == pytest.approx( 1.5 - 0.5 )
    xlo, xhi = extent( gid, 0 )
    assert xlo == pytest.approx( 8.0, abs=1e-6 )
    assert xhi == pytest.approx( 9.5, abs=1e-6 )

    # The panel at roll 0 is at the top, so the span is along z.
    zlo, zhi = extent( gid, 2 )
    assert zlo == pytest.approx( 0.5, abs=1e-3 )
    assert zhi == pytest.approx( 0.5 + 2.0, abs=1e-3 )


def testFinSetXleGivenMakesSweepFollow():
    gid = new( "MDC_FIN", NPANEL=1, SSPAN_2=2.0, XLEFlag_1=1, XLE_2=1.0, STA_1=0.0 )
    assert get( gid, "SWEEP_1" ) == pytest.approx( math.degrees( math.atan2( 1.0, 2.0 ) ) )


def testFinSetPanelsAreEvenlySpacedUnlessGiven():
    gid = new( "MDC_FIN", NPANEL=3 )
    for k in range( 3 ):
        assert get( gid, "PHIF_%d" % ( k + 1 ) ) == pytest.approx( 120.0 * k )
    put( gid, PHIFFlag=1, PHIF_2=70.0 )
    assert get( gid, "PHIF_2" ) == pytest.approx( 70.0 )


@pytest.mark.parametrize( "sectyp", range( 4 ) )
def testFinSetSectionTypesBuildASurfacePerPanel( sectyp ):
    gid = new( "MDC_FIN", SECTYP=sectyp )
    assert vsp.GetNumMainSurfs( gid ) == 4
    assert_no_errors()


@pytest.mark.parametrize( "series,designation", [ ( 0, 2412 ), ( 1, 2412 ), ( 2, 23012 ), ( 3, 23012 ), ( 4, 12 ),
                                                  ( 5, 6412 ), ( 6, 5 ), ( 7, 6 ), ( 8, 8 ) ] )
def testFinSetNacaSeriesBuildASurfacePerPanel( series, designation ):
    gid = new( "MDC_FIN", SECTYP=1, NACASER=series, NACADES=designation )
    assert vsp.GetNumMainSurfs( gid ) == 4
    assert_no_errors()


@pytest.mark.parametrize( "series,shape", [ ( 6, vsp.XS_WEDGE ), ( 7, vsp.XS_BICONVEX ), ( 8, vsp.XS_WEDGE ) ] )
def testFinSetSupersonicNacaIsABuiltInAirfoil( series, shape ):
    gid = new( "MDC_FIN", SECTYP=1, NPANEL=1, NACASER=series, NACADES=6.0, NACAXMAX=40.0, NACAFLAT=20.0 )
    xs = vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), 0 )
    assert vsp.GetXSecShape( xs ) == shape
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "ThickChord" ) ) == pytest.approx( 0.06 )
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "ThickLoc" ) ) == pytest.approx( 0.4 )
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "ThickLocLow" ) ) == pytest.approx( 0.4 )
    if series == 8:
        assert vsp.GetParmVal( vsp.GetXSecParm( xs, "FlatUp" ) ) == pytest.approx( 0.2 )
    elif series == 6:
        assert vsp.GetParmVal( vsp.GetXSecParm( xs, "FlatUp" ) ) == 0.0

def testFinSetLeadingEdgeIsSharpAsDatcomDrawsItUnlessLerTypeRoundsIt():
    """LER is a DATCOM input either way, but the section is rounded only when LERType says so."""
    sharp_pts = sample( new( "MDC_FIN", NPANEL=1, ZUPPER_1=0.05, ZUPPER_2=0.05 ) )
    gid = new( "MDC_FIN", NPANEL=1, ZUPPER_1=0.05, ZUPPER_2=0.05, LER_1=0.02, LER_2=0.02 )
    assert sample( gid ) == sharp_pts

    put( gid, LERType=1 )
    rounded_pts = sample( gid )
    assert len( rounded_pts ) == len( sharp_pts )
    assert rounded_pts != sharp_pts


@pytest.mark.parametrize( "lertype,keep", [ ( 1, vsp.LE_RADIUS_KEEP_CREST ), ( 2, vsp.LE_RADIUS_KEEP_ANGLE ) ] )
@pytest.mark.parametrize( "sectyp", [ 0, 2 ], ids=[ "hex", "arc" ] )
def testFinSetRoundedSectionKeepsItsChordAndItsRadius( sectyp, lertype, keep ):
    """A rounded HEX or ARC section is the airfoil rounded to LER over the chord, so the nose is still at XLE, a
    circle of radius LER."""
    zu = 0.025
    ler = 0.005
    gid = new( "MDC_FIN", NPANEL=1, SECTYP=sectyp, LERType=lertype, ZUPPER_1=zu, ZUPPER_2=zu, LER_1=ler, LER_2=ler )
    xs = vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), 0 )
    chord = vsp.GetParmVal( vsp.FindParm( gid, "CHORD_1", "Design" ) )
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "LERadius" ) ) == pytest.approx( ler / chord )
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "LERadiusType" ) ) == keep
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "LE_Trim_Type" ) ) == vsp.TRIM_NONE
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "LE_Cap_Type" ) ) == vsp.FLAT_END_CAP

    # The fin's front is still XLE.
    x0 = vsp.GetParmVal( vsp.FindParm( gid, "X_Rel_Location", "XForm" ) )
    assert extent( gid, 0 )[0] - x0 == pytest.approx( 0.0, abs=1e-6 )

    # The root section's nose is a circle of radius LER centered LER behind its leading edge, toward its trailing edge.
    le = vsp.ComputeXSecPnt( xs, 0.5 )
    te = vsp.ComputeXSecPnt( xs, 0.0 )
    along = ( te - le ) * ( 1.0 / ( te - le ).mag() )
    center = le + along * ler
    pts = [ vsp.ComputeXSecPnt( xs, 0.49 + 0.02 * j / 400.0 ) for j in range( 401 ) ]
    near = [ p for p in pts if vsp.dot( p - le, along ) < 0.5 * ler ]
    assert len( near ) > 3
    for p in near:
        assert ( p - center ).mag() == pytest.approx( ler, abs=5e-4 * ler )


def testFinSetCfocMakesAFlapPerSegmentAndRemovesThem():
    gid = new( "MDC_FIN", NPANEL=2, NSPAN=3, SSPAN_2=1.0, SSPAN_3=2.0, CHORD_3=0.5, CFOC_1=0.3, CFOC_2=0.3, CFOC_3=0.2 )
    names = sorted( vsp.GetSubSurfName( gid, ss ) for ss in vsp.GetSubSurfIDVec( gid ) )
    assert names == [ "FLAP_1", "FLAP_2" ]

    put( gid, CFOC_3=0.0 )
    names = sorted( vsp.GetSubSurfName( gid, ss ) for ss in vsp.GetSubSurfIDVec( gid ) )
    assert names == [ "FLAP_1" ]

    # An all moving fin has no flap.
    put( gid, CFOC_1=1.0 )
    assert vsp.GetSubSurfIDVec( gid ) == ()


def testFinSetArcSectionIsABiconvexAirfoil():
    gid = new( "MDC_FIN", SECTYP=2, NPANEL=1, ZLOWERRelFlag=0, ZUPPER_1=0.06, ZLOWER_1=0.02, ZUPPER_2=0.06,
               ZLOWER_2=0.02 )
    xs = vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), 0 )
    assert vsp.GetXSecShape( xs ) == vsp.XS_BICONVEX
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "ThickChord" ) ) == pytest.approx( 0.08 )
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "ThickFracUp" ) ) == pytest.approx( 0.75 )

    # A rounded leading edge is still the biconvex airfoil, rounded to LER over the chord.
    put( gid, LERType=1, LER_1=0.01, LER_2=0.01 )
    xs = vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), 0 )
    assert vsp.GetXSecShape( xs ) == vsp.XS_BICONVEX
    chord = vsp.GetParmVal( vsp.FindParm( gid, "CHORD_1", "Design" ) )
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "LERadius" ) ) == pytest.approx( 0.01 / chord )

    # Without LER the section is sharp again.
    put( gid, LER_1=0.0, LER_2=0.0 )
    xs = vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), 0 )
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "LERadius" ) ) == 0.0


def testFinRootIsOnTheBodyUnlessBuried():
    # One panel at the top, its span along z and its thickness along y, on a body 0.5 in radius.
    gid = new( "MDC_FIN", NPANEL=1, RBODY=0.5 )
    pts = [ vsp.CompPnt01( gid, 0, i / 100.0, j / 100.0 ) for i in range( 101 ) for j in range( 101 ) ]
    zlo = min( p.z() for p in pts )
    zhi = max( p.z() for p in pts )
    assert zlo == pytest.approx( 0.5, abs=1e-9 )

    # The default section is a diamond 0.025 of the 1.5 chord either side.
    half = 0.025 * 1.5

    # Buried, the root goes in as far as the body falls under its sides, and a hundredth of the radius further.
    put( gid, BuryRoot=1 )
    pts = [ vsp.CompPnt01( gid, 0, i / 100.0, j / 100.0 ) for i in range( 101 ) for j in range( 101 ) ]
    depth = 0.5 - math.sqrt( 0.5 ** 2 - half ** 2 ) + 0.005
    assert min( p.z() for p in pts ) == pytest.approx( 0.5 - depth, abs=1e-5 )
    assert min( p.z() for p in pts ) <= 0.5 - depth
    assert max( p.z() for p in pts ) == pytest.approx( zhi, abs=1e-9 )


def chord_line( gid, u ):
    """The leading and trailing edge x and the span z of the top panel's section at u."""
    pts = [ vsp.CompPnt01( gid, 0, u, j / 400.0 ) for j in range( 401 ) ]
    return min( p.x() for p in pts ), max( p.x() for p in pts ), pts[0].z()


def testFinRootExtrapolatedFollowsTheFirstSegment():
    # A tapered, swept panel on a body 0.5 in radius: the buried section is on the lines through the root's and
    # the tip's leading and trailing edges.  With the end caps the surface has four patches, the buried section at
    # the start of the second.
    gid = new( "MDC_FIN", NPANEL=1, RBODY=0.5, CHORD_1=1.5, CHORD_2=0.5, SWEEP_1=40.0, BuryRoot=1, BuryRootType=1 )
    le0, te0, z0 = chord_line( gid, 0.25 )
    ler, ter, zr = chord_line( gid, 0.5 )
    let, tet, zt = chord_line( gid, 0.75 )
    assert zr == pytest.approx( 0.5, abs=1e-9 )
    assert z0 < zr
    f = ( zr - z0 ) / ( zt - zr )
    assert le0 == pytest.approx( ler - f * ( let - ler ), abs=1e-6 )
    assert te0 == pytest.approx( ter - f * ( tet - ter ), abs=1e-6 )

    # Straight in, it is the root's chord at the root's place.
    put( gid, BuryRootType=0 )
    le0, te0, z0 = chord_line( gid, 0.25 )
    assert le0 == pytest.approx( ler, abs=1e-9 )
    assert te0 == pytest.approx( ter, abs=1e-9 )


def testFinRootBuriedOnANoseIsInsideIt():
    # On a cone nose the body is smallest under the root's leading edge, so the root goes in past it there.
    body = new( "MDC_BOD", TNOSE=0, LNOSE=4.0, DNOSE=2.0, LCENTR=6.0 )
    for kind in ( 0, 1 ):
        gid = child( "MDC_FIN", body, 1.5, NPANEL=1, CHORD_1=1.5, CHORD_2=0.5, SWEEP_1=40.0, BuryRoot=1,
                     BuryRootType=kind )
        for j in range( 201 ):
            p = vsp.CompPnt01( gid, 0, 0.0, j / 200.0 )
            assert math.hypot( p.y(), p.z() ) < min( p.x(), 4.0 ) / 4.0
        vsp.DeleteGeom( gid )


@pytest.mark.parametrize( "gam", ( -30.0, 10.0, 30.0 ) )
def testFinRootWithDihedralIsBuriedInsideTheBody( gam ):
    # The buried section goes in along the panel, turned with its dihedral, on a round body 0.5 in radius.
    gid = new( "MDC_FIN", NPANEL=4, RBODY=0.5, GAM_1=gam, GAM_2=gam, GAM_3=gam, GAM_4=gam, BuryRoot=1 )
    for s in range( 4 ):
        for j in range( 201 ):
            p = vsp.CompPnt01( gid, s, 0.25, j / 200.0 )
            assert math.hypot( p.y(), p.z() ) < 0.5


@pytest.mark.parametrize( "kind", ( 0, 1 ) )
def testFinRootDepthChangesSmoothlyWithDihedral( kind ):
    # Turned far enough, a side of the root crosses the round body and leaves it again, and further still it misses
    # the body; the depth grows to where the side just touches and falls away after it, never jumping across the body.
    gid = new( "MDC_FIN", NPANEL=1, RBODY=0.5, BuryRoot=1, BuryRootType=kind )
    depths = []
    for gam in range( 0, 90 ):
        put( gid, GAM_1=float( gam ) )
        g = math.radians( gam )
        q = vsp.CompPnt01( gid, 0, 0.25, 0.0 )
        depths.append( -( q.y() * math.sin( g ) + ( q.z() - 0.5 ) * math.cos( g ) ) )
    # The depth rises steeply just before the side touches, to about 0.19 here, but never reaches across the body.
    assert min( depths ) > 0.0
    assert max( depths ) < 0.25
    assert max( abs( b - a ) for a, b in zip( depths, depths[1:] ) ) < 0.05


def testFinRootOffTheBodyIsNotBuried():
    gid = new( "MDC_FIN", NPANEL=1, SSPAN_1=1.0, SSPAN_2=2.0, BuryRoot=1 )
    zlo, zhi = extent( gid, 2 )
    assert zlo == pytest.approx( 1.0, abs=1e-9 )


def testBuriedFinFlapStaysOnItsSegment():
    # The root carried into the body is one more patch before the segment.
    gid = new( "MDC_FIN", NPANEL=1, CFOC_1=0.25, CFOC_2=0.25 )
    ss = vsp.GetSubSurfIDVec( gid )[0]
    assert vsp.GetParmVal( vsp.FindParm( ss, "UStart", "SS_Control" ) ) == pytest.approx( 1.0 / 3.0 )
    assert vsp.GetParmVal( vsp.FindParm( ss, "UEnd", "SS_Control" ) ) == pytest.approx( 2.0 / 3.0 )
    put( gid, BuryRoot=1 )
    assert vsp.GetParmVal( vsp.FindParm( ss, "UStart", "SS_Control" ) ) == pytest.approx( 2.0 / 4.0 )
    assert vsp.GetParmVal( vsp.FindParm( ss, "UEnd", "SS_Control" ) ) == pytest.approx( 3.0 / 4.0 )


def testFinSetFlapIsAControlSurfaceAsDeepAsCfocTimesTheChord():
    gid = new( "MDC_FIN", NPANEL=1, CFOC_1=0.25, CFOC_2=0.25 )
    ss = vsp.GetSubSurfIDVec( gid )[0]
    assert vsp.GetParmVal( vsp.FindParm( ss, "Length_C_Start", "SS_Control" ) ) == pytest.approx( 0.25 )
    assert vsp.GetParmVal( vsp.FindParm( ss, "Length_C_End", "SS_Control" ) ) == pytest.approx( 0.25 )
    assert vsp.GetSubSurfType( ss ) == vsp.SS_CONTROL


def testFinSetTakesAControlSurfaceOfItsOwn():
    gid = new( "MDC_FIN" )
    ss = vsp.AddSubSurf( gid, vsp.SS_CONTROL, 0 )
    assert ss
    assert_no_errors()


def testFinSetFlapsAreSavedAndReadBack():
    gid = new( "MDC_FIN", NPANEL=2, CFOC_1=0.3, CFOC_2=0.3 )
    back = roundtrip( gid )
    names = sorted( vsp.GetSubSurfName( back, ss ) for ss in vsp.GetSubSurfIDVec( back ) )
    assert names == [ "FLAP_1" ]


def flaps( gid ):
    return vsp.GetSubSurfIDVec( gid )


def testFinSetFlapReadBackIsTheOneItKeeps():
    gid = new( "MDC_FIN", NPANEL=2, CFOC_1=0.3, CFOC_2=0.3 )
    back = roundtrip( gid )
    put( back, CFOC_2=0.4 )
    assert len( flaps( back ) ) == 1
    ss = flaps( back )[0]
    assert vsp.GetParmVal( vsp.FindParm( ss, "Length_C_End", "SS_Control" ) ) == pytest.approx( 0.4 )


def testFinSetKeepsARenamedFlap():
    gid = new( "MDC_FIN", NPANEL=2, CFOC_1=0.3, CFOC_2=0.3 )
    ss = flaps( gid )[0]
    vsp.SetSubSurfName( gid, ss, "Rudder" )
    put( gid, CFOC_2=0.4 )
    assert flaps( gid ) == ( ss, )
    assert vsp.GetSubSurfName( gid, ss ) == "Rudder"
    assert vsp.GetParmVal( vsp.FindParm( ss, "Length_C_End", "SS_Control" ) ) == pytest.approx( 0.4 )


def testFinSetLeavesAControlSurfaceNamedLikeAFlap():
    gid = new( "MDC_FIN", CFOC_1=1.0 )
    ss = vsp.AddSubSurf( gid, vsp.SS_CONTROL, 0 )
    vsp.SetSubSurfName( gid, ss, "FLAP_1" )
    put( gid, CFOC_2=0.5 )
    assert flaps( gid ) == ( ss, )


def testCopiedFinSetHasFlapsOfItsOwn():
    gid = new( "MDC_FIN", NPANEL=2, CFOC_1=0.3, CFOC_2=0.3 )
    vsp.CopyGeomToClipboard( gid )
    copy = vsp.PasteGeomClipboard()[0]
    vsp.Update()
    assert len( flaps( gid ) ) == 1
    assert len( flaps( copy ) ) == 1
    assert flaps( copy ) != flaps( gid )

    put( copy, CFOC_2=0.4 )
    assert len( flaps( copy ) ) == 1
    assert len( flaps( gid ) ) == 1
    ss = flaps( copy )[0]
    assert vsp.GetParmVal( vsp.FindParm( ss, "Length_C_End", "SS_Control" ) ) == pytest.approx( 0.4 )


#===== On a body =====#

def child( name, body, x, **parms ):
    gid = vsp.AddGeom( name, body )
    assert gid
    place( gid, x )
    put( gid, **parms )
    return gid


def testProtuberanceTakesItsRadiusFromItsBody():
    body = new( "MDC_BOD", DNOSE=1.0, CENTRDefault=0, DCENTR=2.0, LNOSE=2.0, LCENTR=4.0 )
    # On the centerbody, a frustum from 1 to 2 across over 2 to 6, at 4 the body is 1.5 across.
    gid = child( "MDC_PROTUB", body, 4.0, PTYPE=5, LPROT_1=0.4, WPROT_1=0.2, HPROT_1=0.3, OPROT_1=0.1 )
    assert get( gid, "RBODY" ) == pytest.approx( 0.75, abs=1e-6 )
    zlo, zhi = extent( gid, 2 )
    assert zhi == pytest.approx( 0.75 + 0.1 + 0.3, abs=1e-6 )

    # Moved along the body, it follows the body's radius.
    place( gid, 3.0 )
    assert get( gid, "RBODY" ) == pytest.approx( 0.625, abs=1e-6 )


def testProtuberanceFollowsItsBodysChange():
    body = new( "MDC_BOD", DNOSE=1.0, LNOSE=2.0, LCENTR=6.0 )
    gid = child( "MDC_PROTUB", body, 4.0 )
    put( body, DNOSE=1.6 )
    assert get( gid, "RBODY" ) == pytest.approx( 0.8, abs=1e-6 )


@pytest.mark.parametrize( "name", ( "MDC_PROTUB", "MDC_INLET", "MDC_FIN" ) )
def testComponentFollowsItsBodyWhenTheBodyMoves( name ):
    # On the centerbody, a frustum from 1 to 2 across over 2 to 6.  With the body moved 1 aft, the component at 4 is
    # over its 3, as one put there after the move is.
    body = new( "MDC_BOD", DNOSE=1.0, CENTRDefault=0, DCENTR=2.0, LNOSE=2.0, LCENTR=4.0 )
    gid = child( name, body, 4.0 )
    rbody = get( gid, "RBODY" )
    place( body, 1.0 )

    moved = child( name, body, 4.0 )
    assert get( gid, "RBODY" ) == pytest.approx( get( moved, "RBODY" ) )
    assert get( gid, "RBODY" ) < rbody - 1e-3
    assert_same( sample( gid ), sample( moved ) )


def testComponentPutOnABodyTakesItsRadius():
    body = new( "MDC_BOD", DNOSE=1.0, LNOSE=2.0, LCENTR=10.0 )
    gid = vsp.AddGeom( "MDC_FIN" )
    put( gid, RBODY=3.0 )
    vsp.SetGeomParent( gid, body )
    vsp.Update()
    assert get( gid, "RBODY" ) == pytest.approx( 0.5, abs=1e-6 )


def testComponentOffABodyKeepsItsRbody():
    vsp.VSPRenew()
    gid = vsp.AddGeom( "MDC_INLET" )
    put( gid, RBODY=0.8 )
    assert get( gid, "RBODY" ) == pytest.approx( 0.8 )


def testInletSitsOnItsBody():
    body = new( "MDC_BOD", DNOSE=1.0, LNOSE=2.0, LCENTR=6.0 )
    gid = child( "MDC_INLET", body, 4.0, INTYPE=3, HDIV=0.0, NIN=2 )
    assert get( gid, "RBODY" ) == pytest.approx( 0.5, abs=1e-6 )


def testFinPanelsSitOnAnEllipticBodyAtTheirRolls():
    # 2 wide and 1 high: the top and bottom panels' roots are 0.5 from the axis, the side panels' 1.
    body = new( "MDC_BOD", NAMELIST=1, WNOSE=2.0, ENOSE=0.5, CENTRDefault=1, ECENTR=0.5, LNOSE=2.0, LCENTR=8.0 )
    gid = child( "MDC_FIN", body, 6.0, NPANEL=4, NSPAN=2, SSPAN_2=1.0 )
    roots = []
    for s in range( 4 ):
        p = vsp.CompPnt01( gid, s, 0.0, 0.5 )
        roots.append( math.hypot( p.y(), p.z() ) )
    assert sorted( roots ) == pytest.approx( [ 0.5, 0.5, 1.0, 1.0 ], abs=2e-3 )
    assert get( gid, "RBODY" ) == pytest.approx( 0.5, abs=1e-6 )


def testProtuberancesSitOnAnEllipticBodyAtTheirRolls():
    body = new( "MDC_BOD", NAMELIST=1, WNOSE=2.0, ENOSE=0.5, CENTRDefault=1, ECENTR=0.5, LNOSE=2.0, LCENTR=8.0 )
    gid = child( "MDC_PROTUB", body, 6.0, PTYPE=5, NLOC=4, LPROT_1=0.4, WPROT_1=0.1, HPROT_1=0.3, OPROT_1=0.1 )
    reach = []
    for s in range( vsp.GetNumMainSurfs( gid ) ):
        lo = vsp.GetGeomBBoxMin( gid, s )
        hi = vsp.GetGeomBBoxMax( gid, s )
        reach.append( max( abs( lo.y() ), abs( hi.y() ), abs( lo.z() ), abs( hi.z() ) ) )
    assert sorted( reach ) == pytest.approx( [ 0.9, 0.9, 1.4, 1.4 ], abs=1e-3 )


#===== All of them =====#

TYPES = ( "MDC_BOD", "MDC_PROTUB", "MDC_INLET", "MDC_FIN" )


@pytest.mark.parametrize( "name,nsurf", [ ( "MDC_BOD", 1 ), ( "MDC_PROTUB", 1 ), ( "MDC_INLET", 2 ), ( "MDC_FIN", 4 ) ] )
def testEachComponentIsAGeomTypeAndBuildsItsSurfaces( name, nsurf ):
    gid = new( name )
    assert vsp.GetGeomTypeName( gid ) == name
    assert vsp.GetNumMainSurfs( gid ) == nsurf
    assert_no_errors()


def section_ids( gid ):
    ids = []
    for i in range( vsp.GetNumXSecSurfs( gid ) ):
        surf = vsp.GetXSecSurf( gid, i )
        ids.append( tuple( vsp.GetXSec( surf, j ) for j in range( vsp.GetNumXSec( surf ) ) ) )
    return ids


RESIZE = { "MDC_BOD": dict( LCENTR=6.0 ), "MDC_PROTUB": dict( LPROT_1=0.7 ), "MDC_INLET": dict( H3=0.4 ),
           "MDC_FIN": dict( CHORD_1=1.2 ) }


@pytest.mark.parametrize( "name", TYPES )
def testEachComponentKeepsItsSectionsWhenResized( name ):
    gid = new( name )
    before = section_ids( gid )
    assert before and all( before )
    put( gid, **RESIZE[name] )
    assert section_ids( gid ) == before


def built( name, *steps ):
    gid = new( name, **steps[0] )
    for step in steps[1:]:
        put( gid, **step )
    return sample( gid )


# Steps that change the sections each surface is made of, and back.  The component afterwards is the one built
# straight from where the steps end.
RESHAPE = [
    ( "MDC_BOD", [ dict( LAFT=3.0, TAFT=1 ), dict( TAFT=0 ) ] ),
    ( "MDC_BOD", [ dict( LAFT=3.0 ), dict( LCENTR=0.0 ), dict( LCENTR=5.0 ) ] ),
    ( "MDC_FIN", [ dict( SECTYP=1 ), dict( SECTYP=0 ) ] ),
    ( "MDC_FIN", [ dict( NSPAN=4 ), dict( NSPAN=2 ), dict( NSPAN=3 ) ] ),
    ( "MDC_FIN", [ dict( NPANEL=4 ), dict( NPANEL=2 ), dict( NPANEL=3 ) ] ),
    ( "MDC_PROTUB", [ dict( PTYPE=1 ), dict( PTYPE=5 ), dict( PTYPE=3 ) ] ),
    ( "MDC_PROTUB", [ dict( NLOC=1 ), dict( NLOC=3 ), dict( NLOC=2 ) ] ),
    ( "MDC_INLET", [ dict( INTYPE=1 ), dict( INTYPE=3 ), dict( INTYPE=2 ) ] ),
    ( "MDC_INLET", [ dict( NIN=1 ), dict( NIN=3 ), dict( NIN=2 ) ] ),
]


@pytest.mark.parametrize( "name,steps", RESHAPE )
def testEachComponentReshapedIsAsIfBuiltSo( name, steps ):
    final = {}
    for step in steps:
        final.update( step )
    assert_same( built( name, *steps ), built( name, final ) )


@pytest.mark.parametrize( "name", TYPES )
def testEachComponentIsSavedAndReadBackToTheSameSurfaces( name ):
    gid = new( name )
    before = sample( gid )
    after = sample( roundtrip( gid ) )
    assert_same( before, after )
    assert_no_errors()


@pytest.mark.parametrize( "name", TYPES )
def testEachComponentIsCopiedToTheSameSurfaces( name ):
    gid = new( name )
    vsp.CopyGeomToClipboard( gid )
    ids = vsp.PasteGeomClipboard()
    assert len( ids ) == 1
    assert_same( sample( gid ), sample( ids[0] ) )
    assert_no_errors()


# Components with more stations, panels, protuberances, members and inlets than a new one has, each with values of
# its own: the Parms each count makes are there when the file's values are read.
COUNTED = [
    ( "MDC_FIN", dict( NSPAN=4, NPANEL=3, SSPAN_2=0.5, SSPAN_3=1.0, SSPAN_4=1.6, CHORD_3=0.6, CHORD_4=0.3,
                      SWEEP_3=30.0, ZUPPER_4=0.04, PHIFFlag=1, PHIF_1=10.0, PHIF_2=130.0, PHIF_3=250.0, GAM_3=15.0 ),
      [ "SSPAN_4", "CHORD_4", "SWEEP_3", "ZUPPER_4", "PHIF_3", "GAM_3" ] ),
    ( "MDC_PROTUB", dict( PTYPE=3, NLOC=3, PHIPROFlag=1, PHIPRO_1=10.0, PHIPRO_2=100.0, PHIPRO_3=200.0, WPROT_4=0.6,
                         HPROT_4=0.08 ),
      [ "PHIPRO_3", "WPROT_4", "HPROT_4" ] ),
    ( "MDC_INLET", dict( NIN=3, PHIFlag=1, PHI_1=20.0, PHI_2=140.0, PHI_3=260.0 ), [ "PHI_2", "PHI_3" ] ),
]


@pytest.mark.parametrize( "name,parms,check", COUNTED, ids=[ c[0] for c in COUNTED ] )
def testEachComponentReadsBackTheParmsItsCountsMake( name, parms, check ):
    gid = new( name, **parms )
    before = sample( gid )
    back = roundtrip( gid )
    for p in check:
        assert get( back, p ) == pytest.approx( parms[p] )
    assert_same( before, sample( back ) )


def testFinSetUserSectionIsAFileAirfoilKeptWithTheComponent():
    gid = new( "MDC_FIN", SECTYP=3, NPANEL=1 )
    xs = vsp.GetMdcFinUserXSec( gid )
    assert xs

    up = [ vsp.vec3d( i / 10.0, 0.08 * math.sin( math.pi * i / 10.0 ), 0.0 ) for i in range( 11 ) ]
    lo = [ vsp.vec3d( i / 10.0, -0.02 * math.sin( math.pi * i / 10.0 ), 0.0 ) for i in range( 11 ) ]
    vsp.SetAirfoilPnts( xs, up, lo )
    vsp.Update()
    before = sample( gid )

    back = roundtrip( gid )
    assert_same( before, sample( back ) )
    saved_up = vsp.GetAirfoilUpperPnts( vsp.GetMdcFinUserXSec( back ) )
    assert len( saved_up ) == 11
    assert saved_up[5].y() == pytest.approx( 0.08, abs=1e-9 )


def testCopiedFinSetHasAUserSectionOfItsOwn():
    gid = new( "MDC_FIN", SECTYP=3, NPANEL=1 )
    up = [ vsp.vec3d( i / 10.0, 0.08 * math.sin( math.pi * i / 10.0 ), 0.0 ) for i in range( 11 ) ]
    lo = [ vsp.vec3d( i / 10.0, -0.02 * math.sin( math.pi * i / 10.0 ), 0.0 ) for i in range( 11 ) ]
    vsp.SetAirfoilPnts( vsp.GetMdcFinUserXSec( gid ), up, lo )
    vsp.Update()

    vsp.CopyGeomToClipboard( gid )
    copy = vsp.PasteGeomClipboard()[0]
    vsp.Update()
    assert vsp.GetMdcFinUserXSec( copy ) != vsp.GetMdcFinUserXSec( gid )
    assert_same( sample( gid ), sample( copy ) )
