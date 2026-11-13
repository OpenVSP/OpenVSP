"""After a section is cut, pasted, inserted or has a curve pasted onto it, one Update leaves the Geom
exactly as it is when built from scratch.

A section keeps its placed curve between Updates and places it again only when told it is out of
date.  An edit that leaves an old placement in use shows up as a Geom that changes when the file
is saved and read back.  So each edit here is followed by one Update, and the Geom -- its surface
and every Parm of it and of its sections -- is compared with the same Geom read back from a file.

A skinning value left free is written back to its section after every skin, which tells the section
it is out of date whether or not the edit did.  So every edit is also made with every skinning value
set, where nothing is written back and only the edit itself can bring a section up to date.
"""

import openvsp as vsp

import pytest

from testhelp import ( drop_errors, assert_no_errors )


TYPES = [ "WING", "FUSELAGE", "STACK", "PROP" ]

NSECT = 5

# Section placement Parms set per section, so that a section placed from a stale neighbor or a
# stale self lands somewhere else.  Every Stack section is placed from the one before it, so its
# turns reach every section after.
PLACEMENT = {
    "WING": { "Span": [ 2.0, 3.0, 1.5, 2.5 ], "Sweep": [ 10.0, 25.0, 5.0, 40.0 ],
              "Dihedral": [ 3.0, -2.0, 8.0, 15.0 ], "Twist": [ 1.0, -2.0, -3.0, 2.0 ] },
    "FUSELAGE": { "ZLocPercent": [ 0.0, 0.05, -0.03, 0.02, 0.0 ], "XRotate": [ 0.0, 5.0, -4.0, 9.0, 0.0 ] },
    "STACK": { "XDelta": [ 0.0, 1.5, 2.0, 1.0, 2.5 ], "ZDelta": [ 0.0, 0.3, -0.4, 0.2, 0.5 ],
               "YRotate": [ 0.0, 6.0, -8.0, 4.0, 10.0 ] },
    "PROP": {},
}

# The first section an edit may touch.  A Wing's first section only holds the root airfoil.
FIRST = { "WING": 1, "FUSELAGE": 0, "STACK": 0, "PROP": 0 }

SHAPES = [ vsp.XS_SUPER_ELLIPSE, vsp.XS_ROUNDED_RECTANGLE, vsp.XS_ELLIPSE, vsp.XS_SUPER_ELLIPSE,
           vsp.XS_GENERAL_FUSE ]
WING_SHAPES = [ vsp.XS_FOUR_SERIES, vsp.XS_SIX_SERIES, vsp.XS_FOUR_SERIES, vsp.XS_BICONVEX,
                vsp.XS_SIX_SERIES ]


SKINNING = [ "free", "set" ]


def build( gtype, skinning ):
    """A Geom of the type with NSECT sections, every one of them different."""
    gid = vsp.AddGeom( gtype )
    vsp.Update()
    xsurf = vsp.GetXSecSurf( gid, 0 )
    while vsp.GetNumXSec( xsurf ) < NSECT:
        if gtype == "WING":
            vsp.InsertXSec( gid, vsp.GetNumXSec( xsurf ) - 1, vsp.XS_FOUR_SERIES )
        else:
            vsp.InsertXSec( gid, 0, vsp.XS_SUPER_ELLIPSE )
        vsp.Update()

    for i in range( NSECT ):
        if gtype == "WING":
            vsp.ChangeXSecShape( xsurf, i, WING_SHAPES[i] )
            xs = vsp.GetXSec( xsurf, i )
            vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickChord" ), 0.08 + 0.02 * i )
            if i > 0:
                for name, vals in PLACEMENT[gtype].items():
                    vsp.SetParmVal( vsp.GetXSecParm( xs, name ), vals[ i - 1 ] )
        else:
            # A closed Stack or Fuselage end is a point; keep it one.
            if gtype != "PROP" and ( i == 0 or i == NSECT - 1 ):
                xs = vsp.GetXSec( xsurf, i )
            else:
                vsp.ChangeXSecShape( xsurf, i, SHAPES[i] )
                xs = vsp.GetXSec( xsurf, i )
                vsp.SetXSecWidthHeight( xs, 0.8 + 0.3 * i, 0.6 + 0.2 * ( NSECT - i ) )
            for name, vals in PLACEMENT[gtype].items():
                vsp.SetParmVal( vsp.GetXSecParm( xs, name ), vals[i] )
        vsp.Update()

    if skinning == "set":
        for i in range( NSECT ):
            for pid in vsp.GetXSecParmIDs( vsp.GetXSec( xsurf, i ) ):
                if vsp.GetParmName( pid ).endswith( "Set" ):
                    vsp.SetParmVal( pid, 1.0 )
        vsp.Update()

    drop_errors()
    return gid


