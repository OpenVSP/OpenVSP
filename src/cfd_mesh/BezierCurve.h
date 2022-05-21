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
#include <cstdio>
#include <cmath>

#include "eli/code_eli.hpp"

#include "eli/geom/curve/bezier.hpp"
#include "eli/geom/curve/piecewise.hpp"

typedef eli::geom::curve::bezier<double, 3> curve_segment_type;
typedef eli::geom::curve::piecewise<eli::geom::curve::bezier, double, 3> piecewise_curve_type;
typedef piecewise_curve_type::point_type curve_point_type;

using namespace std;            //jrg windows??

class Bezier_curve
{
protected:

    piecewise_curve_type m_Curve;

public:

    Bezier_curve();
    Bezier_curve( const piecewise_curve_type &crv );
    ~Bezier_curve();

    int  GetNumSections() const
    {
        return m_Curve.number_segments();
    }

    vec3d FirstPnt() const;
    vec3d LastPnt() const;

    vec3d CompPnt01( double u ) const;

    void TessAdaptXYZ( const Surf& srf, std::vector< vec3d >& output, double tol, int Nlimit );
    void TessAdaptXYZ( const Surf& srf, std::vector< vec3d >& output, double tol, int Nlimit, std::vector< double >& uvec );
    void TessAdaptXYZ( const Surf &srf, double umin, double umax, std::vector< vec3d > & pnts, double tol, int Nlimit, std::vector< double >& uvec );
    void TessAdaptXYZ( const Surf &srf, double umin, double umax, const vec3d & pmin, const vec3d & pmax, std::vector< vec3d > & pnts, double tol, int Nlimit, std::vector< double >& uvec, int Nadapt = 0 );

    void InterpolateLinear( const vector< vec3d > & input_pnt_vec );
    void PromoteTo( int deg );

    void FlipCurve();
    void BuildWakeTECurve( const piecewise_curve_type& lecrv, double endx, double angle, double start_stretch_x, double scale );
    static curve_point_type ComputeWakeTrailEdgePnt( const curve_point_type &pnt, double endx, double angle );

    void UWCurveToXYZCurve( const Surf *srf );
    void XYZCurveToUWCurve( const Surf *srf );

    int CountMatch( const Bezier_curve &ocrv, double tol ) const;
    bool Match( const Bezier_curve &ocrv, double tol ) const;
    bool MatchFwd( const Bezier_curve &ocrv, double tol ) const;
    bool MatchBkwd( const Bezier_curve &ocrv, double tol ) const;

    bool SingleLinear();

    void GetBBox( BndBox &box );

    void GetControlPoints( vector< vec3d > &pnts_out );

    piecewise_curve_type GetCurve() const
    {
        return m_Curve;
    }
};

#endif


