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

    virtual void ComputeCenter();
    virtual void Scale();
    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual void ReadV2File( xmlNodePtr &root );

    virtual void OffsetXSecs( double off );

    Parm m_Length;                  // Length of Pod
    Parm m_FineRatio;               // Fineness Ratio of Pod

protected:
    virtual void UpdateSurf();

    double m_Xoff;
};


#endif // !defined(VSPPODGEOM__INCLUDED_)
