//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// GridDensity.h
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(GRIDDENSITY_GRIDDENSITY__INCLUDED_)
#define GRIDDENSITY_GRIDDENSITY__INCLUDED_

#ifdef WIN32
#include <windows.h>
#endif

#include "Defines.h"
#include <stdio.h>
#include "Vec2d.h"
#include "Vec3d.h"
#include "BndBox.h"
#include "XmlUtil.h"
#include "Parm.h"
#include "ParmContainer.h"
#include "DrawObj.h"

#include <assert.h>
#include <cmath>

#include <vector>
#include <set>
#include <string>
using namespace std;

class Geom;

class SourceData
{
public:

    int m_Type;
    int m_SurfIndex;
    double m_Len1;
    double m_Rad1;
    double m_Len2;
    double m_Rad2;
    double m_U1;
    double m_W1;
    double m_U2;
    double m_W2;
};


//////////////////////////////////////////////////////////////////////
class BaseSource : public ParmContainer
{
public:

    BaseSource();
    virtual ~BaseSource()   {};

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual void AdjustLen( double val );
    virtual void AdjustRad( double val );

    virtual void SetNamedVal( string name, double val )         {}

    virtual int GetType()
    {
        return m_Type;
    }

    xmlNodePtr EncodeXml( xmlNodePtr & node );

    virtual void ReadV2File( xmlNodePtr &root ) = 0;

    Parm m_Len;
    Parm m_Rad;

    IntParm m_MainSurfIndx;

protected:

    int m_Type;

    string m_GroupName;
};

//////////////////////////////////////////////////////////////////////
class PointSource : public BaseSource
{
public:
    PointSource();
    virtual ~PointSource()      {}

    virtual void SetNamedVal( string name, double val );

    virtual void ReadV2File( xmlNodePtr &root );

    Parm m_ULoc;
    Parm m_WLoc;
};

//////////////////////////////////////////////////////////////////////
class LineSource : public BaseSource
{
public:
    LineSource();
    virtual ~LineSource()       {}

    virtual void AdjustRad( double val );
    virtual void AdjustLen( double val );

    void SetNamedVal( string name, double val );

    virtual void ReadV2File( xmlNodePtr &root );

    Parm m_ULoc1;
    Parm m_WLoc1;

    Parm m_ULoc2;
    Parm m_WLoc2;

    Parm m_Len2;
    Parm m_Rad2;
};

//////////////////////////////////////////////////////////////////////
class BoxSource : public BaseSource
{
public:
    BoxSource();
    virtual ~BoxSource()        {};

    void SetNamedVal( string name, double val );

    virtual void ReadV2File( xmlNodePtr &root );

    Parm m_ULoc1;
    Parm m_WLoc1;

    Parm m_ULoc2;
    Parm m_WLoc2;
};

//////////////////////////////////////////////////////////////////////
class ConstLineSource : public BaseSource
{
public:
    ConstLineSource();
    virtual ~ConstLineSource()       {}

    void SetNamedVal( string name, double val );

    virtual void ReadV2File( xmlNodePtr &root )  {};

    Parm m_Val;
};

class ULineSource : public ConstLineSource
{
public:
    ULineSource();
    virtual ~ULineSource()       {}
};

class WLineSource : public ConstLineSource
{
public:
    WLineSource();
    virtual ~WLineSource()       {}
};

//////////////////////////////////////////////////////////////////////

class BaseSimpleSource
{
public:

    BaseSimpleSource();
    virtual ~BaseSimpleSource()   {};

    virtual void AdjustLen( double val );

    virtual double GetTargetLen( double base_len, vec3d &  pos, const string & geomid, const int & surfindx, const double & u, const double &w ) = 0;

    virtual void Draw()                                             {}

    virtual void Update( Geom* geomPtr )                            {}

    virtual void CopyFrom( BaseSource* s ) = 0;

    virtual vector< vec3d > CreateSphere( double rad, const vec3d& loc );

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec ) = 0;
    virtual void Show( bool flag ) = 0;
    virtual void Highlight( bool flag ) = 0;

    double m_Len;
    double m_Rad;

    int m_SurfIndx;
    string m_OrigSourceID;

protected:

    BndBox m_Box;

    string m_DrawObjID;
};

//////////////////////////////////////////////////////////////////////
class PointSimpleSource : public BaseSimpleSource
{
public:
    PointSimpleSource();
    virtual ~PointSimpleSource()      {}

    virtual double GetTargetLen( double base_len, vec3d &  pos, const string & geomid, const int & surfindx, const double & u, const double &w );

    virtual void Update( Geom* geomPtr );

    virtual void CopyFrom( BaseSource* s );

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    virtual void Show( bool flag );
    virtual void Highlight( bool flag );

    double m_ULoc;
    double m_WLoc;

protected:

    vec3d m_Loc;

    DrawObj m_PointDO;
};

//////////////////////////////////////////////////////////////////////
class LineSimpleSource : public BaseSimpleSource
{
public:
    LineSimpleSource();
    virtual ~LineSimpleSource()       {}

    void SetEndPnts( const vec3d & pnt1, const vec3d & pnt2 );
    void UpdateBBox();

    virtual void AdjustLen( double val );

