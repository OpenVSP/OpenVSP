# Helpers shared by the flip tests: bounding extents and their reflection, setting a flip,
# a shape asymmetric about every plane, and the winding of a written mesh.
#
# Not a test file -- pytest collects test_*.py, which import this.

import openvsp as vsp
import re


def span( gid, axis = "Y" ):
    """The Geom's (min, max) along one axis."""
    lo = vsp.GetParmVal( vsp.FindParm( gid, axis + "_Min", "BBox" ) )
    return ( lo, lo + vsp.GetParmVal( vsp.FindParm( gid, axis + "_Len", "BBox" ) ) )


def yspan( gid ):
    return span( gid, "Y" )


def reflected( s, about = 0.0 ):
    """The extent reflected about a plane at 'about' (default the origin)."""
    return ( 2.0 * about - s[1], 2.0 * about - s[0] )


def a_chiral_wing( offset = None ):
    """A wing with dihedral, asymmetric about all three coordinate planes, so any flip shows."""
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( wing, "Sym_Planar_Flag", "Sym" ), 0 )
    vsp.SetParmVal( vsp.FindParm( wing, "Dihedral", "XSec_1" ), 25.0 )
    if offset is not None:
        for parm, val in zip( ( "X_Rel_Location", "Y_Rel_Location", "Z_Rel_Location" ), offset ):
            vsp.SetParmVal( vsp.FindParm( wing, parm, "XForm" ), val )
    vsp.Update()
    return wing


def flip( gid, flag ):
    vsp.SetParmVal( vsp.FindParm( gid, "Flip_Flag", "Sym" ), flag )
    vsp.Update()


def signed_volume( nodes, tris ):
    """Signed volume of a closed indexed mesh, from its winding alone; positive is outward.

    Stored normals are ignored: they can look right on a mesh that is inside out.
    """
    total = 0.0
    for tri in tris:
        a, b, c = ( nodes[i] for i in tri )
        total += ( a[0] * ( b[1] * c[2] - b[2] * c[1] )
                 - a[1] * ( b[0] * c[2] - b[2] * c[0] )
                 + a[2] * ( b[0] * c[1] - b[1] * c[0] ) ) / 6.0
    return total


def read_stl( path ):
    """Every facet's three vertices, in the order written."""
    nodes = []
    tris = []
    for line in open( path ).read().split( "\n" ):
        parts = line.split()
        if len( parts ) == 4 and parts[0] == "vertex":
            nodes.append( tuple( float( x ) for x in parts[1:] ) )
    for i in range( 0, len( nodes ) - 2, 3 ):
        tris.append( ( i, i + 1, i + 2 ) )
    return nodes, tris


def read_cart3d( path ):
    """Cart3D ASCII: "np nt", np xyz rows, then nt one-based index triples."""
    toks = open( path ).read().split()
    npt = int( toks[0] )
    ntri = int( toks[1] )
    at = 2
    nodes = []
    for _ in range( npt ):
        nodes.append( ( float( toks[at] ), float( toks[at + 1] ), float( toks[at + 2] ) ) )
        at += 3
    tris = []
    for _ in range( ntri ):
        tris.append( ( int( toks[at] ) - 1, int( toks[at + 1] ) - 1, int( toks[at + 2] ) - 1 ) )
        at += 3
    return nodes, tris


def read_vspgeom( path, alternate = False ):
    """The finest mesh of a VSPGEOM file, each face fanned into triangles in the order written.

    With alternate, read the alternate block's triangles instead.
    """
    lines = [ l for l in open( path ).read().split( "\n" ) if l.strip() and not l.startswith( "#" ) ]
    nmesh = int( lines[0] )
    npt, nface, nwake = ( int( t ) for t in lines[1].split()[:3] )
    at = 1 + nmesh
    nodes = []
    for _ in range( npt ):
        nodes.append( tuple( float( t ) for t in lines[at].split()[:3] ) )
        at += 1
    assert int( lines[at] ) == nface
    at += 1
    tris = []
    for _ in range( nface ):
        idx = [ int( t ) - 1 for t in lines[at].split()[1:] ]
        for k in range( 1, len( idx ) - 1 ):
            tris.append( ( idx[0], idx[k], idx[k + 1] ) )
        at += 1
    if not alternate:
        return nodes, tris

    # Parts, then parents, one line per face each.
    at += 2 * nface
    assert int( lines[at] ) == nwake
    at += 1
    for _ in range( nwake ):
        # A wake: node count, part, then that many nodes over any number of lines.
        toks = lines[at].split()
        at += 1
        need = 2 + abs( int( toks[0] ) )
        while len( toks ) < need:
            toks += lines[at].split()
            at += 1
    tris = []
    for _ in range( nface ):
        toks = [ int( t ) for t in lines[at].split() ]
        ntri = toks[1]
        assert len( toks ) == 2 + 3 * ntri, "not an alternate triangle line: %s" % lines[at]
        for k in range( ntri ):
            tris.append( tuple( toks[2 + 3 * k + j] - 1 for j in range( 3 ) ) )
        at += 1
    return nodes, tris


def read_x3d( path ):
    """Every indexed face set of an X3D file, each face fanned into triangles in the order written."""
    text = open( path ).read()
    faces = re.findall( r'coordIndex="([^"]*)"', text )
    points = re.findall( r'<Coordinate point="([^"]*)"', text )
    assert len( faces ) == len( points ), "a face set without its points"
    nodes = []
    tris = []
    for idx_text, pnt_text in zip( faces, points ):
        offset = len( nodes )
        vals = [ float( t ) for t in pnt_text.replace( ",", " " ).split() ]
        for i in range( 0, len( vals ) - 2, 3 ):
            nodes.append( ( vals[i], vals[i + 1], vals[i + 2] ) )
        face = []
        for t in idx_text.replace( ",", " " ).split():
            i = int( t )
            if i < 0:
                for k in range( 1, len( face ) - 1 ):
                    tris.append( ( offset + face[0], offset + face[k], offset + face[k + 1] ) )
                face = []
            else:
                face.append( i )
    return nodes, tris
