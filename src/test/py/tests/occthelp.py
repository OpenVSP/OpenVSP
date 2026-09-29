# Reading CAD files back through OpenCASCADE, for the tests that check what OpenVSP writes.
#
# Not a test file itself -- pytest collects test_*.py, and this is imported by those.  It
# needs the OCP bindings (pip install cadquery-ocp); a test that imports it should skip
# when they are missing.

from OCP.STEPControl import STEPControl_Reader
from OCP.IGESControl import IGESControl_Reader
from OCP.IFSelect import IFSelect_RetDone
from OCP.BRepCheck import BRepCheck_Analyzer
from OCP.TopExp import TopExp_Explorer
from OCP.TopAbs import TopAbs_FACE, TopAbs_EDGE, TopAbs_SHELL, TopAbs_SOLID, TopAbs_IN
from OCP.GProp import GProp_GProps
from OCP.BRepGProp import BRepGProp
from OCP.ShapeAnalysis import ShapeAnalysis_ShapeTolerance
from OCP.BRep import BRep_Tool
from OCP.TopoDS import TopoDS
from OCP.Interface import Interface_Static
from OCP.BRepTools import BRepTools
from OCP.BRepGProp import BRepGProp_Face
from OCP.BRepClass import BRepClass_FaceClassifier
from OCP.BRepBndLib import BRepBndLib
from OCP.Bnd import Bnd_Box
from OCP.ShapeAnalysis import ShapeAnalysis_Surface
from OCP.gp import gp_Pnt, gp_Pnt2d, gp_Vec


# Relative accuracy asked of OCCT's area and volume integration.  Its default fixed-order
# rule is not accurate enough on patches much narrower in one parameter than the other.
INTEGRATION_EPS = 1.0e-7


def read( path, curves_2d = False ):
    """The shape in a STEP or IGES file, or None when OCCT cannot read it.

    curves_2d makes an IGES file's faces trimmed by their curves in the surfaces' parameters
    alone, as a reader would that has no use for the model space curves.
    """
    if path.lower().endswith( ( '.stp', '.step' ) ):
        reader = STEPControl_Reader()
    else:
        reader = IGESControl_Reader()

    mode = 0
    if curves_2d:
        mode = -2
    Interface_Static.SetIVal_s( "read.surfacecurve.mode", mode )

    try:
        if reader.ReadFile( path ) != IFSelect_RetDone:
            return None

        reader.TransferRoots()
        shape = reader.OneShape()
    finally:
        Interface_Static.SetIVal_s( "read.surfacecurve.mode", 0 )

    if shape.IsNull():
        return None
    return shape


def _unique( shape, kind ):
    """Each distinct sub-shape of a kind once, with a lookup from hash to the shapes."""
    found = []
    buckets = {}
    ex = TopExp_Explorer( shape, kind )
    while ex.More():
        cur = ex.Current()
        bucket = buckets.setdefault( hash( cur ), [] )
        if not any( cur.IsSame( other ) for other in bucket ):
            bucket.append( cur )
            found.append( cur )
        ex.Next()
    return found, buckets


def signed_volume( path ):
    """The volume the faces of a file enclose, whether or not they are joined into closed shells:
    negative where they face into the body."""
    shape = read( path )
    if shape is None:
        return None

    props = GProp_GProps()
    BRepGProp.VolumeProperties_s( shape, props, INTEGRATION_EPS, False )
    return props.Mass()


def _inside_normal( face ):
    """A point inside a face and the face's outward normal there, or ( None, None )."""
    u0, u1, v0, v1 = BRepTools.UVBounds_s( face )
    props = BRepGProp_Face( face )
    for fu, fv in ( ( 0.5, 0.5 ), ( 0.3, 0.3 ), ( 0.7, 0.7 ), ( 0.3, 0.7 ), ( 0.7, 0.3 ),
                    ( 0.5, 0.25 ), ( 0.5, 0.75 ), ( 0.25, 0.5 ), ( 0.75, 0.5 ) ):
        u = u0 + fu * ( u1 - u0 )
        v = v0 + fv * ( v1 - v0 )
        if BRepClass_FaceClassifier( face, gp_Pnt2d( u, v ), 1.0e-9 ).State() != TopAbs_IN:
            continue
        p = gp_Pnt()
        n = gp_Vec()
        props.Normal( u, v, p, n )
        if n.Magnitude() > 0.0:
            return p, n.Normalized()
    return None, None


def _same_way( fa, fb, scale ):
    """Whether two faces lying on one another face the same way, taken at a point inside the
    first; None where either gives no normal there."""
    p, n = _inside_normal( fa )
    if p is None:
        return None
    uv = ShapeAnalysis_Surface( BRep_Tool.Surface_s( fb ) ).ValueOfUV( p, 1.0e-7 * scale )
    q = gp_Pnt()
    m = gp_Vec()
    BRepGProp_Face( fb ).Normal( uv.X(), uv.Y(), q, m )
    if m.Magnitude() == 0.0 or q.Distance( p ) > 1.0e-4 * scale:
        return None
    return n.Dot( m.Normalized() ) >= 0.0


