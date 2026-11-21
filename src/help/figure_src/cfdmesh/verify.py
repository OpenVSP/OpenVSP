"""Checks the numbers and directions CFDMesh.md states against VSP.

    <build>/venv/bin/python verify.py

Each check meshes a small model through the openvsp module and reads the mesh back from the Cart3D .tri file it
wrote.  It prints one line per check and exits non-zero if any fails.
"""

import os
import sys
import math
import time

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, HERE )

import numpy as np
import openvsp as vsp
import cfdmodel as cm

FAILS = []


def check( ok, what ):
    print( ( "ok    " if ok else "FAIL  " ) + what )
    if not ok:
        FAILS.append( what )


def result( name ):
    rid = vsp.FindLatestResultsID( "CFDMesh" )
    return vsp.GetIntResults( rid, name )[ 0 ]


def defaults():
    vsp.VSPRenew()
    want = { vsp.CFD_MAX_EDGE_LEN: 0.5, vsp.CFD_MIN_EDGE_LEN: 0.025, vsp.CFD_MAX_GAP: 0.005,
             vsp.CFD_NUM_CIRCLE_SEGS: 16.0, vsp.CFD_GROWTH_RATIO: 1.3, vsp.CFD_LIMIT_GROWTH_FLAG: 0.0,
             vsp.CFD_INTERSECT_SUBSURFACE_FLAG: 1.0, vsp.CFD_FAR_MAX_EDGE_LEN: 2.0, vsp.CFD_FAR_MAX_GAP: 0.02,
             vsp.CFD_FAR_NUM_CIRCLE_SEGS: 16.0, vsp.CFD_FAR_X_SCALE: 4.0, vsp.CFD_FAR_Y_SCALE: 4.0,
             vsp.CFD_FAR_Z_SCALE: 4.0, vsp.CFD_FAR_LENGTH: 4.0, vsp.CFD_FAR_WIDTH: 4.0, vsp.CFD_FAR_HEIGHT: 4.0,
             vsp.CFD_SPLIT_JOIN_SURFS_FLAG: 0.0, vsp.CFD_PARALLEL_MESH_FLAG: 1.0, vsp.CFD_POGS_NUM_REFINE: 0.0,
             vsp.CFD_TOPO_INTERIOR_FLAG: 1.0, vsp.CFD_HALF_MESH_FLAG: 0.0, vsp.CFD_FAR_FIELD_FLAG: 0.0 }
    bad = [ k for k, v in want.items() if abs( vsp.GetCFDMeshVal( k ) - v ) > 1e-12 ]
    check( not bad, "defaults of the CFD_CONTROL_TYPE settings" )
    other = [ ( cm.density( "WidthLimit" ), 1.0 ), ( cm.density( "NWidthSeg" ), 1.0 ),
               ( cm.setting( "ConvertToQuadsFlag", "Global" ), 0.0 ),
               ( cm.setting( "SymmetrySplitting", "FarField" ), 0.0 ),
              ( cm.setting( "RelCurveTol", "Global" ), 0.005 ), ( cm.setting( "DemoteSurfsCubicFlag", "Global" ), 0.0 ),
              ( cm.setting( "CubicSurfTolerance", "Global" ), 1e-6 ), ( cm.setting( "Set", "Global" ), vsp.SET_SHOWN ),
              ( cm.setting( "DegenSet", "Global" ), vsp.SET_NONE ) ]
    check( all( abs( vsp.GetParmVal( p ) - v ) < 1e-12 for p, v in other ), "defaults of the other settings" )
    flags = [ vsp.GetParmVal( vsp.FindParm( vsp.FindContainer( "CFDMeshSettings", 0 ), n, "ExportCFD" ) )
              for n in [ "STL_Export", "POLY_Export", "TRI_Export", "OBJ_Export", "DAT_Export", "KEY_Export",
                         "GMSH_Export", "TKEY_Export", "FACET_Export", "VSPGEOM_Export" ] ]
    pogs = vsp.GetParmVal( vsp.FindParm( vsp.FindContainer( "CFDMeshSettings", 0 ), "POGS_Export", "ExportCFD" ) )
    check( all( f == 1.0 for f in flags ) and pogs == 0.0, "every file on by default except POGS" )
    w = vsp.AddGeom( "WING" )
    check( vsp.GetParmVal( w, "WakeScale", "WakeSettings" ) == 2.0 and
           vsp.GetParmVal( w, "WakeAngle", "WakeSettings" ) == 0.0,
           "wake defaults: Scale Wake 2, Wake Angle 0" )


def curvature():
    """Spheres meshed at the defaults have median edges near the rule of Figure 2; and the crossover radii."""
    g, N, lmin, lmax = 0.005, 16.0, 0.025, 0.5
    worst = 0.0
    for r in [ 0.1, 0.2, 0.3, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0 ]:
        vsp.VSPRenew()
        e = vsp.AddGeom( "ELLIPSOID" )
        for n in [ "A_Radius", "B_Radius", "C_Radius" ]:
            vsp.SetParmVal( e, n, "Design", r )
        vsp.Update()
        V, T, tag = cm.mesh( "sphere" )
        E, L = cm.edges( V, T )
        glen = 2.0 * math.sqrt( 2.0 * r * g - g * g )
        nlen = 2.0 * r * math.sin( math.pi / N )
        t = min( max( min( glen, nlen ), lmin ), lmax )
        worst = max( worst, abs( np.median( L ) / t - 1.0 ) )
    check( worst < 0.07, "sphere edges follow the curvature rules to within 7%% (worst %.3f)" % worst )
    rx = g / ( 1.0 - math.cos( math.pi / N ) )
    rmin = lmin / ( 2.0 * math.sin( math.pi / N ) )
    rmax = ( ( lmax / 2.0 ) ** 2 + g * g ) / ( 2.0 * g )
    check( abs( rx - 0.26 ) < 0.005 and abs( rmin - 0.064 ) < 0.0005 and abs( rmax - 6.25 ) < 0.005,
           "crossovers at the defaults: %.3f, %.4f, %.3f" % ( rx, rmin, rmax ) )


