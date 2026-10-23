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
