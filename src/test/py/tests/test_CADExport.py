# Every CAD export path, read back through OpenCASCADE.
#
# OpenVSP writes STEP and IGES four ways: untrimmed surfaces (File > Export), untrimmed
# structure parts, trimmed surfaces from Surface Intersection, and trimmed structure parts
# from the FEA mesher.  What is checked is what a CAD tool would find: that the file reads,
# that OCCT's own checker passes it, that a trimmed body is closed -- every edge shared by
# exactly two faces -- and that its area and volume are the model's.  A model whose
# components never meet is measured against its own untrimmed surfaces, which it must match
# exactly; one whose components intersect, against a finely tessellated CompGeom.  Options
# are exercised in combination: splitting surfaces, demoting to cubic, the feature-based
# split and join of the mesh pipeline, units, labels, and the STEP representation.
#
# Needs the OCP bindings (pip install cadquery-ocp); without them the module skips.  Every
# model is built here.  --cad-survey-dir DIR adds a survey of models read from DIR, one folder
# of .vsp3 files per model; there is no survey without it.  --cad-slow adds the survey's
# structures, which take minutes each to mesh.  The files are written to a temporary
# directory, removed at the end unless --cad-keep is given.

import atexit
import functools
import glob
import os
import re
import shutil
import tempfile

import numpy
import pytest

pytest.importorskip( "OCP" )

from OCP.BRepAdaptor import BRepAdaptor_Surface
from OCP.GeomAbs import GeomAbs_BSplineSurface
from OCP.TopAbs import TopAbs_FACE
from OCP.TopExp import TopExp_Explorer
from OCP.TopoDS import TopoDS

import openvsp as vsp

import cadoptions
import occthelp
import testhelp


OUT = tempfile.mkdtemp( prefix = "vsp_cad_" )
if not cadoptions.keep:
    atexit.register( shutil.rmtree, OUT, True )

SURVEY_DIR = cadoptions.survey_dir

# CompGeom reference tessellation, as a multiple of each Geom's own.  At 4x the trimmed
# models here are within 0.2% in area and 0.6% in volume of it.
REF_REFINE = 4

AREA_TOL = 0.005
VOLUME_TOL = 0.01

# A model whose components never meet is trimmed only by its own borders, which are exact
EXACT_TOL = 1.0e-6

# The STEP tolerance, which trimmed curves are also built to, and the largest tolerance OCCT
# may need to give a trimmed file, as a multiple of it.  A file is written in model units,
# declared as the units asked for.
STEP_TOL = 1.0e-6
MAX_TOL_FACTOR = 2.0

# OCCT reads every file into millimetres.  A file declared in feet -- Surface Intersection's
# default -- is scaled by this on the way in, and so are the gaps OCCT has to tolerate.
MM_PER = { vsp.LEN_MM: 1.0, vsp.LEN_FT: 304.8 }


#==== Models built here ====#

def _pods():
    vsp.AddGeom( "POD" )
    b = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( b, "Z_Rel_Rotation", "XForm" ), 90 )
    vsp.SetParmVal( vsp.FindParm( b, "X_Rel_Location", "XForm" ), 5 )


def _disjoint_pods():
    vsp.AddGeom( "POD" )
    b = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( b, "Y_Rel_Location", "XForm" ), 5 )


def _negative_pod():
    vsp.AddGeom( "POD" )
    b = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( b, "X_Rel_Location", "XForm" ), 8 )
    vsp.SetParmVal( vsp.FindParm( b, "Z_Rel_Rotation", "XForm" ), 90 )
    vsp.SetParmVal( vsp.FindParm( b, "Negative_Volume_Flag", "Negative_Volume_Props" ), 1 )


def _wing_pod():
    vsp.AddGeom( "WING" )
    p = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( p, "X_Rel_Location", "XForm" ), -2 )


def _wing_fuselage():
    vsp.AddGeom( "FUSELAGE" )
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( w, "X_Rel_Location", "XForm" ), 10 )


def _line_wing():
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( w, "Sym_Planar_Flag", "Sym" ), 0 )
    vsp.AddSubSurf( w, vsp.SS_LINE )


def _sym_wing():
    """The two halves' root caps coincide on the centerline."""
    vsp.AddGeom( "WING" )


def _sym_wing_offset():
    """The same, moved off the centerline so the root caps stand apart."""
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( w, "Y_Rel_Location", "XForm" ), 0.05 )


def _round_cap_wing():
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( w, "Y_Rel_Location", "XForm" ), 0.05 )
    vsp.SetParmVal( vsp.FindParm( w, "CapUMaxOption", "EndCap" ), vsp.ROUND_END_CAP )


# Stacks skinned to each degree.  Across a span, between the after side of one section and
# the before side of the next, every condition set raises the degree in u by one: with none
# the span is linear between the sections, and an angle and a curvature on each side make it
# quintic.  Sides of one section skinned differently raise the degree in w.
SIDES = ( "Top", "Right", "Bottom", "Left" )

# Degree in u -> ( angle, curvature ) set on each section's after side, then its before side
U_CONDITIONS = { 1: ( ( 0, 0 ), ( 0, 0 ) ), 2: ( ( 1, 0 ), ( 0, 0 ) ), 3: ( ( 1, 0 ), ( 1, 0 ) ),
                 4: ( ( 1, 1 ), ( 1, 0 ) ), 5: ( ( 1, 1 ), ( 1, 1 ) ) }


def _xsec_parm( xsec, name, val ):
    vsp.SetParmVal( vsp.GetXSecParm( xsec, name ), val )


def _stack_xsecs():
    g = vsp.AddGeom( "STACK" )
    vsp.Update()
    xs = vsp.GetXSecSurf( g, 0 )
    return [ vsp.GetXSec( xs, i ) for i in range( vsp.GetNumXSec( xs ) ) ]


def _stack_u( degree ):
    """A default Stack whose every span is skinned to the given degree in u."""
    def build():
        xsecs = _stack_xsecs()
        for xsec in xsecs:
            for side in SIDES:
                for eq in ( "LRAngleEq", "LRCurveEq", "LRSlewEq", "LRStrengthEq" ):
                    _xsec_parm( xsec, side + eq, 0 )
                for lr, ( angle, curve ) in zip( ( "R", "L" ), U_CONDITIONS[ degree ] ):
                    for what in ( "AngleSet", "SlewSet", "StrengthSet" ):
                        _xsec_parm( xsec, side + lr + what, angle )
                    _xsec_parm( xsec, side + lr + "CurveSet", curve )
        return xsecs
    return build


