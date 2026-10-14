"""A propeller's chord and solidity outputs are not capped: a wide or many-bladed rotor reports its own."""

import math

import openvsp as vsp

TOL = 1e-9


def test_wide_many_bladed_rotor():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( pid, "NumBlade", "Design", 20 )
    # A constant chord of 1.5 R, so every mean chord is 1.5.
    vsp.SetPCurve( pid, vsp.PROP_CHORD, [ 0.2, 1.0 ], [ 1.5, 1.5 ], vsp.LINEAR )
    vsp.Update()
    for name in ( "Chord", "TChord", "PChord" ):
        assert abs( vsp.GetParmVal( pid, name, "Design" ) - 1.5 ) < TOL, name
    for name in ( "Solidity", "TSolidity", "PSolidity" ):
        assert abs( vsp.GetParmVal( pid, name, "Design" ) - 20 * 1.5 / math.pi ) < TOL, name
