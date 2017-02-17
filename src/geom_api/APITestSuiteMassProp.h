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

private:

    // Test Tolerance
    double GetTol( double val );
    double GetCGTol( double val, double mass );
    double GetInertiaTol( double val, double mass );

};

#endif // !defined(VSPAPITESTSUITEMASSPROP__INCLUDED_)