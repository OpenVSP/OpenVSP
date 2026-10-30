# Clones of the end caps, the DATCOM components and the biconvex airfoil.
#
# A Clone borrows its original's surfaces and places them where the Clone is.  A Clone of a Geom using each of these
# features must show the original's surfaces exactly, follow the original when its inputs change, and keep doing so
# once saved and read back and once copied and pasted.  It takes the original's subsurfaces, a fin set's flaps among
# them, and tessellates and makes its DegenGeom as the original does.
#
# The Custom Geom is CustomScripts/EndCapTube.vsppart, which OpenVSP reads from ./CustomScripts when it starts, so
# these tests run from src/test/py.

import math
import os
import tempfile

import openvsp as vsp
import pytest

from testhelp import drop_errors, assert_no_errors
from clonehelp import scratch_output, comp_geom_of

XFORM = ( "X_Rel_Location", "Y_Rel_Location", "Z_Rel_Location", "X_Rel_Rotation", "Y_Rel_Rotation",
          "Z_Rel_Rotation" )
ORIGINAL_PLACE = ( 1.3, 0.2, -0.4, 5.0, 7.0, -3.0 )
CLONE_PLACE = ( 4.0, -2.0, 1.0, 20.0, -10.0, 30.0 )

COUNTS = ( "PTYPE", "NLOC", "NIN", "NSPAN", "NPANEL" )

CUSTOM = "EndCapTube"


def set_place( gid, vals ):
    for name, val in zip( XFORM, vals ):
        vsp.SetParmVal( vsp.FindParm( gid, name, "XForm" ), val )
    vsp.Update()


def get_place( gid ):
    return tuple( vsp.GetParmVal( vsp.FindParm( gid, name, "XForm" ) ) for name in XFORM )


def sample( gid, n=17 ):
    pts = []
    for s in range( vsp.GetNumMainSurfs( gid ) ):
        for i in range( n ):
            for j in range( n ):
                p = vsp.CompPnt01( gid, s, i / ( n - 1 ), j / ( n - 1 ) )
                pts += [ p.x(), p.y(), p.z() ]
    return pts


def assert_same( a, b, tol=1e-12 ):
    assert len( a ) == len( b )
    assert a == pytest.approx( b, abs=tol )


def put( gid, group="Design", **vals ):
    for name in COUNTS:
        if name in vals:
            vsp.SetParmVal( vsp.FindParm( gid, name, group ), vals[name] )
            vsp.Update()
    for name, val in vals.items():
        if name not in COUNTS:
            pid = vsp.FindParm( gid, name, group )
            assert pid, "no Parm " + name
            vsp.SetParmVal( pid, val )
    vsp.Update()


def clone_of( gid ):
    """A Clone of gid, placed on its own."""
    set_place( gid, ORIGINAL_PLACE )
    clone = vsp.CloneGeomVec( [ gid ] )[0]
    vsp.Update()
    return clone


def assert_clone_shows( gid, clone, copy_too=True ):
    """The Clone placed where gid is shows gid's surfaces; placed elsewhere it shows those of a copy put there, unless
    copy_too is off.  A copy of a component on a body put elsewhere stands on another part of the body, where the
    Clone shows the original's shape."""
    place = get_place( clone )
    set_place( clone, get_place( gid ) )
    assert vsp.GetNumMainSurfs( clone ) == vsp.GetNumMainSurfs( gid )
    assert_same( sample( clone ), sample( gid ) )
    if not copy_too:
        set_place( clone, place )
        return

    vsp.CopyGeomToClipboard( gid )
    copy = vsp.PasteGeomClipboard()[0]
    set_place( copy, CLONE_PLACE )
    set_place( clone, CLONE_PLACE )
    assert_same( sample( clone ), sample( copy ), 1e-9 )
    vsp.DeleteGeom( copy )
    set_place( clone, place )


