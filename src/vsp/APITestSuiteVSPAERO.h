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
    // The solves of the apitest_TestVSPAero model crash vspaero now and then (in VSP_EDGE::Xc()
    // and CDwForCase), and the unsteady one takes minutes, so they run apart from the rest: with
    // unstable, the suite is those solves, the geometry they read and the CpSlicer test that reads
    // their results.
    APITestSuiteVSPAERO( bool unstable = false )
    {
        // VSPAERO
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroCreateModel )     //this test just creates the model with various features
        if ( unstable )
        {
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
            // CpSlicer Tests
            TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroCpSlicer );
            return;
        }
        //  Geometry for the Vortex Lattice and Panel Methods
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroComputeGeom )
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroComputeGeomPanel )
        //  Verification Tests
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSharpTrailingEdge );
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroBluntTrailingEdge );
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroSupersonicDeltaWing );
        // Functionality Tests
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroCreateFunctionalityModel );
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroReadControlSurfaceGroupsFromFile );
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroReadRotorDisksFromFile );
        TEST_ADD( APITestSuiteVSPAERO::TestVSPAeroParmContainersAccessibleAfterSave );
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
    // Functionality Tests
    void TestVSPAeroCreateFunctionalityModel();
    void TestVSPAeroReadControlSurfaceGroupsFromFile();
    void TestVSPAeroReadRotorDisksFromFile();
    void TestVSPAeroParmContainersAccessibleAfterSave();
    // CpSlicer Test
    void TestVSPAeroCpSlicer();

    static double calcTessWCheckVal( double t_tess_w );

    string m_vspfname_for_vspaerotests;
    string m_vspfname_for_vspaerofunctionalitytests;
};

#endif // !defined(VSPAPITESTSUITEVSPAERO__INCLUDED_)