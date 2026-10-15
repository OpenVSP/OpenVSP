"""A NumPy model of how OpenVSP's PropGeom shapes and places a blade.

It mirrors src/geom_core/PropGeom.cpp (PropPositioner::Update, BladeToProp, BladeAxisToProp,
ApplyHinge, PropGeom::RigidBladeMotion and the hinge set-up in UpdateSurf).  verify.py checks it
against the openvsp module; the figures are drawn from it.

Every matrix here is a 4x4 homogeneous matrix acting on column vectors, p' = M p, and every
rotation is right-handed about the axis given.  Matrix4d post-multiplies (mat = mat * M), so a
chain written in code order hub to tip is the same product as written here, and the right-most
factor acts on the points first.

Angles are in degrees.  Lengths are absolute (the default radius is 15).
"""

import numpy as np

# Default hinge stations e/R.  RFeatherAx is the coming parm; its value only places the feather
# bearing in the chain, because sliding a bearing along its own axis changes nothing.
DEFAULT_ER = { "flap": 0.05, "lag": 0.10, "feather": 0.15, "fold": 0.20 }

# Ties in e/R break in this order.
TIE_ORDER = [ "flap", "lag", "fold", "feather" ]


# ---------------------------------------------------------------- elementary matrices

def T( v ):
    m = np.eye( 4 )
    m[ :3, 3 ] = v
    return m


def R3( axis, deg ):
    """Right-handed rotation by deg about the unit vector axis (3x3)."""
    a = np.asarray( axis, float )
    a = a / np.linalg.norm( a )
    t = np.radians( deg )
    c = np.cos( t )
    s = np.sin( t )
    K = np.array( [ [ 0, -a[2], a[1] ], [ a[2], 0, -a[0] ], [ -a[1], a[0], 0 ] ] )
    return np.eye( 3 ) + s * K + ( 1 - c ) * K @ K


def R( axis, deg ):
    m = np.eye( 4 )
    m[ :3, :3 ] = R3( axis, deg )
    return m


def Rx( deg ):
    return R( [ 1, 0, 0 ], deg )


def Ry( deg ):
    return R( [ 0, 1, 0 ], deg )


def Rz( deg ):
    return R( [ 0, 0, 1 ], deg )


def J( p, axis, deg ):
    """Turn by deg, right-handed, about the line through p along axis."""
    p = np.asarray( p, float )
    return T( p ) @ R( axis, deg ) @ T( -p )


def apply( M, pts ):
    """Apply a 4x4 to an (..., 3) array of points."""
    pts = np.asarray( pts, float )
    flat = pts.reshape( -1, 3 )
    out = flat @ M[ :3, :3 ].T + M[ :3, 3 ]
    return out.reshape( pts.shape )


def unit_or( v ):
    v = np.asarray( v, float )
    return v / np.linalg.norm( v )


def apply_dir( M, v ):
    return np.asarray( v, float ) @ M[ :3, :3 ].T


# ---------------------------------------------------------------- blade axes B

def blade_to_prop( b, rev ):
    """BladeToProp: x_B aft = rev*Z, y_B radial = Y, z_B toward thrust = -X."""
    b = np.asarray( b, float )
    return np.array( [ -b[2], b[1], rev * b[0] ] )


def blade_axis_to_prop( a, rev ):
    """BladeAxisToProp: an axis of rotation picks up a factor rev under the reflection."""
    return blade_to_prop( a, rev ) * rev


def B_matrix( rev ):
    """3x3 whose columns are x_B, y_B, z_B in propeller axes."""
    return np.column_stack( [ blade_to_prop( e, rev ) for e in np.eye( 3 ) ] )


# ---------------------------------------------------------------- hinges

def hinge_axes_B( p ):
    """Right-handed axis for a positive deflection of each hinge, in blade axes B.

    The code stores the opposite vector (or the same vector with Matrix4d::rotate, which turns
    by minus the angle); these are the axes about which a positive angle is a right-handed turn.
    """
    ex = np.array( [ 1.0, 0, 0 ] )
    ey = np.array( [ 0, 1.0, 0 ] )
    ez = np.array( [ 0, 0, 1.0 ] )
    flap = R3( ey, p[ "AzFlapDir" ] ) @ R3( ez, -p[ "ElFlapDir" ] ) @ ex
    lag = R3( ey, p[ "AzLagDir" ] ) @ R3( ex, -p[ "CantLagDir" ] ) @ ( -ez )
    fold = R3( ey, p[ "AzFoldDir" ] ) @ R3( ez, -p[ "ElFoldDir" ] ) @ ( -ex )
    feather = ey
    return { "flap": flap, "lag": lag, "fold": fold, "feather": feather }


def hinge_points_B( p, radius, croot ):
    """A point on each hinge line in blade axes B (all deflections zero)."""
    def loc( tag ):
        return np.array( [ p[ "Off%sAx" % tag ], p[ "R%sAx" % tag ], -p[ "Ax%sAx" % tag ] ] ) * radius
    return { "flap": loc( "Flap" ), "lag": loc( "Lag" ), "fold": loc( "Fold" ),
             "feather": np.array( [ croot * p[ "FeatherOffsetXoC" ], p.get( "RFeatherAx", DEFAULT_ER[ "feather" ] ) * radius, 0.0 ] ) }


