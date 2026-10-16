"""Figures for the OpenVSP Human help page (src/help/Human.md).

    <build>/venv/bin/python human_figs.py [names...]      # writes ../../images/Human_*.png

The human is VSP's own, read back through an STL export.  Lengths are in meters (Length Unit M).  3D views are
from about (-1, -1, 1) with Z up; 2D views keep the model frame and carry an axis key.
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )

import numpy as np

import meshfig as mf
from meshfig import fl, vsp, COL

GHOST = "#6FA8DC"
POSED = "#C8CCD2"


def human( pose = None, sym = False ):
    """A default human in meters, R/L Sym off unless sym, posed by pose (a dict of Pose Parms); its ID and triangles."""
    vsp.VSPRenew()
    h = vsp.AddGeom( "HUMAN" )
    vsp.SetParmVal( h, "LenUnit", "Anthropometric", vsp.LEN_M )
    # Updated once with R/L Sym on, as a new human is, so the left side starts as a copy of the right.
    vsp.Update()
    vsp.SetParmVal( h, "RLSym", "Pose", 1.0 if sym else 0.0 )
    for k, v in ( pose or {} ).items():
        vsp.SetParmVal( h, k, "Pose", v )
    vsp.Update()
    return h, mf.geom_tris( h )


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


def fig_arm():
    fig = new_fig()
    cans = pose_panels( fig, [
        ( mf.UPSTREAM, "upstream", { "ShoulderABADRt": 45 }, {}, "Shoulder AB/AD 45" ),
        ( mf.FRONT, "front", { "ShoulderFELt": 45 }, {}, "Shoulder FE 45" ),
        ( mf.FRONT, "front", { "ElbowLt": 90 }, {}, "Elbow 90" ),
        ( mf.TOP, "top", { "ShoulderIERt": 45 }, { "ElbowRt": 90 }, "Shoulder IE 45, Elbow 90" ),
    ] )
    mf.save( fig, "Human_pose_arm", [ cans[ :3 ], cans[ 3: ] ], hmax = 9.0 )


def fig_leg():
    fig = new_fig()
    cans = pose_panels( fig, [
        ( mf.UPSTREAM, "upstream", { "HipABADRt": 30 }, {}, "Hip AB/AD 30" ),
        ( mf.FRONT, "front", { "HipFELt": 60 }, {}, "Hip FE 60" ),
        ( mf.FRONT, "front", { "KneeLt": 60 }, {}, "Knee 60" ),
    ] )
    mf.save( fig, "Human_pose_leg", [ cans ], hmax = 4.8 )


# The Sitting preset, as the Pose tab's Set puts it: every other joint at zero, R/L Sym on.
SITTING = { "ElbowRt": 80, "ShoulderIERt": 45, "HipABADRt": 0.5, "HipFERt": 80, "KneeRt": 80 }


def fig_seated():
    """The human standing and in the Sitting preset, from the side, each with its origin."""
    fig = new_fig()
    cans = []
    for pose, label in ( ( {}, "Standing" ), ( SITTING, "Sitting" ) ):
        h, T = human( pose, sym = True )
        if pose:
            # R/L Sym poses the left side as the right.
            for k, v in pose.items():
                assert vsp.GetParmVal( h, k.replace( "Rt", "Lt" ), "Pose" ) == v
        else:
            assert vsp.GetParmVal( h, "KneeRt", "Pose" ) == 0.0 and vsp.GetParmVal( h, "HipFERt", "Pose" ) == 2.5
        P = T.reshape( -1, 3 )
        if pose:
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


FIGS = { "axes": fig_axes, "seated": fig_seated, "body": fig_body, "arm": fig_arm, "leg": fig_leg }

if __name__ == "__main__":
    mf.style()
    for n in ( sys.argv[ 1: ] or FIGS.keys() ):
        FIGS[ n ]()
