"""Checks the numbers and statements of POGS.md against VSP.

    cd src/help/figure_src/pogs
    <build>/venv/bin/python verify.py

Every run writes into a temporary folder; nothing is left in the source tree.
"""

import os
import re
import sys
import struct
import tempfile

import numpy as np

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "cfdmesh" ) )
import openvsp as vsp
import cfdmodel as cm

TMP = tempfile.mkdtemp()
NFAIL = [ 0 ]
NCHECK = [ 0 ]

A = "CfdMeshAnalysis"
OTHERS = [ "STL", "POLY", "TRI", "FACET", "OBJ", "GMSH", "VSPGEOM" ]


def check( ok, what ):
    NCHECK[ 0 ] += 1
    if ok:
        print( "ok    " + what )
    else:
        NFAIL[ 0 ] += 1
        print( "FAIL  " + what )


def settings():
    return vsp.FindContainer( "CFDMeshSettings", 0 )


def pogs_run( name, split = True, nref = None, tol = None ):
    """Mesh the current model through the Analysis Manager with the POGS files the only output.  Returns the folder
    and the CFDMesh Result."""
    d = os.path.join( TMP, name )
    os.makedirs( d, exist_ok = True )
    vsp.SetAnalysisInputDefaults( A )
    for f in OTHERS:
        vsp.SetIntAnalysisInput( A, f + "FileFlag", [ 0 ] )
    vsp.SetIntAnalysisInput( A, "POGSFileFlag", [ 1 ] )
    vsp.SetStringAnalysisInput( A, "POGSFileName", [ os.path.join( d, "plane.i.tri" ) ] )
    vsp.SetIntAnalysisInput( A, "SplitJoinSurfs", [ int( split ) ] )
    if nref is not None:
        vsp.SetIntAnalysisInput( A, "POGSNRef", [ nref ] )
    if tol is not None:
        vsp.SetDoubleAnalysisInput( A, "RelCurveTol", [ tol ] )
    s = settings()
    for f in ( "DAT", "KEY", "TKEY" ):
        vsp.SetParmVal( vsp.FindParm( s, f + "_Export", "ExportCFD" ), 0.0 )
    rid = vsp.ExecAnalysis( A )
    return d, rid


def topo( d ):
    """ISYM, NEDGE, NFACE from the first line of the .topo file."""
    t = open( os.path.join( d, "plane.topo" ) ).readline().split()
    return int( t[ 0 ] ), int( t[ 1 ] ), int( t[ 2 ] )


def gcomp( d ):
    """The components of the .gcomp file: [ ( name, [ faces ] ) ]."""
    out = []
    for blk in open( os.path.join( d, "plane.gcomp" ) ).read().split( "\n\n" ):
        lines = [ l for l in blk.split( "\n" ) if l.strip() ]
        if lines:
            out.append( ( lines[ 0 ], [ int( v ) for v in lines[ 1 ].split( "," ) ] ) )
    return out


def pogsi( d ):
    """The numbers of the .pogs.i file's three parameter lines, by name."""
    vals = {}
    for line in open( os.path.join( d, "plane.pogs.i" ) ).read().split( "\n" )[ 2:5 ]:
        nums, names = line.rsplit( "   ", 1 )
        for n, v in zip( names.split( "," ), nums.split() ):
            vals[ n.strip() ] = float( v )
    return vals


def plot3d_dims( fn ):
    """The ( NI, NJ ) of every block of an unformatted Plot3D file."""
    b = open( fn, "rb" ).read()
    p = [ 0 ]

    def rec():
        n = struct.unpack_from( "<i", b, p[ 0 ] )[ 0 ]
        r = b[ p[ 0 ] + 4:p[ 0 ] + 4 + n ]
        p[ 0 ] += n + 8
        return r

    nb = struct.unpack( "<i", rec() )[ 0 ]
    d = struct.unpack( "<%di" % ( 3 * nb ), rec() )
    return [ ( d[ 3 * i ], d[ 3 * i + 1 ] ) for i in range( nb ) ]


