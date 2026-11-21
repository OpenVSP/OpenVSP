"""The charts of POGS.md, drawn from the .uvin file: one face at two Num Ref. settings, and the blanking of a
wing and a body where they meet.

    <build>/venv/bin/python pogs_figs.py
"""

import os
import sys
import struct
import tempfile

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )
sys.path.insert( 0, HERE )

import numpy as np
import matplotlib
matplotlib.use( "Agg" )
import matplotlib.pyplot as plt
import figlib as fl
import openvsp as vsp
import pogsmodel as pm

OUT = os.path.join( HERE, "..", "..", "images" )
NREF = ( 0, 2 )


def read_uvin( fn ):
    """The blocks of an unformatted Plot3D grid file with iblanks: [ ( x, y, z, iblank ) ], each ( NJ, NI )."""
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
        ni, nj = d[ 3 * i ], d[ 3 * i + 1 ]
        n = ni * nj
        r = rec()
        xyz = np.frombuffer( r[ :24 * n ], dtype = "<f8" ).reshape( 3, nj, ni )
        ib = np.frombuffer( r[ 24 * n:24 * n + 4 * n ], dtype = "<i4" ).reshape( nj, ni )
        out.append( ( xyz[ 0 ], xyz[ 1 ], xyz[ 2 ], ib ) )
    return out


def uvin( nref, comps = None ):
    """The .uvin blocks of the model at Num Ref. nref, with Split and Join Surfaces on.  With comps, a list, the
    components of the .gcomp file are put in it as ( name, [ faces ] )."""
    pm.blunt_aircraft()
    d = tempfile.mkdtemp()
    a = "CfdMeshAnalysis"
    vsp.SetAnalysisInputDefaults( a )
    for f in ( "STL", "POLY", "TRI", "FACET", "OBJ", "GMSH", "VSPGEOM" ):
        vsp.SetIntAnalysisInput( a, f + "FileFlag", [ 0 ] )
    vsp.SetIntAnalysisInput( a, "POGSFileFlag", [ 1 ] )
    vsp.SetIntAnalysisInput( a, "SplitJoinSurfs", [ 1 ] )
    vsp.SetIntAnalysisInput( a, "POGSNRef", [ nref ] )
    vsp.SetStringAnalysisInput( a, "POGSFileName", [ os.path.join( d, "plane.i.tri" ) ] )
    s = vsp.FindContainer( "CFDMeshSettings", 0 )
    for f in ( "DAT", "KEY", "TKEY" ):
        vsp.SetParmVal( vsp.FindParm( s, f + "_Export", "ExportCFD" ), 0.0 )
    vsp.ExecAnalysis( a )
    if comps is not None:
        for blk in open( os.path.join( d, "plane.gcomp" ) ).read().split( "\n\n" ):
            lines = [ l for l in blk.split( "\n" ) if l.strip() ]
            if lines:
                comps.append( ( lines[ 0 ], [ int( v ) for v in lines[ 1 ].split( "," ) ] ) )
    return read_uvin( os.path.join( d, "plane.uvin" ) )


def fuselage_half( blocks ):
    """The +Y half of the Fuselage: the block of the first two that lies on the +Y side."""
    for k in ( 0, 1 ):
        if blocks[ k ][ 1 ].mean() > 0.0:
            return blocks[ k ]
    return None


def draw( ax, blk, label ):
    """The face seen from its side, along -Y: its grid lines."""
    x, y, z, ib = blk
    for j in range( x.shape[ 0 ] ):
        ax.plot( x[ j ], z[ j ], color = "0.45", lw = 0.5 )
    for i in range( x.shape[ 1 ] ):
        ax.plot( x[ :, i ], z[ :, i ], color = "0.45", lw = 0.5 )
    ax.set_aspect( "equal" )
    ax.set_axis_off()
    ax.text( 0.0, 1.0, "%s: %d x %d points" % ( label, x.shape[ 1 ], x.shape[ 0 ] ), transform = ax.transAxes,
             ha = "left", va = "bottom" )


def fig_nref():
    fl.style()
    blks = [ fuselage_half( uvin( n ) ) for n in NREF ]
    fig, axs = plt.subplots( 2, 1, figsize = ( fl.PAGE_W, 4.4 ) )
    for ax, blk, n in zip( axs, blks, NREF ):
        draw( ax, blk, "Num Ref. %d" % n )
    fig.subplots_adjust( left = 0.02, right = 0.98, top = 0.95, bottom = 0.02, hspace = 0.15 )
    fn = os.path.join( OUT, "POGS_num_ref.png" )
    fig.savefig( fn, dpi = 200, metadata = { "Software": None } )
    plt.close( fig )
    print( "made", fn )


BLANK_NREF = 2
# The view of the wing root: from outboard, a little behind and above (the viewer toward VIEW_FROM from the model),
# and the part of the model shown, in X and Y.
VIEW_FROM = ( 0.35, 0.75, 0.55 )
BOX_X = ( 2.8, 6.4 )
BOX_Y = ( -0.1, 2.3 )
FUS_COLOR = "0.35"        # the Fuselage's grid edges
FUS_FILL = "0.80"         # and its surface
WING_COLOR = "#0072B2"    # the Wing's grid edges
WING_FILL = "#9CC9E8"     # and its surface
FILL_ALPHA = 0.35