def width():
    import cfdmesh_figs as F
    out = {}
    for on in ( 0.0, 1.0 ):
        F.blunt_wing()
        cm.coarse()
        vsp.SetParmVal( cm.density( "WidthLimit" ), on )
        vsp.Update()
        V, T, tag = cm.mesh( "blunt" )
        a = cm.min_angles( V, T )
        out[ on ] = ( 100.0 * ( a < 17 ).mean(), int( ( a < 5 ).sum() ) )
    print( "      width limit off: %.2f%% < 17 deg, %d < 5 deg; on: %.2f%%, %d" % ( out[ 0.0 ] + out[ 1.0 ] ) )
    check( abs( out[ 0.0 ][ 0 ] - 1.7 ) < 0.05 and out[ 0.0 ][ 1 ] == 56 and
           out[ 1.0 ][ 0 ] < 0.05 and out[ 1.0 ][ 1 ] == 0,
           "blunt trailing edge: 1.7% and 56 under 5 deg off, 0.04% and none on" )


def point_source():
    import cfdmesh_figs as F
    x, y = F.source_profile()
    d = np.linspace( 0, 7, 1401 )
    law = F.point_source_len( d )
    lim = F.limited_len( d, law )
    k = np.argmax( np.abs( lim - law ) > 1e-9 )
    top = d[ np.argmax( lim >= F.SRC_BASE - 1e-9 ) ]
    check( abs( d[ k ] - 2.53 ) < 0.01 and abs( lim[ k ] - 0.43 ) < 0.005 and abs( top - 4.43 ) < 0.01,
           "the growth limit takes over from the source at d 2.53 (t 0.43) and reaches Max Edge Len at 4.43" )
    want = np.interp( x, d, lim )
    raw = np.interp( x, d, law )
    ratio = y / want
    check( np.all( ( ratio > 0.8 ) & ( ratio < 1.7 ) ), "edges near a point source follow the limited target "
           "(%.2f..%.2f)" % ( ratio.min(), ratio.max() ) )
    held = ( x > 2.6 ) & ( x < 4.4 )
    check( np.mean( np.abs( y[ held ] / want[ held ] - 1.0 ) ) < np.mean( np.abs( y[ held ] / raw[ held ] - 1.0 ) ),
           "between 2.6 and 4.4 the edges follow the growth limit, not the source alone" )


def wing_sources():
    import cfdmesh_figs as F
    w = F.wing_alone()
    # AddDefaultSources adds nothing while CFD Mesh still holds the previous model's sources.
    vsp.DeleteAllCFDSources()
    vsp.AddDefaultSources()
    ids = vsp.GetCFDSourceIDVec( w )
    names = [ vsp.GetCFDSourceNameByID( s ) for s in ids ]
    te = ids[ names.index( "Def_TE_LS_1" ) ]
    v = lambda n: vsp.GetParmVal( vsp.FindParm( te, n, "Source" ) )
    check( abs( v( "SrcLen" ) - 0.02 ) < 1e-12 and abs( v( "SrcLen2" ) - 0.01 ) < 1e-12 and
           abs( v( "SrcRad" ) - 0.4 ) < 1e-12 and abs( v( "SrcRad2" ) - 0.2 ) < 1e-12,
           "default wing sources: Len 1% and Rad 20% of the local chord" )
    check( sorted( names ) == [ "Def_LE_LS_1", "Def_TE_LS_1", "Def_Tip_LS" ],
           "one LE and one TE line per section, one tip" )
    check( abs( v( "U_Loc1" ) - 1.0 / 3.0 ) < 1e-12 and abs( v( "U_Loc2" ) - 2.0 / 3.0 ) < 1e-12,
           "a section and each cap take an equal share of U" )
    p0 = vsp.CompPnt01( w, 0, 0.5, 0.0 )
    p25 = vsp.CompPnt01( w, 0, 0.5, 0.25 )
    p50 = vsp.CompPnt01( w, 0, 0.5, 0.5 )
    p75 = vsp.CompPnt01( w, 0, 0.5, 0.75 )
    check( p0.x() > p25.x() > p50.x() and p25.z() < 0.0 < p75.z(), "wing W: TE, lower, LE at 0.5, upper" )