def plot3d_bounds( fn ):
    """The ( xmin, xmax, ymin, ymax, zmin, zmax ) of every block of an unformatted Plot3D grid file of doubles."""
    import numpy as np
    b = open( fn, "rb" ).read()
    p = [ 0 ]

    def rec():
        n = struct.unpack_from( "<i", b, p[ 0 ] )[ 0 ]
        r = b[ p[ 0 ] + 4:p[ 0 ] + 4 + n ]
        p[ 0 ] += n + 8
        return r

    nb = struct.unpack( "<i", rec() )[ 0 ]
    d = struct.unpack( "<%di" % ( 3 * nb ), rec() )
    out = []
    for i in range( nb ):
        n = d[ 3 * i ] * d[ 3 * i + 1 ]
        a = np.frombuffer( rec()[ :24 * n ], dtype = "<f8" ).reshape( 3, n )
        out.append( tuple( round( float( v ), 6 ) + 0.0 for k in range( 3 ) for v in ( a[ k ].min(), a[ k ].max() ) ) )
    return out


# ---------------------------------------------------------------- defaults

cm.aircraft()
s = settings()
check( vsp.GetParmVal( vsp.FindParm( s, "POGS_Export", "ExportCFD" ) ) == 0.0, "the POGS files are off by default" )
others_on = all( vsp.GetParmVal( vsp.FindParm( s, f + "_Export", "ExportCFD" ) ) == 1.0
                  for f in OTHERS + [ "DAT", "KEY", "TKEY" ] )
check( others_on, "every other file is on by default" )
check( vsp.GetParmVal( vsp.FindParm( s, "SplitJoinSurfsFlag", "Global" ) ) == 0.0,
       "Split and Join Surfaces is off by default" )
vsp.SetAnalysisInputDefaults( A )
check( vsp.GetIntAnalysisInput( A, "POGSNRef" )[ 0 ] == 0, "Num Ref. is 0 by default" )
check( abs( vsp.GetDoubleAnalysisInput( A, "RelCurveTol" )[ 0 ] - 0.005 ) < 1e-12,
       "Curve Relative Tol. is 0.005 by default" )
check( vsp.GetStringAnalysisInput( A, "POGSFileName" )[ 0 ].endswith( ".i.tri" ), "the POGS file is named .i.tri" )

# ---------------------------------------------------------------- the files, and Split and Join

cm.aircraft()
d_off, r_off = pogs_run( "off", split = False )
cm.aircraft()
d_on, r_on = pogs_run( "on", split = True )

files = sorted( os.listdir( d_on ) )
check( files == sorted( [ "plane.i.tri", "plane.uvin", "plane.cur", "plane.topo", "plane.gcomp", "plane.pogs.i" ] ),
       "the POGS output is .i.tri, .uvin, .cur, .topo, .gcomp and .pogs.i, and nothing else: %s" % files )
check( vsp.GetIntResults( r_on, "Water_Tight" )[ 0 ] == 1, "the aircraft's POGS run is water tight" )
tris_on = vsp.GetIntResults( r_on, "Num_Tris" )[ 0 ]

isym, nedge_off, nface_off = topo( d_off )
isym, nedge_on, nface_on = topo( d_on )
check( ( nface_off, nface_on ) == ( 24, 14 ), "the aircraft has 24 faces with Split and Join off, 14 on: %d, %d"
       % ( nface_off, nface_on ) )
check( ( nedge_off, nedge_on ) == ( 46, 32 ), "and 46 edges off, 32 on: %d, %d" % ( nedge_off, nedge_on ) )
check( len( plot3d_dims( os.path.join( d_on, "plane.uvin" ) ) ) == nface_on, "the .uvin holds one surface per face" )
fus_off = [ f for n, f in gcomp( d_off ) if n.startswith( "FuselageGeom_0_" ) ][ 0 ]
fus_on = [ f for n, f in gcomp( d_on ) if n.startswith( "FuselageGeom_0_" ) ][ 0 ]
check( ( len( fus_off ), len( fus_on ) ) == ( 8, 2 ), "the Fuselage is 8 faces off, 2 on (two halves): %d, %d"
       % ( len( fus_off ), len( fus_on ) ) )
