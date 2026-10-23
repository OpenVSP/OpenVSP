# FEA mesh output that rests on an index into some other list: a node, a part, a subsurface or
# a property.  Each case builds the model that left one of those indices pointing at the wrong
# thing.  A case that can read or write past the end of a list runs in a subprocess, since that
# can take the process down.

import os
import re
import subprocess
import sys

import openvsp as vsp


def _run( script, tmp_path ):
    """Run a script in a fresh interpreter from tmp_path and return its exit code and output."""
    done = subprocess.run( [ sys.executable, "-c", "import openvsp as vsp\nprint( 'READY', flush = True )\n" + script ],
                           capture_output = True, text = True, timeout = 600, cwd = str( tmp_path ) )
    lines = done.stdout.split( "\n" )
    assert "READY" in lines, "the subprocess did not load OpenVSP:\n" + done.stderr
    return done.returncode, lines


def _pod_structure( max_len = 1.0 ):
    """A pod with one coarse structure, made the current one."""
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    ind = vsp.AddFeaStruct( pod )
    vsp.SetFeaMeshVal( pod, ind, vsp.CFD_MAX_EDGE_LEN, max_len )
    vsp.SetFeaMeshVal( pod, ind, vsp.CFD_MIN_EDGE_LEN, max_len / 5.0 )
    vsp.SetFeaMeshStructIndex( ind )
    return pod, ind, vsp.GetFeaStructID( pod, ind )


def _nastran( pod, ind, path ):
    """Mesh the structure, write its NASTRAN deck and return the bulk data file's lines."""
    vsp.Update()
    vsp.SetFeaMeshFileName( pod, ind, vsp.FEA_NASTRAN_FILE_NAME, str( path ) )
    vsp.ComputeFeaMesh( vsp.GetFeaStructID( pod, ind ), vsp.FEA_NASTRAN_FILE_NAME )
    return open( str( path ) + ".bdf" ).read().split( "\n" )


def testASpiderOnAPartWithNoNodesHasNoLegs( tmp_path ):
    """A fixed point's spider legs are the nearest nodes of its parent part.  A slice whose
    shells are deleted leaves no nodes on the part, so the spider has no legs -- rather than
    six copies of whatever node is first in the mesh."""
    pod, ind, sid = _pod_structure()
    sl = vsp.AddFeaPart( pod, ind, vsp.FEA_SLICE )
    vsp.SetParmVal( vsp.FindParm( sl, "MarkDelShellElements", "FeaPart" ), vsp.FEA_DELETE )
    fp = vsp.AddFeaPart( pod, ind, vsp.FEA_FIX_POINT )
    vsp.SetFeaFixPointParentID( fp, sl )
    vsp.SetParmVal( vsp.FindParm( fp, "PosU", "FeaFixPoint" ), 0.5 )
    vsp.SetParmVal( vsp.FindParm( fp, "PosW", "FeaFixPoint" ), 0.5 )

    lines = _nastran( pod, ind, tmp_path / "s.nas" )

    heads = [ i for i, l in enumerate( lines ) if l.startswith( "$ Node" ) and "spider points" in l ]
    assert len( heads ) == 1, "expected one spider, got %d" % len( heads )
    legs = []
    for l in lines[ heads[0] + 1: ]:
        if not l.startswith( "GRID" ):
            break
        legs.append( l )
    assert legs == [], "a part with no nodes gave the spider legs:\n" + "\n".join( legs )

    ids = [ int( l.split( "," )[1] ) for l in lines if l.startswith( "GRID" ) ]
    assert len( ids ) == len( set( ids ) ), "the deck defines a GRID more than once"


def testAPointMassOnAStructureWithNoMesh( tmp_path ):
    """A fixed point carrying a mass, on a structure whose only part is a removed skin, leaves
    the mesh with no nodes to find the point among.  The mass goes on the fixed point's own
    gridpoint."""
    script = """
vsp.VSPRenew()
pod = vsp.AddGeom( "POD" )
ind = vsp.AddFeaStruct( pod )
sid = vsp.GetFeaStructID( pod, ind )
vsp.SetFeaMeshVal( pod, ind, vsp.CFD_MAX_EDGE_LEN, 1.0 )
vsp.SetFeaMeshVal( pod, ind, vsp.CFD_MIN_EDGE_LEN, 0.2 )
skin = vsp.GetFeaPartID( sid, 0 )
vsp.SetParmVal( vsp.FindParm( skin, "RemoveSkinTrisFlag", "FeaSkin" ), 1 )
fp = vsp.AddFeaPart( pod, ind, vsp.FEA_FIX_POINT )
vsp.SetFeaFixPointParentID( fp, skin )
vsp.SetParmVal( vsp.FindParm( fp, "FixPointMassFlag", "FeaFixPoint" ), 1 )
vsp.SetParmVal( vsp.FindParm( fp, "FixPointMass_FEM", "FeaFixPoint" ), 5.0 )
vsp.Update()
vsp.SetFeaMeshFileName( pod, ind, vsp.FEA_NASTRAN_FILE_NAME, "s.nas" )
vsp.ComputeFeaMesh( sid, vsp.FEA_NASTRAN_FILE_NAME )
print( "DONE", flush = True )
"""
    code, lines = _run( script, tmp_path )
    assert code == 0 and "DONE" in lines, "meshing a point mass with no mesh failed (exit %d)" % code

    bdf = open( str( tmp_path / "s.nas.bdf" ) ).read().split( "\n" )
    masses = [ l for l in bdf if l.startswith( "CONM2" ) ]
    assert len( masses ) == 1, "expected one point mass, got:\n" + "\n".join( masses )
    assert int( masses[0].split( "," )[2] ) == 1, "the mass is not on the fixed point's gridpoint: " + masses[0]


