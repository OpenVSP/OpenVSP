//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// APITestSuiteMassProp.h: Unit tests for geom_api
// Justin Gravett
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPAPITESTSUITEMASSPROP__INCLUDED_)
#define VSPAPITESTSUITEMASSPROP__INCLUDED_

#include "cpptest.h"

class APITestSuiteMassProp : public Test::Suite
{
public:
    APITestSuiteMassProp()
    {
        // Analysis
        TEST_ADD( APITestSuiteMassProp::TestSolidCylinder )
        TEST_ADD( APITestSuiteMassProp::TestThickWallCylinder )
        TEST_ADD( APITestSuiteMassProp::TestCylindricalShell )
        TEST_ADD( APITestSuiteMassProp::TestSphere )
        TEST_ADD( APITestSuiteMassProp::TestSolidCone )
        TEST_ADD( APITestSuiteMassProp::TestShellCone )
        TEST_ADD( APITestSuiteMassProp::TestRectangularPrism )
    }

private:

    // Test Tolerance
    double GetTol( double val );
    double GetCGTol( double val, double mass );
    double GetInertiaTol( double val, double mass );

    // Analysis
    void TestSolidCylinder();
    void TestThickWallCylinder();
    void TestCylindricalShell();
    void TestSphere();
    void TestSolidCone();
    void TestShellCone();
    void TestRectangularPrism();
};

#endif // !defined(VSPAPITESTSUITEMASSPROP__INCLUDED_)