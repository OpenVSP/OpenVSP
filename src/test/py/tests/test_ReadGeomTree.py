# Reading a file keeps every Geom in the tree.
#
# Geoms are updated while a file is still being read, and an update dropped any child that had
# not been read yet.  A Wing under an Auxiliary, with a custom Geom earlier in the file, was
# lost from the tree; its Clone then copied an original that was never built, and DegenGeom
# read past the end of the Clone's cap flags.
#
# A file can also name a detached conformal's surface Geom that is built from the conformal,
# which the flip followed around until the stack ran out.
#
# A gear read before the Geom its bogie attaches to read that Geom's surface before it was built.

import openvsp as vsp
import os
import re
import tempfile

from testhelp import ( pop_errors, drop_errors, assert_no_errors )


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


def read_with_conformal_parent( conformal, conformal_parent ):
    # Only the GUI sets a detached conformal's surface Geom, so it is set in the file
    fname = os.path.join( tempfile.mkdtemp(), "conformal.vsp3" )
    vsp.WriteVSPFile( fname, vsp.SET_ALL )
    with open( fname ) as f:
        text = f.read()
    text, n = re.subn( r"(<ID>%s</ID>.*?<ConformalParentID>)[^<]*(</ConformalParentID>)" % conformal,
                       r"\g<1>%s\g<2>" % conformal_parent, text, count=1, flags=re.S )
    assert n == 1
    with open( fname, "w" ) as f:
        f.write( text )
    vsp.VSPRenew()
    drop_errors()
    vsp.ReadVSPFile( fname )
    vsp.Update()


def testAConformalBuiltOnItsOwnCloneIsPutBackOnItsParent():
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    conformal = vsp.AddGeom( "CONFORMAL", pod )
    vsp.SetParmVal( conformal, "DetachFlag", "Design", 1 )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ conformal ] )[0]
    vsp.Update()

    read_with_conformal_parent( conformal, clone )
    assert pop_errors(), "a conformal built on its own Clone raised no error"
    vsp.ComputeDegenGeom( vsp.SET_ALL, vsp.DEGEN_GEOM_CSV_TYPE )
    assert_no_errors()


def testAConformalBuiltOnItsOwnAuxiliaryIsPutBackOnItsParent():
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    conformal = vsp.AddGeom( "CONFORMAL", pod )
    vsp.SetParmVal( conformal, "DetachFlag", "Design", 1 )
    aux = vsp.AddGeom( "AUXILIARY", conformal )
    vsp.Update()

    read_with_conformal_parent( conformal, aux )
    assert pop_errors(), "a conformal built on its own Auxiliary raised no error"
    vsp.ComputeDegenGeom( vsp.SET_ALL, vsp.DEGEN_GEOM_CSV_TYPE )
    assert_no_errors()


def stowed_gear_box( gear_first ):
    vsp.VSPRenew()
    if gear_first:
        gear = vsp.AddGeom( "GEAR" )
        pod = vsp.AddGeom( "POD" )
    else:
        pod = vsp.AddGeom( "POD" )
        gear = vsp.AddGeom( "GEAR" )
    bogie = vsp.CreateAndAddBogie( gear )
    vsp.SetParmVal( pod, "X_Rel_Location", "XForm", 3.0 )
    vsp.SetParmVal( pod, "Z_Rel_Location", "XForm", 2.0 )
    vsp.SetParmVal( vsp.GetParm( bogie, "StowTransAttachFlag", "StowAttach" ), vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( vsp.GetParm( bogie, "StowULoc", "StowAttach" ), 0.3 )
    vsp.SetParmVal( vsp.GetParm( bogie, "StowWLoc", "StowAttach" ), 0.2 )
    vsp.SetParmVal( gear, "GearConfigMode", "Gear", vsp.GEAR_CONFIGURATION_UP )
    vsp.Update()

    # Only the GUI sets a bogie's attach Geom, so it is set in the file, as the GUI saves it
    fname = os.path.join( tempfile.mkdtemp(), "gear.vsp3" )
    vsp.WriteVSPFile( fname, vsp.SET_ALL )
    with open( fname ) as f:
        text = f.read()
    for name in ( "StowParentID", "MechParentID" ):
        text = re.sub( r"<%s\s*/>|<%s>[^<]*</%s>" % ( name, name, name ), "<%s>%s</%s>" % ( name, pod, name ), text )
    text, n = re.subn( r"(<ID>%s</ID>.*?)<Step_Child_List\s*/>" % pod,
                       r"\g<1><Step_Child_List><Step_Child><ID>%s</ID></Step_Child></Step_Child_List>" % gear,
                       text, count=1, flags=re.S )
    assert n == 1
    with open( fname, "w" ) as f:
        f.write( text )

    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    gear = vsp.FindGeomsWithName( "GearGeom" )[0]
    box = []
    for i in range( vsp.GetNumMainSurfs( gear ) ):
        for corner in ( vsp.GetGeomBBoxMin( gear, i, True ), vsp.GetGeomBBoxMax( gear, i, True ) ):
            box += [ corner.x(), corner.y(), corner.z() ]
    return box


def testAGearReadBeforeTheGeomItAttachesToIsPlacedTheSame():
    drop_errors()
    first = stowed_gear_box( True )
    second = stowed_gear_box( False )
    assert len( first ) == len( second )
    for a, b in zip( first, second ):
        assert abs( a - b ) < 1e-9, ( first, second )
    assert_no_errors()
