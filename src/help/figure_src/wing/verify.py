"""Checks, against the openvsp module, the statements Wing.md makes about how a wing is built.

    <build>/venv/bin/python verify.py
"""

import numpy as np
import openvsp as vsp

import wingmodel as wm

TOL = 1e-9
D = np.radians


def close( a, b, tol = TOL, what = "" ):
    a = np.asarray( a, float )
    b = np.asarray( b, float )
    assert np.allclose( a, b, atol = tol, rtol = 0 ), ( what, a, b )


def foil_angle( w, i ):
    """Incidence of airfoil i, degrees: positive with the leading edge up."""
    le, te = wm.lete( w, i )
    return np.degrees( np.arctan2( le[2] - te[2], te[0] - le[0] ) )


def foil_normal( w, i ):
    """Unit normal of the plane airfoil i lies in."""
    F = wm.foil( w, i, 101 )
    P = F - F.mean( 0 )
    n = np.linalg.svd( P )[2][2]
    if n[1] < 0:
        n = -n
    return n


def section_planform():
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    b, cr, ct, sw = 9.0, 4.0, 1.0, 30.0
    close( [ wm.getx( w, 1, n ) for n in ( "Span", "Root_Chord", "Tip_Chord", "Sweep", "Sweep_Location", "Sec_Sweep_Location" ) ],
           [ b, cr, ct, sw, 0.0, 1.0 ], what = "default section" )
    close( wm.getx( w, 1, "Area" ), b * ( cr + ct ) / 2, what = "area" )
    close( wm.getx( w, 1, "Aspect" ), b * b / wm.getx( w, 1, "Area" ), what = "AR" )
    close( wm.getx( w, 1, "Taper" ), ct / cr, what = "taper" )
    le0, te0 = wm.lete( w, 0 )
    le1, te1 = wm.lete( w, 1 )
    close( le1, [ b * np.tan( D( sw ) ), b, 0 ], 1e-9, "tip LE from sweep" )
    # Sec Sweep is the sweep of the Sec SW Loc line (the TE by default).
    tsec = ( te1[0] - te0[0] ) / b
    close( np.tan( D( wm.getx( w, 1, "Sec_Sweep" ) ) ), tsec, 1e-9, "TE sweep" )
    close( tsec, np.tan( D( sw ) ) - ( 1.0 - 0.0 ) * ( cr - ct ) / b, 1e-12, "sweep formula" )
    # Moving the sweep location keeps the angle: the line at the new location takes it.
    wm.setx( w, 1, "Sweep_Location", 0.25 )
    sw25 = wm.getx( w, 1, "Sweep" )
    close( sw25, sw, 1e-12, "sweep kept when its location moves" )
    le1, te1 = wm.lete( w, 1 )
    q0 = le0 + 0.25 * ( te0 - le0 )
    q1 = le1 + 0.25 * ( te1 - le1 )
    close( ( q1[0] - q0[0] ) / b, np.tan( D( sw ) ), 1e-9, "quarter chord now swept" )
    # ... and holds that line's sweep when a chord changes.
    wm.setx( w, 1, "Tip_Chord", 2.0 )
    le1, te1 = wm.lete( w, 1 )
    q0 = le0 + 0.25 * ( te0 - le0 )
    q1 = le1 + 0.25 * ( te1 - le1 )
    close( ( q1[0] - q0[0] ) / b, np.tan( D( sw25 ) ), 1e-9, "quarter chord sweep held" )
    print( "section planform: area, AR, taper, sweep and its location ok" )


def drivers():
    w = wm.new_wing()
    # Span, Root C and Sec SW: the tip chord follows from the two sweeps.
    vsp.SetDriverGroup( w, 1, vsp.SPAN_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.SECSWEEP_WSECT_DRIVER )
    wm.setx( w, 1, "Sec_Sweep", 20.0 )
    b = wm.getx( w, 1, "Span" )
    close( wm.getx( w, 1, "Tip_Chord" ), 4.0 + b * ( np.tan( D( 20.0 ) ) - np.tan( D( 30.0 ) ) ), 1e-9, "tip chord from LE and TE sweep" )
    # A neighbor keeps its span and its other chord, whatever its drivers.
    w = wm.new_wing()
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    vsp.SetDriverGroup( w, 1, vsp.AREA_WSECT_DRIVER, vsp.AR_WSECT_DRIVER, vsp.TAPER_WSECT_DRIVER )
    vsp.Update()
    s1 = wm.getx( w, 1, "Span" )
    a1 = wm.getx( w, 1, "Area" )
    wm.setx( w, 2, "Root_Chord", 3.0 )
    close( [ wm.getx( w, 1, n ) for n in ( "Span", "Root_Chord", "Tip_Chord" ) ], [ s1, 4.0, 3.0 ], what = "neighbor" )
    assert abs( wm.getx( w, 1, "Area" ) - a1 ) > 1.0
    # Split halves the span and puts the average chord at the new airfoil.
    close( s1, 4.5, what = "split span" )
    print( "drivers: sec sweep driver, neighbor follows a shared chord, split ok" )


