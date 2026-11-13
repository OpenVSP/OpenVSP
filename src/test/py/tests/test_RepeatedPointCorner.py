"""A point repeated in a spline's data marks a corner: continuous there, with a slope break."""

import math

import numpy as np
import openvsp as vsp

# Values of the same curves without the repeated point, from main before repeated points made corners.
REF_PCURVE = [0.1166666666666667, 0.21755555555555559, 0.31355199999999994, 0.3172118518518519, 0.17148148148148146, 0.0866666666666667]
REF_EDIT = [15.0, 1.181126983722081, -0.3632699561622331, 15.0, -0.37840620433565936, -1.1338819043731978, 15.0, -1.2470254660685585, -0.06934558466706031, 15.0, -1.2470254660685585, 0.0693455846670603, 15.0, 0.37840620433565986, 1.1338819043731976]
REF_FUSE = [15.0, 0.5, 0.0, 15.0, 0.35719619285123666, -0.29481896632019355, 15.0, -0.12800172798849094, -0.2907715219327157, 15.0, -0.3643663995409249, 0.20506646046815893, 15.0, 0.3643663995409244, 0.20506646046815916]
REF_AIRFOIL = [5.997789247410439, 9.0, -0.02739354913369014, 5.595479573273519, 9.0, -0.05033136839899638, 5.19615242270663, 9.0, 0.0, 5.594702906376594, 9.0, 0.10050424627018056, 5.999008897180042, 9.0, 0.05447882346132437]


def angle( a, b ):
    a = a / np.linalg.norm( a )
    b = b / np.linalg.norm( b )
    return math.degrees( math.acos( min( 1.0, max( -1.0, float( np.dot( a, b ) ) ) ) ) )


def kink( fn, u, h = 1e-5 ):
    """Turn in degrees between the secants just before and just after u."""
    p = fn( u )
    return angle( p - fn( u - h ), fn( u + h ) - p )


# ==== Propeller blade parameter curve ====#

R_CORNER = [ 0.2, 0.35, 0.5, 0.6, 0.6, 0.75, 0.9, 1.0 ]
C_CORNER = [ 0.10, 0.16, 0.25, 0.32, 0.32, 0.20, 0.12, 0.05 ]


def pcurve_value( pid, r, c, rr, typ = vsp.PCHIP ):
    """The chord curve's value at rr, read from the point a split puts there."""
    vsp.SetPCurve( pid, vsp.PROP_CHORD, r, c, typ )
    vsp.Update()
    i = vsp.PCurveSplit( pid, vsp.PROP_CHORD, rr )
    vsp.Update()
    assert abs( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD )[ i ] - rr ) < 1e-12
    return vsp.PCurveGetValVec( pid, vsp.PROP_CHORD )[ i ]


def pcurve_slopes( pid, r, c, rc, h = 1e-4 ):
    left = ( pcurve_value( pid, r, c, rc - h ) - pcurve_value( pid, r, c, rc - 2 * h ) ) / h
    right = ( pcurve_value( pid, r, c, rc + 2 * h ) - pcurve_value( pid, r, c, rc + h ) ) / h
    return left, right


def test_pcurve_corner():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )

    vsp.SetPCurve( pid, vsp.PROP_CHORD, R_CORNER, C_CORNER, vsp.PCHIP )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == R_CORNER, "the repeated point kept its radius"

    # Continuous at the corner, from both sides.
    for h in ( 1e-3, 1e-4 ):
        assert abs( pcurve_value( pid, R_CORNER, C_CORNER, 0.6 - h ) - 0.32 ) < 10 * h
        assert abs( pcurve_value( pid, R_CORNER, C_CORNER, 0.6 + h ) - 0.32 ) < 10 * h

    left, right = pcurve_slopes( pid, R_CORNER, C_CORNER, 0.6 )
    assert left > 0.3 and right < -0.3, ( left, right )

    # Without the repeat the same data is smooth there.
    r0 = R_CORNER[ :4 ] + R_CORNER[ 5: ]
    c0 = C_CORNER[ :4 ] + C_CORNER[ 5: ]
    l0, r0s = pcurve_slopes( pid, r0, c0, 0.6 )
    assert abs( l0 - r0s ) < 0.05, ( l0, r0s )


def test_pcurve_linear_keeps_repeat():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    assert abs( pcurve_value( pid, R_CORNER, C_CORNER, 0.55, vsp.LINEAR ) - 0.285 ) < 1e-12
    assert abs( pcurve_value( pid, R_CORNER, C_CORNER, 0.65, vsp.LINEAR ) - 0.28 ) < 1e-12


