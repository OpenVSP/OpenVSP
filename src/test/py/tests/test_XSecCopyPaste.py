"""Copying and pasting cross sections, between Geoms of the same type and of different types.

A cross section can be pasted into another instance of the same Geom type or into a different
one -- Fuselage to Stack, say.  The XSec classes differ across those, so some of what the
source held has no home in the destination and is dropped; the rest is carried across by name.

Skinning spines make that more interesting than it sounds.  A spine runs the length of the
body, so how many there are, where they sit and what they are called belong to the Geom, while
the angle, slew, strength and curvature it enforces belong to each cross section.  Pasting one
cross section therefore must not restructure the body, but must carry the values that section
was holding.  These tests pin down which is which.
"""

import openvsp as vsp
import os

import pytest

EQUALS = [ "LRAngleEq", "LRSlewEq", "LRStrengthEq", "LRCurveEq" ]


def make_stack( spine_w01 = None, angle = 33.0, per_xsec = None ):
    """A Stack, optionally carrying one spine with a known angle on every cross section."""
    gid = vsp.AddGeom( "STACK" )
    xsurf = vsp.GetXSecSurf( gid, 0 )

    if spine_w01 is not None:
        vsp.AddSkinSpine( gid, spine_w01 )
        vsp.Update()

        for i in range( vsp.GetNumXSec( xsurf ) ):
            spine = vsp.GetSkinSpineID( vsp.GetXSec( xsurf, i ), 0 )
            for nm in EQUALS:
                vsp.SetParmVal( vsp.GetSkinSpineParm( spine, nm ), 0.0 )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngleSet" ), 1.0 )

            a = angle
            if per_xsec and i in per_xsec:
                a = per_xsec[i]
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngle" ), a )

    vsp.Update()
    return gid, xsurf


def spine_angles( gid, xsurf ):
    out = []
    for i in range( vsp.GetNumXSec( xsurf ) ):
        spine = vsp.GetSkinSpineID( vsp.GetXSec( xsurf, i ), 0 )
        out.append( vsp.GetParmVal( vsp.GetSkinSpineParm( spine, "LAngle" ) ) )
    return out


def spine_w01( gid, xsurf ):
    spine = vsp.GetSkinSpineID( vsp.GetXSec( xsurf, 0 ), 0 )
    return vsp.GetParmVal( vsp.GetSkinSpineParm( spine, "W01" ) )


def test_pasting_within_one_geom_carries_the_spine_values():
    """What a cross section enforces travels with it."""
    vsp.VSPRenew()
    gid, xsurf = make_stack( 0.35, 33.0, { 1: 50.0 } )

    assert spine_angles( gid, xsurf ) == pytest.approx( [ 33.0, 50.0, 33.0, 33.0, 33.0 ] )

    vsp.CopyXSec( gid, 1 )
    vsp.PasteXSec( gid, 3 )
    vsp.Update()

    assert spine_angles( gid, xsurf ) == pytest.approx( [ 33.0, 50.0, 33.0, 50.0, 33.0 ] )
    assert vsp.GetNumSkinSpines( gid ) == 1


def test_pasting_a_cross_section_does_not_restructure_the_body():
    """How many spines a Geom has follows the first cross section, not the one pasted into.

    Pasting into any section but the first cannot change the spine count: the sync takes its
    lead from section 0 and puts the body back the way it was.  Pasting into section 0 is a
    different matter and is checked separately below -- that section owns the spine set, so
    replacing it replaces the set.
    """
    vsp.VSPRenew()
    src, srcx = make_stack( 0.35 )
    dst, dstx = make_stack( None )

    assert vsp.GetNumSkinSpines( src ) == 1
    assert vsp.GetNumSkinSpines( dst ) == 0

    vsp.CopyXSec( src, 1 )
    vsp.PasteXSec( dst, 1 )
    vsp.Update()

    assert vsp.GetNumSkinSpines( dst ) == 0

    # And into a Geom carrying more spines than the source.
    vsp.VSPRenew()
    src, srcx = make_stack( 0.35 )
    dst = vsp.AddGeom( "STACK" )
    vsp.AddSkinSpine( dst, 0.2 )
    vsp.AddSkinSpine( dst, 0.6 )
    vsp.Update()

    vsp.CopyXSec( src, 1 )
    vsp.PasteXSec( dst, 1 )
    vsp.Update()

    assert vsp.GetNumSkinSpines( dst ) == 2


