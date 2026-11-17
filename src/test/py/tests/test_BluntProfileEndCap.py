# Blunted profile end caps: power law, Von Karman and Sears-Haack caps with a radius.
#
# A profile cap with CapU*Radius above zero, not truncated, is blunted by a sphere of that radius times the cap's
# length scale whose front is the cap's point, so the cap keeps its length and its end.  The sphere's arc runs from
# the point round to where the profile touches it, and the profile runs on to the end from a virtual tip, ahead of
# the point or just behind it, placed so that it touches the sphere, as a cone cap's sides run from the end to touch
# its sphere.  A power law of exponent one is then the blunted cone.  A profile whose tip is rounder than the sphere,
# a power law's of exponent one-half or less, touches no sphere that small, and the cap stays sharp.  A sphere as big
# as the end is met by the line from the end that touches it, as on a cone cap.
#
# MDC_BOD builds its POWER, HAACK and KARMAN noses as these caps, BNOSE their radius, so its noses are checked
# against the exact blunted profile here.

import math

import openvsp as vsp
import pytest

from testhelp import drop_errors, assert_no_errors

LNOSE = 30.0
RNOSE = 6.0
CONE, OGIVE, POWER, HAACK, KARMAN = range( 5 )


def nose_profile( tnose, n ):
    """The nose's radius fraction at s, the distance from its tip as a fraction of its length, and its slope."""
    if tnose == POWER:
        def fn( s ):
            if s <= 0.0:
                return 0.0, math.inf
            return s ** n, n * s ** ( n - 1.0 )
        return fn

    c = 0.0
    if tnose == HAACK:
        c = 1.0 / 3.0

    def haack( s ):
        if s <= 0.0:
            return 0.0, math.inf
        if s >= 1.0:
            return 1.0, 0.0
        th = math.acos( 1.0 - 2.0 * s )
        st = math.sin( th )
        f = math.sqrt( max( th - 0.5 * math.sin( 2.0 * th ) + c * st ** 3, 0.0 ) / math.pi )
        dth = 1.0 / math.sqrt( s * ( 1.0 - s ) )
        return f, ( 2.0 * st * st + 3.0 * c * st * st * math.cos( th ) ) / ( 2.0 * math.pi * f ) * dth
    return haack


def touching_sphere( g, lp, s ):
    """Where the normal of the profile g, from a virtual tip lp ahead of the base, meets the axis at s, and the
    radius of the sphere about that point that touches it, both measured from the nose's tip."""
    f, df = g( s )
    x = LNOSE - lp + lp * s
    r = RNOSE * f
    rp = RNOSE * df / lp
    return x + r * rp, r * math.sqrt( 1.0 + rp * rp )


def touch( g, lp, b, nscan=400 ):
    """The s nearest the base at which a sphere b in radius touches g from a virtual tip lp ahead of the base,
    and that sphere's front, or None."""
    hi = 1.0
    if not touching_sphere( g, lp, hi )[1] > b:
        return None
    for i in range( 1, nscan + 1 ):
        lo = ( nscan - i ) / nscan
        if i == nscan:
            lo = 1e-12
        if not touching_sphere( g, lp, lo )[1] > b:
            for k in range( 80 ):
                mid = 0.5 * ( lo + hi )
                if touching_sphere( g, lp, mid )[1] > b:
                    hi = mid
                else:
                    lo = mid
            s = 0.5 * ( lo + hi )
            return s, touching_sphere( g, lp, s )[0] - b
        hi = lo
    return None


