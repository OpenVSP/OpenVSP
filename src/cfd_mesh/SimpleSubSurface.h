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

    void CopyFrom( SubSurface *ss, const vector < string > & );

    void SplitSegsU( const double & u ); // Split line segments that cross a constant U value
    void SplitSegsW( const double & w ); // Split line segments that cross a constant W value
    void SplitSegsU( const double & u, vector<SSLineSeg> &splitvec ); // Split line segments that cross a constant U value
    void SplitSegsW( const double & w, vector<SSLineSeg> &splitvec ); // Split line segments that cross a constant W value
    static void ReorderSplitSegs( int ind, vector<SSLineSeg> &splitvec );
    bool Subtag( const vec3d & center );

    std::vector< std::vector< SSLineSeg > >& GetSplitSegs()
    {
        return m_SplitLVec;
    }
    bool GetPolyFlag()
    {
        return m_PolyFlag;
    }
    int GetFeaPropertyIndex()
    {
        return m_FeaPropertyIndex;
    }
    int GetCapFeaPropertyIndex()
    {
        return m_CapFeaPropertyIndex;
    }
    string GetFeaPropertyID()
    {
        return m_FeaPropertyID;
    }
    string GetCapFeaPropertyID()
    {
        return m_CapFeaPropertyID;
    }
    int GetFeaOrientationType()
    {
        return m_FeaOrientationType;
    }
    std::vector < vec3d > GetFeaOrientationVec()
    {
        return m_FeaOrientationVec;
    }
    string GetName()
    {
        return m_Name;
    }
    string GetCompID()
    {
        return m_CompID;
    }
    string GetSSID()
    {
        return m_SSID;
    }
    int GetType()
    {
        return m_Type;
    }

    int m_Tag;
    int m_TestType;
    int m_MainSurfIndx;
    bool m_CreateBeamElements;
    int m_KeepDelShellElements;

protected:

    string m_SSID;
    string m_CompID; // Component ID used to match SimpleSubSurface to a specific geom
    string m_Name;
    int m_Type; // Type of SimpleSubSurface

    vector< SSLineSeg > m_LVec; // Line Segment Vector
    vector< vector<SSLineSeg> > m_SplitLVec; // Split Line Vector

    int m_FeaPropertyIndex;
    int m_CapFeaPropertyIndex;
    string m_FeaPropertyID;
    string m_CapFeaPropertyID;

    int m_FeaOrientationType;
    std::vector < vec3d > m_FeaOrientationVec;

    std::vector< std::vector< vec2d > > m_PolyPntsVec;
    bool m_PolyPntsReadyFlag;
    bool m_FirstSplit;
    bool m_PolyFlag; // Flag to indicate if the SimpleSubSurface is a Polygon ( this affects how it is treated in CFDMesh )
};

#endif