def test_pcurve_corner_round_trip( tmp_path ):
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetPCurve( pid, vsp.PROP_CHORD, R_CORNER, C_CORNER, vsp.PCHIP )
    vsp.Update()
    fname = str( tmp_path / "pcurve_corner.vsp3" )
    vsp.WriteVSPFile( fname )

    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    pid = vsp.FindGeoms()[ 0 ]
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == R_CORNER
    assert list( vsp.PCurveGetValVec( pid, vsp.PROP_CHORD ) ) == C_CORNER


def test_pcurve_unchanged_without_repeat():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    r0 = R_CORNER[ :4 ] + R_CORNER[ 5: ]
    c0 = C_CORNER[ :4 ] + C_CORNER[ 5: ]
    vals = [ pcurve_value( pid, r0, c0, rr ) for rr in ( 0.25, 0.45, 0.58, 0.62, 0.8, 0.95 ) ]
    assert np.allclose( vals, REF_PCURVE, rtol = 0, atol = 1e-14 ), vals


# ==== Edit Curve cross section ====#

def edit_xsec( sym = vsp.SYM_NONE ):
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    surf = vsp.GetXSecSurf( fid, 0 )
    vsp.ChangeXSecShape( surf, 2, vsp.XS_EDIT_CURVE )
    xs = vsp.GetXSec( surf, 2 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "SymType" ), sym )
    vsp.EditXSecConvertTo( xs, vsp.PCHIP )
    vsp.Update()
    return fid, xs


def xsec_fn( xs ):
    def fn( u ):
        p = vsp.ComputeXSecPnt( xs, u )
        return np.array( [ p.x(), p.y(), p.z() ] )
    return fn


EDIT_U = [ 0.0, 0.125, 0.25, 0.375, 0.5, 0.5, 0.625, 0.75, 0.875, 1.0 ]


def edit_pnts( repeat ):
    pts = []
    for u in EDIT_U[ :5 ] + EDIT_U[ 6: ]:
        th = -2.0 * math.pi * u
        pts.append( vsp.vec3d( 0.5 * math.cos( th ), 0.4 * math.sin( th ), 0.0 ) )
    u = list( EDIT_U )
    if repeat:
        pts.insert( 4, pts[ 4 ] )
    else:
        del u[ 4 ]
    return u, pts


def test_edit_curve_corner():
    fid, xs = edit_xsec()
    u, pts = edit_pnts( True )
    vsp.SetEditXSecPnts( xs, u, pts, [ 0.0 ] * len( u ) )
    vsp.Update()
    assert list( vsp.GetEditXSecUVec( xs ) ) == u, "the repeated point kept its U"

    fn = xsec_fn( xs )
    corner = kink( fn, 0.5 )
    assert corner > 10.0, corner
    for h in ( 1e-3, 1e-4, 1e-5 ):
        assert np.linalg.norm( fn( 0.5 - h ) - fn( 0.5 ) ) < 10 * h
        assert np.linalg.norm( fn( 0.5 + h ) - fn( 0.5 ) ) < 10 * h

    # Smooth everywhere else.
    for uu in ( 0.1, 0.3, 0.45, 0.55, 0.7, 0.9 ):
        assert kink( fn, uu ) < 0.1, uu

    # Without the repeat the same data is smooth at the point.
    u0, pts0 = edit_pnts( False )
    vsp.SetEditXSecPnts( xs, u0, pts0, [ 0.0 ] * len( u0 ) )
    vsp.Update()
    assert kink( fn, 0.5 ) < 0.1


def test_edit_curve_corner_symmetric():
    # With left/right symmetry the right side's points are mirrored, repeats and all.
    fid, xs = edit_xsec( vsp.SYM_RL )
    u = [ 0.0, 0.125, 0.125, 0.25, 0.5, 0.75, 1.0 ]
    pts = [ vsp.vec3d( 0.5, 0.0, 0.0 ), vsp.vec3d( 0.3, -0.35, 0.0 ), vsp.vec3d( 0.3, -0.35, 0.0 ),
            vsp.vec3d( 0.0, -0.4, 0.0 ), vsp.vec3d( -0.5, 0.0, 0.0 ), vsp.vec3d( 0.0, 0.4, 0.0 ), vsp.vec3d( 0.5, 0.0, 0.0 ) ]
    vsp.SetEditXSecPnts( xs, u, pts, [ 0.0 ] * len( u ) )
    vsp.Update()
    uvec = list( vsp.GetEditXSecUVec( xs ) )
    assert uvec.count( 0.125 ) == 2 and uvec.count( 0.375 ) == 2, uvec

    fn = xsec_fn( xs )
    assert kink( fn, 0.125 ) > 20.0
    assert abs( kink( fn, 0.375 ) - kink( fn, 0.125 ) ) < 1.0


