//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
//// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VarPresetMgr.h: Design Variable Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "VarPresetMgr.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include "IDMgr.h"

//=========================================================================//
//=======================        Setting       ============================//
//=========================================================================//

Setting::Setting()
{
    m_ID = GenerateID();
}

Setting::~Setting()
{

}

void Setting::ChangeID( const string& newID )
{
    VarPresetMgr.RemoveSetting( this );

    m_ID = newID;

    VarPresetMgr.AddSetting( this );
}

void Setting::SetParmVal( int index, double val )
{
    if ( index >= 0 && index < m_ParmValVec.size() )
    {
        m_ParmValVec[ index ] = val;
    }
}

double Setting::GetParmVal( int index ) const
{
    if ( index >= 0 && index < m_ParmValVec.size() )
    {
        return m_ParmValVec[ index ];
    }
    return 0.0;
}

void Setting::PushBackParmVal( double val )
{
    m_ParmValVec.push_back( val );
}

void Setting::EraseParmVal( int index )
{
    if ( index >= 0 && index < m_ParmValVec.size() )
    {
        m_ParmValVec.erase( m_ParmValVec.begin() + index );
    }
}

void Setting::ClearAllParmVals()
{
    m_ParmValVec.clear();
}

xmlNodePtr Setting::EncodeXml( xmlNodePtr &node )
{
    xmlNodePtr setting_node = xmlNewChild( node, NULL, BAD_CAST "Setting", NULL );
    if ( setting_node )
    {
        XmlUtil::AddStringNode( setting_node, "ID", m_ID );
        XmlUtil::AddStringNode( setting_node, "Name", m_Name );
        XmlUtil::AddVectorDoubleNode( setting_node, "ParmVals", m_ParmValVec );
    }
    return node;
}

xmlNodePtr Setting::DecodeXml( xmlNodePtr &setting_node )
{
    if ( setting_node )
    {
        m_ID = IDMgr.RemapID( XmlUtil::FindString( setting_node, "ID", m_ID ) );
        m_Name = XmlUtil::FindString( setting_node, "Name", string() );
        m_ParmValVec = XmlUtil::ExtractVectorDoubleNode( setting_node, "ParmVals" );
    }
    return setting_node;
}

string Setting::GenerateID()
{
    return GenerateRandomID( vsp::ID_LENGTH_PRESET_SETTING );
}

//=========================================================================//
//====================        SettingGroup       ==========================//
//=========================================================================//

SettingGroup::SettingGroup()
{
    m_ID = GenerateID();
}

SettingGroup::~SettingGroup()
{

}

void SettingGroup::ChangeID( const string& newID )
{
    VarPresetMgr.RemoveSettingGroup( this );

    m_ID = newID;

    VarPresetMgr.AddSettingGroup( this );
}

bool SettingGroup::AddSetting( Setting* s, bool savevals )
{
    if ( VarPresetMgr.AddSetting( s ) )
    {
        if ( savevals )
        {
            vector < double > valvec( m_ParmIDVec.size(), 0.0 );
            s->SetParmValVec( valvec );
            SaveSetting( s->GetID() );
        }
        m_SettingIDVec.push_back( s->GetID() );

        return true;
    }
    return false;
}

void SettingGroup::RemoveSetting( Setting* s )
{
    vector_remove_val( m_SettingIDVec, s->GetID() );
}

bool SettingGroup::AddParm( const string &id )
{
    Parm *p = ParmMgr.FindParm( id );

    if ( p )
    {
        if ( !vector_contains_val( m_ParmIDVec, id ) )
        {
            m_ParmIDVec.push_back( id );
            double val = p->Get();

            for ( int i = 0; i < m_SettingIDVec.size(); i++ )
            {
                Setting *s = VarPresetMgr.FindSetting( m_SettingIDVec[i] );
                if ( s )
                {
                    s->PushBackParmVal( val );
                }
            }
            return true;
        }
    }
    return false;
}

void SettingGroup::RemoveParm( const string &id )
{
    Parm *p = ParmMgr.FindParm( id );

    if ( p )
    {
        int index = vector_find_val( m_ParmIDVec, id );

        m_ParmIDVec.erase( m_ParmIDVec.begin() + index );

        for ( int i = 0; i < m_SettingIDVec.size(); i++ )
        {
            Setting *s = VarPresetMgr.FindSetting( m_SettingIDVec[i] );
            if ( s )
            {
                s->EraseParmVal( index );
            }
        }
    }
}

