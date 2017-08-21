//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Geom_API.h"
#include "APITestSuite.h"
#include <float.h>

//Default tolerance to use for tests.  Most calculations are done as doubles and choosing single precision FLT_MIN gives some allowance for precision stackup in calculations
#define TEST_TOL FLT_MIN

//==== Test Geometry Creation ====//
void APITestSuite::CheckSetup()
{

    printf( "APITestSuite::CheckSetup()\n" );
    vsp::VSPCheckSetup();
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "VSPCheckSetup ERROR" );    // if this happens something is very wrong
    }
}
//==== Test Geometry Creation ====//
void APITestSuite::CreateGeometry()
{
    printf( "APITestSuite::CreateGeometry()\n" );
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    vector<string> types = vsp::GetGeomTypes( );
    TEST_ASSERT( types.size() != 0 );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "\t[geom_id]\t[geom_name]\n" );
    for ( unsigned int i_geom_type = 0; i_geom_type < types.size(); i_geom_type++ )
    {
        //==== Create geometry =====//
        string geom_id = vsp::AddGeom( types[i_geom_type] );
        printf( "\t%s", geom_id.c_str() );
        TEST_ASSERT( geom_id.c_str() != NULL );
        TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

        //==== Set Name ====//
        string geom_name = string( "TestGeom_" ) + types[i_geom_type];
        vsp::SetGeomName( geom_id, geom_name );
        printf( "\t%s\n", geom_name.c_str() );
        TEST_ASSERT( vsp::GetGeomName( geom_id ) == geom_name )
        TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

        //==== Check to make sure it got added to the list ====//
        vector<string> geoms = vsp::FindGeoms();
        TEST_ASSERT( geoms.size() == i_geom_type + 1 );
        TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    }
    printf( "\n" );

    //==== Save Vehicle to File ====//
    string fname = "apitest_CreateGeometry.vsp3";
    vsp::WriteVSPFile( fname );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
}

void APITestSuite::ChangePodParams()
{
    printf( "APITestSuite::ChangePodParams()\n" );
    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Pod Geom =====//
    string pod_id = vsp::AddGeom( "POD" );
    TEST_ASSERT( pod_id.c_str() != NULL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Set Name ====//
    string pod_name = "Pod";
    vsp::SetGeomName( pod_id, pod_name );
    TEST_ASSERT( vsp::GetGeomName( pod_id ) == pod_name )
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change Length with TWO step method: step 1 - GetParm(), step 2 - SetParmValUpdate()
    double len_val = 7.0;
    string len_id = vsp::GetParm( pod_id, "Length", "Design" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( len_id, len_val ), len_val, TEST_TOL );    //tests SetParmValUpdate)
    TEST_ASSERT_DELTA( vsp::GetParmVal( len_id ), len_val, TEST_TOL );                //tests GetParmVal
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change Finess Ratio with ONE step method: SetParmValUpdate()
    double finess_val = 10;
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "FineRatio", "Design", finess_val ), finess_val, TEST_TOL ) ;
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change X Location  with ONE step method: SetParmValUpdate()
    double x_loc_val = 3.0;
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "X_Rel_Location", "XForm", x_loc_val ), x_loc_val, TEST_TOL ) ;
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change Y Location  with ONE step method: SetParmValUpdate()
    double y_loc_val = 1.0;
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "Y_Rel_Location", "XForm", y_loc_val ), y_loc_val, TEST_TOL ) ;
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change Z Location  with ONE step method: SetParmValUpdate()
    double z_loc_val = 4.2;
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "Z_Rel_Location", "XForm", z_loc_val ), z_loc_val, TEST_TOL ) ;
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change Symmetry =====//
    string sym_flag_id = vsp::GetParm( pod_id, "Sym_Planar_Flag", "Sym" );
    vsp::SetParmValUpdate( sym_flag_id, vsp::SYM_XZ  );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Save Vehicle to File ====//
    string fname = "apitest_ChangePodParams.vsp3";
    vsp::WriteVSPFile( fname );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
}

