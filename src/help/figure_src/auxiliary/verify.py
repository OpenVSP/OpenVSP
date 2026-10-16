"""Checks the directions Auxiliary.md states against the openvsp module.

    <build>/venv/bin/python verify.py
"""

import numpy as np
import openvsp as vsp


def check( ok, what ):
    print( ( "ok    " if ok else "FAIL  " ) + what )
    return ok


def pt( g, s, u, w ):
    p = vsp.CompPnt01( g, s, u, w )
    return np.array( [ p.x(), p.y(), p.z() ] )


def aux( parent, mode ):
    a = vsp.AddGeom( "AUXILIARY", parent )
    vsp.SetParmVal( a, "AuxiliaryGeomType", "Design", mode )
    vsp.Update()
    return a


def tip_path():
    vsp.VSPRenew()
    p = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( p, "RFlapAx", "Articulation", 0.1 )
    a = aux( p, vsp.AUX_GEOM_ROTOR_TIP_PATH )
    vsp.SetParmVal( a, "ThetaThrust", "Design", 20.0 )
    vsp.SetParmVal( a, "ThetaAntiThrust", "Design", 10.0 )
    vsp.Update()
    P = np.array( [ pt( a, 0, u, 0.0 ) for u in np.linspace( 0, 1, 201 ) ] )
    ok = check( abs( P[ :, 0 ].min() + 13.5 * np.sin( np.radians( 20 ) ) ) < 1e-6, "Theta Thrust tips toward -X, the thrust" )
    ok = check( abs( P[ 0, 1 ] - 1.5 ) < 1e-9, "Automatic r_flap/R puts the hinge at the propeller's flap hinge" ) and ok
    vsp.SetParmVal( a, "AutoFlapRadius", "Design", 0 )
    vsp.SetParmVal( a, "FlapRadiusFract", "Design", 0.2 )
    vsp.Update()
    ok = check( abs( pt( a, 0, 0.0, 0.0 )[ 1 ] - 3.0 ) < 1e-9, "the flap hinge is r_flap/R from the shaft" ) and ok
    return check( abs( vsp.GetParmVal( a, "Diameter", "Design" ) - vsp.GetParmVal( p, "Diameter", "Design" ) ) < 1e-12,
                  "Automatic Diameter is the propeller's" ) and ok


def fragment():
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    a = aux( pod, vsp.AUX_GEOM_ROTOR_FRAGMENT )
    vsp.SetParmVal( a, "RotorFragmentMode", "Design", vsp.GENERIC_FRAGMENT )
    vsp.SetParmVal( a, "FragLength", "Design", 1.0 )
    vsp.SetParmVal( a, "CGRadius", "Design", 2.0 )
    vsp.SetParmVal( a, "ReleaseAngle", "Design", 30.0 )
    vsp.SetParmVal( a, "RotDir", "Design", 1 )
    vsp.Update()
    c = np.mean( [ pt( a, 0, 0.0, w ) for w in ( 0.0, 0.5 ) ], axis = 0 )
    far = np.mean( [ pt( a, 0, 0.5, w ) for w in ( 0.0, 0.25, 0.5, 0.75 ) ], axis = 0 )
    rel = np.radians( 30 )
    ok = check( np.abs( c - 2.0 * np.array( [ 0, np.sin( rel ), np.cos( rel ) ] ) ).max() < 1e-9,
                "the release point is at CG Radius, Release Angle from +Z about -X" )
    d = ( far - c ) / np.linalg.norm( far - c )
    ok = check( d @ np.array( [ 0, np.cos( rel ), -np.sin( rel ) ] ) > 0.98, "with Rotate Direction checked it leaves turning about -X" ) and ok
    vsp.SetParmVal( a, "RotDir", "Design", 0 )
    vsp.Update()
    c = np.mean( [ pt( a, 0, 0.0, w ) for w in ( 0.0, 0.5 ) ], axis = 0 )
    return check( c[1] < 0, "unchecked, it turns about +X" ) and ok


