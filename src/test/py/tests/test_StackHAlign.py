"""H Align moves a Stack section across its width, as V Align moves it across its height.

A Stack's sections lie in the Stack's YZ plane: width along Y, height along Z.  At H Align 1 the
section's +Y side lies on the Stack's line through the section centers, at -1 its -Y side does.
"""

import os

import openvsp as vsp

TESTS = os.path.dirname( os.path.abspath( __file__ ) )


def new_stack():
    vsp.VSPRenew()
    sid = vsp.AddGeom( "STACK" )
    vsp.Update()
    return sid


def xsec( sid, i ):
    return vsp.GetXSec( vsp.GetXSecSurf( sid, 0 ), i )


def setx( sid, i, name, val ):
    vsp.SetParmVal( vsp.GetXSecParm( xsec( sid, i ), name ), val )


def getx( sid, i, name ):
    return vsp.GetParmVal( vsp.GetXSecParm( xsec( sid, i ), name ) )


def points( sid ):
    vsp.Update()
    out = []
    for iu in range( 41 ):
        for iw in range( 21 ):
            p = vsp.CompPnt01( sid, 0, iu / 40.0, iw / 20.0 )
            out.append( ( p.x(), p.y(), p.z() ) )
    return out


def section( sid, i, n = 64 ):
    """Points around section i ( u = i / 4 on a new Stack )."""
    vsp.Update()
    return [ vsp.CompPnt01( sid, 0, i / 4.0, k / float( n ) ) for k in range( n ) ]


def test_default_is_zero_and_changes_nothing():
    sid = new_stack()
    a = points( sid )
    for i in range( 5 ):
        assert getx( sid, i, "HAlign" ) == 0.0
        setx( sid, i, "HAlign", 0.0 )
    assert points( sid ) == a


def test_one_puts_the_plus_y_side_on_the_line():
    sid = new_stack()
    w = getx( sid, 2, "Ellipse_Width" )
    before = section( sid, 2 )
    nxt = section( sid, 3 )
    setx( sid, 2, "HAlign", 1.0 )
    after = section( sid, 2 )
    for p, q in zip( before, after ):
        assert abs( q.y() - ( p.y() - 0.5 * w ) ) < 1e-9
        assert abs( q.x() - p.x() ) < 1e-9 and abs( q.z() - p.z() ) < 1e-9
    assert abs( max( q.y() for q in after ) ) < 1e-9
    # The move is the section's alone: the next is placed as at H Align 0.
    for p, q in zip( nxt, section( sid, 3 ) ):
        assert vsp.dist( p, q ) < 1e-9


def test_minus_one_puts_the_minus_y_side_on_the_line():
    sid = new_stack()
    setx( sid, 2, "HAlign", -1.0 )
    assert abs( min( q.y() for q in section( sid, 2 ) ) ) < 1e-9


def test_shift_is_a_fraction_of_the_width():
    sid = new_stack()
    setx( sid, 2, "Ellipse_Width", 4.0 )
    before = section( sid, 2 )
    setx( sid, 2, "HAlign", 0.3 )
    for p, q in zip( before, section( sid, 2 ) ):
        assert abs( q.y() - ( p.y() - 0.3 * 0.5 * 4.0 ) ) < 1e-9


def test_h_and_v_align_together():
    sid = new_stack()
    w = getx( sid, 2, "Ellipse_Width" )
    h = getx( sid, 2, "Ellipse_Height" )
    before = section( sid, 2 )
    setx( sid, 2, "HAlign", 1.0 )
    setx( sid, 2, "VAlign", -1.0 )
    for p, q in zip( before, section( sid, 2 ) ):
        assert abs( q.y() - ( p.y() - 0.5 * w ) ) < 1e-9
        assert abs( q.z() - ( p.z() + 0.5 * h ) ) < 1e-9


def test_saved_and_read_back( tmp_path ):
    sid = new_stack()
    setx( sid, 2, "HAlign", 0.7 )
    a = points( sid )
    fname = str( tmp_path / "halign.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    sid = vsp.FindGeoms()[0]
    assert getx( sid, 2, "HAlign" ) == 0.7
    assert points( sid ) == a


def test_a_file_without_it_reads_zero():
    vsp.VSPRenew()
    vsp.ReadVSPFile( os.path.join( TESTS, "CfdConeNose.vsp3" ) )
    vsp.Update()
    n = 0
    for gid in vsp.FindGeoms():
        if vsp.GetGeomTypeName( gid ) == "Stack":
            for i in range( vsp.GetNumXSec( vsp.GetXSecSurf( gid, 0 ) ) ):
                assert getx( gid, i, "HAlign" ) == 0.0
                n += 1
    assert n > 0


def test_a_pasted_section_keeps_the_place_it_replaces():
    """Like V Align, H Align is part of a Stack section's place, which a paste keeps."""
    sid = new_stack()
    for name in ( "HAlign", "VAlign" ):
        setx( sid, 1, name, 0.4 )
        setx( sid, 3, name, 0.2 )
    vsp.Update()
    vsp.CopyXSec( sid, 1 )
    vsp.PasteXSec( sid, 3 )
    vsp.Update()
    for name in ( "HAlign", "VAlign" ):
        assert getx( sid, 3, name ) == 0.2


def test_a_loop_closes_on_the_first_section():
    sid = new_stack()
    setx( sid, 0, "HAlign", 0.5 )
    vsp.SetParmVal( sid, "OrderPolicy", "Design", vsp.STACK_LOOP )
    vsp.Update()
    n = vsp.GetNumXSec( vsp.GetXSecSurf( sid, 0 ) )
    assert getx( sid, n - 1, "HAlign" ) == 0.5
