# Converting a Fuselage into a Stack.  The surface must not move, and everything that named the
# Fuselage -- its ID, its Parms, its cross sections, its subsurfaces, its attributes -- must go
# on naming the Stack.

import openvsp as vsp
import pytest

import os
import tempfile

def drain():
    em = vsp.ErrorMgrSingleton.getInstance()
    msgs = []
    while em.GetNumTotalErrors() > 0:
        msgs.append( em.PopLastError().GetErrorString() )
    return msgs


def xparm( fid, i, name, val ):
    vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( vsp.GetXSecSurf( fid, 0 ), i ), name ), val )


def convert( fid ):
    """Converts, checks the Stack took the Fuselage's place, and answers how far any surface of
    any Geom moved -- measured against a twin of the whole model, inserted from a file before
    converting and deleted after."""
    vsp.Update()
    originals = list( vsp.FindGeoms() )
    fname = os.path.join( tempfile.mkdtemp(), "twin.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.InsertVSPFile( fname, "" )
    vsp.Update()
    twins = [ g for g in vsp.FindGeoms() if g not in originals ]
    assert [ vsp.GetGeomTypeName( g ) for g in twins ] == [ vsp.GetGeomTypeName( g ) for g in originals ]

    drain()
    sid = vsp.ConvertFuselageToStack( fid )
    assert drain() == []
    vsp.Update()
    assert sid == fid, "the Stack did not take the Fuselage's ID"
    assert vsp.GetGeomTypeName( sid ) == "Stack"

    bounds = [ vsp.CompareGeomSurfaces( a, b ) for a, b in zip( originals, twins ) ]
    assert drain() == []
    assert min( bounds ) >= 0.0

    vsp.DeleteGeomVec( twins )
    vsp.Update()
    return max( bounds )


def testDefaultFuselageConvertsExactly():
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    assert convert( fid ) == 0.0


def testShapesOffsetsRotationsAndSkinningCarryAcross():
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    xss = vsp.GetXSecSurf( fid, 0 )
    vsp.ChangeXSecShape( xss, 1, vsp.XS_SUPER_ELLIPSE )
    vsp.ChangeXSecShape( xss, 2, vsp.XS_EDIT_CURVE )
    vsp.Update()
    xparm( fid, 1, "XLocPercent", 0.18 )
    xparm( fid, 2, "YLocPercent", 0.02 )
    xparm( fid, 2, "YRotate", 7.0 )
    xparm( fid, 3, "ZLocPercent", 0.06 )
    xparm( fid, 3, "XRotate", 11.0 )
    xparm( fid, 3, "Spin", 0.1 )
    for i in ( 1, 2, 3 ):
        xs = vsp.GetXSec( xss, i )
        vsp.SetXSecContinuity( xs, 1 )
        vsp.SetXSecTanAngles( xs, vsp.XSEC_BOTH_SIDES, 3.0 * i, -2.0, 5.0, 1.0 )
        vsp.SetXSecTanStrengths( xs, vsp.XSEC_LEFT_SIDE, 0.4, 0.6, 0.5, 0.7 )
        vsp.SetXSecTanStrengths( xs, vsp.XSEC_RIGHT_SIDE, 0.9, 0.3, 0.45, 0.35 )
        vsp.SetXSecCurvatures( xs, vsp.XSEC_BOTH_SIDES, 0.2, -0.1, 0.05, 0.0 )
    vsp.Update()
    vsp.AddSkinSpine( fid, 0.125 )
    assert convert( fid ) < 1e-13


@pytest.mark.parametrize( "policy", [ 0, 1, 2 ] )
def testLoopAndFreePoliciesConvert( policy ):
    """A loop Fuselage starts at its tail, and a free one here starts off its origin -- both put
    the Stack's first section somewhere other than the Fuselage's origin."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fid, "OrderPolicy", "Design", policy )
    vsp.Update()
    if policy == 2:
        vsp.ChangeXSecShape( vsp.GetXSecSurf( fid, 0 ), 0, vsp.XS_ELLIPSE )
        vsp.Update()
        xparm( fid, 0, "XLocPercent", 0.05 )
        xparm( fid, 0, "ZLocPercent", 0.02 )
        xparm( fid, 0, "YRotate", 6.0 )
    else:
        xparm( fid, 2, "XLocPercent", 0.0 )
    assert convert( fid ) < 1e-13
    # MONOTONIC and FREE are both FREE on a Stack.
    assert vsp.GetParmVal( fid, "OrderPolicy", "Design" ) == { 0: 0, 1: 1, 2: 0 }[ policy ]


def testPlacementRotationOriginAndChildrenStayPut():
    """Rotated about a point along its length, and with its first section offset: the Stack's
    placement absorbs both, and a child placed relative to it is put back where it was."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fid, "X_Rel_Location", "XForm", 3.0 )
    vsp.SetParmVal( fid, "Y_Rel_Rotation", "XForm", 8.0 )
    vsp.SetParmVal( fid, "Z_Rel_Rotation", "XForm", 5.0 )
    vsp.SetParmVal( fid, "Origin", "XForm", 0.4 )
    vsp.SetParmVal( fid, "Scale", "XForm", 1.3 )
    vsp.Update()
    xparm( fid, 0, "ZLocPercent", 0.03 )
    pod = vsp.AddGeom( "POD", fid )
    vsp.SetParmVal( pod, "X_Rel_Location", "XForm", 6.0 )
    vsp.SetParmVal( pod, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( pod, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_COMP )
    uv = vsp.AddGeom( "POD", fid )
    vsp.SetParmVal( uv, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( uv, "U_Attach_Location", "Attach", 0.4 )
    assert convert( fid ) < 1e-12


def nose_moved_fuselage():
    """A free Fuselage whose first section is offset and rotated, placed off the origin and
    rotated itself -- the arrangement that once needed the Stack's frame moved."""
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fid, "OrderPolicy", "Design", 2 )
    vsp.Update()
    vsp.ChangeXSecShape( vsp.GetXSecSurf( fid, 0 ), 0, vsp.XS_ELLIPSE )
    vsp.Update()
    xparm( fid, 0, "XLocPercent", 0.05 )
    xparm( fid, 0, "ZLocPercent", 0.02 )
    xparm( fid, 0, "YRotate", 6.0 )
    vsp.SetParmVal( fid, "Z_Rel_Location", "XForm", 1.0 )
    vsp.SetParmVal( fid, "X_Rel_Rotation", "XForm", 10.0 )
    vsp.Update()
    return fid


def testAbsolutePlacementCarriesAcross():
    vsp.VSPRenew()
    fid = nose_moved_fuselage()
    vsp.SetParmVal( fid, "Abs_Or_Relitive_flag", "XForm", vsp.ABS )
    vsp.SetParmVal( fid, "X_Location", "XForm", 2.0 )
    vsp.SetParmVal( fid, "Y_Rotation", "XForm", 4.0 )
    assert convert( fid ) < 1e-12
    assert vsp.GetParmVal( fid, "Abs_Or_Relitive_flag", "XForm" ) == vsp.ABS


def testEveryKindOfChildStaysPut():
    """On the surface, by component with a child of its own, and a Conformal -- which is shaped
    in its parent's frame, so it is only right if the Stack's frame is the Fuselage's."""
    vsp.VSPRenew()
    fid = nose_moved_fuselage()
    uv = vsp.AddGeom( "POD", fid )
    vsp.SetParmVal( uv, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( uv, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_UV )
    vsp.SetParmVal( uv, "U_Attach_Location", "Attach", 0.4 )
    vsp.SetParmVal( uv, "V_Attach_Location", "Attach", 0.3 )
    wing = vsp.AddGeom( "WING", fid )
    vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 8.0 )
    vsp.SetParmVal( wing, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( wing, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_COMP )
    grand = vsp.AddGeom( "POD", wing )
    vsp.SetParmVal( grand, "Y_Rel_Location", "XForm", 3.0 )
    vsp.SetParmVal( grand, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_COMP )
    vsp.Update()
    conf = vsp.AddGeom( "CONFORMAL", fid )
    vsp.Update()
    assert convert( fid ) < 1e-12
    assert sorted( vsp.GetGeomChildren( fid ) ) == sorted( [ uv, wing, conf ] )
    for g in ( uv, wing, conf ):
        assert vsp.GetGeomParent( g ) == fid
    assert vsp.GetGeomParent( grand ) == wing


def testCurvesWithoutWidthAndHeightParmsCarryAcross():
    """An edit curve and an airfoil keep their shape in Parms of their own, not a width and a
    height, and in the edit curve's case in control points that are not Parms at all."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    xss = vsp.GetXSecSurf( fid, 0 )
    vsp.ChangeXSecShape( xss, 2, vsp.XS_EDIT_CURVE )
    vsp.ChangeXSecShape( xss, 3, vsp.XS_FOUR_SERIES )
    vsp.Update()
    assert convert( fid ) == 0.0


def testAConvertedModelReopensTheSame( tmp_path ):
    vsp.VSPRenew()
    fid = nose_moved_fuselage()
    vsp.ChangeXSecShape( vsp.GetXSecSurf( fid, 0 ), 2, vsp.XS_SUPER_ELLIPSE )
    assert convert( fid ) < 1e-12

    fname = str( tmp_path / "converted.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.InsertVSPFile( fname, "" )
    vsp.Update()
    reread = [ g for g in vsp.FindGeoms() if g != fid ]

    assert len( reread ) == 1 and vsp.GetGeomTypeName( reread[0] ) == "Stack"
    assert vsp.CompareGeomSurfaces( fid, reread[0] ) == 0.0


def testIdentityCarriesAcross():
    """Parm IDs, cross section IDs, subsurfaces, links, design variables and attributes."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    xs2 = vsp.GetXSec( vsp.GetXSecSurf( fid, 0 ), 2 )
    width = vsp.GetXSecParm( xs2, "Ellipse_Width" )
    strength = vsp.GetXSecParm( xs2, "TopLStrength" )
    tess = vsp.GetParm( fid, "Tess_W", "Shape" )
    length = vsp.GetParm( fid, "Length", "Design" )
    xloc = vsp.GetXSecParm( xs2, "XLocPercent" )
    xrot = vsp.GetXSecParm( xs2, "XRotate" )
    ss = vsp.AddSubSurf( fid, vsp.SS_RECTANGLE )
    pod_len = vsp.GetParm( pod, "Length", "Design" )
    link = vsp.AddParmLink( pod_len, width )
    vsp.SetParmLinkOffsetFlag( link, False )
    vsp.AddDesignVar( strength, vsp.XDDM_VAR )
    vsp.AddAttributeString( vsp.GetChildCollection( fid ), "GeomNote", "g" )
    vsp.AddAttributeString( vsp.GetChildCollection( xs2 ), "XSecNote", "x" )
    vsp.AddAttributeString( vsp.GetChildCollection( width ), "ParmNote", "p" )

    convert( fid )

    assert vsp.GetXSec( vsp.GetXSecSurf( fid, 0 ), 2 ) == xs2
    for p in ( width, strength, tess, xrot ):
        assert vsp.ValidParm( p )
    # Paired with the Parm that places a Stack XSec in the same mode, though it is a length
    # rather than a fraction of one.
    assert vsp.ValidParm( xloc ) and vsp.GetParmName( xloc ) == "XDelta"
    assert vsp.GetParmName( xrot ) == "XRotate"
    assert vsp.GetParmContainer( width ) != "" and vsp.GetParmName( width ) == "Ellipse_Width"
    assert not vsp.ValidParm( length ), "a Stack has no Length for the Fuselage's to become"
    assert ss in list( vsp.GetSubSurfIDVec( fid ) )
    assert vsp.GetDesignVar( 0 ) == strength and vsp.GetParmName( strength ) == "TopLStrength"

    vsp.SetParmVal( pod_len, 5.5 )
    vsp.Update()
    assert vsp.GetParmVal( width ) == pytest.approx( 5.5 )

    for obj, name, val in ( ( fid, "GeomNote", "g" ), ( xs2, "XSecNote", "x" ), ( width, "ParmNote", "p" ) ):
        aid = vsp.FindAttributeInCollection( obj, name, 0 )
        assert aid and list( vsp.GetAttributeStringVal( aid ) ) == [ val ]


def testAMirrorInTheFuselagesFrameStaysPut():
    """A wing hung off a Fuselage whose nose is offset and rotated is mirrored in the
    Fuselage's frame.  The Stack keeps that frame, so the mirror image stays where it was."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.ChangeXSecShape( vsp.GetXSecSurf( fid, 0 ), 0, vsp.XS_ELLIPSE )
    vsp.Update()
    xparm( fid, 0, "YLocPercent", 0.02 )
    xparm( fid, 0, "ZRotate", 4.0 )
    wid = vsp.AddGeom( "WING", fid )
    vsp.SetParmVal( wid, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( wid, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_COMP )
    vsp.SetParmVal( fid, "Sym_Planar_Flag", "Sym", vsp.SYM_XY )
    vsp.SetParmVal( fid, "Sym_Ancestor", "Sym", 1 )
    vsp.SetParmVal( fid, "Sym_Ancestor_Origin_Flag", "Sym", 0 )
    assert convert( fid ) < 1e-12


def testRefusesWhatIsNotAFuselage():
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    drain()
    assert vsp.ConvertFuselageToStack( pod ) == ""
    em = vsp.ErrorMgrSingleton.getInstance()
    assert em.GetNumTotalErrors() == 1 and em.PopLastError().GetErrorCode() == vsp.VSP_WRONG_GEOM_TYPE
    assert vsp.GetGeomTypeName( pod ) == "Pod"


def testAttributesFollowEverythingThatCarriesAcross():
    """An attribute on the Geom, the XSecSurf, a cross section, its curve, a spine, a subsurface,
    or any Parm with a counterpart is the same attribute afterwards -- same ID, in a collection
    with the same ID, found once.  One on a Parm with no counterpart is found on the container
    that held it: the length's on the Stack, a cross section's reference length on that cross
    section.  Nothing else is added or lost anywhere in the model."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    xss = vsp.GetXSecSurf( fid, 0 )
    xs = vsp.GetXSec( xss, 2 )
    k = vsp.AddSkinSpine( fid, 0.125 )
    ss = vsp.AddSubSurf( fid, vsp.SS_RECTANGLE )
    vsp.Update()
    spine = vsp.GetSkinSpineID( xs, k )
    width = vsp.GetXSecParm( xs, "Ellipse_Width" )

    kept = {
        "Geom": fid,
        "Tess_W": vsp.GetParm( fid, "Tess_W", "Shape" ),
        "X_Rel_Location": vsp.GetParm( fid, "X_Rel_Location", "XForm" ),
        "XSecSurf": xss,
        "XSec": xs,
        "TopLStrength": vsp.GetXSecParm( xs, "TopLStrength" ),
        "XRotate": vsp.GetXSecParm( xs, "XRotate" ),
        "XLocPercent": vsp.GetXSecParm( xs, "XLocPercent" ),
        "Curve": vsp.GetParmContainer( width ),
        "Ellipse_Width": width,
        "Spine": spine,
        "LAngle": vsp.GetSkinSpineParm( spine, "LAngle" ),
        "SubSurface": ss,
        "SubSurfaceParm": vsp.GetSubSurfParmIDs( ss )[0],
    }
    gone = {
        "Length": vsp.GetParm( fid, "Length", "Design" ),
        "RefLength": vsp.GetXSecParm( xs, "RefLength" ),
    }
    attr = {}
    coll = {}
    for name, obj in list( kept.items() ) + list( gone.items() ):
        coll[name] = vsp.GetChildCollection( obj )
        attr[name] = vsp.AddAttributeString( coll[name], "Note_" + name, name )

    everything = sorted( vsp.FindAllAttributes() )

    assert convert( fid ) < 1e-12

    assert sorted( vsp.FindAllAttributes() ) == everything

    # With no counterpart, what was said about a Parm is said about its container.
    expected = { name: [ attr[name] ] for name in kept }
    expected["Geom"].append( attr["Length"] )
    expected["XSec"].append( attr["RefLength"] )
    for name, obj in kept.items():
        assert vsp.GetChildCollection( obj ) == coll[name], name
        assert sorted( vsp.FindAttributesInCollection( coll[name] ) ) == sorted( expected[name] ), name
        assert list( vsp.GetAttributeStringVal( attr[name] ) ) == [ name ], name
    for name, obj in gone.items():
        assert not vsp.ValidParm( obj ), name
        assert list( vsp.GetAttributeStringVal( attr[name] ) ) == [ name ], name


def testAttributesOnWhatIsHandedOverKeepTheirIDs():
    """A structure and one of its parts, a mesh source and a texture are handed to the Stack
    whole, so an attribute on any of them, or on a Parm of theirs, is the same attribute
    afterwards, and nothing else in the model changes."""
    vsp.VSPRenew()
    drain()
    fid = vsp.AddGeom( "FUSELAGE" )
    fea = vsp.AddFeaStruct( fid )
    part = vsp.AddFeaPart( fid, fea, vsp.FEA_SLICE )
    vsp.AddCFDSource( vsp.POINT_SOURCE, fid, 0, 0.5, 1.0, 0.5, 0.5 )
    tex = vsp.AttachGeomTexture( fid, "hull.png" )
    vsp.Update()

    objs = [ vsp.GetFeaStructID( fid, fea ), part, vsp.GetCFDSourceID( fid, 0 ), tex ]
    objs += [ vsp.FindContainerParmIDs( obj )[0] for obj in list( objs ) ]
    tags = []
    for i, obj in enumerate( objs ):
        coll = vsp.GetChildCollection( obj )
        tags.append( ( obj, coll, vsp.AddAttributeString( coll, "Note_%d" % i, "n" ) ) )
    everything = sorted( vsp.FindAllAttributes() )

    assert convert( fid ) < 1e-12

    assert sorted( vsp.FindAllAttributes() ) == everything
    for obj, coll, attr in tags:
        assert vsp.GetChildCollection( obj ) == coll, obj
        assert list( vsp.FindAttributesInCollection( coll ) ) == [ attr ], obj


def testParmsAreFoundByGroupAndNameAfterConverting():
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.AddSkinSpine( fid, 0.125 )
    vsp.Update()
    xs = vsp.GetXSec( vsp.GetXSecSurf( fid, 0 ), 2 )
    spine = vsp.GetSkinSpineID( xs, 0 )
    width = vsp.GetXSecParm( xs, "Ellipse_Width" )
    lookups = [ ( xs, "TopLStrength", "XSec" ), ( vsp.GetParmContainer( width ), "Ellipse_Width", "XSecCurve" ),
                ( spine, "LStrength", "SkinSpine" ), ( fid, "X_Rel_Location", "XForm" ) ]
    before = [ vsp.FindParm( *key ) for key in lookups ]

    convert( fid )

    assert [ vsp.FindParm( *key ) for key in lookups ] == before


@pytest.mark.parametrize( "mode", [ "ABS", "REL" ] )
@pytest.mark.parametrize( "origin", [ 0.0, 0.4 ] )
@pytest.mark.parametrize( "own_rot", [ 0.0, 4.0 ] )
def testAFuselageHungOffAMovedParentStaysPut( mode, origin, own_rot ):
    """The Stack's placement is worked out in the frame it will hang from.  In absolute mode the
    parent's move was once applied a second time."""
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( pod, "X_Rel_Location", "XForm", 1.5 )
    vsp.SetParmVal( pod, "Z_Rel_Rotation", "XForm", 12.0 )
    fid = vsp.AddGeom( "FUSELAGE", pod )
    vsp.SetParmVal( fid, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( fid, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_COMP )
    vsp.SetParmVal( fid, "Origin", "XForm", origin )
    vsp.SetParmVal( fid, "Y_Rel_Rotation", "XForm", own_rot )
    vsp.Update()
    if mode == "ABS":
        vsp.SetParmVal( fid, "Abs_Or_Relitive_flag", "XForm", vsp.ABS )
    assert convert( fid ) < 1e-12
    assert vsp.GetGeomParent( fid ) == pod


@pytest.mark.parametrize( "rot", [ 0.0, 8.0 ] )
def testAbsolutePlacementAboutAnOffOriginCenterStaysPut( rot ):
    """An absolute location names where the rotation center is, so it differs from the Stack's
    whenever there is a center -- rotated or not."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fid, "Origin", "XForm", 0.4 )
    vsp.SetParmVal( fid, "Y_Rel_Rotation", "XForm", rot )
    vsp.Update()
    vsp.SetParmVal( fid, "Abs_Or_Relitive_flag", "XForm", vsp.ABS )
    assert convert( fid ) < 1e-12


def testAnAuxiliaryChildDoesNotStopTheConversion():
    """An auxiliary Geom's surface can be empty; the check before and after must not read it."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.AddGeom( "AUXILIARY", fid )
    vsp.Update()
    assert convert( fid ) < 1e-12


def testWhatIsNotAParmCarriesAcross():
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.SetGeomName( fid, "Hull" )
    vsp.SetSetFlag( fid, vsp.SET_SHOWN, False )
    vsp.SetSetFlag( fid, vsp.SET_FIRST_USER, True )
    vsp.SetGeomWireColor( fid, 10, 200, 30 )
    vsp.SetGeomMaterialName( fid, "Ruby" )
    tex = vsp.AttachGeomTexture( fid, "hull.png" )
    fea = vsp.GetFeaStructID( fid, vsp.AddFeaStruct( fid ) )
    vsp.AddCFDSource( vsp.POINT_SOURCE, fid, 0, 0.5, 1.0, 0.5, 0.5 )
    source = vsp.GetCFDSourceID( fid, 0 )
    vsp.Update()

    convert( fid )

    assert vsp.GetGeomName( fid ) == "Hull"
    assert not vsp.GetSetFlag( fid, vsp.SET_SHOWN )
    assert vsp.GetSetFlag( fid, vsp.SET_FIRST_USER )
    color = vsp.GetGeomWireColor( fid )
    assert ( color.x(), color.y(), color.z() ) == ( 10, 200, 30 )
    assert vsp.GetGeomMaterialName( fid ) == "Ruby"
    assert list( vsp.GetGeomTextureIDVec( fid ) ) == [ tex ]
    assert vsp.NumFeaStructures() == 1 and vsp.GetFeaStructID( fid, 0 ) == fea
    assert vsp.GetNumCFDSources( fid ) == 1 and vsp.GetCFDSourceID( fid, 0 ) == source


def testAPresetForTheOrderPolicyFollowsIt():
    """A preset keeps the order policy by the Parm's ID, which the Stack's answers to after
    converting; its values are renumbered the way the policy itself is."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    policy = vsp.GetParm( fid, "OrderPolicy", "Design" )
    group = vsp.AddVarPresetGroup( "Policies" )
    vsp.AddVarPresetParm( group, policy )
    settings = {}
    for name, value in ( ( "Monotonic", 0 ), ( "Loop", 1 ), ( "Free", 2 ) ):
        vsp.SetParmVal( policy, value )
        vsp.Update()
        settings[name] = vsp.AddVarPresetSetting( group, name )
        vsp.SaveVarPresetParmVals( group, settings[name] )
    vsp.SetParmVal( policy, 0 )
    vsp.Update()

    convert( fid )

    for name, expected in ( ( "Monotonic", 0 ), ( "Loop", 1 ), ( "Free", 0 ) ):
        vsp.ApplyVarPresetSetting( group, settings[name] )
        vsp.Update()
        assert vsp.GetParmVal( policy ) == expected, name
