# Dimensional end cap length and offset.
#
# CapUMinRelFlag / CapUMaxRelFlag false makes CapU*LengthDim and CapU*OffsetDim drive the cap; the
# non-dimensional CapU*Length and CapU*Offset follow.  The length is the greatest height of the cap's tip
# curve above the end, measured across the end's forward direction (along its upper half, w 0 to 0.5);
# a point cap's is measured above the point it has with no length.  The offset is the greatest distance
# the offset moves the tip.
#
# Every cap type with a length is covered; the extended round caps are checked on wings.  Flat caps
# have no length, and leave their dimensional values alone.
#
# The size of a unit cap depends on the end's curve and on the surface's tangents across it, so the
# shapes here vary the end section, the section next to it (taper, flare, droop, a different type),
# and whether the end's skinning angles are set or follow the neighbouring sections.

import math
import os

import openvsp as vsp
import pytest

from testhelp import drop_errors

LENGTH_TYPES = ( vsp.ROUND_END_CAP, vsp.EDGE_END_CAP, vsp.SHARP_END_CAP, vsp.POINT_END_CAP )
EXT_TYPES = ( vsp.ROUND_EXT_END_CAP_NONE, vsp.ROUND_EXT_END_CAP_LE, vsp.ROUND_EXT_END_CAP_TE,
              vsp.ROUND_EXT_END_CAP_BOTH )
ENDS = ( ( "UMin", 0.0 ), ( "UMax", 1.0 ) )

# End section: type, width, height.
END_SHAPES = { "circle": ( vsp.XS_ELLIPSE, 2.0, 2.0 ),
               "wide": ( vsp.XS_ELLIPSE, 4.0, 1.0 ),
               "tall": ( vsp.XS_ELLIPSE, 1.0, 3.0 ),
               "superell": ( vsp.XS_SUPER_ELLIPSE, 3.0, 1.5 ),
               "rrect": ( vsp.XS_ROUNDED_RECTANGLE, 2.5, 1.5 ) }

# Section next to the end: its type (None keeps the end's), width and height as fractions of the
# end's, and a Z step from it.
NEIGHBOURS = { "same": ( None, 1.0, 1.0, 0.0 ),
               "taper": ( None, 0.5, 0.5, 0.0 ),
               "taperw": ( None, 0.4, 1.0, 0.0 ),
               "taperh": ( None, 1.0, 0.4, 0.0 ),
               "flare": ( None, 1.6, 1.6, 0.0 ),
               "droop": ( None, 1.0, 1.0, 0.6 ),
               "rrect": ( vsp.XS_ROUNDED_RECTANGLE, 1.3, 0.7, 0.0 ),
               "superell": ( vsp.XS_SUPER_ELLIPSE, 0.8, 1.8, 0.0 ) }

SIZE_PARMS = { vsp.XS_ELLIPSE: ( "Ellipse_Width", "Ellipse_Height" ),
               vsp.XS_SUPER_ELLIPSE: ( "Super_Width", "Super_Height" ),
               vsp.XS_ROUNDED_RECTANGLE: ( "RoundedRect_Width", "RoundedRect_Height" ) }


def cap_parm( gid, name ):
    return vsp.FindParm( gid, name, "EndCap" )


def set_cap( gid, name, val ):
    vsp.SetParmVal( cap_parm( gid, name ), val )


def get_cap( gid, name ):
    return vsp.GetParmVal( cap_parm( gid, name ) )


def set_section( xss, i, typ, w, h ):
    vsp.ChangeXSecShape( xss, i, typ )
    xs = vsp.GetXSec( xss, i )
    wname, hname = SIZE_PARMS[typ]
    vsp.SetParmVal( vsp.GetXSecParm( xs, wname ), w )
    vsp.SetParmVal( vsp.GetXSecParm( xs, hname ), h )


