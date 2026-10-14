"""GetPropBladeXForm and GetPropBladeMotionXForm, which place a rotor's blades at any rotation angle for
a program that moves its own mesh of them.  Each is checked against where the model puts every blade
of every symmetric copy, read from DegenGeom, which reaches the copies.
"""

import math

import openvsp as vsp

from prophelp import group

TOL = 1e-9

# Harmonic flap, lag and feather, so the matrices must carry the hinges as well as the rotation.
MOTION = {
    "BladeMotionMode": vsp.PROP_BLADE_HARMONIC, "NumHarmonics": 2,
    "FlapMean": 3.0, "FlapCos_1": 2.0, "FlapSin_1": -1.5, "FlapCos_2": 0.5,
    "LagMean": -2.0, "LagCos_1": 1.0, "LagSin_1": 2.5,
    "FeatherMean": 12.0, "FeatherCos_1": -3.0, "FeatherSin_1": 4.0,
    "Precone": 3.0, "PsiRef": 15.0,
}

# A Geom placed off the origin, with a mirror copy.
PLACE = { "X_Rel_Location": 1.5, "Y_Rel_Location": -2.0, "Z_Rel_Location": 0.5, "Y_Rel_Rotation": 20.0,
          "Z_Rel_Rotation": -10.0 }


def set_parms( pid, parms ):
    # A harmonic count makes its coefficient parms in an Update, so counts go first.
    for first in ( True, False ):
        for name, val in parms.items():
            if name.startswith( "Num" ) == first:
                g = group( name )
                if name in PLACE:
                    g = "XForm"
                assert vsp.FindParm( pid, name, g ), name
                vsp.SetParmVal( pid, name, g, val )
        vsp.Update()


