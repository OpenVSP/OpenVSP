"""Check propmodel.py, and every sign the figures state, against the openvsp module.

Run with the build's own venv:
    <build>/venv/bin/python verify.py

Writes nothing.  Exits non-zero on the first failed check.
"""

import sys
import time

import numpy as np
import openvsp as vsp

import propmodel as pm
from geom import parm_ids

RADIUS = 15.0
U = np.linspace( 0.02, 0.98, 9 )
W = np.linspace( 0.0, 1.0, 13 )
FAILS = []


def check( ok, what ):
    """Record one check."""
    print( "%-4s %s" % ( "ok" if ok else "FAIL", what ) )
    if not ok:
        FAILS.append( what )


def new_prop( parms = None, rev = False ):
    """A fresh PROP with the given parms, Feather 0 unless set."""
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( pid, "UseBeta34Flag", "Design", 0.0 )
    vsp.SetParmVal( pid, "Feather", "Design", 0.0 )
    vsp.SetParmVal( pid, "ReverseFlag", "Design", float( rev ) )
    vsp.Update()
    if parms:
        setp( pid, parms )
    return pid


def setp( pid, parms ):
    """Set parms by name and update."""
    for k, v in parms.items():
        q = parm_ids( pid ).get( k )
        assert q, k
        vsp.SetParmVal( q, v )
    vsp.Update()


def getp( pid ):
    """A parm's value by name."""
    return { k: vsp.GetParmVal( q ) for k, q in parm_ids( pid ).items() }


def surf( pid, isurf, u = U, w = W ):
    """Surface isurf sampled on the U x W grid."""
    return np.array( [ [ list( vsp.CompPnt01( pid, isurf, a, b ) ) for b in w ] for a in u ] )


def croot_of( pid ):
    """The root chord, absolute."""
    return vsp.GetParmVal( pid, "crd_0", "Chord" ) * RADIUS


def fit_rotation( P, Q ):
    """Rigid fit Q = R P + t; returns R, t."""
    P = P.reshape( -1, 3 )
    Q = Q.reshape( -1, 3 )
    pc = P.mean( 0 )
    qc = Q.mean( 0 )
    H = ( P - pc ).T @ ( Q - qc )
    U_, S, Vt = np.linalg.svd( H )
    d = np.sign( np.linalg.det( Vt.T @ U_.T ) )
    D = np.diag( [ 1, 1, d ] )
    Rm = Vt.T @ D @ U_.T
    return Rm, qc - Rm @ pc


def rot_axis_angle( Rm ):
    """The axis and angle of a rotation matrix."""
    ang = np.degrees( np.arccos( np.clip( ( np.trace( Rm ) - 1 ) / 2, -1, 1 ) ) )
    ax = np.array( [ Rm[2,1] - Rm[1,2], Rm[0,2] - Rm[2,0], Rm[1,0] - Rm[0,1] ] )
    return ax / np.linalg.norm( ax ), ang


# ------------------------------------------------------------------ 1. the chain

