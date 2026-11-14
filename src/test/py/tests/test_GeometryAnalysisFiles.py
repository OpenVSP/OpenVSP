# Files the legacy-type Geometry Analysis cases write.
#
# CompGeom, Planar Slice, Projected Area and Mass Properties cases write the files their legacy
# forms write, each behind a flag Parm that starts off, under a name keyed by file type.

import openvsp as vsp

import os
import shutil
import xml.etree.ElementTree as ET

import pytest


# Analysis type, flag Parm, file type, default suffix
FILES = [
    ( vsp.COMP_GEOM, "CompGeomTxtFileFlag", vsp.COMP_GEOM_TXT_TYPE, "_CompGeom.txt" ),
    ( vsp.COMP_GEOM, "CompGeomCsvFileFlag", vsp.COMP_GEOM_CSV_TYPE, "_CompGeom.csv" ),
    ( vsp.PLANAR_SLICE, "SliceTxtFileFlag", vsp.SLICE_TXT_TYPE, "_Slice.txt" ),
    ( vsp.PROJ_AREA, "ProjAreaCsvFileFlag", vsp.PROJ_AREA_CSV_TYPE, "_ProjArea.csv" ),
    ( vsp.MASS_PROP, "MassPropTxtFileFlag", vsp.MASS_PROP_TXT_TYPE, "_MassProps.txt" ),
]

FLAGS = [ f[1] for f in FILES ]


def parm( case, name ):
    return vsp.FindParm( case, name, "InterferenceCase" )


def drainErrors():
    mgr = vsp.ErrorMgrSingleton.getInstance()
    n = mgr.GetNumTotalErrors()
    for _ in range( n ):
        mgr.PopLastError()
    return n


def evaluate( case ):
    a = "GeometryAnalysis"
    vsp.SetAnalysisInputDefaults( a )
    vsp.SetStringAnalysisInput( a, "CaseID", [ case ] )
    vsp.ExecAnalysis( a )
    drainErrors()


def makeModel( folder, name = "model.vsp3" ):
    """A pod and a wing, saved in folder so the default names land there."""
    vsp.VSPRenew()
    vsp.ErrorMgrSingleton.getInstance().SilenceErrors()
    vsp.AddGeom( "POD" )
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 2.0 )
    vsp.Update()
    fname = os.path.join( folder, name )
    vsp.WriteVSPFile( fname )
    return fname


def newCase( analysis_type, flags = () ):
    case = vsp.AddGeometryAnalysis()
    vsp.SetParmVal( parm( case, "IntererenceCheckType" ), analysis_type )
    for f in flags:
        vsp.SetParmVal( parm( case, f ), 1 )
    vsp.Update()
    return case


@pytest.fixture
def work( tmp_path, monkeypatch ):
    """Run from an empty folder, so a file written anywhere unexpected is seen."""
    monkeypatch.chdir( tmp_path )
    return tmp_path


def testEveryFlagStartsOff( work ):
    makeModel( str( work ) )
    case = vsp.AddGeometryAnalysis()
    for f in FLAGS:
        pid = parm( case, f )
        assert pid != "", f
        assert vsp.GetParmVal( pid ) == 0.0, f


@pytest.mark.parametrize( "analysis_type", [ vsp.COMP_GEOM, vsp.PLANAR_SLICE, vsp.PROJ_AREA, vsp.MASS_PROP ] )
def testNothingIsWrittenWithTheFlagsOff( work, analysis_type ):
    fname = makeModel( str( work ) )
    case = newCase( analysis_type )
    evaluate( case )
    assert sorted( os.listdir( str( work ) ) ) == [ os.path.basename( fname ) ]


def testTheDefaultNameIsTheModelTheCaseAndTheSuffix( work ):
    makeModel( str( work ), "My Plane.vsp3" )
    case = vsp.AddGeometryAnalysis()
    vsp.SetParmVal( parm( case, "IntererenceCheckType" ), vsp.COMP_GEOM )
    for _, _, ftype, suffix in FILES:
        want = str( work / ( "My Plane_Default" + suffix ) )
        assert vsp.GetGeometryAnalysisFileName( case, ftype ) == want