def _constrained( lines ):
    """How many SPC1 cards a bulk data file holds."""
    return sum( 1 for l in lines if l.startswith( "SPC1" ) )


def testASubsurfaceConstraintWithNoSubsurfaceConstrainsNothing( tmp_path ):
    """A constraint on a subsurface that names none, or names one since deleted, applies to no
    node -- not to the structure's last part."""
    pod, ind, sid = _pod_structure()
    vsp.AddFeaPart( pod, ind, vsp.FEA_SLICE )
    bc = vsp.AddFeaBC( sid, vsp.FEA_BC_SUBSURF )
    vsp.SetParmVal( vsp.FindParm( bc, "Constraints", "FeaBC" ), 63 )

    assert _constrained( _nastran( pod, ind, tmp_path / "none.nas" ) ) == 0, \
           "a constraint naming no subsurface constrained nodes"

    ss = vsp.AddFeaSubSurf( pod, ind, vsp.SS_RECTANGLE )
    vsp.SetFeaBCSubSurfID( bc, ss )
    assert _constrained( _nastran( pod, ind, tmp_path / "some.nas" ) ) > 0, \
           "a constraint on a subsurface constrained no nodes"

    vsp.DeleteFeaSubSurf( pod, ind, ss )
    assert _constrained( _nastran( pod, ind, tmp_path / "gone.nas" ) ) == 0, \
           "a constraint naming a deleted subsurface constrained nodes"


_SUBSURFACES_WITH_SLICES = """
import re
vsp.VSPRenew()
pod = vsp.AddGeom( "POD" )
ind = vsp.AddFeaStruct( pod )
sid = vsp.GetFeaStructID( pod, ind )
vsp.SetFeaMeshVal( pod, ind, vsp.CFD_MAX_EDGE_LEN, 1.0 )
vsp.SetFeaMeshVal( pod, ind, vsp.CFD_MIN_EDGE_LEN, 0.2 )
for i in range( %d ):
    sl = vsp.AddFeaPart( pod, ind, vsp.FEA_SLICE )
    vsp.SetParmVal( vsp.FindParm( sl, "RotationAxis", "FeaSlice" ), vsp.Z_DIR )
    vsp.SetParmVal( vsp.FindParm( sl, "ZRot", "FeaSlice" ), 20.0 )
for name, u in ( ( "SSA", 0.3 ), ( "SSB", 0.7 ) ):
    ss = vsp.AddFeaSubSurf( pod, ind, vsp.SS_RECTANGLE )
    vsp.SetFeaSubSurfName( ss, name )
    vsp.SetParmVal( vsp.FindParm( ss, "Center_U", "SS_Rectangle" ), u )
vsp.Update()
vsp.SetFeaMeshFileName( pod, ind, vsp.FEA_NASTRAN_FILE_NAME, "s.nas" )
vsp.ComputeFeaMesh( sid, vsp.FEA_NASTRAN_FILE_NAME )
cnt = { "SSA": 0, "SSB": 0 }
cur = None
for l in open( "s.nas.bdf" ):
    m = re.match( r"\\$ (\\S+) \\S+ shell", l )
    if m:
        cur = m.group( 1 )
    elif l.startswith( "$" ):
        cur = None
    elif cur in cnt and ( l.startswith( "CTRIA" ) or l.startswith( "CQUAD" ) ):
        cnt[cur] += 1
print( "COUNTS", cnt[ "SSA" ], cnt[ "SSB" ], flush = True )
"""


def _subsurface_counts( nslice, tmp_path ):
    code, lines = _run( _SUBSURFACES_WITH_SLICES % nslice, tmp_path )
    assert code == 0, "meshing subsurfaces with %d coplanar slices failed (exit %d)" % ( nslice, code )
    counts = [ l for l in lines if l.startswith( "COUNTS" ) ]
    assert counts, "the subprocess printed no counts"
    return tuple( int( n ) for n in counts[0].split()[1:] )


def testSubsurfacesKeepTheirElementsWhenCoplanarPartsMerge( tmp_path ):
    """Two slices in the same place are merged into one before meshing, which leaves the second
    with no surface.  Each subsurface still gets the elements it gets beside a single slice."""
    one = _subsurface_counts( 1, tmp_path )
    two = _subsurface_counts( 2, tmp_path )

    assert one[0] > 0 and one[1] > 0, "a subsurface got no elements beside one slice: %s" % ( one, )
    assert two == one, "subsurface elements (SSA, SSB) were %s beside one slice, %s beside two merged" % ( one, two )


