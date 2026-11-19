"""Figures for the OpenVSP Human help page (src/help/Human.md).

    <build>/venv/bin/python human_figs.py [names...]      # writes ../../images/Human_*.png

The human is VSP's own, read back through an STL export (the surfaces through CompVecPnt01).  Lengths are in meters
(Length Unit M).  3D views are from about (-1, -1, 1) with Z up; 2D views keep the model frame and carry an axis key.
"""

import os
import re
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )

import numpy as np

import meshfig as mf
from meshfig import fl, vsp, COL

GHOST = "#6FA8DC"
POSED = "#C8CCD2"

# The order of the preset table in HumanGeom.cpp: the trunk, then each side's limb joints.
TRUNK = [ "Back", "Waist", "TorsoTwist", "TorsoBend", "Nod", "RotateHead", "HeadTilt" ]
SIDE = [ "Wrist", "WristDev", "Forearm", "Elbow", "ShoulderABAD", "ShoulderFE", "ShoulderIE", "HipABAD", "HipFE", "HipIE", "Knee",
         "Ankle" ]


def presets():
    """The preset poses, read from HumanGeom.cpp's table: name -> ( R/L Sym, { Pose Parm: angle } )."""
    src = os.path.join( HERE, "..", "..", "..", "geom_core", "HumanGeom.cpp" )
    with open( src ) as f:
        text = f.read()
    body = text[ text.index( "human_preset_poses[" ): ]
    body = body[ body.index( "{" ) + 1: body.index( "};" ) ]
    out = {}
    for name, entry in re.findall( r"//\s*([A-Z_]+)\s*\n(.*?)(?=//|\Z)", body, re.S ):
        sym = "true" in entry.split( "," )[ 0 ]
        vals = [ float( v ) for v in re.findall( r"-?\d+\.\d*|-?\d+", entry ) ]
        assert len( vals ) == len( TRUNK ) + 2 * len( SIDE ), "the preset table's layout changed: " + name
        pose = dict( zip( TRUNK, vals[ :len( TRUNK ) ] ) )
        for s, suffix in enumerate( ( "Rt", "Lt" ) ):
            row = vals[ len( TRUNK ) + s * len( SIDE ): len( TRUNK ) + ( s + 1 ) * len( SIDE ) ]
            pose.update( { j + suffix: v for j, v in zip( SIDE, row ) } )
        out[ name ] = ( sym, pose )
    assert len( out ) == 17, "expected seventeen presets, found %d" % len( out )
    return out


def human( pose = None, sym = False, rep = None, pitch = 0.0 ):
    """A default human in meters, R/L Sym off unless sym, posed by pose (a dict of Pose Parms); its ID and triangles."""
    vsp.VSPRenew()
    h = vsp.AddGeom( "HUMAN" )
    vsp.SetParmVal( h, "LenUnit", "Anthropometric", vsp.LEN_M )
    if rep is not None:
        vsp.SetParmVal( h, "Representation", "Design", rep )
    # Updated once with R/L Sym on, as a new human is, so the left side starts as a copy of the right.
    vsp.Update()
    vsp.SetParmVal( h, "RLSym", "Pose", 1.0 if sym else 0.0 )
    for k, v in ( pose or {} ).items():
        vsp.SetParmVal( h, k, "Pose", v )
    vsp.SetParmVal( h, "Y_Rel_Rotation", "XForm", pitch )
    vsp.Update()
    if rep == vsp.HUMAN_REP_SURF:
        return h, None
    return h, mf.geom_tris( h )


def surf_tris( h, i, nu = 41, nw = 41 ):
    """Surface i of a Geom, sampled on an nu by nw grid of (u, w): the triangles of that grid."""
    u = np.linspace( 0.0, 1.0, nu )
    w = np.linspace( 0.0, 1.0, nw )
    U, W = np.meshgrid( u, w, indexing = "ij" )
    pts = vsp.CompVecPnt01( h, i, list( U.ravel() ), list( W.ravel() ) )
    P = np.array( [ [ p.x(), p.y(), p.z() ] for p in pts ] ).reshape( nu, nw, 3 )
    a = P[ :-1, :-1 ]
    b = P[ 1:, :-1 ]
    c = P[ 1:, 1: ]
    d = P[ :-1, 1: ]
    T = np.concatenate( [ np.stack( [ a, b, c ], 2 ).reshape( -1, 3, 3 ),
                          np.stack( [ a, c, d ], 2 ).reshape( -1, 3, 3 ) ] )
    return mf.clean( T )


