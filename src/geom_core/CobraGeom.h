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

    Parm m_XradN;
    Parm m_YradN;
    Parm m_PowX;
    Parm m_ZradNL;
    Parm m_PowNL;
    BoolParm m_NoseULSymm;
    Parm m_ZradNU;
    Parm m_PowNU;

    Parm m_XlenA;
    Parm m_YradA;
    Parm m_ZradAL;
    Parm m_PowAL;
    BoolParm m_AftULSymm;
    Parm m_ZradAU;
    Parm m_PowAU;

protected:
    virtual void UpdateSurf();

    double m_Xoff;
};


#endif // !defined(VSP_COBRA_GEOM__INCLUDED_)