def assert_clone_survives( clone ):
    """A Clone reads back, and a pasted copy of it, the same as it was.  Answers the read Clone."""
    set_place( clone, CLONE_PLACE )
    shown = sample( clone )
    nsub = len( vsp.GetSubSurfIDVec( clone ) )

    path = os.path.join( tempfile.mkdtemp(), "clone.vsp3" )
    vsp.WriteVSPFile( path, vsp.SET_ALL )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()
    back = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Clone" ]
    assert len( back ) == 1
    assert_same( sample( back[0] ), shown )
    assert len( vsp.GetSubSurfIDVec( back[0] ) ) == nsub

    vsp.CopyGeomToClipboard( back[0] )
    pasted = vsp.PasteGeomClipboard()[0]
    vsp.Update()
    assert vsp.GetGeomTypeName( pasted ) == "Clone"
    set_place( pasted, CLONE_PLACE )
    assert_same( sample( pasted ), shown )
    assert len( vsp.GetSubSurfIDVec( pasted ) ) == nsub
    return back[0]


#===== End caps =====#

MODIFIERS = {
    "plain": {},
    "truncated": { "TruncFlag": 1 },
    "sheared": { "ShearFlag": 1 },
    "scaled rows": { "ScaledFlag": 1 },
    "angle and exponent": { "OgiveAngle": 5.0, "PowerExp": 0.6, "HaackC": 0.2 },
    "nose at the origin": {},
}


def set_caps( gid, cap_type, rel, mod ):
    vals = { "Option": cap_type, "RelFlag": rel }
    if rel:
        vals.update( { "Length": 0.8, "Offset": 0.2, "Radius": 0.1 } )
    else:
        vals.update( { "LengthDim": 1.5, "OffsetDim": 0.1, "RadiusDim": 0.05 } )
    vals.update( MODIFIERS[mod] )
    for end in ( "UMin", "UMax" ):
        put( gid, "EndCap", **{ "Cap" + end + n: v for n, v in vals.items() } )
    if mod == "nose at the origin":
        put( gid, NoseOriginXFlag=1, NoseOriginYFlag=1, NoseOriginZFlag=1 )


def capped( typ, cap_type, rel, mod ):
    vsp.VSPRenew()
    drop_errors()
    if typ == CUSTOM:
        assert CUSTOM in vsp.GetGeomTypes(), "run from src/test/py, where CustomScripts/%s.vsppart is read" % CUSTOM
    gid = vsp.AddGeom( typ )
    if typ == CUSTOM:
        # Several main surfaces, each capped.
        put( gid, NumCopy=2, Turn=30.0 )
    vsp.Update()
    set_caps( gid, cap_type, rel, mod )
    return gid


@pytest.mark.parametrize( "mod", sorted( MODIFIERS ) )
@pytest.mark.parametrize( "rel", ( True, False ) )
@pytest.mark.parametrize( "cap_type", tuple( range( vsp.NUM_END_CAP_OPTIONS ) ) )
@pytest.mark.parametrize( "typ", ( "FUSELAGE", "STACK", CUSTOM ) )
def testACloneShowsTheCapsOfItsOriginal( typ, cap_type, rel, mod ):
    gid = capped( typ, cap_type, rel, mod )
    clone = clone_of( gid )
    assert_clone_shows( gid, clone )

    # The cap changes and the Clone follows.
    if rel:
        put( gid, "EndCap", CapUMinRadius=0.15, CapUMaxLength=0.6 )
    else:
        put( gid, "EndCap", CapUMinRadiusDim=0.08, CapUMaxLengthDim=1.2 )
    assert_clone_shows( gid, clone )

    assert_clone_survives( clone )
    assert_no_errors()


def degen_rows( ids ):
    """The x, y and z of each row of each DegenGeom surface of the Geoms ids."""
    vsp.SetAnalysisInputDefaults( "DegenGeom" )
    vsp.SetIntAnalysisInput( "DegenGeom", "WriteCSVFlag", [ 0 ] )
    vsp.SetIntAnalysisInput( "DegenGeom", "WriteMFileFlag", [ 0 ] )
    rid = vsp.ExecAnalysis( "DegenGeom" )
    rows = { gid: [] for gid in ids }
    for dg in vsp.GetStringResults( rid, "Degen_DegenGeoms" ):
        gid = vsp.GetStringResults( dg, "geom_id" )[0]
        if gid in rows:
            surf = vsp.GetStringResults( dg, "surf" )[0]
            for name in ( "x", "y", "z" ):
                rows[gid] += [ v for row in vsp.GetDoubleMatResults( surf, name ) for v in row ]
    vsp.DeleteAllResults()
    return rows


