"""Each side's chevron Parms drive that side."""

import openvsp as vsp


def chevron_fuselage():
    vsp.VSPRenew()
    gid = vsp.AddGeom( "FUSELAGE" )
    xs = vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), 2 )
    curve = vsp.GetParmContainer( vsp.GetXSecParm( xs, "Chevron_Type" ) )
    for n in ( "AllSym", "RLSym", "TBSym" ):
        vsp.SetParmVal( vsp.FindParm( curve, n, "Chevron" ), 0 )
    vsp.SetParmVal( vsp.FindParm( curve, "Chevron_Type", "Chevron" ), vsp.CHEVRON_FULL )
    return gid, curve


def points( gid ):
    vsp.Update()
    return [ vsp.CompPnt01( gid, 0, 0.5, i / 400.0 ) for i in range( 400 ) ]


def side_moved( name, value ):
    """The y of the point a Parm moves most: the side it drives."""
    gid, curve = chevron_fuselage()
    a = points( gid )
    vsp.SetParmVal( vsp.FindParm( curve, name, "Chevron" ), value )
    b = points( gid )
    d = [ vsp.dist( p, q ) for p, q in zip( a, b ) ]
    return a[ d.index( max( d ) ) ].y()


def test_each_amplitude_drives_the_side_its_angle_drives():
    for side in ( "Right", "Left" ):
        y_angle = side_moved( side + "_Angle", 30.0 )
        y_amp = side_moved( side + "Amplitude", 3.0 )
        assert y_angle * y_amp > 0, ( side, y_angle, y_amp )
