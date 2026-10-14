"""A propeller's BEM export: each section's shape belongs to the radius it is listed at."""

import os
import tempfile

import openvsp as vsp


def bem_results( pid ):
    vsp.SetBEMPropID( pid )
    vsp.ExportFile( os.path.join( tempfile.mkdtemp(), "prop.bem" ), vsp.SET_ALL, vsp.EXPORT_BEM )
    return vsp.FindLatestResultsID( "PropBEM" )


def test_sections_match_their_radii():
    # The default blade thins quickly from the root, so a section taken at the wrong radius shows in its t/c.
    for root, tip in ( ( 1.0, 1.0 ), ( 3.0, 0.5 ), ( 0.4, 2.0 ) ):
        vsp.VSPRenew()
        pid = vsp.AddGeom( "PROP" )
        vsp.SetParmVal( pid, "InCluster", "Design", root )
        vsp.SetParmVal( pid, "OutCluster", "Design", tip )
        vsp.Update()
        rid = bem_results( pid )
        for i, toc in enumerate( vsp.GetDoubleResults( rid, "Thick" ) ):
            xs = vsp.GetDoubleResults( rid, "XSection_%03d" % i )
            ys = vsp.GetDoubleResults( rid, "YSection_%03d" % i )
            assert abs( ( max( ys ) - min( ys ) ) / ( max( xs ) - min( xs ) ) - toc ) < 0.01, ( root, tip, i )
