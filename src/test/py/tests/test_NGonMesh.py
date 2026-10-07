# The polygon mesh a model is turned into for panel analysis, and the two optional passes that
# tidy it: one merges regions of degenerate triangles, the other collapses short
# intersection-curve segments.
#
# Nothing can look inside an NGonMeshGeom from the API, so these read the .vspgeom it exports --
# the same file VSPAERO is handed.

import openvsp as vsp
import pytest

import os
import tempfile
from collections import Counter


#==== Reading what was written ====#

def _read_vspgeom( path ):
    """Nodes and face loops out of a single-mesh .vspgeom v3 file.

    Layout: a comment line, the mesh count, 'nnode nface nwake', one line per node, a repeat of
    the face count, then one line per face -- the loop length followed by that many 1-based
    indices.  Faces the writer could not close are written as comments and counted, not parsed.
    """
    lines = open( path ).read().split( "\n" )

    i = 0
    while lines[i].startswith( "#" ) or not lines[i].strip():
        i += 1
    i += 1                                              # the mesh count

    head = lines[i].split()
    nnode, nface = int( head[0] ), int( head[1] )
    i += 1

    nodes = [ tuple( float( x ) for x in lines[i + k].split()[:3] ) for k in range( nnode ) ]
    i += nnode

    assert int( lines[i].split()[0] ) == nface, \
           "the face count in the header disagrees with the one before the faces"
    i += 1

    faces = []
    broken = 0
    for k in range( nface ):
        line = lines[i + k]
        if "FACE ERROR" in line:
            broken += 1
            continue
        p = [ int( x ) for x in line.split() ]
        assert len( p ) >= p[0] + 1, "face %d is shorter than it says it is: %r" % ( k, line )
        faces.append( p[1:1 + p[0]] )

    return nodes, faces, broken


def _read_vspgeom_meshes( path ):
    """Per-mesh ( nodes, faces ) out of the header, coarse to fine.

    A .vspgeom holds one mesh per refinement level and one shared point block for all of them.
    """
    lines = open( path ).read().split( "\n" )
    i = 0
    while lines[i].startswith( "#" ) or not lines[i].strip():
        i += 1

    nmesh = int( lines[i] )
    return [ ( int( lines[i + 1 + k].split()[0] ), int( lines[i + 1 + k].split()[1] ) )
             for k in range( nmesh ) ]


def _edge_use( faces ):
    """How many faces each edge belongs to."""
    used = Counter()
    for f in faces:
        for a, b in zip( f, f[1:] + f[:1] ):
            used[ ( min( a, b ), max( a, b ) ) ] += 1
    return used


#==== Building something to mesh ====#

def _scratch():
    """Send analysis reports and the exported mesh to a scratch directory.

    Every VSPRenew resets these, so this is called after each one.
    """
    out = tempfile.mkdtemp()
    for t in ( vsp.COMP_GEOM_TXT_TYPE, vsp.COMP_GEOM_CSV_TYPE ):
        vsp.SetComputationFileName( t, os.path.join( out, "comp_geom.txt" ) )
    vsp.SetVSP3FileName( os.path.join( out, "model.vsp3" ) )
    return out


def _wing_over_fuselage():
    """A wing sitting high on a fuselage, just grazing it.

    A grazing intersection is what yields segments short enough for the cleaning pass to touch;
    a square-on one yields none.
    """
    vsp.VSPRenew()
    out = _scratch()
    vsp.AddGeom( "FUSELAGE" )
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( wing, "X_Rel_Location", "XForm" ), 2.0 )
    vsp.SetParmVal( vsp.FindParm( wing, "Z_Rel_Location", "XForm" ), 0.62 )
    vsp.Update()
    return out


def _export_ngon( out, counts_only = False ):
    """The NGonMeshGeom in the model, written out and read back.

    counts_only reads the header and stops, giving one ( nodes, faces ) pair per refinement
    level.  A refined file holds several meshes sharing one point block, so its body does not
    walk the way a single-mesh one does.
    """
    ngons = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "NGonMesh" ]
    assert len( ngons ) == 1, "expected one NGonMesh, found %d" % len( ngons )

    for g in vsp.FindGeoms():
        vsp.SetSetFlag( g, vsp.SET_FIRST_USER, g == ngons[0] )

    path = os.path.join( out, "ngon.vspgeom" )
    vsp.ExportFile( path, vsp.SET_FIRST_USER, vsp.EXPORT_VSPGEOM )

    if counts_only:
        return _read_vspgeom_meshes( path )

    return _read_vspgeom( path )


_panel_cache = {}

