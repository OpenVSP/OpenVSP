# How CFDMesh cuts a component into patches when the split-join option is on.
#
# With the option on, a surface is cut only where the mesh needs a patch edge: at sharp
# corners, at the edge strips of a lifting surface, at its end caps, and once more where a
# surface closed on itself would otherwise be one periodic patch.  Every other feature line
# becomes a join inside a patch.  With the option off, every feature line is a patch edge,
# as it always was.
#
# The number of surfaces meshed is the measure: it counts the patches that reached the
# mesher, after the coincident faces of two halves of a mirrored component are removed.
# Each model is also meshed to closure, since a patch rule that loses a face shows up as
# open edges, not as a wrong count.  A count does not say where the cuts fall, so the faces
# of the POGS file are placed against the body's planes of symmetry; nor does it say a feature
# line was kept as a join, so the mesh nodes along such lines are counted.

import collections
import struct

import openvsp as vsp
import pytest


def _mesh( splitjoin, half = 0 ):
    """Mesh every Geom with coarse edges and hand back the CFDMesh results as a plain dict."""
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.8 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.2 )
    vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, splitjoin )
    vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, half )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_STL_TYPE )

    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert rid, "the mesher wrote no CFDMesh results entry"

    out = {}
    for name in ( "Num_Surfs", "Num_Border_Edges", "Num_Over_Connected_Edges" ):
        out[name] = vsp.GetIntResults( rid, name )[0]
    return out


def _check( build, nsurf_off, nsurf_on ):
    """Mesh a model with split-join off and on: the surface counts, and both meshes closed."""
    for splitjoin, nsurf in ( ( 0, nsurf_off ), ( 1, nsurf_on ) ):
        vsp.VSPRenew()
        build()
        vsp.Update()

        res = _mesh( splitjoin )

        assert res["Num_Surfs"] == nsurf, \
               "split-join %d meshed %d surfaces, expected %d" % ( splitjoin, res["Num_Surfs"], nsurf )
        assert res["Num_Border_Edges"] == 0 and res["Num_Over_Connected_Edges"] == 0, \
               "split-join %d left %d border and %d over-connected edges" % \
               ( splitjoin, res["Num_Border_Edges"], res["Num_Over_Connected_Edges"] )


def _blunt( wing ):
    xs_surf = vsp.GetXSecSurf( wing, 0 )
    for i in range( vsp.GetNumXSec( xs_surf ) ):
        xs = vsp.GetXSec( xs_surf, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Close_Type" ), vsp.CLOSE_SKEWBOTH )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Close_AbsRel" ), vsp.REL )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "TE_Close_Thick_Chord" ), 0.02 )


def _wing( blunt = False, tipcap = vsp.FLAT_END_CAP ):
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wing, "CapUMaxOption", "EndCap", tipcap )
    if blunt:
        _blunt( wing )
    return wing


def _box_fuselage():
    """A fuselage whose sections are rectangles with sharp corners."""
    fuse = vsp.AddGeom( "FUSELAGE" )
    xs_surf = vsp.GetXSecSurf( fuse, 0 )
    for i in range( 1, vsp.GetNumXSec( xs_surf ) - 1 ):
        vsp.ChangeXSecShape( xs_surf, i, vsp.XS_ROUNDED_RECTANGLE )
        xs = vsp.GetXSec( xs_surf, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundedRect_Width" ), 1.5 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundedRect_Height" ), 1.0 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundRectXSec_Radius" ), 0.0 )


def _two_section_wing( dihedral = 0.0, sweep = 0.0 ):
    """A rectangular wing of two panels, 4 long and 3 in chord, whose outer panel alone is turned."""
    wing = vsp.AddGeom( "WING" )
    vsp.InsertXSec( wing, 1, vsp.XS_FOUR_SERIES )
    vsp.Update()
    for xs in ( "XSec_1", "XSec_2" ):
        vsp.SetParmVal( wing, "Span", xs, 4.0 )
        vsp.SetParmVal( wing, "Root_Chord", xs, 3.0 )
        vsp.SetParmVal( wing, "Tip_Chord", xs, 3.0 )
        vsp.SetParmVal( wing, "Sweep", xs, 0.0 )
    vsp.SetParmVal( wing, "Dihedral", "XSec_2", dihedral )
    vsp.SetParmVal( wing, "Sweep", "XSec_2", sweep )
    return wing


