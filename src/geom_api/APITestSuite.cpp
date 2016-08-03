//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Geom_API.h"
#include "APITestSuite.h"
#include <stdio.h>
#include <float.h>
#include "APIDefines.h"

//Default tolerance to use for tests.  Most calculations are done as doubles and choosing single precision FLT_MIN gives some allowance for precision stackup in calculations
#define TEST_TOL FLT_MIN

//==== Test Geometry Creation ====//
void APITestSuite::CheckSetup()
{
    
    printf("APITestSuite::CheckSetup()\n");
    vsp::VSPCheckSetup();
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "VSPCheckSetup ERROR" );    // if this happens something is very wrong
    }
}
//==== Test Geometry Creation ====//
void APITestSuite::CreateGeometry()
{
    printf("APITestSuite::CreateGeometry()\n");
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    vector<string> types = vsp::GetGeomTypes( );
    TEST_ASSERT( types.size() != 0 );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf("\t[geom_id]\t[geom_name]\n");
    for (unsigned int i_geom_type = 0; i_geom_type<types.size(); i_geom_type++ )
    {
        //==== Create geometry =====//
        string geom_id = vsp::AddGeom( types[i_geom_type] );
        printf("\t%s", geom_id.c_str());
        TEST_ASSERT( geom_id.c_str() != NULL );
        TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

        //==== Set Name ====//
        string geom_name = "TestGeom_" + types[i_geom_type];
        vsp::SetGeomName( geom_id, geom_name );
        printf("\t%s\n", geom_name.c_str());
        TEST_ASSERT( vsp::GetGeomName( geom_id ) == geom_name )
        TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

        //==== Check to make sure it got added to the list ====//
        vector<string> geoms = vsp::FindGeoms();
        TEST_ASSERT( geoms.size() == i_geom_type+1 );
        TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    }
    printf("\n");

    //==== Save Vehicle to File ====//
    string fname = "apitest_CreateGeometry.vsp3";
    vsp::WriteVSPFile( fname );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
}

void APITestSuite::ChangePodParams()
{
    printf("APITestSuite::ChangePodParams()\n");
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
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "FineRatio", "Design", finess_val ), finess_val, TEST_TOL) ;
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change X Location  with ONE step method: SetParmValUpdate()
    double x_loc_val = 3.0;
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "X_Rel_Location", "XForm", x_loc_val ), x_loc_val, TEST_TOL) ;
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change Y Location  with ONE step method: SetParmValUpdate()
    double y_loc_val = 1.0;
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "Y_Rel_Location", "XForm", y_loc_val ), y_loc_val, TEST_TOL) ;
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change Z Location  with ONE step method: SetParmValUpdate()
    double z_loc_val = 4.2;
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "Z_Rel_Location", "XForm", z_loc_val ), z_loc_val, TEST_TOL) ;
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
    printf("APITestSuite::CopyPasteGeometry()\n");
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
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  first_pod_id, "Length", "Design", 7.0 ), 7.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( first_pod_id, "FineRatio", "Design", 10.0 ), 10.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( first_pod_id, "X_Rel_Location", "XForm", 3.0 ), 3.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( first_pod_id, "Y_Rel_Location", "XForm", 1.0 ), 1.0, TEST_TOL);
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
    TEST_ASSERT_DELTA( vsp::SetParmVal( second_pod_id, "Y_Rel_Location", "XForm", 0.0 ), 0.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmVal( second_pod_id, "Z_Rel_Location", "XForm", 1.0 ), 1.0, TEST_TOL);
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
    printf("APITestSuite::CheckAnalysisMgr()\n");
    unsigned int n_analysis = (unsigned int)vsp::GetNumAnalysis();
    std::vector < std::string > analysis_names = vsp::ListAnalysis();
    printf("    Analyses found: %d\n",n_analysis);
    printf("\t[analysis_name]\n");
    printf("\t\t%-20s%s\t%s\n","[input_name]","[type]","[#]");
    for ( unsigned int i_analysis = 0; i_analysis<n_analysis; i_analysis++)
    {
        // print out name
        printf("\t%s\n",analysis_names[i_analysis].c_str());

        // get input names
        vector < string > input_names = vsp::GetAnalysisInputNames( analysis_names[i_analysis] );
        for ( unsigned int i_input_name = 0; i_input_name<input_names.size(); i_input_name++)
        {
            int current_input_type = vsp::GetAnalysisInputType(analysis_names[i_analysis],input_names[i_input_name]);
            int current_input_num_data = vsp::GetNumAnalysisInputData(analysis_names[i_analysis],input_names[i_input_name]);

            // print out name and type enumeration
            printf("\t\t%-20s%d\t\t%d",input_names[i_input_name].c_str(), current_input_type, current_input_num_data);

            // ASSERT if an invalid type is found
            TEST_ASSERT( current_input_type!= vsp::INVALID_TYPE);

            printf("\n");
        }

    }
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuite::TestAnalysesWithPod()
{
    printf("APITestSuite::TestAnalysesWithPod()\n");

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Pod Geom and set some parameters =====//
    string pod_id = vsp::AddGeom( "POD");
    TEST_ASSERT( pod_id.c_str() != NULL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Change Pod Parameters (name, length, finess ratio, y location, x location, symetry) ====//
    string pod_name = "Pod_Test";
    vsp::SetGeomName( pod_id, pod_name );
    //    test that the parameters got set within the TEST_TOL tolerance
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  pod_id, "Length", "Design", 7.0 ), 7.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "FineRatio", "Design", 10.0 ), 10.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "X_Rel_Location", "XForm", 3.0 ), 3.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "Y_Rel_Location", "XForm", 1.0 ), 1.0, TEST_TOL);
    vsp::SetParmValUpdate( pod_id, "Sym_Planar_Flag", "Sym", vsp::SYM_XZ  );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Save Vehicle to File ====//
    string fname = "apitest_TestAnalysesWithPod.vsp3";
    vsp::WriteVSPFile( fname );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Analysis: CompGeom ====//
    string analysis_name = "CompGeom";
    printf("\t%s\n",analysis_name.c_str());

    // Set defaults
    vsp::SetAnalysisInputDefaults(analysis_name);

    // list inputs, type, and current values
    PrintAnalysisInputs(analysis_name);

    // Execute
    string results_id = vsp::ExecAnalysis(analysis_name);

    // Get & Display Results
    PrintResults(results_id);
}