def test_edit_curve_corner_round_trip( tmp_path ):
    fid, xs = edit_xsec()
    u, pts = edit_pnts( True )
    vsp.SetEditXSecPnts( xs, u, pts, [ 0.0 ] * len( u ) )
    vsp.Update()
    fname = str( tmp_path / "edit_corner.vsp3" )
    vsp.WriteVSPFile( fname )

    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    fid = vsp.FindGeoms()[ 0 ]
    xs = vsp.GetXSec( vsp.GetXSecSurf( fid, 0 ), 2 )
    assert list( vsp.GetEditXSecUVec( xs ) ) == u
    assert kink( xsec_fn( xs ), 0.5 ) > 10.0


def test_edit_curve_unchanged_without_repeat():
    fid, xs = edit_xsec()
    u0, pts0 = edit_pnts( False )
    vsp.SetEditXSecPnts( xs, u0, pts0, [ 0.0 ] * len( u0 ) )
    vsp.Update()
    fn = xsec_fn( xs )
    vals = [ float( v ) for uu in ( 0.05, 0.3, 0.49, 0.51, 0.8 ) for v in fn( uu ) ]
    assert np.allclose( vals, REF_EDIT, rtol = 0, atol = 1e-13 ), vals


# ==== File fuselage cross section ====#

def fuse_pnts( repeat_at ):
    """A closed ellipse of 16 points from the right-hand point, the point at index repeat_at listed twice."""
    pts = []
    for i in range( 16 ):
        th = -2.0 * math.pi * i / 16
        p = vsp.vec3d( 0.5 * math.cos( th ), 0.3 * math.sin( th ), 0.0 )
        if i == 2:
            p = vsp.vec3d( 0.25, -0.35, 0.0 )
        pts.append( p )
        if i == repeat_at:
            pts.append( vsp.vec3d( p.x(), p.y(), p.z() ) )
    return pts


def file_fuse_xsec():
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    surf = vsp.GetXSecSurf( fid, 0 )
    vsp.ChangeXSecShape( surf, 2, vsp.XS_FILE_FUSE )
    return vsp.GetXSec( surf, 2 )


def test_file_fuse_corner():
    xs = file_fuse_xsec()
    vsp.SetXSecPnts( xs, fuse_pnts( 2 ) )
    vsp.Update()
    fn = xsec_fn( xs )

    # The curve turns at one point, and is continuous there.
    pc = None
    best = 0.0
    for u in np.linspace( 0.05, 0.2, 3001 ):
        k = kink( fn, u, 2e-5 )
        if k > best:
            best = k
            pc = u
    assert best > 20.0, best
    for h in ( 1e-3, 1e-4, 1e-5 ):
        assert np.linalg.norm( fn( pc - h ) - fn( pc ) ) < 10 * h
        assert np.linalg.norm( fn( pc + h ) - fn( pc ) ) < 10 * h

    # Without the repeat it is smooth everywhere.
    vsp.SetXSecPnts( xs, fuse_pnts( -1 ) )
    vsp.Update()
    worst = max( kink( fn, u, 2e-5 ) for u in np.linspace( 0.0, 1.0, 2001 ) if 1e-4 < u < 1.0 - 1e-4 )
    assert worst < 1.0, worst


def test_file_fuse_corner_at_start():
    xs = file_fuse_xsec()
    pts = fuse_pnts( -1 )
    # Listing the first point twice puts a corner at the start, where the curve closes.
    pts.insert( 0, vsp.vec3d( pts[ 0 ].x(), pts[ 0 ].y(), pts[ 0 ].z() ) )
    vsp.SetXSecPnts( xs, pts )
    vsp.Update()
    fn = xsec_fn( xs )
    h = 1e-5
    p = fn( 0.0 )
    assert np.linalg.norm( fn( 1.0 ) - p ) < 1e-12
    assert angle( p - fn( 1.0 - h ), fn( h ) - p ) > 10.0

    # Without it the closure is smooth.
    xs = file_fuse_xsec()
    vsp.SetXSecPnts( xs, fuse_pnts( -1 ) )
    vsp.Update()
    fn = xsec_fn( xs )
    p = fn( 0.0 )
    assert angle( p - fn( 1.0 - h ), fn( h ) - p ) < 0.1


