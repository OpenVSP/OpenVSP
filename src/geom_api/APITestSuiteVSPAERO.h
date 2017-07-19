//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// APITestSuiteVSPAERO.h: Unit tests for geom_api
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPAPITESTSUITEVSPAERO__INCLUDED_)
#define VSPAPITESTSUITEVSPAERO__INCLUDED_

#include "cpptest.h"

class APITestSuiteVSPAERO : public Test::Suite
{
public:
    APITestSuiteVSPAERO()
    {
        // VSPAERO
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroCreateModel )     //this test just creates the model with various features
        //  Control Surface Deflection Test
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroControlSurfaceDeflection )
        //  Vortex Lattice Method Tests
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroComputeGeom )
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSinglePoint )
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSinglePointStab )
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSinglePointUnsteady );
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSweep )
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSweepBatch )
        //  Panel Method Tests
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroComputeGeomPanel )
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSinglePointPanel )
        //  Verification Tests
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSharpTrailingEdge );
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroBluntTrailingEdge );
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSupersonicDeltaWing );
    }

private:
    // VSPAERO
    void TestVSPAeroCreateModel();
    //  Vortex Lattice Method Tests
    void TestVSPAeroComputeGeom();        //<--Execute this VSPERO test first
    void TestVSPAeroControlSurfaceDeflection();
    void TestVSPAeroSinglePoint();
    void TestVSPAeroSinglePointStab();
    void TestVSPAeroSinglePointUnsteady();
    void TestVSPAeroSweep();
    void TestVSPAeroSweepBatch();
    //  Panel Method Tests
    void TestVSPAeroComputeGeomPanel();        //<--Execute this VSPERO test first for panel methods
    void TestVSPAeroSinglePointPanel();
    //  Verification Tests
    void TestVSPAeroSharpTrailingEdge();
    void TestVSPAeroBluntTrailingEdge();
    void TestVSPAeroSupersonicDeltaWing();

    double calcTessWCheckVal( double t_tess_w );

    string m_vspfname_for_vspaerotests;
};

#endif // !defined(VSPAPITESTSUITEVSPAERO__INCLUDED_)