def design_eye( h ):
    """Where a Super Cone Auxiliary attached to the human places its apex: the design eye."""
    a = vsp.AddGeom( "AUXILIARY", h )
    vsp.SetParmVal( a, "AuxiliaryGeomType", "Design", vsp.AUX_GEOM_SUPER_CONE )
    vsp.Update()
    p = vsp.CompPnt01( a, 0, 1.0, 0.0 )
    vsp.DeleteGeom( a )
    vsp.Update()
    return np.array( [ p.x(), p.y(), p.z() ] )


def fig_axes():
    """The human, its origin at the waist joint, and the model axes."""
    h, T = human( sym = True )
    eye = design_eye( h )
    fig, c = mf.new( mf.ISO )
    mf.tris( c, T, POSED )
    O = np.zeros( 3 )
    L = 0.55
    for v, lab in ( ( mf.X, r"$X$" ), ( mf.Y, r"$Y$" ), ( mf.Z, r"$Z$" ) ):
        c.arrow( O, O + L * v, color = "k", lw = 1.3, z = 65 )
        c.text( O + 1.15 * L * v, lab, z = 70 )
    c.point( O, color = "k", s = 10, z = 70 )
    c.text( O, "origin (waist joint)", dx = 8, dy = -8, ha = "left", va = "top" )
    c.point( eye, color = COL[ "lag" ], s = 10, z = 72 )
    c.text( eye, "design eye", color = COL[ "lag" ], dx = -8, dy = 4, ha = "right", va = "bottom" )
    # The person's right hand, so the side is read without a mirror image in mind.
    P = T.reshape( -1, 3 )
    rh = P[ np.argmax( P[ :, 1 ] ) ]
    c.text( rh, "right hand", dx = -6, dy = -4, ha = "right", va = "top" )
    mf.save( fig, "Human_axes", [ [ c ] ], hmax = 4.8 )


# The surfaces' parts, in their order: torso, head, then each right part and its left one.
PARTS = [ "torso", "head" ] + [ p for p in ( "arm", "palm", "thumb", "finger", "finger", "finger", "finger", "leg",
                                             "foot" ) for _ in ( 0, 1 ) ]
PART_COL = { "torso": "#C8CCD2", "head": "#E3C9A8", "arm": "#9FC5E8", "palm": "#B6D7A8", "thumb": "#F9CB9C",
             "finger": "#D5A6BD", "leg": "#B4A7D6", "foot": "#FFE599" }


def fig_surface():
    """The SURFACE representation: its twenty parts, each in its own color."""
    h, _ = human( sym = True, rep = vsp.HUMAN_REP_SURF )
    n = vsp.GetNumMainSurfs( h )
    assert n == len( PARTS ), "expected %d surfaces, found %d" % ( len( PARTS ), n )
    fig, c = mf.new( mf.ISO )
    for i in range( n ):
        k = 31
        if PARTS[ i ] in ( "torso", "head", "arm", "leg" ):
            k = 101
        mf.tris( c, surf_tris( h, i, k, k ), PART_COL[ PARTS[ i ] ] )
    mf.save( fig, "Human_surface", [ [ c ] ], hmax = 4.8 )


