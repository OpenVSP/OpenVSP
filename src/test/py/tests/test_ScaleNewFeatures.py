# Scaling the end caps, the DATCOM components and the biconvex airfoil.
#
# Scale multiplies a Geom about its own origin: every point of its surface goes to s times as far from the origin,
# along the same direction in the Geom's frame.  Each dimensional Parm is multiplied by s and each ratio, angle, count
# and flag is left alone.  These tests scale by 2 and by 0.5, compare densely sampled points, and list the Parms the
# scale changed.

import os
import tempfile

import openvsp as vsp
import pytest

from testhelp import drop_errors, assert_no_errors

SCALES = ( 2.0, 0.5 )
PLACE = ( ( "X_Rel_Location", 1.3 ), ( "Y_Rel_Location", 0.2 ), ( "Z_Rel_Location", -0.4 ),
          ( "X_Rel_Rotation", 5.0 ), ( "Y_Rel_Rotation", 7.0 ), ( "Z_Rel_Rotation", -3.0 ) )

# The Parms that are not inputs: results the Geom measures, and the scale itself.
MEASURED_GROUPS = ( "BBox", "XForm" )
MEASURED_NAMES = ( "Area", "TotalArea", "CurvedArea" )

# The counts that make the Parms of each station, panel or member, set before those Parms.
COUNTS = ( "PTYPE", "NLOC", "NIN", "NSPAN", "NPANEL" )


def xform( gid, name ):
    return vsp.FindParm( gid, name, "XForm" )


def place( gid ):
    for name, val in PLACE:
        vsp.SetParmVal( xform( gid, name ), val )
    vsp.Update()


def origin( gid ):
    return tuple( vsp.GetParmVal( xform( gid, n ) ) for n in ( "X_Location", "Y_Location", "Z_Location" ) )


def flat( pts ):
    return [ c for p in pts for c in p ]


def sample( gid, n=21 ):
    pts = []
    for s in range( vsp.GetNumMainSurfs( gid ) ):
        for i in range( n ):
            for j in range( n ):
                p = vsp.CompPnt01( gid, s, i / ( n - 1 ), j / ( n - 1 ) )
                pts.append( ( p.x(), p.y(), p.z() ) )
    return pts


def parms( gid, xsecs=True ):
    """Every input Parm of the Geom and its subsurfaces, and of its cross sections where xsecs is on, by a name that
    tells them apart."""
    vals = {}

    def take( prefix, pid ):
        group = vsp.GetParmGroupName( pid )
        name = vsp.GetParmName( pid )
        if group in MEASURED_GROUPS or name in MEASURED_NAMES:
            return
        vals[ "%s%s:%s" % ( prefix, group, name ) ] = vsp.GetParmVal( pid )

    for pid in vsp.GetGeomParmIDs( gid ):
        take( "", pid )
    for k in range( vsp.GetNumXSecSurfs( gid ) * xsecs ):
        xss = vsp.GetXSecSurf( gid, k )
        for i in range( vsp.GetNumXSec( xss ) ):
            for pid in vsp.GetXSecParmIDs( vsp.GetXSec( xss, i ) ):
                take( "xsec %d %d " % ( k, i ), pid )
    return vals


def scale( gid, s ):
    vsp.SetParmVal( xform( gid, "Scale" ), s )
    vsp.Update()


def assert_scales( gid, s, ratios=True, xsecs=True ):
    """Scales gid by s and checks its surface went with it.  Answers the Parms that changed, each by s unless ratios
    is off, the cross sections' among them where xsecs is on."""
    o = origin( gid )
    before = sample( gid )
    pb = parms( gid, xsecs )
    scale( gid, s )
    after = sample( gid )
    pa = parms( gid, xsecs )

    assert len( after ) == len( before )
    size = max( max( abs( c - oc ) for c, oc in zip( p, o ) ) for p in before )
    for p, q in zip( before, after ):
        want = tuple( oc + s * ( c - oc ) for c, oc in zip( p, o ) )
        assert q == pytest.approx( want, abs=1e-9 * size )

    assert set( pa ) == set( pb )
    changed = set()
    for name in pb:
        if pa[name] == pytest.approx( pb[name], rel=1e-9, abs=1e-12 ):
            continue
        changed.add( name )
        if ratios:
            assert pa[name] == pytest.approx( s * pb[name], rel=1e-9 ), "%s went from %g to %g" % ( name, pb[name],
                                                                                              pa[name] )
    return changed