def check_chain():
    """The model's blade chain against VSP's blades, with every hinge set."""
    hinge = { "RFlapAx": 0.06, "AxFlapAx": 0.01, "OffFlapAx": 0.02, "AzFlapDir": 12.0, "ElFlapDir": 25.0,
              "RLagAx": 0.11, "AxLagAx": -0.015, "OffLagAx": 0.01, "AzLagDir": -7.0, "CantLagDir": 18.0,
              "RFoldAx": 0.3, "AxFoldAx": 0.02, "OffFoldAx": -0.03, "AzFoldDir": 9.0, "ElFoldDir": -14.0,
              "FeatherAxisXoC": 0.3, "FeatherOffsetXoC": 0.15 }
    defl = { "FlapAngle": 11.0, "LagAngle": 8.0, "FoldAngle": 27.0, "Feather": 14.0,
             "Precone": 6.0, "Rotate": 23.0 }
    cases = [ ( "default stations", {} ),
              ( "feather outboard of fold (legacy RFeatherAx 1)", { "RFeatherAx": 1.0 } ),
              ( "all four at one station (tie order)", { "RFlapAx": 0.1, "RLagAx": 0.1, "RFeatherAx": 0.1, "RFoldAx": 0.1 } ),
              ( "all four at the hub, r/R 0", { "RFlapAx": 0.0, "RLagAx": 0.0, "RFeatherAx": 0.0, "RFoldAx": 0.0 } ),
              ( "lag inboard of flap", { "RFlapAx": 0.12, "RLagAx": 0.04 } ) ]
    for rev in ( False, True ):
        for label, extra in cases:
            pid = new_prop( dict( hinge, **extra ), rev )
            p0 = surf( pid, 0 )
            cr = croot_of( pid )
            setp( pid, defl )
            p = getp( pid )
            worst = 0.0
            for ib in range( 3 ):
                M = pm.product( pm.chain( p, RADIUS, cr, ib ) )
                worst = max( worst, np.abs( surf( pid, ib ) - pm.apply( M, p0 ) ).max() )
            M = pm.product( pm.chain( p, RADIUS, cr, 0, order = pm.TIE_ORDER ) )
            dold = np.abs( surf( pid, 0 ) - pm.apply( M, p0 ) ).max()
            check( worst < 1e-9, "chain model == vsp, rev=%d, %s, order %s (max err %.1e; fixed order flap,lag,fold,feather would be off by %.3f)"
                   % ( rev, label, pm.hinge_order( p ), worst, dold ) )


# ------------------------------------------------------------------ 1b. azimuth, PsiRef and the blade motion modes

def mat_of( m ):
    """A Matrix4d as a 4x4 array acting on column vectors."""
    o = m.xform( vsp.vec3d( 0.0, 0.0, 0.0 ) )
    M = np.eye( 4 )
    M[ :3, 3 ] = [ o.x(), o.y(), o.z() ]
    for j, e in enumerate( ( ( 1.0, 0.0, 0.0 ), ( 0.0, 1.0, 0.0 ), ( 0.0, 0.0, 1.0 ) ) ):
        q = m.xform( vsp.vec3d( *e ) )
        M[ :3, j ] = [ q.x() - o.x(), q.y() - o.y(), q.z() - o.z() ]
    return M


def check_motion():
    """Each motion mode against VSP: collective, individual and harmonic."""
    hinge = { "RFlapAx": 0.06, "ElFlapDir": 20.0, "AzFlapDir": 8.0, "RLagAx": 0.11, "CantLagDir": 12.0,
              "RFoldAx": 0.3, "AzFoldDir": 9.0, "Precone": 4.0, "Rotate": 37.0, "PsiRef": 23.0 }
    harmonic = { "BladeMotionMode": pm.HARMONIC, "FlapMean": 3.0, "FlapCos_1": 4.0, "FlapSin_1": -2.0, "FlapCos_2": 1.0,
                 "FlapSin_2": 0.5, "LagMean": -2.0, "LagCos_1": 1.5, "LagSin_1": 2.5, "FeatherMean": 9.0,
                 "FeatherCos_1": -3.0, "FeatherSin_1": 2.0, "FeatherSin_2": -1.0 }
    individual = { "BladeMotionMode": pm.INDIVIDUAL, "IndividualBladeFoldFlag": 1.0, "FlapAngle": 5.0, "FlapAngle_1": -3.0,
                   "FlapAngle_2": 7.0, "LagAngle_2": 4.0, "Feather_1": 12.0, "FoldAngle": 10.0, "FoldAngle_2": 25.0 }
    beta34 = dict( individual, UseBeta34Flag = 1.0, Beta34 = 30.0 )
    cases = [ ( "harmonic, 2 harmonics", harmonic ), ( "individual, fold Ind.", individual ),
              ( "individual with Beta34 driving", beta34 ) ]
    for rev in ( False, True ):
        for label, parms in cases:
            pid = new_prop( { "NumBlade": 3, "NumHarmonics": 2, "FeatherOffsetXoC": 0.15 }, rev )
            p0 = surf( pid, 0 )
            cr = croot_of( pid )
            setp( pid, hinge )
            setp( pid, parms )
            p = getp( pid )
            worst = 0.0
            for ib in range( 3 ):
                M = pm.product( pm.chain( p, RADIUS, cr, ib ) )
                worst = max( worst, np.abs( surf( pid, ib ) - pm.apply( M, p0 ) ).max() )
            check( worst < 1e-9, "motion model == vsp, rev=%d, %s, PsiRef %g (max err %.1e)" % ( rev, label, p[ "PsiRef" ], worst ) )
            # GetPropBladeXForm at an angle Rotate never had: the model with rotate in its place.
            worst = 0.0
            for ib in range( 3 ):
                Mv = mat_of( vsp.GetPropBladeXForm( pid, ib, 0, 410.0 ) )
                Mm = pm.product( pm.chain( p, RADIUS, cr, ib, rotate = 410.0 ) )
                worst = max( worst, np.abs( Mv - Mm ).max() )
            check( worst < 1e-9, "GetPropBladeXForm( psi = 410 ) == model, rev=%d, %s (max err %.1e)" % ( rev, label, worst ) )


