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

    virtual void UpdateSurf();
    virtual void UpdateDrawObj();
    virtual void LoadMainDrawObjs(vector< DrawObj* > & draw_obj_vec);
    virtual void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);

    virtual void UpdateSymmAttach();

    virtual void ReadV2File( xmlNodePtr &root );

    BoolParm m_BlankPointMassFlag;
    Parm     m_BlankPointMass;

protected:

};

#endif // !defined(VSPBLANKGEOM__INCLUDED_)