//==== Use Case 1 =====//
void APITestSuite::CopyPasteGeometry()
{
    printf( "APITestSuite::CopyPasteGeometry()\n" );
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    vector<string> types = vsp::GetGeomTypes( );
    TEST_ASSERT( types.size() != 0 );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Fuselage Geom =====//
    string fuse_id = vsp::AddGeom( "FUSELAGE" );
    TEST_ASSERT( fuse_id.c_str() != NULL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Pod Geom and set some parameters =====//
    string first_pod_id = vsp::AddGeom( "POD", fuse_id );
    TEST_ASSERT( first_pod_id.c_str() != NULL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change First Pod Parameters (name, length, finess ratio, y location, x location, symetry) ====//
    string pod_name = "Pod";
    vsp::SetGeomName( first_pod_id, pod_name );
    //    test that the parameters got set within the TEST_TOL tolerance
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  first_pod_id, "Length", "Design", 7.0 ), 7.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( first_pod_id, "FineRatio", "Design", 10.0 ), 10.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( first_pod_id, "X_Rel_Location", "XForm", 3.0 ), 3.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( first_pod_id, "Y_Rel_Location", "XForm", 1.0 ), 1.0, TEST_TOL );
    vsp::SetParmValUpdate( first_pod_id, "Sym_Planar_Flag", "Sym", vsp::SYM_XZ  );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Copy/Paste Pod Geom =====//
    vsp::CopyGeomToClipboard( first_pod_id );                // copy pod to clipboard
    vsp::PasteGeomClipboard( first_pod_id );                    // Make fuse_id parent
    vsp::SetGeomName( first_pod_id, "Original_Pod" );            // change name of first pod so that the newly paasted pod can be found by searching for the name "Pod"
    string second_pod_id = vsp::FindGeom( "Pod", 0 );    // search for the copied pod
    TEST_ASSERT( second_pod_id != "" );                // assert if the 2nd pod was not found (copy/paste operation FAILED)
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    // set name of second pod to something unique
    vsp::SetGeomName( second_pod_id, "Second_Pod" );    // change the name of the second pod to something more descriptive
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change Second Pod Parameters (name, y location, z location, symetry) ====//
    TEST_ASSERT_DELTA( vsp::SetParmVal( second_pod_id, "Y_Rel_Location", "XForm", 0.0 ), 0.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( second_pod_id, "Z_Rel_Location", "XForm", 1.0 ), 1.0, TEST_TOL );
    vsp::SetParmVal( second_pod_id, "Sym_Planar_Flag", "Sym", 0 );    // no symetry
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Check Second pod has the same length, finess ratio, and x location as the first ====//
    TEST_ASSERT_DELTA( vsp::GetParmVal( first_pod_id, "Length", "Design"  ), vsp::GetParmVal( second_pod_id, "Length", "Design" ), TEST_TOL );
    TEST_ASSERT_DELTA( vsp::GetParmVal( first_pod_id, "FineRatio", "Design" ), vsp::GetParmVal( second_pod_id, "FineRatio", "Design" ), TEST_TOL );
    TEST_ASSERT_DELTA( vsp::GetParmVal( first_pod_id, "X_Rel_Location", "XForm" ), vsp::GetParmVal( second_pod_id, "X_Rel_Location", "XForm" ), TEST_TOL );

    //==== Save Vehicle to File ====//
    string fname = "apitest_CopyPasteGeometry.vsp3";
    vsp::WriteVSPFile( fname );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

}

