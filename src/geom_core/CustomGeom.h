//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CustomGeom.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

/*
Three ways to create custom components:
1. Read from file with script attached
2. Create from GUI with script (preparsed) stored in local dir
3. Create from API with location of script

Script:

Init()
- Name & type & script_name
- Create/init parms
- Describe GUI
- Set num cuv_vec

UpdateSurf()
- load crv_vecs based on parms
- Skin surfs over cuv_vecs

CustomGeomMgr - store parsed scripts

*/



#if !defined(VSPCUSTOMGEOM__INCLUDED_)
#define VSPCUSTOMGEOM__INCLUDED_

#include "Geom.h"
#include "XSec.h"
#include "XSecSurf.h"

#include <map>
using std::map;


//====Custom Geom Manager ====//
class CustomGeomMgrSingleton
{
public:
    static CustomGeomMgrSingleton& getInstance()
    {
        static CustomGeomMgrSingleton instance;
        return instance;
    }
    void Init()                                         {}

    //==== Set/Get Script Dir ====//
    void SetScriptDir( const string & dir )                 { m_ScriptDir = dir; }
    string GetScriptDir()                                   { return m_ScriptDir; }

    //==== Init Geom ====//
    void InitGeom( const string& id, const string& module_name );
    void SetCurrCustomGeom( const string& id )            { m_CurrGeom = id; }
    string GetCurrCustomGeom()                            { return m_CurrGeom; }

    void ReadCustomScripts();
    vector< GeomType > GetCustomTypes()                    { return m_CustomTypeVec; }

    //==== Adds A Parm To The Current Custom Geom - Return Index of The Added Parm ====//
    string AddParm( int type, const string & name, const string & group );
    string GetCustomParm( int index );

    //==== Add A Gui Device Constructor to Current Custom Geom - Gui Will Be Created By Custom Screen (if connected) ===//
    int AddGui( int type, const string & label, const string & parm_name, const string & group_name, double range );
    vector< GuiDef > GetGuiDefVec( const string & geom_id );

    //==== Add Gui->Parm Pairing To Update Vec ====//
    void AddUpdateGui( int gui_id, const string & parm_id );
    vector< GuiUpdate > GetGuiUpdateVec();
    bool CheckClearTriggerEvent( int gui_id );

    //==== Add XSec Surface To Current Geom - Return ID =====//
    string AddXSecSurf();
    void RemoveXSecSurf( const string& id );
    void ClearXSecSurfs();
    void SkinXSecSurf( bool closed_flag );
    void TransformSurf( int index, Matrix4d & mat );
    void CloneSurf( int index, Matrix4d & mat );

    //==== Set Up Default Sources =====//
    void SetupCustomDefaultSource( int type, int surf_index, double l1, double r1, double u1, double w1,
                                   double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 );
    void ClearAllCustomDefaultSources();

    //==== Custom XSecs Functions ====//
    void SetCustomXSecLoc( const string & xsec_id, const vec3d & loc );
    vec3d GetCustomXSecLoc( const string & xsec_id );
    void SetCustomXSecRot( const string & xsec_id, const vec3d & rot );
    vec3d GetCustomXSecRot( const string & xsec_id );

    //==== Get All Custom Script Module Name ====//
    vector< string > GetCustomScriptModuleNames();

    //==== Save Custom Script Content To File ====//
    int SaveScriptContentToFile( const string & module_name, const string & file_name );


private:

    CustomGeomMgrSingleton();
    CustomGeomMgrSingleton( CustomGeomMgrSingleton const& copy );          // Not Implemented
    CustomGeomMgrSingleton& operator=( CustomGeomMgrSingleton const& copy ); // Not Implemented

    string m_CurrGeom;
    string m_ScriptDir;
    vector< GeomType > m_CustomTypeVec;
    map< string, string > m_ModuleGeomIDMap;

};

#define CustomGeomMgr CustomGeomMgrSingleton::getInstance()

//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//

//==== Custom Geom XSec =====//
class CustomXSec : public SkinXSec
{
public:

    CustomXSec( XSecCurve *xsc, bool use_left );

    virtual void Update();
    virtual void CopyBasePos( XSec* xs );

    virtual void SetLoc( const vec3d & loc );
    virtual vec3d GetLoc()                                  { return m_Loc; }
    virtual void SetRot( const vec3d & rot );
    virtual vec3d GetRot()                                  { return m_Rot; }
    virtual void SetCenterRot( const vec3d & cent );
    virtual vec3d GetCenterRot()                            { return m_CenterRot; }

    virtual double GetScale();

protected:

    vec3d m_Loc;
    vec3d m_Rot;
    vec3d m_CenterRot;

};


//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//

//==== Custom Geom ====//
class CustomGeom : public Geom
{
public:
    CustomGeom( Vehicle* vehicle_ptr );
    virtual ~CustomGeom();

    void Clear();
    void InitGeom( );
    void SetScriptModuleName( const string& name )      { m_ScriptModuleName = name; }
    string GetScriptModuleName()                        { return m_ScriptModuleName; }

    //==== Add a Parm Return ID ====//
    string AddParm( int type, const string & name, const string & group );
    string FindParmID( int index );

    //==== Add Gui ====//
    int AddGui( const GuiDef & gd );
    vector< GuiDef > GetGuiDefVec()                    { return m_GuiDefVec; }
    void AddGuiTriggerEvent( int gui_index );
    bool CheckClearTriggerEvent( int gui_index );

    void AddUpdateGui( const GuiUpdate & gu );
    vector< GuiUpdate > GetGuiUpdateVec();

    //==== Add XSec Surface Return ID =====//
    string AddXSecSurf();
    void RemoveXSecSurf( const string& id );
    void ClearXSecSurfs();

    virtual int GetNumXSecSurfs()                    { return ( int )m_XSecSurfVec.size(); }
    virtual XSecSurf* GetXSecSurf( int index );

    //==== Skin XSecs ====//
    virtual void SkinXSecSurf( bool closed_flag );
    virtual void CloneSurf( int index, Matrix4d & mat );
    virtual void TransformSurf( int index, Matrix4d & mat );

    //==== Encode/Decode XML ====//
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    //==== CFD Sources =====//
    virtual void AddDefaultSources( double base_len = 1.0);
    virtual void SetUpDefaultSource( SourceData & sd )      { m_DefaultSourceVec.push_back( sd ); }
    virtual void ClearAllDefaultSources()                   { m_DefaultSourceVec.clear(); }

    virtual void ComputeCenter();

protected:

    bool m_InitGeomFlag;
    string m_ScriptModuleName;

    vector< Parm* > m_ParmVec;              // Storage For Pointers
    vector< GuiDef > m_GuiDefVec;           // Gui Definition
    vector< GuiUpdate > m_UpdateGuiVec;     // Match Gui with Parms
    vector< XSecSurf* > m_XSecSurfVec;
    vector< int > m_TriggerVec;
    vector< SourceData > m_DefaultSourceVec;



    virtual void UpdateSurf();

};


#endif // !defined(VSPCUSTOMGEOM__INCLUDED_)