void APITestSuite::TestDXFExport()
{
    printf( "APITestSuite::TestDXFExport()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Wing Geom and set some parameters =====//
    string wing_id = vsp::AddGeom( "WING" );
    TEST_ASSERT( wing_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TotalSpan", "WingGeom", 30.0 ), 30.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "LECluster", "WingGeom", 0.0 ), 0.0, TEST_TOL );
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

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuite::TestVSPAeroComputeGeom()
{
    printf("APITestSuite::TestVSPAeroComputeGeom()\n");

    // make sure setup works
    vsp::VSPCheckSetup();        //TODO check that vspaero.exe is found
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Wing Geom and set some parameters =====//
    string wing_id = vsp::AddGeom( "WING");
    TEST_ASSERT( wing_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TotalSpan", "WingGeom", 17.0 ), 17.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "LECluster", "WingGeom", 0.0 ), 0.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TECluster", "WingGeom", 2.0 ), 2.0, TEST_TOL);
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    
    //==== Save Vehicle to File ====//
    m_vspfname_for_vspaerotests = "apitest_TestVSPAeroWithWing.vsp3";
    vsp::WriteVSPFile( m_vspfname_for_vspaerotests );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";
    printf("\t%s\n",analysis_name.c_str());

    // Set defaults
    vsp::SetAnalysisInputDefaults(analysis_name);

    // list inputs, type, and current values
    PrintAnalysisInputs(analysis_name);

    // Execute
    string results_id = vsp::ExecAnalysis(analysis_name);

    // Get & Display Results
    PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuite::TestVSPAeroComputeGeomPanel()
{
    printf("APITestSuite::TestVSPAeroComputeGeomPanel()\n");

    // make sure setup works
    vsp::VSPCheckSetup();        //TODO check that vspaero.exe is found
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Wing Geom and set some parameters =====//
    string wing_id = vsp::AddGeom( "WING");
    TEST_ASSERT( wing_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TotalSpan", "WingGeom", 17.0 ), 17.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "LECluster", "WingGeom", 0.0 ), 0.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TECluster", "WingGeom", 2.0 ), 2.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "SectTess_U", "XSec_1", 25 ), 25, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "InCluster", "XSec_1", 0.1 ), 0.1, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "OutCluster", "XSec_1", 0.0 ), 0.0, TEST_TOL);
    string pod_id = vsp::AddGeom( "POD");
    TEST_ASSERT( pod_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  pod_id, "X_Rel_Location", "XForm", 0.0 ), 0.0, TEST_TOL);
    string disk_id = vsp::AddGeom( "DISK");
    TEST_ASSERT( disk_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  disk_id, "Diameter", "Design", 3.0 ), 3.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  disk_id, "Y_Rel_Location", "XForm", 3.5 ), 3.5, TEST_TOL);
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Setup export filenames ====//
    // Execution of one of these methods is required to propperly set the export filenames for creation of vspaero input files and execution commands
    m_vspfname_for_vspaerotests_panel = "apitest_TestVSPAeroWithWingPanelMethod.vsp3";
    vsp::SetVSP3FileName( m_vspfname_for_vspaerotests_panel );  // this still needs to be done even if a call to WriteVSPFile is made
    //vsp::ReadVSPFile( m_vspfname_for_vspaerotests_panel);   // reading the vsp file calls vehicle::resetFilenames() function to reset ALL filenames to their default built off of the vsp filename
    //vsp::SetComputationFileName(vsp::COMPUTATION_FILE_TYPE::VSPAERO_PANEL_TRI_TYPE,"apitest_TestVSPAeroWithWingPanelMethod");

    //==== Save Vehicle to File ====//
    vsp::WriteVSPFile( vsp::GetVSPFileName(), vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";
    printf("\t%s\n",analysis_name.c_str());

    // Set defaults
    vsp::SetAnalysisInputDefaults(analysis_name);
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Set to panel method
    std::vector< int > analysis_method; analysis_method.push_back( vsp::VSPAERO_ANALYSIS_METHOD::PANEL );
    vsp::SetIntAnalysisInput(analysis_name, "AnalysisMethod", analysis_method, 0);
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    PrintAnalysisInputs(analysis_name);

    // Execute
    string results_id = vsp::ExecAnalysis(analysis_name);
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuite::TestVSPAeroSinglePointPanel()
{
    printf("APITestSuite::TestVSPAeroSinglePointPanel()\n");

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroComputeGeom
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests_panel );
    if ( m_vspfname_for_vspaerotests_panel == string() )
    {
        TEST_FAIL("m_vspfname_for_vspaerotests_panel = NULL, need to run: APITestSuite::TestVSPAeroComputeGeomPanel");
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL("m_vspfname_for_vspaerotests_panel failed to open");
        return;
    }

    //==== Analysis: VSPAERO Single Point ====//
    string analysis_name = "VSPAEROSinglePoint";
    printf("\t%s\n",analysis_name.c_str());
    // Set defaults
    vsp::SetAnalysisInputDefaults(analysis_name);
    
    // Change some input values
    //    Reference geometry set
    std::vector< int > geom_set; geom_set.push_back(0);
    vsp::SetIntAnalysisInput(analysis_name, "GeomSet", geom_set, 0);
    // Set to panel method
    std::vector< int > analysis_method; analysis_method.push_back( vsp::VSPAERO_ANALYSIS_METHOD::PANEL );
    vsp::SetIntAnalysisInput(analysis_name, "AnalysisMethod", analysis_method, 0);
    // Force creation of new setup file
    std::vector< int > force_new_setup_file; force_new_setup_file.push_back( 1 );
    vsp::SetIntAnalysisInput(analysis_name, "ForceNewSetupfile", force_new_setup_file, 0);
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back(10);
    vsp::SetDoubleAnalysisInput(analysis_name, "Sref", sref, 0);
    std::vector< double > bref; bref.push_back(17);
    vsp::SetDoubleAnalysisInput(analysis_name, "bref", bref, 0);
    std::vector< double > cref; cref.push_back(3);
    vsp::SetDoubleAnalysisInput(analysis_name, "cref", cref, 0);
    std::vector< int > ref_flag; ref_flag.push_back(3);
    vsp::SetIntAnalysisInput(analysis_name, "RefFlag", ref_flag, 0);
    //    freestream parameters
    std::vector< double > alpha; alpha.push_back(5);
    vsp::SetDoubleAnalysisInput(analysis_name, "Alpha", alpha, 0);
    std::vector< double > beta; beta.push_back(2.5);
    vsp::SetDoubleAnalysisInput(analysis_name, "Beta", beta, 0);
    std::vector< double > mach; mach.push_back(0.1);
    vsp::SetDoubleAnalysisInput(analysis_name, "Mach", mach, 0);
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    PrintAnalysisInputs(analysis_name);

    // Execute
    string results_id = vsp::ExecAnalysis(analysis_name);
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    PrintResults( results_id );
    
    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuite::TestVSPAeroSinglePoint()
{
    printf("APITestSuite::TestVSPAeroSinglePoint()\n");

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroComputeGeom
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL("m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom");
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL("m_vspfname_for_vspaerotests failed to open");
        return;
    }

    //==== Analysis: VSPAERO Single Point ====//
    string analysis_name = "VSPAEROSinglePoint";
    printf("\t%s\n",analysis_name.c_str());
    // Set defaults
    vsp::SetAnalysisInputDefaults(analysis_name);
    
    // Change some input values
    //    Reference geometry set
    std::vector< int > geom_set; geom_set.push_back(0);
    vsp::SetIntAnalysisInput(analysis_name, "GeomSet", geom_set, 0);
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back(10);
    vsp::SetDoubleAnalysisInput(analysis_name, "Sref", sref, 0);
    std::vector< double > bref; bref.push_back(17);
    vsp::SetDoubleAnalysisInput(analysis_name, "bref", bref, 0);
    std::vector< double > cref; cref.push_back(3);
    vsp::SetDoubleAnalysisInput(analysis_name, "cref", cref, 0);
    std::vector< int > ref_flag; ref_flag.push_back(3);
    vsp::SetIntAnalysisInput(analysis_name, "RefFlag", ref_flag, 0);
    //    freestream parameters
    std::vector< double > alpha; alpha.push_back(5);
    vsp::SetDoubleAnalysisInput(analysis_name, "Alpha", alpha, 0);
    std::vector< double > beta; beta.push_back(2.5);
    vsp::SetDoubleAnalysisInput(analysis_name, "Beta", beta, 0);
    std::vector< double > mach; mach.push_back(0.1);
    vsp::SetDoubleAnalysisInput(analysis_name, "Mach", mach, 0);
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    PrintAnalysisInputs(analysis_name);

    // Execute
    string results_id = vsp::ExecAnalysis(analysis_name);
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    PrintResults( results_id );
    
    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuite::TestVSPAeroSinglePointStab()
{
    printf("APITestSuite::TestVSPAeroSinglePointStab()\n");

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroComputeGeom
    m_vspfname_for_vspaerotests = "apitest_TestVSPAeroWithWing.vsp3";
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL("m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom");
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL("m_vspfname_for_vspaerotests failed to open");
        return;
    }

    //==== Analysis: VSPAERO Single Point  stabilityFlag = TRUE ====//
    string analysis_name = "VSPAEROSinglePoint";
    printf("\t%s\n",analysis_name.c_str());
    // Set defaults
    vsp::SetAnalysisInputDefaults(analysis_name);
    
    // Change some input values
    //    Reference geometry set
    std::vector< int > geom_set; geom_set.push_back(0);
    vsp::SetIntAnalysisInput(analysis_name, "GeomSet", geom_set, 0);
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back(10);
    vsp::SetDoubleAnalysisInput(analysis_name, "Sref", sref, 0);
    std::vector< double > bref; bref.push_back(17);
    vsp::SetDoubleAnalysisInput(analysis_name, "bref", bref, 0);
    std::vector< double > cref; cref.push_back(3);
    vsp::SetDoubleAnalysisInput(analysis_name, "cref", cref, 0);
    std::vector< int > ref_flag; ref_flag.push_back(3);
    vsp::SetIntAnalysisInput(analysis_name, "RefFlag", ref_flag, 0);
    //    freestream parameters
    std::vector< double > alpha; alpha.push_back(4.0);
    vsp::SetDoubleAnalysisInput(analysis_name, "Alpha", alpha, 0);
    std::vector< double > beta; beta.push_back(-3.0);
    vsp::SetDoubleAnalysisInput(analysis_name, "Beta", beta, 0);
    std::vector< double > mach; mach.push_back(0.4);
    vsp::SetDoubleAnalysisInput(analysis_name, "Mach", mach, 0);
    //     Case Setup
    std::vector< int > stabilityCalcFlag; stabilityCalcFlag.push_back(1);
    vsp::SetIntAnalysisInput(analysis_name, "StabilityCalcFlag", stabilityCalcFlag, 0);
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    PrintAnalysisInputs(analysis_name);

    // Execute
    string results_id = vsp::ExecAnalysis(analysis_name);
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    PrintResults( results_id );
    
    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuite::TestVSPAeroSweep()
{
    printf("APITestSuite::TestVSPAeroSweep()\n");
    
    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroComputeGeom
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL("m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom");
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL("m_vspfname_for_vspaerotests failed to open");
        return;
    }
    
    //==== Analysis: VSPAERO Sweep ====//
    string analysis_name = "VSPAEROSweep";
    printf("\t%s\n",analysis_name.c_str());
    // Set defaults
    vsp::SetAnalysisInputDefaults(analysis_name);
    
    // Change some input values
    //    Reference geometry set
    std::vector< int > geom_set; geom_set.push_back(0);
    vsp::SetIntAnalysisInput(analysis_name, "GeomSet", geom_set, 0);
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back(10);
    vsp::SetDoubleAnalysisInput(analysis_name, "Sref", sref, 0);
    std::vector< double > bref; bref.push_back(17);
    vsp::SetDoubleAnalysisInput(analysis_name, "bref", bref, 0);
    std::vector< double > cref; cref.push_back(3);
    vsp::SetDoubleAnalysisInput(analysis_name, "cref", cref, 0);
    std::vector< int > ref_flag; ref_flag.push_back(3);
    vsp::SetIntAnalysisInput(analysis_name, "RefFlag", ref_flag, 0);
    //    freestream parameters
    //        Alpha
    std::vector< double > alpha_start; alpha_start.push_back(1);
    vsp::SetDoubleAnalysisInput(analysis_name, "AlphaStart", alpha_start, 0);
    std::vector< double > alpha_end; alpha_end.push_back(10);
    vsp::SetDoubleAnalysisInput(analysis_name, "AlphaEnd", alpha_end, 0);
    std::vector< int > alpha_npts; alpha_npts.push_back(3);
    vsp::SetIntAnalysisInput(analysis_name, "AlphaNpts", alpha_npts, 0);
    //        Beta
    std::vector< double > beta_start; beta_start.push_back(0);
    vsp::SetDoubleAnalysisInput(analysis_name, "BetaStart", beta_start, 0);
    std::vector< double > beta_end; beta_end.push_back(5);
    vsp::SetDoubleAnalysisInput(analysis_name, "BetaEnd", beta_end, 0);
    std::vector< int > beta_npts; beta_npts.push_back(3);
    vsp::SetIntAnalysisInput(analysis_name, "BetaNpts", beta_npts, 0);
    //        Mach
    std::vector< double > mach_start; mach_start.push_back(0.05);
    vsp::SetDoubleAnalysisInput(analysis_name, "MachStart", mach_start, 0);
    std::vector< double > mach_end; mach_end.push_back(0.15);
    vsp::SetDoubleAnalysisInput(analysis_name, "MachEnd", mach_end, 0);
    std::vector< int > mach_npts; mach_npts.push_back(3);
    vsp::SetIntAnalysisInput(analysis_name, "MachNpts", mach_npts, 0);
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    PrintAnalysisInputs(analysis_name);

    // Execute
    string results_id = vsp::ExecAnalysis(analysis_name);
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    PrintResults( results_id );
    
    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuite::PrintAnalysisInputs(const string analysis_name)
{
    printf("\t\t%-20s%s\t%s\t%s\n","[input_name] ","[type]","[#]","[current values-->]");

    vector < string > input_names = vsp::GetAnalysisInputNames( analysis_name );
    for ( unsigned int i_input_name = 0; i_input_name<input_names.size(); i_input_name++)
    {
        // print out type and number of data entries
        int current_input_type = vsp::GetAnalysisInputType(analysis_name,input_names[i_input_name]);
        unsigned int current_input_num_data = (unsigned int) vsp::GetNumAnalysisInputData(analysis_name,input_names[i_input_name]);
        printf("\t\t%-20s%d\t\t%d",input_names[i_input_name].c_str(), current_input_type, current_input_num_data);

        // print out the current value (this needs to handle different types and vector lengths
        printf("\t");
        for ( unsigned int i_val = 0; i_val<current_input_num_data; i_val++)
        {
            switch(current_input_type) 
            {
            case vsp::RES_DATA_TYPE::INT_DATA :
                {
                    vector<int> current_int_val = vsp::GetIntAnalysisInput(analysis_name,input_names[i_input_name],i_val);
                    for ( unsigned int j_val=0; j_val<current_int_val.size(); j_val++)
                    {
                        printf("%d ",current_int_val[j_val]);
                    }
                    break;
                }
            case vsp::RES_DATA_TYPE::DOUBLE_DATA :
                {
                    vector<double> current_double_val = vsp::GetDoubleAnalysisInput(analysis_name,input_names[i_input_name],i_val);
                    for ( unsigned int j_val=0; j_val<current_double_val.size(); j_val++)
                    {
                        printf("%f ",current_double_val[j_val]);
                    }
                    break;
                }
            case vsp::RES_DATA_TYPE::STRING_DATA :
                {
                    vector<string> current_string_val = vsp::GetStringAnalysisInput(analysis_name,input_names[i_input_name],i_val);
                    for ( unsigned int j_val=0; j_val<current_string_val.size(); j_val++)
                    {
                        printf("%s ",current_string_val[j_val].c_str());
                    }
                    break;
                }
            case vsp::RES_DATA_TYPE::VEC3D_DATA :
                {
                    vector<vec3d> current_vec3d_val = vsp::GetVec3dAnalysisInput(analysis_name,input_names[i_input_name],i_val);
                    for ( unsigned int j_val=0; j_val<current_vec3d_val.size(); j_val++)
                    {
                        printf("%f,%f,%f ",current_vec3d_val[j_val].x(),current_vec3d_val[j_val].y(),current_vec3d_val[j_val].z());
                    }
                    break;
                }
            default:
                {
                    TEST_FAIL("INVALID TYPE");
                    break;
                }
            }    //end switch
        }    // end for
        
        printf("\n");
    }
}

void APITestSuite::PrintResults(const vector < string > &results_id_vec )
{
    for ( unsigned int i = 0; i < results_id_vec.size(); i++ )
    {
        PrintResults( results_id_vec[i] );
    }
}

void APITestSuite::PrintResults(const string &results_id)
{
    printf("\t\t%-20s%s\t%s\t%s\n","[result_name]","[type]","[#]","[current values-->]");

    vector<string> results_names = vsp::GetAllDataNames(results_id);
    for ( unsigned int i_result_name = 0; i_result_name< results_names.size(); i_result_name++)
    {
        int current_result_type = vsp::GetResultsType(results_id,results_names[i_result_name]);
        unsigned int current_result_num_data = (unsigned int)vsp::GetNumData(results_id,results_names[i_result_name]);
        printf("\t\t%-20s%d\t\t%d",results_names[i_result_name].c_str(), current_result_type, current_result_num_data);
        // print out the current value (this needs to handle different types and vector lengths
        printf("\t");
        for ( unsigned int i_val = 0; i_val<current_result_num_data; i_val++)
        {
            switch(current_result_type)
            {
            case vsp::RES_DATA_TYPE::INT_DATA :
            {
                vector<int> current_int_val = vsp::GetIntResults(results_id,results_names[i_result_name],i_val);
                for ( unsigned int j_val=0; j_val<current_int_val.size(); j_val++)
                {
                    printf("%d ",current_int_val[j_val]);
                }
                break;
            }
            case vsp::RES_DATA_TYPE::DOUBLE_DATA :
            {
                vector<double> current_double_val = vsp::GetDoubleResults(results_id,results_names[i_result_name],i_val);
                for ( unsigned int j_val=0; j_val<current_double_val.size(); j_val++)
                {
                    printf("%f ",current_double_val[j_val]);
                }
                break;
            }
            case vsp::RES_DATA_TYPE::STRING_DATA :
            {
                vector<string> current_string_val = vsp::GetStringResults(results_id,results_names[i_result_name],i_val);
                for ( unsigned int j_val=0; j_val<current_string_val.size(); j_val++)
                {
                    printf("%s ",current_string_val[j_val].c_str());
                }
                //Recursive call if results vector found
                if( strcmp(results_names[i_result_name].c_str(),"ResultsVec")==0)
                {
                    PrintResults( vsp::GetStringResults( results_id, "ResultsVec", 0 ) );
                }
                break;
            }
            case vsp::RES_DATA_TYPE::VEC3D_DATA :
            {
                vector<vec3d> current_vec3d_val = vsp::GetVec3dResults(results_id,results_names[i_result_name],i_val);
                for ( unsigned int j_val=0; j_val<current_vec3d_val.size(); j_val++)
                {
                    printf("%f,%f,%f ",current_vec3d_val[j_val].x(),current_vec3d_val[j_val].y(),current_vec3d_val[j_val].z());
                }
                break;
            }
            default:
            {
                TEST_FAIL("INVALID TYPE");
                break;
            }
            }    //end switch

        }    // end for

        printf("\n");
    }
}
