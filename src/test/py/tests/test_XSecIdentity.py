# A cross section, or a curve, that is replaced by one of another shape keeps its identity.
#
# Replacing is how a shape changes: a new object of the new type is made, the old one's values
# are copied across, and the old one is deleted.  Everything that named the old one -- a script
# holding its ID, a link, a design variable, an attribute and its collection -- has to go on
# naming the new one.

import openvsp as vsp
import pytest


def drain():
    em = vsp.ErrorMgrSingleton.getInstance()
    out = []
    while em.GetNumTotalErrors() > 0:
        out.append( em.PopLastError().GetErrorString() )
    return out


def tag( obj, name ):
    coll = vsp.GetChildCollection( obj )
    return ( coll, vsp.AddAttributeString( coll, name, name ) )


def assert_kept( obj, coll, attr, label ):
    assert vsp.GetChildCollection( obj ) == coll, label + ": the collection took a new ID"
    assert list( vsp.FindAttributesInCollection( coll ) ) == [ attr ], \
           label + ": the attribute took a new ID, or was lost or doubled"
    assert list( vsp.GetAttributeStringVal( attr ) ) == [ vsp.GetAttributeName( attr ) ], label


def curve_of( xs ):
    return vsp.GetParmContainer( vsp.GetXSecParm( xs, "Scale" ) )


@pytest.mark.parametrize( "gtype, idx, shape", [ ( "FUSELAGE", 2, vsp.XS_SUPER_ELLIPSE ),
                                                 ( "STACK", 2, vsp.XS_ROUNDED_RECTANGLE ),
                                                 ( "WING", 1, vsp.XS_CST_AIRFOIL ),
                                                 ( "PROP", 1, vsp.XS_SIX_SERIES ) ] )
def testAShapeChangeKeepsTheSectionItsCurveAndItsSpines( gtype, idx, shape ):
    """The cross section, its curve and its spines keep their IDs, and so does every attribute
    on them or on a Parm of theirs, in a collection with the same ID.  Nothing is added or lost
    anywhere else."""
    vsp.VSPRenew()
    drain()
    gid = vsp.AddGeom( gtype )
    spine = None
    if gtype in ( "FUSELAGE", "STACK" ):
        k = vsp.AddSkinSpine( gid, 0.125 )
    vsp.Update()

    xss = vsp.GetXSecSurf( gid, 0 )
    xs = vsp.GetXSec( xss, idx )
    crv = curve_of( xs )
    held = { "XSec": xs, "Curve": crv, "Scale": vsp.GetXSecParm( xs, "Scale" ),
             "TE_Close_Thick": vsp.FindParm( crv, "TE_Close_Thick", "Close" ) }
    if gtype in ( "FUSELAGE", "STACK" ):
        spine = vsp.GetSkinSpineID( xs, k )
        held["Spine"] = spine
        held["LAngle"] = vsp.GetSkinSpineParm( spine, "LAngle" )
    tags = { name: tag( obj, "Note_" + name ) for name, obj in held.items() }
    everything = sorted( vsp.FindAllAttributes() )

    vsp.ChangeXSecShape( xss, idx, shape )
    vsp.Update()
    assert drain() == []

    after = vsp.GetXSec( xss, idx )
    assert after == xs, "the cross section took a new ID"
    assert curve_of( after ) == crv, "the curve took a new ID"
    if spine:
        assert vsp.GetSkinSpineID( after, k ) == spine, "the spine took a new ID"

    assert sorted( vsp.FindAllAttributes() ) == everything
    for name, obj in held.items():
        assert_kept( obj, tags[name][0], tags[name][1], name )


def testAShapeChangeKeepsTheSpineParmsLinksAndDesignVariables():
    """A spine's Parms keep their IDs, so a link driving one and a design variable on another
    go on doing so."""
    vsp.VSPRenew()
    drain()
    fid = vsp.AddGeom( "FUSELAGE" )
    k = vsp.AddSkinSpine( fid, 0.125 )
    pod = vsp.AddGeom( "POD" )
    vsp.Update()

    xss = vsp.GetXSecSurf( fid, 0 )
    spine = vsp.GetSkinSpineID( vsp.GetXSec( xss, 2 ), k )
    angle = vsp.GetSkinSpineParm( spine, "LAngle" )
    strength = vsp.GetSkinSpineParm( spine, "LStrength" )
    link = vsp.AddParmLink( vsp.GetParm( pod, "Length", "Design" ), angle )
    vsp.SetParmLinkOffsetFlag( link, False )
    vsp.AddDesignVar( strength, vsp.XDDM_VAR )

    vsp.ChangeXSecShape( xss, 2, vsp.XS_ROUNDED_RECTANGLE )
    vsp.Update()

    spine = vsp.GetSkinSpineID( vsp.GetXSec( xss, 2 ), k )
    assert vsp.GetSkinSpineParm( spine, "LAngle" ) == angle
    assert vsp.GetSkinSpineParm( spine, "LStrength" ) == strength
    assert vsp.GetDesignVar( 0 ) == strength

    vsp.SetParmVal( vsp.GetParm( pod, "Length", "Design" ), 13.0 )
    vsp.Update()
    assert vsp.GetParmVal( angle ) == pytest.approx( 13.0 )


def testAShapeChangedAndChangedBackKeepsItsIDs():
    """There and back again: the cross section, its curve and every Parm the two shapes share
    come back under the IDs they started with."""
    vsp.VSPRenew()
    drain()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    xss = vsp.GetXSecSurf( fid, 0 )
    xs = vsp.GetXSec( xss, 1 )
    crv = curve_of( xs )
    parms = sorted( vsp.FindContainerParmIDs( crv ) )

    vsp.ChangeXSecShape( xss, 1, vsp.XS_SUPER_ELLIPSE )
    vsp.Update()
    vsp.ChangeXSecShape( xss, 1, vsp.XS_ELLIPSE )
    vsp.Update()

    assert vsp.GetXSec( xss, 1 ) == xs
    assert curve_of( xs ) == crv
    assert sorted( vsp.FindContainerParmIDs( crv ) ) == parms


def testAnAttributeOnAParmThatGoesAwayMovesToItsCurve():
    """A circle has no height, so an ellipse's height has nothing to become.  What was said
    about it is said about the curve instead of being lost with nothing said."""
    vsp.VSPRenew()
    drain()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    xss = vsp.GetXSecSurf( fid, 0 )
    xs = vsp.GetXSec( xss, 2 )
    crv = curve_of( xs )
    height = vsp.GetXSecParm( xs, "Ellipse_Height" )
    coll, attr = tag( height, "HeightNote" )
    everything = sorted( vsp.FindAllAttributes() )

    vsp.ChangeXSecShape( xss, 2, vsp.XS_CIRCLE )
    vsp.Update()

    assert not vsp.ValidParm( height )
    assert sorted( vsp.FindAllAttributes() ) == everything
    assert attr in vsp.FindAttributesInCollection( vsp.GetChildCollection( crv ) )
