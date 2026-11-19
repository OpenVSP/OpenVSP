# Converting an MDC_FIN into a Wing and Clones of it.  The surfaces must not move.  A conversion can add them beside the
# MDC_FIN, or replace the MDC_FIN with them.

import math
import os
import tempfile

import openvsp as vsp
import pytest

from testhelp import drop_errors, pop_errors, assert_no_errors


# The Parms that set how many Parms of a station, panel or member there are.
COUNTS = ( "PTYPE", "NLOC", "NIN", "NSPAN", "NPANEL" )


def put( gid, group="Design", **parms ):
    # The Parms of each station, panel or member are made by the update after their count, so the counts go first.
    for name in COUNTS:
        if name in parms:
            vsp.SetParmVal( vsp.FindParm( gid, name, group ), parms[name] )
    vsp.Update()
    for name, val in parms.items():
        if name in COUNTS:
            continue
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


def fin_deviation( fin, wings, nu=9, nw=65 ):
    """How far each surface of the fin set is from the nearest surface of the Wing and Clones, which run around their
    sections one way or the other, and the worst of them."""
    worst = 0.0
    for s in range( vsp.GetTotalNumSurfs( fin ) ):
        best = None
        for w in wings:
            for t in range( vsp.GetTotalNumSurfs( w ) ):
                for reverse in ( False, True ):
                    d = 0.0
                    for i in range( nu ):
                        for j in range( nw ):
                            u = i / ( nu - 1 )
                            v = j / ( nw - 1 )
                            if reverse:
                                vw = 1.0 - v
                            else:
                                vw = v
                            p = vsp.CompPnt01( fin, s, u, v )
                            q = vsp.CompPnt01( w, t, u, vw )
                            d = max( d, math.dist( ( p.x(), p.y(), p.z() ), ( q.x(), q.y(), q.z() ) ) )
                    if best is None or d < best:
                        best = d
        worst = max( worst, best )
    return worst


# ---------------------------------------------------------------------------------------------------------------
# MDC_FIN to Wing
# ---------------------------------------------------------------------------------------------------------------

def phif( gid, *angles ):
    put( gid, NPANEL=len( angles ), PHIFFlag=1 )
    for i, a in enumerate( angles ):
        put( gid, **{ "PHIF_" + str( i + 1 ): a } )


FIN_CASES = [
    ( "default", dict(), None, 4, 1 ),
    ( "naca", dict( SECTYP=1 ), None, 4, 1 ),
    ( "arc", dict( SECTYP=2 ), None, 4, 1 ),
    ( "rounded leading edge", dict( LERType=1, LER_1=0.01, LER_2=0.01 ), None, 4, 1 ),
    ( "rounded keeping the angle", dict( LERType=2, LER_1=0.01, LER_2=0.01 ), None, 4, 1 ),
    ( "flat hexagon", dict( LFLATU_1=0.3, LFLATU_2=0.3 ), None, 4, 1 ),
    ( "sweep measured at the leading edge", dict( STA_1=0.0, SWEEP_1=35.0 ), None, 4, 1 ),
    ( "stations given by XLE", dict( XLEFlag_1=1, XLE_2=1.5 ), None, 4, 1 ),
    ( "two cross sections that differ", dict( LMAXU_1=0.25, LMAXU_2=0.5, LFLATU_1=0.5, LFLATU_2=0.0 ), None, 4, 1 ),
    ( "point tip", dict( CHORD_2=0.0 ), None, 4, 1 ),
    ( "three stations", dict( NSPAN=3, SSPAN_3=2.5, CHORD_3=0.4, SWEEP_2=40.0 ), None, 4, 1 ),
    ( "root off the axis", dict( SSPAN_1=1.0, SSPAN_2=3.0 ), None, 4, 1 ),
    ( "dihedral", dict( GAM_1=10.0, GAM_2=10.0, GAM_3=10.0, GAM_4=10.0 ), None, 4, 1 ),
    ( "one panel", dict(), ( 90.0, ), 1, 1 ),
    ( "one panel on top", dict(), ( 0.0, ), 1, 1 ),
    ( "two panels", dict(), ( 90.0, 270.0 ), 2, 1 ),
    ( "panels not evenly spaced", dict(), ( 10.0, 100.0, 250.0 ), 3, 1 ),
    ( "cambered, which a reflection and a rotation make differently", dict( SECTYP=1, NACADES=4412.0 ),
      ( 45.0, 135.0, 225.0, 315.0 ), 4, 1 ),
    ( "cambered arc", dict( SECTYP=2, ZUPPER_1=0.06, ZUPPER_2=0.06, ZLOWER_1=0.02, ZLOWER_2=0.02, ZLOWERRelFlag=0 ),
      ( 0.0, 90.0 ), 2, 1 ),
    ( "buried root", dict( BuryRoot=1 ), None, 4, 1 ),
    ( "buried root, three swept stations", dict( BuryRoot=1, NSPAN=3, SSPAN_3=2.5, CHORD_3=0.4, SWEEP_1=30.0,
                                                  SWEEP_2=40.0 ), None, 4, 1 ),
    ( "buried root, cambered", dict( BuryRoot=1, SECTYP=1, NACADES=4412.0 ), ( 45.0, 135.0, 225.0, 315.0 ), 4, 1 ),
    ( "buried root extrapolated", dict( BuryRoot=1, BuryRootType=1, CHORD_2=0.5, SWEEP_1=40.0 ), None, 4, 1 ),
    ( "buried root extrapolated, three stations", dict( BuryRoot=1, BuryRootType=1, NSPAN=3, SSPAN_3=2.5, CHORD_3=0.4,
                                                        SWEEP_1=30.0, SWEEP_2=40.0, STA_1=0.5 ), None, 4, 1 ),
]


