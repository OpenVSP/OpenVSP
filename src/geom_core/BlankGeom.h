//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPBLANKGEOM__INCLUDED_)
#define VSPBLANKGEOM__INCLUDED_

#include "Geom.h"


//==== Blank Geom ====//
class BlankGeom : public Geom
{
public:
    BlankGeom( Vehicle* vehicle_ptr );
    virtual ~BlankGeom();

    virtual int GetNumMainSurfs()
    {
        return 0;
    };

    virtual void UpdateSurf();

    BoolParm m_PointMassFlag;
    Parm     m_PointMass;
    vec3d m_Origin;

protected:

};

#endif // !defined(VSPBLANKGEOM__INCLUDED_)
