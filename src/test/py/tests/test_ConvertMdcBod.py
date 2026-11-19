# Converting an MDC_BOD into a Stack.  The surface must not move.  A conversion can add the Stack beside the MDC_BOD, or
# replace the MDC_BOD with it.

import math
import os
import tempfile

import openvsp as vsp
import pytest

from testhelp import drop_errors, pop_errors, assert_no_errors


def put( gid, group="Design", **parms ):
    for name, val in parms.items():
        pid = vsp.FindParm( gid, name, group )
        assert pid, "no Parm " + name
        vsp.SetParmVal( pid, val )
    vsp.Update()


def new( name, parent="", **parms ):
    gid = vsp.AddGeom( name, parent )
    assert gid
    put( gid, **parms )
    drop_errors()
    return gid


def points( gid, nu=7, nw=33 ):
    pts = []
    for s in range( vsp.GetTotalNumSurfs( gid ) ):
        for i in range( nu ):
            for j in range( nw ):
                p = vsp.CompPnt01( gid, s, i / ( nu - 1 ), j / ( nw - 1 ) )
                pts.append( ( p.x(), p.y(), p.z() ) )
    return pts


def apart( a, b ):
    return max( math.dist( p, q ) for p, q in zip( a, b ) )


# ---------------------------------------------------------------------------------------------------------------
# MDC_BOD to Stack
# ---------------------------------------------------------------------------------------------------------------

AFT = dict( LAFT=2.0, DAFT=0.6 )
CAMBER = dict( ZTIP=0.1, ZNOSE=0.05, ZCENTR=0.2 )

EXACT_BODIES = [
    ( "default", dict() ),
    ( "cone nose", dict( TNOSE=0 ) ),
    ( "power nose", dict( TNOSE=2, POWER=0.6 ) ),
    ( "haack nose", dict( TNOSE=3 ) ),
    ( "karman nose", dict( TNOSE=4 ) ),
    ( "secant ogive nose", dict( SLPNOS=0.1 ) ),
    ( "blunt cone", dict( TNOSE=0, BNOSE=0.1 ) ),
    ( "truncated ogive", dict( BNOSE=0.1, TRUNC=1 ) ),
    ( "nose alone", dict( LCENTR=0.0 ) ),
    ( "centerbody size", dict( CENTRDefault=0, DCENTR=1.4 ) ),
    ( "cone boattail", dict( **AFT ) ),
    ( "ogive boattail", dict( TAFT=1, SLPTAL=0.05, **AFT ) ),
    ( "ogive flare", dict( LAFT=2.0, DAFT=1.6, TAFT=1 ) ),
    ( "ellbod", dict( NAMELIST=1, ENOSE=0.5, ECENTR=0.7, WNOSE=1.2 ) ),
    ( "ellbod ogive boattail", dict( NAMELIST=1, ENOSE=0.5, ECENTR=0.7, WAFT=0.6, LAFT=2.0, TAFT=1 ) ),
    ( "camber", dict( **CAMBER ) ),
    ( "camber of a cone boattail", dict( ZAFT=0.3, **AFT, **CAMBER ) ),
    ( "camber of a haack nose", dict( TNOSE=3, **CAMBER ) ),
    ( "camber of a truncated nose", dict( BNOSE=0.1, TRUNC=1, **CAMBER ) ),
    ( "nozzle exit", dict( DEXIT=0.6, **AFT ) ),
    ( "nozzle exit as wide as the base", dict( DEXIT=0.8, **AFT ) ),
    ( "nozzle exit of a cambered body", dict( DEXIT=0.6, ZAFT=0.3, **AFT, **CAMBER ) ),
]


@pytest.mark.parametrize( "name,parms", EXACT_BODIES, ids=[ n for n, _ in EXACT_BODIES ] )
def testBodyConvertsToTheSameSurface( name, parms ):
    vsp.VSPRenew()
    mid = new( "MDC_BOD", **parms )

    sid = vsp.ConvertMdcBodToStack( mid, False )
    assert_no_errors()
    vsp.Update()

    assert sid and sid != mid
    assert vsp.GetGeomTypeName( sid ) == "Stack"
    bound = vsp.CompareGeomSurfaces( mid, sid )
    assert_no_errors()
    assert 0.0 <= bound < 1e-12


def testBodyPlacementIsKept():
    vsp.VSPRenew()
    parent = new( "POD" )
    mid = new( "MDC_BOD", parent=parent, **CAMBER )
    put( mid, "XForm", Origin=0.4, Y_Rel_Rotation=8.0, X_Rel_Location=1.0, Z_Rel_Location=0.2 )

    sid = vsp.ConvertMdcBodToStack( mid, False )
    assert_no_errors()
    vsp.Update()

    assert vsp.GetGeomParent( sid ) == parent
    assert 0.0 <= vsp.CompareGeomSurfaces( mid, sid ) < 1e-12


def testBodyAddLeavesTheBodyAndCopiesItsSubsurfaces():
    vsp.VSPRenew()
    mid = new( "MDC_BOD" )
    vsp.SetGeomName( mid, "Body B" )
    vsp.AddSubSurf( mid, vsp.SS_RECTANGLE )
    vsp.Update()
    before = points( mid )
    n = len( vsp.FindGeoms() )

    sid = vsp.ConvertMdcBodToStack( mid, False )
    assert_no_errors()
    vsp.Update()

    assert len( vsp.FindGeoms() ) == n + 1
    assert vsp.GetGeomTypeName( mid ) == "MDC_BOD"
    assert vsp.GetGeomName( mid ) == "Body B"
    assert vsp.GetGeomName( sid ) == "Body B_Stack"
    assert apart( before, points( mid ) ) == 0.0
    assert vsp.GetNumSubSurf( mid ) == 1 and vsp.GetNumSubSurf( sid ) == 1
    assert vsp.GetSubSurf( mid, 0 ) != vsp.GetSubSurf( sid, 0 )