def _panel_mesh( merge = True, clean = True, ratio = 0.1, nref = 0, levels = False ):
    """The panel mesh VSPAERO would be given, with the cleanup passes set as asked.

    The flags are analysis inputs, not Parms -- Execute copies its inputs over the manager's
    Parms, so setting the Parms directly would be overwritten.
    """
    key = ( merge, clean, ratio, nref, levels )
    if key in _panel_cache:
        return _panel_cache[ key ]

    out = _wing_over_fuselage()

    vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_ALL ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ vsp.SET_NONE ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "MergeDegenTriRegionsFlag", [ int( merge ) ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "CleanIntersectionsFlag", [ int( clean ) ] )
    vsp.SetDoubleAnalysisInput( "VSPAEROComputeGeometry", "CleanIntersectionsLenRatio", [ ratio ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "NRef", [ nref ] )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )

    _panel_cache[ key ] = _export_ngon( out, counts_only = levels )
    return _panel_cache[ key ]


#==== The mesh a MeshGeom is turned into ====#

def _ngon_of_a_pod():
    vsp.VSPRenew()
    out = _scratch()
    vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    mesh = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Mesh" ][0]
    ngon = vsp.CreateNGonMeshGeom( mesh )
    assert vsp.GetGeomTypeName( ngon ) == "NGonMesh"
    return _export_ngon( out )


def testAMeshBecomesAPolygonMesh():
    nodes, faces, broken = _ngon_of_a_pod()
    assert len( nodes ) > 0 and len( faces ) > 0
    assert broken == 0, "the writer could not close %d faces" % broken


def testEveryFaceIsASimpleLoop():
    """A face names each of its nodes once, names at least three, and names nodes that exist."""
    nodes, faces, broken = _ngon_of_a_pod()

    for f in faces:
        assert len( f ) >= 3, "a face with %d nodes is not a polygon" % len( f )
        assert len( set( f ) ) == len( f ), "a face names the same node twice: %r" % f
        for n in f:
            assert 1 <= n <= len( nodes ), "a face names node %d of %d" % ( n, len( nodes ) )


def testThePolygonMeshIsClosed():
    """A pod is a closed body, so every edge belongs to exactly two faces."""
    nodes, faces, broken = _ngon_of_a_pod()

    used = _edge_use( faces )
    wrong = { e: n for e, n in used.items() if n != 2 }
    assert not wrong, "%d edges are not shared by exactly two faces, e.g. %r" % \
                      ( len( wrong ), list( wrong.items() )[:3] )


def testMakingItTwiceGivesTheSameMesh():
    """Nothing about this depends on what ran before it."""
    first = _ngon_of_a_pod()
    second = _ngon_of_a_pod()
    assert ( len( first[0] ), len( first[1] ) ) == ( len( second[0] ), len( second[1] ) )


#==== Merging regions of degenerate triangles ====#

def testMergingRegionsRemovesFaces():
    merged_n, merged_f, _ = _panel_mesh( merge = True )
    plain_n, plain_f, _ = _panel_mesh( merge = False )

    assert len( merged_f ) < len( plain_f ), \
           "merging left as many faces as not merging: %d and %d" % ( len( merged_f ), len( plain_f ) )


def testMergingRegionsLeavesTheNodesAlone():
    """Merging joins faces along shared edges; it does not move or remove any node."""
    merged_n, _, _ = _panel_mesh( merge = True )
    plain_n, _, _ = _panel_mesh( merge = False )

    assert len( merged_n ) == len( plain_n ), \
           "merging changed the node count: %d against %d" % ( len( merged_n ), len( plain_n ) )


def _malformed( faces ):
    """Faces that are not simple loops: too few nodes, or a node named more than once."""
    return ( [ f for f in faces if len( f ) < 3 ],
             [ f for f in faces if len( set( f ) ) != len( f ) ] )


def testMergingRegionsAddsNoMalformedFaces():
    """Merging must not leave a face that is not a simple loop, measured against not merging."""
    _, merged, merged_broken = _panel_mesh( merge = True )
    _, plain, plain_broken = _panel_mesh( merge = False )

    assert merged_broken <= plain_broken, \
           "merging left %d faces unclosed where not merging left %d" % ( merged_broken, plain_broken )

    m_small, m_rep = _malformed( merged )
    p_small, p_rep = _malformed( plain )

    assert len( m_small ) <= len( p_small ), "merging made %d faces with fewer than three nodes, against %d" % ( len( m_small ), len( p_small ) )
    assert len( m_rep ) <= len( p_rep ), "merging pinched %d faces, against %d" % ( len( m_rep ), len( p_rep ) )


#==== Cleaning short intersection segments ====#

# The shortest intersection segments on this model are 0.10 to 0.15 of their neighbours: 0.15
# collapses 2, 0.2 collapses 4.  The default, 0.1, has nothing here to take.
CLEAN_RATIO = 0.2


