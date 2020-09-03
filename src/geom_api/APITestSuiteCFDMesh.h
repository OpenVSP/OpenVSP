//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// APITestSuiteCFDMesh.h: Unit tests for geom_api
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPAPITESTSUITECFDMESH__INCLUDED_)
#define VSPAPITESTSUITECFDMESH__INCLUDED_

#include "cpptest.h"

class APITestSuiteCFDMesh : public Test::Suite
{
public:

    APITestSuiteCFDMesh()
    {
        TEST_ADD( APITestSuiteCFDMesh::TestCFDHalfMesh )
        TEST_ADD( APITestSuiteCFDMesh::TestSurfaceIntersection )
        TEST_ADD( APITestSuiteCFDMesh::CFDMeshAnalysisTest)
        TEST_ADD( APITestSuiteCFDMesh::FEAMeshAnalysisTest)
    }

private:

    void TestCFDHalfMesh();
    void TestSurfaceIntersection();
    int  GetFileSize( string file_name );
    void RunAnalysis( vector < string > file_1, vector < string > file_2 );
    void RunScaleTest( double scale_value, double test_compare_percent );
    void CFDMeshAnalysisTest();
    void FEAMeshAnalysisTest();
};

#endif // !defined(VSPAPITESTSUITECFDMESH__INCLUDED_)
