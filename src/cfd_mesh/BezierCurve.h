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
#include "Surf.h"

#include <vector>               //jrg windows?? 
#include <stdio.h>
#include <math.h>

#include "eli/code_eli.hpp"

#include "eli/geom/curve/bezier.hpp"
#include "eli/geom/curve/piecewise.hpp"

typedef eli::geom::curve::bezier<double, 3> curve_segment_type;
typedef eli::geom::curve::piecewise<eli::geom::curve::bezier, double, 3> piecewise_curve_type;

using namespace std;            //jrg windows??

class Bezier_curve
{
protected:

    piecewise_curve_type m_Curve;

public:

    Bezier_curve();
    ~Bezier_curve();

    int  get_num_sections() const
    {
        return m_Curve.number_segments();
    }

    void put_pnts( const vector< vec3d > &pnts_in );

    vec3d first_pnt() const;
    vec3d last_pnt() const;

    vec3d comp_pnt( double u ) const;                   // u between 0 and 1

    void buildCurve( const vector< vec3d > & pVec );
    void flipCurve();

    void UWCurveToXYZCurve( const Surf *srf );
    void XYZCurveToUWCurve( const Surf *srf );

    bool MatchFwd( const Bezier_curve &ocrv ) const;
    bool MatchBkwd( const Bezier_curve &ocrv ) const;
    bool MatchFwd( const Bezier_curve &ocrv, double tol ) const;
    bool MatchBkwd( const Bezier_curve &ocrv, double tol ) const;
};

#endif


