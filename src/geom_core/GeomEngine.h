//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GeomEngine.h: Extension to GeomXSec for engine modeling (for stack and fuselage)
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPGEOMENGINE__INCLUDED_)
#define VSPGEOMENGINE__INCLUDED_

#include "Geom.h"

//==== GeomEngine  ====//
class GeomEngine : public GeomXSec
{
public:
    GeomEngine( Vehicle* vehicle_ptr );
    virtual ~GeomEngine();

    virtual void Update( bool fullupdate = true );

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

protected:

    virtual void ValidateParms();
    virtual void UpdateEngine();

    virtual void UpdateHighlightDrawObj();
};

#endif // !defined(VSPGEOMENGINE__INCLUDED_)
