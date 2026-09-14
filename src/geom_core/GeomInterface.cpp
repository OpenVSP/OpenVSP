//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GeomInterface.h"
#include "Geom.h"

// Cross cast from the interface to the Geom that implements it.
Matrix4d GeomInterface::GetRoleModelMatrix() const
{
    const GeomXForm* geom = dynamic_cast< const GeomXForm* >( this );
    if ( geom )
    {
        return geom->getModelMatrix();
    }

    return Matrix4d();
}
