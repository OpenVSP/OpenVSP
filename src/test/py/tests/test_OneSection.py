# A Fuselage or Stack of one section: a surface of no length, which its end caps give a shape.  Each cap is the one a
# cylinder of that section has on the same end.

import math
import os
import tempfile

import openvsp as vsp
import pytest

from testhelp import drop_errors, assert_no_errors


CAPS = [ ( "round", vsp.ROUND_END_CAP ), ( "edge", vsp.EDGE_END_CAP ), ( "sharp", vsp.SHARP_END_CAP ),
         ( "point", vsp.POINT_END_CAP ), ( "cone", vsp.CONE_END_CAP ), ( "ogive", vsp.OGIVE_END_CAP ),
         ( "ogive surf", vsp.OGIVE_SURF_END_CAP ), ( "power", vsp.POWER_LAW_END_CAP ),
         ( "karman", vsp.VON_KARMAN_END_CAP ), ( "haack", vsp.SEARS_HAACK_END_CAP ) ]


def set_section( xs, w, h ):
    vsp.SetParmVal( vsp.GetXSecParm( xs, "Ellipse_Width" ), w )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "Ellipse_Height" ), h )


def set_caps( gid, cmin, cmax, mods ):
    vsp.SetParmVal( vsp.FindParm( gid, "CapUMinOption", "EndCap" ), cmin )
    vsp.SetParmVal( vsp.FindParm( gid, "CapUMaxOption", "EndCap" ), cmax )
    vsp.Update()
    for end in ( "CapUMin", "CapUMax" ):
        for name, val in mods.items():
            vsp.SetParmVal( vsp.FindParm( gid, end + name, "EndCap" ), val )
    vsp.Update()


def one_section( geom, w=2.0, h=2.0 ):
    gid = vsp.AddGeom( geom )
    vsp.Update()
    surf = vsp.GetXSecSurf( gid, 0 )
    while vsp.GetNumXSec( surf ) > 1:
        vsp.CutXSec( gid, vsp.GetNumXSec( surf ) - 1 )
        vsp.Update()
    vsp.ChangeXSecShape( surf, 0, vsp.XS_ELLIPSE )
    vsp.Update()
    set_section( vsp.GetXSec( surf, 0 ), w, h )
    vsp.Update()
    return gid


def cylinder( geom, cmin, cmax, mods, w, h, at_end ):
    """A cylinder of the section, straight between two sections, its umin end at the origin, or its umax end where
    at_end is set."""
    gid = vsp.AddGeom( geom )
    vsp.Update()
    surf = vsp.GetXSecSurf( gid, 0 )
    while vsp.GetNumXSec( surf ) > 2:
        vsp.CutXSec( gid, vsp.GetNumXSec( surf ) - 1 )
        vsp.Update()
    for i in range( 2 ):
        vsp.ChangeXSecShape( surf, i, vsp.XS_ELLIPSE )
        vsp.Update()
        xs = vsp.GetXSec( surf, i )
        set_section( xs, w, h )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "AllSym" ), 1 )
        for name in ( "TopLAngle", "TopRAngle" ):
            vsp.SetParmVal( vsp.GetXSecParm( xs, name ), 0.0 )
        for name in ( "TopLStrength", "TopRStrength" ):
            vsp.SetParmVal( vsp.GetXSecParm( xs, name ), 1.0 )
    vsp.Update()
    set_caps( gid, cmin, cmax, mods )
    if at_end:
        length = vsp.ComputeXSecPnt( vsp.GetXSec( surf, 1 ), 0.0 ).x() - vsp.ComputeXSecPnt( vsp.GetXSec( surf, 0 ), 0.0 ).x()
        vsp.SetParmVal( vsp.FindParm( gid, "X_Rel_Location", "XForm" ), -length )
        vsp.Update()
    return gid


def worst_from_cylinder( geom, cap, mods, w, h ):
    vsp.VSPRenew()
    drop_errors()
    gid = one_section( geom, w, h )
    set_caps( gid, cap, cap, mods )
    front = cylinder( geom, cap, vsp.NO_END_CAP, mods, w, h, False )
    back = cylinder( geom, vsp.NO_END_CAP, cap, mods, w, h, True )
    worst = 0.0
    for i in range( 41 ):
        for j in range( 0, 41, 2 ):
            p = vsp.CompPnt01( gid, 0, i / 40, j / 40 )
            worst = max( worst, min( vsp.ProjPnt01I( front, p )[0], vsp.ProjPnt01I( back, p )[0] ) )
    assert_no_errors()
    return worst


