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
        TEST_ADD( APITestSuite::TestSaveLoad )
        // FEA Mesh
        TEST_ADD( APITestSuite::TestFEAMesh )
        // XSec
        TEST_ADD( APITestSuite::TestEditXSec )
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
    // FEA Mesh
    void TestFEAMesh();
    // XSec
    void TestEditXSec();
};

#endif // !defined(VSPAPITESTSUITE__INCLUDED_)
