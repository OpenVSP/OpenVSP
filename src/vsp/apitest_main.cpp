//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Leak Detection http://www.codeproject.com/KB/applications/visualleakdetector.aspx
//#include "vld.h"

#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "main.h"
#include "VSP_Geom_API.h"
#include "APITestSuite.h"
#include "APITestSuiteMassProp.h"
#include "APITestSuiteVSPAERO.h"
#include "APITestSuiteParasiteDrag.h"
#include "APITestSuiteCFDMesh.h"
#include "UtilTestSuite.h"


void vsp_exit()
{

    exit( 0 );
}

bool run_tests()
{
    // Add desired suites to parent suite
    Test::Suite ts;
    ts.add(std::unique_ptr<Test::Suite>(new UtilTestSuite));
    ts.add(std::unique_ptr<Test::Suite>(new APITestSuite));    //This line can be copied to add new test suites
    ts.add(std::unique_ptr<Test::Suite>(new APITestSuiteMassProp));
    ts.add(std::unique_ptr<Test::Suite>(new APITestSuiteParasiteDrag));
    ts.add(std::unique_ptr<Test::Suite>(new APITestSuiteVSPAERO));
    ts.add(std::unique_ptr<Test::Suite>(new APITestSuiteCFDMesh));
    
    // Test Suite run parameters
    Test::TextOutput output(Test::TextOutput::Verbose);
    bool cont_after_fail = true; //TRUE continues test execution after failure

    // Run the test and return success or failure
    return ts.run(output, cont_after_fail) ? EXIT_SUCCESS : EXIT_FAILURE;
}

//========================================================//
//========================================================//
//========================= Main =========================//
int main( int argc, char** argv )
{
//==== Use CPPTest =====//
    run_tests();
    printf("\n\n");

//==== Use Case 1 ====//
    printf( "\n//==== Use Case 1 ====//\n");
    printf( "Description: Create/Delete/Copy/Paste Geometry\n" );

    //==== Create/Delete/Copy/Paste Geometry ====//
    printf( "Checking Setup\n" );
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Print Out All Available Geom Types ====//
    vector<string> types = vsp::GetGeomTypes( );
    printf( "All available Geom types.\n" );
    for ( int i = 0 ; i < ( int )types.size() ; i++ )
    {
        printf( "Type %d = %s \n", i, types[i].c_str() );
    }
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Add Fuselage Geom =====//
    printf( "Adding a fuselage\n" );
    string fuse_id = vsp::AddGeom( "FUSELAGE" );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Add Pod Geom =====//
    printf( "Adding a pod\n" );
    string pod_id = vsp::AddGeom( "POD", fuse_id );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );
    
    //==== Edit the Pod ====//
    printf( "Editing the pod\n" );
    //==== Set Name ====//
    vsp::SetGeomName( pod_id, "Pod" );
    //==== Change Length ====//
    string len_id = vsp::GetParm( pod_id, "Length", "Design" );
    vsp::SetParmValUpdate( len_id, 7.0 );
    //==== Change Finess Ration
    vsp::SetParmValUpdate( pod_id, "FineRatio", "Design", 10.0 );
    //==== Change Y Location ====//
    string y_loc_id = vsp::GetParm( pod_id, "Y_Rel_Location", "XForm" );
    vsp::SetParmValUpdate( y_loc_id, 1.0 );
    //==== Change X Location ====//
    vsp::SetParmValUpdate( pod_id, "X_Rel_Location", "XForm", 3.0 );
    //==== Change Symmetry =====//
    string sym_flag_id = vsp::GetParm( pod_id, "Sym_Planar_Flag", "Sym" );
    vsp::SetParmValUpdate( sym_flag_id, vsp::SYM_XZ  );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );
    
    //==== Copy/Paste Pod Geom =====//
    printf( "Copy/Paste pod\n" );
    vsp::CopyGeomToClipboard( pod_id );
    vsp::PasteGeomClipboard( fuse_id );         // Make fuse_id parent
    //==== Set Name ====//
    vsp::SetGeomName( pod_id, "Original_Pod" );
    string second_pod_id = vsp::FindGeom( "Pod", 0 );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Change Y Location ====//
    printf( "Edit second pod\n" );
    vsp::SetParmVal( second_pod_id, "Sym_Planar_Flag", "Sym", 0 );
    vsp::SetParmVal( second_pod_id, "Y_Rel_Location", "XForm", 0.0 );
    vsp::SetParmVal( second_pod_id, "Z_Rel_Location", "XForm", 1.0 );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );
    
    //==== List out all geoms ====//
    printf( "All geoms in Vehicle:\n" );
    vector<string> geoms = vsp::FindGeoms();
    for ( int i = 0; i < ( int ) geoms.size(); i++ )
    {
        printf( "Geom id: %s name: %s \n", geoms[i].c_str(), vsp::GetGeomName( geoms[i] ).c_str() );
    }
    vsp::ErrorMgr.PopErrorAndPrint( stdout );
    
    //==== Save Vehicle to File ====//
    string fname = "apitest.vsp3";
    vsp::WriteVSPFile( fname );
    printf( "Saved file to: %s\n",fname.c_str() );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