void SettingGroup::RemoveAllParms()
{
    for ( int i = 0; i < m_SettingIDVec.size(); i++ )
    {
        Setting *s = VarPresetMgr.FindSetting( m_SettingIDVec[i] );
        if ( s )
        {
            s->ClearAllParmVals();
        }
    }

    m_ParmIDVec.clear();
}


void SettingGroup::ApplySetting( const string &id )
{
    Setting* s = VarPresetMgr.FindSetting( id );
    if ( s )
    {
        vector < double > valvec = s->GetParmValVec();

        for ( int i = 0; i < m_ParmIDVec.size(); i++ )
        {
            Parm *p = ParmMgr.FindParm( m_ParmIDVec[i] );
            if ( p )
            {
                p->Set( valvec[i] );
            }
        }
        VehicleMgr.GetVehicle()->Update();
    }
}

void SettingGroup::SaveSetting( const string &id )
{
    Setting* s = VarPresetMgr.FindSetting( id );
    if ( s )
    {
        vector < double > valvec( m_ParmIDVec.size(), 0.0 );
        for ( int i = 0; i < m_ParmIDVec.size(); i++ )
        {
            Parm *p = ParmMgr.FindParm( m_ParmIDVec[i] );
            if ( p )
            {
                valvec[i] = p->Get();
            }
        }
        s->SetParmValVec( valvec );
    }
}

bool SettingGroup::CheckSetting( const string &id )
{
    Setting* s = VarPresetMgr.FindSetting( id );
    if ( s )
    {
        bool match = true;
        vector < double > valvec = s->GetParmValVec();

        for ( int i = 0; i < m_ParmIDVec.size(); i++ )
        {
            Parm *p = ParmMgr.FindParm( m_ParmIDVec[i] );
            if ( p )
            {
                if ( p->Get() != valvec[i] ) // Parm does not match
                {
                    match = false;
                }
            }
            else // Parm not ound
            {
                match = false;
            }
        }
        return match;
    }
    return false; // Setting not found
}

bool SettingGroup::HasSetting( const string &id ) const
{
    return vector_contains_val( m_SettingIDVec, id );
}

xmlNodePtr SettingGroup::EncodeXml( xmlNodePtr &node )
{
    xmlNodePtr settinggroup_node = xmlNewChild( node, NULL, BAD_CAST "SettingGroup", NULL );
    if ( settinggroup_node )
    {
        XmlUtil::AddStringNode( settinggroup_node, "ID", m_ID );
        XmlUtil::AddStringNode( settinggroup_node, "Name", m_Name );

        for ( int i = 0; i < m_ParmIDVec.size(); i++ )
        {
            xmlNodePtr n = xmlNewChild( settinggroup_node, NULL, BAD_CAST "Parm", NULL );
            if ( n )
            {
                XmlUtil::AddStringNode( n, "ID", m_ParmIDVec[i] );
            }
        }

        for ( int i = 0; i < m_SettingIDVec.size(); i++ )
        {
            Setting *s = VarPresetMgr.FindSetting( m_SettingIDVec[i] );
            if ( s )
            {
                s->EncodeXml( settinggroup_node );
            }
        }
    }
    return node;
}

xmlNodePtr SettingGroup::DecodeXml( xmlNodePtr &grp_node )
{
    if ( grp_node )
    {
        m_ID = IDMgr.RemapID( XmlUtil::FindString( grp_node, "ID", m_ID ) );

        m_Name = XmlUtil::FindString( grp_node, "Name", string() );

        int num_parm = XmlUtil::GetNumNames( grp_node, "Parm" );

        for ( int i = 0; i < num_parm; i++ )
        {
            xmlNodePtr n = XmlUtil::GetNode( grp_node, "Parm", i );
            m_ParmIDVec.push_back( IDMgr.RemapID( XmlUtil::FindString( n, "ID", string() ) ) );
        }

        int num_setting = XmlUtil::GetNumNames( grp_node, "Setting" );

        for ( int i = 0; i < num_setting; i++ )
        {
            xmlNodePtr setting_node = XmlUtil::GetNode( grp_node, "Setting", i );

            if ( setting_node )
            {
                Setting* s = new Setting();

                if ( s )
                {
                    s->DecodeXml( setting_node );
                    AddSetting( s, false );
                }
            }
        }

    }

    return grp_node;
}