@pytest.mark.parametrize( "name,parms,angles,ngeom,nsurf", FIN_CASES, ids=[ c[0] for c in FIN_CASES ] )
def testFinSetConvertsToTheSameSurfaces( name, parms, angles, ngeom, nsurf ):
    vsp.VSPRenew()
    fid = new( "MDC_FIN", **parms )
    if angles:
        phif( fid, *angles )

    drop_errors()
    wings = vsp.ConvertMdcFinToWing( fid, False )
    assert_no_errors()
    vsp.Update()

    assert len( wings ) == ngeom
    assert vsp.GetGeomTypeName( wings[0] ) == "Wing"
    assert all( vsp.GetGeomTypeName( w ) == "Clone" for w in wings[1:] )
    assert [ vsp.GetTotalNumSurfs( w ) for w in wings ] == [ nsurf ] * ngeom
    assert fin_deviation( fid, wings ) < 1e-9


@pytest.mark.parametrize( "bury,kind", ( ( 0, 0 ), ( 1, 0 ), ( 1, 1 ) ) )
def testFinSetOnAnEllipticBodyConvertsToTheSameSurfaces( bury, kind ):
    vsp.VSPRenew()
    body = new( "MDC_BOD", NAMELIST=1, WNOSE=2.0, ENOSE=0.5, CENTRDefault=1, ECENTR=0.5, LNOSE=2.0, LCENTR=8.0 )
    fid = new( "MDC_FIN", parent=body, NPANEL=4, BuryRoot=bury, BuryRootType=kind, CHORD_2=0.5, SWEEP_1=40.0 )
    vsp.SetParmVal( vsp.FindParm( fid, "X_Rel_Location", "XForm" ), 6.0 )
    vsp.Update()

    drop_errors()
    wings = vsp.ConvertMdcFinToWing( fid, False )
    assert_no_errors()
    assert fin_deviation( fid, wings ) < 1e-6


def testFinSetKeepsItsOwnSymmetryAndPlacement():
    vsp.VSPRenew()
    parent = new( "POD" )
    fid = new( "MDC_FIN", parent=parent )
    phif( fid, 20.0, 110.0, 200.0 )
    put( fid, "XForm", X_Rel_Location=2.0, Y_Rel_Location=0.3, Z_Rel_Rotation=5.0, X_Rel_Rotation=12.0, Origin=0.3 )
    put( fid, "Sym", Sym_Planar_Flag=2 )

    wings = vsp.ConvertMdcFinToWing( fid, False )
    assert_no_errors()
    vsp.Update()

    # Three panels and their reflections.
    assert len( wings ) == 6
    assert all( vsp.GetGeomParent( w ) == parent for w in wings )
    assert fin_deviation( fid, wings ) < 1e-9