def assert_clone_measures_the_same( gid, clone ):
    """The DegenGeom and the tessellation CompGeom measures of a Clone placed where its original is are its
    original's."""
    set_place( clone, get_place( gid ) )
    rows = degen_rows( [ gid, clone ] )
    assert len( rows[gid] ) > 0
    assert_same( rows[clone], rows[gid] )

    # Apart, so CompGeom does not take one for inside the other.
    set_place( clone, CLONE_PLACE )
    scratch_output()
    areas, vols = comp_geom_of( [ gid, clone ] )
    assert areas[ vsp.GetGeomName( clone ) ] == pytest.approx( areas[ vsp.GetGeomName( gid ) ], rel=1e-9 )
    assert vols[ vsp.GetGeomName( clone ) ] == pytest.approx( vols[ vsp.GetGeomName( gid ) ], rel=1e-9 )


@pytest.mark.parametrize( "cap_type", tuple( range( vsp.NUM_END_CAP_OPTIONS ) ) )
@pytest.mark.parametrize( "typ", ( "FUSELAGE", "STACK" ) )
def testACloneOfACappedBodyMeasuresAsItsOriginal( typ, cap_type ):
    gid = capped( typ, cap_type, False, "truncated" )
    clone = clone_of( gid )
    assert_clone_measures_the_same( gid, clone )
    assert_no_errors()


#===== The DATCOM components =====#

def component( name, **vals ):
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( name )
    put( gid, **vals )
    return gid


FIN = dict( NPANEL=3, NSPAN=3, SSPAN_2=1.0, SSPAN_3=2.0, CHORD_3=0.5, SWEEP_2=20.0, CFOC_1=0.3, CFOC_2=0.3,
            CFOC_3=0.2, LER_1=0.01, LER_2=0.01, LER_3=0.01, GAM_2=10.0 )

# name: ( type, inputs, an input changed after the Clone is made )
COMPONENTS = {
    "cambered body": ( "MDC_BOD", dict( TNOSE=2, POWER=0.6, BNOSE=0.1, LAFT=2.0, DAFT=0.6, ZTIP=0.1, ZNOSE=0.05,
                                        ZCENTR=0.2, DEXIT=0.3 ), dict( LNOSE=2.0, ZTIP=0.3 ) ),
    "elliptic truncated body": ( "MDC_BOD", dict( NAMELIST=1, WNOSE=2.0, ENOSE=0.5, BNOSE=0.2, TRUNC=1, LAFT=2.0,
                                                  TAFT=1 ), dict( BNOSE=0.3 ) ),
    "lugs": ( "MDC_PROTUB", dict( PTYPE=3, NLOC=2, RBODY=1.2 ), dict( LPROT_2=0.8 ) ),
    "shoes": ( "MDC_PROTUB", dict( PTYPE=4, NLOC=3, PHIPROFlag=1, PHIPRO_2=100.0 ), dict( LPROT_1=0.8 ) ),
    "blocks": ( "MDC_PROTUB", dict( PTYPE=5, NLOC=4, OPROT_1=0.1 ), dict( HPROT_1=0.4 ) ),
    "fairing": ( "MDC_PROTUB", dict( PTYPE=6, RBODY=6.0, LPROT_1=5.0, WPROT_1=2.0, HPROT_1=1.0 ),
                 dict( HPROT_1=1.5 ) ),
    "covered inlets": ( "MDC_INLET", dict( INTYPE=3, NIN=2, COVER=1 ), dict( W1=0.7 ) ),
    "2D inlets": ( "MDC_INLET", dict( INTYPE=1, NIN=3 ), dict( HDIV=0.0 ) ),
}
for sectyp, sname in enumerate( ( "HEX", "NACA", "ARC", "USER" ) ):
    COMPONENTS[ "fin set of %s sections" % sname ] = ( "MDC_FIN", dict( FIN, SECTYP=sectyp ),
                                                       dict( CHORD_1=1.8, CFOC_3=0.0 ) )