# ------------------------------------------------------------------ 2. single-joint senses

def tip_le_te( pid, isurf = 0 ):
    """The tip's LE and TE points."""
    le = np.array( list( vsp.CompPnt01( pid, isurf, 1.0, 0.5 ) ) )
    te = np.array( list( vsp.CompPnt01( pid, isurf, 1.0, 0.0 ) ) )
    return le, te


def measured_axis( pid, parm, val, isurf = 0 ):
    """The right-handed axis and angle of the rigid motion that parm = val makes."""
    p0 = surf( pid, isurf )
    setp( pid, { parm: val } )
    p1 = surf( pid, isurf )
    setp( pid, { parm: 0.0 } )
    Rm, t = fit_rotation( p0, p1 )
    resid = np.abs( pm.apply( np.block( [ [ Rm, t[:, None] ], [ np.zeros( ( 1, 3 ) ), np.ones( ( 1, 1 ) ) ] ] ), p0 ) - p1 ).max()
    ax, ang = rot_axis_angle( Rm )
    return ax, ang, resid, p0, p1


def check_senses():
    """The sense of each positive angle the page states."""
    for rev in ( False, True ):
        r = 1.0 if not rev else -1.0
        pid = new_prop( { "Precone": 0.0 }, rev )
        le, te = tip_le_te( pid )
        check( np.sign( le[2] ) == -r and np.sign( te[2] ) == r, "rev=%d: LE at %sZ, TE at %sZ" % ( rev, "-" if r > 0 else "+", "+" if r > 0 else "-" ) )
        # Axes of rotation are pseudovectors: they map with BladeAxisToProp.
        xB = pm.blade_axis_to_prop( [ 1, 0, 0 ], r )
        yB = pm.blade_axis_to_prop( [ 0, 1, 0 ], r )
        zB = pm.blade_axis_to_prop( [ 0, 0, 1 ], r )
        # Each joint alone: the measured right-handed axis, compared to the stated one.
        for parm, want, label in ( ( "FlapAngle", xB, "flap axis +x_B" ),
                                   ( "LagAngle", -zB, "lag axis -z_B" ),
                                   ( "FoldAngle", -xB, "fold axis -x_B" ),
                                   ( "Feather", yB, "feather axis +y_B" ),
                                   ( "Precone", np.array( [ 0, 0, 1.0 ] ), "precone axis +Z" ),
                                   ( "Rotate", np.array( [ -r, 0, 0 ] ), "Rotate axis -rev*X" ) ):
            ax, ang, resid, p0, p1 = measured_axis( pid, parm, 20.0 )
            check( resid < 1e-9 and abs( ang - 20 ) < 1e-6 and np.allclose( ax, want, atol = 1e-9 ),
                   "rev=%d: +%s turns right-handed about %s (meas %s, %.3f deg)" % ( rev, parm, label, np.round( ax, 6 ), ang ) )
            dtip = p1[ -1 ].mean( 0 ) - p0[ -1 ].mean( 0 )
            print( "       tip moves by %s" % np.round( dtip, 3 ) )
        # Meanings in words.
        def tipmove( parm ):
            p0 = surf( pid, 0 )[ -1 ].mean( 0 )
            setp( pid, { parm: 10.0 } )
            p1 = surf( pid, 0 )[ -1 ].mean( 0 )
            setp( pid, { parm: 0.0 } )
            return p1 - p0
        check( tipmove( "FlapAngle" )[0] < 0, "rev=%d: +FlapAngle moves the tip toward the thrust (-X)" % rev )
        check( tipmove( "Precone" )[0] < 0, "rev=%d: +Precone moves the tip toward the thrust (-X)" % rev )
        check( tipmove( "FoldAngle" )[0] > 0, "rev=%d: +FoldAngle moves the tip downstream (+X)" % rev )
        check( r * tipmove( "LagAngle" )[2] > 0, "rev=%d: +LagAngle moves the tip aft (toward the TE, rev*Z)" % rev )
        check( r * tipmove( "Rotate" )[2] < 0, "rev=%d: +Rotate moves blade 1 toward its LE (the direction of rotation)" % rev )
        le0, _ = tip_le_te( pid )
        setp( pid, { "Feather": 10.0 } )
        le1, _ = tip_le_te( pid )
        setp( pid, { "Feather": 0.0 } )
        check( le1[0] < le0[0], "rev=%d: +Feather moves the LE toward the thrust (-X)" % rev )
        # Blade 2 sits at +BladeAz_2 in the direction of rotation.
        b1 = surf( pid, 0 )[ -1 ].mean( 0 )
        b2 = surf( pid, 1 )[ -1 ].mean( 0 )
        az = np.degrees( np.arctan2( -r * b2[2], b2[1] ) - np.arctan2( -r * b1[2], b1[1] ) )
        check( abs( az - 120.0 ) < 1e-6, "rev=%d: blade 2 lies 120 deg from blade 1 in the direction of rotation (meas %.6f)" % ( rev, az ) )