def blanking_faces():
    """The .uvin blocks the blanking figure draws, at Num Ref. BLANK_NREF: the +Y half of the Fuselage and the upper
    and lower faces of the right ( +Y ) Wing.  Returns ( fuselage block, [ wing blocks ] )."""
    comps = []
    blocks = uvin( BLANK_NREF, comps )
    fus = fuselage_half( blocks )
    wing_faces = [ f for n, f in comps if n.startswith( "WingGeom_0_" ) ][ 0 ]
    # The two skins: the faces that run the wing's whole chord (the trailing edge face and the tip cap do not).
    wing = [ blocks[ k - 1 ] for k in wing_faces
             if blocks[ k - 1 ][ 0 ].max() - blocks[ k - 1 ][ 0 ].min() > 1.5 and blocks[ k - 1 ][ 1 ].max() > 4.9 ]
    return fus, wing


def project( x, y, z ):
    """Screen coordinates of model points seen from VIEW_FROM, Z up, orthographic."""
    d = np.array( VIEW_FROM ) / np.linalg.norm( VIEW_FROM )
    right = np.cross( np.array( [ 0.0, 0.0, 1.0 ] ), d )
    right /= np.linalg.norm( right )
    up = np.cross( d, right )
    p = np.stack( [ x, y, z ], axis = -1 )
    return p @ right, p @ up


def in_box( x, y ):
    return ( x >= BOX_X[ 0 ] ) & ( x <= BOX_X[ 1 ] ) & ( y >= BOX_Y[ 0 ] ) & ( y <= BOX_Y[ 1 ] )


def draw3d( ax, blk, edge_color, fill_color, lw ):
    """A face inside the box: its surface, shaded lightly and seen through, and on it the edges of its grid whose two
    ends are both kept (iblank 1).  An edge with a blanked end is not drawn, so the grid stops in a jagged line where
    the file blanks it.  Returns the number of edges drawn and left out."""
    from matplotlib.collections import PolyCollection, LineCollection
    x, y, z, ib = blk
    sx, sy = project( x, y, z )
    inside = in_box( x, y )
    nj, ni = x.shape
    quads = []
    for j in range( nj - 1 ):
        for i in range( ni - 1 ):
            if inside[ j, i ] and inside[ j + 1, i ] and inside[ j, i + 1 ] and inside[ j + 1, i + 1 ]:
                quads.append( [ ( sx[ j, i ], sy[ j, i ] ), ( sx[ j, i + 1 ], sy[ j, i + 1 ] ),
                                ( sx[ j + 1, i + 1 ], sy[ j + 1, i + 1 ] ), ( sx[ j + 1, i ], sy[ j + 1, i ] ) ] )
    ax.add_collection( PolyCollection( quads, facecolors = fill_color, edgecolors = "none", alpha = FILL_ALPHA ) )
    kept = ib != 0
    segs = []
    dropped = 0
    for j in range( nj ):
        for i in range( ni ):
            for dj, di in ( ( 0, 1 ), ( 1, 0 ) ):
                j2 = j + dj
                i2 = i + di
                if j2 >= nj or i2 >= ni or not ( inside[ j, i ] and inside[ j2, i2 ] ):
                    continue
                if kept[ j, i ] and kept[ j2, i2 ]:
                    segs.append( [ ( sx[ j, i ], sy[ j, i ] ), ( sx[ j2, i2 ], sy[ j2, i2 ] ) ] )
                else:
                    dropped += 1
    ax.add_collection( LineCollection( segs, colors = edge_color, linewidths = lw ) )
    return len( segs ), dropped


def fig_blanking():
    """Two panels in one view: the Wing and the Fuselage together, and the Fuselage alone.  The Fuselage's grid stops
    around the Wing's root, but the Wing lies on that notch in any view from outside, so the second panel shows it
    without the Wing."""
    fl.style()
    fus, wing = blanking_faces()
    fig, axs = plt.subplots( 2, 1, figsize = ( fl.PAGE_W, 6.0 ) )
    for k, ax in enumerate( axs ):
        draw3d( ax, fus, FUS_COLOR, FUS_FILL, 0.4 )
        if k == 0:
            for blk in wing:
                draw3d( ax, blk, WING_COLOR, WING_FILL, 0.4 )
        ax.autoscale_view()
        ax.set_aspect( "equal" )
        ax.set_axis_off()
    axs[ 0 ].text( 0.0, 1.0, "The Wing and the Fuselage", transform = axs[ 0 ].transAxes, ha = "left", va = "top" )
    axs[ 1 ].text( 0.0, 1.0, "The Fuselage alone", transform = axs[ 1 ].transAxes, ha = "left", va = "top" )
    fig.subplots_adjust( left = 0.01, right = 0.99, top = 0.99, bottom = 0.01, hspace = 0.04 )
    fn = os.path.join( OUT, "POGS_blanking.png" )
    fig.savefig( fn, dpi = 200, metadata = { "Software": None } )
    plt.close( fig )
    print( "made", fn )


if __name__ == "__main__":
    fig_nref()
    fig_blanking()