@pytest.mark.parametrize( "case", sorted( COMPONENTS ) )
def testACloneShowsItsComponent( case ):
    typ, vals, change = COMPONENTS[case]
    gid = component( typ, **vals )
    clone = clone_of( gid )
    assert_clone_shows( gid, clone )

    put( gid, **change )
    assert_clone_shows( gid, clone )

    assert_clone_survives( clone )
    assert_no_errors()


@pytest.mark.parametrize( "case", sorted( COMPONENTS ) )
def testACloneOfAComponentMeasuresAsItsOriginal( case ):
    typ, vals, change = COMPONENTS[case]
    gid = component( typ, **vals )
    clone = clone_of( gid )
    assert_clone_measures_the_same( gid, clone )
    assert_no_errors()


def flap_lengths( gid ):
    out = []
    for ss in vsp.GetSubSurfIDVec( gid ):
        out.append( ( vsp.GetSubSurfName( gid, ss ),
                      vsp.GetParmVal( vsp.FindParm( ss, "Length_C_Start", "SS_Control" ) ),
                      vsp.GetParmVal( vsp.FindParm( ss, "Length_C_End", "SS_Control" ) ) ) )
    return out


def testACloneOfAFinSetHasItsFlaps():
    gid = component( "MDC_FIN", **FIN )
    clone = clone_of( gid )
    assert len( flap_lengths( gid ) ) == 2
    assert flap_lengths( clone ) == flap_lengths( gid )

    # A flap changes, then one goes.
    put( gid, CFOC_2=0.4 )
    assert flap_lengths( clone ) == flap_lengths( gid )
    put( gid, CFOC_3=0.0 )
    assert len( flap_lengths( gid ) ) == 1
    assert flap_lengths( clone ) == flap_lengths( gid )

    back = assert_clone_survives( clone )
    original = vsp.GetGeomCloneOriginal( back )
    assert flap_lengths( back ) == flap_lengths( original )
    assert_no_errors()


#===== On a body =====#

def body():
    gid = vsp.AddGeom( "MDC_BOD" )
    put( gid, NAMELIST=1, WNOSE=2.0, ENOSE=0.5, CENTRDefault=0, WCENTR=2.5, ECENTR=0.6, LNOSE=2.0, LCENTR=8.0 )
    return gid


def child( name, parent, x, **vals ):
    gid = vsp.AddGeom( name, parent )
    vsp.SetParmVal( vsp.FindParm( gid, "X_Rel_Location", "XForm" ), x )
    put( gid, **vals )
    return gid


CHILDREN = ( ( "MDC_PROTUB", dict( PTYPE=6, LPROT_1=0.8, WPROT_1=0.3, HPROT_1=0.2 ) ),
             ( "MDC_PROTUB", dict( PTYPE=5, NLOC=3, LPROT_1=0.4, WPROT_1=0.2, HPROT_1=0.3, OPROT_1=0.1 ) ),
             ( "MDC_INLET", dict( INTYPE=3, NIN=2 ) ),
             ( "MDC_FIN", dict( NPANEL=4, CFOC_1=0.3, CFOC_2=0.3 ) ) )


@pytest.mark.parametrize( "name,vals", CHILDREN )
def testACloneOfAComponentOnABodyFollowsTheBody( name, vals ):
    """The component stands on its body, and its Clone shows it there as the body changes, moves and scales."""
    vsp.VSPRenew()
    drop_errors()
    parent = body()
    gid = child( name, parent, 4.0, **vals )
    clone = vsp.CloneGeomVec( [ gid ] )[0]
    vsp.Update()
    assert_clone_shows( gid, clone, False )

    rbody = vsp.GetParmVal( vsp.FindParm( gid, "RBODY", "Design" ) )
    put( parent, WCENTR=3.2 )
    assert vsp.GetParmVal( vsp.FindParm( gid, "RBODY", "Design" ) ) > rbody
    assert_clone_shows( gid, clone, False )

    vsp.SetParmVal( vsp.FindParm( parent, "X_Rel_Location", "XForm" ), -1.0 )
    vsp.Update()
    assert_clone_shows( gid, clone, False )

    vsp.SetParmVal( vsp.FindParm( parent, "Scale", "XForm" ), 1.5 )
    vsp.Update()
    assert_clone_shows( gid, clone, False )

    assert_clone_survives( clone )
    assert_no_errors()


