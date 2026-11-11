"""The two kinds of cross section copy and paste, on every Geom built from cross sections.

CopyXSec and PasteXSec carry the whole section: its curve and what the section holds about
itself.  CopyXSecCurve and PasteXSecCurve carry the curve alone.  On a Wing CopyAirfoil and
PasteAirfoil are the curve kind under the Airfoil name.

What the whole-section kind carries differs by type.  A Wing section carries its planform --
span, chords, sweep, twist, dihedral -- so the spacing between airfoils travels with it.  A
Fuselage, Stack or Propeller section keeps the place of the one it replaces (a Fuselage its
location, a Stack its steps, turns, spin and V Align, a Propeller its radius) and carries its
curve, tessellation, clustering, skinning and, on a Fuselage, its rotation and spin.

Pasting a whole section into a Geom of another type carries the curve alone, as the curve kind
does.
"""

import openvsp as vsp

import pytest


# Section Parms set on the source and checked on the target, per Geom type: the ones the
# whole-section kind carries and the ones every kind leaves where they were.
CARRIED = {
    "WING": { "Span": 3.5, "Sweep": 22.0, "Dihedral": 7.0, "Twist": -3.0 },
    "FUSELAGE": { "XRotate": 11.0, "Spin": 0.25, "SectTess_U": 9, "FwdCluster": 1.7, "TopLAngle": 17.0 },
    "STACK": { "SectTess_U": 9, "FwdCluster": 1.7, "TopLAngle": 17.0 },
    "PROP": { "SectTess_U": 9 },
}

KEPT = {
    "WING": [],
    "FUSELAGE": [ "XLocPercent", "ZLocPercent" ],
    "STACK": [ "XDelta", "ZDelta", "XRotate", "Spin", "VAlign" ],
    "PROP": [ "RadiusFrac" ],
}

TYPES = [ "WING", "FUSELAGE", "STACK", "PROP" ]

SOURCE = 1
TARGET = 2


def curve_marker():
    """A shape type unlike any type's default, and a Parm of it no planform drives."""
    return vsp.XS_SUPER_ELLIPSE, "Super_M", 3.3


def add_geom( gtype ):
    """A Geom of the type with at least three sections, and an empty error stack."""
    gid = vsp.AddGeom( gtype )
    vsp.Update()
    if gtype == "WING":
        vsp.InsertXSec( gid, 1, vsp.XS_FOUR_SERIES )
        vsp.Update()

    clear_errors()
    return gid


def clear_errors():
    err_mgr = vsp.ErrorMgrSingleton.getInstance()
    while err_mgr.GetNumTotalErrors() > 0:
        err_mgr.PopLastError()


def num_errors():
    return vsp.ErrorMgrSingleton.getInstance().GetNumTotalErrors()


def section_val( xsec_id, name ):
    return vsp.GetParmVal( vsp.GetXSecParm( xsec_id, name ) )


def make_source( gtype ):
    """A Geom whose source section has a marked curve and every CARRIED Parm set."""
    gid = add_geom( gtype )
    xsurf = vsp.GetXSecSurf( gid, 0 )

    shape, pname, pval = curve_marker()
    vsp.ChangeXSecShape( xsurf, SOURCE, shape )
    xs = vsp.GetXSec( xsurf, SOURCE )
    vsp.SetParmVal( vsp.GetXSecParm( xs, pname ), pval )

    if gtype == "FUSELAGE" or gtype == "STACK":
        vsp.SetParmVal( vsp.GetXSecParm( xs, "TopLAngleSet" ), 1.0 )

    for name, val in CARRIED[ gtype ].items():
        vsp.SetParmVal( vsp.GetXSecParm( xs, name ), val )

    vsp.Update()
    return gid


def make_target( gtype ):
    """A Geom whose target section has every KEPT Parm set apart from the source's."""
    gid = add_geom( gtype )
    xs = vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), TARGET )

    if gtype == "STACK":
        vsp.SetParmVal( vsp.GetXSecParm( xs, "XDelta" ), 2.75 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "ZDelta" ), 0.4 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "XRotate" ), 5.0 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "Spin" ), -0.2 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "VAlign" ), 0.3 )
    if gtype == "FUSELAGE":
        vsp.SetParmVal( vsp.GetXSecParm( xs, "ZLocPercent" ), 0.05 )

    vsp.Update()
    return gid