# ------------------------------------------------------------------ 3. hinge orientation and coupling

def pitch_change( pid, setup, parm, val, isurf = 0 ):
    """Pitch change of the blade: the component of the rigid rotation about the (moved) radial axis
    y_B, with positive pitch = LE toward thrust."""
    setp( pid, setup )
    p0 = surf( pid, isurf )
    setp( pid, { parm: val } )
    p1 = surf( pid, isurf )
    setp( pid, { parm: 0.0 } )
    Rm, _ = fit_rotation( p0, p1 )
    return Rm


def euler_flap_lag_pitch( Rm, rev ):
    """Decompose a rotation given in propeller axes into blade-axes angles R = Rx(beta) Rz(-zeta) Ry(theta)
    (flap about x_B, lag about -z_B, pitch about y_B) and return them."""
    Bm = pm.B_matrix( rev )
    Rb = Bm.T @ Rm @ Bm
    # Rb = Rx(b) Rz(-z) Ry(t).  Column y of Rb: Rx(b) Rz(-z) e_y = Rx(b) ( sin z, cos z, 0 )
    zeta = np.degrees( np.arcsin( np.clip( Rb[0, 1], -1, 1 ) ) )
    beta = np.degrees( np.arctan2( Rb[2, 1], Rb[1, 1] ) )
    A = ( pm.R3( [ 1, 0, 0 ], beta ) @ pm.R3( [ 0, 0, 1 ], -zeta ) ).T @ Rb
    theta = np.degrees( np.arctan2( A[0, 2], A[0, 0] ) )
    return beta, zeta, theta


