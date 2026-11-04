# The CFD export files of one mesh describe the same points, faces and tags.
#
# The ASCII writers format their items in chunks of 2048 on several threads.  Each writer has
# its own chunk body, so a slip in one (an index taken relative to the chunk, a chunk written
# out of turn) changes that file alone while the mesh, its counts and its closure stay right.
# The serial and threaded runs share the bodies, so only files cross-checked against each other
# can show it.  The model is large enough that every list runs to several chunks.

import openvsp as vsp


def _airplane():
    vsp.VSPRenew()
    vsp.AddGeom( "FUSELAGE", "" )
    wing = vsp.AddGeom( "WING", "" )
    vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 10.0 )
    vsp.SetParmVal( wing, "Z_Rel_Location", "XForm", -0.5 )
    for y, z in ( ( 1.5, -0.2 ), ( 4.0, -0.3 ) ):
        pod = vsp.AddGeom( "POD", wing )
        vsp.SetParmVal( pod, "X_Rel_Location", "XForm", -2.0 )
        vsp.SetParmVal( pod, "Y_Rel_Location", "XForm", y )
        vsp.SetParmVal( pod, "Z_Rel_Location", "XForm", z )
        vsp.SetParmVal( pod, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
    vsp.Update()
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.4 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.05 )


def _read_tri( path ):
    tok = open( path ).read().split()
    npt, ntri = int( tok[0] ), int( tok[1] )
    k = 2
    pts = [ tuple( float( v ) for v in tok[k + 3 * i: k + 3 * i + 3] ) for i in range( npt ) ]
    k += 3 * npt
    tris = [ tuple( int( v ) for v in tok[k + 3 * i: k + 3 * i + 3] ) for i in range( ntri ) ]
    k += 3 * ntri
    tags = [ int( v ) for v in tok[k: k + ntri] ]
    return pts, tris, tags


def _read_dat( path ):
    """NASCART: points as x, z, -y; faces as i0 i2 i1 tag.0, one based."""
    lines = open( path ).read().split( "\n" )
    npt, ntri = ( int( v ) for v in lines[0].split() )
    pts = []
    for line in lines[1: 1 + npt]:
        x, z, my = ( float( v ) for v in line.split() )
        pts.append( ( x, -my, z ) )
    tris, tags = [], []
    for line in lines[1 + npt: 1 + npt + ntri]:
        w = line.split()
        tris.append( ( int( w[0] ), int( w[2] ), int( w[1] ) ) )
        tags.append( int( float( w[3] ) ) )
    return pts, tris, tags


def testTheTriAndNascartFilesAgree( tmp_path ):
    _airplane()
    tri = str( tmp_path / "m.tri" )
    dat = str( tmp_path / "m.dat" )
    vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, tri )
    vsp.SetComputationFileName( vsp.CFD_DAT_TYPE, dat )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE | vsp.CFD_DAT_TYPE )

    tp, tt, tg = _read_tri( tri )
    dp, dt, dg = _read_dat( dat )

    assert len( tt ) > 3 * 2048, "the mesh is too small to run to several chunks"
    assert len( tp ) == len( dp ) and len( tt ) == len( dt )
    for i in range( len( tp ) ):
        assert max( abs( a - b ) for a, b in zip( tp[i], dp[i] ) ) < 1e-6 * ( 1.0 + max( abs( a ) for a in tp[i] ) ), i
    assert tt == dt, "the TRI and NASCART faces differ"
    assert tg == dg, "the TRI and NASCART tags differ at %d faces" % sum( a != b for a, b in zip( tg, dg ) )
    # A tag per component, and more than one in use.
    assert len( set( tg ) ) > 1
