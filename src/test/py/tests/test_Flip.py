# Flipping a Geom's own shape.
#
# Flip_Flag (same bits as Sym_Planar_Flag) reflects the shape about the planes of the Geom's
# own frame, before placement and symmetry, so the Geom stays where it was put.  A left wing is
# a right wing flipped about XZ.
#
# Each test uses a shape that is not symmetric about the plane, and checks a volume as well as a
# position: points reflected without reversing the faces give a negative volume.

import openvsp as vsp
import pytest
import os
import re
import tempfile

from fliphelp import ( span, yspan, reflected, flip, a_chiral_wing, signed_volume,
                       read_stl, read_cart3d, read_vspgeom, read_x3d )
from clonehelp import ( switch, box, comp_geom_of, scratch_output, drop_errors,
                        assert_no_errors, a_mesh, a_wireframe, a_point_cloud, a_polygon_mesh,
                        a_route )


def fresh():
    vsp.VSPRenew()
    drop_errors()
    return scratch_output()


def at( gid, x = 0.0, y = 0.0, z = 0.0 ):
    for axis, val in zip( "XYZ", ( x, y, z ) ):
        vsp.SetParmVal( vsp.FindParm( gid, axis + "_Rel_Location", "XForm" ), val )
    vsp.Update()


def no_symmetry( gid ):
    vsp.SetParmVal( vsp.FindParm( gid, "Sym_Planar_Flag", "Sym" ), 0 )
    vsp.Update()


def located( gid, x = 0.0, y = 0.0, z = 0.0 ):
    """Set the Geom's location without updating."""
    for axis, val in zip( "XYZ", ( x, y, z ) ):
        vsp.SetParmVal( vsp.FindParm( gid, axis + "_Rel_Location", "XForm" ), val )


# The coordinate each plane reflects.
PLANE_AXIS = { vsp.SYM_XY: 2, vsp.SYM_XZ: 1, vsp.SYM_YZ: 0 }


def reflect( p, flag, about = ( 0.0, 0.0, 0.0 ) ):
    """A point reflected about the planes in flag, through the point 'about'."""
    out = list( p )
    for plane, i in PLANE_AXIS.items():
        if flag & plane:
            out[i] = 2.0 * about[i] - out[i]
    return tuple( out )


def xyz( v ):
    return ( v.x(), v.y(), v.z() )


def mass_props():
    vsp.ComputeMassProps( vsp.SET_ALL, 20, vsp.X_DIR )
    return vsp.FindLatestResultsID( "Mass_Properties" )


def total_cg():
    return xyz( list( vsp.GetVec3dResults( mass_props(), "Total_CG" ) )[0] )


def _only_in_a_set( gid, others ):
    only = 4
    vsp.SetSetFlag( gid, only, True )
    for other in others:
        vsp.SetSetFlag( other, only, False )
    vsp.Update()
    return only


#==== The surface Geoms ====#
# Measured in x about YZ: each starts at its origin and runs aft, so a YZ flip moves it forward.

SURFACE_TYPES = [ "POD", "WING", "FUSELAGE", "STACK", "BODYOFREVOLUTION", "PROP" ]


@pytest.mark.parametrize( "kind", SURFACE_TYPES )
def testAFlipReflectsTheShapeAboutTheGeomsOwnOrigin( kind ):
    fresh()
    gid = vsp.AddGeom( kind )
    no_symmetry( gid )
    at( gid, x = 7.0 )
    before = span( gid, "X" )
    assert before != pytest.approx( reflected( before, about = 7.0 ) ), \
           "the shape is symmetric about its own YZ plane, so this would measure nothing"

    flip( gid, vsp.SYM_YZ )
    assert span( gid, "X" ) == pytest.approx( reflected( before, about = 7.0 ) )

    # The Geom's reported location does not move.
    assert vsp.GetParmVal( vsp.FindParm( gid, "X_Location", "XForm" ) ) == pytest.approx( 7.0 )
    assert_no_errors()


@pytest.mark.parametrize( "kind", SURFACE_TYPES )
@pytest.mark.parametrize( "flag", [ vsp.SYM_XZ, vsp.SYM_XY | vsp.SYM_YZ,
                                    vsp.SYM_XY | vsp.SYM_XZ | vsp.SYM_YZ ] )
def testAFlippedGeomIsNotInsideOut( kind, flag ):
    """A flipped Geom keeps the unflipped volume and area, for one, two and three planes."""
    fresh()
    plain = vsp.AddGeom( kind )
    no_symmetry( plain )
    flipped = vsp.AddGeom( kind )
    no_symmetry( flipped )
    vsp.SetGeomName( plain, "Plain" )
    vsp.SetGeomName( flipped, "Flipped" )
    at( flipped, y = 40.0 )
    flip( flipped, flag )

    areas, vols = comp_geom_of( [ plain, flipped ] )
    a = vsp.GetGeomName( plain )
    b = vsp.GetGeomName( flipped )
    assert a != b
    assert vols[ a ] > 0.0, "the shape enclosed no volume, so nothing is measured"
    assert vols[ b ] == pytest.approx( vols[ a ], rel = 1e-6 ), \
           "the flipped %s is inside out: %+f against %+f" % ( kind, vols[ b ], vols[ a ] )
    assert areas[ b ] == pytest.approx( areas[ a ], rel = 1e-6 )
    assert_no_errors()


def testALeftWingIsARightWingFlipped():
    """A wing without symmetry, flipped about XZ, is the half its symmetry would make."""
    fresh()
    whole = vsp.AddGeom( "WING" )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( whole, "Sym_Planar_Flag", "Sym" ) ) == vsp.SYM_XZ
    whole_box = box( whole )

    left = vsp.AddGeom( "WING" )
    no_symmetry( left )
    right_half = yspan( left )
    flip( left, vsp.SYM_XZ )

    assert yspan( left ) == pytest.approx( reflected( right_half ) )
    assert ( min( right_half[0], yspan( left )[0] ), max( right_half[1], yspan( left )[1] ) ) == \
           pytest.approx( ( whole_box[2], whole_box[2] + whole_box[3] ) )
    assert_no_errors()


def testSymmetryLaysItsCopiesOutFromTheFlippedShape():
    """The flip reflects about the wing's own root, symmetry then copies across the model's XZ
    plane, and neither half is inside out."""
    fresh()
    half = vsp.AddGeom( "WING" )
    no_symmetry( half )
    at( half, y = 5.0 )
    one = yspan( half )
    vsp.DeleteGeom( half )

    def both( s ):
        other = reflected( s )
        return ( min( s[0], other[0] ), max( s[1], other[1] ) )

    plain = vsp.AddGeom( "WING" )
    flipped = vsp.AddGeom( "WING" )
    vsp.SetGeomName( plain, "Plain" )
    vsp.SetGeomName( flipped, "Flipped" )
    at( plain, y = 5.0 )
    at( flipped, x = 30.0, y = 5.0 )
    assert yspan( plain ) == pytest.approx( both( one ) )
    flip( flipped, vsp.SYM_XZ )

    assert yspan( flipped ) == pytest.approx( both( reflected( one, about = 5.0 ) ) )
    assert yspan( flipped ) != pytest.approx( yspan( plain ) )

    areas, vols = comp_geom_of( [ plain, flipped ] )
    assert vols[ vsp.GetGeomName( plain ) ] > 0.0
    assert vols[ vsp.GetGeomName( flipped ) ] == pytest.approx( vols[ vsp.GetGeomName( plain ) ], rel = 1e-6 )
    assert_no_errors()


