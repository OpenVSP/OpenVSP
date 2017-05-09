//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// APITestSuiteParasiteDrag.h: Unit tests for geom_api
// B. Schmidt
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPAPITESTSUITEPARASITEDRAG__INCLUDED_)
#define VSPAPITESTSUITEPARASITEDRAG__INCLUDED_

#include "cpptest.h"

class APITestSuiteParasiteDrag : public Test::Suite
{
public:
    APITestSuiteParasiteDrag()
    {
        TEST_ADD( APITestSuiteParasiteDrag::TestParasiteDragCreateModel );
        TEST_ADD( APITestSuiteParasiteDrag::TestFirstParasiteDragCalc );
        TEST_ADD( APITestSuiteParasiteDrag::TestAddExcrescence );
        TEST_ADD( APITestSuiteParasiteDrag::TestSecondParasiteDragCalc );
        TEST_ADD( APITestSuiteParasiteDrag::TestChangeOptions );
        TEST_ADD( APITestSuiteParasiteDrag::TestRevertToSimpleModel );
        TEST_ADD( APITestSuiteParasiteDrag::TestS3VikingModel );
    }

private:

    void TestParasiteDragCreateModel();
    void TestFirstParasiteDragCalc();
    void TestAddExcrescence();
    void TestSecondParasiteDragCalc();
    void TestChangeOptions();
    void TestRevertToSimpleModel();
    void TestS3VikingModel();

    string m_vspfname_for_parasitedragtests;
};

#endif // !defined(VSPAPITESTSUITEPARASITEDRAG__INCLUDED_)
