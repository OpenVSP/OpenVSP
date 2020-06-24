//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// APITestSuiteCFDMesh.cpp: Unit tests for cfd_mesh
//
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Geom_API.h"
#include "APITestSuiteCFDMesh.h"
#include "APIDefines.h"

#include<fstream>

#include <chrono>
using namespace std::chrono;
//=============================================================================//
//========================== APITestSuiteCFDMesh ==============================//
//=============================================================================//

void APITestSuiteCFDMesh::TestCFDHalfMesh()
{
    printf( "APITestSuiteCFDMesh::TestCFDHalfMesh()\n" );
    vsp::VSPCheckSetup();
    vsp::VSPRenew();

    //Create Geoms
    printf( "\tGenerating geometry\n" );
    //Fuselage
    string fuselage_id = vsp::AddGeom( "FUSELAGE" );
    //Wing
    string wing_id = vsp::AddGeom( "WING" );
    //Horizontal stabilizer
    string horz_tail_id = vsp::AddGeom( "WING" );
    //Vertical stabilizer
    string vert_tail_id = vsp::AddGeom( "WING" );

    //Edit fuselage
    vsp::SetParmVal( fuselage_id, "Ellipse_Height", "XSecCurve_1", 3.75 );
    vsp::SetParmVal( fuselage_id, "Ellipse_Width", "XSecCurve_1", 3.75 );

    //Edit wing
    vsp::SetParmVal( wing_id, "X_Rel_Location", "XForm", 10 );
    vsp::SetParmVal( wing_id, "Z_Rel_Location", "XForm", -0.870 );

    vsp::SetParmVal( wing_id, "Dihedral", "XSec_1", 6.95652 );
    vsp::SetParmVal( wing_id, "Sweep", "XSec_1", 34.45455 );

    vsp::SetParmVal( wing_id, "Span", "XSec_1", 14.0 );
    vsp::SetParmVal( wing_id, "Root_Chord", "XSec_1", 6 );
    vsp::SetParmVal( wing_id, "Tip_Chord", "XSec_1", 1.5 );

    //Edit Horizontal stabilizer
    vsp::SetParmVal( horz_tail_id, "X_Rel_Location", "XForm", 27.174 );
    vsp::SetParmVal( horz_tail_id, "Z_Rel_Location", "XForm", .50 );

    vsp::SetParmVal( horz_tail_id, "Dihedral", "XSec_1", 6.95652 );
    vsp::SetParmVal( horz_tail_id, "Sweep", "XSec_1", 34.45455 );

    vsp::SetParmVal( horz_tail_id, "Span", "XSec_1", 4.0 );
    vsp::SetParmVal( horz_tail_id, "Root_Chord", "XSec_1", 2.2 );
    vsp::SetParmVal( horz_tail_id, "Tip_Chord", "XSec_1", 0.65885 );

    //Edit Vertical stabilizer
    vsp::SetParmVal( vsp::FindParm(vert_tail_id, "Sym_Planar_Flag", "Sym" ), 0.0 );
    vsp::SetParmVal( vert_tail_id, "X_Rel_Rotation", "XForm", 90.0 );
    vsp::SetParmVal( vert_tail_id, "X_Rel_Location", "XForm", 27.065 );

    vsp::SetParmVal( vert_tail_id, "Span", "XSec_1", 5.0 );
    vsp::SetParmVal( vert_tail_id, "Root_Chord", "XSec_1", 2.6 );
    vsp::SetParmVal( vert_tail_id, "Tip_Chord", "XSec_1", 0.5 );

    vsp::Update();

    //Write mesh to file
    string fname = "apitest_TestCFDHalfMesh.vsp3";
    vsp::WriteVSPFile( fname );

    //Full mesh test
    vsp::SetComputationFileName( vsp::CFD_STL_TYPE, "FullMeshTest.stl" );

    vsp::SetCFDMeshVal( vsp::CFD_MIN_EDGE_LEN, 1.0 );
    vsp::SetCFDMeshVal( vsp::CFD_MAX_EDGE_LEN, 2.0 );
    vsp::SetCFDMeshVal( vsp::CFD_HALF_MESH_FLAG, 0.0 );

    printf( "\tComputing full mesh...\n" );
    seconds before_full_mesh_test = duration_cast < seconds > ( system_clock::now().time_since_epoch() );

    vsp::ComputeCFDMesh( vsp::SET_ALL, vsp::CFD_STL_TYPE );

    seconds after_full_mesh_test = duration_cast < seconds > ( system_clock::now().time_since_epoch() );
    printf( "\tMesh complete\n" );

    seconds full_mesh_test_results = after_full_mesh_test - before_full_mesh_test;

    //Half mesh test
    vsp::SetComputationFileName( vsp::CFD_STL_TYPE, "HalfMeshTest.stl" );

    vsp::SetCFDMeshVal( vsp::CFD_HALF_MESH_FLAG, 1.0 );

    printf( "\tComputing half mesh...\n" );
    seconds before_half_mesh_test = duration_cast < seconds > ( system_clock::now().time_since_epoch() );

    vsp::ComputeCFDMesh( vsp::SET_ALL, vsp::CFD_STL_TYPE );

    seconds after_half_mesh_test = duration_cast < seconds > ( system_clock::now().time_since_epoch() );
    printf( "\tMesh complete\n" );

    seconds half_mesh_test_results = after_half_mesh_test - before_half_mesh_test;

    TEST_ASSERT( full_mesh_test_results > half_mesh_test_results );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );
}