def testTheFlipIsAppliedBeforeTheRotation():
    """The flip applies before rotation: a wing turned 90 degrees about x has its span along z,
    and its XZ flip sends the span down while the dihedral, now along y, is unchanged."""
    fresh()
    wing = a_chiral_wing()
    vsp.SetParmVal( vsp.FindParm( wing, "X_Rel_Rotation", "XForm" ), 90.0 )
    vsp.Update()
    before_z = span( wing, "Z" )
    before_y = yspan( wing )

    flip( wing, vsp.SYM_XZ )
    assert span( wing, "Z" ) == pytest.approx( reflected( before_z ) )
    assert yspan( wing ) == pytest.approx( before_y )
    assert_no_errors()


def testTheFlipSurvivesARoundTrip():
    path = os.path.join( fresh(), "flip_roundtrip.vsp3" )
    wing = a_chiral_wing( ( 2.0, 3.0, 0.0 ) )
    flip( wing, vsp.SYM_XZ | vsp.SYM_YZ )
    before = box( wing )

    vsp.WriteVSPFile( path, vsp.SET_ALL )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()

    read = vsp.FindGeoms()[0]
    assert vsp.GetParmVal( vsp.FindParm( read, "Flip_Flag", "Sym" ) ) == vsp.SYM_XZ | vsp.SYM_YZ
    assert box( read ) == pytest.approx( before )
    assert_no_errors()


#==== Geoms that are not made of surfaces ====#
# These are flipped by a different code path from the surface Geoms, so each is measured.

SHAPES = [ a_mesh, lambda: vsp.AddGeom( "HUMAN" ), a_point_cloud, a_wireframe, a_polygon_mesh ]


@pytest.mark.parametrize( "make", SHAPES )
def testAFlipReflectsEveryOtherKindOfShape( make ):
    """Measured in x about YZ, since none of these is symmetric in x."""
    fresh()
    gid = make()
    vsp.Update()
    before = span( gid, "X" )
    assert before != pytest.approx( reflected( before ) ), \
           "the shape is symmetric in x, so a flip about YZ would measure nothing"

    flip( gid, vsp.SYM_YZ )
    assert span( gid, "X" ) == pytest.approx( reflected( before ) )
    assert_no_errors()


@pytest.mark.parametrize( "make", [ a_mesh, lambda: vsp.AddGeom( "HUMAN" ), a_wireframe, a_polygon_mesh ] )
@pytest.mark.parametrize( "flag", [ vsp.SYM_XZ, vsp.SYM_XY | vsp.SYM_YZ ] )
def testAFlippedShapeIsNotInsideOut( make, flag ):
    """Compared with an unflipped Clone placed elsewhere: same shape, same volume."""
    fresh()
    gid = make()
    plain = vsp.CloneGeomVec( [ gid ] )[0]
    at( plain, y = 30.0 )
    flip( gid, flag )

    areas, vols = comp_geom_of( [ gid, plain ] )
    a = vsp.GetGeomName( plain )
    b = vsp.GetGeomName( gid )
    assert vols[ a ] > 0.0, "the shape enclosed no volume, so nothing is measured"
    assert vols[ b ] == pytest.approx( vols[ a ], rel = 1e-6 ), \
           "the flipped shape is inside out: %+f against %+f" % ( vols[ b ], vols[ a ] )
    assert_no_errors()


@pytest.mark.parametrize( "make", SHAPES )
def testACloneOfAFlippedShapeShowsTheFlip( make ):
    """With CloneSym on, a Clone of a flipped shape is flipped too; with it off, it is not."""
    fresh()
    gid = make()
    vsp.Update()
    flip( gid, vsp.SYM_YZ )
    clone = vsp.CloneGeomVec( [ gid ] )[0]
    vsp.Update()
    assert box( clone ) == pytest.approx( box( gid ) )

    switch( clone, "CloneSym", False )
    assert span( clone, "X" ) == pytest.approx( reflected( span( gid, "X" ) ) )
    assert span( clone, "X" ) != pytest.approx( span( gid, "X" ) )
    assert_no_errors()


@pytest.mark.parametrize( "flag, planar, axial", [ ( vsp.SYM_XZ, vsp.SYM_XY, 3 ),
                                                   ( vsp.SYM_XY | vsp.SYM_YZ, vsp.SYM_XZ, 4 ) ] )
def testAFlippedHumanWithSymmetryEnclosesEveryCopy( flag, planar, axial ):
    """Planar and axial symmetry on a flipped Human: each copy encloses what one Human does."""
    fresh()
    one = vsp.AddGeom( "HUMAN" )
    many = vsp.AddGeom( "HUMAN" )
    vsp.SetGeomName( one, "One" )
    vsp.SetGeomName( many, "Many" )
    located( many, y = 30.0, z = 20.0 )
    vsp.SetParmVal( vsp.FindParm( many, "Sym_Planar_Flag", "Sym" ), planar )
    vsp.SetParmVal( vsp.FindParm( many, "Sym_Axial_Flag", "Sym" ), vsp.SYM_ROT_X )
    vsp.SetParmVal( vsp.FindParm( many, "Sym_Rot_N", "Sym" ), axial )
    vsp.Update()
    flip( many, flag )

    areas, vols = comp_geom_of( [ one, many ] )
    names = list( vsp.GetStringResults( vsp.FindLatestResultsID( "Comp_Geom" ), "Comp_Name" ) )
    copies = names.count( vsp.GetGeomName( many ) )
    assert copies == 2 * axial
    assert vols[ vsp.GetGeomName( one ) ] > 0.0
    assert vols[ vsp.GetGeomName( many ) ] == pytest.approx( copies * vols[ vsp.GetGeomName( one ) ], rel = 1e-6 )
    assert_no_errors()


def testAFlippedPolygonMeshIsWrittenRightWayRound():
    """A polygon mesh writes its own faces, so its writer reverses each one."""
    out = fresh()

    def written_volume( flag ):
        fresh()
        ngon = a_polygon_mesh()
        flip( ngon, flag )
        others = [ g for g in vsp.FindGeoms() if g != ngon ]
        only = _only_in_a_set( ngon, others )
        path = os.path.join( out, "flip_ngon_%d.vspgeom" % flag )
        vsp.ExportFile( path, only, vsp.EXPORT_VSPGEOM )
        vols = []
        for alternate in ( False, True ):
            nodes, tris = read_vspgeom( path, alternate )
            assert tris, "the export wrote no faces"
            vols.append( signed_volume( nodes, tris ) )
        return vols

    plain = written_volume( 0 )
    assert abs( plain[0] ) > 1e-6, "the mesh enclosed no volume, so nothing is measured"
    assert plain[1] == pytest.approx( plain[0], rel = 1e-6 ), "the two blocks disagree unflipped"
    for flag in ( vsp.SYM_XZ, vsp.SYM_XY | vsp.SYM_YZ ):
        assert written_volume( flag ) == pytest.approx( plain, rel = 1e-6 ), flag
    assert_no_errors()


