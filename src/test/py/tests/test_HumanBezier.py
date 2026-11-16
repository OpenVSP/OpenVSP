"""Human components represented by their closed Bezier surfaces."""

import os
import tempfile

import numpy as np
import openvsp as vsp

# The parts, in surface order.
PARTS = [ "torso", "head", "arm_R", "arm_L", "hand_R", "hand_L", "thumb_R", "thumb_L",
          "finger1_R", "finger1_L", "finger2_R", "finger2_L", "finger3_R", "finger3_L", "finger4_R", "finger4_L",
          "leg_R", "leg_L", "foot_R", "foot_L" ]

POSE = { "KneeRt": 90.0, "ElbowRt": 100.0, "HipFERt": 60.0, "ShoulderABADRt": 30.0, "Nod": 15.0,
         "KneeLt": 30.0, "ShoulderFELt": 40.0, "ForearmRt": 30.0, "AnkleLt": 10.0 }


def human( rep = vsp.HUMAN_REP_SURF, sex = vsp.MALE, pose = None, anthro = None ):
    vsp.VSPRenew()
    hid = vsp.AddGeom( "HUMAN" )
    vsp.SetParmVal( hid, "LenUnit", "Anthropometric", vsp.LEN_MM )
    vsp.SetParmVal( hid, "GenderFlag", "Anthropometric", sex )
    vsp.Update()
    if anthro:
        for name, val in anthro:
            vsp.SetParmVal( hid, name, "Anthropometric", val )
            vsp.Update()
    if pose:
        vsp.SetParmVal( hid, "RLSym", "Pose", 0 )
        for name, val in pose.items():
            vsp.SetParmVal( hid, name, "Pose", val )
    vsp.SetParmVal( hid, "Representation", "Design", rep )
    vsp.Update()
    return hid


def pnts( hid, isurf, us, ws ):
    """Surface points on the grid us x ws, as an array."""
    uu, ww = np.meshgrid( us, ws, indexing = "ij" )
    p = vsp.CompVecPnt01( hid, isurf, list( uu.ravel() ), list( ww.ravel() ) )
    return np.array( [ [ q.x(), q.y(), q.z() ] for q in p ] )


def dist_to_surf( hid, isurf, P ):
    _, _, d = vsp.ProjVecPnt01( hid, isurf, [ vsp.vec3d( *p ) for p in P ] )
    return np.array( d )


def mesh_verts( hid ):
    """The vertices of the Human's mesh, from an STL export of its mesh representation."""
    rep = vsp.GetParmVal( hid, "Representation", "Design" )
    vsp.SetParmVal( hid, "Representation", "Design", vsp.HUMAN_REP_MESH )
    vsp.Update()
    fn = os.path.join( tempfile.mkdtemp(), "human.stl" )
    vsp.ExportFile( fn, vsp.SET_ALL, vsp.EXPORT_STL )
    vsp.SetParmVal( hid, "Representation", "Design", rep )
    vsp.Update()
    V = [ [ float( x ) for x in line.split()[1:4] ] for line in open( fn ) if line.strip().startswith( "vertex" ) ]
    return np.unique( np.round( np.array( V ), 6 ), axis = 0 )


def mesh_to_surfs( hid, step = 4 ):
    """Distance of every step-th mesh vertex to the nearest surface, mm."""
    V = mesh_verts( hid )[::step]
    d = np.full( len( V ), np.inf )
    for i in range( vsp.GetNumMainSurfs( hid ) ):
        d = np.minimum( d, dist_to_surf( hid, i, V ) )
    return d


def test_representation_switches():
    hid = human( vsp.HUMAN_REP_MESH )
    assert vsp.GetNumMainSurfs( hid ) == 0
    vol = vsp.GetParmVal( hid, "Volume", "Anthropometric" )

    vsp.SetParmVal( hid, "Representation", "Design", vsp.HUMAN_REP_SURF )
    vsp.Update()
    assert vsp.GetNumMainSurfs( hid ) == len( PARTS )
    # The volume Parm is the mesh's in either representation.
    assert vsp.GetParmVal( hid, "Volume", "Anthropometric" ) == vol

    vsp.SetParmVal( hid, "Representation", "Design", vsp.HUMAN_REP_MESH )
    vsp.Update()
    assert vsp.GetNumMainSurfs( hid ) == 0


