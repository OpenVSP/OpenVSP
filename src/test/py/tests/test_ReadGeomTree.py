# Reading a file keeps every Geom in the tree.
#
# Geoms are updated while a file is still being read, and an update dropped any child that had
# not been read yet.  A Wing under an Auxiliary, with a custom Geom earlier in the file, was
# lost from the tree; its Clone then copied an original that was never built, and DegenGeom
# read past the end of the Clone's cap flags.

import openvsp as vsp
import os
import tempfile

from testhelp import ( drop_errors, assert_no_errors )


def testAChildReadAfterItsParentIsKept():
    vsp.VSPRenew()
    drop_errors()
    hinge = vsp.AddGeom( "HINGE" )
    vsp.AddGeom( "Capsule", hinge )
    aux = vsp.AddGeom( "AUXILIARY" )
    wing = vsp.AddGeom( "WING", aux )
    vsp.Update()
    vsp.CloneGeomVec( [ wing ] )
    vsp.Update()

    fname = os.path.join( tempfile.mkdtemp(), "tree.vsp3" )
    vsp.WriteVSPFile( fname, vsp.SET_ALL )
    vsp.ClearVSPModel()
    vsp.ReadVSPFile( fname )
    vsp.Update()

    names = sorted( vsp.GetGeomName( g ) for g in vsp.FindGeoms() )
    assert names == sorted( [ "HingeGeom", "Capsule", "AuxiliaryGeom", "WingGeom", "WingGeom_Clone" ] ), names
    aux = vsp.FindGeomsWithName( "AuxiliaryGeom" )[0]
    # A Clone is made beside its original
    assert sorted( vsp.GetGeomName( c ) for c in vsp.GetGeomChildren( aux ) ) == [ "WingGeom", "WingGeom_Clone" ]

    vsp.ComputeDegenGeom( vsp.SET_ALL, vsp.DEGEN_GEOM_CSV_TYPE )
    assert_no_errors()
