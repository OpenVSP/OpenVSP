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


def testAllTheVolumeIsSharedOut():
    # A small pod mostly inside a large one: the large pod's share would pass its own volume, so it
    # is held there and the rest goes to the small pod.  All of the trimmed volume is shared out.
    def build():
        big = vsp.AddGeom( "POD" )
        vsp.SetParmVal( big, "Length", "Design", 20.0 )
        vsp.SetParmVal( big, "FineRatio", "Design", 3.0 )
        small = vsp.AddGeom( "POD" )
        vsp.SetParmVal( small, "X_Rel_Location", "XForm", -3.0 )
        vsp.SetParmVal( small, "Length", "Design", 4.0 )
        vsp.SetParmVal( small, "FineRatio", "Design", 2.0 )

    theo, wet, total_theo, total_wet = comp_geom_volumes( build )

    assert abs( sum( wet ) - total_wet ) < 1e-12 * total_wet, ( wet, total_wet )
    assert wet[0] == theo[0], ( wet, theo )
    assert 0.0 <= wet[1] <= theo[1], ( wet, theo )

    assert_no_errors()


def testSharingWithOverlappingPositiveAndNegativeComponents():
    # A large pod, three small pods overlapping its nose and a negative pod across its tail.  The
    # large pod's share would pass its own volume, so it is held there, and the rest is shared
    # among the small pods and the negative one.  Every share stays between zero and its own
    # theoretical volume, a negative one below zero, and all of the trimmed volume is shared out.
    def pod( x, z, length, fine, negative = False ):
        p = vsp.AddGeom( "POD" )
        vsp.SetParmVal( p, "X_Rel_Location", "XForm", x )
        vsp.SetParmVal( p, "Z_Rel_Location", "XForm", z )
        vsp.SetParmVal( p, "Length", "Design", length )
        vsp.SetParmVal( p, "FineRatio", "Design", fine )
        if negative:
            vsp.SetParmVal( p, "Negative_Volume_Flag", "Negative_Volume_Props", 1 )

    def build():
        pod( 0.0, 0.0, 20.0, 3.0 )
        pod( -2.61, 0.21, 3.87, 2.92 )
        pod( -3.89, -0.14, 3.6, 2.69 )
        pod( -1.5, 0.2, 4.53, 2.9 )
        pod( 17.69, 0.43, 3.16, 2.64, True )

    theo, wet, total_theo, total_wet = comp_geom_volumes( build )

    assert abs( sum( wet ) - total_wet ) < 1e-12 * total_wet, ( wet, total_wet )
    assert wet[0] == theo[0], ( wet, theo )
    for i in [ 1, 2, 3 ]:
        assert 0.0 < wet[i] < theo[i], ( i, wet, theo )
    assert theo[4] < wet[4] < 0.0, ( wet, theo )

    assert_no_errors()


def testANegativeRemainderGoesToTheNegativeComponent():
    # A negative pod poking out of a large pod's nose.  Holding the large pod at its own volume
    # leaves less than nothing to share, which belongs to the negative pod.
    def build():
        big = vsp.AddGeom( "POD" )
        vsp.SetParmVal( big, "Length", "Design", 20.0 )
        vsp.SetParmVal( big, "FineRatio", "Design", 3.0 )
        neg = vsp.AddGeom( "POD" )
        vsp.SetParmVal( neg, "X_Rel_Location", "XForm", -3.0 )
        vsp.SetParmVal( neg, "Length", "Design", 4.0 )
        vsp.SetParmVal( neg, "FineRatio", "Design", 2.0 )
        vsp.SetParmVal( neg, "Negative_Volume_Flag", "Negative_Volume_Props", 1 )

    theo, wet, total_theo, total_wet = comp_geom_volumes( build )

    assert abs( sum( wet ) - total_wet ) < 1e-12 * total_wet, ( wet, total_wet )
    assert wet[0] == theo[0], ( wet, theo )
    assert theo[1] < wet[1] < 0.0, ( wet, theo )

    assert_no_errors()