def domain():
    f, w, h = cm.aircraft()
    cm.coarse()
    # A component outside the set still sizes the far field box and the wake.
    far = vsp.AddGeom( "POD" )
    vsp.SetParmVal( far, "X_Rel_Location", "XForm", 30.0 )
    vsp.SetParmVal( far, "Length", "Design", 2.0 )
    for g in [ f, w, h ]:
        vsp.SetSetFlag( g, vsp.SET_FIRST_USER, True )
    vsp.Update()
    bbmin = np.array( [ 0.0, -5.0, -1.5 ] )
    bbmax = np.array( [ 32.0, 5.0, 1.5 ] )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 1.0 )
    V, T, tag = cm.mesh( "far", set = vsp.SET_FIRST_USER )
    P = V[ T[ tag == tag.max() ] ].reshape( -1, 3 )
    c = 0.5 * ( bbmin + bbmax )
    want_lo = c - 2.0 * ( bbmax - bbmin )
    want_hi = c + 2.0 * ( bbmax - bbmin )
    check( np.allclose( P.min( 0 ), want_lo, atol = 1e-6 ) and np.allclose( P.max( 0 ), want_hi, atol = 1e-6 ),
           "far field box: 4 times the bounding box of every component, centered" )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_LOC_MAN_FLAG, 1.0 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_LOC_X, -50.0 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_LOC_Y, 3.0 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_LOC_Z, 7.0 )
    V, T, tag = cm.mesh( "far", set = vsp.SET_FIRST_USER )
    P = V[ T[ tag == tag.max() ] ].reshape( -1, 3 )
    lo, hi = P.min( 0 ), P.max( 0 )
    check( abs( lo[ 0 ] + 50.0 ) < 1e-6 and abs( 0.5 * ( lo[ 1 ] + hi[ 1 ] ) - 3.0 ) < 1e-6 and
           abs( 0.5 * ( lo[ 2 ] + hi[ 2 ] ) - 7.0 ) < 1e-6,
           "Man: X Loc is the upstream face, Y Loc and Z Loc the center" )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_LOC_MAN_FLAG, 0.0 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_SIZE_ABS_FLAG, 1.0 )
    V, T, tag = cm.mesh( "far", set = vsp.SET_FIRST_USER )
    P = V[ T[ tag == tag.max() ] ].reshape( -1, 3 )
    want = 4.0 * ( bbmax - bbmin )
    check( np.allclose( P.max( 0 ) - P.min( 0 ), want, atol = 1e-6 ),
           "Abs keeps the box the Rel scales made: its lengths follow the scales" )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_SIZE_ABS_FLAG, 0.0 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 0.0 )
    # At Scale Wake 1 the wake ends half the largest dimension behind the last point of every component.
    vsp.SetCFDWakeFlag( w, True )
    vsp.SetParmVal( w, "WakeAngle", "WakeSettings", 10.0 )
    vsp.SetParmVal( w, "WakeScale", "WakeSettings", 1.0 )
    vsp.Update()
    V, T, tag = cm.mesh( "wake", set = vsp.SET_FIRST_USER )
    P = V[ T[ tag >= 1000 ] ].reshape( -1, 3 )
    xend = bbmax[ 0 ] + 0.5 * np.max( bbmax - bbmin )
    check( abs( P[ :, 0 ].max() - xend ) < 1e-6,
           "at Scale Wake 1 the wake ends at %.1f, half the largest dimension behind the model" % xend )
    k = np.argmax( P[ :, 0 ] )
    x0 = 5.38
    check( P[ k, 2 ] > -0.25 + 0.9 * ( xend - x0 ) * math.tan( math.radians( 10.0 ) ),
           "a positive Wake Angle raises the wake" )
    check( bool( np.all( tag[ tag >= 1000 ] > 1000 ) ), "wake tags are above 1000" )
    # The stretch starts 1% of the largest dimension behind the model; past it the length is multiplied by the scale.
    xs = bbmax[ 0 ] + 0.01 * np.max( bbmax - bbmin )
    ok = True
    for sc in ( 2.0, 4.0 ):
        vsp.SetParmVal( w, "WakeScale", "WakeSettings", sc )
        vsp.Update()
        V, T, tag = cm.mesh( "wake", set = vsp.SET_FIRST_USER )
        end = V[ T[ tag >= 1000 ] ].reshape( -1, 3 )[ :, 0 ].max()
        print( "      Scale Wake %.0f: the wake ends at %.3f" % ( sc, end ) )
        ok = ok and abs( end - ( xs + sc * ( xend - xs ) ) ) < 1e-6
    check( ok, "Scale Wake multiplies the wake's length past the model" )


def half():
    f, w, h = cm.aircraft()
    cm.coarse()
    vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, 1.0 )
    V, T, tag = cm.mesh( "half" )
    on0 = np.all( np.abs( V[ T ][ :, :, 1 ] ) < 1e-9, axis = 1 )
    check( V[ :, 1 ].min() > -1e-9 and on0.sum() == 0 and result( "Water_Tight" ) == 1,
           "half mesh alone: +Y side, no symmetry plane, open edges along Y = 0 not counted" )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 1.0 )
    V, T, tag = cm.mesh( "half" )
    on0 = np.all( np.abs( V[ T ][ :, :, 1 ] ) < 1e-9, axis = 1 )
    check( on0.sum() > 0 and result( "Water_Tight" ) == 1, "half mesh in a far field: symmetry plane meshed, closed" )


def repeat_shown():
    vsp.VSPRenew()
    vsp.AddGeom( "POD" )
    vsp.Update()
    first = cm.mesh( "shown", set = vsp.SET_SHOWN )
    second = cm.mesh( "shown", set = vsp.SET_SHOWN )
    check( first is not None and second is None, "a second run on Shown meshes nothing and writes no file" )


def quads_threads_splitjoin():
    f, w, h = cm.aircraft()
    cm.coarse()
    V, T, tag = cm.mesh( "tri" )
    E, L = cm.edges( V, T )
    tri_med = np.median( L )
    vsp.SetParmVal( cm.setting( "ConvertToQuadsFlag", "Global" ), 1.0 )
    vsp.Update()
    fn = os.path.join( cm.TMP, "q.obj" )
    vsp.SetComputationFileName( vsp.CFD_OBJ_TYPE, fn )
    V, T, tag = cm.mesh( "quad", types = vsp.CFD_TRI_TYPE | vsp.CFD_OBJ_TYPE )
    faces = [ l for l in open( fn ) if l.startswith( "f " ) ]
    nq = sum( len( l.split() ) == 5 for l in faces )
    check( nq == len( faces ) and len( T ) == 2 * nq, "quads kept in .obj, split in two in .tri" )
    # The quads' edges, read from the .obj, against the triangles' edges.
    Vo = np.array( [ [ float( v ) for v in l.split()[ 1:4 ] ] for l in open( fn ) if l.startswith( "v " ) ] )
    Q = np.array( [ [ int( v.split( "/" )[ 0 ] ) - 1 for v in l.split()[ 1: ] ] for l in faces ] )
    qe = np.concatenate( [ np.linalg.norm( Vo[ Q[ :, k ] ] - Vo[ Q[ :, ( k + 1 ) % 4 ] ], axis = 1 )
                           for k in range( 4 ) ] )
    check( abs( np.median( qe ) / tri_med - 1.0 ) < 0.15, "quad edges come out near the triangles' (%.3f vs %.3f)"
           % ( np.median( qe ), tri_med ) )

    for sj, want in ( ( 0.0, 24 ), ( 1.0, 14 ) ):
        f, w, h = cm.aircraft()
        cm.coarse()
        vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, sj )
        cm.mesh( "sj" )
        check( result( "Num_Surfs" ) == want, "Split and Join Surfaces %d: %d patches" % ( sj, want ) )

    counts = {}
    times = {}
    for topo in ( 1.0, 0.0 ):
        f, w, h = cm.aircraft()
        cm.coarse()
        vsp.SetCFDMeshVal( vsp.CFD_TOPO_INTERIOR_FLAG, topo )
        V, T, tag = cm.mesh( "topo" )
        counts[ topo ] = ( len( T ), result( "Num_Topo_Surfs" ) )
    check( counts[ 1.0 ][ 0 ] == counts[ 0.0 ][ 0 ] and counts[ 0.0 ][ 1 ] == 0,
           "Find Interior by Topology gives the same mesh" )
    for th in ( 1.0, 0.0 ):
        f, w, h = cm.aircraft()
        cm.coarse()
        vsp.SetCFDMeshVal( vsp.CFD_PARALLEL_MESH_FLAG, th )
        t0 = time.time()
        V, T, tag = cm.mesh( "thr" )
        times[ th ] = ( time.time() - t0, len( T ) )
    print( "      threads: %.2f s, one thread: %.2f s" % ( times[ 1.0 ][ 0 ], times[ 0.0 ][ 0 ] ) )
    check( times[ 1.0 ][ 1 ] == times[ 0.0 ][ 1 ] and times[ 0.0 ][ 0 ] > 2.0 * times[ 1.0 ][ 0 ],
           "multi-threaded: same mesh, several times faster" )