bo = plot3d_bounds( os.path.join( d_off, "plane.uvin" ) )
quarters = sorted( set( ( b[ 0 ], b[ 1 ], b[ 2 ] < -0.1, b[ 3 ] > 0.1, b[ 4 ] < -0.1, b[ 5 ] > 0.1 )
                        for b in [ bo[ k - 1 ] for k in fus_off ] ) )
check( len( quarters ) == 8 and sorted( set( ( q[ 0 ], q[ 1 ] ) for q in quarters ) ) == [ ( 0.0, 5.0 ), ( 5.0, 10.0 ) ]
       and all( q[ 2 ] != q[ 3 ] and q[ 4 ] != q[ 5 ] for q in quarters ),
       "off, the Fuselage is four quarters, split again at its middle cross section (X 5)" )
bn = plot3d_bounds( os.path.join( d_on, "plane.uvin" ) )
halves = [ bn[ k - 1 ] for k in fus_on ]
check( all( h[ 0 ] == 0.0 and h[ 1 ] == 10.0 and ( h[ 2 ] < -0.1 ) != ( h[ 3 ] > 0.1 ) and h[ 4 ] < -0.1
            and h[ 5 ] > 0.1 for h in halves ),
       "on, it is two halves, left and right, each running nose to tail and top to bottom" )
names = [ n for n, f in gcomp( d_on ) ]
check( [ re.sub( "_[A-Z]{10}$", "", n ) for n in names ] ==
       [ "FuselageGeom_0", "WingGeom_0", "WingGeom_1", "HTail_0", "HTail_1" ],
       "the .gcomp has a component for each surface of each Geom, a symmetric copy its own: %s" % names )
check( all( re.search( "_[A-Z]{10}$", n ) for n in names ), "each component's name ends with its Geom's ID" )

# ---------------------------------------------------------------- Figures 3 and 4: a blunt trailing edge and a tip cap

import pogsmodel as pm


def wing_faces( d ):
    """The faces of the right Wing half: ( all, trailing edge faces, tip cap faces ), from their .uvin bounds."""
    bounds = plot3d_bounds( os.path.join( d, "plane.uvin" ) )
    faces = [ f for n, f in gcomp( d ) if n.startswith( "WingGeom_0_" ) ][ 0 ]
    te = [ k for k in faces if bounds[ k - 1 ][ 0 ] > 5.3 ]
    tip = [ k for k in faces if bounds[ k - 1 ][ 2 ] > 4.99 ]
    return faces, te, tip


pm.blunt_aircraft()
d_boff, r = pogs_run( "blunt_off", split = False )
pm.blunt_aircraft()
d_bon, r = pogs_run( "blunt_on", split = True )
off = wing_faces( d_boff )
on = wing_faces( d_bon )
check( ( len( off[ 0 ] ), len( on[ 0 ] ) ) == ( 6, 4 ),
       "with a blunt trailing edge each wing half goes from 6 faces to 4: %d, %d"
       % ( len( off[ 0 ] ), len( on[ 0 ] ) ) )
check( ( len( off[ 1 ] ), len( on[ 1 ] ) ) == ( 2, 1 ), "its blunt trailing edge goes from 2 faces to 1: %d, %d"
       % ( len( off[ 1 ] ), len( on[ 1 ] ) ) )
check( ( len( off[ 2 ] ), len( on[ 2 ] ) ) == ( 2, 1 ), "its tip cap goes from 2 faces to 1: %d, %d"
       % ( len( off[ 2 ] ), len( on[ 2 ] ) ) )
fb = plot3d_bounds( os.path.join( d_boff, "plane.uvin" ) )
te_split = sorted( round( fb[ k - 1 ][ 5 ], 3 ) for k in off[ 1 ] )
check( abs( te_split[ 0 ] - ( -0.25 ) ) < 1e-3, "split along the middle of its thickness: tops at %s" % te_split )

# ---------------------------------------------------------------- the .pogs.i file

