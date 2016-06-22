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
        TEST_ADD( APITestSuite::CheckSetup)
        // Geometry manipulation
        TEST_ADD( APITestSuite::CreateGeometry)
        TEST_ADD( APITestSuite::ChangePodParams)
        TEST_ADD( APITestSuite::CopyPasteGeometry)
        // Analysis
        TEST_ADD( APITestSuite::CheckAnalysisMgr)
        TEST_ADD( APITestSuite::TestAnalysesWithPod)

        // VSPAERO
        TEST_ADD( APITestSuite::TestVSPAeroComputeGeom)
        TEST_ADD( APITestSuite::TestVSPAeroSinglePoint)
        TEST_ADD( APITestSuite::TestVSPAeroSinglePointStab)
        TEST_ADD( APITestSuite::TestVSPAeroSweep)
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
    // VSPAERO
    void TestVSPAeroComputeGeom();        //<--Execute this VSPERO test first
    void TestVSPAeroSinglePoint();
    void TestVSPAeroSinglePointStab();
    void TestVSPAeroSweep();

    // Helper functions
    void PrintAnalysisInputs(const string analysis_name);
    void PrintResults(const vector < string > &results_id_vec );
    void PrintResults(const string &results_id);

    string m_vspfname_for_vspaerotests;
};

#endif // !defined(VSPAPITESTSUITE__INCLUDED_)