def subsurfaces():
    f, w, h = cm.aircraft()
    cm.coarse()
    vsp.AddSubSurf( w, vsp.SS_CONTROL )
    vsp.Update()
    V, T, tag = cm.mesh( "ss" )
    n_on = len( np.unique( tag ) )
    vsp.SetCFDMeshVal( vsp.CFD_INTERSECT_SUBSURFACE_FLAG, 0.0 )
    V, T, tag = cm.mesh( "ss" )
    check( n_on == 7 and len( np.unique( tag ) ) == 5, "a sub-surface gets tags of its own only when intersected" )


def degen():
    """A wing in the Degen Set is a flat camber sheet; a body two plates, and each symmetric copy its own."""
    f, w, h = cm.aircraft()
    cm.coarse()
    vsp.SetSetFlag( w, vsp.SET_FIRST_USER, True )
    vsp.Update()
    V, T, tag = cm.mesh( "degen", set = vsp.SET_NONE, degen = vsp.SET_FIRST_USER )
    ext = V.max( 0 ) - V.min( 0 )
    check( ext[ 2 ] < 1e-9 and ext[ 1 ] > 1.0, "Degen Set: a wing is a flat sheet (its camber surface)" )
    planes = {}
    for typ in [ "FUSELAGE", "STACK", "ELLIPSOID", "POD" ]:
        vsp.VSPRenew()
        g = vsp.AddGeom( typ )
        vsp.SetParmVal( g, "Y_Rel_Location", "XForm", 2.0 )
        vsp.SetParmVal( g, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
        vsp.Update()
        cm.coarse()
        V, T, tag = cm.mesh( "degen", set = vsp.SET_NONE, degen = vsp.SET_ALL )
        got = []
        for t in np.unique( tag ):
            P = V[ np.unique( T[ tag == t ] ) ]
            side = int( np.sign( P[ :, 1 ].mean() ) )
            if np.ptp( P[ :, 1 ] ) < 1e-9:
                got.append( ( side, "XZ" ) )
            elif np.ptp( P[ :, 2 ] ) < 1e-9:
                got.append( ( side, "XY" ) )
            else:
                got.append( ( side, "other" ) )
        planes[ typ ] = sorted( got )
    print( "      degen plates:", planes )
    want = [ ( -1, "XY" ), ( -1, "XZ" ), ( 1, "XY" ), ( 1, "XZ" ) ]
    check( all( v == want for v in planes.values() ),
           "Degen Set: each body and its mirror copy give two plates each, in their X-Z and X-Y planes" )


def relative_from_api():
    vsp.VSPRenew()
    vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.SetParmVal( cm.density( "BaseAbsRel" ), vsp.REL )
    vsp.SetParmVal( cm.density( "BaseLenFrac" ), 0.05 )
    vsp.Update()
    cm.mesh( "rel" )
    d = np.array( list( vsp.GetGeomBBoxMax( vsp.FindGeoms()[ 0 ] ) ) ) - np.array( list( vsp.GetGeomBBoxMin( vsp.FindGeoms()[ 0 ] ) ) )
    check( abs( vsp.GetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN ) - 0.05 * np.linalg.norm( d ) ) < 1e-9,
           "from a script, a relative Max Edge Len is a fraction of the meshed components' diagonal" )


GROWTH_PCT = ( 0.03, 0.08, 0.17 )


def growth():
    want = { 1.1: GROWTH_PCT[ 0 ], 1.3: GROWTH_PCT[ 1 ], 2.0: GROWTH_PCT[ 2 ] }
    ok = True
    for g, pct in want.items():
        cm.aircraft()
        cm.coarse( growth = g )
        vsp.DeleteAllCFDSources()
        vsp.AddDefaultSources()
        V, T, tag = cm.mesh( "growth" )
        a = cm.min_angles( V, T )
        print( "      growth %.1f: %d triangles, %.3f%% under 17 deg" % ( g, len( T ), 100.0 * ( a < 17 ).mean() ) )
        ok = ok and abs( 100.0 * ( a < 17 ).mean() - pct ) < 0.006
    check( ok, "growth ratios 1.1, 1.3, 2.0 on the aircraft with wing sources" )


def default_sources_stale():
    """AddDefaultSources counts the model's own sources."""
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.AddCFDSource( vsp.POINT_SOURCE, p, 0, 0.05, 1.0, 0.0, 0.0 )
    cm.mesh( "stale" )
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    vsp.AddDefaultSources()
    before = vsp.GetNumCFDSources( w )
    vsp.DeleteAllCFDSources()
    vsp.AddDefaultSources()
    after = vsp.GetNumCFDSources( w )
    check( before == 3 and after == 3, "AddDefaultSources adds a new model's sources after a run" )
    f, w, h = cm.aircraft()
    vsp.DeleteAllCFDSources()
    vsp.AddDefaultSources()
    n = [ vsp.GetNumCFDSources( g ) for g in ( f, w, h ) ]
    check( n == [ 2, 3, 3 ], "the aircraft's default sources: fuselage 2, wing 3, tail 3 (%s)" % n )


