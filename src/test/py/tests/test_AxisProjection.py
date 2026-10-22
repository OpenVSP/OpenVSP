# Projection of a point onto a surface along an axis, where the ray along the axis misses.
#
# A pod lies along X with radius about 0.5, so a ray along Z from y = 50 passes it by.  The API
# documents -1 for the distance and both surface parameters of a miss.

import openvsp as vsp


def make_pod():
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    return pod


def test_axis_proj_miss_returns_minus_one():
    pod = make_pod()
    miss = vsp.vec3d( 2.0, 50.0, 5.0 )

    d, u, w = vsp.AxisProjPnt01( pod, 0, vsp.Z_DIR, miss )
    assert ( d, u, w ) == ( -1.0, -1.0, -1.0 )

    d, u, w = vsp.AxisProjPnt01Guess( pod, 0, vsp.Z_DIR, miss, 0.5, 0.5 )
    assert ( d, u, w ) == ( -1.0, -1.0, -1.0 )

    us, ws, ds = vsp.AxisProjVecPnt01( pod, 0, vsp.Z_DIR, [ miss ] )
    assert ( ds[0], us[0], ws[0] ) == ( -1.0, -1.0, -1.0 )


def test_axis_proj_hit_is_on_surface():
    pod = make_pod()
    hit = vsp.vec3d( 2.0, 0.0, 5.0 )

    d, u, w = vsp.AxisProjPnt01( pod, 0, vsp.Z_DIR, hit )
    assert d > 0.0
    assert 0.0 <= u <= 1.0 and 0.0 <= w <= 1.0

    p = vsp.CompPnt01( pod, 0, u, w )
    assert abs( p.x() - 2.0 ) < 1e-6 and abs( p.y() ) < 1e-6
    assert abs( 5.0 - p.z() - d ) < 1e-6


def test_pt_cloud_projection_keeps_a_missed_point():
    pod = make_pod()
    hit = vsp.vec3d( 2.0, 0.0, 5.0 )
    miss = vsp.vec3d( 2.0, 50.0, 5.0 )

    cloud = vsp.CreatePtCloudGeomFromPts( [ hit, miss ], "Cloud" )
    vsp.Update()

    d, u, w = vsp.AxisProjPnt01( pod, 0, vsp.Z_DIR, hit )
    on = vsp.CompPnt01( pod, 0, u, w )

    vsp.ProjectPtCloudPts( cloud, pod, 0, vsp.Z_DIR )
    pts = vsp.GetPtCloudPnts( cloud )

    assert vsp.dist( pts[0], on ) < 1e-6
    assert vsp.dist( pts[1], miss ) == 0.0