#===== End caps =====#

CAP_TYPES = tuple( range( vsp.NUM_END_CAP_OPTIONS ) )

# name: Parms set on both ends, beyond the type, the Rel flag and its sizes.
MODIFIERS = {
    "plain": {},
    "truncated": { "TruncFlag": 1 },
    "sheared": { "ShearFlag": 1 },
    "scaled rows": { "ScaledFlag": 1 },
    "angle and exponent": { "OgiveAngle": 5.0, "PowerExp": 0.6, "HaackC": 0.2 },
    "swept": { "SweepFlag": 1 },
}

REL_SIZES = { "Length": 0.8, "Offset": 0.2, "Radius": 0.1 }
ABS_SIZES = { "LengthDim": 1.5, "OffsetDim": 0.1, "RadiusDim": 0.05 }

DIM_CAP_PARMS = ( "LengthDim", "OffsetDim", "RadiusDim" )
NONDIM_CAP_PARMS = ( "Option", "RelFlag", "Length", "Offset", "Radius", "OgiveAngle", "PowerExp", "HaackC",
                     "TruncFlag", "ShearFlag", "ScaledFlag", "SweepFlag", "Strength" )


def cap_parm( gid, end, name ):
    return vsp.FindParm( gid, "Cap" + end + name, "EndCap" )


def capped_body( typ, cap_type, rel, mod, nose ):
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( typ )
    vsp.Update()
    place( gid )
    vals = { "Option": cap_type, "RelFlag": rel }
    if rel:
        vals.update( REL_SIZES )
    else:
        vals.update( ABS_SIZES )
    vals.update( MODIFIERS[mod] )
    for end in ( "UMin", "UMax" ):
        for name, val in vals.items():
            vsp.SetParmVal( cap_parm( gid, end, name ), val )
    if nose:
        for flag in ( "NoseOriginXFlag", "NoseOriginYFlag", "NoseOriginZFlag" ):
            vsp.SetParmVal( vsp.FindParm( gid, flag, "Design" ), True )
    vsp.Update()
    return gid


@pytest.mark.parametrize( "mod", sorted( MODIFIERS ) )
@pytest.mark.parametrize( "rel", ( True, False ) )
@pytest.mark.parametrize( "cap_type", CAP_TYPES )
@pytest.mark.parametrize( "typ", ( "FUSELAGE", "STACK" ) )
def testACappedBodyScalesAboutItsOrigin( typ, cap_type, rel, mod ):
    """The caps scale with the body, Rel or Abs, and their dimensional sizes scale while their ratios, angles and
    flags stay put."""
    for s in SCALES:
        gid = capped_body( typ, cap_type, rel, mod, False )
        caps = { ( end, n ): vsp.GetParmVal( cap_parm( gid, end, n ) ) for end in ( "UMin", "UMax" )
                 for n in DIM_CAP_PARMS + NONDIM_CAP_PARMS }
        changed = assert_scales( gid, s )

        for ( end, n ), val in caps.items():
            now = vsp.GetParmVal( cap_parm( gid, end, n ) )
            if n in DIM_CAP_PARMS:
                assert now == pytest.approx( s * val, rel=1e-9, abs=1e-12 ), "Cap%s%s" % ( end, n )
            else:
                assert now == pytest.approx( val, rel=1e-9, abs=1e-12 ), "Cap%s%s" % ( end, n )

        # Of the caps' Parms, only the dimensional sizes changed.
        for name in changed:
            if name.startswith( "EndCap:" ):
                assert name.endswith( "Dim" ), name
        assert_no_errors()