def far_abs_sliders():
    """Under Abs the lengths set the box; and the split rectangle's size."""
    f, w, h = cm.aircraft()
    cm.coarse()
    vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 1.0 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_SIZE_ABS_FLAG, 1.0 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_LENGTH, 60.0 )
    V, T, tag = cm.mesh( "abs" )
    P = V[ T[ tag == tag.max() ] ].reshape( -1, 3 )
    check( abs( ( P.max( 0 ) - P.min( 0 ) )[ 0 ] - 60.0 ) < 1e-6 and
           abs( vsp.GetCFDMeshVal( vsp.CFD_FAR_X_SCALE ) - 6.0 ) < 1e-9,
           "under Abs the box takes its Length, and the Length's scale follows" )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_SIZE_ABS_FLAG, 0.0 )
    vsp.SetParmVal( cm.density( "FarAbsRel" ), vsp.REL )
    vsp.SetParmVal( cm.density( "FarFrac" ), 0.05 )
    for k in ( vsp.CFD_FAR_X_SCALE, vsp.CFD_FAR_Y_SCALE, vsp.CFD_FAR_Z_SCALE ):
        vsp.SetCFDMeshVal( k, 4.0 )
    cm.mesh( "farrel" )
    box = np.array( [ 40.0, 40.0, 12.0 ] )
    check( abs( vsp.GetParmVal( cm.density( "MaxFar" ) ) - 0.05 * np.linalg.norm( box ) ) < 1e-9,
           "a relative far field Max Edge Len is a fraction of the box's diagonal" )
    vsp.SetParmVal( cm.density( "FarAbsRel" ), vsp.ABS )
    for scale, want in ( ( 4.0, 2.0 ), ( 1.5, 1.25 ) ):
        f, w, h = cm.aircraft()
        cm.coarse()
        vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 1.0 )
        vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, 1.0 )
        vsp.SetCFDMeshVal( vsp.CFD_FAR_X_SCALE, scale )
        vsp.SetCFDMeshVal( vsp.CFD_FAR_Z_SCALE, scale )
        vsp.SetParmVal( cm.setting( "SymmetrySplitting", "FarField" ), 1.0 )
        vsp.Update()
        V, T, tag = cm.mesh( "split" )
        # The inner rectangle's corners are mesh points on Y = 0: the bounding box (0..10, -1.5..1.5) scaled.
        cx, cz, lx, lz = 5.0, 0.0, 10.0, 3.0
        corner = np.array( [ cx + 0.5 * want * lx, 0.0, cz + 0.5 * want * lz ] )
        dmin = np.min( np.linalg.norm( V - corner, axis = 1 ) )
        check( dmin < 1e-6, "split rectangle enlarged %.2f times at box scale %.1f" % ( want, scale ) )


def quad_counts():
    f, w, h = cm.aircraft()
    cm.coarse()
    vsp.SetParmVal( cm.setting( "ConvertToQuadsFlag", "Global" ), 1.0 )
    vsp.Update()
    fn = os.path.join( cm.TMP, "q.facet" )
    vsp.SetComputationFileName( vsp.CFD_FACET_TYPE, fn )
    on = os.path.join( cm.TMP, "q2.obj" )
    vsp.SetComputationFileName( vsp.CFD_OBJ_TYPE, on )
    V, T, tag = cm.mesh( "q", types = vsp.CFD_TRI_TYPE | vsp.CFD_FACET_TYPE | vsp.CFD_OBJ_TYPE )
    nq = sum( 1 for l in open( on ) if l.startswith( "f " ) )
    L = open( fn ).read().splitlines()
    nv = int( L[ 4 ].split()[ 0 ] )
    corners = { int( L[ k ].split()[ 1 ] ) for k in range( 5 + nv, len( L ) ) if len( L[ k ].split() ) == 2 }
    check( corners == { 3 }, ".facet splits the quads into triangles" )
    check( result( "Num_Tris" ) * 3 == nq, "Num_Tris counts the triangles before they become quads (%d, %d quads)"
           % ( result( "Num_Tris" ), nq ) )


def mode_and_settings():
    f, w, h = cm.aircraft()
    cm.coarse()
    vsp.SetSetFlag( w, vsp.SET_FIRST_USER, True )
    vsp.Update()
    mid = vsp.CreateAndAddMode( "WingOnly", vsp.SET_FIRST_USER, vsp.SET_NONE )
    vsp.SetCFDMeshModeID( mid )
    V, T, tag = cm.mesh( "mode", set = vsp.SET_ALL )
    off = len( np.unique( tag ) )
    vsp.SetParmVal( cm.setting( "UseMode", "Global" ), 1.0 )
    vsp.Update()
    V, T, tag = cm.mesh( "mode", set = vsp.SET_ALL )
    on = len( np.unique( tag ) )
    check( off == 5 and on == 2, "SetCFDMeshModeID acts only with UseMode on, and then overrides the set passed" )
    vsp.SetParmVal( cm.setting( "UseMode", "Global" ), 0.0 )
    vsp.Update()


def analysis():
    f, w, h = cm.aircraft()
    cm.coarse()
    fn = os.path.join( cm.TMP, "an.tri" )
    # Only the .tri: the other files would be written beside the script.
    sett = vsp.FindContainer( "CFDMeshSettings", 0 )
    for n in [ "STL", "POLY", "TRI", "OBJ", "DAT", "KEY", "GMSH", "TKEY", "FACET", "VSPGEOM", "POGS" ]:
        vsp.SetParmVal( vsp.FindParm( sett, n + "_Export", "ExportCFD" ), 0.0 )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "CfdMeshAnalysis" )
    vsp.SetIntAnalysisInput( "CfdMeshAnalysis", "SelectedSetIndex", [ vsp.SET_ALL ] )
    vsp.SetStringAnalysisInput( "CfdMeshAnalysis", "TRIFileName", [ fn ] )
    vsp.SetIntAnalysisInput( "CfdMeshAnalysis", "TRIFileFlag", [ 1 ] )
    vsp.SetDoubleAnalysisInput( "CfdMeshAnalysis", "BaseLen", [ 0.3 ] )
    rid = vsp.ExecAnalysis( "CfdMeshAnalysis" )
    names = set( vsp.GetAllDataNames( rid ) )
    check( vsp.GetResultsName( rid ) == "CFDMesh" and "Analysis_Duration_Sec" in names and os.path.exists( fn ) and
           abs( vsp.GetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN ) - 0.5 ) < 1e-12,
           "CfdMeshAnalysis returns a CFDMesh Result and puts the settings back" )
    want = { "Num_Tris", "Num_Surfs", "Num_Border_Edges", "Num_Over_Connected_Edges", "Water_Tight", "Num_Topo_Surfs" }
    cm.mesh( "names" )
    got = set( vsp.GetAllDataNames( vsp.FindLatestResultsID( "CFDMesh" ) ) )
    check( got == want, "the CFDMesh Result's data names" )