def test_representation_is_saved():
    hid = human()
    fn = os.path.join( tempfile.mkdtemp(), "human.vsp3" )
    vsp.WriteVSPFile( fn )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fn )
    vsp.Update()
    hid = vsp.FindGeoms()[0]
    assert vsp.GetParmVal( hid, "Representation", "Design" ) == vsp.HUMAN_REP_SURF
    assert vsp.GetNumMainSurfs( hid ) == len( PARTS )


def test_surfaces_are_closed():
    hid = human( pose = POSE )
    us = np.linspace( 0, 1, 7 )
    ws = np.linspace( 0, 1, 9 )
    for i in range( len( PARTS ) ):
        P = pnts( hid, i, us, ws ).reshape( len( us ), len( ws ), 3 )
        # Closed around, and both ends are single points.
        assert np.abs( P[:, 0] - P[:, -1] ).max() < 1e-9, PARTS[i]
        assert np.ptp( P[0], axis = 0 ).max() < 1e-9, PARTS[i]
        assert np.ptp( P[-1], axis = 0 ).max() < 1e-9, PARTS[i]


def test_surfaces_are_symmetric():
    for sex in ( vsp.MALE, vsp.FEMALE ):
        hid = human( sex = sex )
        us = np.linspace( 0.02, 0.98, 9 )
        ws = np.linspace( 0, 1, 13 )
        mir = np.array( [ 1.0, -1.0, 1.0 ] )
        # Centre parts are their own mirror images; each left part is its right part's.
        for i, name in enumerate( PARTS ):
            if name.endswith( "_R" ):
                continue
            j = PARTS.index( name.replace( "_L", "_R" ) )
            P = pnts( hid, i, us, ws ) * mir
            assert dist_to_surf( hid, j, P ).max() < 1e-6, PARTS[i]


def test_feature_lines_are_symmetric():
    # The W feature lines (0, 0.25, 0.5, 0.75) map onto each other in the mirror: a centre part's
    # w = 0 and 0.5 lie on the symmetry plane, and a left part at w mirrors its right part at -w.
    hid = human()
    us = np.linspace( 0.05, 0.95, 7 )
    ws = np.array( [ 0.0, 0.25, 0.5, 0.75 ] )
    mir = np.array( [ 1.0, -1.0, 1.0 ] )
    for i, name in enumerate( PARTS ):
        if name.endswith( "_R" ):
            continue
        j = PARTS.index( name.replace( "_L", "_R" ) )
        P = pnts( hid, i, us, ws ).reshape( len( us ), len( ws ), 3 ) * mir
        Q = pnts( hid, j, us, np.mod( 1.0 - ws, 1.0 ) ).reshape( len( us ), len( ws ), 3 )
        assert np.abs( P - Q ).max() < 1e-6, name


def test_surfaces_are_linear_in_anthropometry():
    # Where only unmoved bones act -- the head above the neck, at no nod or turn -- the surface is
    # the net Pbar + Q X, so its points at the mean of two bodies are the mean of their points.
    us = np.linspace( 0.5, 1.0, 5 )
    ws = np.linspace( 0, 1, 7 )
    bodies = [ ( ( "Stature", 1650.0 ), ( "BMI", 22.0 ) ), ( ( "Stature", 1850.0 ), ( "BMI", 36.0 ) ),
               ( ( "Stature", 1750.0 ), ( "BMI", 29.0 ) ) ]
    P = [ pnts( human( anthro = b ), 1, us, ws ) for b in bodies ]
    assert np.abs( P[0] - P[1] ).max() > 10.0
    assert np.abs( 0.5 * ( P[0] + P[1] ) - P[2] ).max() < 1e-6


def check_matches_mesh( hid ):
    d = mesh_to_surfs( hid )
    # Measured: median 0.38-0.43, 90th percentile 1.9-2.5, max 10.6-19.5 mm.  At rest the largest
    # are at the joined toes and the sharpest features of the front of the trunk and the face;
    # seated, at the groin, where the mesh passes through itself.
    assert np.median( d ) < 0.8
    assert np.percentile( d, 90 ) < 3.0
    assert d.max() < 22.0


