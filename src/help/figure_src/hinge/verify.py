"""Checks the signs and frames Hinge.md states against the openvsp module.

    <build>/venv/bin/python verify.py
"""

import numpy as np
import openvsp as vsp


def check( ok, what ):
    print( ( "ok    " if ok else "FAIL  " ) + what )
    return ok


def nose( g ):
    p = vsp.CompPnt01( g, 0, 0.0, 0.0 )
    return np.array( [ p.x(), p.y(), p.z() ] )


def hinge_with_pod():
    vsp.VSPRenew()
    h = vsp.AddGeom( "HINGE" )
    pod = vsp.AddGeom( "POD", h )
    vsp.SetParmVal( pod, "Y_Rel_Location", "XForm", 2.0 )
    vsp.Update()
    return h, pod


def motion():
    h, pod = hinge_with_pod()
    ok = True
    vsp.SetParmVal( h, "JointRotate", "Hinge", 90.0 )
    vsp.Update()
    ok = check( np.abs( nose( pod ) - [ 0, 0, 2 ] ).max() < 1e-9, "Rotate is right-handed about the axis: +Y turns to +Z" ) and ok
    vsp.SetParmVal( h, "JointRotate", "Hinge", 0.0 )
    vsp.SetParmVal( h, "JointTranslateFlag", "Hinge", 1 )
    vsp.SetParmVal( h, "JointTranslate", "Hinge", 3.0 )
    vsp.Update()
    ok = check( np.abs( nose( pod ) - [ 3, 2, 0 ] ).max() < 1e-9, "Translate slides along the axis" ) and ok
    vsp.SetParmVal( h, "JointTranslateFlag", "Hinge", 0 )
    vsp.Update()
    ok = check( vsp.GetParmVal( h, "JointTranslate", "Hinge" ) == 0.0, "a motion not allowed is held at zero" ) and ok
    vsp.SetParmVal( pod, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_NONE )
    vsp.SetParmVal( pod, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_NONE )
    vsp.SetParmVal( h, "JointRotate", "Hinge", 90.0 )
    vsp.Update()
    ok = check( np.abs( nose( pod ) - [ 0, 0, 2 ] ).max() < 1e-9, "a child rides the joint whatever its attachment" ) and ok
    return ok


def flip():
    h, pod = hinge_with_pod()
    ok = True
    vsp.SetParmVal( h, "JointTranslateFlag", "Hinge", 1 )
    vsp.SetParmVal( h, "JointTranslate", "Hinge", 3.0 )
    vsp.SetParmVal( h, "Flip_Flag", "Sym", vsp.SYM_YZ )
    vsp.Update()
    ok = check( np.abs( nose( pod ) - [ -3, 2, 0 ] ).max() < 1e-9, "a flip square to the axis reverses Translate" ) and ok
    vsp.SetParmVal( h, "JointTranslate", "Hinge", 0.0 )
    vsp.SetParmVal( h, "JointRotate", "Hinge", 90.0 )
    vsp.SetParmVal( h, "Flip_Flag", "Sym", vsp.SYM_XY )
    vsp.Update()
    ok = check( np.abs( nose( pod ) - [ 0, 0, -2 ] ).max() < 1e-9, "a flip containing the axis reverses Rotate" ) and ok
    return ok


def reference_vector():
    """The reference vector orients the Hinge with every Type, POINT3D included."""
    vsp.VSPRenew()
    h = vsp.AddGeom( "HINGE" )
    vsp.SetParmVal( h, "OrientRotFlag", "Hinge", vsp.HINGE_ORIENT_VEC )
    vsp.SetParmVal( h, "PrimType", "Hinge", vsp.HINGE_PRIM_POINT3D )
    vsp.SetParmVal( h, "PrimXOffRel", "Hinge", 1.0 )
    vsp.SetParmVal( h, "PrimYOffRel", "Hinge", 1.0 )
    rots = []
    for sd in ( vsp.X_DIR, vsp.Z_DIR ):
        vsp.SetParmVal( h, "SecondaryVecDir", "Hinge", sd )
        vsp.Update()
        rots.append( [ vsp.GetParmVal( h, n, "XForm" ) for n in ( "X_Rotation", "Y_Rotation", "Z_Rotation" ) ] )
    return check( np.abs( np.array( rots[0] ) - rots[1] ).max() > 1.0, "the reference vector turns a POINT3D Hinge" )


