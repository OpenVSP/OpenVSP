//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// APITestSuite.h: Unit tests for geom_api
// N. Brake
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPAPITESTSUITE__INCLUDED_)
#define VSPAPITESTSUITE__INCLUDED_


#include "cpptest.h"

class APITestSuite : public Test::Suite
{
public:
    APITestSuite()
    {
        // General
        TEST_ADD( APITestSuite::CheckSetup )
        // Geometry manipulation
        TEST_ADD( APITestSuite::CreateGeometry )
        TEST_ADD( APITestSuite::ChangePodParams )
        TEST_ADD( APITestSuite::CopyPasteGeometry )
        // Analysis
        TEST_ADD( APITestSuite::CheckAnalysisMgr )
        TEST_ADD( APITestSuite::TestAnalysesWithPod )

        // Export
        TEST_ADD( APITestSuite::TestDXFExport )
        TEST_ADD( APITestSuite::TestSVGExport )
        TEST_ADD( APITestSuite::TestFacetExport )
        // Save and Load
        TEST_ADD( APITestSuite::TestSaveLoad)
    }

private:
    // General
    void CheckSetup();
    // Geometry manipulation
    void CreateGeometry();
    void ChangePodParams();
    void CopyPasteGeometry();
    // Analysis
    void CheckAnalysisMgr();
    void TestAnalysesWithPod();
    // Export
    void TestDXFExport();
    void TestSVGExport();
    void TestFacetExport();
    // Save and Load
    void TestSaveLoad();
};

class APITestSuiteVSPAERO : public Test::Suite
{
public:
    APITestSuiteVSPAERO()
    {
        // VSPAERO
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroCreateModel )     //this test just creates the model with various features
        //  Vortex Lattice Method Tests
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroComputeGeom )
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSinglePoint )
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSinglePointStab )
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSweep )
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSweepBatch )
        //  Panel Method Tests
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroComputeGeomPanel )
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSinglePointPanel )
    }

private:
    // VSPAERO
    void TestVSPAeroCreateModel();
    //  Vortex Lattice Method Tests
    void TestVSPAeroComputeGeom();        //<--Execute this VSPERO test first
    void TestVSPAeroSinglePoint();
    void TestVSPAeroSinglePointStab();
    void TestVSPAeroSweep();
    void TestVSPAeroSweepBatch();
    //  Panel Method Tests
    void TestVSPAeroComputeGeomPanel();        //<--Execute this VSPERO test first for panel methods
    void TestVSPAeroSinglePointPanel();

    double  calcTessWCheckVal( double t_tess_w );

    string m_vspfname_for_vspaerotests;
};

#endif // !defined(VSPAPITESTSUITE__INCLUDED_)