def _stack_c2():
    """A default Stack with C2 continuity at its three inner sections, which are two apart and
    the middle one wider.  Degree 4 in u."""
    xsecs = _stack_xsecs()
    for i in ( 1, 2, 3 ):
        for side in SIDES:
            _xsec_parm( xsecs[i], "Continuity" + side, 2 )
            _xsec_parm( xsecs[i], side + "LRSlewEq", 1 )
            _xsec_parm( xsecs[i], side + "LRStrengthEq", 1 )
    for i in ( 1, 3, 4 ):
        _xsec_parm( xsecs[i], "XDelta", 2 )
    _xsec_parm( xsecs[2], "Ellipse_Width", 6.36734693877551 )
    _xsec_parm( xsecs[2], "Ellipse_Height", 6.071428571428571 )


def _free_sides( xsec ):
    """Leave a section's left and right sides unset while its top and bottom keep theirs."""
    _xsec_parm( xsec, "AllSym", 0 )
    for side in ( "Right", "Left" ):
        _xsec_parm( xsec, side + "LRAngleEq", 0 )
        for lr in ( "L", "R" ):
            for what in ( "AngleSet", "SlewSet", "StrengthSet", "CurveSet" ):
                _xsec_parm( xsec, side + lr + what, 0 )


def _stack_blend():
    """A default Stack, its middle section wider and its sides skinned differently.  Degree 6
    in w."""
    xsec = _stack_xsecs()[2]
    _xsec_parm( xsec, "Ellipse_Width", 9.922448979591836 )
    _xsec_parm( xsec, "Ellipse_Height", 6.071428571428571 )
    _free_sides( xsec )


def _stack_both():
    """The quintic Stack with one section's sides skinned differently.  Degree 5 in u and 6 in
    w."""
    xsecs = _stack_u( 5 )()
    _free_sides( xsecs[2] )


def _wing_structure():
    """A two-segment wing, off the centerline, with a spar and two ribs."""
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( w, "Y_Rel_Location", "XForm" ), 0.05 )
    vsp.InsertXSec( w, 1, vsp.XS_FOUR_SERIES )
    vsp.Update()
    vsp.SetParmVal( vsp.FindParm( w, "Sweep", "XSec_2" ), 30 )
    vsp.Update()
    vsp.AddFeaStruct( w )
    spar = vsp.AddFeaPart( w, 0, vsp.FEA_SPAR )
    vsp.SetParmVal( vsp.FindParm( spar, "RelCenterLocation", "FeaPart" ), 0.4 )
    for loc in ( 0.35, 0.7 ):
        rib = vsp.AddFeaPart( w, 0, vsp.FEA_RIB )
        vsp.SetParmVal( vsp.FindParm( rib, "RelCenterLocation", "FeaPart" ), loc )


# name -> ( builder, whether the untrimmed surfaces' area is CompGeom's theoretical area,
#           whether the components never meet )
# CompGeom leaves out root caps that coincide on the centerline, so for those the exported
# area is larger by the caps.
BUILT = {
    "pods": ( _pods, True, False ),
    "disjoint_pods": ( _disjoint_pods, True, True ),
    "negative_pod": ( _negative_pod, True, False ),
    "wing_pod": ( _wing_pod, False, False ),
    "wing_fuselage": ( _wing_fuselage, False, False ),
    "line_wing": ( _line_wing, True, True ),
    "sym_wing": ( _sym_wing, False, False ),
    "sym_wing_offset": ( _sym_wing_offset, True, True ),
    "round_cap_wing": ( _round_cap_wing, True, True ),
    "stack_u1": ( _stack_u( 1 ), True, True ),
    "stack_u2": ( _stack_u( 2 ), True, True ),
    "stack_u3": ( _stack_u( 3 ), True, True ),
    "stack_u4": ( _stack_u( 4 ), True, True ),
    "stack_u5": ( _stack_u( 5 ), True, True ),
    "stack_c2": ( _stack_c2, True, True ),
    "stack_blend": ( _stack_blend, True, True ),
    "stack_both": ( _stack_both, True, True ),
    "wing_structure": ( _wing_structure, True, True ),
}


def _survey():
    found = {}
    if not SURVEY_DIR:
        return found
    for path in sorted( glob.glob( os.path.join( SURVEY_DIR, "*", "*.vsp3" ) ) ):
        name = "survey_" + os.path.basename( os.path.dirname( path ) )
        found[ name ] = path
    return found


SURVEY = _survey()

MODELS = sorted( BUILT ) + sorted( SURVEY )


def load( model ):
    vsp.VSPRenew()
    testhelp.drop_errors()
    if model in BUILT:
        BUILT[ model ][0]()
    else:
        vsp.ReadVSPFile( SURVEY[ model ] )
    vsp.Update()


def _separate( model ):
    return model in BUILT and BUILT[ model ][2]


#==== What the model says about itself ====#

def _surface_types():
    types = set()
    for g in vsp.FindGeoms():
        for s in range( vsp.GetNumMainSurfs( g ) ):
            types.add( vsp.GetGeomVSPSurfCfdType( g, s ) )
    return types


def _count_surfaces( cfd_type ):
    """How many surfaces of a type the model writes, symmetric copies included."""
    n = 0
    for g in vsp.FindGeoms():
        nmain = vsp.GetNumMainSurfs( g )
        if nmain == 0:
            continue
        copies = vsp.GetTotalNumSurfs( g ) // nmain
        for s in range( nmain ):
            if vsp.GetGeomVSPSurfCfdType( g, s ) == cfd_type:
                n += copies
    return n


