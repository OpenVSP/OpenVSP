//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Parameter Link Mgr Class 
// 
//   J.R. Gloudemans - 10/19/09
//
//    
//******************************************************************************
#ifndef PARM_LINK_PARM_LINK_H
#define PARM_LINK_PARM_LINK_H

#include <stdio.h>

#ifdef WIN32
#include <windows.h>		
#endif

#include "geom.h"
#include "xmlvsp.h"
#include "stringc.h"
#include <vector>	
#include <map>
#include <list>

using namespace std;	

class Geom;
class GeomBase;
class Parm;
class Aircraft;
class ParmButton;

class ParmLink
{
public:

	ParmLink();
	virtual ~ParmLink();

	virtual void SetParmA( Parm* p );
	virtual void SetParmB( Parm* p );
	virtual Parm* GetParmA()					{ return m_ParmA; }
	virtual Parm* GetParmB()					{ return m_ParmB; }
	virtual void InitOffsetScale();

	virtual void SetOffsetFlag( bool f )		{ m_OffsetFlag = f; }
	virtual bool GetOffsetFlag()				{ return m_OffsetFlag; }
	virtual void SetOffset( double v )			{ m_Offset = v; }
	virtual double GetOffset()					{ return m_Offset; }

	virtual void SetScaleFlag( bool f )			{ m_ScaleFlag = f; }
	virtual bool GetScaleFlag()					{ return m_ScaleFlag; }
	virtual void SetScale( double v )			{ m_Scale = v; }
	virtual double GetScale()					{ return m_Scale; }

	virtual void SetLowerLimitFlag( bool f )	{ m_LowerLimitFlag = f; }
	virtual bool GetLowerLimitFlag()			{ return m_LowerLimitFlag; }
	virtual void SetLowerLimit( double v )		{ m_LowerLimit = v; }
	virtual double GetLowerLimit()				{ return m_LowerLimit; }

	virtual void SetUpperLimitFlag( bool f )	{ m_UpperLimitFlag = f; }
	virtual bool GetUpperLimitFlag()			{ return m_UpperLimitFlag; }
	virtual void SetUpperLimit( double v )		{ m_UpperLimit = v; }
	virtual double GetUpperLimit()				{ return m_UpperLimit; }

protected:

	Parm* m_ParmA;
	Parm* m_ParmB;

	bool m_OffsetFlag;
	double m_Offset;

	bool m_ScaleFlag;
	double m_Scale;

	bool m_LowerLimitFlag;
	double m_LowerLimit;

	bool m_UpperLimitFlag;
	double m_UpperLimit;

};

class UserGeom : public Geom
{
public:
	UserGeom(Aircraft* aptr);
	virtual ~UserGeom();

	virtual void write(xmlNodePtr node);
	virtual void read(xmlNodePtr node);

	virtual void copy( Geom* fromGeom );
	virtual void parm_changed(Parm*);

	virtual void acceptScaleFactor()								{}
	virtual void resetScaleFactor()									{}
	virtual void update_bbox()										{}
	virtual vec3d getVertex3d(int surf, double x, double p, int r)	{ return vec3d(); }
	virtual void  getVertexVec(vector< VertexID > *vertVec)			{}


	virtual int getNumXSecSurfs()									{ return 0; }

	virtual void LoadLinkableParms( vector< Parm* > & parmVec );

	virtual void SetUserParmValue( int id, double val );


	Parm userParm1;
	Parm userParm2;
	Parm userParm3;
	Parm userParm4;
	Parm userParm5;
	Parm userParm6;
	Parm userParm7;
	Parm userParm8;

};


class ParmLinkMgr
{
public:

	ParmLinkMgr();
	virtual ~ParmLinkMgr();

	virtual void WriteLinks(xmlNodePtr node);
	virtual void ReadLinks(xmlNodePtr node, vector< Geom* > & gVec );

	virtual void SetAircraftPtr( Aircraft* aptr )			{ aircraftPtr = aptr; }

	virtual void RemoveAllReferencesLink( Geom* geomPtr );
	virtual void RemoveParmReferencesLink( Parm* parmPtr );

	virtual void RebuildAllLink();

	virtual void SetCurrParmLinkIndex( int i );
	virtual int  GetCurrParmLinkIndex()						{ return m_CurrParmLinkIndex; }
	virtual ParmLink* GetCurrParmLink();
	virtual vector< ParmLink* > GetParmLinkVec();

	virtual void RebuildParmLinkMap();

	virtual bool AddCurrLink();
	virtual void DelCurrLink();
	virtual void DelAllLinks();
	virtual bool LinkAllComp();
	virtual bool LinkAllGroup();
	virtual bool CheckForDuplicateLink( Parm* pA, Parm* pB );

	virtual void AddLink( Parm* pA, Parm* pB );

	virtual void ParmChanged( Parm* parmPtr, bool start_flag );

