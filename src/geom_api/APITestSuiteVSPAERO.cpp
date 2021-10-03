//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// APITestSuiteVSPAero.cpp: Unit tests for geom_api
//
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Geom_API.h"
#include "APITestSuiteVSPAERO.h"
#include <float.h>

//Default tolerance to use for tests.  Most calculations are done as doubles and choosing single precision FLT_MIN gives some allowance for precision stackup in calculations
#define TEST_TOL FLT_MIN

//=============================================================================//
//========================== APITestSuiteVSPAERO ==============================//
//=============================================================================//

//===== Create geometry model  =====//
void APITestSuiteVSPAERO::TestVSPAeroCreateModel()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroCreateModel()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Wing Geom and set some parameters =====//
    printf( "\tAdding WING (MainWing)..." );
    string wing_id = vsp::AddGeom( "WING" );
    vsp::SetGeomName( wing_id, "MainWing" );
    TEST_ASSERT( wing_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( wing_id, "TotalSpan", "WingGeom", 17.0 ), 17.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( wing_id, "Z_Rel_Location", "XForm", 0.5 ), 0.5, TEST_TOL );
    // Adjust chordwise Tessellation
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( wing_id, "Tess_W", "Shape", 20 ), calcTessWCheckVal( 20 ), TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( wing_id, "LECluster", "WingGeom", 0.1 ), 0.1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( wing_id, "TECluster", "WingGeom", 2.0 ), 2.0, TEST_TOL );
    // Adjust spanwise Tessellation
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( wing_id, "SectTess_U", "XSec_1", 15 ), 15, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( wing_id, "InCluster", "XSec_1", 0.1 ), 0.1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( wing_id, "OutCluster", "XSec_1", 0.1 ), 0.1, TEST_TOL );
    // Set to cambered airfoils
    int xsec_surf_index = 0;
    string xsec_surf_id = vsp::GetXSecSurf( wing_id, xsec_surf_index );
    int xsec_index;
    string xsec_id;
    string parm_id;
    // Root airfoil - NACA 2412
    xsec_index = 0;
    vsp::ChangeXSecShape( xsec_surf_id, xsec_index, vsp::XSEC_CRV_TYPE::XS_FOUR_SERIES );
    xsec_id = vsp::GetXSec( xsec_surf_id, xsec_index );
    TEST_ASSERT( vsp::XSEC_CRV_TYPE::XS_FOUR_SERIES == vsp::GetXSecShape( xsec_id ) );
    parm_id = vsp::GetXSecParm( xsec_id, "Camber" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( parm_id, 0.02 ), 0.02, TEST_TOL );
    parm_id = vsp::GetXSecParm( xsec_id, "CamberLoc" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( parm_id, 0.4 ), 0.4, TEST_TOL );
    parm_id = vsp::GetXSecParm( xsec_id, "ThickChord" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( parm_id, 0.12 ), 0.12, TEST_TOL );
    // Tip airfoil - NACA 2210
    xsec_index = 1;
    vsp::ChangeXSecShape( xsec_surf_id, xsec_index, vsp::XSEC_CRV_TYPE::XS_FOUR_SERIES );
    xsec_id = vsp::GetXSec( xsec_surf_id, xsec_index );
    TEST_ASSERT( vsp::XSEC_CRV_TYPE::XS_FOUR_SERIES == vsp::GetXSecShape( xsec_id ) );
    parm_id = vsp::GetXSecParm( xsec_id, "Camber" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( parm_id, 0.02 ), 0.02, TEST_TOL );
    parm_id = vsp::GetXSecParm( xsec_id, "CamberLoc" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( parm_id, 0.2 ), 0.2, TEST_TOL );
    parm_id = vsp::GetXSecParm( xsec_id, "ThickChord" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( parm_id, 0.10 ), 0.10, TEST_TOL );

    // Add aileron control surface
    string aileron_id = AddSubSurf( wing_id, vsp::SS_CONTROL );
    vsp::SetSubSurfName( wing_id, aileron_id, "Aileron" );
    TEST_ASSERT( aileron_id.c_str() != NULL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Add Vertical tail and set some parameters =====//
    printf( "\tAdding WING (Vert)..." );
    string vert_id = vsp::AddGeom( "WING" );
    vsp::SetGeomName( vert_id, "Vert" );
    TEST_ASSERT( vert_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( vert_id, "TotalArea", "WingGeom", 10.0 ), 10.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( vert_id, "X_Rel_Location", "XForm", 8.5 ), 8.5, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( vert_id, "Z_Rel_Location", "XForm", 0.2 ), 0.2, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( vert_id, "X_Rel_Rotation", "XForm", 90 ), 90, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( vert_id, "Sym_Planar_Flag", "Sym", 0 ), 0, TEST_TOL );
    // Adjust chordwise Tessellation
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( vert_id, "Tess_W", "Shape", 20 ), calcTessWCheckVal( 20 ), TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( vert_id, "LECluster", "WingGeom", 0.1 ), 0.1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( vert_id, "TECluster", "WingGeom", 2.0 ), 2.0, TEST_TOL );
    // Adjust spanwise Tessellation
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( vert_id, "SectTess_U", "XSec_1", 8 ), 8, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( vert_id, "InCluster", "XSec_1", 0.1 ), 0.1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( vert_id, "OutCluster", "XSec_1", 0.1 ), 0.1, TEST_TOL );
    // Add rudder control surface
    string rudder_id = AddSubSurf( vert_id, vsp::SS_CONTROL );
    vsp::SetSubSurfName( vert_id, rudder_id, "Rudder" );
    TEST_ASSERT( rudder_id.c_str() != NULL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Add Horizontal tail and set some parameters =====//
    printf( "\tAdding WING (Horiz)..." );
    string horiz_id = vsp::AddGeom( "WING" );
    vsp::SetGeomName( horiz_id, "Horiz" );
    TEST_ASSERT( horiz_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( horiz_id, "TotalArea", "WingGeom", 10.0 ), 10.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( horiz_id, "X_Rel_Location", "XForm", 8.5 ), 8.5, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( horiz_id, "Z_Rel_Location", "XForm", 0.2 ), 0.2, TEST_TOL );
    // Adjust chordwise Tessellation
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( horiz_id, "Tess_W", "Shape", 20 ), calcTessWCheckVal( 20 ), TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( horiz_id, "LECluster", "WingGeom", 0.1 ), 0.1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( horiz_id, "TECluster", "WingGeom", 2.0 ), 2.0, TEST_TOL );
    // Adjust spanwise Tessellation
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( horiz_id, "SectTess_U", "XSec_1", 8 ), 8, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( horiz_id, "InCluster", "XSec_1", 0.1 ), 0.1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( horiz_id, "OutCluster", "XSec_1", 0.1 ), 0.1, TEST_TOL );
    // Add elevator control surface
    string elevator_id = AddSubSurf( horiz_id, vsp::SS_CONTROL );
    vsp::SetSubSurfName( horiz_id, elevator_id, "Elevator" );
    TEST_ASSERT( elevator_id.c_str() != NULL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Add Pod and set some parameters =====//
    printf( "\tAdding POD..." );
    string pod_id = vsp::AddGeom( "POD" );
    TEST_ASSERT( pod_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "Length", "Design", 14.5 ), 14.5, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( pod_id, "X_Rel_Location", "XForm", -3.0 ), -3.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( wing_id, "Tess_U", "Shape", 15 ), 15, TEST_TOL ); //lengthwise Tessellation
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( wing_id, "Tess_W", "Shape", 15 ), calcTessWCheckVal( 15 ), TEST_TOL ); //radial Tessellation
    printf( "COMPLETE\n" );

    //==== Set VSPAERO Reference lengths & areas ====//
    printf( "\tSetting reference wing..." );
    // Set as reference wing for VSPAERO
    TEST_ASSERT( wing_id == vsp::SetVSPAERORefWingID( wing_id ) );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Set VSPAERO Xcg position ====//
    printf( "\tSetting reference position..." );
    string vspaero_settings_container_id = vsp::FindContainer( "VSPAEROSettings", 0 );
    string xcg_id = vsp::FindParm( vspaero_settings_container_id, "Xcg", "VSPAERO" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( xcg_id, 2 ), 2, TEST_TOL );
    printf( "COMPLETE\n" );

    //==== Auto Group Control Surfaces ====//
    printf( "\tGrouping Control Surfaces..." );
    vsp::AutoGroupVSPAEROControlSurfaces();
    TEST_ASSERT( vsp::GetNumControlSurfaceGroups() == 3 );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );
    printf( "COMPLETE\n" );
    string control_group_settings_container_id = vsp::FindContainer( "VSPAEROSettings", 0 );   // auto grouping produces parm containers within VSPAEROSettings

    //==== Set Control Surface Group Deflection Angle ====//
    printf( "\tSetting control surface group deflection angles..." );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );
    //  setup asymmetric deflection for aileron
    string deflection_gain_id;
    // subsurfaces get added to groups with "CSGQualities_[geom_name]_[control_surf_name]"
    // subsurfaces gain parm name is "Surf[surfndx]_Gain" starting from 0 to NumSymmetricCopies-1
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron_id + "_0_Gain", "ControlSurfaceGroup_0" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_gain_id, 0.8 ), 0.8, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron_id + "_1_Gain", "ControlSurfaceGroup_0" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_gain_id, 1.2 ), 1.2, TEST_TOL );
    //  deflect aileron
    string deflection_angle_id = vsp::FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_0" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_angle_id, 1.0 ), 1.0, TEST_TOL );
    printf( "COMPLETE\n" );

    //==== Setup export filenames ====//
    m_vspfname_for_vspaerotests = "apitest_TestVSPAero.vsp3";
    printf( "\tSetting export name: %s...", m_vspfname_for_vspaerotests.c_str() );
    vsp::SetVSP3FileName( m_vspfname_for_vspaerotests );  // this still needs to be done even if a call to WriteVSPFile is made
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Final vehicle update ====//
    printf( "\tVehicle update..." );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Save Vehicle to File ====//
    printf( "\tSaving vehicle file to: %s ...", m_vspfname_for_vspaerotests.c_str() );
    vsp::WriteVSPFile( vsp::GetVSPFileName(), vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroComputeGeom()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroComputeGeom()\n" );

    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom" );
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests failed to open" );
        return;
    }

    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";
    printf( "\t%s\n", analysis_name.c_str() );

    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    // Change some input values
    //    Analysis method
    std::vector< int > analysis_method; analysis_method.push_back( vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE );
    vsp::SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method, 0 );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf( "\n\t\tExecuting..." );
    string results_id = vsp::ExecAnalysis( analysis_name );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n\n" );

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroControlSurfaceDeflection()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroControlSurfaceDeflection()\n" );

    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom" );
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests failed to open" );
        return;
    }

    printf( "\t\tExecuting Comp Geom..." );
    string results_id = vsp::ExecAnalysis( "VSPAEROComputeGeometry" );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n\n" );

    // Find All Control Surface IDs to be used in Gain Parm Names
    printf("\tFinding All Necessary Geometry IDs...\n");
    string wing_id = vsp::FindGeom( "MainWing", 0 );
    if (!wing_id.empty())
    {
        printf("\t\tMainWing Found.\n");
    }
    string aileron_id = vsp::GetSubSurf( wing_id, 0 );
    if (!aileron_id.empty())
    {
        printf("\t\tAileron Found.\n");
    }
    printf("\tCOMPLETE.\n");

    /// ==== Test Asymmetric Deflection ==== ///
    // Edit Control Surface Group Angle and Contained Control Surface Gains
    printf("\tEditing Aileron Control Surface Deflection Gains and Angle...");
    string csg_id = vsp::FindContainer( "VSPAEROSettings", 0 );
    TEST_ASSERT( csg_id.c_str() != NULL );
    string deflection_angle_id = vsp::FindParm( csg_id, "DeflectionAngle", "ControlSurfaceGroup_0" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_angle_id, 1.0 ), 1.0, TEST_TOL );
    string surf0_gain_id = vsp::FindParm( csg_id, "Surf_" + aileron_id + "_0_Gain", "ControlSurfaceGroup_0" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( surf0_gain_id, 2.0 ), 2.0, TEST_TOL );
    string surf1_gain_id = vsp::FindParm( csg_id, "Surf_" + aileron_id + "_1_Gain", "ControlSurfaceGroup_0" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( surf1_gain_id, 2.0 ), 2.0, TEST_TOL );
    printf("COMPLETE\n");

    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROSinglePoint";
    printf( "\tAnalysis: %s\n", analysis_name.c_str() );

    // Set defaults
    printf("\tSetting Defaults...");
    vsp::SetAnalysisInputDefaults( analysis_name );
    printf("COMPLETE\n");

    // Execute
    printf( "\tExecuting..." );
    vector <int> num_wake_iter;
    num_wake_iter.push_back( 1 );
    vsp::SetIntAnalysisInput( analysis_name, "WakeNumIter", num_wake_iter );
    results_id = vsp::ExecAnalysis( analysis_name );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n" );

    // Check for within 5% of v3.13 Rolling Moment
    /// old cmx = -0.01010 // Version Checked VSP 3.11 | VSPAERO 3.1
    double current_cmx = -0.00882; // Version Last Checked VSP 3.13 | VSPAERO 4.1
    string history_id = vsp::FindLatestResultsID( "VSPAERO_History" );
    double roll_mom_tol = std::abs( 0.05 * vsp::GetDoubleResults( history_id, "CMx" )[0] );
    TEST_ASSERT_DELTA( vsp::GetDoubleResults( history_id, "CMx" )[0], current_cmx, roll_mom_tol );

    /// ==== Test Symmetric Deflection ==== ///
    // Edit Control Surface Group Angle and Contained Control Surface Gains
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_angle_id, 1.0 ), 1.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( surf0_gain_id, 1.0 ), 1.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( surf1_gain_id, -1.0 ), -1.0, TEST_TOL );

    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    // Execute
    printf( "\n\t\tExecuting..." );
    vsp::SetIntAnalysisInput( analysis_name, "WakeNumIter", num_wake_iter );
    results_id = vsp::ExecAnalysis( analysis_name );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n\n" );

    // Check for within 5% of v3.11 Rolling Moment
    history_id = vsp::FindLatestResultsID( "VSPAERO_History" );
    TEST_ASSERT_DELTA( vsp::GetDoubleResults( history_id, "CMx" )[0], 0.0, TEST_TOL );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroComputeGeomPanel()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroComputeGeomPanel()\n" );

    // make sure setup works
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom" );
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests failed to open" );
        return;
    }

    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";
    printf( "\t%s\n", analysis_name.c_str() );

    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Change some input values
    //    Analysis method
    std::vector< int > analysis_method; analysis_method.push_back( vsp::VSPAERO_ANALYSIS_METHOD::PANEL );
    vsp::SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf( "\n\t\tExecuting..." );
    string results_id = vsp::ExecAnalysis( analysis_name );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroSinglePointPanel()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroSinglePointPanel()\n" );

    // make sure setup works
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeomPanel" );
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests failed to open" );
        return;
    }

    //==== Analysis: VSPAERO Single Point ====//
    string analysis_name = "VSPAEROSinglePoint";
    printf( "\t%s\n", analysis_name.c_str() );
    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    // Change some input values
    //    Analysis method
    std::vector< int > analysis_method; analysis_method.push_back( vsp::VSPAERO_ANALYSIS_METHOD::PANEL );
    vsp::SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method, 0 );
    //    Reference geometry set
    std::vector< int > geom_set; geom_set.push_back( 0 );
    vsp::SetIntAnalysisInput( analysis_name, "GeomSet", geom_set );
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back( 10 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Sref", sref );
    std::vector< double > bref; bref.push_back( 17 );
    vsp::SetDoubleAnalysisInput( analysis_name, "bref", bref );
    std::vector< double > cref; cref.push_back( 3 );
    vsp::SetDoubleAnalysisInput( analysis_name, "cref", cref );
    std::vector< int > ref_flag; ref_flag.push_back( 3 );
    vsp::SetIntAnalysisInput( analysis_name, "RefFlag", ref_flag );
    //    freestream parameters
    std::vector< double > alpha; alpha.push_back( 5 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Alpha", alpha );
    std::vector< double > beta; beta.push_back( 2.5 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Beta", beta );
    std::vector< double > mach; mach.push_back( 0.1 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Mach", mach );

    // Reduce wake iteration for speed (force new setup file ensures wake iter setting is re-generated for this test)
    std::vector< int > wakeNumIter; wakeNumIter.push_back( 2 );
    vsp::SetIntAnalysisInput( analysis_name, "WakeNumIter", wakeNumIter );

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf( "\n\t\tExecuting..." );
    string results_id = vsp::ExecAnalysis( analysis_name );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroSinglePoint()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroSinglePoint()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom" );
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests failed to open" );
        return;
    }

    //==== Analysis: VSPAERO Single Point ====//
    string analysis_name = "VSPAEROSinglePoint";
    printf( "\t%s\n", analysis_name.c_str() );
    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    // Change some input values
    //    Analysis method
    std::vector< int > analysis_method; analysis_method.push_back( vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE );
    vsp::SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method, 0 );
    //    Reference geometry set
    std::vector< int > geom_set; geom_set.push_back( 0 );
    vsp::SetIntAnalysisInput( analysis_name, "GeomSet", geom_set );
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back( 10 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Sref", sref );
    std::vector< double > bref; bref.push_back( 17 );
    vsp::SetDoubleAnalysisInput( analysis_name, "bref", bref );
    std::vector< double > cref; cref.push_back( 3 );
    vsp::SetDoubleAnalysisInput( analysis_name, "cref", cref );
    std::vector< int > ref_flag; ref_flag.push_back( 3 );
    vsp::SetIntAnalysisInput( analysis_name, "RefFlag", ref_flag );
    //    freestream parameters
    std::vector< double > alpha; alpha.push_back( 5 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Alpha", alpha );
    std::vector< double > beta; beta.push_back( 2.5 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Beta", beta );
    std::vector< double > mach; mach.push_back( 0.1 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Mach", mach );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf( "\n\t\tExecuting..." );
    string results_id = vsp::ExecAnalysis( analysis_name );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroSinglePointStab()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroSinglePointStab()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom" );
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests failed to open" );
        return;
    }

    //==== Analysis: VSPAERO Single Point  stabilityFlag = TRUE ====//
    string analysis_name = "VSPAEROSinglePoint";
    printf( "\t%s\n", analysis_name.c_str() );
    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    // Change some input values
    //    Analysis method
    std::vector< int > analysis_method; analysis_method.push_back( vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE );
    vsp::SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method, 0 );
    //    Reference geometry set
    std::vector< int > geom_set; geom_set.push_back( 0 );
    vsp::SetIntAnalysisInput( analysis_name, "GeomSet", geom_set );
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back( 10 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Sref", sref );
    std::vector< double > bref; bref.push_back( 17 );
    vsp::SetDoubleAnalysisInput( analysis_name, "bref", bref );
    std::vector< double > cref; cref.push_back( 3 );
    vsp::SetDoubleAnalysisInput( analysis_name, "cref", cref );
    std::vector< int > ref_flag; ref_flag.push_back( 3 );
    vsp::SetIntAnalysisInput( analysis_name, "RefFlag", ref_flag );
    //    freestream parameters
    std::vector< double > alpha; alpha.push_back( 4.0 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Alpha", alpha );
    std::vector< double > beta; beta.push_back( -3.0 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Beta", beta );
    std::vector< double > mach; mach.push_back( 0.4 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Mach", mach );
    //     Case Setup
    std::vector< int > wakeNumIter; wakeNumIter.push_back( 1 );
    vsp::SetIntAnalysisInput( analysis_name, "WakeNumIter", wakeNumIter );
    std::vector< int > stabilityTypeFlag; stabilityTypeFlag.push_back( vsp::STABILITY_DEFAULT );
    vsp::SetIntAnalysisInput( analysis_name, "UnsteadyType", stabilityTypeFlag );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf( "\n\t\tExecuting..." );
    string results_id = vsp::ExecAnalysis( analysis_name );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroSinglePointUnsteady()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroSinglePointUnsteady()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    printf("\tReading in file...");
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom" );
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests failed to open" );
        return;
    }
    printf("COMPLETE\n");

    //==== Analysis: VSPAERO Single Point  stabilityFlag = TRUE ====//
    string analysis_name = "VSPAEROSinglePoint";
    printf( "\tAnalysis Type: %s\n", analysis_name.c_str() );
    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    printf("\tChanging Analysis Inputs...");
    // Change some input values
    //    Analysis method
    std::vector< int > analysis_method; analysis_method.push_back( vsp::VORTEX_LATTICE );
    vsp::SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method, 0 );
    //    Reference geometry set
    std::vector< int > geom_set; geom_set.push_back( 0 );
    vsp::SetIntAnalysisInput( analysis_name, "GeomSet", geom_set );
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back( 10 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Sref", sref );
    std::vector< double > bref; bref.push_back( 17 );
    vsp::SetDoubleAnalysisInput( analysis_name, "bref", bref );
    std::vector< double > cref; cref.push_back( 3 );
    vsp::SetDoubleAnalysisInput( analysis_name, "cref", cref );
    std::vector< int > ref_flag; ref_flag.push_back( 0 );
    vsp::SetIntAnalysisInput( analysis_name, "RefFlag", ref_flag );
    //    freestream parameters
    std::vector< double > alpha; alpha.push_back( 4.0 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Alpha", alpha );
    std::vector< double > beta; beta.push_back( -3.0 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Beta", beta );
    std::vector< double > mach; mach.push_back( 0.4 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Mach", mach );
    //     Case Setup
    std::vector< int > wakeNumIter; wakeNumIter.push_back( 1 );
    vsp::SetIntAnalysisInput( analysis_name, "WakeNumIter", wakeNumIter );
    std::vector< int> stabilityCalcType; stabilityCalcType.push_back( vsp::STABILITY_P_ANALYSIS );
    vsp::SetIntAnalysisInput( analysis_name, "UnsteadyType", stabilityCalcType );
    std::vector< int > jacobiPrecondition; jacobiPrecondition.push_back( vsp::PRECON_JACOBI );
    vsp::SetIntAnalysisInput( analysis_name, "Precondition", jacobiPrecondition );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("COMPLETE\n");

    // Execute
    printf( "\tExecuting..." );
    string results_id = vsp::ExecAnalysis( analysis_name );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroSweep()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroSweep()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom" );
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests failed to open" );
        return;
    }

    //==== Analysis: VSPAERO Sweep ====//
    string analysis_name = "VSPAEROSweep";
    printf( "\t%s\n", analysis_name.c_str() );
    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    // Change some input values
    //    Analysis method
    std::vector< int > analysis_method; analysis_method.push_back( vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE );
    vsp::SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method, 0 );
    //    Reference geometry set
    std::vector< int > geom_set; geom_set.push_back( 0 );
    vsp::SetIntAnalysisInput( analysis_name, "GeomSet", geom_set );
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back( 10 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Sref", sref );
    std::vector< double > bref; bref.push_back( 17 );
    vsp::SetDoubleAnalysisInput( analysis_name, "bref", bref );
    std::vector< double > cref; cref.push_back( 3 );
    vsp::SetDoubleAnalysisInput( analysis_name, "cref", cref );
    std::vector< int > ref_flag; ref_flag.push_back( 3 );
    vsp::SetIntAnalysisInput( analysis_name, "RefFlag", ref_flag );
    //    freestream parameters
    //        Alpha
    std::vector< double > alpha_start; alpha_start.push_back( 1 );
    vsp::SetDoubleAnalysisInput( analysis_name, "AlphaStart", alpha_start );
    std::vector< double > alpha_end; alpha_end.push_back( 10 );
    vsp::SetDoubleAnalysisInput( analysis_name, "AlphaEnd", alpha_end );
    std::vector< int > alpha_npts; alpha_npts.push_back( 4 );
    vsp::SetIntAnalysisInput( analysis_name, "AlphaNpts", alpha_npts );
    //        Beta
    std::vector< double > beta_start; beta_start.push_back( 0 );
    vsp::SetDoubleAnalysisInput( analysis_name, "BetaStart", beta_start );
    std::vector< double > beta_end; beta_end.push_back( 5 );
    vsp::SetDoubleAnalysisInput( analysis_name, "BetaEnd", beta_end );
    std::vector< int > beta_npts; beta_npts.push_back( 3 );
    vsp::SetIntAnalysisInput( analysis_name, "BetaNpts", beta_npts );
    //        Mach
    std::vector< double > mach_start; mach_start.push_back( 0.05 );
    vsp::SetDoubleAnalysisInput( analysis_name, "MachStart", mach_start );
    std::vector< double > mach_end; mach_end.push_back( 0.15 );
    vsp::SetDoubleAnalysisInput( analysis_name, "MachEnd", mach_end );
    std::vector< int > mach_npts; mach_npts.push_back( 2 );
    vsp::SetIntAnalysisInput( analysis_name, "MachNpts", mach_npts );
    vsp::Update();
    //     Case Setup
    std::vector< int > wakeNumIter; wakeNumIter.push_back( 1 );
    vsp::SetIntAnalysisInput( analysis_name, "WakeNumIter", wakeNumIter );
    std::vector< int > batch_mode_flag; batch_mode_flag.push_back( 0 );
    vsp::SetIntAnalysisInput( analysis_name, "BatchModeFlag", batch_mode_flag );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf( "\n\t\tExecuting..." );
    string results_id = vsp::ExecAnalysis( analysis_name );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroSweepBatch()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroSweepBatch()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom" );
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests failed to open" );
        return;
    }

    //==== Analysis: VSPAERO Sweep ====//
    string analysis_name = "VSPAEROSweep";
    printf( "\t%s\n", analysis_name.c_str() );
    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    // Change some input values
    //    Analysis method
    std::vector< int > analysis_method; analysis_method.push_back( vsp::VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE );
    vsp::SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method, 0 );
    //    Reference geometry set
    std::vector< int > geom_set; geom_set.push_back( 0 );
    vsp::SetIntAnalysisInput( analysis_name, "GeomSet", geom_set );
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back( 10 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Sref", sref );
    std::vector< double > bref; bref.push_back( 17 );
    vsp::SetDoubleAnalysisInput( analysis_name, "bref", bref );
    std::vector< double > cref; cref.push_back( 3 );
    vsp::SetDoubleAnalysisInput( analysis_name, "cref", cref );
    std::vector< int > ref_flag; ref_flag.push_back( 3 );
    vsp::SetIntAnalysisInput( analysis_name, "RefFlag", ref_flag );
    //    freestream parameters
    //        Alpha
    std::vector< double > alpha_start; alpha_start.push_back( 1 );
    vsp::SetDoubleAnalysisInput( analysis_name, "AlphaStart", alpha_start );
    std::vector< double > alpha_end; alpha_end.push_back( 10 );
    vsp::SetDoubleAnalysisInput( analysis_name, "AlphaEnd", alpha_end );
    std::vector< int > alpha_npts; alpha_npts.push_back( 4 );
    vsp::SetIntAnalysisInput( analysis_name, "AlphaNpts", alpha_npts );
    //        Beta
    std::vector< double > beta_start; beta_start.push_back( 0 );
    vsp::SetDoubleAnalysisInput( analysis_name, "BetaStart", beta_start );
    std::vector< double > beta_end; beta_end.push_back( 5 );
    vsp::SetDoubleAnalysisInput( analysis_name, "BetaEnd", beta_end );
    std::vector< int > beta_npts; beta_npts.push_back( 3 );
    vsp::SetIntAnalysisInput( analysis_name, "BetaNpts", beta_npts );
    //        Mach
    std::vector< double > mach_start; mach_start.push_back( 0.05 );
    vsp::SetDoubleAnalysisInput( analysis_name, "MachStart", mach_start );
    std::vector< double > mach_end; mach_end.push_back( 0.15 );
    vsp::SetDoubleAnalysisInput( analysis_name, "MachEnd", mach_end );
    std::vector< int > mach_npts; mach_npts.push_back( 2 );
    vsp::SetIntAnalysisInput( analysis_name, "MachNpts", mach_npts );
    //     Case Setup
    std::vector< int > wakeNumIter; wakeNumIter.push_back( 1 );
    vsp::SetIntAnalysisInput( analysis_name, "WakeNumIter", wakeNumIter );
    std::vector< int > batch_mode_flag; batch_mode_flag.push_back( 1 );
    vsp::SetIntAnalysisInput( analysis_name, "BatchModeFlag", batch_mode_flag );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf( "\n\t\tExecuting..." );
    string results_id = vsp::ExecAnalysis( analysis_name );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroSharpTrailingEdge()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroSharpTrailingEdge()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Analysis: VSPAero Single Point ====//
    string analysis_name = "VSPAEROSinglePoint";
    printf( "\t%s\n", analysis_name.c_str() );

    //==== Create Symmetric Airfoil Wing Geometry ====//
    printf( "--> Generating Geometries\n" );

    string wing_id = vsp::AddGeom( "WING" );
    TEST_ASSERT( wing_id.c_str() != NULL );

    // Get Wing Section IDs
    string wingxsurf_id = vsp::GetXSecSurf( wing_id, 0 );
    string xsec_id0 = vsp::GetXSec( wingxsurf_id, 0 );
    string xsec_id1 = vsp::GetXSec( wingxsurf_id, 1 );

    //  Set Root and Tip Chord to 3 and Area to 25:
    SetDriverGroup( wing_id, 1, vsp::AREA_WSECT_DRIVER, vsp::ROOTC_WSECT_DRIVER, vsp::TIPC_WSECT_DRIVER );
    string xtipchord_id1 = vsp::GetXSecParm( xsec_id1, "Tip_Chord" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xtipchord_id1, 3 ), 3, TEST_TOL );
    string xrootchord_id1 = vsp::GetXSecParm( xsec_id1, "Root_Chord" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xrootchord_id1, 3 ), 3, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( wing_id, "Area", "XSec_1", 25.0 ), 25.0, TEST_TOL );

    //  Set Sweep to 0:
    string xsweep_id1 = vsp::GetXSecParm( xsec_id1, "Sweep" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xsweep_id1, 0.0 ), 0.0, TEST_TOL );

    //  Increase W Tessellation:
    TEST_ASSERT_DELTA( vsp::SetParmVal( wing_id, "Tess_W", "Shape", 69 ), calcTessWCheckVal( 69 ), TEST_TOL );

    //  Increase U Tessellation
    string xutess_id1 = vsp::GetXSecParm( xsec_id1, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id1, 16 ), 16, TEST_TOL );
    string xrtcluster_id1 = vsp::GetXSecParm( xsec_id1, "InCluster" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xrtcluster_id1, 0.1 ), 0.1, TEST_TOL );
    string xtipcluster_id1 = vsp::GetXSecParm( xsec_id1, "OutCluster" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xtipcluster_id1, 0.1 ), 0.1, TEST_TOL );

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Setup export filenames ====//
    // Execution of one of these methods is required to properly set the export filenames for creation of vspaero input files and execution commands
    string fname_sharptrailingedge = "apitest_VSPAeroSharpTrailingEdge.vsp3";
    printf( "\tSetting export name: %s\n", fname_sharptrailingedge.c_str( ) );
    vsp::SetVSP3FileName( fname_sharptrailingedge );  // this still needs to be done even if a call to WriteVSPFile is made
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Save Vehicle to File ====//
    printf( "\tSaving vehicle file to: %s ...\n", fname_sharptrailingedge.c_str( ) );
    vsp::WriteVSPFile( vsp::GetVSPFileName(), vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Analysis: VSPAero Compute Geometry to Create Vortex Lattice DegenGeom File ====//
    string compgeom_name = "VSPAEROComputeGeometry";
    printf( "\t%s\n", compgeom_name.c_str() );

    // Set defaults
    vsp::SetAnalysisInputDefaults( compgeom_name );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( compgeom_name );

    // Execute
    printf( "\tExecuting...\n" );
    string compgeom_resid = vsp::ExecAnalysis( compgeom_name );
    TEST_ASSERT( compgeom_resid.size() > 0 );
    printf( "COMPLETE\n" );

    // Get & Display Results
    vsp::PrintResults( compgeom_resid );

    //==== Analysis: VSPAero Single Point ====//
    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    // Reference geometry set
    vector< int > geom_set;
    geom_set.push_back( 0 );
    vsp::SetIntAnalysisInput( analysis_name, "GeomSet", geom_set, 0 );
    vector< int > ref_flag;
    ref_flag.push_back( vsp::REF_WING_TYPE::COMPONENT_REF );
    vsp::SetIntAnalysisInput( analysis_name, "RefFlag", ref_flag, 0 );
    vector< string > wid = vsp::FindGeomsWithName( "WingGeom" );
    vsp::SetStringAnalysisInput( analysis_name, "WingID", wid, 0 );

    // Freestream Parameters
    vector< double > alpha;
    alpha.push_back( 0.0 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Alpha", alpha, 0 );
    vector< double > mach;
    mach.push_back( 0.1 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Mach", mach, 0 );

    // Reduce wake iteration for speed (force new setup file ensures wake iter setting is re-generated for this test)
    std::vector< int > wakeNumIter; wakeNumIter.push_back( 1 );
    vsp::SetIntAnalysisInput( analysis_name, "WakeNumIter", wakeNumIter );

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf( "\tExecuting...\n" );
    string results_id = vsp::ExecAnalysis( analysis_name  );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    string history_res= vsp::FindLatestResultsID( "VSPAERO_History" );
    string load_res = vsp::FindLatestResultsID( "VSPAERO_Load" );

    vector<double> CL = vsp::GetDoubleResults( history_res, "CL", 0 );
    vector<double> cl = vsp::GetDoubleResults( load_res, "cl", 0 );

    printf( "   CL: " );
    for ( unsigned int i = 0; i < CL.size(); i++ )
    {
        TEST_ASSERT_DELTA( CL[i], 0.0, TEST_TOL );
        printf( "%7.3f", CL[i] );
    }
    printf( "\n" );
    printf( "   cl: " );
    for ( unsigned int i = 0; i < cl.size(); i++ )
    {
        TEST_ASSERT_DELTA( cl[i], 0.0, TEST_TOL );
        printf( "%7.3f", cl[i] );
    }
    printf( "\n" );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroBluntTrailingEdge()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroBluntTrailingEdge()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Analysis: VSPAero Single Point ====//
    string analysis_name = "VSPAEROSinglePoint";
    printf( "\t%s\n", analysis_name.c_str() );

    //==== Create Symmetric Airfoil Wing Geometry ====//
    printf( "--> Generating Geometries\n" );

    string wing_id = vsp::AddGeom( "WING" );
    TEST_ASSERT( wing_id.c_str() != NULL );

    // Get Wing Section IDs
    string wingxsurf_id = vsp::GetXSecSurf( wing_id, 0 );
    string xsec_id0 = vsp::GetXSec( wingxsurf_id, 0 );
    string xsec_id1 = vsp::GetXSec( wingxsurf_id, 1 );

    //  Set Root and Tip Chord to 3 and Area to 25:
    SetDriverGroup( wing_id, 1, vsp::AREA_WSECT_DRIVER, vsp::ROOTC_WSECT_DRIVER, vsp::TIPC_WSECT_DRIVER );
    string xtipchord_id1 = vsp::GetXSecParm( xsec_id1, "Tip_Chord" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xtipchord_id1, 3 ), 3, TEST_TOL );
    string xrootchord_id1 = vsp::GetXSecParm( xsec_id1, "Root_Chord" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xrootchord_id1, 3 ), 3, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( wing_id, "Area", "XSec_1", 25.0 ), 25.0, TEST_TOL );

    //  Set Sweep to 0:
    string xsweep_id1 = vsp::GetXSecParm( xsec_id1, "Sweep" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xsweep_id1, 0.0 ), 0.0, TEST_TOL );

    //  Set Trailing Edge to Blunt:
    string xtrimtype_id0 = vsp::GetXSecParm( xsec_id0, "TE_Trim_Type" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xtrimtype_id0, 1 ), 1, TEST_TOL );
    string xtrimval_id0 = vsp::GetXSecParm( xsec_id0, "TE_Trim_X" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xtrimval_id0, 0.5 ), 0.5, TEST_TOL );
    string xtrimtype_id1 = vsp::GetXSecParm( xsec_id1, "TE_Trim_Type" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xtrimtype_id1, 1 ), 1, TEST_TOL );
    string xtrimval_id1 = vsp::GetXSecParm( xsec_id1, "TE_Trim_X" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xtrimval_id1, 0.5 ), 0.5, TEST_TOL );

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //  Increase W Tessellation:
    TEST_ASSERT_DELTA( vsp::SetParmVal( wing_id, "Tess_W", "Shape", 69 ), calcTessWCheckVal( 69 ), TEST_TOL );

    //  Increase U Tessellation
    string xutess_id1 = vsp::GetXSecParm( xsec_id1, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id1, 16 ), 16, TEST_TOL );
    string xrtcluster_id1 = vsp::GetXSecParm( xsec_id1, "InCluster" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xrtcluster_id1, 0.1 ), 0.1, TEST_TOL );
    string xtipcluster_id1 = vsp::GetXSecParm( xsec_id1, "OutCluster" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xtipcluster_id1, 0.1 ), 0.1, TEST_TOL );

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Setup export filenames ====//
    // Execution of one of these methods is required to properly set the export filenames for creation of vspaero input files and execution commands
    string fname_blunttrailingedge = "apitest_VSPAeroBluntTrailingEdge.vsp3";
    printf( "\tSetting export name: %s\n", fname_blunttrailingedge.c_str( ) );
    vsp::SetVSP3FileName( fname_blunttrailingedge );  // this still needs to be done even if a call to WriteVSPFile is made
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Save Vehicle to File ====//
    printf( "\tSaving vehicle file to: %s ...\n", fname_blunttrailingedge.c_str( ) );
    vsp::WriteVSPFile( vsp::GetVSPFileName(), vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Analysis: VSPAero Compute Geometry to Create Vortex Lattice DegenGeom File ====//
    string compgeom_name = "VSPAEROComputeGeometry";
    printf( "\t%s\n", compgeom_name.c_str() );

    // Set defaults
    vsp::SetAnalysisInputDefaults( compgeom_name );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( compgeom_name );

    // Execute
    printf( "\tExecuting...\n" );
    string compgeom_resid = vsp::ExecAnalysis( compgeom_name );
    TEST_ASSERT( compgeom_resid.size() > 0 );
    printf( "COMPLETE\n" );

    // Get & Display Results
    vsp::PrintResults( compgeom_resid );

    //==== Analysis: VSPAero Single Point ====//
    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    // Reference geometry set
    vector< int > geom_set;
    geom_set.push_back( 0 );
    vsp::SetIntAnalysisInput( analysis_name, "GeomSet", geom_set, 0 );
    vector< int > ref_flag;
    ref_flag.push_back( vsp::REF_WING_TYPE::COMPONENT_REF );
    vsp::SetIntAnalysisInput( analysis_name, "RefFlag", ref_flag, 0 );
    vector< string > wid = vsp::FindGeomsWithName( "WingGeom" );
    vsp::SetStringAnalysisInput( analysis_name, "WingID", wid, 0 );

    // Freestream Parameters
    vector< double > alpha;
    alpha.push_back( 0.0 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Alpha", alpha, 0 );
    vector< double > mach;
    mach.push_back( 0.1 );
    vsp::SetDoubleAnalysisInput( analysis_name, "Mach", mach, 0 );

    // Reduce wake iteration for speed (force new setup file ensures wake iter setting is re-generated for this test)
    std::vector< int > wakeNumIter; wakeNumIter.push_back( 1 );
    vsp::SetIntAnalysisInput( analysis_name, "WakeNumIter", wakeNumIter );

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf( "\tExecuting...\n" );
    string results_id = vsp::ExecAnalysis( analysis_name  );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    string history_res = vsp::FindLatestResultsID( "VSPAERO_History" );
    string load_res = vsp::FindLatestResultsID( "VSPAERO_Load" );

    vector<double> CL = vsp::GetDoubleResults( history_res, "CL", 0 );
    vector<double> cl = vsp::GetDoubleResults( load_res, "cl", 0 );

    printf( "   CL: " );
    for ( unsigned int i = 0; i < CL.size(); i++ )
    {
        TEST_ASSERT_DELTA( CL[i], 0.0, TEST_TOL );
        printf( "%7.3f", CL[i] );
    }
    printf( "\n" );
    printf( "   cl: " );
    for ( unsigned int i = 0; i < cl.size(); i++ )
    {
        TEST_ASSERT_DELTA( cl[i], 0.0, TEST_TOL );
        printf( "%7.3f", cl[i] );
    }
    printf( "\n" );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroSupersonicDeltaWing()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroSupersonicDeltaWing()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Analysis: VSPAero Sweep ====//
    string analysis_name = "VSPAEROSweep";
    printf( "\t%s\n", analysis_name.c_str() );

    //==== Create Delta Wing Geometry ====//
    printf( "--> Generating Geometry\n" );

    string wing_id = vsp::AddGeom( "WING" );
    TEST_ASSERT( wing_id.c_str() != NULL );

    // Get Wing Section IDs
    string wingxsurf_id = vsp::GetXSecSurf( wing_id, 0 );
    string xsec_id0 = vsp::GetXSec( wingxsurf_id, 0 );
    string xsec_id1 = vsp::GetXSec( wingxsurf_id, 1 );

    //  Set Root and Tip Chord to 3 and Area to 25:
    SetDriverGroup( wing_id, 1, vsp::SPAN_WSECT_DRIVER, vsp::ROOTC_WSECT_DRIVER, vsp::TIPC_WSECT_DRIVER );
    string xtipchord_id1 = vsp::GetXSecParm( xsec_id1, "Tip_Chord" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xtipchord_id1, 0.01 ), 0.01, TEST_TOL );
    string xrootchord_id1 = vsp::GetXSecParm( xsec_id1, "Root_Chord" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xrootchord_id1, 8 ), 8, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( wing_id, "Span", "XSec_1", 8 ), 8, TEST_TOL );

    //  Set Sweep to 45:
    string xsweep_id1 = vsp::GetXSecParm( xsec_id1, "Sweep" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xsweep_id1, 45 ), 45, TEST_TOL );

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //  Increase W Tessellation:
    TEST_ASSERT_DELTA( vsp::SetParmVal( wing_id, "Tess_W", "Shape", 69 ), calcTessWCheckVal( 69 ), TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( wing_id, "LECluster", "WingGeom", 0.1 ), 0.1, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmVal( wing_id, "TECluster", "WingGeom", 0.1 ), 0.1, TEST_TOL );

    //  Increase U Tessellation
    string xutess_id1 = vsp::GetXSecParm( xsec_id1, "SectTess_U" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xutess_id1, 16 ), 16, TEST_TOL );
    string xrtcluster_id1 = vsp::GetXSecParm( xsec_id1, "InCluster" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xrtcluster_id1, 0.1 ), 0.1, TEST_TOL );
    string xtipcluster_id1 = vsp::GetXSecParm( xsec_id1, "OutCluster" );
    TEST_ASSERT_DELTA( vsp::SetParmVal( xtipcluster_id1, 0.1 ), 0.1, TEST_TOL );

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Setup export filenames ====//
    // Execution of one of these methods is required to properly set the export filenames for creation of vspaero input files and execution commands
    string fname_deltawing = "apitest_VSPAeroSupersonicDeltaWing.vsp3";
    printf( "\tSetting export name: %s\n", fname_deltawing.c_str( ) );
    vsp::SetVSP3FileName( fname_deltawing );  // this still needs to be done even if a call to WriteVSPFile is made
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Save Vehicle to File ====//
    printf( "\tSaving vehicle file to: %s ...\n", fname_deltawing.c_str( ) );
    vsp::WriteVSPFile( vsp::GetVSPFileName(), vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Analysis: VSPAero Compute Geometry to Create Vortex Lattice DegenGeom File ====//
    string compgeom_name = "VSPAEROComputeGeometry";
    printf( "\t%s\n", compgeom_name.c_str() );

    // Set defaults
    vsp::SetAnalysisInputDefaults( compgeom_name );

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( compgeom_name );

    // Execute
    printf( "\tExecuting...\n" );
    string compgeom_resid = vsp::ExecAnalysis( compgeom_name );
    TEST_ASSERT( compgeom_resid.size() > 0 );
    printf( "COMPLETE\n" );

    // Get & Display Results
    vsp::PrintResults( compgeom_resid );

    //==== Analysis: VSPAero Single Point ====//
    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    // Reference geometry set
    vector< int > geom_set;
    geom_set.push_back( 0 );
    vsp::SetIntAnalysisInput( analysis_name, "GeomSet", geom_set, 0 );
    vector< int > ref_flag;
    ref_flag.push_back( vsp::REF_WING_TYPE::COMPONENT_REF );
    vsp::SetIntAnalysisInput( analysis_name, "RefFlag", ref_flag, 0 );
    vector< string > wid = vsp::FindGeomsWithName( "WingGeom" );
    vsp::SetStringAnalysisInput( analysis_name, "WingID", wid, 0 );

    // Freestream Parameters
    vector< double > alpha_start;
    alpha_start.push_back( 1 );
    vsp::SetDoubleAnalysisInput( analysis_name, "AlphaStart", alpha_start, 0 );
    vector< int > alpha_npts;
    alpha_npts.push_back( 1 );
    vsp::SetIntAnalysisInput( analysis_name, "AlphaNpts", alpha_npts, 0 );
    vector< double > mach_start;
    mach_start.push_back( 1.2 );
    vsp::SetDoubleAnalysisInput( analysis_name, "MachStart", mach_start, 0 );
    vector< int > mach_npts;
    mach_npts.push_back( 8 );
    vsp::SetIntAnalysisInput( analysis_name, "MachNpts", mach_npts, 0 );
    vector< double > mach_end;
    mach_end.push_back( 4.4 );
    vsp::SetDoubleAnalysisInput( analysis_name, "MachEnd", mach_end, 0 );

    // Reduce wake iteration for speed (force new setup file ensures wake iter setting is re-generated for this test)
    std::vector< int > wakeNumIter; wakeNumIter.push_back( 1 );
    vsp::SetIntAnalysisInput( analysis_name, "WakeNumIter", wakeNumIter );

    // Set Batch Mode
    vector< int > batch_mode_flag; batch_mode_flag.push_back( 1 );
    vsp::SetIntAnalysisInput( analysis_name, "BatchModeFlag", batch_mode_flag, 0 );

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf( "\tExecuting...\n" );
    string results_id = vsp::ExecAnalysis( analysis_name  );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroCreateFunctionalityModel()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroCreateFunctionalityModel()\n" );

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Wing Geom and set some parameters =====//
    printf( "\tAdding WING (MainWing)..." );
    string wing_id = vsp::AddGeom( "WING" );
    vsp::SetGeomName( wing_id, "MainWing" );
    TEST_ASSERT( wing_id.c_str() != NULL );

    // Add aileron control surfaces
    string aileron1_id = AddSubSurf( wing_id, vsp::SS_CONTROL );
    vsp::SetSubSurfName( wing_id, aileron1_id, "Inner Aileron" );
    TEST_ASSERT( aileron1_id.c_str() != NULL );
    string tessstart = vsp::GetParm( wing_id, "UStart", "SS_Control_1");
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( tessstart, 0.35 ), 0.35, TEST_TOL );
    string tessend = vsp::GetParm( wing_id, "UEnd", "SS_Control_1" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( tessend, 0.45 ), 0.45, TEST_TOL );

    string aileron2_id = AddSubSurf( wing_id, vsp::SS_CONTROL );
    vsp::SetSubSurfName( wing_id, aileron2_id, "Outer Aileron" );
    TEST_ASSERT( aileron2_id.c_str() != NULL );
    tessstart = vsp::GetParm( wing_id, "UStart", "SS_Control_2");
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( tessstart, 0.47 ), 0.47, TEST_TOL );
    tessend = vsp::GetParm( wing_id, "UEnd", "SS_Control_2");
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( tessend, 0.6 ), 0.6, TEST_TOL );

    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Add Horizontal tail and set some parameters =====//
    printf( "\tAdding WING (Horiz)..." );
    string horiz_id = vsp::AddGeom( "WING" );
    vsp::SetGeomName( horiz_id, "Tail" );
    TEST_ASSERT( horiz_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( horiz_id, "TotalArea", "WingGeom", 10.0 ), 10.0, TEST_TOL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( horiz_id, "X_Rel_Location", "XForm", 8.5 ), 8.5, TEST_TOL );

    // Add elevator control surface
    string elevator_id = AddSubSurf( horiz_id, vsp::SS_CONTROL );
    vsp::SetSubSurfName( horiz_id, elevator_id, "Elevator" );
    TEST_ASSERT( elevator_id.c_str() != NULL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    /// TODO
    //==== Add Inner Disk and set some parameters ====//
    //==== Add Outer Disk and set some parameters ====//

    //==== Auto Group Control Surfaces ====//
    printf( "\tGrouping Control Surfaces..." );
    vsp::AutoGroupVSPAEROControlSurfaces();
    TEST_ASSERT( vsp::GetNumControlSurfaceGroups() == 3 );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );
    printf( "COMPLETE\n" );
    string control_group_settings_container_id = vsp::FindContainer( "VSPAEROSettings", 0 );   // auto grouping produces parm containers within VSPAEROSettings

    //==== Set Control Surface Group Deflection Angle ====//
    printf( "\tSetting control surface group deflection angles..." );
    string deflection_gain_id, deflection_angle_id;
    // subsurfaces get added to groups with "ControlSurfaceGroup_[index]"
    // subsurfaces gain parm name is "Surf_[surfid]_[surfndx]_Gain" starting from 0 to NumSymmetricCopies-1
    
    double gain0 = 0.1;
    double angle0 = 1.0;
    double gain1 = 0.2;
    double angle1 = 2.0;
    double gain2 = 0.3;
    double angle2 = 3.0;

    // ControlSurfaceGroup_0
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron1_id + "_0_Gain", "ControlSurfaceGroup_0" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_gain_id, gain0 ), gain0, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron1_id + "_1_Gain", "ControlSurfaceGroup_0" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_gain_id, -gain0 ), -gain0, TEST_TOL );
    //  deflect inside aileron
    deflection_angle_id = vsp::FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_0" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_angle_id, angle0 ), angle0, TEST_TOL );

    // ControlSurfaceGroup_1
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron2_id + "_0_Gain", "ControlSurfaceGroup_1" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_gain_id, gain1 ), gain1, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron2_id + "_1_Gain", "ControlSurfaceGroup_1" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_gain_id, -gain1 ), -gain1, TEST_TOL );
    //  deflect inside aileron
    deflection_angle_id = vsp::FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_1" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_angle_id, angle1 ), angle1, TEST_TOL );

    // ControlSurfaceGroup_2
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + elevator_id + "_0_Gain", "ControlSurfaceGroup_2" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_gain_id, gain2 ), gain2, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + elevator_id + "_1_Gain", "ControlSurfaceGroup_2" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_gain_id, -gain2 ), -gain2, TEST_TOL );
    //  deflect inside aileron
    deflection_angle_id = vsp::FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_2" );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate( deflection_angle_id, angle2 ), angle2, TEST_TOL );

    printf("COMPLETE. \n");

    //==== Setup export filenames ====//
    m_vspfname_for_vspaerofunctionalitytests = "apitest_TestVSPAeroFunctionality.vsp3";
    printf( "\tSetting export name: %s...", m_vspfname_for_vspaerofunctionalitytests.c_str() );
    vsp::SetVSP3FileName( m_vspfname_for_vspaerofunctionalitytests );  // this still needs to be done even if a call to WriteVSPFile is made
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Final vehicle update ====//
    printf( "\tVehicle update..." );
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Save Vehicle to File ====//
    printf( "\tSaving vehicle file to: %s ...", m_vspfname_for_vspaerofunctionalitytests.c_str() );
    vsp::WriteVSPFile( vsp::GetVSPFileName(), vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "COMPLETE\n\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroReadControlSurfaceGroupsFromFile()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroReadControlSurfaceGroupsFromFile()\n" );

    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    printf("\tLoading File...");
    vsp::ReadVSPFile( m_vspfname_for_vspaerofunctionalitytests );
    if ( m_vspfname_for_vspaerofunctionalitytests == string() )
    {
        TEST_FAIL( "m_vspfname_for_vspaerofunctionalitytests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom" );
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "m_vspfname_for_vspaerofunctionalitytests failed to open" );
        return;
    }
    printf("COMPLETE.\n");

    //==== Find Control Surface Group Parm Containers ====//
    string deflection_gain_id, deflection_angle_id;
    string control_group_settings_container_id = vsp::FindContainer( "VSPAEROSettings", 0 );   // auto grouping produces parm containers within VSPAEROSettings

    // Find All Control Surface IDs to be used in Gain Parm Names
    printf("\tFinding All Geometry IDs...\n");
    string wing_id = vsp::FindGeom( "MainWing", 0 );
    if (!wing_id.empty())
    {
        printf("\t\tMainWing Found.\n");
    }
    string aileron1_id = vsp::GetSubSurf( wing_id, 0 );
    if (!aileron1_id.empty())
    {
        printf("\t\tInner Aileron Found.\n");
    }
    string aileron2_id = vsp::GetSubSurf( wing_id, 1 );
    if (!aileron2_id.empty())
    {
        printf("\t\tOuter Aileron Found.\n");
    }
    string horiz_id = vsp::FindGeom( "Tail", 0 );
    if (!horiz_id.empty())
    {
        printf("\t\tTail Found.\n");
    }
    string elevator_id = vsp::GetSubSurf( horiz_id, 0 );
    if (!elevator_id.empty())
    {
        printf("\t\tElevator Found.\n");
    }
    printf("\tCOMPLETE.\n");

    double gain0 = 0.1;
    double angle0 = 1.0;
    double gain1 = 0.2;
    double angle1 = 2.0;
    double gain2 = 0.3;
    double angle2 = 3.0;

    // ControlSurfaceGroup_0
    printf("\tChecking All Parms for ControlSurfaceGroup_0...");
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron1_id + "_0_Gain", "ControlSurfaceGroup_0");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), gain0, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron1_id + "_1_Gain", "ControlSurfaceGroup_0");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), -gain0, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_0");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), angle0, TEST_TOL );
    printf("COMPLETE.\n");

    // ControlSurfaceGroup_1
    printf("\tChecking All Parms for ControlSurfaceGroup_1...");
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron2_id + "_0_Gain", "ControlSurfaceGroup_1");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), gain1, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron2_id + "_1_Gain", "ControlSurfaceGroup_1");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), -gain1, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_1");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), angle1, TEST_TOL );
    printf("COMPLETE.\n");

    // ControlSurfaceGroup_2
    printf("\tChecking All Parms for ControlSurfaceGroup_2...");
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + elevator_id + "_0_Gain", "ControlSurfaceGroup_2");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), gain2, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + elevator_id + "_1_Gain", "ControlSurfaceGroup_2");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), -gain2, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_2");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), angle2, TEST_TOL );
    printf("COMPLETE.\n");

    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    printf( "COMPLETE\n\n" );
}