@functools.lru_cache( maxsize = None )
def facts( model ):
    """CompGeom's areas and volume, and what kinds of surface the model has."""
    load( model )
    types = _surface_types()

    for g in vsp.FindGeoms():
        for p in vsp.GetGeomParmIDs( g ):
            if vsp.GetParmName( p ) in ( "Tess_W", "Tess_U", "SectTess_U" ):
                vsp.SetParmVal( p, vsp.GetParmVal( p ) * REF_REFINE )
    vsp.Update()

    vsp.SetAnalysisInputDefaults( "CompGeom" )
    vsp.SetIntAnalysisInput( "CompGeom", "Set", [ vsp.SET_ALL ] )
    vsp.SetIntAnalysisInput( "CompGeom", "WriteCSVFlag", [ 0 ] )
    vsp.SetIntAnalysisInput( "CompGeom", "WriteTXTFlag", [ 0 ] )
    rid = vsp.ExecAnalysis( "CompGeom" )

    def total( name ):
        return vsp.GetDoubleResults( rid, name )[0]

    return { "theo_area": total( "Total_Theo_Area" ),
             # Every component's volume counted whole, a negative one as a body of its own
             "theo_vol_whole": sum( abs( v ) for v in vsp.GetDoubleResults( rid, "Theo_Vol" ) ),
             "wet_area": total( "Total_Wet_Area" ),
             "wet_vol": total( "Total_Wet_Vol" ),
             # Only a model of closed, solid bodies is a closed shell once trimmed.
             "solid": types <= { vsp.CFD_NORMAL, vsp.CFD_NEGATIVE },
             "normal": types == { vsp.CFD_NORMAL },
             "transparent": _count_surfaces( vsp.CFD_TRANSPARENT ) }


#==== Reading the files ====#

def _text( path ):
    with open( path, errors = "replace" ) as f:
        return f.read()


def _count( path, entity ):
    """How many of a STEP entity a file holds."""
    return len( re.findall( r"=\s*%s\s*\(" % entity, _text( path ) ) )


def _iges_count( path, entity_type ):
    """How many entities of a type an IGES file holds, from its directory entries."""
    n = 0
    with open( path, errors = "replace" ) as f:
        for line in f:
            if len( line ) > 72 and line[72] == "D" and int( line[73:80] ) % 2 == 1 and int( line[0:8] ) == entity_type:
                n += 1
    return n


def _iges_params( path, entity_type ):
    """The parameters of each entity of a type in an IGES file, as strings."""
    params = []
    with open( path, errors = "replace" ) as f:
        for line in f:
            if len( line ) > 72 and line[72] == "P" and line.startswith( "%d," % entity_type ):
                params.append( line[:64].split( ";" )[0].split( "," )[1:] )
    return params


def _iges_records( path ):
    """Each entity of an IGES file, by directory entry: its type and its parameters as strings."""
    types = {}
    params = {}
    with open( path, errors = "replace" ) as f:
        lines = f.read().splitlines()
    dir_lines = [ l for l in lines if len( l ) > 72 and l[72] == "D" ]
    for i in range( 0, len( dir_lines ), 2 ):
        types[ int( dir_lines[i][73:80] ) ] = int( dir_lines[i][0:8] )
    for l in lines:
        if len( l ) > 72 and l[72] == "P":
            params.setdefault( int( l[64:72] ), [] ).append( l[:64] )
    return { de: ( types[de], "".join( params[de] ).split( ";" )[0].split( "," ) ) for de in params if de in types }


def _iges_untrimmed_surfaces( path ):
    """The rational B-spline surfaces (128) of an IGES file that no trimmed surface (144) is
    built on, left whole and untrimmed."""
    recs = _iges_records( path )
    surfs = set( de for de, ( t, p ) in recs.items() if t == 128 )
    trimmed = set( int( p[1] ) for de, ( t, p ) in recs.items() if t == 144 )
    return sorted( surfs - trimmed )


def _num( field ):
    return float( field.replace( "D", "E" ) )


def _iges_flag_errors( path ):
    """The curves and surfaces of an IGES file whose planar or closed flags their control
    points contradict."""
    wrong = []
    for de, ( etype, p ) in _iges_records( path ).items():
        if etype == 126:
            k, m, planar, closed = int( p[1] ), int( p[2] ), int( p[3] ), int( p[4] )
            n = k + 1
            first = 7 + ( k + m + 2 ) + n
            cp = numpy.array( [ _num( x ) for x in p[ first:first + 3 * n ] ] ).reshape( n, 3 )
            size = numpy.ptp( cp, axis = 0 ).max()
            # Planar, not a line, off its plane by less than 1e-10 of its size; not planar, off it
            # by more than 1e-8; between the two, either is right
            sv = numpy.linalg.svd( cp - cp.mean( 0 ), compute_uv = False )
            is_line = n < 3 or sv[1] <= 1.0e-9 * size
            if planar and ( is_line or sv[2] > 1.0e-8 * size ):
                wrong.append( ( de, "126 planar 1" ) )
            if not planar and not is_line and sv[2] < 1.0e-10 * size:
                wrong.append( ( de, "126 planar 0" ) )
            if planar:
                normal = numpy.array( [ _num( x ) for x in p[ first + 3 * n + 2:first + 3 * n + 5 ] ] )
                if numpy.abs( ( cp - cp[0] ) @ normal ).max() > 1.0e-8 * size:
                    wrong.append( ( de, "126 normal" ) )
            is_closed = size > 0.0 and numpy.abs( cp[0] - cp[-1] ).max() <= 1.0e-9 * size
            if bool( closed ) != is_closed:
                wrong.append( ( de, "126 closed %d" % closed ) )
        elif etype == 128:
            k1, k2, m1, m2 = [ int( x ) for x in p[1:5] ]
            closed_u, closed_v = int( p[5] ), int( p[6] )
            n1, n2 = k1 + 1, k2 + 1
            first = 10 + ( k1 + m1 + 2 ) + ( k2 + m2 + 2 ) + n1 * n2
            cp = numpy.array( [ _num( x ) for x in p[ first:first + 3 * n1 * n2 ] ] ).reshape( n2, n1, 3 )
            tol = 1.0e-9 * numpy.ptp( cp.reshape( -1, 3 ), axis = 0 ).max()
            if bool( closed_u ) != ( numpy.abs( cp[:, 0] - cp[:, -1] ).max() <= tol ):
                wrong.append( ( de, "128 closed in u %d" % closed_u ) )
            if bool( closed_v ) != ( numpy.abs( cp[0, :] - cp[-1, :] ).max() <= tol ):
                wrong.append( ( de, "128 closed in v %d" % closed_v ) )
    return wrong