def testFuselageAndStackCutToOneSection():
    for geom in ( "FUSELAGE", "STACK" ):
        vsp.VSPRenew()
        gid = one_section( geom )
        assert vsp.GetNumXSec( vsp.GetXSecSurf( gid, 0 ) ) == 1
        # The last section stays.
        vsp.CutXSec( gid, 0 )
        vsp.Update()
        assert vsp.GetNumXSec( vsp.GetXSecSurf( gid, 0 ) ) == 1

        # And a section can be put back, apart from it.
        drop_errors()
        vsp.InsertXSec( gid, 0, vsp.XS_CIRCLE )
        vsp.Update()
        assert_no_errors()
        surf = vsp.GetXSecSurf( gid, 0 )
        assert vsp.GetNumXSec( surf ) == 2
        x0 = vsp.ComputeXSecPnt( vsp.GetXSec( surf, 0 ), 0.0 ).x()
        x1 = vsp.ComputeXSecPnt( vsp.GetXSec( surf, 1 ), 0.0 ).x()
        assert x1 > x0 + 0.1


@pytest.mark.parametrize( "geom", [ "FUSELAGE", "STACK" ] )
@pytest.mark.parametrize( "name,cap", CAPS, ids=[ c[0] for c in CAPS ] )
def testOneSectionCapsAreACylindersCaps( geom, name, cap ):
    assert worst_from_cylinder( geom, cap, {}, 2.0, 2.0 ) < 1e-6


MODS = { "plain": {}, "truncated": { "Radius": 0.2, "TruncFlag": 1 }, "sheared": { "Offset": 0.3, "ShearFlag": 1 },
         "scaled rows": { "ScaledFlag": 1 }, "offset": { "Offset": 0.3 } }


@pytest.mark.parametrize( "mod", MODS.keys() )
@pytest.mark.parametrize( "name,cap", CAPS, ids=[ c[0] for c in CAPS ] )
def testOneSectionEllipticCapsAreACylindersCaps( name, cap, mod ):
    assert worst_from_cylinder( "STACK", cap, MODS[mod], 3.0, 1.5 ) < 1e-6


def testOneSectionFlatAndRoundIsAHalfEllipsoid():
    # A section L long and W wide, flat at one end and round H long at the other.
    L, W, H = 5.0, 2.0, 1.0
    vsp.VSPRenew()
    drop_errors()
    gid = one_section( "STACK", W, L )
    set_caps( gid, vsp.FLAT_END_CAP, vsp.ROUND_END_CAP, {} )
    vsp.SetParmVal( vsp.FindParm( gid, "CapUMaxRelFlag", "EndCap" ), 0 )
    vsp.Update()
    vsp.SetParmVal( vsp.FindParm( gid, "CapUMaxLengthDim", "EndCap" ), H )
    vsp.Update()
    worst = 0.0
    for i in range( 41 ):
        for j in range( 41 ):
            p = vsp.CompPnt01( gid, 0, i / 40, j / 40 )
            if p.x() < 1e-9:
                continue
            r = math.sqrt( ( p.x() / H ) ** 2 + ( p.y() / ( W / 2 ) ) ** 2 + ( p.z() / ( L / 2 ) ) ** 2 )
            worst = max( worst, abs( r - 1.0 ) )
    assert worst < 1e-3

    # Closed: its volume tends to the half ellipsoid's, and its mesh is watertight.
    vsp.SetParmVal( vsp.FindParm( gid, "Tess_W", "Shape" ), 65 )
    vsp.SetParmVal( vsp.FindParm( gid, "CapUMinTess", "EndCap" ), 32 )
    vsp.Update()
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    vol = vsp.GetDoubleResults( vsp.FindLatestResultsID( "Comp_Geom" ), "Theo_Vol" )[0]
    assert vol == pytest.approx( 2.0 / 3.0 * math.pi * H * W / 2 * L / 2, rel=0.01 )

    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.2 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.02 )
    vsp.SetComputationFileName( vsp.CFD_STL_TYPE, os.path.join( tempfile.mkdtemp(), "one.stl" ) )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_STL_TYPE )
    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert vsp.GetIntResults( rid, "Num_Border_Edges" )[0] == 0
    assert vsp.GetIntResults( rid, "Num_Over_Connected_Edges" )[0] == 0
    assert vsp.GetIntResults( rid, "Num_Tris" )[0] > 0
    assert_no_errors()


def testOneSectionSurvivesSaveAndRead():
    vsp.VSPRenew()
    drop_errors()
    gid = one_section( "FUSELAGE" )
    set_caps( gid, vsp.FLAT_END_CAP, vsp.OGIVE_END_CAP, {} )
    shown = [ vsp.CompPnt01( gid, 0, i / 10, j / 10 ) for i in range( 11 ) for j in range( 11 ) ]
    path = os.path.join( tempfile.mkdtemp(), "one.vsp3" )
    vsp.WriteVSPFile( path, vsp.SET_ALL )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()
    gid = vsp.FindGeoms()[0]
    back = [ vsp.CompPnt01( gid, 0, i / 10, j / 10 ) for i in range( 11 ) for j in range( 11 ) ]
    assert max( vsp.dist( p, q ) for p, q in zip( shown, back ) ) < 1e-12
    assert_no_errors()
