//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PodGeom.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPPODGEOM__INCLUDED_)
#define VSPPODGEOM__INCLUDED_

#include "Geom.h"
#include "XSec.h"


//==== Pod Geom ====//
class PodGeom : public Geom
{
public:
    PodGeom( Vehicle* vehicle_ptr );
    virtual ~PodGeom();

    virtual int GetNumMainSurfs()
    {
        return 1;
    }

    virtual void ComputeCenter();
    virtual void Scale();

    Parm m_Length;                  // Length of Pod
    Parm m_FineRatio;               // Fineness Ratio of Pod

protected:
    virtual void UpdateSurf();
};


#endif // !defined(VSPPODGEOM__INCLUDED_)
