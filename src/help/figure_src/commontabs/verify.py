"""Check what the pages of the common tabs (GenTab.md, XFormTab.md, MassTab.md and SubTab.md) state about
placement, attachment, flip, symmetry, scale, mass and sub-surfaces against the openvsp module.

Run with the build's own venv:
    <build>/venv/bin/python verify.py

Its analyses write their files to a temporary folder, which it removes.  Exits non-zero if any check fails.
"""

import os
import shutil
import sys
import tempfile

import numpy as np
import openvsp as vsp

FAILS = []


def check( ok, what ):
    print( "%-4s %s" % ( "ok" if ok else "FAIL", what ) )
    if not ok:
        FAILS.append( what )


def P( gid, s, u, w ):
    p = vsp.CompPnt01( gid, s, u, w )
    return np.array( [ p.x(), p.y(), p.z() ] )


def g( gid, n, grp = "XForm" ):
    return vsp.GetParmVal( gid, n, grp )


def s( gid, n, grp, v ):
    vsp.SetParmVal( gid, n, grp, float( v ) )
    vsp.Update()


def close( a, b, tol = 1e-6 ):
    return np.abs( np.asarray( a, float ) - np.asarray( b, float ) ).max() < tol


def R( axis, a ):
    a = np.radians( a )
    c, sn = np.cos( a ), np.sin( a )
    if axis == 0:
        return np.array( [ [ 1, 0, 0 ], [ 0, c, -sn ], [ 0, sn, c ] ] )
    if axis == 1:
        return np.array( [ [ c, 0, sn ], [ 0, 1, 0 ], [ -sn, 0, c ] ] )
    return np.array( [ [ c, -sn, 0 ], [ sn, c, 0 ], [ 0, 0, 1 ] ] )


def placement():
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    p0 = P( p, 0, 0.5, 0.25 )
    s( p, "X_Rel_Rotation", "XForm", 30 )
    s( p, "Y_Rel_Rotation", "XForm", 50 )
    s( p, "Z_Rel_Rotation", "XForm", 70 )
    check( close( P( p, 0, 0.5, 0.25 ), R( 0, 30 ) @ R( 1, 50 ) @ R( 2, 70 ) @ p0 ), "rotations apply as Rx Ry Rz (Z first)" )
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    s( p, "Origin", "XForm", 0.5 )
    check( g( p, "X_Location" ) == 5.0 and g( p, "X_Rel_Location" ) == 0.0, "Rot Origin 0.5 on a 10 pod: Abs XLoc 5, Rel 0" )
    s( p, "Z_Rel_Rotation", "XForm", 90 )
    check( close( P( p, 0, 0, 0 ), [ 5, -5, 0 ] ) and close( P( p, 0, 1, 0 ), [ 5, 5, 0 ] ), "the pod turns about its middle" )