def _surface_labels( path ):
    return re.findall( r"B_SPLINE_SURFACE_WITH_KNOTS\s*\(\s*'([^']*)'", _text( path ) )


def _geometry( path ):
    """A STEP file's data with every string taken out: its geometry and topology alone."""
    text = _text( path )
    text = text[ text.index( "DATA;" ): ]
    return re.sub( r"'[^']*'", "''", text )


def _in_model_units( m, unit ):
    if m is None:
        return None
    k = MM_PER[ unit ]
    m[ "area" ] /= k ** 2
    if m[ "volume" ] is not None:
        m[ "volume" ] /= k ** 3
    m[ "max_tol" ] /= k
    return m


#==== The exports ====#

def _isect_parm( name, val ):
    c = vsp.FindContainer( "SurfaceIntersectSettings", 0 )
    vsp.SetParmVal( vsp.FindParm( c, name, "Global" ), val )


def trimmed( model, split, demote, unit = vsp.LEN_MM, rep = vsp.STEP_BREP ):
    """Surface Intersection's STEP and IGES, measured, in model units, with what it put on
    the error stack and the files' paths."""
    return _trimmed( model, split, demote, unit, rep )


@functools.lru_cache( maxsize = None )
def _trimmed( model, split, demote, unit, rep ):
    load( model )
    _isect_parm( "SplitJoinSurfsFlag", split )
    _isect_parm( "DemoteSurfsCubicFlag", demote )
    c = vsp.FindContainer( "SurfaceIntersectSettings", 0 )
    vsp.SetParmVal( vsp.FindParm( c, "HalfMesh", "FarField" ), 0 )

    tag = "%s_trim_s%d_c%d_u%d_r%d" % ( model, split, demote, unit, rep )
    stp = os.path.join( OUT, tag + ".stp" )
    igs = os.path.join( OUT, tag + ".igs" )

    an = "SurfaceIntersection"
    vsp.SetAnalysisInputDefaults( an )
    vsp.SetIntAnalysisInput( an, "SelectedSetIndex", [ vsp.SET_ALL ] )
    vsp.SetIntAnalysisInput( an, "CADLenUnit", [ unit ] )
    vsp.SetIntAnalysisInput( an, "STEPRepresentation", [ rep ] )
    vsp.SetDoubleAnalysisInput( an, "STEPTol", [ STEP_TOL ] )
    vsp.SetIntAnalysisInput( an, "STEPFileFlag", [ 1 ] )
    vsp.SetStringAnalysisInput( an, "STEPFileName", [ stp ] )
    vsp.SetIntAnalysisInput( an, "IGESFileFlag", [ 1 ] )
    vsp.SetStringAnalysisInput( an, "IGESFileName", [ igs ] )
    vsp.ExecAnalysis( an )
    errors = testhelp.pop_errors()

    # Demoting a model already cubic changes nothing; the same geometry reads the same
    if demote:
        base = trimmed( model, split, 0, unit, rep )
        if _geometry( stp ) == _geometry( base[ "stp_path" ] ):
            return dict( base, errors = errors, stp_path = stp, igs_path = igs,
                         igs = _in_model_units( occthelp.measure( igs, topology = False ), unit ) )

    return { "stp": _in_model_units( occthelp.measure( stp ), unit ),
             "igs": _in_model_units( occthelp.measure( igs, topology = False ), unit ),
             "errors": errors, "stp_path": stp, "igs_path": igs }


def _vehicle_parms( group, settings ):
    veh = vsp.GetVehicleID()
    for name, val in settings:
        vsp.SetParmVal( vsp.FindParm( veh, name, group ), val )
    vsp.Update()


def untrimmed( model, split, cubic, option = None ):
    """File > Export's STEP and IGES, measured, in model units.

    option is one of None, "trimte", "mergelete", "mergepoints" (STEP only) or "ft".
    """
    return _untrimmed( model, split, cubic, option )


@functools.lru_cache( maxsize = None )
def _untrimmed( model, split, cubic, option ):
    load( model )
    unit = vsp.LEN_MM
    if option == "ft":
        unit = vsp.LEN_FT

    common = [ ( "LenUnit", unit ), ( "SplitSurfs", split ), ( "ToCubic", cubic ),
               ( "TrimTE", int( option == "trimte" ) ), ( "MergeLETE", int( option == "mergelete" ) ) ]
    _vehicle_parms( "STEPSettings", common + [ ( "MergePoints", int( option == "mergepoints" ) ) ] )
    _vehicle_parms( "IGESSettings", common )

    tag = "%s_untrim_s%d_c%d_%s" % ( model, split, cubic, option )
    stp = os.path.join( OUT, tag + ".stp" )
    igs = os.path.join( OUT, tag + ".igs" )
    vsp.ExportFile( stp, vsp.SET_ALL, vsp.EXPORT_STEP )
    vsp.ExportFile( igs, vsp.SET_ALL, vsp.EXPORT_IGES )
    errors = testhelp.pop_errors()

    return { "stp": _in_model_units( occthelp.measure( stp, topology = False ), unit ),
             "igs": _in_model_units( occthelp.measure( igs, topology = False ), unit ),
             "errors": errors, "stp_path": stp, "igs_path": igs }


def _structure_models():
    """The models with a structure.  sbw's takes minutes to mesh, so it runs only on request."""
    found = [ "wing_structure" ]
    if cadoptions.slow and "survey_sbw" in SURVEY:
        found.append( "survey_sbw" )
    return found