v = pogsi( d_on )
check( abs( v[ "DSM" ] - 0.5 ) < 1e-9, "DSM is Max Edge Len (0.5): %g" % v[ "DSM" ] )
check( abs( v[ "SRMAX" ] - 1.3 ) < 1e-9, "SRMAX is the Growth Ratio (1.3): %g" % v[ "SRMAX" ] )
check( abs( v[ "MAXA" ] - 22.5 ) < 1e-9, "MAXA is 360 / Num Circle Segments (22.5): %g" % v[ "MAXA" ] )
check( v[ "ISYM" ] == 0.0, "ISYM is 0 for a whole model" )
check( abs( v[ "DSWALL" ] - 1.0e-4 ) < 1e-12 and v[ "DOBND" ] == -10.0 and v[ "NFRINGE" ] == 2.0,
       "DSWALL 1e-4, DOBND -10 and NFRINGE 2 are defaults: %g %g %g" % ( v[ "DSWALL" ], v[ "DOBND" ],
                                                                         v[ "NFRINGE" ] ) )
first = open( os.path.join( d_on, "plane.pogs.i" ) ).readline().strip()
check( first == "plane", "the .pogs.i names the case by the file name, without a path: %s" % first )

cm.aircraft()
cm.coarse( maxlen = 0.4, ncirc = 24.0, growth = 1.2 )
d, r = pogs_run( "settings" )
v = pogsi( d )
check( abs( v[ "DSM" ] - 0.4 ) < 1e-9 and abs( v[ "SRMAX" ] - 1.2 ) < 1e-9 and abs( v[ "MAXA" ] - 15.0 ) < 1e-9,
       "and they follow the settings: DSM %g, SRMAX %g, MAXA %g" % ( v[ "DSM" ], v[ "SRMAX" ], v[ "MAXA" ] ) )

# ---------------------------------------------------------------- half model, far field, wakes

cm.aircraft()
vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, 1.0 )
d, r = pogs_run( "half" )
isym, ne, nf = topo( d )
check( isym == 2 and pogsi( d )[ "ISYM" ] == 2.0, "a half mesh gives ISYM 2 (the +Y half) in .topo and .pogs.i" )
check( not [ n for n, f in gcomp( d ) if n == "SymmetryPlane" ], "the symmetry plane is not a face of a half mesh" )
check( vsp.GetIntResults( r, "Water_Tight" )[ 0 ] == 1, "the half mesh is reported water tight" )

cm.aircraft()
vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 1.0 )
d, r = pogs_run( "far" )
check( "FarField" in [ n for n, f in gcomp( d ) ], "a far field box is a component of its own, FarField" )

cm.aircraft()
vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, 1.0 )
vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 1.0 )
d, r = pogs_run( "halffar" )
nm = [ n for n, f in gcomp( d ) ]
check( "FarField" in nm and "SymmetryPlane" in nm, "with a far field, the symmetry plane is a face: %s" % nm[ -2: ] )

f, w, h = cm.aircraft()
vsp.SetCFDWakeFlag( w, True )
vsp.Update()
d, r = pogs_run( "wake" )
isym, ne, nf = topo( d )
check( nf == nface_on, "a wake adds no face: %d faces" % nf )

# ---------------------------------------------------------------- the .uvin sampling

base = plot3d_dims( os.path.join( d_on, "plane.uvin" ) )
cm.aircraft()
d, r = pogs_run( "nref1", nref = 1 )
ref1 = plot3d_dims( os.path.join( d, "plane.uvin" ) )
check( all( ( 2 * ( a[ 0 ] - 1 ) + 1, 2 * ( a[ 1 ] - 1 ) + 1 ) == b for a, b in zip( base, ref1 ) ),
       "Num Ref. 1 halves every interval of the .uvin surfaces: %s -> %s" % ( base[ 0 ], ref1[ 0 ] ) )

f, w, h = cm.aircraft()
vsp.SetParmVal( f, "Tess_W", "Shape", 33 )
vsp.Update()
d, r = pogs_run( "tessw" )
tw = plot3d_dims( os.path.join( d, "plane.uvin" ) )
check( tw[ 0 ][ 1 ] > base[ 0 ][ 1 ] and tw[ 2: ] == base[ 2: ],
       "the Fuselage's own Tess_W sets its .uvin samples around it: %s -> %s" % ( base[ 0 ], tw[ 0 ] ) )
check( vsp.GetIntResults( r, "Num_Tris" )[ 0 ] == tris_on,
       "and leaves the triangulation as it is: %d, %d triangles"
       % ( tris_on, vsp.GetIntResults( r, "Num_Tris" )[ 0 ] ) )


