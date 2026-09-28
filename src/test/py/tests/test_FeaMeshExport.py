# Meshing a structure and writing its files are separate analyses: FeaMeshAnalysis meshes,
# FeaMeshExport writes what it is asked for.

import openvsp as vsp
import os
import tempfile

from testhelp import ( drop_errors )


def a_structure():
    """A pod with one coarse structure, made the current one."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    ind = vsp.AddFeaStruct( pod )
    vsp.SetFeaMeshVal( pod, ind, vsp.CFD_MAX_EDGE_LEN, 1.0 )
    vsp.SetFeaMeshVal( pod, ind, vsp.CFD_MIN_EDGE_LEN, 0.2 )
    vsp.SetFeaMeshStructIndex( ind )
    vsp.Update()
    return pod, ind


def no_files( analysis ):
    for name in vsp.GetAnalysisInputNames( analysis ):
        if name.endswith( "FileFlag" ):
            vsp.SetIntAnalysisInput( analysis, name, [ 0 ] )


def mesh():
    vsp.SetAnalysisInputDefaults( "FeaMeshAnalysis" )
    vsp.ExecAnalysis( "FeaMeshAnalysis" )


def export( files ):
    """Write the given { input prefix: path } with FeaMeshExport and return the error codes."""
    vsp.SetAnalysisInputDefaults( "FeaMeshExport" )
    no_files( "FeaMeshExport" )
    for prefix, path in files.items():
        vsp.SetIntAnalysisInput( "FeaMeshExport", prefix + "FileFlag", [ 1 ] )
        vsp.SetStringAnalysisInput( "FeaMeshExport", prefix + "FileName", [ path ] )
    drop_errors()
    vsp.ExecAnalysis( "FeaMeshExport" )
    em = vsp.ErrorMgrSingleton.getInstance()
    codes = []
    while em.GetNumTotalErrors() > 0:
        codes.append( em.PopLastError().m_ErrorCode )
    return codes


def written( path ):
    return os.path.exists( path ) and os.path.getsize( path ) > 0


def testMeshingWritesNoFiles():
    """The file inputs belong to FeaMeshExport, so meshing alone writes nothing."""
    a_structure()
    assert not any( n.endswith( "FileFlag" ) or n.endswith( "FileName" )
                    for n in vsp.GetAnalysisInputNames( "FeaMeshAnalysis" ) )
    out = tempfile.mkdtemp()
    here = os.getcwd()
    os.chdir( out )
    try:
        mesh()
    finally:
        os.chdir( here )
    assert os.listdir( out ) == []
    drop_errors()


def testExportWritesTheMeshAndCADFilesAskedFor():
    a_structure()
    mesh()
    out = tempfile.mkdtemp()
    files = { "CALCULIX": os.path.join( out, "s.dat" ), "STL": os.path.join( out, "s.stl" ),
              "STEP": os.path.join( out, "s.stp" ), "IGES": os.path.join( out, "s.igs" ) }
    assert export( files ) == []
    for path in files.values():
        assert written( path ), path
    assert sorted( os.listdir( out ) ) == sorted( os.path.basename( p ) for p in files.values() )


def testExportingBeforeMeshingReportsAnError():
    a_structure()
    out = tempfile.mkdtemp()
    stl = os.path.join( out, "s.stl" )
    assert export( { "STL": stl } ) == [ vsp.VSP_FILE_WRITE_FAILURE ]
    assert not os.path.exists( stl )


def testCADFilesNeedTheStructureMeshedLast():
    """Selecting another structure drops the intersection data the CAD files are written from,
    but the mesh is kept, so its files can still be written."""
    pod, ind = a_structure()
    mesh()
    other = vsp.AddFeaStruct( pod )
    vsp.SetFeaMeshStructIndex( other )
    vsp.SetFeaMeshStructIndex( ind )
    out = tempfile.mkdtemp()
    stl = os.path.join( out, "s.stl" )
    stp = os.path.join( out, "s.stp" )
    assert export( { "STL": stl, "STEP": stp } ) == [ vsp.VSP_FILE_WRITE_FAILURE ]
    assert written( stl )
    assert not os.path.exists( stp )


def testComputeFeaMeshWritesTheCADFileItIsAskedFor():
    """The deprecated one-call form meshes and writes the one file named, CAD types included."""
    pod, ind = a_structure()
    stp = os.path.join( tempfile.mkdtemp(), "s.stp" )
    vsp.SetFeaMeshFileName( pod, ind, vsp.FEA_STEP_FILE_NAME, stp )
    vsp.ComputeFeaMesh( pod, ind, vsp.FEA_STEP_FILE_NAME )
    assert written( stp )
    drop_errors()
