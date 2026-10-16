"""Checks what Routing.md states about rounded corners and offsets against the openvsp module.

    <build>/venv/bin/python verify.py
"""

import math
import numpy as np
import openvsp as vsp


def check( ok, what ):
    print( ( "ok    " if ok else "FAIL  " ) + what )
    return ok


def corner():
    """A right-angle corner rounded by r shortens the route by (2 - pi/2) r and cuts inside the point; ends are never rounded."""
    vsp.VSPRenew()
    blanks = []
    for x, y in ( ( 0, 0 ), ( 10, 0 ), ( 10, 10 ) ):
        b = vsp.AddGeom( "BLANK" )
        vsp.SetParmVal( b, "X_Rel_Location", "XForm", x )
        vsp.SetParmVal( b, "Y_Rel_Location", "XForm", y )
        blanks.append( b )
    r = vsp.AddGeom( "ROUTING" )
    pts = [ vsp.AddRoutingPt( r, b, 0 ) for b in blanks ]
    vsp.SetParmVal( vsp.FindParm( pts[0], "Radius", "RoutePt" ), 2.0 )
    vsp.SetParmVal( vsp.FindParm( pts[2], "Radius", "RoutePt" ), 2.0 )
    vsp.Update()
    ok = check( abs( vsp.GetParmVal( r, "Length", "Results" ) - 20.0 ) < 1e-9, "the ends of a route are not rounded" )
    vsp.SetParmVal( vsp.FindParm( pts[1], "Radius", "RoutePt" ), 2.0 )
    vsp.Update()
    L = vsp.GetParmVal( r, "Length", "Results" )
    ok = check( abs( L - ( 20.0 - ( 4.0 - math.pi ) ) ) < 1e-3, "a rounded corner is a circular arc tangent to both legs" ) and ok
    crv = vsp.GetRoutingCurve( r, 0 )
    d = min( math.hypot( p.x() - 10, p.y() ) for p in crv )
    return check( abs( d - 2 * ( math.sqrt( 2 ) - 1 ) ) < 1e-3, "the route passes inside a rounded point" ) and ok


def offsets():
    """Rel offsets turn with the parent; Abs offsets are in the model's axes."""
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( pod, "Z_Rel_Rotation", "XForm", 90.0 )
    r = vsp.AddGeom( "ROUTING" )
    p = vsp.AddRoutingPt( r, pod, 0 )
    vsp.SetParmVal( vsp.FindParm( p, "W", "RoutePt" ), 0.0 )
    vsp.Update()
    c0 = vsp.GetMainRoutingPtCoord( p )
    vsp.SetParmVal( vsp.FindParm( p, "DeltaXRel", "RoutePt" ), 1.0 )
    vsp.Update()
    c1 = vsp.GetMainRoutingPtCoord( p )
    ok = check( abs( c1.y() - c0.y() - 1.0 ) < 1e-9 and abs( c1.x() - c0.x() ) < 1e-9, "a Rel offset turns with the parent" )
    vsp.SetParmVal( vsp.FindParm( p, "DeltaType", "RoutePt" ), vsp.ABS )
    vsp.SetParmVal( vsp.FindParm( p, "DeltaX", "RoutePt" ), 1.0 )
    vsp.SetParmVal( vsp.FindParm( p, "DeltaY", "RoutePt" ), 0.0 )
    vsp.Update()
    c2 = vsp.GetMainRoutingPtCoord( p )
    return check( abs( c2.x() - c0.x() - 1.0 ) < 1e-9 and abs( c2.y() - c0.y() ) < 1e-9, "an Abs offset is in the model's axes" ) and ok


def picked_uw_needs_uw():
    """Setting U and W moves a point only while its Coord Type is UW."""
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    r = vsp.AddGeom( "ROUTING" )
    p = vsp.AddRoutingPt( r, pod, 0 )
    vsp.Update()
    c0 = vsp.GetMainRoutingPtCoord( p )
    vsp.SetParmVal( vsp.FindParm( p, "CoordType", "RoutePt" ), vsp.ROUTE_PT_RST )
    vsp.Update()
    vsp.SetParmVal( vsp.FindParm( p, "U", "RoutePt" ), 0.2 )
    vsp.Update()
    c1 = vsp.GetMainRoutingPtCoord( p )
    return check( abs( c1.x() - c0.x() ) < 1e-9, "U is overwritten while Coord Type is RST" )


def follows_any_component():
    """A route follows a component it has a point on, though that component is not its parent: here a pod on a
    Hinge, under a Wing, while the route sits at the top of the tree."""
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    h = vsp.AddGeom( "HINGE", w )
    vsp.Update()
    n = vsp.AddGeom( "POD", h )
    vsp.SetParmVal( n, "X_Rel_Location", "XForm", 2.0 )
    f = vsp.AddGeom( "POD" )
    vsp.SetParmVal( f, "Y_Location", "XForm", -3.0 )
    r = vsp.AddGeom( "ROUTING" )
    vsp.Update()
    vsp.AddRoutingPt( r, f, 0 )
    p = vsp.AddRoutingPt( r, n, 0 )
    vsp.Update()
    a = vsp.GetMainRoutingPtCoord( p )
    l0 = vsp.GetParmVal( r, "Length", "Results" )
    vsp.SetParmVal( h, "JointRotate", "Hinge", 40.0 )
    vsp.Update()
    b = vsp.GetMainRoutingPtCoord( p )
    l1 = vsp.GetParmVal( r, "Length", "Results" )
    moved = ( b - a ).mag() > 0.1 and abs( l1 - l0 ) > 1e-3
    return check( vsp.GetGeomParent( r ) in ( "", "NONE" ) and moved,
                  "a top-level route follows a hinged pod it has a point on (length %.3f -> %.3f)" % ( l0, l1 ) )


if __name__ == "__main__":
    results = [ corner(), offsets(), picked_uw_needs_uw(), follows_any_component() ]
    print( "all passed" if all( results ) else "SOME FAILED" )
