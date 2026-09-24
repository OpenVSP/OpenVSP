# Engine (integrated flowpath) representations of Fuselage and Stack Geoms.
#
# The engine surfaces are trimmed, capped and extended copies of the Geom's surface.  Their
# U tessellation is laid out by relating each section of the engine surface back to a
# section of the original, so a count taken from GetUWTess01 checks that relation directly.
# These are laid out so a broken relation gives a wrong count rather than an out-of-range
# read -- the same fault on other settings reads past the per-section tessellation and runs
# away.

import openvsp as vsp
import pytest

NXSEC = 5
CAP_TESS = 13


def sect_tess( isect ):
    # Distinct per-section U tessellation, so a count identifies the sections it came from.
    return 5 + 2 * isect


def build_engine_body( typ ):
    vsp.VSPRenew()
    gid = vsp.AddGeom( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    while vsp.GetNumXSec( xss ) < NXSEC:
        vsp.InsertXSec( gid, 1, vsp.XS_ELLIPSE )
    while vsp.GetNumXSec( xss ) > NXSEC:
        vsp.CutXSec( gid, 1 )
    for i in range( NXSEC ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_ELLIPSE )
    vsp.Update()

    # SectTess_U of XSec i sets the section that ends at it.
    for i in range( 1, NXSEC ):
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, i ), "SectTess_U" ), sect_tess( i - 1 ) )
    vsp.SetParmVal( gid, "CapUMinTess", "EndCap", CAP_TESS )
    vsp.Update()
    return gid


def set_engine( gid, **kw ):
    for name, val in kw.items():
        vsp.SetParmVal( gid, name, "EngineModel", val )
    vsp.Update()


def num_u( gid, isurf ):
    u, w = vsp.GetUWTess01( gid, isurf )
    return len( u )


def degen_num_u( gid ):
    vsp.SetAnalysisInputDefaults( "DegenGeom" )
    vsp.SetIntAnalysisInput( "DegenGeom", "WriteCSVFlag", [ 0 ] )
    vsp.SetIntAnalysisInput( "DegenGeom", "WriteMFileFlag", [ 0 ] )
    rid = vsp.ExecAnalysis( "DegenGeom" )
    nu = []
    for dg in vsp.GetStringResults( rid, "Degen_DegenGeoms" ):
        if vsp.GetStringResults( dg, "geom_id" )[0] == gid:
            surf = vsp.GetStringResults( dg, "surf" )[0]
            nu.append( len( vsp.GetDoubleMatResults( surf, "x" ) ) )
    vsp.DeleteAllResults()
    return nu


@pytest.mark.parametrize( "typ", [ "FUSELAGE", "STACK" ] )
def testEngineInletTessellatesEachSectionWithItsOwnCount( typ ):
    """An inlet trimmed to its lip keeps each remaining section's U tessellation.

    Trimmed at XSec 1 and capped, the surface is a cap followed by sections 1, 2 and 3 of the
    original.  The cap takes CapUMinTess and each section its own SectTess_U.
    """
    gid = build_engine_body( typ )

    expected_plain = 1 + sum( sect_tess( i ) - 1 for i in range( NXSEC - 1 ) )
    assert num_u( gid, 0 ) == expected_plain

    set_engine( gid,
                GeomIOType=vsp.ENGINE_GEOM_INLET,
                GeomInType=vsp.ENGINE_GEOM_FLOWTHROUGH,
                InletModeType=vsp.ENGINE_MODE_TO_LIP,
                InletLipMode=vsp.ENGINE_LOC_INDEX,
                InletLipIndex=1 )

    assert vsp.GetNumMainSurfs( gid ) == 1
    expected = 1 + ( CAP_TESS - 1 ) + sum( sect_tess( i ) - 1 for i in range( 1, NXSEC - 1 ) )
    assert num_u( gid, 0 ) == expected


@pytest.mark.parametrize( "typ", [ "FUSELAGE", "STACK" ] )
def testEngineNegativeFlowpathTessellatesItsSectionAndCaps( typ ):
    """The negative face-to-lip surface is a cap, section 1 of the original, and a cap.

    With the face at XSec 1 and the lip at XSec 2, the main surface keeps sections 2 and 3
    behind a cap.
    """
    gid = build_engine_body( typ )

    set_engine( gid,
                GeomIOType=vsp.ENGINE_GEOM_INLET,
                GeomInType=vsp.ENGINE_GEOM_FLOWTHROUGH,
                InletModeType=vsp.ENGINE_MODE_TO_FACE_NEG,
                InletLipMode=vsp.ENGINE_LOC_INDEX,
                InletFaceMode=vsp.ENGINE_LOC_INDEX,
                InletFaceIndex=1,
                InletLipIndex=2 )

    assert vsp.GetNumMainSurfs( gid ) == 2

    expected_main = 1 + ( CAP_TESS - 1 ) + sum( sect_tess( i ) - 1 for i in range( 2, NXSEC - 1 ) )
    assert num_u( gid, 0 ) == expected_main

    expected_neg = 1 + ( CAP_TESS - 1 ) + ( sect_tess( 1 ) - 1 ) + ( CAP_TESS - 1 )
    assert num_u( gid, 1 ) == expected_neg


