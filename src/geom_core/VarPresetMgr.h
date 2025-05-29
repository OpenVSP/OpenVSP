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
#include <unordered_map>

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
    void ClearAllParmVals();

    void SetGroupID( const string &id )
    {
        m_GroupID = id;
    }

    string GetGroupID()
    {
        return m_GroupID;
    }

    xmlNodePtr EncodeXml( xmlNodePtr &node );
    xmlNodePtr DecodeXml( xmlNodePtr &node );

    void AttachAttrCollection()
    {
        m_AttributeCollection.SetCollAttach( m_ID, vsp::ATTROBJ_VARSETTING );
    }

    AttributeCollection* GetAttributeCollection()
    {
        return &m_AttributeCollection;
    }

protected:
    string GenerateID();

    string m_ID;
    string m_Name;
    string m_GroupID;
    vector < double > m_ParmValVec;

    AttributeCollection m_AttributeCollection;

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

    bool AddSetting( Setting* s, bool savevals );
    void RemoveSetting( Setting* s );

    bool AddParm( const string &id );
    void RemoveParm( const string &id );
    void RemoveAllParms();

    void SetParmIDVec( const vector < string > &pid_vec )      { m_ParmIDVec = pid_vec; }
    vector < string > GetParmIDVec() const       { return m_ParmIDVec; }

    vector < string > GetSettingIDVec() const                  { return m_SettingIDVec; }

    void ApplySetting( const string &id );
    void SaveSetting( const string &id );
    bool CheckSetting( const string &id );
    bool HasSetting( const string &id ) const;

    xmlNodePtr EncodeXml( xmlNodePtr &node );
    xmlNodePtr DecodeXml( xmlNodePtr &node );

    void AttachAttrCollection()
    {
        m_AttributeCollection.SetCollAttach( m_ID, vsp::ATTROBJ_VARGROUP );
    }

    AttributeCollection* GetAttributeCollection()
    {
        return &m_AttributeCollection;
    }

protected:
    string GenerateID();

    string m_ID;
    string m_Name;

    vector < string > m_ParmIDVec;
    vector < string > m_SettingIDVec;

    AttributeCollection m_AttributeCollection;

};


class VarPresetMgrSingleton
{
public:
    static VarPresetMgrSingleton& getInstance()
    {
        static VarPresetMgrSingleton instance;
        return instance;
    }

    virtual ~VarPresetMgrSingleton();

    void Renew();
    void Wype();

    bool AddSetting( Setting* s );
    void RemoveSetting( Setting* s );
    Setting* FindSetting( const string & id ) const;

    bool AddSettingGroup( SettingGroup* sg );
    void RemoveSettingGroup( SettingGroup* sg );
    SettingGroup* FindSettingGroup( const string & id ) const;

    void DeleteSettingGroup( const string & gid );
    void DeleteAllSettingGroups();
    void DeleteSetting( const string &gid, const string &sid );
    void DeleteAllSettingsInGroup( const string &gid );

    vector < string > GetAllSettingGroups();

    void ConvertOldToNew();

    xmlNodePtr EncodeXml( xmlNodePtr &node ) const;
    xmlNodePtr DecodeXml( xmlNodePtr &node );

private:
    VarPresetMgrSingleton();
    VarPresetMgrSingleton( VarPresetMgrSingleton const& copy ) = delete;             // Not Implemented
    VarPresetMgrSingleton& operator=( VarPresetMgrSingleton const& copy ) = delete;  // Not Implemented

    unordered_map< string, Setting* > m_SettingMap;                         // ID->Setting Map
    unordered_map< string, SettingGroup* > m_SettingGroupMap;               // ID->Setting Group Map
    vector < string > m_SettingGroupVec;                                    // Ordered Setting Group ID's

};

#define VarPresetMgr VarPresetMgrSingleton::getInstance()

// The following two classes are the essence of the original Variable Preset implementation.
// This version relied on lookups based on name and index in a way that it was not suitable for
// extension to Modes.  It was re-written to the preceeding ID-based implementation.
// This essence is enough to read in legacy files with Variable Presets and to convert them to
// the new implementation.

//==== Preset ====//
class OldPreset
{
public:
    OldPreset();
    virtual ~OldPreset();

    virtual void Init( const string &group_name, const vector< string > &p_IDvec);

    virtual string GetGroupName()                               { return m_GroupName; }
    virtual vector < string > GetSettingNameVec()               { return m_SettingNameVec; }
    virtual vector < string > GetParmIDs()                      { return m_ParmIDVec; }
    virtual vector < double > GetParmVals( int set_index )      { return m_ParmValVec[ set_index ]; }

    virtual void NewSet( const string &set_name, const vector<double> &p_ValVec );

    virtual OldPreset DecodeXml( xmlNodePtr &varpresetnode, int i );

protected:
    string m_GroupName;

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
    virtual void DelAllVars();

    virtual vector < string > GetParmIDs( int group_index );
    virtual vector < double> GetParmVals( int group_index, int set_index );

    virtual vector < string > GetSettingNames( int group_index );
    virtual vector < string > GetGroupNames();

    virtual xmlNodePtr DecodeXml( xmlNodePtr &node );

private:
    OldVarPresetMgrSingleton();
    OldVarPresetMgrSingleton( OldVarPresetMgrSingleton const& copy ) = delete;          // Not Implemented
    OldVarPresetMgrSingleton& operator=( OldVarPresetMgrSingleton const& copy ) = delete; // Not Implemented

    void Init();
    void Wype();

    vector < string > m_VarVec;
    vector < OldPreset > m_PresetVec;
};

#define OldVarPresetMgr OldVarPresetMgrSingleton::getInstance()

#endif
