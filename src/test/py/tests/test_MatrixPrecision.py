"""Round trips through Matrix4d, vec3d and vec2d must hold to rounding, including at the
configurations where a naive formula loses its digits: angles near 0, 90 and 180 degrees,
nearly parallel and antiparallel vectors, and nearly parallel planes.

Every reference here is exact for the double inputs used, or within an ulp of it: a rotation
built from a quaternion, a vector pair whose angle is atan2 of two of its own components, or a
plane pair whose intersection is a single division.
"""

import math
import random

import pytest

import openvsp as vsp

TOL = 1e-14


def basis( m ):
    c = [ vsp.vec3d(), vsp.vec3d(), vsp.vec3d() ]
    m.getBasis( c[0], c[1], c[2] )
    return [ [ c[j][i] for j in range( 3 ) ] for i in range( 3 ) ]


def from_rows( M ):
    m = vsp.Matrix4d()
    cols = [ vsp.vec3d( M[0][j], M[1][j], M[2][j] ) for j in range( 3 ) ]
    m.setBasis( cols[0], cols[1], cols[2] )
    return m


def matmul( A, B ):
    return [ [ sum( A[i][k] * B[k][j] for k in range( 3 ) ) for j in range( 3 ) ] for i in range( 3 ) ]


def rx( a ):
    c, s = math.cos( a ), math.sin( a )
    return [ [ 1, 0, 0 ], [ 0, c, -s ], [ 0, s, c ] ]


def ry( a ):
    c, s = math.cos( a ), math.sin( a )
    return [ [ c, 0, s ], [ 0, 1, 0 ], [ -s, 0, c ] ]


def rz( a ):
    c, s = math.cos( a ), math.sin( a )
    return [ [ c, -s, 0 ], [ s, c, 0 ], [ 0, 0, 1 ] ]


def maxdiff( A, B ):
    return max( abs( A[i][j] - B[i][j] ) for i in range( 3 ) for j in range( 3 ) )


def quat_matrix( w, x, y, z ):
    return [ [ 1 - 2 * ( y * y + z * z ), 2 * ( x * y - w * z ), 2 * ( x * z + w * y ) ],
             [ 2 * ( x * y + w * z ), 1 - 2 * ( x * x + z * z ), 2 * ( y * z - w * x ) ],
             [ 2 * ( x * z - w * y ), 2 * ( y * z + w * x ), 1 - 2 * ( x * x + y * y ) ] ]


def random_unit( rng ):
    while True:
        v = [ rng.gauss( 0, 1 ) for _ in range( 3 ) ]
        n = math.sqrt( sum( x * x for x in v ) )
        if n > 1e-3:
            return [ x / n for x in v ]


def half_angles( rng ):
    """Half angles spread over 0..pi/2, crowded at the ends where a naive formula fails."""
    out = []
    for _ in range( 100 ):
        out.append( rng.uniform( 0.0, 0.5 * math.pi ) )
        out.append( 10 ** rng.uniform( -12, -3 ) )
        out.append( 0.5 * math.pi - 10 ** rng.uniform( -12, -3 ) )
    return out


def test_toquat_recovers_quaternion():
    """toQuat must return the quaternion a rotation was built from, to rounding."""
    rng = random.Random( 1 )
    worst = 0.0
    for h in half_angles( rng ):
        a = random_unit( rng )
        q = [ math.cos( h ) ] + [ math.sin( h ) * x for x in a ]
        m = from_rows( quat_matrix( *q ) )
        m.translatef( 0.0, 0.0, 0.0 )
        r = m.toQuat()
        e = min( max( abs( r[i] - q[i] ) for i in range( 4 ) ), max( abs( r[i] + q[i] ) for i in range( 4 ) ) )
        worst = max( worst, e )
    assert worst < 4e-15, "toQuat lost precision: %g" % worst