@pytest.mark.parametrize( "cap_type", ( vsp.CONE_END_CAP, vsp.OGIVE_END_CAP, vsp.POWER_LAW_END_CAP ) )
@pytest.mark.parametrize( "typ", ( "FUSELAGE", "STACK" ) )
def testANoseAtTheOriginStaysThereWhenScaled( typ, cap_type ):
    for s in SCALES:
        gid = capped_body( typ, cap_type, False, "plain", True )
        assert_scales( gid, s )
        assert_no_errors()


#===== The DATCOM components =====#

def component( name, **vals ):
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( name )
    put( gid, **vals )
    place( gid )
    return gid


def put( gid, **vals ):
    for name in COUNTS:
        if name in vals:
            vsp.SetParmVal( vsp.FindParm( gid, name, "Design" ), vals[name] )
            vsp.Update()
    for name, val in vals.items():
        if name not in COUNTS:
            pid = vsp.FindParm( gid, name, "Design" )
            assert pid, "no Parm " + name
            vsp.SetParmVal( pid, val )
    vsp.Update()


BOD_LENGTHS = ( "LNOSE", "DNOSE", "WNOSE", "BNOSE", "ZTIP", "ZNOSE", "LCENTR", "DCENTR", "WCENTR", "ZCENTR",
                "LAFT", "DAFT", "WAFT", "ZAFT", "DEXIT" )

def members( n ):
    """The length, width and height of each of a protuberance's n members."""
    return tuple( "%s_%d" % ( name, k ) for name in ( "LPROT", "WPROT", "HPROT" ) for k in range( 1, n + 1 ) )


# name: ( type, inputs, the Design Parms that are lengths and are not zero )
COMPONENTS = {
    "cambered power law body": ( "MDC_BOD", dict( TNOSE=2, POWER=0.6, BNOSE=0.1, LAFT=2.0, DAFT=0.6, ZTIP=0.1,
                                                   ZNOSE=0.05, ZCENTR=0.2, ZAFT=0.1, DEXIT=0.3 ), BOD_LENGTHS ),
    "elliptic truncated ogive body": ( "MDC_BOD", dict( NAMELIST=1, WNOSE=2.0, ENOSE=0.5, BNOSE=0.2, TRUNC=1,
                                                         CENTRDefault=0, WCENTR=2.2, ECENTR=0.6, LAFT=2.0, TAFT=1,
                                                         SLPTAL=0.1, WAFT=1.5 ),
                                       ( "LNOSE", "DNOSE", "WNOSE", "BNOSE", "LCENTR", "DCENTR", "WCENTR", "LAFT",
                                         "DAFT", "WAFT" ) ),
    "secant ogive body": ( "MDC_BOD", dict( SLPNOS=0.1, LAFT=1.0, DAFT=1.2 ),
                           ( "LNOSE", "DNOSE", "WNOSE", "LCENTR", "DCENTR", "WCENTR", "LAFT", "DAFT", "WAFT" ) ),
    "lugs": ( "MDC_PROTUB", dict( PTYPE=3, NLOC=2, RBODY=1.2, LPROT_1=0.5, WPROT_1=0.3, HPROT_1=0.2,
                                  OPROT_2=0.05 ),
              ( "RBODY", "OPROT_2" ) + members( 4 ) ),
    "shoe": ( "MDC_PROTUB", dict( PTYPE=4, NLOC=3, PHIPROFlag=1, PHIPRO_2=100.0, RBODY=1.2, LPROT_2=0.7 ),
              ( "RBODY", ) + members( 3 ) ),
    "block": ( "MDC_PROTUB", dict( PTYPE=5, NLOC=2, RBODY=1.0, LPROT_1=0.4, WPROT_1=0.2, HPROT_1=0.3, OPROT_1=0.1 ),
               ( "RBODY", "LPROT_1", "WPROT_1", "HPROT_1", "OPROT_1" ) ),
    "fairing": ( "MDC_PROTUB", dict( PTYPE=6, RBODY=6.0, LPROT_1=5.0, WPROT_1=2.0, HPROT_1=1.0 ),
                 ( "RBODY", "LPROT_1", "WPROT_1", "HPROT_1" ) ),
    "covered axisymmetric inlets": ( "MDC_INLET", dict( INTYPE=3, NIN=2, COVER=1, RAMP=20.0 ),
                                     ( "RBODY", "XDIV", "HDIV", "LDIV", "W1", "W2", "W3", "W4", "W5", "X2", "X3",
                                       "X4", "X5", "H2", "H3", "H4", "H5" ) ),
    "2D inlet": ( "MDC_INLET", dict( INTYPE=1, HDIV=0.0, PHIFlag=1, PHI_1=30.0 ),
                  ( "RBODY", "XDIV", "LDIV", "W1", "W2", "W3", "W4", "W5", "X2", "X3", "X4", "X5", "H2", "H3", "H4",
                    "H5" ) ),
}

