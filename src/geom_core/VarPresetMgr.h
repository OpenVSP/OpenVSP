//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VarPresetMgr.h: interface for the Saved Parameter Singleton.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VARPRESET__INCLUDED_)
#define VARPRESET__INCLUDED_

#include "Parm.h"
#include "APIDefines.h"
#include "ParmContainer.h"

#include <vector>
#include <string>

using std::string;
using std::vector;


class Setting
{
public:
    Setting();
    ~Setting();

    string GetName() const                       { return m_Name; }
    void SetName( const string & name )          { m_Name = name; }

    string GetID() const
    {
        return m_ID;
    }
    void ChangeID( const string& newID );

    void SetParmValVec( const vector < double > &valvec )       { m_ParmValVec = valvec; };
    vector < double > GetParmValVec() const      { return m_ParmValVec; }

    void SetParmVal( int index, double val );
    double GetParmVal( int index ) const;

    void PushBackParmVal( double val );
    void EraseParmVal( int index );

    xmlNodePtr EncodeXml( xmlNodePtr &node );
    xmlNodePtr DecodeXml( xmlNodePtr &node );

protected:
    string GenerateID();

    string m_ID;
    string m_Name;
    vector < double > m_ParmValVec;
};

class SettingGroup
{
public:
    SettingGroup();
    ~SettingGroup();

    string GetName() const                       { return m_Name; }
    void SetName( const string & name )          { m_Name = name; }

    string GetID() const
    {
        return m_ID;
    }
    void ChangeID( const string& newID );

    bool AddSetting( Setting* s );
    void RemoveSetting( Setting* s );

    bool AddParm( const string &id );
    void RemoveParm( const string &id );

    void SetParmIDVec( const vector < string > &pid_vec )      { m_ParmIDVec = pid_vec; }
    vector < string > GetParmIDVec() const       { return m_ParmIDVec; }

    vector < string > GetSettingIDVec() const                  { return m_SettingIDVec; }

    void ApplySetting( const string &id );
    void SaveSetting( const string &id );
    bool CheckSetting( const string &id );

    xmlNodePtr EncodeXml( xmlNodePtr &node );
    xmlNodePtr DecodeXml( xmlNodePtr &node );

protected:
    string GenerateID();

    string m_ID;
    string m_Name;

    vector < string > m_ParmIDVec;
    vector < string > m_SettingIDVec;

};


class VarPresetMgrSingleton
{
public:
    static VarPresetMgrSingleton& getInstance()
    {
        static VarPresetMgrSingleton instance;
        return instance;
    }

    void Renew();

    bool AddSetting( Setting* s );
    void RemoveSetting( Setting* s );
    Setting* FindSetting( const string & id );

    bool AddSettingGroup( SettingGroup* sg );
    void RemoveSettingGroup( SettingGroup* sg );
    SettingGroup* FindSettingGroup( const string & id );

    vector < string > GetAllSettingGroups();

    xmlNodePtr EncodeXml( xmlNodePtr &node ) const;
    xmlNodePtr DecodeXml( xmlNodePtr &node );

private:
    VarPresetMgrSingleton();
    VarPresetMgrSingleton( VarPresetMgrSingleton const& copy );             // Not Implemented
    VarPresetMgrSingleton& operator=( VarPresetMgrSingleton const& copy );  // Not Implemented

    unordered_map< string, Setting* > m_SettingMap;                         // ID->Setting Map
    unordered_map< string, SettingGroup* > m_SettingGroupMap;               // ID->Setting Group Map
    vector < string > m_SettingGroupVec;                                    // Ordered Setting Group ID's

};

#define VarPresetMgr VarPresetMgrSingleton::getInstance()






//==== Preset ====//
class OldPreset
{
public:
    OldPreset();
    virtual ~OldPreset();

    virtual void Init( const string &group_name, const vector< string > &p_IDvec);

    virtual string GetGroupName()                               { return m_GroupName; }
    virtual string GetSettingName( int index )                  { return m_SettingNameVec[index]; }
    virtual vector < string > GetSettingNameVec()               { return m_SettingNameVec; }
    virtual string GetCurSetName()                              { return m_CurSetName; }
    virtual vector < string > GetParmIDs()                      { return m_ParmIDVec; }
    virtual int GetNumSet()                                     { return m_SettingNameVec.size(); }
    virtual vector < double > GetParmVals( int set_index )      { return m_ParmValVec[ set_index ]; }

    virtual int GetSettingIndex();
    virtual int GetSettingIndex( const string &set_name );