string SettingGroup::GenerateID()
{
    return GenerateRandomID( vsp::ID_LENGTH_PRESET_GROUP );
}

//=========================================================================//
//===============        VarPresetMgrSingleton       ======================//
//=========================================================================//

VarPresetMgrSingleton::VarPresetMgrSingleton()
{
}

VarPresetMgrSingleton::~VarPresetMgrSingleton()
{
    Wype();
}

void VarPresetMgrSingleton::Renew()
{
    Wype();
}

void VarPresetMgrSingleton::Wype()
{
    unordered_map< string, Setting* >::iterator setting_iter;
    for ( setting_iter = m_SettingMap.begin(); setting_iter != m_SettingMap.end(); ++setting_iter )
    {
        delete setting_iter->second;
    }
    m_SettingMap.clear();

    unordered_map< string, SettingGroup* >::iterator group_iter;
    for ( group_iter = m_SettingGroupMap.begin(); group_iter != m_SettingGroupMap.end(); ++group_iter )
    {
        delete group_iter->second;
    }
    m_SettingGroupMap.clear();
    m_SettingGroupVec.clear();
}

bool VarPresetMgrSingleton::AddSetting( Setting* s )
{
    if ( !s )
    {
        return false;
    }

    string id = s->GetID();

    if ( id.size() == 0 )
    {
        return false;
    }

    //==== Check If Already Added ====//
    if ( m_SettingMap.find( s->GetID() ) != m_SettingMap.end() )
    {
        return false;
    }

    m_SettingMap[id] = s;

    return true;
}

void VarPresetMgrSingleton::RemoveSetting( Setting* s )
{
    unordered_map< string, Setting* >::iterator iter;
    iter = m_SettingMap.find( s->GetID() );

    if ( iter != m_SettingMap.end() && iter->second == s )
    {
        m_SettingMap.erase( iter );
    }
}

Setting* VarPresetMgrSingleton::FindSetting( const string & id ) const
{
    unordered_map< string, Setting* >::const_iterator iter;

    iter = m_SettingMap.find( id );
    if ( iter != m_SettingMap.end() )
    {
        return iter->second;
    }
    return nullptr;
}

bool VarPresetMgrSingleton::AddSettingGroup( SettingGroup* sg )
{
    if ( !sg )
    {
        return false;
    }

    string id = sg->GetID();

    if ( id.size() == 0 )
    {
        return false;
    }

    //==== Check If Already Added ====//
    if ( m_SettingGroupMap.find( sg->GetID() ) != m_SettingGroupMap.end() )
    {
        return false;
    }
    m_SettingGroupMap[id] = sg;
    m_SettingGroupVec.push_back( sg->GetID() );

    return true;
}

void VarPresetMgrSingleton::RemoveSettingGroup( SettingGroup* sg )
{
    unordered_map< string, SettingGroup* >::iterator iter;
    iter = m_SettingGroupMap.find( sg->GetID() );

    if ( iter !=  m_SettingGroupMap.end() && iter->second == sg )
    {
        m_SettingGroupMap.erase( iter );
        vector_remove_val( m_SettingGroupVec, sg->GetID() );
    }
}

SettingGroup* VarPresetMgrSingleton::FindSettingGroup( const string & id ) const
{
    unordered_map< string, SettingGroup* >::const_iterator iter;

    iter = m_SettingGroupMap.find( id );
    if ( iter != m_SettingGroupMap.end() )
    {
        return iter->second;
    }
    return nullptr;
}

void VarPresetMgrSingleton::DeleteSettingGroup( const string & gid )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

    if ( sg )
    {
        VarPresetMgr.DeleteAllSettingsInGroup( gid );
        VarPresetMgr.RemoveSettingGroup( sg );
        delete sg;
    }
}

void VarPresetMgrSingleton::DeleteAllSettingGroups()
{
    Wype();
}

void VarPresetMgrSingleton::DeleteSetting( const string &gid, const string &sid )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

    if ( sg )
    {
        Setting *s = VarPresetMgr.FindSetting( sid );
        if ( s )
        {
            sg->RemoveSetting( s );
            RemoveSetting( s );

            delete s;
        }
    }
}

