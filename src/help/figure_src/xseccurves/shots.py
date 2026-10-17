"""Screenshots of the Edit Curve window for XSecCurves.md.  See ../screenshots/snap.py."""
import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "screenshots" ) )
import snap
import openvsp as vsp


def model():
    """A Fuselage whose middle section is a Cubic Bezier Edit Curve, R-L Symmetry off, of no symmetry."""
    g = vsp.AddGeom( "FUSELAGE" )
    s = vsp.GetXSecSurf( g, 0 )
    vsp.SetParmVal( vsp.GetParm( g, "ActiveXSec", "Index" ), 2 )
    vsp.ConvertXSecToEdit( g, 2 )
    xs = vsp.GetXSec( s, 2 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "SymType" ), vsp.SYM_NONE )
    pts = [ ( 0.5, 0.0 ), ( 0.5, -0.3 ), ( 0.35, -0.5 ),
            ( 0.05, -0.5 ), ( -0.2, -0.5 ), ( -0.5, -0.35 ),
            ( -0.5, -0.05 ), ( -0.5, 0.25 ), ( -0.35, 0.5 ),
            ( 0.1, 0.5 ), ( 0.3, 0.5 ), ( 0.5, 0.4 ),
            ( 0.5, 0.0 ) ]
    u = [ i / 12.0 for i in range( 13 ) ]
    vsp.SetEditXSecPnts( xs, u, [ vsp.vec3d( x, y, 0.0 ) for x, y in pts ], [ 0.0 ] * 13 )
    vsp.Update()


snap.shots( model, "VSP_CURVE_EDIT_SCREEN", { "XSecCurves_cedit_window": ( "Edit_Curve_Curve", ( 0, 0, 750, 632 ) ) },
            name = "xseccurves_cedit" )