def _export( calculix, nastran ):
    """Write the current structure's mesh without meshing it again."""
    vsp.SetAnalysisInputDefaults( "FeaMeshExport" )
    for name in vsp.GetAnalysisInputNames( "FeaMeshExport" ):
        if name.endswith( "FileFlag" ):
            vsp.SetIntAnalysisInput( "FeaMeshExport", name, [ 0 ] )
    for prefix, path in ( ( "CALCULIX", calculix ), ( "NASTRAN", nastran ) ):
        vsp.SetIntAnalysisInput( "FeaMeshExport", prefix + "FileFlag", [ 1 ] )
        vsp.SetStringAnalysisInput( "FeaMeshExport", prefix + "FileName", [ str( path ) ] )
    vsp.ExecAnalysis( "FeaMeshExport" )


def _skin_with_property():
    """A pod whose skin takes a user property of a user material, with two more properties
    made on either side of it, meshed."""
    pod, ind, sid = _pod_structure()
    mat = vsp.AddFeaMaterial()
    props = []
    for name in ( "PropBefore", "PropSkin", "PropAfter" ):
        prop = vsp.AddFeaProperty()
        vsp.SetContainerName( prop, name )
        vsp.SetParmVal( vsp.FindParm( prop, "Thickness", "FeaProperty" ), 0.01 )
        vsp.SetFeaPropertyMaterialID( prop, mat )
        props.append( prop )
    vsp.SetFeaPartPropertyID( vsp.GetFeaPartID( sid, 0 ), props[1] )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "FeaMeshAnalysis" )
    vsp.ExecAnalysis( "FeaMeshAnalysis" )
    return mat, props


def testExportAfterDeletingAPropertyKeepsThePartsProperty( tmp_path ):
    """Exporting a mesh after deleting a property the mesh does not use writes each part with the
    property it was meshed with."""
    script = """
import sys
sys.path.insert( 0, %r )
from test_FeaMeshIndices import _skin_with_property, _export
mat, props = _skin_with_property()
vsp.DeleteFeaProperty( props[0] )
_export( "s.dat", "s.nas" )
print( "DONE", flush = True )
""" % os.path.dirname( os.path.abspath( __file__ ) )
    code, lines = _run( script, tmp_path )
    assert code == 0 and "DONE" in lines, "exporting after deleting a property failed (exit %d)" % code
    bdf = open( str( tmp_path / "s.nas.bdf" ) ).read()
    used = re.findall( r"^\$ (\S+) using", bdf, re.M )
    assert "PropSkin" in used and "PropAfter" not in used, \
           "the skin was meshed with PropSkin, but the deck's properties are %s" % ( used, )


def testExportAfterDeletingAMaterialInUseWritesNothing( tmp_path ):
    """Exporting a mesh after deleting the material its property is made of has no material to
    write, so it writes no file, as meshing would refuse."""
    script = """
import sys
sys.path.insert( 0, %r )
from test_FeaMeshIndices import _skin_with_property, _export
mat, props = _skin_with_property()
vsp.DeleteFeaMaterial( mat )
_export( "s.dat", "s.nas" )
print( "DONE", flush = True )
""" % os.path.dirname( os.path.abspath( __file__ ) )
    code, lines = _run( script, tmp_path )
    assert code == 0 and "DONE" in lines, "exporting after deleting a material failed (exit %d)" % code
    assert not os.path.exists( str( tmp_path / "s.dat" ) ), "a CalculiX file was written with its material deleted"
    assert not os.path.exists( str( tmp_path / "s.nas" ) ), "a NASTRAN file was written with its material deleted"


def testAssemblyExportAfterDeletingAMaterialInUseWritesNothing( tmp_path ):
    """An assembly is written from its structures' meshes as they stand, so the same holds for
    it: with a structure's material deleted, no mesh file is written."""
    script = """
import sys
sys.path.insert( 0, %r )
from test_FeaMeshIndices import _skin_with_property
mat, props = _skin_with_property()
assy = vsp.AddFeaAssembly()
vsp.AddFeaAssemblyStructure( assy, vsp.GetFeaMeshStructID() )
vsp.SetFeaAssemblyFileName( assy, vsp.FEA_CALCULIX_FILE_NAME, "a.dat" )
vsp.SetFeaAssemblyFileName( assy, vsp.FEA_NASTRAN_FILE_NAME, "a.nas" )
vsp.DeleteFeaMaterial( mat )
vsp.ComputeFeaAssemblyMesh( assy )
print( "DONE", flush = True )
""" % os.path.dirname( os.path.abspath( __file__ ) )
    code, lines = _run( script, tmp_path )
    assert code == 0 and "DONE" in lines, "exporting an assembly after deleting a material failed (exit %d)" % code
    assert not os.path.exists( str( tmp_path / "a.dat" ) ), "a CalculiX file was written with its material deleted"
    assert not os.path.exists( str( tmp_path / "a.nas" ) ), "a NASTRAN file was written with its material deleted"
