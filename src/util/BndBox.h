//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Bounding Box  BndBox.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(BNDBOX__INCLUDED_)
#define BNDBOX__INCLUDED_

#include "Vec3d.h"

#include <vector>

class BndBox;
bool Compare( const BndBox& bb1, const BndBox& bb2, double tol = 1.0e-12 );

class VSPDLL BndBox
{
public:

    BndBox();
    BndBox( const vec3d& min_pnt, const vec3d& max_pnt );

    void Reset();
    void SetMax( int ind, double val );
    void SetMin( int ind, double val );
    double GetMax( int ind ) const;
    double GetMin( int ind ) const;
    vec3d GetMax() const
    {
        return m_Max;
    }
    vec3d GetMin() const
    {
        return m_Min;
    }

    void Update( const vec3d& pnt );
    void Update( const BndBox& bb );
    double DiagDist() const;
    double GetLargestDist() const;
    double GetSmallestDist() const;
    double GetEstArea() const;
    vec3d GetCenter() const;
    vec3d GetCornerPnt( int  i ) const;
    std::vector< vec3d > GetCornerPnts() const;

    void Expand( double val );
    void Scale( const vec3d & scale_xyz );

    bool CheckPnt( double x, double y, double z ) const;
    bool CheckPnt( const vec3d & pnt ) const;
    friend bool Compare( const BndBox& bb1, const BndBox& bb2, double tol );

    std::vector< vec3d > GetBBoxDrawLines() const;

protected:

    vec3d m_Min;
    vec3d m_Max;

};

#endif
