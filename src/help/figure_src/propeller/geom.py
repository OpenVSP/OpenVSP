"""Blade geometry from the openvsp module, for the figures.

lofted_blade() returns the blade as PropPositioner and the loft leave it -- before any rigid
motion -- by asking VSP for blade 1 with every rigid motion zero.  place() and the figures then
apply propmodel's chain; check_pose() proves on each call that the result is what VSP itself
builds with the same parms.
"""

import numpy as np
import openvsp as vsp

import propmodel as pm

RADIUS = 15.0

# The rigid motions zeroed to get the lofted blade.
RIGID_ZERO = { "Rotate": 0.0, "PsiRef": 0.0, "Precone": 0.0, "FlapAngle": 0.0, "LagAngle": 0.0, "Feather": 0.0,
               "FoldAngle": 0.0, "BladeMotionMode": 0.0, "IndividualBladeFoldFlag": 0.0 }


# The propeller's own parms: the older ones in Design, the articulated rotor's in Articulation.
GROUPS = ( "Design", "Articulation" )

# Samples along the span (u): VSP's own tessellation, clustered toward the tip; and around the section (w).
TESS_U = 81
TIP_CLUSTER = 0.25
CAP_TESS = 9
NW = 49

# A generic helicopter blade: constant chord, no twist, symmetric sections, flat tip.
ROTOR_CHORD = 0.07
ROTOR_THICK = 0.12


def parm_ids( pid ):
    """The propeller's own parms by name: those in the Design and Articulation groups."""
    return { vsp.GetParmName( q ): q for q in vsp.GetGeomParmIDs( pid ) if vsp.GetParmGroupName( q ) in GROUPS }


def make_rotor( pid, root ):
    """Make pid's blade the plain rectangular rotor blade."""
    r0 = root
    if r0 is None:
        r0 = 0.2
    xs = vsp.GetXSecSurf( pid, 0 )
    for i in range( vsp.GetNumXSec( xs ) ):
        vsp.ChangeXSecShape( xs, i, vsp.XS_ONE_SIX_SERIES )
    for curve, val in ( ( vsp.PROP_CHORD, ROTOR_CHORD ), ( vsp.PROP_TWIST, 0.0 ), ( vsp.PROP_THICK, ROTOR_THICK ),
                        ( vsp.PROP_CLI, 0.0 ) ):
        vsp.SetPCurve( pid, curve, [ r0, 1.0 ], [ val, val ], vsp.LINEAR )
    vsp.SetParmVal( pid, "CapUMaxOption", "EndCap", vsp.FLAT_END_CAP )


def new_prop( parms, rev = False, root = None, xsec_parms = None, curves = None, rotor = False, pcurves = None ):
    """A fresh PROP with parms set (counts first), optional root radius, parm values and PCurves."""
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( pid, "UseBeta34Flag", "Design", 0.0 )
    vsp.SetParmVal( pid, "ReverseFlag", "Design", float( rev ) )
    vsp.Update()
    if root is not None:
        xs = vsp.GetXSecSurf( pid, 0 )
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xs, 0 ), "RadiusFrac" ), root )
    if rotor:
        make_rotor( pid, root )
    if curves:
        for ( group, name ), val in curves.items():
            vsp.SetParmVal( pid, name, group, val )
    if pcurves:
        for curve, ( rs, vals, kind ) in pcurves.items():
            vsp.SetPCurve( pid, curve, list( rs ), list( vals ), kind )
    vsp.SetParmVal( pid, "Tess_U", "Shape", TESS_U )
    vsp.SetParmVal( pid, "OutCluster", "Design", TIP_CLUSTER )
    vsp.SetParmVal( pid, "CapUMinTess", "EndCap", CAP_TESS )
    vsp.Update()
    # Counts first: NumBlade and NumHarmonics make the per-blade and harmonic parms in an Update.
    for first in ( True, False ):
        ids = parm_ids( pid )
        for k, v in parms.items():
            if k.startswith( "Num" ) == first:
                assert k in ids, k
                vsp.SetParmVal( ids[ k ], v )
        vsp.Update()
    return pid


def span_u( pid ):
    """The u01 of VSP's own tessellation: the blade runs from 1/3 (root) to 2/3 (tip), the caps outside it."""
    u = np.array( vsp.GetUWTess01( pid, 0 )[0] )
    assert np.abs( u - 1.0 / 3.0 ).min() < 1e-12 and np.abs( u - 2.0 / 3.0 ).min() < 1e-12
    return u


def section_w( nw ):
    """w around the section, TE -> LE -> TE, bunched at the LE (w = 0.5) and the TE; nw odd keeps the LE at nw // 2."""
    assert nw % 2 == 1
    t = np.linspace( 0.0, 1.0, nw // 2 + 1 )
    half = 0.25 * ( 1.0 - np.cos( np.pi * t ) )
    return np.concatenate( [ half, 1.0 - half[ -2::-1 ] ] )


def grid( pid, isurf, u, nw = NW ):
    """Surface isurf sampled at span stations u and nw section stations bunched at the LE and TE."""
    w = section_w( nw )
    return np.array( [ [ list( vsp.CompPnt01( pid, isurf, a, b ) ) for b in w ] for a in u ] )


def design_parms( pid ):
    """The values of the propeller's own parms by name."""
    return { k: vsp.GetParmVal( q ) for k, q in parm_ids( pid ).items() }


class Prop:
    """One illustration propeller: its parms, lofted blade and root chord."""

    def __init__( self, parms, rev = False, root = None, curves = None, nw = NW, rotor = False, pcurves = None ):
        self.parms_in = dict( parms )
        self.rev = rev
        self.root = root
        self.curves = curves
        self.nw = nw
        lofted = dict( parms )
        lofted.update( RIGID_ZERO )
        pid = new_prop( lofted, rev, root, curves = curves, rotor = rotor, pcurves = pcurves )
        self.u = span_u( pid )
        self.nu = len( self.u )
        self.G0 = grid( pid, 0, self.u, nw )
        self.croot = vsp.GetParmVal( pid, "crd_0", "Chord" ) * RADIUS
        pid = new_prop( parms, rev, root, curves = curves, rotor = rotor, pcurves = pcurves )
        self.p = design_parms( pid )
        self.nblade = int( round( self.p[ "NumBlade" ] ) )
        assert np.array_equal( span_u( pid ), self.u )
        self.vsp_blades = [ grid( pid, i, self.u, nw ) for i in range( self.nblade ) ]

    def chain( self, iblade = 0, angles = None, parms = None ):
        p = dict( self.p )
        if parms:
            p.update( parms )
        return pm.chain( p, RADIUS, self.croot, iblade, angles = angles )

    def hinges( self, parms = None ):
        p = dict( self.p )
        if parms:
            p.update( parms )
        return pm.hinges_prop( p, RADIUS, self.croot )

    def check_pose( self, tol = 1e-9 ):
        """Model chain on the lofted blade == VSP's own blades."""
        worst = 0.0
        for i in range( self.nblade ):
            M = pm.product( self.chain( i ) )
            worst = max( worst, np.abs( pm.apply( M, self.G0 ) - self.vsp_blades[ i ] ).max() )
        assert worst < tol, worst
        return worst

    def iu( self, frac ):
        """The span sample nearest frac of the way from root to tip."""
        return int( np.argmin( np.abs( self.u - ( 1.0 + frac ) / 3.0 ) ) )