def exact_meridian( tnose, n, b, npts=1500 ):
    """The exact blunted nose from its tip to its base as ( x, r ), and whether it is blunted."""
    g = nose_profile( tnose, n )
    if tnose == CONE:
        g = nose_profile( POWER, 1.0 )

    # The virtual tip whose profile's touching sphere has its front at the tip, nearest the nose's own length.
    sol = None
    if b > 0.0:
        for step in range( 1, 161 ):
            for j in ( step, -step ):
                if j < -16 or sol:
                    continue
                jin = j - 1 if j > 0 else j + 1
                lin = LNOSE * 2.0 ** ( jin / 4.0 )
                lout = LNOSE * 2.0 ** ( j / 4.0 )
                a = touch( g, lin, b )
                c = touch( g, lout, b )
                if a and c and ( a[1] > 0.0 ) != ( c[1] > 0.0 ):
                    lo, hi, flo = lin, lout, a[1]
                    for k in range( 80 ):
                        mid = math.sqrt( lo * hi )
                        t = touch( g, mid, b )
                        if ( t[1] > 0.0 ) == ( flo > 0.0 ):
                            lo, flo = mid, t[1]
                        else:
                            hi = mid
                    lp = math.sqrt( lo * hi )
                    sol = ( lp, touch( g, lp, b )[0] )

    pts = []
    if sol is None and b >= RNOSE:
        d = math.hypot( LNOSE - b, RNOSE )
        phi = math.atan2( RNOSE, LNOSE - b ) + math.acos( b / d )
        for k in range( npts + 1 ):
            th = math.pi + ( phi - math.pi ) * k / npts
            pts.append( ( b + b * math.cos( th ), b * math.sin( th ) ) )
        qx = b + b * math.cos( phi )
        qr = b * math.sin( phi )
        for k in range( npts + 1 ):
            t = k / npts
            pts.append( ( qx + ( LNOSE - qx ) * t, qr + ( RNOSE - qr ) * t ) )
        return pts, True
    if sol is None:
        for k in range( npts + 1 ):
            s = ( k / npts ) ** 2
            pts.append( ( LNOSE * s, RNOSE * g( s )[0] ) )
        return pts, False

    lp, st = sol
    xt = LNOSE - lp + lp * st
    rt = RNOSE * g( st )[0]
    tht = math.atan2( rt, xt - b )
    for k in range( npts + 1 ):
        th = math.pi + ( tht - math.pi ) * k / npts
        pts.append( ( b + b * math.cos( th ), b * math.sin( th ) ) )
    for k in range( npts + 1 ):
        s = st + ( 1.0 - st ) * k / npts
        pts.append( ( LNOSE - lp + lp * s, RNOSE * g( s )[0] ) )
    return pts, True


def gap( poly, q ):
    """The distance from q to the polyline poly."""
    best = math.inf
    for a, c in zip( poly[:-1], poly[1:] ):
        dx = c[0] - a[0]
        dy = c[1] - a[1]
        den = dx * dx + dy * dy
        t = 0.0
        if den > 0.0:
            t = max( 0.0, min( 1.0, ( ( q[0] - a[0] ) * dx + ( q[1] - a[1] ) * dy ) / den ) )
        best = min( best, math.hypot( a[0] + t * dx - q[0], a[1] + t * dy - q[1] ) )
    return best


