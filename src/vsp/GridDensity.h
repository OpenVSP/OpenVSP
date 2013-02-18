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

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#endif
				

#include <stdio.h>
#include "vec2d.h"
#include "vec3d.h"
#include "bbox.h"
#include "stringc.h"
#include "xmlvsp.h"

#include <assert.h>
#include <math.h>

#include <vector>
#include <set>
#include <string>
using namespace std;

class Geom;

//////////////////////////////////////////////////////////////////////
class BaseSource
{
public:

	BaseSource();
	virtual ~BaseSource()	{};

	virtual void  SetGeomPtr( Geom* g )						{ m_GeomPtr = g; }
	virtual Geom* GetGeomPtr(  )							{ return m_GeomPtr; }

	virtual void SetName( const char* str )					{ m_Name = str; }
	virtual Stringc GetName()								{ return m_Name; }

	virtual double GetTargetLen( double base_len, vec3d &  pos ) = 0;

	virtual void   SetLen( double len )								{ m_Len = len; }
	virtual double GetLen()											{ return m_Len; }
	virtual void   AdjustLen( double val  );
	virtual void   SetRad( double rad )								{ m_Rad = rad; m_RadSquared = rad*rad; }
	virtual double GetRad()											{ return m_Rad; }
	virtual void   AdjustRad( double val  );

	virtual void SetNamedVal( Stringc name, double val )			{}

	virtual int GetType()											{ return m_Type; }

	virtual void CheckCorrectRad( double base_len );
	virtual void Draw()												{}
	virtual void SetBBox( bbox & b )								{ m_Box = b; }
	virtual bbox GetBBox()											{ return m_Box; }

	virtual void Update( Geom* geomPtr )							{}

	virtual void WriteParms( xmlNodePtr node ) = 0;
	virtual void ReadParms( xmlNodePtr node )  = 0;
	virtual void Copy( BaseSource* s ) = 0;

	virtual void SetReflSource( BaseSource* s )						{ m_ReflSource = s; }
	virtual BaseSource* GetReflSource()								{ return m_ReflSource; }

	virtual void DrawSphere( double rad, const vec3d& loc );


	enum{ POINT_SOURCE, LINE_SOURCE, BOX_SOURCE, NUM_SOURCE_TYPES, };

protected:

	Stringc m_Name;

	Geom* m_GeomPtr;

	int m_Type;

	double m_Len;
	double m_Rad;
	double m_RadSquared;
	bbox m_Box;

	BaseSource* m_ReflSource;
};

//////////////////////////////////////////////////////////////////////
class PointSource : public BaseSource
{
public:
	PointSource()			
		{ m_Type = POINT_SOURCE; 	m_Name = Stringc("Point_Name"); }
	PointSource( double len, double rad, vec3d & loc )			
		{ m_Len = len; m_Rad = rad; m_Loc = loc;  m_Type = POINT_SOURCE;  m_Name = Stringc("Point_Name");}
	virtual ~PointSource()		{}

	void SetLoc( const vec3d & loc )									{ m_Loc = loc; }
	void SetUWLoc( const vec2d & uw )									{ m_UWLoc = uw; }
	vec2d GetUWLoc()											{ return m_UWLoc; }	

	virtual void SetNamedVal( Stringc name, double val );
	double GetTargetLen( double base_len, vec3d &  pos );

	bool ReadData( char* buff );

	virtual void Update( Geom* geomPtr );

	virtual void WriteParms( xmlNodePtr node );				
	virtual void ReadParms( xmlNodePtr node );
	virtual void Copy( BaseSource* s );

	virtual void Draw();

protected:

	vec2d m_UWLoc;
	vec3d m_Loc;
};

//////////////////////////////////////////////////////////////////////
class LineSource : public BaseSource
{
public:
	LineSource();
	LineSource( double len1, double len2, double r1, double r2, vec3d & pnt1, vec3d & pnt2 );
	virtual ~LineSource()		{}

	virtual void SetRad( double r1 );
	virtual void SetRad2( double r2 );
	virtual void SetLen( double len1 );
	virtual void SetLen2( double len2 );
	virtual double GetRad()					{ return m_Rad1; }
	virtual double GetRad2()					{ return m_Rad2; }
	virtual double GetLen()					{ return m_Len1; }
	virtual double GetLen2()					{ return m_Len2; }

	virtual void   AdjustRad( double val  );
	virtual void   AdjustLen( double val  );