def dihedral_and_rotation():
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    wm.setx( w, 1, "Dihedral", 10.0 )
    le1, te1 = wm.lete( w, 1 )
    b = 9.0
    close( le1, [ b * np.tan( D( 30.0 ) ), b * np.cos( D( 10.0 ) ), b * np.sin( D( 10.0 ) ) ], 1e-9, "dihedral" )
    close( wm.getx( w, 1, "ProjectedSpan" ), b * np.cos( D( 10.0 ) ), what = "projected span" )
    # Rotate To Match Dihedral (All But Root, the default): the tip foil is normal to its panel.
    close( foil_normal( w, 1 ), [ 0, np.cos( D( 10.0 ) ), np.sin( D( 10.0 ) ) ], 1e-6, "tip foil rotated" )
    close( foil_normal( w, 0 ), [ 0, 1, 0 ], 1e-6, "root foil upright" )
    # Correct Foil Thickness: the upright root foil is thickened by 1 / cos( dihedral ).
    F = wm.foil( w, 0, 2001 )
    close( np.ptp( F[:, 2] ), 0.1 * 4.0 / np.cos( D( 10.0 ) ), 2e-4, "root foil thickened" )
    # Three airfoils: the middle one lies on the bisector and is thickened by 1 / cos( half the kink ).
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    wm.setx( w, 2, "Dihedral", 40.0 )
    close( foil_normal( w, 1 ), [ 0, np.cos( D( 20.0 ) ), np.sin( D( 20.0 ) ) ], 1e-6, "bisector" )
    F = wm.foil( w, 1, 2001 )
    n = foil_normal( w, 1 )
    up = np.cross( [ 1, 0, 0 ], n )
    c = 2.5
    close( np.ptp( F @ up ), 0.1 * c / np.cos( D( 20.0 ) ), 2e-4, "mid foil thickened" )
    # Relative dihedral accumulates.
    wm.setx( w, 1, "Dihedral", 5.0 )
    wm.setx( w, 2, "Dihedral", 5.0 )
    wm.setw( w, "RelativeDihedralFlag", 1.0 )
    le1, _ = wm.lete( w, 1 )
    le2, _ = wm.lete( w, 2 )
    d = le2 - le1
    close( np.degrees( np.arctan2( d[2], d[1] ) ), 10.0, 1e-9, "relative dihedral" )
    print( "dihedral: placement, projected span, foil rotation, thickness correction, relative ok" )


def twist():
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    le, te = wm.lete( w, 1 )
    q = le + 0.25 * ( te - le )
    wm.setx( w, 1, "Twist", 5.0 )
    close( foil_angle( w, 1 ), 5.0, 1e-9, "positive twist raises the LE" )
    le, te = wm.lete( w, 1 )
    close( le + 0.25 * ( te - le ), q, 1e-9, "twist about Twist Loc" )
    # Absolute (default): the root incidence turns the root airfoil only.
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    wm.setx( w, 0, "Twist", 3.0 )
    close( [ foil_angle( w, 0 ), foil_angle( w, 1 ) ], [ 3.0, 0.0 ], 1e-9, "absolute twist" )
    wm.setw( w, "RelativeTwistFlag", 1.0 )
    wm.setx( w, 1, "Twist", -2.0 )
    close( [ foil_angle( w, 0 ), foil_angle( w, 1 ) ], [ 3.0, 1.0 ], 1e-9, "relative twist adds the incidence" )
    print( "twist: sign, location, absolute and relative, root incidence ok" )


