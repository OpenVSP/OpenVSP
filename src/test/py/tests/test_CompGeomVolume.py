# CompGeom shares the trimmed volume out among the components in proportion to each one's
# estimate of its own, so one bad estimate spoils every component's Wet_Vol.

import math
import os
import tempfile

import openvsp as vsp

from testhelp import drop_errors, assert_no_errors


def comp_geom_volumes( build ):
    vsp.VSPRenew()
    drop_errors()
    out = tempfile.mkdtemp()
    vsp.SetComputationFileName( vsp.COMP_GEOM_TXT_TYPE, os.path.join( out, "comp_geom.txt" ) )
    vsp.SetComputationFileName( vsp.COMP_GEOM_CSV_TYPE, os.path.join( out, "comp_geom.csv" ) )

    build()
    vsp.Update()

    rid = vsp.ExecAnalysis( "CompGeom" )
    return ( list( vsp.GetDoubleResults( rid, "Theo_Vol" ) ), list( vsp.GetDoubleResults( rid, "Wet_Vol" ) ),
             vsp.GetDoubleResults( rid, "Total_Theo_Vol" )[0], vsp.GetDoubleResults( rid, "Total_Wet_Vol" )[0] )


def add_disk( x ):
    prop = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( prop, "PropMode", "Design", vsp.PROP_DISK )
    vsp.SetParmVal( prop, "Diameter", "Design", 4.0 )
    vsp.SetParmVal( prop, "X_Rel_Location", "XForm", x )


def testNoVolumeToShareGivesEveryComponentNone():
    theo, wet, total_theo, total_wet = comp_geom_volumes( lambda: add_disk( 0.0 ) )

    assert all( math.isfinite( v ) for v in wet ), wet
    assert wet == [ 0.0 ] * len( wet )
    assert total_wet == 0.0

    assert_no_errors()


def testADiskAddsNoVolume():
    # An open disk's triangles sum to its area times its distance from the origin over three, so
    # where the disk sat used to decide the totals and, through the shares, every component's.
    pod_only = comp_geom_volumes( lambda: vsp.AddGeom( "POD" ) )
    pod_theo = pod_only[0][0]
    pod_wet = pod_only[1][0]

    for x in [ 5.0, 20.0, -20.0, -1.0 ]:
        theo, wet, total_theo, total_wet = comp_geom_volumes( lambda: ( vsp.AddGeom( "POD" ), add_disk( x ) ) )

        assert theo[1] == 0.0 and wet[1] == 0.0, ( x, theo, wet )
        assert abs( theo[0] - pod_theo ) < 1e-9 * pod_theo, ( x, theo )
        assert abs( wet[0] - pod_wet ) < 1e-9 * pod_theo, ( x, wet )
        assert abs( total_theo - pod_theo ) < 1e-9 * pod_theo, ( x, total_theo )
        assert abs( total_wet - pod_wet ) < 1e-9 * pod_theo, ( x, total_wet )

    assert_no_errors()


def testAGroundPlaneAddsNoVolume():
    # A gear with no bogie is only its ground plane, an open transparent sheet like a disk.
    pod_only = comp_geom_volumes( lambda: vsp.AddGeom( "POD" ) )
    pod_theo = pod_only[0][0]

    for z in [ -5.0, -50.0 ]:
        def build():
            vsp.AddGeom( "POD" )
            gear = vsp.AddGeom( "GEAR" )
            vsp.SetParmVal( gear, "Z_Rel_Location", "XForm", z )

        theo, wet, total_theo, total_wet = comp_geom_volumes( build )

        assert theo[1] == 0.0 and wet[1] == 0.0, ( z, theo, wet )
        assert abs( total_theo - pod_theo ) < 1e-9 * pod_theo, ( z, total_theo )

    assert_no_errors()