def hinges_prop( p, radius, croot ):
    """Each hinge's point and right-handed axis in propeller axes, before precone."""
    rev = -1.0 if p[ "ReverseFlag" ] else 1.0
    axB = hinge_axes_B( p )
    ptB = hinge_points_B( p, radius, croot )
    out = {}
    for k in axB:
        out[ k ] = ( blade_to_prop( ptB[ k ], rev ), blade_axis_to_prop( axB[ k ], rev ) )
    return out


ANGLE_PARM = { "flap": "FlapAngle", "lag": "LagAngle", "fold": "FoldAngle", "feather": "Feather" }

# The harmonic parms of the hinges that take one: FlapMean, FlapCos_n, FlapSin_n, ...
HARMONIC_NAME = { "flap": "Flap", "lag": "Lag", "feather": "Feather" }

COLLECTIVE, INDIVIDUAL, HARMONIC = 0, 1, 2


def blade_psi( p, iblade, rotate ):
    """GetBladePsi: the blade's azimuth psi, Rotate plus its BladeAz, in the direction of rotation."""
    psi = rotate
    if iblade > 0:
        psi += p[ "BladeAz_%d" % ( iblade + 1 ) ]
    return psi


def harmonic( p, name, psi ):
    """mean + sum_n ( Cos_n cos n psi + Sin_n sin n psi ), degrees."""
    val = p[ name + "Mean" ]
    t = np.radians( psi )
    for n in range( 1, int( round( p.get( "NumHarmonics", 0 ) ) ) + 1 ):
        val += p.get( "%sCos_%d" % ( name, n ), 0.0 ) * np.cos( n * t ) + p.get( "%sSin_%d" % ( name, n ), 0.0 ) * np.sin( n * t )
    return val


def blade_angles( p, iblade, rotate ):
    """Each hinge's angle on blade iblade (0 based) with the propeller turned to rotate.

    BladeMotionMode picks the flap, lag and feather: every blade at the propeller's angle
    (collective), each at its own Angle_n (individual; blade n+1), or a harmonic series in the
    blade's psi.  While Beta34 drives, the feather is collective whatever the mode.  The fold has
    its own IndividualBladeFoldFlag.
    """
    mode = int( round( p.get( "BladeMotionMode", COLLECTIVE ) ) )
    out = {}
    for k in ( "flap", "lag", "feather" ):
        m = mode
        if k == "feather" and p.get( "UseBeta34Flag", 0 ):
            m = COLLECTIVE
        if m == HARMONIC:
            out[ k ] = harmonic( p, HARMONIC_NAME[ k ], blade_psi( p, iblade, rotate ) )
        elif m == INDIVIDUAL and iblade > 0:
            out[ k ] = p[ "%s_%d" % ( ANGLE_PARM[ k ], iblade ) ]
        else:
            out[ k ] = p[ ANGLE_PARM[ k ] ]
    if p.get( "IndividualBladeFoldFlag", 0 ) and iblade > 0:
        out[ "fold" ] = p[ "FoldAngle_%d" % iblade ]
    else:
        out[ "fold" ] = p[ "FoldAngle" ]
    return out


def hinge_order( p, order = None ):
    if order is not None:
        return list( order )
    er = { k: p.get( "R%sAx" % k.capitalize(), DEFAULT_ER[ k ] ) for k in TIE_ORDER }
    er[ "feather" ] = p.get( "RFeatherAx", DEFAULT_ER[ "feather" ] )
    return sorted( TIE_ORDER, key = lambda k: ( er[ k ], TIE_ORDER.index( k ) ) )


def chain( p, radius, croot, iblade = 0, order = None, upto = None, angles = None, rotate = None ):
    """RigidBladeMotion for blade iblade (0 based): the list of ( name, matrix ) hub to tip.

    rotate plays the part of Rotate (GetPropBladeXForm's angle); angles overrides the deflections,
    which otherwise follow BladeMotionMode at rotate.
    """
    rev = -1.0 if p[ "ReverseFlag" ] else 1.0
    if rotate is None:
        rotate = p[ "Rotate" ]
    if angles is None:
        angles = blade_angles( p, iblade, rotate )
    hp = hinges_prop( p, radius, croot )
    out = []
    if iblade > 0:
        out.append( ( "BladeAz", Rx( -rev * p[ "BladeAz_%d" % ( iblade + 1 ) ] ) ) )
    out.append( ( "PsiRef", Rx( -rev * p.get( "PsiRef", 0.0 ) ) ) )
    out.append( ( "Rotate", Rx( -rev * rotate ) ) )
    out.append( ( "Precone", Rz( p[ "Precone" ] ) ) )
    for k in hinge_order( p, order ):
        pt, ax = hp[ k ]
        out.append( ( k, J( pt, ax, angles[ k ] ) ) )
    return out


def product( mats ):
    M = np.eye( 4 )
    for _, m in mats:
        M = M @ m
    return M


