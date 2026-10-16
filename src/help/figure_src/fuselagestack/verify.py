"""Checks the statements of the body help pages against the openvsp module:
FuselageStack.md, EngineModel.md, Chevron.md, Pod.md, Ellipsoid.md, BOR.md, Cobra.md and Conformal.md, and the
Negative Volume advice of GenTab.md.

    <build>/venv/bin/python verify.py

Each check prints what it measured; a failed check raises.  Rerun after any change to the Geoms these pages describe.
"""

import os
import tempfile

import numpy as np

import bodyfig as bf
from bodyfig import vsp


def P( g, u, w, s = 0 ):
    return np.array( list( vsp.CompPnt01( g, s, u, w ) ) )


def near( a, b, tol, what ):
    assert abs( a - b ) <= tol, "%s: %g, expected %g" % ( what, a, b )
    print( "ok  %-60s %.6g" % ( what, a ) )


def check( cond, what ):
    assert cond, what
    print( "ok  %s" % what )


def fuselage_stack():
    bf.renew()
    f = vsp.AddGeom( "FUSELAGE" )
    s = vsp.AddGeom( "STACK" )
    vsp.Update()
    near( P( f, 1.0, 0.0 )[0], 30.0, 1e-9, "Fuselage default length" )
    near( P( s, 1.0, 0.0 )[0], 4.5, 1e-9, "Stack default length" )
    near( P( f, 0.5, 0.0 )[1], 1.25, 1e-9, "Fuselage default half-width" )
    near( P( f, 0.5, 0.75 )[2], 1.5, 1e-9, "Fuselage default half-height (W 0.75 is the top)" )
    near( P( s, 0.5, 0.0 )[1], 1.5, 1e-9, "Stack default half-width (W 0 is +Y)" )
    # Rotations: Fuselage alone, Stack carried on.
    bf.setx( f, 2, "YRotate", 30.0 )
    bf.setx( s, 2, "YRotate", 30.0 )
    vsp.Update()
    check( P( f, 0.5, 0.75 )[0] > 15.0, "Fuselage Rot Y + tilts the section's top aft" )
    near( P( f, 0.75, 0.0 )[0], 22.5, 1e-9, "Fuselage Rot Y leaves the next section in place" )
    near( P( s, 0.75, 0.0 )[2], -0.5, 1e-9, "Stack Rot Y + turns later sections down" )
    bf.setx( f, 2, "YRotate", 0.0 )
    bf.setx( s, 2, "YRotate", 0.0 )
    bf.setx( f, 2, "XRotate", 30.0 )
    bf.setx( s, 2, "ZRotate", 30.0 )
    bf.setx( f, 3, "ZRotate", 30.0 )
    vsp.Update()
    check( P( f, 0.5, 0.0 )[2] > 0.0, "Fuselage Rot X + raises the right side" )
    check( P( f, 0.75, 0.0 )[0] < 22.5, "Fuselage Rot Z + moves the right side forward" )
    check( P( s, 0.75, 0.0 )[1] > 1.5, "Stack Rot Z + turns later sections toward +Y" )
    # Length scales Y and Z offsets.
    bf.renew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    bf.setx( f, 2, "ZLocPercent", 0.1 )
    vsp.SetParmVal( f, "Length", "Design", 60.0 )
    vsp.Update()
    near( P( f, 0.5, 0.75 )[2], 7.5, 1e-9, "Z fraction scales with Length (0.1 x 60 + 1.5)" )
    # V Align 1 puts the top on the line.
    bf.renew()
    s = vsp.AddGeom( "STACK" )
    vsp.Update()
    bf.setx( s, 2, "VAlign", 1.0 )
    vsp.Update()
    near( P( s, 0.5, 0.75 )[2], 0.0, 1e-9, "V Align 1: the top on the Stack's line" )
    near( P( s, 0.75, 0.75 )[2], 1.25, 1e-9, "V Align moves its section alone: the next is placed as at V Align 0" )
    # Monotonic clamps an out-of-order section on its neighbor.
    bf.renew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    vsp.SetParmVal( f, "OrderPolicy", "Design", vsp.FUSE_FREE )
    vsp.Update()
    bf.setx( f, 2, "XLocPercent", 0.9 )
    vsp.Update()
    vsp.SetParmVal( f, "OrderPolicy", "Design", vsp.FUSE_MONOTONIC )
    vsp.Update()
    near( bf.getx( f, 2, "XLocPercent" ), 0.75, 1e-12, "MONOTONIC pulls an out-of-order section onto its neighbor" )
    vsp.SetParmVal( f, "OrderPolicy", "Design", vsp.FUSE_LOOP )
    vsp.Update()
    near( P( f, 0.0, 0.0 )[0], 30.0, 1e-9, "Fuselage LOOP: first section at X 1" )
    # Insert in a Stack moves later sections on.
    bf.renew()
    s = vsp.AddGeom( "STACK" )
    vsp.Update()
    vsp.InsertXSec( s, 1, vsp.XS_ELLIPSE )
    vsp.Update()
    near( bf.getx( s, 5, "XAbs" ), 5.5, 1e-9, "Stack Insert: the tail moves on by the copied step" )
    # Cut in a Stack: the later sections close up, and the surface with them.
    bf.renew()
    s = vsp.AddGeom( "STACK" )
    vsp.Update()
    vsp.CutXSec( s, 2 )
    vsp.Update()
    near( bf.getx( s, 2, "XAbs" ), 2.0, 1e-9, "Stack Cut: the next section closes up" )
    near( P( s, 2.0 / 3.0, 0.0 )[0], 2.0, 1e-9, "Stack Cut: the surface follows" )
    # Insert after the last section (FREE): the new section takes the last one's step, and the Stack grows by it.
    bf.renew()
    s = vsp.AddGeom( "STACK" )
    vsp.Update()
    vsp.InsertXSec( s, 4, vsp.XS_ELLIPSE )
    vsp.Update()
    near( bf.getx( s, 5, "XDelta" ), 0.5, 1e-12, "Stack Insert after the last: the new section takes the last step" )
    near( max( P( s, u, 0.25 )[0] for u in np.linspace( 0.0, 1.0, 101 ) ), 5.0, 1e-9,
          "Stack Insert after the last: the Stack grows by that step" )
    # A Round end cap reaches out by the local half-height: a hemisphere on a circle, half the height on an ellipse.
    for W, H in ( ( 2.0, 2.0 ), ( 3.0, 1.0 ), ( 1.0, 2.0 ) ):
        bf.renew()
        s = vsp.AddGeom( "STACK" )
        vsp.Update()
        vsp.InitStackPreset( s, vsp.STACK_PRESET_CYLINDER_ENDCAPS )
        vsp.Update()
        xs = vsp.GetXSecSurf( s, 0 )
        for i in ( 0, 1 ):
            vsp.ChangeXSecShape( xs, i, vsp.XS_ELLIPSE )
            vsp.Update()
            c = vsp.GetXSec( xs, i )
            vsp.SetParmVal( vsp.GetXSecParm( c, "Ellipse_Width" ), W )
            vsp.SetParmVal( vsp.GetXSecParm( c, "Ellipse_Height" ), H )
        vsp.SetParmVal( s, "CapUMaxOption", "EndCap", vsp.ROUND_END_CAP )
        vsp.Update()
        x0 = P( s, 2.0 / 3.0, 0.0 )[0]
        tip = np.array( [ P( s, 1.0, w ) for w in np.linspace( 0.0, 0.5, 101 ) ] )
        hh = 0.5 * H * np.sqrt( np.clip( 1.0 - ( 2.0 * tip[:, 1] / W ) ** 2, 0.0, 1.0 ) )
        near( np.abs( tip[:, 0] - x0 - hh ).max(), 0.0, 2e-3,
              "Round end cap %g x %g reaches out by the local half-height" % ( W, H ) )
    # Clustering below 1 bunches points toward that end.
    bf.renew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    bf.setx( f, 2, "SectTess_U", 11 )
    bf.setx( f, 2, "FwdCluster", 0.2 )
    vsp.Update()
    u = np.array( vsp.GetUWTess01( f, 0 )[0] ) * 4.0
    d = np.diff( u[ ( u >= 1 - 1e-9 ) & ( u <= 2 + 1e-9 ) ] )
    check( len( d ) == 10 and d[0] < d[5], "Num U is the points of the span ending at the section; Fwd Cluster < 1 bunches forward" )