def _body_of_revolution( mode ):
    bor = vsp.AddGeom( "BODYOFREVOLUTION" )
    vsp.SetParmVal( bor, "Mode", "Design", mode )


def testASmoothBodyIsTwoHalves():
    """A pod is cut along its own symmetry plane into two halves, never one periodic patch."""
    _check( lambda: vsp.AddGeom( "POD" ), 8, 2 )


def testAFuselageIsTwoHalves():
    _check( lambda: vsp.AddGeom( "FUSELAGE" ), 8, 2 )


def testABoxFuselageIsCutAtItsCorners():
    """Four sharp corners give four patches, and no cut on the top and bottom lines between them."""
    _check( _box_fuselage, 16, 4 )


def testAFlowThroughBodyIsTwoHalvesInsideAndOut():
    _check( lambda: _body_of_revolution( 0 ), 8, 4 )


def testAClosedBodyOfRevolutionIsTwoHalves():
    _check( lambda: _body_of_revolution( 1 ), 8, 2 )


def testASharpTrailingEdgeWing():
    """Each half: upper and lower surfaces and the tip cap.  The root caps of the halves meet and both go."""
    _check( _wing, 8, 6 )


def testABluntTrailingEdgeIsOnePatch():
    """Each half adds its blunt trailing edge as one patch, not an upper and a lower piece."""
    _check( lambda: _wing( blunt = True ), 12, 8 )


def testARoundTipCapOnABluntWing():
    _check( lambda: _wing( blunt = True, tipcap = vsp.ROUND_END_CAP ), 16, 10 )


def testAWingOnAFuselage():
    def build():
        vsp.AddGeom( "FUSELAGE" )
        wing = _wing()
        vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 10.0 )

    _check( build, 16, 8 )


def testAHalfMesh():
    """With the half mesh on, the y < 0 side goes and the mesh is open only on the symmetry plane,
    which is not counted as a border.

    A body is cut on its top and bottom lines, so the y < 0 side goes as whole patches: the pod
    keeps its +y half, and the box fuselage its top, bottom and +y side.
    """
    for build, nsurf in ( ( lambda: vsp.AddGeom( "POD" ), 1 ), ( _box_fuselage, 3 ) ):
        vsp.VSPRenew()
        build()
        vsp.Update()

        res = _mesh( 1, half = 1 )

        assert res["Num_Surfs"] == nsurf, "the half mesh meshed %d surfaces, expected %d" % ( res["Num_Surfs"], nsurf )
        assert res["Num_Border_Edges"] == 0, "the half mesh is open off its symmetry plane"
        assert res["Num_Over_Connected_Edges"] == 0, \
               "the half mesh left %d over-connected edges" % res["Num_Over_Connected_Edges"]


def _read_tri( path ):
    """The points and triangles of a Cart3D .tri file, its triangles as 0-based point indices."""
    tok = open( path ).read().split()
    nv, nt = int( tok[0] ), int( tok[1] )
    pts = [ tuple( float( x ) for x in tok[2 + 3 * i:5 + 3 * i] ) for i in range( nv ) ]
    p = 2 + 3 * nv
    tris = [ tuple( int( x ) - 1 for x in tok[p + 3 * i:p + 3 * i + 3] ) for i in range( nt ) ]
    return pts, tris


