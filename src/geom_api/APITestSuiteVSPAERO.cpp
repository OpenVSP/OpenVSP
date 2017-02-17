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
#include <stdio.h>
#include <float.h>
#include "APIDefines.h"

//Default tolerance to use for tests.  Most calculations are done as doubles and choosing single precision FLT_MIN gives some allowance for precision stackup in calculations
#define TEST_TOL FLT_MIN

//=============================================================================//
//========================== APITestSuiteVSPAERO ==============================//
//=============================================================================//

//===== Create geometry model  =====//
void APITestSuiteVSPAERO::TestVSPAeroCreateModel()
{
    printf("APITestSuiteVSPAERO::TestVSPAeroCreateModel()\n");

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Add Wing Geom and set some parameters =====//
    printf("\tAdding WING (Main)\n");
    string wing_id = vsp::AddGeom( "WING");
    vsp::SetGeomName( wing_id, "MainWing" );
    TEST_ASSERT( wing_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TotalSpan", "WingGeom", 17.0 ), 17.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "Z_Rel_Location", "XForm", 0.5 ), 0.5, TEST_TOL);
    // Adjust chordwise tesselation
    //TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "Tess_W", "Shape", 33 ), 33, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "LECluster", "WingGeom", 0.0 ), 0.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "TECluster", "WingGeom", 2.0 ), 2.0, TEST_TOL);
    // Adjust spanwise tesselation
    //TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "SectTess_U", "XSec_1", 25 ), 25, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "InCluster", "XSec_1", 0.1 ), 0.1, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "OutCluster", "XSec_1", 0.0 ), 0.0, TEST_TOL);
    string subsurf_id = AddSubSurf( wing_id, vsp::SS_CONTROL, 0 );
    TEST_ASSERT( subsurf_id.c_str() != NULL );
    /*
    printf("\tAdding POD\n");
    string pod_id = vsp::AddGeom( "POD");
    TEST_ASSERT( pod_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  pod_id, "Length", "Design", 14.5 ), 14.5, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  pod_id, "X_Rel_Location", "XForm", -3.0 ), 0.0, TEST_TOL);
    //TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "Tess_U", "Shape", 25 ), 25, TEST_TOL); //lengthwise tesselation
    //TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing_id, "Tess_W", "Shape", 25 ), 25, TEST_TOL); //radial tesselation
    */
    /*
    printf("\tAdding DISK\n");
    string disk_id = vsp::AddGeom( "DISK");
    TEST_ASSERT( disk_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  disk_id, "Diameter", "Design", 3.0 ), 3.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  disk_id, "Y_Rel_Location", "XForm", 3.5 ), 3.5, TEST_TOL);

    // Add tail with X-axis symetry and 3 total surfaces (Y-Tail configuration)
    printf("\tAdding WING (Tail)\n");
    string wing2_id = vsp::AddGeom( "WING");
    vsp::SetGeomName( wing2_id, "Tail" );
    TEST_ASSERT( wing2_id.c_str() != NULL );
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing2_id, "X_Rel_Location", "XForm", 9.0 ), 9.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing2_id, "X_Rel_Rotation", "XForm", 30.0 ), 60.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing2_id, "TotalSpan", "WingGeom", 5.0 ), 5.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing2_id, "LECluster", "WingGeom", 0.0 ), 0.0, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing2_id, "TECluster", "WingGeom", 2.0 ), 2.0, TEST_TOL);
    //TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing2_id, "SectTess_U", "XSec_1", 25 ), 25, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing2_id, "InCluster", "XSec_1", 0.1 ), 0.1, TEST_TOL);
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing2_id, "OutCluster", "XSec_1", 0.0 ), 0.0, TEST_TOL);
    // change symetry for tail to make Y shape tail
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing2_id, "Sym_Planar_Flag", "Sym", 0 ), 0, TEST_TOL); //no planar symetry
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing2_id, "Sym_Axial_Flag", "Sym", vsp::SYM_ROT_X ), vsp::SYM_ROT_X, TEST_TOL); // X-a
    TEST_ASSERT_DELTA( vsp::SetParmValUpdate(  wing2_id, "Sym_Rot_N", "Sym", 3 ), 3, TEST_TOL); //no planar symetry
    */
    //TODO Organize geometry into sets: 1 setf for VLM and 1 set for panel

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Setup export filenames ====//
    // Execution of one of these methods is required to propperly set the export filenames for creation of vspaero input files and execution commands
    m_vspfname_for_vspaerotests = "apitest_TestVSPAero.vsp3";
    printf("\tSetting export name: %s\n", m_vspfname_for_vspaerotests.c_str());
    vsp::SetVSP3FileName( m_vspfname_for_vspaerotests );  // this still needs to be done even if a call to WriteVSPFile is made
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //==== Save Vehicle to File ====//
    printf("\tSaving vehicle file to: %s ...\n", m_vspfname_for_vspaerotests.c_str());
    vsp::WriteVSPFile( vsp::GetVSPFileName(), vsp::SET_ALL );
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("COMPLETE\n");

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuiteVSPAERO::TestVSPAeroComputeGeom()
{
    printf("APITestSuiteVSPAERO::TestVSPAeroComputeGeom()\n");

    // make sure setup works
    vsp::VSPCheckSetup();        //TODO check that vspaero.exe is found
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
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

    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";
    printf("\t%s\n",analysis_name.c_str());

    // Set defaults
    vsp::SetAnalysisInputDefaults(analysis_name);

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs(analysis_name);

    // Execute
    printf("\tExecuting...\n");
    string results_id = vsp::ExecAnalysis(analysis_name);
    printf("COMPLETE\n");

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuiteVSPAERO::TestVSPAeroComputeGeomPanel()
{
    printf("APITestSuiteVSPAERO::TestVSPAeroComputeGeomPanel()\n");

    // make sure setup works
    vsp::VSPCheckSetup();        //TODO check that vspaero.exe is found
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
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
    vsp::PrintAnalysisInputs(analysis_name);

    // Execute
    printf("\tExecuting...\n");
    string results_id = vsp::ExecAnalysis(analysis_name);
    printf("COMPLETE\n");
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );

    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuiteVSPAERO::TestVSPAeroSinglePointPanel()
{
    printf("APITestSuiteVSPAERO::TestVSPAeroSinglePointPanel()\n");

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );
    if ( m_vspfname_for_vspaerotests == string() )
    {
        TEST_FAIL("m_vspfname_for_vspaerotests = NULL, need to run: APITestSuite::TestVSPAeroComputeGeomPanel");
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
    std::vector< int > ref_flag; ref_flag.push_back( vsp::VSPAERO_COMP_REFERENCE_TYPE::MANUAL_REF );
    vsp::SetIntAnalysisInput(analysis_name, "RefFlag", ref_flag, 0);
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
    //    freestream parameters
    std::vector< double > alpha; alpha.push_back(5);
    vsp::SetDoubleAnalysisInput(analysis_name, "Alpha", alpha, 0);
    std::vector< double > beta; beta.push_back(2.5);
    vsp::SetDoubleAnalysisInput(analysis_name, "Beta", beta, 0);
    std::vector< double > mach; mach.push_back(0.1);
    vsp::SetDoubleAnalysisInput(analysis_name, "Mach", mach, 0);
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs(analysis_name);

    // Execute
    printf("\tExecuting...\n");
    string results_id = vsp::ExecAnalysis(analysis_name);
    printf("COMPLETE\n");
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );
    
    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuiteVSPAERO::TestVSPAeroSinglePoint()
{
    printf("APITestSuiteVSPAERO::TestVSPAeroSinglePoint()\n");

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
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
    std::vector< int > ref_flag; ref_flag.push_back( vsp::VSPAERO_COMP_REFERENCE_TYPE::MANUAL_REF );
    vsp::SetIntAnalysisInput(analysis_name, "RefFlag", ref_flag, 0);
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back(10);
    vsp::SetDoubleAnalysisInput(analysis_name, "Sref", sref, 0);
    std::vector< double > bref; bref.push_back(17);
    vsp::SetDoubleAnalysisInput(analysis_name, "bref", bref, 0);
    std::vector< double > cref; cref.push_back(3);
    vsp::SetDoubleAnalysisInput(analysis_name, "cref", cref, 0);
    //    freestream parameters
    std::vector< double > alpha; alpha.push_back(5);
    vsp::SetDoubleAnalysisInput(analysis_name, "Alpha", alpha, 0);
    std::vector< double > beta; beta.push_back(2.5);
    vsp::SetDoubleAnalysisInput(analysis_name, "Beta", beta, 0);
    std::vector< double > mach; mach.push_back(0.1);
    vsp::SetDoubleAnalysisInput(analysis_name, "Mach", mach, 0);
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs(analysis_name);

    // Execute
    printf("\tExecuting...\n");
    string results_id = vsp::ExecAnalysis(analysis_name);
    printf("COMPLETE\n");
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );
    
    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuiteVSPAERO::TestVSPAeroSinglePointStab()
{
    printf("APITestSuiteVSPAERO::TestVSPAeroSinglePointStab()\n");

    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
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
    std::vector< int > ref_flag; ref_flag.push_back( vsp::VSPAERO_COMP_REFERENCE_TYPE::MANUAL_REF );
    vsp::SetIntAnalysisInput(analysis_name, "RefFlag", ref_flag, 0);
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back(10);
    vsp::SetDoubleAnalysisInput(analysis_name, "Sref", sref, 0);
    std::vector< double > bref; bref.push_back(17);
    vsp::SetDoubleAnalysisInput(analysis_name, "bref", bref, 0);
    std::vector< double > cref; cref.push_back(3);
    vsp::SetDoubleAnalysisInput(analysis_name, "cref", cref, 0);
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
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs(analysis_name);

    // Execute
    printf("\tExecuting...\n");
    string results_id = vsp::ExecAnalysis(analysis_name);
    printf("COMPLETE\n");
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );
    
    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuiteVSPAERO::TestVSPAeroSweep()
{
    printf("APITestSuiteVSPAERO::TestVSPAeroSweep()\n");
    
    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
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
    std::vector< int > ref_flag; ref_flag.push_back( vsp::VSPAERO_COMP_REFERENCE_TYPE::MANUAL_REF );
    vsp::SetIntAnalysisInput(analysis_name, "RefFlag", ref_flag, 0);
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back(10);
    vsp::SetDoubleAnalysisInput(analysis_name, "Sref", sref, 0);
    std::vector< double > bref; bref.push_back(17);
    vsp::SetDoubleAnalysisInput(analysis_name, "bref", bref, 0);
    std::vector< double > cref; cref.push_back(3);
    vsp::SetDoubleAnalysisInput(analysis_name, "cref", cref, 0);
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
    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs( analysis_name );

    // Execute
    printf("\tExecuting...\n");
    string results_id = vsp::ExecAnalysis(analysis_name);
    printf("COMPLETE\n");
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );
    
    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}

void APITestSuiteVSPAERO::TestVSPAeroSweepBatch()
{
    printf("APITestSuiteVSPAERO::TestVSPAeroSweepBatch()\n");
    
    // make sure setup works
    vsp::VSPCheckSetup();
    vsp::VSPRenew();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    //open the file created in TestVSPAeroCreateModel
    vsp::ReadVSPFile( m_vspfname_for_vspaerotests );    // Sets VSP3 file name
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
    std::vector< int > ref_flag; ref_flag.push_back( vsp::VSPAERO_COMP_REFERENCE_TYPE::MANUAL_REF );
    vsp::SetIntAnalysisInput(analysis_name, "RefFlag", ref_flag, 0);
    //    Reference areas, lengths
    std::vector< double > sref; sref.push_back(10);
    vsp::SetDoubleAnalysisInput(analysis_name, "Sref", sref, 0);
    std::vector< double > bref; bref.push_back(17);
    vsp::SetDoubleAnalysisInput(analysis_name, "bref", bref, 0);
    std::vector< double > cref; cref.push_back(3);
    vsp::SetDoubleAnalysisInput(analysis_name, "cref", cref, 0);
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
    //        Set Batch Mode
    std::vector< int > batch_mode_flag; batch_mode_flag.push_back(1);
    vsp::SetIntAnalysisInput(analysis_name, "BatchModeFlag", batch_mode_flag, 0);

    vsp::Update();
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // list inputs, type, and current values
    vsp::PrintAnalysisInputs(analysis_name);

    // Execute
    printf("\tExecuting...\n");
    string results_id = vsp::ExecAnalysis(analysis_name);
    printf("COMPLETE\n");
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

    // Get & Display Results
    vsp::PrintResults( results_id );
    
    // Final check for errors
    TEST_ASSERT( !vsp::ErrorMgr.PopErrorAndPrint( stdout ) );    //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE
    printf("\n");
}