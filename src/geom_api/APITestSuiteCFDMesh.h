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
    }

private:

    void TestCFDHalfMesh();
};

#endif // !defined(VSPAPITESTSUITECFDMESH__INCLUDED_)