def engine():
    def run( preset, inm = None, outm = None ):
        bf.renew()
        s = vsp.AddGeom( "STACK" )
        vsp.Update()
        vsp.InitStackPreset( s, getattr( vsp, "STACK_PRESET_" + preset ) )
        vsp.SetParmVal( s, "ExtensionDistance", "EngineModel", 1.0 )
        if inm:
            vsp.SetParmVal( s, "InletModeType", "EngineModel", getattr( vsp, "ENGINE_MODE_" + inm ) )
        if outm:
            vsp.SetParmVal( s, "OutletModeType", "EngineModel", getattr( vsp, "ENGINE_MODE_" + outm ) )
        vsp.Update()
        return [ "-" if vsp.GetGeomVSPSurfCfdType( s, i ) == vsp.CFD_NEGATIVE else "+" for i in range( vsp.GetNumMainSurfs( s ) ) ]
    table = [ ( "FLOWTHRU_MID_ORIG", "FLOWTHROUGH", None, "+" ), ( "FLOWTHRU_MID_ORIG", "FLOWTHROUGH_NEG", None, "+-" ),
              ( "FLOWTHRU_MID_ORIG", "FLOWTHROUGH_NEG_ONLY", None, "-" ), ( "FLOWTHRU_MID_ORIG", "TO_FACE_NEG", None, "+-" ),
              ( "FLOWTHRU_MID_ORIG", "TO_FACE_NEG_ONLY", None, "-" ), ( "FLOWPATH_IN", "TO_LIP", None, "+" ),
              ( "FLOWPATH_IN", "TO_FACE_NEG", None, "-" ), ( "FLOWPATH_IN", "TO_FACE_NEG_ONLY", None, "" ),
              ( "FLOWPATH_OUT", None, "TO_FACE_NEG", "-" ), ( "FLOWPATH_BOTH", "FLOWTHROUGH_NEG", None, "-" ),
              ( "FLOWPATH_BOTH", "FLOWTHROUGH_NEG_ONLY", None, "-" ), ( "FLOWPATH_BOTH", "TO_FACE_NEG", None, "-+" ),
              ( "FLOWPATH_IN", None, None, "+" ), ( "FLOWPATH_OUT", None, None, "+" ), ( "FLOWPATH_BOTH", None, None, "+" ),
              ( "FLOWTHRU_MID_ORIG", None, None, "+" ),
              ( "INFACE", "FLOWTHROUGH_NEG_ONLY", None, "+" ) ]
    for pre, inm, outm, want in table:
        got = "".join( sorted( run( pre, inm, outm ) ) )
        check( got == "".join( sorted( want ) ), "%s %s %s: surfaces %s" % ( pre, inm, outm, want or "none" ) )
    # Chevrons: each side's amplitude drives that side; Angle tilts out; Slew leans as stated.
    def tube():
        bf.renew()
        s = vsp.AddGeom( "STACK" )
        vsp.Update()
        vsp.InitStackPreset( s, vsp.STACK_PRESET_CYLINDER_ENDCAPS )
        vsp.SetParmVal( s, "CapUMaxOption", "EndCap", vsp.NO_END_CAP )
        vsp.Update()
        return s
    def peak( s ):
        w = max( ( vsp.CompPnt01( s, 0, 1.0, a / 400.0 ).x(), a / 400.0 ) for a in range( 401 ) )[1]
        return P( s, 1.0, w )
    for side, ws, we, wp in ( ( "Right", 0.875, 0.125, 0.0 ), ( "Top", 0.625, 0.875, 0.75 ) ):
        s = tube()
        for k, v in ( ( "Chevron_Type", vsp.CHEVRON_PARTIAL ), ( "Number", 1 ), ( "AllSym", 0 ), ( "TBSym", 0 ),
                      ( "RLSym", 0 ), ( "W01_Start", ws ), ( "W01_End", we ) ):
            bf.setx( s, 1, k, v, "Chevron" )
        for k in ( "TopAmplitude", "BottomAmplitude", "LeftAmplitude", "RightAmplitude" ):
            bf.setx( s, 1, k, 0.0, "Chevron" )
        bf.setx( s, 1, side + "Amplitude", 0.5, "Chevron" )
        bf.setx( s, 1, side + "_Slew", 30.0, "Chevron" )
        vsp.Update()
        p = peak( s )
        near( p[0], 3.0 + 0.5 * np.cos( np.radians( 30 ) ), 1e-6, side + " tooth: amplitude along +X" )
        if side == "Right":
            check( p[2] < -0.2, "positive Right Slew leans the right tooth toward -Z" )
        else:
            check( p[1] > 0.2, "positive Top Slew leans the top tooth toward +Y" )


