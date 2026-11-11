"""A propeller's XRotor export: the restart file XRotor's LOAD reads, built from the BEM results."""

import math
import os
import tempfile

import openvsp as vsp


def export( pid, **settings ):
    veh = vsp.GetVehicleID()
    for name, val in settings.items():
        vsp.SetParmVal( vsp.FindParm( veh, name, "XRotorSettings" ), val )
    vsp.SetBEMPropID( pid )
    fname = os.path.join( tempfile.mkdtemp(), "prop.xrotor" )
    vsp.ExportFile( fname, vsp.SET_ALL, vsp.EXPORT_XROTOR )
    return fname


def read_xrotor( fname ):
    # The reads of XRotor's LOAD (xio.f): comment and blank lines skipped, list-directed values.
    with open( fname ) as f:
        lines = [ l.rstrip( "\n" ) for l in f ]
    assert all( len( l ) <= 128 for l in lines )
    assert lines[0].startswith( "XROTOR" )
    version = float( lines[0][16:22] )
    data = [ l for l in lines[1:] if l.strip() and l[0] not in "!#" ]
    out = { "version": version, "name": data[0] }
    out["rho"], out["vso"], out["rmu"], out["alt"] = map( float, data[1].split() )
    out["rad"], out["vel"], out["adv"], out["rake"] = map( float, data[2].split() )
    out["xi0"], out["xw0"] = map( float, data[3].split() )
    naero = int( data[4] )
    k = 5
    out["aero"] = []
    for n in range( naero ):
        xisect = float( data[k] )
        a0deg = float( data[k + 1].split()[0] )
        out["aero"].append( ( xisect, a0deg ) )
        k += 5
    out["flags"] = data[k].split()
    ii, nblds = map( int, data[k + 1].split() )
    out["nblds"] = nblds
    out["blade"] = [ tuple( map( float, data[k + 2 + i].split() ) ) for i in range( ii ) ]
    out["urduct"] = float( data[k + 2 + ii] )
    return out


def bem_results( pid ):
    vsp.SetBEMPropID( pid )
    vsp.ExportFile( os.path.join( tempfile.mkdtemp(), "prop.bem" ), vsp.SET_ALL, vsp.EXPORT_BEM )
    return vsp.FindLatestResultsID( "PropBEM" )


def test_blade_matches_bem():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( pid, "UseBeta34Flag", "Design", 0 )
    vsp.SetParmVal( pid, "Feather", "Design", 5.0 )
    vsp.SetParmVal( pid, "NumBlade", "Design", 4 )
    vsp.Update()
    rid = bem_results( pid )
    r = vsp.GetDoubleResults( rid, "Radius" )
    chord = vsp.GetDoubleResults( rid, "Chord" )
    twist = vsp.GetDoubleResults( rid, "Twist" )
    diam = vsp.GetDoubleResults( rid, "Diameter" )[0]

    xr = read_xrotor( export( pid, LenUnit = vsp.LEN_IN, Vinf = 20.0, RPM = 3000.0, Alt = 0.0 ) )

    assert xr["nblds"] == 4
    assert len( xr["blade"] ) == len( r )
    for ( xi, ch, beta, ubody ), ri, ci, ti in zip( xr["blade"], r, chord, twist ):
        assert abs( xi - ri ) < 1e-7 and abs( ch - ci ) < 1e-7
        assert abs( beta - ( ti + 5.0 ) ) < 1e-6
        assert ubody == 0.0
    assert abs( xr["xi0"] - r[0] ) < 1e-7

    rad = 0.5 * diam * 0.0254
    assert abs( xr["rad"] - rad ) < 1e-9
    assert abs( xr["adv"] - 20.0 / ( 3000.0 * math.pi / 30.0 * rad ) ) < 1e-7
    assert abs( xr["rho"] - 1.225 ) < 1e-3 and abs( xr["vso"] - 340.3 ) < 0.1


def test_zero_lift_angle_from_camber():
    # Thin NACA 4-digit sections: the zero-lift angle approaches thin-airfoil theory for the mean line.
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.Update()
    cli, p = 0.5, 0.4
    xsurf = vsp.GetXSecSurf( pid, 0 )
    for i in range( vsp.GetNumXSec( xsurf ) ):
        vsp.ChangeXSecShape( xsurf, i, vsp.XS_FOUR_SERIES )
        xs = vsp.GetXSec( xsurf, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "CamberInputFlag" ), vsp.DESIGN_CL )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "CamberLoc" ), p )
    vsp.SetPCurve( pid, vsp.PROP_CLI, [ 0.0, 1.0 ], [ cli, cli ], vsp.LINEAR )
    vsp.SetPCurve( pid, vsp.PROP_THICK, [ 0.0, 1.0 ], [ 0.01, 0.01 ], vsp.LINEAR )
    vsp.Update()

    xr = read_xrotor( export( pid, NumAero = 3 ) )

    # Mean line slope dz/dx integrated against ( cos(th) - 1 ), m from the four-digit CLi relation.
    m = cli / four_digit_cli( 1.0, p )
    n = 20000
    s = 0.0
    for k in range( n ):
        th = math.pi * ( k + 0.5 ) / n
        x = 0.5 * ( 1.0 - math.cos( th ) )
        if x < p:
            dz = 2.0 * m / p ** 2 * ( p - x )
        else:
            dz = 2.0 * m / ( 1.0 - p ) ** 2 * ( p - x )
        s += dz * ( math.cos( th ) - 1.0 ) * math.pi / n
    exact = -s / math.pi * 180.0 / math.pi

    assert len( xr["aero"] ) == 3
    for xisect, a0 in xr["aero"]:
        assert abs( a0 - exact ) < 0.05, ( xisect, a0, exact )


def four_digit_cli( m, p ):
    # Airfoil.cpp CalcFourDigitCLi.
    p2 = p * p
    p3 = p2 * p
    p4 = p2 * p2
    return -( ( m - 2 * m * p ) * math.sin( 2 * math.acos( 2 * p - 1 ) ) + ( 2 * m - 4 * m * p ) * math.acos( 2 * p - 1 ) +
              math.sqrt( 4 * p - 4 * p2 ) * ( 16 * m * p2 - 16 * m * p + 4 * m ) - 2 * math.pi * m * p2 +
              4 * math.pi * m * p - 2 * math.pi * m ) / ( 2 * p4 - 4 * p3 + 2 * p2 )


def test_symmetric_sections_have_no_zero_lift_angle():
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetPCurve( pid, vsp.PROP_CLI, [ 0.0, 1.0 ], [ 0.0, 0.0 ], vsp.LINEAR )
    vsp.Update()
    xr = read_xrotor( export( pid, NumAero = 4 ) )
    for xisect, a0 in xr["aero"]:
        assert abs( a0 ) < 1e-6, ( xisect, a0 )


def test_not_a_propeller_writes_nothing():
    vsp.VSPRenew()
    vsp.AddGeom( "POD" )
    vsp.Update()
    mgr = vsp.ErrorMgrSingleton.getInstance()
    while mgr.GetNumTotalErrors() > 0:
        mgr.PopLastError()
    fname = os.path.join( tempfile.mkdtemp(), "none.xrotor" )
    vsp.ExportFile( fname, vsp.SET_ALL, vsp.EXPORT_XROTOR )
    assert not os.path.exists( fname )
    assert mgr.GetNumTotalErrors() > 0
    while mgr.GetNumTotalErrors() > 0:
        mgr.PopLastError()