def parm_names():
    dens = vsp.FindContainer( "CFDGridDensity", 0 )
    sett = vsp.FindContainer( "CFDMeshSettings", 0 )
    w = vsp.AddGeom( "WING" )
    ids = [ vsp.FindParm( dens, n, "CFDGridDensity" ) for n in
            [ "NWidthSeg", "WidthLimit", "BaseAbsRel", "MinAbsRel", "MaxGapAbsRel", "BaseLenFrac", "MinFrac",
              "MaxGapFrac", "FarAbsRel", "FarMaxGapAbsRel", "FarFrac", "FarMaxGapFrac" ] ]
    ids += [ vsp.FindParm( sett, n, g ) for n, g in
             [ ( "ConvertToQuadsFlag", "Global" ), ( "DemoteSurfsCubicFlag", "Global" ),
               ( "CubicSurfTolerance", "Global" ), ( "SymmetrySplitting", "FarField" ), ( "FarComp", "FarField" ),
               ( "UseMode", "Global" ) ] ]
    ids.append( vsp.FindParm( vsp.GetVehicleID(), "MultiSolid", "STLSettings" ) )
    ids += [ vsp.FindParm( w, n, "WakeSettings" ) for n in [ "WakeScale", "WakeAngle" ] ]
    check( all( len( i ) > 0 for i in ids ), "every Parm in the Scripting table is found" )


def watertight():
    f, w, h = cm.aircraft()
    cm.coarse()
    V, T, tag = cm.mesh( "wt" )
    check( result( "Water_Tight" ) == 1 and result( "Num_Tris" ) == len( T ), "CFDMesh Result: Water_Tight, Num_Tris" )


def near_median( V, T, p, r, extra = None ):
    """The median length of the edges whose midpoints lie within r of p."""
    E, L = cm.edges( V, T )
    M = 0.5 * ( V[ E[ :, 0 ] ] + V[ E[ :, 1 ] ] )
    m = np.linalg.norm( M - np.asarray( p ), axis = 1 ) < r
    if extra is not None:
        m = m & extra( M )
    return np.median( L[ m ] )


def rigorous():
    """The puck's face under the ball, rigorous limit off and on."""
    out = []
    for rig in ( 0.0, 1.0 ):
        cm.puck_ball()
        vsp.SetParmVal( cm.density( "RigorLimit" ), rig )
        vsp.Update()
        V, T, tag = cm.mesh( "puck" )
        face = lambda M: np.abs( M[ :, 2 ] - 0.5 * cm.PUCK_W ) < 1e-6
        under = [ 15.0, 0.0, 0.5 * cm.PUCK_W ]
        out.append( ( near_median( V, T, under, 0.5, face ), near_median( V, T, under, 2.0, face ) ) )
    print( "      puck face within 0.5 and 2 of the point under the ball: off %.3f %.3f, on %.3f %.3f"
           % ( out[ 0 ][ 0 ], out[ 0 ][ 1 ], out[ 1 ][ 0 ], out[ 1 ][ 1 ] ) )
    check( abs( out[ 0 ][ 0 ] - 0.94 ) < 0.01 and abs( out[ 1 ][ 0 ] - 0.15 ) < 0.01 and
           abs( out[ 0 ][ 1 ] - 0.93 ) < 0.01 and abs( out[ 1 ][ 1 ] - 0.27 ) < 0.01,
           "rigorous growth limiting refines the puck under the ball (0.94 to 0.15 within 0.5, 0.93 to 0.27 within 2)" )


def background_map():
    """A line source of Rad 0.6 on one wing section 12 long is caught at its middle, not its ends."""
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0.0 )
    for n, v in ( ( "Span", 12.0 ), ( "Root_Chord", 4.0 ), ( "Tip_Chord", 4.0 ), ( "Sweep", 0.0 ) ):
        vsp.SetParmVal( w, n, "XSec_1", v )
    xss = vsp.GetXSecSurf( w, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, i ), "ThickChord" ), 0.04 )
    vsp.Update()
    cm.coarse()
    vsp.DeleteAllCFDSources()
    # U: the root cap, the section and the tip cap take a third each.
    u5 = ( 1.0 + 5.0 / 12.0 ) / 3.0
    u7 = ( 1.0 + 7.0 / 12.0 ) / 3.0
    vsp.AddCFDSource( vsp.LINE_SOURCE, w, 0, 0.04, 0.6, u5, 0.6, 0.04, 0.6, u7, 0.9 )
    V, T, tag = cm.mesh( "map" )
    up = lambda M: M[ :, 2 ] > 0.0
    ends = [ near_median( V, T, [ 0.79, 5.0, 0.07 ], 0.2, up ), near_median( V, T, [ 3.2, 7.0, 0.03 ], 0.2, up ) ]
    mid = near_median( V, T, [ 2.0, 6.0, 0.07 ], 0.2, up )
    print( "      line source: ends %.3f %.3f, middle %.3f" % ( ends[ 0 ], ends[ 1 ], mid ) )
    check( abs( mid - 0.06 ) < 0.01 and all( 0.125 < e < 0.155 for e in ends ),
           "a small line source on a long section is caught at its middle (0.06) but not its ends (0.13-0.15)" )


