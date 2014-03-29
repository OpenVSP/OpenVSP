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

    //==== Init Geom ====//
    void InitGeom( const string& id );
    void SetCurrCustomGeom( const string& id )
    {
        m_CurrGeom = id;
    }
    string GetCurrCustomGeom()
    {
        return m_CurrGeom;
    }

    void ReadCustomScripts();
    vector< GeomType > GetCustomTypes()
    {
        return m_CustomTypeVec;
    }

    //==== Adds A Parm To The Current Custom Geom - Return Index of The Added Parm ====//
    string AddParm( int type, const string & name, const string & group );
    string GetCustomParm( int index );

    //==== Add A Gui Device Constructor to Current Custom Geom - Gui Will Be Created By Custom Screen (if connected) ===//
    int AddGui( int type, const string & label );
    vector< GuiDef > GetGuiDefVec( const string & geom_id );

    //==== Add Gui->Parm Pairing To Update Vec ====//
    void AddUpdateGui( int gui_id, const string & parm_id );
    vector< GuiUpdate > GetGuiUpdateVec();

    //==== Add XSec Surface To Current Geom - Return ID =====//
    string AddXSecSurf();
    void SkinXSecSurf();

private:

    CustomGeomMgrSingleton();
    CustomGeomMgrSingleton( CustomGeomMgrSingleton const& copy );          // Not Implemented
    CustomGeomMgrSingleton& operator=( CustomGeomMgrSingleton const& copy ); // Not Implemented

    string m_CurrGeom;
    string m_ScriptDir;
    vector< GeomType > m_CustomTypeVec;

};

#define CustomGeomMgr CustomGeomMgrSingleton::getInstance()

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

    void InitGeom( const string & module_name );
    void SetScriptModuleName( const char* name )
    {
        m_ScriptModuleName = name;
    }
    string GetScriptModuleName()
    {
        return m_ScriptModuleName;
    }


    //==== Add a Parm Return ID ====//
    string AddParm( int type, const string & name, const string & group );
    string FindParmID( int index );

    //==== Add Gui ====//
    int AddGui( const GuiDef & gd );
    vector< GuiDef > GetGuiDefVec()
    {
        return m_GuiDefVec;
    }

    void AddUpdateGui( const GuiUpdate & gu );
    vector< GuiUpdate > GetGuiUpdateVec();

    //==== Add XSec Surface Return ID =====//
    string AddXSecSurf();

    virtual int GetNumXSecSurfs()
    {
        return ( int )m_XSecSurfVec.size();
    }
    virtual XSecSurf* GetXSecSurf( int index );

    //==== Skin XSecs ====//
    virtual void SkinXSecSurf();


protected:

    bool m_InitGeomFlag;
    string m_ScriptModuleName;

    vector< Parm* > m_ParmVec;              // Storage For Pointers
    vector< GuiDef > m_GuiDefVec;           // Gui Definition
    vector< GuiUpdate > m_UpdateGuiVec;     // Match Gui with Parms
    vector< XSecSurf* > m_XSecSurfVec;

    virtual void UpdateSurf();
    virtual int GetNumMainSurfs()
    {
        return 1;
    }

};


#endif // !defined(VSPCUSTOMGEOM__INCLUDED_)