# ---------------------------------------------------------------- shaping (PropPositioner)

def basic_orientation( chord ):
    """XSecSurf::GetBasicTransformation( Y_DIR, Z_DIR, XS_SHIFT_MID, flip ): curve x (chord) to Z,
    curve y (thickness) to -X, curve z to Y, then shift half a chord along Z."""
    m = np.zeros( ( 4, 4 ) )
    m[ 2, 0 ] = 1.0
    m[ 0, 1 ] = -1.0
    m[ 1, 2 ] = 1.0
    m[ 3, 3 ] = 1.0
    m[ 2, 3 ] = -chord / 2.0
    return m


def shaping_steps( s ):
    """PropPositioner::Update as a list of ( label, matrix ) in POINT order (first acts first).

    s holds Chord, Reverse (+1/-1), Construct, FeatherAxis, FeatherOffset, RootChord, RootTwist,
    Twist, ZRotate, Rake, Skew, Sweep, Axial, Tangential, Radius (all absolute lengths).
    The optional projection onto the cylinder is not a matrix; it sits between the last two.
    """
    c = s[ "Chord" ]
    rev = s[ "Reverse" ]
    tr = np.radians( s[ "RootTwist" ] )
    steps = []
    steps.append( ( "scale", np.diag( [ c, c, c, 1.0 ] ) ) )
    if rev < 0:
        f = np.eye( 4 )
        f[ 0, 0 ] = -1.0
        f[ 0, 3 ] = c
        steps.append( ( "flip", f ) )
    else:
        steps.append( ( "flip", np.eye( 4 ) ) )
    steps.append( ( "basic", basic_orientation( c ) ) )
    steps.append( ( "construct", T( [ 0, 0, rev * c * ( 0.5 - s[ "Construct" ] ) ] ) ) )
    steps.append( ( "zrotate", Rz( s[ "ZRotate" ] ) ) )
    steps.append( ( "rakeskew", T( [ s[ "Rake" ], 0, rev * s[ "Skew" ] ] ) ) )
    steps.append( ( "twist", Ry( rev * s[ "Twist" ] ) ) )
    steps.append( ( "axialtan", T( [ s[ "Axial" ], 0, rev * s[ "Tangential" ] ] ) ) )
    x = -s[ "RootChord" ] * ( 0.5 - s[ "Construct" ] )
    steps.append( ( "rootconstruct", T( [ x * np.sin( tr ), 0, rev * x * np.cos( tr ) ] ) ) )
    steps.append( ( "radius", T( [ 0, s[ "Radius" ], 0 ] ) ) )
    steps.append( ( "sweep", Rx( rev * s[ "Sweep" ] ) ) )
    xb = s[ "RootChord" ] * ( 0.5 - s[ "FeatherAxis" ] )
    steps.append( ( "featheraxis", T( [ xb * np.sin( tr ), 0, rev * xb * np.cos( tr ) ] ) ) )
    steps.append( ( "featheroffset", T( [ 0, 0, rev * s[ "RootChord" ] * s[ "FeatherOffset" ] ] ) ) )
    return steps


def cylinder_wrap( pts, radius ):
    """VspCurve::ProjectOntoCylinder (Code-Eli cylinder projector): Z is wrapped onto the cylinder
    of the section's radius about the shaft as arc length; Y is discarded, X kept."""
    pts = np.asarray( pts, float )
    th = pts[ ..., 2 ] / radius
    return np.stack( [ pts[ ..., 0 ], radius * np.cos( th ), radius * np.sin( th ) ], axis = -1 )


def shape_section( s, af_xy, upto = None ):
    """Place a unit-chord airfoil ( N x 2, x from LE 0 to TE 1, y up ) as PropPositioner does.

    upto: stop after the step of that name.  s[ "Cylinder" ] true wraps onto the cylinder before
    the feather offset.
    """
    pts = np.column_stack( [ af_xy, np.zeros( len( af_xy ) ) ] )
    for name, m in shaping_steps( s ):
        if name == "featheroffset" and s.get( "Cylinder", False ):
            pts = cylinder_wrap( pts, s[ "Radius" ] )
            if upto == "cylinder":
                return pts
        pts = apply( m, pts )
        if name == upto:
            return pts
    return pts


def naca4( t = 0.12, m = 0.04, pp = 0.4, n = 61 ):
    """A closed NACA 4-digit loop TE (lower) -> LE -> TE (upper), unit chord."""
    beta = np.linspace( 0, np.pi, n )
    x = 0.5 * ( 1 - np.cos( beta ) )
    yt = 5 * t * ( 0.2969 * np.sqrt( x ) - 0.1260 * x - 0.3516 * x**2 + 0.2843 * x**3 - 0.1036 * x**4 )
    yc = np.where( x < pp, m / pp**2 * ( 2 * pp * x - x**2 ), m / ( 1 - pp )**2 * ( 1 - 2 * pp + 2 * pp * x - x**2 ) )
    up = np.column_stack( [ x, yc + yt ] )
    lo = np.column_stack( [ x, yc - yt ] )
    return np.vstack( [ lo[ ::-1 ], up[ 1: ] ] )