@functools.lru_cache( maxsize = None )
def structure_untrimmed( model, split, cubic ):
    """The first structure's parts, untrimmed, as STEP and IGES, measured."""
    load( model )
    veh = vsp.GetVehicleID()
    vsp.SetParmVal( vsp.FindParm( veh, "StructUnit", "FeaStructure" ), vsp.MPA_UNIT )
    for group in ( "STEPSettings", "IGESSettings" ):
        _vehicle_parms( group, [ ( "StructureExportIndex", 0 ), ( "StructureSplitSurfs", split ),
                                 ( "StructureToCubic", cubic ) ] )

    tag = "%s_struct_s%d_c%d" % ( model, split, cubic )
    stp = os.path.join( OUT, tag + ".stp" )
    igs = os.path.join( OUT, tag + ".igs" )
    vsp.ExportFile( stp, vsp.SET_ALL, vsp.EXPORT_STEP_STRUCTURE )
    vsp.ExportFile( igs, vsp.SET_ALL, vsp.EXPORT_IGES_STRUCTURE )
    errors = testhelp.pop_errors()

    return { "stp": occthelp.measure( stp, topology = False ), "igs": occthelp.measure( igs, topology = False ),
             "errors": errors }


def _model_size():
    """The diagonal of the box around every Geom."""
    lo = [ 1.0e300 ] * 3
    hi = [ -1.0e300 ] * 3
    for g in vsp.FindGeoms():
        a = vsp.GetGeomBBoxMin( g )
        b = vsp.GetGeomBBoxMax( g )
        for k in range( 3 ):
            lo[k] = min( lo[k], a[k] )
            hi[k] = max( hi[k], b[k] )
    return sum( ( hi[k] - lo[k] ) ** 2 for k in range( 3 ) ) ** 0.5


def _first_structure():
    """The first structure in the model: its Geom, its index on that Geom, and its ID."""
    sid = vsp.GetFeaStructIDVec()[0]
    return vsp.GetFeaStructParentGeomID( sid ), vsp.GetFeaStructIndex( sid ), sid


@functools.lru_cache( maxsize = None )
def fea_trimmed( model ):
    """The FEA mesher's trimmed STEP and IGES of the first structure, measured, in model units."""
    load( model )
    geom, index, sid = _first_structure()

    # The CAD files are made from the intersection alone, so the mesh can be as coarse as the
    # mesher allows: edges a twentieth of the model's size
    size = _model_size()
    vsp.SetFeaMeshVal( geom, index, vsp.CFD_MAX_EDGE_LEN, size / 20.0 )
    vsp.SetFeaMeshVal( geom, index, vsp.CFD_MIN_EDGE_LEN, size / 200.0 )

    stp = os.path.join( OUT, "%s_fea.stp" % model )
    igs = os.path.join( OUT, "%s_fea.igs" % model )

    # The files are declared in the structures' analysis units
    veh = vsp.GetVehicleID()
    vsp.SetParmVal( vsp.FindParm( veh, "StructUnit", "FeaStructure" ), vsp.MPA_UNIT )

    # One mesh, both files
    vsp.SetFeaMeshStructIndex( 0 )
    vsp.SetAnalysisInputDefaults( "FeaMeshAnalysis" )
    vsp.ExecAnalysis( "FeaMeshAnalysis" )

    an = "FeaMeshExport"
    vsp.SetAnalysisInputDefaults( an )
    for name in vsp.GetAnalysisInputNames( an ):
        if name.endswith( "FileFlag" ):
            vsp.SetIntAnalysisInput( an, name, [ 0 ] )
    vsp.SetDoubleAnalysisInput( an, "STEPTol", [ STEP_TOL ] )
    vsp.SetIntAnalysisInput( an, "STEPFileFlag", [ 1 ] )
    vsp.SetStringAnalysisInput( an, "STEPFileName", [ stp ] )
    vsp.SetIntAnalysisInput( an, "IGESFileFlag", [ 1 ] )
    vsp.SetStringAnalysisInput( an, "IGESFileName", [ igs ] )
    vsp.ExecAnalysis( an )
    errors = testhelp.pop_errors()

    return { "stp": occthelp.measure( stp ), "igs": occthelp.measure( igs, topology = False ),
             "errors": errors, "stp_path": stp, "igs_path": igs }


#==== Checks ====#

def _rel( a, b ):
    return abs( a - b ) / abs( b )


# Failures measured and traced to something other than the CAD writers, by the check they
# fail, as { check: reason }.  A known failure that stops failing fails the test, so it is
# taken off the list.  None of the models built here has one.
def _known_trimmed( model, split ):
    return {}


class Checks:
    """Every check a test makes, gathered before any fails, so a known failure does not hide
    another behind it."""

    def __init__( self, known ):
        self.known = dict( known )
        self.failed = []
        self.expected = []

    def check( self, name, ok, msg ):
        reason = self.known.pop( name, None )
        if ok and reason:
            self.failed.append( "%s passes now; take it off the known failures: %s" % ( name, reason ) )
        elif not ok and reason:
            self.expected.append( "%s (%s): %s" % ( name, msg, reason ) )
        elif not ok:
            self.failed.append( "%s: %s" % ( name, msg ) )

    def done( self ):
        # A known failure no check looked at is a misnamed or stale entry
        for name, reason in self.known.items():
            self.failed.append( "known failure %s was never checked: %s" % ( name, reason ) )
        if self.failed:
            pytest.fail( "\n".join( self.failed ) )
        if self.expected:
            pytest.xfail( "\n".join( self.expected ) )


def _closed( m ):
    """Every edge of a face is shared by exactly two faces.

    Edges no face uses are the loose curves written for subsurface lines.
    """
    return set( m[ "edge_use" ] ) - { 0 } == { 2 }


def _same_facing( r ):
    """Each IGES face faces the way the STEP face it lies on does."""
    opposed, unknown = occthelp.facing( r[ "igs_path" ], r[ "stp_path" ] )
    assert opposed == 0 and unknown == 0, "%d IGES faces face the other way, %d not compared" % ( opposed, unknown )


TRIM_OPTS = [ ( 1, 0, vsp.LEN_MM ), ( 0, 0, vsp.LEN_MM ), ( 1, 1, vsp.LEN_MM ), ( 0, 1, vsp.LEN_MM ),
              ( 1, 0, vsp.LEN_FT ) ]
TRIM_IDS = [ "split%d_demote%d_%s" % ( o[0], o[1], { vsp.LEN_MM: "mm", vsp.LEN_FT: "ft" }[ o[2] ] ) for o in TRIM_OPTS ]