FIN_STATIONS = dict( NPANEL=3, NSPAN=3, SSPAN_2=1.0, SSPAN_3=2.0, CHORD_3=0.5, SWEEP_2=20.0, CFOC_1=0.3,
                     CFOC_2=0.3, CFOC_3=0.2, LER_1=0.01, LER_2=0.01, LER_3=0.01, GAM_2=10.0 )
FIN_LENGTHS = ( "RBODY", "SSPAN_2", "SSPAN_3", "CHORD_1", "CHORD_2", "CHORD_3", "XLE_2", "XLE_3", "LER_1", "LER_2",
                "LER_3" )
for sectyp, sname in enumerate( ( "HEX", "NACA", "ARC", "USER" ) ):
    COMPONENTS[ "fin set of %s sections" % sname ] = ( "MDC_FIN", dict( FIN_STATIONS, SECTYP=sectyp ), FIN_LENGTHS )
COMPONENTS[ "fin set of supersonic hexagonal sections" ] = ( "MDC_FIN", dict( FIN_STATIONS, SECTYP=1, NACASER=8,
                                                                               NACADES=6.0, NACAFLAT=20.0 ),
                                                             FIN_LENGTHS )


@pytest.mark.parametrize( "case", sorted( COMPONENTS ) )
def testAComponentScalesItsLengthsOnly( case ):
    """Each DATCOM component scales about its origin, and the Parms that changed are its lengths and nothing else."""
    typ, vals, lengths = COMPONENTS[case]
    for s in SCALES:
        gid = component( typ, **vals )
        # The component builds its cross sections from its inputs on every update.
        changed = assert_scales( gid, s, xsecs=False )
        design = { n.split( ":" )[-1] for n in changed if n.startswith( "Design:" ) }
        assert design == set( lengths )

        # Else only the sizes of the caps and of the flaps, which the component sets from its shape.
        for name in changed:
            if name.startswith( "Design:" ):
                continue
            assert name.startswith( "EndCap:" ) or name.startswith( "SS_Control:" ), name
        assert_no_errors()


def testAScaledComponentReadsBackScaled():
    for typ, vals, lengths in COMPONENTS.values():
        gid = component( typ, **vals )
        unscaled = sample( gid )
        scale( gid, 2.0 )
        scaled = sample( gid )

        path = os.path.join( tempfile.mkdtemp(), "scaled.vsp3" )
        vsp.WriteVSPFile( path, vsp.SET_ALL )
        vsp.VSPRenew()
        vsp.ReadVSPFile( path )
        vsp.Update()
        back = vsp.FindGeoms()[0]
        assert flat( sample( back ) ) == pytest.approx( flat( scaled ), abs=1e-12 )

        scale( back, 1.0 )
        assert flat( sample( back ) ) == pytest.approx( flat( unscaled ), abs=1e-12 )
        assert_no_errors()


#===== On a body =====#

def body( size ):
    gid = vsp.AddGeom( "MDC_BOD" )
    put( gid, DNOSE=1.0 * size, CENTRDefault=0, DCENTR=2.0 * size, LNOSE=2.0 * size, LCENTR=8.0 * size )
    return gid


def child( name, parent, x, **vals ):
    gid = vsp.AddGeom( name, parent )
    vsp.SetParmVal( xform( gid, "X_Rel_Location" ), x )
    put( gid, **vals )
    return gid


