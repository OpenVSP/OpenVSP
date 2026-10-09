# Control surface subsurfaces on the Geoms that can and cannot carry one.
#
# A control surface is built from a wing-like section, so only a Wing, a Prop, or a Clone of
# either can have one.  The API used to accept it on any Geom and on any main surface index,
# and the model crashed on the next Update.

import openvsp as vsp

from testhelp import ( pop_errors, drop_errors, assert_no_errors )


def fresh():
    vsp.VSPRenew()
    drop_errors()


def testAControlSurfaceGoesOnAWingAPropAndAClone():
    fresh()
    wing = vsp.AddGeom( "WING" )
    prop = vsp.AddGeom( "PROP" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.Update()

    for gid in ( wing, prop, clone ):
        ss = vsp.AddSubSurf( gid, vsp.SS_CONTROL, 0 )
        assert ss, "no control surface on %s" % vsp.GetGeomTypeName( gid )
    vsp.Update()
    assert_no_errors()


def testAControlSurfaceIsRefusedOnOtherGeoms():
    fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    aux = vsp.AddGeom( "AUXILIARY", pod )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ pod ] )[0]
    vsp.Update()

    for gid in ( pod, aux, clone ):
        ss = vsp.AddSubSurf( gid, vsp.SS_CONTROL, 0 )
        assert not ss, "a control surface was added to %s" % vsp.GetGeomTypeName( gid )
        assert pop_errors(), "refusing it raised no error"
        assert vsp.GetNumSubSurf( gid ) == 0
    vsp.Update()


def testASubSurfaceOnAMissingMainSurfaceIsRefused():
    fresh()
    wing = vsp.AddGeom( "WING" )
    vsp.Update()

    ss = vsp.AddSubSurf( wing, vsp.SS_CONTROL, 3 )
    assert not ss, "a control surface was added to main surface 3 of a one-surface wing"
    assert pop_errors(), "refusing it raised no error"

    ss = vsp.AddSubSurf( wing, vsp.SS_RECTANGLE, -2 )
    assert not ss
    assert pop_errors()

    ss = vsp.AddSubSurf( wing, vsp.SS_RECTANGLE, -1 )
    assert ss, "-1 (every main surface) was refused"
    assert_no_errors()


def testAControlSurfaceMovedOffTheMainSurfacesStillUpdates():
    """The index is a Parm, so it can still be set past the main surfaces after the add."""
    fresh()
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wing, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
    vsp.Update()
    ss = vsp.AddSubSurf( wing, vsp.SS_CONTROL, 0 )
    vsp.Update()

    indx = vsp.FindParm( ss, "MainSurfIndx", "SubSurface" )
    vsp.SetParmVal( indx, 5 )
    vsp.Update()
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    drop_errors()