def test_toquat_translation():
    m = vsp.Matrix4d()
    m.translatef( 1.0 / 3.0, -2.0 / 7.0, 1e10 / 3.0 )
    r = m.toQuat()
    assert r[4:] == ( 1.0 / 3.0, -2.0 / 7.0, 1e10 / 3.0 ) or list( r[4:] ) == [ 1.0 / 3.0, -2.0 / 7.0, 1e10 / 3.0 ]


def arcball_rebuild( a ):
    return matmul( matmul( ry( a.y() ), rx( a.x() ) ), rz( a.z() ) )


@pytest.mark.parametrize( "case", [ "random", "near90", "at90" ] )
def test_arcball_angles_round_trip( case ):
    """getArcballAngles' Y, X, Z must rebuild the matrix, also with X at or near +-90 degrees."""
    rng = random.Random( 2 )
    worst = 0.0
    for _ in range( 300 ):
        y = rng.uniform( -math.pi, math.pi )
        z = rng.uniform( -math.pi, math.pi )
        if case == "random":
            x = rng.uniform( -0.5 * math.pi, 0.5 * math.pi )
        elif case == "near90":
            x = rng.choice( [ -1, 1 ] ) * ( 0.5 * math.pi - 10 ** rng.uniform( -12, -1 ) )
        else:
            x = rng.choice( [ -1, 1 ] ) * 0.5 * math.pi
        M = matmul( matmul( ry( y ), rx( x ) ), rz( z ) )
        worst = max( worst, maxdiff( M, arcball_rebuild( from_rows( M ).getArcballAngles() ) ) )
    assert worst < 2e-15, "getArcballAngles round trip: %g" % worst


def test_arcball_angles_values():
    """Away from +-90 degrees the angles themselves come back."""
    rng = random.Random( 3 )
    for _ in range( 200 ):
        y = rng.uniform( -3.0, 3.0 )
        x = rng.uniform( -1.5, 1.5 )
        z = rng.uniform( -3.0, 3.0 )
        a = from_rows( matmul( matmul( ry( y ), rx( x ) ), rz( z ) ) ).getArcballAngles()
        assert abs( a.x() - x ) < 1e-13 and abs( a.y() - y ) < 1e-13 and abs( a.z() - z ) < 1e-13


@pytest.mark.parametrize( "deg", [ 90.0 * k for k in range( -8, 9 ) ] )
def test_rotate_xyz_quarter_turns_exact( deg ):
    """A multiple of 90 degrees gives a matrix of exact zeros and ones."""
    for f in ( "rotateX", "rotateY", "rotateZ" ):
        m = vsp.Matrix4d()
        getattr( m, f )( deg )
        for row in basis( m ):
            for v in row:
                assert v in ( -1.0, 0.0, 1.0 ), "%s( %g ) gave %r" % ( f, deg, v )


def test_rotate_xyz_full_turns():
    """Whole turns added to the angle change nothing."""
    rng = random.Random( 4 )
    for _ in range( 200 ):
        a = rng.uniform( -180.0, 180.0 )
        k = rng.randint( -3000, 3000 )
        m0 = vsp.Matrix4d()
        m0.rotateZ( a )
        m1 = vsp.Matrix4d()
        m1.rotateZ( a + 360.0 * k )
        # a + 360 k is itself rounded, by at most half an ulp of it
        slop = abs( ( a + 360.0 * k ) - 360.0 * k - a ) * math.pi / 180.0
        assert maxdiff( basis( m0 ), basis( m1 ) ) <= slop + 4e-16


def test_rotate_xyz_odd():
    """rotate( -a ) is the transpose of rotate( a ), bit for bit."""
    rng = random.Random( 5 )
    for _ in range( 200 ):
        a = rng.uniform( -720.0, 720.0 )
        for f in ( "rotateX", "rotateY", "rotateZ" ):
            m0 = vsp.Matrix4d()
            getattr( m0, f )( a )
            m1 = vsp.Matrix4d()
            getattr( m1, f )( -a )
            A = basis( m0 )
            B = basis( m1 )
            assert all( A[i][j] == B[j][i] for i in range( 3 ) for j in range( 3 ) )


