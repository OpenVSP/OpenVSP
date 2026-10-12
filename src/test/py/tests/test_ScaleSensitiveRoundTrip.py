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


def round_trip( count ):
    tmpdir = tempfile.mkdtemp()
    for i in range( count ):
        fname = os.path.join( tmpdir, "scale%d.vsp3" % i )
        vsp.WriteVSPFile( fname, vsp.SET_ALL )
        vsp.ClearVSPModel()
        vsp.ReadVSPFile( fname )
        vsp.Update()
        yield i


def testGroundPlaneAndEngineExtensionSurviveReadBack():
    # Ahead of the fuselage in the tree, so they update before the rest of the box is known.
    vsp.VSPRenew()
    drop_errors()

    gear = vsp.AddGeom( "GEAR" )
    vsp.CreateAndAddBogie( gear )
    vsp.SetParmVal( gear, "AutoPlaneFlag", "GroundPlane", 1 )

    engine = vsp.AddGeom( "STACK" )
    vsp.SetParmVal( engine, "X_Rel_Location", "XForm", 12.0 )
    vsp.SetParmVal( engine, "GeomIOType", "EngineModel", vsp.ENGINE_GEOM_INLET_OUTLET )
    vsp.SetParmVal( engine, "AutoExtensionFlag", "EngineModel", 1 )

    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fuse, "Length", "Design", 30.0 )
    vsp.Update()

    plane = vsp.GetParmVal( gear, "PlaneSize", "GroundPlane" )
    extension = vsp.GetParmVal( engine, "ExtensionDistance", "EngineModel" )

    # The engine extends by the length of the model, not its own.
    assert extension > 30.0

    for i in round_trip( 3 ):
        gear = vsp.FindGeomsWithName( "GearGeom" )[0]
        engine = vsp.FindGeomsWithName( "StackGeom" )[0]
        assert vsp.GetParmVal( gear, "PlaneSize", "GroundPlane" ) == plane
        assert vsp.GetParmVal( engine, "ExtensionDistance", "EngineModel" ) == extension

    assert_no_errors()


def testNewBogieHasItsTireWhenTheModelGrows():
    # Adding a Geom resizes the model; the gear's scale-only update must still build a new bogie.
    vsp.VSPRenew()
    drop_errors()

    gear = vsp.AddGeom( "GEAR" )
    vsp.CreateAndAddBogie( gear )

    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fuse, "Length", "Design", 30.0 )
    vsp.Update()

    # The tire is a surface of revolution about the axle, not a point at the origin.
    p = vsp.CompPnt01( gear, vsp.GetNumMainSurfs( gear ) - 1, 0.5, 0.25 )
    q = vsp.CompPnt01( gear, vsp.GetNumMainSurfs( gear ) - 1, 0.5, 0.75 )
    assert ( p - q ).mag() > 0.1

    assert_no_errors()
