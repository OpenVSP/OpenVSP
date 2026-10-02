//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GeomInterface.h:
// Role interfaces a Geom can offer.  Callers cast to a role rather than a concrete class, so a
// Clone can offer the same roles as the Geom it copies.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPGEOMINTERFACE__INCLUDED_)
#define VSPGEOMINTERFACE__INCLUDED_

#include "Geom.h"
#include "Matrix4d.h"
#include "Vec3d.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

class Bogie;
class TMesh;
class GeomXForm;
class Parm;

//==== Common to every interface a Geom can offer ====//
// Placement of the Geom that implements the interface.
class GeomInterface
{
public:
    virtual ~GeomInterface()   {}

    // The Geom's model matrix (a Clone's own).  Excludes the flip, since other Geoms attach
    // to this frame and must not be reflected.
    Matrix4d GetRoleModelMatrix() const;

    // The model matrix with the flip applied.  Use it to take shape-frame results to world.
    Matrix4d GetRoleShapeMatrix() const;

    // Whether the flip reverses the shape (an odd number of planes).  Separate from the matrix
    // because some writers that need it never see one.
    bool GetRoleShapeFlipNormal() const;

    // The flip alone, in the Geom's own frame.
    Matrix4d GetRoleFlipMat() const;
};

#endif // !defined(VSPGEOMINTERFACE__INCLUDED_)