@pytest.mark.parametrize( "split, demote, unit", TRIM_OPTS, ids = TRIM_IDS )
@pytest.mark.parametrize( "model", MODELS )
def test_TrimmedSTEP( model, split, demote, unit ):
    """Surface Intersection's STEP reads as valid closed bodies with the model's size."""
    r = trimmed( model, split, demote, unit )
    stp = r[ "stp" ]
    ref = facts( model )
    c = Checks( { k: v for k, v in _known_trimmed( model, split ).items() if not k.startswith( "iges_" ) } )

    assert r[ "errors" ] == []
    assert stp is not None, "no readable STEP file"
    assert stp[ "faces" ] > 0

    c.check( "valid", stp[ "valid" ], "OCCT rejects %d faces" % stp[ "bad_faces" ] )
    c.check( "max_tol", stp[ "max_tol" ] <= MAX_TOL_FACTOR * STEP_TOL, "OCCT needs a tolerance of %g" % stp[ "max_tol" ] )
    c.check( "edge_use", max( stp[ "edge_use" ] ) <= 2, "an edge bounds more than two faces: %s" % stp[ "edge_use" ] )

    if ref[ "solid" ]:
        # A closed body is a solid, and only what bounds none is left as a shell
        closed = _closed( stp ) and _count( r[ "stp_path" ], "OPEN_SHELL" ) == 0
        c.check( "closed", closed, "faces per edge %s, %d solids of %d shells" % ( stp[ "edge_use" ], stp[ "solids" ], stp[ "shells" ] ) )

        if _separate( model ):
            # Demoting to cubic approximates the surfaces
            tol = EXACT_TOL
            if demote:
                tol = 1.0e-4
            base = untrimmed( model, 1, 0 )[ "stp" ]
            c.check( "area", _rel( stp[ "area" ], base[ "area" ] ) < tol,
                     "area %.12g, untrimmed %.12g" % ( stp[ "area" ], base[ "area" ] ) )
        else:
            c.check( "area", _rel( stp[ "area" ], ref[ "wet_area" ] ) < AREA_TOL,
                     "area %g, CompGeom %g" % ( stp[ "area" ], ref[ "wet_area" ] ) )

        # Only closed shells have a volume, so an open body leaves its out
        if closed:
            c.check( "volume", _rel( stp[ "volume" ], ref[ "wet_vol" ] ) < VOLUME_TOL,
                     "volume %g, CompGeom %g" % ( stp[ "volume" ], ref[ "wet_vol" ] ) )
    c.done()


@pytest.mark.parametrize( "split, demote, unit", TRIM_OPTS, ids = TRIM_IDS )
@pytest.mark.parametrize( "model", MODELS )
def test_TrimmedIGESMatchesSTEP( model, split, demote, unit ):
    """The IGES written beside the STEP holds the same faces and area, facing the same way."""
    r = trimmed( model, split, demote, unit )
    stp = r[ "stp" ]
    igs = r[ "igs" ]
    c = Checks( { k[ len( "iges_" ): ]: v for k, v in _known_trimmed( model, split ).items() if k.startswith( "iges_" ) } )

    assert igs is not None, "no readable IGES file"

    # OCCT splits a trimmed IGES surface where it is only C0, so the faces are counted in the files
    nface = _count( r[ "stp_path" ], "ADVANCED_FACE" )
    assert _iges_count( r[ "igs_path" ], 144 ) == nface

    # Every surface is trimmed; one no loop bounds is left out, as STEP leaves out its face
    assert _iges_untrimmed_surfaces( r[ "igs_path" ] ) == []

    # Its curves share one color, and every flag the file sets is true
    assert _iges_count( r[ "igs_path" ], 314 ) <= 1
    assert _iges_flag_errors( r[ "igs_path" ] ) == []

    c.check( "valid", igs[ "valid" ], "OCCT rejects %d faces" % igs[ "bad_faces" ] )

    # What OCCT makes of faces it rejects has no area to compare
    if igs[ "valid" ]:
        c.check( "area", _rel( igs[ "area" ], stp[ "area" ] ) < 1.0e-4, "area %g, STEP %g" % ( igs[ "area" ], stp[ "area" ] ) )
        _same_facing( r )
    c.done()


@pytest.mark.parametrize( "model", MODELS )
def test_TrimmedIGESParameterSpace( model ):
    """Every IGES boundary is also written in its surface's parameters, and trims the same faces
    by those curves alone."""
    r = trimmed( model, 1, 0 )
    stp = r[ "stp" ]

    c = Checks( { k[ len( "iges_" ): ]: v for k, v in _known_trimmed( model, 1 ).items() if k in ( "iges_area", "iges_valid" ) } )

    # CRTN, SPTR, BPTR, CPTR, PREF: a parameter space curve, and neither representation preferred
    bounds = _iges_params( r[ "igs_path" ], 142 )
    assert len( bounds ) > 0
    assert all( int( b[2] ) != 0 and int( b[4] ) == 3 for b in bounds )

    igs = occthelp.measure( r[ "igs_path" ], topology = False, curves_2d = True )
    assert igs is not None, "no readable IGES file"
    c.check( "valid", igs[ "valid" ], "OCCT rejects %d faces" % igs[ "bad_faces" ] )
    if igs[ "valid" ]:
        c.check( "area", _rel( igs[ "area" ], stp[ "area" ] ) < 1.0e-6,
                 "area %g, STEP %g" % ( igs[ "area" ], stp[ "area" ] ) )
    c.done()


@pytest.mark.parametrize( "model", MODELS )
def test_TrimmedShell( model ):
    """The shell representation holds the same faces as the solid one, with no solids, and a
    closed shell for each solid."""
    r = trimmed( model, 1, 0, vsp.LEN_MM, vsp.STEP_SHELL )
    brep = trimmed( model, 1, 0 )
    stp = r[ "stp" ]
    c = Checks( { k: v for k, v in _known_trimmed( model, 1 ).items() if k == "valid" } )

    assert r[ "errors" ] == []
    assert stp is not None, "no readable STEP file"
    assert stp[ "solids" ] == 0
    assert _count( r[ "stp_path" ], "MANIFOLD_SOLID_BREP" ) == 0
    assert _count( r[ "stp_path" ], "CLOSED_SHELL" ) == _count( brep[ "stp_path" ], "MANIFOLD_SOLID_BREP" )
    assert stp[ "faces" ] == brep[ "stp" ][ "faces" ]
    assert _rel( stp[ "area" ], brep[ "stp" ][ "area" ] ) < 1.0e-12

    c.check( "valid", stp[ "valid" ], "OCCT rejects %d faces" % stp[ "bad_faces" ] )
    c.done()