def release_skinning( xs ):
    # Equality flags force their angles on, so they go first.
    for suffix in ( "Eq", "Set" ):
        for pid in vsp.GetXSecParmIDs( xs ):
            name = vsp.GetParmName( pid )
            if name.endswith( suffix ) and ( "Angle" in name or "Strength" in name or "Slew" in name ):
                vsp.SetParmVal( pid, 0 )


def stack( end, neighbour, free ):
    """A Stack with both ends of shape end, next to sections of shape neighbour.  free releases the end
    sections' skinning angles, so the neighbours set the tangents across the ends."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( "STACK" )
    xss = vsp.GetXSecSurf( gid, 0 )
    n = vsp.GetNumXSec( xss )
    typ, w, h = END_SHAPES[end]
    ntyp, wfrac, hfrac, dz = NEIGHBOURS[neighbour]
    if ntyp is None:
        ntyp = typ
    for iend, inext in ( ( 0, 1 ), ( n - 1, n - 2 ) ):
        set_section( xss, iend, typ, w, h )
        set_section( xss, inext, ntyp, wfrac * w, hfrac * h )
        if free:
            release_skinning( vsp.GetXSec( xss, iend ) )
    vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, 1 ), "ZDelta" ), dz )
    vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, n - 1 ), "ZDelta" ), dz )
    vsp.Update()
    return gid


def body( typ, width=4.0, height=1.5 ):
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        set_section( xss, i, vsp.XS_ELLIPSE, width, height )
    vsp.Update()
    return gid


# Wing: sweep, end cap sweep correction, and a tip section that is tapered, thicker and cambered.
WINGS = { "plain": ( 0.0, False, False ),
          "swept": ( 30.0, False, False ),
          "sweepcorrected": ( 30.0, True, False ),
          "thicktip": ( 10.0, True, True ) }


def wing( name ):
    sweep, swflag, thicktip = WINGS[name]
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( "WING" )
    vsp.SetParmVal( gid, "Sweep", "XSec_1", sweep )
    if thicktip:
        vsp.SetParmVal( gid, "Tip_Chord", "XSec_1", 2.0 )
        xs = vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), 1 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickChord" ), 0.2 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "Camber" ), 0.06 )
    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "SweepFlag", swflag )
    vsp.Update()
    return gid


def point( gid, u, w ):
    p = vsp.CompPnt01( gid, 0, u, w )
    return ( p.x(), p.y(), p.z() )


def tip_curve( gid, u ):
    """The cap's tip curve, over the first half of the end; the second half folds back onto it."""
    return lambda w: point( gid, u, w )


def seam( gid, u ):
    """The middle of the uncapped end: halfway between the points the cap pairs across it."""
    return lambda w: tuple( ( a + b ) * 0.5 for a, b in zip( point( gid, u, w ), point( gid, u, 1.0 - w ) ) )


def forward( gid, u ):
    """Unit vector along the upper half of the end, from w 0 to w 0.5."""
    d = [ b - a for a, b in zip( point( gid, u, 0.0 ), point( gid, u, 0.5 ) ) ]
    n = math.sqrt( sum( c * c for c in d ) )
    return [ c / n for c in d ]


def height( p, q, fwd ):
    """Distance from q to p across fwd."""
    d = [ a - b for a, b in zip( p, q ) ]
    along = sum( a * b for a, b in zip( d, fwd ) )
    return math.sqrt( max( 0.0, sum( c * c for c in d ) - along * along ) )


def greatest( f ):
    """Largest f( w ) for w in [0, 0.5]: a scan, then a golden section search around the best sample."""
    n = 200
    ws = [ 0.5 * i / n for i in range( n + 1 ) ]
    vals = [ f( w ) for w in ws ]
    i = max( range( n + 1 ), key=lambda k: vals[k] )
    a = ws[ max( i - 1, 0 ) ]
    b = ws[ min( i + 1, n ) ]
    g = ( math.sqrt( 5.0 ) - 1.0 ) * 0.5
    c = b - g * ( b - a )
    d = a + g * ( b - a )
    for k in range( 80 ):
        if f( c ) > f( d ):
            b = d
        else:
            a = c
        c = b - g * ( b - a )
        d = a + g * ( b - a )
    return max( vals[i], f( 0.5 * ( a + b ) ) )


