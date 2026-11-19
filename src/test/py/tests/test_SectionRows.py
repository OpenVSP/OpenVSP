# The rows of a section made of several patches are even steps along the whole section, wherever its patches meet.

import openvsp as vsp
import pytest

from testhelp import assert_no_errors


def first_rows( gid, n ):
    """The first n rows of the main surface, in its normalized u."""
    u, w = vsp.GetUWTess01( gid, 0 )
    return u[:n]


def even( rows ):
    steps = [ rows[i + 1] - rows[i] for i in range( len( rows ) - 1 ) ]
    return max( steps ) - min( steps ) < 1e-12


def among( coarse, fine ):
    return all( min( abs( a - b ) for b in fine ) < 1e-12 for a in coarse )


@pytest.mark.parametrize( "n", ( 4, 5, 8, 13 ) )
def testACobrasFirstSectionHasEvenRows( n ):
    """A Cobra's first section is several patches; its rows are even steps along it."""
    vsp.VSPRenew()
    gid = vsp.AddGeom( "COBRA" )
    vsp.SetParmVal( gid, "Tess_U", "Shape", n )
    vsp.Update()
    assert even( first_rows( gid, n ) )
    assert_no_errors()


def testARocketNosesRowsAreEvenStepsAlongIt():
    """The Generic Rocket's Von Karman nose is many patches; the rows of a coarser tessellation are among those of a
    finer one."""
    vsp.VSPRenew()
    gid = vsp.AddGeom( "MDC_BOD" )
    vsp.SetMdcPreset( gid, vsp.MDC_BOD_PRESET_GENERIC_ROCKET )
    rows = {}
    for n in ( 3, 5, 9, 17 ):
        vsp.SetParmVal( gid, "NoseTess_U", "Shape", n )
        vsp.Update()
        rows[n] = first_rows( gid, n )
        assert even( rows[n] )
    assert among( rows[3], rows[5] )
    assert among( rows[5], rows[9] )
    assert among( rows[9], rows[17] )
    assert_no_errors()