def snapshot( gid, index, names ):
    xs = vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), index )
    return { name: section_val( xs, name ) for name in names }


def all_section_names( gtype ):
    return list( CARRIED[ gtype ].keys() ) + KEPT[ gtype ]


def curve_is_marked( gid, index, gtype ):
    shape, pname, pval = curve_marker()
    xs = vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), index )
    return vsp.GetXSecShape( xs ) == shape and section_val( xs, pname ) == pytest.approx( pval )


def check_whole_section( src, dst, gtype, before ):
    """The target took the source's curve and CARRIED Parms and kept its own KEPT Parms."""
    assert curve_is_marked( dst, TARGET, gtype )

    after = snapshot( dst, TARGET, all_section_names( gtype ) )
    for name, val in CARRIED[ gtype ].items():
        assert after[ name ] == pytest.approx( val ), name
    for name in KEPT[ gtype ]:
        assert after[ name ] == pytest.approx( before[ name ] ), name


def check_curve_alone( dst, gtype, before ):
    """The target took the source's curve and kept every section Parm."""
    assert curve_is_marked( dst, TARGET, gtype )

    after = snapshot( dst, TARGET, all_section_names( gtype ) )
    for name in after:
        assert after[ name ] == pytest.approx( before[ name ] ), name


@pytest.mark.parametrize( "gtype", TYPES )
def test_whole_section_within_one_geom( gtype ):
    vsp.VSPRenew()
    gid = make_source( gtype )
    before = snapshot( gid, TARGET, all_section_names( gtype ) )
    assert not curve_is_marked( gid, TARGET, gtype )

    vsp.CopyXSec( gid, SOURCE )
    vsp.PasteXSec( gid, TARGET )
    vsp.Update()

    assert num_errors() == 0
    check_whole_section( gid, gid, gtype, before )


@pytest.mark.parametrize( "gtype", TYPES )
def test_curve_alone_within_one_geom( gtype ):
    vsp.VSPRenew()
    gid = make_source( gtype )
    before = snapshot( gid, TARGET, all_section_names( gtype ) )
    assert not curve_is_marked( gid, TARGET, gtype )

    vsp.CopyXSecCurve( gid, SOURCE )
    vsp.PasteXSecCurve( gid, TARGET )
    vsp.Update()

    assert num_errors() == 0
    check_curve_alone( gid, gtype, before )


@pytest.mark.parametrize( "gtype", TYPES )
def test_whole_section_across_instances( gtype ):
    vsp.VSPRenew()
    src = make_source( gtype )
    dst = make_target( gtype )
    before = snapshot( dst, TARGET, all_section_names( gtype ) )
    assert not curve_is_marked( dst, TARGET, gtype )

    vsp.CopyXSec( src, SOURCE )
    vsp.PasteXSec( dst, TARGET )
    vsp.Update()

    assert num_errors() == 0
    check_whole_section( src, dst, gtype, before )


@pytest.mark.parametrize( "gtype", TYPES )
def test_curve_alone_across_instances( gtype ):
    vsp.VSPRenew()
    src = make_source( gtype )
    dst = make_target( gtype )
    before = snapshot( dst, TARGET, all_section_names( gtype ) )
    assert not curve_is_marked( dst, TARGET, gtype )

    vsp.CopyXSecCurve( src, SOURCE )
    vsp.PasteXSecCurve( dst, TARGET )
    vsp.Update()

    assert num_errors() == 0
    check_curve_alone( dst, gtype, before )


@pytest.mark.parametrize( "gtype", TYPES )
def test_by_id_forms_match( gtype ):
    vsp.VSPRenew()
    src = make_source( gtype )
    dst = make_target( gtype )
    src_xs = vsp.GetXSec( vsp.GetXSecSurf( src, 0 ), SOURCE )
    dst_xs = vsp.GetXSec( vsp.GetXSecSurf( dst, 0 ), TARGET )
    before = snapshot( dst, TARGET, all_section_names( gtype ) )
    assert not curve_is_marked( dst, TARGET, gtype )

    vsp.CopyXSecCurveByID( src_xs )
    vsp.PasteXSecCurveByID( dst_xs )
    vsp.Update()
    check_curve_alone( dst, gtype, before )

    vsp.CopyXSecByID( src_xs )
    vsp.PasteXSecByID( dst_xs )
    vsp.Update()
    check_whole_section( src, dst, gtype, before )

    assert num_errors() == 0


