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
