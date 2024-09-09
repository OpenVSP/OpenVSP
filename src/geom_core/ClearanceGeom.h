//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ClearanceGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////



#if !defined(VSPCLEARANCEGEOM__INCLUDED_)
#define VSPCLEARANCEGEOM__INCLUDED_

#include "Defines.h"
#include "Geom.h"
#include "XSec.h"
#include "XSecSurf.h"


//==== Clearance Geom ====//
class ClearanceGeom : public Geom
{
public:
    ClearanceGeom( Vehicle* vehicle_ptr );
    virtual ~ClearanceGeom();

    virtual void ComputeCenter();

    virtual void Scale();

    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual void OffsetXSecs( double off );


protected:

    virtual void UpdateSurf();
    virtual void CopyDataFrom( Geom* geom_ptr );

    virtual void UpdateDrawObj();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

};

#endif // !defined(VSPCLEARANCEGEOM__INCLUDED_)
