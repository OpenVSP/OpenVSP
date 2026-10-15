"""Which of a Conformal's controls act, by its parent."""

import openvsp as vsp


def conformal_on( parent_type ):
    vsp.VSPRenew()
    pid = vsp.AddGeom( parent_type )
    cid = vsp.AddGeom( "CONFORMAL", pid )
    vsp.Update()
    return cid


def test_offset_ends_acts_only_on_a_parent_built_from_ribs():
    for parent, active in ( ( "FUSELAGE", True ), ( "STACK", True ), ( "WING", True ), ( "POD", False ),
                            ( "ELLIPSOID", False ), ( "BODYOFREVOLUTION", False ), ( "COBRA", False ) ):
        cid = conformal_on( parent )
        assert vsp.GetParmActiveFlag( vsp.FindParm( cid, "OffSetEnds", "Design" ) ) == active, parent


def test_a_conformals_symmetry_controls_are_inactive_attached_or_detached():
    cid = conformal_on( "WING" )
    for detached in ( 0.0, 1.0 ):
        vsp.SetParmVal( cid, "DetachFlag", "Design", detached )
        vsp.Update()
        assert not vsp.GetParmActiveFlag( vsp.GetParm( cid, "Sym_Planar_Flag", "Sym" ) ), detached