#==== The files written ====#

@pytest.mark.parametrize( "kind, export, read", [
    ( "stl", vsp.EXPORT_STL, read_stl ),
    ( "tri", vsp.EXPORT_CART3D, read_cart3d ),
    ( "x3d", vsp.EXPORT_X3D, read_x3d ),
    ( "vspgeom", vsp.EXPORT_VSPGEOM, read_vspgeom ),
    ( "vspgeom", vsp.EXPORT_VSPGEOM, lambda path: read_vspgeom( path, alternate = True ) ) ] )
@pytest.mark.parametrize( "make", [ lambda: vsp.AddGeom( "POD" ), a_mesh ] )
def testAFlippedGeomIsWrittenOutRightWayRound( kind, export, read, make ):
    """An exported flipped mesh is wound outward.  A MeshGeom writes its triangles by its own
    path, so it is checked as well as a surface Geom."""
    out = fresh()

    def written_volume( flag ):
        fresh()
        gid = make()
        vsp.Update()
        flip( gid, flag )
        others = [ g for g in vsp.FindGeoms() if g != gid ]
        only = _only_in_a_set( gid, others )
        path = os.path.join( out, "flip_%s_%d.%s" % ( kind, flag, kind ) )
        vsp.ExportFile( path, only, export )
        nodes, tris = read( path )
        assert tris, "the export wrote no triangles"
        return signed_volume( nodes, tris )

    plain = written_volume( 0 )
    flipped = written_volume( vsp.SYM_XZ )

    assert abs( plain ) > 1e-6, "the mesh enclosed no volume, so nothing is measured"
    assert flipped == pytest.approx( plain, rel = 1e-6 ), \
           "the flipped Geom was written inside out: %+f against %+f" % ( flipped, plain )
    assert_no_errors()


#==== What hangs off a flipped Geom ====#

def testAConformalFollowsItsParentsFlip():
    """A conformal is an inset of its parent's shape, so it is flipped with it."""
    fresh()
    pod = vsp.AddGeom( "POD" )
    at( pod, x = 5.0 )
    conformal = vsp.AddGeom( "CONFORMAL", pod )
    vsp.Update()
    before = span( conformal, "X" )
    assert before[1] > before[0], "the conformal has no size, so nothing is measured"

    flip( pod, vsp.SYM_YZ )
    assert span( conformal, "X" ) == pytest.approx( reflected( before, about = 5.0 ) )

    areas, vols = comp_geom_of( [ pod, conformal ] )
    assert vols[ vsp.GetGeomName( conformal ) ] > 0.0
    assert_no_errors()