def test_file_fuse_unchanged_without_repeat():
    xs = file_fuse_xsec()
    vsp.SetXSecPnts( xs, fuse_pnts( -1 ) )
    vsp.Update()
    fn = xsec_fn( xs )
    vals = [ float( v ) for uu in ( 0.0, 0.1, 0.3, 0.6, 0.9 ) for v in fn( uu ) ]
    assert np.allclose( vals, REF_FUSE, rtol = 0, atol = 1e-13 ), vals


# ==== File airfoil ====#

def wedge( x ):
    s = min( x, 1.0 - x )
    return 0.3 * s * ( 1.0 - 0.4 * s )


def airfoil_pnts( crest_repeat, le_repeat ):
    up = []
    lo = []
    m = 20
    for i in range( m + 1 ):
        x = 0.5 * ( 1.0 - math.cos( math.pi * i / m ) )
        up.append( vsp.vec3d( x, wedge( x ), 0.0 ) )
        lo.append( vsp.vec3d( x, -0.5 * wedge( x ), 0.0 ) )
        if crest_repeat and i == m // 2:
            up.append( vsp.vec3d( x, wedge( x ), 0.0 ) )
    if le_repeat:
        lo.insert( 0, vsp.vec3d( 0.0, 0.0, 0.0 ) )
    return up, lo


def file_airfoil_xsec():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    surf = vsp.GetXSecSurf( wid, 0 )
    vsp.ChangeXSecShape( surf, 1, vsp.XS_FILE_AIRFOIL )
    return vsp.GetXSec( surf, 1 )


def test_file_airfoil_corner():
    xs = file_airfoil_xsec()
    up, lo = airfoil_pnts( True, False )
    vsp.SetAirfoilPnts( xs, up, lo )
    vsp.Update()
    fn = xsec_fn( xs )

    # The crest of the upper surface is at u = 0.75.
    assert kink( fn, 0.75 ) > 15.0
    for uu in ( 0.6, 0.7, 0.8, 0.9 ):
        assert kink( fn, uu ) < 0.5, uu

    # Scaling the thickness keeps the corner.
    vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickChord" ), 0.04 )
    vsp.Update()
    assert kink( fn, 0.75 ) > 5.0

    # Without the repeat the crest is smooth.
    xs = file_airfoil_xsec()
    up, lo = airfoil_pnts( False, False )
    vsp.SetAirfoilPnts( xs, up, lo )
    vsp.Update()
    assert kink( xsec_fn( xs ), 0.75 ) < 0.5


def test_file_airfoil_le_corner( tmp_path ):
    # A Selig file whose leading edge point is listed twice gets a sharp leading edge.
    up, lo = airfoil_pnts( False, True )
    fname = str( tmp_path / "sharp_le.dat" )
    with open( fname, "w" ) as f:
        f.write( "Sharp LE\n" )
        for p in reversed( up ):
            f.write( "%.8f %.8f\n" % ( p.x(), p.y() ) )
        for p in lo[ 1: ]:
            f.write( "%.8f %.8f\n" % ( p.x(), p.y() ) )

    xs = file_airfoil_xsec()
    vsp.ReadFileAirfoil( xs, fname )
    vsp.Update()
    assert kink( xsec_fn( xs ), 0.5 ) > 20.0


def test_file_airfoil_unchanged_without_repeat():
    xs = file_airfoil_xsec()
    up, lo = airfoil_pnts( False, False )
    vsp.SetAirfoilPnts( xs, up, lo )
    vsp.Update()
    fn = xsec_fn( xs )
    vals = [ float( v ) for uu in ( 0.1, 0.3, 0.5, 0.7, 0.9 ) for v in fn( uu ) ]
    assert np.allclose( vals, REF_AIRFOIL, rtol = 0, atol = 1e-13 ), vals


# ==== G0 flags in the curve editors ====#

def pcurve_g0( pid, i ):
    return vsp.FindParm( pid, "G0_%d" % i, "Chord" )