f, w, h = cm.aircraft()
xss = vsp.GetXSecSurf( f, 0 )
for i in range( vsp.GetNumXSec( xss ) ):
    pid = vsp.GetXSecParm( vsp.GetXSec( xss, i ), "SectTess_U" )
    if pid:
        vsp.SetParmVal( pid, 12 )
vsp.Update()
d, r = pogs_run( "secttess" )
st = plot3d_dims( os.path.join( d, "plane.uvin" ) )
check( st[ 0 ][ 0 ] > base[ 0 ][ 0 ] and st[ 0 ][ 1 ] == base[ 0 ][ 1 ],
       "its sections' tessellation sets its .uvin samples along it: %s -> %s" % ( base[ 0 ], st[ 0 ] ) )

f, w, h = cm.aircraft()
vsp.AddCFDSource( vsp.POINT_SOURCE, f, 0, 0.02, 1.0, 0.5, 0.5 )
vsp.AddCFDSource( vsp.POINT_SOURCE, w, 0, 0.02, 0.5, 0.0, 0.5 )
vsp.Update()
d, r = pogs_run( "sources" )
check( topo( d ) == topo( d_on ) and plot3d_dims( os.path.join( d, "plane.uvin" ) ) == base,
       "sources change neither the faces, the edges nor the .uvin samples" )
check( vsp.GetIntResults( r, "Num_Tris" )[ 0 ] != tris_on,
       "but they do change the triangulation: %d -> %d triangles"
       % ( tris_on, vsp.GetIntResults( r, "Num_Tris" )[ 0 ] ) )


def cur_points( d ):
    return sum( n for n, one in plot3d_dims( os.path.join( d, "plane.cur" ) ) )


cm.aircraft()
d, r = pogs_run( "tolfine", tol = 0.0005 )
fine = cur_points( d )
coarse_pts = cur_points( d_on )
check( fine > coarse_pts, "a smaller Curve Relative Tol. puts more points on the .cur curves: %d -> %d"
       % ( coarse_pts, fine ) )
check( topo( d )[ 2 ] == nface_on, "and leaves the faces as they are" )

# ---------------------------------------------------------------- the starting settings of Getting Started


def rel_start():
    """Max Edge Len, Min Edge Len and Max Gap relative at 0.01, Num Circle Segments 90."""
    for absrel, frac in ( ( "BaseAbsRel", "BaseLenFrac" ), ( "MinAbsRel", "MinFrac" ),
                          ( "MaxGapAbsRel", "MaxGapFrac" ) ):
        vsp.SetParmVal( cm.density( absrel ), vsp.REL )
        vsp.SetParmVal( cm.density( frac ), 0.01 )
    vsp.SetCFDMeshVal( vsp.CFD_NUM_CIRCLE_SEGS, 90.0 )
    # The runs below are made with ComputeCFDMesh, which also writes the files these flags turn on, named after the
    # model, in the current folder; they are turned off so that nothing is left here.
    s = settings()
    for f in ( "DAT", "KEY", "TKEY" ):
        vsp.SetParmVal( vsp.FindParm( s, f + "_Export", "ExportCFD" ), 0.0 )
    vsp.Update()


def lens():
    return [ vsp.GetParmVal( cm.density( n ) ) for n in ( "BaseLen", "MinLen", "MaxGap" ) ]


cm.aircraft()
rel_start()
lo, hi = cm.aircraft_bbox()
diag = float( ( ( hi - lo ) ** 2 ).sum() ** 0.5 )
# The lengths are worked out from the fractions when a run starts; the Analysis Manager puts the settings back after
# its run, so they are read after a run made through ComputeCFDMesh.
vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, os.path.join( TMP, "rel.tri" ) )
vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )
base, mn, gap = lens()
cm.aircraft()
rel_start()
d, r = pogs_run( "rel90" )
check( abs( base - 0.01 * diag ) < 1e-9 * diag,
       "Rel 0.01 makes Max Edge Len 1%% of the bounding box diagonal: %g of %g" % ( base, diag ) )
check( abs( mn - 0.01 * base ) < 1e-12 and abs( gap - 0.01 * base ) < 1e-12,
       "and Min Edge Len and Max Gap 1%% of Max Edge Len: %g, %g" % ( mn, gap ) )