def super_cone():
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    a = aux( pod, vsp.AUX_GEOM_SUPER_CONE )
    P = np.array( [ pt( a, 0, u, w ) for u in np.linspace( 0.05, 1, 20 ) for w in np.linspace( 0, 1, 41 ) ] )
    P = P[ np.linalg.norm( P, axis = 1 ) > 1e-6 ]
    d = P / np.linalg.norm( P, axis = 1 )[ :, None ]
    az = np.degrees( np.arctan2( d[ :, 1 ], -d[ :, 0 ] ) )
    el = np.degrees( np.arcsin( d[ :, 2 ] ) )
    return check( abs( az.min() + 120 ) < 0.5 and abs( az.max() - 20 ) < 0.5 and abs( el.max() - 35 ) < 0.5,
                  "the AC 25.773 profile is azimuth toward +Y and elevation up, looking along -X" )


def spray_scale():
    vsp.VSPRenew()
    g = vsp.AddGeom( "GEAR" )
    vsp.CreateAndAddBogie( g )
    a = aux( g, vsp.AUX_GEOM_TIRE_SPRAY )
    w0 = vsp.GetParmVal( a, "SprayTireContactWidth", "Design" )
    vsp.SetParmVal( a, "Scale", "XForm", 2.0 )
    vsp.Update()
    return check( abs( vsp.GetParmVal( a, "SprayTireContactWidth", "Design" ) - w0 ) < 1e-12,
                  "scaling leaves the spray's tire-contact fractions" )


def ground_states():
    """A ground plane passes through each bogie's contact point in its state: the pivot, moved by the suspension, less
    the tire's radius in the Tire Mode."""
    vsp.VSPRenew()
    g = vsp.AddGeom( "GEAR" )
    vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", 0 )
    ids = []
    for x, y, sym in ( ( 0.0, 0.0, 0 ), ( 8.0, 3.0, 1 ) ):
        b = vsp.CreateAndAddBogie( g )
        vsp.SetParmVal( vsp.FindParm( b, "XContactPt", "Bogie" ), x )
        vsp.SetParmVal( vsp.FindParm( b, "YContactPt", "Bogie" ), y )
        vsp.SetParmVal( vsp.FindParm( b, "Symmetrical", "Bogie" ), sym )
        ids.append( b )
    nose = ids[ 0 ]
    for k, v in ( ( "TravelX", 0.3 ), ( "TravelZ", 1.0 ), ( "TravelCompressed", 0.4 ), ( "TravelExtended", 0.2 ) ):
        vsp.SetParmVal( vsp.FindParm( nose, k, "Bogie" ), v )
    vsp.Update()
    a = aux( g, vsp.AUX_GEOM_THREE_PT_GROUND )
    for i, b, isym in ( ( 0, nose, 0 ), ( 1, ids[ 1 ], 0 ), ( 2, ids[ 1 ], 1 ) ):
        vsp.SetAuxiliaryGeomContactPtID( a, i, b )
        vsp.SetParmVal( a, "ContactPt%d_Isymm" % ( i + 1 ), "Design", isym )
    vsp.Update()

    def tp( n ):
        return vsp.GetParmVal( vsp.FindParm( nose, n, "Tire" ) )
    slr = tp( "StaticRadiusModel" )
    radius = { vsp.TIRE_STATIC_LODED_CONTACT: slr, vsp.TIRE_NOMINAL_CONTACT: 0.5 * tp( "DiameterModel" ),
               vsp.TIRE_GROWTH_CONTACT: 0.5 * tp( "DGModel" ), vsp.TIRE_FLAT_CONTACT: 0.5 * tp( "DFlangeModel" ) }
    d = np.array( [ 0.3, 0.0, 1.0 ] ) / np.hypot( 0.3, 1.0 )
    move = { vsp.GEAR_SUSPENSION_NOMINAL: 0.0, vsp.GEAR_SUSPENSION_COMPRESSED: 0.4, vsp.GEAR_SUSPENSION_EXTENDED: -0.2 }
    pivot = np.array( [ 0.0, 0.0, slr ] )
    worst = 0.0
    for susp, m in move.items():
        for tire, r in radius.items():
            vsp.SetParmVal( a, "ContactPt1_SuspensionMode", "Design", susp )
            vsp.SetParmVal( a, "ContactPt1_TireMode", "Design", tire )
            vsp.Update()
            c = pivot + m * d - np.array( [ 0.0, 0.0, r ] )
            P = [ pt( a, 0, u, w ) for u, w in ( ( 0, 0 ), ( 1, 0 ), ( 0, 1 ) ) ]
            n = np.cross( P[ 1 ] - P[ 0 ], P[ 2 ] - P[ 0 ] )
            worst = max( worst, abs( ( c - P[ 0 ] ) @ n ) / np.linalg.norm( n ) )
    ok = check( worst < 1e-9, "each Suspension Mode and Tire Mode puts the contact point at the pivot, moved along the "
                              "suspension direction (compressed toward the aircraft), less the tire's radius" )
    return check( radius[ vsp.TIRE_FLAT_CONTACT ] < slr < radius[ vsp.TIRE_NOMINAL_CONTACT ] < radius[ vsp.TIRE_GROWTH_CONTACT ],
                  "a flat tire's radius is below the static loaded radius, and the grown above the unloaded" ) and ok