def test_the_first_cross_section_owns_where_a_spine_sits():
    """Position belongs to the spine as a whole, and the first cross section holds it.

    So pasting over the first section moves the spine for the whole body, and pasting over any
    other section does not -- it carries only the values that section enforces.
    """
    vsp.VSPRenew()
    src, srcx = make_stack( 0.70, 44.0 )
    dst, dstx = make_stack( 0.20, 11.0 )

    vsp.CopyXSec( src, 0 )
    vsp.PasteXSec( dst, 0 )
    vsp.Update()

    assert spine_w01( dst, dstx ) == pytest.approx( 0.70 )
    assert spine_angles( dst, dstx )[0] == pytest.approx( 44.0 )

    vsp.VSPRenew()
    src, srcx = make_stack( 0.70, 44.0 )
    dst, dstx = make_stack( 0.20, 11.0 )

    vsp.CopyXSec( src, 1 )
    vsp.PasteXSec( dst, 2 )
    vsp.Update()

    assert spine_w01( dst, dstx ) == pytest.approx( 0.20 )
    assert spine_angles( dst, dstx ) == pytest.approx( [ 11.0, 11.0, 44.0, 11.0, 11.0 ] )


def test_pasting_between_different_geom_types():
    """Fuselage to Stack.  The XSec classes differ, so XSecSurf::PasteXSec refuses the whole
    cross section and falls back to pasting the curve alone.  The cross section curve is the
    substance of it and must arrive intact, including its type and the width and height,
    which the curve types spell differently and so cannot carry by name alone.  Nothing the
    XSec itself holds crosses, spines included."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    fx = vsp.GetXSecSurf( fid, 0 )
    vsp.ChangeXSecShape( fx, 1, vsp.XS_ROUNDED_RECTANGLE )
    vsp.Update()

    fxsec = vsp.GetXSec( fx, 1 )
    vsp.SetParmVal( vsp.GetXSecParm( fxsec, "RoundedRect_Width" ), 4.25 )
    vsp.SetParmVal( vsp.GetXSecParm( fxsec, "RoundedRect_Height" ), 1.75 )
    vsp.Update()

    gid, gx = make_stack( 0.35 )

    vsp.CopyXSec( fid, 1 )
    vsp.PasteXSec( gid, 1 )
    vsp.Update()

    gx = vsp.GetXSecSurf( gid, 0 )
    gxsec = vsp.GetXSec( gx, 1 )

    assert vsp.GetXSecShape( gxsec ) == vsp.XS_ROUNDED_RECTANGLE
    assert vsp.GetParmVal( vsp.GetXSecParm( gxsec, "RoundedRect_Width" ) ) == pytest.approx( 4.25 )
    assert vsp.GetParmVal( vsp.GetXSecParm( gxsec, "RoundedRect_Height" ) ) == pytest.approx( 1.75 )

    # The Stack keeps the spine it had; the Fuselage section had nothing to say about it.
    assert vsp.GetNumSkinSpines( gid ) == 1
    assert vsp.GetNumXSec( gx ) == 5


def test_cutting_and_pasting_a_cross_section_keeps_the_spines_consistent():
    vsp.VSPRenew()
    gid, xsurf = make_stack( 0.35, 33.0, { 2: 60.0 } )

    n = vsp.GetNumXSec( xsurf )
    vsp.CutXSec( gid, 2 )
    vsp.Update()

    assert vsp.GetNumXSec( vsp.GetXSecSurf( gid, 0 ) ) == n - 1
    assert vsp.GetNumSkinSpines( gid ) == 1

    vsp.PasteXSec( gid, 1 )
    vsp.Update()

    xsurf = vsp.GetXSecSurf( gid, 0 )
    assert vsp.GetNumSkinSpines( gid ) == 1
    # The section that was cut carried 60, and it comes back where it was pasted.
    angles = spine_angles( gid, xsurf )
    assert any( a == pytest.approx( 60.0 ) for a in angles ), \
        "the cut section carried 60 and it did not come back: %s" % angles


def test_pasting_a_cross_section_curve_leaves_the_spines_alone():
    """A spine belongs to the cross section, not to the curve, so copying a curve moves the
    shape and nothing else."""
    vsp.VSPRenew()
    src, srcx = make_stack( 0.35 )
    dst, dstx = make_stack( 0.60, 21.0 )

    vsp.ChangeXSecShape( srcx, 2, vsp.XS_ROUNDED_RECTANGLE )
    vsp.Update()
    sxsec = vsp.GetXSec( vsp.GetXSecSurf( src, 0 ), 2 )
    vsp.SetParmVal( vsp.GetXSecParm( sxsec, "RoundedRect_Width" ), 3.5 )
    vsp.SetParmVal( vsp.GetXSecParm( sxsec, "RoundedRect_Height" ), 1.25 )
    vsp.Update()

    before_w01 = spine_w01( dst, dstx )
    before_angles = spine_angles( dst, dstx )

    vsp.CopyXSecCurve( src, 2 )
    vsp.PasteXSecCurve( dst, 2 )
    vsp.Update()

    dstx = vsp.GetXSecSurf( dst, 0 )
    dxsec = vsp.GetXSec( dstx, 2 )

    assert vsp.GetParmVal( vsp.GetXSecParm( dxsec, "RoundedRect_Width" ) ) == pytest.approx( 3.5 )
    assert vsp.GetParmVal( vsp.GetXSecParm( dxsec, "RoundedRect_Height" ) ) == pytest.approx( 1.25 )
    assert vsp.GetNumSkinSpines( dst ) == 1
    assert spine_w01( dst, dstx ) == pytest.approx( before_w01 )
    assert spine_angles( dst, dstx ) == pytest.approx( before_angles )


def test_a_pasted_geom_keeps_its_own_spine_parms():
    """Two Stacks built the same way must not share Parm IDs, or editing one would move the
    other."""
    vsp.VSPRenew()
    a, ax = make_stack( 0.35, 33.0 )
    b, bx = make_stack( 0.35, 33.0 )

    pa = vsp.GetSkinSpineParm( vsp.GetSkinSpineID( vsp.GetXSec( ax, 1 ), 0 ), "LAngle" )
    pb = vsp.GetSkinSpineParm( vsp.GetSkinSpineID( vsp.GetXSec( bx, 1 ), 0 ), "LAngle" )
    assert pa != pb

    vsp.SetParmVal( pa, 77.0 )
    vsp.Update()

    assert vsp.GetParmVal( pa ) == pytest.approx( 77.0 )
    assert vsp.GetParmVal( pb ) == pytest.approx( 33.0 )


def test_pasting_into_the_first_cross_section_replaces_the_spine_set():
    """Section 0 owns how many spines there are, so pasting over it restructures the body.

    That is the same ownership that puts position and name there, and it is why pasting into
    any other section leaves the count alone.  Worth stating plainly, because it means a paste
    into section 0 reaches every other section.
    """
    vsp.VSPRenew()
    src, srcx = make_stack( 0.35 )
    dst, dstx = make_stack( None )

    assert vsp.GetNumSkinSpines( dst ) == 0

    vsp.CopyXSec( src, 0 )
    vsp.PasteXSec( dst, 0 )
    vsp.Update()

    assert vsp.GetNumSkinSpines( dst ) == 1
    assert spine_w01( dst, vsp.GetXSecSurf( dst, 0 ) ) == pytest.approx( 0.35 )

    # And the other way: a spineless section pasted over section 0 takes the spines with it.
    vsp.VSPRenew()
    plain = vsp.AddGeom( "STACK" )
    spined, spinedx = make_stack( 0.35 )

    assert vsp.GetNumSkinSpines( spined ) == 1

    vsp.CopyXSec( plain, 0 )
    vsp.PasteXSec( spined, 0 )
    vsp.Update()

    assert vsp.GetNumSkinSpines( spined ) == 0


def test_two_spines_do_not_trade_places():
    """Each cross section's copy of a spine knows which spine it is a copy of.

    Every cross section holds a copy of every spine, carrying the values it enforces there.
    Keeping those copies in step used to mean matching by position in the array, which holds
    only while nothing reorders it.  With two spines, moving one must move that one -- not
    whichever happens to sit at the same index elsewhere.
    """
    vsp.VSPRenew()
    gid = vsp.AddGeom( "STACK" )
    xsurf = vsp.GetXSecSurf( gid, 0 )

    vsp.AddSkinSpine( gid, 0.40 )
    vsp.AddSkinSpine( gid, 0.60 )
    vsp.Update()

    # Tell them apart by what they enforce on the middle section.
    xsec = vsp.GetXSec( xsurf, 2 )
    for k, ang in [ ( 0, 40.0 ), ( 1, 60.0 ) ]:
        spine = vsp.GetSkinSpineID( xsec, k )
        for nm in EQUALS:
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, nm ), 0.0 )
        vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngleSet" ), 1.0 )
        vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngle" ), ang )
    vsp.Update()

    first = vsp.GetSkinSpineID( vsp.GetXSec( xsurf, 0 ), 0 )
    vsp.SetParmVal( vsp.GetSkinSpineParm( first, "W01" ), 0.35 )
    vsp.Update()

    # The one that moved is the one that was at 0.40, on every cross section, and the values
    # each was carrying stayed with it.
    for i in range( vsp.GetNumXSec( xsurf ) ):
        xs = vsp.GetXSec( xsurf, i )
        w0 = vsp.GetParmVal( vsp.GetSkinSpineParm( vsp.GetSkinSpineID( xs, 0 ), "W01" ) )
        w1 = vsp.GetParmVal( vsp.GetSkinSpineParm( vsp.GetSkinSpineID( xs, 1 ), "W01" ) )
        assert w0 == pytest.approx( 0.35 ), "XSec %d spine 0 is at %.3f" % ( i, w0 )
        assert w1 == pytest.approx( 0.60 ), "XSec %d spine 1 is at %.3f" % ( i, w1 )

    xsec = vsp.GetXSec( xsurf, 2 )
    a0 = vsp.GetParmVal( vsp.GetSkinSpineParm( vsp.GetSkinSpineID( xsec, 0 ), "LAngle" ) )
    a1 = vsp.GetParmVal( vsp.GetSkinSpineParm( vsp.GetSkinSpineID( xsec, 1 ), "LAngle" ) )
    assert a0 == pytest.approx( 40.0 )
    assert a1 == pytest.approx( 60.0 )


def test_spine_identity_survives_a_save_and_reload( tmp_path ):
    vsp.VSPRenew()
    gid = vsp.AddGeom( "STACK" )
    xsurf = vsp.GetXSecSurf( gid, 0 )
    vsp.AddSkinSpine( gid, 0.40 )
    vsp.AddSkinSpine( gid, 0.60 )
    vsp.Update()

    path = os.path.join( str( tmp_path ), "spine_ids.vsp3" )
    vsp.WriteVSPFile( path )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()

    gid = vsp.FindGeoms()[0]
    xsurf = vsp.GetXSecSurf( gid, 0 )

    first = vsp.GetSkinSpineID( vsp.GetXSec( xsurf, 0 ), 0 )
    vsp.SetParmVal( vsp.GetSkinSpineParm( first, "W01" ), 0.30 )
    vsp.Update()

    for i in range( vsp.GetNumXSec( xsurf ) ):
        xs = vsp.GetXSec( xsurf, i )
        assert vsp.GetParmVal( vsp.GetSkinSpineParm( vsp.GetSkinSpineID( xs, 0 ), "W01" ) ) == pytest.approx( 0.30 )
        assert vsp.GetParmVal( vsp.GetSkinSpineParm( vsp.GetSkinSpineID( xs, 1 ), "W01" ) ) == pytest.approx( 0.60 )


def test_pasting_across_geoms_matches_spines_by_position():
    """A pasted spine's values land on the spine at the same place, not the same list index.

    Two Geoms agree about where their spines sit.  They agree about nothing at all about the
    order the spines were added in, and a paste rebuilds one cross section's list in the
    source's order.  The tag that keeps copies straight within a Geom is no help here -- the
    destination has never seen the source's tags -- so the fallback has to match on the one
    thing both sides mean the same way, which is the position around the section.

    Matching on list index instead puts each spine's values on the other one, silently.
    """
    vsp.VSPRenew()

    def stack( order ):
        gid = vsp.AddGeom( "STACK" )
        vsp.Update()
        for w in order:
            vsp.AddSkinSpine( gid, w )
        vsp.Update()
        return gid

    # Same two positions, opposite order of creation.
    src = stack( [ 0.15, 0.62 ] )
    dst = stack( [ 0.62, 0.15 ] )
    vsp.Update()

    srcx = vsp.GetXSecSurf( src, 0 )
    for i in range( vsp.GetNumXSec( srcx ) ):
        xsec = vsp.GetXSec( srcx, i )
        for k, ang in [ ( 0, 15.0 ), ( 1, 62.0 ) ]:
            spine = vsp.GetSkinSpineID( xsec, k )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngleSet" ), 1.0 )
            vsp.SetParmVal( vsp.GetSkinSpineParm( spine, "LAngle" ), ang )
    vsp.Update()

    vsp.CopyXSec( src, 2 )
    vsp.PasteXSec( dst, 2 )
    vsp.Update()

    dstx = vsp.GetXSecSurf( dst, 0 )
    got = {}
    for k in range( vsp.GetNumSkinSpines( dst ) ):
        spine = vsp.GetSkinSpineID( vsp.GetXSec( dstx, 2 ), k )
        w = vsp.GetParmVal( vsp.GetSkinSpineParm( spine, "W01" ) )
        got[ round( w, 3 ) ] = vsp.GetParmVal( vsp.GetSkinSpineParm( spine, "LAngle" ) )

    assert got.get( 0.15 ) == pytest.approx( 15.0 ), \
        "the spine at W01 0.15 carries %s, not the 15.0 that was set there" % got.get( 0.15 )
    assert got.get( 0.62 ) == pytest.approx( 62.0 ), \
        "the spine at W01 0.62 carries %s, not the 62.0 that was set there" % got.get( 0.62 )
