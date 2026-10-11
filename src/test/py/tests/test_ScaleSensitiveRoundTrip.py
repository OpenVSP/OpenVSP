# A Geom sized from the model's bounding box keeps its size when the file is read back.
#
# A rotor burst Auxiliary with an automatic diameter takes it from the model's
# scale-independent bounding box.  The box's lengths are Parms saved in the file; the Geoms
# sized from the box must still be rebuilt once the box is known on reading.

import os
import tempfile

import openvsp as vsp

from testhelp import drop_errors, assert_no_errors


def burst_diameter():
    burst = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Auxiliary" ][0]
    return vsp.GetParmVal( burst, "Diameter", "Design" )


def testRotorBurstDiameterSurvivesReadBack():
    vsp.VSPRenew()
    drop_errors()

    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fuse, "Length", "Design", 30.0 )

    prop = vsp.AddGeom( "PROP", fuse )
    vsp.SetParmVal( prop, "Diameter", "Design", 8.0 )

    burst = vsp.AddGeom( "AUXILIARY", prop )
    vsp.SetParmVal( burst, "AuxiliaryGeomType", "Design", vsp.AUX_GEOM_ROTOR_BURST )
    vsp.SetParmVal( burst, "AutoDiam", "Design", 1 )
    vsp.Update()

    diameter = burst_diameter()

    # 1.5 times the diagonal of the model, not the 30 used before any box exists.
    assert diameter != 30.0

    tmpdir = tempfile.mkdtemp()
    for i in range( 3 ):
        fname = os.path.join( tmpdir, "burst%d.vsp3" % i )
        vsp.WriteVSPFile( fname, vsp.SET_ALL )
        vsp.ClearVSPModel()
        vsp.ReadVSPFile( fname )
        vsp.Update()

        assert burst_diameter() == diameter

    assert_no_errors()