def testCleaningIntersectionsRemovesNodes():
    cleaned_n, _, _ = _panel_mesh( clean = True, ratio = CLEAN_RATIO )
    plain_n, _, _ = _panel_mesh( clean = False )

    assert len( cleaned_n ) < len( plain_n ), \
           "cleaning removed no nodes: %d and %d" % ( len( cleaned_n ), len( plain_n ) )


def testALengthRatioOfZeroCleansNothing():
    """A zero ratio has to match leaving the pass switched off."""
    zero_n, zero_f, _ = _panel_mesh( clean = True, ratio = 0.0 )
    off_n, off_f, _ = _panel_mesh( clean = False )

    assert ( len( zero_n ), len( zero_f ) ) == ( len( off_n ), len( off_f ) )


def testALongerRatioCleansAtLeastAsMuch():
    """The ratio is what counts as short, so raising it can only take more away."""
    counts = [ len( _panel_mesh( clean = True, ratio = r )[0] ) for r in ( 0.0, 0.05, 0.1, 0.3 ) ]

    for a, b in zip( counts, counts[1:] ):
        assert b <= a, "a longer ratio kept more nodes: %s" % counts
    assert counts[-1] < counts[0], "raising the ratio never removed anything: %s" % counts


def testCleaningIntersectionsAddsNoMalformedFaces():
    """Collapsing a short segment must not pinch the faces on either side of it."""
    nodes, cleaned, cleaned_broken = _panel_mesh( clean = True, ratio = CLEAN_RATIO )
    _, plain, plain_broken = _panel_mesh( clean = False )

    assert cleaned_broken <= plain_broken, \
           "cleaning left %d faces unclosed where not cleaning left %d" % ( cleaned_broken, plain_broken )

    c_small, c_rep = _malformed( cleaned )
    p_small, p_rep = _malformed( plain )

    assert len( c_small ) <= len( p_small ), "cleaning made %d faces with fewer than three nodes, against %d" % ( len( c_small ), len( p_small ) )
    assert len( c_rep ) <= len( p_rep ), "cleaning pinched %d faces, against %d" % ( len( c_rep ), len( p_rep ) )

    for f in cleaned:
        for n in f:
            assert 1 <= n <= len( nodes ), "cleaning left a face naming node %d of %d" % ( n, len( nodes ) )


def testThePanelMeshHasNoMalformedFaces():
    """With the passes off, no face has fewer than three nodes or names a node twice."""
    _, faces, broken = _panel_mesh( merge = False, clean = False )
    small, repeated = _malformed( faces )

    assert broken == 0, "the writer could not close %d faces" % broken
    assert len( small ) == 0, "%d faces with fewer than three nodes" % len( small )
    assert len( repeated ) == 0, "%d faces naming a node twice" % len( repeated )


def _edge_health( merge = False, clean = False, thin = False ):
    """The panel mesh's own edge counts, as Prepare Geometry reports them.

    The .vspgeom is written by node, so two edges joining the same pair of nodes -- each holding
    one face -- read as one closed edge there.  Only the mesh can say.
    """
    _wing_over_fuselage()
    vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
    if thin:
        vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_NONE ] )
        vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ vsp.SET_ALL ] )
    else:
        vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_ALL ] )
        vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ vsp.SET_NONE ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "MergeDegenTriRegionsFlag", [ int( merge ) ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "CleanIntersectionsFlag", [ int( clean ) ] )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )
    res = vsp.FindLatestResultsID( "VSPAERO_Geom" )
    return tuple( vsp.GetIntResults( res, k )[0]
                  for k in ( "Num_Edges_One_Face", "Num_Edges_Many_Faces", "Num_Duplicate_Edges" ) )


@pytest.mark.parametrize( "merge, clean", [ ( False, False ), ( True, False ), ( False, True ), ( True, True ) ] )
def testAllThickGeometryMakesAManifoldMesh( merge, clean ):
    """A wing grazing a fuselage, all thick, closes on itself: every edge has exactly two faces.

    Edges with one face or more than two belong where thin surfaces end or meet thick ones;
    here there are none.  The thin set is none: a body in both sets is meshed as its thin
    plates too, and those split its skin where they meet it.  (A control surface grouped for
    VSPAERO still leaves doubled edges along its outline; that is not covered here.)
    """
    one, many, dup = _edge_health( merge, clean )
    assert ( one, many, dup ) == ( 0, 0, 0 ), \
           "one-face edges %d, many-face edges %d, duplicated edges %d" % ( one, many, dup )


@pytest.mark.parametrize( "merge, clean", [ ( False, True ), ( True, True ) ] )
def testThinGeometryIsCleaned( merge, clean ):
    """Thin surfaces end in free edges; cleaning a mesh of them completes and leaves them there."""
    one, many, dup = _edge_health( merge, clean, thin = True )
    assert one > 0 and many >= 0 and dup >= 0


