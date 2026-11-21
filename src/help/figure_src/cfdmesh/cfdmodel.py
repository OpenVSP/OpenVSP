"""Models made through the openvsp module for the CFD Mesh help figures and checks, and readers for the meshes
CFD Mesh writes."""

import os
import tempfile

import numpy as np
import openvsp as vsp

TMP = tempfile.mkdtemp()


def aircraft( wing_first = False ):
    """A fresh model of a small aircraft: a Fuselage with a Wing through it and a horizontal tail.  wing_first adds
    the Wing before the Fuselage, so that it is the first component."""
    vsp.VSPRenew()
    if wing_first:
        w = vsp.AddGeom( "WING" )
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( f, "Length", "Design", 10.0 )
    if not wing_first:
        w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "X_Rel_Location", "XForm", 3.5 )
    vsp.SetParmVal( w, "Z_Rel_Location", "XForm", -0.25 )
    vsp.SetParmVal( w, "Span", "XSec_1", 5.0 )
    vsp.SetParmVal( w, "Root_Chord", "XSec_1", 2.0 )
    vsp.SetParmVal( w, "Tip_Chord", "XSec_1", 1.0 )
    vsp.SetParmVal( w, "Sweep", "XSec_1", 10.0 )
    h = vsp.AddGeom( "WING" )
    vsp.SetGeomName( h, "HTail" )
    vsp.SetParmVal( h, "X_Rel_Location", "XForm", 8.6 )
    vsp.SetParmVal( h, "Z_Rel_Location", "XForm", 0.1 )
    vsp.SetParmVal( h, "Span", "XSec_1", 1.8 )
    vsp.SetParmVal( h, "Root_Chord", "XSec_1", 1.0 )
    vsp.SetParmVal( h, "Tip_Chord", "XSec_1", 0.5 )
    vsp.SetParmVal( h, "Sweep", "XSec_1", 20.0 )
    vsp.Update()
    return f, w, h


def puck_ball():
    """A small ball close above the flat face of a large box-shaped body (Rob McDonald's PuckBall model, laid flat): a
    Fuselage 30 long whose sections are sharp-cornered rectangles 2.5 wide and 13 tall, turned 90 degrees about X so
    that the box is 2.5 thick in Z, and an Ellipsoid of radius 0.25 whose surface is 0.05 above the box's top face, at
    its middle.  Mesh settings: Max Edge Len 1, Min Edge Len 0.01, Max Gap
    0.001, the rest at their defaults.  Returns the IDs of the box and the ball."""
    vsp.VSPRenew()
    p = vsp.AddGeom( "FUSELAGE" )
    vsp.SetGeomName( p, "Puck" )
    vsp.SetParmVal( p, "Length", "Design", 30.0 )
    xss = vsp.GetXSecSurf( p, 0 )
    while vsp.GetNumXSec( xss ) > 4:
        vsp.CutXSec( p, 2 )
    vsp.Update()
    for i, x in ( ( 1, 0.0 ), ( 2, 1.0 ) ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_ROUNDED_RECTANGLE )
        xs = vsp.GetXSec( xss, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundedRect_Width" ), PUCK_W )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundedRect_Height" ), 13.0 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundRectXSec_Radius" ), 0.0 )
    for i, x in ( ( 0, 0.0 ), ( 1, 0.0 ), ( 2, 1.0 ), ( 3, 1.0 ) ):
        xs = vsp.GetXSec( xss, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "XLocPercent" ), x )
        # No skinning constraints, so the box's sides and ends are flat: the left and right values are
        # unlinked first, or setting one side's flag would set the other's back.
        for side in ( "Top", "Bottom", "Left", "Right" ):
            for what in ( "Angle", "Strength", "Slew", "Curve" ):
                pid = vsp.GetXSecParm( xs, side + "LR" + what + "Eq" )
                if pid:
                    vsp.SetParmVal( pid, 0.0 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "AllSym" ), 0.0 )
        for side in ( "Top", "Bottom", "Left", "Right" ):
            for lr in ( "L", "R" ):
                for what in ( "Angle", "Strength", "Slew", "Curve" ):
                    pid = vsp.GetXSecParm( xs, side + lr + what + "Set" )
                    if pid:
                        vsp.SetParmVal( pid, 0.0 )
    vsp.SetParmVal( p, "X_Rel_Rotation", "XForm", 90.0 )
    b = vsp.AddGeom( "ELLIPSOID" )
    for n in ( "A_Radius", "B_Radius", "C_Radius" ):
        vsp.SetParmVal( b, n, "Design", BALL_R )
    vsp.SetParmVal( b, "X_Rel_Location", "XForm", 15.0 )
    vsp.SetParmVal( b, "Z_Rel_Location", "XForm", 0.5 * PUCK_W + BALL_GAP + BALL_R )
    vsp.Update()
    coarse( maxlen = 1.0, minlen = 0.01, gap = 0.001 )
    return p, b