def totals():
    w = wm.new_wing( sym = True )
    close( [ wm.getw( w, n ) for n in ( "TotalSpan", "TotalProjectedSpan", "TotalArea", "TotalChord", "MAC" ) ],
           [ 18.0, 18.0, 45.0, 2.5, 2.0 / 3.0 * 4.0 * ( 1 + 0.25 + 0.0625 ) / 1.25 ], 1e-9, "default totals" )
    wm.setx( w, 1, "Dihedral", 30.0 )
    close( wm.getw( w, "TotalAR" ), ( 18.0 * np.cos( D( 30.0 ) ) ) ** 2 / 45.0, 1e-9, "AR from projected span" )
    # Total Chord is the plain mean of the sections' average chords.
    w = wm.new_wing( sym = True )
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    wm.setx( w, 2, "Span", 2.0 )
    close( wm.getw( w, "TotalChord" ), 0.5 * ( wm.getx( w, 1, "Avg_Chord" ) + wm.getx( w, 2, "Avg_Chord" ) ), what = "chord" )
    # Total Area scales every section, keeping AR and taper.
    ar = [ wm.getx( w, i, "Aspect" ) for i in ( 1, 2 ) ]
    tr = [ wm.getx( w, i, "Taper" ) for i in ( 1, 2 ) ]
    wm.setw( w, "TotalArea", 100.0 )
    close( [ wm.getx( w, i, "Aspect" ) for i in ( 1, 2 ) ], ar, 1e-9, "area keeps AR" )
    close( [ wm.getx( w, i, "Taper" ) for i in ( 1, 2 ) ], tr, 1e-9, "area keeps taper" )
    close( wm.getx( w, 1, "Sweep" ), 30.0, 1e-9, "area keeps sweep" )
    # Total Span scales spans and keeps chords.
    c = [ wm.getx( w, i, "Tip_Chord" ) for i in ( 0, 1, 2 ) ]
    wm.setw( w, "TotalSpan", 30.0 )
    close( [ wm.getx( w, i, "Tip_Chord" ) for i in ( 0, 1, 2 ) ], c, 1e-9, "span keeps chords" )
    print( "totals: span, projected span, chord, area, AR, MAC ok" )


def blending():
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    # A straight edge reads its dihedral on both sides.
    wm.setw( w, "RotateAirfoilMatchDideralFlag", 0.0 )
    wm.setx( w, 1, "Dihedral", 10.0 )
    wm.setx( w, 2, "Dihedral", 10.0 )
    close( [ wm.getx( w, 1, "InLEDihedral" ), wm.getx( w, 1, "OutLEDihedral" ) ], [ 10.0, 10.0 ], 1e-6, "dihedral read" )
    close( [ wm.getx( w, 1, "InLESweep" ), wm.getx( w, 1, "OutLESweep" ) ], [ 30.0, 30.0 ], 1e-6, "sweep read" )
    # IN_ANGLES takes the inboard sweep and dihedral: the edge goes on smoothly, rising outboard.
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    wm.setx( w, 1, "InLEMode", vsp.BLEND_ANGLES )
    wm.setx( w, 1, "InLEDihedral", 10.0 )
    wm.setx( w, 1, "OutLEMode", vsp.BLEND_MATCH_IN_ANGLES )
    u = wm.u01( w, 1.0 )
    h = 1e-4
    pm = wm.pnt( w, u - h, 0.5 )
    p0 = wm.pnt( w, u, 0.5 )
    pp = wm.pnt( w, u + h, 0.5 )
    sl = ( p0[2] - pm[2] ) / ( p0[1] - pm[1] )
    sr = ( pp[2] - p0[2] ) / ( pp[1] - p0[1] )
    close( [ sl, sr ], [ np.tan( D( 10.0 ) ), np.tan( D( 10.0 ) ) ], 1e-3, "IN_ANGLES dihedral smooth" )
    sl = ( p0[0] - pm[0] ) / ( p0[1] - pm[1] )
    sr = ( pp[0] - p0[0] ) / ( pp[1] - p0[1] )
    close( sl, sr, 1e-3, "IN_ANGLES sweep smooth" )
    print( "blending: angle conventions, IN_ANGLES smooth ok" )


def extremes():
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    wm.setx( w, 1, "Tip_Chord", 0.0 )
    le, te = wm.lete( w, 1 )
    close( le, te, 1e-9, "pointed tip" )
    wm.setx( w, 1, "Dihedral", 90.0 )
    le, _ = wm.lete( w, 1 )
    close( le[1:], [ 0.0, 9.0 ], 1e-9, "vertical panel" )
    print( "extremes: zero tip chord, 90 degree dihedral ok" )