def testAChildIsPlacedNotFlipped():
    """A pod attached to a flipped pod's origin stays put and is not itself flipped."""
    fresh()
    parent = vsp.AddGeom( "POD" )
    at( parent, x = 5.0 )
    child = vsp.AddGeom( "POD", parent )
    vsp.SetParmVal( vsp.FindParm( child, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_COMP )
    at( child, x = 2.0, y = 3.0 )
    before = box( child )

    flip( parent, vsp.SYM_YZ | vsp.SYM_XZ )
    assert box( child ) == pytest.approx( before )
    assert_no_errors()


# Each mode on the upper skin, where the child's own z is the outward normal.
SURFACE_ATTACH = {
    "UV": ( vsp.ATTACH_TRANS_UV, vsp.ATTACH_ROT_UV,
            { "U_Attach_Location": 0.4, "V_Attach_Location": 0.7 } ),
    "RST": ( vsp.ATTACH_TRANS_RST, vsp.ATTACH_ROT_RST,
             { "R_Attach_Location": 0.4, "S_Attach_Location": 0.3, "T_Attach_Location": 1.0 } ),
    "LMN": ( vsp.ATTACH_TRANS_LMN, vsp.ATTACH_ROT_LMN,
             { "L_Attach_Location": 0.4, "M_Attach_Location": 0.3, "N_Attach_Location": 1.0 } ),
    "EtaMN": ( vsp.ATTACH_TRANS_EtaMN, vsp.ATTACH_ROT_EtaMN,
               { "Eta_Attach_Location": 0.4, "M_Attach_Location": 0.3, "N_Attach_Location": 1.0 } ) }


def _offsets_from_the_skin( flag ):
    """Per mode: where a child attached to the wing sits, which way an offset along its own z
    moves it, and how much of that is along the wing's outward normal."""
    fresh()
    wing = a_chiral_wing()
    flip( wing, flag )
    found = {}
    for name, ( trans, rots, locs ) in SURFACE_ATTACH.items():
        kid = vsp.AddGeom( "POD", wing )
        vsp.SetParmVal( vsp.FindParm( kid, "Trans_Attach_Flag", "Attach" ), trans )
        vsp.SetParmVal( vsp.FindParm( kid, "Rots_Attach_Flag", "Attach" ), rots )
        for parm, val in locs.items():
            vsp.SetParmVal( vsp.FindParm( kid, parm, "Attach" ), val )
        vsp.Update()
        p0 = placed( kid )
        at( kid, z = 1.0 )
        d = tuple( b - a for a, b in zip( p0, placed( kid ) ) )

        dist, u, w = vsp.ProjPnt01( wing, 0, vsp.vec3d( *p0 ) )
        assert dist < 1e-6, "the %s child is not on the skin" % name
        n = xyz( vsp.CompNorm01( wing, 0, u, w ) )
        found[ name ] = ( p0, d, sum( a * b for a, b in zip( d, n ) ) )
    return found


@pytest.mark.parametrize( "flag", [ vsp.SYM_XZ, vsp.SYM_XY, vsp.SYM_YZ ] )
def testAChildOnAFlippedSkinStaysOutside( flag ):
    """In every attach mode, a child on a flipped wing sits at the reflected position, and an
    offset along its own z still points out of the skin."""
    plain = _offsets_from_the_skin( 0 )
    got = _offsets_from_the_skin( flag )
    for name in SURFACE_ATTACH:
        assert plain[ name ][2] == pytest.approx( 1.0 ), "%s does not offset along the normal" % name
        assert got[ name ][0] == pytest.approx( reflect( plain[ name ][0], flag ) ), name
        assert got[ name ][1] == pytest.approx( reflect( plain[ name ][1], flag ) ), name
        assert got[ name ][2] == pytest.approx( plain[ name ][2] ), name
    assert_no_errors()


def testAFlippedGeomsNormalIsReflectedAndPointsOut():
    """CompNorm01 on a flipped pod is the reflection of the unflipped normal, and points away
    from the pod's axis."""
    fresh()
    pod = vsp.AddGeom( "POD" )
    no_symmetry( pod )
    at( pod, x = 2.0, y = 3.0, z = 1.0 )
    u, w = 0.3, 0.15
    p = xyz( vsp.CompPnt01( pod, 0, u, w ) )
    n = xyz( vsp.CompNorm01( pod, 0, u, w ) )
    assert min( abs( c ) for c in n ) > 0.01, "the normal lies in a plane, so this measures less"

    def outward( p, n ):
        return ( p[1] - 3.0 ) * n[1] + ( p[2] - 1.0 ) * n[2]

    assert outward( p, n ) > 0.0
    about = ( 2.0, 3.0, 1.0 )
    for flag in ( vsp.SYM_YZ, vsp.SYM_XZ, vsp.SYM_XY | vsp.SYM_YZ ):
        flip( pod, flag )
        pm = xyz( vsp.CompPnt01( pod, 0, u, w ) )
        nm = xyz( vsp.CompNorm01( pod, 0, u, w ) )
        assert pm == pytest.approx( reflect( p, flag, about ) )
        assert nm == pytest.approx( reflect( n, flag ) )
        assert outward( pm, nm ) > 0.0
    assert_no_errors()


#==== Analyses of a flipped Geom ====#

def _fea_centroids( flag, out ):
    """Area-weighted centroid of each part of a coarse wing structure: skin, rib, spar and a
    slice on the body's XZ plane."""
    fresh()
    wing = a_chiral_wing()
    flip( wing, flag )
    struct = vsp.AddFeaStruct( wing )
    rib = vsp.AddFeaPart( wing, struct, vsp.FEA_RIB )
    vsp.SetParmVal( vsp.FindParm( rib, "RelCenterLocation", "FeaPart" ), 0.7 )
    vsp.AddFeaPart( wing, struct, vsp.FEA_SPAR )
    cut = vsp.AddFeaPart( wing, struct, vsp.FEA_SLICE )
    vsp.SetParmVal( vsp.FindParm( cut, "OrientationPlane", "FeaSlice" ), vsp.XZ_BODY )
    vsp.SetParmVal( vsp.FindParm( cut, "RelCenterLocation", "FeaPart" ), 0.3 )
    vsp.Update()

    path = os.path.join( out, "flip_fea_%d.stl" % flag )
    vsp.SetFeaMeshFileName( wing, struct, vsp.FEA_STL_FILE_NAME, path )
    vsp.SetFeaMeshVal( wing, struct, vsp.CFD_MAX_EDGE_LEN, 1.0 )
    vsp.SetFeaMeshVal( wing, struct, vsp.CFD_MIN_EDGE_LEN, 0.5 )
    vsp.ComputeFeaMesh( wing, struct, vsp.FEA_STL_FILE_NAME )

    sums = {}
    part = None
    corners = []
    for line in open( path ):
        toks = line.split()
        if not toks:
            continue
        if toks[0] == "solid":
            part = toks[1]
            sums[ part ] = [ 0.0, 0.0, 0.0, 0.0 ]
        elif toks[0] == "vertex":
            corners.append( [ float( t ) for t in toks[1:] ] )
        elif toks[0] == "endloop":
            a, b, c = corners
            corners = []
            e = [ b[i] - a[i] for i in range( 3 ) ]
            f = [ c[i] - a[i] for i in range( 3 ) ]
            cross = ( e[1] * f[2] - e[2] * f[1], e[2] * f[0] - e[0] * f[2], e[0] * f[1] - e[1] * f[0] )
            area = 0.5 * sum( x * x for x in cross ) ** 0.5
            for i in range( 3 ):
                sums[ part ][i] += area * ( a[i] + b[i] + c[i] ) / 3.0
            sums[ part ][3] += area
    return { k: tuple( v[i] / v[3] for i in range( 3 ) ) for k, v in sums.items() }


@pytest.mark.parametrize( "flag", [ vsp.SYM_XZ, vsp.SYM_XY, vsp.SYM_YZ ] )
def testAStructureOnAFlippedWingIsFlippedWithIt( flag ):
    """Each part of a structure on a flipped wing sits at its reflected position, to a mesh
    tolerance."""
    out = fresh()
    plain = _fea_centroids( 0, out )
    got = _fea_centroids( flag, out )
    assert len( plain ) == 4, "expected a skin and three parts: %s" % list( plain )
    assert sorted( got ) == sorted( plain )
    axis = PLANE_AXIS[ flag ]
    for name, c in plain.items():
        assert abs( c[ axis ] ) > 0.5, "%s is on the plane, so this would measure nothing" % name
        assert got[ name ] == pytest.approx( reflect( c, flag ), abs = 1e-2 ), name
    assert_no_errors()


@pytest.mark.parametrize( "flag", [ vsp.SYM_XZ, vsp.SYM_XY | vsp.SYM_YZ ] )
def testAControlSurfaceOnAFlippedWingCoversTheSameArea( flag ):
    """CompGeom's area per tag of a flipped wing with a control surface matches the unflipped one."""
    out = fresh()

    def tag_areas( flag ):
        fresh()
        wing = a_chiral_wing()
        vsp.AddSubSurf( wing, vsp.SS_CONTROL )
        flip( wing, flag )
        vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
        res = vsp.FindLatestResultsID( "Comp_Geom" )
        return dict( zip( vsp.GetStringResults( res, "Tag_Name" ),
                          vsp.GetDoubleResults( res, "Tag_Theo_Area" ) ) )

    plain = tag_areas( 0 )
    assert len( plain ) == 2 and min( plain.values() ) > 0.0, "the control surface was not tagged: %s" % plain
    got = tag_areas( flag )
    assert sorted( got ) == sorted( plain )
    for name, area in plain.items():
        assert got[ name ] == pytest.approx( area, rel = 1e-9 ), name
    assert_no_errors()


def testAScaledGeomIsFlippedAboutWhereItStands():
    """A scaled, flipped shape is reflected about the Geom's origin and keeps the scaled volume."""
    fresh()
    flag = vsp.SYM_XZ | vsp.SYM_XY
    about = ( 1.0, 2.0, 3.0 )

    def measured( flag ):
        fresh()
        wing = a_chiral_wing( about )
        vsp.SetParmVal( vsp.FindParm( wing, "Scale", "XForm" ), 1.7 )
        vsp.Update()
        flip( wing, flag )
        spans = [ span( wing, axis ) for axis in "XYZ" ]
        areas, vols = comp_geom_of( [ wing ] )
        return spans, vols[ vsp.GetGeomName( wing ) ]

    plain, plain_vol = measured( 0 )
    got, got_vol = measured( flag )
    assert plain[1][1] - plain[1][0] > 9.0 * 1.5, "the scale did not reach the shape"
    assert got[0] == pytest.approx( plain[0] )
    assert got[1] == pytest.approx( reflected( plain[1], about = about[1] ) )
    assert got[2] == pytest.approx( reflected( plain[2], about = about[2] ) )
    assert plain_vol > 0.0
    assert got_vol == pytest.approx( plain_vol, rel = 1e-6 )
    assert_no_errors()


PRODUCTS = { "Total_Ixy": ( 0, 1 ), "Total_Ixz": ( 0, 2 ), "Total_Iyz": ( 1, 2 ) }


@pytest.mark.parametrize( "flag", [ vsp.SYM_XZ, vsp.SYM_XY, vsp.SYM_YZ ] )
def testAFlippedGeomWeighsAsItsReflection( flag ):
    """Mass properties of a flipped wing: the CG is reflected about the wing's location, the
    moments are kept, and a product of inertia changes sign when one of its axes is reflected."""
    fresh()
    about = ( 1.0, 2.0, 3.0 )

    def props( flag ):
        fresh()
        wing = a_chiral_wing( about )
        flip( wing, flag )
        res = mass_props()
        vals = { n: list( vsp.GetDoubleResults( res, n ) )[0]
                 for n in ( "Total_Mass", "Total_Ixx", "Total_Iyy", "Total_Izz" ) + tuple( PRODUCTS ) }
        return xyz( list( vsp.GetVec3dResults( res, "Total_CG" ) )[0] ), vals

    plain_cg, plain = props( 0 )
    got_cg, got = props( flag )
    assert got_cg == pytest.approx( reflect( plain_cg, flag, about ) )
    for name in ( "Total_Mass", "Total_Ixx", "Total_Iyy", "Total_Izz" ):
        assert got[ name ] == pytest.approx( plain[ name ], rel = 1e-6 ), name
    axis = PLANE_AXIS[ flag ]
    for name, axes in PRODUCTS.items():
        assert abs( plain[ name ] ) > 0.1, "%s is zero, so its sign measures nothing" % name
        sign = -1.0 if axis in axes else 1.0
        assert got[ name ] == pytest.approx( sign * plain[ name ], rel = 1e-6 ), name
    assert_no_errors()


@pytest.mark.parametrize( "flag", [ vsp.SYM_XZ, vsp.SYM_XY | vsp.SYM_YZ ] )
def testAPointMassIsReflectedWithTheShape( flag ):
    """A point mass is placed in the shape's frame, so its CG is reflected about the Geom's
    location."""
    fresh()
    about = ( 1.0, 2.0, 3.0 )

    def cg( flag ):
        fresh()
        wing = a_chiral_wing( about )
        vsp.SetParmVal( vsp.FindParm( wing, "Density", "Mass_Props" ), 0.0 )
        vsp.SetParmVal( vsp.FindParm( wing, "PointMass", "Mass_Props" ), 2.0 )
        for name, val in zip( ( "CGx", "CGy", "CGz" ), ( 0.5, 1.5, 0.7 ) ):
            vsp.SetParmVal( vsp.FindParm( wing, name, "Mass_Props" ), val )
        vsp.Update()
        flip( wing, flag )
        return total_cg()

    plain = cg( 0 )
    assert plain == pytest.approx( ( 1.5, 3.5, 3.7 ) ), "the CG is not the point mass alone"
    assert cg( flag ) == pytest.approx( reflect( plain, flag, about ) )
    assert_no_errors()


def testCopyAndPasteKeepsTheFlip():
    """A pasted copy of a flipped Geom is flipped the same way, at the same place."""
    fresh()
    wing = a_chiral_wing( ( 1.0, 2.0, 3.0 ) )
    before = box( wing )
    flip( wing, vsp.SYM_XZ | vsp.SYM_XY )
    flipped = box( wing )
    assert flipped != pytest.approx( before )

    vsp.CopyGeomToClipboard( wing )
    pasted = vsp.PasteGeomClipboard()
    vsp.Update()
    assert len( pasted ) == 1
    assert vsp.GetParmVal( vsp.FindParm( pasted[0], "Flip_Flag", "Sym" ) ) == vsp.SYM_XZ | vsp.SYM_XY
    assert box( pasted[0] ) == pytest.approx( flipped )
    assert_no_errors()


def testAFlippedMeshHandsOutItsTrianglesRightWayRound():
    """The triangles a mesh returns as results enclose the same volume, flipped or not."""
    fresh()

    def result_volume( flag ):
        fresh()
        mesh = a_mesh()
        flip( mesh, flag )
        res = vsp.CreateGeomResults( mesh, "Comp_Mesh" )
        nodes = [ xyz( p ) for p in vsp.GetVec3dResults( res, "Tri_Pnts" ) ]
        tris = list( zip( *( vsp.GetIntResults( res, "Tri_Index%d" % i ) for i in range( 3 ) ) ) )
        assert tris, "the results held no triangles"
        return signed_volume( nodes, tris )

    plain = result_volume( 0 )
    assert plain > 1e-6, "the mesh enclosed no volume, so nothing is measured"
    for flag in ( vsp.SYM_XZ, vsp.SYM_XY | vsp.SYM_YZ ):
        assert result_volume( flag ) == pytest.approx( plain, rel = 1e-6 ), flag
    assert_no_errors()


@pytest.mark.parametrize( "flag, flips", [ ( vsp.SYM_YZ, True ), ( vsp.SYM_XZ, False ),
                                           ( vsp.SYM_XY, False ) ] )
def testAFlippedPropsAxisTurnsWithItsFace( flag, flips ):
    """A YZ flip reverses a prop's BEM axis; a flip about a plane containing the axis keeps
    it.  The centre stays at the prop's location."""
    out = fresh()

    def axis( flag ):
        fresh()
        prop = vsp.AddGeom( "PROP" )
        located( prop, x = 3.0, y = 5.0, z = 2.0 )
        vsp.SetParmVal( vsp.FindParm( prop, "Y_Rel_Rotation", "XForm" ), 10.0 )
        vsp.SetParmVal( vsp.FindParm( prop, "Z_Rel_Rotation", "XForm" ), 20.0 )
        vsp.Update()
        flip( prop, flag )
        vsp.SetBEMPropID( prop )
        vsp.ExportFile( os.path.join( out, "flip_%d.bem" % flag ), vsp.SET_ALL, vsp.EXPORT_BEM )
        res = vsp.FindLatestResultsID( "PropBEM" )
        return ( xyz( list( vsp.GetVec3dResults( res, "Center" ) )[0] ),
                 xyz( list( vsp.GetVec3dResults( res, "Normal" ) )[0] ) )

    plain_center, plain_normal = axis( 0 )
    assert min( abs( c ) for c in plain_normal ) > 0.1, "the axis lies in a plane, so this measures less"
    center, normal = axis( flag )
    assert center == pytest.approx( plain_center )
    if flips:
        assert normal == pytest.approx( tuple( -c for c in plain_normal ) )
    else:
        assert normal == pytest.approx( plain_normal )
    assert_no_errors()


#==== A flipped gear ====#

def _gear_cg( flag, local, cg ):
    """A gear's nominal CG set in one frame and read back in both."""
    fresh()
    gear = vsp.AddGeom( "GEAR" )
    located( gear, x = 2.0, y = 3.0, z = 1.0 )
    vsp.SetParmVal( vsp.FindParm( gear, "Z_Rel_Rotation", "XForm" ), 30.0 )
    vsp.SetParmVal( vsp.FindParm( gear, "CGLocalFlag", "GroundPlane" ), local )
    suffix = "Local" if local else "Global"
    for axis, val in zip( "XYZ", cg ):
        vsp.SetParmVal( vsp.FindParm( gear, axis + "CGNominal" + suffix, "GroundPlane" ), val )
    vsp.Update()
    flip( gear, flag )
    return tuple( tuple( vsp.GetParmVal( vsp.FindParm( gear, axis + "CGNominal" + frame, "GroundPlane" ) )
                         for axis in "XYZ" ) for frame in ( "Local", "Global" ) )


@pytest.mark.parametrize( "flag", [ vsp.SYM_XZ, vsp.SYM_XY, vsp.SYM_YZ ] )
@pytest.mark.parametrize( "local", [ True, False ] )
def testAFlippedGearsCGIsInTheFlippedFrame( flag, local ):
    """A gear's local CG is in the shape's frame: on a flipped gear it matches the reflected
    local CG of the unflipped gear, and a global CG reads back reflected in local terms."""
    cg = ( 1.0, 0.5, -2.0 )
    got_local, got_global = _gear_cg( flag, local, cg )
    if local:
        assert got_local == pytest.approx( cg )
        assert got_global == pytest.approx( _gear_cg( 0, local, reflect( cg, flag ) )[1] )
    else:
        assert got_global == pytest.approx( cg )
        assert got_local == pytest.approx( reflect( _gear_cg( 0, local, cg )[0], flag ) )
    assert_no_errors()


def _stowed_reach( flag, parent_at, out ):
    """The extent of a bogie stowed on a pod, with the gear placed elsewhere and flipped.  The
    stow parent can only be set through the file."""
    fresh()
    pod = vsp.AddGeom( "POD" )
    located( pod, *parent_at )
    gear = vsp.AddGeom( "GEAR" )
    vsp.SetParmVal( vsp.FindParm( gear, "ShowNominalGroundPlane", "GroundPlane" ), 0 )
    located( gear, x = 1.0 )
    bogie = vsp.CreateAndAddBogie( gear )
    vsp.SetParmVal( vsp.FindParm( bogie, "Symmetrical", "Bogie" ), 0 )
    vsp.SetParmVal( vsp.FindParm( bogie, "StowTransAttachFlag", "StowAttach" ), vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( vsp.FindParm( bogie, "StowRotAttachFlag", "StowAttach" ), vsp.ATTACH_ROT_COMP )
    for name, val in zip( ( "StowXRelLoc", "StowYRelLoc", "StowZRelLoc", "StowXRelRot" ),
                          ( 0.5, 1.5, -0.7, 20.0 ) ):
        vsp.SetParmVal( vsp.FindParm( bogie, name, "Retract" ), val )
    vsp.SetParmVal( vsp.FindParm( gear, "GearConfigMode", "Gear" ), vsp.GEAR_CONFIGURATION_UP )
    vsp.Update()

    path = os.path.join( out, "flip_stow_%d.vsp3" % flag )
    vsp.WriteVSPFile( path )
    text = open( path ).read()
    text, n = re.subn( r'<StowParentID>[^<]*</StowParentID>|<StowParentID/>',
                       '<StowParentID>%s</StowParentID>' % pod, text )
    assert n == 1
    open( path, 'w' ).write( text )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()
    flip( gear, flag )

    steps = [ i / 10.0 for i in range( 11 ) ]
    pts = [ xyz( vsp.CompPnt01( gear, s, u, w ) ) for s in range( vsp.GetTotalNumSurfs( gear ) )
            for u in steps for w in steps ]
    assert pts, "the gear drew no tire"
    return [ ( min( p[i] for p in pts ), max( p[i] for p in pts ) ) for i in range( 3 ) ]


@pytest.mark.parametrize( "flag", [ vsp.SYM_XZ, vsp.SYM_XY, vsp.SYM_YZ ] )
def testABogieStowedOnAnotherGeomStaysThere( flag ):
    """A bogie stowed on a pod stays at the pod when the gear is flipped: the tire and stow
    offsets are reflected about the pod's planes, not the gear's."""
    out = fresh()
    parent_at = ( 10.0, 4.0, 2.0 )
    plain = _stowed_reach( 0, parent_at, out )
    got = _stowed_reach( flag, parent_at, out )

    # Stowed elsewhere, the tire moves with it, so the stow parent places it.
    elsewhere = _stowed_reach( 0, ( 20.0, 8.0, 4.0 ), out )
    assert elsewhere[0][0] > plain[0][1]

    axis = PLANE_AXIS[ flag ]
    for i in range( 3 ):
        if i == axis:
            assert got[i] == pytest.approx( reflected( plain[i], about = parent_at[i] ) )
            assert got[i] != pytest.approx( plain[i] )
        else:
            assert got[i] == pytest.approx( plain[i] )
    assert_no_errors()


def _thrown_blade( flag, after, reverse, on_clone ):
    """The box and direction of a blade thrown off a prop at y = 5, z = 2."""
    fresh()
    prop = vsp.AddGeom( "PROP" )
    located( prop, y = 5.0, z = 2.0 )
    vsp.SetParmVal( vsp.FindParm( prop, "ReverseFlag", "Design" ), reverse )
    vsp.Update()
    if not after:
        flip( prop, flag )
    parent = prop
    if on_clone:
        parent = vsp.CloneGeomVec( [ prop ] )[0]
        vsp.Update()
    aux = vsp.AddGeom( "AUXILIARY", parent )
    vsp.SetParmVal( vsp.FindParm( aux, "AuxiliaryGeomType", "Design" ), vsp.AUX_GEOM_THROWN_BLADE )
    vsp.Update()
    if after:
        flip( prop, flag )
    return box( aux ), vsp.GetParmVal( vsp.FindParm( aux, "RotDir", "Design" ) )


@pytest.mark.parametrize( "flag, axis, about", [ ( vsp.SYM_XZ, 2, 5.0 ), ( vsp.SYM_XY, 4, 2.0 ),
                                                 ( vsp.SYM_YZ, 0, 0.0 ) ] )
@pytest.mark.parametrize( "after", [ False, True ] )
@pytest.mark.parametrize( "reverse", [ 0, 1 ] )
@pytest.mark.parametrize( "on_clone", [ False, True ] )
def testAThrownBladeIsTheReflection( flag, axis, about, after, reverse, on_clone ):
    """A blade thrown off a flipped prop is the reflection of the unflipped one, whether
    added before or after the flip, or on a Clone of the prop."""
    plain, plain_dir = _thrown_blade( 0, after, reverse, on_clone )
    got, got_dir = _thrown_blade( flag, after, reverse, on_clone )

    def reach( b, i ):
        return ( b[i], b[i] + b[i + 1] )

    assert reach( plain, 2 ) != pytest.approx( reflected( reach( plain, 2 ), about = 5.0 ) ), \
           "the blade is thrown both ways, so this would measure nothing"
    assert reach( got, axis ) == pytest.approx( reflected( reach( plain, axis ), about = about ) )
    for other in ( 0, 2, 4 ):
        if other != axis:
            assert reach( got, other ) == pytest.approx( reach( plain, other ) )
    assert got_dir == plain_dir
    assert_no_errors()


#==== A flipped joint ====#

def placed( gid ):
    return tuple( vsp.GetParmVal( vsp.FindParm( gid, axis + "_Location", "XForm" ) )
                  for axis in "XYZ" )


def testAFlippedHingeTurnsItsChildTheOtherWay():
    """Flipped about a plane containing its axis, a hinge rotates the other way.  The child is
    not flipped: it stays on the same side of the axis and deflects the other way."""
    fresh()
    hinge = vsp.AddGeom( "HINGE" )
    vsp.SetParmVal( vsp.FindParm( hinge, "PrimaryDir", "Hinge" ), vsp.X_DIR )
    vsp.Update()
    pod = vsp.AddGeom( "POD", hinge )
    no_symmetry( pod )
    vsp.SetParmVal( vsp.FindParm( pod, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( vsp.FindParm( pod, "Rots_Attach_Flag", "Attach" ), vsp.ATTACH_ROT_COMP )
    at( pod, y = 2.0 )

    vsp.SetParmVal( vsp.FindParm( hinge, "JointRotate", "Hinge" ), 25.0 )
    vsp.Update()
    turned = placed( pod )
    assert turned[2] > 0.1, "the deflection did not move the child, so this measures nothing"

    flip( hinge, vsp.SYM_XZ )
    flipped = placed( pod )
    assert flipped[0] == pytest.approx( turned[0] )
    assert flipped[1] == pytest.approx( turned[1] )
    assert flipped[2] == pytest.approx( -turned[2] )
    assert_no_errors()


#==== Clones and the flip ====#

def testACloneOfAFlippedGeomIsFlippedToo():
    """With CloneSym on, a Clone takes its original's flip planes."""
    fresh()
    wing = a_chiral_wing()
    flip( wing, vsp.SYM_XZ )
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( clone, "Flip_Flag", "Sym" ) ) == 0
    assert yspan( clone ) == pytest.approx( yspan( wing ) )

    switch( clone, "CloneSym", False )
    assert yspan( clone ) == pytest.approx( reflected( yspan( wing ) ) )
    assert_no_errors()


def testAClonesOwnFlipReflectsWhatItShows():
    """A Clone's own planes combine with the original's: flipping a Clone of a flipped wing
    about the same plane undoes it."""
    fresh()
    wing = a_chiral_wing()
    unflipped = yspan( wing )
    flip( wing, vsp.SYM_XZ )
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    vsp.Update()

    flip( clone, vsp.SYM_XZ )
    assert yspan( clone ) == pytest.approx( unflipped )

    areas, vols = comp_geom_of( [ wing, clone ] )
    assert vols[ vsp.GetGeomName( wing ) ] > 0.0
    assert vols[ vsp.GetGeomName( clone ) ] == pytest.approx( vols[ vsp.GetGeomName( wing ) ], rel = 1e-6 )
    assert_no_errors()


@pytest.mark.parametrize( "own, expected", [ ( 0, vsp.SYM_XZ ), ( vsp.SYM_XZ, 0 ),
                                             ( vsp.SYM_YZ, vsp.SYM_XZ | vsp.SYM_YZ ) ] )
def testReplacingACloneOfAFlippedGeomKeepsTheFlip( own, expected ):
    """The Geom replacing a Clone carries the Clone's combined flip planes, so it matches the
    Clone exactly."""
    fresh()
    wing = a_chiral_wing( ( 0.0, 4.0, 0.0 ) )
    flip( wing, vsp.SYM_XZ )
    clone = vsp.CloneGeomVec( [ wing ] )[0]
    at( clone, x = 20.0, y = 4.0 )
    switch( clone, "CloneXForm", False )
    flip( clone, own )
    shown = box( clone )

    real = vsp.ReplaceCloneGeom( clone )
    vsp.Update()
    assert_no_errors()
    assert vsp.GetGeomTypeName( real ) == "Wing"
    assert vsp.GetParmVal( vsp.FindParm( real, "Flip_Flag", "Sym" ) ) == expected
    assert box( real ) == pytest.approx( shown )


#==== Where the flip does not apply ====#

@pytest.mark.parametrize( "kind, moves", [ ( "BLANK", False ), ( "POD", True ) ] )
def testABlankIsNotFlipped( kind, moves ):
    """A flip flag on a Blank does not move its point mass; on a pod the same point mass is
    reflected, so the check can see a flip."""
    fresh()

    def cg( flag ):
        fresh()
        gid = vsp.AddGeom( kind )
        located( gid, x = 3.0 )
        vsp.SetParmVal( vsp.FindParm( gid, "Density", "Mass_Props" ), 0.0 )
        vsp.SetParmVal( vsp.FindParm( gid, "PointMass", "Mass_Props" ), 2.0 )
        for name, val in zip( ( "CGx", "CGy", "CGz" ), ( 0.5, 1.5, 0.7 ) ):
            vsp.SetParmVal( vsp.FindParm( gid, name, "Mass_Props" ), val )
        vsp.Update()
        flip( gid, flag )
        return total_cg()

    flag = vsp.SYM_YZ | vsp.SYM_XZ
    plain = cg( 0 )
    if moves:
        assert cg( flag ) == pytest.approx( reflect( plain, flag, about = ( 3.0, 0.0, 0.0 ) ) )
    else:
        assert cg( flag ) == pytest.approx( plain )
    assert_no_errors()


def _a_route_off_the_plane():
    """A route through a pod at y = 5; an XZ flip would move it to y = -5."""
    route = a_route()
    pod = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Pod" ][0]
    at( pod, y = 5.0 )
    assert box( route )[2] > 1.0, "the route is on the XZ plane, so this would measure nothing"
    return route


def testAFlippedRouteIsNotFlipped():
    """A flip flag on a route does nothing: its points follow the Geoms they sit on."""
    fresh()
    route = _a_route_off_the_plane()
    before = box( route )

    flip( route, vsp.SYM_XZ )
    assert box( route ) == pytest.approx( before )
    assert_no_errors()


@pytest.mark.parametrize( "on_clone", [ False, True ] )
def testACloneOfARouteIsNotFlipped( on_clone ):
    """A Clone of a route is not flipped, whether the flag is on the route or on the Clone."""
    fresh()
    route = _a_route_off_the_plane()
    clone = vsp.CloneGeomVec( [ route ] )[0]
    vsp.Update()
    before = box( clone )

    if on_clone:
        flip( clone, vsp.SYM_XZ )
    else:
        flip( route, vsp.SYM_XZ )
    assert box( clone ) == pytest.approx( before )
    assert_no_errors()


def testACloneOfAShapeThatFollowsItsParentFollowsItToo():
    """A conformal and its Clone follow the parent's flip; a flag on the conformal itself is
    ignored by both."""
    fresh()
    wing = a_chiral_wing()
    conformal = vsp.AddGeom( "CONFORMAL", wing )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ conformal ] )[0]
    vsp.Update()

    flip( wing, vsp.SYM_XZ )
    assert yspan( clone ) == pytest.approx( yspan( conformal ) )
    assert yspan( conformal )[1] < 0.0, "the conformal did not follow the wing, so this measures nothing"

    flip( wing, 0 )
    before = yspan( conformal )
    flip( conformal, vsp.SYM_XZ )
    assert yspan( conformal ) == pytest.approx( before )
    assert yspan( clone ) == pytest.approx( before )
    assert_no_errors()


def _gear_angle( flag, kind, mode, contacts, out ):
    """A gear angle analysis of a wing and fuselage over a nose and main bogie, the gear flipped
    about flag.  The gear arrangement can only be set through the file."""
    fresh()
    wing = vsp.AddGeom( "WING" )
    at( wing, x = 5.0, z = 2.5 )
    fuse = vsp.AddGeom( "FUSELAGE" )
    at( fuse, z = 2.5 )
    gear = vsp.AddGeom( "GEAR" )
    vsp.SetParmVal( vsp.FindParm( gear, "ShowNominalGroundPlane", "GroundPlane" ), 0 )
    bogies = {}
    for name, x, y, symm in ( ( "nose", 2.0, 0.0, 0 ), ( "main", 12.0, 4.0, 1 ) ):
        b = vsp.CreateAndAddBogie( gear )
        vsp.SetParmVal( vsp.FindParm( b, "XContactPt", "Bogie" ), x )
        vsp.SetParmVal( vsp.FindParm( b, "YContactPt", "Bogie" ), y )
        vsp.SetParmVal( vsp.FindParm( b, "Symmetrical", "Bogie" ), symm )
        bogies[ name ] = b
    vsp.Update()
    flip( gear, flag )

    aux = vsp.AddGeom( "AUXILIARY", gear )
    vsp.SetParmVal( vsp.FindParm( aux, "AuxiliaryGeomType", "Design" ), mode )
    vsp.Update()
    for i, ( name, isymm ) in enumerate( contacts ):
        vsp.SetAuxiliaryGeomContactPtID( aux, i, bogies[ name ] )
        vsp.SetParmVal( vsp.FindParm( aux, "ContactPt%d_Isymm" % ( i + 1 ), "Design" ), isymm )
    vsp.Update()

    for g in ( wing, fuse ):
        vsp.SetSetFlag( g, 5, True )
    case = vsp.AddGeometryAnalysis()
    vsp.SetParmVal( vsp.FindParm( case, "IntererenceCheckType", "InterferenceCase" ), kind )
    vsp.SetParmVal( vsp.FindParm( case, "PrimaryType", "InterferenceCase" ), vsp.SET_TARGET )
    vsp.SetParmVal( vsp.FindParm( case, "PrimarySet", "InterferenceCase" ), 5 )
    vsp.SetParmVal( vsp.FindParm( case, "SecondaryType", "InterferenceCase" ), vsp.GEOM_TARGET )

    path = os.path.join( out, "gear_angle_%d_%d.vsp3" % ( flag, kind ) )
    vsp.WriteVSPFile( path )
    text = open( path ).read()
    text = re.sub( r'<SecondaryGeomID>[^<]*</SecondaryGeomID>|<SecondaryGeomID/>',
                   '<SecondaryGeomID>%s</SecondaryGeomID>' % aux, text )
    open( path, 'w' ).write( text )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()

    vsp.SetAnalysisInputDefaults( "GeometryAnalysis" )
    vsp.ExecAnalysis( "GeometryAnalysis" )
    for name in vsp.GetAllResultsNames():
        if "Angle_Interference" in name:
            res = vsp.FindLatestResultsID( name )
            return list( vsp.GetDoubleResults( res, "Con_Val" ) )[0]
    assert False, "the analysis gave no angle"


def testAFlippedGearTipsAndRollsAsItsReflection():
    """Flipped about XZ, a gear tips back by the same angle, and its contact on one side rolls
    like the unflipped gear's contact on the other side."""
    out = fresh()
    pitch = ( vsp.PLANE_2PT_ANGLE_INTERFERENCE, vsp.AUX_GEOM_TWO_PT_GROUND, [ ( "main", 0 ), ( "main", 1 ) ] )
    assert _gear_angle( vsp.SYM_XZ, *pitch, out ) == pytest.approx( _gear_angle( 0, *pitch, out ) )

    roll = ( vsp.PLANE_1PT_ANGLE_INTERFERENCE, vsp.AUX_GEOM_ONE_PT_GROUND )
    right = _gear_angle( 0, *roll, [ ( "main", 0 ) ], out )
    left = _gear_angle( 0, *roll, [ ( "main", 1 ) ], out )
    assert right == pytest.approx( -left ), "the two sides do not roll opposite ways, so nothing is measured"
    assert _gear_angle( vsp.SYM_XZ, *roll, [ ( "main", 0 ) ], out ) == pytest.approx( left )
    drop_errors()


@pytest.mark.parametrize( "kind", [ "STACK", "FUSELAGE" ] )
def testAnInletExtensionAlignedWithXIsFlippedWithTheInlet( kind ):
    """An extension along X still runs away from the inlet when a YZ flip reverses the engine,
    so the whole engine is flipped."""
    fresh()

    def engine( flag, aligned ):
        fresh()
        gid = vsp.AddGeom( kind )
        xss = vsp.GetXSecSurf( gid, 0 )
        for i in range( vsp.GetNumXSec( xss ) ):
            vsp.ChangeXSecShape( xss, i, vsp.XS_ELLIPSE )
        vsp.Update()
        at( gid, x = 2.0 )
        for name, val in ( ( "GeomIOType", vsp.ENGINE_GEOM_INLET ),
                           ( "GeomInType", vsp.ENGINE_GEOM_FLOWTHROUGH ),
                           ( "GeomOutType", vsp.ENGINE_GEOM_FLOWTHROUGH ),
                           ( "InletModeType", vsp.ENGINE_MODE_EXTEND ),
                           ( "InletLipMode", vsp.ENGINE_LOC_INDEX ), ( "InletLipIndex", 1 ),
                           ( "InletFaceMode", vsp.ENGINE_LOC_INDEX ), ( "InletFaceIndex", 2 ),
                           ( "RotExtensionFlag", aligned ), ( "ExtensionDistance", 5.0 ) ):
            vsp.SetParmVal( vsp.FindParm( gid, name, "EngineModel" ), val )
        vsp.Update()
        flip( gid, flag )
        return span( gid, "X" )

    plain = engine( 0, 1 )
    assert engine( vsp.SYM_YZ, 1 ) == pytest.approx( reflected( plain, about = 2.0 ) )
    assert engine( vsp.SYM_YZ, 0 ) == pytest.approx( reflected( engine( 0, 0 ), about = 2.0 ) )
    assert_no_errors()


@pytest.mark.parametrize( "flag, axis", [ ( vsp.SYM_YZ, "X" ), ( vsp.SYM_XY, "Z" ), ( vsp.SYM_XZ, "Y" ) ] )
def testAVisionConeLooksOutOfTheFlippedHead( flag, axis ):
    """A super cone on a flipped Human is the reflection of the unflipped one, placed at the
    eye of the flipped head."""
    fresh()
    human = vsp.AddGeom( "HUMAN" )
    at( human, x = 10.0 )
    cone = vsp.AddGeom( "AUXILIARY", human )
    vsp.SetParmVal( vsp.FindParm( cone, "AuxiliaryGeomType", "Design" ), vsp.AUX_GEOM_SUPER_CONE )
    vsp.Update()
    before = span( cone, axis )

    about = 10.0 if axis == "X" else 0.0
    flip( human, flag )
    assert span( cone, axis ) == pytest.approx( reflected( before, about = about ), abs = 1e-6 )
    assert_no_errors()