def cone_windshield():
    """An Intersect sub-surface whose Intersect Geom is a Super Cone outlines where the cone meets the fuselage."""
    vsp.VSPRenew()
    f = vsp.AddGeom( "FUSELAGE" )
    b = vsp.AddGeom( "BLANK" )
    vsp.SetParmVal( b, "X_Rel_Location", "XForm", 8.0 )
    vsp.SetParmVal( b, "Z_Rel_Location", "XForm", 0.5 )
    vsp.Update()
    a = aux( b, vsp.AUX_GEOM_SUPER_CONE )
    ss = vsp.AddSubSurf( f, vsp.SS_INTERSECT )
    vsp.SetIntersectSubSurfGeomID( ss, a )
    vsp.IntersectSubSurf( ss )
    vsp.Update()
    n = vsp.GetParmVal( vsp.FindParm( ss, "NCurves", "SS_Intersect" ) )
    return check( n >= 1, "an Intersect sub-surface on a fuselage finds where a Super Cone meets it" )


def spray_center():
    """A center plume only for a bogie with two or more wheels across and one in tandem."""
    ok = True
    for across, tandem, plumes in ( ( 2, 1, 3 ), ( 3, 1, 4 ), ( 2, 2, 2 ), ( 1, 1, 2 ) ):
        vsp.VSPRenew()
        g = vsp.AddGeom( "GEAR" )
        vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", 0 )
        bo = vsp.CreateAndAddBogie( g )
        vsp.SetParmVal( vsp.FindParm( bo, "NumAcross", "Bogie" ), across )
        vsp.SetParmVal( vsp.FindParm( bo, "NumTandem", "Bogie" ), tandem )
        vsp.Update()
        a = aux( g, vsp.AUX_GEOM_TIRE_SPRAY )
        vsp.SetAuxiliaryGeomContactPtID( a, 0, bo )
        vsp.Update()
        ok = check( vsp.GetNumMainSurfs( a ) == plumes, "%d across, %d in tandem: %d plumes" % ( across, tandem, plumes ) ) and ok
    return ok


