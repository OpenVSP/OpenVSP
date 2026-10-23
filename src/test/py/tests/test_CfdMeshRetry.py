# CFD mesh surfaces that the main triangulator refuses.
#
# A surface whose constraint segments still cross after they are split cannot be triangulated
# with the segments held, so InitMesh shuffles the point order and tries again, and when that
# fails too it hands the surface to the fallback triangulator.  Each node of the mesh must still
# carry the u,w of the point it is placed at.
#
# The engine below is a closed loop Stack of circles, skinned freely, with an inlet and an
# outlet.  Meshed this coarsely, twelve of its surfaces go through the shuffle to the fallback.
# The .vspgeom file carries every corner's u,w beside its node, so the test evaluates the Geom
# at each corner's u,w and checks it lands on the node.

import os
import tempfile

import openvsp as vsp


def build_engine():
    vsp.VSPRenew()
    gid = vsp.AddGeom( "STACK" )
    # Order policy 1: the last XSec closes the loop back to the first.
    vsp.SetParmVal( gid, "OrderPolicy", "Design", 1 )

    xss = vsp.GetXSecSurf( gid, 0 )
    sections = [ ( 0.0, 1.0 ), ( 1.0, 2.5 ), ( 2.0, 2.5 ), ( 1.0, 1.0 ), ( -4.0, 1.0 ) ]
    assert vsp.GetNumXSec( xss ) == len( sections )
    for i, ( dx, diameter ) in enumerate( sections ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_CIRCLE )
        xs = vsp.GetXSec( xss, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "XDelta" ), dx )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "Circle_Diameter" ), diameter )

        # No skinning constraint is set: angles, slews and strengths all free.
        for p in vsp.GetXSecParmIDs( xs ):
            name = vsp.GetParmName( p )
            if name.endswith( "Set" ) or name.endswith( "LRAngleEq" ):
                vsp.SetParmVal( p, 0 )
    vsp.Update()

    engine = dict( GeomIOType=vsp.ENGINE_GEOM_INLET_OUTLET,
                   GeomInType=vsp.ENGINE_GEOM_FLOWTHROUGH,
                   GeomOutType=vsp.ENGINE_GEOM_TO_LIP,
                   InletModeType=vsp.ENGINE_MODE_FLOWTHROUGH_NEG,
                   InletLipMode=vsp.ENGINE_LOC_INDEX,
                   InletLipIndex=0,
                   InletFaceMode=vsp.ENGINE_LOC_U,
                   InletFaceU=0.95,
                   OutletModeType=vsp.ENGINE_MODE_TO_LIP,
                   OutletLipMode=vsp.ENGINE_LOC_INDEX,
                   OutletLipIndex=3,
                   OutletFaceMode=vsp.ENGINE_LOC_U,
                   OutletFaceU=0.8 )
    for name, val in engine.items():
        vsp.SetParmVal( gid, name, "EngineModel", val )
    vsp.Update()
    return gid


def read_vspgeom( fname ):
    with open( fname ) as f:
        f.readline()
        f.readline()
        nnode, nface = map( int, f.readline().split()[:2] )
        pts = [ vsp.vec3d( *map( float, f.readline().split()[:3] ) ) for _ in range( nnode ) ]
        f.readline()
        faces = [ [ int( i ) - 1 for i in f.readline().split()[1:4] ] for _ in range( nface ) ]
        uws = [ list( map( float, f.readline().split()[2:8] ) ) for _ in range( nface ) ]
    return pts, faces, uws


def test_cfd_mesh_nodes_carry_their_own_uw_after_a_retry():
    gid = build_engine()

    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.5 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.025 )

    with tempfile.TemporaryDirectory() as tmp:
        fname = os.path.join( tmp, "engine.vspgeom" )
        vsp.SetComputationFileName( vsp.CFD_VSPGEOM_TYPE, fname )
        vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_VSPGEOM_TYPE )
        pts, faces, uws = read_vspgeom( fname )

    assert len( faces ) > 1000

    nsurf = vsp.GetTotalNumSurfs( gid )
    off = 0
    for face, uw in zip( faces, uws ):
        for c in range( 3 ):
            p = pts[ face[c] ]
            d = min( ( vsp.CompPnt01( gid, s, uw[ 2 * c ], uw[ 2 * c + 1 ] ) - p ).mag() for s in range( nsurf ) )
            if d > 1e-3:
                off += 1

    assert off == 0
