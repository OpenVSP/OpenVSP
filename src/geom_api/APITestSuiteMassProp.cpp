//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// APITestSuiteMassProp.cpp: Unit tests for geom_api
// Justin Gravett
//
//////////////////////////////////////////////////////////////////////


#include "VSP_Geom_API.h"
#include "APITestSuiteMassProp.h"
#include <stdio.h>
#include <float.h>
#include "APIDefines.h"

//Default tolerance to use for tests.  Most calculations are done as doubles and choosing single precision FLT_MIN gives some allowance for precision stackup in calculations
#define TEST_TOL FLT_MIN
 
double APITestSuiteMassProp::GetTol( double val )
{
    double TOL;

    if ( val != 0 )
    {
        TOL = val * 0.005;
    }
    else if ( val == 0 )
    { 
        TOL = 0.1;
    }

    return TOL;
}

double APITestSuiteMassProp::GetCGTol( double val, double mass )
{
    double TOL;

    if ( val != 0 )
    {
        TOL = val * 0.001;
    }
    else if ( val == 0 )
    { 
        TOL = 0.005 * mass;
    }

    return TOL;
}

double APITestSuiteMassProp::GetInertiaTol( double val, double mass )
{
    double TOL;

    if ( val != 0 )
    {
        TOL = val * 0.01;
    }
    else if ( val == 0 )
    { 
        TOL = 0.005 * mass;
    }

    return TOL;
}