def test_surfaces_match_the_mesh():
    check_matches_mesh( human( sex = vsp.MALE ) )
    check_matches_mesh( human( sex = vsp.FEMALE, anthro = ( ( "Stature_pct", 0.95 ), ( "BMI_pct", 0.95 ) ) ) )


def test_surfaces_pose_with_the_mesh():
    check_matches_mesh( human( pose = POSE ) )
    sit = { "ElbowRt": 80.0, "ShoulderIERt": 45.0, "HipFERt": 80.0, "KneeRt": 80.0,
            "ElbowLt": 80.0, "ShoulderIELt": 45.0, "HipFELt": 80.0, "KneeLt": 80.0 }
    check_matches_mesh( human( sex = vsp.FEMALE, pose = sit ) )


def comp_geom_volume( hid ):
    mid = vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    rid = vsp.FindLatestResultsID( "Comp_Geom" )
    vsp.DeleteGeom( mid )
    vsp.Update()
    return sum( vsp.GetDoubleResults( rid, "Wet_Vol" ) )


def test_comp_geom_and_mass_of_the_surfaces():
    hid = human( vsp.HUMAN_REP_MESH, pose = POSE )
    vmesh = comp_geom_volume( hid )
    vsp.SetParmVal( hid, "Representation", "Design", vsp.HUMAN_REP_SURF )
    vsp.Update()
    # The union of the overlapping parts holds the body's volume.
    assert abs( comp_geom_volume( hid ) / vmesh - 1.0 ) < 0.01

    mid = vsp.ComputeMassProps( vsp.SET_ALL, 20, vsp.X_DIR )
    vsp.DeleteGeom( mid )
    rid = vsp.FindLatestResultsID( "Mass_Properties" )
    assert abs( vsp.GetDoubleResults( rid, "Total_Volume" )[0] / vmesh - 1.0 ) < 0.01


def test_export_of_the_surfaces():
    human()
    d = tempfile.mkdtemp()
    for name, kind in ( ( "h.stl", vsp.EXPORT_STL ), ( "h.stp", vsp.EXPORT_STEP ), ( "h.igs", vsp.EXPORT_IGES ) ):
        fn = os.path.join( d, name )
        vsp.ExportFile( fn, vsp.SET_ALL, kind )
        assert os.path.getsize( fn ) > 1000, name


def test_a_clone_shows_the_surfaces():
    hid = human()
    clone = vsp.CloneGeomVec( [ hid ] )[0]
    vsp.Update()
    assert vsp.GetNumMainSurfs( clone ) == len( PARTS )
    vsp.SetParmVal( hid, "Representation", "Design", vsp.HUMAN_REP_MESH )
    vsp.Update()
    assert vsp.GetNumMainSurfs( clone ) == 0


def cone_bbox( hid ):
    """The bounding box of a super cone hung off the Human's eyes."""
    aux = vsp.AddGeom( "AUXILIARY", hid )
    vsp.SetParmVal( vsp.FindParm( aux, "AuxiliaryGeomType", "Design" ), vsp.AUX_GEOM_SUPER_CONE )
    vsp.Update()
    box = [ vsp.GetParmVal( vsp.FindParm( aux, n, "BBox" ) ) for n in ( "X_Min", "Y_Min", "Z_Min" ) ]
    vsp.DeleteGeom( aux )
    vsp.Update()
    return np.array( box )


def test_the_eyes_stay_on_the_surface_where_the_mesh_has_them():
    for sex, pose in ( ( vsp.MALE, None ), ( vsp.FEMALE, { "Nod": 20.0, "RotateHead": -30.0, "Waist": 15.0 } ) ):
        hid = human( vsp.HUMAN_REP_MESH, sex = sex, pose = pose )
        on_mesh = cone_bbox( hid )
        vsp.SetParmVal( hid, "Representation", "Design", vsp.HUMAN_REP_SURF )
        vsp.Update()
        # The surface is within about two millimetres of the mesh at the eyes; the cone's length
        # follows the shape's bounding box, which the surfaces' toes stretch (measured 3.2 mm).
        assert np.abs( cone_bbox( hid ) - on_mesh ).max() < 4.0