void APITestSuiteCFDMesh::TestSurfaceIntersection()
{
    //Set up for test
    printf( "APITestSuiteCFDMesh::TestSurfaceIntersection() \n" );
    vsp::VSPCheckSetup();
    vsp::VSPRenew();

    //The test_compare_value is for when comparing file sizes by %
    //to see if the differance in size is within the test_compare_value
    double test_compare_value = 10; //10%

    //Add and edit Geometry
    string pod_id = vsp::AddGeom( "POD" );
    string wing_id = vsp::AddGeom( "WING" );
    vsp::SetParmValUpdate( wing_id, "X_Rel_Location", "XForm", 3.0 );

    //Run Tests
    printf( "\tPerforming 0.01x scale test\n" );
    RunScaleTest( .01, test_compare_value );
    printf( "\tPerforming 100x scale test\n" );
    RunScaleTest( 100, test_compare_value );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );
}

void APITestSuiteCFDMesh::RunAnalysis( vector < string > file_1, vector < string > file_2 )
{
    //Get analysis type
    string analysis_name = "SurfaceIntersection";

    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    //Set these files to be generated
    vsp::SetStringAnalysisInput( analysis_name, "IGESFileName", file_1 );
    vsp::SetStringAnalysisInput( analysis_name, "STEPFileName", file_2 );

    //This sets the Geoms to be shown again after analysis
    vector < int > sets;
    sets.push_back( vsp::SET_ALL );
    vsp::SetIntAnalysisInput( analysis_name, "SelectedSetIndex", sets );

    vector < int > representation;
    representation.push_back( vsp::STEP_SHELL );

    vsp::SetIntAnalysisInput( analysis_name, "STEPRepresentation", representation, 0 );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    printf("Executing Analysis\n");
    string resid = vsp::ExecAnalysis( analysis_name );
}

void APITestSuiteCFDMesh::RunScaleTest( double scale_value, double test_compare_percent )
{
    //Add names for files
    vector < string > iges_name_before;
    iges_name_before.push_back( "API_Trimmed_IGES_Orig.igs" );
    vector < string > step_name_before;
    step_name_before.push_back( "API_Trimmed_STEP_Orig.stp" );

    //Run analysis before scaling Geometry
    RunAnalysis( iges_name_before, step_name_before );

    //Get the size of files after the first analysis
    int iges_file_size_before_scale = GetFileSize( iges_name_before[0] );

    //Check to make sure files were found and file sizes were capturedafter first analysis
    if ( iges_file_size_before_scale == -1 )
    {
        TEST_FAIL( "File not found for iges_file_size_before_scale" );
    }
    int step_file_size_before_scale = GetFileSize( step_name_before[0] );
    if ( step_file_size_before_scale == -1 )
    {
        TEST_FAIL( "File not found for step_file_size_before_scale" );
    }

    //Scale Geometry after first analysis for test 
    vsp::ScaleSet( vsp::SET_ALL, scale_value );

    //Add names for files
    vector < string > iges_name_after;
    iges_name_after.push_back( "API_Trimmed_IGES_Scaled.igs" );
    vector < string > step_name_after;
    step_name_after.push_back( "API_Trimmed_STEP_Scaled.stp" );

    //Run analysis after scaling geometry
    RunAnalysis( iges_name_after, step_name_after );
    
    //Reset Scale to original size
    vsp::ScaleSet( vsp::SET_ALL, 1 / scale_value );

    //Get the size of files after the second analysis
    int iges_file_size_after_scale = GetFileSize( iges_name_after[0] );

    //Check to make sure files were found and file sizes were capturedafter first analysis
    if ( iges_file_size_after_scale == -1 )
    {
        TEST_FAIL( "File not found for iges_file_size_after_scale" );
    }
    int step_file_size_after_scale = GetFileSize( step_name_after[0] );
    if ( step_file_size_after_scale == -1 )
    {
        TEST_FAIL( "File not found for step_file_size_after_scale" );
    }

    //Iges files results
    printf( "Scale value: %Lf IGES before %d, IGES after %d \n", scale_value, iges_file_size_before_scale, iges_file_size_after_scale );
    double iges_dif = 100 * ( ( iges_file_size_after_scale - iges_file_size_before_scale ) / (double)iges_file_size_before_scale );
    printf( "IGES difference %Lf%%\n", iges_dif );

    //Step files results
    printf( "Scale value: %Lf STEP before %d, STEP after %d \n", scale_value, step_file_size_before_scale, step_file_size_after_scale );
    double step_dif = 100 * ( ( step_file_size_after_scale - step_file_size_before_scale ) / (double)step_file_size_before_scale );
    printf( "STEP difference %Lf%%\n", step_dif );

    //Testing if file size is within acceptible range
    TEST_ASSERT( abs( iges_dif ) <= test_compare_percent );
    TEST_ASSERT( abs( step_dif ) <= test_compare_percent );
}

int APITestSuiteCFDMesh::GetFileSize( string file_name )
{
    int fileSize = 0;
    std::ifstream in_file(file_name, std::ios::binary );
    if ( !in_file.is_open() )
    {
        return -1;
    }
    in_file.seekg( 0, std::ios::end );
    fileSize = int( in_file.tellg() );
    in_file.close();
    return fileSize;
}