def reference_plane():
    """With Vectors, the Reference axis lies in the plane of the hinge axis and the reference vector, square to the axis."""
    vsp.VSPRenew()
    h = vsp.AddGeom( "HINGE" )
    d = np.array( [ 0.5, -0.785, 0.366 ] )
    d = d / np.linalg.norm( d )
    vsp.SetParmVal( h, "OrientRotFlag", "Hinge", vsp.HINGE_ORIENT_VEC )
    vsp.SetParmVal( h, "PrimType", "Hinge", vsp.HINGE_PRIM_VECTOR3D )
    vsp.SetParmVal( h, "PrimVecAbsRelFlag", "Hinge", vsp.ABS )
    for n, v in zip( ( "PrimXVec", "PrimYVec", "PrimZVec" ), d ):
        vsp.SetParmVal( h, n, "Hinge", v )
    vsp.SetParmVal( h, "SecondaryVecDir", "Hinge", vsp.Z_DIR )
    vsp.SetParmVal( h, "SecVecAbsRelFlag", "Hinge", vsp.ABS )
    pods = []
    for k in range( 2 ):
        p = vsp.AddGeom( "BLANK", h )
        vsp.SetParmVal( p, ( "X_Rel_Location", "Y_Rel_Location" )[ k ], "XForm", 1.0 )
        pods.append( p )
    vsp.Update()
    ax = [ np.array( [ vsp.GetParmVal( p, n, "XForm" ) for n in ( "X_Location", "Y_Location", "Z_Location" ) ] ) for p in pods ]
    ref = np.array( [ 0, 0, 1.0 ] ) - d[2] * d
    ref = ref / np.linalg.norm( ref )
    return check( np.abs( ax[0] - d ).max() < 1e-9 and np.abs( ax[1] - ref ).max() < 1e-9,
                  "the Reference axis is the reference vector made square to the hinge axis" )


def every_pair():
    """With Vectors, every choice of Axis and Reference puts the Axis on the direction vector and the Reference on the
    reference vector made square to it, in a right-handed frame."""
    d = np.array( [ 0.5, -0.785, 0.366 ] )
    d = d / np.linalg.norm( d )
    ref = np.array( [ 0, 0, 1.0 ] ) - d[2] * d
    ref = ref / np.linalg.norm( ref )
    ok = True
    for p in range( 3 ):
        for s in range( 3 ):
            if s == p:
                continue
            vsp.VSPRenew()
            h = vsp.AddGeom( "HINGE" )
            vsp.SetParmVal( h, "OrientRotFlag", "Hinge", vsp.HINGE_ORIENT_VEC )
            vsp.SetParmVal( h, "PrimType", "Hinge", vsp.HINGE_PRIM_VECTOR3D )
            vsp.SetParmVal( h, "PrimVecAbsRelFlag", "Hinge", vsp.ABS )
            for n, v in zip( ( "PrimXVec", "PrimYVec", "PrimZVec" ), d ):
                vsp.SetParmVal( h, n, "Hinge", v )
            vsp.SetParmVal( h, "PrimaryDir", "Hinge", p )
            vsp.SetParmVal( h, "SecondaryDir", "Hinge", s )
            vsp.SetParmVal( h, "SecondaryVecDir", "Hinge", vsp.Z_DIR )
            vsp.SetParmVal( h, "SecVecAbsRelFlag", "Hinge", vsp.ABS )
            blanks = []
            for k in range( 3 ):
                b = vsp.AddGeom( "BLANK", h )
                vsp.SetParmVal( b, ( "X_Rel_Location", "Y_Rel_Location", "Z_Rel_Location" )[ k ], "XForm", 1.0 )
                blanks.append( b )
            vsp.Update()
            ax = [ np.array( [ vsp.GetParmVal( b, n, "XForm" ) for n in ( "X_Location", "Y_Location", "Z_Location" ) ] )
                   for b in blanks ]
            ok = ok and np.abs( ax[ p ] - d ).max() < 1e-9 and np.abs( ax[ s ] - ref ).max() < 1e-9
            ok = ok and np.dot( np.cross( ax[ 0 ], ax[ 1 ] ), ax[ 2 ] ) > 1.0 - 1e-9
    return check( ok, "every Axis and Reference pair gives a right-handed frame on the two vectors" )


def chain():
    """A Hinge carried by a child of another Hinge turns about its own axis as the first one has moved it."""
    vsp.VSPRenew()
    h1 = vsp.AddGeom( "HINGE" )
    b = vsp.AddGeom( "BLANK", h1 )
    vsp.SetParmVal( b, "Y_Rel_Location", "XForm", 2.0 )
    h2 = vsp.AddGeom( "HINGE", b )
    vsp.SetParmVal( h2, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( h2, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_COMP )
    pod = vsp.AddGeom( "POD", h2 )
    vsp.SetParmVal( pod, "Z_Rel_Location", "XForm", 1.0 )
    vsp.SetParmVal( h1, "JointRotate", "Hinge", 90.0 )
    vsp.SetParmVal( h2, "JointRotate", "Hinge", 90.0 )
    vsp.Update()
    # h2 turns the pod's offset ( 0, 0, 1 ) to ( 0, -1, 0 ), so ( 0, 1, 0 ) in h1's joint frame; h1 turns that to ( 0, 0, 1 ).
    return check( np.abs( nose( pod ) - [ 0, 0, 1 ] ).max() < 1e-9, "a chain of Hinges composes their turns" )


if __name__ == "__main__":
    results = [ motion(), flip(), reference_vector(), reference_plane(), every_pair(), chain() ]
    print( "all passed" if all( results ) else "SOME FAILED" )