def check_coupling():
    """Pitch-flap and pitch-lag coupling from a skewed or canted hinge."""
    for rev in ( False, True ):
        r = -1.0 if rev else 1.0
        pid = new_prop( {}, rev )
        b = 10.0
        for d3 in ( 30.0, -30.0 ):
            Rm = pitch_change( pid, { "ElFlapDir": d3, "AzFlapDir": 0.0 }, "FlapAngle", b )
            beta, zeta, theta = euler_flap_lag_pitch( Rm, r )
            # Small angle: d(theta)/d(beta) = -tan(delta_3).
            check( np.sign( theta ) == -np.sign( d3 ) and beta > 0,
                   "rev=%d: flap %+.0f with delta_3=%+.0f gives pitch %+.3f (-beta tan d3 = %+.3f) and flap %.3f"
                   % ( rev, b, d3, theta, -b * np.tan( np.radians( d3 ) ), beta ) )
        setp( pid, { "ElFlapDir": 0.0 } )
        for a1 in ( 30.0, -30.0 ):
            Rm = pitch_change( pid, { "CantLagDir": a1, "AzLagDir": 0.0 }, "LagAngle", b )
            beta, zeta, theta = euler_flap_lag_pitch( Rm, r )
            check( np.sign( theta ) == -np.sign( a1 ) and zeta > 0,
                   "rev=%d: lag %+.0f with alpha_1=%+.0f gives pitch %+.3f (-zeta tan a1 = %+.3f) and lag %.3f"
                   % ( rev, b, a1, theta, -b * np.tan( np.radians( a1 ) ), zeta ) )
        setp( pid, { "CantLagDir": 0.0 } )
        # theta_F, theta_L, fold Az: the axis turns about +y_B (the sense of positive pitch).
        for parm, angparm, base in ( ( "AzFlapDir", "FlapAngle", [ 1, 0, 0 ] ), ( "AzLagDir", "LagAngle", [ 0, 0, -1 ] ),
                                     ( "AzFoldDir", "FoldAngle", [ -1, 0, 0 ] ) ):
            setp( pid, { parm: 25.0 } )
            ax, ang, resid, _, _ = measured_axis( pid, angparm, 20.0 )
            setp( pid, { parm: 0.0 } )
            # Turned the way positive pitch turns a body vector: rotation about +y_B, mapped as a vector.
            want = pm.blade_to_prop( pm.R3( [ 0, 1, 0 ], 25.0 ) @ np.array( base, float ), r ) * r
            check( np.allclose( ax, want, atol = 1e-9 ), "rev=%d: %s=+25 turns the %s axis like +pitch (meas %s)" % ( rev, parm, angparm, np.round( ax, 4 ) ) )
        # Fold elevation: about the shaft, the same sense as flap's delta_3.
        setp( pid, { "ElFoldDir": 25.0 } )
        ax, ang, resid, _, _ = measured_axis( pid, "FoldAngle", 20.0 )
        setp( pid, { "ElFoldDir": 0.0 } )
        want = pm.blade_to_prop( pm.R3( [ 0, 0, 1 ], -25.0 ) @ np.array( [ -1.0, 0, 0 ] ), r ) * r
        check( np.allclose( ax, want, atol = 1e-9 ), "rev=%d: ElFoldDir=+25 axis = Rz(-25)(-x_B) (meas %s)" % ( rev, np.round( ax, 4 ) ) )


# ------------------------------------------------------------------ 4. mirror