def testPrepareGeometryReportsTheEdgeCounts():
    """The counts are reported for the panel mesh, never the -1 that means there is none."""
    one, many, dup = _edge_health()
    assert one >= 0 and many >= 0 and dup >= 0


#==== Which triangles merge ====#

def _read_levels( path ):
    """Points and every level of a .vspgeom v3, coarse to fine.

    Each level is a dict of faces (0-based point-index loops), tags, and parents (for each face,
    the 1-based index of the face on the next coarser level that holds it; 0 on the coarsest).  Per
    level the file holds: the face count, one loop per face, per face its part, tag and a uw per
    node, per face its index and parent, the wakes, and two blocks of triangulation data.
    """
    tokens = []
    for line in open( path ).read().split( "\n" )[1:]:
        tokens.extend( line.split( "#" )[0].split() )
    it = iter( tokens )
    nxt = lambda: next( it )

    nmesh = int( nxt() )
    heads = [ ( int( nxt() ), int( nxt() ), int( nxt() ) ) for m in range( nmesh ) ]
    points = [ ( float( nxt() ), float( nxt() ), float( nxt() ) ) for k in range( max( h[0] for h in heads ) ) ]

    levels = []
    for m in range( nmesh ):
        nface = int( nxt() )
        faces = []
        for k in range( nface ):
            n = int( nxt() )
            faces.append( [ int( nxt() ) - 1 for q in range( n ) ] )
        tags = []
        for f in faces:
            nxt()
            tags.append( int( nxt() ) )
            for q in range( 2 * len( f ) ):
                nxt()
        parents = []
        for k in range( nface ):
            nxt()
            parents.append( int( nxt() ) )
        for k in range( int( nxt() ) ):
            nwn = abs( int( nxt() ) )
            for q in range( nwn + 1 ):
                nxt()
        ntri = []
        for k in range( nface ):
            nxt()
            ntri.append( int( nxt() ) )
            for q in range( 3 * ntri[-1] ):
                nxt()
        for k in range( nface ):
            for q in range( 3 + 6 * ntri[k] ):
                nxt()
        levels.append( dict( faces = faces, tags = tags, parents = parents ) )
    return points, levels


def _prepare_levels( merge = True, clean = False, ratio = 0.1, nref = 0, cull = False ):
    """Prepare Geometry on the model in hand, and every level of the panel mesh it makes."""
    out = _scratch()
    vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_ALL ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ vsp.SET_NONE ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "MergeDegenTriRegionsFlag", [ int( merge ) ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "CleanIntersectionsFlag", [ int( clean ) ] )
    vsp.SetDoubleAnalysisInput( "VSPAEROComputeGeometry", "CleanIntersectionsLenRatio", [ ratio ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "NRef", [ nref ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "CullFracFlag", [ int( cull ) ] )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )
    return _read_levels( os.path.join( out, "model.vspgeom" ) )


def _faces_at( faces, node ):
    return sum( 1 for f in faces if node in f )


@pytest.mark.parametrize( "tess_w", [ 9, 13, 17, 21 ] )
@pytest.mark.parametrize( "nref", [ 0, 1, 2 ] )
def testABodyNoseAndTailBecomeFourQuarters( tess_w, nref ):
    """Each end's fan of triangles merges into four faces, whatever the parity of n in 4n+1."""
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( pod, "Tess_W", "Shape" ), tess_w )
    vsp.Update()
    points, levels = _prepare_levels( nref = nref )
    fine = levels[-1][ "faces" ]

    assert [ f for f in fine if len( f ) == 3 ] == [], "triangles were left unmerged"

    used = sorted( set( i for f in fine for i in f ), key = lambda i: points[i][0] )
    assert _faces_at( fine, used[0] ) == 4, "the nose is not four quarters"
    assert _faces_at( fine, used[-1] ) == 4, "the tail is not four quarters"