void VarPresetMgrSingleton::DeleteAllSettingsInGroup( const string &gid )
{
    SettingGroup *sg = VarPresetMgr.FindSettingGroup( gid );

    if ( sg )
    {
        vector < string > settings = sg->GetSettingIDVec();
        for ( int i = 0; i < settings.size(); i++ )
        {
            Setting *s = VarPresetMgr.FindSetting( settings[i] );
            if ( s )
            {
                sg->RemoveSetting( s );
                RemoveSetting( s );

                delete s;
            }
        }
    }
}

vector < string > VarPresetMgrSingleton::GetAllSettingGroups()
{
    return m_SettingGroupVec;
}

void VarPresetMgrSingleton::ConvertOldToNew()
{
    vector < string > gnames = OldVarPresetMgr.GetGroupNames();

    for ( int i = 0; i < gnames.size(); i++ )
    {
        SettingGroup *sg = new SettingGroup();
        sg->SetName( gnames[ i ] );

        if ( VarPresetMgr.AddSettingGroup( sg ) )
        {
            vector < string > pids = OldVarPresetMgr.GetParmIDs( i );

            sg->SetParmIDVec( pids );

            vector < string > snames = OldVarPresetMgr.GetSettingNames( i );

            for ( int j = 0; j < snames.size(); j++ )
            {
                Setting *s = new Setting();
                s->SetName( snames[ j ] );

                vector < double > valvec = OldVarPresetMgr.GetParmVals( i, j );
                s->SetParmValVec( valvec );

                if ( !sg->AddSetting( s, false ) )
                {
                    delete s;
                }
            }
        }
        else
        {
            delete sg;
        }
    }

    OldVarPresetMgr.Renew();
}

xmlNodePtr VarPresetMgrSingleton::EncodeXml( xmlNodePtr &node ) const
{
    xmlNodePtr varpresetmgr_node = xmlNewChild( node, NULL, BAD_CAST"VariablePresets", NULL );

    for ( int i = 0; i < m_SettingGroupVec.size(); i++ )
    {
        FindSettingGroup( m_SettingGroupVec[i] )->EncodeXml( varpresetmgr_node );
    }

    return varpresetmgr_node;
}

xmlNodePtr VarPresetMgrSingleton::DecodeXml( xmlNodePtr &node )
{
    xmlNodePtr varpresetmgr_node = XmlUtil::GetNode( node, "VariablePresets", 0 );

    if ( varpresetmgr_node )
    {
        int num_grp = XmlUtil::GetNumNames( varpresetmgr_node, "SettingGroup" );

        for ( unsigned int i = 0; i < num_grp; i++ )
        {
            xmlNodePtr grp_node = XmlUtil::GetNode( varpresetmgr_node, "SettingGroup", i );

            if ( grp_node )
            {
                SettingGroup* grp = new SettingGroup();

                if ( grp )
                {
                    grp->DecodeXml( grp_node );

                    AddSettingGroup( grp );
                }
            }
        }
    }

    return node;
}




//=========================================================================//
//=======================        Preset        ============================//
//=========================================================================//

//==== Constructor ====//
OldPreset::OldPreset()
{
    m_GroupName = "";
}

//==== Destructor ====//
OldPreset::~OldPreset()
{
}

//==== Init ====//
void OldPreset::Init( const string &group_name, const vector < string > &p_IDvec)
{
    m_GroupName = group_name;

    m_ParmIDVec = p_IDvec;
}

//==== New Setting ====//
void OldPreset::NewSet( const string &set_name, vector < double > p_ValVec)
{
    m_SettingNameVec.push_back( set_name );

    m_ParmValVec.push_back( p_ValVec );
}