@pytest.mark.parametrize( "name,vals", CHILDREN )
def testClonesOfABodyAndItsComponentShowThem( name, vals ):
    """Cloned together, the component's Clone hangs from the body's Clone and shows the component."""
    vsp.VSPRenew()
    drop_errors()
    parent = body()
    gid = child( name, parent, 4.0, **vals )
    clones = vsp.CloneGeomVec( [ parent, gid ] )
    vsp.Update()
    assert vsp.GetGeomParent( clones[1] ) == clones[0]
    assert_same( sample( clones[0] ), sample( parent ) )
    assert_same( sample( clones[1] ), sample( gid ) )
    assert_no_errors()


def attached_child( name, parent, x, **vals ):
    gid = child( name, parent, x, **vals )
    vsp.SetParmVal( vsp.FindParm( gid, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( vsp.FindParm( gid, "Rots_Attach_Flag", "Attach" ), vsp.ATTACH_ROT_COMP )
    vsp.Update()
    return gid


@pytest.mark.parametrize( "name,vals", CHILDREN )
def testAComponentOnACloneOfABodyStandsOnIt( name, vals ):
    """A component stands on a Clone of a body as on the body, as the body changes and the Clone moves."""
    vsp.VSPRenew()
    drop_errors()
    parent = body()
    clone = clone_of( parent )
    on_body = attached_child( name, parent, 4.0, **vals )
    on_clone = attached_child( name, clone, 4.0, **vals )

    set_place( clone, CLONE_PLACE )
    set_place( clone, get_place( parent ) )
    assert_same( sample( on_clone ), sample( on_body ), 1e-9 )

    put( parent, WCENTR=3.2, ZCENTR=0.4 )
    assert_same( sample( on_clone ), sample( on_body ), 1e-9 )

    shown = sample( on_clone )
    path = os.path.join( tempfile.mkdtemp(), "standing.vsp3" )
    vsp.WriteVSPFile( path, vsp.SET_ALL )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()
    assert_same( sample( on_clone ), shown, 1e-9 )
    assert_no_errors()


def testAProtuberanceOnAFlippedCloneOfABodyStandsOnIt():
    """On a Clone flipped in the XZ plane, a protuberance at PHIPRO stands where one at -PHIPRO does on the body,
    reflected."""
    vsp.VSPRenew()
    drop_errors()
    parent = body()
    put( parent, ZCENTR=0.4 )
    clone = clone_of( parent )
    set_place( clone, ( 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 ) )
    set_place( parent, ( 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 ) )
    vsp.SetParmVal( vsp.FindParm( clone, "Flip_Flag", "Sym" ), vsp.SYM_XZ )
    on_body = attached_child( "MDC_PROTUB", parent, 4.0, PHIPROFlag=1 )
    on_clone = attached_child( "MDC_PROTUB", clone, 4.0, PHIPROFlag=1 )
    put( on_body, PHIPRO_1=-60.0 )
    put( on_clone, PHIPRO_1=60.0 )

    a = sample( on_body )
    a[1::3] = [ -y for y in a[1::3] ]
    b = sample( on_clone )
    pa = [ tuple( a[i:i + 3] ) for i in range( 0, len( a ), 3 ) ]
    pb = [ tuple( b[i:i + 3] ) for i in range( 0, len( b ), 3 ) ]
    assert max( min( math.dist( p, q ) for q in pb ) for p in pa ) < 1e-9
    assert_no_errors()


#===== The biconvex airfoil =====#

def testACloneShowsABiconvexWing():
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
    clone = clone_of( wid )
    assert_clone_shows( wid, clone )

    xs = vsp.GetXSec( xss, 1 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickLocLow" ), 0.2 )
    vsp.Update()
    assert_clone_shows( wid, clone )

    assert_clone_measures_the_same( wid, clone )
    assert_clone_survives( clone )
    assert_no_errors()