@pytest.mark.parametrize( "cap", [ vsp.FLAT_END_CAP, vsp.ROUND_END_CAP ] )
@pytest.mark.parametrize( "blunt", [ False, True ] )
@pytest.mark.parametrize( "cap_tess", [ 3, 4, 7 ] )
@pytest.mark.parametrize( "nref", [ 0, 1 ] )
def testAWingTipCapsFansBecomeTopAndBottomHalves( cap, blunt, cap_tess, nref ):
    """The tip cap's LE and TE fans each merge into an upper and a lower face.

    The tip's LE and TE points then each sit in four faces: the two cap halves and the upper
    and lower wing surface.
    """
    vsp.VSPRenew()
    wing = vsp.AddGeom( "WING" )
    if blunt:
        xsurf = vsp.GetXSecSurf( wing, 0 )
        for i in range( vsp.GetNumXSec( xsurf ) ):
            xsec = vsp.GetXSec( xsurf, i )
            vsp.SetParmVal( vsp.GetXSecParm( xsec, "TE_Trim_Type" ), 1 )
            vsp.SetParmVal( vsp.GetXSecParm( xsec, "TE_Trim_X" ), 0.1 )
    vsp.SetParmVal( vsp.FindParm( wing, "CapUMaxOption", "EndCap" ), cap )
    vsp.SetParmVal( vsp.FindParm( wing, "CapUMinTess", "EndCap" ), cap_tess )
    vsp.Update()
    points, levels = _prepare_levels( nref = nref )
    fine = levels[-1][ "faces" ]

    assert [ f for f in fine if len( f ) == 3 ] == [], "triangles were left unmerged"

    used = set( i for f in fine for i in f )
    ymax = max( points[i][1] for i in used )
    tip = [ i for i in used if points[i][1] > ymax - 1e-9 ]
    le = min( tip, key = lambda i: points[i][0] )
    te = max( tip, key = lambda i: points[i][0] )
    assert _faces_at( fine, le ) == 4, "the tip LE is in %d faces" % _faces_at( fine, le )
    assert _faces_at( fine, te ) == 4, "the tip TE is in %d faces" % _faces_at( fine, te )


def _every_level_is_well_formed( levels ):
    for k, level in enumerate( levels ):
        small, repeated = _malformed( level[ "faces" ] )
        assert small == [] and repeated == [], "level %d: %d faces under three nodes, %d naming a node twice" % \
               ( k, len( small ), len( repeated ) )
        many = [ e for e, n in _edge_use( level[ "faces" ] ).items() if n > 2 ]
        assert many == [], "level %d: %d edges with more than two faces" % ( k, len( many ) )


def testAFanFarFromTheOriginStillMerges():
    """A fuselage 30000 long merges its tail fan like one 30 long.

    Its tail point sits an ULP, 3.6e-12, from where the last ring of points collapses, so a
    collapsed edge is taken as one within the precision of its coordinates.
    """
    vsp.VSPRenew()
    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( vsp.FindParm( fuse, "Tess_W", "Shape" ), 9 )
    vsp.SetParmVal( vsp.FindParm( fuse, "Scale", "XForm" ), 1000.0 )
    vsp.Update()
    points, levels = _prepare_levels( nref = 2 )

    assert [ f for f in levels[-1][ "faces" ] if len( f ) == 3 ] == [], "triangles were left unmerged"
    _every_level_is_well_formed( levels )


@pytest.mark.parametrize( "cap", [ vsp.ROUND_EXT_END_CAP_LE, vsp.ROUND_EXT_END_CAP_TE ] )
def testAFanWithRowsOfZeroWidthCoarsensCleanly( cap ):
    """An extended round tip cap tessellates some rows to zero width; its fans still coarsen.

    The zero width rows leave faces of two nodes inside the fan, which split it into pieces
    that coarsening, taking a region to be one face, turned into malformed coarse faces.
    """
    vsp.VSPRenew()
    wing = vsp.AddGeom( "WING" )
    vsp.InsertXSec( wing, 1, vsp.XS_FOUR_SERIES )
    vsp.Update()
    vsp.SetParmVal( vsp.FindParm( wing, "Tess_W", "Shape" ), 13 )
    vsp.SetParmVal( vsp.FindParm( wing, "CapUMaxOption", "EndCap" ), cap )
    vsp.SetParmVal( vsp.FindParm( wing, "CapUMinTess", "EndCap" ), 3 )
    vsp.Update()
    points, levels = _prepare_levels( nref = 3 )

    _every_level_is_well_formed( levels )


def testEveryCoarseFaceHoldsOnlyItsChildrensNodes():
    """A coarse face is the union of the finer faces that name it as their parent.

    Cleaning the intersections runs on every level, so this is where it could break.
    """
    _wing_over_fuselage()
    points, levels = _prepare_levels( clean = True, ratio = 0.5, nref = 2 )

    for k, ( coarse, fine ) in enumerate( zip( levels, levels[1:] ) ):
        children = {}
        for f, p in zip( fine[ "faces" ], fine[ "parents" ] ):
            children.setdefault( p - 1, set() ).update( f )
        for c, cf in enumerate( coarse[ "faces" ] ):
            assert c in children, "level %d face %d has no children" % ( k, c )
            assert set( cf ) <= children[c], "level %d face %d holds a node its children do not" % ( k, c )


def _area( points, loop ):
    """Area of a face loop, from its vector area."""
    a = [ 0.0, 0.0, 0.0 ]
    for p, q in zip( loop, loop[1:] + loop[:1] ):
        p, q = points[p], points[q]
        a[0] += p[1] * q[2] - p[2] * q[1]
        a[1] += p[2] * q[0] - p[0] * q[2]
        a[2] += p[0] * q[1] - p[1] * q[0]
    return 0.5 * ( a[0] ** 2 + a[1] ** 2 + a[2] ** 2 ) ** 0.5


