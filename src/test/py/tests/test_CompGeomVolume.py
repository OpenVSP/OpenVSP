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