//==== Decode XML ====//
OldPreset OldPreset::DecodeXml( xmlNodePtr &varpresetnode, int i )
{
    char buffer [50];
    string name = "";
    string groupName;
    string setName;
    vector < double > p_vals;
    vector < string > p_IDs;
    OldPreset temp_preset;

    // Grab Size of Settings and Parms/Setting
    vector <int> numSetqGroup = XmlUtil::ExtractVectorIntNode( varpresetnode, "NumSettingsPerGroup" );
    vector <int> numParmqGroup = XmlUtil::ExtractVectorIntNode( varpresetnode, "NumParmsPerGroup" );

    // Grab Group Qualities
    snprintf( buffer, sizeof( buffer ),  "Group%i", i );
    groupName = XmlUtil::FindString( varpresetnode, buffer, name );

    snprintf( buffer, sizeof( buffer ),  "Group%iQualities", i );
    xmlNodePtr groupqualnode = XmlUtil::GetNode( varpresetnode, buffer, 0 );

    for ( int j = 0; j < numParmqGroup[ i ]; j++ )
    {
        snprintf( buffer, sizeof( buffer ),  "ParmID%i", j);
        p_IDs.push_back( IDMgr.RemapID( XmlUtil::FindString( groupqualnode, buffer, name) ) );
    }

    // Grab Setting Qualities
    for ( int j = 0; j < numSetqGroup[ i ]; j++ )
    {
        snprintf( buffer, sizeof( buffer ),  "SettingName%i", j );
        setName = XmlUtil::FindString( groupqualnode, buffer, name );

        snprintf( buffer, sizeof( buffer ),  "Setting%iQualities", j );
        xmlNodePtr setqualnode = XmlUtil::GetNode( groupqualnode, buffer, 0 );
        p_vals = XmlUtil::ExtractVectorDoubleNode( setqualnode, "ParmVals" );

        temp_preset.NewSet( setName, p_vals );

        p_vals.clear();
        setName.clear();
    }

    // Initialize Preset & Set Current Setting Name
    temp_preset.Init( groupName, p_IDs );

    p_IDs.clear();
    groupName.clear();

    return temp_preset;
}

//=========================================================================//
//=======================      VarPresetMgr     ===========================//
//=========================================================================//

//==== Constructor ====//
OldVarPresetMgrSingleton::OldVarPresetMgrSingleton()
{
    Init();
}

void OldVarPresetMgrSingleton::Init()
{
    m_PresetVec.clear();
}

void OldVarPresetMgrSingleton::Wype()
{
    DelAllVars();
    m_VarVec = vector< string >();
}

void OldVarPresetMgrSingleton::Renew()
{
    Wype();
    Init();
}

//==== Delete All Variables ====//
void OldVarPresetMgrSingleton::DelAllVars()
{
    m_VarVec.clear();
}

vector <string> OldVarPresetMgrSingleton::GetParmIDs( int group_index )
{
    vector <string> vec;

    if (group_index != -1 )
    {
        return m_PresetVec[ group_index ].GetParmIDs();
    }
    else
    {
        return vec;
    }
}

vector <double> OldVarPresetMgrSingleton::GetParmVals( int group_index, int set_index )
{
    vector<double> vec;

    if ( group_index != -1 && set_index != -1 )
    {
        return m_PresetVec[ group_index ].GetParmVals(set_index);
    }
    else
    {
        return vec;
    }
}

//==== Get Setting Names ====//
vector <string> OldVarPresetMgrSingleton::GetSettingNames( int group_index )
{
    if ( group_index >= 0 && group_index < m_PresetVec.size() )
    {
        return m_PresetVec[group_index].GetSettingNameVec();
    }
    else
    {
        return vector < string > {};
    }
}

//==== Get Group Names ====//
vector <string> OldVarPresetMgrSingleton::GetGroupNames()
{
    vector <string> vec( m_PresetVec.size() );
    for ( int i = 0; i < m_PresetVec.size(); i++ )
    {
        vec[i] = m_PresetVec[ i ].GetGroupName();
    }
    return vec;
}

//==== Decode XML ====//
xmlNodePtr OldVarPresetMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    // Find Var Preset Node
    xmlNodePtr varpresetnode = XmlUtil::GetNode( node, "VariablePresets", 0 );

    // If Not Exist - Return
    if ( varpresetnode == NULL )
    {
        return NULL;
    }

    // Decode # of Presets
    unsigned int numGroup = XmlUtil::FindInt( varpresetnode, "NumGroups", 0 );

    // Resize PresetVec
    m_PresetVec.resize(numGroup);

    for ( int i = 0; i < numGroup; i++ )
    {
        // Decode Preset Details
        m_PresetVec[ i ] = m_PresetVec[ i ].DecodeXml( varpresetnode, i );
    }

    return varpresetnode;
}
