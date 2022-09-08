//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MeshAnalysis.h
// Gary Gray, ESAero, 10/24/20
//
//////////////////////////////////////////////////////////////////////

#if !defined(MESHANALYSIS__INCLUDED_)
#define MESHANALYSIS__INCLUDED_

#include "AnalysisMgr.h"

using std::string;

class CfdMeshAnalysis : public Analysis
{
public:

    CfdMeshAnalysis();

    virtual void SetDefaults();
    virtual string Execute();
};

class FeaMeshAnalysis : public Analysis
{
public:

    FeaMeshAnalysis();

    virtual void SetDefaults();
    virtual string Execute();
};

class SurfaceIntersectionAnalysis : public Analysis
{
public:

    SurfaceIntersectionAnalysis();

    virtual void SetDefaults();
    virtual string Execute();
};

#endif // !defined(MESHANALYSIS__INCLUDED_)