def facing( path_a, path_b ):
    """How many faces of one file face the other way from the same face in the other file, and
    how many could not be compared.  Faces are paired by area and centroid, and compared at a
    point inside the first."""
    def props( faces ):
        out = []
        for f in faces:
            f = TopoDS.Face( f )
            p = GProp_GProps()
            BRepGProp.SurfaceProperties_s( f, p, INTEGRATION_EPS, False )
            out.append( ( f, p.Mass(), p.CentreOfMass() ) )
        return out

    shape_a = read( path_a )
    faces_a = props( _unique( shape_a, TopAbs_FACE )[0] )
    faces_b = props( _unique( read( path_b ), TopAbs_FACE )[0] )

    box = Bnd_Box()
    BRepBndLib.Add_s( shape_a, box )
    scale = box.SquareExtent() ** 0.5

    opposed = 0
    unknown = 0
    for fa, area, cen in faces_a:
        best = None
        for fb, area_b, cen_b in faces_b:
            if abs( area_b - area ) > 1.0e-3 * area:
                continue
            d = cen.Distance( cen_b )
            if best is None or d < best[0]:
                best = ( d, fb )

        if best is None or best[0] > 1.0e-4 * scale:
            unknown += 1
            continue

        # Taken inside the first face, or inside the second where the first gives no normal
        same = _same_way( fa, best[1], scale )
        if same is None:
            same = _same_way( best[1], fa, scale )
        if same is None and area < 1.0e-6 * scale * scale:
            # A face this small barely turns, so its own normal anywhere inside will do
            p, n = _inside_normal( fa )
            q, m = _inside_normal( best[1] )
            if p is not None and q is not None:
                same = n.Dot( m ) >= 0.0
        if same is None:
            unknown += 1
        elif not same:
            opposed += 1

    return opposed, unknown


def measure( path, topology = True, curves_2d = False ):
    """What OCCT makes of a file.

    faces, shells, solids   counts of distinct sub-shapes
    valid                   BRepCheck_Analyzer on the whole shape
    bad_faces               faces BRepCheck_Analyzer rejects on their own, counted only when
                            the whole shape is rejected
    edge_use                {faces using an edge: number of such edges}, degenerate edges left
                            out, and a seam counted twice for the face it closes
    max_tol                 the largest tolerance OCCT had to give a vertex, edge or face
    area, volume            volume is of the closed shells, None where there are none

    Without topology, edge_use and volume are not measured.  curves_2d reads as read does.
    """
    shape = read( path, curves_2d )
    if shape is None:
        return None

    faces, _ = _unique( shape, TopAbs_FACE )

    edge_use = None
    volume = None
    solids = len( _unique( shape, TopAbs_SOLID )[0] )

    if topology:
        edges, edge_buckets = _unique( shape, TopAbs_EDGE )

        use = {}
        for face in faces:
            face_edges, _ = _unique( face, TopAbs_EDGE )
            for edge in face_edges:
                n = 1
                if BRep_Tool.IsClosed_s( TopoDS.Edge( edge ), TopoDS.Face( face ) ):
                    n = 2
                for other in edge_buckets.get( hash( edge ), [] ):
                    if edge.IsSame( other ):
                        use[ id( other ) ] = use.get( id( other ), 0 ) + n
                        break

        edge_use = {}
        for edge in edges:
            if BRep_Tool.Degenerated_s( TopoDS.Edge( edge ) ):
                continue
            n = use.get( id( edge ), 0 )
            edge_use[ n ] = edge_use.get( n, 0 ) + 1

        props = GProp_GProps()
        BRepGProp.VolumeProperties_s( shape, props, INTEGRATION_EPS, True )
        if props.Mass() != 0.0:
            volume = props.Mass()

    props = GProp_GProps()
    BRepGProp.SurfaceProperties_s( shape, props, INTEGRATION_EPS, False )
    area = props.Mass()

    valid = BRepCheck_Analyzer( shape ).IsValid()
    bad_faces = 0
    if not valid:
        bad_faces = sum( 1 for f in faces if not BRepCheck_Analyzer( f ).IsValid() )

    return { 'faces': len( faces ),
             'shells': len( _unique( shape, TopAbs_SHELL )[0] ),
             'solids': solids,
             'valid': valid,
             'bad_faces': bad_faces,
             'edge_use': edge_use,
             'max_tol': ShapeAnalysis_ShapeTolerance().Tolerance( shape, 1 ),
             'area': area,
             'volume': volume }