def sections_and_loft():
    # Split with absolute twist: the new airfoil takes the mean of its neighbors' twists, the tip keeps its own.
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    wm.setx( w, 1, "Twist", -4.0 )
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    close( [ foil_angle( w, 1 ), foil_angle( w, 2 ) ], [ -2.0, -4.0 ], 1e-9, "split, absolute twist" )
    close( [ wm.getx( w, 1, "Tip_Chord" ) ], [ 2.5 ], what = "split chord" )
    # Insert on the tip section adds the pasted section beyond the tip.
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    wm.setx( w, 1, "Dihedral", 7.0 )
    vsp.CopyXSec( w, 1 )
    vsp.InsertXSec( w, 2, vsp.XS_FOUR_SERIES )
    vsp.Update()
    close( [ wm.getx( w, 2, "Dihedral" ), wm.getx( w, 3, "Dihedral" ), wm.getx( w, 3, "Span" ) ], [ 0.0, 7.0, 4.5 ],
           what = "insert at the tip" )
    # Without blending the surface between airfoils is ruled.
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    wm.setx( w, 1, "Twist", -6.0 )
    wm.setx( w, 1, "Dihedral", 10.0 )
    err = 0.0
    for t in np.linspace( 0.0, 1.0, 37 ):
        a = wm.pnt( w, 0.0, t )
        b = wm.pnt( w, 1.0, t )
        for s in ( 0.25, 0.5, 0.8 ):
            err = max( err, np.linalg.norm( wm.pnt( w, s, t ) - ( ( 1 - s ) * a + s * b ) ) )
    assert err < 1e-12, err
    # Other shapes take the chord as their width and keep their height.
    vsp.ChangeXSecShape( vsp.GetXSecSurf( w, 0 ), 1, vsp.XS_ELLIPSE )
    vsp.Update()
    wm.setx( w, 1, "Twist", 0.0 )
    wm.setx( w, 1, "Dihedral", 0.0 )
    wm.setx( w, 1, "Tip_Chord", 2.0 )
    F = wm.foil( w, 1, 401 )
    close( [ np.ptp( F[:, 0] ), np.ptp( F[:, 2] ) ], [ 2.0, 0.1 ], 1e-6, "ellipse on a wing" )
    print( "sections: split, insert at the tip, ruled loft, other shapes ok" )


def placement():
    # The origin is the root leading edge; X Rotation 90 turns the span to +Z.
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    le, _ = wm.lete( w, 0 )
    close( le, [ 0, 0, 0 ], 1e-12, "origin" )
    vsp.SetParmVal( w, "X_Rel_Rotation", "XForm", 90.0 )
    vsp.Update()
    le, _ = wm.lete( w, 1 )
    close( le[1:], [ 0.0, 9.0 ], 1e-9, "fin" )
    # Y Rotation about Rot Origin(X), a fraction of the root chord; positive raises the LE.
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    vsp.SetParmVal( w, "Origin", "XForm", 0.25 )
    vsp.SetParmVal( w, "Y_Rel_Rotation", "XForm", 5.0 )
    vsp.Update()
    le, te = wm.lete( w, 0 )
    close( le + 0.25 * ( te - le ), [ 1.0, 0, 0 ], 1e-9, "rot origin" )
    close( foil_angle( w, 0 ), 5.0, 1e-9, "Y rotation raises the LE" )
    # A dihedral near 90 thickens the upright root airfoil without bound.
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    wm.setx( w, 1, "Dihedral", 89.0 )
    F = wm.foil( w, 0, 801 )
    assert np.ptp( F[:, 2] ) > 20.0
    print( "placement: origin, fin, rotation origin, root thickening ok" )


def flap():
    # A flap on the tip airfoil only fades linearly to the root.
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    xs = wm.xsec( w, 1 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_Type" ), vsp.FLAP_PLAIN )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Flap_Deflection" ), 20.0 )
    vsp.Update()
    z = [ ( wm.pnt( w, u, 0.0 ) - wm.pnt( w, u, 0.5 ) )[2] for u in ( 0.0, 0.5, 1.0 ) ]
    close( z[1], 0.5 * ( z[0] + z[2] ), 1e-12, "flap fades" )
    assert z[2] < -0.05
    print( "flap: fades across the section ok" )