def test_get_angles_quarter_turns_round_trip():
    """Angles at multiples of 90 degrees survive the round trip through a matrix exactly."""
    for ax in ( -90.0, 0.0, 90.0, 180.0 ):
        for ay in ( -90.0, -45.0, 0.0, 45.0, 90.0 ):
            for az in ( -90.0, 0.0, 90.0, 180.0 ):
                m = vsp.Matrix4d()
                m.rotateX( ax )
                m.rotateY( ay )
                m.rotateZ( az )
                a = m.getAngles()
                m2 = vsp.Matrix4d()
                m2.rotateX( a.x() )
                m2.rotateY( a.y() )
                m2.rotateZ( a.z() )
                assert maxdiff( basis( m ), basis( m2 ) ) < 1e-15


@pytest.mark.parametrize( "case", [ "random", "near90" ] )
def test_get_angles_round_trip( case ):
    rng = random.Random( 6 )
    worst = 0.0
    for _ in range( 300 ):
        ax = rng.uniform( -180, 180 )
        az = rng.uniform( -180, 180 )
        if case == "random":
            ay = rng.uniform( -90, 90 )
        else:
            ay = rng.choice( [ -1, 1 ] ) * ( 90 - 10 ** rng.uniform( -12, -1 ) )
        m = vsp.Matrix4d()
        m.rotateX( ax )
        m.rotateY( ay )
        m.rotateZ( az )
        a = m.getAngles()
        m2 = vsp.Matrix4d()
        m2.rotateX( a.x() )
        m2.rotateY( a.y() )
        m2.rotateZ( a.z() )
        worst = max( worst, maxdiff( basis( m ), basis( m2 ) ) )
    assert worst < 2e-15


@pytest.mark.parametrize( "case", [ "random", "near0", "nearpi" ] )
def test_rotation_axis_round_trip( case ):
    """getRotationAxis recovers the axis and angle a quaternion rotation was built from."""
    rng = random.Random( 7 )
    worst = 0.0
    for _ in range( 300 ):
        if case == "random":
            h = rng.uniform( 0.0, 0.5 * math.pi )
        elif case == "near0":
            h = 10 ** rng.uniform( -12, -3 )
        else:
            h = 0.5 * math.pi - 10 ** rng.uniform( -12, -3 )
        a = random_unit( rng )
        q = [ math.cos( h ) ] + [ math.sin( h ) * x for x in a ]
        m = from_rows( quat_matrix( *q ) )
        d = vsp.vec3d()
        p = vsp.vec3d()
        ang = m.getRotationAxis( d, p )
        hb = 0.5 * ang
        qb = [ math.cos( hb ), math.sin( hb ) * d.x(), math.sin( hb ) * d.y(), math.sin( hb ) * d.z() ]
        worst = max( worst, maxdiff( quat_matrix( *q ), quat_matrix( *qb ) ) )
    assert worst < 4e-15


def vec_pair( rng, t, flip ):
    """Two vectors whose angle is exactly atan2( s, c ) for the doubles s, c used, in a random
    axis permutation and scale."""
    c = math.cos( t )
    s = math.sin( t )
    perm = rng.sample( range( 3 ), 3 )
    k0 = 2.0 ** rng.randint( -20, 20 )
    k1 = 2.0 ** rng.randint( -20, 20 )
    a = [ 0.0, 0.0, 0.0 ]
    b = [ 0.0, 0.0, 0.0 ]
    a[perm[0]] = k0
    b[perm[0]] = k1 * c * flip
    b[perm[1]] = k1 * s
    ref = math.atan2( s, c * flip )
    return vsp.vec3d( *a ), vsp.vec3d( *b ), ref, perm


@pytest.mark.parametrize( "flip", [ 1.0, -1.0 ] )
def test_angle_vec3d_near_parallel( flip ):
    rng = random.Random( 8 )
    for _ in range( 300 ):
        t = 10 ** rng.uniform( -14, -1 )
        a, b, ref, perm = vec_pair( rng, t, flip )
        ang = vsp.angle( a, b )
        assert abs( ang - ref ) <= 4e-16 * ref + 1e-300, "angle %r ref %r" % ( ang, ref )