def pod_ellipsoid():
    bf.renew()
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    G = bf.grid( p, 0, nu = 401, w = np.linspace( 0, 1, 9 ) )
    near( np.hypot( G[ ..., 1 ], G[ ..., 2 ] ).max(), 10.0 / 15.0, 1e-9, "Pod greatest radius is L / FineRatio" )
    bf.renew()
    e = vsp.AddGeom( "ELLIPSOID" )
    vsp.Update()
    for k, v in ( ( "A_Radius", 3.0 ), ( "B_Radius", 2.0 ), ( "C_Radius", 1.0 ) ):
        vsp.SetParmVal( e, k, "Design", v )
    vsp.Update()
    G = bf.grid( e, 0, nu = 201, nw = 201 )
    f = np.sqrt( ( ( G[ ..., 0 ] - 3 ) / 3 ) ** 2 + ( G[ ..., 1 ] / 2 ) ** 2 + G[ ..., 2 ] ** 2 )
    near( G[ ..., 0 ].min(), 0.0, 1e-9, "Ellipsoid nose at the origin" )
    check( np.abs( f - 1 ).max() < 4e-4, "Ellipsoid within 0.04 percent: %.2e" % np.abs( f - 1 ).max() )


def bor():
    def make( mode = vsp.BOR_FLOWTHROUGH, angle = 0.0 ):
        bf.renew()
        b = vsp.AddGeom( "BODYOFREVOLUTION" )
        vsp.Update()
        vsp.ChangeBORXSecShape( b, vsp.XS_FOUR_SERIES )
        vsp.Update()
        ids = { vsp.GetParmName( q ): q for q in vsp.GetGeomParmIDs( b ) if vsp.GetParmGroupName( q ) == "XSecCurve" }
        tc = vsp.GetParmVal( ids[ "ThickChord" ] )
        vsp.SetParmVal( ids[ "ThickChord" ], 0.12 )
        vsp.SetParmVal( ids[ "Camber" ], 0.04 )
        vsp.SetParmVal( b, "Mode", "Design", mode )
        vsp.SetParmVal( b, "Angle", "Design", angle )
        vsp.Update()
        return b, tc
    b, tc = make()
    near( tc, 1.0, 1e-12, "BOR: an airfoil chosen for the circle starts at T/C 1" )
    near( vsp.GetParmVal( b, "A0", "Design" ), np.pi, 1e-9, "BOR A0 = pi (D/2)^2" )
    check( P( b, 0.0, 0.75 )[1] > 1.0, "BOR Flowthrough: upper surface outward" )
    b, tc = make( angle = 10.0 )
    check( np.hypot( *P( b, 0.0, 0.0 )[ 1: ] ) < 0.9, "BOR Angle +: trailing edge toward the axis" )
    b, tc = make( angle = -5.0 )
    a = { n: vsp.GetParmVal( b, n, "Design" ) for n in ( "A0", "Ae", "Amin", "AminW" ) }
    check( a[ "Ae" ] > a[ "A0" ] > a[ "Amin" ] and 0.0 < a[ "AminW" ] < 0.5,
           "BOR Angle -5: Ae > A0 > Amin, the throat on the inner surface (W %.3f)" % a[ "AminW" ] )
    # The NACA 4412 of the BOR figures: Amin reaches the trailing edge between Angle 1.75 and 2.
    def amin_ae( angle ):
        b, tc = make( angle = angle )
        ids = { vsp.GetParmName( q ): q for q in vsp.GetGeomParmIDs( b ) if vsp.GetParmGroupName( q ) == "XSecCurve" }
        vsp.SetParmVal( ids[ "CamberLoc" ], 0.4 )
        vsp.Update()
        return vsp.GetParmVal( b, "Amin", "Design" ) - vsp.GetParmVal( b, "Ae", "Design" )
    check( amin_ae( 1.75 ) < -0.01, "BOR 4412 Angle 1.75: Amin < Ae" )
    near( amin_ae( 2.0 ), 0.0, 1e-12, "BOR 4412 Angle 2: the trailing edge is the throat, Amin = Ae" )
    b, tc = make( mode = vsp.BOR_UPPER )
    G = bf.grid( b, 0, nu = 41, nw = 81 )
    check( np.hypot( G[ ..., 1 ], G[ ..., 2 ] ).max() < 0.11, "BOR Upper: radius is the upper surface (4412: under 0.11)" )