check( abs( pogsi( d )[ "MAXA" ] - 4.0 ) < 1e-9, "90 Num Circle Segments write MAXA 4: %g" % pogsi( d )[ "MAXA" ] )
check( abs( pogsi( d )[ "DSM" ] - base ) < 1e-5 * base,
       "and DSM is the relative Max Edge Len: %g" % pogsi( d )[ "DSM" ] )
check( vsp.GetIntResults( r, "Water_Tight" )[ 0 ] == 1, "the starting settings mesh the aircraft water tight" )
check( all( vsp.GetParmVal( cm.density( n ) ) == vsp.ABS for n in ( "FarAbsRel", "FarMaxGapAbsRel" ) ) and
       vsp.FindParm( vsp.FindContainer( "CFDGridDensity", 0 ), "NCircSeg", "CFDGridDensity" ) != "",
       "the Global tab's other controls have no Rel (the far field's Abs and Rel are on the Domain tab)" )


def shown_trap( setidx ):
    """Mesh twice with the starting settings and the Normal Set setidx, through ComputeCFDMesh's set argument as the
    GUI's run does.  Returns ( Max Edge Len each run used, whether it wrote a mesh ) for the two runs, and how many
    components are shown at the end."""
    cm.aircraft()
    rel_start()
    out = []
    for k in range( 2 ):
        fn = os.path.join( TMP, "trap_%d_%d.tri" % ( setidx, k ) )
        vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, fn )
        vsp.ComputeCFDMesh( setidx, vsp.SET_NONE, vsp.CFD_TRI_TYPE )
        vsp.Update()
        out.append( ( lens()[ 0 ], os.path.exists( fn ) ) )
    shown = [ g for g in vsp.FindGeoms() if vsp.GetSetFlag( g, vsp.SET_SHOWN ) ]
    return out, len( shown )


out, nshown = shown_trap( vsp.SET_SHOWN )
check( nshown == 0, "a run leaves every component hidden, so Shown is empty" )
check( abs( out[ 1 ][ 0 ] / 0.01 - 3.4641e12 ) < 1e9,
       "with Shown and Rel, the next run takes Max Edge Len from a box of diagonal 3.5e12: %g" % out[ 1 ][ 0 ] )
check( out[ 0 ][ 1 ] and not out[ 1 ][ 1 ], "and the next run meshes nothing" )
out, nshown = shown_trap( vsp.SET_ALL )
check( abs( out[ 0 ][ 0 ] - out[ 1 ][ 0 ] ) < 1e-12 and out[ 1 ][ 1 ], "with the Set All, nothing changes: %g, %g"
       % ( out[ 0 ][ 0 ], out[ 1 ][ 0 ] ) )

# ---------------------------------------------------------------- Num Ref. and Figure 5

import pogs_figs as pf


def nref_run( nref ):
    """The +Y Fuselage half's .uvin block at Num Ref. nref, the triangle count and the Fuselage's Tess_W after."""
    f, w, h = pm.blunt_aircraft()
    d, r = pogs_run( "nref%d_b" % nref, nref = nref )
    blk = pf.fuselage_half( pf.read_uvin( os.path.join( d, "plane.uvin" ) ) )
    return blk, vsp.GetIntResults( r, "Num_Tris" )[ 0 ], vsp.GetParmVal( f, "Tess_W", "Shape" )


runs = [ nref_run( n ) for n in ( 0, 1, 2 ) ]
dims = [ ( b[ 0 ].shape[ 1 ], b[ 0 ].shape[ 0 ] ) for b, nt, tw in runs ]
check( dims == [ ( 21, 9 ), ( 41, 17 ), ( 81, 33 ) ],
       "one Fuselage half's .uvin grid is 21 x 9, 41 x 17 and 81 x 33 at Num Ref. 0, 1 and 2: %s" % dims )
check( len( set( nt for b, nt, tw in runs ) ) == 1 and len( set( tw for b, nt, tw in runs ) ) == 1,
       "Num Ref. leaves the triangle mesh and the component's tessellation as they are: %s"
       % sorted( set( ( nt, tw ) for b, nt, tw in runs ) ) )