def scripting_example():
    """The Scripting section's example, run in a folder of its own: only the .tri is written."""
    d = os.path.join( cm.TMP, "example" )
    os.makedirs( d, exist_ok = True )
    here = os.getcwd()
    os.chdir( d )
    vsp.VSPRenew()
    src = open( os.path.join( HERE, "..", "..", "CFDMesh.md" ) ).read()
    code = src.split( "```python\n" )[ 1 ].split( "```" )[ 0 ]
    exec( code, { "__name__": "example" } )
    os.chdir( here )
    files = sorted( os.listdir( d ) )
    rid = vsp.FindLatestResultsID( "CFDMesh" )
    check( files == [ "pod.tri" ] and vsp.GetIntResults( rid, "Water_Tight" )[ 0 ] == 1,
           "the Scripting example writes pod.tri alone, closed (%s)" % files )


def analysis_rel():
    """Under Rel the analysis's BaseLen input changes nothing."""
    n = []
    for base in ( 0.3, 0.6 ):
        vsp.VSPRenew()
        vsp.AddGeom( "POD" )
        vsp.Update()
        vsp.SetParmVal( cm.density( "BaseAbsRel" ), vsp.REL )
        sett = vsp.FindContainer( "CFDMeshSettings", 0 )
        for f in [ "STL", "POLY", "TRI", "OBJ", "DAT", "KEY", "GMSH", "TKEY", "FACET", "VSPGEOM", "POGS" ]:
            vsp.SetParmVal( vsp.FindParm( sett, f + "_Export", "ExportCFD" ), 0.0 )
        vsp.SetAnalysisInputDefaults( "CfdMeshAnalysis" )
        vsp.SetIntAnalysisInput( "CfdMeshAnalysis", "SelectedSetIndex", [ vsp.SET_ALL ] )
        vsp.SetDoubleAnalysisInput( "CfdMeshAnalysis", "BaseLen", [ base ] )
        rid = vsp.ExecAnalysis( "CfdMeshAnalysis" )
        n.append( vsp.GetIntResults( rid, "Num_Tris" )[ 0 ] )
    check( n[ 0 ] == n[ 1 ], "under Rel the analysis's BaseLen input changes nothing (%s)" % n )


def symmetry_split():
    """The split plane: its rectangle's corners in a box 10 times the model."""
    import cfdmesh_figs as F
    V, T, lo, hi = F.sym_mesh( 1.0 )
    ok = True
    for p0, p1, p2, p3 in F.sym_pieces( 1.0, lo, hi ):
        for q in ( p0, p1, p2, p3 ):
            d = np.min( np.hypot( V[ :, 0 ] - q[ 0 ], V[ :, 2 ] - q[ 1 ] ) + np.abs( V[ :, 1 ] ) )
            ok = ok and d < 1e-6
    check( ok, "the split plane's pieces as Figure 19 draws them: every corner is a mesh point" )


def sym_bands( split, default_box ):
    """Median edge lengths of the symmetry plane at 1-2 and 2-4 from the fuselage."""
    import cfdmesh_figs as F
    if default_box:
        cm.aircraft()
        cm.coarse()
        vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, 1.0 )
        vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 1.0 )
        vsp.SetParmVal( cm.setting( "SymmetrySplitting", "FarField" ), split )
        vsp.Update()
        V, T, tag = cm.mesh( "symdef" )
        P = V[ T ]
        T = T[ np.all( np.abs( P[ :, :, 1 ] ) < 1e-9, axis = 1 ) ]
    else:
        V, T, lo, hi = F.sym_mesh( split )
    E, L = cm.edges( V, T )
    M = 0.5 * ( V[ E[ :, 0 ] ] + V[ E[ :, 1 ] ] )
    d = np.hypot( np.maximum( np.maximum( M[ :, 0 ] - 10.0, 0.0 ), np.maximum( -M[ :, 0 ], 0.0 ) ), M[ :, 2 ] )
    return [ np.median( L[ ( d >= a ) & ( d < b ) ] ) for a, b in ( ( 1.0, 2.0 ), ( 2.0, 4.0 ) ) ]


def symmetry_bands():
    import cfdmesh_figs as F
    V, T, lo, hi = F.sym_mesh( 0.0 )
    check( np.allclose( hi - lo, 722.84, atol = 0.01 ), "the box of Figure 19 is 723 square (%s)" % ( hi - lo ) )
    d0, d1 = sym_bands( 0.0, True ), sym_bands( 1.0, True )
    b0, b1 = sym_bands( 0.0, False ), sym_bands( 1.0, False )
    print( "      default box 1-2: %.3f %.3f; 50x box 1-2: %.3f %.3f, 2-4: %.3f %.3f"
           % ( d0[ 0 ], d1[ 0 ], b0[ 0 ], b1[ 0 ], b0[ 1 ], b1[ 1 ] ) )
    check( abs( d0[ 0 ] - SYM_DEF[ 0 ] ) < 0.0015 and abs( d1[ 0 ] - SYM_DEF[ 1 ] ) < 0.0015 and
           abs( b0[ 0 ] - SYM_BIG[ 0 ] ) < 0.0015 and abs( b1[ 0 ] - SYM_BIG[ 1 ] ) < 0.0015 and
           abs( b0[ 1 ] - SYM_BIG[ 2 ] ) < 0.0015 and abs( b1[ 1 ] - SYM_BIG[ 3 ] ) < 0.0015,
           "symmetry plane edge lengths near the fuselage, split off and on, as the page gives them" )


SYM_DEF = ( 0.308, 0.296 )
SYM_BIG = ( 0.343, 0.296, 0.659, 0.554 )


def tessellation():
    """A component's tessellation does not change the CFD mesh."""
    got = []
    for tu, tw in ( ( 0, 0 ), ( 40, 49 ) ):
        f, w, h = cm.aircraft()
        cm.coarse()
        if tu:
            for g in ( f, w, h ):
                vsp.SetParmVal( g, "Tess_W", "Shape", tw )
            vsp.SetParmVal( f, "Tess_U", "Shape", tu )
            vsp.Update()
        V, T, tag = cm.mesh( "tess" )
        got.append( ( len( T ), V.sum() ) )
    check( got[ 0 ][ 0 ] == got[ 1 ][ 0 ] and abs( got[ 0 ][ 1 ] - got[ 1 ][ 1 ] ) < 1e-6,
           "Num_U and Num_W leave the mesh unchanged (%d triangles)" % got[ 0 ][ 0 ] )