def pose_panels( fig, specs ):
    """One canvas per (view, key, pose, base pose, label): the posed human shaded, and the part that moved, as it was
    in the base pose, tinted over it."""
    cans = []
    for view, which, pose, base, label in specs:
        _, G = human( base )
        merged = dict( base )
        merged.update( pose )
        _, T = human( merged )
        c = mf.panel( fig, view )
        mf.tris( c, T, POSED )
        c.flush( z0 = 1.0 )
        # The limb as it was: the triangles that moved, one translucent tint over the posed human.
        dmax = np.linalg.norm( T - G, axis = 2 ).max( axis = 1 )
        moved = dmax > 0.2 * dmax.max()
        mf.tris( c, G[ moved ], GHOST, flat = True, z0 = 5.0, group_alpha = 0.5 )
        Q = c.v.p2( np.vstack( [ T.reshape( -1, 3 ), G.reshape( -1, 3 ) ] ) )
        lo = Q.min( 0 )
        hi = Q.max( 0 )
        c.ax.annotate( label, xy = ( 0.5 * ( lo[0] + hi[0] ), lo[1] ), xytext = ( 0, -6 ), textcoords = "offset points",
                       ha = "center", va = "top", fontsize = fl.FS )
        mf.key( c, which, at = ( 1.0, 0.0 ), dx = 20, dy = 16 )
        cans.append( c )
    return cans


def new_fig():
    import matplotlib.pyplot as plt
    return plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )


# In the front view (looking along +Y) the person's left side, -Y, is nearer the reader, so those panels move the
# left side; the left angles mean the same motions as the right.

def fig_body():
    fig = new_fig()
    cans = pose_panels( fig, [
        ( mf.FRONT, "front", { "Back": 30 }, {}, "Back 30" ),
        ( mf.FRONT, "front", { "Waist": 30 }, {}, "Waist 30" ),
        ( mf.FRONT, "front", { "Nod": 30 }, {}, "Nod 30" ),
    ] )
    mf.save( fig, "Human_pose_body", [ cans ], hmax = 4.8 )


def fig_torso():
    fig = new_fig()
    cans = pose_panels( fig, [
        ( mf.TOP, "top", { "TorsoTwist": 30 }, {}, "Twist 30" ),
        ( mf.TOP, "top", { "RotateHead": 45 }, {}, "Rotate 45" ),
        ( mf.UPSTREAM, "upstream", { "TorsoBend": 20 }, {}, "Side Bend 20" ),
        ( mf.UPSTREAM, "upstream", { "HeadTilt": 30 }, {}, "Tilt 30" ),
    ] )
    mf.save( fig, "Human_pose_torso", [ cans[ :2 ], cans[ 2: ] ], hmax = 9.0 )


def fig_arm():
    fig = new_fig()
    cans = pose_panels( fig, [
        ( mf.UPSTREAM, "upstream", { "ShoulderABADRt": 90 }, {}, "Shoulder AB/AD 90" ),
        ( mf.FRONT, "front", { "ShoulderFELt": 90 }, {}, "Shoulder FE 90" ),
        ( mf.FRONT, "front", { "ElbowLt": 90 }, {}, "Elbow 90" ),
        ( mf.TOP, "top", { "ShoulderIERt": 45 }, { "ElbowRt": 90 }, "Shoulder IE 45, Elbow 90" ),
        ( mf.UPSTREAM, "upstream", { "ForearmRt": 60 }, { "ElbowRt": 90 }, "Forearm 60, Elbow 90" ),
        ( mf.UPSTREAM, "upstream", { "WristRt": 45 }, {}, "Wrist FE 45" ),
        ( mf.FRONT, "front", { "WristDevLt": 35 }, {}, "Wrist Deviation 35" ),
    ] )
    mf.save( fig, "Human_pose_arm", [ cans[ :3 ], cans[ 3:6 ], cans[ 6: ] ], hmax = 13.0 )


def fig_leg():
    fig = new_fig()
    cans = pose_panels( fig, [
        ( mf.UPSTREAM, "upstream", { "HipABADRt": 30 }, {}, "Hip AB/AD 30" ),
        ( mf.FRONT, "front", { "HipFELt": 60 }, {}, "Hip FE 60" ),
        ( mf.FRONT, "front", { "KneeLt": 60 }, {}, "Knee 60" ),
        ( mf.UPSTREAM, "upstream", { "HipIERt": 30 }, { "HipFERt": 90, "KneeRt": 90 },
          "Hip IE 30, Hip FE 90, Knee 90" ),
        ( mf.FRONT, "front", { "AnkleLt": 20 }, {}, "Ankle 20" ),
    ] )
    mf.save( fig, "Human_pose_leg", [ cans[ :3 ], cans[ 3: ] ], hmax = 9.0 )