def cobra():
    bf.renew()
    c = vsp.AddGeom( "COBRA" )
    vsp.Update()
    G = bf.grid( c, 0, nu = 81, nw = 33 )
    near( G[ ..., 0 ].max(), 2.0, 1e-9, "Cobra default: nose 1 plus aft 1" )
    near( np.hypot( G[ ..., 1 ], G[ ..., 2 ] ).max(), 1.0, 1e-4, "Cobra default radius 1" )


def conformal():
    bf.renew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    c = vsp.AddGeom( "CONFORMAL", f )
    vsp.Update()
    G = bf.grid( c, 0, nu = 61, nw = 33 )
    d = np.array( [ vsp.ProjPnt01( f, 0, vsp.vec3d( *p ) )[0] for p in G[ 5:-5 ].reshape( -1, 3 )[ ::5 ] ] )
    check( 0.09 < d.min() and d.max() < 0.105, "Conformal offset 0.1 lies %.3f to %.3f from the skin" % ( d.min(), d.max() ) )
    vsp.SetParmVal( c, "Side4TrimFlag", "Design", 1 )
    vsp.SetParmVal( c, "Side4Trim", "Design", 0.3 )
    vsp.Update()
    near( P( c, 0.5, 0.7 )[2], P( c, 0.5, 0.8 )[2], 1e-9, "Side trim leaves a flat face" )
    bf.renew()
    wg = vsp.AddGeom( "WING" )
    vsp.Update()
    c = vsp.AddGeom( "CONFORMAL", wg )
    vsp.Update()
    for k, v in ( ( "ChordTrimFlag", 1 ), ( "ChordTrimMin", 0.2 ), ( "ChordTrimMax", 0.6 ) ):
        vsp.SetParmVal( c, k, "Design", v )
    vsp.Update()
    ring = bf.grid( c, 0, u = np.array( [ 0.0 ] ), w = np.linspace( 0, 1, 121 ) )[0]
    near( ( ring[ :, 0 ].min() - 0.0 ) / 4.0, 0.41, 0.01, "Chord trim 0.2-0.6 keeps x/c from 0.41 at the root" )