@pytest.mark.parametrize( "analysis_type, flag, ftype, suffix", FILES )
def testEachFileIsWrittenWithItsFlagOn( work, analysis_type, flag, ftype, suffix ):
    makeModel( str( work ) )
    case = newCase( analysis_type, [ flag ] )
    evaluate( case )

    fname = vsp.GetGeometryAnalysisFileName( case, ftype )
    assert fname == str( work / ( "model_Default" + suffix ) )
    assert os.path.isfile( fname )
    assert sorted( os.listdir( str( work ) ) ) == [ "model.vsp3", os.path.basename( fname ) ]


def testOnlyTheFlagsOfTheCaseTypeWrite( work ):
    makeModel( str( work ) )
    case = newCase( vsp.PLANAR_SLICE, FLAGS )
    evaluate( case )
    assert sorted( os.listdir( str( work ) ) ) == [ "model.vsp3", "model_Default_Slice.txt" ]


def testANameThatIsSetIsWhereTheFileGoes( work ):
    makeModel( str( work ) )
    case = newCase( vsp.COMP_GEOM, [ "CompGeomTxtFileFlag", "CompGeomCsvFileFlag" ] )

    out = work / "out"
    out.mkdir()
    txt = str( out / "wet.txt" )
    vsp.SetGeometryAnalysisFileName( case, vsp.COMP_GEOM_TXT_TYPE, txt )
    assert vsp.GetGeometryAnalysisFileName( case, vsp.COMP_GEOM_TXT_TYPE ) == txt

    evaluate( case )
    assert os.path.isfile( txt )
    assert os.path.isfile( str( work / "model_Default_CompGeom.csv" ) )
    assert not os.path.exists( str( work / "model_Default_CompGeom.txt" ) )

    # An empty name goes back to the default.
    vsp.SetGeometryAnalysisFileName( case, vsp.COMP_GEOM_TXT_TYPE, "" )
    assert vsp.GetGeometryAnalysisFileName( case, vsp.COMP_GEOM_TXT_TYPE ) == str( work / "model_Default_CompGeom.txt" )


def testTheDefaultNameFollowsTheCaseAndTheModel( work ):
    makeModel( str( work ) )
    case = vsp.AddGeometryAnalysis()

    vsp.SetGeometryAnalysisFileName( case, vsp.SLICE_TXT_TYPE, "" )
    vsp.SetParmVal( parm( case, "IntererenceCheckType" ), vsp.PLANAR_SLICE )

    # The case name is a ParmContainer name; it is reached by saving and editing the file.
    other = str( work / "renamed.vsp3" )
    vsp.WriteVSPFile( other )
    tree = ET.parse( other )
    tree.getroot().find( ".//GeometryAnalysis/ParmContainer/Name" ).text = "Wet Area"
    tree.write( other )

    vsp.VSPRenew()
    vsp.ReadVSPFile( other )
    case = vsp.GetAllGeometryAnalysesIDVec()[0]
    assert vsp.GetGeometryAnalysisFileName( case, vsp.SLICE_TXT_TYPE ) == str( work / "renamed_Wet_Area_Slice.txt" )


def testFlagsAndNamesComeBackFromTheFile( work ):
    model_dir = work / "model"
    model_dir.mkdir()
    fname = makeModel( str( model_dir ) )

    case = newCase( vsp.MASS_PROP, [ "MassPropTxtFileFlag", "CompGeomCsvFileFlag" ] )
    out = model_dir / "out"
    out.mkdir()
    vsp.SetGeometryAnalysisFileName( case, vsp.MASS_PROP_TXT_TYPE, str( out / "mp.txt" ) )
    vsp.WriteVSPFile( fname )

    # Set names are saved relative to the model; defaults are not saved.
    node = ET.parse( fname ).getroot().find( ".//GeometryAnalysis" )
    assert node.find( "MassPropTxtFileName" ).text == "out/mp.txt"
    assert node.find( "CompGeomTxtFileName" ) is None

    # Move the model and its folder of output, then read it from somewhere else.
    moved = work / "moved"
    shutil.copytree( str( model_dir ), str( moved ) )
    shutil.rmtree( str( model_dir ) )

    vsp.VSPRenew()
    vsp.ReadVSPFile( str( moved / "model.vsp3" ) )
    case = vsp.GetAllGeometryAnalysesIDVec()[0]

    on = { "MassPropTxtFileFlag", "CompGeomCsvFileFlag" }
    for f in FLAGS:
        assert vsp.GetParmVal( parm( case, f ) ) == float( f in on ), f

    assert vsp.GetGeometryAnalysisFileName( case, vsp.MASS_PROP_TXT_TYPE ) == str( moved / "out" / "mp.txt" )
    assert vsp.GetGeometryAnalysisFileName( case, vsp.COMP_GEOM_CSV_TYPE ) == str( moved / "model_Default_CompGeom.csv" )

    evaluate( case )
    assert os.path.isfile( str( moved / "out" / "mp.txt" ) )


