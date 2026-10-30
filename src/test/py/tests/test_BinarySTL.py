# Reading binary STL files.
#
# A standard binary STL file is an 80 byte header, a facet count, and that many 50 byte facets,
# each ending in a 2 byte attribute.  DATCOM writes one such block per solid, one after another,
# with a 4 byte attribute on every facet.  The reader takes either layout when the file's size
# agrees with it and refuses anything else.  The files are written here, at test time.

import openvsp as vsp

import os
import struct
import tempfile


def _tetra( shift ):
    """The four facets of a tetrahedron, each as a normal and three vertices."""
    p = [ ( -1.0, -1.0, -1.0 ), ( 1.0, -1.0, -1.0 ), ( -1.0, 1.0, -1.0 ), ( -1.0, -1.0, 1.0 ) ]
    p = [ ( x + shift, y, z ) for ( x, y, z ) in p ]
    return [ ( ( 0.0, 0.0, 0.0 ), p[0], p[2], p[1] ),
             ( ( 0.0, 0.0, 0.0 ), p[0], p[1], p[3] ),
             ( ( 0.0, 0.0, 0.0 ), p[0], p[3], p[2] ),
             ( ( 0.0, 0.0, 0.0 ), p[1], p[2], p[3] ) ]


def _block( facets, att_size ):
    """One header, its facet count and its facets, each followed by att_size attribute bytes."""
    data = b"binary stl".ljust( 80, b" " ) + struct.pack( "<I", len( facets ) )
    for f in facets:
        for v in f:
            data += struct.pack( "<3f", *v )
        data += bytes( att_size )
    return data


def _write( data ):
    path = os.path.join( tempfile.mkdtemp(), "part.stl" )
    with open( path, "wb" ) as f:
        f.write( data )
    return path


def _import( path ):
    """Import an STL file; return the mesh's triangle count, its bounds and the errors raised."""
    vsp.VSPRenew()
    mgr = vsp.ErrorMgrSingleton.getInstance()
    while mgr.GetNumTotalErrors() > 0:
        mgr.PopLastError()

    vsp.ImportFile( path, vsp.IMPORT_STL, "" )
    codes = [ mgr.PopLastError().m_ErrorCode for _ in range( mgr.GetNumTotalErrors() ) ]

    meshes = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Mesh" ]
    if not meshes:
        return 0, None, codes
    vsp.Update()

    out = os.path.join( tempfile.mkdtemp(), "out.stl" )
    vsp.ExportFile( out, vsp.SET_ALL, vsp.EXPORT_STL )
    num_tri = 0
    with open( out ) as f:
        for line in f:
            if line.split()[:1] == [ "facet" ]:
                num_tri += 1

    lo = vsp.GetGeomBBoxMin( meshes[0], 0, False )
    hi = vsp.GetGeomBBoxMax( meshes[0], 0, False )
    return num_tri, ( lo.x(), lo.y(), lo.z(), hi.x(), hi.y(), hi.z() ), codes


def testAStandardBinarySTLFileImports():
    num_tri, bounds, codes = _import( _write( _block( _tetra( 0.0 ), 2 ) ) )
    assert codes == []
    assert num_tri == 4
    assert bounds == ( -1.0, -1.0, -1.0, 1.0, 1.0, 1.0 )


def testABinarySTLFileOfDATCOMBlocksImports():
    """Two solids, each its own block of 52 byte facets."""
    data = _block( _tetra( 0.0 ), 4 ) + _block( _tetra( 10.0 ), 4 )
    num_tri, bounds, codes = _import( _write( data ) )
    assert codes == []
    assert num_tri == 8
    assert bounds == ( -1.0, -1.0, -1.0, 11.0, 1.0, 1.0 )


def testATruncatedBinarySTLFileIsRefused():
    data = _block( _tetra( 0.0 ), 2 )
    num_tri, bounds, codes = _import( _write( data[:-10] ) )
    assert num_tri == 0
    assert codes == [ vsp.VSP_FILE_READ_FAILURE ]


def testABinarySTLFileWithTrailingBytesIsRefused():
    data = _block( _tetra( 0.0 ), 2 ) + bytes( [ 0xff ] * 7 )
    num_tri, bounds, codes = _import( _write( data ) )
    assert num_tri == 0
    assert codes == [ vsp.VSP_FILE_READ_FAILURE ]
