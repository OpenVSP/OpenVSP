"""CompGeom decides whole regions of tris at once and lets the pieces of a split tri share answers.  A
tri lying on another mesh's face is decided by the faces, which its neighbors are not, so these models
put such faces where a region or a split tri's pieces would otherwise carry the wrong answer to them.
The expected areas are those deciding every tri on its own gives."""

import os

import pytest

import openvsp as vsp


def box_tris( lo, hi, n ):
    """A box's faces as tris facing out, each face an n[u] by n[v] grid of squares split in two."""
    tris = []
    for ax in range( 3 ):
        u, v = [ a for a in range( 3 ) if a != ax ]
        for side in ( 0, 1 ):
            def pnt( i, j ):
                p = [ 0.0, 0.0, 0.0 ]
                p[ax] = hi[ax] if side else lo[ax]
                p[u] = lo[u] + ( hi[u] - lo[u] ) * i / n[u]
                p[v] = lo[v] + ( hi[v] - lo[v] ) * j / n[v]
                return p
            for i in range( n[u] ):
                for j in range( n[v] ):
                    a, b, c, d = pnt( i, j ), pnt( i + 1, j ), pnt( i + 1, j + 1 ), pnt( i, j + 1 )
                    for t in ( ( a, b, c ), ( a, c, d ) ):
                        e1 = [ t[1][k] - t[0][k] for k in range( 3 ) ]
                        e2 = [ t[2][k] - t[0][k] for k in range( 3 ) ]
                        # The normal's component along ax; u, v, ax are cyclic only for ax 1
                        nax = e1[u] * e2[v] - e1[v] * e2[u]
                        if ax == 1:
                            nax = -nax
                        if ( nax > 0.0 ) != ( side == 1 ):
                            t = ( t[0], t[2], t[1] )
                        tris.append( t )
    return tris


def write_stl( fn, tris ):
    with open( fn, 'w' ) as f:
        f.write( 'solid s\n' )
        for t in tris:
            e1 = [ t[1][k] - t[0][k] for k in range( 3 ) ]
            e2 = [ t[2][k] - t[0][k] for k in range( 3 ) ]
            nrm = [ e1[1] * e2[2] - e1[2] * e2[1], e1[2] * e2[0] - e1[0] * e2[2], e1[0] * e2[1] - e1[1] * e2[0] ]
            mag = ( nrm[0] ** 2 + nrm[1] ** 2 + nrm[2] ** 2 ) ** 0.5
            f.write( ' facet normal %.17g %.17g %.17g\n  outer loop\n' % tuple( c / mag for c in nrm ) )
            for p in t:
                f.write( '   vertex %.17g %.17g %.17g\n' % tuple( p ) )
            f.write( '  endloop\n endfacet\n' )
        f.write( 'endsolid s\n' )


def wet_area( tmp_path, boxes, rot=0.0 ):
    """CompGeom's total wet area for boxes, each ( lo, hi, n ), turned alike by rot degrees."""
    vsp.VSPRenew()
    for k, ( lo, hi, n ) in enumerate( boxes ):
        fn = os.path.join( str( tmp_path ), 'box%d.stl' % k )
        write_stl( fn, box_tris( lo, hi, n ) )
        gid = vsp.ImportFile( fn, vsp.IMPORT_STL, '' )
        if rot != 0.0:
            vsp.SetParmVal( gid, 'X_Rel_Rotation', 'XForm', rot )
            vsp.SetParmVal( gid, 'Y_Rel_Rotation', 'XForm', rot * 0.7 )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( 'CompGeom' )
    vsp.SetIntAnalysisInput( 'CompGeom', 'WriteTXTFlag', [ 0 ] )
    vsp.SetIntAnalysisInput( 'CompGeom', 'WriteCSVFlag', [ 0 ] )
    vsp.ExecAnalysis( 'CompGeom' )
    rid = vsp.FindLatestResultsID( 'Comp_Geom' )
    return sum( vsp.GetDoubleResults( rid, 'Wet_Area' ) )


BIG = ( ( 0.0, 0.0, -10.0 ), ( 10.0, 10.0, 0.0 ), ( 20, 20, 20 ) )


def small_box( gap ):
    """A 0.6 cube over the big box's top face, gap above it."""
    return ( ( 4.95, 4.95, gap ), ( 5.55, 5.55, gap + 0.6 ), ( 4, 4, 4 ) )


@pytest.mark.parametrize( "gap", [ 1e-12, 1e-11, 1e-10 ] )
def test_a_face_just_above_another_is_decided_by_the_faces( tmp_path, gap ):
    # The big box's tris under the small one lie on its face, inside one region with the rest of the top
    assert wet_area( tmp_path, [ BIG, small_box( gap ) ] ) == pytest.approx( 601.550001, abs=1e-5 )


def test_turned_faces_on_each_other_are_decided_by_the_faces( tmp_path ):
    assert wet_area( tmp_path, [ BIG, small_box( 0.0 ) ], rot=30.0 ) == pytest.approx( 601.531951, abs=1e-5 )


@pytest.mark.parametrize( "slab, small, expected", [
    ( ( ( 3.9, 4.3, -0.2 ), ( 6.3, 4.7, 0.2 ), ( 8, 2, 2 ) ),
      ( ( 4.2, 4.0, 1e-11 ), ( 4.8, 4.6, 0.6 ), ( 2, 2, 2 ) ), 602.155 ),
    ( ( ( 4.3, 3.3, -0.25 ), ( 4.7, 6.7, 0.25 ), ( 2, 8, 2 ) ),
      ( ( 3.9, 4.1, 1e-11 ), ( 4.5, 4.7, 0.6 ), ( 3, 3, 3 ) ), 603.0075 ),
] )
def test_a_split_tri_on_another_face_decides_each_piece( tmp_path, slab, small, expected ):
    # The slab splits the big box's top tris; the small box lies on some of the pieces but crosses none
    assert wet_area( tmp_path, [ BIG, slab, small ] ) == pytest.approx( expected, abs=1e-5 )