def tip_caps():
    # Kink between the upper and lower halves of the cap at its tip, degrees (0 = smooth), at mid-chord.
    def kink( cap, strength ):
        w = wm.new_wing( tip_cap = cap )
        vsp.SetParmVal( w, "CapUMaxStrength", "EndCap", strength )
        vsp.Update()
        tip = [ wm.pnt( w, 1.0, t ) for t in ( 0.75, 0.25 ) ]
        d = [ wm.pnt( w, 1.0 - 1e-4, t ) - p for t, p in zip( ( 0.75, 0.25 ), tip ) ]
        d = [ v / np.linalg.norm( v ) for v in d ]
        return 180.0 - np.degrees( np.arccos( np.clip( d[0] @ d[1], -1.0, 1.0 ) ) ), tip[0]
    k_edge, _ = kink( vsp.EDGE_END_CAP, 0.0 )
    k_sharp0, _ = kink( vsp.SHARP_END_CAP, 0.0 )
    k_sharp9, _ = kink( vsp.SHARP_END_CAP, 0.9 )
    k_sharp1, a_sharp = kink( vsp.SHARP_END_CAP, 1.0 )
    k_round, a_round = kink( vsp.ROUND_END_CAP, 1.0 )
    close( k_sharp0, k_edge, 1e-6, "Sharp 0 is Edge" )
    assert 11.0 < k_sharp9 < 13.0, k_sharp9
    assert k_sharp1 < 0.1 and k_round < 0.1, ( k_sharp1, k_round )
    reach = ( a_sharp[1] - 9.0 ) / ( a_round[1] - 9.0 )
    assert 1.005 < reach < 1.03, reach
    # Choosing a cap type sets the Strength; Sharp keeps what it finds.
    w = wm.new_wing()
    seq = []
    for cap in ( vsp.SHARP_END_CAP, vsp.ROUND_END_CAP, vsp.SHARP_END_CAP, vsp.EDGE_END_CAP, vsp.SHARP_END_CAP ):
        vsp.SetParmVal( w, "CapUMaxOption", "EndCap", cap )
        vsp.Update()
        seq.append( vsp.GetParmVal( w, "CapUMaxStrength", "EndCap" ) )
    close( seq, [ 0.5, 1.0, 1.0, 0.0, 0.0 ], what = "cap strength" )
    print( "tip caps: Sharp strength 0, 0.9, 1 against Edge and Round; strength reset ok" )


def section_edits():
    def three():
        w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
        vsp.SplitWingXSec( w, 1 )
        vsp.Update()
        vsp.SplitWingXSec( w, 2 )
        vsp.Update()
        for i, b in ( ( 1, 2.0 ), ( 2, 3.0 ), ( 3, 4.0 ) ):
            wm.setx( w, i, "Span", b )
        return w
    # Cut: the inboard section takes the outboard one's root chord; spans stay.
    w = three()
    rc3 = wm.getx( w, 3, "Root_Chord" )
    vsp.CutXSec( w, 2 )
    vsp.Update()
    close( [ wm.getx( w, 1, "Tip_Chord" ), wm.getx( w, 1, "Span" ), wm.getx( w, 2, "Span" ) ], [ rc3, 2.0, 4.0 ],
           what = "cut" )
    # Paste: the neighbors' shared chords follow the pasted section.
    w = three()
    vsp.CopyXSec( w, 1 )
    vsp.PasteXSec( w, 3 )
    vsp.Update()
    close( [ wm.getx( w, 2, "Tip_Chord" ), wm.getx( w, 3, "Span" ) ], [ 4.0, 2.0 ], what = "paste" )
    # Insert with nothing copied: span 1, chords 1, no sweep.
    w = three()
    vsp.InsertXSec( w, 1, vsp.XS_FOUR_SERIES )
    vsp.Update()
    got = [ wm.getx( w, 2, n ) for n in ( "Span", "Root_Chord", "Tip_Chord", "Sweep" ) ]
    got.append( wm.getx( w, 1, "Tip_Chord" ) )
    close( got, [ 1.0, 1.0, 1.0, 0.0, 1.0 ], what = "insert, nothing copied" )
    print( "section edits: cut, paste, insert with nothing copied ok" )