def testCleaningKeepsAControlSurfacesCorners():
    """Cleaning may take nodes off a control surface outline, but never one of its corners.

    Taking a straight-run node changes no area; taking a corner cuts it off and moves area from
    one tag to the other.  The surfaces are grouped for VSPAERO, which is what gives them their
    own tags in the panel mesh.
    """
    def mesh( clean ):
        vsp.VSPRenew()
        wing = vsp.AddGeom( "WING" )
        vsp.AddSubSurf( wing, vsp.SS_CONTROL )
        vsp.Update()
        vsp.AutoGroupVSPAEROControlSurfaces()
        vsp.Update()
        points, levels = _prepare_levels( clean = clean, ratio = 0.1 )
        fine = levels[-1]
        areas = {}
        for f, t in zip( fine[ "faces" ], fine[ "tags" ] ):
            areas[t] = areas.get( t, 0.0 ) + _area( points, f )
        return set( points[i] for f in fine[ "faces" ] for i in f ), areas

    cleaned, cleaned_areas = mesh( True )
    plain, plain_areas = mesh( False )

    assert len( plain_areas ) == 4, "expected two wing and two control surface tags, got %r" % plain_areas
    assert len( cleaned ) < len( plain ), "cleaning took no node off the outline"
    for t in plain_areas:
        assert cleaned_areas[t] == pytest.approx( plain_areas[t], rel = 1e-6 ), \
               "tag %d area went from %g to %g" % ( t, plain_areas[t], cleaned_areas[t] )


def _rod_through_a_pod( pod_tess_w, pod_tess_u, x, z, fine_ratio ):
    """A thin rod across a pod, cutting a small hole in each side."""
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( pod, "Tess_W", "Shape" ), pod_tess_w )
    if pod_tess_u:
        vsp.SetParmVal( vsp.FindParm( pod, "Tess_U", "Shape" ), pod_tess_u )
    rod = vsp.AddGeom( "POD" )
    for name, group, value in ( ( "Length", "Design", 2.0 ), ( "FineRatio", "Design", fine_ratio ),
                                ( "Z_Rel_Rotation", "XForm", 90.0 ), ( "X_Rel_Location", "XForm", x ),
                                ( "Y_Rel_Location", "XForm", -1.0 ), ( "Z_Rel_Location", "XForm", z ) ):
        vsp.SetParmVal( vsp.FindParm( rod, name, group ), value )
    vsp.Update()


@pytest.mark.parametrize( "case, nref", [ ( "nose_ring", 2 ), ( "one_panel", 2 ),
                                          ( "one_panel_on_a_diagonal", 0 ), ( "one_panel_on_a_diagonal", 2 ) ] )
def testAHoleNeverEndsUpInsideOneFace( case, nref ):
    """A face that would ring an intersection hole is left in two pieces, at every level.

    nose_ring: the hole sits in the second ring of a pod's nose, which coarsening merges into
    the nose fan.  one_panel: the hole sits inside a single quad of a coarse pod, whose cut
    pieces the polygon mesh would join back up.  On a diagonal, cleaning the hole's short
    edges must not pinch the face beside it either; that model has edges short enough to clean
    only at NRef 0.
    """
    if case == "nose_ring":
        _rod_through_a_pod( 9, 0, 0.08, 0.04, 200.0 )
    elif case == "one_panel":
        _rod_through_a_pod( 5, 5, 2.0, 0.066, 100.0 )
    else:
        _rod_through_a_pod( 5, 5, 2.0, 0.11, 100.0 )
    points, levels = _prepare_levels( clean = True, nref = nref )

    for k, level in enumerate( levels ):
        small, repeated = _malformed( level[ "faces" ] )
        assert small == [] and repeated == [], "level %d: %d faces under three nodes, %d naming a node twice" % \
               ( k, len( small ), len( repeated ) )
        wrong = [ e for e, n in _edge_use( level[ "faces" ] ).items() if n != 2 ]
        assert wrong == [], "level %d: %d edges not shared by exactly two faces" % ( k, len( wrong ) )


def _swept_wing_with_control_surfaces( sweep, tess_w, spans ):
    """A wing swept by sweep degrees with one control surface per ( eta start, eta end ) pair."""
    vsp.VSPRenew()
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( wing, "Sweep", "XSec_1" ), sweep )
    vsp.SetParmVal( vsp.FindParm( wing, "Tess_W", "Shape" ), tess_w )
    vsp.Update()
    for eta0, eta1 in spans:
        ss = vsp.AddSubSurf( wing, vsp.SS_CONTROL )
        vsp.Update()
        vsp.SetParmVal( vsp.FindParm( ss, "EtaFlag", "SS_Control" ), 1 )
        vsp.SetParmVal( vsp.FindParm( ss, "EtaStart", "SS_Control" ), eta0 )
        vsp.SetParmVal( vsp.FindParm( ss, "EtaEnd", "SS_Control" ), eta1 )
        vsp.Update()
    vsp.AutoGroupVSPAEROControlSurfaces()
    vsp.Update()