	virtual ParmLink* ResetWorkingParmLink();

	virtual void SetParmA( Parm* p );
	virtual void SetParmB( Parm* p );
	virtual void SetParm( bool flagA, int comp, int group, int parm );

	virtual void SwapGeom( Geom* gOld, Geom* gNew );

	virtual Parm* GetDefaultParm()					{ return m_DefaultParm; }
	virtual void SetDefaultParm( Parm* p )			{ m_DefaultParm = p; }

protected:

	Aircraft* aircraftPtr;

	int m_CurrParmLinkIndex;
	vector< ParmLink* > m_ParmLinkVec;
	map< Parm*, vector< ParmLink* > > m_ParmLinkMap;

	ParmLink m_WorkingParmLink;

	Parm* m_DefaultParm;

	vector< Parm* > m_UpdatedParmVec;			// Prevent Circular Links

};


class PLM_Single
{
public:
	PLM_Single();
	ParmLinkMgr* parmLinkMgr;
};


static PLM_Single singlePLM;

#define parmLinkMgrPtr (singlePLM.parmLinkMgr)



class ParmMgr
{
public:

	ParmMgr();
	virtual ~ParmMgr();

	virtual void SetAircraftPtr( Aircraft* aptr )			{ aircraftPtr = aptr; }
	virtual void RebuildAll();

	virtual void LoadAllParms();
	virtual void Register( Parm* parmPtr, GeomBase* geomPtr, string groupName );
	virtual void RegisterParmButton( ParmButton* b );

	virtual void RemoveAllReferences( Geom* geomPtr );
	virtual void RemoveParmReferences( Parm* parmPtr );

	virtual vector< string > GetGroupNameVec( GeomBase* geomPtr );
	virtual vector< Parm* > GetParmVec( GeomBase* geomPtr, string group_name );
	virtual string GetGroupName( GeomBase* geomPtr, int name_index );

	virtual int GetCurrGeomNameVec( Parm* parmPtr, vector< string > & nameVec );
	virtual int GetCurrGroupNameVec( Parm* parmPtr, vector< string > & nameVec );
	virtual int GetCurrParmNameVec( Parm* parmPtr, vector< string > & nameVec );

	virtual Parm* FindParm( vector< Geom* > & gVec, int ptrID,
							Stringc& group_name, Stringc& parm_name );
	virtual Parm* FindParm( Geom* gPtr, Stringc& group_name, Stringc& parm_name );


protected:

	Aircraft* aircraftPtr;

	map< GeomBase*, map< string, vector< Parm* > > > m_ParmMap;

};

class PM_Single
{
public:
	PM_Single();
	ParmMgr* parmMgr;
};


static PM_Single singlePM;

#define parmMgrPtr (singlePM.parmMgr)



class PHolderListMgr
{
public:

	PHolderListMgr();
	virtual ~PHolderListMgr();

	virtual void WritePHolderListDES( char *newfile );
	virtual void ReadPHolderListDES( char *newfile );

	virtual void WritePHolderListXDDM( char *newfile );
	virtual void ReadPHolderListXDDM( char *newfile );

	virtual void SetAircraftPtr( Aircraft* aptr )			{ aircraftPtr = aptr; }

	virtual void RemoveAllReferencesPHolderList( Geom* geomPtr );
	virtual void RemoveParmReferencesPHolderList( Parm* parmPtr );

	virtual void RebuildAllPHolderList();

	virtual void SetCurrPHolderIndex( int i );
	virtual int  GetCurrPHolderIndex()						{ return m_CurrPHolderIndex; }
	virtual ParmHolder* GetCurrPHolder();
	virtual vector< ParmHolder* > GetPHolderVec();

	virtual void RebuildPHolderMap();

	virtual bool AddCurrPHolder();
	virtual void DelCurrPHolder();
	virtual void DelAllPHolders();

	virtual void AddPHolder( Parm* p );

	virtual ParmHolder* ResetWorkingPHolder();

	virtual void SetParm( Parm* p );
	virtual void SetParm( int comp, int group, int parm );

	virtual Parm* GetDefaultParm()					{ return m_DefaultParm; }
	virtual void SetDefaultParm( Parm* p )			{ m_DefaultParm = p; }

protected:

	Aircraft* aircraftPtr;

	int m_CurrPHolderIndex;
	vector< ParmHolder* > m_PHolderVec;
	map< Parm*, vector< ParmHolder* > > m_PHolderMap;

	ParmHolder m_WorkingPHolder;

	Parm* m_DefaultParm;

};

class PHL_Single
{
public:
	PHL_Single();
	PHolderListMgr* pHolderListMgr;
};


static PHL_Single singlePHL;

#define pHolderListMgrPtr (singlePHL.pHolderListMgr)



#endif  //PARM_LINK_PARM_LINK_H