def roll_theta():
    """A positive Roll Theta lowers the 1pt plane toward +Y, about the line along X through the outer tire's contact
    point, whichever side the bogie is on."""
    ok = True
    for y, isym in ( ( 4.0, 0 ), ( 4.0, 1 ) ):
        vsp.VSPRenew()
        g = vsp.AddGeom( "GEAR" )
        vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", 0 )
        b = vsp.CreateAndAddBogie( g )
        vsp.SetParmVal( vsp.FindParm( b, "YContactPt", "Bogie" ), y )
        vsp.SetParmVal( vsp.FindParm( b, "Symmetrical", "Bogie" ), 1 )
        vsp.SetParmVal( vsp.FindParm( b, "NumAcross", "Bogie" ), 2 )
        vsp.Update()
        a = aux( g, vsp.AUX_GEOM_ONE_PT_GROUND )
        vsp.SetAuxiliaryGeomContactPtID( a, 0, b )
        vsp.SetParmVal( a, "ContactPt1_Isymm", "Design", isym )
        vsp.SetParmVal( a, "RollTheta", "Design", 8.0 )
        vsp.Update()
        P = [ pt( a, 0, u, w ) for u, w in ( ( 0, 0 ), ( 1, 0 ), ( 0, 1 ) ) ]
        n = np.cross( P[ 1 ] - P[ 0 ], P[ 2 ] - P[ 0 ] )
        n = n / np.linalg.norm( n ) * np.sign( n[ 2 ] )
        semi = 0.5 * vsp.GetParmVal( vsp.FindParm( b, "Spacing", "Bogie" ) )
        side = 1.0 if isym == 0 else -1.0
        cp = np.array( [ 0.0, side * ( y + semi ), 0.0 ] )
        ok = check( abs( ( cp - P[ 0 ] ) @ n ) < 1e-9 and abs( np.degrees( np.arctan2( n[ 1 ], n[ 2 ] ) ) - 8.0 ) < 1e-9,
                    "Roll Theta 8 under a %s bogie lowers the plane toward +Y, through the outer tire's contact point"
                    % ( "right" if isym == 0 else "left" ) ) and ok
    return ok


def spray_start():
    """The plumes leave the ground 2h ahead of the contact point; a side plume 0.75 b_g outboard of its tire's center;
    the side plume widens by tan( lambda_s ) per unit run and leans outward by phi_s."""
    vsp.VSPRenew()
    g = vsp.AddGeom( "GEAR" )
    vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", 0 )
    bo = vsp.CreateAndAddBogie( g )
    vsp.SetParmVal( vsp.FindParm( bo, "NumAcross", "Bogie" ), 2 )
    vsp.SetParmVal( vsp.FindParm( bo, "NumTandem", "Bogie" ), 1 )
    vsp.Update()
    a = aux( g, vsp.AUX_GEOM_TIRE_SPRAY )
    vsp.SetAuxiliaryGeomContactPtID( a, 0, bo )
    vsp.Update()
    D = vsp.GetParmVal( vsp.FindParm( bo, "DiameterModel", "Tire" ) )
    h = vsp.GetParmVal( a, "SprayTireContactHalfLength", "Design" ) * D
    bg = vsp.GetParmVal( a, "SprayTireContactWidth", "Design" ) * D
    semi = 0.5 * vsp.GetParmVal( vsp.FindParm( bo, "Spacing", "Bogie" ) )
    p0 = pt( a, 0, 0.0, 0.0 )
    ok = check( np.abs( p0 - [ -2.0 * h, semi + 0.75 * bg, 0.0 ] ).max() < 1e-9,
                "a side plume starts h ahead of the footprint and b_g / 4 outboard of it" )
    F = np.array( [ pt( a, 0, 0.5, w ) for w in ( 0.0, 0.25, 0.5, 0.75 ) ] )
    run = F[ 0, 0 ] - p0[ 0 ]
    lam = np.radians( vsp.GetParmVal( a, "SpraySideIncrementalAngle", "Design" ) )
    phi = np.radians( vsp.GetParmVal( a, "SpraySideInclinationAngle", "Design" ) )
    ok = check( abs( F[ 0, 1 ] - F[ 3, 1 ] - run * np.tan( lam ) ) < 1e-9, "the side plume is run tan( lambda_s ) wide" ) and ok
    ok = check( abs( F[ 0, 1 ] - F[ 1, 1 ] - F[ 0, 2 ] * np.tan( phi ) ) < 1e-9 and F[ 0, 1 ] > F[ 1, 1 ],
                "its sides lean outward by phi_s" ) and ok
    c0 = pt( a, 2, 0.0, 0.0 )
    return check( abs( c0[ 0 ] + 2.0 * h ) < 1e-9, "the center plume starts as far ahead" ) and ok


if __name__ == "__main__":
    results = [ tip_path(), fragment(), super_cone(), spray_scale(), ground_states(), cone_windshield(), spray_center(),
                roll_theta(), spray_start() ]
    print( "all passed" if all( results ) else "SOME FAILED" )