def _read_pogs_faces( path ):
    """The box around each face of a POGS .i.tri file, by face tag.

    The file is Fortran unformatted, little-endian: counts, points, triangles, then a tag per
    triangle, which is the POGS face it belongs to.
    """
    data = open( path, "rb" ).read()
    recs = []
    p = 0
    while p < len( data ):
        n = struct.unpack( "<i", data[p:p + 4] )[0]
        recs.append( data[p + 4:p + 4 + n] )
        p += 8 + n

    nv, nt = struct.unpack( "<2i", recs[0] )
    xyz = struct.unpack( "<%df" % ( 3 * nv ), recs[1] )
    tri = struct.unpack( "<%di" % ( 3 * nt ), recs[2] )
    tags = struct.unpack( "<%di" % nt, recs[3] )

    box = {}
    for i in range( nt ):
        lo, hi = box.setdefault( tags[i], ( [ 1e30 ] * 3, [ -1e30 ] * 3 ) )
        for k in range( 3 ):
            v = tri[3 * i + k] - 1
            for m in range( 3 ):
                lo[m] = min( lo[m], xyz[3 * v + m] )
                hi[m] = max( hi[m], xyz[3 * v + m] )
    return box


def _mesh_to( path, filetype, half = 0 ):
    """Mesh every Geom with split-join on and write the file of the type given to path."""
    vsp.SetComputationFileName( filetype, str( path ) )
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.8 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.2 )
    vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 1 )
    vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, half )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, filetype )

    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert rid, "the mesher wrote no CFDMesh results entry"

    out = {}
    for name in ( "Num_Surfs", "Num_Border_Edges", "Num_Over_Connected_Edges" ):
        out[name] = vsp.GetIntResults( rid, name )[0]
    return out


def _sides( lo, hi, ymid, tol ):
    """Which sides of the planes y = ymid and z = 0 a box lies wholly on: '+y', '-y', '+z', '-z'."""
    sides = []
    if lo[1] >= ymid - tol:
        sides.append( "+y" )
    if hi[1] <= ymid + tol:
        sides.append( "-y" )
    if lo[2] >= -tol:
        sides.append( "+z" )
    if hi[2] <= tol:
        sides.append( "-z" )
    return "".join( sides )


@pytest.mark.parametrize( "name, build, faces", [
    ( "pod", lambda: vsp.AddGeom( "POD" ), [ "+y", "-y" ] ),
    ( "box fuselage", _box_fuselage, [ "+y", "+z", "-y", "-z" ] ) ] )
@pytest.mark.parametrize( "ymid", [ 0.0, 3.0 ] )
def testAClosedBodyIsCutAlongItsOwnSymmetryPlane( name, build, faces, ymid, tmp_path ):
    """A body is cut along its top and bottom lines, whether or not it sits on y = 0.

    Each POGS face then lies wholly on one side of the vertical plane through the body's axis.
    The box fuselage is cut at its four corners instead, so its top and bottom lie above and
    below the horizontal plane through its axis.
    """
    vsp.VSPRenew()
    build()
    for gid in vsp.FindGeoms():
        vsp.SetParmVal( gid, "Y_Rel_Location", "XForm", ymid )
    vsp.Update()

    path = tmp_path / "body.i.tri"
    res = _mesh_to( path, vsp.CFD_POGS_TYPE )
    assert res["Num_Border_Edges"] == 0 and res["Num_Over_Connected_Edges"] == 0, \
           "the %s at y = %g left %d border and %d over-connected edges" % \
           ( name, ymid, res["Num_Border_Edges"], res["Num_Over_Connected_Edges"] )

    sides = sorted( _sides( lo, hi, ymid, 1e-4 ) for lo, hi in _read_pogs_faces( path ).values() )
    assert sides == faces, "the %s at y = %g has faces on sides %s, expected %s" % ( name, ymid, sides, faces )


def testACornerAlongTheSpanIsCut():
    """A dihedral break and a sweep break past the sharp angle each cut the wing at the break.

    Each half: upper and lower surfaces of each panel and the tip cap.
    """
    _check( lambda: _two_section_wing( dihedral = 30.0 ), 12, 10 )
    _check( lambda: _two_section_wing( sweep = 10.0 ), 12, 10 )