def test_pcurve_g0_ties_moves_and_releases():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetPCurve( pid, vsp.PROP_CHORD, [ 0.2, 0.5, 0.8, 1.0 ], [ 0.1, 0.3, 0.2, 0.05 ], vsp.PCHIP )
    vsp.Update()

    # An end point ties nothing.
    vsp.SetParmValUpdate( pcurve_g0( pid, 0 ), 1.0 )
    vsp.Update()
    assert vsp.GetParmVal( pcurve_g0( pid, 0 ) ) == 0.0
    assert len( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == 4

    # G0 on a point whose next point is elsewhere puts a copy of it there, tied to it.
    vsp.SetParmValUpdate( pcurve_g0( pid, 1 ), 1.0 )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == [ 0.2, 0.5, 0.5, 0.8, 1.0 ]
    assert list( vsp.PCurveGetValVec( pid, vsp.PROP_CHORD ) ) == [ 0.1, 0.3, 0.3, 0.2, 0.05 ]
    assert vsp.GetParmVal( pcurve_g0( pid, 1 ) ) == 1.0 and vsp.GetParmVal( pcurve_g0( pid, 2 ) ) == 0.0

    # The tied point follows its point.
    vsp.SetParmValUpdate( vsp.FindParm( pid, "crd_1", "Chord" ), 0.35 )
    vsp.SetParmValUpdate( vsp.FindParm( pid, "r_1", "Chord" ), 0.55 )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == [ 0.2, 0.55, 0.55, 0.8, 1.0 ]
    assert list( vsp.PCurveGetValVec( pid, vsp.PROP_CHORD ) ) == [ 0.1, 0.35, 0.35, 0.2, 0.05 ]

    # Released, the two stay where they are and move apart.
    vsp.SetParmValUpdate( pcurve_g0( pid, 1 ), 0.0 )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == [ 0.2, 0.55, 0.55, 0.8, 1.0 ]
    vsp.SetParmValUpdate( vsp.FindParm( pid, "r_2", "Chord" ), 0.65 )
    vsp.SetParmValUpdate( vsp.FindParm( pid, "crd_2", "Chord" ), 0.25 )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == [ 0.2, 0.55, 0.65, 0.8, 1.0 ]
    assert list( vsp.PCurveGetValVec( pid, vsp.PROP_CHORD ) ) == [ 0.1, 0.35, 0.25, 0.2, 0.05 ]

    # Tied again where the next point is elsewhere, another copy is put in.
    vsp.SetParmValUpdate( pcurve_g0( pid, 1 ), 1.0 )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == [ 0.2, 0.55, 0.55, 0.65, 0.8, 1.0 ]

    # Deleting the tied point unties its point.
    vsp.PCurveDeletePt( pid, vsp.PROP_CHORD, 2 )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == [ 0.2, 0.55, 0.65, 0.8, 1.0 ]
    assert vsp.GetParmVal( pcurve_g0( pid, 1 ) ) == 0.0


def test_pcurve_g0_ties_coincident_points():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    r = [ 0.2, 0.5, 0.5, 0.8, 1.0 ]
    c = [ 0.1, 0.3, 0.3, 0.2, 0.05 ]
    vsp.SetPCurve( pid, vsp.PROP_CHORD, r, c, vsp.PCHIP )
    vsp.Update()

    # Points already together are tied as they are.
    vsp.SetParmValUpdate( pcurve_g0( pid, 1 ), 1.0 )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == r


def test_pcurve_points_meet_by_hand():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetPCurve( pid, vsp.PROP_CHORD, [ 0.2, 0.5, 0.8, 1.0 ], [ 0.1, 0.3, 0.2, 0.05 ], vsp.PCHIP )
    vsp.Update()

    # A point slides all the way to its neighbor's radius, but not past it.
    vsp.SetParmValUpdate( vsp.FindParm( pid, "r_2", "Chord" ), 0.4 )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == [ 0.2, 0.5, 0.5, 1.0 ]

    # At one radius with two values the curve steps there.
    t = vsp.PCurveGetTVec( pid, vsp.PROP_CHORD )
    v = vsp.PCurveGetValVec( pid, vsp.PROP_CHORD )
    i = vsp.PCurveSplit( pid, vsp.PROP_CHORD, 0.7 )
    vsp.Update()
    assert math.isfinite( vsp.PCurveGetValVec( pid, vsp.PROP_CHORD )[ i ] )
    vsp.SetPCurve( pid, vsp.PROP_CHORD, t, v, vsp.PCHIP )
    vsp.Update()

    # Given the same value too, the two are a corner, untied.
    vsp.SetParmValUpdate( vsp.FindParm( pid, "crd_2", "Chord" ), 0.3 )
    vsp.Update()
    assert vsp.GetParmVal( pcurve_g0( pid, 1 ) ) == 0.0
    vsp.SetParmValUpdate( vsp.FindParm( pid, "crd_1", "Chord" ), 0.28 )
    vsp.Update()
    assert list( vsp.PCurveGetValVec( pid, vsp.PROP_CHORD ) ) == [ 0.1, 0.28, 0.3, 0.05 ]


def test_pcurve_g0_round_trip( tmp_path ):
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetPCurve( pid, vsp.PROP_CHORD, [ 0.2, 0.5, 0.8, 1.0 ], [ 0.1, 0.3, 0.2, 0.05 ], vsp.PCHIP )
    vsp.Update()
    vsp.SetParmValUpdate( pcurve_g0( pid, 1 ), 1.0 )
    vsp.Update()
    fname = str( tmp_path / "pcurve_g0.vsp3" )
    vsp.WriteVSPFile( fname )

    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    pid = vsp.FindGeoms()[ 0 ]
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == [ 0.2, 0.5, 0.5, 0.8, 1.0 ]
    assert vsp.GetParmVal( pcurve_g0( pid, 1 ) ) == 1.0
    vsp.SetParmValUpdate( vsp.FindParm( pid, "crd_1", "Chord" ), 0.33 )
    vsp.Update()
    assert list( vsp.PCurveGetValVec( pid, vsp.PROP_CHORD ) ) == [ 0.1, 0.33, 0.33, 0.2, 0.05 ]


def test_edit_curve_g0_ties_moves_and_releases():
    fid, xs = edit_xsec()
    u, pts = edit_pnts( False )
    vsp.SetEditXSecPnts( xs, u, pts, [ 0.0 ] * len( u ) )
    vsp.Update()
    fn = xsec_fn( xs )
    assert kink( fn, 0.5 ) < 0.1

    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "G0_4" ), 1.0 )
    vsp.Update()
    assert list( vsp.GetEditXSecUVec( xs ) ) == EDIT_U
    assert kink( fn, 0.5 ) > 10.0

    # Moving either point moves both, so the corner stays a corner.
    vsp.MoveEditXSecPnt( xs, 5, vsp.vec3d( -0.6, 0.05, 0.0 ) )
    vsp.Update()
    p = vsp.GetEditXSecCtrlVec( xs, True )
    assert ( p[ 4 ].x(), p[ 4 ].y() ) == ( p[ 5 ].x(), p[ 5 ].y() ) == ( -0.6, 0.05 )

    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "X_4" ), -0.55 )
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "U_4" ), 0.52 )
    vsp.Update()
    p = vsp.GetEditXSecCtrlVec( xs, True )
    uv = vsp.GetEditXSecUVec( xs )
    assert p[ 4 ].x() == p[ 5 ].x() == -0.55 and uv[ 4 ] == uv[ 5 ] == 0.52
    assert kink( fn, 0.52 ) > 10.0

    # Released, each point moves alone.
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "G0_4" ), 0.0 )
    vsp.Update()
    vsp.MoveEditXSecPnt( xs, 5, vsp.vec3d( -0.5, 0.1, 0.0 ) )
    vsp.Update()
    p = vsp.GetEditXSecCtrlVec( xs, True )
    assert ( p[ 4 ].x(), p[ 4 ].y() ) == ( -0.55, 0.05 ) and ( p[ 5 ].x(), p[ 5 ].y() ) == ( -0.5, 0.1 )

    # Deleting the tied point unties its point.
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "G0_4" ), 1.0 )
    vsp.Update()
    n = len( vsp.GetEditXSecUVec( xs ) )
    vsp.EditXSecDelPnt( xs, 5 )
    vsp.Update()
    assert len( vsp.GetEditXSecUVec( xs ) ) == n - 1
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "G0_4" ) ) == 0.0