def test_signed_angle_vec3d_near_parallel():
    rng = random.Random( 9 )
    for _ in range( 300 ):
        t = 10 ** rng.uniform( -14, -1 )
        a, b, ref, perm = vec_pair( rng, t, 1.0 )
        n = vsp.cross( a, b )
        assert abs( vsp.signed_angle( a, b, n ) - ref ) <= 4e-16 * ref
        assert abs( vsp.signed_angle( a, b, -1.0 * n ) + ref ) <= 4e-16 * ref


def test_angle_vec3d_zero_vector():
    """A zero vector is at no angle to anything, whatever the sign of its dot product's zero."""
    assert vsp.angle( vsp.vec3d( 0, 0, 0 ), vsp.vec3d( 1, 2, 3 ) ) == 0.0
    assert vsp.angle( vsp.vec3d( 0, 0, 0 ), vsp.vec3d( -1, -2, -3 ) ) == 0.0
    assert vsp.angle( vsp.vec2d( 0, 0 ), vsp.vec2d( -1, -2 ) ) == 0.0
    assert vsp.angle( vsp.vec3d( 1, 0, 0 ), vsp.vec3d( -1, 0, 0 ) ) == math.pi
    assert vsp.angle( vsp.vec3d( 1, 2, 3 ), vsp.vec3d( 2, 4, 6 ) ) == 0.0


def test_slerp_near_parallel():
    """slerp between nearly parallel vectors turns t of the way from a to b."""
    rng = random.Random( 10 )
    for _ in range( 300 ):
        th = 10 ** rng.uniform( -5.9, -1 )
        a, b, ref, perm = vec_pair( rng, th, 1.0 )
        t = rng.uniform( 0.0, 1.0 )
        r = vsp.slerp( a, b, t )
        got = math.atan2( r[perm[1]], r[perm[0]] )
        assert abs( got - t * ref ) <= 1e-15 * ref + 1e-300
        assert abs( r.mag() - 1.0 ) < 1e-15


@pytest.mark.parametrize( "flip", [ 1.0, -1.0 ] )
def test_angle_vec2d( flip ):
    rng = random.Random( 11 )
    for _ in range( 300 ):
        t = 10 ** rng.uniform( -14, -1 )
        k0 = 2.0 ** rng.randint( -20, 20 )
        k1 = 2.0 ** rng.randint( -20, 20 )
        a = vsp.vec2d( k0, 0.0 )
        b = vsp.vec2d( k1 * math.cos( t ) * flip, k1 * math.sin( t ) )
        ref = math.atan2( math.sin( t ), math.cos( t ) * flip )
        assert abs( vsp.angle( a, b ) - ref ) <= 4e-16 * ref


def test_cos_angle_vec2d_parallel():
    """cos_angle of a vector with itself or its negative is +-1, never 0."""
    rng = random.Random( 12 )
    for _ in range( 300 ):
        a = vsp.vec2d( rng.uniform( -10, 10 ), rng.uniform( -10, 10 ) )
        assert abs( vsp.cos_angle( a, a ) - 1.0 ) < 1e-15
        assert abs( vsp.cos_angle( a, -1.0 * a ) + 1.0 ) < 1e-15
        assert abs( vsp.angle( a, -1.0 * a ) - math.pi ) < 1e-15


def test_proj_vec_to_plane():
    """The projection of any vector into a plane is the vector less its part along the normal."""
    rng = random.Random( 13 )
    for _ in range( 300 ):
        v = [ x * 10 ** rng.uniform( -2, 2 ) for x in random_unit( rng ) ]
        n = random_unit( rng )
        r = vsp.proj_vec_to_plane( vsp.vec3d( *v ), vsp.vec3d( *n ) )
        d = sum( x * y for x, y in zip( v, n ) )
        ref = [ x - d * y for x, y in zip( v, n ) ]
        scale = math.sqrt( sum( x * x for x in v ) )
        assert max( abs( r[i] - ref[i] ) for i in range( 3 ) ) < 2e-15 * scale
        assert abs( vsp.dot( r, vsp.vec3d( *n ) ) ) < 2e-15 * scale