def testABadIDOrFileTypeIsAnError( work ):
    makeModel( str( work ) )
    case = vsp.AddGeometryAnalysis()
    drainErrors()

    vsp.SetGeometryAnalysisFileName( "NOSUCHCASE", vsp.COMP_GEOM_TXT_TYPE, "x.txt" )
    assert drainErrors() > 0
    assert vsp.GetGeometryAnalysisFileName( "NOSUCHCASE", vsp.COMP_GEOM_TXT_TYPE ) == ""
    assert drainErrors() > 0

    vsp.SetGeometryAnalysisFileName( case, vsp.CFD_STL_TYPE, "x.stl" )
    assert drainErrors() > 0
    assert vsp.GetGeometryAnalysisFileName( case, vsp.CFD_STL_TYPE ) == ""
    assert drainErrors() > 0


# ==== The files match the ones the legacy analyses write ==== #

def legacyFiles( model, folder ):
    """Run each legacy analysis on its own fresh read of the model; return the files by type."""
    names = { vsp.COMP_GEOM_TXT_TYPE: "L_cg.txt", vsp.COMP_GEOM_CSV_TYPE: "L_cg.csv",
              vsp.SLICE_TXT_TYPE: "L_slice.txt", vsp.MASS_PROP_TXT_TYPE: "L_mp.txt" }

    def fresh():
        vsp.VSPRenew()
        vsp.ReadVSPFile( model )
        vsp.DeleteAllGeometryAnalyses()
        vsp.Update()
        for t, n in names.items():
            vsp.SetComputationFileName( t, os.path.join( folder, n ) )

    fresh()
    vsp.ComputeCompGeom( vsp.SET_ALL, False, vsp.COMP_GEOM_TXT_TYPE | vsp.COMP_GEOM_CSV_TYPE )
    fresh()
    vsp.ComputePlaneSlice( vsp.SET_ALL, 10, vsp.vec3d( 1, 0, 0 ), True )
    fresh()
    vsp.ComputeMassProps( vsp.SET_ALL, 10, vsp.X_DIR )
    fresh()
    vsp.SetAnalysisInputDefaults( "Projection" )
    rid = vsp.ExecAnalysis( "Projection" )
    proj = os.path.join( folder, "L_proj.csv" )
    vsp.WriteResultsCSVFile( rid, proj )
    drainErrors()

    files = { t: os.path.join( folder, n ) for t, n in names.items() }
    files[ vsp.PROJ_AREA_CSV_TYPE ] = proj
    return files


def csvRows( fname ):
    """Rows of a results CSV, less those that name the run rather than the answer."""
    skip = ( "Results_Name", "Results_Timestamp", "Results_Date", "Results_Time", "Results_ID",
             "Analysis_Duration_Sec", "Mesh_GeomID", "Result", "Comp_IDs" )
    rows = []
    with open( fname ) as f:
        for line in f:
            if line.split( "," )[0] not in skip:
                rows.append( line )
    return rows


def testTheFilesMatchTheLegacyFiles( work ):
    model = makeModel( str( work ) )
    cases = {}
    for analysis_type, flag, ftype, suffix in FILES:
        if analysis_type not in cases:
            cases[ analysis_type ] = newCase( analysis_type )
        vsp.SetParmVal( parm( cases[ analysis_type ], flag ), 1 )
    for case in cases.values():
        evaluate( case )

    ga = {}
    for analysis_type, flag, ftype, suffix in FILES:
        ga[ ftype ] = vsp.GetGeometryAnalysisFileName( cases[ analysis_type ], ftype )

    legacy = legacyFiles( model, str( work ) )

    for ftype in ( vsp.COMP_GEOM_TXT_TYPE, vsp.COMP_GEOM_CSV_TYPE, vsp.SLICE_TXT_TYPE, vsp.MASS_PROP_TXT_TYPE ):
        with open( ga[ ftype ] ) as f:
            got = f.read()
        with open( legacy[ ftype ] ) as f:
            want = f.read()
        assert got == want, ga[ ftype ]

    assert csvRows( ga[ vsp.PROJ_AREA_CSV_TYPE ] ) == csvRows( legacy[ vsp.PROJ_AREA_CSV_TYPE ] )
