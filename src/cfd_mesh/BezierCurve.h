//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Bezier Curve Class  (Cubic)
//
//
//   J.R. Gloudemans - 7/20/94
//   Sterling Software
//
//******************************************************************************


#ifndef BEZIER_CURVE_H
#define BEZIER_CURVE_H

#include "Vec3d.h"

#include <vector>               //jrg windows?? 
#include <stdio.h>
#include <math.h>

using namespace std;            //jrg windows??

class Bezier_curve
{
protected:

    int num_sections;
    vector <vec3d> pnts;

    void blend_funcs( double u, double& F1, double& F2, double& F3, double& F4 );
    vec3d comp_pnt( int sec_num, double u );      // Section and u between 0 and 1

public:

    Bezier_curve();
    ~Bezier_curve();

    int  get_num_sections()
    {
        return( num_sections );
    }
    int  get_num_control_pnts()
    {
        return pnts.size();
    }
    void put_pnts( const vector< vec3d > &pnts_in );
    vec3d& get_pnt( int ind )
    {
        return( pnts[ind] );
    }

    vec3d& first_pnt()  // Could be implemented with comp_pnt, but should be faster/more accurate.
    {
        return pnts[0];
    }

    vec3d& last_pnt()
    {
        return pnts.back();
    }

    vec3d comp_pnt( double u );                   // u between 0 and 1

    void buildCurve( const vector< vec3d > & pVec, double tanStr, int closeFlag );
    void flipCurve();
};

#endif