PUCK_W = 2.5
BALL_R = 0.25
BALL_GAP = 0.05


def negative_volume():
    """A Fuselage 10 long with a Pod marked Negative Volume sunk into its top, at the default mesh settings."""
    vsp.VSPRenew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( f, "Length", "Design", 10.0 )
    n = vsp.AddGeom( "POD" )
    vsp.SetParmVal( n, "Length", "Design", 2.0 )
    vsp.SetParmVal( n, "FineRatio", "Design", 2.0 )
    vsp.SetParmVal( n, "X_Rel_Location", "XForm", 2.0 )
    vsp.SetParmVal( n, "Z_Rel_Location", "XForm", 1.1 )
    vsp.SetParmVal( n, "Negative_Volume_Flag", "Negative_Volume_Props", 1.0 )
    vsp.Update()
    coarse()
    return f, n


def actuator_disk():
    """A Pod 4 long with a Propeller of diameter 3 drawn as a disk around it, at the default mesh settings."""
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.SetParmVal( p, "Length", "Design", 4.0 )
    pr = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( pr, "PropMode", "Design", vsp.PROP_DISK )
    vsp.SetParmVal( pr, "Diameter", "Design", 3.0 )
    vsp.SetParmVal( pr, "X_Rel_Location", "XForm", 1.0 )
    vsp.Update()
    coarse()
    return p, pr


def top():
    """A slender Pod 4 long and 0.5 in radius at the default settings, without sources: its radius of curvature runs
    from 0.5 in the middle down to nothing at its pointed ends, through every curvature regime."""
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.SetParmVal( p, "Length", "Design", 4.0 )
    vsp.SetParmVal( p, "FineRatio", "Design", 8.0 )
    vsp.Update()
    coarse()
    vsp.DeleteAllCFDSources()
    return p


def duct_disk():
    """A flow-through nacelle: a Pod 4 long, of radius 1, cut through end to end by a longer Pod of radius 0.7 marked
    Negative Volume, with a Propeller of diameter 1.7 drawn as a disk across the duct, which trims it to the duct."""
    vsp.VSPRenew()
    n = vsp.AddGeom( "POD" )
    vsp.SetParmVal( n, "Length", "Design", 4.0 )
    vsp.SetParmVal( n, "FineRatio", "Design", 4.0 )
    d = vsp.AddGeom( "POD" )
    vsp.SetParmVal( d, "Length", "Design", 6.0 )
    vsp.SetParmVal( d, "FineRatio", "Design", 6.0 / DUCT_D )
    vsp.SetParmVal( d, "X_Rel_Location", "XForm", -1.0 )
    vsp.SetParmVal( d, "Negative_Volume_Flag", "Negative_Volume_Props", 1.0 )
    pr = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( pr, "PropMode", "Design", vsp.PROP_DISK )
    vsp.SetParmVal( pr, "Diameter", "Design", 1.7 )
    vsp.SetParmVal( pr, "X_Rel_Location", "XForm", 2.1 )
    vsp.Update()
    coarse( maxlen = 0.25, minlen = 0.01 )
    return n, d, pr


DUCT_D = 0.7


def aircraft_bbox():
    """The bounding box of every component of the model, symmetric copies included, from the Vehicle's BBox Parms."""
    v = vsp.GetVehicleID()
    lo = np.array( [ vsp.GetParmVal( vsp.FindParm( v, n + "_Min", "BBox" ) ) for n in ( "X", "Y", "Z" ) ] )
    ln = np.array( [ vsp.GetParmVal( vsp.FindParm( v, n + "_Len", "BBox" ) ) for n in ( "X", "Y", "Z" ) ] )
    return lo, lo + ln


