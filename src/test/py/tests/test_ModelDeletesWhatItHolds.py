# Starting a new model deletes everything the old one held: a Parm of a part of it is no longer valid.

import openvsp as vsp
import pytest


def parms_of( container_id ):
    ids = vsp.FindContainerParmIDs( container_id )
    assert ids, "the container has no Parms to check"
    return ids


def still_valid( parm_ids ):
    return [ p for p in parm_ids if vsp.ValidParm( p ) ]


def testABodyOfRevolutionsCurveIsDeleted():
    vsp.VSPRenew()
    bor = vsp.AddGeom( "BODYOFREVOLUTION" )
    vsp.Update()
    ids = [ p for p in parms_of( bor ) if vsp.GetParmGroupName( p ).startswith( "XSecCurve" ) ]
    assert ids, "no curve Parms listed under the body of revolution"
    vsp.VSPRenew()
    assert still_valid( ids ) == []


def testAnAuxiliaryGeomsCurveIsDeleted():
    vsp.VSPRenew()
    fuse = vsp.AddGeom( "FUSELAGE" )
    aux = vsp.AddGeom( "AUXILIARY", fuse )
    vsp.Update()
    ids = [ p for p in parms_of( aux ) if vsp.GetParmGroupName( p ).startswith( "XSecCurve" ) ]
    assert ids, "no curve Parms listed under the auxiliary Geom"
    vsp.VSPRenew()
    assert still_valid( ids ) == []


@pytest.mark.parametrize( "ss_type", [ vsp.SS_XSEC_CURVE, vsp.SS_INTERSECT ] )
def testACurveSubsurfacesCurveIsDeleted( ss_type ):
    vsp.VSPRenew()
    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    ss = vsp.AddSubSurf( fuse, ss_type )
    vsp.Update()
    ids = [ p for p in parms_of( ss ) if vsp.GetParmGroupName( p ).startswith( "XSecCurve" ) ]
    assert ids, "no curve Parms listed under the subsurface"
    vsp.VSPRenew()
    assert still_valid( ids ) == []


def testAMaterialsLayersAreDeleted():
    vsp.VSPRenew()
    mat = vsp.AddFeaMaterial()
    vsp.SetParmVal( mat, "FeaMaterialType", "FeaMaterial", vsp.FEA_LAMINATE )
    layer = vsp.AddFeaLayer( mat )
    ids = parms_of( layer )
    vsp.VSPRenew()
    assert still_valid( ids ) == []