def test_plane_plane_intersection_near_parallel():
    """Two nearly parallel planes still meet on the right line."""
    rng = random.Random( 14 )
    for _ in range( 300 ):
        e = 10 ** rng.uniform( -12, -1 )
        n1y = math.sin( e )
        n1z = math.cos( e )
        h = rng.uniform( 0.5, 2.0 )
        p = vsp.vec3d()
        v = vsp.vec3d()
        ok = vsp.plane_plane_intersection( vsp.vec3d( 0, 0, 0 ), vsp.vec3d( 0, 0, 1 ),
                                           vsp.vec3d( 0, 0, h / n1z ), vsp.vec3d( 0, n1y, n1z ), p, v )
        assert ok
        # plane 0 is z = 0 and plane 1 is n1y y + n1z z = h, so the line is z = 0, y = h / n1y,
        # less what rounding h / n1z * n1z leaves of h
        y = ( h / n1z * n1z ) / n1y
        assert abs( p.z() ) <= 4e-16 * abs( y )
        assert abs( p.y() - y ) <= 4e-15 * abs( y ), "%r vs %r" % ( p.y(), y )
        assert abs( p.x() ) <= 4e-16 * abs( y )
        assert abs( abs( v.x() ) - 1.0 ) < 4e-16


def test_plane_plane_intersection_parallel():
    p = vsp.vec3d()
    v = vsp.vec3d()
    assert not vsp.plane_plane_intersection( vsp.vec3d( 0, 0, 0 ), vsp.vec3d( 0, 0, 1 ),
                                             vsp.vec3d( 0, 0, 1 ), vsp.vec3d( 0, 0, -1 ), p, v )


def naca2412( n = 61 ):
    up = []
    lo = []
    m, p, t = 0.02, 0.4, 0.12
    for i in range( n ):
        x = 0.5 * ( 1 - math.cos( math.pi * i / ( n - 1 ) ) )
        yt = 5 * t * ( 0.2969 * math.sqrt( x ) - 0.126 * x - 0.3516 * x * x + 0.2843 * x ** 3 - 0.1036 * x ** 4 )
        if x < p:
            yc = m / p ** 2 * ( 2 * p * x - x * x )
        else:
            yc = m / ( 1 - p ) ** 2 * ( 1 - 2 * p + 2 * p * x - x * x )
        up.append( ( x, yc + yt ) )
        lo.append( ( x, yc - yt ) )
    return up, lo


@pytest.mark.parametrize( "ang", [ 5.0, 89.0, 120.0, 175.0, -150.0 ] )
def test_cst_fit_theta( ang ):
    """A CST fit of a turned airfoil finds the turn, also past 90 degrees."""
    up, lo = naca2412()
    c = math.cos( math.radians( ang ) )
    s = math.sin( math.radians( ang ) )
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    xs = vsp.GetXSecSurf( w, 0 )
    vsp.ChangeXSecShape( xs, 1, vsp.XS_FILE_AIRFOIL )
    xsec = vsp.GetXSec( xs, 1 )
    vsp.SetAirfoilPnts( xsec, [ vsp.vec3d( c * x - s * y, s * x + c * y, 0 ) for x, y in up ],
                        [ vsp.vec3d( c * x - s * y, s * x + c * y, 0 ) for x, y in lo ] )
    vsp.Update()
    vsp.FitAfCST( xs, 1, 7 )
    vsp.Update()
    th = vsp.GetParmVal( vsp.GetXSecParm( vsp.GetXSec( xs, 1 ), "Theta" ) )
    assert abs( th - ang ) < 1e-10, "Theta %r for a turn of %r" % ( th, ang )