def testFinSetKeepsItsOwnFlip():
    vsp.VSPRenew()
    fid = new( "MDC_FIN", SECTYP=1, NACADES=4412.0 )
    phif( fid, 20.0, 110.0, 200.0 )
    put( fid, "XForm", X_Rel_Location=2.0, Z_Rel_Rotation=5.0 )
    put( fid, "Sym", Flip_Flag=2 )

    wings = vsp.ConvertMdcFinToWing( fid, False )
    assert_no_errors()
    vsp.Update()

    assert len( wings ) == 3
    assert fin_deviation( fid, wings ) < 1e-9


def testFinSetFlapsAreCarriedToTheWings():
    vsp.VSPRenew()
    fid = new( "MDC_FIN", NSPAN=3, SSPAN_3=2.5, CHORD_3=0.4, CFOC_1=0.0, CFOC_2=0.3, CFOC_3=0.25 )
    assert vsp.GetNumSubSurf( fid ) == 1
    phif( fid, 10.0, 100.0, 250.0 )

    wings = vsp.ConvertMdcFinToWing( fid, False )
    assert_no_errors()
    vsp.Update()

    assert len( wings ) == 3
    assert [ vsp.GetNumSubSurf( w ) for w in wings ] == [ 1, 1, 1 ]
    assert vsp.GetNumSubSurf( fid ) == 1
    ids = [ vsp.GetSubSurf( w, 0 ) for w in wings ] + [ vsp.GetSubSurf( fid, 0 ) ]
    assert len( set( ids ) ) == 4


def testFinSetAddLeavesTheFinSet():
    vsp.VSPRenew()
    fid = new( "MDC_FIN" )
    vsp.SetGeomName( fid, "Tail" )
    before = points( fid )
    n = len( vsp.FindGeoms() )

    wings = vsp.ConvertMdcFinToWing( fid, False )
    assert_no_errors()
    vsp.Update()

    assert len( vsp.FindGeoms() ) == n + 4
    assert vsp.GetGeomTypeName( fid ) == "MDC_FIN"
    assert vsp.GetGeomName( fid ) == "Tail"
    assert [ vsp.GetGeomName( w ) for w in wings ] == [ "Tail_Wing_1", "Tail_Wing_2", "Tail_Wing_3", "Tail_Wing_4" ]
    assert apart( before, points( fid ) ) == 0.0


def testFinSetReplaceTakesItsPlaceAndWhatItOwns():
    vsp.VSPRenew()
    parent = new( "POD" )
    fid = new( "MDC_FIN", parent=parent, NSPAN=3, SSPAN_3=2.5, CHORD_3=0.4, CFOC_1=0.3, CFOC_2=0.3, CFOC_3=0.3 )
    phif( fid, 10.0, 100.0, 250.0 )
    put( fid, "XForm", X_Rel_Location=2.0, Z_Rel_Rotation=5.0, X_Rel_Rotation=12.0 )
    child = new( "POD", parent=fid )
    put( child, "XForm", X_Rel_Location=0.5 )
    vsp.SetGeomName( fid, "Tail" )
    vsp.Update()
    fin_before = points( fid )
    child_before = points( child, 3, 9 )

    wings = vsp.ConvertMdcFinToWing( fid, True )
    assert_no_errors()
    vsp.Update()

    assert len( wings ) == 3
    assert fid not in vsp.FindGeoms()
    assert [ vsp.GetGeomName( w ) for w in wings ] == [ "Tail_1", "Tail_2", "Tail_3" ]
    assert all( vsp.GetGeomParent( w ) == parent for w in wings )
    assert vsp.GetGeomParent( child ) == wings[0]
    assert [ vsp.GetNumSubSurf( w ) for w in wings ] == [ 2, 0, 0 ]
    assert apart( child_before, points( child, 3, 9 ) ) < 1e-9

    # Each panel of the fin set is a surface of one of the Wings.
    after = []
    for w in wings:
        after += points( w )
    for p in fin_before[::5]:
        assert min( math.dist( p, q ) for q in after ) < 0.1


