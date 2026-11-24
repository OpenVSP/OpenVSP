"""A Geom's point mass is counted once for each symmetric copy of the Geom, however many surfaces it has."""

import pytest

import openvsp as vsp


def total_mass():
    """The total mass, zero when there is nothing with mass to report."""
    vsp.ComputeMassProps( vsp.SET_ALL, 20, vsp.X_DIR )
    if vsp.GetNumResults( "Mass_Properties" ) == 0:
        return 0.0
    rid = vsp.FindLatestResultsID( "Mass_Properties" )
    return vsp.GetDoubleResults( rid, "Total_Mass" )[0]


def added_mass( typ, point_mass, sym=0, setup=None ):
    """The mass Mass Properties adds for the Geom's point mass."""
    masses = []
    for pm in ( 0.0, point_mass ):
        vsp.VSPRenew()
        gid = vsp.AddGeom( typ )
        if setup:
            setup( gid )
        vsp.SetParmVal( gid, "Sym_Planar_Flag", "Sym", sym )
        vsp.SetParmVal( gid, "Y_Rel_Location", "XForm", 2.0 )
        vsp.SetParmVal( gid, "PointMass", "Mass_Props", pm )
        vsp.Update()
        masses.append( total_mass() )
    return masses[1] - masses[0]


def human_shape( rep ):
    def setup( gid ):
        vsp.SetParmVal( gid, "Representation", "Design", rep )
    return setup


@pytest.mark.parametrize( "rep", [0, 1] )
@pytest.mark.parametrize( "sym, copies", [( 0, 1 ), ( vsp.SYM_XZ, 2 )] )
def test_a_human_point_mass_counts_once_per_copy( rep, sym, copies ):
    # The surface Human is built of many parts; its point mass is still the person's, once (#405)
    assert added_mass( "HUMAN", 80.0, sym, human_shape( rep ) ) == pytest.approx( 80.0 * copies, rel=1e-9 )


def test_a_prop_point_mass_counts_once_not_per_blade():
    assert added_mass( "PROP", 80.0 ) == pytest.approx( 80.0, rel=1e-9 )


@pytest.mark.parametrize( "sym, copies", [( 0, 1 ), ( vsp.SYM_XZ, 2 )] )
def test_a_blank_point_mass_counts_once_per_copy( sym, copies ):
    assert added_mass( "BLANK", 50.0, sym ) == pytest.approx( 50.0 * copies, rel=1e-9 )
