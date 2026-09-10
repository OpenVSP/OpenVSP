//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CloneGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPCLONEGEOM__INCLUDED_)
#define VSPCLONEGEOM__INCLUDED_

#include "Geom.h"


//==== Clone Geom ====//
class CloneGeom : public Geom
{
public:
    CloneGeom( Vehicle* vehicle_ptr );
    virtual ~CloneGeom();

    virtual void ComputeCenter() override;
    virtual void AddDefaultSources( double base_len = 1.0 ) override;


protected:
    virtual void UpdateSurf() override;
};


#endif // !defined(VSPCLONEGEOM__INCLUDED_)