def check_mirror():
    """Reverse makes the mirror image in Z = 0, and with PsiRef 180 more, in Y = 0."""
    parms = { "RFlapAx": 0.06, "OffFlapAx": 0.02, "AzFlapDir": 12.0, "ElFlapDir": 25.0, "FlapAngle": 9.0,
              "CantLagDir": 18.0, "AzLagDir": -6.0, "LagAngle": 7.0, "FoldAngle": 20.0, "ElFoldDir": 10.0,
              "OffFoldAx": 0.02, "Feather": 12.0, "FeatherOffsetXoC": 0.2, "FeatherAxisXoC": 0.3,
              "Precone": 5.0, "Rotate": 17.0 }
    a = new_prop( parms, False )
    pa = [ surf( a, i ) for i in range( 3 ) ]
    b = new_prop( parms, True )
    pb = [ surf( b, i ) for i in range( 3 ) ]
    S = np.array( [ 1, 1, -1.0 ] )
    worst = max( np.abs( pa[i] * S - pb[i] ).max() for i in range( 3 ) )
    check( worst < 1e-9, "Reverse is the mirror z -> -z of the normal prop (max err %.1e)" % worst )
    c = new_prop( dict( parms, PsiRef = 180.0 ), True )
    pc = [ surf( c, i ) for i in range( 3 ) ]
    S = np.array( [ 1, -1.0, 1 ] )
    worst = max( np.abs( pa[i] * S - pc[i] ).max() for i in range( 3 ) )
    check( worst < 1e-9, "Reverse with PsiRef + 180 is the mirror y -> -y of the normal prop (max err %.1e)" % worst )



# ------------------------------------------------------------------ 5. shaping

def set_curve( pid, group, prefix, vals ):
    """Set one blade curve."""
    for i, v in enumerate( vals ):
        vsp.SetParmVal( pid, "%s_%d" % ( prefix, i ), group, v )