def test_TrimmedLabels():
    """Each surface is labelled as asked, joined by the delimiter asked for."""
    load( "wing_pod" )
    stp = os.path.join( OUT, "labels.stp" )

    an = "SurfaceIntersection"
    vsp.SetAnalysisInputDefaults( an )
    vsp.SetIntAnalysisInput( an, "SelectedSetIndex", [ vsp.SET_ALL ] )
    vsp.SetIntAnalysisInput( an, "IGESFileFlag", [ 0 ] )
    for name, val in ( ( "CADLabelID", 0 ), ( "CADLabelName", 1 ), ( "CADLabelSurfNo", 1 ), ( "CADLabelSplitNo", 0 ),
                       ( "CADLabelDelim", vsp.DELIM_USCORE ) ):
        vsp.SetIntAnalysisInput( an, name, [ val ] )
    vsp.SetIntAnalysisInput( an, "STEPFileFlag", [ 1 ] )
    vsp.SetStringAnalysisInput( an, "STEPFileName", [ stp ] )
    vsp.ExecAnalysis( an )
    testhelp.assert_no_errors()

    labels = set( _surface_labels( stp ) )
    assert { "Surf_WingGeom_0", "Surf_PodGeom_0" } <= labels
    assert labels <= { "Surf_WingGeom_0", "Surf_WingGeom_1", "Surf_PodGeom_0" }


UNTRIM_OPTS = [ ( 0, 0 ), ( 1, 0 ), ( 0, 1 ), ( 1, 1 ) ]
UNTRIM_IDS = [ "split%d_cubic%d" % o for o in UNTRIM_OPTS ]


@pytest.mark.parametrize( "split, cubic", UNTRIM_OPTS, ids = UNTRIM_IDS )
@pytest.mark.parametrize( "model", MODELS )
def test_Untrimmed( model, split, cubic ):
    """File > Export's surfaces read as valid, and splitting or demoting does not change them.

    Splitting drops degenerate patches, which have no area, so the area is the same either
    way; demoting to cubic approximates, to its tolerance.  The split surfaces are the
    reference: OCCT integrates an unsplit surface's collapsed patches less well, 1.4e-4 off
    on WingMatrix, though the 204 patches splitting drops there come to exactly nothing.
    """
    r = untrimmed( model, split, cubic )
    stp = r[ "stp" ]
    igs = r[ "igs" ]
    ref = facts( model )

    assert r[ "errors" ] == []
    for m in ( stp, igs ):
        assert m is not None, "no readable file"
        assert m[ "faces" ] > 0
        assert m[ "valid" ], "OCCT rejects %d faces" % m[ "bad_faces" ]

    assert _rel( igs[ "area" ], stp[ "area" ] ) < 1.0e-6

    if ( split, cubic ) != ( 1, 0 ):
        base = untrimmed( model, 1, 0 )[ "stp" ]
        if split:
            assert _rel( stp[ "area" ], base[ "area" ] ) < 1.0e-4
        else:
            assert _rel( stp[ "area" ], base[ "area" ] ) < 5.0e-4

    assert stp[ "area" ] > ref[ "theo_area" ] * ( 1 - AREA_TOL )
    if model in BUILT and BUILT[ model ][1]:
        assert _rel( stp[ "area" ], ref[ "theo_area" ] ) < AREA_TOL

    assert _iges_flag_errors( r[ "igs_path" ] ) == []

    # Every surface faces out of its body: each component is whole, so the faces enclose the sum
    # of their volumes, and one facing in takes away twice its share
    if ref[ "normal" ]:
        for path in ( r[ "stp_path" ], r[ "igs_path" ] ):
            vol = occthelp.signed_volume( path ) / MM_PER[ vsp.LEN_MM ] ** 3
            assert _rel( vol, ref[ "theo_vol_whole" ] ) < VOLUME_TOL, \
                "%s encloses %g, CompGeom %g" % ( os.path.basename( path ), vol, ref[ "theo_vol_whole" ] )


# The degree in u and in w each stack is skinned to
STACK_DEGREE = { "stack_u1": ( 1, 3 ), "stack_u2": ( 2, 3 ), "stack_u3": ( 3, 3 ), "stack_u4": ( 4, 3 ),
                 "stack_u5": ( 5, 3 ), "stack_c2": ( 4, 3 ), "stack_blend": ( 3, 6 ), "stack_both": ( 5, 6 ) }


def _degrees( path ):
    """The highest degree in u and in v among a file's faces."""
    du = dv = 0
    ex = TopExp_Explorer( occthelp.read( path ), TopAbs_FACE )
    while ex.More():
        s = BRepAdaptor_Surface( TopoDS.Face( ex.Current() ) )
        if s.GetType() == GeomAbs_BSplineSurface:
            du = max( du, s.UDegree() )
            dv = max( dv, s.VDegree() )
        ex.Next()
    return du, dv


@pytest.mark.parametrize( "model", sorted( STACK_DEGREE ) )
def test_StackDegree( model ):
    """The stacks are written at the high degree they are skinned to, and at no more than cubic
    when demoted."""
    assert _degrees( untrimmed( model, 0, 0 )[ "stp_path" ] ) == STACK_DEGREE[ model ]
    du, dv = _degrees( untrimmed( model, 0, 1 )[ "stp_path" ] )
    assert du <= 3 and dv <= 3