@pytest.mark.parametrize( "typ", [ "FUSELAGE", "STACK" ] )
def testTurningTheEngineOffLeavesOneSurface( typ ):
    """The negative flowpath surface goes away with the engine representation."""
    gid = build_engine_body( typ )
    expected_plain = num_u( gid, 0 )

    set_engine( gid,
                GeomIOType=vsp.ENGINE_GEOM_INLET,
                GeomInType=vsp.ENGINE_GEOM_FLOWTHROUGH,
                InletModeType=vsp.ENGINE_MODE_TO_FACE_NEG,
                InletLipMode=vsp.ENGINE_LOC_INDEX,
                InletFaceMode=vsp.ENGINE_LOC_INDEX,
                InletFaceIndex=1,
                InletLipIndex=2 )
    assert vsp.GetNumMainSurfs( gid ) == 2

    set_engine( gid, GeomIOType=vsp.ENGINE_GEOM_NONE )
    assert vsp.GetNumMainSurfs( gid ) == 1
    assert num_u( gid, 0 ) == expected_plain


@pytest.mark.parametrize( "typ", [ "FUSELAGE", "STACK" ] )
def testAnEngineWithNoSurfaceCanBeTurnedBackOn( typ ):
    """A flowpath shown only as its negative surface, with no inlet flowpath to show, has no
    surfaces at all.  The next update skins the Geom again from nothing.
    """
    gid = build_engine_body( typ )
    expected_plain = num_u( gid, 0 )

    set_engine( gid,
                GeomIOType=vsp.ENGINE_GEOM_INLET,
                GeomInType=vsp.ENGINE_GEOM_FLOWPATH,
                InletModeType=vsp.ENGINE_MODE_TO_FACE_NEG_ONLY )
    assert vsp.GetNumMainSurfs( gid ) == 0

    set_engine( gid, GeomIOType=vsp.ENGINE_GEOM_NONE )
    assert vsp.GetNumMainSurfs( gid ) == 1
    assert num_u( gid, 0 ) == expected_plain


@pytest.mark.parametrize( "typ", [ "FUSELAGE", "STACK" ] )
def testDegenGeomKeepsEveryEngineSurfaceWhole( typ ):
    """With no Geom end caps, DegenGeom drops no section of any engine surface.

    An inlet and an outlet each shown with a negative flowpath make three main surfaces.
    DegenGeom skips a surface's first or last section when that surface's end cap flag is set,
    so a flag for a surface past the first shows as missing rows.
    """
    gid = build_engine_body( typ )

    set_engine( gid,
                GeomIOType=vsp.ENGINE_GEOM_INLET_OUTLET,
                GeomInType=vsp.ENGINE_GEOM_FLOWTHROUGH,
                GeomOutType=vsp.ENGINE_GEOM_FLOWTHROUGH,
                InletModeType=vsp.ENGINE_MODE_TO_FACE_NEG,
                OutletModeType=vsp.ENGINE_MODE_TO_FACE_NEG,
                InletLipMode=vsp.ENGINE_LOC_INDEX,
                InletFaceMode=vsp.ENGINE_LOC_INDEX,
                OutletLipMode=vsp.ENGINE_LOC_INDEX,
                OutletFaceMode=vsp.ENGINE_LOC_INDEX,
                InletFaceIndex=0,
                InletLipIndex=1,
                OutletLipIndex=3,
                OutletFaceIndex=4 )

    nsurf = vsp.GetNumMainSurfs( gid )
    assert nsurf == 3
    assert degen_num_u( gid ) == [ num_u( gid, i ) for i in range( nsurf ) ]


if __name__ == "__main__":
    for t in [ "FUSELAGE", "STACK" ]:
        testEngineInletTessellatesEachSectionWithItsOwnCount( t )
        testEngineNegativeFlowpathTessellatesItsSectionAndCaps( t )
        testTurningTheEngineOffLeavesOneSurface( t )
        testAnEngineWithNoSurfaceCanBeTurnedBackOn( t )
        testDegenGeomKeepsEveryEngineSurfaceWhole( t )
