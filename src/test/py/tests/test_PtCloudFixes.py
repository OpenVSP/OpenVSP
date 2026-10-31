"""Point cloud files: what the reader accepts."""

import openvsp as vsp


def read_cloud( tmp_path, text ):
    fname = str( tmp_path / "cloud.pts" )
    with open( fname, "w" ) as f:
        f.write( text )
    vsp.VSPRenew()
    gid = vsp.ImportFile( fname, vsp.IMPORT_PTS, "" )
    vsp.Update()
    return gid


def test_header_and_comment_lines_are_skipped( tmp_path ):
    gid = read_cloud( tmp_path, "x,y,z\n1,2,3\n# a comment\n4 5 6\n10, 11, 12\n" )
    assert list( vsp.GetGeomBBoxMin( gid ) ) == [ 1.0, 2.0, 3.0 ]
    assert list( vsp.GetGeomBBoxMax( gid ) ) == [ 10.0, 11.0, 12.0 ]


def test_tabs_separate_values( tmp_path ):
    gid = read_cloud( tmp_path, "1\t2\t3\n7\t8\t9\n" )
    assert list( vsp.GetGeomBBoxMin( gid ) ) == [ 1.0, 2.0, 3.0 ]
    assert list( vsp.GetGeomBBoxMax( gid ) ) == [ 7.0, 8.0, 9.0 ]


def test_a_point_that_misses_stays_put( tmp_path ):
    # Projected along Z onto a default pod: x 5 lands on the pod, x 9 is past its nose and misses.
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    fname = str( tmp_path / "cloud.pts" )
    with open( fname, "w" ) as f:
        f.write( "5 0.3 5\n9 0.3 5\n" )
    gid = vsp.ImportFile( fname, vsp.IMPORT_PTS, "" )
    vsp.Update()
    vsp.ProjectPtCloudPts( gid, pod, 0, 2 )
    vsp.Update()
    lo = vsp.GetGeomBBoxMin( gid )
    hi = vsp.GetGeomBBoxMax( gid )
    assert abs( lo.z() - 0.5954 ) < 1e-3
    assert abs( hi.x() - 9.0 ) < 1e-12 and abs( hi.z() - 5.0 ) < 1e-12