def testBodyReplaceTakesTheBodysPlaceAndWhatItOwns():
    vsp.VSPRenew()
    parent = new( "POD" )
    mid = new( "MDC_BOD", parent=parent, **CAMBER, **AFT )
    put( mid, "XForm", Origin=0.4, Y_Rel_Rotation=8.0, X_Rel_Location=1.0 )
    child = new( "POD", parent=mid )
    put( child, "XForm", X_Rel_Location=6.0 )
    vsp.SetGeomName( mid, "Body A" )
    vsp.AddSubSurf( mid, vsp.SS_RECTANGLE )
    vsp.Update()
    body_before = points( mid )
    child_before = points( child, 3, 9 )

    sid = vsp.ConvertMdcBodToStack( mid, True )
    assert_no_errors()
    vsp.Update()

    assert sid and sid not in ( mid, )
    assert mid not in vsp.FindGeoms()
    assert vsp.GetGeomTypeName( sid ) == "Stack"
    assert vsp.GetGeomName( sid ) == "Body A"
    assert vsp.GetGeomParent( sid ) == parent
    assert vsp.GetGeomParent( child ) == sid
    assert vsp.GetNumSubSurf( sid ) == 1
    assert apart( body_before, points( sid ) ) < 1e-9
    assert apart( child_before, points( child, 3, 9 ) ) < 1e-9


CAMBERED_OGIVES = [
    ( "ogive boattail", dict( TAFT=1, SLPTAL=0.05, ZAFT=0.3, **AFT, **CAMBER ) ),
    ( "ogive flare", dict( LAFT=2.0, DAFT=1.6, TAFT=1, **CAMBER ) ),
    ( "ellbod ogive boattail", dict( NAMELIST=1, ENOSE=0.5, ECENTR=0.7, WAFT=0.6, LAFT=2.0, TAFT=1, **CAMBER ) ),
]


@pytest.mark.parametrize( "name,parms", CAMBERED_OGIVES, ids=[ n for n, _ in CAMBERED_OGIVES ] )
def testCamberAcrossAnOgiveIsNearlyTheSame( name, parms ):
    # The camber lifts each point of the surface between the sections, and tilts what it goes along.  The Stack's
    # tangent conditions are tilted the same at the four sides of each section, and are blended between them.  What is
    # left is the blend, a few thousandths of the body's size.
    vsp.VSPRenew()
    mid = new( "MDC_BOD", **parms )

    sid = vsp.ConvertMdcBodToStack( mid, False )
    assert_no_errors()
    vsp.Update()

    assert apart( points( mid, 21, 65 ), points( sid, 21, 65 ) ) < 8e-3


def testBodyConvertsOnlyAMdcBod():
    vsp.VSPRenew()
    pod = new( "POD" )

    drop_errors()
    assert vsp.ConvertMdcBodToStack( pod, False ) == ""
    assert len( pop_errors() ) == 1

    assert vsp.ConvertMdcBodToStack( "NOSUCHID", True ) == ""
    assert len( pop_errors() ) == 1


TESS_BODIES = [
    ( "default", dict( **AFT ), ( 9, 9, 9 ) ),
    ( "by part", dict( **AFT ), ( 5, 13, 4 ) ),
    ( "karman nose past the old cap limit", dict( TNOSE=4, **AFT ), ( 60, 3, 6 ) ),
    ( "nozzle exit", dict( DEXIT=0.5, **AFT ), ( 17, 3, 6 ) ),
    ( "nose alone", dict( LCENTR=0.0 ), ( 7, 3, 3 ) ),
]


@pytest.mark.parametrize( "name,parms,tess", TESS_BODIES, ids=[ n for n, _, _ in TESS_BODIES ] )
def testStackTakesTheBodysRowsByPart( name, parms, tess ):
    vsp.VSPRenew()
    mid = new( "MDC_BOD", **parms )
    put( mid, "Shape", NoseTess_U=tess[0], CentrTess_U=tess[1], AftTess_U=tess[2] )
    sid = vsp.ConvertMdcBodToStack( mid, False )
    vsp.Update()

    # The Stack's caps take the nose's rows, and its rows are the body's, row for row.
    assert vsp.GetParmVal( vsp.FindParm( sid, "CapUMinTess", "EndCap" ) ) == tess[0]
    ub, _ = vsp.GetUWTess01( mid, 0 )
    us, _ = vsp.GetUWTess01( sid, 0 )
    assert len( ub ) == len( us )
    for a, b in zip( ub, us ):
        for w in ( 0.0, 0.3 ):
            assert ( vsp.CompPnt01( mid, 0, a, w ) - vsp.CompPnt01( sid, 0, b, w ) ).mag() < 1e-9


def testStackFromABodySavesAndReadsBack():
    vsp.VSPRenew()
    mid = new( "MDC_BOD", **AFT, **CAMBER )
    sid = vsp.ConvertMdcBodToStack( mid, False )
    vsp.Update()
    before = points( sid )

    fname = os.path.join( tempfile.mkdtemp(), "stack.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.ClearVSPModel()
    vsp.ReadVSPFile( fname )
    vsp.Update()

    stacks = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Stack" ]
    assert len( stacks ) == 1
    assert apart( before, points( stacks[0] ) ) < 1e-9
