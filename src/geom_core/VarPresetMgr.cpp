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
        m_ID = ParmMgr.RemapID( XmlUtil::FindString( setting_node, "ID", m_ID ) );
        m_Name = XmlUtil::FindString( setting_node, "Name", string() );
        m_ParmValVec = XmlUtil::ExtractVectorDoubleNode( setting_node, "ParmVals" );
    }
    return setting_node;
}

string Setting::GenerateID()
{
    return GenerateRandomID( 6 );
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
    VarPresetMgr.RemoveSetting( s );
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
        m_ID = ParmMgr.RemapID( XmlUtil::FindString( grp_node, "ID", m_ID ) );

        m_Name = XmlUtil::FindString( grp_node, "Name", string() );

        int num_parm = XmlUtil::GetNumNames( grp_node, "Parm" );

        for ( int i = 0; i < num_parm; i++ )
        {
            xmlNodePtr n = XmlUtil::GetNode( grp_node, "Parm", i );
            m_ParmIDVec.push_back( ParmMgr.RemapID( XmlUtil::FindString( n, "ID", string() ) ) );
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
    return GenerateRandomID( 5 );
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

vector < string > VarPresetMgrSingleton::GetAllSettingGroups()
{
    return m_SettingGroupVec;
}

xmlNodePtr VarPresetMgrSingleton::EncodeXml( xmlNodePtr &node ) const
{
    xmlNodePtr varpresetmgr_node = xmlNewChild( node, NULL, BAD_CAST"VarPresetMgr", NULL );

    for ( int i = 0; i < m_SettingGroupVec.size(); i++ )
    {
        FindSettingGroup( m_SettingGroupVec[i] )->EncodeXml( varpresetmgr_node );
    }

    return varpresetmgr_node;
}

xmlNodePtr VarPresetMgrSingleton::DecodeXml( xmlNodePtr &node )
{
    xmlNodePtr varpresetmgr_node = XmlUtil::GetNode( node, "VarPresetMgr", 0 );

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

    m_CurSetName = set_name;
}

//==== Delete Setting ====//
void OldPreset::DeleteSet( int set_index )
{
    m_SettingNameVec.erase( m_SettingNameVec.begin() + set_index );
    m_ParmValVec.erase( m_ParmValVec.begin() + set_index );
}

//==== EditParm ====//
void OldPreset::EditParm( int set_index, const string &p_id, double p_val )
{
    int p_index = vector_find_val( m_ParmIDVec, p_id );
    m_ParmValVec[ set_index ][ p_index ] = p_val;
}

//==== Setting Name Exist ====//
bool OldPreset::SettingNameExist( const string &name )
{
    return vector_contains_val( m_SettingNameVec, name );
}

//==== Add Parm ====//
void OldPreset::AddParm( const string &p_ID, double p_val )
{
    if( !vector_contains_val( m_ParmIDVec, p_ID ) )
    {
        m_ParmIDVec.push_back( p_ID );
        for ( int i = 0; i < (int)m_SettingNameVec.size(); i++ )
        {
            m_ParmValVec[ i ].push_back( p_val );
        }
    }
}

//==== Delete Parm ====//
void OldPreset::DeleteParm( const string &p_id )
{
    int p_index = vector_find_val( m_ParmIDVec, p_id );

    m_ParmIDVec.erase( m_ParmIDVec.begin() + p_index );

    for (int i = 0; i < (int)m_ParmValVec.size(); i++ )
    {
        m_ParmValVec[ i ].erase( m_ParmValVec[ i ].begin() + p_index );
    }
}

//==== Encode XML ====//
xmlNodePtr OldPreset::EncodeXml( xmlNodePtr &varpresetnode, int i )
{
    char buffer [50];

    // Encode Group Name
    snprintf( buffer, sizeof( buffer ),  "Group%i", i );
    XmlUtil::AddStringNode( varpresetnode, buffer, m_GroupName );

    // Encode Group Qualities
    snprintf( buffer, sizeof( buffer ),  "Group%iQualities", i );
    xmlNodePtr groupqualnode = xmlNewChild( varpresetnode, NULL, BAD_CAST buffer, NULL );

    XmlUtil::AddStringNode( groupqualnode, "ActiveSettingName" , m_CurSetName );
    for ( int j = 0; j < m_ParmIDVec.size(); j++ )
    {
        snprintf( buffer, sizeof( buffer ),  "ParmID%i", j );
        XmlUtil::AddStringNode( groupqualnode, buffer, m_ParmIDVec[ j ] );
    }

    // Encode Setting Qualities
    for ( int j = 0; j < m_SettingNameVec.size(); j++ )
    {
        snprintf( buffer, sizeof( buffer ),  "SettingName%i", j );
        XmlUtil::AddStringNode( groupqualnode, buffer , m_SettingNameVec[ j ] );

        snprintf( buffer, sizeof( buffer ),  "Setting%iQualities", j );
        xmlNodePtr setqualnode = xmlNewChild( groupqualnode, NULL, BAD_CAST buffer, NULL );

        XmlUtil::AddVectorDoubleNode( setqualnode, "ParmVals", m_ParmValVec[ j ] );
    }

    return varpresetnode;
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
        p_IDs.push_back( ParmMgr.RemapID( XmlUtil::FindString( groupqualnode, buffer, name) ) );
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
    temp_preset.SetCurSetName( XmlUtil::FindString( groupqualnode, "ActiveSettingName", "" ) );

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
    m_CurrVarIndex = -1;
    m_PrevDeleteFlag = false;
    m_WorkingParmID = "";
    m_CurGroupIndex = -1;
    m_CurSettingIndex = -1;
    m_CurGroupText = "";
    m_CurSettingText = "";
    m_PresetVec.clear();
}

void OldVarPresetMgrSingleton::ResetIndexAndText()
{
    m_CurGroupIndex = -1;
    m_CurSettingIndex = -1;
    m_CurGroupText = "";
    m_CurSettingText = "";
}

void OldVarPresetMgrSingleton::Wype()
{
    m_CurrVarIndex = int();
    m_WorkingParmID = string();

    DelAllVars();
    m_VarVec = vector< string >();
}

void OldVarPresetMgrSingleton::Renew()
{
    Wype();
    Init();
}

//==== Check For Duplicate Variable  ====//
bool OldVarPresetMgrSingleton::CheckForDuplicateVar( const string &p )
{
    for ( int i = 0 ; i < ( int )m_VarVec.size() ; i++ )
    {
        if ( m_VarVec[ i ] == p )
        {
            return true;
        }
    }
    return false;
}

bool OldVarPresetMgrSingleton::SortVars()
{
    bool wassorted = std::is_sorted( m_VarVec.begin(), m_VarVec.end(), NameCompare );

    if ( !wassorted )
    {
        std::sort( m_VarVec.begin(), m_VarVec.end(), NameCompare );
    }

    return wassorted;
}

//==== Add New Variable ====//
bool OldVarPresetMgrSingleton::AddVar( const string& parm_id  )
{
    if ( CheckForDuplicateVar( parm_id ) )
    {
        return false;
    }

    //==== Check If ParmIDs Are Valid ====//
    Parm* p = ParmMgr.FindParm( parm_id );

    if ( p == NULL )
    {
        return false;
    }

    m_PresetVec[ m_CurGroupIndex ].AddParm( parm_id, ParmMgr.FindParm( parm_id )->Get() );

    m_VarVec.push_back( parm_id );
    SortVars();
    m_CurrVarIndex = -1;

    return true;
}

//==== Delete Curr Variable ====//
void OldVarPresetMgrSingleton::DelCurrVar()
{
    if ( m_CurrVarIndex < 0 || m_CurrVarIndex >= ( int )m_VarVec.size() )
    {
        return;
    }

    m_PresetVec[ m_CurGroupIndex ].DeleteParm( m_VarVec[ m_CurrVarIndex ] );

    m_VarVec.erase( m_VarVec.begin() +  m_CurrVarIndex );

    m_CurrVarIndex = -1;
}

//==== Delete All Variables ====//
void OldVarPresetMgrSingleton::DelAllVars()
{
    m_VarVec.clear();
    m_CurrVarIndex = -1;
}

//==== Set Working Parm ID ====//
void OldVarPresetMgrSingleton::SetWorkingParmID( const string & parm_id )
{
    if ( !ParmMgr.FindParm( parm_id ) )
    {
        m_WorkingParmID = string();
    }
    else
    {
        m_WorkingParmID = parm_id;
    }
}

//==== Check For Empty ====//
bool OldVarPresetMgrSingleton::CheckForEmpty( int curGroupIndex, int curSettingIndex )
{
    // If there aren't any existing groups or settings, return
    if ( curSettingIndex == -1 || curGroupIndex == -1 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//====  Add Group ====//
void OldVarPresetMgrSingleton::AddGroup( const string &groupText )
{
    //cout << "Add Group Pressed-------------------------------------" << endl;

    // Initialize Variables
    vector <string> pIDvec;
    vector <Parm*> pVec;
    int groupExistFlag = 0;

    // If it exists find the Matching Preset with the Group Name
    if ( GetGroupNames().size() != 0 ) // If there are more than 0 groups
    {
        if ( vector_contains_val( GetGroupNames(), groupText ) )
        {
            return;
        }
    }
    else // No Groups Have Yet to Be Assigned
    {
        // Raise Exists Flag with the creation of 1st group
        groupExistFlag = 1;

        m_CurGroupIndex = 0;
        m_CurGroupText = groupText;
        m_CurSettingIndex = -1;

        OldPreset new_group;
        new_group.Init( groupText, pIDvec );
        m_PresetVec.push_back( new_group );
    }


    // If Group Does Not Exist, Create
    if ( groupExistFlag == 0 )
    {
        OldPreset new_group;
        new_group.Init( groupText, pIDvec );
        m_PresetVec.push_back( new_group );

        m_CurGroupIndex = GetGroupNames().size() - 1;
        m_CurGroupText = groupText;
        m_CurSettingIndex = -1;
        m_CurSettingText = "";

        // Clear VarPresetMgr
        Wype();
    }

    m_PrevDeleteFlag = false;
}

//==== Add Setting ====//
void OldVarPresetMgrSingleton::AddSetting( const string &settingText )
{
    //cout << "Add Setting Pressed -------------------------" << endl;
    string groupText = m_PresetVec[ m_CurGroupIndex ].GetGroupName();
    vector <double> parmVals ( m_PresetVec[ m_CurGroupIndex ].GetParmIDs().size() );

    // Check to See if Set Name already exists
    if ( m_PresetVec[ m_CurGroupIndex ].SettingNameExist( settingText ) )
    {
        return;
    }

    // Need to Add Filling of pVec
    vector <string> pIDvec = m_PresetVec[ m_CurGroupIndex ].GetParmIDs();
    for ( int i = 0; i < GetNumVars(); i++ )
    {
        parmVals[i] = ParmMgr.FindParm( pIDvec[ i ] )->Get();
    }

    // If Does NOT Exist Create and Save
    m_PresetVec[m_CurGroupIndex].NewSet( settingText, parmVals );

    m_CurSettingIndex = m_PresetVec[ m_CurGroupIndex ].GetNumSet() - 1;
    m_CurSettingText = settingText;

    m_PrevDeleteFlag = false;
}

//==== Preset Save ====//
bool OldVarPresetMgrSingleton::SavePreset()
{
    //cout << "Save Button Has Been Pressed----------------------------" << endl;
    int resetFlag = 0;

    if ( CheckForEmpty( m_CurGroupIndex, m_CurSettingIndex ) )
    {
        return false; // Did not save
    }

    // Determine What to do When Save Button is Pressed
    vector <double> p_valvec = m_PresetVec[ m_CurGroupIndex ].GetParmVals( m_CurSettingIndex );
    vector <string> p_IDold = m_PresetVec[ m_CurGroupIndex ].GetParmIDs();
    int pSizeNew = GetNumVars();

    // Check that Parm ID Vec is same size
    if ( p_IDold.size() == pSizeNew )
    {
        for ( int i = 0; i < p_IDold.size(); i++ )
        {
            double val = ParmMgr.FindParm( p_IDold[i] )->Get();
            m_PresetVec[ m_CurGroupIndex ].EditParm( m_CurSettingIndex , p_IDold[ i ], val );
        }
    }
    else if ( p_IDold.size() > pSizeNew || p_IDold.size() < pSizeNew )
    {
        resetFlag = 1;
    }

    return true;
}

void OldVarPresetMgrSingleton::ApplySetting()
{
    //cout << "Setting Change Has Occurred -----------------------------------------" << endl;
    int group_index = GetActiveGroupIndex();

    // Group Contains at least 1 Setting
    if (m_CurSettingIndex == -1)
    {
        return;
    }

    // Change Parm Values accordingly
    // If group has any parms to change
    if ( m_PresetVec[group_index].GetParmIDs().size() > 0 )
    {
        vector <string> p_IDs = m_PresetVec[ group_index ].GetParmIDs();
        vector <double> p_val = m_PresetVec[ group_index ].GetParmVals( m_CurSettingIndex );
        for ( int j = 0; j < p_IDs.size(); j++ )
        {
            // Change Values in ParmScreen
            ParmMgr.FindParm( p_IDs[ j ] )->Set( p_val[ j ] );
        }

        VehicleMgr.GetVehicle()->Update();
    }
}

//==== Setting Change ====//
void OldVarPresetMgrSingleton::SettingChange( int set_index )
{
    //cout << "Setting Change Has Occurred -----------------------------------------" << endl;
    int group_index = GetActiveGroupIndex();

    // Group Contains at least 1 Setting
    if (set_index == -1)
    {
        return;
    }

    // Change Current Setting Index and Text
    m_CurSettingIndex = set_index;
    m_CurSettingText = m_PresetVec[ group_index ].GetSettingName( set_index );

    // Change Current Set Name in Preset
    m_PresetVec[ group_index ].SetCurSetName( m_CurSettingText );

    m_PrevDeleteFlag = false;
}

void OldVarPresetMgrSingleton::SettingChange( const string & set_name )
{
    int set_index = vector_find_val( m_PresetVec[ m_CurGroupIndex ].GetSettingNameVec(), set_name );

    SettingChange( set_index );
}

//==== Group Change ====//
void OldVarPresetMgrSingleton::GroupChange( int group_index )
{
    //cout << "Group Change Has Occurred -------------------------------" << endl;
    // Get Current Group Index and Text
    m_CurGroupIndex = group_index;
    m_CurGroupText = m_PresetVec[ m_CurGroupIndex ].GetGroupName();

    // Find Set Index of Last Active Set in Current Group
    m_CurSettingIndex = vector_find_val( m_PresetVec[ m_CurGroupIndex ].GetSettingNameVec(), m_PresetVec[ m_CurGroupIndex ].GetCurSetName() );

    // Delete All
    DelAllVars();

    // Replace All
    vector <string> p_ID = m_PresetVec[ m_CurGroupIndex ].GetParmIDs();
    vector <double> p_valvec = m_PresetVec[ m_CurGroupIndex ].GetParmVals( m_CurSettingIndex );
    for ( int i = 0; i < p_ID.size(); i++ )
    {
        if ( ParmMgr.FindParm( p_ID[ i ] ) )
        {
            AddVar( p_ID[i] );
        }
        else
        {
            m_PresetVec[m_CurGroupIndex].DeleteParm( p_ID[ i ] );
        }
    }

    SettingChange( m_CurSettingIndex );

    // Set Active Group
    if ( m_CurSettingIndex != -1 && m_PresetVec[ m_CurGroupIndex ].GetNumSet() > 0 )
    {
        for ( int i = 0; i < m_PresetVec.size(); i++ )
        {
            if ( i == m_CurGroupIndex )
            {
                m_CurGroupText = m_PresetVec[ i ].GetGroupName();
                m_CurSettingText = m_PresetVec[ i ].GetSettingName( m_CurSettingIndex );
            }
        }
    }
    else
    {
        // Delete All
        DelAllVars();
        m_CurSettingText = "";
    }

    m_PrevDeleteFlag = false;
}

void OldVarPresetMgrSingleton::GroupChange( const string &group_name )
{
    int g_index = vector_find_val( GetGroupNames(), group_name );

    if ( g_index != -1 )
    {
        GroupChange( g_index );
    }
}

//==== Delete Preset ====//
bool OldVarPresetMgrSingleton::DeletePreset( int group_index, int set_index )
{
    //cout << "Deleting Preset ===============================" << endl;

    // Check if Group and Preset is Empty
    if ( CheckForEmpty( group_index, set_index ) )
    {
        return false;
    }

    if ( set_index == -1 )
    {
        m_PresetVec.erase( m_PresetVec.begin() + group_index );
        ResetIndexAndText();
        m_PrevDeleteFlag = true;
        return true;
    }

    OldPreset group = m_PresetVec[group_index];

    // If Group Has ONLY 1 Setting Delete It All
    if (group.GetNumSet() == 1)
    {
        m_PresetVec.erase( m_PresetVec.begin() + group_index );

        // Refresh Parm Picker
        Wype();

        // If that was the only group react accordingly
        if (m_PresetVec.size() == 0)
        {
            ResetIndexAndText();
        }
        else
        {
            m_CurGroupIndex = 0;
            m_CurSettingIndex = 0;
            m_CurGroupText = "";
            m_CurSettingText = "";
        }
    }
    else // else delete setting and it's connected values
    {
        group.DeleteSet( set_index );

        m_CurSettingIndex = 0;
        m_CurSettingText = "";
        group.SetCurSetName( group.GetSettingName( 0 ) );

        m_PresetVec[ group_index ] = group;
    }

    m_PrevDeleteFlag = true;
    return true;
}

//==== Delete Save Parm Setting ====//
bool OldVarPresetMgrSingleton::DeletePreset( const string &group_name, const string &setting_name )
{
    int group_index = vector_find_val( GetGroupNames(), group_name );
    int set_index = vector_find_val( m_PresetVec[ group_index ].GetSettingNameVec(), setting_name );

    return DeletePreset( group_index, set_index );
}

vector <string> OldVarPresetMgrSingleton::GetParmIDs( const string &group_name )
{
    int group_index = vector_find_val( GetGroupNames(), group_name );
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

vector <double> OldVarPresetMgrSingleton::GetParmVals( const string &group_name, const string &setting_name )
{
    int group_index = vector_find_val( GetGroupNames(), group_name );
    int set_index = vector_find_val( m_PresetVec[ group_index ].GetSettingNameVec(), setting_name );
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

vector <string> OldVarPresetMgrSingleton::GetSettingNames( const string & group_name )
{
    int group_index = vector_find_val( GetGroupNames(), group_name );
    if ( group_index >= 0 )
    {
        return GetSettingNames( group_index );
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

//==== Encode XML ====//
xmlNodePtr OldVarPresetMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    // Create Var Preset Node
    xmlNodePtr varpresetnode = xmlNewChild( node, NULL, BAD_CAST"VariablePresets", NULL );

    // Initialize Variables
    vector <int> numSetqGroup( m_PresetVec.size() );
    vector <int> numParmqGroup( m_PresetVec.size() );

    for ( int i = 0; i < m_PresetVec.size(); i++ )
    {
        // Grab # of Settings per Group
        numSetqGroup[ i ]  = m_PresetVec[ i ].GetNumSet();

        // Grab # of Parms per Group
        numParmqGroup[ i ] = m_PresetVec[ i ].GetParmIDs().size();

        // Let Preset Class Encode Preset Info
        m_PresetVec[ i ].EncodeXml( varpresetnode, i );
    }

    // Encode # of Groups
    XmlUtil::AddIntNode( varpresetnode, "NumGroups", m_PresetVec.size() );

    // Encode # of Settings per Group
    XmlUtil::AddVectorIntNode( varpresetnode, "NumSettingsPerGroup", numSetqGroup );

    // Encode # of Parm per Group
    XmlUtil::AddVectorIntNode( varpresetnode, "NumParmsPerGroup", numParmqGroup );

    // Encode Current Group and Setting Index
    XmlUtil::AddIntNode( varpresetnode, "CurrentGroupIndex", m_CurGroupIndex );
    XmlUtil::AddIntNode( varpresetnode, "CurrentSettingIndex", m_CurSettingIndex );

    return varpresetnode;
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

    // Decode Group and Setting Index
    int val = -1;
    m_CurGroupIndex = XmlUtil::FindInt( varpresetnode, "CurrentGroupIndex", val );
    m_CurSettingIndex = XmlUtil::FindInt( varpresetnode, "CurrentSettingIndex", val );

    if ( m_CurGroupIndex >= 0 && m_CurSettingIndex >= 0 )
    {
        // Assign Current Group and Setting Name
        m_CurGroupText = m_PresetVec[m_CurGroupIndex].GetGroupName();
        m_CurSettingText = m_PresetVec[ m_CurGroupIndex ].GetSettingName( m_CurSettingIndex );
        m_PresetVec[ m_CurGroupIndex ].SetCurSetName( m_CurSettingText );
    }

    return varpresetnode;
}
