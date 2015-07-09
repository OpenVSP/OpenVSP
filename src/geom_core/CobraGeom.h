//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CobraGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_COBRA_GEOM__INCLUDED_)
#define VSP_COBRA_GEOM__INCLUDED_

#include "Geom.h"

//==== Pod Geom ====//
class CobraGeom : public Geom
{
public:
    CobraGeom( Vehicle* vehicle_ptr );
    virtual ~CobraGeom();

    virtual void ComputeCenter();
    virtual void Scale();
    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual void OffsetXSecs( double off );

    Parm m_Length;
    Parm m_FineRatio;

protected:
    virtual void UpdateSurf();

    double m_Xoff;
};


#endif // !defined(VSP_COBRA_GEOM__INCLUDED_)
