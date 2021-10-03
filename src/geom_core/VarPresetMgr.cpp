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
//=======================        Preset        ============================//
//=========================================================================//

//==== Constructor ====//
Preset::Preset()
{
    m_GroupName = "";
}

//==== Destructor ====//
Preset::~Preset()
{
}

//==== Init ====//
void Preset::Init( const string &group_name, const vector < string > &p_IDvec)
{
    m_GroupName = group_name;

    m_ParmIDVec = p_IDvec;
}

//==== Get Setting Index ====//
int Preset::GetSettingIndex()
{
    if ( !m_SettingNameVec.empty() )
    {
        return vector_find_val( m_SettingNameVec, m_CurSetName );
    }
    else
    {
        return -1;
    }
}

int Preset::GetSettingIndex( const string &name )
{
    return vector_find_val( m_SettingNameVec, name );
}

//==== New Setting ====//
void Preset::NewSet( const string &set_name, vector < double > p_ValVec)
{
    m_SettingNameVec.push_back( set_name );

    m_ParmValVec.push_back( p_ValVec );

    m_CurSetName = set_name;
}

//==== Delete Setting ====//
void Preset::DeleteSet( int set_index )
{
    m_SettingNameVec.erase( m_SettingNameVec.begin() + set_index );
    m_ParmValVec.erase( m_ParmValVec.begin() + set_index );
}

//==== EditParm ====//
void Preset::EditParm( int set_index, const string &p_id, double p_val )
{
    int p_index = vector_find_val( m_ParmIDVec, p_id );
    m_ParmValVec[ set_index ][ p_index ] = p_val;
}

//==== Setting Name Exist ====//
bool Preset::SettingNameExist( const string &name )
{
    return vector_contains_val( m_SettingNameVec, name );
}

//==== Add Parm ====//
void Preset::AddParm( const string &p_ID, double p_val )
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
void Preset::DeleteParm( const string &p_id )
{
    int p_index = vector_find_val( m_ParmIDVec, p_id );

    m_ParmIDVec.erase( m_ParmIDVec.begin() + p_index );

    for (int i = 0; i < (int)m_ParmValVec.size(); i++ )
    {
        m_ParmValVec[ i ].erase( m_ParmValVec[ i ].begin() + p_index );
    }
}

//==== Encode XML ====//
xmlNodePtr Preset::EncodeXml( xmlNodePtr &varpresetnode, int i )
{
    char buffer [50];

    // Encode Group Name
    sprintf( buffer, "Group%i", i );
    XmlUtil::AddStringNode( varpresetnode, buffer, m_GroupName );

    // Encode Group Qualities
    sprintf( buffer, "Group%iQualities", i );
    xmlNodePtr groupqualnode = xmlNewChild( varpresetnode, NULL, BAD_CAST buffer, NULL );

    XmlUtil::AddStringNode( groupqualnode, "ActiveSettingName" , m_CurSetName );
    for ( int j = 0; j < m_ParmIDVec.size(); j++ )
    {
        sprintf(buffer, "ParmID%i", j );
        XmlUtil::AddStringNode( groupqualnode, buffer, m_ParmIDVec[ j ] );
    }

    // Encode Setting Qualities
    for ( int j = 0; j < m_SettingNameVec.size(); j++ )
    {
        sprintf( buffer, "SettingName%i", j );
        XmlUtil::AddStringNode( groupqualnode, buffer , m_SettingNameVec[ j ] );

        sprintf( buffer, "Setting%iQualities", j );
        xmlNodePtr setqualnode = xmlNewChild( groupqualnode, NULL, BAD_CAST buffer, NULL );

        XmlUtil::AddVectorDoubleNode( setqualnode, "ParmVals", m_ParmValVec[ j ] );
    }

    return varpresetnode;
}

