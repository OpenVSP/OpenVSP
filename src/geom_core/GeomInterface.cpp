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

Matrix4d GeomInterface::GetRoleShapeMatrix() const
{
    const Geom* geom = dynamic_cast< const Geom* >( this );
    if ( geom )
    {
        return geom->GetShapeMatrix();
    }

    return Matrix4d();
}

bool GeomInterface::GetRoleShapeFlipNormal() const
{
    const Geom* geom = dynamic_cast< const Geom* >( this );
    if ( geom )
    {
        return geom->GetFlipReversesNormal();
    }

    return false;
}

Matrix4d GeomInterface::GetRoleFlipMat() const
{
    const Geom* geom = dynamic_cast< const Geom* >( this );
    if ( geom )
    {
        return geom->GetFlipMat();
    }

    return Matrix4d();
}