def test_edit_curve_g0_symmetric():
    # On a symmetric curve, a corner on the right is mirrored on the left.
    fid, xs = edit_xsec( vsp.SYM_RL )
    u = [ 0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1.0 ]
    pts = [ vsp.vec3d( 0.5, 0.0, 0.0 ), vsp.vec3d( 0.3, -0.35, 0.0 ), vsp.vec3d( 0.0, -0.4, 0.0 ), vsp.vec3d( -0.3, -0.35, 0.0 ),
            vsp.vec3d( -0.5, 0.0, 0.0 ), vsp.vec3d( -0.3, 0.35, 0.0 ), vsp.vec3d( 0.0, 0.4, 0.0 ), vsp.vec3d( 0.3, 0.35, 0.0 ),
            vsp.vec3d( 0.5, 0.0, 0.0 ) ]
    vsp.SetEditXSecPnts( xs, u, pts, [ 0.0 ] * len( u ) )
    vsp.Update()

    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "G0_1" ), 1.0 )  # u = 0.125
    vsp.Update()
    uvec = list( vsp.GetEditXSecUVec( xs ) )
    assert uvec.count( 0.125 ) == 2 and uvec.count( 0.375 ) == 2, uvec

    # The first copy holds each flag.
    g0 = [ vsp.GetParmVal( vsp.GetXSecParm( xs, "G0_%d" % i ) ) for i in range( len( uvec ) ) ]
    assert [ uvec[ i ] for i in range( len( uvec ) ) if g0[ i ] ] == [ 0.125, 0.375 ], ( uvec, g0 )

    fn = xsec_fn( xs )
    assert kink( fn, 0.125 ) > 10.0
    assert abs( kink( fn, 0.375 ) - kink( fn, 0.125 ) ) < 1.0