def shape( gid ):
    return [ point( gid, i / 40.0, j / 20.0 ) for i in range( 41 ) for j in range( 21 ) ]


def max_move( a, b ):
    return max( math.dist( p, q ) for p, q in zip( a, b ) )


def unit_cap( gid, cap_type ):
    """Cap both ends and return each end's dimensional length at non-dimensional length 1 and
    dimensional offset at non-dimensional offset 1."""
    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "Option", cap_type )
        set_cap( gid, "Cap" + end + "Length", 1.0 )
        set_cap( gid, "Cap" + end + "Offset", 1.0 )
    vsp.Update()
    return [ ( get_cap( gid, "Cap" + end + "LengthDim" ), get_cap( gid, "Cap" + end + "OffsetDim" ) )
             for end, u in ENDS ]


def copies( gid, n ):
    """n copies of gid beside it, so surfaces with different caps can be evaluated together."""
    vsp.CopyGeomToClipboard( gid )
    ids = [ vsp.PasteGeomClipboard()[0] for i in range( n ) ]
    vsp.Update()
    return ids


def check_tip( gid, cap_type ):
    """Drive both ends dimensionally and measure the tip curve against the uncapped end."""
    bare, flat, up, down = copies( gid, 4 )
    refs = unit_cap( gid, cap_type )

    for i, ( end, u ) in enumerate( ENDS ):
        lref, oref = refs[i]
        assert lref > 0.0 and oref > 0.0, end
        for cid, off in ( ( flat, 0.0 ), ( up, 0.3 * oref ), ( down, -0.3 * oref ) ):
            set_cap( cid, "Cap" + end + "Option", cap_type )
            set_cap( cid, "Cap" + end + "RelFlag", False )
            set_cap( cid, "Cap" + end + "LengthDim", 0.6 * lref )
            set_cap( cid, "Cap" + end + "OffsetDim", off )
    vsp.Update()

    for i, ( end, u ) in enumerate( ENDS ):
        lref, oref = refs[i]
        base = seam( bare, u )
        if cap_type == vsp.POINT_END_CAP:
            mid = base( 0.25 )
            base = lambda w: mid
        fwd = forward( bare, u )
        tip0 = tip_curve( flat, u )
        tipup = tip_curve( up, u )
        tipdn = tip_curve( down, u )

        lmeas = greatest( lambda w: height( tip0( w ), base( w ), fwd ) )
        omeas = greatest( lambda w: math.dist( tipup( w ), tip0( w ) ) )
        assert lmeas == pytest.approx( 0.6 * lref, abs=1e-9 ), end
        assert omeas == pytest.approx( 0.3 * oref, abs=1e-9 ), end

        # Opposite offsets move the tip equally the other way.
        for k in range( 51 ):
            w = 0.5 * k / 50
            mid = tuple( ( a + b ) * 0.5 for a, b in zip( tipup( w ), tipdn( w ) ) )
            assert math.dist( mid, tip0( w ) ) < 1e-9, end

        assert get_cap( flat, "Cap" + end + "LengthDim" ) == pytest.approx( 0.6 * lref, abs=1e-12 ), end
        assert get_cap( down, "Cap" + end + "OffsetDim" ) == pytest.approx( -0.3 * oref, abs=1e-12 ), end


SWEEP = [ ( end, neighbour, True ) for end in END_SHAPES for neighbour in NEIGHBOURS ] + \
        [ ( end, "same", False ) for end in END_SHAPES ]


@pytest.mark.parametrize( "cap_type", LENGTH_TYPES )
@pytest.mark.parametrize( "end, neighbour, free", SWEEP )
def testDimensionalCapPlacesTheTipOnAnyShape( end, neighbour, free, cap_type ):
    check_tip( stack( end, neighbour, free ), cap_type )