def neighbor_drivers():
    """A neighbor is solved as Span, Root C, Tip C for a shared-chord change; its drivers are kept."""
    names = ( "Aspect", "Span", "Area", "Taper", "Avg_Chord", "Root_Chord", "Tip_Chord", "Sec_Sweep" )
    for drv in ( ( vsp.AR_WSECT_DRIVER, vsp.AREA_WSECT_DRIVER, vsp.TAPER_WSECT_DRIVER ),
                 ( vsp.SPAN_WSECT_DRIVER, vsp.AREA_WSECT_DRIVER, vsp.TAPER_WSECT_DRIVER ) ):
        w = wm.new_wing()
        vsp.SplitWingXSec( w, 1 )
        vsp.Update()
        vsp.SetDriverGroup( w, 2, *drv )
        vsp.Update()
        b2 = wm.getx( w, 2, "Span" )
        ct2 = wm.getx( w, 2, "Tip_Chord" )
        wm.setx( w, 1, "Tip_Chord", 3.0 )
        close( [ wm.getx( w, 2, n ) for n in ( "Span", "Root_Chord", "Tip_Chord" ) ], [ b2, 3.0, ct2 ], what = "neighbor span, chords" )
        close( [ wm.getx( w, 2, "Area" ), wm.getx( w, 2, "Taper" ) ], [ b2 * ( 3.0 + ct2 ) / 2, ct2 / 3.0 ], what = "neighbor drivers recomputed" )
        assert tuple( vsp.GetDriverGroup( w, 2 ) ) == drv, "neighbor keeps its drivers"
        # The drivers then hold their new values: another edit does not restore the old ones.
        a = wm.getx( w, 2, "Area" )
        wm.setx( w, 2, "Sweep", 31.0 )
        close( wm.getx( w, 2, "Area" ), a, what = "new values held" )
    # With the shared chords as drivers, an Area change moves only the span; the neighbor is untouched.
    w = wm.new_wing()
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    for i in ( 1, 2 ):
        vsp.SetDriverGroup( w, i, vsp.AREA_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
    vsp.Update()
    before = [ wm.getx( w, 2, n ) for n in names ]
    wm.setx( w, 1, "Area", 20.0 )
    close( [ wm.getx( w, 1, n ) for n in ( "Root_Chord", "Tip_Chord", "Span" ) ], [ 4.0, 2.5, 20.0 / 3.25 ], what = "area moves span" )
    close( [ wm.getx( w, 2, n ) for n in names ], before, what = "neighbor untouched" )
    print( "neighbor drivers: solved as span/root/tip, drivers kept, shared chords as drivers isolate ok" )


def driver_sets():
    """39 of the 56 sets of three are accepted; the 17 refused are the four families the page lists."""
    import itertools
    w = wm.new_wing()
    em = vsp.ErrorMgrSingleton.getInstance()
    A, S, AR_, T, C, R, TC, SW = ( vsp.AREA_WSECT_DRIVER, vsp.SPAN_WSECT_DRIVER, vsp.AR_WSECT_DRIVER, vsp.TAPER_WSECT_DRIVER,
                                    vsp.AVEC_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER, vsp.SECSWEEP_WSECT_DRIVER )

    def refused_by_page( c ):
        c = set( c )
        return ( c <= { AR_, S, A, C } or c <= { R, TC, C, T } or { T, SW } <= c or
                 ( SW in c and len( c & { R, TC, C } ) == 2 ) )
    nvalid = 0
    for c in itertools.combinations( range( vsp.NUM_WSECT_DRIVER ), 3 ):
        vsp.SetDriverGroup( w, 1, *c )
        ok = em.GetNumTotalErrors() == 0
        while em.GetNumTotalErrors() > 0:
            em.PopLastError()
        assert ok != refused_by_page( c ), c
        nvalid += ok
        vsp.SetDriverGroup( w, 1, S, R, TC )
    assert nvalid == 39, nvalid
    print( "driver sets: 39 of 56 valid, refused sets as listed ok" )


def foil_flags():
    """Plan-tab and Sect-tab rotate controls, how they combine, and the thinning at an upright airfoil."""
    def make():
        w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
        vsp.SplitWingXSec( w, 1 )
        vsp.Update()
        wm.setx( w, 1, "Dihedral", 10.0 )
        wm.setx( w, 2, "Dihedral", 40.0 )
        return w

    def roll( w, i ):
        n = foil_normal( w, i )
        return np.degrees( np.arctan2( n[2], n[1] ) )

    def rolls( w ):
        return [ roll( w, i ) for i in range( 3 ) ]
    close( rolls( make() ), [ 0, 25, 40 ], 1e-4, "default rotation" )
    w = make()
    wm.setx( w, 0, "RotateMatchDideralFlag", 1 )
    close( rolls( w ), [ 10, 25, 40 ], 1e-4, "Wing Root" )
    w = make()
    wm.setw( w, "RotateAirfoilMatchDideralFlag", 0 )
    close( rolls( w ), [ 0, 0, 0 ], 1e-4, "All But Root off" )
    wm.setx( w, 1, "RotateMatchDideralFlag", 1 )
    close( rolls( w ), [ 0, 25, 0 ], 1e-4, "section 1 flag turns airfoil 1" )
    w = make()
    wm.setx( w, 2, "RotateMatchDideralFlag", 0 )
    wm.setx( w, 1, "RotateMatchDideralFlag", 1 )
    close( rolls( w ), [ 0, 25, 40 ], 1e-4, "section flags ignored under All But Root" )
    # Twist All In XZ Plane does not reach an airfoil turned by its own flag.
    w = make()
    wm.setw( w, "RotateAirfoilMatchDideralFlag", 0 )
    wm.setx( w, 2, "RotateMatchDideralFlag", 1 )
    wm.setw( w, "TwistAllInXZPlaneFlag", 1 )
    wm.setx( w, 2, "Twist", 10.0 )
    close( roll( w, 2 ), 40.0, 1e-4, "own flag beats twist in XZ" )
    # An upright airfoil meeting a section of dihedral G: the wing across the section is cos G of the airfoil.
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    wm.setx( w, 1, "Tip_Chord", 4.0 )
    wm.setx( w, 1, "Sweep", 0.0 )
    wm.setx( w, 1, "Dihedral", 40.0 )
    wm.setw( w, "RotateAirfoilMatchDideralFlag", 0 )
    wm.setw( w, "CorrectAirfoilthicknessFlag", 0 )
    u = wm.u01( w, 0.5 )
    P = np.array( [ wm.pnt( w, u, t ) for t in np.linspace( 0, 1, 2001 ) ] )
    n = np.array( [ 0, -np.sin( D( 40.0 ) ), np.cos( D( 40.0 ) ) ] )
    close( np.ptp( P @ n ), 0.1 * 4.0 * np.cos( D( 40.0 ) ), 2e-3, "upright thinning" )
    print( "foil flags: Plan/Sect rotate controls, precedence, upright thinning ok" )


def incidence_vs_yrot():
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    le, te = wm.lete( w, 1 )
    q1 = le + 0.25 * ( te - le )
    wm.setw( w, "RelativeTwistFlag", 1.0 )
    wm.setx( w, 0, "Twist", 8.0 )
    le, te = wm.lete( w, 1 )
    close( le + 0.25 * ( te - le ), q1, 1e-9, "rel incidence keeps the twist line" )
    close( foil_angle( w, 1 ), 8.0, 1e-9, "rel incidence turns the tip" )
    w = wm.new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    vsp.SetParmVal( w, "Origin", "XForm", 0.25 )
    vsp.SetParmVal( w, "Y_Rel_Rotation", "XForm", 8.0 )
    vsp.Update()
    le, te = wm.lete( w, 1 )
    r1 = le + 0.25 * ( te - le )
    close( foil_angle( w, 1 ), 8.0, 1e-9, "Y rotation raises the LE" )
    close( r1[2], -( q1[0] - 1.0 ) * np.sin( D( 8.0 ) ), 1e-9, "Y rotation drops the tip" )
    close( round( q1[2] - r1[2], 2 ), 0.62, 1e-12, "drop quoted" )
    print( "incidence vs Y rotation: twist line kept, rigid turn drops the tip 0.62 ok" )


def eta():
    """eta: running in-plane span over total, linear in U per section, caps excluded."""
    w = wm.new_wing( root_cap = vsp.FLAT_END_CAP, tip_cap = vsp.ROUND_END_CAP )
    vsp.SplitWingXSec( w, 1 )
    vsp.Update()
    wm.setx( w, 1, "Span", 3.0 )
    wm.setx( w, 2, "Span", 6.0 )
    wm.setx( w, 2, "Dihedral", 30.0 )
    close( [ vsp.ConvertEtatoU( w, e ) for e in ( 0, 1 / 3, 0.5, 1 ) ], [ 1, 2, 2.25, 3 ], 1e-12, "eta to U with root cap" )
    vsp.SetParmVal( w, "CapUMinOption", "EndCap", vsp.NO_END_CAP )
    vsp.Update()
    close( [ vsp.ConvertEtatoU( w, e ) for e in ( 0, 1 / 3, 1 ) ], [ 0, 1, 2 ], 1e-12, "eta to U without root cap" )
    # Outside the airfoils (in a cap) U converts to eta 0.
    vsp.SetParmVal( w, "CapUMinOption", "EndCap", vsp.FLAT_END_CAP )
    vsp.Update()
    close( [ vsp.ConvertUtoEta( w, u ) for u in ( 0.5, 3.5 ) ], [ 0, 0 ], 1e-12, "caps give eta 0" )
    # Attachment at eta 0.5 sits half way along the in-plane span: 1.5 into the 30 degree section.
    p = vsp.AddGeom( "POD", w )
    vsp.SetParmVal( p, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_EtaMN )
    vsp.SetParmVal( p, "Eta_Attach_Location", "Attach", 0.5 )
    vsp.SetParmVal( p, "M_Attach_Location", "Attach", 1.0 )
    vsp.SetParmVal( p, "N_Attach_Location", "Attach", 0.5 )
    vsp.Update()
    close( [ vsp.GetParmVal( p, n, "XForm" ) for n in ( "Y_Location", "Z_Location" ) ],
           [ 3.0 + 1.5 * np.cos( D( 30.0 ) ), 1.5 * np.sin( D( 30.0 ) ) ], 1e-6, "eta attach in plane" )
    # A propeller's eta is r/R: from the hub radius fraction at the root airfoil to 1 at the tip.
    vsp.VSPRenew()
    pr = vsp.AddGeom( "PROP" )
    vsp.Update()
    ss = vsp.AddSubSurf( pr, vsp.SS_CONTROL )
    vsp.Update()
    ids = { vsp.GetParmName( x ): x for x in vsp.GetSubSurfParmIDs( ss ) }
    vsp.SetParmVal( ids[ "EtaFlag" ], 1 )
    vsp.SetParmVal( ids[ "EtaStart" ], 0.3 )
    vsp.Update()
    r0 = vsp.GetParmVal( vsp.GetXSecParm( vsp.GetXSec( vsp.GetXSecSurf( pr, 0 ), 0 ), "RadiusFrac" ) )
    close( vsp.GetParmVal( ids[ "UStart" ] ) * 3.0 - 1.0, ( 0.3 - r0 ) / ( 1.0 - r0 ), 1e-9, "prop eta is r/R" )
    print( "eta: span fraction, caps excluded, in-plane, propeller r/R ok" )


def winglet_example():
    """The four-section fillet and winglet example: no NaN, and the edges tangent where the page says."""
    w = wm.winglet_example()
    h = 1e-5
    for i in ( 1, 2, 3 ):
        u = wm.u01( w, i )
        for t in ( 0.5, 0.0 ):
            pm, p0, pp = ( wm.pnt( w, u + d, t ) for d in ( -h, 0.0, h ) )
            assert np.all( np.isfinite( [ pm, p0, pp ] ) )
            # The TRAP modes match the neighbor's edge closely, not exactly: at the fillet's airfoil 1 the leading
            # edge kinks by 1.6 degrees; elsewhere the edges kink by under 0.1.
            ang = np.degrees( np.arccos( np.clip( fl_unit( p0 - pm ) @ fl_unit( pp - p0 ), -1, 1 ) ) )
            assert ang < 2.0, ( "edge tangent at airfoil %d" % i, ang )
    # The main wing's leading edge stays straight.
    P = [ wm.pnt( w, wm.u01( w, U ), 0.5 ) for U in ( 1.0, 1.5, 2.0 ) ]
    close( P[1], 0.5 * ( P[0] + P[2] ), 1e-9, "main LE straight" )
    # The fillet's edges leave the root square to it.
    for t in ( 0.5, 0.0 ):
        d = wm.pnt( w, 1e-6, t ) - wm.pnt( w, 0.0, t )
        assert abs( np.degrees( np.arctan2( d[0], d[1] ) ) ) < 1.0, ( "root sweep", t )
    print( "winglet example: square at the root, tangent at airfoils 1-3, main wing straight ok" )


def fl_unit( v ):
    return v / np.linalg.norm( v )


if __name__ == "__main__":
    section_planform()
    drivers()
    dihedral_and_rotation()
    twist()
    totals()
    blending()
    extremes()
    sections_and_loft()
    placement()
    flap()
    tip_caps()
    section_edits()
    neighbor_drivers()
    driver_sets()
    foil_flags()
    incidence_vs_yrot()
    eta()
    winglet_example()
