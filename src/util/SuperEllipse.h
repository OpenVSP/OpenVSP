//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SupperEllipse.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_SUPERELLIPSE_H
#define VSP_SUPERELLIPSE_H

#include "Vec3d.h"

#include <vector>

class SuperEllipse
{
public:
    SuperEllipse();
    virtual ~SuperEllipse();

    std::vector< vec3d > Build( int num_pnts, double a, double b );
    std::vector< vec3d > Build( int num_pnts, double a, double b, double m, double n );

protected:

    void BuildCosSinTables( int num_pnts );
    std::vector< double > m_CosU;
    std::vector< double > m_SinU;
};



#endif


