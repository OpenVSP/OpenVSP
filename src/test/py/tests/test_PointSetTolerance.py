# Point set operations with a zero tolerance.
#
# A tolerance of zero is accepted, and with it no two distinct points are the same point, so
# every point is kept.

import openvsp as vsp


def pts():
    return [ vsp.vec3d( 0.0, 0.0, 0.0 ), vsp.vec3d( 1.0, 0.0, 0.0 ), vsp.vec3d( 2.0, 0.0, 0.0 ) ]


def test_unique_pts_zero_tolerance_keeps_every_point():
    assert len( vsp.UniquePts( pts(), 0.0 ) ) == 3


def test_union_pts_zero_tolerance_keeps_both_sets():
    assert len( vsp.UnionPts( pts()[:1], pts()[1:], 0.0 ) ) == 3