trimmed = [ int( ( b[ 3 ] == 0 ).sum() ) for b, nt, tw in runs ]
check( trimmed[ 0 ] == 0 and trimmed[ 2 ] == 17, "points trimmed away on that half: %s at Num Ref. 0, 1, 2" % trimmed )
mid = runs[ 2 ][ 0 ]
xs = mid[ 0 ][ mid[ 3 ] == 0 ]
check( xs.min() > 3.4 and ( ( xs < 5.6 ) | ( xs > 8.5 ) ).all(),
       "they lie where the wing (X 3.5 to 5.5) and the tail (from X 8.6) meet the Fuselage: X %.2f to %.2f"
       % ( xs.min(), xs.max() ) )
check( mid[ 1 ].mean() > 0.0, "the half drawn is the right, +Y, half" )
cm.aircraft()
nrp = vsp.FindParm( vsp.FindContainer( "CFDMeshSettings", 0 ), "POGSNRef", "Global" )
check( vsp.GetParmLowerLimit( nrp ) == 0.0 and vsp.GetParmUpperLimit( nrp ) == 6.0, "Num Ref. goes from 0 to 6" )

# ---------------------------------------------------------------- blanking, Figure 6

fus, wing = pf.blanking_faces()
fx, fy, fz, fib = fus
fb = fib == 0
check( pf.BLANK_NREF == 2 and fx.size == 2673 and int( fb.sum() ) == 17,
       "at Num Ref. 2 the Fuselage half has 17 of its 2673 points blanked: %d of %d" % ( fb.sum(), fx.size ) )
rows = sorted( set( int( j ) for j in np.nonzero( fb & ( fx < 6.0 ) )[ 0 ] ) )
xs = fx[ fb & ( fx < 6.0 ) ]
check( len( rows ) == 1 and abs( xs.min() - 3.88 ) < 0.005 and abs( xs.max() - 5.12 ) < 0.005,
       "those inside the Wing's root are one row of points, from X %.2f to %.2f" % ( xs.min(), xs.max() ) )
check( int( ( fb & ( fx < 6.0 ) ).sum() ) == 11 and int( ( fb & ( fx > 8.0 ) ).sum() ) == 6,
       "11 inside the Wing's root and 6 inside the tail: %d, %d" % ( ( fb & ( fx < 6.0 ) ).sum(),
                                                                  ( fb & ( fx > 8.0 ) ).sum() ) )
check( len( wing ) == 2, "the figure draws the Wing's two skins: %d faces" % len( wing ) )
for k, ( wx, wy, wz, wib ) in enumerate( wing ):
    wb = wib == 0
    stations = sorted( set( float( v ) for v in np.round( wy[ wb ], 6 ) ) )
    check( wx.size == 1365 and int( wb.sum() ) == 325 and stations == [ 0.0, 0.25, 0.5, 0.75, 1.0 ],
           "Wing skin %d: 325 of 1365 points blanked, the stations at Y %s" % ( k, stations ) )
    check( ( np.abs( wy[ wb ] ) < 1.25 ).all(), "  all of them within the Fuselage's half width, 1.25" )

# ---------------------------------------------------------------- the page's scripting example

page = open( os.path.join( HERE, "..", "..", "POGS.md" ) ).read()
code = re.search( r"```python\n(.*?)```", page, re.S ).group( 1 )
cwd = os.getcwd()
os.chdir( TMP )
vsp.VSPRenew()
try:
    ns = {}
    exec( code, ns )
    check( sorted( f for f in os.listdir( TMP ) if f.startswith( "plane." ) ) ==
           sorted( [ "plane.i.tri", "plane.uvin", "plane.cur", "plane.topo", "plane.gcomp", "plane.pogs.i" ] ),
           "the page's example writes the POGS files alone" )
    check( ns[ "nface" ] == 8,
           "and its fuselage and wing make 8 faces, 2 and 3 for each wing half: %d" % ns[ "nface" ] )
finally:
    os.chdir( cwd )

print( "%d checks, %d failed" % ( NCHECK[ 0 ], NFAIL[ 0 ] ) )
sys.exit( 1 if NFAIL[ 0 ] else 0 )