def attach():
    vsp.VSPRenew()
    f = vsp.AddGeom( "POD" )
    c = vsp.AddGeom( "POD", f )
    vsp.Update()
    check( g( c, "Trans_Attach_Flag", "Attach" ) == vsp.ATTACH_TRANS_NONE and g( c, "Abs_Or_Relitive_flag" ) == vsp.REL,
           "a new child is unattached, in Rel" )
    s( f, "X_Rel_Location", "XForm", 3 )
    check( close( P( c, 0, 0, 0 ), [ 0, 0, 0 ] ), "an unattached child stays when the parent moves" )
    s( c, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_COMP )
    check( close( P( c, 0, 0, 0 ), [ 3, 0, 0 ] ) and g( c, "X_Rel_Location" ) == 0, "attaching keeps Rel, so the child jumps" )
    s( f, "Z_Rel_Rotation", "XForm", 30 )
    check( close( P( c, 0, 1, 0 ), [ 13, 0, 0 ] ), "Translate Comp alone does not turn with the parent" )
    s( c, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_COMP )
    check( close( P( c, 0, 1, 0 ), [ 3 + 10 * np.cos( np.radians( 30 ) ), 5, 0 ] ), "Rotate Comp turns with the parent" )
    s( f, "Z_Rel_Rotation", "XForm", 0 )
    s( c, "Abs_Or_Relitive_flag", "XForm", vsp.ABS )
    vsp.SetParmValUpdate( vsp.GetParm( c, "X_Location", "XForm" ), 5.0 )
    check( g( c, "X_Rel_Location" ) == 2.0, "Abs XLoc 5 on a parent at 3 gives Rel 2" )
    s( f, "X_Rel_Location", "XForm", 6 )
    check( g( c, "X_Location" ) == 8.0, "an attached child in Abs still moves with its parent" )

    # UW axes: x along U, z the outward normal.
    vsp.VSPRenew()
    f = vsp.AddGeom( "FUSELAGE" )
    b = vsp.AddGeom( "BLANK", f )
    vsp.Update()
    s( b, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    s( b, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_UV )
    for w in ( 0.0, 0.25, 0.5, 0.75 ):
        vsp.SetParmVal( b, "U_Attach_Location", "Attach", 0.4 )
        s( b, "V_Attach_Location", "Attach", w )
        Rm = R( 0, g( b, "X_Rotation" ) ) @ R( 1, g( b, "Y_Rotation" ) ) @ R( 2, g( b, "Z_Rotation" ) )
        n = vsp.CompNorm01( f, 0, 0.4, w )
        n = np.array( [ n.x(), n.y(), n.z() ] )
        check( ( Rm @ [ 0, 0, 1 ] ) @ n > 0.999 and ( Rm @ [ 1, 0, 0 ] )[0] > 0.9, "UW axes at W %g: z normal, x along U" % w )
    sides = { 0.0: "+Y", 0.25: "-Z", 0.5: "-Y", 0.75: "+Z" }
    for w, d in sides.items():
        q = P( f, 0, 0.5, w )
        k = "XYZ".index( d[1] )
        check( np.sign( q[k] ) == ( 1 if d[0] == "+" else -1 ), "Fuselage W %g is the %s side" % ( w, d ) )
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    check( P( p, 0, 0.5, 0.0 )[2] > 0 and P( p, 0, 0.5, 0.25 )[1] > 0, "Pod W 0 is the top, W 0.25 the +Y side" )
    vsp.VSPRenew()
    w_ = vsp.AddGeom( "WING" )
    vsp.Update()
    te, lo, le, up = ( P( w_, 0, 0.5, t ) for t in ( 0.0, 0.25, 0.5, 0.75 ) )
    check( te[0] > le[0] and lo[2] < 0 and up[2] > 0, "Wing W 0 trailing edge, 0.25 lower, 0.5 leading edge, 0.75 upper" )

    # U 0N on a wing counts the caps; eta is the span fraction.
    b = vsp.AddGeom( "BLANK", w_ )
    vsp.Update()
    s( b, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    s( b, "U_01", "Attach", 0 )
    s( b, "U_Attach_Location0N", "Attach", 1e6 )
    check( g( b, "U_Attach_Location0N", "Attach" ) == 3.0, "a one-panel wing runs U 0 to 3" )
    s( b, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_EtaMN )
    vsp.SetParmVal( b, "M_Attach_Location", "Attach", 0.25 )
    vsp.SetParmVal( b, "N_Attach_Location", "Attach", 0.5 )
    for eta in ( 0.0, 0.5, 1.0 ):
        s( b, "Eta_Attach_Location", "Attach", eta )
        check( abs( g( b, "Y_Location" ) - 9.0 * eta ) < 1e-6, "eta %g is that fraction of the span" % eta )
    s( b, "Eta_Attach_Location", "Attach", 0.0 )
    check( abs( g( b, "X_Location" ) - 3.0 ) < 1e-4, "M 0.25 at the root (chord 4, LE at 0) is 75% chord" )

    # Switching UW -> RST -> LMN keeps the point.
    s( b, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    s( b, "U_01", "Attach", 1 )
    vsp.SetParmVal( b, "U_Attach_Location", "Attach", 0.55 )
    s( b, "V_Attach_Location", "Attach", 0.3 )
    a0 = np.array( [ g( b, n ) for n in ( "X_Location", "Y_Location", "Z_Location" ) ] )
    ok = True
    for t in ( vsp.ATTACH_TRANS_RST, vsp.ATTACH_TRANS_LMN ):
        s( b, "Trans_Attach_Flag", "Attach", t )
        ok = ok and close( a0, [ g( b, n ) for n in ( "X_Location", "Y_Location", "Z_Location" ) ] )
    check( ok, "switching UW to RST to LMN keeps the attachment point" )


def flip_and_symmetry():
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    s( w, "Sym_Planar_Flag", "Sym", 0 )
    s( w, "X_Rel_Rotation", "XForm", 30 )
    s( w, "Flip_Flag", "Sym", vsp.SYM_XZ )
    t = P( w, 0, 1, 0.5 )
    check( t[1] < 0 and t[2] < 0, "a wing flipped in XZ and turned by XRot 30 tips down" )
    s( w, "X_Rel_Rotation", "XForm", 0 )
    c1 = vsp.AddGeom( "BLANK", w )
    vsp.Update()
    s( c1, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_COMP )
    s( c1, "Y_Rel_Location", "XForm", 5 )
    check( g( c1, "Y_Location" ) == 5.0, "a Comp child of a flipped parent is not reflected" )
    s( c1, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    s( c1, "Y_Rel_Location", "XForm", 0 )
    vsp.SetParmVal( c1, "U_Attach_Location", "Attach", 2.0 / 3.0 )
    s( c1, "V_Attach_Location", "Attach", 0.5 )
    check( g( c1, "Y_Location" ) < -8.9, "a UW child of a flipped parent follows the reflected surface" )

    # Symmetry frames.
    vsp.VSPRenew()
    f = vsp.AddGeom( "POD" )
    w = vsp.AddGeom( "WING", f )
    vsp.Update()
    s( f, "Y_Rel_Location", "XForm", 2 )
    s( f, "Z_Rel_Rotation", "XForm", 10 )
    check( g( w, "Sym_Ancestor", "Sym" ) == 1 and g( w, "Sym_Ancestor_Origin_Flag", "Sym" ) == 1, "default About 1, Attach" )
    check( close( P( w, 1, 1, 0.5 ), P( w, 0, 1, 0.5 ) * [ 1, -1, 1 ] ), "unattached: default mirrors in the model XZ plane" )
    s( w, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_COMP )
    s( w, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_COMP )
    a, b = P( w, 0, 1, 0.5 ), P( w, 1, 1, 0.5 )
    o = np.array( [ 0, 2, 0 ] )
    nrm = R( 2, 10 ) @ [ 0, 1, 0 ]
    check( close( a - 2 * ( ( a - o ) @ nrm ) * nrm, b ), "attached by Comp: default mirrors in the parent's XZ plane" )
    s( w, "Sym_Ancestor", "Sym", 0 )
    check( close( P( w, 1, 1, 0.5 ), P( w, 0, 1, 0.5 ) * [ 1, -1, 1 ] ), "About 0 mirrors in the model XZ plane" )

    # UW attachment: the default mirrors through the attachment point; About the parent with Object does not.
    vsp.VSPRenew()
    f = vsp.AddGeom( "FUSELAGE" )
    w = vsp.AddGeom( "WING", f )
    vsp.Update()
    s( w, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( w, "U_Attach_Location", "Attach", 0.4 )
    s( w, "V_Attach_Location", "Attach", 0.0 )
    check( close( P( w, 0, 1 / 3., 0.5 ), P( w, 1, 1 / 3., 0.5 ) ), "UW-attached wing: default plane passes through the point" )
    s( w, "Sym_Ancestor", "Sym", 2 )
    s( w, "Sym_Ancestor_Origin_Flag", "Sym", 0 )
    check( close( P( w, 1, 1 / 3., 0.5 ), P( w, 0, 1 / 3., 0.5 ) * [ 1, -1, 1 ] ), "About the fuselage, Object: its center plane" )

    # Planar then axial: XZ with Axial X, N 3 gives six copies.
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    s( p, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
    s( p, "Sym_Axial_Flag", "Sym", vsp.SYM_ROT_X )
    s( p, "Sym_Rot_N", "Sym", 3 )
    check( vsp.GetNumMainSurfs( p ) == 1 and len( vsp.GetGeomDisplayTessPnts( p, 5 ) ) > 0, "six copies: surface 5 exists" )


def xform_more():
    """XFormTab.md: Abs/Rel when the attachment changes, U against L and eta, a vertical tail's symmetry."""
    vsp.VSPRenew()
    f = vsp.AddGeom( "POD" )
    c = vsp.AddGeom( "POD", f )
    vsp.Update()
    s( f, "X_Rel_Location", "XForm", 3 )
    s( c, "Y_Rel_Location", "XForm", 2 )
    s( c, "Abs_Or_Relitive_flag", "XForm", vsp.ABS )
    vsp.SetParmValUpdate( c, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_COMP )
    check( close( P( c, 0, 0, 0 ), [ 0, 2, 0 ] ) and g( c, "X_Rel_Location" ) == -3.0,
           "in Abs, attaching keeps the child in place and recomputes Rel" )
    s( c, "Abs_Or_Relitive_flag", "XForm", vsp.REL )
    check( close( P( c, 0, 0, 0 ), [ 0, 2, 0 ] ), "switching back to Rel does not move it" )
    s( f, "X_Rel_Location", "XForm", 5 )
    check( close( P( c, 0, 0, 0 ), [ 2, 2, 0 ] ), "and it then follows its parent" )

    # U counts sections whatever their spacing; L is a fraction of the length.
    vsp.VSPRenew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    xs = vsp.GetXSecSurf( f, 0 )
    for i, x in ( ( 1, 0.1 ), ( 2, 0.25 ) ):
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xs, i ), "XLocPercent" ), x )
        vsp.Update()
    b = vsp.AddGeom( "BLANK", f )
    vsp.Update()
    s( b, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_RST )
    s( b, "R_Attach_Location", "Attach", 0.5 )
    check( abs( g( b, "X_Location" ) - 7.5 ) < 1e-6, "R 0.5 is the third cross section, wherever it is (X 7.5 of 30)" )
    s( b, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_LMN )
    s( b, "L_Attach_Location", "Attach", 0.5 )
    check( abs( g( b, "X_Location" ) - 15.0 ) < 1e-2, "L 0.5 is half the length (X 15 of 30)" )

    # A panel's span changed: a UW point stays with its section, an eta point at its fraction of the span.
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    bu = vsp.AddGeom( "BLANK", w )
    be = vsp.AddGeom( "BLANK", w )
    vsp.Update()
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    vsp.SetParmVal( bu, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( bu, "U_Attach_Location", "Attach", 0.5 )
    vsp.SetParmVal( be, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_EtaMN )
    s( be, "Eta_Attach_Location", "Attach", 0.5 )
    s( w, "Span", "XSec_1", 2.0 )
    half = 0.5 * g( w, "TotalSpan", "WingGeom" )
    check( abs( g( bu, "Y_Location" ) - 2.0 ) < 1e-6 and abs( g( be, "Y_Location" ) - 0.5 * half ) < 1e-6,
           "a panel's span changed: U stays at its section, eta at its fraction of the span" )

    # A wing turned XRot 90 keeps its default XZ symmetry: the copy lies on it, and Total Span counts both.
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    check( g( w, "Sym_Planar_Flag", "Sym" ) == vsp.SYM_XZ, "a new Wing has Planar XZ symmetry" )
    s( w, "X_Rel_Rotation", "XForm", 90 )
    check( close( P( w, 0, 1, 0.5 ), P( w, 1, 1, 0.5 ) ), "XRot 90 with XZ symmetry: the copy coincides" )
    span2 = g( w, "TotalSpan", "WingGeom" )
    s( w, "Sym_Planar_Flag", "Sym", 0 )
    check( span2 == 2 * g( w, "TotalSpan", "WingGeom" ), "Total Span counts the symmetric copy" )


def scale():
    vsp.VSPRenew()
    f = vsp.AddGeom( "POD" )
    c = vsp.AddGeom( "POD", f )
    vsp.Update()
    s( f, "X_Rel_Location", "XForm", 2 )
    s( f, "Scale", "XForm", 2 )
    check( g( f, "Length", "Design" ) == 20 and g( f, "X_Rel_Location" ) == 2 and g( c, "Length", "Design" ) == 10,
           "Scale grows the Geom only, not its place or its children" )
    check( close( P( f, 0, 0, 0 ), [ 2, 0, 0 ] ), "it grows about its own origin" )
    s( f, "Length", "Design", 25 )
    s( f, "Scale", "XForm", 1 )
    check( g( f, "Length", "Design" ) == 12.5, "an edit made while scaled is scaled back" )


def mass():
    def total():
        vsp.ComputeMassProps( vsp.SET_ALL, 40, vsp.X_DIR )
        r = vsp.FindLatestResultsID( "Mass_Properties" )
        return vsp.GetDoubleResults( r, "Total_Mass" )[0]
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    for n, v in ( ( "PointMass", 10 ), ( "CGy", 2 ), ( "Density", 0 ) ):
        vsp.SetParmVal( w, n, "Mass_Props", v )
    vsp.Update()
    check( abs( total() - 20 ) < 1e-9, "a mirrored wing with Mass 10 adds 20" )

    masses = {}
    for pa, pb in ( ( 0, 0 ), ( 1, 0 ), ( 0, 1 ) ):
        vsp.VSPRenew()
        a = vsp.AddGeom( "POD" )
        b = vsp.AddGeom( "POD" )
        vsp.Update()
        vsp.SetParmVal( b, "X_Rel_Location", "XForm", 5 )
        vsp.SetParmVal( b, "Density", "Mass_Props", 10 )
        vsp.SetParmVal( a, "Mass_Prior", "Mass_Props", pa )
        vsp.SetParmVal( b, "Mass_Prior", "Mass_Props", pb )
        vsp.Update()
        masses[ ( pa, pb ) ] = total()
    check( abs( masses[ ( 0, 0 ) ] - masses[ ( 1, 0 ) ] ) < 1e-9 and masses[ ( 0, 1 ) ] > masses[ ( 1, 0 ) ] + 1,
           "overlap goes to the higher priority; at a tie, to the earlier Geom" )

    vsp.VSPRenew()
    a = vsp.AddGeom( "POD" )
    b = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.SetParmVal( b, "X_Rel_Location", "XForm", 5 )
    for q in ( a, b ):
        vsp.SetParmVal( q, "Density", "Mass_Props", 0 )
        vsp.SetParmVal( q, "Shell_Flag", "Mass_Props", 1 )
    vsp.Update()
    m = total()
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    r = vsp.FindLatestResultsID( "Comp_Geom" )
    wet = sum( vsp.GetDoubleResults( r, "Wet_Area" )[ :2 ] )
    check( abs( m - wet ) / wet < 2e-3, "thin shell mass is Mass/Area times the wetted area" )

    vsp.VSPRenew()
    a = vsp.AddGeom( "POD" )
    b = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.SetParmVal( b, "X_Rel_Location", "XForm", 5 )
    vsp.Update()
    both = total()
    vsp.SetParmVal( b, "Negative_Volume_Flag", "Negative_Volume_Props", 1 )
    vsp.Update()
    cut = total()
    vol = 2 * 7.647160648941055 - both
    check( abs( cut - ( 7.647160648941055 - vol ) ) < 2e-2, "a negative volume removes the overlap" )


def tess():
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    s( p, "Tess_W", "Shape", 10 )
    s( f, "Tess_W", "Shape", 10 )
    check( g( p, "Tess_W", "Shape" ) == 13 and g( f, "Tess_W", "Shape" ) == 17, "Num_W rounds up to 4n+1 (8n+1 Fuselage)" )
    s( p, "Tess_U", "Shape", 5 )
    xs = set( round( q.x(), 6 ) for q in vsp.GetGeomDisplayTessPnts( p, 0 ) )
    check( len( xs ) == 9, "Num_U counts points per span between sections (a pod has two)" )


def subsurf():
    def tags( ctype, test ):
        vsp.VSPRenew()
        p = vsp.AddGeom( "POD" )
        vsp.Update()
        ss = vsp.AddSubSurf( p, vsp.SS_LINE )
        vsp.Update()
        ids = { vsp.GetParmName( q ): q for q in vsp.GetSubSurfParmIDs( ss ) }
        vsp.SetParmVal( ids[ "Const_Line_Type" ], ctype )
        vsp.SetParmVal( ids[ "Const_Line_Value" ], 0.2 )
        vsp.SetParmVal( ids[ "Test_Type" ], test )
        vsp.Update()
        vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
        r = vsp.FindLatestResultsID( "Comp_Geom" )
        d = dict( zip( vsp.GetStringResults( r, "Tag_Name" ), vsp.GetDoubleResults( r, "Tag_Wet_Area" ) ) )
        tagged = [ v for k, v in d.items() if "SS_LINE" in k ][0]
        return tagged / sum( d.values() )
    check( tags( vsp.CONST_U, vsp.SS_LINE_TEST_GREATER ) > 0.8 and tags( vsp.CONST_U, vsp.SS_LINE_TEST_LESS ) < 0.2,
           "Greater tags larger U (U 0.2)" )
    check( tags( vsp.CONST_W, vsp.SS_LINE_TEST_GREATER ) > 0.7, "Greater tags larger W (W 0.2)" )

    vsp.VSPRenew()
    p = vsp.AddGeom( "PROP" )
    vsp.Update()
    a = vsp.AddSubSurf( p, vsp.SS_RECTANGLE )
    b = vsp.AddSubSurf( p, vsp.SS_RECTANGLE, -1 )
    vsp.Update()
    idx = lambda sid: vsp.GetParmVal( { vsp.GetParmName( q ): q for q in vsp.GetSubSurfParmIDs( sid ) }[ "MainSurfIndx" ] )
    check( idx( a ) == 0 and idx( b ) == -1, "AddSubSurf defaults to surface 0; -1 is all" )

    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    x = vsp.AddSubSurf( p, vsp.SS_XSEC_CURVE )
    vsp.Update()
    diam = [ vsp.GetParmVal( q ) for c in vsp.FindContainers() for q in vsp.FindContainerParmIDs( c )
             if vsp.GetParmName( q ) == "Circle_Diameter" ]
    check( diam == [ 1.0 ], "an XSec Curve sub-surface starts as a circle of diameter 1" )


def subtab_ssp( sid, name, val = None ):
    ids = { vsp.GetParmName( q ): q for q in vsp.GetSubSurfParmIDs( sid ) }
    if val is not None:
        vsp.SetParmVal( ids[ name ], float( val ) )
        vsp.Update()
    return vsp.GetParmVal( ids[ name ] )


def subtab_hinges( sid ):
    """( start, end ) of a control surface's hinge line on each surface copy, from DegenGeom, upper and lower averaged."""
    vsp.SetComputationFileName( vsp.DEGEN_GEOM_CSV_TYPE, os.path.abspath( "subtab_degen.csv" ) )
    vsp.ComputeDegenGeom( vsp.SET_ALL, vsp.DEGEN_GEOM_CSV_TYPE )
    lines = open( "subtab_degen.csv" ).read().splitlines()
    out = []
    for k, L in enumerate( lines ):
        if L.startswith( "HINGELINE," ) and L.split( "," )[1] == vsp.GetSubSurfName( sid ):
            n = int( L.split( "," )[2] )
            rows = np.array( [ [ float( v ) for v in lines[ k + 2 + i ].split( "," ) ] for i in range( n ) ] )
            out.append( ( rows[ :, 4:7 ].mean( 0 ), rows[ :, 7:10 ].mean( 0 ) ) )
    return out


def subtab_more():
    """SubTab.md: Line [0, N], Ellipse polygons, control surface hinge direction and VSPAERO gains."""
    # [0, N]: whole numbers fall on sections, caps counted on a Wing; W 2 is a Wing's leading edge.
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    ln = vsp.AddSubSurf( w, vsp.SS_LINE )
    vsp.Update()
    subtab_ssp( ln, "Value_01", 0 )
    subtab_ssp( ln, "Const_Line_Value0N", 1 )
    root = P( w, 0, subtab_ssp( ln, "Const_Line_Value" ), 0.0 )
    check( abs( subtab_ssp( ln, "Const_Line_Value" ) - 1.0 / 3.0 ) < 1e-12 and abs( root[1] ) < 1e-9,
           "Wing Line U 1 of [0, N] is the root airfoil" )
    subtab_ssp( ln, "Const_Line_Type", vsp.CONST_W )
    subtab_ssp( ln, "Const_Line_Value0N", 2 )
    wv = subtab_ssp( ln, "Const_Line_Value" )
    xs = [ P( w, 0, 0.5, a )[0] for a in np.linspace( 0, 1, 401 ) ]
    check( abs( wv - 0.5 ) < 1e-12 and abs( P( w, 0, 0.5, wv )[0] - min( xs ) ) < 1e-9,
           "Wing Line W 2 of [0, N] is the leading edge" )
    vsp.VSPRenew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    a = vsp.AddSubSurf( f, vsp.SS_LINE )
    b = vsp.AddSubSurf( f, vsp.SS_LINE )
    vsp.Update()
    subtab_ssp( a, "Value_01", 0 )
    subtab_ssp( a, "Const_Line_Value0N", 2 )
    subtab_ssp( b, "Const_Line_Value", 0.5 )
    check( abs( subtab_ssp( a, "Const_Line_Value" ) - 0.5 ) < 1e-12, "Fuselage Line U 2 of [0, N] is the third section" )
    vsp.InsertXSec( f, 3, vsp.XS_CIRCLE )
    vsp.Update()
    check( abs( subtab_ssp( a, "Const_Line_Value0N" ) - 2.0 ) < 1e-12 and abs( subtab_ssp( b, "Const_Line_Value0N" ) - 2.5 ) < 1e-12,
           "a section added behind: [0, N] stays on section 2, [0, 1] 0.5 moves to 2.5" )

    # Ellipse: Num Points corners, the first at Theta from U.
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    e = vsp.AddSubSurf( p, vsp.SS_ELLIPSE )
    vsp.Update()
    for n, th in ( ( 3, 0.0 ), ( 3, 60.0 ), ( 6, 30.0 ) ):
        subtab_ssp( e, "Tess_Num", n )
        subtab_ssp( e, "Theta", th )
        vsp.SetComputationFileName( vsp.DEGEN_GEOM_CSV_TYPE, os.path.abspath( "subtab_degen.csv" ) )
        vsp.ComputeDegenGeom( vsp.SET_ALL, vsp.DEGEN_GEOM_CSV_TYPE )
        lines = open( "subtab_degen.csv" ).read().splitlines()
        k = [ i for i, L in enumerate( lines ) if L.startswith( "SUBSURF_BNDY" ) ][0]
        m = int( lines[k].split( "," )[1] )
        uw = np.array( [ [ float( v ) for v in lines[ k + 2 + i ].split( "," )[ :2 ] ] for i in range( m ) ] ) / [ 2.0, 4.0 ]
        got = uw[ :-1 ] - [ 0.5, 0.5 ]
        ang = np.radians( th + 360.0 * np.arange( n ) / n )
        want = 0.1 * np.column_stack( [ np.cos( ang ), np.sin( ang ) ] )
        ok = len( got ) == n and all( np.abs( got - q ).max( 1 ).min() < 1e-9 for q in want )
        check( ok, "Ellipse Num Points %d, Theta %g: %d corners, at Theta + k 360/N from U" % ( n, th, n ) )
        if th > 0:
            # Theta 180/N: two corners share the largest U, so a side, not a corner, faces along U.
            check( np.sum( np.abs( got[ :, 0 ] - got[ :, 0 ].max() ) < 1e-9 ) == 2,
                   "Theta 180/N with Num Points %d puts a side across U" % n )

    # Control surface: hinge from Start to End, root to tip on both copies; positive is right-handed about it.
    for le in ( 0, 1 ):
        vsp.VSPRenew()
        w = vsp.AddGeom( "WING" )
        vsp.Update()
        cs = vsp.AddSubSurf( w, vsp.SS_CONTROL )
        vsp.Update()
        subtab_ssp( cs, "LE_Flag", le )
        h = subtab_hinges( cs )
        ok = len( h ) == 2 and h[0][1][1] > h[0][0][1] > 0 and h[1][1][1] < h[1][0][1] < 0
        check( ok, "control surface hinge runs Start (root) to End (tip) on the wing and on its mirror%s" % ( " (LE)" if le else "" ) )
        # Right-handed about +Y-ish on the main surface: a point aft of the hinge goes down, ahead of it up.
        xs, xe = h[0]
        k = ( xe - xs ) / np.linalg.norm( xe - xs )
        v = np.array( [ 1.0, 0, 0 ] ) * ( -1 if le else 1 )
        t = np.radians( 10 )
        r = v * np.cos( t ) + np.cross( k, v ) * np.sin( t )
        check( ( r[2] < 0 ) if not le else ( r[2] > 0 ),
               "positive deflection turns a %s surface %s" % ( "leading-edge" if le else "trailing-edge", "up" if le else "down" ) )

    # VSPAERO: gains 1, 1 roll the wing (no lift); gains 1, -1 lift it (a flap).
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    cs = vsp.AddSubSurf( w, vsp.SS_CONTROL )
    vsp.Update()
    vsp.SetVSP3FileName( os.path.abspath( "subtab_cs.vsp3" ) )
    vsp.AutoGroupVSPAEROControlSurfaces()
    vsp.Update()
    cid = vsp.FindContainer( "VSPAEROSettings", 0 )
    g0 = vsp.FindParm( cid, "Surf_" + cs + "_0_Gain", "ControlSurfaceGroup_0" )
    g1 = vsp.FindParm( cid, "Surf_" + cs + "_1_Gain", "ControlSurfaceGroup_0" )
    check( vsp.GetParmVal( g0 ) == 1.0 and vsp.GetParmVal( g1 ) == 1.0, "automatic grouping gives both copies gain 1" )
    vsp.SetParmVal( vsp.FindParm( cid, "DeflectionAngle", "ControlSurfaceGroup_0" ), 10.0 )
    res = {}
    for gains in ( ( 1, 1 ), ( 1, -1 ) ):
        vsp.SetParmVal( g0, gains[0] )
        vsp.SetParmVal( g1, gains[1] )
        vsp.Update()
        vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
        vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_NONE ] )
        vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ vsp.SET_ALL ] )
        vsp.ExecAnalysis( "VSPAEROComputeGeometry" )
        an = "VSPAEROSweep"
        vsp.SetAnalysisInputDefaults( an )
        vsp.SetIntAnalysisInput( an, "GeomSet", [ vsp.SET_NONE ] )
        vsp.SetIntAnalysisInput( an, "ThinGeomSet", [ vsp.SET_ALL ] )
        vsp.SetDoubleAnalysisInput( an, "AlphaStart", [ 0.0 ] )
        vsp.SetIntAnalysisInput( an, "AlphaNpts", [ 1 ] )
        vsp.SetDoubleAnalysisInput( an, "MachStart", [ 0.0 ] )
        vsp.SetIntAnalysisInput( an, "MachNpts", [ 1 ] )
        vsp.SetIntAnalysisInput( an, "WakeNumIter", [ 3 ] )
        vsp.ExecAnalysis( an )
        pol = vsp.FindLatestResultsID( "VSPAERO_Polar" )
        res[ gains ] = ( vsp.GetDoubleResults( pol, "CLtot" )[0], vsp.GetDoubleResults( pol, "CMxtot" )[0] )
    a, b = res[ ( 1, 1 ) ], res[ ( 1, -1 ) ]
    check( abs( a[0] ) < 1e-6 and a[1] > 0.05, "VSPAERO gains 1, 1: no lift, a rolling moment (ailerons) %s" % ( a, ) )
    check( b[0] > 0.02 and abs( b[1] ) < 1e-6, "VSPAERO gains 1, -1: lift up, no roll (flaps, trailing edge down) %s" % ( b, ) )


if __name__ == "__main__":
    tmp = tempfile.mkdtemp()
    os.chdir( tmp )
    for fn in ( placement, attach, flip_and_symmetry, xform_more, scale, mass, tess, subsurf, subtab_more ):
        fn()
    shutil.rmtree( tmp, ignore_errors = True )
    print( "%d failed" % len( FAILS ) )
    sys.exit( 1 if FAILS else 0 )