@pytest.mark.parametrize( "cap_type", LENGTH_TYPES + EXT_TYPES )
@pytest.mark.parametrize( "name", WINGS )
def testDimensionalCapPlacesTheTipOnWings( name, cap_type ):
    check_tip( wing( name ), cap_type )


@pytest.mark.parametrize( "cap_type", ( vsp.ROUND_EXT_END_CAP_LE, vsp.ROUND_EXT_END_CAP_TE,
                                       vsp.ROUND_EXT_END_CAP_BOTH ) )
@pytest.mark.parametrize( "name", WINGS )
def testExtendedRoundCapIsAsHighAsTheRoundCap( name, cap_type ):
    """An extended round cap holds the round cap's greatest height.  The root is capped first, so both
    caps there start from the same surface."""
    gid = wing( name )
    lround = unit_cap( gid, vsp.ROUND_END_CAP )[0][0]
    lext = unit_cap( gid, cap_type )[0][0]
    assert lext == pytest.approx( lround, rel=1e-12 )


def testAFlatCapIgnoresItsDimensions():
    gid = wing( "thicktip" )
    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "Option", vsp.FLAT_END_CAP )
    vsp.Update()
    assert get_cap( gid, "CapUMinOption" ) == vsp.FLAT_END_CAP
    before = shape( gid )
    nd = [ ( get_cap( gid, "Cap" + end + "Length" ), get_cap( gid, "Cap" + end + "Offset" ) ) for end, u in ENDS ]

    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "RelFlag", False )
        set_cap( gid, "Cap" + end + "LengthDim", 3.0 )
        set_cap( gid, "Cap" + end + "OffsetDim", 0.4 )
    vsp.Update()

    assert max_move( before, shape( gid ) ) < 1e-12
    for i, ( end, u ) in enumerate( ENDS ):
        assert get_cap( gid, "Cap" + end + "Length" ) == nd[i][0], end
        assert get_cap( gid, "Cap" + end + "Offset" ) == nd[i][1], end
        assert get_cap( gid, "Cap" + end + "LengthDim" ) == 3.0, end
        assert get_cap( gid, "Cap" + end + "OffsetDim" ) == 0.4, end


@pytest.mark.parametrize( "typ", ( "FUSELAGE", "STACK" ) )
@pytest.mark.parametrize( "cap_type", LENGTH_TYPES )
def testDimensionalCapPlacesTheTipOnDefaultBodies( typ, cap_type ):
    check_tip( body( typ ), cap_type )


@pytest.mark.parametrize( "cap_type", LENGTH_TYPES )
@pytest.mark.parametrize( "gid_maker", ( lambda: body( "FUSELAGE" ), lambda: stack( "tall", "flare", True ) ),
                          ids=( "fuselage", "tall-flare" ) )
def testOffsetDimIsTheOffsetsMove( gid_maker, cap_type ):
    """At a unit length, these caps' length scale exceeds their offset scale."""
    gid = gid_maker()
    flat = copies( gid, 1 )[0]
    for cid, off in ( ( gid, 1.0 ), ( flat, 0.0 ) ):
        for end, u in ENDS:
            set_cap( cid, "Cap" + end + "Option", cap_type )
            set_cap( cid, "Cap" + end + "Length", 1.0 )
            set_cap( cid, "Cap" + end + "Offset", off )
    vsp.Update()

    for end, u in ENDS:
        tip = tip_curve( gid, u )
        tip0 = tip_curve( flat, u )
        omeas = greatest( lambda w: math.dist( tip( w ), tip0( w ) ) )
        assert get_cap( gid, "Cap" + end + "OffsetDim" ) == pytest.approx( omeas, abs=1e-9 ), end


@pytest.mark.parametrize( "cap_type", ( vsp.ROUND_END_CAP, vsp.EDGE_END_CAP, vsp.SHARP_END_CAP ) )
def testNeighbouringSectionsChangeTheUnitCap( cap_type ):
    """Guards the sweep: with the end's skinning released, the neighbours must reach a cap built from the
    end's tangents."""
    sizes = set()
    for neighbour in NEIGHBOURS:
        refs = unit_cap( stack( "circle", neighbour, True ), cap_type )
        sizes.add( round( refs[0][0], 9 ) )
    assert len( sizes ) >= 5