    virtual bool SettingNameExist( const string &name );
    //virtual void SetGroupName( const string &name )             { m_GroupName = name; }
    virtual void SetCurSetName( const string &name )            { m_CurSetName = name; }

    virtual void NewSet( const string &set_name, vector < double > p_ValVec );
    virtual void DeleteSet( int set_index );

    virtual void AddParm( const string &p_ID, double p_val );
    virtual void EditParm( int set_index, const string &p_id, double p_val );
    virtual void DeleteParm( const string &p_id );

    virtual xmlNodePtr EncodeXml( xmlNodePtr &varpresetnode, int i );
    virtual OldPreset DecodeXml( xmlNodePtr &varpresetnode, int i );

protected:

    string m_GroupName;
    string m_CurSetName;

    vector < string > m_ParmIDVec;
    vector < string > m_SettingNameVec;

    vector < vector < double > > m_ParmValVec;
};

//==== Design Variable Manager ====//
class OldVarPresetMgrSingleton
{
public:
    static OldVarPresetMgrSingleton& getInstance()
    {
        static OldVarPresetMgrSingleton instance;
        return instance;
    }

    virtual void Renew();
    virtual void ResetIndexAndText();

    virtual void AddGroup( const string &groupText );
    virtual void AddSetting( const string &settingText );
    virtual bool SavePreset();
    virtual void ApplySetting();
    virtual void SettingChange( int set_index );
    virtual void SettingChange( const string &set_name );
    virtual void GroupChange( int group_index );
    virtual void GroupChange( const string &group_name );
    virtual bool DeletePreset( int group_index, int set_index );
    virtual bool DeletePreset( const string &group_name, const string &set_name );

    bool CheckForDuplicateParm( const string &p_ID );
    bool CheckForDuplicateVar( const string &p );

    virtual bool AddCurrVar();
    virtual void DelCurrVar();
    virtual void DelAllVars();
    virtual bool AddVar( const string &parm_id );
    virtual void SetCurrVarIndex( int i )           { m_CurrVarIndex = i; }
    virtual int  GetCurrVarIndex()                  { return m_CurrVarIndex; }
    virtual string GetCurrVar();
    virtual void CheckVars();
    virtual bool SortVars();
    virtual int GetNumVars()                        { return ( int )m_VarVec.size(); }
    virtual string GetVar( int index );
    virtual void SetWorkingParmID( const string &parm_id );
    virtual void ResetWorkingVar();

    virtual bool CheckForEmpty( int curGroupIndex, int curSettingIndex );

    virtual int GetActiveGroupIndex()                   { return m_CurGroupIndex; }
    virtual string GetActiveGroupText()                 { return m_CurGroupText; }
    virtual int GetActiveSettingIndex()                 { return m_CurSettingIndex; }
    virtual string GetActiveSettingText()               { return m_CurSettingText; }
    virtual int GetNumSet();
    virtual int GetActiveSettingIndexFromGroup( int g_index )   { return m_PresetVec[ g_index ].GetSettingIndex(); }
    virtual vector < OldPreset > GetPresetVec()            { return m_PresetVec; }
    virtual bool GetDeleteFlag()                        { return m_PrevDeleteFlag; }

    virtual vector < string > GetCurrentParmIDs()
    {
        return m_PresetVec[ m_CurGroupIndex ].GetParmIDs();
    }
    virtual vector < string > GetParmIDs( const string &group_name );

    virtual vector < double > GetCurrentParmVals()
    {
        return m_PresetVec[ m_CurGroupIndex ].GetParmVals( m_CurSettingIndex );
    }
    virtual vector < double> GetParmVals( const string &group_name, const string &set_name );

    virtual vector < string > GetSettingNames( int group_index );
    virtual vector < string > GetSettingNames( const string &group_name );
    virtual vector < string > GetGroupNames();

    virtual xmlNodePtr EncodeXml( xmlNodePtr &node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr &node );

private:

    OldVarPresetMgrSingleton();
    OldVarPresetMgrSingleton( OldVarPresetMgrSingleton const& copy );          // Not Implemented
    OldVarPresetMgrSingleton& operator=( OldVarPresetMgrSingleton const& copy ); // Not Implemented

    void Init();
    void Wype();

    int m_CurrVarIndex;

    string m_WorkingParmID;

    vector < string > m_VarVec;

    bool m_PrevDeleteFlag;
    int m_CurGroupIndex;
    int m_CurSettingIndex;
    string m_CurGroupText;
    string m_CurSettingText;
    vector < OldPreset > m_PresetVec;
};

#define OldVarPresetMgr OldVarPresetMgrSingleton::getInstance()

#endif