CHILDREN = ( ( "MDC_PROTUB", dict( PTYPE=6, LPROT_1=0.8, WPROT_1=0.3, HPROT_1=0.2 ) ),
             ( "MDC_INLET", dict( INTYPE=3, NIN=2 ) ),
             ( "MDC_FIN", dict( NPANEL=4 ) ) )


@pytest.mark.parametrize( "name,vals", CHILDREN )
def testAScaledBodyGivesItsChildrenTheRadiusThere( name, vals ):
    """A child stays where it was put along the scaled body, and takes the scaled body's radius there."""
    vsp.VSPRenew()
    drop_errors()
    big = body( 2.0 )
    wanted = child( name, big, 3.0, **vals )

    small = body( 1.0 )
    gid = child( name, small, 3.0, **vals )
    scale( small, 2.0 )

    rbody = vsp.GetParmVal( vsp.FindParm( gid, "RBODY", "Design" ) )
    assert rbody == pytest.approx( vsp.GetParmVal( vsp.FindParm( wanted, "RBODY", "Design" ) ), rel=1e-6 )
    assert flat( sample( gid ) ) == pytest.approx( flat( sample( wanted ) ), abs=1e-6 )
    assert_no_errors()


def testAScaledChildKeepsStandingOnItsBody():
    """A child's own sizes scale, and its RBODY stays the body's radius under it."""
    vsp.VSPRenew()
    drop_errors()
    parent = body( 1.0 )
    gid = child( "MDC_PROTUB", parent, 3.0, PTYPE=5, LPROT_1=0.4, WPROT_1=0.2, HPROT_1=0.3, OPROT_1=0.1 )
    rbody = vsp.GetParmVal( vsp.FindParm( gid, "RBODY", "Design" ) )
    scale( gid, 2.0 )
    assert vsp.GetParmVal( vsp.FindParm( gid, "RBODY", "Design" ) ) == pytest.approx( rbody )
    for name, val in ( ( "LPROT_1", 0.8 ), ( "WPROT_1", 0.4 ), ( "HPROT_1", 0.6 ), ( "OPROT_1", 0.2 ) ):
        assert vsp.GetParmVal( vsp.FindParm( gid, name, "Design" ) ) == pytest.approx( val )
    assert_no_errors()


#===== Converted components =====#

def testAScaledBodyConvertsToTheScaledStack():
    for s in SCALES:
        gid = component( "MDC_BOD", TNOSE=2, POWER=0.6, LAFT=2.0, DAFT=0.6 )
        scale( gid, s )
        stack = vsp.ConvertMdcBodToStack( gid, False )
        assert flat( sample( stack ) ) == pytest.approx( flat( sample( gid ) ), abs=1e-9 )

        # Both scaled back are the same again.
        scale( gid, 1.0 )
        scale( stack, 1.0 )
        assert flat( sample( stack ) ) == pytest.approx( flat( sample( gid ) ), abs=1e-9 )
        assert_no_errors()


#===== The biconvex airfoil =====#

def biconvex_wing():
    vsp.VSPRenew()
    drop_errors()
    wid = vsp.AddGeom( "WING" )
    xss = vsp.GetXSecSurf( wid, 0 )
    for i in range( 2 ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_BICONVEX )
        xs = vsp.GetXSec( xss, i )
        for name, val in ( ( "ThickChord", 0.08 ), ( "ThickFracUp", 0.7 ), ( "ThickLoc", 0.3 ),
                           ( "ThickLocLow", 0.6 ) ):
            vsp.SetParmVal( vsp.GetXSecParm( xs, name ), val )
    vsp.Update()
    place( wid )
    return wid


def testABiconvexWingScalesAboutItsOrigin():
    for s in SCALES:
        wid = biconvex_wing()
        # A Wing's first section has Parms found from the planform, such as its aspect ratio.
        changed = assert_scales( wid, s, False )
        for name in changed:
            assert not any( n in name for n in ( "ThickChord", "ThickFracUp", "ThickLoc" ) ), name
        assert_no_errors()
