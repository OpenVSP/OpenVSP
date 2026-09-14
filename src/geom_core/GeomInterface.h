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

    // The Geom's model matrix (a Clone's own).
    Matrix4d GetRoleModelMatrix() const;
};

#endif // !defined(VSPGEOMINTERFACE__INCLUDED_)
