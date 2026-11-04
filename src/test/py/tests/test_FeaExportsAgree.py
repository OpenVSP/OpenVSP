# The FEA export files of one structure mesh list the same shell elements, wound the same way.
#
# Each writer (NASTRAN, Gmsh, Calculix, STL) formats its own elements in threaded chunks, with a
# body of its own.  A slip in one body (a corner order, an index taken relative to the chunk)
# changes that file alone; the counts in the NASTRAN header, the serial and threaded runs and the
# other files stay as they were.  Only files cross-checked against each other show it.

import openvsp as vsp


def _wing_with_a_structure():
    """A wing carrying a spar and two ribs, with both end caps rounded."""
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING", "" )
    vsp.SetParmVal( vsp.FindParm( wid, "CapUMinOption", "EndCap" ), vsp.ROUND_END_CAP )
    vsp.SetParmVal( vsp.FindParm( wid, "CapUMaxOption", "EndCap" ), vsp.ROUND_END_CAP )
    vsp.Update()
    vsp.AddFeaStruct( wid )
    spar = vsp.AddFeaPart( wid, 0, vsp.FEA_SPAR )
    vsp.SetParmVal( vsp.FindParm( spar, "RelCenterLocation", "FeaPart" ), 0.4 )
    for loc in ( 0.35, 0.7 ):
        rib = vsp.AddFeaPart( wid, 0, vsp.FEA_RIB )
        vsp.SetParmVal( vsp.FindParm( rib, "RelCenterLocation", "FeaPart" ), loc )
    vsp.Update()
    vsp.SetFeaMeshVal( wid, 0, vsp.CFD_MAX_EDGE_LEN, 0.5 )
    vsp.SetFeaMeshVal( wid, 0, vsp.CFD_MIN_EDGE_LEN, 0.1 )
    return wid


def _canon( tri ):
    """A triangle's corners from its least one on, keeping the winding."""
    k = tri.index( min( tri ) )
    return tri[k:] + tri[:k]


NCORNER_NAS = { "CTRIA3": 3, "CTRIA6": 3, "CQUAD4": 4, "CQUAD8": 4 }
NCORNER_GMSH = { 2: 3, 9: 3, 3: 4, 10: 4, 16: 4 }


def _nastran_elems( path ):
    """Each shell element's corners, from the bulk data (.bdf) file the deck includes."""
    elems = []
    for line in open( path + ".bdf" ):
        f = [ v.strip() for v in line.split( "," ) ]
        if f[0] in NCORNER_NAS:
            nc = NCORNER_NAS[ f[0] ]
            elems.append( _canon( tuple( int( v ) for v in f[3: 3 + nc] ) ) )
    return sorted( elems )


def _gmsh_elems( path ):
    lines = open( path ).read().split( "\n" )
    k = lines.index( "$Elements" )
    n = int( lines[k + 1] )
    elems = []
    for line in lines[k + 2: k + 2 + n]:
        w = [ int( v ) for v in line.split() ]
        if w[1] in NCORNER_GMSH:
            ntag = w[2]
            elems.append( _canon( tuple( w[3 + ntag: 3 + ntag + NCORNER_GMSH[ w[1] ] ] ) ) )
    return sorted( elems )


def testTheNastranAndGmshFilesListTheSameElements( tmp_path ):
    wid = _wing_with_a_structure()
    sid = vsp.GetFeaStructID( wid, 0 )
    nas = str( tmp_path / "w.nas" )
    msh = str( tmp_path / "w.msh" )
    vsp.SetFeaMeshFileName( wid, 0, vsp.FEA_NASTRAN_FILE_NAME, nas )
    vsp.SetFeaMeshFileName( wid, 0, vsp.FEA_GMSH_FILE_NAME, msh )
    vsp.ComputeFeaMesh( sid, vsp.FEA_NASTRAN_FILE_NAME )
    vsp.ComputeFeaMesh( sid, vsp.FEA_GMSH_FILE_NAME )

    a = _nastran_elems( nas )
    b = _gmsh_elems( msh )
    assert len( a ) > 0, "the NASTRAN deck lists no shell elements"
    assert len( a ) == len( b ), "NASTRAN lists %d elements, Gmsh %d" % ( len( a ), len( b ) )
    nbad = sum( 1 for x, y in zip( a, b ) if x != y )
    assert a == b, "%d of %d elements differ between the NASTRAN and Gmsh files" % ( nbad, len( a ) )