def check_shaping():
    """The model's section placement against VSP's sections."""
    for rev in ( False, True ):
        r = -1.0 if rev else 1.0
        pid = new_prop( {}, rev )
        # Linear curves, so every value at r/R = 0.6 is known exactly.
        vsp.SetParmVal( pid, "CrvType", "Twist", 0.0 )
        set_curve( pid, "Twist", "tw", [ 50 - 40 * 0.2, 50 - 40 * 0.75, 50 - 40 * 1.0 ] )
        set_curve( pid, "Rake", "rak", [ 0.02, 0.10 ] )
        set_curve( pid, "Skew", "skw", [ -0.01, 0.07 ] )
        set_curve( pid, "Sweep", "sw", [ 0.0, 12.0 ] )
        set_curve( pid, "Axial", "ax", [ 0.01, 0.04 ] )
        set_curve( pid, "Tangential", "tan", [ 0.0, -0.05 ] )
        setp( pid, { "ConstructXoC": 0.25, "FeatherAxisXoC": 0.35, "FeatherOffsetXoC": 0.2 } )
        rr = 0.6

        def lin( a0, a1 ):
            return a0 + ( a1 - a0 ) * ( rr - 0.2 ) / 0.8

        s = { "Chord": 0.2 * RADIUS, "Reverse": r, "Construct": 0.25, "FeatherAxis": 0.35,
              "FeatherOffset": 0.2, "RootChord": 0.08 * RADIUS, "RootTwist": 50 - 40 * 0.2,
              "Twist": 50 - 40 * rr,
              "ZRotate": np.degrees( np.arctan( -( 0.10 - 0.02 ) / 0.8 - ( 0.04 - 0.01 ) / 0.8 ) ),
              "Rake": lin( 0.02, 0.10 ) * RADIUS, "Skew": lin( -0.01, 0.07 ) * RADIUS,
              "Sweep": lin( 0.0, 12.0 ), "Axial": lin( 0.01, 0.04 ) * RADIUS,
              "Tangential": lin( 0.0, -0.05 ) * RADIUS, "Radius": rr * RADIUS }
        pts = pm.shape_section( s, np.array( [ [ 0.0, 0.0 ], [ 1.0, 0.0 ] ] ) )
        for name, pt, wwant in ( ( "LE", pts[0], 0.5 ), ( "TE", pts[1], 0.0 ) ):
            d, si, u, w = vsp.ProjPnt01I( pid, vsp.vec3d( *pt ) )
            wok = min( abs( w - wwant ), abs( w - 1.0 ) if wwant == 0.0 else 1.0 ) < 2e-3
            check( d < 1e-6 and si == 0 and wok,
                   "rev=%d: shaping model %s at r/R=0.6 lies on the blade (dist %.1e, surf %d, w %.4f)" % ( rev, name, d, si, w ) )
        # The same with the sections wrapped onto their cylinders.
        vsp.SetParmVal( pid, "CylindricalSectionsFlag", "Design", 1.0 )
        vsp.Update()
        s[ "Cylinder" ] = True
        pts = pm.shape_section( s, np.array( [ [ 0.0, 0.0 ], [ 1.0, 0.0 ] ] ) )
        for name, pt in ( ( "LE", pts[0] ), ( "TE", pts[1] ) ):
            d, si, u, w = vsp.ProjPnt01I( pid, vsp.vec3d( *pt ) )
            check( d < 1e-6, "rev=%d: cylinder-wrapped %s at r/R=0.6 lies on the blade (dist %.1e)" % ( rev, name, d ) )
    # Single-parm senses at r/R = 0.6 (normal rotation): which way the LE moves.
    for group, prefix, vals, what in ( ( "Rake", "rak", [ 0.1, 0.1 ], "+Rake moves the section downstream (+X)" ),
                                       ( "Axial", "ax", [ 0.1, 0.1 ], "+Axial moves the section downstream (+X)" ),
                                       ( "Skew", "skw", [ 0.1, 0.1 ], "+Skew moves the section aft (+rev Z)" ),
                                       ( "Tangential", "tan", [ 0.1, 0.1 ], "+Tangential moves the section aft (+rev Z)" ),
                                       ( "Sweep", "sw", [ 10.0, 10.0 ], "+Sweep turns the section aft (+rev Z) about the shaft" ) ):
        for rev in ( False, True ):
            r = -1.0 if rev else 1.0
            pid = new_prop( {}, rev )
            vsp.SetParmVal( pid, "CrvType", "Twist", 0.0 )
            set_curve( pid, "Twist", "tw", [ 0, 0, 0 ] )
            vsp.Update()
            a = np.array( list( vsp.CompPnt01( pid, 0, 0.6, 0.5 ) ) )
            set_curve( pid, group, prefix, vals )
            vsp.Update()
            b = np.array( list( vsp.CompPnt01( pid, 0, 0.6, 0.5 ) ) )
            d = b - a
            if "X" in what:
                ok = d[0] > 0 and abs( d[2] ) < 1e-9
            else:
                ok = r * d[2] > 0 and abs( d[0] ) < 1e-9
            check( ok, "rev=%d: %s (dLE %s)" % ( rev, what, np.round( d, 4 ) ) )
    pid = new_prop( {}, False )
    vsp.SetParmVal( pid, "CrvType", "Twist", 0.0 )
    set_curve( pid, "Twist", "tw", [ 0, 0, 0 ] )
    vsp.Update()
    a = np.array( list( vsp.CompPnt01( pid, 0, 0.5, 0.25 ) ) )
    b = np.array( list( vsp.CompPnt01( pid, 0, 0.5, 0.75 ) ) )
    print( "     w=0.25 X=%.4f, w=0.75 X=%.4f (the w > 0.5 half is %s)" % ( a[0], b[0], "the -X (thrust) side" if b[0] < a[0] else "the +X side" ) )


def max_radius( pid ):
    """The largest distance of the blade surface from the shaft."""
    P = surf( pid, 0, np.linspace( 0.0, 1.0, 201 ), np.linspace( 0.0, 1.0, 41 ) )
    return np.hypot( P[ :, :, 1 ], P[ :, :, 2 ] ).max()


def te_gap( pid, r ):
    """Trailing edge thickness and chord of the section at r/R, on an uncapped, untwisted blade."""
    u = ( r - 0.2 ) / 0.8
    P = np.array( [ list( vsp.CompPnt01( pid, 0, u, j / 2000.0 ) ) for j in range( 2001 ) ] )
    zmax = P[ :, 2 ].max()
    ch = zmax - P[ :, 2 ].min()
    near = P[ P[ :, 2 ] > zmax - 1e-4 * ch, 0 ]
    return near.max() - near.min(), ch