def snapshot( gid ):
    """The Geom's surface and every Parm of it and of its sections."""
    pnts = []
    for u in [ 0.0, 0.1, 0.25, 0.4, 0.5, 0.6, 0.75, 0.9, 1.0 ]:
        for w in [ 0.0, 0.13, 0.25, 0.37, 0.5, 0.63, 0.75, 0.88, 1.0 ]:
            p = vsp.CompPnt01( gid, 0, u, w )
            pnts.append( ( p.x(), p.y(), p.z() ) )

    parms = {}
    ids = list( vsp.GetGeomParmIDs( gid ) )
    xsurf = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xsurf ) ):
        ids += list( vsp.GetXSecParmIDs( vsp.GetXSec( xsurf, i ) ) )
    for pid in ids:
        parms[ vsp.GetParmDisplayGroupName( pid ) + ":" + vsp.GetParmName( pid ) ] = vsp.GetParmVal( pid )
    return pnts, parms


def assert_matches_rebuilt( gid, tmp_path ):
    """The Geom as it is equals the Geom read back from a file."""
    live_pnts, live_parms = snapshot( gid )

    fname = str( tmp_path / "edit.vsp3" )
    vsp.WriteVSPFile( fname, vsp.SET_ALL )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()

    read_pnts, read_parms = snapshot( gid )

    assert len( live_pnts ) == len( read_pnts )
    for a, b in zip( live_pnts, read_pnts ):
        assert a == pytest.approx( b, abs=1e-9 )

    assert live_parms.keys() == read_parms.keys()
    for name in live_parms:
        assert live_parms[ name ] == pytest.approx( read_parms[ name ], rel=1e-9, abs=1e-9 ), name


def edit_indices( gtype ):
    first = FIRST[ gtype ]
    return [ first, ( first + NSECT - 1 ) // 2, NSECT - 1 ]


@pytest.mark.parametrize( "skinning", SKINNING )
@pytest.mark.parametrize( "gtype", TYPES )
def testUnEditedGeomMatchesRebuilt( gtype, skinning, tmp_path, monkeypatch ):
    """The comparison itself: a Geom nothing was done to equals the Geom read back."""
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    gid = build( gtype, skinning )
    assert_matches_rebuilt( gid, tmp_path )
    assert_no_errors()


@pytest.mark.parametrize( "skinning", SKINNING )
@pytest.mark.parametrize( "gtype", TYPES )
@pytest.mark.parametrize( "which", [ 0, 1, 2 ] )
def testCutMatchesRebuilt( gtype, which, skinning, tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    gid = build( gtype, skinning )
    vsp.CutXSec( gid, edit_indices( gtype )[ which ] )
    vsp.Update()
    assert_matches_rebuilt( gid, tmp_path )
    assert_no_errors()


@pytest.mark.parametrize( "skinning", SKINNING )
@pytest.mark.parametrize( "gtype", TYPES )
@pytest.mark.parametrize( "which", [ 0, 1, 2 ] )
def testPasteMatchesRebuilt( gtype, which, skinning, tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    gid = build( gtype, skinning )
    idx = edit_indices( gtype )
    target = idx[ which ]
    source = idx[ ( which + 1 ) % 3 ]
    vsp.CopyXSec( gid, source )
    vsp.PasteXSec( gid, target )
    vsp.Update()
    assert_matches_rebuilt( gid, tmp_path )
    assert_no_errors()


@pytest.mark.parametrize( "skinning", SKINNING )
@pytest.mark.parametrize( "gtype", TYPES )
@pytest.mark.parametrize( "which", [ 0, 1 ] )
def testInsertMatchesRebuilt( gtype, which, skinning, tmp_path, monkeypatch ):
    """Insert after the first and a middle section (no type inserts after its last)."""
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    gid = build( gtype, skinning )
    # A Wing inserts what is on the clipboard.
    vsp.CopyXSec( gid, edit_indices( gtype )[ 2 ] )
    vsp.InsertXSec( gid, edit_indices( gtype )[ which ], vsp.XS_SUPER_ELLIPSE )
    vsp.Update()
    assert_matches_rebuilt( gid, tmp_path )
    assert_no_errors()


@pytest.mark.parametrize( "skinning", SKINNING )
@pytest.mark.parametrize( "gtype", TYPES )
@pytest.mark.parametrize( "which", [ 0, 1, 2 ] )
def testPasteCurveMatchesRebuilt( gtype, which, skinning, tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    gid = build( gtype, skinning )
    idx = edit_indices( gtype )
    # A Wing's root airfoil takes a curve too.
    target = [ 0, idx[1], idx[2] ][ which ]
    source = idx[ ( which + 1 ) % 3 ]
    vsp.CopyXSecCurve( gid, source )
    vsp.PasteXSecCurve( gid, target )
    vsp.Update()
    assert_matches_rebuilt( gid, tmp_path )
    assert_no_errors()


@pytest.mark.parametrize( "skinning", SKINNING )
@pytest.mark.parametrize( "gtype", TYPES )
@pytest.mark.parametrize( "other", TYPES )
def testPasteFromOtherTypeMatchesRebuilt( gtype, other, skinning, tmp_path, monkeypatch ):
    """A section from another type of Geom pastes its curve alone."""
    if gtype == other:
        pytest.skip( "same type" )
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    src = build( other, skinning )
    gid = build( gtype, skinning )
    vsp.CopyXSec( src, edit_indices( other )[ 1 ] )
    vsp.PasteXSec( gid, edit_indices( gtype )[ 1 ] )
    vsp.Update()
    assert_matches_rebuilt( gid, tmp_path )
    assert_no_errors()