def fig_seated():
    """The human standing and in the Sitting preset, from the side, each with its origin."""
    fig = new_fig()
    cans = []
    for name, label in ( ( "STANDING", "Standing" ), ( "SITTING", "Sitting" ) ):
        sym, pose = presets()[ name ]
        h, T = human( pose, sym = sym )
        P = T.reshape( -1, 3 )
        if name == "SITTING":
            # Seated, the thighs reach forward of the origin (toward -X) and the feet hang below it.
            assert P[ :, 0 ].min() < -0.4 and P[ :, 2 ].min() < -0.4
        c = mf.panel( fig, mf.FRONT )
        mf.tris( c, T, POSED )
        c.flush()
        O = np.zeros( 3 )
        c.point( O, color = "k", s = 10, z = 70 )
        for d in ( mf.X, mf.Z ):
            c.line( np.array( [ -0.25 * d, 0.25 * d ] ), color = "k", lw = 0.6, z = 69 )
        c.text( O, "origin", dx = 8, dy = 6, ha = "left", va = "bottom" )
        cans.append( ( c, label ) )
    box = ( np.min( [ c.limits()[0] for c, _ in cans ], 0 ), np.max( [ c.limits()[1] for c, _ in cans ], 0 ) )
    for c, label in cans:
        c.bounds = box
        c.ax.annotate( label, xy = ( 0.0, box[0][1] ), xytext = ( 0, -6 ), textcoords = "offset points", ha = "center",
                       va = "top", fontsize = fl.FS )
    cans = [ c for c, _ in cans ]
    mf.key( cans[ 1 ], "front", at = ( 1.0, 0.0 ), dx = 20, dy = 16 )
    mf.save( fig, "Human_seated", [ cans ], hmax = 4.8 )


def preset_gallery( name, rows ):
    """The presets named in rows, each as Set puts it, in one 3D view at one scale."""
    table = presets()
    fig = new_fig()
    grid = []
    for row in rows:
        cans = []
        for p in row:
            sym, pose = table[ p ]
            # Crawling is drawn as it is meant to be used, pitched forward 90 degrees on the XForm tab.
            pitch = 0.0
            if p == "CRAWLING":
                pitch = -90.0
            _, T = human( pose, sym = sym, pitch = pitch )
            c = mf.panel( fig, mf.ISO )
            mf.tris( c, T, POSED )
            c.flush()
            cans.append( ( c, p.replace( "_", " " ) ) )
        grid.append( cans )
    for cans in grid:
        for c, label in cans:
            lo, hi = c.limits()
            c.ax.annotate( label, xy = ( 0.5 * ( lo[0] + hi[0] ), lo[1] ), xytext = ( 0, -6 ),
                           textcoords = "offset points", ha = "center", va = "top", fontsize = fl.FS )
    mf.save( fig, name, [ [ c for c, _ in cans ] for cans in grid ], hmax = 9.0 )


def fig_presets():
    preset_gallery( "Human_presets_seated", [ [ "SITTING", "SEATED_ERECT", "PASSENGER", "DRIVER" ],
                                               [ "PILOT", "RECUMBENT", "NBP_SKYLAB", "NBP_ISS" ] ] )
    preset_gallery( "Human_presets_other", [ [ "STANDING", "KNEELING", "KNEELING_ONE_KNEE", "SQUATTING", "CRAWLING" ],
                                              [ "FORWARD_REACH", "OVERHEAD_REACH", "SPAN", "WALKING" ] ] )


FIGS = { "axes": fig_axes, "seated": fig_seated, "surface": fig_surface, "presets": fig_presets, "body": fig_body,
         "torso": fig_torso, "arm": fig_arm, "leg": fig_leg }

if __name__ == "__main__":
    mf.style()
    for n in ( sys.argv[ 1: ] or FIGS.keys() ):
        FIGS[ n ]()