// Test of analysis manager
void APITestSuite::CheckAnalysisMgr()
{
    printf( "APITestSuite::CheckAnalysisMgr()\n" );
    unsigned int n_analysis = ( unsigned int )vsp::GetNumAnalysis();
    std::vector < std::string > analysis_names = vsp::ListAnalysis();
    printf( "    Analyses found: %d\n", n_analysis );
    printf( "\t[analysis_name]\n" );
    printf( "\t\t%-20s%s\t%s\n", "[input_name]", "[type]", "[#]" );
    for ( unsigned int i_analysis = 0; i_analysis < n_analysis; i_analysis++ )
    {
        // print out name
        printf( "\t%s\n", analysis_names[i_analysis].c_str() );

        // get input names
        vector < string > input_names = vsp::GetAnalysisInputNames( analysis_names[i_analysis] );
        for ( unsigned int i_input_name = 0; i_input_name < input_names.size(); i_input_name++ )
        {
            int current_input_type = vsp::GetAnalysisInputType( analysis_names[i_analysis], input_names[i_input_name] );
            int current_input_num_data = vsp::GetNumAnalysisInputData( analysis_names[i_analysis], input_names[i_input_name] );

            // print out name and type enumeration
            printf( "\t\t%-20s%d\t\t%d", input_names[i_input_name].c_str(), current_input_type, current_input_num_data );

            // ASSERT if an invalid type is found
            TEST_ASSERT( current_input_type != vsp::INVALID_TYPE );

            printf( "\n" );
        }

    }
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuite::TestAnalysesWithPod()
{
    printf( "APITestSuite::TestAnalysesWithPod()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Pod Geom and set some parameters =====//
    string pod_id = vsp::AddGeom( "POD" );
    TEST_ASSERT( pod_id.c_str() != NULL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change Pod Parameters (name, length, finess ratio, y location, x location, symetry) ====//
    string pod_name = "Pod_Test";
    vsp::SetGeomName( pod_id, pod_name );
    //    test that the parameters got set within the TEST_TOL tolerance
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  pod_id, "Length", "Design", 7.0 ), 7.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "FineRatio", "Design", 10.0 ), 10.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "X_Rel_Location", "XForm", 3.0 ), 3.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "Y_Rel_Location", "XForm", 1.0 ), 1.0, TEST_TOL );
    vsp::SetParmValUpdate( pod_id, "Sym_Planar_Flag", "Sym", vsp::SYM_XZ  );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Save Vehicle to File ====//
    string fname = "apitest_TestAnalysesWithPod.vsp3";
    vsp::WriteVSPFile( fname );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Analysis: CompGeom ====//
    string analysis_name = "CompGeom";
    printf( "\t%s\n", analysis_name.c_str() );

    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf( "\n\t\tExecuting..." );
    string results_id = vsp::ExecAnalysis( analysis_name );
    printf( "COMPLETE\n\n" );

    // Get & Display Results

    vsp::PrintResults( results_id );

    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

}