//==== Decode XML ====//
Preset Preset::DecodeXml( xmlNodePtr &varpresetnode, int i )
{
    char buffer [50];
    string name = "";
    string groupName;
    string setName;
    vector < double > p_vals;
    vector < string > p_IDs;
    Preset temp_preset;

    // Grab Size of Settings and Parms/Setting
    vector <int> numSetqGroup = XmlUtil::ExtractVectorIntNode( varpresetnode, "NumSettingsPerGroup" );
    vector <int> numParmqGroup = XmlUtil::ExtractVectorIntNode( varpresetnode, "NumParmsPerGroup" );

    // Grab Group Qualities
    sprintf( buffer, "Group%i", i );
    groupName = XmlUtil::FindString( varpresetnode, buffer, name );

    sprintf( buffer, "Group%iQualities", i );
    xmlNodePtr groupqualnode = XmlUtil::GetNode( varpresetnode, buffer, 0 );

    for ( int j = 0; j < numParmqGroup[ i ]; j++ )
    {
        sprintf(buffer, "ParmID%i", j);
        p_IDs.push_back( XmlUtil::FindString( groupqualnode, buffer, name) );
    }

    // Grab Setting Qualities
    for ( int j = 0; j < numSetqGroup[ i ]; j++ )
    {
        sprintf( buffer, "SettingName%i", j );
        setName = XmlUtil::FindString( groupqualnode, buffer, name );

        sprintf( buffer, "Setting%iQualities", j );
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
VarPresetMgrSingleton::VarPresetMgrSingleton()
{
    Init();
}

void VarPresetMgrSingleton::Init()
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

void VarPresetMgrSingleton::ResetIndexAndText()
{
    m_CurGroupIndex = -1;
    m_CurSettingIndex = -1;
    m_CurGroupText = "";
    m_CurSettingText = "";
}

void VarPresetMgrSingleton::Wype()
{
    m_CurrVarIndex = int();
    m_WorkingParmID = string();

    DelAllVars();
    m_VarVec = vector< string >();
}

void VarPresetMgrSingleton::Renew()
{
    Wype();
    Init();
}

//==== Get Current Design Variable ====//
string VarPresetMgrSingleton::GetCurrVar()
{
    return GetVar( m_CurrVarIndex );
}

//==== Get Design Variable Given Index ====//
string VarPresetMgrSingleton::GetVar( int index )
{
    if ( index >= 0 && index < ( int )m_VarVec.size() )
    {
        return m_VarVec[ index ];
    }
    return string();
}

//==== Add Curr Variable ====//
bool VarPresetMgrSingleton::AddCurrVar()
{
    //==== Check if Modifying Already Add Link ====//
    if (  m_CurrVarIndex >= 0 && m_CurrVarIndex < ( int )m_VarVec.size() )
    {
        return false;
    }

    if ( CheckForDuplicateParm( m_WorkingParmID ) )
    {
        return false;
    }

    return AddVar( m_WorkingParmID );
}

//==== Check For Duplicate Variable  ====//
bool VarPresetMgrSingleton::CheckForDuplicateVar( const string &p )
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

bool VarPresetMgrSingleton::SortVars()
{
    bool wassorted = std::is_sorted( m_VarVec.begin(), m_VarVec.end(), NameCompare );

    if ( !wassorted )
    {
        std::sort( m_VarVec.begin(), m_VarVec.end(), NameCompare );
    }

    return wassorted;
}

//==== Add New Variable ====//
bool VarPresetMgrSingleton::AddVar( const string& parm_id  )
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

//==== Check All Vars For Valid Parms ====//
void VarPresetMgrSingleton::CheckVars()
{
    //==== Check If Any Parms Have Added/Removed From Last Check ====//
    static int check_links_stamp = 0;
    if ( ParmMgr.GetNumParmChanges() == check_links_stamp )
    {
        return;
    }

    check_links_stamp = ParmMgr.GetNumParmChanges();

    deque< int > del_indices;
    for ( int i = 0 ; i < ( int )m_VarVec.size() ; i++ )
    {
        Parm* pA = ParmMgr.FindParm( m_VarVec[ i ] );

        if ( !pA )
        {
            del_indices.push_front( i );
        }
    }

    if ( del_indices.size() )
    {
        m_CurrVarIndex = -1;
    }

    // Every Parm in Preset was Deleted w/ Connected Geom
    if ( del_indices.size() == m_VarVec.size() )
    {
        m_VarVec.clear();
    }
    // Some Parms in Preset were Deleted w/ Connected Geom
    else
    {
        for ( int i = del_indices.size() - 1; i >= 0; i-- )
        {
            m_VarVec.erase( m_VarVec.begin() + del_indices[ i ] );
        }
    }

}

//==== Delete Curr Variable ====//
void VarPresetMgrSingleton::DelCurrVar()
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
void VarPresetMgrSingleton::DelAllVars()
{
    m_VarVec.clear();
    m_CurrVarIndex = -1;
}

//==== Reset Working Variable ====//
void VarPresetMgrSingleton::ResetWorkingVar()
{
    m_CurrVarIndex = -1;

    m_WorkingParmID = string();
}

//==== Set Working Parm ID ====//
void VarPresetMgrSingleton::SetWorkingParmID( const string & parm_id )
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
bool VarPresetMgrSingleton::CheckForEmpty( int curGroupIndex, int curSettingIndex )
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

int VarPresetMgrSingleton::GetNumSet()
{
    if ( m_CurGroupIndex < 0 || m_CurGroupIndex >= m_PresetVec.size() )
    {
        return 0;
    }

    return m_PresetVec[ m_CurGroupIndex ].GetNumSet();
}

//====  Add Group ====//
void VarPresetMgrSingleton::AddGroup( const string &groupText )
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

        Preset new_group;
        new_group.Init( groupText, pIDvec );
        m_PresetVec.push_back( new_group );
    }


    // If Group Does Not Exist, Create
    if ( groupExistFlag == 0 )
    {
        Preset new_group;
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
void VarPresetMgrSingleton::AddSetting( const string &settingText )
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
bool VarPresetMgrSingleton::SavePreset()
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

void VarPresetMgrSingleton::ApplySetting()
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
void VarPresetMgrSingleton::SettingChange( int set_index )
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

void VarPresetMgrSingleton::SettingChange( const string & set_name )
{
    int set_index = vector_find_val( m_PresetVec[ m_CurGroupIndex ].GetSettingNameVec(), set_name );

    SettingChange( set_index );
}

//==== Group Change ====//
void VarPresetMgrSingleton::GroupChange( int group_index )
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

void VarPresetMgrSingleton::GroupChange( const string &group_name )
{
    int g_index = vector_find_val( GetGroupNames(), group_name );

    if ( g_index != -1 )
    {
        GroupChange( g_index );
    }
}

//==== Delete Preset ====//
bool VarPresetMgrSingleton::DeletePreset( int group_index, int set_index )
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

    Preset group = m_PresetVec[group_index];

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
bool VarPresetMgrSingleton::DeletePreset( const string &group_name, const string &setting_name )
{
    int group_index = vector_find_val( GetGroupNames(), group_name );
    int set_index = vector_find_val( m_PresetVec[ group_index ].GetSettingNameVec(), setting_name );

    return DeletePreset( group_index, set_index );
}

vector <string> VarPresetMgrSingleton::GetParmIDs( const string &group_name )
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

vector <double> VarPresetMgrSingleton::GetParmVals( const string &group_name, const string &setting_name )
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
vector <string> VarPresetMgrSingleton::GetSettingNames( int group_index )
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

vector <string> VarPresetMgrSingleton::GetSettingNames( const string & group_name )
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
vector <string> VarPresetMgrSingleton::GetGroupNames()
{
    vector <string> vec( m_PresetVec.size() );
    for ( int i = 0; i < m_PresetVec.size(); i++ )
    {
        vec[i] = m_PresetVec[ i ].GetGroupName();
    }
    return vec;
}

//==== Check For Repeat Parameters ====//
// Checks for Repeat Parameters across all groups
bool VarPresetMgrSingleton::CheckForDuplicateParm( const string & p_ID )
{
    for ( int i = 0; i < m_PresetVec.size(); i++ )
    {
        vector <string> IDvec = m_PresetVec[ i ].GetParmIDs();
        if ( vector_contains_val( IDvec, p_ID ) )
        {
            return true;
        }
    }
    return false;
}

//==== Encode XML ====//
xmlNodePtr VarPresetMgrSingleton::EncodeXml( xmlNodePtr & node )
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
xmlNodePtr VarPresetMgrSingleton::DecodeXml( xmlNodePtr & node )
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