def building():
    """FuselageStack.md, Building a Body: U counts sections, so inserting a section moves what is placed by U."""
    bf.renew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    bf.setx( f, 1, "XLocPercent", 0.1 )
    vsp.Update()
    near( P( f, 0.25, 0.0 )[0], 3.0, 1e-9, "U 0.25 is section 1, wherever it lies (X 0.1 of 30)" )
    bf.setx( f, 1, "XLocPercent", 0.25 )
    vsp.Update()
    nu0 = len( vsp.GetUWTess01( f, 0 )[0] )
    cu = vsp.AddGeom( "POD", f )
    cl = vsp.AddGeom( "POD", f )
    vsp.SetParmVal( cu, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( cu, "U_Attach_Location", "Attach", 0.6 )
    vsp.SetParmVal( cu, "V_Attach_Location", "Attach", 0.75 )
    vsp.SetParmVal( cl, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_LMN )
    vsp.SetParmVal( cl, "L_Attach_Location", "Attach", 0.6 )
    vsp.Update()
    vsp.Update()
    near( P( cu, 0.0, 0.0 )[0], 18.0, 1e-6, "Child at U 0.6 of the default Fuselage" )
    near( P( cl, 0.0, 0.0 )[0], 18.0, 1e-3, "Child at L 0.6 of the default Fuselage" )
    vsp.InsertXSec( f, 1, vsp.XS_ELLIPSE )
    vsp.Update()
    vsp.Update()
    near( P( cu, 0.0, 0.0 )[0], 15.0, 1e-6, "Inserting after section 1 moves the U 0.6 child to X 15" )
    near( P( cl, 0.0, 0.0 )[0], 18.0, 1e-2, "... and leaves the L 0.6 child in place" )
    nu1 = len( vsp.GetUWTess01( f, 0 )[0] )
    near( nu1 - nu0, bf.getx( f, 1, "SectTess_U" ) - 1, 1e-12, "Each inserted section adds its span's Num U points" )
    b = vsp.AddGeom( "BODYOFREVOLUTION" )
    check( b != "", "BOR added" )
    check( vsp.FindParm( b, "Diameter", "Design" ) != "", "BOR found its own Diameter" )
    vsp.Update()
    check( vsp.FindParm( b, "GeomIOType", "EngineModel" ) == "", "BOR.md: a BOR has no engine model" )


def flush_cutter():
    """GenTab.md, Negative Volume: a cutter ending flush on a face cuts wrongly; carried past it, it cuts fully."""
    removed = {}
    for dx in ( 0.0, 0.01 ):
        bf.renew()
        a = vsp.AddGeom( "STACK" )
        b = vsp.AddGeom( "STACK" )
        for g in ( a, b ):
            vsp.InitStackPreset( g, vsp.STACK_PRESET_CYLINDER_ENDCAPS )
        vsp.SetParmVal( b, "Scale", "XForm", 0.5 )
        vsp.SetParmVal( b, "Negative_Volume_Flag", "Negative_Volume_Props", 1 )
        vsp.Update()
        xa1 = P( a, 1.0, 0.0 )[0]
        xb0, xb1 = P( b, 0.0, 0.0 )[0], P( b, 1.0, 0.0 )[0]
        vsp.SetParmVal( b, "X_Rel_Location", "XForm", xa1 - ( xb1 - xb0 ) - xb0 + dx )
        vsp.Update()
        # CompGeom writes its report files into the working directory; keep them out of the source tree.
        here = os.getcwd()
        with tempfile.TemporaryDirectory() as tmp:
            os.chdir( tmp )
            try:
                rid = vsp.ExecAnalysis( "CompGeom" )
            finally:
                os.chdir( here )
        theo = vsp.GetDoubleResults( rid, "Theo_Vol" )
        wet = vsp.GetDoubleResults( rid, "Wet_Vol" )
        # The part of the cutter inside the cylinder is the whole cutter less the dx that sticks out.
        inside = -theo[1] * ( 1.0 - dx / ( xb1 - xb0 ) )
        removed[ dx ] = ( theo[0] - sum( wet ) ) / inside
    near( removed[ 0.01 ], 1.0, 1e-3, "A cutter carried past the end face removes all its volume inside" )
    near( removed[ 0.0 ], 1.0 / 3.0, 0.02, "A cutter flush with the end face cuts wrongly" )


if __name__ == "__main__":
    fuselage_stack()
    engine()
    pod_ellipsoid()
    bor()
    cobra()
    conformal()
    building()
    flush_cutter()
    print( "all checks pass" )