void APITestSuiteVSPAERO::TestVSPAeroReadRotorDisksFromFile()
{
    // TODO - Need to Figure Out how to put Disks into basic files
}

void APITestSuiteVSPAERO::TestVSPAeroParmContainersAccessibleAfterSave()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroParmContainersAccessibleAfterSave()\n" );

    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateFunctionalityModel
    printf("\tLoading File...");
    vsp::ReadVSPFile( m_vspfname_for_vspaerofunctionalitytests );
    if ( m_vspfname_for_vspaerofunctionalitytests == string() )
    {
        TEST_FAIL( "m_vspfname_for_vspaerofunctionalitytests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeom" );
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "m_vspfname_for_vspaerofunctionalitytests failed to open" );
        return;
    }
    printf("COMPLETE.\n");

    //==== Save Vehicle to File ====//
    printf( "\tSaving vehicle file to: %s ...", m_vspfname_for_vspaerofunctionalitytests.c_str() );
    vsp::WriteVSPFile( vsp::GetVSPFileName(), vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "COMPLETE\n" );

    //==== Check if ParmContainers are still accessible ====//
    string deflection_gain_id, deflection_angle_id;
    string control_group_settings_container_id = vsp::FindContainer( "VSPAEROSettings", 0 );   // auto grouping produces parm containers within VSPAEROSettings

    // Find All Control Surface IDs to be used in Gain Parm Names
    printf("\tFinding All Geometry IDs...\n");
    string wing_id = vsp::FindGeom( "MainWing", 0 );
    if (!wing_id.empty())
    {
        printf("\t\tMainWing Found.\n");
    }
    string aileron1_id = vsp::GetSubSurf( wing_id, 0 );
    if (!aileron1_id.empty())
    {
        printf("\t\tInner Aileron Found.\n");
    }
    string aileron2_id = vsp::GetSubSurf( wing_id, 1 );
    if (!aileron2_id.empty())
    {
        printf("\t\tOuter Aileron Found.\n");
    }
    string horiz_id = vsp::FindGeom( "Tail", 0 );
    if (!horiz_id.empty())
    {
        printf("\t\tTail Found.\n");
    }
    string elevator_id = vsp::GetSubSurf( horiz_id, 0 );
    if (!elevator_id.empty())
    {
        printf("\t\tElevator Found.\n");
    }
    printf("\tCOMPLETE.\n");

    double gain0 = 0.1;
    double angle0 = 1.0;
    double gain1 = 0.2;
    double angle1 = 2.0;
    double gain2 = 0.3;
    double angle2 = 3.0;

    // ControlSurfaceGroup_0
    printf("\tChecking All Parms for ControlSurfaceGroup_0...");
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron1_id + "_0_Gain", "ControlSurfaceGroup_0");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), gain0, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron1_id + "_1_Gain", "ControlSurfaceGroup_0");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), -gain0, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_0");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), angle0, TEST_TOL );
    printf("COMPLETE.\n");

    // ControlSurfaceGroup_1
    printf("\tChecking All Parms for ControlSurfaceGroup_1...");
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron2_id + "_0_Gain", "ControlSurfaceGroup_1");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), gain1, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + aileron2_id + "_1_Gain", "ControlSurfaceGroup_1");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), -gain1, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_1");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), angle1, TEST_TOL );
    printf("COMPLETE.\n");

    // ControlSurfaceGroup_2
    printf("\tChecking All Parms for ControlSurfaceGroup_2...");
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + elevator_id + "_0_Gain", "ControlSurfaceGroup_2");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), gain2, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "Surf_" + elevator_id + "_1_Gain", "ControlSurfaceGroup_2");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), -gain2, TEST_TOL );
    deflection_gain_id = vsp::FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_2");
    TEST_ASSERT_DELTA( vsp::GetParmVal( deflection_gain_id ), angle2, TEST_TOL );
    printf("COMPLETE.\n");
}