def _tag_areas( points, faces, tags ):
    areas = {}
    for f, t in zip( faces, tags ):
        areas[t] = areas.get( t, 0.0 ) + _area( points, f )
    return areas


def testCleaningKeepsASweptControlSurfacesCorners():
    """On a 65 degree swept wing a hinge corner turns by only about 28 degrees; it still stays.

    A control surface outline runs straight between its corners, so any real turn on it is a
    corner, however sharp the sweep makes it.
    """
    def mesh( clean ):
        _swept_wing_with_control_surfaces( 65.0, 13, [ ( 0.1, 0.35 ) ] )
        points, levels = _prepare_levels( clean = clean, ratio = 0.1 )
        return _tag_areas( points, levels[-1][ "faces" ], levels[-1][ "tags" ] )

    cleaned = mesh( True )
    plain = mesh( False )
    for t in plain:
        assert cleaned[t] == pytest.approx( plain[t], rel = 1e-4 ), \
               "tag %d area went from %g to %g" % ( t, plain[t], cleaned[t] )


def _pod_through_a_wing_body_junction( x ):
    """A fuselage, a wing on it, and a mirrored pod through where the two meet."""
    vsp.VSPRenew()
    vsp.AddGeom( "FUSELAGE" )
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( wing, "X_Rel_Location", "XForm" ), 2.0 )
    vsp.SetParmVal( vsp.FindParm( wing, "Z_Rel_Location", "XForm" ), 0.3 )
    pod = vsp.AddGeom( "POD" )
    for name, group, value in ( ( "Length", "Design", 3.0 ), ( "FineRatio", "Design", 8.0 ),
                                ( "X_Rel_Location", "XForm", x ), ( "Y_Rel_Location", "XForm", 0.9 ),
                                ( "Z_Rel_Location", "XForm", 0.3 ) ):
        vsp.SetParmVal( vsp.FindParm( pod, name, group ), value )
    vsp.SetParmVal( vsp.FindParm( pod, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()


def _corners( points, faces, tags ):
    """Nodes where faces of three or more tags meet."""
    around = {}
    for f, t in zip( faces, tags ):
        for i in f:
            around.setdefault( points[i], set() ).add( t )
    return set( p for p, ts in around.items() if len( ts ) >= 3 )


def testCleaningKeepsEveryJunction():
    """A node where three surfaces meet is never collapsed, however short the edges around it."""
    def corners( clean ):
        _pod_through_a_wing_body_junction( 1.0 )
        points, levels = _prepare_levels( clean = clean, ratio = 0.5, nref = 1 )
        fine = levels[-1]
        return _corners( points, fine[ "faces" ], fine[ "tags" ] )

    plain = corners( False )
    assert plain, "the model has no junction to keep"
    lost = plain - corners( True )
    assert not lost, "cleaning removed %d junctions, e.g. %r" % ( len( lost ), sorted( lost )[:2] )


def testCleaningKeepsAMirroredModelSymmetric():
    """Cleaning makes the same choices on both halves of a model mirrored about y = 0."""
    _pod_through_a_wing_body_junction( 3.0 )
    points, levels = _prepare_levels( clean = True, ratio = 0.5 )
    used = set( points[i] for f in levels[-1][ "faces" ] for i in f )
    key = lambda p: ( round( p[0], 5 ), round( abs( p[1] ), 5 ), round( p[2], 5 ) )
    upper = set( key( p ) for p in used if p[1] > 1e-7 )
    lower = set( key( p ) for p in used if p[1] < -1e-7 )
    assert upper == lower, "%d nodes have no mirror image" % len( upper ^ lower )


def _folded( points, faces ):
    """Faces facing away from the faces around them."""
    def normal( f ):
        n = [ 0.0, 0.0, 0.0 ]
        for p, q in zip( f, f[1:] + f[:1] ):
            p, q = points[p], points[q]
            n[0] += p[1] * q[2] - p[2] * q[1]
            n[1] += p[2] * q[0] - p[0] * q[2]
            n[2] += p[0] * q[1] - p[1] * q[0]
        m = ( n[0] ** 2 + n[1] ** 2 + n[2] ** 2 ) ** 0.5
        return [ c / m for c in n ]
    normals = [ normal( f ) for f in faces ]
    by_edge = {}
    for i, f in enumerate( faces ):
        for a, b in zip( f, f[1:] + f[:1] ):
            by_edge.setdefault( ( min( a, b ), max( a, b ) ), [] ).append( i )
    folded = 0
    for i, f in enumerate( faces ):
        near = set( j for a, b in zip( f, f[1:] + f[:1] ) for j in by_edge[ ( min( a, b ), max( a, b ) ) ] if j != i )
        s = [ sum( normals[j][c] for j in near ) for c in range( 3 ) ]
        if sum( s[c] * normals[i][c] for c in range( 3 ) ) < 0.0:
            folded += 1
    return folded


def testCleaningFoldsNoPanel():
    """Moving the end of a mesh line along a curve must not turn the face beside it over."""
    def folded( clean ):
        _swept_wing_with_control_surfaces( 65.0, 33, [ ( 0.1, 0.35 ), ( 0.4, 0.62 ), ( 0.66, 0.93 ) ] )
        points, levels = _prepare_levels( clean = clean, ratio = 0.3, nref = 1 )
        return _folded( points, levels[-1][ "faces" ] )

    assert folded( True ) <= folded( False )


def testCullingLeavesTheCoarseLevelsCoarser():
    """Culling thin orphan regions must not stop the coarse levels from coarsening."""
    vsp.VSPRenew()
    vsp.AddGeom( "POD" )
    vsp.Update()
    points, levels = _prepare_levels( nref = 2, cull = True )
    counts = [ len( level[ "faces" ] ) for level in levels ]
    assert counts[0] < counts[1] < counts[2], "faces per level, coarse to fine: %r" % counts


#==== What the analysis is allowed to change ====#

def testPreparingGeometryLeavesTheSettingsAlone():
    """The analysis copies its inputs over the manager's Parms and puts them back afterwards."""
    vsp.VSPRenew()
    _scratch()
    vsp.AddGeom( "FUSELAGE" )
    vsp.Update()

    settings = vsp.FindContainer( "VSPAEROSettings", 0 )
    watched = ( "MergeDegenTriRegions", "CleanIntersections", "CleanIntersectionsLenRatio",
                "FindBodyWakesFlag", "CullFrac", "CullFracFlag", "NRef" )
    before = { n: vsp.GetParmVal( vsp.FindParm( settings, n, "VSPAERO" ) ) for n in watched }

    vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_ALL ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ vsp.SET_NONE ] )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )

    after = { n: vsp.GetParmVal( vsp.FindParm( settings, n, "VSPAERO" ) ) for n in watched }

    assert after == before, "the analysis changed %r" % \
           { n: ( before[n], after[n] ) for n in watched if before[n] != after[n] }