def testTheNeighboursLeaveAPointCapsUnitAlone():
    """A point cap is measured in the body's frame, so its unit length and offset are the end's size across
    the body's axis, whatever the sections next to the end."""
    sizes = set()
    for neighbour in NEIGHBOURS:
        refs = unit_cap( stack( "circle", neighbour, True ), vsp.POINT_END_CAP )
        sizes.add( round( refs[0][0], 9 ) )
        assert refs[0][1] == pytest.approx( refs[0][0], rel=1e-12 ), neighbour
    assert len( sizes ) == 1


@pytest.mark.parametrize( "cap_type", LENGTH_TYPES )
def testSwitchingToDimensionalKeepsTheShape( cap_type ):
    gid = stack( "rrect", "superell", True )
    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "Option", cap_type )
        set_cap( gid, "Cap" + end + "Length", 0.8 )
        set_cap( gid, "Cap" + end + "Offset", 0.1 )
    vsp.Update()
    before = shape( gid )

    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "RelFlag", False )
    vsp.Update()

    assert max_move( before, shape( gid ) ) < 1e-12
    for end, u in ENDS:
        assert get_cap( gid, "Cap" + end + "Length" ) == pytest.approx( 0.8, abs=1e-12 ), end
        assert get_cap( gid, "Cap" + end + "Offset" ) == pytest.approx( 0.1, abs=1e-12 ), end


def testDimensionalCapScalesWithTheGeom():
    gid = stack( "superell", "droop", True )
    set_cap( gid, "CapUMinOption", vsp.ROUND_END_CAP )
    set_cap( gid, "CapUMinRelFlag", False )
    set_cap( gid, "CapUMinLengthDim", 0.3 )
    set_cap( gid, "CapUMinOffsetDim", 0.05 )
    vsp.Update()
    nd = ( get_cap( gid, "CapUMinLength" ), get_cap( gid, "CapUMinOffset" ) )

    vsp.SetParmVal( vsp.FindParm( gid, "Scale", "XForm" ), 2.0 )
    vsp.Update()

    assert get_cap( gid, "CapUMinLengthDim" ) == pytest.approx( 0.6, abs=1e-12 )
    assert get_cap( gid, "CapUMinOffsetDim" ) == pytest.approx( 0.1, abs=1e-12 )
    assert get_cap( gid, "CapUMinLength" ) == pytest.approx( nd[0], abs=1e-12 )
    assert get_cap( gid, "CapUMinOffset" ) == pytest.approx( nd[1], abs=1e-12 )


def testDimensionalCapSurvivesAFile( tmp_path ):
    gid = stack( "wide", "rrect", True )
    set_cap( gid, "CapUMaxOption", vsp.SHARP_END_CAP )
    set_cap( gid, "CapUMaxRelFlag", False )
    set_cap( gid, "CapUMaxLengthDim", 0.45 )
    set_cap( gid, "CapUMaxOffsetDim", 0.12 )
    vsp.Update()
    before = shape( gid )

    fname = os.path.join( str( tmp_path ), "endcapdims.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    gid = vsp.FindGeoms()[0]

    assert not get_cap( gid, "CapUMaxRelFlag" )
    assert max_move( before, shape( gid ) ) < 1e-12


def testDimensionalLengthIsClampedWithItsNonDimensionalLength():
    gid = body( "FUSELAGE" )
    set_cap( gid, "CapUMinOption", vsp.EDGE_END_CAP )
    set_cap( gid, "CapUMinRelFlag", False )
    set_cap( gid, "CapUMinLengthDim", 1e4 )
    vsp.Update()

    lmax = vsp.GetParmUpperLimit( cap_parm( gid, "CapUMinLength" ) )
    assert get_cap( gid, "CapUMinLength" ) == pytest.approx( lmax )
    assert get_cap( gid, "CapUMinLengthDim" ) < 1e4