void APITestSuiteVSPAERO::TestVSPAeroCpSlicer()
{
    printf( "APITestSuiteVSPAERO::TestVSPAeroCpSlicer()\n" );

    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeomPanel" );
        return;
    }
    if ( vsp::ErrorMgr.PopErrorAndPrint( stdout ) )
    {
        TEST_FAIL( "m_vspfname_for_vspaerotests failed to open" );
        return;
    }

    //==== Analysis: CpSlicer ====//
    string analysis_name = "CpSlicer";
    printf( "\t%s\n", analysis_name.c_str() );
    // Set defaults
    vsp::SetAnalysisInputDefaults( analysis_name );

    // Setup cuts
    vector < double > ycuts;
    ycuts.push_back( 2.0 );
    ycuts.push_back( 4.5 );
    ycuts.push_back( 8.0 );

    vsp::SetDoubleAnalysisInput( analysis_name, "YSlicePosVec", ycuts, 0 );

    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf( "\n\t\tExecuting..." );
    string results_id = vsp::ExecAnalysis( analysis_name );
    TEST_ASSERT( results_id.size() > 0 );
    printf( "COMPLETE\n\n" );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf( "\n" );
}

double  APITestSuiteVSPAERO::calcTessWCheckVal( double t_tess_w )
{
    double t_mult = 4;
    double t_shift = 1;

    return t_mult * std::ceil( ( t_tess_w - t_shift ) / t_mult ) + t_shift;
}
