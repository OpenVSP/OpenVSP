# An IGES file may be written to a name without an extension.
#
# libIGES read the extension from one past the end of a name with no dot and threw, which
# aborted the process.  Each export runs in its own interpreter so an abort fails the test
# rather than the whole run.

import os
import subprocess
import sys
import tempfile

GEOM_SCRIPT = """
import sys
import openvsp as vsp
vsp.AddGeom( "POD" )
vsp.Update()
vsp.ExportFile( sys.argv[1], vsp.SET_ALL, vsp.EXPORT_IGES )
"""

FEA_SCRIPT = """
import sys
import openvsp as vsp
wing = vsp.AddGeom( "WING" )
ind = vsp.AddFeaStruct( wing )
vsp.SetFeaMeshVal( wing, ind, vsp.CFD_MAX_EDGE_LEN, 1.0 )
vsp.SetFeaMeshVal( wing, ind, vsp.CFD_MIN_EDGE_LEN, 0.2 )
vsp.AddFeaPart( wing, ind, vsp.FEA_RIB )
vsp.SetFeaMeshStructIndex( ind )
vsp.Update()
vsp.SetAnalysisInputDefaults( "FeaMeshAnalysis" )
vsp.ExecAnalysis( "FeaMeshAnalysis" )
vsp.SetAnalysisInputDefaults( "FeaMeshExport" )
for name in vsp.GetAnalysisInputNames( "FeaMeshExport" ):
    if name.endswith( "FileFlag" ):
        vsp.SetIntAnalysisInput( "FeaMeshExport", name, [ 0 ] )
vsp.SetIntAnalysisInput( "FeaMeshExport", "IGESFileFlag", [ 1 ] )
vsp.SetStringAnalysisInput( "FeaMeshExport", "IGESFileName", [ sys.argv[1] ] )
vsp.ExecAnalysis( "FeaMeshExport" )
"""


def export( script, name ):
    path = os.path.join( tempfile.mkdtemp( prefix = "vsp_iges_" ), name )
    run = subprocess.run( [ sys.executable, "-c", script, path ], capture_output = True, text = True )
    assert run.returncode == 0, run.stderr[-2000:]
    assert os.path.getsize( path ) > 0
    return path


def testGeomIGESToANameWithoutExtension():
    export( GEOM_SCRIPT, "pod" )


def testGeomIGESToANameWithExtension():
    export( GEOM_SCRIPT, "pod.igs" )


def testFeaIGESToANameWithoutExtension():
    export( FEA_SCRIPT, "wing_mesh" )