def check_page_claims():
    """Diameter under Tangential and Axial, Abs and Rel closures, Disk mode update time."""
    base = {}
    for name, crv in ( ( "none", None ), ( "Tangential", vsp.PROP_TANGENTIAL ), ( "Skew", vsp.PROP_SKEW ),
                       ( "Axial", vsp.PROP_AXIAL ), ( "Rake", vsp.PROP_RAKE ) ):
        pid = new_prop( {}, False )
        vsp.SetPCurve( pid, vsp.PROP_TWIST, [ 0.2, 1.0 ], [ 0.0, 0.0 ], vsp.LINEAR )
        if crv is not None:
            vsp.SetPCurve( pid, crv, [ 0.2, 1.0 ], [ 0.3, 0.3 ], vsp.LINEAR )
        vsp.Update()
        base[ name ] = max_radius( pid )
    for name in ( "Tangential", "Skew" ):
        check( base[ name ] > base[ "none" ] + 0.5, "%s 0.3 moves the blade beyond the Diameter (max r %.3f vs %.3f)"
               % ( name, base[ name ], base[ "none" ] ) )
    for name in ( "Axial", "Rake" ):
        check( abs( base[ name ] - base[ "none" ] ) < 1e-6, "%s 0.3 keeps the blade's radius (max r %.6f vs %.6f)"
               % ( name, base[ name ], base[ "none" ] ) )

    for absrel in ( vsp.ABS, vsp.REL ):
        pid = new_prop( {}, False )
        vsp.SetPCurve( pid, vsp.PROP_TWIST, [ 0.2, 1.0 ], [ 0.0, 0.0 ], vsp.LINEAR )
        vsp.SetPCurve( pid, vsp.PROP_CHORD, [ 0.2, 1.0 ], [ 0.05, 0.25 ], vsp.LINEAR )
        vsp.SetParmVal( pid, "CapUMaxOption", "EndCap", 0.0 )
        vsp.SetParmVal( pid, "CapUMinOption", "EndCap", 0.0 )
        xs = vsp.GetXSecSurf( pid, 0 )
        for i in range( vsp.GetNumXSec( xs ) ):
            x = vsp.GetXSec( xs, i )
            vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_Type" ), vsp.CLOSE_SKEWBOTH )
            vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_AbsRel" ), absrel )
            vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_Thick" ), 0.1 )
            vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_Thick_Chord" ), 0.02 )
        vsp.Update()
        g = [ te_gap( pid, r ) for r in ( 0.3, 0.5, 0.7, 0.9 ) ]
        if absrel == vsp.ABS:
            ok = all( abs( a - 0.1 ) < 1e-3 for a, c in g )
            what = "Abs TE closure 0.1 keeps its thickness as the chord grows"
        else:
            ok = all( abs( a / c - 0.02 ) < 1e-3 for a, c in g )
            what = "Rel TE closure 0.02 stays a fraction of the local chord"
        check( ok, "%s (%s)" % ( what, ", ".join( "%.4f/%.3f" % ( a, c ) for a, c in g ) ) )

    t = {}
    for mode in ( vsp.PROP_BLADES, vsp.PROP_DISK ):
        vsp.VSPRenew()
        ps = [ vsp.AddGeom( "PROP" ) for k in range( 12 ) ]
        for q in ps:
            vsp.SetParmVal( q, "PropMode", "Design", mode )
        vsp.Update()
        t0 = time.time()
        for k in range( 5 ):
            for q in ps:
                vsp.SetParmVal( q, "Diameter", "Design", 31.0 + k )
            vsp.Update()
        t[ mode ] = time.time() - t0
    ratio = t[ vsp.PROP_BLADES ] / max( t[ vsp.PROP_DISK ], 1e-9 )
    check( ratio > 10.0, "twelve propellers update much faster in Disk mode (%.1f times; the page says about 25)" % ratio )


if __name__ == "__main__":
    check_chain()
    check_motion()
    check_senses()
    check_coupling()
    check_mirror()
    check_shaping()
    check_page_claims()
    print( "\n%d failed" % len( FAILS ) )
    for f in FAILS:
        print( "  FAIL", f )
    sys.exit( 1 if FAILS else 0 )