#==== Deleting one Geom ====#

def testDeletingAGeomKeepsItsChildren():
    """DeleteGeom deletes the one Geom named and hands its children up to where it was."""
    vsp.VSPRenew()
    parent = vsp.AddGeom( "POD" )
    child = vsp.AddGeom( "POD", parent )
    grandchild = vsp.AddGeom( "POD", child )
    vsp.Update()

    assert vsp.GetGeomParent( child ) == parent
    assert vsp.GetGeomParent( grandchild ) == child

    vsp.DeleteGeom( parent )
    vsp.Update()

    left = sorted( vsp.FindGeoms() )
    assert left == sorted( [ child, grandchild ] ), \
           "deleting one Geom left %r" % left
    assert vsp.GetGeomParent( child ) in ( "", "NONE" ), \
           "the child was left pointing at a Geom that no longer exists: %r" % vsp.GetGeomParent( child )
    assert vsp.GetGeomParent( grandchild ) == child, "the rest of the tree moved"


#==== The names an analysis declares and the names it reads back ====#

def testTheRefinementCountReachesTheMesher():
    """Asking for n refinements writes n + 1 meshes, coarse to fine."""
    for nref in ( 0, 1, 2 ):
        meshes = _panel_mesh( nref = nref, levels = True )
        assert len( meshes ) == nref + 1, \
               "%d refinements gave %d meshes" % ( nref, len( meshes ) )

        # Each level is finer than the one before it.
        for coarse, fine in zip( meshes, meshes[1:] ):
            assert fine[0] > coarse[0], "a finer level had %d nodes against %d" % ( fine[0], coarse[0] )
            assert fine[1] > coarse[1], "a finer level had %d faces against %d" % ( fine[1], coarse[1] )


def testTheSweepAnalysisDeclaresTheNamesItReadsBack():
    """The spellings VSPAEROSweep declares for the two flags Execute reads back.

    Nothing outside the analysis can see the name Execute looks up, so only the declared side
    can be pinned.
    """
    vsp.VSPRenew()
    names = list( vsp.GetAnalysisInputNames( "VSPAEROSweep" ) )

    for wanted, wrong in ( ( "SCurveFlag", "ScurveFlag" ),
                           ( "CGUseModeFlag", "UseCGModeFlag" ) ):
        assert wanted in names, "VSPAEROSweep does not declare %s" % wanted
        assert wrong not in names, "VSPAEROSweep still declares %s" % wrong
