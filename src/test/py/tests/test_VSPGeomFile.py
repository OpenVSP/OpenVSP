"""The VSPAERO geometry file Prepare Geometry writes for a fuselage and wing, against a stored copy.  The
file's structure and every integer must match; real numbers may differ only in their last bits, as they
can between platforms."""

import os

import openvsp as vsp

REFERENCE = os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), 'PrepareGeometryFuselageWing.vspgeom' )


def is_real( tok ):
    """A number that is not an integer."""
    try:
        int( tok )
        return False
    except ValueError:
        pass
    try:
        float( tok )
        return True
    except ValueError:
        return False


def write_vspgeom( stem ):
    vsp.VSPRenew()
    fid = vsp.AddGeom( 'FUSELAGE' )
    vsp.SetParmVal( fid, 'Tess_U', 'Shape', 6 )
    vsp.SetParmVal( fid, 'Tess_W', 'Shape', 9 )
    wid = vsp.AddGeom( 'WING' )
    vsp.SetParmVal( wid, 'X_Rel_Location', 'XForm', 2.0 )
    vsp.SetParmVal( wid, 'Tess_W', 'Shape', 9 )
    vsp.Update()
    vsp.SetVSP3FileName( stem + '.vsp3' )
    vsp.SetAnalysisInputDefaults( 'VSPAEROComputeGeometry' )
    vsp.SetIntAnalysisInput( 'VSPAEROComputeGeometry', 'GeomSet', [ vsp.SET_ALL ] )
    vsp.SetIntAnalysisInput( 'VSPAEROComputeGeometry', 'ThinGeomSet', [ vsp.SET_NONE ] )
    vsp.ExecAnalysis( 'VSPAEROComputeGeometry' )
    return stem + '.vspgeom'


def test_prepare_geometry_writes_the_stored_file( tmp_path ):
    with open( write_vspgeom( os.path.join( str( tmp_path ), 'fuse_wing' ) ) ) as f:
        got = f.read().splitlines()
    with open( REFERENCE ) as f:
        want = f.read().splitlines()

    assert len( got ) == len( want )
    for n, ( g, w ) in enumerate( zip( got, want ) ):
        gt = g.split()
        wt = w.split()
        assert len( gt ) == len( wt ), 'line %d: %s != %s' % ( n + 1, g, w )
        for a, b in zip( gt, wt ):
            if is_real( b ):
                assert abs( float( a ) - float( b ) ) <= 1e-12 * ( 1.0 + abs( float( b ) ) ), 'line %d: %s != %s' % ( n + 1, g, w )
            else:
                assert a == b, 'line %d: %s != %s' % ( n + 1, g, w )