//==== Use Case 2 ====//
    printf( "\n//==== Use Case 2 ====//\n");
    printf( "Description: Fuselage editing\n" );
    
    //==== Reset Geometry ====//
    printf( "Resetting VSP model to blank slate\n" );
    vsp::VSPRenew();
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== List out all geoms ====//
    printf( "All geoms in Vehicle:\n" );
    geoms = vsp::FindGeoms();
    for ( int i = 0; i < ( int ) geoms.size(); i++ )
    {
        printf( "Geom id: %s name: %s \n", geoms[i].c_str(), vsp::GetGeomName( geoms[i] ).c_str() );
    }
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Add Fuselage Geom =====//
    printf( "Adding a fuselage\n" );
    fuse_id = vsp::AddGeom( "FUSELAGE" );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== List out all geoms ====//
    printf( "All geoms in Vehicle:\n" );
    geoms = vsp::FindGeoms();
    for ( int i = 0; i < ( int ) geoms.size(); i++ )
    {
        printf( "Geom id: %s name: %s \n", geoms[i].c_str(), vsp::GetGeomName( geoms[i] ).c_str() );
    }
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Get XSec Surf ID ====//
    printf( "Change one of the cross-sections to use XS_SUPER_ELLIPSE type\n" );
    string xsurf_id = vsp::GetXSecSurf( fuse_id, 0 );
    //==== Change Type of First XSec ====//
    vsp::ChangeXSecShape( xsurf_id, 0, vsp::XS_SUPER_ELLIPSE );
    //==== Change Type of First XSec ====//
    string xsec_id = vsp::GetXSec( xsurf_id, 0 );
    string width_id  = vsp::GetXSecParm( xsec_id, "Super_Width" );
    string height_id = vsp::GetXSecParm( xsec_id, "Super_Height" );
    vsp::SetParmVal( width_id, 4.0 );
    vsp::SetParmVal( height_id, 2.0 );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Copy Cros-Section to Clipboard ====//
    printf( "Copy/Paste cross-section\n" );
    vsp::CopyXSec( fuse_id, 0 );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );
    //==== Paste Cross-Section ====///
    vsp::PasteXSec( fuse_id, 1 );
    vsp::PasteXSec( fuse_id, 2 );
    vsp::PasteXSec( fuse_id, 3 );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //===== Change Type To File XSec ====//
    printf( "Change one of the cross-sections to use XS_FILE_FUSE type\n" );
    vsp::ChangeXSecShape( xsurf_id, 0, vsp::XS_FILE_FUSE );
    string file_xsec_id = vsp::GetXSec( xsurf_id, 0 );
    //===== Build Point Vec ====//
    vector< vec3d > pnt_vec;
    pnt_vec.push_back( vec3d( 0.0, 0.0, 2.0 ) );
    pnt_vec.push_back( vec3d( 0.0, 1.0, 0.0 ) );
    pnt_vec.push_back( vec3d( 0.0, 0.0, -2.0 ) );
    pnt_vec.push_back( vec3d( 0.0, -1.0, 0.0 ) );
    pnt_vec.push_back( vec3d( 0.0, 0.0, 2.0 ) );
    //===== Load Point Into XSec ====//
    vsp::SetXSecPnts( file_xsec_id, pnt_vec );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== List out all geoms ====//
    printf( "All geoms in Vehicle:\n" );
    geoms = vsp::FindGeoms();
    for ( int i = 0; i < ( int ) geoms.size(); i++ )
    {
        printf( "Geom id: %s name: %s \n", geoms[i].c_str(), vsp::GetGeomName( geoms[i] ).c_str() );
    }
    vsp::ErrorMgr.PopErrorAndPrint( stdout );
    
//==== Use Case 3 ====//
    printf( "\n//==== Use Case 3 ====//\n");
    printf( "Description: Read in first-case file.\n" );
    
    //==== Reset Geometry ====//
    printf( "Resetting VSP model to blank slate\n" );
    vsp::VSPRenew();
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== Read Geometry From File ====//
    printf( "Reading model from: %s\n",fname.c_str() );
    vsp::ReadVSPFile( fname );
    vsp::ErrorMgr.PopErrorAndPrint( stdout );

    //==== List out all geoms ====//
    printf( "All geoms in Vehicle:\n" );
    geoms = vsp::FindGeoms();
    for ( int i = 0; i < ( int ) geoms.size(); i++ )
    {
        printf( "Geom id: %s name: %s \n", geoms[i].c_str(), vsp::GetGeomName( geoms[i] ).c_str() );
    }
    vsp::ErrorMgr.PopErrorAndPrint( stdout );
    
//==== Final check for errors ====//
    printf( "\n//==== Final check for errors ====//\n" );
    //==== Check And Print Any Errors ====//
    int num_err = vsp::ErrorMgr.GetNumTotalErrors();
    for ( int i = 0 ; i < num_err ; i++ )
    {
        vsp::ErrorObj err = vsp::ErrorMgr.PopLastError();
        printf( "err = %s\n", err.m_ErrorString.c_str() );
    }

    vsp::StartGui();

}
