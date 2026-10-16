"""Checks, against the openvsp module, the directions and behavior Human.md states.

    <build>/venv/bin/python verify.py

Each joint is set positive on its own, with R/L Sym off, and the part of the body that moved is followed.
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )

import numpy as np

import meshfig as mf
from meshfig import vsp
import human_figs as hf

FAIL = []


def check( ok, what ):
    print( ( "ok    " if ok else "FAIL  " ) + what )
    if not ok:
        FAIL.append( what )


def moved( pose, base = None ):
    """Centroid of the vertices that moved, before and after, and the vertex that moved most."""
    _, G = hf.human( base or {} )
    merged = dict( base or {} )
    merged.update( pose )
    _, T = hf.human( merged )
    G = G.reshape( -1, 3 )
    T = T.reshape( -1, 3 )
    d = np.linalg.norm( T - G, axis = 1 )
    m = d > 0.2 * d.max()
    k = np.argmax( d )
    return G[ m ].mean( 0 ), T[ m ].mean( 0 ), G[ k ], T[ k ]


def main():
    _, T = hf.human()
    P = T.reshape( -1, 3 )
    feet = P[ P[ :, 2 ] < P[ :, 2 ].min() + 0.03 ]
    check( feet[ :, 0 ].min() < -0.1 and feet[ :, 0 ].max() < 0.1, "the toes point along -X: the human faces -X" )
    check( abs( P[ :, 2 ].max() - P[ :, 2 ].min() - 1.756 ) < 0.01, "a 50th percentile man is 1.756 m tall in M" )

    c0, c1, p0, p1 = moved( { "ShoulderABADRt": 30 } )
    check( c0[ 1 ] > 0.2, "the right arm is on the +Y side" )
    check( c1[ 1 ] > c0[ 1 ] and c1[ 2 ] > c0[ 2 ], "Shoulder AB/AD + raises the arm out to the side" )
    c0, c1, p0, p1 = moved( { "ShoulderABADLt": 30 } )
    check( c0[ 1 ] < -0.2 and c1[ 1 ] < c0[ 1 ], "Shoulder AB/AD + on the left raises the left arm out to its side" )
    c0, c1, p0, p1 = moved( { "ShoulderFERt": 30 } )
    check( c1[ 0 ] < c0[ 0 ] and c1[ 2 ] > c0[ 2 ], "Shoulder FE + swings the arm forward and up" )
    c0, c1, p0, p1 = moved( { "ShoulderIERt": 30 }, { "ElbowRt": 90 } )
    check( c1[ 1 ] < c0[ 1 ], "Shoulder IE + swings a bent forearm in across the body" )
    c0, c1, p0, p1 = moved( { "ElbowRt": 90 } )
    check( c1[ 0 ] < c0[ 0 ] and c1[ 2 ] > c0[ 2 ], "Elbow + brings the forearm forward and up" )
    c0, c1, p0, p1 = moved( { "ForearmRt": 30 } )
    check( p1[ 1 ] < p0[ 1 ] and p1[ 0 ] > p0[ 0 ], "Forearm + turns the thumb, at the front, in and back" )
    c0, c1, p0, p1 = moved( { "WristRt": 30 } )
    check( c1[ 1 ] > c0[ 1 ], "Wrist + bends the fingers outward, away from the thigh" )
    c0, c1, p0, p1 = moved( { "HipABADRt": 20 } )
    check( c1[ 1 ] > c0[ 1 ], "Hip AB/AD + swings the leg out to the side" )
    c0, c1, p0, p1 = moved( { "HipFERt": 30 } )
    check( c1[ 0 ] < c0[ 0 ], "Hip FE + swings the leg forward" )
    c0, c1, p0, p1 = moved( { "KneeRt": 30 } )
    check( c1[ 0 ] > c0[ 0 ], "Knee + moves the foot back" )
    c0, c1, p0, p1 = moved( { "AnkleRt": 15 } )
    check( p1[ 2 ] > p0[ 2 ], "Ankle + raises the toes" )
    for j in ( "Back", "Waist" ):
        c0, c1, p0, p1 = moved( { j: 30 } )
        check( c1[ 0 ] < c0[ 0 ] and c1[ 2 ] < c0[ 2 ], j + " + bends the upper body forward" )
        _, G = hf.human()
        _, T = hf.human( { j: 30 } )
        G = G.reshape( -1, 3 )
        T = T.reshape( -1, 3 )
        low = G[ :, 2 ] < -0.5
        check( np.abs( T[ low ] - G[ low ] ).max() < 1e-4, j + " leaves the legs where they were, to 0.1 mm" )
    c0, c1, p0, p1 = moved( { "Nod": 30 } )
    check( c1[ 0 ] < c0[ 0 ] and c1[ 2 ] < c0[ 2 ], "Nod + tips the head forward" )
    c0, c1, p0, p1 = moved( { "RotateHead": 30 } )
    check( p0[ 0 ] < -0.1 and p1[ 1 ] > p0[ 1 ], "Rotate + turns the face toward the person's right, +Y" )

    # Units, scale and the sex
    vsp.VSPRenew()
    h = vsp.AddGeom( "HUMAN" )
    vsp.Update()
    z = vsp.GetParmVal( h, "Z_Len", "BBox" )
    vsp.SetParmVal( h, "Scale", "XForm", 2.0 )
    vsp.Update()
    check( abs( vsp.GetParmVal( h, "Z_Len", "BBox" ) - z ) < 1e-9, "Scale on the XForm tab does not change a human" )
    vsp.SetParmVal( h, "Scale", "XForm", 1.0 )
    vsp.SetParmVal( h, "Stature_pct", "Anthropometric", 0.95 )
    vsp.SetParmVal( h, "GenderFlag", "Anthropometric", vsp.FEMALE )
    vsp.Update()
    check( abs( vsp.GetParmVal( h, "Stature_pct", "Anthropometric" ) - 0.95 ) < 1e-9 and
           abs( vsp.GetParmVal( h, "Stature", "Anthropometric" ) - 1735 / 304.8 ) < 1e-3,
           "switching to female keeps the 95th percentile, 1735 mm" )
    m0 = vsp.GetParmVal( h, "Mass", "Anthropometric" )
    bmi = vsp.GetParmVal( h, "BMI", "Anthropometric" )
    vsp.SetParmVal( h, "Stature_pct", "Anthropometric", 0.5 )
    vsp.Update()
    check( abs( vsp.GetParmVal( h, "BMI", "Anthropometric" ) - bmi ) < 1e-9 and
           vsp.GetParmVal( h, "Mass", "Anthropometric" ) < m0, "a shorter stature keeps the BMI and lowers the mass" )

    # The design eye, through a Super Cone on the human
    vsp.VSPRenew()
    h = vsp.AddGeom( "HUMAN" )
    vsp.SetParmVal( h, "LenUnit", "Anthropometric", vsp.LEN_M )
    vsp.Update()
    a = vsp.AddGeom( "AUXILIARY", h )
    vsp.SetParmVal( a, "AuxiliaryGeomType", "Design", vsp.AUX_GEOM_SUPER_CONE )
    vsp.Update()
    e0 = vsp.CompPnt01( a, 0, 1.0, 0.0 )
    check( e0.x() < -0.05 and abs( e0.y() ) < 1e-9 and e0.z() > 0.5, "the design eye is forward, on the centerline, high" )
    vsp.SetParmVal( h, "Nod", "Pose", 30 )
    vsp.Update()
    e1 = vsp.CompPnt01( a, 0, 1.0, 0.0 )
    vsp.SetParmVal( a, "X_Rel_Location", "XForm", 1e-9 )
    vsp.Update()
    e2 = vsp.CompPnt01( a, 0, 1.0, 0.0 )
    check( e1.z() < e0.z() - 0.01 and abs( e2.z() - e1.z() ) < 1e-9, "a Super Cone follows the eye when the head nods" )

    # The Stature limits follow the sex: a 50th percentile woman after a man, in meters.
    vsp.VSPRenew()
    h = vsp.AddGeom( "HUMAN" )
    vsp.SetParmVal( h, "LenUnit", "Anthropometric", vsp.LEN_M )
    vsp.Update()
    vsp.SetParmVal( h, "GenderFlag", "Anthropometric", vsp.FEMALE )
    vsp.Update()
    check( abs( vsp.GetParmVal( h, "Stature", "Anthropometric" ) - 1.619 ) < 1e-3,
           "switching to female after the unit gives the female 50th percentile, 1619 mm" )

    print( "\n%d failed" % len( FAIL ) )


if __name__ == "__main__":
    main()
