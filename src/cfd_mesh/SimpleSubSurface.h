// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// SimpleSubSurface.h
// Justin Gravett
//////////////////////////////////////////////////////////////////////

#if !defined(SIMPLESUBSURFACE_SIMPLESUBSURFACE__INCLUDED_)
#define SIMPLESUBSURFACE_SIMPLESUBSURFACE__INCLUDED_

#include "SubSurface.h"

using namespace std;

class SimpleSubSurface
{
public:
    SimpleSubSurface();
    virtual ~SimpleSubSurface();

    virtual void CopyFrom( SubSurface* ss );

    virtual void SplitSegsU( const double & u ); // Split line segments that cross a constant U value
    virtual void SplitSegsW( const double & w ); // Split line segments that cross a constant W value
    virtual void ReorderSplitSegs( int ind );
    virtual bool Subtag( const vec3d & center );
    virtual void UpdatePolygonPnts( bool ss_con_both = true );

    virtual std::vector< SSLineSeg >& GetSplitSegs()
    {
        return m_SplitLVec;
    }
    virtual bool GetPolyFlag()
    {
        return m_PolyFlag;
    }
    virtual int GetFeaPropertyIndex()
    {
        return m_FeaPropertyIndex;
    }
    virtual int GetCapFeaPropertyIndex()
    {
        return m_CapFeaPropertyIndex;
    }
    virtual string GetName()
    {
        return m_Name;
    }

    int m_Tag;
    int m_TestType;
    int m_MainSurfIndx;
    bool m_IntersectionCapFlag;
    int m_ControlSurfType;
    bool m_RemoveSubSurfTrisFlag;

protected:

    string m_SSID;
    string m_CompID; // Component ID used to match SimpleSubSurface to a specific geom
    string m_Name;
    int m_Type; // Type of SimpleSubSurface

    vector< SSLineSeg > m_LVec; // Line Segment Vector
    vector< SSLineSeg > m_SplitLVec; // Split Line Vector

    int m_FeaPropertyIndex;
    int m_CapFeaPropertyIndex;

    std::vector< std::vector< vec2d > > m_PolyPntsVec;
    bool m_PolyPntsReadyFlag;
    bool m_FirstSplit;
    bool m_PolyFlag; // Flag to indicate if the SimpleSubSurface is a Polygon ( this affects how it is treated in CFDMesh )

};

#endif