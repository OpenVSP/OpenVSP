//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// EllipsoidGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_ELLIPSOID_GEOM__INCLUDED_)
#define VSP_ELLIPSOID_GEOM__INCLUDED_

#include "Geom.h"
#include "XSec.h"


//==== Ellipsoid Geom ====//
class EllipsoidGeom : public Geom
{
public:
    EllipsoidGeom( Vehicle* vehicle_ptr );
    virtual ~EllipsoidGeom();

    virtual void ComputeCenter();
    virtual void Scale();
    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual void OffsetXSecs( double off );

    Parm m_Aradius;
    Parm m_Bradius;
    Parm m_Cradius;

protected:
    virtual void UpdateSurf();

    double m_Xoff;
};


#endif // !defined(VSP_ELLIPSOID_GEOM__INCLUDED_)