def test_edit_curve_g0_at_closure():
    fid, xs = edit_xsec()
    u, pts = edit_pnts( False )
    vsp.SetEditXSecPnts( xs, u, pts, [ 0.0 ] * len( u ) )
    vsp.Update()
    fn = xsec_fn( xs )
    h = 1e-5
    p = fn( 0.0 )
    assert angle( p - fn( 1.0 - h ), fn( h ) - p ) < 0.1

    # The last point of a closed curve is its first, so it ties nothing.
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "G0_%d" % ( len( u ) - 1 ) ), 1.0 )
    vsp.Update()
    assert len( vsp.GetEditXSecUVec( xs ) ) == len( u )

    # The first point makes the corner where the curve closes.
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "G0_0" ), 1.0 )
    vsp.Update()
    uv = vsp.GetEditXSecUVec( xs )
    assert uv[ 0 ] == uv[ 1 ] == 0.0, uv
    p = fn( 0.0 )
    assert angle( p - fn( 1.0 - h ), fn( h ) - p ) > 10.0


def test_edit_curve_linear_radii_skip_a_corner():
    # A Linear curve rounds each joint with its own point's radius, whatever corners come before it.
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    surf = vsp.GetXSecSurf( fid, 0 )
    vsp.ChangeXSecShape( surf, 2, vsp.XS_EDIT_CURVE )
    xs = vsp.GetXSec( surf, 2 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "SymType" ), vsp.SYM_NONE )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "ShapeType" ), vsp.EDIT_XSEC_RECTANGLE )
    vsp.EditXSecInitShape( xs )
    vsp.Update()
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "R_5" ), 0.1 )
    vsp.Update()
    fn = xsec_fn( xs )
    before = [ fn( uu ) for uu in np.linspace( 0.55, 0.7, 31 ) ]

    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "G0_2" ), 1.0 )
    vsp.Update()
    assert len( vsp.GetEditXSecUVec( xs ) ) == 10
    after = [ fn( uu ) for uu in np.linspace( 0.55, 0.7, 31 ) ]
    assert np.allclose( before, after, rtol = 0, atol = 1e-12 )


def test_pcurve_g0_tied_point_leads_too():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetPCurve( pid, vsp.PROP_CHORD, [ 0.2, 0.5, 0.8, 1.0 ], [ 0.1, 0.3, 0.2, 0.05 ], vsp.PCHIP )
    vsp.Update()
    vsp.SetParmValUpdate( pcurve_g0( pid, 1 ), 1.0 )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == [ 0.2, 0.5, 0.5, 0.8, 1.0 ]

    # Setting the tied point moves its point too.
    vsp.SetParmValUpdate( vsp.FindParm( pid, "crd_2", "Chord" ), 0.33 )
    vsp.SetParmValUpdate( vsp.FindParm( pid, "r_2", "Chord" ), 0.6 )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == [ 0.2, 0.6, 0.6, 0.8, 1.0 ]
    assert list( vsp.PCurveGetValVec( pid, vsp.PROP_CHORD ) ) == [ 0.1, 0.33, 0.33, 0.2, 0.05 ]
    assert vsp.GetParmVal( pcurve_g0( pid, 1 ) ) == 1.0


def test_pcurve_g0_finds_the_point_before():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    r = [ 0.2, 0.5, 0.5 + 1e-9, 0.8, 1.0 ]
    c = [ 0.1, 0.3, 0.3, 0.2, 0.05 ]
    vsp.SetPCurve( pid, vsp.PROP_CHORD, r, c, vsp.PCHIP )
    vsp.Update()

    # G0 on the second of two points at one place ties it to the first; nothing is added.
    vsp.SetParmValUpdate( pcurve_g0( pid, 2 ), 1.0 )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == [ 0.2, 0.5, 0.5, 0.8, 1.0 ]
    assert vsp.GetParmVal( pcurve_g0( pid, 1 ) ) == 1.0 and vsp.GetParmVal( pcurve_g0( pid, 2 ) ) == 0.0

    # G0 on the first of two points nearly at one place ties the second; nothing is added.
    vsp.SetParmValUpdate( pcurve_g0( pid, 1 ), 0.0 )
    vsp.Update()
    vsp.SetPCurve( pid, vsp.PROP_CHORD, r, c, vsp.PCHIP )
    vsp.Update()
    vsp.SetParmValUpdate( pcurve_g0( pid, 1 ), 1.0 )
    vsp.Update()
    assert list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) ) == [ 0.2, 0.5, 0.5, 0.8, 1.0 ]


