# How far a Conformal reaches along the Geom it is conformal to.
#
# Each end of a Conformal is moved in from the parent's end by the offset.  On a wing that is
# all: the sections are already offset, and a wing's end is a section, not a point.  3.52.0
# walked a wing Conformal's ends in until they were clear of their own offset surface by the
# offset, which a tapered wing never is, and the default wing Conformal covered only the middle
# third of the span.

import openvsp as vsp
import pytest

from testhelp import drop_errors


def box( gid ):
    return { p: vsp.GetParmVal( vsp.FindParm( gid, p, "BBox" ) )
             for p in ( "X_Min", "Y_Min", "Z_Min", "X_Len", "Y_Len", "Z_Len" ) }


def conformal_of( typ, setup=None, offset=None ):
    vsp.VSPRenew()
    drop_errors()
    parent = vsp.AddGeom( typ )
    sym = vsp.FindParm( parent, "Sym_Planar_Flag", "Sym" )
    if sym:
        vsp.SetParmVal( sym, 0 )
    if setup:
        setup( parent )
    vsp.Update()
    conf = vsp.AddGeom( "CONFORMAL", parent )
    if offset is not None:
        vsp.SetParmVal( vsp.FindParm( conf, "Offset", "Design" ), offset )
    vsp.Update()
    return parent, conf


def caps( gid, cap_type ):
    vsp.SetParmVal( vsp.FindParm( gid, "CapUMinOption", "EndCap" ), cap_type )
    vsp.SetParmVal( vsp.FindParm( gid, "CapUMaxOption", "EndCap" ), cap_type )


@pytest.mark.parametrize( "setup", [ None,
                                     lambda g: caps( g, vsp.ROUND_END_CAP ),
                                     lambda g: vsp.SetParmVal( vsp.FindParm( g, "Tip_Chord", "XSec_1" ), 0.0 ),
                                     lambda g: vsp.InsertXSec( g, 1, vsp.XS_FOUR_SERIES ) ],
                          ids=[ "flat caps", "round caps", "pointed tip", "two sections" ] )
def testAWingConformalReachesAlmostToEachEnd( setup ):
    wing, conf = conformal_of( "WING", setup )
    w = box( wing )
    c = box( conf )

    # Each end in by about the offset, not by a fraction of the span.
    assert c[ "Y_Min" ] - w[ "Y_Min" ] < 0.3, ( w, c )
    assert ( w[ "Y_Min" ] + w[ "Y_Len" ] ) - ( c[ "Y_Min" ] + c[ "Y_Len" ] ) < 0.3, ( w, c )


def testAWingConformalEndIsTheOffsetInFromTheRoot():
    wing, conf = conformal_of( "WING", offset=0.3 )

    # The root end moves by the offset measured along the swept spine, so a little less in y.
    y_in = box( conf )[ "Y_Min" ] - box( wing )[ "Y_Min" ]
    assert 0.2 < y_in < 0.3, y_in


def testABodyConformalStaysInsideItsParent():
    """The end search is kept for bodies, where a pointed nose needs it."""
    fuse, conf = conformal_of( "FUSELAGE" )
    f = box( fuse )
    c = box( conf )

    # The nose is a point, so the end is walked in past the offset until it clears the walls.
    assert c[ "X_Min" ] > f[ "X_Min" ] + 0.1
    for axis in ( "Y", "Z" ):
        assert c[ axis + "_Min" ] > f[ axis + "_Min" ]
        assert c[ axis + "_Min" ] + c[ axis + "_Len" ] < f[ axis + "_Min" ] + f[ axis + "_Len" ]


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