	void SetEndPnts( const vec3d & pnt1, const vec3d & pnt2 );
	void UpdateBBox();

	void SetNamedVal( Stringc name, double val );
	void SetUWLoc1( const vec2d & uw )								{ m_UWLoc1 = uw; }
	vec2d GetUWLoc1()											{ return m_UWLoc1; }	
	void SetUWLoc2( const vec2d & uw )								{ m_UWLoc2 = uw; }
	vec2d GetUWLoc2()											{ return m_UWLoc2; }	

	double GetTargetLen( double base_len, vec3d &  pos );

	//bool ReadData( char* buff );
	virtual void Update( Geom* geomPtr );

	virtual void WriteParms( xmlNodePtr node );				
	virtual void ReadParms( xmlNodePtr node );
	virtual void Copy( BaseSource* s );

	virtual void Draw();

protected:

	vec3d m_Pnt1;
	vec3d m_Pnt2;

	vec2d m_UWLoc1;
	vec2d m_UWLoc2;

	double m_Len1;
	double m_Len2;
	double m_Rad1;
	double m_Rad2;
	double m_RadSquared1;
	double m_RadSquared2;

	vec3d m_Line;						// m_Pnt2 - m_Pnt1
	double m_DotLine;					// dot( m_Line, m_Line )
};

//////////////////////////////////////////////////////////////////////
class BoxSource : public BaseSource
{
public:
	BoxSource()				
		{ m_Type = BOX_SOURCE;  m_Rad = 0; m_Name = Stringc("Box_Name"); }
	virtual ~BoxSource()		{};

	void SetMinMaxPnts( const vec3d & min_pnt, const vec3d & max_pnt );

	void ComputeCullPnts();
	double GetTargetLen( double base_len, vec3d &  pos );

	bool ReadData( char* buff );
	void Update( Geom* geomPtr );

	void SetRad( double rad );

	void SetNamedVal( Stringc name, double val );
	void SetUWLoc1( const vec2d & uw )								{ m_UWLoc1 = uw; }
	vec2d GetUWLoc1()											{ return m_UWLoc1; }	
	void SetUWLoc2( const vec2d & uw )								{ m_UWLoc2 = uw; }
	vec2d GetUWLoc2()											{ return m_UWLoc2; }	

	vec2d m_UWLoc1;
	vec2d m_UWLoc2;

	virtual void WriteParms( xmlNodePtr node );				
	virtual void ReadParms( xmlNodePtr node );
	virtual void Copy( BaseSource* s );

	virtual void Draw();

protected:

	vec3d m_MinPnt;
	vec3d m_MaxPnt;

	vec3d m_CullMinPnt;
	vec3d m_CullMaxPnt;
};

//////////////////////////////////////////////////////////////////////
class GridDensity
{
public:

	GridDensity();
	virtual ~GridDensity();

	//void ReadFile( const char* filename );

	double GetBaseLen()							{ return m_BaseLen; }
	void   SetBaseLen( double v )				{ m_BaseLen = v; }

	double GetMinLen()					{ return m_MinLen; }
	void   SetMinLen( double v )				{ m_MinLen = v; }

	double GetNCircSeg()					{ return m_NCircSeg; }
	void   SetNCircSeg( double v );

	double GetRadFrac()					{ return m_RadFrac; }

	double GetMaxGap()					{ return m_MaxGap; }
	void   SetMaxGap( double v )				{ m_MaxGap = v; }

	double GetGrowRatio()					{ return m_GrowRatio; }
	void   SetGrowRatio( double v )				{ m_GrowRatio = v; }

	bool GetRigorLimit()					{ return m_RigorLimit; }
	void SetRigorLimit( bool v )			{ m_RigorLimit = v; }

	double GetTargetLen( vec3d& pos );

	void ClearSources()							{ m_Sources.clear(); } //Deleted in Geom
	void RemoveSource( BaseSource* s );
	void AddSource( BaseSource* s )				{ m_Sources.push_back( s ); }
	int  GetNumSources()						{ return m_Sources.size(); }

	void ScaleAllSources( double scale );

	void Draw( BaseSource* curr_source );


protected:

	double m_BaseLen;

	double m_MinLen;
	double m_NCircSeg;
	double m_RadFrac;
	double m_MaxGap;
	double m_GrowRatio;

	bool m_RigorLimit;

	vector< BaseSource* > m_Sources;				// Sources + Ref Sources in 3D Space

};


#endif 