def testAKinkBelowTheSharpAngleKeepsNodesAlongIt( tmp_path ):
    """A 5 deg sweep break is no cut, so the panels are one patch, but the mesh still has a row of
    nodes along the section at the break."""
    _check( lambda: _two_section_wing( sweep = 5.0 ), 12, 6 )

    path = tmp_path / "kink.tri"
    _mesh_to( path, vsp.CFD_TRI_TYPE )
    pts, tris = _read_tri( path )

    # The section at the break is 3 long, upper and lower, at edges of no more than 0.8.
    onbreak = sum( 1 for p in pts if abs( p[1] - 4.0 ) < 1e-6 )
    assert onbreak >= 8, "only %d mesh nodes lie on the section at the break" % onbreak


@pytest.mark.parametrize( "ymid", [ 0.0, 3.0 ] )
def testAFuselageKeepsNodesAlongItsFeatureLines( ymid, tmp_path ):
    """A fuselage is two patches, but its top and bottom lines, its side lines and its middle
    section each keep a row of mesh nodes along them, whether or not it sits on y = 0."""
    vsp.VSPRenew()
    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fuse, "Y_Rel_Location", "XForm", ymid )
    vsp.Update()

    path = tmp_path / "fuse.tri"
    res = _mesh_to( path, vsp.CFD_TRI_TYPE )
    assert res["Num_Surfs"] == 2, "the fuselage at y = %g meshed %d surfaces, expected 2" % ( ymid, res["Num_Surfs"] )
    pts, tris = _read_tri( path )

    xmid = 0.5 * vsp.GetParmVal( fuse, "Length", "Design" )
    lines = ( ( "top and bottom", lambda p: abs( p[1] - ymid ) < 1e-6 ),
              ( "side", lambda p: abs( p[2] ) < 1e-6 ),
              ( "middle section", lambda p: abs( p[0] - xmid ) < 1e-6 ) )
    for name, online in lines:
        n = sum( 1 for p in pts if online( p ) )
        assert n >= 20, "the fuselage at y = %g has only %d mesh nodes on its %s lines" % ( ymid, n, name )


def _blunt_fin():
    """A blunt wing turned on end, so the y = 0 plane runs through it from leading to trailing edge."""
    fin = _wing( blunt = True )
    vsp.SetParmVal( fin, "Sym_Planar_Flag", "Sym", 0 )
    vsp.SetParmVal( fin, "X_Rel_Rotation", "XForm", 90.0 )


@pytest.mark.parametrize( "name, build, nsurf", [ ( "blunt wing", lambda: _wing( blunt = True ), 4 ),
                                                  ( "blunt fin", _blunt_fin, 4 ) ] )
def testABluntTrailingEdgeInAHalfMesh( name, build, nsurf, tmp_path ):
    """With the half mesh on, the y < 0 side goes, and the mesh is open on the symmetry plane and
    nowhere else.

    The wing keeps its +y upper and lower surfaces, its blunt trailing edge as one patch and its
    tip cap.  The fin keeps its +y side, and the +y halves of its blunt trailing edge, root cap
    and tip cap.
    """
    vsp.VSPRenew()
    build()
    vsp.Update()

    path = tmp_path / "blunt.tri"
    res = _mesh_to( path, vsp.CFD_TRI_TYPE, half = 1 )
    assert res["Num_Surfs"] == nsurf, "the %s half mesh meshed %d surfaces, expected %d" % ( name, res["Num_Surfs"], nsurf )
    assert res["Num_Over_Connected_Edges"] == 0, \
           "the %s half mesh left %d over-connected edges" % ( name, res["Num_Over_Connected_Edges"] )

    pts, tris = _read_tri( path )
    ymin = min( p[1] for p in pts )
    assert ymin >= -1e-6, "the %s half mesh has a point at y = %g" % ( name, ymin )

    uses = collections.Counter()
    for t in tris:
        for k in range( 3 ):
            a, b = t[k], t[( k + 1 ) % 3]
            uses[( min( a, b ), max( a, b ) )] += 1
    off_plane = [ e for e, n in uses.items() if n == 1 and ( abs( pts[e[0]][1] ) > 1e-6 or abs( pts[e[1]][1] ) > 1e-6 ) ]
    assert not off_plane, "the %s half mesh has %d open edges off the symmetry plane" % ( name, len( off_plane ) )