def duct():
    """The flow-through nacelle: closed, the disk trimmed to the duct, a disk only in Disk or Both mode."""
    n, d, pr = cm.duct_disk()
    V, T, tag = cm.mesh( "duct" )
    P = V[ np.unique( T[ tag == tag.max() ] ) ]
    r = np.hypot( P[ :, 1 ], P[ :, 2 ] ).max()
    check( result( "Water_Tight" ) == 1 and len( np.unique( tag ) ) == 3 and r < 0.7,
           "a disk across a Negative Volume duct is trimmed to its wall (r %.3f) and the mesh is closed" % r )
    ntag = {}
    for mode in ( vsp.PROP_BOTH, vsp.PROP_BLADES ):
        vsp.SetParmVal( pr, "PropMode", "Design", mode )
        vsp.Update()
        V, T, tag = cm.mesh( "duct" )
        ntag[ mode ] = len( np.unique( tag ) )
    check( ntag[ vsp.PROP_BOTH ] == 3 and ntag[ vsp.PROP_BLADES ] == 2, "a disk in Disk and Both mode, none in Blades" )
    vsp.SetParmVal( pr, "PropMode", "Design", vsp.PROP_DISK )
    vsp.SetParmVal( pr, "X_Rel_Location", "XForm", 2.0 )
    vsp.Update()
    cm.mesh( "duct" )
    check( result( "Water_Tight" ) == 0, "a disk on the Pods' middle section fails to close (as the page warns)" )


def top():
    p = cm.top()
    lo, hi = vsp.GetGeomBBoxMin( p ), vsp.GetGeomBBoxMax( p )
    check( abs( 0.5 * ( hi.y() - lo.y() ) - 0.5 ) < 1e-6 and abs( hi.x() - lo.x() - 4.0 ) < 1e-6,
           "Figure 7's Pod is 4 long and 0.5 in radius" )


def rules_off():
    """Min Edge Len equal to Max Edge Len, or a tiny Num Circle Segments with a large Max Gap, leave Max Edge Len."""
    med = []
    for setup in ( "minmax", "tiny" ):
        vsp.VSPRenew()
        g = vsp.AddGeom( "ELLIPSOID" )
        vsp.Update()
        cm.coarse()
        if setup == "minmax":
            vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.5 )
        else:
            vsp.SetCFDMeshVal( vsp.CFD_NUM_CIRCLE_SEGS, 1e-5 )
            vsp.SetCFDMeshVal( vsp.CFD_MAX_GAP, 10.0 )
        V, T, tag = cm.mesh( "off" )
        E, L = cm.edges( V, T )
        med.append( np.median( L ) )
    print( "      unit sphere, rules off: median edge %.3f and %.3f (Max Edge Len 0.5)" % tuple( med ) )
    check( all( 0.4 < m < 0.6 for m in med ), "turning the curvature rules off leaves Max Edge Len on a unit sphere" )


def rel_defaults():
    vsp.VSPRenew()
    vsp.AddGeom( "POD" )
    vsp.Update()
    cm.mesh( "reldef" )
    mf = vsp.GetParmVal( cm.density( "MinFrac" ) )
    gf = vsp.GetParmVal( cm.density( "MaxGapFrac" ) )
    check( abs( mf - 0.05 ) < 1e-12 and abs( gf - 0.01 ) < 1e-12, "at the defaults Rel Min Edge Len is 5 percent, Max Gap 1 percent" )


def watertight_wake():
    f, w, h = cm.aircraft()
    cm.coarse()
    vsp.SetCFDWakeFlag( w, True )
    vsp.Update()
    cm.mesh( "wtw" )
    check( result( "Water_Tight" ) == 1, "a wake's free edges do not count against the water tight check" )


def split_join_counts():
    """Patches meshed with Split and Join Surfaces off and on: a Fuselage, a blunt and a sharp trailing edge wing."""
    import cfdmesh_figs as F
    out = {}
    for name in ( "fuselage", "blunt", "sharp" ):
        n = []
        for sj in ( 0.0, 1.0 ):
            if name == "fuselage":
                vsp.VSPRenew()
                vsp.AddGeom( "FUSELAGE" )
                vsp.Update()
            elif name == "blunt":
                F.blunt_wing()
            else:
                F.wing_alone()
            cm.coarse()
            vsp.DeleteAllCFDSources()
            vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, sj )
            vsp.Update()
            cm.mesh( "sj" )
            n.append( result( "Num_Surfs" ) )
        out[ name ] = n
    print( "      Split and Join patches off/on:", out )
    check( out == { "fuselage": [ 8, 2 ], "blunt": [ 8, 5 ], "sharp": [ 6, 4 ] },
           "Split and Join: Fuselage 8 to 2, blunt wing 8 to 5, sharp wing 6 to 4 patches" )


def sphere_centered():
    """The Ellipsoid far field of Figure 25 is centered on the aircraft."""
    import meshshots as M
    cm.aircraft()
    e, c = cm.centered_sphere( M.FAR_SPHERE_R )
    a, b = vsp.GetGeomBBoxMin( e ), vsp.GetGeomBBoxMax( e )
    ec = 0.5 * ( np.array( list( a ) ) + np.array( list( b ) ) )
    print( "      sphere center", ec, "aircraft center", c )
    check( np.allclose( ec, c, atol = 1e-6 ) and abs( c[ 1 ] ) < 1e-9, "the Ellipsoid far field is centered on the aircraft" )

CHECKS = [ defaults, curvature, width, point_source, wing_sources, domain, half, repeat_shown, quads_threads_splitjoin,
           subsurfaces, degen, relative_from_api, growth, default_sources_stale, far_abs_sliders,
           quad_counts, mode_and_settings, analysis, parm_names, watertight, rigorous, background_map,
           scripting_example, analysis_rel, symmetry_split, symmetry_bands, tessellation, duct, top, rules_off,
           rel_defaults, watertight_wake, split_join_counts, sphere_centered ]

if __name__ == "__main__":
    for c in CHECKS:
        c()
    print( "%d failed" % len( FAILS ) )
    sys.exit( 1 if FAILS else 0 )