void APITestSuite::TestDXFExport()
{
    printf( "APITestSuite::TestDXFExport()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();

    //==== Add Wing Geom and set some parameters =====//
    string wing_id = vsp::AddGeom( "WING" );
    TEST_ASSERT( wing_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TotalSpan", "WingGeom", 30.0 ), 30.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "LECluster", "WingGeom", 0.1 ), 0.1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TECluster", "WingGeom", 2.0 ), 2.0, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Fuselage Geom and set some parameters =====//
    string fus_id = vsp::AddGeom( "FUSELAGE" );
    TEST_ASSERT( fus_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  fus_id, "X_Rel_Location", "XForm", -9.0 ), -9.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  fus_id, "Z_Rel_Location", "XForm", -1.0 ), -1.0, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Test Default 3D DXF Export =====//
    ExportFile( "TestDXF_3D_API.dxf", vsp::SET_ALL, vsp::EXPORT_DXF );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    string geom_id = vsp::FindContainer( "Vehicle", 0 );

    //==== Test Default 2D 4 View DXF Export =====//
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "DimFlag", "DXFSettings" ), vsp::SET_2D ), vsp::SET_2D, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    ExportFile( "TestDXF_2D_4View_API.dxf", vsp::SET_ALL, vsp::EXPORT_DXF );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== 2D 1 View DXF Export ====//
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "ViewType", "DXFSettings" ), vsp::VIEW_1 ), vsp::VIEW_1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "TopLeftView", "DXFSettings" ), vsp::VIEW_BOTTOM ), vsp::VIEW_BOTTOM, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "TopLeftRotation", "DXFSettings" ), vsp::ROT_90 ), vsp::ROT_90, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    ExportFile( "TestDXF_2D_1View_API.dxf", vsp::SET_ALL, vsp::EXPORT_DXF );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== 2D 2 Horizontal View DXF Export ====//
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "ViewType", "DXFSettings" ), vsp::VIEW_2HOR ), vsp::VIEW_2HOR, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "TopRightView", "DXFSettings" ), vsp::VIEW_RIGHT ), vsp::VIEW_RIGHT, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "TopRightRotation", "DXFSettings" ), vsp::ROT_270 ), vsp::ROT_270, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    ExportFile( "TestDXF_2D_2HView_API.dxf", vsp::SET_ALL, vsp::EXPORT_DXF );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== 2D 2 Vertical View DXF Export ====//
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "ViewType", "DXFSettings" ), vsp::VIEW_2VER ), vsp::VIEW_2VER, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "BottomLeftView", "DXFSettings" ), vsp::VIEW_REAR ), vsp::VIEW_REAR, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "BottomLeftRotation", "DXFSettings" ), vsp::ROT_0 ), vsp::ROT_0, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    ExportFile( "TestDXF_2D_2VView_API.dxf", vsp::SET_ALL, vsp::EXPORT_DXF );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Open Each DXF File In A Viewer To Verify ====//
    printf( "-> COMPLETE: Open Each DXF File In A DXF Viewer To Verify \n" );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuite::TestSVGExport()
{
    printf( "APITestSuite::TestSVGExport()\n" );

    printf( "->Generating geometries...\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();

    //==== Add Wing Geom and set some parameters =====//
    string wing_id = vsp::AddGeom( "WING" );
    TEST_ASSERT( wing_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TotalSpan", "WingGeom", 30.0 ), 30.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "LECluster", "WingGeom", 0.1 ), 0.1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TECluster", "WingGeom", 2.0 ), 2.0, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Fuselage Geom and set some parameters =====//
    string fus_id = vsp::AddGeom( "FUSELAGE" );
    TEST_ASSERT( fus_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  fus_id, "X_Rel_Location", "XForm", -9.0 ), -9.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  fus_id, "Z_Rel_Location", "XForm", -1.0 ), -1.0, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    string geom_id = vsp::FindContainer( "Vehicle", 0 );

    //==== Manually Add Scale Bar ====//
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "LenUnit", "SVGSettings" ), vsp::LEN_IN ), vsp::LEN_IN, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "Scale", "SVGSettings" ), 30.0 ), 30.0, TEST_TOL );

    //==== Test Default 4 View SVG Export =====//
    ExportFile( "TestSVG_4View_API.svg", vsp::SET_ALL, vsp::EXPORT_SVG );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "--> 4 View SVG Export Saved To: TestSVG_4View_API.svg \n" );

    //==== 1 View SVG Export ====//
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "ViewType", "SVGSettings" ), vsp::VIEW_1 ), vsp::VIEW_1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "TopLeftView", "SVGSettings" ), vsp::VIEW_BOTTOM ), vsp::VIEW_BOTTOM, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "TopLeftRotation", "SVGSettings" ), vsp::ROT_0 ), vsp::ROT_0, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    ExportFile( "TestSVG_1View_API.svg", vsp::SET_ALL, vsp::EXPORT_SVG );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "--> 1 View SVG Export Saved To: TestSVG_1View_API.svg \n" );

    //==== 2 Horizontal View SVG Export ====//
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "ViewType", "SVGSettings" ), vsp::VIEW_2HOR ), vsp::VIEW_2HOR, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "TopRightView", "SVGSettings" ), vsp::VIEW_RIGHT ), vsp::VIEW_RIGHT, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "TopRightRotation", "SVGSettings" ), vsp::ROT_0 ), vsp::ROT_0, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    ExportFile( "TestSVG_2HView_API.svg", vsp::SET_ALL, vsp::EXPORT_SVG );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "--> 2 Horizontal View SVG Export Saved To: TestSVG_2HView_API.svg \n" );

    //==== 2 Vertical View SVG Export ====//
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "ViewType", "SVGSettings" ), vsp::VIEW_2VER ), vsp::VIEW_2VER, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "BottomLeftView", "SVGSettings" ), vsp::VIEW_FRONT ), vsp::VIEW_FRONT, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( geom_id, "BottomLeftRotation", "SVGSettings" ), vsp::ROT_0 ), vsp::ROT_0, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    ExportFile( "TestSVG_2VView_API.svg", vsp::SET_ALL, vsp::EXPORT_SVG );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "--> 2 Vertical View SVG Export Saved To: TestSVG_2VView_API.svg \n" );

    //==== Open Each SVG File In A Viewer To Verify ====//
    printf( "-> COMPLETE: Open Each SVG File In A SVG Viewer To Verify \n" );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuite::TestFacetExport()
{
    printf( "APITestSuite::TestFacetExport()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();

    //==== Add Pod Geom and set some parameters =====//
    string pod_id = vsp::AddGeom( "POD" );
    TEST_ASSERT( pod_id.c_str() != NULL );


    //==== Add SubSurfaces and set some parameters ====/
    string subsurf_ellipse_id = vsp::AddSubSurf( pod_id, vsp::SS_ELLIPSE, 0 );
    TEST_ASSERT( subsurf_ellipse_id.c_str() != NULL );

    string subsurf_rectangle_id = vsp::AddSubSurf( pod_id, vsp::SS_RECTANGLE, 0 );
    TEST_ASSERT( subsurf_rectangle_id.c_str() != NULL );

    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( subsurf_rectangle_id, "Center_U", "SS_Rectangle" ), 0.6 ), 0.6, TEST_TOL );


    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== CFDMesh Method Facet Export =====//
    vsp::SetComputationFileName( vsp::CFD_FACET_TYPE, "TestCFDMeshFacet_API.facet" );

    printf( "\tComputing CFDMesh..." );

    vsp::ComputeCFDMesh( vsp::SET_ALL, vsp::CFD_FACET_TYPE );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "COMPLETE\n" );

    //==== MeshGeom Method Facet Export =====//
    printf( "\tComputing MeshGeom..." );

    ExportFile( "TestMeshGeomFacet_API.facet", vsp::SET_ALL, vsp::EXPORT_FACET );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "COMPLETE\n" );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuite::TestSaveLoad()
{
    printf( "APITestSuite::TestSaveLoad()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Wing Geom and set some parameters =====//
    string wing_id = vsp::AddGeom( "WING" );
    TEST_ASSERT( wing_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TotalSpan", "WingGeom", 30.0 ), 30.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "LECluster", "WingGeom", 0.1 ), 0.1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TECluster", "WingGeom", 2.0 ), 2.0, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Fuselage Geom and set some parameters =====//
    string fus_id = vsp::AddGeom( "FUSELAGE" );
    TEST_ASSERT( fus_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  fus_id, "X_Rel_Location", "XForm", -9.0 ), -9.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  fus_id, "Z_Rel_Location", "XForm", -1.0 ), -1.0, TEST_TOL );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Save Vehicle to File ====//
    printf( "Saving VSP model\n" );
    string fname = "apitest_SaveLoad.vsp3";
    vsp::WriteVSPFile( fname );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Reset Geometry ====//
    printf( "Resetting VSP model to blank slate\n" );
    vsp::VSPRenew();
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Read Geometry From File ====//
    printf( "Reading model from: %s\n", fname.c_str() );
    vsp::ReadVSPFile( fname );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== List out all geoms ====//
    printf( "All geoms in Vehicle:\n" );
    vector<string> geoms = vsp::FindGeoms();
    for ( int i = 0; i < ( int ) geoms.size(); i++ )
    {
        printf( "Geom id: %s name: %s \n", geoms[i].c_str(), vsp::GetGeomName( geoms[i] ).c_str() );
    }
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuite::TestFEAMesh()
{
    printf( "APITestSuite::TestFEAMesh()\n" );

    // Make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Pod Geometry ====//
    printf( "\tAdding Geometry and Creating FeaStructure\n" );
    string pod_id = vsp::AddGeom( "POD" );
    TEST_ASSERT( pod_id.c_str() != NULL );

    double length = 15.0;

    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "X_Rel_Location", "XForm", 5.0 ), 5.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "X_Rel_Rotation", "XForm", 90 ), 90, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "Length", "Design", length ), length, TEST_TOL );

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Set Structure Units ====//
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( pod_id, "StructUnit", "FeaStructure" ), vsp::BFT_UNIT ), vsp::BFT_UNIT, TEST_TOL );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = vsp::AddFeaStruct( pod_id );
    TEST_ASSERT( struct_ind != -1 );

    //==== Create FeaMaterial ====//
    string mat_id = vsp::AddFeaMaterial();
    TEST_ASSERT( mat_id.c_str() != NULL );

    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( mat_id, "MassDensity", "FeaMaterial" ), 0.016 ), 0.016, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( mat_id, "ElasticModulus", "FeaMaterial" ), 2.0e6 ), 2.0e6, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( mat_id, "PoissonsRatio", "FeaMaterial" ), 0.4 ), 0.4, TEST_TOL );

    //==== Create FeaProperty ====//
    string prop_id = vsp::AddFeaProperty();
    TEST_ASSERT( prop_id.c_str() != NULL );

    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( prop_id, "FeaMaterialIndex", "FeaProperty" ), 4 ), 4, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( prop_id, "Thickness", "FeaProperty" ), 0.01 ), 0.01, TEST_TOL );

    //==== Adjust FeaMeshSettings ====//
    vsp::SetFeaMeshVal( pod_id, struct_ind, vsp::CFD_MAX_EDGE_LEN, 0.75 );
    vsp::SetFeaMeshVal( pod_id, struct_ind, vsp::CFD_MIN_EDGE_LEN, 0.2 );

    //==== Add Floor ====//
    string floor_id = vsp::AddFeaPart( pod_id, struct_ind, vsp::FEA_SLICE );
    TEST_ASSERT( floor_id.c_str() != NULL );

    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( floor_id, "IncludedElements", "FeaPart" ), vsp::FEA_PART_ELEMENT_TYPE::FEA_SHELL_AND_BEAM ), vsp::FEA_PART_ELEMENT_TYPE::FEA_SHELL_AND_BEAM, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( floor_id, "RelCenterLocation", "FeaPart" ), 0.34 ), 0.34, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( floor_id, "OrientationPlane", "FeaSlice" ), vsp::FEA_SLICE_TYPE::XZ_BODY ), vsp::FEA_SLICE_TYPE::XZ_BODY, TEST_TOL ); // XZ_BODY

    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( floor_id, "FeaPropertyIndex", "FeaPart" ), 2 ), 2, TEST_TOL );

    //==== Add Bulkead ====//
    string bulkhead_id = vsp::AddFeaPart( pod_id, struct_ind, vsp::FEA_SLICE );
    TEST_ASSERT( bulkhead_id.c_str() != NULL );

    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( bulkhead_id, "IncludedElements", "FeaPart" ), vsp::FEA_PART_ELEMENT_TYPE::FEA_SHELL_AND_BEAM ), vsp::FEA_PART_ELEMENT_TYPE::FEA_SHELL_AND_BEAM, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( bulkhead_id, "RelCenterLocation", "FeaPart" ), 0.15 ), 0.15, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( bulkhead_id, "OrientationPlane", "FeaSlice" ), vsp::FEA_SLICE_TYPE::CONST_U ), vsp::FEA_SLICE_TYPE::CONST_U, TEST_TOL );

    //==== Add Dome ====//
    string dome_id = vsp::AddFeaPart( pod_id, struct_ind, vsp::FEA_DOME );
    TEST_ASSERT( dome_id.c_str() != NULL );

    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( dome_id, "IncludedElements", "FeaPart" ), vsp::FEA_PART_ELEMENT_TYPE::FEA_SHELL ), vsp::FEA_PART_ELEMENT_TYPE::FEA_SHELL, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( dome_id, "X_Location", "FeaDome" ), 0.7 * length ), 0.7 * length, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( dome_id, "A_Radius", "FeaDome" ), 1.5 ), 1.5, TEST_TOL );

    //==== Add Stiffener ====//
    string stiffener_id = vsp::AddFeaPart( pod_id, struct_ind, vsp::FEA_SLICE );
    TEST_ASSERT( stiffener_id.c_str() != NULL );

    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( stiffener_id, "IncludedElements", "FeaPart" ), vsp::FEA_PART_ELEMENT_TYPE::FEA_BEAM ), vsp::FEA_PART_ELEMENT_TYPE::FEA_BEAM, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( stiffener_id, "RelCenterLocation", "FeaPart" ), 0.45 ), 0.45, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( stiffener_id, "OrientationPlane", "FeaSlice" ), vsp::FEA_SLICE_TYPE::CONST_U ), vsp::FEA_SLICE_TYPE::CONST_U, TEST_TOL );

    //==== Add LineArray ====//
    string line_array_id = vsp::AddFeaSubSurf( pod_id, struct_ind, vsp::SS_LINE_ARRAY );
    TEST_ASSERT( line_array_id.c_str() != NULL );

    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( line_array_id, "ConstLineType", "SS_LineArray" ), 1 ), 1, TEST_TOL ); // Constant W
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( line_array_id, "Spacing", "SS_LineArray" ), 0.25 ), 0.25, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( line_array_id, "StartLocation", "SS_LineArray" ), 0.125 ), 0.125, TEST_TOL );

    //==== Add Hole ====//
    string hole_id = vsp::AddFeaSubSurf( pod_id, struct_ind, vsp::SS_RECTANGLE );
    TEST_ASSERT( hole_id.c_str() != NULL );

    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( hole_id, "IncludedElements", "SS_Rectangle" ), vsp::FEA_PART_ELEMENT_TYPE::FEA_BEAM ), vsp::FEA_PART_ELEMENT_TYPE::FEA_BEAM, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( hole_id, "Center_U", "SS_Rectangle" ), 0.65 ), 0.65, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( hole_id, "Center_W", "SS_Rectangle" ), 0.5 ), 0.5, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( hole_id, "U_Length", "SS_Rectangle" ), 0.1 ), 0.1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( hole_id, "W_Length", "SS_Rectangle" ), 0.1 ), 0.1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( vsp::FindParm( hole_id, "Test_Type", "SS_Rectangle" ), vsp::INSIDE ), vsp::INSIDE, TEST_TOL );

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Save Vehicle to File ====//
    string fname = "apitest_FEAMesh.vsp3";
    vsp::WriteVSPFile( fname );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    string print_str = "\tVehicle Saved to " + fname + " \n";
    printf( print_str.c_str() );

    //=== Set Export File Name ===//
    string export_name = "apitest_FEAMesh_calculix.dat";
    vsp::SetFeaMeshFileName( pod_id, struct_ind, vsp::FEA_CALCULIX_FILE_NAME, export_name );
    print_str = "\tExport File Name Set to " + export_name + " \n";
    printf( print_str.c_str() );

    //==== Generate FEA Mesh and Export ====//
    printf( "\tGenerating FEA Mesh\n" );
    vsp::ComputeFeaMesh( pod_id, struct_ind, vsp::FEA_CALCULIX_FILE_NAME );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );

    printf( "COMPLETE\n" );
}