"""Human components posed by ARAP and FAST on the tet mesh."""

import os
import tempfile

import numpy as np
import openvsp as vsp

SEATED = { "HipFERt": 80.0, "HipFELt": 80.0, "KneeRt": 80.0, "KneeLt": 80.0, "ElbowRt": 60.0, "ElbowLt": 60.0,
           "ShoulderFERt": 30.0, "ShoulderFELt": 30.0 }


def posed_human( method, pose ):
    vsp.VSPRenew()
    hid = vsp.AddGeom( "HUMAN" )
    vsp.SetParmVal( hid, "LenUnit", "Anthropometric", vsp.LEN_MM )
    vsp.SetParmVal( hid, "PoseMethod", "Pose", method )
    vsp.SetParmVal( hid, "RLSym", "Pose", 0 )
    for name, val in pose.items():
        vsp.SetParmVal( hid, name, "Pose", val )
    vsp.Update()
    return hid


def mesh_verts():
    """The mesh's vertices, from an STL export; the MeshGeom the export leaves is deleted."""
    fn = os.path.join( tempfile.mkdtemp(), "human.stl" )
    mid = vsp.ExportFile( fn, vsp.SET_ALL, vsp.EXPORT_STL )
    vsp.DeleteGeom( mid )
    vsp.Update()
    with open( fn ) as f:
        V = [ [ float( x ) for x in line.split()[1:4] ] for line in f if line.strip().startswith( "vertex" ) ]
    return np.unique( np.round( np.array( V ), 6 ), axis = 0 )


def nearest( P, Q ):
    """Each point of P's distance to the nearest point of Q."""
    d = np.empty( len( P ) )
    for i in range( 0, len( P ), 500 ):
        d[i:i + 500] = np.sqrt( ( ( P[i:i + 500, None, :] - Q[None, :, :] ) ** 2 ).sum( 2 ) ).min( 1 )
    return d


def test_a_symmetric_pose_is_posed_symmetrically():
    for method in ( vsp.HUMAN_POSE_FAST, vsp.HUMAN_POSE_ARAP ):
        posed_human( method, SEATED )
        V = mesh_verts()
        # Measured 0.000 mm for both methods.
        assert nearest( V * np.array( [ 1.0, -1.0, 1.0 ] ), V ).max() < 0.01, method


def test_arap_poses_the_body():
    posed_human( vsp.HUMAN_POSE_ARAP, {} )
    R = mesh_verts()
    posed_human( vsp.HUMAN_POSE_ARAP, SEATED )
    V = mesh_verts()
    assert np.isfinite( V ).all()
    # Seated, the body is lower (measured 0.80 of standing) and the knees come forward (-X, 373 mm).
    assert np.ptp( V[:, 2] ) < 0.85 * np.ptp( R[:, 2] )
    assert V[:, 0].min() < R[:, 0].min() - 300.0


def test_switching_methods_and_back_gives_the_same_shape():
    hid = posed_human( vsp.HUMAN_POSE_FAST, SEATED )
    V0 = mesh_verts()
    vsp.SetParmVal( hid, "PoseMethod", "Pose", vsp.HUMAN_POSE_ARAP )
    vsp.Update()
    V1 = mesh_verts()
    vsp.SetParmVal( hid, "PoseMethod", "Pose", vsp.HUMAN_POSE_FAST )
    vsp.Update()
    V2 = mesh_verts()
    assert np.abs( V2 - V0 ).max() < 1e-6
    assert np.abs( V1 - V0 ).max() > 1.0