def testFinSetReplaceDoesNotRefuseASweepAWingCannotHave():
    # A Wing's sweep stops at 89 degrees, so the Wing is a little off.  It is still made.
    vsp.VSPRenew()
    fid = new( "MDC_FIN", SWEEP_1=89.5 )
    n = len( vsp.FindGeoms() )

    wings = vsp.ConvertMdcFinToWing( fid, True )
    assert_no_errors()
    vsp.Update()

    assert len( wings ) == 4
    assert len( vsp.FindGeoms() ) == n + 3
    assert fid not in vsp.FindGeoms()


def testFinSetConvertsOnlyAFinSet():
    vsp.VSPRenew()
    pod = new( "POD" )

    drop_errors()
    assert len( vsp.ConvertMdcFinToWing( pod, False ) ) == 0
    assert len( pop_errors() ) == 1

    assert len( vsp.ConvertMdcFinToWing( "NOSUCHID", True ) ) == 0
    assert len( pop_errors() ) == 1


TESS_FINS = [
    ( "one segment", dict(), False, ( 5, ) ),
    ( "two segments", dict( NSPAN=3, SSPAN_3=2.5, CHORD_3=0.4, SWEEP_2=30.0 ), False, ( 12, 3 ) ),
    ( "buried root", dict(), True, ( 6, ) ),
    ( "buried root extrapolated", dict( NSPAN=3, SSPAN_3=2.5, CHORD_3=0.4, BuryRootType=1 ), True, ( 9, 2 ) ),
]


@pytest.mark.parametrize( "name,parms,bury,tess", TESS_FINS, ids=[ n for n, _, _, _ in TESS_FINS ] )
def testWingTakesTheFinSetsRowsBySegment( name, parms, bury, tess ):
    vsp.VSPRenew()
    parent = ""
    if bury:
        parent = new( "MDC_BOD" )
    fid = new( "MDC_FIN", parent, NPANEL=1, BuryRoot=int( bury ), **parms )
    put( fid, "Shape", **{ "SectTess_U_%d" % ( k + 1 ): t for k, t in enumerate( tess ) } )
    wid = vsp.ConvertMdcFinToWing( fid, False )[0]
    vsp.Update()

    # A Wing runs around its sections the other way.
    uf, _ = vsp.GetUWTess01( fid, 0 )
    uw, _ = vsp.GetUWTess01( wid, 0 )
    assert len( uf ) == len( uw )
    for a, b in zip( uf, uw ):
        for w in ( 0.1, 0.3, 0.6 ):
            assert ( vsp.CompPnt01( fid, 0, a, w ) - vsp.CompPnt01( wid, 0, b, 1.0 - w ) ).mag() < 1e-9


def testWingsFromAFinSetSaveAndReadBack():
    vsp.VSPRenew()
    fid = new( "MDC_FIN", SECTYP=1 )
    wings = vsp.ConvertMdcFinToWing( fid, False )
    vsp.Update()
    before = points( wings[0] )

    fname = os.path.join( tempfile.mkdtemp(), "wing.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.ClearVSPModel()
    vsp.ReadVSPFile( fname )
    vsp.Update()

    read = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Wing" ]
    assert len( read ) == 1
    assert apart( before, points( read[0] ) ) < 1e-9


def testWingsAndClonesSitAfterTheFinSetInOrder():
    vsp.VSPRenew()
    parent = new( "POD" )
    before = new( "POD", parent=parent )
    fid = new( "MDC_FIN", parent=parent )
    after = new( "POD", parent=parent )

    wings = vsp.ConvertMdcFinToWing( fid, False )
    assert_no_errors()
    vsp.Update()

    assert list( vsp.GetGeomChildren( parent ) ) == [ before, fid ] + list( wings ) + [ after ]