    virtual double GetTargetLen( double base_len, vec3d &  pos, const string & geomid, const int & surfindx, const double & u, const double &w );

    virtual void Update( Geom* geomPtr );

    virtual void CopyFrom( BaseSource* s );

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    virtual void Show( bool flag );
    virtual void Highlight( bool flag );

    double m_ULoc1;
    double m_WLoc1;

    double m_ULoc2;
    double m_WLoc2;

    double m_Len2;
    double m_Rad2;

protected:

    vec3d m_Pnt1;
    vec3d m_Pnt2;

    double m_RadSquared1;
    double m_RadSquared2;

    vec3d m_Line;                       // m_Pnt2 - m_Pnt1
    double m_DotLine;                   // dot( m_Line, m_Line )

    DrawObj m_LineDO1;
    DrawObj m_LineDO2;
    DrawObj m_LineDO3;
};

//////////////////////////////////////////////////////////////////////
class BoxSimpleSource : public BaseSimpleSource
{
public:
    BoxSimpleSource();
    virtual ~BoxSimpleSource()        {};

    void SetMinMaxPnts( const vec3d & min_pnt, const vec3d & max_pnt );

    void ComputeCullPnts();

    virtual double GetTargetLen( double base_len, vec3d &  pos, const string & geomid, const int & surfindx, const double & u, const double &w );

    void Update( Geom* geomPtr );

    double m_ULoc1;
    double m_WLoc1;

    double m_ULoc2;
    double m_WLoc2;

    virtual void CopyFrom( BaseSource* s );

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    virtual void Show( bool flag );
    virtual void Highlight( bool flag );

protected:

    vec3d m_MinPnt;
    vec3d m_MaxPnt;

    vec3d m_CullMinPnt;
    vec3d m_CullMaxPnt;

    DrawObj m_BoxDO1;
    DrawObj m_BoxDO2;
    DrawObj m_BoxDO3;
};

//////////////////////////////////////////////////////////////////////

class ConstLineSimpleSource : public BaseSimpleSource
{
public:
    ConstLineSimpleSource();
    virtual ~ConstLineSimpleSource()       {}

    void UpdateBBox();

    virtual void AdjustLen( double val );

    virtual double GetTargetLen( double base_len, vec3d &  pos, const string & geomid, const int & surfindx, const double & u, const double &w );

    virtual void CopyFrom( BaseSource* s );

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    virtual void Show( bool flag );
    virtual void Highlight( bool flag );

    double m_Val;

protected:

    Geom * m_GeomPtr;

    vector < vec3d > m_Pts;
    vector < vec3d > m_UWPts;

    DrawObj m_SpheresDO;
    DrawObj m_LinesDO;
};

//////////////////////////////////////////////////////////////////////

class ULineSimpleSource : public ConstLineSimpleSource
{
public:
    virtual void Update( Geom* geomPtr );
};

//////////////////////////////////////////////////////////////////////

class WLineSimpleSource : public ConstLineSimpleSource
{
public:
    virtual void Update( Geom* geomPtr );
};

//////////////////////////////////////////////////////////////////////
class GridDensity : public ParmContainer
{
public:

    GridDensity();
    virtual ~GridDensity();

    virtual void InitParms();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void ReadV2File( xmlNodePtr &root );

    virtual void ParmChanged( Parm* parm_ptr, int type );

    bool GetRigorLimit()
    {
        return m_RigorLimit.Get();
    }
    void SetRigorLimit( bool v )
    {
        m_RigorLimit = v;
    }

    double GetBaseLen( bool farflag = false )
    {
        if ( !farflag )
        {
            return m_BaseLen();
        }
        else
        {
            return m_FarMaxLen();
        }
    }

    double GetMaxGap( bool farflag = false )
    {
        if ( !farflag )
        {
            return m_MaxGap();
        }
        else
        {
            return m_FarMaxGap();
        }
    }

    double GetRadFrac( bool farflag = false );

    double GetFarRadFrac();

    double GetTargetLen( vec3d& pos, bool farFlag = false, const string & geomid = string(), const int & surfindx = 0, const double & u = 0.0, const double &w = 0.0 );

    void ClearSources()
    {
        m_Sources.clear();    //Deleted in Geom
    }
    void AddSource( BaseSimpleSource* s )
    {
        m_Sources.push_back( s );
    }
    int  GetNumSources()
    {
        return m_Sources.size();
    }

    vector< BaseSimpleSource* > GetSimpleSourceVec()
    {
        return m_Sources;
    }

    void ScaleAllSources( double scale );

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    void Show( bool flag );
    void Highlight( BaseSource * source );

    BoolParm m_RigorLimit;
    Parm m_BaseLen;
    Parm m_FarMaxLen;
    Parm m_MinLen;
    Parm m_NCircSeg;
    Parm m_FarNCircSeg;
    Parm m_MaxGap;
    Parm m_FarMaxGap;
    Parm m_GrowRatio;

protected:

    string m_GroupName;
    vector< BaseSimpleSource* > m_Sources;                // Sources + Ref Sources in 3D Space

};

class CfdGridDensity : public GridDensity
{
public:
    CfdGridDensity();
};

class FeaGridDensity : public GridDensity
{
public:
    FeaGridDensity();
};

#endif