def edit_ctrl( xs ):
    return [ ( p.x(), p.y() ) for p in vsp.GetEditXSecCtrlVec( xs, True ) ]


def test_edit_curve_g0_same_xy_other_u():
    fid, xs = edit_xsec()
    u, pts = edit_pnts( True )
    u[ 5 ] = 0.55
    vsp.SetEditXSecPnts( xs, u, pts, [ 0.0 ] * len( u ) )
    vsp.Update()
    n = len( vsp.GetEditXSecUVec( xs ) )

    # The next point is at the same X and Y, so it is tied as it is, taking the point's U.
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "G0_4" ), 1.0 )
    vsp.Update()
    uv = vsp.GetEditXSecUVec( xs )
    assert len( uv ) == n and uv[ 4 ] == uv[ 5 ] == 0.5, uv


def test_edit_curve_g0_finds_the_point_before():
    fid, xs = edit_xsec()
    u, pts = edit_pnts( True )
    u[ 5 ] = 0.55
    vsp.SetEditXSecPnts( xs, u, pts, [ 0.0 ] * len( u ) )
    vsp.Update()
    n = len( vsp.GetEditXSecUVec( xs ) )

    # G0 on the second of two points at one X and Y ties it to the first.
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "G0_5" ), 1.0 )
    vsp.Update()
    uv = vsp.GetEditXSecUVec( xs )
    assert len( uv ) == n and uv[ 4 ] == uv[ 5 ] == 0.5, uv
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "G0_4" ) ) == 1.0
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "G0_5" ) ) == 0.0
    assert kink( xsec_fn( xs ), 0.5 ) > 10.0


def test_edit_curve_g0_tied_point_leads_too():
    fid, xs = edit_xsec()
    u, pts = edit_pnts( False )
    vsp.SetEditXSecPnts( xs, u, pts, [ 0.0 ] * len( u ) )
    vsp.Update()
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "G0_4" ), 1.0 )
    vsp.Update()

    # Setting the tied point's X, Y or U moves its point too.
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "X_5" ), -0.55 )
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "Y_5" ), 0.05 )
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "U_5" ), 0.52 )
    vsp.Update()
    p = edit_ctrl( xs )
    uv = vsp.GetEditXSecUVec( xs )
    assert p[ 4 ] == p[ 5 ] == ( -0.55, 0.05 ), p
    assert uv[ 4 ] == uv[ 5 ] == 0.52, uv
    assert kink( xsec_fn( xs ), 0.52 ) > 10.0


def test_pcurve_g0_before_a_corner_keeps_it():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetPCurve( pid, vsp.PROP_CHORD, [ 0.2, 0.35, 0.5, 0.8, 1.0 ], [ 0.1, 0.16, 0.3, 0.2, 0.05 ], vsp.PCHIP )
    vsp.Update()
    vsp.SetParmValUpdate( pcurve_g0( pid, 2 ), 1.0 )
    vsp.Update()

    # G0 on the point before the corner puts in a copy; the corner keeps its flag.
    vsp.SetParmValUpdate( pcurve_g0( pid, 1 ), 1.0 )
    vsp.Update()
    t = list( vsp.PCurveGetTVec( pid, vsp.PROP_CHORD ) )
    assert t == [ 0.2, 0.35, 0.35, 0.5, 0.5, 0.8, 1.0 ], t
    g0 = [ vsp.GetParmVal( pcurve_g0( pid, i ) ) for i in range( len( t ) ) ]
    assert g0 == [ 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0 ], g0


def test_edit_curve_g0_before_a_corner_keeps_it():
    fid, xs = edit_xsec()
    u, pts = edit_pnts( False )
    vsp.SetEditXSecPnts( xs, u, pts, [ 0.0 ] * len( u ) )
    vsp.Update()
    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "G0_4" ), 1.0 )
    vsp.Update()

    vsp.SetParmValUpdate( vsp.GetXSecParm( xs, "G0_3" ), 1.0 )
    vsp.Update()
    uv = list( vsp.GetEditXSecUVec( xs ) )
    assert uv == [ 0.0, 0.125, 0.25, 0.375, 0.375, 0.5, 0.5, 0.625, 0.75, 0.875, 1.0 ], uv
    g0 = [ vsp.GetParmVal( vsp.GetXSecParm( xs, "G0_%d" % i ) ) for i in range( len( uv ) ) ]
    assert g0 == [ 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0 ], g0