def mdc_body( tnose, bnose, power=1.0 ):
    """An MDC_BOD with a nose LNOSE long and RNOSE in radius on a cylinder, its nose's tip at the origin."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( "MDC_BOD" )
    for name, val in ( ( "TNOSE", tnose ), ( "POWER", power ), ( "LNOSE", LNOSE ), ( "DNOSE", 2.0 * RNOSE ),
                       ( "BNOSE", bnose ), ( "LCENTR", LNOSE ), ( "LAFT", 0.0 ) ):
        vsp.SetParmVal( gid, name, "Design", val )
    vsp.Update()
    assert_no_errors()
    return gid


def point_at_x( gid, w, x ):
    """The point of the body along W w where it reaches x, by bisection in U."""
    lo = 0.0
    hi = 0.9
    for k in range( 80 ):
        mid = 0.5 * ( lo + hi )
        if vsp.CompPnt01( gid, 0, mid, w ).x() < x:
            lo = mid
        else:
            hi = mid
    return vsp.CompPnt01( gid, 0, 0.5 * ( lo + hi ), w )


def nose_meridian( gid, w=0.0, nu=1500 ):
    """The nose along W w as ( x, r ), r from the x axis."""
    out = []
    for i in range( nu + 1 ):
        p = vsp.CompPnt01( gid, 0, 0.5 * i / nu, w )
        if p.x() <= LNOSE:
            out.append( ( p.x(), math.hypot( p.y(), p.z() ) ) )
    return out


CASES = [ ( POWER, 0.75, 0.6 ), ( POWER, 0.75, 1.5 ), ( POWER, 0.5, 1.5 ), ( POWER, 0.3, 3.0 ), ( POWER, 2.0, 1.5 ),
          ( HAACK, 1.0, 0.6 ), ( HAACK, 1.0, 3.0 ), ( KARMAN, 1.0, 1.5 ), ( KARMAN, 1.0, 5.5 ) ]


@pytest.mark.parametrize( "tnose,power,bnose", CASES )
def testBluntedNoseIsTheExactBluntedProfile( tnose, power, bnose ):
    exact, blunt = exact_meridian( tnose, power, bnose )
    assert blunt
    gid = mdc_body( tnose, bnose, power )
    for w in ( 0.0, 0.3 ):
        mer = nose_meridian( gid, w )
        # The tip stays at the nose's point, the base keeps its radius, and the whole nose is the exact one.
        assert min( x for x, r in mer ) == pytest.approx( 0.0, abs=1e-9 )
        base = point_at_x( gid, w, LNOSE )
        body = point_at_x( gid, w, 1.5 * LNOSE )
        assert math.hypot( base.y(), base.z() ) == pytest.approx( math.hypot( body.y(), body.z() ), abs=1e-9 )
        assert max( gap( exact, q ) for q in mer[::3] ) < 2e-3


@pytest.mark.parametrize( "bnose", [ 0.3, 1.5, 4.0 ] )
def testPowerLawOfExponentOneIsTheBluntedCone( bnose ):
    cone = nose_meridian( mdc_body( CONE, bnose ) )
    power = nose_meridian( mdc_body( POWER, bnose, 1.0 ) )
    assert max( gap( cone, q ) for q in power[::3] ) < 2e-3
    assert max( gap( power, q ) for q in cone[::3] ) < 2e-3


@pytest.mark.parametrize( "power,bnose", [ ( 0.5, 0.5 ), ( 0.3, 1.5 ), ( 0.3, 2.5 ) ] )
def testProfileRounderThanTheSphereStaysSharp( power, bnose ):
    exact, blunt = exact_meridian( POWER, power, bnose )
    assert not blunt
    sharp = mdc_body( POWER, 0.0, power )
    pts = [ vsp.CompPnt01( sharp, 0, 0.5 * i / 200, j / 8 ) for i in range( 201 ) for j in range( 8 ) ]
    gid = mdc_body( POWER, bnose, power )
    for k, p in enumerate( [ vsp.CompPnt01( gid, 0, 0.5 * i / 200, j / 8 ) for i in range( 201 ) for j in range( 8 ) ] ):
        assert ( p - pts[k] ).mag() < 1e-12


@pytest.mark.parametrize( "tnose,power", [ ( POWER, 0.75 ), ( POWER, 2.0 ), ( HAACK, 1.0 ), ( KARMAN, 1.0 ) ] )
def testSphereAsBigAsTheBaseIsMetAsOnACone( tnose, power ):
    for bnose in ( RNOSE, 8.0 ):
        cone = nose_meridian( mdc_body( CONE, bnose ) )
        prof = nose_meridian( mdc_body( tnose, bnose, power ) )
        assert max( gap( cone, q ) for q in prof[::3] ) < 2e-3


def testBluntingIsContinuousUpToTheBase():
    # Just short of the base's radius the blunted profile is nearly the cone's sphere and line.
    cone = nose_meridian( mdc_body( CONE, 0.999 * RNOSE ) )
    prof = nose_meridian( mdc_body( KARMAN, 0.999 * RNOSE ) )
    assert max( gap( cone, q ) for q in prof[::3] ) < 2e-2


def stack_nose( captype, rad ):
    """Points of a Stack's nose, its first section a circle 2 across and its nose cap of captype 3 long with a
    sphere rad in radius."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( "STACK" )
    xss = vsp.GetXSecSurf( gid, 0 )
    vsp.ChangeXSecShape( xss, 0, vsp.XS_CIRCLE )
    vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, 0 ), "Circle_Diameter" ), 2.0 )
    vsp.SetParmVal( gid, "CapUMinOption", "EndCap", captype )
    vsp.SetParmVal( gid, "CapUMinRelFlag", "EndCap", 0.0 )
    vsp.SetParmVal( gid, "CapUMinLengthDim", "EndCap", 3.0 )
    vsp.SetParmVal( gid, "CapUMinRadiusDim", "EndCap", rad )
    vsp.Update()
    assert_no_errors()
    return [ vsp.CompPnt01( gid, 0, 0.3 * i / 600, j / 16 ) for i in range( 601 ) for j in range( 16 ) ]


@pytest.mark.parametrize( "captype", [ vsp.POWER_LAW_END_CAP, vsp.VON_KARMAN_END_CAP, vsp.SEARS_HAACK_END_CAP ] )
def testStackProfileCapIsBluntedAtItsPoint( captype ):
    # Blunting keeps the cap's point where the sharp cap has it, and the cap's tip is on the sphere.
    sharp = min( stack_nose( captype, 0.0 ), key=lambda p: p.x() )
    pts = stack_nose( captype, 0.4 )
    tip = min( pts, key=lambda p: p.x() )
    assert ( tip - sharp ).mag() < 1e-9
    cen = vsp.vec3d( tip.x() + 0.4, tip.y(), tip.z() )
    near = [ p for p in pts if p.x() < tip.x() + 0.15 ]
    assert len( near ) > 10
    for p in near:
        assert ( p - cen ).mag() == pytest.approx( 0.4, abs=1e-3 )