def centered_sphere( r ):
    """An Ellipsoid of radius r centered on the model's bounding box, as it is before the Ellipsoid is added.  Its
    origin is at its upstream pole, so it is placed r upstream of the box's center."""
    lo, hi = aircraft_bbox()
    c = 0.5 * ( lo + hi )
    e = vsp.AddGeom( "ELLIPSOID" )
    for n in ( "A_Radius", "B_Radius", "C_Radius" ):
        vsp.SetParmVal( e, n, "Design", r )
    vsp.SetParmVal( e, "X_Rel_Location", "XForm", c[ 0 ] - r )
    vsp.SetParmVal( e, "Y_Rel_Location", "XForm", c[ 1 ] )
    vsp.SetParmVal( e, "Z_Rel_Location", "XForm", c[ 2 ] )
    vsp.Update()
    return e, c


def coarse( maxlen = 0.5, minlen = 0.025, gap = 0.005, ncirc = 16.0, growth = 1.3 ):
    """Set the main sizing controls."""
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, maxlen )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, minlen )
    vsp.SetCFDMeshVal( vsp.CFD_MAX_GAP, gap )
    vsp.SetCFDMeshVal( vsp.CFD_NUM_CIRCLE_SEGS, ncirc )
    vsp.SetCFDMeshVal( vsp.CFD_GROWTH_RATIO, growth )


def mesh( name = "m", set = vsp.SET_ALL, degen = vsp.SET_NONE, types = vsp.CFD_TRI_TYPE ):
    """Mesh the set and read back the .tri: vertices ( n, 3 ), triangles ( m, 3 ) and the tag of each triangle.
    Returns None when no .tri was written."""
    fn = os.path.join( TMP, name + ".tri" )
    vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, fn )
    if os.path.exists( fn ):
        os.remove( fn )
    vsp.ComputeCFDMesh( set, degen, types )
    if not os.path.exists( fn ):
        return None
    return read_tri( fn )


def read_tri( fn ):
    """The vertices, triangles (from 0) and tags of a Cart3D .tri file."""
    with open( fn ) as f:
        toks = f.read().split()
    nv, nt = int( toks[ 0 ] ), int( toks[ 1 ] )
    p = 2
    V = np.array( toks[ p:p + 3 * nv ], float ).reshape( nv, 3 )
    p += 3 * nv
    T = np.array( toks[ p:p + 3 * nt ], int ).reshape( nt, 3 ) - 1
    p += 3 * nt
    tag = np.array( toks[ p:p + nt ], int )
    return V, T, tag


def edges( V, T ):
    """The unique edges of a triangle mesh and their lengths."""
    E = np.sort( np.vstack( [ T[ :, [ 0, 1 ] ], T[ :, [ 1, 2 ] ], T[ :, [ 2, 0 ] ] ] ), axis = 1 )
    E = np.unique( E, axis = 0 )
    return E, np.linalg.norm( V[ E[ :, 0 ] ] - V[ E[ :, 1 ] ], axis = 1 )


def min_angles( V, T ):
    """The smallest angle of each triangle, degrees."""
    P = V[ T ]
    ang = []
    for k in range( 3 ):
        a = P[ :, ( k + 1 ) % 3 ] - P[ :, k ]
        b = P[ :, ( k + 2 ) % 3 ] - P[ :, k ]
        c = np.einsum( "ij,ij->i", a, b ) / np.maximum( np.linalg.norm( a, axis = 1 ) * np.linalg.norm( b, axis = 1 ),
                                                        1e-300 )
        ang.append( np.degrees( np.arccos( np.clip( c, -1, 1 ) ) ) )
    return np.min( ang, axis = 0 )


def setting( name, group ):
    """The ID of a Parm of the CFD Mesh settings."""
    return vsp.FindParm( vsp.FindContainer( "CFDMeshSettings", 0 ), name, group )


def density( name ):
    """The ID of a Parm of the CFD Mesh grid density."""
    return vsp.FindParm( vsp.FindContainer( "CFDGridDensity", 0 ), name, "CFDGridDensity" )
