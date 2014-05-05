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

#include <stdio.h>
#include "Vec2d.h"
#include "Vec3d.h"
#include "BndBox.h"
#include "XmlUtil.h"
#include "Parm.h"
#include "ParmContainer.h"
#include "DrawObj.h"

#include <assert.h>
#include <math.h>

#include <vector>
#include <set>
#include <string>
using namespace std;

class Geom;

//////////////////////////////////////////////////////////////////////
class BaseSource : public ParmContainer
{
public:

    BaseSource();
    virtual ~BaseSource()   {};

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual void SetName( const string str )
    {
        m_Name = str;
    }
    virtual string GetName()
    {
        return m_Name;
    }

    virtual double GetTargetLen( double base_len, vec3d &  pos ) = 0;

    virtual void   AdjustLen( double val  );
    virtual void   AdjustRad( double val  );

    virtual void SetNamedVal( string name, double val )         {}

    virtual int GetType()
    {
        return m_Type;
    }

    virtual void CheckCorrectRad( double base_len );
    virtual void Draw()                                             {}
    virtual void SetBBox( BndBox & b )
    {
        m_Box = b;
    }
    virtual BndBox GetBBox()
    {
        return m_Box;
    }

    virtual void Update( Geom* geomPtr )                            {}

    virtual void Copy( BaseSource* s ) = 0;

//  virtual void SetReflSource( BaseSource* s )                     { m_ReflSource = s; }
//  virtual BaseSource* GetReflSource()                             { return m_ReflSource; }

//  virtual void DrawSphere( double rad, const vec3d& loc );
    virtual vector< vec3d > CreateSphere( double rad, const vec3d& loc );

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec ) = 0;
    virtual void Show( bool flag ) = 0;
    virtual void Highlight( bool flag ) = 0;

    xmlNodePtr EncodeXml(  xmlNodePtr & node  );

    enum { POINT_SOURCE, LINE_SOURCE, BOX_SOURCE, NUM_SOURCE_TYPES, };

    Parm m_Len;
    Parm m_Rad;


protected:

    string m_Name;

    int m_Type;

    BndBox m_Box;

    string m_GroupName;

//  BaseSource* m_ReflSource;
};

//////////////////////////////////////////////////////////////////////
class PointSource : public BaseSource
{
public:
    PointSource();
    virtual ~PointSource()      {}

    void SetLoc( const vec3d & loc )
    {
        m_Loc = loc;
    }

    virtual void SetNamedVal( string name, double val );
    double GetTargetLen( double base_len, vec3d &  pos );

    bool ReadData( char* buff );

    virtual void Update( Geom* geomPtr );

    virtual void Copy( BaseSource* s );

//  virtual void Draw();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    virtual void Show( bool flag );
    virtual void Highlight( bool flag );

    Parm m_ULoc;
    Parm m_WLoc;

protected:

    vec3d m_Loc;

    DrawObj m_PointDO;
};

//////////////////////////////////////////////////////////////////////
class LineSource : public BaseSource
{
public:
    LineSource();
    virtual ~LineSource()       {}

    virtual void   AdjustRad( double val  );
    virtual void   AdjustLen( double val  );

    void SetEndPnts( const vec3d & pnt1, const vec3d & pnt2 );
    void UpdateBBox();

    void SetNamedVal( string name, double val );

    double GetTargetLen( double base_len, vec3d &  pos );

    //bool ReadData( char* buff );
    virtual void Update( Geom* geomPtr );

    virtual void Copy( BaseSource* s );

//  virtual void Draw();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    virtual void Show( bool flag );
    virtual void Highlight( bool flag );

    Parm m_ULoc1;
    Parm m_WLoc1;

    Parm m_ULoc2;
    Parm m_WLoc2;

    Parm m_Len2;
    Parm m_Rad2;

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
class BoxSource : public BaseSource
{
public:
    BoxSource();
    virtual ~BoxSource()        {};

    void SetMinMaxPnts( const vec3d & min_pnt, const vec3d & max_pnt );

    void ComputeCullPnts();
    double GetTargetLen( double base_len, vec3d &  pos );

    bool ReadData( char* buff );
    void Update( Geom* geomPtr );

    void SetRad( double rad );

    void SetNamedVal( string name, double val );

    Parm m_ULoc1;
    Parm m_WLoc1;

    Parm m_ULoc2;
    Parm m_WLoc2;

    virtual void Copy( BaseSource* s );

//  virtual void Draw();
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
class GridDensity : public ParmContainer
{
public:

    GridDensity();
    virtual ~GridDensity();

    virtual void ParmChanged( Parm* parm_ptr, int type );

    //void ReadFile( const char* filename );

    bool GetRigorLimit()
    {
        return m_RigorLimit;
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

    double GetTargetLen( vec3d& pos, bool farFlag = false );

    void ClearSources()
    {
        m_Sources.clear();    //Deleted in Geom
    }
    void RemoveSource( BaseSource* s );
    void AddSource( BaseSource* s )
    {
        m_Sources.push_back( s );
    }
    int  GetNumSources()
    {
        return m_Sources.size();
    }

    void ScaleAllSources( double scale );

    void Draw( BaseSource* curr_source );

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    void Show( bool flag );
    void Highlight( BaseSource * source );

    Parm m_BaseLen;
    Parm m_FarMaxLen;
    Parm m_MinLen;
    Parm m_NCircSeg;
    Parm m_FarNCircSeg;
    Parm m_MaxGap;
    Parm m_FarMaxGap;
    Parm m_GrowRatio;

protected:

    bool m_RigorLimit;

    vector< BaseSource* > m_Sources;                // Sources + Ref Sources in 3D Space

};


#endif