TYPE_PAIRS = [ ( a, b ) for a in TYPES for b in TYPES if a != b ]


@pytest.mark.parametrize( "src_type,dst_type", TYPE_PAIRS )
@pytest.mark.parametrize( "whole", [ True, False ] )
def test_across_types_carries_the_curve_alone( src_type, dst_type, whole ):
    """Between two types either kind carries the curve and leaves every section Parm."""
    vsp.VSPRenew()
    src = make_source( src_type )
    dst = make_target( dst_type )
    before = snapshot( dst, TARGET, all_section_names( dst_type ) )
    assert not curve_is_marked( dst, TARGET, dst_type )

    if whole:
        vsp.CopyXSec( src, SOURCE )
        vsp.PasteXSec( dst, TARGET )
    else:
        vsp.CopyXSecCurve( src, SOURCE )
        vsp.PasteXSecCurve( dst, TARGET )
    vsp.Update()

    assert num_errors() == 0
    check_curve_alone( dst, dst_type, before )


@pytest.mark.parametrize( "gtype", TYPES )
def test_the_two_clipboards_replace_each_other( gtype ):
    """Copying one kind empties the other, so a paste of either kind takes the last copy."""
    vsp.VSPRenew()
    gid = make_source( gtype )
    before = snapshot( gid, TARGET, all_section_names( gtype ) )
    assert not curve_is_marked( gid, TARGET, gtype )

    vsp.CopyXSec( gid, SOURCE )
    vsp.CopyXSecCurve( gid, SOURCE )

    # The curve was copied last, so a whole-section paste brings only the curve.
    vsp.PasteXSec( gid, TARGET )
    vsp.Update()
    check_curve_alone( gid, gtype, before )

    assert num_errors() == 0


@pytest.mark.parametrize( "gtype", TYPES )
def test_a_curve_paste_takes_the_curve_of_a_copied_section( gtype ):
    vsp.VSPRenew()
    gid = make_source( gtype )
    before = snapshot( gid, TARGET, all_section_names( gtype ) )
    assert not curve_is_marked( gid, TARGET, gtype )

    vsp.CopyXSec( gid, SOURCE )
    vsp.PasteXSecCurve( gid, TARGET )
    vsp.Update()
    check_curve_alone( gid, gtype, before )

    assert num_errors() == 0


def test_airfoil_is_the_curve_kind_on_a_wing():
    vsp.VSPRenew()
    src = make_source( "WING" )
    dst = make_target( "WING" )
    before = snapshot( dst, TARGET, all_section_names( "WING" ) )

    vsp.CopyAirfoil( src, SOURCE )
    vsp.PasteAirfoil( dst, TARGET )
    vsp.Update()

    assert num_errors() == 0
    check_curve_alone( dst, "WING", before )



def surface_samples( gid ):
    pts = []
    for i in range( 11 ):
        for j in range( 9 ):
            p = vsp.CompPnt01( gid, 0, i / 10.0, j / 8.0 )
            pts.extend( [ p.x(), p.y(), p.z() ] )
    return pts


@pytest.mark.parametrize( "gtype", TYPES )
@pytest.mark.parametrize( "whole", [ True, False ] )
def test_a_paste_rebuilds_the_surface( gtype, whole, tmp_path ):
    """The surface after a paste changes, and is the one the model builds afresh when read back."""
    vsp.VSPRenew()
    gid = make_source( gtype )
    before = surface_samples( gid )

    if whole:
        vsp.CopyXSec( gid, SOURCE )
        vsp.PasteXSec( gid, TARGET )
    else:
        vsp.CopyXSecCurve( gid, SOURCE )
        vsp.PasteXSecCurve( gid, TARGET )
    vsp.Update()
    after = surface_samples( gid )

    fname = str( tmp_path / "paste.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    reread = surface_samples( vsp.FindGeoms()[0] )

    assert after != pytest.approx( before )
    assert after == pytest.approx( reread, abs = 1e-9 )
    assert num_errors() == 0