@pytest.mark.parametrize( "option", [ "trimte", "mergelete", "mergepoints", "ft" ] )
@pytest.mark.parametrize( "model", sorted( BUILT ) )
def test_UntrimmedOptions( model, option ):
    """File > Export's other options keep the surfaces valid and their area.  These wings'
    trailing edges are sharp, so trimming them takes nothing away."""
    r = untrimmed( model, 1, 0, option )
    base = untrimmed( model, 1, 0 )[ "stp" ]
    stp = r[ "stp" ]
    igs = r[ "igs" ]

    assert r[ "errors" ] == []
    for m in ( stp, igs ):
        assert m is not None, "no readable file"
        assert m[ "faces" ] > 0
        assert m[ "valid" ], "OCCT rejects %d faces" % m[ "bad_faces" ]

    assert _rel( igs[ "area" ], stp[ "area" ] ) < 1.0e-6

    if option == "mergelete":
        assert _rel( stp[ "area" ], base[ "area" ] ) < 1.0e-4
    else:
        assert _rel( stp[ "area" ], base[ "area" ] ) < 1.0e-9


def test_UntrimmedTrimTE():
    """Trimming the trailing edge takes away a blunt edge's base, span times thickness, and
    nothing else."""
    span = 5.0
    thick = 0.05

    vsp.VSPRenew()
    testhelp.drop_errors()
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( vsp.FindParm( w, "Sym_Planar_Flag", "Sym" ), 0 )
    for name, val in ( ( "Root_Chord", 2.0 ), ( "Tip_Chord", 2.0 ), ( "Span", span ), ( "Sweep", 0.0 ) ):
        vsp.SetParmVal( vsp.FindParm( w, name, "XSec_1" ), val )
    vsp.Update()
    xs = vsp.GetXSecSurf( w, 0 )
    for i in range( vsp.GetNumXSec( xs ) ):
        x = vsp.GetXSec( xs, i )
        vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_Type" ), vsp.CLOSE_SKEWBOTH )
        vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_AbsRel" ), vsp.ABS )
        vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_Thick" ), thick )
    vsp.Update()

    area = {}
    for trim in ( 0, 1 ):
        common = [ ( "LenUnit", vsp.LEN_MM ), ( "SplitSurfs", 1 ), ( "ToCubic", 0 ), ( "TrimTE", trim ),
                   ( "MergeLETE", 0 ) ]
        _vehicle_parms( "STEPSettings", common )
        _vehicle_parms( "IGESSettings", common )
        for ext, ftype in ( ( "stp", vsp.EXPORT_STEP ), ( "igs", vsp.EXPORT_IGES ) ):
            path = os.path.join( OUT, "blunt_te_trim%d.%s" % ( trim, ext ) )
            vsp.ExportFile( path, vsp.SET_ALL, ftype )
            m = occthelp.measure( path, topology = False )
            assert m is not None and m[ "valid" ], "%s does not read valid" % os.path.basename( path )
            area[ ( trim, ext ) ] = m[ "area" ]
    testhelp.assert_no_errors()

    for ext in ( "stp", "igs" ):
        removed = area[ ( 0, ext ) ] - area[ ( 1, ext ) ]
        assert _rel( removed, span * thick ) < 1.0e-6, "%s: trimming took away %g, the base is %g" % ( ext, removed, span * thick )


def test_UntrimmedLabels():
    """Each wing surface is labelled with the side of the airfoil it is."""
    load( "sym_wing_offset" )
    _vehicle_parms( "STEPSettings", [ ( "LenUnit", vsp.LEN_MM ), ( "LabelID", 0 ), ( "LabelName", 1 ),
                                      ( "LabelSurfNo", 0 ), ( "LabelSplitNo", 0 ), ( "LabelAirfoilPart", 1 ),
                                      ( "LabelDelim", vsp.DELIM_USCORE ) ] )
    stp = os.path.join( OUT, "untrim_labels.stp" )
    vsp.ExportFile( stp, vsp.SET_ALL, vsp.EXPORT_STEP )
    testhelp.assert_no_errors()

    labels = set( _surface_labels( stp ) )
    assert { "Surf_WingGeom_upper", "Surf_WingGeom_lower" } <= labels
    assert all( l.startswith( "Surf_WingGeom" ) for l in labels )


STRUCT_OPTS = [ ( 1, 0 ), ( 0, 0 ), ( 1, 1 ) ]
STRUCT_IDS = [ "split%d_cubic%d" % o for o in STRUCT_OPTS ]


@pytest.mark.parametrize( "split, cubic", STRUCT_OPTS, ids = STRUCT_IDS )
@pytest.mark.parametrize( "model", _structure_models() )
def test_StructureUntrimmed( model, split, cubic ):
    """The structure parts' surfaces read as valid STEP and IGES of the same area, whatever
    the options."""
    r = structure_untrimmed( model, split, cubic )
    base = structure_untrimmed( model, 1, 0 )[ "stp" ]
    stp = r[ "stp" ]
    igs = r[ "igs" ]

    assert r[ "errors" ] == []
    for m in ( stp, igs ):
        assert m is not None, "no readable file"
        assert m[ "faces" ] > 0
        assert m[ "valid" ]
    assert _rel( igs[ "area" ], stp[ "area" ] ) < 1.0e-6
    assert _rel( stp[ "area" ], base[ "area" ] ) < 1.0e-4


@pytest.mark.parametrize( "model", _structure_models() )
def test_FEATrimmed( model ):
    """The FEA mesher's trimmed STEP and IGES read as valid and agree with each other.

    The skin is a closed body.  The parts are trimmed to it but not joined to it -- their
    edges along the skin are their own -- so they are a shell of their own, open along every
    edge the skin cuts.
    """
    r = fea_trimmed( model )
    stp = r[ "stp" ]
    igs = r[ "igs" ]

    assert r[ "errors" ] == []
    for m in ( stp, igs ):
        assert m is not None, "no readable file"
        assert m[ "faces" ] > 0
        assert m[ "valid" ], "OCCT rejects %d faces" % m[ "bad_faces" ]
    assert stp[ "solids" ] >= 1
    assert max( stp[ "edge_use" ] ) <= 2, "an edge bounds more than two faces: %s" % stp[ "edge_use" ]

    # The skin closes; the parts are an open shell beside it
    assert _count( r[ "stp_path" ], "CLOSED_SHELL" ) >= 1
    assert _count( r[ "stp_path" ], "OPEN_SHELL" ) >= 1
    assert _iges_count( r[ "igs_path" ], 144 ) == _count( r[ "stp_path" ], "ADVANCED_FACE" )
    assert _rel( igs[ "area" ], stp[ "area" ] ) < 1.0e-4