def new_prop( nblade, parms, rev = False, symm = True ):
    pid = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( pid, "UseBeta34Flag", "Design", 0.0 )
    vsp.SetParmVal( pid, "ReverseFlag", "Design", float( rev ) )
    vsp.SetParmVal( pid, "NumBlade", "Design", nblade )
    if symm:
        vsp.SetParmVal( vsp.FindParm( pid, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()
    set_parms( pid, parms )
    return pid


def degen_blades( gid ):
    """Points of each surface of gid, by ( blade, symmetric copy )."""
    vsp.SetAnalysisInputDefaults( "DegenGeom" )
    vsp.SetIntAnalysisInput( "DegenGeom", "WriteCSVFlag", [ 0 ] )
    vsp.SetIntAnalysisInput( "DegenGeom", "WriteMFileFlag", [ 0 ] )
    rid = vsp.ExecAnalysis( "DegenGeom" )
    out = {}
    for dg in vsp.GetStringResults( rid, "Degen_DegenGeoms" ):
        if vsp.GetStringResults( dg, "geom_id" )[0] != gid:
            continue
        key = ( vsp.GetIntResults( dg, "main_surf_index" )[0], vsp.GetIntResults( dg, "sym_copy_index" )[0] )
        surf = vsp.GetStringResults( dg, "surf" )[0]
        xs = vsp.GetDoubleMatResults( surf, "x" )
        ys = vsp.GetDoubleMatResults( surf, "y" )
        zs = vsp.GetDoubleMatResults( surf, "z" )
        out[ key ] = [ ( x, y, z ) for rx, ry, rz in zip( xs, ys, zs ) for x, y, z in zip( rx, ry, rz ) ]
    vsp.DeleteAllResults()
    return out


def xform( m, pts ):
    out = []
    for p in pts:
        q = m.xform( vec3( p ) )
        out.append( ( q.x(), q.y(), q.z() ) )
    return out


def vec3( p ):
    return vsp.vec3d( p[0], p[1], p[2] )


def furthest( a, b ):
    assert len( a ) == len( b ) and a
    return max( math.dist( p, q ) for p, q in zip( a, b ) )


def built_blade( nblade, rev ):
    """The blade as built: the same propeller with nothing to move it, at the origin."""
    vsp.VSPRenew()
    pid = new_prop( nblade, {}, rev, symm = False )
    return degen_blades( pid )[ ( 0, 0 ) ]


def test_blades_placed_at_any_angle():
    # 400 degrees is past Rotate's limits; the model is turned to the same place, 40.
    for rev in ( False, True ):
        built = built_blade( 3, rev )
        vsp.VSPRenew()
        pid = new_prop( 3, dict( MOTION, Rotate = 40.0, **PLACE ), rev )
        placed = degen_blades( pid )
        assert len( placed ) == 6
        for ( iblade, isymm ), pts in placed.items():
            m = vsp.GetPropBladeXForm( pid, iblade, isymm, 400.0 )
            assert furthest( xform( m, built ), pts ) < TOL, ( rev, iblade, isymm )


def test_blades_moved_from_psi_zero():
    vsp.VSPRenew()
    pid = new_prop( 3, dict( MOTION, **PLACE ) )
    at_zero = degen_blades( pid )
    set_parms( pid, { "Rotate": 125.0 } )
    later = degen_blades( pid )
    for ( iblade, isymm ), pts in later.items():
        m = vsp.GetPropBladeMotionXForm( pid, iblade, isymm, 125.0 )
        assert furthest( xform( m, at_zero[ ( iblade, isymm ) ] ), pts ) < TOL, ( iblade, isymm )


def test_matrix_does_not_move_the_propeller():
    vsp.VSPRenew()
    pid = new_prop( 3, MOTION )
    before = degen_blades( pid )
    vsp.GetPropBladeXForm( pid, 1, 1, 77.0 )
    vsp.Update()
    assert vsp.GetParmVal( pid, "Rotate", "Design" ) == 0.0
    after = degen_blades( pid )
    assert max( furthest( before[k], after[k] ) for k in before ) == 0.0


def test_clone_turned_apart():
    # A Clone placed on its own and turned to its own angle: its matrices place its own blades.
    built = built_blade( 3, False )
    vsp.VSPRenew()
    pid = new_prop( 3, MOTION )
    clone = vsp.CloneGeomVec( [ pid ] )[0]
    vsp.SetParmVal( clone, "CloneRotate", "Behavior", 0.0 )
    vsp.SetParmVal( clone, "Rotate", "Rotor", 70.0 )
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 4.0 )
    vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Rotation", "XForm" ), 30.0 )
    vsp.Update()
    placed = degen_blades( clone )
    assert len( placed ) == 6
    for ( iblade, isymm ), pts in placed.items():
        m = vsp.GetPropBladeXForm( clone, iblade, isymm, 70.0 )
        assert furthest( xform( m, built ), pts ) < TOL, ( iblade, isymm )


def test_planar_and_rotational_copies():
    # With both, the Geom numbers its copies apart from the order of its transforms; symm_index
    # follows DegenGeom's sym_copy_index.
    built = built_blade( 2, False )
    vsp.VSPRenew()
    pid = new_prop( 2, dict( MOTION, **PLACE ) )
    vsp.SetParmVal( vsp.FindParm( pid, "Sym_Axial_Flag", "Sym" ), vsp.SYM_ROT_X )
    vsp.SetParmVal( vsp.FindParm( pid, "Sym_Rot_N", "Sym" ), 3 )
    vsp.Update()
    placed = degen_blades( pid )
    assert len( placed ) == 12
    for ( iblade, isymm ), pts in placed.items():
        m = vsp.GetPropBladeXForm( pid, iblade, isymm, 0.0 )
        assert furthest( xform( m, built ), pts ) < TOL, ( iblade, isymm )


def test_parms_changed_then_update():
    # After an Update the matrices follow changed parms, as for a propeller built with them at once.
    vsp.VSPRenew()
    pid = new_prop( 3, { "FlapAngle": 10.0, "RFlapAx": 0.05 } )
    vsp.SetParmVal( pid, "FlapAngle", "Articulation", 20.0 )
    vsp.SetParmVal( pid, "RFlapAx", "Articulation", 0.4 )
    vsp.SetParmVal( pid, "NumBlade", "Design", 5 )
    vsp.Update()
    got = [ vsp.GetPropBladeXForm( pid, 4, 1, 30.0 ), vsp.GetPropBladeMotionXForm( pid, 4, 1, 30.0 ) ]
    vsp.VSPRenew()
    pid = new_prop( 5, { "FlapAngle": 20.0, "RFlapAx": 0.4 } )
    want = [ vsp.GetPropBladeXForm( pid, 4, 1, 30.0 ), vsp.GetPropBladeMotionXForm( pid, 4, 1, 30.0 ) ]
    p = [ ( 0.1, 5.0, 0.3 ) ]
    for a, b in zip( got, want ):
        assert furthest( xform( a, p ), xform( b, p ) ) < TOL


def drop_errors():
    mgr = vsp.ErrorMgrSingleton.getInstance()
    return [ mgr.PopLastError().GetErrorCode() for _ in range( mgr.GetNumTotalErrors() ) ]


def is_identity( m ):
    p = m.xform( vsp.vec3d( 0.1, 2.0, 0.3 ) )
    return abs( p.x() - 0.1 ) < 1e-15 and abs( p.y() - 2.0 ) < 1e-15 and abs( p.z() - 0.3 ) < 1e-15


def test_bad_arguments():
    vsp.VSPRenew()
    pid = new_prop( 3, {} )
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    drop_errors()
    for args, code in ( ( ( pid, 3, 0, 0.0 ), vsp.VSP_INDEX_OUT_RANGE ),
                        ( ( pid, -1, 0, 0.0 ), vsp.VSP_INDEX_OUT_RANGE ),
                        ( ( pid, 0, 2, 0.0 ), vsp.VSP_INDEX_OUT_RANGE ),
                        ( ( pod, 0, 0, 0.0 ), vsp.VSP_WRONG_GEOM_TYPE ) ):
        for call in ( vsp.GetPropBladeXForm, vsp.GetPropBladeMotionXForm ):
            assert is_identity( call( *args ) ), ( call.__name__, args )
            assert drop_errors()[:1] == [ code ], ( call.__name__, args )


def test_a_blade_added_since_the_last_update_is_out_of_range():
    vsp.VSPRenew()
    pid = new_prop( 3, {} )
    vsp.Update()
    vsp.SetParmVal( vsp.FindParm( pid, "NumBlade", "Design" ), 4 )
    drop_errors()
    assert is_identity( vsp.GetPropBladeXForm( pid, 3, 0, 0.0 ) )
    assert drop_errors()[:1] == [ vsp.VSP_INDEX_OUT_RANGE ]
    vsp.Update()
    assert not is_identity( vsp.GetPropBladeXForm( pid, 3, 0, 0.0 ) )
    assert drop_errors() == []
